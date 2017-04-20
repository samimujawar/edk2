/**
  IORT table parser

  Copyright (c) 2016 - 2018, ARM Limited. All rights reserved.
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  @par Reference(s):
    - IO Remapping Table, Platform Design Document, Revision C, 15 May 2017
**/

#include <IndustryStandard/IoRemappingTable.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/AcpiView/AcpiParser.h>
#include <Library/AcpiView/AcpiTableParser.h>

// Local variables
STATIC ACPI_DESCRIPTION_HEADER_INFO AcpiHdrInfo;

/** The EIORT_NODE enum describes the IORT Node types.
*/
typedef enum IortNode {
  EIORT_NODE_ITS_GROUP,        ///< ITS Group node
  EIORT_NODE_NAMED_COMPONENT,  ///< Named Component node
  EIORT_NODE_ROOT_COMPLEX,     ///< Root Complex node
  EIORT_NODE_SMMUV1_V2,        ///< SMMU v1/v2 node
  EIORT_NODE_SMMUV3,           ///< SMMU v3 node
  EIORT_NODE_PMCG,             ///< PMC group node
  EIORT_NODE_MAX
} EIORT_NODE;

// Local Variables
STATIC CONST UINT32* IortNodeCount;
STATIC CONST UINT32* IortNodeOffset;

STATIC CONST UINT8* IortNodeType;
STATIC CONST UINT16* IortNodeLength;
STATIC CONST UINT32* IortIdMappingCount;
STATIC CONST UINT32* IortIdMappingOffset;

STATIC CONST UINT32* InterruptContextCount;
STATIC CONST UINT32* InterruptContextOffset;
STATIC CONST UINT32* PmuInterruptCount;
STATIC CONST UINT32* PmuInterruptOffset;

STATIC CONST UINT32* ItsCount;

/** This function validates the ID Mapping array count for the ITS node.

  @param [in] Ptr     Pointer to the start of the field data.
  @param [in] Context Pointer to context specific information e.g. this
                      could be a pointer to the ACPI table header.
*/
STATIC
VOID
ValidateItsIdMappingCount (
  IN UINT8* Ptr,
  IN VOID*  Context
  );

/** This function validates the ID Mapping array offset for the ITS node.

  @param [in] Ptr     Pointer to the start of the field data.
  @param [in] Context Pointer to context specific information e.g. this
                      could be a pointer to the ACPI table header.
*/
STATIC
VOID
ValidateItsIdArrayReference (
  IN UINT8* Ptr,
  IN VOID*  Context
  );

/** Helper Macro for populating the IORT Node header in the ACPI_PARSER
    array.

  @param [out] ValidateIdMappingCount    Optional pointer to a function for
                                         validating the ID Mapping count.
  @param [out] ValidateIdArrayReference  Optional pointer to a function for
                                         validating the ID Array reference.
*/
#define PARSE_IORT_NODE_HEADER(ValidateIdMappingCount,                   \
                               ValidateIdArrayReference)                 \
  { L"Type", 1, 0, L"%d", NULL, (VOID**)&IortNodeType, NULL, NULL },     \
  { L"Length", 2, 1, L"%d", NULL, (VOID**)&IortNodeLength, NULL, NULL }, \
  { L"Revision", 1, 3, L"%d", NULL, NULL, NULL, NULL },                  \
  { L"Reserved", 4, 4, L"0x%x", NULL, NULL, NULL, NULL },                \
  { L"Number of ID mappings", 4, 8, L"%d", NULL,                         \
    (VOID**)&IortIdMappingCount, ValidateIdMappingCount, NULL },         \
  { L"Reference to ID Array", 4, 12, L"0x%x", NULL,                      \
    (VOID**)&IortIdMappingOffset, ValidateIdArrayReference, NULL }

/** An ACPI_PARSER array describing the ACPI IORT Table
*/
STATIC CONST ACPI_PARSER IortParser[] = {
  PARSE_ACPI_HEADER (&AcpiHdrInfo),
  {L"Number of IORT Nodes", 4, 36, L"%d", NULL,
   (VOID**)&IortNodeCount, NULL, NULL},
  {L"Offset to Array of IORT Nodes", 4, 40, L"0x%x", NULL,
   (VOID**)&IortNodeOffset, NULL, NULL},
  {L"Reserved", 4, 44, L"0x%x", NULL, NULL, NULL, NULL}
};

/** An ACPI_PARSER array describing the IORT node header structure.
*/
STATIC CONST ACPI_PARSER IortNodeHeaderParser[] = {
  PARSE_IORT_NODE_HEADER (NULL, NULL)
};

/** An ACPI_PARSER array describing the IORT SMMUv1/2 node.
*/
STATIC CONST ACPI_PARSER IortNodeSmmuV1V2Parser[] = {
  PARSE_IORT_NODE_HEADER (NULL, NULL),
  {L"Base Address", 8, 16, L"0x%lx", NULL, NULL, NULL, NULL},
  {L"Span", 8, 24, L"0x%lx", NULL, NULL, NULL, NULL},
  {L"Model", 4, 32, L"%d", NULL, NULL, NULL, NULL},
  {L"Flags", 4, 36, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Reference to Global Interrupt Array", 4, 40, L"0x%x", NULL, NULL, NULL,
   NULL},
  {L"Number of context interrupts", 4, 44, L"%d", NULL,
   (VOID**)&InterruptContextCount, NULL, NULL},
  {L"Reference to Context Interrupt Array", 4, 48, L"0x%x", NULL,
   (VOID**)&InterruptContextOffset, NULL, NULL},
  {L"Number of PMU Interrupts", 4, 52, L"%d", NULL,
   (VOID**)&PmuInterruptCount, NULL, NULL},
  {L"Reference to PMU Interrupt Array", 4, 56, L"0x%x", NULL,
   (VOID**)&PmuInterruptOffset, NULL, NULL},

  // Interrupt Array
  {L"SMMU_NSgIrpt", 4, 60, L"0x%x", NULL, NULL, NULL, NULL},
  {L"SMMU_NSgIrpt interrupt flags", 4, 64, L"0x%x", NULL, NULL, NULL, NULL},
  {L"SMMU_NSgCfgIrpt", 4, 68, L"0x%x", NULL, NULL, NULL, NULL},
  {L"SMMU_NSgCfgIrpt interrupt flags", 4, 72, L"0x%x", NULL, NULL, NULL, NULL}
};

/** An ACPI_PARSER array describing the SMMUv1/2 Node Interrupt Array.
*/
STATIC CONST ACPI_PARSER InterruptArrayParser[] = {
  {L"  Interrupt GSIV", 4, 0, L"0x%x", NULL, NULL, NULL, NULL},
  {L"  Flags", 4, 4, L"0x%x", NULL, NULL, NULL, NULL}
};

/** An ACPI_PARSER array describing the IORT ID Mapping.
*/
STATIC CONST ACPI_PARSER IortNodeIdMappingParser[] = {
  {L"  Input base", 4, 0, L"0x%x", NULL, NULL, NULL, NULL},
  {L"  Number of IDs", 4, 4, L"0x%x", NULL, NULL, NULL, NULL},
  {L"  Output base", 4, 8, L"0x%x", NULL, NULL, NULL, NULL},
  {L"  Output reference", 4, 12, L"0x%x", NULL, NULL, NULL, NULL},
  {L"  Flags", 4, 16, L"0x%x", NULL, NULL, NULL, NULL}
};

/** An ACPI_PARSER array describing the IORT SMMUv3 node.
*/
STATIC CONST ACPI_PARSER IortNodeSmmuV3Parser[] = {
  PARSE_IORT_NODE_HEADER (NULL, NULL),
  {L"Base Address", 8, 16, L"0x%lx", NULL, NULL, NULL, NULL},
  {L"Flags", 4, 24, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Reserved", 4, 28, L"0x%x", NULL, NULL, NULL, NULL},
  {L"VATOS Address", 8, 32, L"0x%lx", NULL, NULL, NULL, NULL},
  {L"Model", 4, 40, L"%d", NULL, NULL, NULL, NULL},
  {L"Event", 4, 44, L"0x%x", NULL, NULL, NULL, NULL},
  {L"PRI", 4, 48, L"0x%x", NULL, NULL, NULL, NULL},
  {L"GERR", 4, 52, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Sync", 4, 56, L"0x%x", NULL, NULL, NULL, NULL}
};

/** An ACPI_PARSER array describing the IORT ITS node.
*/
STATIC CONST ACPI_PARSER IortNodeItsParser[] = {
  PARSE_IORT_NODE_HEADER (
    ValidateItsIdMappingCount,
    ValidateItsIdArrayReference
    ),
  {L"  Number of ITSs", 4, 16, L"%d", NULL, (VOID**)&ItsCount, NULL}
};

/** An ACPI_PARSER array describing the ITS ID.
*/
STATIC CONST ACPI_PARSER ItsIdParser[] = {
  { L"  GIC ITS Identifier", 4, 0, L"%d", NULL, NULL, NULL }
};

/** An ACPI_PARSER array describing the IORT Names Component node.
*/
STATIC CONST ACPI_PARSER IortNodeNamedComponentParser[] = {
  PARSE_IORT_NODE_HEADER (NULL, NULL),
  {L"Node Flags", 4, 16, L"%d", NULL, NULL, NULL, NULL},
  {L"Memory access properties", 8, 20, L"0x%lx", NULL, NULL, NULL, NULL},
  {L"Device memory address size limit", 1, 28, L"%d", NULL, NULL, NULL, NULL}
};

/** An ACPI_PARSER array describing the IORT Root Complex node.
*/
STATIC CONST ACPI_PARSER IortNodeRootComplexParser[] = {
  PARSE_IORT_NODE_HEADER (NULL, NULL),
  {L"Memory access properties", 8, 16, L"0x%lx", NULL, NULL, NULL, NULL},
  {L"ATS Attribute", 4, 24, L"0x%x", NULL, NULL, NULL, NULL},
  {L"PCI Segment number", 4, 28, L"0x%x", NULL, NULL, NULL, NULL}
};

/** An ACPI_PARSER array describing the IORT PMCG node.
*/
STATIC CONST ACPI_PARSER IortNodePmcgParser[] = {
  PARSE_IORT_NODE_HEADER (NULL, NULL),
  {L"Base Address", 8, 16, L"0x%lx", NULL, NULL, NULL, NULL},
  {L"Overflow interrupt GSIV", 4, 24, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Node reference", 4, 28, L"0x%x", NULL, NULL, NULL, NULL},
};

/** This function validates the ID Mapping array count for the ITS node.

  @param [in] Ptr     Pointer to the start of the field data.
  @param [in] Context Pointer to context specific information e.g. this
                      could be a pointer to the ACPI table header.
*/
STATIC
VOID
ValidateItsIdMappingCount (
  IN UINT8* Ptr,
  VOID*     Context
  )
{
  if (*(UINT32*)Ptr != 0) {
    IncrementErrorCount ();
    Print (L"\nERROR: IORT ID Mapping count must be zero.");
  }
}

/** This function validates the ID Mapping array offset for the ITS node.

  @param [in] Ptr     Pointer to the start of the field data.
  @param [in] Context Pointer to context specific information e.g. this
                      could be a pointer to the ACPI table header.
*/
STATIC
VOID
ValidateItsIdArrayReference (
  IN UINT8* Ptr,
  IN VOID*  Context
  )
{
  if (*(UINT32*)Ptr != 0) {
    IncrementErrorCount ();
    Print (L"\nERROR: IORT ID Mapping offset must be zero.");
  }
}

/** This function parses the IORT Node Id Mapping array.

  @param [in] Ptr            Pointer to the start of the IORT Table.
  @param [in] MappingCount   The ID Mapping count.
  @param [in] MappingOffset  The offset of the ID Mapping array
                             from the start of the IORT table.
*/
STATIC
VOID
DumpIortNodeIdMappings (
  IN UINT8* Ptr,
  IN UINT32 MappingCount,
  IN UINT32 MappingOffset
  )
{
  UINT8* IdMappingPtr;
  UINT32 Index;
  UINT32 Offset;
  CHAR8  Buffer[40];  // Used for AsciiName param of ParseAcpi

  IdMappingPtr = Ptr + MappingOffset;
  Index = 0;
  while (Index < MappingCount) {
    AsciiSPrint (
      Buffer,
      sizeof (Buffer),
      "ID Mapping [%d]",
      Index
      );
    Offset = ParseAcpi (
               TRUE,
               4,
               Buffer,
               IdMappingPtr,
               20,
               PARSER_PARAMS (IortNodeIdMappingParser)
               );
    IdMappingPtr += Offset;
    Index++;
  }
}

/** This function parses the IORT SMMUv1/2 node.

  @param [in] Ptr            Pointer to the start of the buffer.
  @param [in] Length         Length of the buffer.
  @param [in] MappingCount   The ID Mapping count.
  @param [in] MappingOffset  The offset of the ID Mapping array
                             from the start of the IORT table.
*/
STATIC
VOID
DumpIortNodeSmmuV1V2 (
  IN UINT8* Ptr,
  IN UINT16 Length,
  IN UINT32 MappingCount,
  IN UINT32 MappingOffset
  )
{
  UINT32 Index;
  UINT32 Offset;
  CHAR8  Buffer[50];  // Used for AsciiName param of ParseAcpi

  UINT8* ArrayPtr;

  ParseAcpi (
    TRUE,
    2,
    "SMMUv1 or SMMUv2 Node",
    Ptr,
    Length,
    PARSER_PARAMS (IortNodeSmmuV1V2Parser)
    );

  ArrayPtr = Ptr + *InterruptContextOffset;
  Index = 0;
  while (Index < *InterruptContextCount) {
    AsciiSPrint (
      Buffer,
      sizeof (Buffer),
      "Context Interrupts Array [%d]",
      Index
      );
    Offset = ParseAcpi (
               TRUE,
               4,
               Buffer,
               ArrayPtr,
               8,
               PARSER_PARAMS (InterruptArrayParser)
               );
    ArrayPtr += Offset;
    Index++;
  }

  ArrayPtr = Ptr + *PmuInterruptOffset;
  Index = 0;
  while (Index < *PmuInterruptCount) {
    AsciiSPrint (
      Buffer,
      sizeof (Buffer),
      "PMU Interrupts Array [%d]",
      Index
      );
    Offset = ParseAcpi (
               TRUE,
               4,
               Buffer,
               ArrayPtr,
               8,
               PARSER_PARAMS (InterruptArrayParser)
               );
    ArrayPtr += Offset;
    Index++;
  }

  if (*IortIdMappingCount != 0) {
    DumpIortNodeIdMappings (Ptr, MappingCount, MappingOffset);
  }
}

/** This function parses the IORT SMMUv3 node.

  @param [in] Ptr            Pointer to the start of the buffer.
  @param [in] Length         Length of the buffer.
  @param [in] MappingCount   The ID Mapping count.
  @param [in] MappingOffset  The offset of the ID Mapping array
                             from the start of the IORT table.
*/
STATIC
VOID
DumpIortNodeSmmuV3 (
  IN UINT8* Ptr,
  IN UINT16 Length,
  IN UINT32 MappingCount,
  IN UINT32 MappingOffset
  )
{
  ParseAcpi (
    TRUE,
    2,
    "SMMUV3 Node",
    Ptr,
    Length,
    PARSER_PARAMS (IortNodeSmmuV3Parser)
    );

  if (*IortIdMappingCount != 0) {
    DumpIortNodeIdMappings (Ptr, MappingCount, MappingOffset);
  }
}

/** This function parses the IORT ITS node.

  @param [in] Ptr            Pointer to the start of the buffer.
  @param [in] Length         Length of the buffer.
*/
STATIC
VOID
DumpIortNodeIts (
  IN UINT8* Ptr,
  IN UINT16 Length
  )
{
  UINT32 Offset;
  UINT32 Index;
  UINT8* ItsIdPtr;
  CHAR8  Buffer[80];  // Used for AsciiName param of ParseAcpi

  Offset = ParseAcpi (
             TRUE,
             2,
             "ITS Node",
             Ptr,
             Length,
             PARSER_PARAMS (IortNodeItsParser)
             );

  ItsIdPtr = Ptr + Offset;
  Index = 0;
  while (Index < *ItsCount) {
    AsciiSPrint (
      Buffer,
      sizeof (Buffer),
      "GIC ITS Identifier Array [%d]",
      Index
      );
    Offset = ParseAcpi (
               TRUE,
               4,
               Buffer,
               ItsIdPtr,
               4,
               PARSER_PARAMS (ItsIdParser)
               );
    ItsIdPtr += Offset;
    Index++;
  }

  // Note: ITS does not have the ID Mappings Array
}

/** This function parses the IORT Named Component node.

  @param [in] Ptr            Pointer to the start of the buffer.
  @param [in] Length         Length of the buffer.
  @param [in] MappingCount   The ID Mapping count.
  @param [in] MappingOffset  The offset of the ID Mapping array
                             from the start of the IORT table.
*/
STATIC
VOID
DumpIortNodeNamedComponent (
  IN UINT8* Ptr,
  IN UINT16 Length,
  IN UINT32 MappingCount,
  IN UINT32 MappingOffset
  )
{
  UINT32 Offset;
  UINT32 Index;
  UINT8* DeviceNamePtr;
  UINT32 DeviceNameLength;

  Offset = ParseAcpi (
             TRUE,
             2,
             "Named Component Node",
             Ptr,
             Length,
             PARSER_PARAMS (IortNodeNamedComponentParser)
             );

  DeviceNamePtr = Ptr + Offset;
  // Estimate the Device Name length
  DeviceNameLength = Length - Offset - (MappingCount * 20);
  PrintFieldName (2, L"Device Object Name");
  Index = 0;
  while ((Index < DeviceNameLength) && (DeviceNamePtr[Index] != 0)) {
    Print (L"%c", DeviceNamePtr[Index++]);
  }
  Print (L"\n");

  if (*IortIdMappingCount != 0) {
    DumpIortNodeIdMappings (Ptr, MappingCount, MappingOffset);
  }
}

/** This function parses the IORT Root Complex node.

  @param [in] Ptr            Pointer to the start of the buffer.
  @param [in] Length         Length of the buffer.
  @param [in] MappingCount   The ID Mapping count.
  @param [in] MappingOffset  The offset of the ID Mapping array
                             from the start of the IORT table.
*/
STATIC
VOID
DumpIortNodeRootComplex (
  IN UINT8* Ptr,
  IN UINT16 Length,
  IN UINT32 MappingCount,
  IN UINT32 MappingOffset
  )
{
  ParseAcpi (
    TRUE,
    2,
    "Root Complex Node",
    Ptr,
    Length,
    PARSER_PARAMS (IortNodeRootComplexParser)
    );

  if (*IortIdMappingCount != 0) {
    DumpIortNodeIdMappings (Ptr, MappingCount, MappingOffset);
  }
}

/** This function parses the IORT PMCG node.

  @param [in] Ptr            Pointer to the start of the buffer.
  @param [in] Length         Length of the buffer.
  @param [in] MappingCount   The ID Mapping count.
  @param [in] MappingOffset  The offset of the ID Mapping array
                             from the start of the IORT table.
*/
STATIC
VOID
DumpIortNodePmcg (
  IN UINT8* Ptr,
  IN UINT16 Length,
  IN UINT32 MappingCount,
  IN UINT32 MappingOffset
)
{
  ParseAcpi (
    TRUE,
    2,
    "PMCG Node",
    Ptr,
    Length,
    PARSER_PARAMS (IortNodePmcgParser)
  );

  if (*IortIdMappingCount != 0) {
    DumpIortNodeIdMappings (Ptr, MappingCount, MappingOffset);
  }

  if (*IortIdMappingCount > 1) {
    IncrementErrorCount ();
    Print (
      L"ERROR: ID mapping must not be greater than 1. Id Mapping Count =%d\n",
      *IortIdMappingCount
      );
  }
}

/** This function parses the ACPI IORT table.
  When trace is enabled this function parses the IORT table and
  traces the ACPI fields.

  This function also parses the following nodes:
    - ITS Group
    - Named Component
    - Root Complex
    - SMMUv1/2
    - SMMUv3
    - PMCG

  This function also performs validation of the ACPI table fields.

  @param [in] Trace              If TRUE, trace the ACPI fields.
  @param [in] Ptr                Pointer to the start of the buffer.
  @param [in] AcpiTableLength    Length of the ACPI table.
  @param [in] AcpiTableRevision  Revision of the ACPI table.
*/
VOID
EFIAPI
ParseAcpiIort (
  IN BOOLEAN Trace,
  IN UINT8*  Ptr,
  IN UINT32  AcpiTableLength,
  IN UINT8   AcpiTableRevision
  )
{
  UINT32 Offset;
  UINT32 Index;
  UINT8* NodePtr;

  if (!Trace) {
    return;
  }

  ParseAcpi (
    TRUE,
    0,
    "IORT",
    Ptr,
    AcpiTableLength,
    PARSER_PARAMS (IortParser)
    );
  Offset = *IortNodeOffset;
  NodePtr = Ptr + Offset;
  Index = 0;

  while ((Index < *IortNodeCount) && (Offset < AcpiTableLength)) {
    // Parse the IORT Node Header
    ParseAcpi (
      FALSE,
      0,
      "IORT Node Header",
      NodePtr,
      16,
      PARSER_PARAMS (IortNodeHeaderParser)
      );
    if (*IortNodeLength == 0) {
      IncrementErrorCount ();
      Print (L"ERROR: Parser error. Invalid table data.\n");
      return;
    }

    PrintFieldName (2, L"* Node Offset *");
    Print (L"0x%x\n", Offset);

    switch (*IortNodeType) {
      case EIORT_NODE_ITS_GROUP:
        DumpIortNodeIts (
          NodePtr,
          *IortNodeLength
          );
        break;
      case EIORT_NODE_NAMED_COMPONENT:
        DumpIortNodeNamedComponent (
          NodePtr,
          *IortNodeLength,
          *IortIdMappingCount,
          *IortIdMappingOffset
          );
        break;
      case EIORT_NODE_ROOT_COMPLEX:
        DumpIortNodeRootComplex (
          NodePtr,
          *IortNodeLength,
          *IortIdMappingCount,
          *IortIdMappingOffset
          );
        break;
      case EIORT_NODE_SMMUV1_V2:
        DumpIortNodeSmmuV1V2 (
          NodePtr,
          *IortNodeLength,
          *IortIdMappingCount,
          *IortIdMappingOffset
          );
        break;
      case EIORT_NODE_SMMUV3:
        DumpIortNodeSmmuV3 (
          NodePtr,
          *IortNodeLength,
          *IortIdMappingCount,
          *IortIdMappingOffset
          );
        break;
      case EIORT_NODE_PMCG:
        DumpIortNodePmcg (
          NodePtr,
          *IortNodeLength,
          *IortIdMappingCount,
          *IortIdMappingOffset
        );
        break;

      default:
        IncrementErrorCount ();
        Print (L"ERROR: Unsupported IORT Node type = %d\n", *IortNodeType);
    } // switch

    NodePtr += (*IortNodeLength);
    Offset += (*IortNodeLength);
  } // while
}

/** Register the parser.

  @param [in]  ImageHandle  The handle to the image.
  @param [in]  SystemTable  Pointer to the System Table.

  @retval EFI_SUCCESS           The parser is registered.
  @retval EFI_ALREADY_STARTED   The parser for the ACPI Table
                                was already registered.
  @retval EFI_INVALID_PARAMETER A parameter is invalid
  @retval EFI_OUT_OF_RESOURCES  No space to register the
                                parser.
*/
EFI_STATUS
EFIAPI
IortParserLibConstructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  return RegisterParser (
           EFI_ACPI_6_2_IO_REMAPPING_TABLE_SIGNATURE,
           ParseAcpiIort
           );
}

/** Deregister the parser.

  @param [in]  ImageHandle  The handle to the image.
  @param [in]  SystemTable  Pointer to the System Table.

  @retval EFI_SUCCESS           The parser is deregistered.
  @retval EFI_NOT_FOUND         The parser was not registered.
*/
EFI_STATUS
EFIAPI
IortParserLibDestructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  return DeregisterParser (
           EFI_ACPI_6_2_IO_REMAPPING_TABLE_SIGNATURE
           );
}
