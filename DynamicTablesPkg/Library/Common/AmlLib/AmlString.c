/** @file
  Aml String.

  Copyright (c) 2010 - 2018, Intel Corporation. All rights reserved. <BR>
  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Aml.h"

/**
  Check if it is AML Root name

  @param [in]   Buffer AML path.

  @retval TRUE    AML path is root.
  @retval FALSE   AML path is not root.
**/
BOOLEAN
EFIAPI
AmlIsRootPath (
  IN  CONST  UINT8  * Buffer
  )
{
  if ((Buffer[0] == AML_ROOT_CHAR) && (Buffer[1] == 0)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Check if it is AML LeadName.

  @param[in]    Ch   Char.

  @retval TRUE    Char is AML LeadName.
  @retval FALSE   Char is not AML LeadName.
**/
BOOLEAN
EFIAPI
AmlIsLeadName (
  IN  CHAR8  Ch
  )
{
  if ((Ch == '_') || (Ch >= 'A' && Ch <= 'Z')) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Check if it is AML Name.

  @param[in]    Ch   Char.

  @retval TRUE    Char is AML Name.
  @retval FALSE   Char is not AML Name.
**/
BOOLEAN
EFIAPI
AmlIsName (
  IN  CHAR8  Ch
  )
{
  if (AmlIsLeadName (Ch) || (Ch >= '0' && Ch <= '9')) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Return is buffer is AML NameSeg.

  @param[in]    Buffer     AML NameSement.

  @retval TRUE    It is AML NameSegment.
  @retval FALSE   It is not AML NameSegment.
**/
BOOLEAN
EFIAPI
AmlIsNameSeg (
  IN  CONST  UINT8  * Buffer
  )
{
  UINT32  Index;
  if (!AmlIsLeadName (Buffer[0])) {
    return FALSE;
  }
  for (Index = 1; Index < AML_NAME_SEG_SIZE; Index++) {
    if (!AmlIsName (Buffer[Index])) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
  Get AML NameString size.

  @param[in]    Buffer     AML NameString.
  @param[out]   BufferSize AML NameString size

  @retval EFI_SUCCESS             Success.
  @retval EFI_INVALID_PARAMETER   Buffer does not refer to a
                                  valid AML NameString.
**/
EFI_STATUS
EFIAPI
AmlGetNameStringSize (
  IN   CONST  UINT8   * Buffer,
  OUT         UINT32  * BufferSize
  )
{
  UINT32                SegCount;
  UINT32                Length;
  UINT32                Index;

  Length = 0;

  //
  // Parse root or parent prefix
  //
  if (*Buffer == AML_ROOT_CHAR) {
    Buffer++;
    Length++;
  } else if (*Buffer == AML_PARENT_PREFIX_CHAR) {
    do {
      Buffer++;
      Length++;
    } while (*Buffer == AML_PARENT_PREFIX_CHAR);
  }

  //
  // Parse name segment
  //
  if (*Buffer == AML_DUAL_NAME_PREFIX) {
    Buffer++;
    Length++;
    SegCount = 2;
  } else if (*Buffer == AML_MULTI_NAME_PREFIX) {
    Buffer++;
    Length++;
    SegCount = *Buffer;
    Buffer++;
    Length++;
  } else if (*Buffer == 0) {
    //
    // NULL Name, only for Root and single caret
    //
    SegCount = 0;
    Buffer--;
    if (((Length == 1) &&
         (*Buffer == AML_ROOT_CHAR)) ||
        (*Buffer == AML_PARENT_PREFIX_CHAR)) {
      *BufferSize = 2;
      return EFI_SUCCESS;
    } else {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // NameSeg
    //
    SegCount = 1;
  }

  Index = 0;
  do {
    if (!AmlIsNameSeg (Buffer)) {
      return EFI_INVALID_PARAMETER;
    }
    Buffer += AML_NAME_SEG_SIZE;
    Length += AML_NAME_SEG_SIZE;
    Index++;
  } while (Index < SegCount);

  *BufferSize = Length;
  return EFI_SUCCESS;
}

/**
  Check if it is ASL LeadName.

  @param[in]    Ch   Char.

  @retval TRUE    Char is ASL LeadName.
  @retval FALSE   Char is not ASL LeadName.
**/
BOOLEAN
EFIAPI
AmlIsAslLeadName (
  IN  CHAR8  Ch
  )
{
  if (AmlIsLeadName (Ch) || (Ch >= 'a' && Ch <= 'z')) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Check if it is ASL Name.

  @param[in]    Ch   Char.

  @retval TRUE    Char is ASL Name.
  @retval FALSE   Char is not ASL Name.
**/
BOOLEAN
EFIAPI
AmlIsAslName (
  IN  CHAR8  Ch
  )
{
  if (AmlIsAslLeadName (Ch) || (Ch >= '0' && Ch <= '9')) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Get ASL NameString size.

  @param[in]    Buffer   ASL NameString.

  @return ASL NameString size.
**/
UINT32
EFIAPI
AmlGetAslNameSegLength (
  IN  CONST  UINT8  * Buffer
  )
{
  UINT32 Length;
  UINT32 Index;

  if (*Buffer == 0) {
    return 0;
  }

  Length = 0;
  //
  // 1st
  //
  if (AmlIsAslLeadName (*Buffer)) {
    Length++;
    Buffer++;
  }
  if ((*Buffer == 0) || (*Buffer == '.')) {
    return Length;
  }
  //
  // 2, 3, 4 name char
  //
  for (Index = 0; Index < 3; Index++) {
    if (AmlIsAslName (*Buffer)) {
      Length++;
      Buffer++;
    }
    if ((*Buffer == 0) || (*Buffer == '.')) {
      return Length;
    }
  }

  //
  // Invalid ASL name
  //
  return 0;
}


/*
  Given an AmlPath, return the address of the first NameSeg.

  @param  AmlPath          The AML pathname.
  @param  Root             Is there a '\' in the name.
  @param  Parent           Number of carets in the name.
*/
CONST
CHAR8 *
EFIAPI
AmlGetFirstNameSeg (
  IN  CONST  CHAR8  * AmlPath,
  IN         UINTN    Root,
  IN         UINTN    Parent
  )
{
  CONST CHAR8  * Buffer;
  Buffer = AmlPath + Root + Parent;
  Buffer += ((*Buffer == AML_MULTI_NAME_PREFIX) ? 2
    : (*Buffer == AML_DUAL_NAME_PREFIX) ? 1 : 0);
  return Buffer;
}

/**
  Get AML NameString size.

  @param[in]    Buffer   AML NameString.
  @param[out]   Root     On return, points to Root char number.
  @param[out]   Parent   On return, points to Parent char number.
  @param[out]   SegCount On return, points to Segment count.

  @return AML NameString size.
**/
UINT32
EFIAPI
GetAmlNameStringSize (
  IN   CONST  CHAR8   * Buffer,
  OUT         UINT32  * Root,
  OUT         UINT32  * Parent,
  OUT         UINT32  * SegCount
  )
{
  UINT32  TotalLength;

  *Root = 0;
  *Parent = 0;
  *SegCount = 0;
  TotalLength = 0;

  if (Buffer == NULL)
    return 0;

  if (*Buffer == AML_ROOT_CHAR) {
    *Root = 1;
    Buffer++;

  } else if (*Buffer == AML_PARENT_PREFIX_CHAR) {
    do {
      Buffer++;
      (*Parent)++;
    } while (*Buffer == AML_PARENT_PREFIX_CHAR);
  }

  if (*Buffer == AML_DUAL_NAME_PREFIX) {
    *SegCount = 2;
  } else if (*Buffer == AML_MULTI_NAME_PREFIX) {
    *SegCount = *(Buffer + 1);
  } else if (*Buffer != AML_ZERO_OP) {
    *SegCount = 1;
  } else {
    *SegCount = 0;
  }

  TotalLength = *Root + *Parent + (*SegCount) * AML_NAME_SEG_SIZE;
  TotalLength += (*SegCount > 2) ? 2 : ((*SegCount == 2) ? 1 : 0);
  return TotalLength;
}

/**
  Get ASL NameString size.

  @param[in]    Buffer   ASL NameString.
  @param[out]   Root     On return, points to Root char number.
  @param[out]   Parent   On return, points to Parent char number.
  @param[out]   SegCount On return, points to Segment count.

  @return ASL NameString size.
**/
UINT32
EFIAPI
AmlGetAslNameStringSize (
  IN   CONST  UINT8   * Buffer,
  OUT         UINT32  * Root,
  OUT         UINT32  * Parent,
  OUT         UINT32  * SegCount
  )
{
  UINT32  NameLength;
  UINT32  TotalLength;

  *Root = 0;
  *Parent = 0;
  *SegCount = 0;
  TotalLength = 0;
  NameLength = 0;
  if (*Buffer == AML_ROOT_CHAR) {
    *Root = 1;
    Buffer++;
  } else if (*Buffer == AML_PARENT_PREFIX_CHAR) {
    do {
      Buffer++;
      (*Parent)++;
    } while (*Buffer == AML_PARENT_PREFIX_CHAR);
  }

  //
  // Now parse name
  //
  while (*Buffer != 0) {
    NameLength = AmlGetAslNameSegLength (Buffer);
    if ((NameLength == 0) || (NameLength > AML_NAME_SEG_SIZE)) {
      return 0;
    }
    (*SegCount)++;
    Buffer += NameLength;
    if (*Buffer == 0) {
      break;
    }
    Buffer++;
  }

  //
  // Check SegCoount
  //
  if (*SegCount > 0xFF) {
    return 0;
  }

  //
  // Calculate total length
  //
  TotalLength = *Root + *Parent + (*SegCount) * AML_NAME_SEG_SIZE;
  if (*SegCount > 2) {
    TotalLength += 2;
  } else if (*SegCount == 2) {
    TotalLength += 1;
  }

  //
  // Add NULL char
  //
  TotalLength++;

  return TotalLength;
}

/**
  Copy mem, and cast all the char in dest to be upper case.

  @param[in]    DstBuffer   Destination buffer.
  @param[in]    SrcBuffer   Source buffer.
  @param[in]    Length      Buffer length.
**/
VOID
EFIAPI
AmlUpperCaseCopyMem (
  OUT        UINT8  * DstBuffer,
  IN  CONST  UINT8  * SrcBuffer,
  IN         UINTN    Length
  )
{
  UINT32 Index;

  for (Index = 0; Index < Length; Index++) {
    if (SrcBuffer[Index] >= 'a' && SrcBuffer[Index] <= 'z') {
      DstBuffer[Index] = (UINT8)(SrcBuffer[Index] - 'a' + 'A');
    } else {
      DstBuffer[Index] = SrcBuffer[Index];
    }
  }
}

/**
  Return AML name according to ASL name.
  The caller need free the AmlName returned.

  @param[in]    AslPath     ASL name.

  @return AmlName
**/
UINT8 *
EFIAPI
AmlNameFromAslName (
  IN  CONST  UINT8 * AslPath
  )
{
  UINT32    Root;
  UINT32    Parent;
  UINT32    SegCount;
  UINT32    TotalLength;
  UINT32    NameLength;
  UINT8   * Buffer;
  UINT8   * AmlPath;
  UINT8   * AmlBuffer;

  TotalLength = AmlGetAslNameStringSize (AslPath, &Root, &Parent, &SegCount);
  if (TotalLength == 0) {
    return NULL;
  }

  AmlPath = AllocatePool (TotalLength);
  ASSERT (AmlPath != NULL);

  AmlBuffer = AmlPath;
  Buffer = (UINT8*)AslPath;

  //
  // Handle Root and Parent
  //
  if (Root == 1) {
    *AmlBuffer = AML_ROOT_CHAR;
    AmlBuffer++;
    Buffer++;
  } else if (Parent > 0) {
    SetMem (AmlBuffer, Parent, AML_PARENT_PREFIX_CHAR);
    AmlBuffer += Parent;
    Buffer += Parent;
  }

  //
  // Handle SegCount
  //
  if (SegCount > 2) {
    *AmlBuffer = AML_MULTI_NAME_PREFIX;
    AmlBuffer++;
    *AmlBuffer = (UINT8)SegCount;
    AmlBuffer++;
  } else if (SegCount == 2) {
    *AmlBuffer = AML_DUAL_NAME_PREFIX;
    AmlBuffer++;
  }

  //
  // Now to name
  //
  while (*Buffer != 0) {
    NameLength = AmlGetAslNameSegLength (Buffer);
    ASSERT ((NameLength != 0) && (NameLength <= AML_NAME_SEG_SIZE));
    AmlUpperCaseCopyMem (AmlBuffer, Buffer, NameLength);
    SetMem (
      AmlBuffer + NameLength,
      AML_NAME_SEG_SIZE - NameLength,
      AML_NAME_CHAR__
      );
    Buffer += NameLength;
    AmlBuffer += AML_NAME_SEG_SIZE;
    if (*Buffer == 0) {
      break;
    }
    Buffer++;
  }

  //
  // Add NULL
  //
  AmlPath[TotalLength - 1] = 0;

  return AmlPath;
}

/**
  Print AML NameSeg.

  @param[in] Buffer AML NameSeg.
**/
VOID
EFIAPI
AmlPrintNameSeg (
  IN  CONST  UINT8  * Buffer
  )
{
  DEBUG ((DEBUG_ERROR, "%c", Buffer[0]));
  if ((Buffer[1] == '_') && (Buffer[2] == '_') && (Buffer[3] == '_')) {
    return;
  }
  DEBUG ((DEBUG_ERROR, "%c", Buffer[1]));
  if ((Buffer[2] == '_') && (Buffer[3] == '_')) {
    return;
  }
  DEBUG ((DEBUG_ERROR, "%c", Buffer[2]));
  if (Buffer[3] == '_') {
    return;
  }
  DEBUG ((DEBUG_ERROR, "%c", Buffer[3]));
  return;
}

/**
  Print AML NameString.

  @param[in] Buffer AML NameString.
**/
VOID
EFIAPI
AmlPrintNameString (
  IN  CONST  UINT8  * Buffer
  )
{
  UINT8                 SegCount;
  UINT8                 Index;

  if (*Buffer == AML_ROOT_CHAR) {
    //
    // RootChar
    //
    Buffer++;
    DEBUG ((DEBUG_ERROR, "\\"));
  } else if (*Buffer == AML_PARENT_PREFIX_CHAR) {
    //
    // ParentPrefixChar
    //
    do {
      Buffer++;
      DEBUG ((DEBUG_ERROR, "^"));
    } while (*Buffer == AML_PARENT_PREFIX_CHAR);
  }

  if (*Buffer == AML_DUAL_NAME_PREFIX) {
    //
    // DualName
    //
    Buffer++;
    SegCount = 2;
  } else if (*Buffer == AML_MULTI_NAME_PREFIX) {
    //
    // MultiName
    //
    Buffer++;
    SegCount = *Buffer;
    Buffer++;
  } else if (*Buffer == 0) {
    //
    // NULL Name
    //
    return;
  } else {
    //
    // NameSeg
    //
    SegCount = 1;
  }

  AmlPrintNameSeg (Buffer);
  Buffer += AML_NAME_SEG_SIZE;
  for (Index = 0; Index < SegCount - 1; Index++) {
    DEBUG ((DEBUG_ERROR, "."));
    AmlPrintNameSeg (Buffer);
    Buffer += AML_NAME_SEG_SIZE;
  }

  return;
}

