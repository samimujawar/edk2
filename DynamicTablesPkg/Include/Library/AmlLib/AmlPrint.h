/** @file
  Aml Print Function.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef AML_PRINT_H_
#define AML_PRINT_H_

/** Print Size chars at Buffer address.

  @param  [in]  ErrorLevel    Error level for the DEBUG macro.
  @param  [in]  Buffer        Buffer containing the chars.
  @param  [in]  Size          Number of chars to print.
**/
VOID
EFIAPI
AmlPrintChars (
  IN        UINT32      ErrorLevel,
  IN  CONST CHAR8     * Buffer,
  IN        UINT32      Size
  );

/** Print fields of a data node.

  @param  [in]  DataNode  Pointer to a data node.
  @param  [in]  Level     Level of the indentation.
**/
VOID
EFIAPI
AmlPrintDataNode (
  IN  AML_DATA_NODE_HANDLE  DataNode,
  IN  UINT8                 Level
  );

/** Print fields of an object node.

  @param  [in]  ObjectNode  Pointer to an object node.
  @param  [in]  Level       Level of the indentation.
**/
VOID
EFIAPI
AmlPrintObjectNode (
  IN  AML_OBJECT_NODE_HANDLE  ObjectNode,
  IN  UINT8                   Level
  );

/** Recursively print the subtree under the Node.

  @param  [in]  Node    Pointer to the root of the subtree to print.
                        Can be a root/object/data node.
  @param  [in]  Level   Level of the indentation.
**/
VOID
EFIAPI
AmlPrintTree (
  IN  AML_NODE_HANDLE   Node,
  IN  UINT8             Level
  );

#if defined (MDEPKG_NDEBUG)
/** This function performs a raw data dump of the ACPI table.

  @param  [in]  Ptr     Pointer to the start of the table buffer.
  @param  [in]  Length  The length of the buffer.
**/
VOID
EFIAPI
DumpRaw (
  IN  CONST UINT8   * Ptr,
  IN        UINT32    Length
  );
#else
#define DumpRaw(Ptr, Length)
#endif

#endif // AML_PRINT_H_

