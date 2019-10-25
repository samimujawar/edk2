/** @file
  Aml grammar definitions.

  Copyright (c) 2010 - 2018, Intel Corporation. All rights reserved. <BR>
  Copyright (c) 2019, ARM Limited. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Aml.h"

/** ASL grammar encoding table.

  The ASL language is mainly a description language, used to define abstract
  objects, like devices, thermal zones, etc. and their place in a hierarchical
  tree. The following table stores the AML grammar definition that can be used
  to parse the AML bytestream. To each line correspond the definition of an
  object and what is expected to be found in this object.
  It has been made out of the table 20-440 in the ACPI 6.3, s20.3
  specification, and the AML grammar definition in s20.2.

  -Objects:
  Each object has an associated opcode. An opcode can be made of 1 or 2 bytes.
  When the opcode is the extension opcode prefix (0x5B), the sub-opcode is
  used. Otherwise it is set to 0.

  -Fixed arguments:
  After the object opcode comes the object's fixed arguments. Their number and
  type can be found in the table below. There can be at most 6 fixed arguments
  for an opcode.
  Fixed arguments's type allow to know what is expected in the bytestream. AML
  Knowing the size of the incoming element, AML bytes can be packed and parsed
  accordingly. These types can be found in the same table 20-440 in the
  ACPI 6.3, s20.3 specification.

  -Attributes:
  The attribute field gives additionnal information on each object. This can
  be the presence of a variable list of arguments, the presence of a PkgLen ...
  See their definition for more information.

  To sum up, an AML object looks like, and in the same order:
  OpCode                        {1 byte}
  (SubOpCode)                   {Most object don't have one}
  (PkgLen)                      {Size of the object, most objects don't have
                                 one.}
  FixedArg[0..X]                {The number of fixed arguments varies according
                                 to the object. Can be new objects or data.}
  (Variable list of arguments)  {List of objects. Most object don't have one.}
  (Byte list)                   {List of bytes. Most object don't have one.}
*/
GLOBAL_REMOVE_IF_UNREFERENCED
STATIC
CONST
AML_BYTE_ENCODING mAmlByteEncoding[] = {
  //                             OpCode                      SubOpCode              Num 1           2           3           4           5           6           Attribute
  /* ZeroOp - 0x00 */              {AML_ZERO_OP,               0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* OneOp  - 0x01 */              {AML_ONE_OP,                0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* AliasOp - 0x06 */             {AML_ALIAS_OP,              0,                      2, {AML_NAME,   AML_NAME,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IN_NAMESPACE},
  /* NameOp - 0x08 */              {AML_NAME_OP,               0,                      2, {AML_NAME,   AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IN_NAMESPACE},
  /* BtyePrefix - 0x0A */          {AML_BYTE_PREFIX,           0,                      1, {AML_UINT8,  AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* WordPrefix - 0x0B */          {AML_WORD_PREFIX,           0,                      1, {AML_UINT16, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* DWordPrefix - 0x0C */         {AML_DWORD_PREFIX,          0,                      1, {AML_UINT32, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* StringPrefix - 0x0D */        {AML_STRING_PREFIX,         0,                      1, {AML_STRING, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* QWordPrefix - 0x0E */         {AML_QWORD_PREFIX,          0,                      1, {AML_UINT64, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ScopeOp - 0x10 */             {AML_SCOPE_OP,              0,                      1, {AML_NAME,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_CHILD_OBJ | AML_IN_NAMESPACE},
  /* BufferOp - 0x11 */            {AML_BUFFER_OP,             0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_BYTE_LIST},
  /* PackageOp - 0x12 */           {AML_PACKAGE_OP,            0,                      1, {AML_UINT8,  AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_CHILD_OBJ},
  /* VarPackageOp - 0x13 */        {AML_VAR_PACKAGE_OP,        0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_CHILD_OBJ},
  /* MethodOp - 0x14 */            {AML_METHOD_OP,             0,                      2, {AML_NAME,   AML_UINT8,  AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_CHILD_OBJ | AML_IN_NAMESPACE},
  /* ExternalOp - 0x15 */          {AML_EXTERNAL_OP,           0,                      3, {AML_NAME,   AML_UINT8,  AML_UINT8,  AML_NONE,   AML_NONE,   AML_NONE},   AML_IN_NAMESPACE},
  /* DualNamePrefix - 0x2E */      {AML_DUAL_NAME_PREFIX,      0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* MultiNamePrefix - 0x2F */     {AML_MULTI_NAME_PREFIX,     0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x41 */            {'A',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x42 */            {'B',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x43 */            {'C',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x44 */            {'D',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x45 */            {'E',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x46 */            {'F',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x47 */            {'G',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x48 */            {'H',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x49 */            {'I',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x4A */            {'J',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x4B */            {'K',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x4C */            {'L',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x4D */            {'M',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x4E */            {'N',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x4F */            {'O',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x50 */            {'P',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x51 */            {'Q',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x52 */            {'R',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x53 */            {'S',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x54 */            {'T',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x55 */            {'U',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x56 */            {'V',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x57 */            {'W',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x58 */            {'X',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x59 */            {'Y',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x5A */            {'Z',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* MutexOp - 0x5B 0x01 */        {AML_EXT_OP,                AML_EXT_MUTEX_OP,       2, {AML_NAME,   AML_UINT8,  AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IN_NAMESPACE},
  /* EventOp - 0x5B 0x02 */        {AML_EXT_OP,                AML_EXT_EVENT_OP,       1, {AML_NAME,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IN_NAMESPACE},
  /* CondRefOfOp - 0x5B 0x12 */    {AML_EXT_OP,                AML_EXT_COND_REF_OF_OP, 2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* CreateFieldOp - 0x5B 0x13 */  {AML_EXT_OP,                AML_EXT_CREATE_FIELD_OP,4, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NAME,   AML_NONE,   AML_NONE},   0},
  /* LoadTableOp - 0x5B 0x1F */    {AML_EXT_OP,                AML_EXT_LOAD_TABLE_OP,  6, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_OBJECT}, 0},
  /* LoadOp - 0x5B 0x20 */         {AML_EXT_OP,                AML_EXT_LOAD_OP,        2, {AML_NAME,   AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* StallOp - 0x5B 0x21 */        {AML_EXT_OP,                AML_EXT_STALL_OP,       1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* SleepOp - 0x5B 0x22 */        {AML_EXT_OP,                AML_EXT_SLEEP_OP,       1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* AcquireOp - 0x5B 0x23 */      {AML_EXT_OP,                AML_EXT_ACQUIRE_OP,     2, {AML_OBJECT, AML_UINT16, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* SignalOp - 0x5B 0x24 */       {AML_EXT_OP,                AML_EXT_SIGNAL_OP,      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* WaitOp - 0x5B 0x25 */         {AML_EXT_OP,                AML_EXT_WAIT_OP,        2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ResetOp - 0x5B 0x26 */        {AML_EXT_OP,                AML_EXT_RESET_OP,       1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ReleaseOp - 0x5B 0x27 */      {AML_EXT_OP,                AML_EXT_RELEASE_OP,     1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* FromBCDOp - 0x5B 0x28 */      {AML_EXT_OP,                AML_EXT_FROM_BCD_OP,    2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ToBCDOp - 0x5B 0x29 */        {AML_EXT_OP,                AML_EXT_TO_BCD_OP,      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* UnloadOp - 0x5B 0x2A */       {AML_EXT_OP,                AML_EXT_UNLOAD_OP,      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* RevisionOp - 0x5B 0x30 */     {AML_EXT_OP,                AML_EXT_REVISION_OP,    0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* DebugOp - 0x5B 0x31 */        {AML_EXT_OP,                AML_EXT_DEBUG_OP,       0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* FatalOp - 0x5B 0x32 */        {AML_EXT_OP,                AML_EXT_FATAL_OP,       3, {AML_UINT8,  AML_UINT32, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* TimerOp - 0x5B 0x33 */        {AML_EXT_OP,                AML_EXT_TIMER_OP,       0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* OpRegionOp - 0x5B 0x80 */     {AML_EXT_OP,                AML_EXT_REGION_OP,      4, {AML_NAME,   AML_UINT8,  AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE},   AML_IN_NAMESPACE},
  /* FieldOp - 0x5B 0x81 */        {AML_EXT_OP,                AML_EXT_FIELD_OP,       2, {AML_NAME,   AML_UINT8,  AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_BYTE_LIST},
  /* DeviceOp - 0x5B 0x82 */       {AML_EXT_OP,                AML_EXT_DEVICE_OP,      1, {AML_NAME,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_CHILD_OBJ | AML_IN_NAMESPACE},
  /* ProcessorOp - 0x5B 0x83 */    {AML_EXT_OP,                AML_EXT_PROCESSOR_OP,   4, {AML_NAME,   AML_UINT8,  AML_UINT32, AML_UINT8,  AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_CHILD_OBJ | AML_IN_NAMESPACE},
  /* PowerResOp - 0x5B 0x84 */     {AML_EXT_OP,                AML_EXT_POWER_RES_OP,   3, {AML_NAME,   AML_UINT8,  AML_UINT16, AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_CHILD_OBJ | AML_IN_NAMESPACE},
  /* ThermalZoneOp - 0x5B 0x85 */  {AML_EXT_OP,                AML_EXT_THERMAL_ZONE_OP,1, {AML_NAME,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_CHILD_OBJ | AML_IN_NAMESPACE},
  /* IndexFieldOp - 0x5B 0x86 */   {AML_EXT_OP,                AML_EXT_INDEX_FIELD_OP, 3, {AML_NAME,   AML_NAME,   AML_UINT8,  AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_BYTE_LIST},
  /* BankFieldOp - 0x5B 0x87 */    {AML_EXT_OP,                AML_EXT_BANK_FIELD_OP,  4, {AML_NAME,   AML_NAME,   AML_OBJECT, AML_UINT8,  AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_BYTE_LIST},
  /* DataRegionOp - 0x5B 0x88 */   {AML_EXT_OP,                AML_EXT_DATA_REGION_OP, 4, {AML_NAME,   AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE},   AML_IN_NAMESPACE},
  /* RootChar - 0x5C */            {AML_ROOT_CHAR,             0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* ParentPrefixChar - 0x5E */    {AML_PARENT_PREFIX_CHAR,    0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* NameChar - 0x5F */            {'_',                       0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_IS_NAME_CHAR},
  /* Local0Op - 0x60 */            {AML_LOCAL0,                0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Local1Op - 0x61 */            {AML_LOCAL1,                0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Local2Op - 0x62 */            {AML_LOCAL2,                0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Local3Op - 0x63 */            {AML_LOCAL3,                0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Local4Op - 0x64 */            {AML_LOCAL4,                0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Local5Op - 0x65 */            {AML_LOCAL5,                0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Local6Op - 0x66 */            {AML_LOCAL6,                0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Local7Op - 0x67 */            {AML_LOCAL7,                0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Arg0Op - 0x68 */              {AML_ARG0,                  0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Arg1Op - 0x69 */              {AML_ARG1,                  0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Arg2Op - 0x6A */              {AML_ARG2,                  0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Arg3Op - 0x6B */              {AML_ARG3,                  0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Arg4Op - 0x6C */              {AML_ARG4,                  0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Arg5Op - 0x6D */              {AML_ARG5,                  0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* Arg6Op - 0x6E */              {AML_ARG6,                  0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* StoreOp - 0x70 */             {AML_STORE_OP,              0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* RefOfOp - 0x71 */             {AML_REF_OF_OP,             0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* AddOp - 0x72 */               {AML_ADD_OP,                0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ConcatOp - 0x73 */            {AML_CONCAT_OP,             0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* SubtractOp - 0x74 */          {AML_SUBTRACT_OP,           0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* IncrementOp - 0x75 */         {AML_INCREMENT_OP,          0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* DecrementOp - 0x76 */         {AML_DECREMENT_OP,          0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* MultiplyOp - 0x77 */          {AML_MULTIPLY_OP,           0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* DivideOp - 0x78 */            {AML_DIVIDE_OP,             0,                      4, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE},   0},
  /* ShiftLeftOp - 0x79 */         {AML_SHIFT_LEFT_OP,         0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ShiftRightOp - 0x7A */        {AML_SHIFT_RIGHT_OP,        0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* AndOp - 0x7B */               {AML_AND_OP,                0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* NAndOp - 0x7C */              {AML_NAND_OP,               0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* OrOp - 0x7D */                {AML_OR_OP,                 0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* NorOp - 0x7E */               {AML_NOR_OP,                0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* XOrOp - 0x7F */               {AML_XOR_OP,                0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* NotOp - 0x80 */               {AML_NOT_OP,                0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* FindSetLeftBitOp - 0x81 */    {AML_FIND_SET_LEFT_BIT_OP,  0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* FindSetRightBitOp - 0x82 */   {AML_FIND_SET_RIGHT_BIT_OP, 0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* DerefOfOp - 0x83 */           {AML_DEREF_OF_OP,           0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ConcatResOp - 0x84 */         {AML_CONCAT_RES_OP,         0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ModOp - 0x85 */               {AML_MOD_OP,                0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* NotifyOp - 0x86 */            {AML_NOTIFY_OP,             0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* SizeOfOp - 0x87 */            {AML_SIZE_OF_OP,            0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* IndexOp - 0x88 */             {AML_INDEX_OP,              0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* MatchOp - 0x89 */             {AML_MATCH_OP,              0,                      6, {AML_OBJECT, AML_UINT8,  AML_OBJECT, AML_UINT8,  AML_OBJECT, AML_OBJECT}, 0},
  /* CreateDWordFieldOp - 0x8A */  {AML_CREATE_DWORD_FIELD_OP, 0,                      3, {AML_OBJECT, AML_OBJECT, AML_NAME,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* CreateWordFieldOp - 0x8B */   {AML_CREATE_WORD_FIELD_OP,  0,                      3, {AML_OBJECT, AML_OBJECT, AML_NAME,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* CreateByteFieldOp - 0x8C */   {AML_CREATE_BYTE_FIELD_OP,  0,                      3, {AML_OBJECT, AML_OBJECT, AML_NAME,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* CreateBitFieldOp - 0x8D */    {AML_CREATE_BIT_FIELD_OP,   0,                      3, {AML_OBJECT, AML_OBJECT, AML_NAME,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ObjectTypeOp - 0x8E */        {AML_OBJECT_TYPE_OP,        0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* CreateQWordFieldOp - 0x8F */  {AML_CREATE_QWORD_FIELD_OP, 0,                      3, {AML_OBJECT, AML_OBJECT, AML_NAME,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* LAndOp - 0x90 */              {AML_LAND_OP,               0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* LOrOp - 0x91 */               {AML_LOR_OP,                0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* LNotOp - 0x92 */              {AML_LNOT_OP,               0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* LEqualOp - 0x93 */            {AML_LEQUAL_OP,             0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* LGreaterOp - 0x94 */          {AML_LGREATER_OP,           0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* LLessOp - 0x95 */             {AML_LLESS_OP,              0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ToBufferOp - 0x96 */          {AML_TO_BUFFER_OP,          0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ToDecimalStringOp - 0x97 */   {AML_TO_DEC_STRING_OP,      0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ToHexStringOp - 0x98 */       {AML_TO_HEX_STRING_OP,      0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ToIntegerOp - 0x99 */         {AML_TO_INTEGER_OP,         0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ToStringOp - 0x9C */          {AML_TO_STRING_OP,          0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* CopyObjectOp - 0x9D */        {AML_COPY_OBJECT_OP,        0,                      2, {AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* MidOp - 0x9E */               {AML_MID_OP,                0,                      3, {AML_OBJECT, AML_OBJECT, AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ContinueOp - 0x9F */          {AML_CONTINUE_OP,           0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* IfOp - 0xA0 */                {AML_IF_OP,                 0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_CHILD_OBJ},
  /* ElseOp - 0xA1 */              {AML_ELSE_OP,               0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_CHILD_OBJ},
  /* WhileOp - 0xA2 */             {AML_WHILE_OP,              0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   AML_HAS_PKG_LENGTH | AML_HAS_CHILD_OBJ},
  /* NoopOp - 0xA3 */              {AML_NOOP_OP,               0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* ReturnOp - 0xA4 */            {AML_RETURN_OP,             0,                      1, {AML_OBJECT, AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* BreakOp - 0xA5 */             {AML_BREAK_OP,              0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* BreakPointOp - 0xCC */        {AML_BREAK_POINT_OP,        0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
  /* OnesOp - 0xFF */              {AML_ONES_OP,               0,                      0, {AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE,   AML_NONE},   0},
};

/** AML_OP_PARSE_FORMAT to EFI_ACPI_NODE_TYPE translation table.
*/
GLOBAL_REMOVE_IF_UNREFERENCED
STATIC
CONST
EFI_ACPI_NODE_TYPE  mAmlTypeToAcpiType[] = {
  EFI_ACPI_NODE_TYPE_NONE,         // AML_NONE
  EFI_ACPI_NODE_TYPE_UINT,         // AML_UINT8
  EFI_ACPI_NODE_TYPE_UINT,         // AML_UINT16
  EFI_ACPI_NODE_TYPE_UINT,         // AML_UINT32
  EFI_ACPI_NODE_TYPE_UINT,         // AML_UINT64
  EFI_ACPI_NODE_TYPE_NAME_STRING,  // AML_NAME
  EFI_ACPI_NODE_TYPE_STRING,       // AML_STRING
  EFI_ACPI_NODE_TYPE_CHILD         // AML_OBJECT
};

/** Convert an EFI_ACPI_DATA_TYPE to its corresponding AML_OP_PARSE_FORMAT.

  @param  [in]  AmlType   Input AML Type.

  @return The corresponding EFI_ACPI_DATA_TYPE.
          EFI_ACPI_DATA_TYPE_NONE if not found.
**/
EFI_ACPI_NODE_TYPE
EFIAPI
AmlTypeToAcpiType (
  IN  AML_OP_PARSE_FORMAT   AmlType
  )
{
  if (AmlType >=
      (sizeof (mAmlTypeToAcpiType) / sizeof (mAmlTypeToAcpiType[0]))) {
    ASSERT (0);
    return EFI_ACPI_NODE_TYPE_NONE;
  }

  return mAmlTypeToAcpiType[AmlType];
}

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
  )
{
  UINT8               OpCode;
  UINT8               SubOpCode;
  AML_OP_PARSE_INDEX  Index;

  if (OpByteBuffer == NULL) {
    ASSERT (0);
    return NULL;
  }

  // Get OpCode and SubOpCode.
  OpCode = OpByteBuffer[0];
  if (OpCode == AML_EXT_OP) {
    SubOpCode = OpByteBuffer[1];
  } else {
    SubOpCode = 0;
  }

  // Search the table.
  for (Index = 0;
       Index < (sizeof (mAmlByteEncoding) / sizeof (mAmlByteEncoding[0]));
       Index++) {
    if ((mAmlByteEncoding[Index].OpCode == OpCode) &&
        (mAmlByteEncoding[Index].SubOpCode == SubOpCode)) {
      return &mAmlByteEncoding[Index];
    }
  }

  return NULL;
}

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
  )
{
  AML_OP_PARSE_INDEX  Index;

  // Search the table.
  for (Index = 0;
    Index < (sizeof (mAmlByteEncoding) / sizeof (mAmlByteEncoding[0]));
    Index++) {
    if ((mAmlByteEncoding[Index].OpCode == OpCode) &&
      (mAmlByteEncoding[Index].SubOpCode == SubOpCode)) {
      return TRUE;
    }
  }
  return FALSE;
}

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
  )
{
  UINT8   LeadByte;
  UINT8   ByteCount;
  UINT32  RealLength;
  UINT32  Offset;

  if (PkgLength == NULL) {
    ASSERT (0);
    return 0;
  }

  // Form ACPI 6.3, s20.2.4 specification:
  //
  // <bit 7-6: ByteData count that follows (0-3)>
  // <bit 5-4: Only used if PkgLength < 63>
  // <bit 3-0: Least significant package length nybble>
  //
  // Note: The high 2 bits of the first byte reveal how many follow bytes are in the
  // If the PkgLength has only one byte, bit 0 through 5 are used to encode the
  // package length (in other words, values 0-63). If the package length value is more than
  // 63, more than one byte must be used for the encoding in which case bit 4 and 5 of the
  // PkgLeadByte are reserved and must be zero. If the multiple bytes encoding is used,
  // bits 0-3 of the PkgLeadByte become the least significant 4 bits of the resulting
  // package length value. The next ByteData will become the next least significant 8 bits
  // of the resulting value and so on, up to 3 ByteData bytes. Thus, the maximum package
  // length is 2**28.

  LeadByte = *Buffer;
  ByteCount = (UINT8)((LeadByte >> 6) & 0x03);
  Offset = ByteCount + 1;
  RealLength = 0;

  // Switch on the number of bytes used to store the PkgLen.
  switch (ByteCount) {
  case 0:
    RealLength = LeadByte;
    break;

  case 1:
    RealLength = *(Buffer + 1);
    RealLength = (RealLength << 4) | (LeadByte & 0xF);
    break;

  case 2:
    RealLength = *(Buffer + 1);
    RealLength |= *(Buffer + 2) << 8;
    RealLength = (RealLength << 4) | (LeadByte & 0xF);
    break;

  case 3:
    RealLength = *(Buffer + 1);
    RealLength |= (*(Buffer + 2)) << 8;
    RealLength |= (*(Buffer + 3)) << 16;
    RealLength = (RealLength << 4) | (LeadByte & 0xF);
    break;

  default:
    ASSERT (0);
    Offset = 0;
    break;
  }

  *PkgLength = RealLength;
  return Offset;
}

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
  )
{
  UINT8   LeadByte;
  UINT8   Offset;
  UINT8   CurrentOffset;
  UINT8   CurrentShift;
  UINT32  ComputedLength;

  if (Buffer == NULL) {
    ASSERT (0);
    return 0;
  }

  LeadByte = 0;
  Offset = 0;

  // Length < 2^6, only need one byte to encode it.
  if (!(Length & 0xFFFFFFC0)) {
    LeadByte = Length;

  } else {
    // Need more than one byte to encode it.

    // Length >= 2^28, should not be possible.
    if (Length & 0xF0000000) {
      return 0;

    // Length >= 2^20
    } else if (Length & 0x0FF00000) {
      Offset = 3;

    // Length >= 2^12
    } else if (Length & 0x000FF000) {
      Offset = 2;

    // Length >= 2^6
    } else if (Length & 0x00000FC0) {
      Offset = 1;
    }

    // Set the LeadByte.
    LeadByte = Offset << 6;
    LeadByte |= (Length & 0X0F);
  }

  // Write to the Buffer.
  *(Buffer + 0) = LeadByte;
  CurrentOffset = 1;
  while (CurrentOffset < (Offset + 1)) {
    CurrentShift = ((CurrentOffset - 1) * 8);
    ComputedLength = (Length & (0x00000FF0 << CurrentShift));
    ComputedLength = (ComputedLength) >> (4 + CurrentShift);
    LeadByte = (UINT8)(ComputedLength & 0xFF);
    *(Buffer + CurrentOffset) = LeadByte;
    CurrentOffset++;
  }

  return ++Offset;
}

/** Compute the number of bytes required to write a package length.

  @param  [in]  Length  The length to convert in the AML package length
                        encoding style.

  @return The number of bytes required to write the Length.
**/
UINT8
EFIAPI
AmlComputePkgLengthWidth (
  IN  UINT32  Length
  )
{
  // Length >= 2^28, should not be possible.
  if (Length & 0xF0000000) {
    return 0;

  // Length >= 2^20
  } else if (Length & 0x0FF00000) {
    return 4;

  // Length >= 2^12
  } else if (Length & 0x000FF000) {
    return 3;

  // Length >= 2^6
  } else if (Length & 0x00000FC0) {
    return 2;

  // Length < 2^6
  } else {
    return 1;
  }
}
