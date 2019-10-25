/** @file
  Aml Resource Data Parser.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
  - Rd or RD   - Resource Data
  - Rds or RDS - Resource Data Small
  - Rdl or RDL - Resource Data Large
**/

#ifndef AML_RESOURCE_DATA_PARSER_H_
#define AML_RESOURCE_DATA_PARSER_H_

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
  );

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
  );

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
  );

#endif // AML_RESOURCE_DATA_PARSER_H_

