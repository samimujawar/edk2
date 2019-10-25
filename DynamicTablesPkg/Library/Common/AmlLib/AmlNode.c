/** @file
  Aml Node.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Aml.h"
#include "AmlNode.h"
#include "AmlTree.h"

/** Initialize an AML_NODE_HEADER structure.

  @param  [in]  Node      Pointer to a node header.
  @param  [in]  NodeType  NodeType to initialize the Node with.
                          Must be an EAML_NODE_TYPE.
**/
STATIC
VOID
EFIAPI
AmlInitializeAmlNodeHeader (
  IN  AML_NODE_HEADER   * Node,
  IN  EAML_NODE_TYPE      NodeType
  )
{
  ASSERT (Node != NULL);

  InitializeListHead (&Node->Link);

  Node->Parent = NULL;
  Node->NodeType = NodeType;
}

/** Delete a root node and its SSDT header.

  It is the caller's responsability to check the RootNode has been removed
  from the tree and is not referencing any other node in the tree.

  @param  [in]  RootNode  Pointer to a root node.
**/
STATIC
VOID
EFIAPI
AmlDeleteRootNode (
  IN  AML_ROOT_NODE  * RootNode
  )
{
  if (IS_AML_ROOT_NODE (RootNode)) {
    if ((RootNode->SdtHeader != NULL)) {
      FreePool (RootNode->SdtHeader);
    } else {
      ASSERT (0);
    }
    FreePool (RootNode);

  } else {
    ASSERT (0);
  }
}

/** Create an AML_ROOT_NODE.
    This node will be the root of the tree.

  @param  [in]  SdtHeader       Pointer to SSDT header to copy the data from.
  @param  [out] NewRootNodePtr  The created AML_ROOT_NODE.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
**/
EFI_STATUS
EFIAPI
AmlCreateRootNode (
  IN  CONST EFI_ACPI_DESCRIPTION_HEADER   * SdtHeader,
  OUT       AML_ROOT_NODE                ** NewRootNodePtr
  )
{
  AML_ROOT_NODE   * RootNode;
  UINT8           * Buffer;

  if ((SdtHeader == NULL) ||
      (NewRootNodePtr == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  RootNode = AllocateZeroPool (sizeof (AML_ROOT_NODE));
  if (RootNode == NULL) {
    ASSERT (0);
    return EFI_OUT_OF_RESOURCES;
  }

  AmlInitializeAmlNodeHeader (&RootNode->NodeHeader, EAmlNodeRoot);
  InitializeListHead (&RootNode->VariableArgs);

  Buffer = AllocateZeroPool (sizeof (EFI_ACPI_DESCRIPTION_HEADER));
  if (Buffer == NULL) {
    ASSERT (Buffer != NULL);
    AmlDeleteRootNode (RootNode);
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (Buffer, SdtHeader, sizeof (EFI_ACPI_DESCRIPTION_HEADER));
  RootNode->SdtHeader = (EFI_ACPI_DESCRIPTION_HEADER*)Buffer;

  *NewRootNodePtr = RootNode;

  return EFI_SUCCESS;
}

/** Delete an object node.

  It is the caller's responsability to check the ObjectNode has been removed
  from the tree and is not referencing any other node in the tree.

  @param  [in]  ObjectNode  Pointer to an object node.
**/
STATIC
VOID
EFIAPI
AmlDeleteObjectNode (
  IN  AML_OBJECT_NODE   * ObjectNode
  )
{
  if (IS_AML_OBJECT_NODE (ObjectNode)) {
    FreePool (ObjectNode);
  } else {
    ASSERT (0);
  }
}

/** Create an AML_OBJECT_NODE.

  @param  [in]  AmlByteEncoding   Byte encoding entry.
  @param  [in]  PkgLength         PkgLength of the node if the AmlByteEncoding
                                  has the PkgLen attribute.
                                  0 Otherwise.
  @param  [out] NewObjectNodePtr  The created AML_OBJECT_NODE.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
**/
EFI_STATUS
EFIAPI
AmlCreateObjectNode (
  IN  CONST  AML_BYTE_ENCODING   * AmlByteEncoding,
  IN         UINT32                PkgLength,
  OUT        AML_OBJECT_NODE    ** NewObjectNodePtr
  )
{
  AML_OBJECT_NODE     * ObjectNode;
  AML_OP_PARSE_INDEX    Index;

  if ((AmlByteEncoding == NULL)  ||
      (NewObjectNodePtr == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  ObjectNode = AllocateZeroPool (sizeof (AML_OBJECT_NODE));
  if (ObjectNode == NULL) {
    ASSERT (ObjectNode != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  AmlInitializeAmlNodeHeader (&ObjectNode->NodeHeader, EAmlNodeObject);
  InitializeListHead (&ObjectNode->VariableArgs);

  for (Index = AML_OP_PARSE_INDEX_GET_TERM1;
    Index < AML_OP_PARSE_INDEX_MAX;
    Index++
    ) {
    ObjectNode->FixedArgs[Index] = NULL;
  }

  ObjectNode->AmlByteEncoding = AmlByteEncoding;
  ObjectNode->PkgLen = PkgLength;

  *NewObjectNodePtr = ObjectNode;

  return EFI_SUCCESS;
}

/** Delete a data node and its buffer.

  It is the caller's responsability to check the DataNode has been removed
  from the tree and is not referencing any other node in the tree.

  @param  [in]  DataNode  Pointer to a data node.
**/
STATIC
VOID
EFIAPI
AmlDeleteDataNode (
  IN  AML_DATA_NODE   * DataNode
  )
{
  if (IS_AML_DATA_NODE (DataNode)) {
    if (DataNode->Buffer != NULL) {
      FreePool (DataNode->Buffer);
    } else {
      ASSERT (0);
    }
    FreePool (DataNode);
  } else {
    ASSERT (0);
  }
}

/** Create an AML_DATA_NODE.

  @param  [in]  DataType        DataType of the node.
  @param  [in]  Data            Pointer to the AML bytecode corresponding to
                                this node. Data is copied from there.
  @param  [in]  DataSize        Number of byte to consider at the address
                                pointed by Data.
  @param  [out] NewDataNodePtr  The created AML_DATA_NODE.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
**/
EFI_STATUS
EFIAPI
AmlCreateDataNode (
  IN        UINT8               DataType,
  IN  CONST UINT8             * Data,
  IN        UINT32              DataSize,
  OUT       AML_DATA_NODE    ** NewDataNodePtr
  )
{
  AML_DATA_NODE   * DataNode;
  UINT8           * Buffer;

  // A data node must not be created for certain data types
  if ((DataType == EFI_ACPI_NODE_TYPE_NONE)       ||
      (DataType == EFI_ACPI_NODE_TYPE_RESERVED1)  ||
      (DataType == EFI_ACPI_NODE_TYPE_RESERVED2)  ||
      (DataType == EFI_ACPI_NODE_TYPE_RESERVED3)  ||
      (DataType == EFI_ACPI_NODE_TYPE_RESERVED4)  ||
      (DataType == EFI_ACPI_NODE_TYPE_CHILD)      ||
      (Data == NULL)                              ||
      (DataSize == 0)                             ||
      (NewDataNodePtr == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  DataNode = AllocateZeroPool (sizeof (AML_DATA_NODE));
  if (DataNode == NULL) {
    ASSERT (DataNode != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  AmlInitializeAmlNodeHeader (&DataNode->NodeHeader, EAmlNodeData);

  Buffer = AllocateZeroPool (DataSize);
  if (Buffer == NULL) {
    ASSERT (Buffer != NULL);
    AmlDeleteDataNode (DataNode);
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (Buffer, Data, DataSize);

  DataNode->Buffer = Buffer;
  DataNode->DataType = DataType;
  DataNode->Size = DataSize;

  *NewDataNodePtr = DataNode;

  return EFI_SUCCESS;
}

/** Delete a Node.

  @param  [in]  Node  Pointer to a Node.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlDeleteNode (
  IN  AML_NODE_HEADER   * Node
  )
{
  EFI_STATUS  Status;

  // Check that the node being deleted is unlinked.
  // When removing the node, its parent and list are reset
  // with InitializeListHead. Thus it must be empty.
  if (!IS_AML_HEADER (Node) ||
      !AML_NODE_IS_DETACHED (Node)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;
  switch (Node->NodeType) {
    case EAmlNodeRoot:
    {
      // Check the variable list of arguments has been cleaned.
      ASSERT (IsListEmpty (AmlNodeGetVariableArgList (Node)));

      AmlDeleteRootNode ((AML_ROOT_NODE*)Node);
      break;
    }
    case EAmlNodeObject:
    {
      DEBUG_CODE (
        AML_OP_PARSE_INDEX  Index;
        // Check the fixed argument list has been cleaned.
        for (Index = AML_OP_PARSE_INDEX_GET_TERM1;
            Index < AML_OP_PARSE_INDEX_MAX;
            Index++) {
          ASSERT (((AML_OBJECT_NODE*)Node)->FixedArgs[Index] == NULL);
        }

        // Check the variable list of arguments has been cleaned.
        ASSERT (IsListEmpty (AmlNodeGetVariableArgList (Node)));
      );  // DEBUG_CODE

      AmlDeleteObjectNode ((AML_OBJECT_NODE*)Node);
      break;
    }
    case EAmlNodeData:
    {
      AmlDeleteDataNode ((AML_DATA_NODE*)Node);
      break;
    }
    default:
    {
      Status = EFI_INVALID_PARAMETER;
      break;
    }
  } // switch

  return Status;
}

/** Get the Name of the ObjectNode.

  If a node is in the AML namespace (i.e., has this attribute),
  then its name is its first fixed argument/
  Exception for the alias object.

  @param [in] ObjectNode  Pointer to an object node,
                          which is part of the namespace.

  @return A pointer to the name.
          NULL otherwise.
**/
CHAR8 *
EFIAPI
AmlNodeGetName (
  IN  CONST AML_OBJECT_NODE   * ObjectNode
  )
{
  AML_DATA_NODE * Node;
  // The Node must be an object node and have the Namespace Flag set.
  if (!AmlObjectNodeHasAttribute (ObjectNode, AML_IN_NAMESPACE)) {
    ASSERT (0);
    return NULL;
  }

  // The First Fixed argument will be the Data Node for the Namestring.
  // Return the Data Node buffer that points to the Namestring.
  Node = (AML_DATA_NODE*)ObjectNode->FixedArgs[AML_OP_PARSE_INDEX_GET_TERM1];
  if (Node == NULL) {
    ASSERT (0);
    return NULL;
  }

  return (CHAR8*)Node->Buffer;
}

/** Check whether the input node represents an AML object node and that
    the OpCode and SubOpcode matches.

  @param  [in]  ObjectNode  Pointer to the Node to check.
  @param  [in]  OpCode      OpCode to check
  @param  [in]  SubOpCode   SubOpCode to check

  @retval TRUE    The node is an an AML object and
                  the Opcode and the SubOpCode matches.
  @retval FALSE   Otherwise.
**/
BOOLEAN
EFIAPI
AmlNodeCompareOpCode (
  IN  CONST  AML_OBJECT_NODE  * ObjectNode,
  IN         UINT8              OpCode,
  IN         UINT8              SubOpCode
  )
{
  if (!IS_AML_OBJECT_NODE (ObjectNode) ||
      (ObjectNode->AmlByteEncoding == NULL)) {
    ASSERT (0);
    return FALSE;
  }

  ASSERT (AmlIsOpCodeValid (OpCode, SubOpCode));

  return ((ObjectNode->AmlByteEncoding->OpCode == OpCode) &&
    (ObjectNode->AmlByteEncoding->SubOpCode == SubOpCode)) ?
    TRUE : FALSE;
}

/** Check whether a node represents an AML object (the node
    has an opcode), and whether it has the input attribute.

  @param  [in]  ObjectNode  Pointer to an object node.
  @param  [in]  Attribute   Attribute to check for.

  @retval TRUE    The node is an an AML object and the attribute is present.
  @retval FALSE   Otherwise.
**/
BOOLEAN
EFIAPI
AmlObjectNodeHasAttribute (
  IN  CONST AML_OBJECT_NODE   * ObjectNode,
  IN        AML_OP_ATTRIBUTE    Attribute
  )
{
  if (!IS_AML_OBJECT_NODE (ObjectNode) ||
    (ObjectNode->AmlByteEncoding == NULL)) {
    ASSERT (0);
    return FALSE;
  }

  return ((ObjectNode->AmlByteEncoding->Attribute &
    Attribute) == 0 ? FALSE : TRUE);
}

