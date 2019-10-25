/** @file
  Aml Resource Data Parser.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
  - Rd or RD   - Resource Data
  - Rds or RDS - Resource Data Small
  - Rdl or RDL - Resource Data Large
**/

#include "Aml.h"
#include "AmlNode.h"
#include "AmlTree.h"

#include <Library/AmlLib/AmlInterface.h>
#include <Library/AmlLib/AmlPrint.h>
#include <Library/AmlLib/AmlResourceData.h>

/** Get the next resource data element.
    Checks for buffer overflow must be done by the caller.

  The function does not handle the end tag resource data differently.

  @param  [in]  Header  Pointer to the first byte of a resource data.

  @return Pointer to the next resource data element.
**/
AML_RD_HEADER *
EFIAPI
AmlRdGetNext (
  IN  CONST AML_RD_HEADER   * Header
  )
{
  return (AML_RD_HEADER *)Header + AmlRdGetSize (Header);
}

/** Check the nesting of resource data elements that are dependent
    function descriptors.

  @param  [in]  Header          Pointer to the first byte of a
                                resource data element.
  @param  [out] InFunctionDesc  Pointer holding the nesting of the
                                resource data buffer.
                                InFunctionDesc holds TRUE if the resource data
                                at the address of Buffer is currently in a
                                dependent function descriptor list.

  @retval FALSE   The Header being parsed is ending a function descriptor
                  list when none started. This should not be possible for a
                  resource data buffer.
  @retval TRUE    Otherwise.
**/
STATIC
BOOLEAN
EFIAPI
AmlRdCheckFunctionDescNesting (
  IN      CONST AML_RD_HEADER   * Header,
  IN  OUT       BOOLEAN         * InFunctionDesc
  )
{
  // Starting a dependent function descriptor.
  // It is possible to start one when one has already started.
  if (AmlRdCompareDescId (
        *Header,
        AML_RD_BUILD_SMALL_DESC_ID (EAmlRdsIdStartDepFunc))) {
    *InFunctionDesc = TRUE;
    return TRUE;
  }

  // Ending a dependent function descriptor.
  // It should not be possible to end one when none started.
  if (AmlRdCompareDescId (
        *Header,
        AML_RD_BUILD_SMALL_DESC_ID (EAmlRdsIdEndDepFunc))) {
    if (*InFunctionDesc) {
      *InFunctionDesc = FALSE;
      return TRUE;
    } else {
      return FALSE;
    }
  }

  return TRUE;
}

/** Check whether the Buffer is holding a correct list
    of resource data elements.

  The check is based on the size of resource data elements and on the last
  element being an end tag. This means that a buffer can pass this check
  with non-existing descriptor Ids that have a correct size, and that
  terminates with an end tag element.

  @param  [in]  Buffer      Pointer to the first byte of a resource data buffer.
  @param  [in]  BufferSize  Size of the buffer.

  @retval TRUE    The buffer is holding a correct list of resource data
                  elements.
  @retval FALSE   Otherwise.
**/
BOOLEAN
EFIAPI
AmlRdIsResourceDataBuffer (
  IN  CONST UINT8   * Buffer,
  IN        UINT32    BufferSize
  )
{
  UINT32    BytesParsed;
  UINT32    DataElementSize;
  BOOLEAN   InFunctionDesc;

  if ((Buffer == NULL) ||
      (BufferSize == 0)) {
    ASSERT (0);
    return FALSE;
  }

  // The first element cannot be an end tag.
  if (AmlRdCompareDescId (
       (AML_RD_HEADER)*Buffer,
       AML_RD_BUILD_SMALL_DESC_ID (EAmlRdsIdEndTag))) {
    return FALSE;
  }

  InFunctionDesc = FALSE;
  BytesParsed = 0;
  while (1) {

    // Check the buffer is long enough to read the size of the large
    // resource data.
    if (AML_RD_IS_LARGE (Buffer) &
        (BufferSize < AML_RD_LARGE_HEADER_SIZE)) {
      return FALSE;
    }

    DataElementSize = AmlRdGetSize (Buffer);
    if ((BytesParsed + DataElementSize) > BufferSize) {
      return FALSE;
    } else {
      BytesParsed += DataElementSize;
    }

    if (!AmlRdCheckFunctionDescNesting (Buffer, &InFunctionDesc)) {
      return FALSE;
    }

    // TODO Might want to check the CRC when available.
    if (AmlRdCompareDescId (
          (AML_RD_HEADER)*Buffer,
          AML_RD_BUILD_SMALL_DESC_ID (EAmlRdsIdEndTag))) {
      return ((BufferSize - BytesParsed) == 0);
    }

    Buffer = AmlRdGetNext (Buffer);
  }

  return FALSE;
}

/** Parse the ResourceDataBuffer.

  For each resource data element, create a data node
  and add them to the variable list of arguments of the BufferNode.

  The ResourceDataBuffer is expected to be a valid list of resource data
  elements. A function is available to check it.

  @param  [in]  BufferNode          Buffer node.
  @param  [in]  ResourceDataBuffer  Pointer to the beginning of the buffer
                                    containing all the resource data elements.
  @param  [in]  ResourceDataSize    Size of the buffer (matches the BufferSize
                                    fixed argument).
                                    Must be non-null.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
  @retval EFI_NOT_FOUND           Error while parsing the ResourceDataBuffer.
**/
EFI_STATUS
EFIAPI
AmlParseResourceData (
  IN        AML_OBJECT_NODE   * BufferNode,
  IN  CONST UINT8             * ResourceDataBuffer,
  IN        UINT32              ResourceDataSize
  )
{
  EFI_STATUS        Status;
  AML_DATA_NODE   * NewNode;
  UINT32            ResourceDataElementSize;
  UINT32            BytesParsed;

  // Check that BufferNode is an ObjectNode and is a BufferOp.
  if (!AmlNodeCompareOpCode (BufferNode, AML_BUFFER_OP, 0)    ||
      (ResourceDataBuffer == NULL)                            ||
      (ResourceDataSize == 0)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  BytesParsed = 0;
  // Iterate through the resource data elements and create nodes.
  // We assume the Buffer has already been validated as a list of
  // resource data elements, so less checks are made.
  while (BytesParsed < ResourceDataSize) {
    ResourceDataElementSize = AmlRdGetSize (ResourceDataBuffer);
    Status = AmlCreateDataNode (
               EFI_ACPI_NODE_TYPE_RESOURCE_DATA,
               ResourceDataBuffer,
               ResourceDataElementSize,
               &NewNode
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return Status;
    }

    Status = AmlVarListAddTailInternal (
               (AML_NODE_HEADER*)BufferNode,
               (AML_NODE_HEADER*)NewNode
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      AmlDeleteTree ((AML_NODE_HEADER*)NewNode);
      return Status;
    }

    BytesParsed += ResourceDataElementSize;

    DumpRaw (ResourceDataBuffer, ResourceDataElementSize);

    // Exit the loop when finding the resource data end tag.
    if (*ResourceDataBuffer ==
        AML_RD_BUILD_SMALL_DESC_ID (EAmlRdsIdEndTag)) {
      if (BytesParsed != ResourceDataSize) {
        ASSERT (0);
        return EFI_NOT_FOUND;
      } else {
        break;
      }
    }

    ResourceDataBuffer = AmlRdGetNext (ResourceDataBuffer);
  } // while

  return EFI_SUCCESS;
}
