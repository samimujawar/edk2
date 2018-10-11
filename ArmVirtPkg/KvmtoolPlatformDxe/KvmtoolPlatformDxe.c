/** @file

  The KvmtoolPlatformDxe performs the platform specific initialization like:
  - It parses the kvmtool DT for Non-Volatile memory range to use for runtime
    variable storage and initialises the PcdEmuVariableNvStoreReserved.
  - It decides if the firmware should expose ACPI or Device Tree-based
    hardware description to the operating system.

  Copyright (c) 2018, ARM Limited. All rights reserved.

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Protocol/FdtClient.h>

/** Parse the kvmtool DT for Non-Volatile Memory range and initialize
    PcdEmuVariableNvStoreReserved.

  @retval EFI_SUCCESS           Success.
  @retval EFI_ACCESS_DENIED     Failed to update Pcd.
  @retval EFI_BUFFER_TOO_SMALL  Non-Volatile memory region less than storage
                                required for runtime variable storage.
**/
STATIC
EFI_STATUS
InitializeNvStorageBase (
  VOID
)
{
  EFI_STATUS                     Status;
  FDT_CLIENT_PROTOCOL          * FdtClient;
  INT32                          Node;
  CONST UINT64                 * Reg;
  UINT32                         Len;
  UINT64                         RegSize;
  UINT64                         RegBase;
  RETURN_STATUS                  PcdStatus;

  Status = gBS->LocateProtocol (
                  &gFdtClientProtocolGuid,
                  NULL,
                  (VOID **)&FdtClient
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "Failed to locate Fdt Client Protocol. Status = %r\n",
      Status
      ));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  Status = FdtClient->FindNextCompatibleNode (
                        FdtClient,
                        "kvmtool,NVMem",
                        Node,
                        &Node
                        );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Cannot find NV memory DT node to use for Runtime variable storage."
      " Expected node in DT is \'compatible = \"kvmtool,NVMem\"\'."
      " Status = %r\n",
       __FUNCTION__,
       Status
       ));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  Status = FdtClient->GetNodeProperty (
                        FdtClient,
                        Node,
                        "reg",
                        (CONST VOID **)&Reg,
                        &Len
                        );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: GetNodeProperty () failed. Status = %r\n",
      __FUNCTION__,
      Status
      ));
      return Status;
  }

  if (Len != (2 * sizeof (UINT64))) {
    Status = EFI_INVALID_PARAMETER;
    DEBUG ((
      DEBUG_ERROR,
      "%a: Invalid DT Node data. Status = %r\n",
      __FUNCTION__,
      Status
      ));
      return Status;
  }

  RegBase = SwapBytes64 (((CONST UINT64 *)Reg)[0]);
  RegSize = SwapBytes64 (((CONST UINT64 *)Reg)[1]);
  DEBUG ((DEBUG_INFO, "RegBase = 0x%lx, RegSize = 0x%lx\n", RegBase, RegSize));

  if (RegSize < PcdGet32 (PcdVariableStoreSize)) {
    DEBUG ((
      DEBUG_ERROR,
      "Not enough NV memory available for Runtime variable storage\n"
      ));
    return EFI_BUFFER_TOO_SMALL;
  }

  PcdStatus = PcdSet64S (PcdEmuVariableNvStoreReserved, RegBase);
  if (RETURN_ERROR (PcdStatus)) {
    DEBUG ((
      DEBUG_ERROR,
      "Failed to update PcdEmuVariableNvStoreReserved. Status = %r\n",
      PcdStatus
      ));
    ASSERT_RETURN_ERROR (PcdStatus);
    return EFI_ACCESS_DENIED;
  }
  return EFI_SUCCESS;
}

/** Decide if the firmware should expose ACPI tables or Device Tree and
    install the appropriate protocol interface.

  @param [in]  ImageHandle  Handle for this image.

  @retval EFI_SUCCESS             Success.
  @retval EFI_OUT_OF_RESOURCES    There was not enough memory to install the
                                  protocols.
  @retval EFI_INVALID_PARAMETER   A parameter is invalid.

**/
STATIC
EFI_STATUS
PlatformHasAcpiDt (
  IN EFI_HANDLE           ImageHandle
  )
{
  if (!PcdGetBool (PcdForceNoAcpi)) {
    // Expose ACPI tables
    return gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEdkiiPlatformHasAcpiGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  }

  // Expose the Device Tree.
  return gBS->InstallProtocolInterface (
                &ImageHandle,
                &gEdkiiPlatformHasDeviceTreeGuid,
                EFI_NATIVE_INTERFACE,
                NULL
                );
}

/** Entry point for Kvmtool Platform Dxe

  @param [in]  ImageHandle  Handle for this image.
  @param [in]  SystemTable  Pointer to the EFI system table.

  @retval EFI_SUCCESS             Success.
  @retval EFI_OUT_OF_RESOURCES    There was not enough memory to install the
                                  protocols.
  @retval EFI_INVALID_PARAMETER   A parameter is invalid.

**/
EFI_STATUS
EFIAPI
KvmtoolPlatformDxeEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                     Status;

  Status = InitializeNvStorageBase ();
  if (EFI_ERROR (Status)) {
    goto Failed;
  }

  Status = PlatformHasAcpiDt (ImageHandle);
  if (EFI_ERROR (Status)) {
    goto Failed;
  }

  return Status;

Failed:
  ASSERT_EFI_ERROR (Status);
  CpuDeadLoop ();

  return Status;
}
