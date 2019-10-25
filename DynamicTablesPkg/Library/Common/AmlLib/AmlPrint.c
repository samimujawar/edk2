/** @file
  Aml Print Function.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Aml.h"
#include "AmlNode.h"
#include "AmlString.h"
#include "AmlTreeTraversal.h"

#include <Library/AmlLib/AmlInterface.h>
#include <Library/AmlLib/AmlPrint.h>

/** String table representing AML Data types as defined in macros.
*/
CONST CHAR8 * DataTypeStr[] = {
  "EFI_ACPI_NODE_TYPE_NONE",
  "EFI_ACPI_NODE_TYPE_RESERVED1",
  "EFI_ACPI_NODE_TYPE_RESERVED2",
  "EFI_ACPI_NODE_TYPE_RESERVED3",
  "EFI_ACPI_NODE_TYPE_RESERVED4",
  "EFI_ACPI_NODE_TYPE_NAME_STRING",
  "EFI_ACPI_NODE_TYPE_STRING",
  "EFI_ACPI_NODE_TYPE_CHILD",
  "EFI_ACPI_NODE_TYPE_UINT",
  "EFI_ACPI_NODE_TYPE_RAW",
  "EFI_ACPI_NODE_TYPE_RESOURCE_DATA",
  "EFI_ACPI_NODE_TYPE_FIELD_ELEMENT"
};

/** String table representing AML Node types as defined by EEAML_NODE_TYPE.
*/
CONST CHAR8 * NodeTypeStr[] = {
  "EAmlNodeUnknown",
  "EAmlNodeRoot",
  "EAmlNodeObject",
  "EAmlNodeData",
  "EAmlNodeMax"
};

/** Structure used to construct a string table for representing
    the OpCode/SubOpcode names.
*/
typedef struct AmlOpCodeStr {
  /// OpCode.
  UINT8   OpCode;

  /// SubOpCode.
  UINT8   SubOpCode;

  /// Associated name.
  CHAR8 * Str;
}AML_OP_CODE_STR;

/** Table matching an OpCode/SubOpCode matches to a name.
*/
GLOBAL_REMOVE_IF_UNREFERENCED
STATIC
CONST
AML_OP_CODE_STR mAmlOpCodeStr[] = {
  //                             OpCode                      SubOpCode
  /* ZeroOp - 0x00 */              {AML_ZERO_OP,               0,                       "ZeroOp"},
  /* OneOp  - 0x01 */              {AML_ONE_OP,                0,                       "OneOp"},
  /* AliasOp - 0x06 */             {AML_ALIAS_OP,              0,                       "AliasOp"},
  /* NameOp - 0x08 */              {AML_NAME_OP,               0,                       "NameOp"},
  /* BtyePrefix - 0x0A */          {AML_BYTE_PREFIX,           0,                       "BtyePrefix"},
  /* WordPrefix - 0x0B */          {AML_WORD_PREFIX,           0,                       "WordPrefix"},
  /* DWordPrefix - 0x0C */         {AML_DWORD_PREFIX,          0,                       "DWordPrefix"},
  /* StringPrefix - 0x0D */        {AML_STRING_PREFIX,         0,                       "StringPrefix"},
  /* QWordPrefix - 0x0E */         {AML_QWORD_PREFIX,          0,                       "QWordPrefix"},
  /* ScopeOp - 0x10 */             {AML_SCOPE_OP,              0,                       "ScopeOp"},
  /* BufferOp - 0x11 */            {AML_BUFFER_OP,             0,                       "BufferOp"},
  /* PackageOp - 0x12 */           {AML_PACKAGE_OP,            0,                       "PackageOp"},
  /* VarPackageOp - 0x13 */        {AML_VAR_PACKAGE_OP,        0,                       "VarPackageOp"},
  /* MethodOp - 0x14 */            {AML_METHOD_OP,             0,                       "MethodOp"},
  /* ExternalOp - 0x15 */          {AML_EXTERNAL_OP,           0,                       "ExternalOp"},
  /* DualNamePrefix - 0x2E */      {AML_DUAL_NAME_PREFIX,      0,                       "DualNamePrefix"},
  /* MultiNamePrefix - 0x2F */     {AML_MULTI_NAME_PREFIX,     0,                       "MultiNamePrefix"},
  /* NameChar - 0x41 */            {'A',                       0,                       "NameChar - A"},
  /* NameChar - 0x42 */            {'B',                       0,                       "NameChar - B"},
  /* NameChar - 0x43 */            {'C',                       0,                       "NameChar - C"},
  /* NameChar - 0x44 */            {'D',                       0,                       "NameChar - D"},
  /* NameChar - 0x45 */            {'E',                       0,                       "NameChar - E"},
  /* NameChar - 0x46 */            {'F',                       0,                       "NameChar - F"},
  /* NameChar - 0x47 */            {'G',                       0,                       "NameChar - G"},
  /* NameChar - 0x48 */            {'H',                       0,                       "NameChar - H"},
  /* NameChar - 0x49 */            {'I',                       0,                       "NameChar - I"},
  /* NameChar - 0x4A */            {'J',                       0,                       "NameChar - J"},
  /* NameChar - 0x4B */            {'K',                       0,                       "NameChar - K"},
  /* NameChar - 0x4C */            {'L',                       0,                       "NameChar - L"},
  /* NameChar - 0x4D */            {'M',                       0,                       "NameChar - M"},
  /* NameChar - 0x4E */            {'N',                       0,                       "NameChar - N"},
  /* NameChar - 0x4F */            {'O',                       0,                       "NameChar - O"},
  /* NameChar - 0x50 */            {'P',                       0,                       "NameChar - P" },
  /* NameChar - 0x51 */            {'Q',                       0,                       "NameChar - Q" },
  /* NameChar - 0x52 */            {'R',                       0,                       "NameChar - R" },
  /* NameChar - 0x53 */            {'S',                       0,                       "NameChar - S" },
  /* NameChar - 0x54 */            {'T',                       0,                       "NameChar - T" },
  /* NameChar - 0x55 */            {'U',                       0,                       "NameChar - U" },
  /* NameChar - 0x56 */            {'V',                       0,                       "NameChar - V" },
  /* NameChar - 0x57 */            {'W',                       0,                       "NameChar - W" },
  /* NameChar - 0x58 */            {'X',                       0,                       "NameChar - X" },
  /* NameChar - 0x59 */            {'Y',                       0,                       "NameChar - Y" },
  /* NameChar - 0x5A */            {'Z',                       0,                       "NameChar - Z" },
  /* MutexOp - 0x5B 0x01 */        {AML_EXT_OP,                AML_EXT_MUTEX_OP,        "MutexOp"},
  /* EventOp - 0x5B 0x02 */        {AML_EXT_OP,                AML_EXT_EVENT_OP,        "EventOp"},
  /* CondRefOfOp - 0x5B 0x12 */    {AML_EXT_OP,                AML_EXT_COND_REF_OF_OP,  "CondRefOfOp"},
  /* CreateFieldOp - 0x5B 0x13 */  {AML_EXT_OP,                AML_EXT_CREATE_FIELD_OP, "CreateFieldOp"},
  /* LoadTableOp - 0x5B 0x1F */    {AML_EXT_OP,                AML_EXT_LOAD_TABLE_OP,   "LoadTableOp"},
  /* LoadOp - 0x5B 0x20 */         {AML_EXT_OP,                AML_EXT_LOAD_OP,         "LoadOp"},
  /* StallOp - 0x5B 0x21 */        {AML_EXT_OP,                AML_EXT_STALL_OP,        "StallOp"},
  /* SleepOp - 0x5B 0x22 */        {AML_EXT_OP,                AML_EXT_SLEEP_OP,        "SleepOp"},
  /* AcquireOp - 0x5B 0x23 */      {AML_EXT_OP,                AML_EXT_ACQUIRE_OP,      "AcquireOp"},
  /* SignalOp - 0x5B 0x24 */       {AML_EXT_OP,                AML_EXT_SIGNAL_OP,       "SignalOp"},
  /* WaitOp - 0x5B 0x25 */         {AML_EXT_OP,                AML_EXT_WAIT_OP,         "WaitOp"},
  /* ResetOp - 0x5B 0x26 */        {AML_EXT_OP,                AML_EXT_RESET_OP,        "ResetOp"},
  /* ReleaseOp - 0x5B 0x27 */      {AML_EXT_OP,                AML_EXT_RELEASE_OP,      "ReleaseOp"},
  /* FromBCDOp - 0x5B 0x28 */      {AML_EXT_OP,                AML_EXT_FROM_BCD_OP,     "FromBCDOp"},
  /* ToBCDOp - 0x5B 0x29 */        {AML_EXT_OP,                AML_EXT_TO_BCD_OP,       "ToBCDOp"},
  /* UnloadOp - 0x5B 0x2A */       {AML_EXT_OP,                AML_EXT_UNLOAD_OP,       "UnloadOp"},
  /* RevisionOp - 0x5B 0x30 */     {AML_EXT_OP,                AML_EXT_REVISION_OP,     "RevisionOp"},
  /* DebugOp - 0x5B 0x31 */        {AML_EXT_OP,                AML_EXT_DEBUG_OP,        "DebugOp"},
  /* FatalOp - 0x5B 0x32 */        {AML_EXT_OP,                AML_EXT_FATAL_OP,        "FatalOp"},
  /* TimerOp - 0x5B 0x33 */        {AML_EXT_OP,                AML_EXT_TIMER_OP,        "TimerOp"},
  /* OpRegionOp - 0x5B 0x80 */     {AML_EXT_OP,                AML_EXT_REGION_OP,       "OpRegionOp"},
  /* FieldOp - 0x5B 0x81 */        {AML_EXT_OP,                AML_EXT_FIELD_OP,        "FieldOp"},
  /* DeviceOp - 0x5B 0x82 */       {AML_EXT_OP,                AML_EXT_DEVICE_OP,       "DeviceOp"},
  /* ProcessorOp - 0x5B 0x83 */    {AML_EXT_OP,                AML_EXT_PROCESSOR_OP,    "ProcessorOp"},
  /* PowerResOp - 0x5B 0x84 */     {AML_EXT_OP,                AML_EXT_POWER_RES_OP,    "PowerResOp"},
  /* ThermalZoneOp - 0x5B 0x85 */  {AML_EXT_OP,                AML_EXT_THERMAL_ZONE_OP, "ThermalZoneOp"},
  /* IndexFieldOp - 0x5B 0x86 */   {AML_EXT_OP,                AML_EXT_INDEX_FIELD_OP,  "IndexFieldOp"},
  /* BankFieldOp - 0x5B 0x87 */    {AML_EXT_OP,                AML_EXT_BANK_FIELD_OP,   "BankFieldOp"},
  /* DataRegionOp - 0x5B 0x88 */   {AML_EXT_OP,                AML_EXT_DATA_REGION_OP,  "DataRegionOp"},
  /* RootChar - 0x5C */            {AML_ROOT_CHAR,             0,                       "RootChar"},
  /* ParentPrefixChar - 0x5E */    {AML_PARENT_PREFIX_CHAR,    0,                       "ParentPrefixChar"},
  /* NameChar - 0x5F */            {'_',                       0,                       "NameChar - _"},
  /* Local0Op - 0x60 */            {AML_LOCAL0,                0,                       "Local0Op"},
  /* Local1Op - 0x61 */            {AML_LOCAL1,                0,                       "Local1Op"},
  /* Local2Op - 0x62 */            {AML_LOCAL2,                0,                       "Local2Op"},
  /* Local3Op - 0x63 */            {AML_LOCAL3,                0,                       "Local3Op"},
  /* Local4Op - 0x64 */            {AML_LOCAL4,                0,                       "Local4Op"},
  /* Local5Op - 0x65 */            {AML_LOCAL5,                0,                       "Local5Op"},
  /* Local6Op - 0x66 */            {AML_LOCAL6,                0,                       "Local6Op"},
  /* Local7Op - 0x67 */            {AML_LOCAL7,                0,                       "Local7Op"},
  /* Arg0Op - 0x68 */              {AML_ARG0,                  0,                       "Arg0Op"},
  /* Arg1Op - 0x69 */              {AML_ARG1,                  0,                       "Arg1Op"},
  /* Arg2Op - 0x6A */              {AML_ARG2,                  0,                       "Arg2Op"},
  /* Arg3Op - 0x6B */              {AML_ARG3,                  0,                       "Arg3Op"},
  /* Arg4Op - 0x6C */              {AML_ARG4,                  0,                       "Arg4Op"},
  /* Arg5Op - 0x6D */              {AML_ARG5,                  0,                       "Arg5Op"},
  /* Arg6Op - 0x6E */              {AML_ARG6,                  0,                       "Arg6Op"},
  /* StoreOp - 0x70 */             {AML_STORE_OP,              0,                       "StoreOp"},
  /* RefOfOp - 0x71 */             {AML_REF_OF_OP,             0,                       "RefOfOp"},
  /* AddOp - 0x72 */               {AML_ADD_OP,                0,                       "AddOp"},
  /* ConcatOp - 0x73 */            {AML_CONCAT_OP,             0,                       "ConcatOp"},
  /* SubtractOp - 0x74 */          {AML_SUBTRACT_OP,           0,                       "SubtractOp"},
  /* IncrementOp - 0x75 */         {AML_INCREMENT_OP,          0,                       "IncrementOp"},
  /* DecrementOp - 0x76 */         {AML_DECREMENT_OP,          0,                       "DecrementOp"},
  /* MultiplyOp - 0x77 */          {AML_MULTIPLY_OP,           0,                       "MultiplyOp"},
  /* DivideOp - 0x78 */            {AML_DIVIDE_OP,             0,                       "DivideOp"},
  /* ShiftLeftOp - 0x79 */         {AML_SHIFT_LEFT_OP,         0,                       "ShiftLeftOp"},
  /* ShiftRightOp - 0x7A */        {AML_SHIFT_RIGHT_OP,        0,                       "ShiftRightOp"},
  /* AndOp - 0x7B */               {AML_AND_OP,                0,                       "AndOp"},
  /* NAndOp - 0x7C */              {AML_NAND_OP,               0,                       "NAndOp"},
  /* OrOp - 0x7D */                {AML_OR_OP,                 0,                       "OrOp"},
  /* NorOp - 0x7E */               {AML_NOR_OP,                0,                       "NorOp"},
  /* XOrOp - 0x7F */               {AML_XOR_OP,                0,                       "XOrOp"},
  /* NotOp - 0x80 */               {AML_NOT_OP,                0,                       "NotOp"},
  /* FindSetLeftBitOp - 0x81 */    {AML_FIND_SET_LEFT_BIT_OP,  0,                       "FindSetLeftBitOp"},
  /* FindSetRightBitOp - 0x82 */   {AML_FIND_SET_RIGHT_BIT_OP, 0,                       "FindSetRightBitOp"},
  /* DerefOfOp - 0x83 */           {AML_DEREF_OF_OP,           0,                       "DerefOfOp"},
  /* ConcatResOp - 0x84 */         {AML_CONCAT_RES_OP,         0,                       "ConcatResOp"},
  /* ModOp - 0x85 */               {AML_MOD_OP,                0,                       "ModOp"},
  /* NotifyOp - 0x86 */            {AML_NOTIFY_OP,             0,                       "NotifyOp"},
  /* SizeOfOp - 0x87 */            {AML_SIZE_OF_OP,            0,                       "SizeOfOp"},
  /* IndexOp - 0x88 */             {AML_INDEX_OP,              0,                       "IndexOp"},
  /* MatchOp - 0x89 */             {AML_MATCH_OP,              0,                       "MatchOp"},
  /* CreateDWordFieldOp - 0x8A */  {AML_CREATE_DWORD_FIELD_OP, 0,                       "CreateDWordFieldOp"},
  /* CreateWordFieldOp - 0x8B */   {AML_CREATE_WORD_FIELD_OP,  0,                       "CreateWordFieldOp"},
  /* CreateByteFieldOp - 0x8C */   {AML_CREATE_BYTE_FIELD_OP,  0,                       "CreateByteFieldOp"},
  /* CreateBitFieldOp - 0x8D */    {AML_CREATE_BIT_FIELD_OP,   0,                       "CreateBitFieldOp"},
  /* ObjectTypeOp - 0x8E */        {AML_OBJECT_TYPE_OP,        0,                       "ObjectTypeOp"},
  /* CreateQWordFieldOp - 0x8F */  {AML_CREATE_QWORD_FIELD_OP, 0,                       "CreateQWordFieldOp"},
  /* LAndOp - 0x90 */              {AML_LAND_OP,               0,                       "LAndOp"},
  /* LOrOp - 0x91 */               {AML_LOR_OP,                0,                       "LOrOp"},
  /* LNotOp - 0x92 */              {AML_LNOT_OP,               0,                       "LNotOp"},
  /* LEqualOp - 0x93 */            {AML_LEQUAL_OP,             0,                       "LEqualOp"},
  /* LGreaterOp - 0x94 */          {AML_LGREATER_OP,           0,                       "LGreaterOp"},
  /* LLessOp - 0x95 */             {AML_LLESS_OP,              0,                       "LLessOp"},
  /* ToBufferOp - 0x96 */          {AML_TO_BUFFER_OP,          0,                       "ToBufferOp"},
  /* ToDecimalStringOp - 0x97 */   {AML_TO_DEC_STRING_OP,      0,                       "ToDecimalStringOp"},
  /* ToHexStringOp - 0x98 */       {AML_TO_HEX_STRING_OP,      0,                       "ToHexStringOp"},
  /* ToIntegerOp - 0x99 */         {AML_TO_INTEGER_OP,         0,                       "ToIntegerOp"},
  /* ToStringOp - 0x9C */          {AML_TO_STRING_OP,          0,                       "ToStringOp"},
  /* CopyObjectOp - 0x9D */        {AML_COPY_OBJECT_OP,        0,                       "CopyObjectOp"},
  /* MidOp - 0x9E */               {AML_MID_OP,                0,                       "MidOp"},
  /* ContinueOp - 0x9F */          {AML_CONTINUE_OP,           0,                       "ContinueOp"},
  /* IfOp - 0xA0 */                {AML_IF_OP,                 0,                       "IfOp"},
  /* ElseOp - 0xA1 */              {AML_ELSE_OP,               0,                       "ElseOp"},
  /* WhileOp - 0xA2 */             {AML_WHILE_OP,              0,                       "WhileOp"},
  /* NoopOp - 0xA3 */              {AML_NOOP_OP,               0,                       "NoopOp"},
  /* ReturnOp - 0xA4 */            {AML_RETURN_OP,             0,                       "ReturnOp"},
  /* BreakOp - 0xA5 */             {AML_BREAK_OP,              0,                       "BreakOp"},
  /* BreakPointOp - 0xCC */        {AML_BREAK_POINT_OP,        0,                       "BreakPointOp"},
  /* OnesOp - 0xFF */              {AML_ONES_OP,               0,                       "OnesOp"},
};

/** Look for an OpCode/SubOpCode in the AML grammar, and returns
    their equivalent string meaning.

  @param  [in]  OpCode      The OpCode.
  @param  [in]  SubOpCode   The SubOpCode.
**/
CONST
CHAR8 *
AmlGetOpCodeStr (
  IN  UINT8   OpCode,
  IN  UINT8   SubOpCode
  )
{
  AML_OP_PARSE_INDEX  Index;

  // Search the table.
  for (Index = 0;
    Index < (sizeof (mAmlOpCodeStr) / sizeof (mAmlOpCodeStr[0]));
    Index++) {
    if ((mAmlOpCodeStr[Index].OpCode == OpCode) &&
      (mAmlOpCodeStr[Index].SubOpCode == SubOpCode)) {
      return mAmlOpCodeStr[Index].Str;
    }
  }

  return NULL;
}

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
  )
{
  UINT32  i;

  for (i = 0; i < Size; i++) {
    DEBUG ((ErrorLevel, "%c", Buffer[i]));
  }
}

/** Print the information contained in the header of the Node.

  @param  [in]  Node    Pointer to a node.
  @param  [in]  Level   Level of the indentation.
**/
STATIC
VOID
EFIAPI
AmlPrintNodeHeader (
  IN  AML_NODE_HEADER  * Node,
  IN  UINT8              Level
  )
{
  ASSERT (AML_NODE_HAS_PARENT (Node));

  DEBUG ((DEBUG_ERROR, "%02d %-20s ", Level, NodeTypeStr[Node->NodeType]));
}


/** Print fields of a data node.

  @param  [in]  DataNode  Pointer to a data node.
  @param  [in]  Level     Level of the indentation.
**/
VOID
EFIAPI
AmlPrintDataNode (
  IN  AML_DATA_NODE   * DataNode,
  IN  UINT8             Level
  )
{
  UINT32  i;
  UINT64  Integer;

  ASSERT (IS_AML_DATA_NODE (DataNode));

  AmlPrintNodeHeader ((AML_NODE_HEADER*)DataNode, Level);

  DEBUG ((DEBUG_ERROR, "%-20s ", DataTypeStr[DataNode->DataType]));
  DEBUG ((DEBUG_ERROR, "0x%04x ", DataNode->Size));

  if ((DataNode->DataType == EFI_ACPI_NODE_TYPE_NAME_STRING) ||
      (DataNode->DataType == EFI_ACPI_NODE_TYPE_STRING)) {
    AmlPrintChars (
      DEBUG_ERROR,
      (CONST CHAR8*)DataNode->Buffer,
      DataNode->Size
      );

  } else if (DataNode->DataType == EFI_ACPI_NODE_TYPE_UINT) {
    switch (DataNode->Size) {
      case 1:
      {
        Integer = *((UINT8*)DataNode->Buffer);
        break;
      }
      case 2:
      {
        Integer = *((UINT16*)DataNode->Buffer);
        break;
      }
      case 4:
      {
        Integer = *((UINT32*)DataNode->Buffer);
        break;
      }
      case 8:
      {
        Integer = *((UINT64*)DataNode->Buffer);
        break;
      }
      default:
      {
        ASSERT (0);
        return;
      }
    }
    DEBUG ((DEBUG_ERROR, "0x%x", Integer));

  } else {
    for (i = 0; i < DataNode->Size; i++) {
      DEBUG ((DEBUG_ERROR, "%02x ", DataNode->Buffer[i]));
    }
  }

  DEBUG ((DEBUG_ERROR, "\n"));
}

/** Print fields of an object node.

  @param  [in]  ObjectNode  Pointer to an object node.
  @param  [in]  Level       Level of the indentation.
**/
VOID
EFIAPI
AmlPrintObjectNode (
  IN  AML_OBJECT_NODE  * ObjectNode,
  IN  UINT8              Level
  )
{
  EFI_STATUS       Status;

  UINT32           NameStringSize = 0;
  UINT8          * NameString;
  AML_DATA_NODE  * Node;

  ASSERT (IS_AML_OBJECT_NODE (ObjectNode));

  AmlPrintNodeHeader ((AML_NODE_HEADER*)ObjectNode, Level);

  DEBUG ((DEBUG_ERROR, "0x%02x ", ObjectNode->AmlByteEncoding->OpCode));
  DEBUG ((DEBUG_ERROR, "0x%02x ", ObjectNode->AmlByteEncoding->SubOpCode));
  DEBUG ((DEBUG_ERROR, "%a ", AmlGetOpCodeStr (
                                ObjectNode->AmlByteEncoding->OpCode,
                                ObjectNode->AmlByteEncoding->SubOpCode)
                                ));
  DEBUG ((DEBUG_ERROR, "%01d ", ObjectNode->AmlByteEncoding->MaxIndex));
  DEBUG ((DEBUG_ERROR, "0x%08x ", ObjectNode->AmlByteEncoding->Attribute));
  DEBUG ((DEBUG_ERROR, "0x%04x ", ObjectNode->PkgLen));
  if (AmlObjectNodeHasAttribute (ObjectNode, AML_IN_NAMESPACE)) {
    Node = (AML_DATA_NODE*)AmlGetFixedArgument (
                             ObjectNode,
                             AML_OP_PARSE_INDEX_GET_TERM1
                             );
    if (Node == NULL) {
      ASSERT (0);
      return;
    }
    NameString = Node->Buffer;

    Status = AmlGetNameStringSize (
               NameString,
               &NameStringSize
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return;
    }

    AmlPrintChars (
      DEBUG_ERROR,
      (CONST CHAR8*)NameString,
      NameStringSize
      );
  }

  DEBUG ((DEBUG_ERROR, "\n"));
}

/** Recursively print the subtree under the Node.

  @param  [in]  Node    Pointer to the root of the subtree to print.
                        Can be a root/object/data node.
  @param  [in]  Level   Level of the indentation.
**/
VOID
EFIAPI
AmlPrintTree (
  IN  AML_NODE_HEADER   * Node,
  IN  UINT8               Level
  )
{
  AML_NODE_HEADER   * ChildNode;

  if (!IS_AML_HEADER (Node)) {
    ASSERT(0);
    return;
  }

  if (IS_AML_DATA_NODE(Node)) {
    AmlPrintDataNode ((AML_DATA_NODE*)Node, Level);
    return;

  } else if (IS_AML_OBJECT_NODE(Node)) {
    AmlPrintObjectNode ((AML_OBJECT_NODE*)Node, Level);

  } else if (IS_AML_ROOT_NODE(Node)) {
    DEBUG ((DEBUG_ERROR, "Root Node \n"));
  }

  ChildNode = NULL;
  do {
    ChildNode = AmlGetNextSibling (Node, ChildNode);

    if (ChildNode == NULL) {
      break;
    }

    AmlPrintTree (ChildNode, Level + 1);
  } while (1);
}

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
  )
{
  UINT32  ByteCount;
  UINT32  PartLineChars;
  UINT32  AsciiBufferIndex;
  CHAR8   AsciiBuffer[17];

  ByteCount = 0;
  AsciiBufferIndex = 0;

  DEBUG ((DEBUG_INFO, "Address  : 0x%p\n", Ptr));
  DEBUG ((DEBUG_INFO, "Length   : %lld", Length));

  while (ByteCount < Length) {
    if ((ByteCount & 0x0F) == 0) {
      AsciiBuffer[AsciiBufferIndex] = '\0';
      DEBUG ((DEBUG_INFO, "  %a\n%08X : ", AsciiBuffer, ByteCount));
      AsciiBufferIndex = 0;
    } else if ((ByteCount & 0x07) == 0) {
      DEBUG ((DEBUG_INFO, "- "));
    }

    if ((*Ptr >= ' ') && (*Ptr < 0x7F)) {
      AsciiBuffer[AsciiBufferIndex++] = *Ptr;
    } else {
      AsciiBuffer[AsciiBufferIndex++] = '.';
    }

    DEBUG ((DEBUG_INFO, "%02X ", *Ptr++));

    ByteCount++;
  }

  // Justify the final line using spaces before printing
  // the ASCII data.
  PartLineChars = (Length & 0x0F);
  if (PartLineChars != 0) {
    PartLineChars = 48 - (PartLineChars * 3);
    if ((Length & 0x0F) <= 8) {
      PartLineChars += 2;
    }
    while (PartLineChars > 0) {
      DEBUG ((DEBUG_INFO, " "));
      PartLineChars--;
    }
  }

  // Print ASCII data for the final line.
  AsciiBuffer[AsciiBufferIndex] = '\0';
  DEBUG ((DEBUG_INFO, "  %a\n\n", AsciiBuffer));
}
#endif

