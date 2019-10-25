/** @file
  SSDT Serial Tamplate

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

DefinitionBlock("SsdtSerialTemplate.aml", "SSDT", 1, "ARMLTD", "ARMH-ASL", 1) {
  Scope(_SB) {
    // UART PL011
    Device(COM0) {                // {template}
      Name(_HID, "ARMH0011")
      Name(_CID, "PL011")
      Name(_UID, Zero)

      Method(_STA) {
        Return(0xF)
      }

      Method(_CRS, 0x0, NotSerialized) {
        Name(RBUF, ResourceTemplate() {
          Memory32Fixed (
            ReadWrite,
            0xDEADBEAF,           // {template}
            0x8C8C8C8C            // {template}
            )
          Interrupt (
            ResourceConsumer,
            Level,
            ActiveHigh,
            Exclusive) {
              0xA5                // {template}
              }
        })
        Return (RBUF)
      }
    }
  } // Scope(_SB)
}
