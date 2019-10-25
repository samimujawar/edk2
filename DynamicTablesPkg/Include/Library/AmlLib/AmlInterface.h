/** @file
  Aml Interface.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef AML_INTERFACE_H_
#define AML_INTERFACE_H_

#include "AmlDefines.h"
#include <IndustryStandard/Acpi63.h>
#include <Library/AmlLib/AmlResourceData.h>

/** Parse the definition block.

  The function parse the whole AML blob, starting with the SSDT header,
  and the parsing the AML bytestream.
  A tree structure is returned via the RootPtr.

  @param  [in]  DefinitionBlock   Pointer the defintion block.
  @param  [out] RootPtr           Pointer holding the root node of the tree.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
  @retval EFI_NOT_FOUND           Error while parsing.
**/
EFI_STATUS
EFIAPI
AmlParseDefinitionBlock (
  IN  CONST EFI_ACPI_DESCRIPTION_HEADER   * DefinitionBlock,
  OUT       AML_ROOT_NODE_HANDLE          * RootPtr
  );

/** Delete a Node and its children.

  The input Node needs to be removed from the tree first, or to be
  the root node.

  @param  [in]  Node  Pointer to the node to delete.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlDeleteTree (
  IN  AML_NODE_HANDLE   Node
  );

/** Serialize a tree to create an SSDT table.

  If:
    * the content of BufferSize is >= to size needed to serialize the
      definition block;
    * Data is not NULL;
   first serialize the ACPI SSDT header from the root node,
   then serialize the AML blob from the rest of the tree.

  The content of BufferSize is always updated to the size needed to
  serialize the definition block.

  @param  [in]      RootNode    Pointer to a root node.
  @param  [in]      Buffer      Buffer to write the SSDT table to.
                                Can be NULL.
  @param  [in, out] BufferSize  Pointer holding the size of the Buffer.
                                Its content is always updated to the size
                                needed to serialize the SSDT table.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlSerializeTree (
  IN      AML_ROOT_NODE_HANDLE    RootNode,
  IN      UINT8                 * Buffer,
  IN  OUT UINT32                * BufferSize
  );

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
  IN  AML_NODE_HANDLE   Node
  );

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
  IN  AML_NODE_HANDLE   ParentNode,
  IN  AML_NODE_HANDLE   NewNode
  );

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
  IN  AML_NODE_HANDLE   ParentNode,
  IN  AML_NODE_HANDLE   NewNode
  );

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
  IN  AML_NODE_HANDLE   Node,
  IN  AML_NODE_HANDLE   NewNode
  );

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
  IN  AML_NODE_HANDLE   Node,
  IN  AML_NODE_HANDLE   NewNode
  );

/** Get the parent node of the input Node.

  @param [in] Node  Pointer to a node.

  @return The parent node of the input Node.
          NULL otherwise.
**/
AML_NODE_HANDLE
EFIAPI
AmlGetParent (
  IN  AML_NODE_HANDLE   Node
  );

/** Returns the tree node type (Root/Object/Data).

  @param [in] Node  Pointer to a Node.

  @return The node type.
           EAmlNodeUnknown if invalid parameter.
**/
EAML_NODE_TYPE
EFIAPI
AmlGetNodeType (
  IN  AML_NODE_HANDLE   Node
  );

/** Get the RootNode information.
    The Node must be a root node.

  @param  [in]  RootNode          Pointer to a root node.
  @param  [out] SdtHeaderBuffer   Buffer to copy the SSDT header to.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlGetRootNodeInfo (
  IN  AML_ROOT_NODE_HANDLE            RootNode,
  OUT EFI_ACPI_DESCRIPTION_HEADER   * SdtHeaderBuffer
  );

/** Get the ObjectNode information.
    The Node must be an object node.

  @param  [in]  ObjectNode  Pointer to an object node.
  @param  [out] OpCode      Pointer holding the OpCode.
                            Can be NULL.
  @param  [out] SubOpCode   Pointer holding the SubOpCode.
                            Can be NULL.
  @param  [out] PkgLen      Pointer holding the PkgLen.
                            The PkgLen is 0 for nodes
                            not having the Pkglen attribute.
                            Can be NULL.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlGetObjectNodeInfo (
  IN  AML_OBJECT_NODE_HANDLE    ObjectNode,
  OUT UINT8                   * OpCode,      OPTIONAL
  OUT UINT8                   * SubOpCode,   OPTIONAL
  OUT UINT32                  * PkgLen       OPTIONAL
  );

/** Get the data type of the DataNode.
    The Node must be a data node.

  @param  [in]  DataNode  Pointer to a data node.
  @param  [out] DataType  Pointer holding the data type of the data buffer.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlGetDataNodeType (
  IN  AML_DATA_NODE_HANDLE    DataNode,
  OUT EFI_ACPI_NODE_TYPE    * DataType
  );

/** Get the descriptor Id of the resource data element
    contained in the DataNode.

  The Node must be a data node.
  The Node must have the resource data type, i.e. have the
  EFI_ACPI_NODE_TYPE_RESOURCE_DATA data type.

  @param  [in]  DataNode          Pointer to a data node containing a
                                  resource data element.
  @param  [out] ResourceDataType  Pointer holding the descriptor Id of
                                  the resource data.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlGetResourceDataType (
  IN  AML_DATA_NODE_HANDLE    DataNode,
  OUT AML_RD_HEADER         * ResourceDataType
  );

/** Get the data buffer and size of the DataNode.
    The Node must be a data node.

  BufferSize is always updated to the size of buffer of the DataNode.

  If:
    * the content of BufferSize is >= to the DataNode's buffer size;
    * Buffer is not NULL;
  then copy the content of the DataNode's buffer in Buffer.

  @param  [in]      DataNode    Pointer to a data node.
  @param  [out]     Buffer      Pointer holding the data buffer.
                                If NULL, only update BufferSize.
  @param  [in, out] BufferSize  Pointer holding:
                                * At entry, the size of the Buffer.
                                * At exit, the size of the DataNode's
                                  buffer size.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlGetDataNodeBuffer (
  IN      AML_DATA_NODE_HANDLE    DataNode,
      OUT UINT8                 * Buffer,
  IN  OUT UINT32                * BufferSize
  );

/** Update the ACPI SSDT table header.
    The input SdtHeader information is copied to the tree RootNode.
    The table Length field is automatically updated.
    The checksum field is only updated when serializing the tree.

  @param  [in]  RootNode    Pointer to a root node.
  @param  [in]  SdtHeader   Pointer to a ACPI SSDT table header.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlUpdateRootNode (
  IN        AML_ROOT_NODE_HANDLE            RootNode,
  IN  CONST EFI_ACPI_DESCRIPTION_HEADER   * SdtHeader
  );

/** Update the buffer of a data node.

  @param  [in]  DataNode  Pointer to a data node.
  @param  [in]  Buffer    Buffer containing the new data. The content of
                          the Buffer is copied.
  @param  [in]  Size      Size of the Buffer.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlUpdateDataNode (
  IN  AML_DATA_NODE_HANDLE    DataNode,
  IN  UINT8                 * Buffer,
  IN  UINT32                  Size
  );

/** Returns the count of the fixed arguments for the input Node.

  @param  [in]  Node  Pointer to a node.

  @return Number of fixed arguments.
**/
AML_OP_PARSE_INDEX
AmlGetFixedArgumentCount (
  IN  AML_OBJECT_NODE_HANDLE  Node
  );

/** Get the node at the input Index in the fixed argument list of the input
    ObjectNode.

  @param  [in]  ObjectNode  Pointer to an object node.
  @param  [in]  Index       The Index of the fixed argument to get.

  @return The node at the input Index in the fixed argument list
          of the input ObjectNode.
          NULL otherwise, e.g. if the node is not an object node, or no
          node is available at this Index.
**/
AML_NODE_HANDLE
EFIAPI
AmlGetFixedArgument (
  IN  AML_OBJECT_NODE_HANDLE  ObjectNode,
  IN  AML_OP_PARSE_INDEX      Index
  );

/** Get the next variable argument.

  (Node)        /-i           # Child of fixed argument b
      \        /
       |- [a][b][c][d]        # Fixed Arguments
       |- {(e)->(f)->(g)}     # Variable Arguments
             \
              \-h             # Child of variable argument e

  Traversal Order: e, f, g, NULL

  @param  [in]  Node        Pointer to a Rootnode or Object Node.
  @param  [in]  CurrVarArg  Pointer to the Current Variable Argument.

  @return The node after the CurrVarArg in the variable list of arguments.
          If CurrVarArg is NULL, returns the first node of the
          variable argument list.
          Returns NULL if
          - CurrVarArg is the last node of the list, or
          - Node does not have a variable list of arguments.
**/
AML_NODE_HANDLE
EFIAPI
AmlGetNextVariableArgument (
  IN  AML_NODE_HANDLE   Node,
  IN  AML_NODE_HANDLE   ChildNode
  );

/** Get the previous variable argument.

  (Node)        /-i           # Child of fixed argument b
      \        /
       |- [a][b][c][d]        # Fixed Arguments
       |- {(e)->(f)->(g)}     # Variable Arguments
             \
              \-h             # Child of variable argument e

  Traversal Order: g, f, e, NULL


  @param  [in]  Node        Pointer to a root node or an object node.
  @param  [in]  CurrVarArg  Pointer to the Current Variable Argument.

  @return The node before the CurrVarArg in the variable list of
          arguments.
          If CurrVarArg is NULL, returns the last node of the
          variable list of arguments.
          Returns NULL id
          - CurrVarArg is the first node of the list, or
          - Node doesn't have a variable list of arguments.
**/
AML_NODE_HANDLE
EFIAPI
AmlGetPreviousVariableArgument (
  IN  AML_NODE_HANDLE   Node,
  IN  AML_NODE_HANDLE   ChildNode
  );

/**
  Callback function prototype used when iterating through the tree.

  @param  [in]        Node      The Node currently being processed.
  @param  [in, out ]  Context   A context for the callback function.
  @param  [in, out ]  Status    End the enumeration if pointing to a value
                                evaluated to TRUE.

  @return Return EFI_SUCCESS if the function completed successfully.
          Otherwise, the returned value depends on the Callback.
**/
typedef
BOOLEAN
(EFIAPI * EDKII_AML_TREE_ENUM_CALLBACK) (
  IN       AML_NODE_HANDLE     Node,
  IN  OUT  VOID              * Context,
  IN  OUT  EFI_STATUS        * Status      OPTIONAL
  );

/** Enumerate all nodes of the subtree under the input Node in the AML
    bytestream order, and call the CallBack function with the input Context.
    The prototype of the Callback function is EDKII_AML_TREE_ENUM_CALLBACK.

  @param  [in]      Node      Enumerate nodes of the subtree under this Node.
  @param  [in]      CallBack  Callback function to call on each node.
  @param  [in, out] Context   Void pointer used to pass some information
                              to the Callback function.
  @param  [out]     Status    Optional parameter that can be used to get
                              the status of the Callback function.

  @return EFI_SUCCESS if the function completed successfully.
          Otherwise, the returned value depends on the Callback.
**/
BOOLEAN
EFIAPI
AmlEnumTree (
  IN      AML_NODE_HANDLE                 Node,
  IN      EDKII_AML_TREE_ENUM_CALLBACK    CallBack,
  IN  OUT VOID                          * Context,
      OUT EFI_STATUS                    * Status  OPTIONAL
  );

#endif // AML_INTERFACE_H_
