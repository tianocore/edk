/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  UhciReg.h

Abstract:

  The definition for UHCI register operation routines.

Revision History

--*/

#ifndef _EFI_UHCI_REG_H_
#define _EFI_UHCI_REG_H_

#define BIT(a)  (1 << (a))

enum {
  UHCI_FRAME_NUM        = 1024,
    
  //
  // Register offset and PCI related staff
  //
  CLASSC_OFFSET         = 0x09,
  USBBASE_OFFSET        = 0x20,
  USB_BAR_INDEX         = 4,
  PCI_CLASSC_PI_UHCI    = 0x00,

  USBCMD_OFFSET         = 0,
  USBSTS_OFFSET         = 2,
  USBINTR_OFFSET        = 4,
  USBPORTSC_OFFSET      = 0x10,
  USB_FRAME_NO_OFFSET   = 6,
  USB_FRAME_BASE_OFFSET = 8,
  USB_EMULATION_OFFSET  = 0xC0,
  
  //
  // Packet IDs
  //
  SETUP_PACKET_ID       = 0x2D,
  INPUT_PACKET_ID       = 0x69,
  OUTPUT_PACKET_ID      = 0xE1,
  ERROR_PACKET_ID       = 0x55,
  
  //
  // USB port status and control bit definition.
  //
  USBPORTSC_CCS         = BIT(0),  // Current Connect Status
  USBPORTSC_CSC         = BIT(1),  // Connect Status Change
  USBPORTSC_PED         = BIT(2),  // Port Enable / Disable
  USBPORTSC_PEDC        = BIT(3),  // Port Enable / Disable Change
  USBPORTSC_LSL         = BIT(4),  // Line Status Low BIT
  USBPORTSC_LSH         = BIT(5),  // Line Status High BIT
  USBPORTSC_RD          = BIT(6),  // Resume Detect
  USBPORTSC_LSDA        = BIT(8),  // Low Speed Device Attached
  USBPORTSC_PR          = BIT(9),  // Port Reset
  USBPORTSC_SUSP        = BIT(12), // Suspend

  USB_MAX_ROOTHUB_PORT  = 0x0F,    // Max number of root hub port
  
  //
  // Command register bit definitions
  //
  USBCMD_RS             = BIT(0),  // Run/Stop
  USBCMD_HCRESET        = BIT(1),  // Host reset
  USBCMD_GRESET         = BIT(2),  // Global reset
  USBCMD_EGSM           = BIT(3),  // Global Suspend Mode
  USBCMD_FGR            = BIT(4),  // Force Global Resume
  USBCMD_SWDBG          = BIT(5),  // SW Debug mode
  USBCMD_CF             = BIT(6),  // Config Flag (sw only)
  USBCMD_MAXP           = BIT(7),  // Max Packet (0 = 32, 1 = 64)
  
  //
  // USB Status register bit definitions
  //
  USBSTS_USBINT         = BIT(0),  // Interrupt due to IOC
  USBSTS_ERROR          = BIT(1),  // Interrupt due to error
  USBSTS_RD             = BIT(2),  // Resume Detect
  USBSTS_HSE            = BIT(3),  // Host System Error
  USBSTS_HCPE           = BIT(4),  // Host Controller Process Error
  USBSTS_HCH            = BIT(5),  // HC Halted

  USBTD_ACTIVE          = BIT(7),  // TD is still active
  USBTD_STALLED         = BIT(6),  // TD is stalled
  USBTD_BUFFERR         = BIT(5),  // Buffer underflow or overflow
  USBTD_BABBLE          = BIT(4),  // Babble condition
  USBTD_NAK             = BIT(3),  // NAK is received
  USBTD_CRC             = BIT(2),  // CRC/Time out error
  USBTD_BITSTUFF        = BIT(1),  // Bit stuff error
};

UINT16
UhciReadReg (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  Offset
  )
/*++

Routine Description:

  Read a UHCI register

Arguments:

  PciIo    - The EFI_PCI_IO_PROTOCOL to use
  Offset   - Register offset to USB_BAR_INDEX

Returns:

  Content of register

--*/
;


VOID
UhciWriteReg (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  Offset,
  IN UINT16                  Data
  )
/*++

Routine Description:

  Write data to UHCI register

Arguments:

  PciIo    - The EFI_PCI_IO_PROTOCOL to use
  Offset   - Register offset to USB_BAR_INDEX
  Data     - Data to write

Returns:

  VOID

--*/
;


VOID
UhciSetRegBit (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  Offset,
  IN UINT16                  Bit
  )
/*++

Routine Description:

  Set a bit of the UHCI Register

Arguments:

  PciIo      - The EFI_PCI_IO_PROTOCOL to use
  Offset     - Register offset to USB_BAR_INDEX
  Bit        - The bit to set
  
Returns:

  None

--*/
;


VOID
UhciClearRegBit (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  Offset,
  IN UINT16                  Bit
  )
/*++

Routine Description:

  Clear a bit of the UHCI Register

Arguments:

  PciIo      - The PCI_IO protocol to access the PCI 
  Offset     - Register offset to USB_BAR_INDEX
  Bit        - The bit to clear
Returns:

  None

--*/
;


EFI_STATUS
UhciStopHc (
  IN USB_HC_DEV         *Uhc,
  IN UINTN              Timeout
  )
/*++

Routine Description:

  Stop the host controller

Arguments:

  Uhc     - The UHCI device
  Timeout - Max time allowed 

Returns:

  EFI_SUCCESS - The host controller is stopped
  EFI_TIMEOUT - Failed to stop the host controller

--*/
;


BOOLEAN
UhciIsHcWorking (
  IN EFI_PCI_IO_PROTOCOL     *PciIo
  )
/*++

Routine Description:

  Check whether the host controller operates well

Arguments:

  PciIo  - The PCI_IO protocol to use

Returns:

   TRUE  -  Host controller is working
   FALSE -  Host controller is halted or system error

--*/
;


BOOLEAN
UhciIsHcError (
  IN EFI_PCI_IO_PROTOCOL     *PciIo
  )
/*++

Routine Description:

  Check whether host controller is halt because of system error

Arguments:

  PciIo  - The EFI_PCI_IO_PROTOCOL to use

Returns:

   TRUE  -  The host controller is halt because of system error
   FALSE -  Host controller isn't experiencing a system error, but
            it may still be halted by software.

--*/
;

VOID
UhciSetFrameListBaseAddr (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN VOID                    *Addr
  )
/*++

Routine Description:

  Set the UHCI frame list base address. It can't use 
  UhciWriteReg which access memory in UINT16.

Arguments:

  PciIo   - The EFI_PCI_IO_PROTOCOL to use
  Addr    - Address to set

Returns:

  VOID

--*/
;

VOID
UhciTurnOffUsbEmulation (
  IN EFI_PCI_IO_PROTOCOL     *PciIo
  )
/*++
  
  Routine Description:
  
    Disable USB Emulation
    
  Arguments:
  
    PciIo  -  The EFI_PCI_IO_PROTOCOL protocol to use
    
  Returns:
  
    VOID
    
--*/
;
#endif
