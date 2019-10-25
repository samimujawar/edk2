/** @file
  Aml grammar definitions.

  Copyright (c) 2010 - 2018, Intel Corporation. All rights reserved. <BR>
  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef AML_H_
#define AML_H_

#include <AmlInclude.h>

#include <IndustryStandard/AcpiAml.h>
#include <Library/AmlLib/AmlDefines.h>

/** AML types.

  In the AML bytestream, data is represented using one of the following types.
  These types are used in the parsing logic to know what kind of data is
  expected next in the bytestream.
  These are internal types.
*/
typedef UINT8 AML_OP_PARSE_FORMAT;
#define AML_NONE         0    // No data expected.
#define AML_UINT8        1    // One byte value evaluated as a UINT8.
#define AML_UINT16       2    // Two byte value evaluated as a UINT16.
#define AML_UINT32       3    // Four byte value evaluated as a UINT32.
#define AML_UINT64       4    // Eight byte value evaluated as a UINT64.
#define AML_NAME         5    // Name corresponding to the NameString keyword
                              // in the ACPI specification. E.g.: "\_SB_.DEV0"
#define AML_STRING       6    // NULL terminated string.
#define AML_OBJECT       7    // AML object, starting with an opcode
                              // potentially followed a pkglen.
                              // An AML_NAME is a subtype of AML_OBJECT.
                              // Indeed, an AML_NAME can be evaluated as
                              // an AML_OBJECT in the parsing.


/** Size of a NameSeg.

  cf. ACPI 6.3 specification, s20.2.
*/
 #define AML_NAME_SEG_SIZE  4


/** AML attributes

  To add some more information to the byte encoding, it is possible to
  add these attributes.
*/
typedef UINT32 AML_OP_ATTRIBUTE;
#define AML_HAS_PKG_LENGTH       0x1     // A PkgLength is expected between the
                                         // OpCode and the first fixed argument
                                         // of the object.
#define AML_IS_NAME_CHAR         0x2     // This OpCode is a character.
#define AML_HAS_CHILD_OBJ        0x4     // A variable list of arguments is
                                         // following the last fixed
                                         // argument. Each argument is
                                         // evaluated as an AML_OBJECT.
#define AML_HAS_BYTE_LIST        0x8     // A list of bytes is following the
                                         // last fixed argument.
#define AML_IN_NAMESPACE         0x10000 // The first fixed argument is
                                         // defining the name of an object
                                         // which is part of the AML namespace.
                                         // Exception of the alias opcode.


/** Encoding of an AML object.

  Every AML object has a specific encoding. This encoding information
  is used to parse AML objects.
  Cf. ACPI 6.3 specification, s20.2.
*/
typedef struct _AML_BYTE_ENCODING {
  /// OpCode of the AML object.
  UINT8                      OpCode;

  /// SubOpCode of the AML object.
  /// The SubOpcode field has a valid value when the OpCode is 0x5B,
  /// otherwise this field must be zero.
  UINT8                      SubOpCode;

  /// Number of fixed arguments for the AML statement represented
  /// by the OpCode & SubOpcode.
  /// Maximum is 6.
  AML_OP_PARSE_INDEX         MaxIndex;

  /// Type of each fixed argument.
  AML_OP_PARSE_FORMAT        Format[6];

  /// Additionnal information on the AML object.
  AML_OP_ATTRIBUTE           Attribute;
} AML_BYTE_ENCODING;


/** Convert an EFI_ACPI_DATA_TYPE to its corresponding AML_OP_PARSE_FORMAT.

  @param  [in]  AmlType   Input AML Type.

  @return The corresponding EFI_ACPI_DATA_TYPE.
          EFI_ACPI_DATA_TYPE_NONE if not found.
**/
EFI_ACPI_NODE_TYPE
EFIAPI
AmlTypeToAcpiType (
  IN  AML_OP_PARSE_FORMAT   AmlType
  );

/** Get the AML_BYTE_ENCODING corresponding to the input OpCode/ SubOpCode.

  @param  [in]  OpByteBuffer  Pointer to an OpCode.

  @return The corresponding AML_BYTE_ENCODING entry.
          NULL if not found.
**/
CONST
AML_BYTE_ENCODING *
EFIAPI
AmlGetByOpByte (
  IN  CONST UINT8   * OpByteBuffer
  );

/** Check if the OpCode/SubOpcode couple is valid and supported by the parser.

  @param  [in]  OpCode     OpCode to check.
  @param  [in]  SubOpCode  SubOpCode to check.

  @retval TRUE    The OpCode and SubOpCode is valid.
  @retval FALSE   Otherwise.
**/
BOOLEAN
EFIAPI
AmlIsOpCodeValid (
  IN  UINT8   OpCode,
  IN  UINT8   SubOpCode
  );

/** Get the package length from the buffer.

  @param  [in]  Buffer      AML buffer.
  @param  [out] PkgLength   The interpreted PkgLen value.

  @return The number of bytes to represent the package length.
          0 if an issue occured.
**/
UINT32
EFIAPI
AmlGetPkgLength (
  IN  CONST UINT8   * Buffer,
  OUT       UINT32  * PkgLength
  );

/** Convert the Length to the AML PkgLen encoding,
    then and write it in the Buffer.

  @param  [in]    Length  Length to convert. Length cannot exceed 2^28,
                          thus, it is handled as a UINT32.
  @param  [out]   Buffer  Write the result in the Buffer.

  @return The number of bytes used to write the Length.
**/
UINT8
EFIAPI
AmlSetPkgLength (
  IN  UINT32    Length,
  OUT UINT8   * Buffer
  );

/** Compute the number of bytes required to write a package length.

  @param  [in]  Length  The length to convert in the AML package length
                        encoding style.

  @return The number of bytes required to write the Length.
**/
UINT8
EFIAPI
AmlComputePkgLengthWidth (
  IN  UINT32  Length
  );

#endif // AML_H_

