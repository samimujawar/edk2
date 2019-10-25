/** @file
  Aml Stream.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef AML_STREAM_H_
#define AML_STREAM_H_

#include <AmlInclude.h>

/** Stream.

  This structure allows to write to a buffer while preventing
  from buffer overflow.
*/
typedef struct AmlStream {
  /// Pointer to a Buffer.
  UINT8   * Buffer;

  /// Maximum buffer size.
  UINT32    MaxSize;

  /// Size of the data written to the Buffer.
  UINT32    DataSize;
} AML_STREAM;

/** Initialize a Stream.

  @param  [in]  Stream  Pointer to the Stream to initialize.
  @param  [in]  Buffer  Buffer to initialize the Stream with.
  @param  [in]  Size    Size of the Buffer.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlStreamInit (
  IN  AML_STREAM  * Stream,
  IN  UINT8       * Buffer,
  IN  UINT32        Size
  );

/** Get the buffer of a Stream.

  @param  [in]  Stream  Pointer to a Stream.

  @return The buffer of the Stream.
**/
UINT8*
EFIAPI
AmlStreamGetBuffer (
  IN  CONST AML_STREAM  * Stream
  );

/** Get the size of the data already written in the Stream.

  @param  [in]  Stream  Pointer to a Stream.

  @return The buffer size of the Stream.
**/
UINT32
EFIAPI
AmlStreamGetBufferSize (
  IN  CONST AML_STREAM  * Stream
  );

/** Get the remaining size in the Stream.

  @param  [in]  Stream  Pointer to a Stream.

  @return Remaining size of the Stream.
**/
UINT32
EFIAPI
AmlStreamGetFreeSpace (
  IN  CONST AML_STREAM  * Stream
  );

/** Write bytes in the Stream.

  @param  [in]  Stream  Pointer to a Stream.
  @param  [in]  Buffer  Pointer to the data.
  @param  [in]  Size    Number of bytes to write.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    No space left in the buffer.
**/
EFI_STATUS
EFIAPI
AmlStreamPutBytes (
  IN        AML_STREAM  * Stream,
  IN  CONST UINT8       * Buffer,
  IN        UINT32        Size
  );

#endif // AML_STREAM_H_
