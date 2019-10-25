/** @file
  Aml String.

  Copyright (c) 2010 - 2018, Intel Corporation. All rights reserved. <BR>
  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef AML_STRING_H_
#define AML_STRING_H_

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Get ASL NameString size.

  @param[in]    Buffer   ASL NameString.

  @return ASL NameString size.
**/
UINT32
EFIAPI
AmlGetAslNameSegLength (
  IN  CONST  UINT8  * Buffer
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Print AML NameSeg.

  @param[in] Buffer AML NameSeg.
**/
VOID
EFIAPI
AmlPrintNameSeg (
  IN  CONST  UINT8  * Buffer
  );

/**
  Print AML NameString.

  @param[in] Buffer AML NameString.
**/
VOID
EFIAPI
AmlPrintNameString (
  IN  CONST  UINT8  * Buffer
  );

#endif // AML_STRING_H_

