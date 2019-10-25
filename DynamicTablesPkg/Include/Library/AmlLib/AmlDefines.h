/** @file
  Aml Defines.

  Copyright (c) 2010 - 2018, Intel Corporation. All rights reserved. <BR>
  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef AML_DEFINES_H_
#define AML_DEFINES_H_

/** AML tree node types.

  Data nodes are tagged whith the data type they contains.
  Some data types cannot be used for data nodes (None, Object).
  AML_UINTx types are converted to the EFI_ACPI_NODE_TYPE_UINT type.
  These types are accessible externally.
*/
typedef UINT8 EFI_ACPI_NODE_TYPE;
#define EFI_ACPI_NODE_TYPE_NONE           0   // AML_NONE,   not accessible.
#define EFI_ACPI_NODE_TYPE_RESERVED1      1   // AML_UINT8,  converted.
#define EFI_ACPI_NODE_TYPE_RESERVED2      2   // AML_UINT16, converted.
#define EFI_ACPI_NODE_TYPE_RESERVED3      3   // AML_UINT32, converted.
#define EFI_ACPI_NODE_TYPE_RESERVED4      4   // AML_UINT64, converted.
#define EFI_ACPI_NODE_TYPE_NAME_STRING    5   // AML_NAME,   AML NameString.
#define EFI_ACPI_NODE_TYPE_STRING         6   // AML_STRING, NULL terminated
                                              // string.
#define EFI_ACPI_NODE_TYPE_CHILD          7   // AML_OBJECT, Not accessible.
#define EFI_ACPI_NODE_TYPE_UINT           8   // Integer data, AML_UINTx are
                                              // converted to this type.
#define EFI_ACPI_NODE_TYPE_RAW            9   // Raw bytes contained in a
                                              // buffer.
#define EFI_ACPI_NODE_TYPE_RESOURCE_DATA  10  // Resource data element.
#define EFI_ACPI_NODE_TYPE_FIELD_ELEMENT  11  // Field data element.

/** Indexes of fixed arguments.
*/
typedef UINT8 AML_OP_PARSE_INDEX;
#define AML_OP_PARSE_INDEX_GET_TERM1   0   // First fixed argument index.
#define AML_OP_PARSE_INDEX_GET_TERM2   1   // Second fixed argument index.
#define AML_OP_PARSE_INDEX_GET_TERM3   2   // Third fixed argument index.
#define AML_OP_PARSE_INDEX_GET_TERM4   3   // Fourth fixed argument index.
#define AML_OP_PARSE_INDEX_GET_TERM5   4   // Fifth fixed argument index.
#define AML_OP_PARSE_INDEX_GET_TERM6   5   // Sixth fixed argument index.
#define AML_OP_PARSE_INDEX_MAX         6   // Maximum fixed argument index.
#define AML_OP_PARSE_INDEX_GET_SIZE    MAX_UINT8  // Index analog to a command.
                                                  // Should only be used
                                                  // internally.

/** Node types.
*/
typedef enum EAmlNodeType {
  EAmlNodeUnknown,  ///< Unknown/Invalid Aml Node Type
  EAmlNodeRoot,     ///< Aml Root Node, typically represents a DefinitionBlock
  EAmlNodeObject,   ///< Aml Object Node, typically represents an ASL statement
                    ///  or its arguments
  EAmlNodeData,     ///< Aml Data Node, typically represents arguments for an
                    ///  ASL statement
  EAmlNodeMax
} EAML_NODE_TYPE;

#endif // AML_DEFINES_H_
