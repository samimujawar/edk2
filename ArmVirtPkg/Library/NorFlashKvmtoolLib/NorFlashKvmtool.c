/** @file
   An instance of the NorFlashPlatformLib for Kvmtool platform.

 Copyright (c) 2020, ARM Ltd. All rights reserved.<BR>

 SPDX-License-Identifier: BSD-2-Clause-Patent

 **/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/NorFlashPlatformLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>

/** Macro defining the maximum number of Flash Banks.
 */
#define MAX_FLASH_BANKS       4

STATIC NOR_FLASH_DESCRIPTION  mNorFlashDevices[MAX_FLASH_BANKS];
STATIC UINTN                  mNorFlashDeviceCount = 0;

/** This function performs platform specific actions to initialise
    the NOR flash, if required.

  @retval EFI_SUCCESS           Success.
**/
EFI_STATUS
NorFlashPlatformInitialization (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "NorFlashPlatformInitialization\n"));
  // Nothing to do here
  return EFI_SUCCESS;
}

/** Initialise Non volatile Flash storage variables.

  @param [in]  FlashDevice Pointer to the NOR Flash device.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_OUT_OF_RESOURCES  Insufficient flash storage space.
**/
EFI_STATUS
SetupVariableStore (
  IN NOR_FLASH_DESCRIPTION * FlashDevice
  )
{
  UINTN   FlashRegion;
  UINTN   FlashNvStorageVariableBase;
  UINTN   FlashNvStorageFtwWorkingBase;
  UINTN   FlashNvStorageFtwSpareBase;
  UINTN   FlashNvStorageVariableSize;
  UINTN   FlashNvStorageFtwWorkingSize;
  UINTN   FlashNvStorageFtwSpareSize;

  FlashNvStorageVariableSize = PcdGet32 (PcdFlashNvStorageVariableSize);
  FlashNvStorageFtwWorkingSize = PcdGet32 (PcdFlashNvStorageFtwWorkingSize);
  FlashNvStorageFtwSpareSize =  PcdGet32 (PcdFlashNvStorageFtwSpareSize);

  if ((FlashNvStorageVariableSize == 0)   ||
      (FlashNvStorageFtwWorkingSize == 0) ||
      (FlashNvStorageFtwSpareSize == 0)) {
    DEBUG ((DEBUG_ERROR, "FlashNvStorage size not defined\n"));
    return EFI_INVALID_PARAMETER;
  }

  // Setup the variable store
  FlashRegion = FlashDevice->DeviceBaseAddress;

  FlashNvStorageVariableBase = FlashRegion;
  FlashRegion += PcdGet32 (PcdFlashNvStorageVariableSize);

  FlashNvStorageFtwWorkingBase = FlashRegion;
  FlashRegion += PcdGet32 (PcdFlashNvStorageFtwWorkingSize);

  FlashNvStorageFtwSpareBase = FlashRegion;
  FlashRegion += PcdGet32 (PcdFlashNvStorageFtwSpareSize);

  if (FlashRegion > (FlashDevice->DeviceBaseAddress + FlashDevice->Size)) {
    DEBUG ((DEBUG_ERROR, "Insufficient flash storage size\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  PcdSet32S (
    PcdFlashNvStorageVariableBase,
    FlashNvStorageVariableBase
    );

  PcdSet32S (
    PcdFlashNvStorageFtwWorkingBase,
    FlashNvStorageFtwWorkingBase
    );

  PcdSet32S (
    PcdFlashNvStorageFtwSpareBase,
    FlashNvStorageFtwSpareBase
    );

  DEBUG ((
    DEBUG_INFO,
    "PcdFlashNvStorageVariableBase = 0x%x\n",
    FlashNvStorageVariableBase
    ));
  DEBUG ((
    DEBUG_INFO,
    "PcdFlashNvStorageVariableSize = 0x%x\n",
    FlashNvStorageVariableSize
    ));
  DEBUG ((
    DEBUG_INFO,
    "PcdFlashNvStorageFtwWorkingBase = 0x%x\n",
    FlashNvStorageFtwWorkingBase
    ));
  DEBUG ((
    DEBUG_INFO,
    "PcdFlashNvStorageFtwWorkingSize = 0x%x\n",
    FlashNvStorageFtwWorkingSize
    ));
  DEBUG ((
    DEBUG_INFO,
    "PcdFlashNvStorageFtwSpareBase = 0x%x\n",
    FlashNvStorageFtwSpareBase
    ));
  DEBUG ((
    DEBUG_INFO,
    "PcdFlashNvStorageFtwSpareSize = 0x%x\n",
    FlashNvStorageFtwSpareSize
    ));

  return EFI_SUCCESS;
}

/** Return the Flash devices on the platform.

  @param [out]  NorFlashDescriptions    Pointer to the Flash device description.
  @param [out]  Count                   Number of Flash devices.

  @retval EFI_SUCCESS           Success.
  @retval EFI_NOT_FOUND         Flash device not found.
**/
EFI_STATUS
NorFlashPlatformGetDevices (
  OUT NOR_FLASH_DESCRIPTION   **NorFlashDescriptions,
  OUT UINT32                  *Count
  )
{
  if (mNorFlashDeviceCount > 0) {
    *NorFlashDescriptions = mNorFlashDevices;
    *Count = mNorFlashDeviceCount;
    return EFI_SUCCESS;
  }
  return EFI_NOT_FOUND;
}

/** Entrypoint for NorFlashPlatformLib.

  @param [in]  ImageHandle  The handle to the image.
  @param [in]  SystemTable  Pointer to the System Table.

  @retval EFI_SUCCESS             Success.
  @retval EFI_INVALID_PARAMETER   A parameter is invalid.
  @retval EFI_NOT_FOUND           Flash device not found.
**/
EFI_STATUS
EFIAPI
NorFlashPlatformLibConstructor (
  IN  EFI_HANDLE          ImageHandle,
  IN  EFI_SYSTEM_TABLE  * SystemTable
  )
{
  FDT_CLIENT_PROTOCOL         *FdtClient;
  INT32                       Node;
  EFI_STATUS                  Status;
  EFI_STATUS                  FindNodeStatus;
  CONST UINT32                *Reg;
  UINT32                      PropSize;
  UINT64                      Base;
  UINT64                      Size;

  if (mNorFlashDeviceCount != 0) {
    return EFI_SUCCESS;
  }

  Status = gBS->LocateProtocol (
                  &gFdtClientProtocolGuid,
                  NULL,
                  (VOID **)&FdtClient
                  );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  for (FindNodeStatus = FdtClient->FindCompatibleNode (
                                     FdtClient,
                                     "cfi-flash",
                                     &Node
                                     );
       !EFI_ERROR (FindNodeStatus) && (mNorFlashDeviceCount < MAX_FLASH_BANKS);
       FindNodeStatus = FdtClient->FindNextCompatibleNode (
                                     FdtClient,
                                     "cfi-flash",
                                     Node,
                                     &Node
    )) {
    Status = FdtClient->GetNodeProperty (
                          FdtClient,
                          Node,
                          "reg",
                          (CONST VOID **)&Reg,
                          &PropSize
                          );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: GetNodeProperty () failed (Status == %r)\n",
        __FUNCTION__, Status));
      continue;
    }

    ASSERT ((PropSize % (4 * sizeof (UINT32))) == 0);

    while ((PropSize >= (4 * sizeof (UINT32))) &&
           (mNorFlashDeviceCount < MAX_FLASH_BANKS)) {
      Base = SwapBytes64 (ReadUnaligned64 ((VOID *)&Reg[0]));
      Size = SwapBytes64 (ReadUnaligned64 ((VOID *)&Reg[2]));
      Reg += 4;

      PropSize -= 4 * sizeof (UINT32);

      //
      // Disregard any flash devices that overlap with the primary FV.
      // The firmware is not updatable from inside the guest anyway.
      //
      if ((PcdGet64 (PcdFvBaseAddress) + PcdGet32 (PcdFvSize) > Base) &&
          (Base + Size) > PcdGet64 (PcdFvBaseAddress)) {
        continue;
      }

      DEBUG ((
        DEBUG_INFO,
        "NOR%d : Base = 0x%lx, Size = 0x%lx\n",
        mNorFlashDeviceCount,
        Base,
        Size
        ));

      mNorFlashDevices[mNorFlashDeviceCount].DeviceBaseAddress = (UINTN)Base;
      mNorFlashDevices[mNorFlashDeviceCount].RegionBaseAddress = (UINTN)Base;
      mNorFlashDevices[mNorFlashDeviceCount].Size              = (UINTN)Size;
      mNorFlashDevices[mNorFlashDeviceCount].BlockSize         = SIZE_256KB;
      mNorFlashDeviceCount++;
    }
  }

  // Setup the variable store in the last bank
  if ((mNorFlashDeviceCount > 0) &&
      (mNorFlashDevices[mNorFlashDeviceCount - 1].DeviceBaseAddress != 0)) {
    return SetupVariableStore (&mNorFlashDevices[mNorFlashDeviceCount - 1]);
  }

  return EFI_NOT_FOUND;
}

