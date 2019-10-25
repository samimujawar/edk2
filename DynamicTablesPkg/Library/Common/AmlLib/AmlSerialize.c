/** @file
  Aml Serialize.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Aml.h"
#include "AmlNode.h"
#include "AmlStream.h"
#include "AmlUtility.h"

#include <Library/AmlLib/AmlInterface.h>

/** A callback function to copy the AML bytecodes contained in a node
    (can be a node) to the Stream stored in the Context.
    The data contained in the root node is not serialized by this function.

  @param  [in]      Node      Pointer to the node to copy the AML bytecodes
                              from.
  @param  [in, out] Context   Contains a Stream to write to.
                              (AML_STREAM*)Context.
  @param  [in, out] Status    As input, contains the status returned by the
                              last call to this exact function during the
                              enumeration.
                              As ouput, contains the returned status of the
                              call to this function.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
BOOLEAN
EFIAPI
AmlSerializeNodeCallback (
  IN      CONST AML_NODE_HEADER   * Node,
  IN  OUT       VOID              * Context,
  IN  OUT       EFI_STATUS        * Status
  )
{
  EFI_STATUS          Status1;

  AML_DATA_NODE     * DataNode;
  AML_OBJECT_NODE   * ObjectNode;
  AML_STREAM        * Stream;

  // Bytes needed to store OpCode + SubOpcode + MaxPkgLen = 6 bytes
  UINT8               ObjectNodeInfoArray[6];
  UINT32              Index;
  BOOLEAN             ContinueEnum;

  if ((Node == NULL) || (Context == NULL)) {
    ASSERT (0);
    Status1 = EFI_INVALID_PARAMETER;
    ContinueEnum = FALSE;
    goto error_handler;
  }

  Status1 = EFI_SUCCESS;
  ContinueEnum = TRUE;
  Stream = (AML_STREAM*)Context;

  if (IS_AML_DATA_NODE(Node)) {
    // Copy the content of the Buffer for a DataNode.
    DataNode = (AML_DATA_NODE*)Node;
    Status1 = AmlStreamPutBytes (
                Stream,
                DataNode->Buffer,
                DataNode->Size
                );
    if (EFI_ERROR (Status1)) {
      ASSERT (0);
      ContinueEnum = FALSE;
      goto error_handler;
    }

  } else if (IS_AML_OBJECT_NODE(Node)) {
    ObjectNode = (AML_OBJECT_NODE*)Node;

    Index = 0;
    // Copy the opcode(s).
    ObjectNodeInfoArray[Index++] = ObjectNode->AmlByteEncoding->OpCode;
    if (ObjectNode->AmlByteEncoding->OpCode == AML_EXT_OP) {
      ObjectNodeInfoArray[Index++] = ObjectNode->AmlByteEncoding->SubOpCode;
    }

    // Copy the PkgLen.
    if (AmlObjectNodeHasAttribute (ObjectNode, AML_HAS_PKG_LENGTH)) {
      Index += AmlSetPkgLength (
                 ObjectNode->PkgLen,
                 &ObjectNodeInfoArray[Index]
                 );
    }

    Status1 = AmlStreamPutBytes (
                Stream,
                ObjectNodeInfoArray,
                Index
                );
    if (EFI_ERROR (Status1)) {
      ASSERT (0);
      ContinueEnum = FALSE;
      goto error_handler;
    }
  }

error_handler:

  if (Status != NULL) {
    *Status = Status1;
  }
  return ContinueEnum;
}

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
  IN      AML_ROOT_NODE   * RootNode,
  IN      UINT8           * Buffer,
  IN  OUT UINT32          * BufferSize
  )
{
  EFI_STATUS    Status;
  AML_STREAM    AmlBuffer;
  UINT32        TableSize;

  if (!IS_AML_ROOT_NODE (RootNode) ||
      (BufferSize == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Compute the total size of the AML blob.
  TableSize = 0;
  Status = AmlComputeSize (
              (CONST AML_NODE_HEADER*)RootNode,
              &TableSize
              );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Add the size of the ACPI header.
  TableSize += sizeof (EFI_ACPI_DESCRIPTION_HEADER);

  // Buffer is not big enough, or NULL.
  if ((TableSize < *BufferSize) || (Buffer == NULL)) {
    *BufferSize = TableSize;
    return EFI_SUCCESS;
  }

  // Only give the size that is needed.
  Status = AmlStreamInit (&AmlBuffer, Buffer, TableSize);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Serialize the header.
  Status = AmlStreamPutBytes (
             &AmlBuffer,
             (UINT8*)RootNode->SdtHeader,
             sizeof (EFI_ACPI_DESCRIPTION_HEADER)
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  Status = EFI_SUCCESS;
  AmlEnumTree (
    (AML_NODE_HEADER*)RootNode,
    (EDKII_AML_TREE_ENUM_CALLBACK)&AmlSerializeNodeCallback,
    (VOID*)&AmlBuffer,
    &Status
    );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Update the checksum.
  return AcpiPlatformChecksum ((EFI_ACPI_DESCRIPTION_HEADER*)Buffer);
}
