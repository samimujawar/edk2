/** @file
  Aml Resource Data.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
  - Rd or RD   - Resource Data
  - Rds or RDS - Resource Data Small
  - Rdl or RDL - Resource Data Large
**/

#include "Aml.h"
#include "AmlNode.h"

#include <Library/AmlLib/AmlResourceData.h>

/** Check whether the resource data has the input decriptor Id.

  The small/ large bit is included in the descriptor Id,
  but the size bits are not included for small resource data elements.

  @param  [in]  Header        First byte of a resource data.
  @param  [in]  DescriptorId  The descriptor to check against.

  @retval TRUE    The resource data has the descriptor Id.
  @retval FALSE   Otherwise.
**/
BOOLEAN
EFIAPI
AmlRdCompareDescId (
  IN  AML_RD_HEADER   Header,
  IN  AML_RD_HEADER   DescriptorId
  )
{
  return (Header & DescriptorId) == DescriptorId;
}

/** Get the descriptor Id of the resource data.

  The small/ large bit is included in the descriptor Id,
  but the size bits are not included for small resource data elements.

  @param  [in]  Header  Pointer to the first byte of a resource data.

  @return A descriptor Id.
**/
AML_RD_HEADER
EFIAPI
AmlRdGetDescId (
  IN  CONST AML_RD_HEADER   * Header
  )
{
  if (AML_RD_IS_LARGE (Header)) {
    return *Header;
  } else {
    return *Header & AML_RD_SMALL_ID_MASK;
  }
}

/** Get the size of a resource data element.

  @param  [in]  Header  Pointer to the first byte of a resource data.

  @return The size of the resource data element.
**/
UINT32
EFIAPI
AmlRdGetSize (
  IN  CONST AML_RD_HEADER   * Header
  )
{
  if (AML_RD_IS_LARGE (Header)) {
    return ((AML_RD_LARGE*)Header)->Length +
             AML_RD_LARGE_HEADER_SIZE;
  } else {
    return (((AML_RD_SMALL*)Header)->Id &
             AML_RD_SMALL_SIZE_MASK) + AML_RD_SMALL_HEADER_SIZE;
  }
}
