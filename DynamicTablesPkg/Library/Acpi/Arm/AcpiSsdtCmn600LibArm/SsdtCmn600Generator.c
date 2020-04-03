/** @file
  SSDT CMN-600 AML Table Generator.

  Copyright (c) 2020, ARM Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Reference(s):
  - Arm CoreLink CMN-600 Coherent Mesh Network Technical Reference Manual r3p0
  - Generic ACPI for Arm Components 1.0 Platform Design Document
**/

#include <IndustryStandard/DebugPort2Table.h>
#include <Library/AcpiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/AcpiTable.h>

// Module specific include files.
#include <AcpiTableGenerator.h>
#include <ConfigurationManagerObject.h>
#include <ConfigurationManagerHelper.h>
#include <Library/AmlLib/AmlLib.h>
#include <Library/TableHelperLib.h>
#include <Protocol/ConfigurationManagerProtocol.h>
#include "SsdtCmn600Generator.h"

/** Include files generated by the AmlToHex.py script.
    Contain AML bytecode.
*/
#include <SsdtCmn600Template.hex>

/** SSDT CMN-600 Table Generator.

  Requirements:
  The following Configuration Manager Object(s) are required by
  this Generator:
  - EArmObjCmn600Info
  - EArmObjExtendedInterruptInfo
*/

/** This macro expands to a function that retrieves the CMN-600
    Information from the Configuration Manager.
*/
GET_OBJECT_LIST (
  EObjNameSpaceArm,
  EArmObjCmn600Info,
  CM_ARM_CMN_600_INFO
  );

/** This macro expands to a function that retrieves the Generic Interrupts
    Information of the DTC of the CMN-600 from the Configuration Manager.
*/
GET_OBJECT_LIST (
  EObjNameSpaceArm,
  EArmObjExtendedInterruptInfo,
  CM_ARM_EXTENDED_INTERRUPT
  );

/** Check the CMN-600 Information.

  @param [in]  Cmn600Info                 CMN-600 information structure.
  @param [in]  DtcGenericInterrupt        Array of DTC Generic Interrupt
                                          structures of the CMN-600.
  @param [in]  DtcGenericInterruptCount   Count of DTC Generic Interrupts.

  @retval  EFI_SUCCESS            The function completed successfully.
  @retval  EFI_INVALID_PARAMETER  Invalid parameter.
**/
STATIC
EFI_STATUS
EFIAPI
ValidateCmn600Info (
  IN  CM_ARM_CMN_600_INFO       * Cmn600Info,
  IN  CM_ARM_EXTENDED_INTERRUPT * DtcGenericInterrupt,
  IN  UINT32                      DtcGenericInterruptCount
  )
{
  UINT32      Index;

  if ((Cmn600Info == NULL)            ||
      (DtcGenericInterrupt == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // At least one DTC is required.
  if ((DtcGenericInterruptCount == 0) ||
      (DtcGenericInterruptCount > MAX_DTC_COUNT)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: Invalid DTC configuration:\n"
      ));
    ASSERT (0);
    goto error_handler;
  }

  // Check the count of DTC Generic Interrupt is matching.
  if (Cmn600Info->DtcCount > DtcGenericInterruptCount) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: Insufficient DTC interrupts:"
      " expected count %u and available %u\n",
      Cmn600Info->DtcCount,
      DtcGenericInterruptCount
      ));
    ASSERT (0);
    goto error_handler;
  }

  // Check PERIPHBASE and ROOTNODEBASE address spaces are initialized.
  if ((Cmn600Info->PeriphBaseAddress == 0)    ||
      (Cmn600Info->RootNodeBaseAddress == 0)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: Invalid PERIPHBASE or ROOTNODEBASE.\n"
      ));
    ASSERT (0);
    goto error_handler;
  }

  // The PERIPHBASE address must be 64MB aligned for a (X < 4) && (Y < 4)
  // dimension mesh, and 256MB aligned otherwise.
  // Check it is a least 64MB aligned.
  if (Cmn600Info->PeriphBaseAddress & (PERIPHBASE_MIN_ADDRESS_LENGTH - 1)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: PERIPHBASE address must be 64MB aligned.\n"
      ));
    ASSERT (0);
    goto error_handler;
  }

  // The PERIPHBASE address is at most 64MB for a (X < 4) && (Y < 4)
  // dimension mesh, and 256MB otherwise. Check it is not more than 256MB.
  if (Cmn600Info->PeriphBaseAddressLength > PERIPHBASE_MAX_ADDRESS_LENGTH) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: PERIPHBASE address range must lower than 256MB.\n"
      ));
    ASSERT (0);
    goto error_handler;
  }

  // Check the 16 KB alignment of the ROOTNODEBASE address.
  if (Cmn600Info->PeriphBaseAddress & (ROOTNODEBASE_ADDRESS_LENGTH - 1)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: Root base address must be 16KB aligned.\n"
      ));
    ASSERT (0);
    goto error_handler;
  }

  // The ROOTNODEBASE address space should be included in the PERIPHBASE
  // address space.
  if ((Cmn600Info->PeriphBaseAddress > Cmn600Info->RootNodeBaseAddress)  ||
      ((Cmn600Info->PeriphBaseAddress + Cmn600Info->PeriphBaseAddressLength) <
       (Cmn600Info->RootNodeBaseAddress + ROOTNODEBASE_ADDRESS_LENGTH))) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: "
      "ROOTNODEBASE address space not included in PERIPHBASE address space.\n"
      ));
    ASSERT (0);
    goto error_handler;
  }

  return EFI_SUCCESS;

error_handler:

  DEBUG ((
    DEBUG_ERROR,
    "PeriphBaseAddress = 0x%llx\n"
    "PeriphBaseAddressLength = 0x%llx\n"
    "RootNodeBaseAddress = 0x%llx\n"
    "DtcCount = 0x%lx\n",
    Cmn600Info->PeriphBaseAddress,
    Cmn600Info->PeriphBaseAddressLength,
    Cmn600Info->RootNodeBaseAddress,
    Cmn600Info->DtcCount
    ));

  DEBUG ((
    DEBUG_ERROR,
    "DtcGenericInterrupt(s): Count = %u\n",
    DtcGenericInterruptCount
    ));
  for (Index = 0; Index < DtcGenericInterruptCount; Index++) {
    DEBUG ((
      DEBUG_ERROR,
      "  [%d]:\n",
      Index
      ));
    DEBUG ((
      DEBUG_ERROR,
      "    Interrupt = 0x%lx\n",
      DtcGenericInterrupt[Index].Interrupt
      ));
    DEBUG ((
      DEBUG_ERROR,
      "    Flags = 0x%lx\n",
      DtcGenericInterrupt[Index].Flags
      ));
  }

  ASSERT (0);
  return EFI_INVALID_PARAMETER;
}

/** Fixup CMN-600 SSDT table.

  For each template value:
   - find the node to update;
   - update the value.

  @param  [in]  RootNodeHandle           Pointer to the root of the AML tree.
  @param  [in]  Cmn600Info               Pointer to a Cmn600 structure.
                                         Get the CMN-600 information from
                                         there.
  @param  [in]  DtcGenericInterrupt      Pointer to an array of Arm extended
                                         Interrupt structures.
  @param  [in]  DtcGenericInterruptCount Count of Arm extended Interrupt
                                         structures in the array.
  @param  [out] Table                    On return, hold the serialized
                                         definition block.

  @retval  EFI_SUCCESS            The function completed successfully.
  @retval  EFI_INVALID_PARAMETER  Invalid parameter.
  @retval  EFI_NOT_FOUND          Could not find information.
  @retval  EFI_OUT_OF_RESOURCES   Out of resources.
**/
STATIC
EFI_STATUS
EFIAPI
FixupCmn600Info (
  IN  AML_ROOT_NODE_HANDLE              RootNodeHandle,
  IN  CM_ARM_CMN_600_INFO             * Cmn600Info,
  IN  CM_ARM_EXTENDED_INTERRUPT       * DtcGenericInterrupt,
  IN  UINT32                            DtcGenericInterruptCount,
  OUT EFI_ACPI_DESCRIPTION_HEADER    ** Table
  )
{
  EFI_STATUS                Status;
  UINT8                     Index;

  AML_OBJECT_NODE_HANDLE    NameOpUidNode;

  AML_OBJECT_NODE_HANDLE    NameOpCrsNode;
  AML_DATA_NODE_HANDLE      CurrQWordRdNode;
  AML_DATA_NODE_HANDLE      NextQWordRdNode;
  AML_DATA_NODE_HANDLE      InterruptRdNode;

  // Validate the CMN-600 Info.
  Status = ValidateCmn600Info (
             Cmn600Info,
             DtcGenericInterrupt,
             DtcGenericInterruptCount
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Get the _UID object defined by the "Name ()" statement.
  Status = AmlFindNode (RootNodeHandle, "\\_SB.CMN6._UID", &NameOpUidNode);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Update the _UID value.
  Status = AmlNameOpUidUpdateValue (NameOpUidNode, 0);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Get the _CRS object defined by the "Name ()" statement.
  Status = AmlFindNode (
             RootNodeHandle,
             "\\_SB.CMN6._CRS",
             &NameOpCrsNode
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Get the first Rd node in the "_CRS" object.
  Status = AmlNameOpCrsGetFirstRdNode (NameOpCrsNode, &CurrQWordRdNode);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Update the PERIPHBASE base address and length.
  Status = AmlUpdateRdQWord (
             CurrQWordRdNode,
             Cmn600Info->PeriphBaseAddress,
             Cmn600Info->PeriphBaseAddressLength
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Get the QWord node corresponding to the ROOTNODEBASE.
  // It is the second Resource Data element in the BufferNode's
  // variable list of arguments.
  Status = AmlNameOpCrsGetNextRdNode (CurrQWordRdNode, &NextQWordRdNode);
  if (EFI_ERROR (Status)  ||
      (NextQWordRdNode == NULL)) {
    ASSERT (0);
    return Status;
  }

  CurrQWordRdNode = NextQWordRdNode;

  // Update the base address and length.
  Status = AmlUpdateRdQWord (
             CurrQWordRdNode,
             Cmn600Info->RootNodeBaseAddress,
             ROOTNODEBASE_ADDRESS_LENGTH
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Get the Interrupt node.
  // It is the second Resource Data element in the BufferNode's
  // variable list of arguments.
  Status = AmlNameOpCrsGetNextRdNode (CurrQWordRdNode, &InterruptRdNode);
  if (EFI_ERROR (Status)  ||
      (InterruptRdNode == NULL)) {
    ASSERT (0);
    return Status;
  }

  // Update the interrupt number.
  Status = AmlUpdateRdInterrupt (
             InterruptRdNode,
             DtcGenericInterrupt[0].Interrupt
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // If there are more than 1 interrupt declaration,
  // generate a new Resource Data node corresponding to the "Interrupt ()"
  // ASL function and add it at the last position in the list of
  // Resource Data nodes.
  for (Index = 1; Index < DtcGenericInterruptCount; Index++) {
    Status = AmlNameOpCrsAddRdInterrupt (
               NameOpCrsNode,
               TRUE,                                    // ResourceConsumer
               FALSE,                                   // EdgeTriggered
               FALSE,                                   // ActiveLow
               FALSE,                                   // Shared
               &DtcGenericInterrupt[Index].Interrupt,   // Irq
               1
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return Status;
    }
  }

  Status = AmlSerializeDefinitionBlock (
             RootNodeHandle,
             Table
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: Failed to Serialize SSDT Table Data."
      " Status = %r\n",
      Status
      ));
    ASSERT (0);
  }

  return Status;
}

/** Free any resources allocated for constructing a SSDT table for the CMN-600.

  @param [in]      This           Pointer to the table generator.
  @param [in]      AcpiTableInfo  Pointer to the ACPI Table Info.
  @param [in]      CfgMgrProtocol Pointer to the Configuration Manager
                                  Protocol Interface.
  @param [in, out] Table          Pointer to the ACPI Table.

  @retval EFI_SUCCESS           The resources were freed successfully.
  @retval EFI_INVALID_PARAMETER The table pointer is NULL or invalid.
**/
STATIC
EFI_STATUS
FreeSsdtCmn600TableResources (
  IN      CONST ACPI_TABLE_GENERATOR                  * CONST This,
  IN      CONST CM_STD_OBJ_ACPI_TABLE_INFO            * CONST AcpiTableInfo,
  IN      CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST CfgMgrProtocol,
  IN OUT        EFI_ACPI_DESCRIPTION_HEADER          ** CONST Table
)
{
  ASSERT (This != NULL);
  ASSERT (AcpiTableInfo != NULL);
  ASSERT (CfgMgrProtocol != NULL);
  ASSERT (AcpiTableInfo->TableGeneratorId == This->GeneratorID);
  ASSERT (AcpiTableInfo->AcpiTableSignature == This->AcpiTableSignature);

  if ((Table == NULL) || (*Table == NULL)) {
    DEBUG ((DEBUG_ERROR, "ERROR: SSDT-CMN-600: Invalid Table Pointer\n"));
    ASSERT ((Table != NULL) && (*Table != NULL));
    return EFI_INVALID_PARAMETER;
  }

  // Free the table list.
  FreePool (*Table);
  *Table = NULL;
  return EFI_SUCCESS;
}

/** Construct a SSDT table for a CMN-600 platform.

  Called by the Dynamic Table Manager, this function invokes the
  Configuration Manager protocol interface to get the required hardware
  information for generating the ACPI table.

  If this function allocates any resources then they must be freed
  in the FreeXXXXTableResources function.

  @param [in]  This           Pointer to the table generator.
  @param [in]  AcpiTableInfo  Pointer to the ACPI Table Info.
  @param [in]  CfgMgrProtocol Pointer to the Configuration Manager
                              Protocol Interface.
  @param [out] Table          Pointer to the constructed ACPI Table.

  @retval EFI_SUCCESS           Table generated successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object was not found.
  @retval EFI_OUT_OF_RESOURCES  Memory allocation failed.
**/
STATIC
EFI_STATUS
EFIAPI
BuildSsdtCmn600Table (
  IN  CONST ACPI_TABLE_GENERATOR                  * CONST This,
  IN  CONST CM_STD_OBJ_ACPI_TABLE_INFO            * CONST AcpiTableInfo,
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST CfgMgrProtocol,
  OUT       EFI_ACPI_DESCRIPTION_HEADER          ** CONST Table
  )
{
  EFI_STATUS                        Status;
  EFI_STATUS                        Status1;

  CM_ARM_CMN_600_INFO             * Cmn600Info;
  CM_ARM_EXTENDED_INTERRUPT       * DtcGenericInterrupt;
  UINT32                            DtcGenericInterruptCount;

  EFI_ACPI_DESCRIPTION_HEADER     * SsdtCmn600Template;
  AML_ROOT_NODE_HANDLE              RootNodeHandle;

  // Get information about the CMN-600.
  Cmn600Info = NULL;
  Status = GetEArmObjCmn600Info (
             CfgMgrProtocol,
             CM_NULL_TOKEN,
             &Cmn600Info,
             NULL
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: Failed to get the CMN-600 information."
      " Status = %r\n",
      Status
      ));
  }

  // Get information about the Generic Interrupts of the DTC of the CMN-600.
  // There must be at least one DTC.
  Status = GetEArmObjExtendedInterruptInfo (
             CfgMgrProtocol,
             (CONST CM_OBJECT_TOKEN)Cmn600Info->DtcInterruptListToken,
             &DtcGenericInterrupt,
             &DtcGenericInterruptCount
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: Failed to get the array of DTC Generic Interrupt"
      " Information of the CMN-600."
      " Status = %r\n",
      Status
      ));
  }

  // Parse the Ssdt CMN-600 Template.
  SsdtCmn600Template = (EFI_ACPI_DESCRIPTION_HEADER*)
                          ssdtcmn600template_aml_code;

  RootNodeHandle = NULL;
  Status = AmlParseDefinitionBlock (
             SsdtCmn600Template,
             &RootNodeHandle
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: Failed to parse SSDT CMN-600 Template."
      " Status = %r\n",
      Status
      ));
    ASSERT (0);
    goto error_handler;
  }

  Status = FixupCmn600Info (
             RootNodeHandle,
             Cmn600Info,
             DtcGenericInterrupt,
             DtcGenericInterruptCount,
             Table
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SSDT-CMN-600: Failed to add SSDT CMN-600 Table."
      " Status = %r\n",
      Status
      ));
    ASSERT (0);
    goto error_handler;
  }

  goto exit_handler;

error_handler:
  // Free up the allocated resources in case of an error.
  FreeSsdtCmn600TableResources (
    This,
    AcpiTableInfo,
    CfgMgrProtocol,
    Table
    );

exit_handler:
  if (RootNodeHandle != NULL) {
    Status1 = AmlDeleteTree (RootNodeHandle);
    if (EFI_ERROR (Status1)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: SSDT-CMN-600: Failed to add SSDT CMN-600 table."
        " Status = %r\n",
        Status1
        ));
      ASSERT (0);
      // If Status was success but we failed to delete the AML Tree
      // return Status1 else return the original error code i.e. Status
      if (!EFI_ERROR (Status)) {
        return Status1;
      }
    }
  }

  return Status;
}

/** This macro defines the Raw Generator revision.
*/
#define SSDT_CMN_600_GENERATOR_REVISION CREATE_REVISION (1, 0)

/** The interface for the Raw Table Generator.
*/
STATIC
CONST
ACPI_TABLE_GENERATOR SsdtCmn600Generator = {
  // Generator ID
  CREATE_STD_ACPI_TABLE_GEN_ID (EStdAcpiTableIdSsdtCmn600),
  // Generator Description
  L"ACPI.STD.SSDT.CMN600.GENERATOR",
  // ACPI Table Signature
  EFI_ACPI_6_3_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE,
  // ACPI Table Revision - Unused
  0,
  // Minimum ACPI Table Revision - Unused
  0,
  // Creator ID
  TABLE_GENERATOR_CREATOR_ID_ARM,
  // Creator Revision
  SSDT_CMN_600_GENERATOR_REVISION,
  // Build Table function
  BuildSsdtCmn600Table,
  // Free Resource function
  FreeSsdtCmn600TableResources,
  // Extended build function not needed.
  NULL,
  // Extended build function not implemented by the generator.
  // Hence extended free resource function is not required.
  NULL
};

/** Register the Generator with the ACPI Table Factory.

  @param [in]  ImageHandle  The handle to the image.
  @param [in]  SystemTable  Pointer to the System Table.

  @retval EFI_SUCCESS           The Generator is registered.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_ALREADY_STARTED   The Generator for the Table ID
                                is already registered.
**/
EFI_STATUS
EFIAPI
AcpiSsdtCmn600LibConstructor (
  IN  EFI_HANDLE           ImageHandle,
  IN  EFI_SYSTEM_TABLE  *  SystemTable
  )
{
  EFI_STATUS  Status;

  Status = RegisterAcpiTableGenerator (&SsdtCmn600Generator);
  DEBUG ((
    DEBUG_INFO,
    "SSDT-CMN-600: Register Generator. Status = %r\n",
    Status
    ));
  ASSERT_EFI_ERROR (Status);
  return Status;
}

/** Deregister the Generator from the ACPI Table Factory.

  @param [in]  ImageHandle  The handle to the image.
  @param [in]  SystemTable  Pointer to the System Table.

  @retval EFI_SUCCESS           The Generator is deregistered.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The Generator is not registered.
**/
EFI_STATUS
EFIAPI
AcpiSsdtCmn600LibDestructor (
  IN  EFI_HANDLE           ImageHandle,
  IN  EFI_SYSTEM_TABLE  *  SystemTable
  )
{
  EFI_STATUS  Status;

  Status = DeregisterAcpiTableGenerator (&SsdtCmn600Generator);

  DEBUG ((
    DEBUG_INFO,
    "SSDT-CMN-600: Deregister Generator. Status = %r\n",
    Status
    ));
  ASSERT_EFI_ERROR (Status);
  return Status;
}
