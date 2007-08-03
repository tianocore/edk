/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

    EhciReg.c

Abstract:

    The EHCI register operation routines.


Revision History
--*/

#include "Ehci.h"

UINT32
EhcReadCapRegister (
  IN  USB2_HC_DEV         *Ehc,
  IN  UINT32              Offset
  )
/*++

Routine Description:

  Read  EHCI capability register

Arguments:

  Ehc     - The Ehc device 
  Offset  - Capability register address

Returns:

  The register content read
  
--*/
{
  UINT32                  Data;
  EFI_STATUS              Status;

  Status = Ehc->PciIo->Mem.Read (
                             Ehc->PciIo,
                             EfiPciIoWidthUint32,
                             EHC_BAR_INDEX,
                             (UINT64) Offset,
                             1,
                             &Data
                             );

  if (EFI_ERROR (Status)) {
    EHC_ERROR (("EhcReadCapRegister: Pci Io read error - %r at %d\n", Status, Offset));
    Data = 0xFFFF;
  }

  return Data;
}

UINT32
EhcReadOpReg (
  IN  USB2_HC_DEV         *Ehc,
  IN  UINT32              Offset
  )
/*++

Routine Description:

  Read  Ehc Operation register

Arguments:

  Ehc      - The EHCI device
  Offset   - The operation register offset

Returns:

  The register content read
  
--*/
{
  UINT32                  Data;
  EFI_STATUS              Status;

  ASSERT (Ehc->CapLen != 0);

  Status = Ehc->PciIo->Mem.Read (
                             Ehc->PciIo,
                             EfiPciIoWidthUint32,
                             EHC_BAR_INDEX,
                             (UINT64) (Ehc->CapLen + Offset),
                             1,
                             &Data
                             );

  if (EFI_ERROR (Status)) {
    EHC_ERROR (("EhcReadOpReg: Pci Io Read error - %r at %d\n", Status, Offset));
    Data = 0xFFFF;
  }

  return Data;
}

VOID
EhcWriteOpReg (
  IN USB2_HC_DEV          *Ehc,
  IN UINT32               Offset,
  IN UINT32               Data
  )
/*++

Routine Description:

  Write  the data to the EHCI operation register

Arguments:

  Ehc      - The EHCI device
  Offset   - EHCI operation register offset
  Data     - The data to write

Returns:

  None

--*/
{
  EFI_STATUS              Status;

  ASSERT (Ehc->CapLen != 0);

  Status = Ehc->PciIo->Mem.Write (
                             Ehc->PciIo,
                             EfiPciIoWidthUint32,
                             EHC_BAR_INDEX,
                             (UINT64) (Ehc->CapLen + Offset),
                             1,
                             &Data
                             );

  if (EFI_ERROR (Status)) {
    EHC_ERROR (("EhcWriteOpReg: Pci Io Write error: %r at %d\n", Status, Offset));
  }
}

STATIC
VOID
EhcSetOpRegBit (
  IN USB2_HC_DEV          *Ehc,
  IN UINT32               Offset,
  IN UINT32               Bit
  )
/*++

Routine Description:
  Set one bit of the operational register while keeping other bits

Arguments:
  Ehc     - The EHCI device
  Offset  - The offset of the operational register
  Bit     - The bit mask of the register to set
  
Returns:

  None
  
--*/
{
  UINT32                  Data;

  Data  = EhcReadOpReg (Ehc, Offset);
  Data |= Bit;
  EhcWriteOpReg (Ehc, Offset, Data);
}

STATIC
VOID
EhcClearOpRegBit (
  IN USB2_HC_DEV          *Ehc,
  IN UINT32               Offset,
  IN UINT32               Bit
  )
/*++

Routine Description:
  Clear one bit of the operational register while keeping other bits

Arguments:
  Ehc - The EHCI device
  Offset  - The offset of the operational register
  Bit     - The bit mask of the register to clear

Returns:

  None

--*/
{
  UINT32                  Data;

  Data  = EhcReadOpReg (Ehc, Offset);
  Data &= ~Bit;
  EhcWriteOpReg (Ehc, Offset, Data);
}

STATIC
EFI_STATUS
EhcWaitOpRegBit (
  IN USB2_HC_DEV          *Ehc,
  IN UINT32               Offset,
  IN UINT32               Bit,
  IN BOOLEAN              WaitToSet,
  IN UINT32               Timeout
  )
/*++

Routine Description:

  Wait the operation register's bit as specified by Bit 
  to become set (or clear)

Arguments:

  Ehc         - The EHCI device
  Offset      - The offset of the operation register 
  Bit         - The bit of the register to wait for
  WaitToSet   - Wait the bit to set or clear
  Timeout     - The time to wait before abort (in millisecond)

Returns:

  EFI_SUCCESS - The bit successfully changed by host controller
  EFI_TIMEOUT - The time out occurred

--*/
{
  UINT32                  Index;

  for (Index = 0; Index < Timeout / EHC_SYNC_POLL_TIME + 1; Index++) {
    if (EHC_REG_BIT_IS_SET (Ehc, Offset, Bit) == WaitToSet) {
      return EFI_SUCCESS;
    }

    gBS->Stall (EHC_SYNC_POLL_TIME);
  }

  return EFI_TIMEOUT;
}

VOID
EhcClearLegacySupport (
  IN USB2_HC_DEV          *Ehc
  )
/*++

Routine Description:

  Add support for UEFI Over Legacy (UoL) feature, stop 
  the legacy USB SMI support

Arguments:

  Ehc - The EHCI device.

Returns:

  None
  
--*/
{
  UINT32                    ExtendCap;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  UINT32                    Value;
  UINT32                    TimeOut;

  EHC_DEBUG (("EhcClearLegacySupport: called to clear legacy support\n"));

  PciIo     = Ehc->PciIo;
  ExtendCap = (Ehc->HcCapParams >> 8) & 0xFF;

  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &Value);
  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap + 0x4, 1, &Value);

  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &Value);
  Value |= (0x1 << 24);
  PciIo->Pci.Write (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &Value);

  TimeOut = 40;
  while (TimeOut--) {
    gBS->Stall (500);

    PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &Value);

    if ((Value & 0x01010000) == 0x01000000) {
      break;
    }
  }

  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &Value);
  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap + 0x4, 1, &Value);
}


EFI_STATUS
EhcSetAndWaitDoorBell (
  IN  USB2_HC_DEV         *Ehc,
  IN  UINT32              Timeout
  )
/*++

Routine Description:

  Set door bell and wait it to be ACKed by host controller.
  This function is used to synchronize with the hardware.

Arguments:

  Ehc     - The EHCI device
  Timeout - The time to wait before abort (in millisecond, ms)
Returns:

  EFI_SUCCESS : Synchronized with the hardware
  EFI_TIMEOUT : Time out happened while waiting door bell to set

--*/
{
  EFI_STATUS              Status;
  UINT32                  Data;

  EhcSetOpRegBit (Ehc, EHC_USBCMD_OFFSET, USBCMD_IAAD);

  Status = EhcWaitOpRegBit (Ehc, EHC_USBSTS_OFFSET, USBSTS_IAA, TRUE, Timeout);

  //
  // ACK the IAA bit in USBSTS register. Make sure other
  // interrupt bits are not ACKed. These bits are WC (Write Clean).
  //
  Data  = EhcReadOpReg (Ehc, EHC_USBSTS_OFFSET);
  Data &= ~USBSTS_INTACK_MASK;
  Data |= USBSTS_IAA;

  EhcWriteOpReg (Ehc, EHC_USBSTS_OFFSET, Data);

  return Status;
}

VOID
EhcAckAllInterrupt (
  IN  USB2_HC_DEV         *Ehc
  )
/*++

Routine Description:

  Clear all the interrutp status bits, these bits 
  are Write-Clean

Arguments:

  Ehc - The EHCI device

Returns:

  None
  
--*/
{
  EhcWriteOpReg (Ehc, EHC_USBSTS_OFFSET, USBSTS_INTACK_MASK);
}

STATIC
EFI_STATUS
EhcEnablePeriodSchd (
  IN USB2_HC_DEV          *Ehc,
  IN UINT32               Timeout
  )
/*++

Routine Description:

  Enable the periodic schedule then wait EHC to 
  actually enable it.

Arguments:

  Ehc     - The EHCI device
  Timeout - The time to wait before abort (in millisecond, ms)
Returns:

  EFI_SUCCESS : The periodical schedule is enabled
  EFI_TIMEOUT : Time out happened while enabling periodic schedule

--*/
{
  EFI_STATUS              Status;

  EhcSetOpRegBit (Ehc, EHC_USBCMD_OFFSET, USBCMD_ENABLE_PERIOD);

  Status = EhcWaitOpRegBit (Ehc, EHC_USBSTS_OFFSET, USBSTS_PERIOD_ENABLED, TRUE, Timeout);
  return Status;
}


STATIC
EFI_STATUS
EhcDisablePeriodSchd (
  IN USB2_HC_DEV          *Ehc,
  IN UINT32               Timeout
  )
/*++

Routine Description:

  Disable periodic schedule

Arguments:

  Ehc     - The EHCI device
  Timeout - Time to wait before abort (in millisecond, ms)
  
Returns:

  EFI_SUCCESS      : Periodic schedule is disabled.
  EFI_DEVICE_ERROR : Fail to disable periodic schedule

--*/
{
  EFI_STATUS              Status;

  EhcClearOpRegBit (Ehc, EHC_USBCMD_OFFSET, USBCMD_ENABLE_PERIOD);

  Status = EhcWaitOpRegBit (Ehc, EHC_USBSTS_OFFSET, USBSTS_PERIOD_ENABLED, FALSE, Timeout);
  return Status;
}


STATIC
EFI_STATUS
EhcEnableAsyncSchd (
  IN USB2_HC_DEV          *Ehc,
  IN UINT32               Timeout
  )
/*++

Routine Description:

  Enable asynchrounous schedule

Arguments:

  Ehc     - The EHCI device
  Timeout - Time to wait before abort
  
Returns:

  EFI_SUCCESS : The EHCI asynchronous schedule is enabled
  Others      : Failed to enable the asynchronous scheudle

--*/
{
  EFI_STATUS              Status;

  EhcSetOpRegBit (Ehc, EHC_USBCMD_OFFSET, USBCMD_ENABLE_ASYNC);

  Status = EhcWaitOpRegBit (Ehc, EHC_USBSTS_OFFSET, USBSTS_ASYNC_ENABLED, TRUE, Timeout);
  return Status;
}


STATIC
EFI_STATUS
EhcDisableAsyncSchd (
  IN USB2_HC_DEV          *Ehc,
  IN UINT32               Timeout
  )
/*++

Routine Description:

  Disable asynchrounous schedule

Arguments:

  Ehc     - The EHCI device
  Timeout - Time to wait before abort (in millisecond, ms)
  
Returns:

  EFI_SUCCESS : The asynchronous schedule is disabled
  Others      : Failed to disable the asynchronous schedule

--*/
{
  EFI_STATUS  Status;

  EhcClearOpRegBit (Ehc, EHC_USBCMD_OFFSET, USBCMD_ENABLE_ASYNC);

  Status = EhcWaitOpRegBit (Ehc, EHC_USBSTS_OFFSET, USBSTS_ASYNC_ENABLED, FALSE, Timeout);
  return Status;
}


BOOLEAN
EhcIsHalt (
  IN USB2_HC_DEV          *Ehc
  )
/*++

Routine Description:

  Whether Ehc is halted

Arguments:

  Ehc - The EHCI device

Returns:

  TRUE  : The controller is halted
  FALSE : It isn't halted

--*/
{
  return EHC_REG_BIT_IS_SET (Ehc, EHC_USBSTS_OFFSET, USBSTS_HALT);
}

BOOLEAN
EhcIsSysError (
  IN USB2_HC_DEV          *Ehc
  )
/*++

Routine Description:

  Whether system error occurred

Arguments:

  Ehc - The EHCI device

Returns:

  TRUE  : System error happened 
  FALSE : No system error

--*/
{
  return EHC_REG_BIT_IS_SET (Ehc, EHC_USBSTS_OFFSET, USBSTS_SYS_ERROR);
}

EFI_STATUS
EhcResetHC (
  IN USB2_HC_DEV          *Ehc,
  IN UINT32               Timeout
  )
/*++

Routine Description:

  Reset the host controller

Arguments:

  Ehc     - The EHCI device
  Timeout - Time to wait before abort (in millisecond, ms)

Returns:

  EFI_SUCCESS : The host controller is reset
  Others      : Failed to reset the host

--*/
{
  EFI_STATUS              Status;

  //
  // Host can only be reset when it is halt. If not so, halt it
  //
  if (!EHC_REG_BIT_IS_SET (Ehc, EHC_USBSTS_OFFSET, USBSTS_HALT)) {
    Status = EhcHaltHC (Ehc, Timeout);

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  EhcSetOpRegBit (Ehc, EHC_USBCMD_OFFSET, USBCMD_RESET);
  Status = EhcWaitOpRegBit (Ehc, EHC_USBCMD_OFFSET, USBCMD_RESET, FALSE, Timeout);
  return Status;
}

EFI_STATUS
EhcHaltHC (
  IN USB2_HC_DEV         *Ehc,
  IN UINT32              Timeout
  )
/*++

Routine Description:

  Halt the host controller

Arguments:

  Ehc     - The EHCI device
  Timeout - Time to wait before abort

Returns:

  EFI_SUCCESS : The EHCI is halt
  EFI_TIMEOUT : Failed to halt the controller before Timeout 

--*/
{
  EFI_STATUS              Status;

  EhcClearOpRegBit (Ehc, EHC_USBCMD_OFFSET, USBCMD_RUN);
  Status = EhcWaitOpRegBit (Ehc, EHC_USBSTS_OFFSET, USBSTS_HALT, TRUE, Timeout);
  return Status;
}

EFI_STATUS
EhcRunHC (
  IN USB2_HC_DEV          *Ehc,
  IN UINT32               Timeout
  )
/*++

Routine Description:

  Set the EHCI to run

Arguments:

  Ehc     - The EHCI device
  Timeout - Time to wait before abort

Returns:

  EFI_SUCCESS : The EHCI is running
  Others      : Failed to set the EHCI to run

--*/
{
  EFI_STATUS              Status;

  EhcSetOpRegBit (Ehc, EHC_USBCMD_OFFSET, USBCMD_RUN);
  Status = EhcWaitOpRegBit (Ehc, EHC_USBSTS_OFFSET, USBSTS_HALT, FALSE, Timeout);
  return Status;
}

EFI_STATUS
EhcInitHC (
  IN USB2_HC_DEV          *Ehc
  )
/*++

Routine Description:

  Initialize the HC hardware. 
  EHCI spec lists the five things to do to initialize the hardware
  1. Program CTRLDSSEGMENT
  2. Set USBINTR to enable interrupts
  3. Set periodic list base
  4. Set USBCMD, interrupt threshold, frame list size etc
  5. Write 1 to CONFIGFLAG to route all ports to EHCI

Arguments:

  Ehc - The EHCI device

Returns:

  EFI_SUCCESS : The EHCI has come out of halt state
  EFI_TIMEOUT : Time out happened

--*/
{
  EFI_STATUS              Status;

  ASSERT (EhcIsHalt (Ehc));

  //
  // Allocate the periodic frame and associated memeory
  // management facilities if not already done.
  //
  if (Ehc->PeriodFrame != NULL) {
    EhcFreeSched (Ehc);
  }

  Status = EhcInitSched (Ehc);

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // 1. Program the CTRLDSSEGMENT register with the high 32 bit addr
  //
  EhcWriteOpReg (Ehc, EHC_CTRLDSSEG_OFFSET, Ehc->High32bitAddr);

  //
  // 2. Clear USBINTR to disable all the interrupt. UEFI works by polling
  //
  EhcWriteOpReg (Ehc, EHC_USBINTR_OFFSET, 0);

  //
  // 3. Program periodic frame list, already done in EhcInitSched
  // 4. Start the Host Controller
  //
  EhcSetOpRegBit (Ehc, EHC_USBCMD_OFFSET, USBCMD_RUN);

  //
  // 5. Set all ports routing to EHC
  //
  EhcSetOpRegBit (Ehc, EHC_CONFIG_FLAG_OFFSET, CONFIGFLAG_ROUTE_EHC);

  Status = EhcEnablePeriodSchd (Ehc, EHC_GENERIC_TIME);

  if (EFI_ERROR (Status)) {
    EHC_ERROR (("EhcInitHC: failed to enable period schedule\n"));
    return Status;
  }

  Status = EhcEnableAsyncSchd (Ehc, EHC_GENERIC_TIME);

  if (EFI_ERROR (Status)) {
    EHC_ERROR (("EhcInitHC: failed to enable async schedule\n"));
    return Status;
  }

  return EFI_SUCCESS;
}
