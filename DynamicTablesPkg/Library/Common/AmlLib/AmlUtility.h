/** @file
  Aml Utility.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef AML_UTILITY_H_
#define AML_UTILITY_H_

/** This function computes and updates the ACPI table checksum.

  @param  [in, out] AcpiTable  Pointer to an Acpi table.

  @retval EFI_SUCCESS   The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AcpiPlatformChecksum (
  IN  OUT  EFI_ACPI_DESCRIPTION_HEADER  * AcpiTable
  );

/** Compute the size of a tree/sub-tree.

  @param  [in]      Node      Node to compute the size.
  @param  [in, out] Context   Pointer holding the computed size.
                              (UINT32 *) Context.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlComputeSize (
  IN      CONST AML_NODE_HEADER   * Node,
  IN  OUT       UINT32            * Size
  );

/** Propogate information up the tree.

  The information can be a new size, a new number of arguments.

  @param  [in]  Node        Pointer to a node.
                            Must be a root node or an object node.
  @param  [in]  Operation   Choose the operation to do:
                            * TRUE:   Increment the Node's size and
                                      the Node's count
                            * FALSE:  Decrement the Node's size and
                                      the Node's count
  @param  [in]  Diff        Value to add/substract to the Node's size.
  @param  [in]  NodeCount   Number of nodes added/removed.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlPropagateInformation (
  IN  AML_NODE_HEADER   * Node,
  IN  BOOLEAN             Operation,
  IN  UINT32              Size,
  IN  UINT8               NodeCount
  );

#endif // AML_UTILITY_H_

