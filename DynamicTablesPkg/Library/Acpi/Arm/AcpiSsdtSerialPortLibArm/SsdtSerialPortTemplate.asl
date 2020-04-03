/** @file
  SSDT Serial Tamplate

  Copyright (c) 2019 - 2020, ARM Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

DefinitionBlock("SsdtSerialPortTemplate.aml", "SSDT", 2, "ARMLTD", "SBSAUART", 1) {
  Scope(_SB) {
    // UART PL011
    Device(COM0) {                                        // {template}
      Name(_HID, "ARMH0011")
      Name(_CID, "PL011")
      Name(_UID, 0x0)                                     // {template}

      Method(_STA) {
        Return(0xF)
      }

      Name(_CRS, ResourceTemplate() {
        QWordMemory (
          ,                   // ResourceUsage
          ,                   // Decode
          ,                   // IsMinFixed
          ,                   // IsMaxFixed
          ,                   // Cacheable
          ReadWrite,          // ReadAndWrite
          0x0,                // AddressGranularity
          0xA0000000,         // AddressMinimum           // {template}
          0xAFFFFFFF,         // AddressMaximum           // {template}
          0,                  // AddressTranslation
          0x10000000,         // RangeLength              // {template}
          ,                   // ResourceSourceIndex
          ,                   // ResourceSource
          ,                   // DescriptorName
          ,                   // MemoryRangeType
                              // TranslationType
        ) // QWordMemory
        Interrupt (
          ResourceConsumer,   // ResourceUsage
          Level,              // EdgeLevel
          ActiveHigh,         // ActiveLevel
          Exclusive,          // Shared
          ,                   // ResourceSourceIndex
          ,                   // ResourceSource
                              // DescriptorName
          ) {
            0xA5                                          // {template}
        } // Interrupt
      }) // Name
    } // Device
  } // Scope(_SB)
}
