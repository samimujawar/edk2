/** @file
  Aml Option.

  Copyright (c) 2010 - 2018, Intel Corporation. All rights reserved. <BR>
  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef AML_OPTION_H_
#define AML_OPTION_H_

/** Given a Buffer, interpret it as fixed argument of the AmlByteEncoding
    at the input Index, and return its information.

  @param  [in]  AmlByteEncoding   AML Byte Encoding.
  @param  [in]  Buffer            AML buffer.
  @param  [in]  MaxBufferSize     AML buffer MAX size. The parser can
                                  not parse any data exceed this region.
  @param  [in]  TermIndex         Index of the data to retrieve from
                                  the object.
  @param  [out] DataType          Points to the returned data type or
                                  EFI_ACPI_DATA_TYPE_NONE if no data
                                  exists for the specified index.
  @param  [out] Data              Upon return, points to the pointer
                                  to the data.
  @param  [out] DataSize          Upon return, points to the size
                                  of Data.

  @retval EFI_SUCCESS             Success.
  @retval EFI_INVALID_PARAMETER   Buffer does not refer to a
                                  valid ACPI object.
  @retval EFI_NOT_FOUND           Did not find the argument.
**/
EFI_STATUS
EFIAPI
AmlParseOptionTerm (
  IN  CONST  AML_BYTE_ENCODING     * AmlByteEncoding,
  IN  CONST  UINT8                 * Buffer,
  IN         UINT32                  MaxBufferSize,
  IN         AML_OP_PARSE_INDEX      TermIndex,
  OUT        AML_OP_PARSE_FORMAT   * DataType,
  OUT        UINT8                ** Data,
  OUT        UINT32                * DataSize
  );

#endif // AML_OPTION_H_

