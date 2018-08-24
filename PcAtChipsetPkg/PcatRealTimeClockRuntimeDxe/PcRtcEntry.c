/** @file
  Provides Set/Get time operations.

Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2018, ARM Limited. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/DxeServicesTableLib.h>
#include "PcRtc.h"

PC_RTC_MODULE_GLOBALS  mModuleGlobal;

EFI_HANDLE             mHandle = NULL;

STATIC EFI_EVENT       mVirtualAddrChangeEvent;

EFI_PHYSICAL_ADDRESS   mRtcRegisterBase;

/**
  Returns the current time and date information, and the time-keeping capabilities
  of the hardware platform.

  @param  Time          A pointer to storage to receive a snapshot of the current time.
  @param  Capabilities  An optional pointer to a buffer to receive the real time
                        clock device's capabilities.

  @retval EFI_SUCCESS            The operation completed successfully.
  @retval EFI_INVALID_PARAMETER  Time is NULL.
  @retval EFI_DEVICE_ERROR       The time could not be retrieved due to hardware error.

**/
EFI_STATUS
EFIAPI
PcRtcEfiGetTime (
  OUT EFI_TIME                *Time,
  OUT EFI_TIME_CAPABILITIES   *Capabilities  OPTIONAL
  )
{
  return PcRtcGetTime (Time, Capabilities, &mModuleGlobal);
}

/**
  Sets the current local time and date information.

  @param  Time                   A pointer to the current time.

  @retval EFI_SUCCESS            The operation completed successfully.
  @retval EFI_INVALID_PARAMETER  A time field is out of range.
  @retval EFI_DEVICE_ERROR       The time could not be set due due to hardware error.

**/
EFI_STATUS
EFIAPI
PcRtcEfiSetTime (
  IN EFI_TIME                *Time
  )
{
  return PcRtcSetTime (Time, &mModuleGlobal);
}

/**
  Returns the current wakeup alarm clock setting.

  @param  Enabled  Indicates if the alarm is currently enabled or disabled.
  @param  Pending  Indicates if the alarm signal is pending and requires acknowledgement.
  @param  Time     The current alarm setting.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Enabled is NULL.
  @retval EFI_INVALID_PARAMETER Pending is NULL.
  @retval EFI_INVALID_PARAMETER Time is NULL.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be retrieved due to a hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this platform.

**/
EFI_STATUS
EFIAPI
PcRtcEfiGetWakeupTime (
  OUT BOOLEAN     *Enabled,
  OUT BOOLEAN     *Pending,
  OUT EFI_TIME    *Time
  )
{
  return PcRtcGetWakeupTime (Enabled, Pending, Time, &mModuleGlobal);
}


/**
  Sets the system wakeup alarm clock time.

  @param  Enabled  Enable or disable the wakeup alarm.
  @param  Time     If Enable is TRUE, the time to set the wakeup alarm for.
                   If Enable is FALSE, then this parameter is optional, and may be NULL.

  @retval EFI_SUCCESS            If Enable is TRUE, then the wakeup alarm was enabled.
                                 If Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER  A time field is out of range.
  @retval EFI_DEVICE_ERROR       The wakeup time could not be set due to a hardware error.
  @retval EFI_UNSUPPORTED        A wakeup timer is not supported on this platform.

**/
EFI_STATUS
EFIAPI
PcRtcEfiSetWakeupTime (
  IN BOOLEAN      Enabled,
  IN EFI_TIME    *Time       OPTIONAL
  )
{
  return PcRtcSetWakeupTime (Enabled, Time, &mModuleGlobal);
}

/**
  Fixup internal data so that EFI can be called in virtual mode.
  Call the passed in Child Notify event and convert any pointers in
  lib to virtual mode.

  @param[in]    Event   The Event that is being processed
  @param[in]    Context Event Context
**/
VOID
EFIAPI
LibRtcVirtualNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  // Only needed if you are going to support the OS calling RTC functions in
  // virtual mode. You will need to call EfiConvertPointer (). To convert any
  // stored physical addresses to virtual address. After the OS transitions to
  // calling in virtual mode, all future runtime calls will be made in virtual
  // mode.
  EfiConvertPointer (0x0, (VOID**)&mRtcRegisterBase);
}

/**
  The user Entry Point for PcRTC module.

  This is the entrhy point for PcRTC module. It installs the UEFI runtime service
  including GetTime(),SetTime(),GetWakeupTime(),and SetWakeupTime().

  @param  ImageHandle    The firmware allocated handle for the EFI image.
  @param  SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS    The entry point is executed successfully.
  @retval Others         Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializePcRtc (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;
  EFI_PHYSICAL_ADDRESS   RtcPageBase;

  EfiInitializeLock (&mModuleGlobal.RtcLock, TPL_CALLBACK);
  mModuleGlobal.CenturyRtcAddress = GetCenturyRtcAddress ();

  if (FixedPcdGetBool (PcdRtcUseMmio)) {
    mRtcRegisterBase = PcdGet8 (PcdRtcIndexRegister);
    RtcPageBase = mRtcRegisterBase & ~(EFI_PAGE_SIZE - 1);

    // Declare the controller as EFI_MEMORY_RUNTIME
    Status = gDS->AddMemorySpace (
                    EfiGcdMemoryTypeMemoryMappedIo,
                    RtcPageBase,
                    EFI_PAGE_SIZE,
                    EFI_MEMORY_UC | EFI_MEMORY_RUNTIME
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR, "Failed to add memory space. Status = %r\n",
        Status
        ));
      return Status;
    }

    Status = gDS->AllocateMemorySpace (
                    EfiGcdAllocateAddress,
                    EfiGcdMemoryTypeMemoryMappedIo,
                    0,
                    EFI_PAGE_SIZE,
                    &RtcPageBase,
                    ImageHandle,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "Failed to allocate memory space. Status = %r\n",
        Status
        ));
      gDS->RemoveMemorySpace (
             RtcPageBase,
             EFI_PAGE_SIZE
             );
      return Status;
    }

    Status = gDS->SetMemorySpaceAttributes (
                    RtcPageBase,
                    EFI_PAGE_SIZE,
                    EFI_MEMORY_UC | EFI_MEMORY_RUNTIME
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "Failed to set memory attributes. Status = %r\n",
        Status
        ));
      gDS->FreeMemorySpace (
               RtcPageBase,
               EFI_PAGE_SIZE
               );
      gDS->RemoveMemorySpace (
             RtcPageBase,
             EFI_PAGE_SIZE
             );
      return Status;
    }
  }

  Status = PcRtcInit (&mModuleGlobal);
  ASSERT_EFI_ERROR (Status);

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  PcRtcAcpiTableChangeCallback,
                  NULL,
                  &gEfiAcpi10TableGuid,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  PcRtcAcpiTableChangeCallback,
                  NULL,
                  &gEfiAcpiTableGuid,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  gRT->GetTime       = PcRtcEfiGetTime;
  gRT->SetTime       = PcRtcEfiSetTime;
  gRT->GetWakeupTime = PcRtcEfiGetWakeupTime;
  gRT->SetWakeupTime = PcRtcEfiSetWakeupTime;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mHandle,
                  &gEfiRealTimeClockArchProtocolGuid,
                  NULL,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  if (FixedPcdGetBool (PcdRtcUseMmio)) {
    // Register for the virtual address change event
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    LibRtcVirtualNotifyEvent,
                    NULL,
                    &gEfiEventVirtualAddressChangeGuid,
                    &mVirtualAddrChangeEvent
                    );
    ASSERT_EFI_ERROR (Status);
  }

  return Status;
}
