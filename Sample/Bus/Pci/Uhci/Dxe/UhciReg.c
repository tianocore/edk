/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  UhciReg.c

Abstract:

  The UHCI register operation routines.

Revision History

--*/

#include "Uhci.h"

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
{
  UINT16      Data;
  EFI_STATUS  Status;

  Status = PciIo->Io.Read (
                      PciIo,
                      EfiPciIoWidthUint16,
                      USB_BAR_INDEX,
                      Offset,
                      1,
                      &Data
                      );

  if (EFI_ERROR (Status)) {
    UHCI_ERROR (("UhciReadReg: PciIo Io.Read error: %r at offset %d\n", Status, Offset));

    Data = 0xFFFF;
  }

  return Data;
}

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
{
  EFI_STATUS  Status;

  Status = PciIo->Io.Write (
                      PciIo,
                      EfiPciIoWidthUint16,
                      USB_BAR_INDEX,
                      Offset,
                      1,
                      &Data
                      );

  if (EFI_ERROR (Status)) {
    UHCI_ERROR (("UhciWriteReg: PciIo Io.Write error: %r at offset %d\n", Status, Offset));
  }
}

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
{
  UINT16  Data;

  Data = UhciReadReg (PciIo, Offset);
  Data |= Bit;
  UhciWriteReg (PciIo, Offset, Data);
}

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
{
  UINT16  Data;

  Data = UhciReadReg (PciIo, Offset);
  Data &= ~Bit;
  UhciWriteReg (PciIo, Offset, Data);
}

VOID
UhciAckAllInterrupt (
  IN  USB_HC_DEV          *Uhc
  )
/*++

Routine Description:

  Clear all the interrutp status bits, these bits 
  are Write-Clean

Arguments:

  Uhc - The UHCI device

Returns:

  None
  
--*/
{
  UhciWriteReg (Uhc->PciIo, USBSTS_OFFSET, 0x3F);

  //
  // If current HC is halted, re-enable it. Host Controller Process Error
  // is a temporary error status.
  //
  if (!UhciIsHcWorking (Uhc->PciIo)) {
    UHCI_ERROR (("UhciAckAllInterrupt: re-enable the UHCI from system error\n"));
    Uhc->UsbHc.SetState (&Uhc->UsbHc, EfiUsbHcStateOperational);
  }
}


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
{
  UINT16                UsbSts;
  UINTN                 Index;

  UhciClearRegBit (Uhc->PciIo, USBCMD_OFFSET, USBCMD_RS);
  
  //
  // ensure the HC is in halt status after send the stop command
  // Timeout is in us unit.
  //
  for (Index = 0; Index < (Timeout / 50) + 1; Index++) {
    UsbSts = UhciReadReg (Uhc->PciIo, USBSTS_OFFSET);

    if ((UsbSts & USBSTS_HCH) == USBSTS_HCH) {
      return EFI_SUCCESS;
    }

    gBS->Stall (50);
  }

  return EFI_TIMEOUT;
}

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
{
  UINT16                UsbSts;

  UsbSts = UhciReadReg (PciIo, USBSTS_OFFSET);
  
  if (UsbSts & (USBSTS_HCPE | USBSTS_HSE | USBSTS_HCH)) {
    UHCI_ERROR (("UhciIsHcWorking: current USB state is %x\n", UsbSts));
    return FALSE;
  }

  return TRUE;
}

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
{
  EFI_STATUS              Status;
  UINT32                  Data;

  Data = (UINT32) ((UINTN) Addr & 0xFFFFF000);

  Status = PciIo->Io.Write (
                       PciIo,
                       EfiPciIoWidthUint32,
                       USB_BAR_INDEX,
                       (UINT64) USB_FRAME_BASE_OFFSET,
                       1,
                       &Data
                       );

  if (EFI_ERROR (Status)) {
    UHCI_ERROR (("UhciSetFrameListBaseAddr: PciIo Io.Write error: %r\n", Status));
  }
}

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
{
  UINT16            Command;

  Command = 0;

  PciIo->Pci.Write (
               PciIo,
               EfiPciIoWidthUint16,
               USB_EMULATION_OFFSET,
               1,
               &Command
               );
}
