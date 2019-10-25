/** @file
  Aml Tree.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Aml.h"
#include "AmlNode.h"
#include "AmlTree.h"
#include "AmlTreeTraversal.h"
#include "AmlUtility.h"

#include <Library/AmlLib/AmlInterface.h>

/** Returns the count of the fixed arguments for the input Node.

  @param  [in]  Node  Pointer to a node.

  @return Number of fixed arguments.
**/
AML_OP_PARSE_INDEX
AmlGetFixedArgumentCount (
  IN  AML_OBJECT_NODE   * Node
  )
{
  if (IS_AML_OBJECT_NODE (Node) &&
      (Node->AmlByteEncoding != NULL)) {
    return Node->AmlByteEncoding->MaxIndex;
  }

  return 0;
}

/** Get the node at the input Index in the fixed argument list of the input
    ObjectNode.

  @param  [in]  ObjectNode  Pointer to an object node.
  @param  [in]  Index       The Index of the fixed argument to get.

  @return The node at the input Index in the fixed argument list
          of the input ObjectNode.
          NULL otherwise, e.g. if the node is not an object node, or no
          node is available at this Index.
**/
AML_NODE_HEADER *
EFIAPI
AmlGetFixedArgument (
  IN  AML_OBJECT_NODE     * ObjectNode,
  IN  AML_OP_PARSE_INDEX    Index
  )
{
  if (IS_AML_OBJECT_NODE (ObjectNode)) {
    if (Index < AmlGetFixedArgumentCount (ObjectNode)) {
      return ObjectNode->FixedArgs[Index];
    }
  }

  return NULL;
}

/** Check whether the input Node is in the fixed argument list of its parent
    node.

  If so, IndexPtr contains this Index.

  @param  [in]  Node          Pointer to a Node.
  @param  [out] IndexPtr      Pointer holding the Index of the Node in
                              its parent's fixed argument list.

  @retval TRUE   The node is a fixed argument and the index
                 in IndexPtr is valid.
  @retval FALSE  The node is not a fixed argument.
**/
BOOLEAN
EFIAPI
AmlIsNodeFixedArgument (
  IN  CONST  AML_NODE_HEADER     * Node,
  OUT        AML_OP_PARSE_INDEX  * IndexPtr
  )
{
  AML_NODE_HEADER         * ParentNode;

  AML_OP_PARSE_INDEX        Index;
  AML_OP_PARSE_INDEX        MaxIndex;

  if ((IndexPtr == NULL)               ||
      (!IS_AML_DATA_NODE (Node)        &&
       !IS_AML_OBJECT_NODE (Node))) {
    ASSERT (0);
    return FALSE;
  }

  ParentNode = AmlGetParent ((AML_NODE_HEADER*)Node);
  if (!IS_AML_ROOT_NODE (ParentNode)    &&
      !IS_AML_OBJECT_NODE (ParentNode)) {
    ASSERT (0);
    return FALSE;
  }

  // Check whether the Node is in the fixed argument list.
  MaxIndex = AmlGetFixedArgumentCount ((AML_OBJECT_NODE*)ParentNode);
  for (Index = AML_OP_PARSE_INDEX_GET_TERM1; Index < MaxIndex; Index++) {
    if (AmlGetFixedArgument ((AML_OBJECT_NODE*)ParentNode, Index)
          == Node) {
      *IndexPtr = Index;
      return TRUE;
    }
  }

  return FALSE;
}

/** Set the fixed argument of the ObjectNode at the Index to the NewNode.

  It is the caller's responsability to save the old node, if desired,
  otherwise the reference to the old node will be lost.
  If NewNode is not NULL, set its parent to ObjectNode.

  @param  [in]  ObjectNode    Pointer to an object node.
  @param  [in]  Index         Index in the fixed argument list of
                              the ObjectNode to set.
  @param  [in]  NewNode       Pointer to the NewNode.
                              If not NULL, must be a data or an object node.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlSetFixedArgument (
  IN  AML_OBJECT_NODE     * ObjectNode,
  IN  AML_OP_PARSE_INDEX    Index,
  IN  AML_NODE_HEADER     * NewNode
  )
{
  if (IS_AML_OBJECT_NODE (ObjectNode)                   &&
      (Index >= AML_OP_PARSE_INDEX_GET_TERM1)           &&
      (Index <= AmlGetFixedArgumentCount (ObjectNode))  &&
      ((NewNode == NULL)                                ||
       IS_AML_OBJECT_NODE (NewNode)                     ||
       IS_AML_DATA_NODE (NewNode))) {
    ObjectNode->FixedArgs[Index] = NewNode;

    // If NewNode is a data node or an object node, set its parent.
    if (NewNode != NULL) {
      NewNode->Parent = (AML_NODE_HEADER*)ObjectNode;
    }

    return EFI_SUCCESS;
  }

  ASSERT (0);
  return EFI_INVALID_PARAMETER;
}

/** If the given AML_NODE_HEADER has a variable list of arguments,
    return a pointer to this list.
    Return NULL otherwise.

  @param  [in]  Node  Pointer to the AML_NODE_HEADER to check.

  @return The list of variable arguments if there is one.
          NULL otherwise.
**/
LIST_ENTRY *
EFIAPI
AmlNodeGetVariableArgList (
  IN  CONST AML_NODE_HEADER   * Node
  )
{
  if (IS_AML_ROOT_NODE (Node)) {
    return &(((AML_ROOT_NODE*)Node)->VariableArgs);
  }
  else if (IS_AML_OBJECT_NODE (Node)) {
    return &(((AML_OBJECT_NODE*)Node)->VariableArgs);
  }
  return NULL;
}

/** Remove the Node from its parent's variable list of arguments.

  The function will fail if the Node is in its parent's fixed
  argument list.
  The Node is not deleted. The deletion is done separetely
  from the removal.

  @param  [in]  Node  Pointer to a Node.
                      Must be a data node or an object node.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlRemoveNodeFromVarArgList (
  IN  AML_NODE_HEADER   * Node
  )
{
  EFI_STATUS          Status;
  AML_NODE_HEADER   * ParentNode;
  UINT32              Size;

  if ((!IS_AML_DATA_NODE (Node)           &&
       !IS_AML_OBJECT_NODE (Node))) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  ParentNode = AmlGetParent (Node);
  if (!IS_AML_ROOT_NODE (ParentNode)    &&
      !IS_AML_OBJECT_NODE (ParentNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Check the node is in its parent variable list of arguments.
  if (!IsNodeInList (
         AmlNodeGetVariableArgList (ParentNode),
         &Node->Link)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Unlink Node from the tree.
  RemoveEntryList (&Node->Link);
  InitializeListHead (&Node->Link);
  Node->Parent = NULL;

  // Get the size of the node removed.
  Size = 0;
  Status = AmlComputeSize (Node, &Size);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Propagate the information.
  Status = AmlPropagateInformation (ParentNode, FALSE, Size, 1);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  return EFI_SUCCESS;
}

/** Add the NewNode to the head of the variable list of arguments
    of the ParentNode.

  @param  [in]  ParentNode  Pointer to the parent node.
                            Must be a root or an object node.
  @param  [in]  NewNode     Pointer to the node to add.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlVarListAddHead (
  IN  AML_NODE_HEADER  * ParentNode,
  IN  AML_NODE_HEADER  * NewNode
  )
{
  EFI_STATUS    Status;
  UINT32        NewSize;
  LIST_ENTRY  * ChildrenList;

  // Check arguments and that NewNode is not already attached to a tree.
  if ((!IS_AML_ROOT_NODE (ParentNode)     &&
       !IS_AML_OBJECT_NODE (ParentNode))  ||
      (!IS_AML_DATA_NODE (NewNode)        &&
       !IS_AML_OBJECT_NODE (NewNode))     ||
      !AML_NODE_IS_DETACHED (NewNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Insert it at the head of the list.
  ChildrenList = AmlNodeGetVariableArgList (ParentNode);
  if (ChildrenList == NULL) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  InsertHeadList (ChildrenList, &NewNode->Link);
  NewNode->Parent = ParentNode;

  // Get the size of the NewNode.
  NewSize = 0;
  Status = AmlComputeSize (NewNode, &NewSize);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Propagate the new information.
  Status = AmlPropagateInformation (ParentNode, TRUE, NewSize, 1);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  return EFI_SUCCESS;
}

/** Add the NewNode to the tail of the variable list of arguments
    of the ParentNode.

  This is an internal function which is not propagating the size
  at each new added node.

  @param  [in]  ParentNode  Pointer to the parent node.
                            Must be a root or an object node.
  @param  [in]  NewNode     Pointer to the node to add.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlVarListAddTailInternal (
  IN  AML_NODE_HEADER  * ParentNode,
  IN  AML_NODE_HEADER  * NewNode
  )
{
  LIST_ENTRY  * ChildrenList;

  // Check arguments and that NewNode is not already attached to a tree.
  if ((!IS_AML_ROOT_NODE (ParentNode)     &&
       !IS_AML_OBJECT_NODE (ParentNode))  ||
      (!IS_AML_DATA_NODE (NewNode)        &&
       !IS_AML_OBJECT_NODE (NewNode))     ||
      !AML_NODE_IS_DETACHED (NewNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Insert it at the tail of the list.
  ChildrenList = AmlNodeGetVariableArgList (ParentNode);
  if (ChildrenList == NULL) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  InsertTailList (ChildrenList, &NewNode->Link);
  NewNode->Parent = ParentNode;

  return EFI_SUCCESS;
}

/** Add the NewNode to the tail of the variable list of arguments
    of the ParentNode.

  @param  [in]  ParentNode  Pointer to the parent node.
                            Must be a root or an object node.
  @param  [in]  NewNode     Pointer to the node to add.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlVarListAddTail (
  IN  AML_NODE_HEADER   * ParentNode,
  IN  AML_NODE_HEADER   * NewNode
  )
{
  EFI_STATUS  Status;
  UINT32      NewSize;

  // Add the NewNode and check arguments.
  Status = AmlVarListAddTailInternal (ParentNode, NewNode);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Get the size of the NewNode.
  NewSize = 0;
  Status = AmlComputeSize (NewNode, &NewSize);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Propagate the new information.
  Status = AmlPropagateInformation (ParentNode, TRUE, NewSize, 1);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  return EFI_SUCCESS;
}

/** Add the NewNode after the Node in the variable list of arguments
    of the Node's parent.

  @param  [in]  Node      Pointer to a node.
                          Must be a root or an object node.
  @param  [in]  NewNode   Pointer to the node to add.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlVarListAddAfter (
  IN  AML_NODE_HEADER   * Node,
  IN  AML_NODE_HEADER   * NewNode
  )
{
  EFI_STATUS          Status;
  AML_NODE_HEADER   * ParentNode;
  UINT32              NewSize;

  // Check arguments and that NewNode is not already attached to a tree.
  if ((!IS_AML_DATA_NODE (NewNode)        &&
       !IS_AML_OBJECT_NODE (NewNode))     ||
      !AML_NODE_IS_DETACHED (NewNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  ParentNode = AmlGetParent (Node);
  if (!IS_AML_ROOT_NODE (ParentNode)    &&
      !IS_AML_OBJECT_NODE (ParentNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Insert it at after the input Node.
  InsertHeadList (&Node->Link, &NewNode->Link);
  NewNode->Parent = ParentNode;

  // Get the size of the NewNode.
  NewSize = 0;
  Status = AmlComputeSize (NewNode, &NewSize);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Propagate the new information.
  Status = AmlPropagateInformation (ParentNode, TRUE, NewSize, 1);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  return EFI_SUCCESS;
}

/** Add the NewNode before the Node in the list of variable
    arguments of the Node's parent.

  @param  [in]  Node      Pointer to a node.
                          Must be a root or an object node.
  @param  [in]  NewNode   Pointer to the node to add.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlVarListAddBefore (
  IN  AML_NODE_HEADER  * Node,
  IN  AML_NODE_HEADER  * NewNode
  )
{
  EFI_STATUS         Status;
  AML_NODE_HEADER  * ParentNode;
  UINT32             NewSize;

  // Check arguments and that NewNode is not already attached to a tree.
  if ((!IS_AML_DATA_NODE (NewNode)        &&
       !IS_AML_OBJECT_NODE (NewNode))     ||
      !AML_NODE_IS_DETACHED(NewNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  ParentNode = AmlGetParent (Node);
  if (!IS_AML_ROOT_NODE (ParentNode)    &&
      !IS_AML_OBJECT_NODE (ParentNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Insert it at before the input Node.
  InsertTailList (&Node->Link, &NewNode->Link);
  NewNode->Parent = ParentNode;

  // Get the size of the NewNode.
  NewSize = 0;
  Status = AmlComputeSize (NewNode, &NewSize);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Propagate the new information.
  Status = AmlPropagateInformation (ParentNode, TRUE, NewSize, 1);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  return EFI_SUCCESS;
}

/** Replace the fixed argument at the Index of the ParentNode with the NewNode.

  @param  [in]  ParentNode  Pointer to the parent node.
                            Must be an object node.
  @param  [in]  Index       Index of the fixed argument to replace.
  @param  [in]  NewNode     The new node to insert.
                            Must be an object node or a data node.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
EFI_STATUS
EFIAPI
AmlReplaceFixedArgument (
  IN  AML_OBJECT_NODE      * ParentNode,
  IN  AML_OP_PARSE_INDEX     Index,
  IN  AML_NODE_HEADER      * NewNode
  )
{
  EFI_STATUS              Status;

  AML_NODE_HEADER       * OldNode;
  UINT32                  NewSize;
  UINT32                  OldSize;
  AML_OP_PARSE_FORMAT     FixedArgType;

  // Check arguments and that NewNode is not already attached to a tree.
  if (!IS_AML_OBJECT_NODE (ParentNode)  ||
      (!IS_AML_DATA_NODE (NewNode)      &&
       !IS_AML_OBJECT_NODE (NewNode))   ||
      !AML_NODE_IS_DETACHED (NewNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Perform some compatibility checks between NewNode and OldNode.
  FixedArgType = ParentNode->AmlByteEncoding->Format[Index];
  switch (FixedArgType) {
    case AML_UINT8:
    case AML_UINT16:
    case AML_UINT32:
    case AML_UINT64:
    case AML_NAME:
    case AML_STRING:
    {
      // A uint, a name, and a string can be replaced by either a data node
      // of the name type, or an object node.
      if ((IS_AML_DATA_NODE (NewNode) &&
          (((AML_DATA_NODE*)NewNode)->DataType !=
            AmlTypeToAcpiType (FixedArgType))) &&
          (!IS_AML_OBJECT_NODE (NewNode))) {
        ASSERT (0);
        return EFI_INVALID_PARAMETER;
      }
      break;
    }
    case AML_OBJECT:
    {
      // If it's an object node, the grammar is too complex to do any check.
      break;
    }
    case AML_NONE:
    default:
    {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
      break;
    }
  } // switch

  // Replace the OldNode with the NewNode.
  OldNode = AmlGetFixedArgument (ParentNode, Index);
  if (OldNode == NULL) {
    ASSERT (0);
    return EFI_ABORTED;
  }
  OldNode->Parent = NULL;

  Status = AmlSetFixedArgument (ParentNode, Index, NewNode);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Get the size of the OldNode.
  OldSize = 0;
  Status = AmlComputeSize (OldNode, &OldSize);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Get the size of the NewNode.
  NewSize = 0;
  Status = AmlComputeSize (NewNode, &NewSize);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Propagate the new information.
  Status = AmlPropagateInformation (
            (AML_NODE_HEADER*)ParentNode,
            (NewSize > OldSize) ? TRUE : FALSE,
            (NewSize > OldSize) ? (NewSize - OldSize) : (OldSize - NewSize),
            0
            );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  return EFI_SUCCESS;
}

/** Replace the OldNode, which is in a variable list of arguments,
    with the NewNode.

  OldNode is removed from the tree, but not deleted.

  @param  [in]  OldNode   Pointer to the node to replace.
                          Must be a data node or an object node.
  @param  [in]  NewNode   The new node to insert.
                          Must be a data node or an object node.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
EFI_STATUS
EFIAPI
AmlReplaceVariableArgument (
  IN  AML_NODE_HEADER   * OldNode,
  IN  AML_NODE_HEADER   * NewNode
  )
{
  EFI_STATUS          Status;
  UINT32              NewSize;
  UINT32              OldSize;

  AML_NODE_HEADER   * ParentNode;
  AML_OBJECT_NODE   * ObjectParentNode;
  LIST_ENTRY        * NextLink;

  // Check arguments and that NewNode is not already attached to a tree.
  if ((!IS_AML_DATA_NODE (OldNode)     &&
       !IS_AML_OBJECT_NODE (OldNode))  ||
      (!IS_AML_DATA_NODE (NewNode)     &&
       !IS_AML_OBJECT_NODE (NewNode))  ||
      !AML_NODE_IS_DETACHED (NewNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  ParentNode = AmlGetParent (OldNode);
    if (!IS_AML_ROOT_NODE (ParentNode)    &&
        !IS_AML_OBJECT_NODE (ParentNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Perform some checks.
  if (IS_AML_OBJECT_NODE (ParentNode)) {
    ObjectParentNode = (AML_OBJECT_NODE*)ParentNode;

    // A child node of a node with the BYTE_LIST flag must be a data node.
    if (AmlObjectNodeHasAttribute (ObjectParentNode, AML_HAS_BYTE_LIST) &&
        !IS_AML_DATA_NODE (NewNode)) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;

    } else if (AmlObjectNodeHasAttribute (ObjectParentNode,
                                          AML_HAS_CHILD_OBJ) &&
               (!IS_AML_DATA_NODE (NewNode) ||
                !IS_AML_OBJECT_NODE (NewNode))) {
      // A child node of a node with the HAS_CHILD flag must be a either a
      // data node or an object node.
      ASSERT (0);
      return EFI_INVALID_PARAMETER;

    } else {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
  }

  // Unlink OldNode from the tree.
  NextLink = RemoveEntryList (&OldNode->Link);
  InitializeListHead (&OldNode->Link);
  OldNode->Parent = NULL;

  // Add the NewNode.
  InsertHeadList (NextLink, &NewNode->Link);
  NewNode->Parent = ParentNode;

  // Get the size of the OldNode.
  OldSize = 0;
  Status = AmlComputeSize (OldNode, &OldSize);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Get the size of the NewNode.
  NewSize = 0;
  Status = AmlComputeSize (NewNode, &NewSize);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Propagate the new information.
  Status = AmlPropagateInformation (
             ParentNode,
             (NewSize > OldSize) ? TRUE : FALSE,
             (NewSize > OldSize) ? (NewSize - OldSize) : (OldSize - NewSize),
             0
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  return EFI_SUCCESS;
}

/** Replace the OldNode by the NewNode.

  OldNode is removed from the tree, but not deleted.

  @param  [in]  OldNode   Pointer to the node to replace.
                          Must be a data node or an object node.
  @param  [in]  NewNode   The new node to insert.
                          Must be a data node or an object node.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlReplaceArgument (
  IN  AML_NODE_HEADER   * OldNode,
  IN  AML_NODE_HEADER   * NewNode
  )
{
  EFI_STATUS            Status;
  AML_NODE_HEADER     * ParentNode;
  AML_OP_PARSE_INDEX    Index;

  // Check arguments and that NewNode is not already attached to a tree.
  if ((!IS_AML_DATA_NODE (OldNode)    &&
      !IS_AML_OBJECT_NODE (OldNode))  ||
      (!IS_AML_DATA_NODE (NewNode)    &&
      !IS_AML_OBJECT_NODE (NewNode))  ||
      !AML_NODE_IS_DETACHED (NewNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // ParentNode can only be an object node.
  ParentNode = AmlGetParent (OldNode);
  if (!IS_AML_OBJECT_NODE (ParentNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  if (AmlIsNodeFixedArgument (OldNode, &Index)) {
    // OldNode is in its parent's fixed argument list at the Index.
    Status = AmlReplaceFixedArgument (
               (AML_OBJECT_NODE*)ParentNode,
               Index,
               NewNode
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return Status;
    }
  } else {
    // OldNode is not in its parent's fixed argument list.
    // It must be in its variable list of arguments.
    Status = AmlReplaceVariableArgument (OldNode, NewNode);
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/** Delete a Node and its children.

  The Node must be removed from the tree first,
  or must be the root node.

  @param  [in]  Node  Pointer to the node to delete.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlDeleteTree (
  IN  AML_NODE_HEADER  * Node
  )
{
  EFI_STATUS            Status;

  AML_OP_PARSE_INDEX    Index;
  AML_OP_PARSE_INDEX    MaxIndex;

  AML_NODE_HEADER     * Arg;
  LIST_ENTRY          * StartLink;
  LIST_ENTRY          * CurrentLink;
  LIST_ENTRY          * NextLink;

  // Check that the node being deleted is unlinked.
  // When removing the node, its parent pointer and
  // its lists data structure are reset with
  // InitializeListHead. Thus it must be detached
  // from the tree to avoid memory leaks.
  if (!IS_AML_HEADER (Node)  ||
      !AML_NODE_IS_DETACHED(Node)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Iterate through the fixed list of arguments.
  if (IS_AML_OBJECT_NODE (Node)) {
    MaxIndex = AmlGetFixedArgumentCount ((AML_OBJECT_NODE*)Node);
    for (Index = AML_OP_PARSE_INDEX_GET_TERM1;
         Index < MaxIndex;
         Index++) {

      Arg = AmlGetFixedArgument ((AML_OBJECT_NODE*)Node, Index);
      if (Arg == NULL) {
        ASSERT (0);
        return EFI_ABORTED;
      }

      // Remove the node from the fixed argument list.
      Arg->Parent = NULL;
      Status = AmlSetFixedArgument ((AML_OBJECT_NODE*)Node, Index, NULL);
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        return Status;
      }

      Status = AmlDeleteTree (Arg);
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        return Status;
      }
    }
  }

  // Iterate through the variable list of arguments.
  StartLink = AmlNodeGetVariableArgList (Node);
  if (StartLink != NULL) {
    NextLink = StartLink->ForwardLink;
    while (NextLink != StartLink) {
      CurrentLink = NextLink;

      // Unlink the node from the tree.
      NextLink = RemoveEntryList (CurrentLink);
      InitializeListHead (CurrentLink);
      ((AML_NODE_HEADER*)CurrentLink)->Parent = NULL;

      Status = AmlDeleteTree ((AML_NODE_HEADER*)CurrentLink);
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        return Status;
      }
    } // while
  }

  Status = AmlDeleteNode (Node);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  return Status;
}
