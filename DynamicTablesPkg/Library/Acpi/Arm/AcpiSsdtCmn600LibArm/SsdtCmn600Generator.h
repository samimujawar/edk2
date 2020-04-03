/** @file

  Copyright (c) 2020, ARM Limited. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
    - Cm or CM   - Configuration Manager
    - Obj or OBJ - Object
    - Std or STD - Standard

  @par Reference(s):
  @par Reference(s):
  - Arm CoreLink CMN-600 Coherent Mesh Network Technical Reference Manual r3p0
  - Generic ACPI for Arm Components 1.0 Platform Design Document
**/

#ifndef SSDT_CMN600_GENERATOR_H_
#define SSDT_CMN600_GENERATOR_H_

/** PeriphBase maximum address length is 256MB
    for a (X >= 4) || (Y >= 4) dimensions mesh.
*/
#define PERIPHBASE_MAX_ADDRESS_LENGTH   0x10000000ULL

/** PeriphBase minimum address length is 64MB
    for a (X < 4) && (Y < 4) dimensions mesh.
*/
#define PERIPHBASE_MIN_ADDRESS_LENGTH   0x04000000ULL

/** RootNodeBase address length is 16KB.
*/
#define ROOTNODEBASE_ADDRESS_LENGTH     0x00004000ULL

/** Maximum number of CMN-600 Debug and Trace Logic Controllers (DTC).
*/
#define MAX_DTC_COUNT                   4

#endif // SSDT_CMN600_GENERATOR_H_
