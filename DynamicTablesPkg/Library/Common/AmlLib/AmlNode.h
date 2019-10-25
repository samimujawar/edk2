/** @file
  Aml Node.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef AML_NODE_H_
#define AML_NODE_H_

#include <IndustryStandard/Acpi63.h>

/** AML header node.

  This abstract class represents either a root/object/data node.
  All the enumerated nodes have this same common header.
*/
typedef struct AmlNodeHeader {
  /// Double linked list. Siblings of this node are in this list.
  /// This must be the first field in this structure.
  LIST_ENTRY              Link;

  /// Parent of this node. NULL for the root node.
  struct AmlNodeHeader  * Parent;

  /// Node type allowing to identify a root/object/data node.
  EAML_NODE_TYPE          NodeType;
} AML_NODE_HEADER;

typedef AML_NODE_HEADER* AML_NODE_HANDLE;

/** AML root node.

  The root node is unique and at the head of of tree. It is a fake node used
  to maintain the list of AML statements (stored as object nodes) which are
  at the first scope level.
*/
typedef struct AmlRootNode {
  /// Header information. Must be the first field of the struct.
  AML_NODE_HEADER                NodeHeader;

  /// List of object nodes being at the first scope level.
  /// These are children and can only be object nodes.
  LIST_ENTRY                     VariableArgs;

  /// ACPI SSDT header.
  EFI_ACPI_DESCRIPTION_HEADER  * SdtHeader;
} AML_ROOT_NODE;

typedef AML_ROOT_NODE* AML_ROOT_NODE_HANDLE;

/** AML object node.

  Object nodes correspond to AML statements. They are associated with an
  OpCode/SubOpCode, and can have children.
*/
typedef struct AmlObjectNode {
  /// Header information. Must be the first field of the struct.
  AML_NODE_HEADER            NodeHeader;

  /// Some object nodes have a variable list of arguments.
  /// These are children and can only be object/data nodes.
  /// Cf ACPI specification, s20.3.
  LIST_ENTRY                 VariableArgs;

  /// Fixed arguments of this object node.
  /// These are children and can be object/data nodes.
  /// Cf ACPI specification, s20.3.
  AML_NODE_HEADER          * FixedArgs[AML_OP_PARSE_INDEX_MAX];

  /// AML byte encoding. Stores the encoding information:
  /// (OpCode/SubOpCode/number of fixed arguments/ attributes).
  CONST AML_BYTE_ENCODING  * AmlByteEncoding;

  /// Some nodes have a PkgLen following their OpCode/SubOpCode in the
  /// AML bytestream. This field stores the decoded value of the PkgLen.
  UINT32                     PkgLen;
} AML_OBJECT_NODE;

typedef AML_OBJECT_NODE* AML_OBJECT_NODE_HANDLE;

/** AML data node.

  Data nodes store smallest pieces of information.
  E.g.: UINT8, UINT64, NULL terminated string ...
  Data node don't have children.
*/
typedef struct AmlDataNode {
  /// Header information. Must be the first field of the struct.
  AML_NODE_HEADER       NodeHeader;

  /// Buffer containing the data stored by this node.
  UINT8               * Buffer;

  /// Size of the Buffer.
  UINT32                Size;

  /// Tag identifying what data is stored in this node.
  /// E.g. UINT, NULL terminated string, resource data element ...
  EFI_ACPI_NODE_TYPE    DataType;
} AML_DATA_NODE;

typedef AML_DATA_NODE* AML_DATA_NODE_HANDLE;

/** Check whether a Node is a valid header.

  @param  [in]  Node  The node to check.

  @retval TRUE  The Node is a valid header.
  @retval FALSE Otherwise.
*/
#define IS_AML_HEADER(Node) (                                                 \
          (Node != NULL)                                                   && \
          ((((AML_NODE_HEADER*)Node)->NodeType > EAmlNodeUnknown)          || \
           (((AML_NODE_HEADER*)Node)->NodeType < EAmlNodeMax)))

/** Check whether a Node is a root node.

  @param  [in]  Node  The node to check.

  @retval TRUE  The Node is a root node.
  @retval FALSE Otherwise.
*/
#define IS_AML_ROOT_NODE(Node)  ((Node != NULL)                            && \
                                 (((AML_NODE_HEADER*)Node)->NodeType ==       \
                                  EAmlNodeRoot))

/** Check whether a Node is an object node.

  @param  [in]  Node  The node to check.

  @retval TRUE  The Node is an object node.
  @retval FALSE Otherwise.
*/
#define IS_AML_OBJECT_NODE(Node)  ((Node != NULL)                          && \
                                   (((AML_NODE_HEADER*)Node)->NodeType ==     \
                                    EAmlNodeObject))

/** Check whether a Node is a data node.

  @param  [in]  Node  The node to check.

  @retval TRUE  The Node is a data node.
  @retval FALSE Otherwise.
*/
#define IS_AML_DATA_NODE(Node)  ((Node != NULL)                            && \
                                 (((AML_NODE_HEADER*)Node)->NodeType ==       \
                                  EAmlNodeData))

/** Check whether a Node has a parent.

  @param  [in]  Node  The node to check.

  @retval TRUE  The Node is a data node.
  @retval FALSE Otherwise.
*/
#define AML_NODE_HAS_PARENT(Node)  ((Node != NULL)                         && \
                                    (((AML_NODE_HEADER*)Node)->Parent != NULL))

/** Check that the Node is not attached somewhere.
  This doesn't mean the node cannot have children.

  @param  [in]  Node  The node to check.

  @retval TRUE  The Node has been detached.
  @retval FALSE Otherwise.
*/
#define AML_NODE_IS_DETACHED(Node)  ((Node != NULL)                       && \
                              IsListEmpty ((CONST LIST_ENTRY*)Node)       && \
                              (((AML_NODE_HEADER*)Node)->Parent == NULL))


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
  );

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
  );

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
  );

/** Delete a Node.

  @param  [in]  Node  Pointer to a Node.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlDeleteNode (
  IN  AML_NODE_HEADER   * Node
  );

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
  );

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
  );

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
  );

#endif // AML_NODE_H_

