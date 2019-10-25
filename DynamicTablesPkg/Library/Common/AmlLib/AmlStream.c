/** @file
  Aml Stream.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AmlStream.h"

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
  )
{
  if ((Stream == NULL) ||
      (Buffer == NULL) ||
      (Size == 0)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  Stream->Buffer = Buffer;
  Stream->MaxSize = Size;
  Stream->DataSize = 0;

  return EFI_SUCCESS;
}

/** Get the buffer of a Stream.

  @param  [in]  Stream  Pointer to a Stream.

  @return The buffer of the Stream.
**/
UINT8 *
EFIAPI
AmlStreamGetBuffer (
  IN  CONST AML_STREAM  * Stream
  )
{
  if (Stream == NULL) {
    ASSERT (0);
    return NULL;
  }
  return Stream->Buffer;
}

/** Get the size of the data already written in the Stream.

  @param  [in]  Stream  Pointer to a Stream.

  @return The buffer size of the Stream.
**/
UINT32
EFIAPI
AmlStreamGetBufferSize (
  IN  CONST AML_STREAM  * Stream
  )
{
  if (Stream == NULL) {
    ASSERT (0);
    return 0;
  }
  return Stream->DataSize;
}

/** Get the remaining size in the Stream.

  @param  [in]  Stream  Pointer to a Stream.

  @return Remaining size of the Stream.
**/
UINT32
EFIAPI
AmlStreamGetFreeSpace (
  IN  CONST AML_STREAM  * Stream
  )
{
  if (Stream == NULL) {
    ASSERT (0);
    return 0;
  }

  if (Stream->DataSize > Stream->MaxSize) {
    return 0;
  }

  return Stream->MaxSize - Stream->DataSize;
}

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
  )
{
  UINT8   * CurrPos;

  if ((Stream == NULL) || (Buffer == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Stream is checked in the call.
  if (AmlStreamGetFreeSpace (Stream) < Size) {
    ASSERT (0);
    return EFI_OUT_OF_RESOURCES;
  }

  CurrPos = Stream->Buffer + Stream->DataSize;
  CopyMem (CurrPos, Buffer, Size);
  Stream->DataSize += Size;

  return EFI_SUCCESS;
}
