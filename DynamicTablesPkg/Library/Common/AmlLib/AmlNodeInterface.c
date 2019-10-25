/** @file
  Aml Node Interface.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Aml.h"
#include "AmlNode.h"
#include "AmlString.h"
#include "AmlUtility.h"

#include <Library/AmlLib/AmlResourceData.h>

/** Get the parent node of the input Node.

  @param [in] Node  Pointer to a node.

  @return The parent node of the input Node.
          NULL otherwise.
**/
AML_NODE_HEADER *
EFIAPI
AmlGetParent (
  IN  AML_NODE_HEADER   * Node
  )
{
  if (IS_AML_DATA_NODE (Node) ||
      IS_AML_OBJECT_NODE (Node)) {
    return Node->Parent;
  }

  return NULL;
}

/** Returns the tree node type (Root/Object/Data).

  @param [in] Node  Pointer to a Node.

  @return The node type.
           EAmlNodeUnknown if invalid parameter.
**/
EAML_NODE_TYPE
EFIAPI
AmlGetNodeType (
  IN  AML_NODE_HEADER   * Node
  )
{
  if (!IS_AML_HEADER(Node)) {
    ASSERT (0);
    return EAmlNodeUnknown;
  }

  return Node->NodeType;
}

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
  IN  AML_ROOT_NODE                 * RootNode,
  OUT EFI_ACPI_DESCRIPTION_HEADER   * SdtHeaderBuffer
  )
{
  if (!IS_AML_ROOT_NODE (RootNode)  ||
      (SdtHeaderBuffer == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (
    SdtHeaderBuffer,
    RootNode->SdtHeader,
    sizeof (EFI_ACPI_DESCRIPTION_HEADER)
    );

  return EFI_SUCCESS;
}

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
  IN  AML_OBJECT_NODE   * ObjectNode,
  OUT UINT8             * OpCode,      OPTIONAL
  OUT UINT8             * SubOpCode,   OPTIONAL
  OUT UINT32            * PkgLen       OPTIONAL
  )
{
  if (!IS_AML_OBJECT_NODE (ObjectNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  if (OpCode != NULL) {
    *OpCode = ObjectNode->AmlByteEncoding->OpCode;
  }
  if (SubOpCode != NULL) {
    *SubOpCode = ObjectNode->AmlByteEncoding->SubOpCode;
  }
  if (PkgLen != NULL) {
    *PkgLen = ObjectNode->PkgLen;
  }

  return EFI_SUCCESS;
}

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
  IN  AML_DATA_NODE       * DataNode,
  OUT EFI_ACPI_NODE_TYPE  * DataType
  )
{
  if (!IS_AML_DATA_NODE (DataNode)  ||
      (DataType == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  *DataType = DataNode->DataType;

  return EFI_SUCCESS;
}

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
  IN  AML_DATA_NODE   * DataNode,
  OUT AML_RD_HEADER   * ResourceDataType
  )
{
  if (!IS_AML_DATA_NODE (DataNode) ||
    (ResourceDataType == NULL) ||
    (DataNode->DataType != EFI_ACPI_NODE_TYPE_RESOURCE_DATA)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  *ResourceDataType = AmlRdGetDescId (DataNode->Buffer);

  return EFI_SUCCESS;
}

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
  IN      AML_DATA_NODE   * DataNode,
      OUT UINT8           * Buffer,
  IN  OUT UINT32          * BufferSize
  )
{
  if (!IS_AML_DATA_NODE (DataNode) ||
      (BufferSize == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  if ((*BufferSize >= DataNode->Size)  &&
      (Buffer != NULL)) {
    CopyMem (Buffer, DataNode->Buffer, DataNode->Size);
  }

  *BufferSize = DataNode->Size;

  return EFI_SUCCESS;
}

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
  IN        AML_ROOT_NODE                 * RootNode,
  IN  CONST EFI_ACPI_DESCRIPTION_HEADER   * SdtHeader
  )
{
  EFI_STATUS  Status;
  UINT32      Length;

  if (!IS_AML_ROOT_NODE (RootNode) ||
    (SdtHeader == NULL) ||
    (SdtHeader->Signature !=
      EFI_ACPI_6_3_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (
    RootNode->SdtHeader,
    SdtHeader,
    sizeof (EFI_ACPI_DESCRIPTION_HEADER)
    );

  // Update the Length field.
  Length = 0;
  Status = AmlComputeSize ((AML_NODE_HEADER*)RootNode, &Length);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  RootNode->SdtHeader->Length = Length;

  return EFI_SUCCESS;
}

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
  IN  AML_DATA_NODE   * DataNode,
  IN  UINT8           * Buffer,
  IN  UINT32            Size
  )
{
  EFI_STATUS              Status;
  UINT32                  ExpectedSize;
  AML_OBJECT_NODE       * ParentNode;
  AML_OP_PARSE_FORMAT     ExpectedArgType;

  if (!IS_AML_DATA_NODE (DataNode)      ||
      (Buffer == NULL)                  ||
      (Size == 0)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  ParentNode = (AML_OBJECT_NODE*)AmlGetParent ((AML_NODE_HEADER*)DataNode);
  if (!IS_AML_OBJECT_NODE (ParentNode)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // The NewNode and OldNode must have the same type.
  // We do not allow to change the argument type of a data node.
  // There necessary, the initial asl template should be modified
  // accordingly.
  ExpectedArgType = DataNode->DataType;

  // Perform some compatibility checks.
  switch (ExpectedArgType) {
    case EFI_ACPI_NODE_TYPE_NAME_STRING:
    {
      // Check the name contained in the Buffer is an AML name
      // with the right size.
      Status = AmlGetNameStringSize (Buffer, &ExpectedSize);
      if (EFI_ERROR (Status)  ||
          (Size != ExpectedSize)) {
        ASSERT (0);
        return EFI_INVALID_PARAMETER;
      }
      break;
    }
    case EFI_ACPI_NODE_TYPE_STRING:
    {
      ExpectedSize = 0;
      while (ExpectedSize < Size) {
        // Cf ACPI 6.3 specifation 20.2.2, AsciiChar definition.
        if ((Buffer[ExpectedSize] < 0x01)  &&
            (Buffer[ExpectedSize] > 0x7F)) {
          ASSERT (0);
          return EFI_INVALID_PARAMETER;
        }
        ExpectedSize++;
      }
      break;
    }
    case EFI_ACPI_NODE_TYPE_UINT:
    {
      // Check the size.
      if (DataNode->Size != Size) {
        ASSERT (0);
        return EFI_INVALID_PARAMETER;
      }
      break;
    }
    case EFI_ACPI_NODE_TYPE_RAW:
    case EFI_ACPI_NODE_TYPE_FIELD_ELEMENT:
    {
      // If it's raw data, nothing to check.
      // TODO Temporary solution:
      // TODO Field elements are stored as raw data currently.
      break;
    }
    case EFI_ACPI_NODE_TYPE_RESOURCE_DATA:
    {
      // Large resource datas must be at least this long.
      if (AML_RD_IS_LARGE (Buffer) && (Size < AML_RD_LARGE_HEADER_SIZE)) {
        ASSERT (0);
        return EFI_INVALID_PARAMETER;
      }

      // Check the size of the buffer is equal
      // to the resource data encoded size.
      ExpectedSize = AmlRdGetSize (Buffer);
      if (ExpectedSize != Size) {
        ASSERT (0);
        return EFI_INVALID_PARAMETER;
      }
      break;
    }
    case EFI_ACPI_NODE_TYPE_NONE:
    case EFI_ACPI_NODE_TYPE_CHILD:
    default:
    {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
      break;
    }
  } // switch

  // If the new size is different from the old size, propagate the new size.
  if (DataNode->Size != Size) {
    FreePool (DataNode->Buffer);
    DataNode->Buffer = AllocateZeroPool (Size);
    if (DataNode->Buffer == NULL) {
      ASSERT (0);
      return EFI_OUT_OF_RESOURCES;
    }

    // Propagate the information.
    Status = AmlPropagateInformation (
               DataNode->NodeHeader.Parent,
               (Size > DataNode->Size) ? TRUE : FALSE,
               (Size > DataNode->Size) ?
                (Size - DataNode->Size) :
                (DataNode->Size - Size),
               0
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return Status;
    }
    DataNode->Size = Size;
  }

  CopyMem (DataNode->Buffer, Buffer, Size);

  return EFI_SUCCESS;
}
