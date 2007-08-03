/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  EhciSched.h

Abstract:

  This file contains the definination for host controller schedule routines

Revision History
--*/

#ifndef _EFI_EHCI_SCHED_H_
#define _EFI_EHCI_SCHED_H_

EFI_STATUS
EhcInitSched (
  IN USB2_HC_DEV          *Ehc
  )
/*++

Routine Description:

  Initialize the schedule data structure such as frame list

Arguments:

  Ehc - The EHCI device to init schedule data for

Returns:

  EFI_OUT_OF_RESOURCES  - Failed to allocate resource to init schedule data
  EFI_SUCCESS           - The schedule data is initialized 

--*/
;


VOID
EhcFreeSched (
  IN USB2_HC_DEV          *Ehc
  )
/*++

Routine Description:

  Free the schedule data. It may be partially initialized.

Arguments:

  Ehc - The EHCI device 

Returns:

  None

--*/
;


VOID
EhcLinkQhToAsync (
  IN USB2_HC_DEV          *Ehc,
  IN EHC_QH               *Qh
  )
/*++

Routine Description:

  Link the queue head to the asynchronous schedule list.
  UEFI only supports one CTRL/BULK transfer at a time
  due to its interfaces. This simplifies the AsynList
  management: A reclamation header is always linked to
  the AsyncListAddr, the only active QH is appended to it.

Arguments:

  Ehc - The EHCI device
  Qh  - The queue head to link 

Returns:

  None

--*/
;

VOID
EhcUnlinkQhFromAsync (
  IN USB2_HC_DEV          *Ehc,
  IN EHC_QH               *Qh
  )
/*++

Routine Description:

  Unlink a queue head from the asynchronous schedule list.
  Need to synchronize with hardware

Arguments:

  Ehc - The EHCI device
  Qh  - The queue head to unlink 

Returns:

  None

--*/
;

VOID
EhcLinkQhToPeriod (
  IN USB2_HC_DEV          *Ehc,
  IN EHC_QH               *Qh
  )
/*++

Routine Description:

  Link a queue head for interrupt transfer to the periodic
  schedule frame list. This code is very much the same as
  that in UHCI.

Arguments:

  Ehc - The EHCI device
  Qh  - The queue head to link

Returns:

  None

--*/
;

VOID
EhcUnlinkQhFromPeriod (
  IN USB2_HC_DEV          *Ehc,
  IN EHC_QH               *Qh
  )
/*++

Routine Description:

  Unlink an interrupt queue head from the periodic 
  schedule frame list

Arguments:

  Ehc - The EHCI device
  Qh  - The queue head to unlink

Returns:

  None

--*/
;


EFI_STATUS
EhcExecTransfer (
  IN  USB2_HC_DEV         *Ehc,
  IN  URB                 *Urb,
  IN  UINTN               TimeOut
  )
/*++

Routine Description:

  Execute the transfer by polling the URB. This is a synchronous operation.

Arguments:

  Ehc        - The EHCI device
  Urb        - The URB to execute
  TimeOut    - The time to wait before abort, in millisecond.

Returns:

  EFI_DEVICE_ERROR : The transfer failed due to transfer error
  EFI_TIMEOUT      : The transfer failed due to time out
  EFI_SUCCESS      : The transfer finished OK

--*/
;

EFI_STATUS
EhciDelAsyncIntTransfer (
  IN  USB2_HC_DEV         *Ehc,
  IN  UINT8               DevAddr,
  IN  UINT8               EpNum,  
  OUT UINT8               *DataToggle
  )
/*++

Routine Description:

  Delete a single asynchronous interrupt transfer for
  the device and endpoint

Arguments:

  Ehc         - The EHCI device
  DevAddr     - The address of the target device
  EpNum       - The endpoint of the target
  DataToggle  - Return the next data toggle to use

Returns:

  EFI_SUCCESS   - An asynchronous transfer is removed
  EFI_NOT_FOUND - No transfer for the device is found

--*/
;

VOID
EhciDelAllAsyncIntTransfers (
  IN USB2_HC_DEV          *Ehc
  )
/*++

Routine Description:

  Remove all the asynchronous interrutp transfers

Arguments:

  Ehc - The EHCI device

Returns:

  None

--*/
;


VOID
EhcMoniteAsyncRequests (
  IN EFI_EVENT            Event,
  IN VOID                 *Context
  )
/*++
Routine Description:

  Interrupt transfer periodic check handler

Arguments:
  Event    - Interrupt event
  Context  - Pointer to USB2_HC_DEV

Returns:
  
  None
  
--*/
;

#endif
