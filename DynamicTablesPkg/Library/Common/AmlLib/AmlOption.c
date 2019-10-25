/** @file
  Aml Option.

  Copyright (c) 2010 - 2018, Intel Corporation. All rights reserved. <BR>
  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Aml.h"
#include "AmlString.h"
#include "AmlOption.h"

/** Retrieve information according to an AML_BYTE_ENCODING entry and a Buffer.

  @param  [in]  AmlByteEncoding  AML byte encoding entry.
  @param  [in]  Buffer           AML buffer.
  @param  [in]  MaxBufferSize    AML buffer MAX size.
                                 The parser can not parse any data
                                 exceed this region.
  @param  [in]  Index            Index of the data to retrieve from the
                                 object. In general, indexes read from
                                 left-to-right in the ACPI encoding, with
                                 index 0 always being the ACPI opcode.
  @param  [out] DataType         Points to the returned data type or
                                 EFI_ACPI_DATA_TYPE_NONE if no data exists
                                 for the specified index.
  @param  [out] Data             Upon return, points to the pointer to
                                 the data.
  @param  [out] DataSize         Upon return, points to the size of Data.

  @retval EFI_SUCCESS             Success.
  @retval EFI_INVALID_PARAMETER   Buffer does not refer to a valid
                                  ACPI object.
**/
STATIC
EFI_STATUS
AmlParseOptionCommon (
  IN  CONST AML_BYTE_ENCODING     * AmlByteEncoding,
  IN  CONST UINT8                 * Buffer,
  IN        UINT32                  MaxBufferSize,
  IN        AML_OP_PARSE_INDEX      Index,
  OUT       AML_OP_PARSE_FORMAT   * DataType,
  OUT       UINT8                ** Data,
  OUT       UINT32                * DataSize
  )
{
  UINT8               * CurrentBuffer;
  UINT32                PkgLength;
  UINT32                OpLength;
  UINT32                PkgOffset;
  AML_OP_PARSE_INDEX    TermIndex;
  EFI_STATUS            Status;

  ASSERT ((Index < AmlByteEncoding->MaxIndex) ||
    (Index == AML_OP_PARSE_INDEX_GET_SIZE));

  // 0. Check if this is NAME string.
  if ((AmlByteEncoding->Attribute & AML_IS_NAME_CHAR) != 0) {
    // Only allow GET_SIZE
    if (Index != AML_OP_PARSE_INDEX_GET_SIZE) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
    // Return NameString size.
    Status = AmlGetNameStringSize (Buffer, DataSize);
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
    if (*DataSize > MaxBufferSize) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
    return EFI_SUCCESS;
  }

  //
  // Not NAME string, start parsing
  //
  CurrentBuffer = (UINT8*)Buffer;

  //
  // 1. Get OpCode
  //
  if (*CurrentBuffer == AML_EXT_OP) {
    OpLength = 2;
  } else {
    OpLength = 1;
  }
  if (OpLength > MaxBufferSize) {
    return EFI_INVALID_PARAMETER;
  }
  CurrentBuffer += OpLength;

  //
  // 2. Skip PkgLength field, if have
  //
  if ((AmlByteEncoding->Attribute & AML_HAS_PKG_LENGTH) != 0) {
    PkgOffset = AmlGetPkgLength (CurrentBuffer, &PkgLength);
    //
    // Override MaxBufferSize if it is valid PkgLength
    //
    if (OpLength + PkgLength > MaxBufferSize) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    } else {
      MaxBufferSize = OpLength + PkgLength;
    }
  } else {
    PkgOffset = 0;
    PkgLength = 0;
  }
  CurrentBuffer += PkgOffset;

  //
  // 3. Get Term one by one.
  //
  TermIndex = AML_OP_PARSE_INDEX_GET_TERM1;
  while ((Index >= TermIndex)               &&
    (TermIndex < AmlByteEncoding->MaxIndex) &&
    ((UINTN)CurrentBuffer < (UINTN)Buffer + MaxBufferSize)) {

    Status = AmlParseOptionTerm (
               AmlByteEncoding,
               CurrentBuffer,
               (UINT32)((UINTN)Buffer + MaxBufferSize - (UINTN)CurrentBuffer),
               TermIndex,
               DataType,
               Data,
               DataSize
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }

    if (Index == TermIndex) {
      //
      // Done
      //
      return EFI_SUCCESS;
    }

    //
    // Parse next one
    //
    CurrentBuffer += *DataSize;
    TermIndex++;
  }

  //
  // Finish all options, but no option found.
  //
  if ((UINTN)CurrentBuffer > (UINTN)Buffer + MaxBufferSize) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }
  if ((UINTN)CurrentBuffer == (UINTN)Buffer + MaxBufferSize) {
    if (Index != AML_OP_PARSE_INDEX_GET_SIZE) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // 4. Finish parsing all node, return size
  //
  ASSERT (Index == AML_OP_PARSE_INDEX_GET_SIZE);
  if ((AmlByteEncoding->Attribute & AML_HAS_PKG_LENGTH) != 0) {
    *DataSize = OpLength + PkgLength;
  } else {
    *DataSize = (UINT32)(CurrentBuffer - Buffer);
  }

  return EFI_SUCCESS;
}

/** Return the object size.

  @param  [in]  AmlByteEncoding   AML Byte Encoding.
  @param  [in]  Buffer            AML object buffer.
  @param  [in]  MaxBufferSize     AML object buffer MAX size.
                                  The parser can not parse any data exceed
                                  this region.

  @return Size of the object.
**/
STATIC
UINT32
AmlGetObjectSize (
  IN  CONST AML_BYTE_ENCODING   * AmlByteEncoding,
  IN  CONST UINT8               * Buffer,
  IN        UINT32                MaxBufferSize
  )
{
  EFI_STATUS              Status;

  AML_OP_PARSE_FORMAT     DataType;
  UINT8                 * Data;
  UINT32                  DataSize;

  Status = AmlParseOptionCommon (
             AmlByteEncoding,
             Buffer,
             MaxBufferSize,
             AML_OP_PARSE_INDEX_GET_SIZE,
             &DataType,
             &Data,
             &DataSize
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return 0;
  } else {
    return DataSize;
  }
}

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
  )
{
  CONST AML_BYTE_ENCODING   * ChildAmlByteEncoding;
  EFI_STATUS                  Status;

  if ((AmlByteEncoding == NULL) ||
    (Buffer == NULL) ||
    (DataType == NULL) ||
    (Data == NULL) ||
    (DataSize == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  *DataType = AML_NONE;
  *Data = NULL;
  *DataSize = 0;

  if ((TermIndex != AML_OP_PARSE_INDEX_GET_SIZE) &&
    ((TermIndex < AML_OP_PARSE_INDEX_GET_TERM1) ||
    (TermIndex > AML_OP_PARSE_INDEX_GET_TERM6))) {
    ASSERT (0);
    return EFI_NOT_FOUND;
  }

  if (DataType != NULL) {
    *DataType = AmlByteEncoding->Format[TermIndex];
  }

  if (Data != NULL) {
    *Data = (UINT8*)Buffer;
  }

  //
  // Parse term according to AML type
  //
  switch (AmlByteEncoding->Format[TermIndex]) {
  case AML_UINT8:
    *DataSize = sizeof (UINT8);
    break;
  case AML_UINT16:
    *DataSize = sizeof (UINT16);
    break;
  case AML_UINT32:
    *DataSize = sizeof (UINT32);
    break;
  case AML_UINT64:
    *DataSize = sizeof (UINT64);
    break;
  case AML_STRING:
    *DataSize = (UINT32)AsciiStrSize ((CONST CHAR8*)Buffer);
    break;
  case AML_NAME:
    Status = AmlGetNameStringSize (Buffer, DataSize);
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
    break;
  case AML_OBJECT:
    ChildAmlByteEncoding = AmlGetByOpByte (Buffer);
    if (ChildAmlByteEncoding == NULL) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }

    //
    // NOTE: We need to override DataType here, if the AML_OBJECT
    // is AML_NAME. We also need to convert type from
    // EFI_ACPI_DATA_TYPE_CHILD to EFI_ACPI_DATA_TYPE_NAME_STRING.
    // We should not return CHILD because there is NO OpCode for
    // NameString.
    //
    if ((ChildAmlByteEncoding->Attribute & AML_IS_NAME_CHAR) != 0) {
      if (DataType != NULL) {
        *DataType = AML_NAME;
      }
      Status = AmlGetNameStringSize (Buffer, DataSize);
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        return EFI_INVALID_PARAMETER;
      }
      break;
    }

    //
    // It is real AML_OBJECT
    //
    *DataSize = AmlGetObjectSize (
      ChildAmlByteEncoding,
      Buffer,
      MaxBufferSize
    );
    if (*DataSize == 0) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
    break;
  case AML_NONE:
    //
    // No term
    //
    break;
  default:
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }
  if (*DataSize > MaxBufferSize) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }
  return EFI_SUCCESS;
}
