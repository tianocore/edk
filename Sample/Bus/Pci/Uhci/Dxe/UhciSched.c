/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  UhciSched.c

Abstract:

  The EHCI register operation routines.

Revision History

--*/

#include "Uhci.h"

EFI_STATUS
UhciInitFrameList (
  IN USB_HC_DEV         *Uhc
  )
/*++

Routine Description:

  Create Frame List Structure

Arguments:

  Uhc         - UHCI device

Returns:

  EFI_OUT_OF_RESOURCES - Can't allocate memory resources
  EFI_UNSUPPORTED      - Map memory fail
  EFI_SUCCESS          - Success

--*/
{
  EFI_PHYSICAL_ADDRESS  MappedAddr;
  EFI_STATUS            Status;
  VOID                  *Buffer;
  VOID                  *Mapping;
  UINTN                 Pages;
  UINTN                 Bytes;
  UINTN                 Index;

  //
  // The Frame List is a common buffer that will be
  // accessed by both the cpu and the usb bus master
  // at the same time. The Frame List ocupies 4K bytes,
  // and must be aligned on 4-Kbyte boundaries.
  //
  Bytes = 4096;
  Pages = EFI_SIZE_TO_PAGES (Bytes);

  Status = Uhc->PciIo->AllocateBuffer (
                         Uhc->PciIo,
                         AllocateAnyPages,
                         EfiBootServicesData,
                         Pages,
                         &Buffer,
                         0
                         );

  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = Uhc->PciIo->Map (
                         Uhc->PciIo,
                         EfiPciIoOperationBusMasterCommonBuffer,
                         Buffer,
                         &Bytes,
                         &MappedAddr,
                         &Mapping
                         );

  if (EFI_ERROR (Status) || (Bytes != 4096)) {
    Status = EFI_UNSUPPORTED;
    goto ON_ERROR;
  }

  Uhc->FrameBase    = (UINT32 *) (UINTN) MappedAddr;
  Uhc->FrameMapping = Mapping;

  //
  // Allocate the QH used by sync interrupt/control/bulk transfer.
  // FS ctrl/bulk queue head is set to loopback so additional BW
  // can be reclaimed. Notice, LS don't support bulk transfer and
  // also doesn't support BW reclamation.
  //
  Uhc->SyncIntQh    = UhciCreateQh (Uhc, 1);
  Uhc->LsCtrlQh     = UhciCreateQh (Uhc, 1);
  Uhc->FsCtrlBulkQh = UhciCreateQh (Uhc, 1);

  if ((Uhc->SyncIntQh == NULL) || (Uhc->LsCtrlQh == NULL) || 
      (Uhc->FsCtrlBulkQh == NULL)) {

    Uhc->PciIo->Unmap (Uhc->PciIo, Mapping);
    Status = EFI_OUT_OF_RESOURCES;
    goto ON_ERROR;
  }
  
  //
  //                                                +-------------+
  //                                                |             |
  // Link the three together: SyncIntQh->LsCtrlQh->FsCtrlBulkQh <-+
  // Each frame entry is linked to this sequence of QH. These QH
  // will remain on the schedul, never got removed
  //
  Uhc->SyncIntQh->QhHw.HorizonLink    = QH_HLINK (Uhc->LsCtrlQh, FALSE);
  Uhc->SyncIntQh->NextQh              = Uhc->LsCtrlQh;

  Uhc->LsCtrlQh->QhHw.HorizonLink     = QH_HLINK (Uhc->FsCtrlBulkQh, FALSE);
  Uhc->LsCtrlQh->NextQh               = Uhc->FsCtrlBulkQh;

  Uhc->FsCtrlBulkQh->QhHw.HorizonLink = QH_HLINK (Uhc->FsCtrlBulkQh, FALSE);
  Uhc->FsCtrlBulkQh->NextQh           = NULL;

  for (Index = 0; Index < UHCI_FRAME_NUM; Index++) {
    Uhc->FrameBase[Index] = QH_HLINK (Uhc->SyncIntQh, FALSE);
  }
  
  //
  // Tell the Host Controller where the Frame List lies,
  // by set the Frame List Base Address Register.
  //
  UhciSetFrameListBaseAddr (Uhc->PciIo, (VOID *) (Uhc->FrameBase));
  return EFI_SUCCESS;

ON_ERROR:
  if (Uhc->SyncIntQh != NULL) {
    UhciFreePool (Uhc, (UINT8 *) Uhc->SyncIntQh, sizeof (UHCI_QH_SW));
  }

  if (Uhc->LsCtrlQh != NULL) {
    UhciFreePool (Uhc, (UINT8 *) Uhc->LsCtrlQh, sizeof (UHCI_QH_SW));
  }

  if (Uhc->FsCtrlBulkQh != NULL) {
    UhciFreePool (Uhc, (UINT8 *) Uhc->FsCtrlBulkQh, sizeof (UHCI_QH_SW));
  }

  Uhc->PciIo->FreeBuffer (Uhc->PciIo, Pages, Buffer);
  return Status;
}

VOID
UhciDestoryFrameList (
  IN USB_HC_DEV           *Uhc
  )
/*++

Routine Description:

  Destory FrameList buffer

Arguments:

  Uhc - The UHCI device

Returns:

  VOID

--*/
{
  //
  // Unmap the common buffer for framelist entry,
  // and free the common buffer.
  // Uhci's frame list occupy 4k memory.
  //
  Uhc->PciIo->Unmap (Uhc->PciIo, Uhc->FrameMapping);

  Uhc->PciIo->FreeBuffer (
                Uhc->PciIo,
                EFI_SIZE_TO_PAGES (4096),
                (VOID *) Uhc->FrameBase
                );

  if (Uhc->SyncIntQh != NULL) {
    UhciFreePool (Uhc, (UINT8 *) Uhc->SyncIntQh, sizeof (UHCI_QH_SW));
  }

  if (Uhc->LsCtrlQh != NULL) {
    UhciFreePool (Uhc, (UINT8 *) Uhc->LsCtrlQh, sizeof (UHCI_QH_SW));
  }

  if (Uhc->FsCtrlBulkQh != NULL) {
    UhciFreePool (Uhc, (UINT8 *) Uhc->FsCtrlBulkQh, sizeof (UHCI_QH_SW));
  }

  Uhc->FrameBase    = NULL;
  Uhc->SyncIntQh    = NULL;
  Uhc->LsCtrlQh     = NULL;
  Uhc->FsCtrlBulkQh = NULL;
}

EFI_STATUS
UhciMapUserRequest (
  IN  USB_HC_DEV          *Uhc,
  IN  OUT VOID            *Request,
  OUT UINT8               **MappedAddr,
  OUT VOID                **Map
  )
/*++

Routine Description:

  Map address of request structure buffer

Arguments:

  Uhc        - The UHCI device
  Request    - The user request buffer
  MappedAddr - Mapped address of request 
  Map        - Identificaion of this mapping to return

Returns:

  EFI_SUCCESS      : Success
  EFI_DEVICE_ERROR : Fail to map the user request

--*/
{
  EFI_STATUS            Status;
  UINTN                 Len;
  EFI_PHYSICAL_ADDRESS  PhyAddr;

  Len    = sizeof (EFI_USB_DEVICE_REQUEST);
  Status = Uhc->PciIo->Map (
                         Uhc->PciIo,
                         EfiPciIoOperationBusMasterRead,
                         Request,
                         &Len,
                         &PhyAddr,
                         Map
                         );

  if (!EFI_ERROR (Status)) {
    *MappedAddr = (UINT8 *) (UINTN) PhyAddr;
  }

  return Status;
}

EFI_STATUS
UhciMapUserData (
  IN  USB_HC_DEV              *Uhc,
  IN  EFI_USB_DATA_DIRECTION  Direction,
  IN  VOID                    *Data,
  IN  OUT UINTN               *Len,
  OUT UINT8                   *PktId,
  OUT UINT8                   **MappedAddr,
  OUT VOID                    **Map
  )
/*++

Routine Description:

  Map address of user data buffer

Arguments:

  Uhc        - The UHCI device
  Direction  - direction of the data transfer
  Data       - The user data buffer
  Len        - Length of the user data
  PktId      - Packet identificaion
  MappedAddr - mapped address to return
  Map        - identificaion of this mapping to return

Returns:

  EFI_SUCCESS      : Success
  EFI_DEVICE_ERROR : Fail to map the user data

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhyAddr;

  Status = EFI_SUCCESS;

  switch (Direction) {
  case EfiUsbDataIn:
    //
    // BusMasterWrite means cpu read
    //
    *PktId = INPUT_PACKET_ID;
    Status = Uhc->PciIo->Map (
                           Uhc->PciIo,
                           EfiPciIoOperationBusMasterWrite,
                           Data,
                           Len,
                           &PhyAddr,
                           Map
                           );

    if (EFI_ERROR (Status)) {
      goto EXIT;
    }

    *MappedAddr = (UINT8 *) (UINTN) PhyAddr;
    break;

  case EfiUsbDataOut:
    *PktId = OUTPUT_PACKET_ID;
    Status = Uhc->PciIo->Map (
                           Uhc->PciIo,
                           EfiPciIoOperationBusMasterRead,
                           Data,
                           Len,
                           &PhyAddr,
                           Map
                           );

    if (EFI_ERROR (Status)) {
      goto EXIT;
    }

    *MappedAddr = (UINT8 *) (UINTN) PhyAddr;
    break;

  case EfiUsbNoData:
    if ((Len != NULL) && (*Len != 0)) {
      Status    = EFI_INVALID_PARAMETER;
			goto EXIT;
    }
		
    *PktId      = OUTPUT_PACKET_ID;
    *Len        = 0;
    *MappedAddr = NULL;
    *Map        = NULL;
    break;

  default:
    Status      = EFI_INVALID_PARAMETER;
  }

EXIT:
  return Status;
}

STATIC
UINTN
UhciGetNumberOfTd (
  IN UHCI_TD_SW           *First
  )
/*++

Routine Description:

  Get number of TDs

Arguments:

  First   - The first TD on the link
  
Returns:

  The number of Tds

--*/
{
  UINTN                   Num;

  Num = 0;

  for (Num = 0; First != NULL; First = First->NextTd) {
    Num++;
  }

  return Num;
}

UINTN
UhciConvertPollRate (
  IN  UINTN               Interval
  )
/*++

Routine Description:

  Convert the poll rate to the maxium 2^n that is smaller
  than Interval

Arguments:

  Interval  - The poll rate to convert

Returns:

  The converted poll rate

--*/
{
  UINTN                   BitCount;

  ASSERT (Interval != 0);

  //
  // Find the index (1 based) of the highest non-zero bit
  //
  BitCount = 0;

  while (Interval != 0) {
    Interval >>= 1;
    BitCount++;
  }

  return (UINTN)1 << (BitCount - 1);
}

VOID
UhciLinkQhToFrameList (
  UINT32                  *FrameBase,
  UHCI_QH_SW              *Qh
  )
/*++

Routine Description:

  Link a queue head (for asynchronous interrupt transfer) to  
  the frame list.

Arguments:

  FrameBase - The base of the frame list
  Qh        - The queue head to link into

Returns:

  None

--*/
{
  UINTN                   Index;
  UHCI_QH_SW              *Prev;
  UHCI_QH_SW              *Next;

  ASSERT ((FrameBase != NULL) && (Qh != NULL));
  
  for (Index = 0; Index < UHCI_FRAME_NUM; Index += Qh->Interval) {
    //
    // First QH can't be NULL because we always keep static queue
    // heads on the frame list
    //
    ASSERT (!LINK_TERMINATED (FrameBase[Index]));
    Next  = UHCI_ADDR (FrameBase[Index]);
    Prev  = NULL;

    //
    // Now, insert the queue head (Qh) into this frame:
    // 1. Find a queue head with the same poll interval, just insert
    //    Qh after this queue head, then we are done.
    //
    // 2. Find the position to insert the queue head into:
    //      Previous head's interval is bigger than Qh's
    //      Next head's interval is less than Qh's
    //    Then, insert the Qh between then
    //
    // This method is very much the same as that used by EHCI.
    // Because each QH's interval is round down to 2^n, poll
    // rate is correct.
    //
    while (Next->Interval > Qh->Interval) {
      Prev  = Next;
      Next  = Next->NextQh;
    }

    ASSERT (Next != NULL);

    //
    // The entry may have been linked into the frame by early insertation.
    // For example: if insert a Qh with Qh.Interval == 4, and there is a Qh
    // with Qh.Interval == 8 on the frame. If so, we are done with this frame.
    // It isn't necessary to compare all the QH with the same interval to
    // Qh. This is because if there is other QH with the same interval, Qh
    // should has been inserted after that at FrameBase[0] and at FrameBase[0] it is
    // impossible (Next == Qh)
    //
    if (Next == Qh) {
      continue;
    }

    if (Next->Interval == Qh->Interval) {
      //
      // If there is a QH with the same interval, it locates at
      // FrameBase[0], and we can simply insert it after this QH. We
      // are all done.
      //
      ASSERT ((Index == 0) && (Qh->NextQh == NULL));

      Prev                    = Next;
      Next                    = Next->NextQh;

      Qh->NextQh              = Next;
      Prev->NextQh            = Qh;

      Qh->QhHw.HorizonLink    = Prev->QhHw.HorizonLink;
      Prev->QhHw.HorizonLink  = QH_HLINK (Qh, FALSE);
      break;
    }
    
    //
    // OK, find the right position, insert it in. If Qh's next
    // link has already been set, it is in position. This is
    // guarranted by 2^n polling interval.
    //
    if (Qh->NextQh == NULL) {
      Qh->NextQh            = Next;
      Qh->QhHw.HorizonLink  = QH_HLINK (Next, FALSE);
    }

    if (Prev == NULL) {
      FrameBase[Index]        = QH_HLINK (Qh, FALSE);
    } else {
      Prev->NextQh            = Qh;
      Prev->QhHw.HorizonLink  = QH_HLINK (Qh, FALSE);
    }
  }
}

VOID
UhciUnlinkQhFromFrameList (
  UINT32                *FrameBase,
  UHCI_QH_SW            *Qh
  )
/*++

Routine Description:

  Unlink QH from the frame list is easier: find all
  the precedence node, and pointer there next to QhSw's
  next.

Arguments:

  FrameBase - The base address of the frame list
  Qh        - The queue head to unlink 

Returns:

  None

--*/
{
  UINTN                   Index;
  UHCI_QH_SW              *Prev;
  UHCI_QH_SW              *This;

  ASSERT ((FrameBase != NULL) && (Qh != NULL));
  
  for (Index = 0; Index < UHCI_FRAME_NUM; Index += Qh->Interval) {
    //
    // Frame link can't be NULL because we always keep static 
    // queue heads on the frame list
    //
    ASSERT (!LINK_TERMINATED (FrameBase[Index]));
    This  = UHCI_ADDR (FrameBase[Index]);
    Prev  = NULL;

    //
    // Walk through the frame's QH list to find the 
    // queue head to remove
    //
    while ((This != NULL) && (This != Qh)) {
      Prev  = This;
      This  = This->NextQh;
    }
    
    //
    // Qh may have already been unlinked from this frame 
    // by early action.
    //
    if (This == NULL) {
      continue;
    }

    if (Prev == NULL) {
      //
      // Qh is the first entry in the frame
      //
      FrameBase[Index]        = Qh->QhHw.HorizonLink;
    } else {
      Prev->NextQh            = Qh->NextQh;
      Prev->QhHw.HorizonLink  = Qh->QhHw.HorizonLink;
    }
  }
}

STATIC
BOOLEAN
UhciCheckTdStatus (
  IN  UHCI_TD_SW          *Td,
  OUT UINT32              *Result,
  OUT UINT32              *ErrTdPos,
  OUT UINTN               *ActualLen,
  IN  BOOLEAN             IsLow
  )
/*++

Routine Description:

  Check TDs Results

Arguments:

  Td          - UHCI_TD_SW to check
  RequiredLen - Required Len
  Result      - Transfer result
  ErrTdPos    - Error TD Position
  ActualLen   - Actual Transfer Size
  IsLow       - Is Low Speed Device

Returns:

  Wheterh all the TD of the transfer has been processed
  
--*/
{
  UINTN                   Len;
  BOOLEAN                 Finished;

  Finished   = TRUE;
  *Result    = EFI_USB_NOERROR;
  *ErrTdPos  = 0;
  *ActualLen = 0;

  while (Td != NULL) {
    if (Td->TdHw.Status & USBTD_ACTIVE) {
      *Result |= EFI_USB_ERR_NOTEXECUTE;
    }

    if (Td->TdHw.Status & USBTD_STALLED) {
      *Result |= EFI_USB_ERR_STALL;
    }

    if (Td->TdHw.Status & USBTD_BUFFERR) {
      *Result |= EFI_USB_ERR_BUFFER;
    }

    if (Td->TdHw.Status & USBTD_BABBLE) {
      *Result |= EFI_USB_ERR_BABBLE;
    }

    if (Td->TdHw.Status & USBTD_NAK) {
      *Result |= EFI_USB_ERR_NAK;
    }

    if (Td->TdHw.Status & USBTD_CRC) {
      *Result |= EFI_USB_ERR_TIMEOUT;
    }

    if (Td->TdHw.Status & USBTD_BITSTUFF) {
      *Result |= EFI_USB_ERR_BITSTUFF;
    }
    
    //
    // If any error encountered, stop processing the left TDs.
    //
    if (*Result) {
      Finished = FALSE;
      goto EXIT;
    }
    
    //
    // Retrieve the actual number of bytes that were tansferred.
    // the value is encoded as n-1. so return ActualLen + 1.
    //
    Len = (Td->TdHw.ActualLen + 1) & 0x7FF;

    if (Td->TdHw.PidCode != SETUP_PACKET_ID) {
      *ActualLen += Len;
    }

       
    //
    // Short packet condition for full speed input TD, also 
    // terminate the transfer
    //
    if (!IsLow && (Td->TdHw.PidCode == INPUT_PACKET_ID) && 
       (Len < Td->TdHw.MaxPacketLen)) {
      UHCI_DEBUG ((UHCI_DEBUG_SCHEDULE, "Short Packet Occured\n"));
      Finished = TRUE;
      goto EXIT;
    }
    
    //
    // Accumulate actual transferred data length in each TD.
    // Then record the first Error TD's position in the queue,
    // this value is zero-based.
    //
    Td = Td->NextTd;
    (*ErrTdPos)++;
  }

EXIT:
  return Finished;
}

EFI_STATUS
UhciExecuteTransfer (
  IN  USB_HC_DEV          *Uhc,
  IN  UHCI_TD_SW          *Td,
  OUT UINTN               *ActualLen,
  OUT UINT8               *DataToggle,
  IN  UINTN               TimeOut,
  OUT UINT32              *Result,
  IN  BOOLEAN             IsControl,
  IN  BOOLEAN             IsLow
  )
/*++

Routine Description:

  Check the result of the transfer

Arguments:

  Uhc            - The UHCI device
  Td             - The first TDs of the transfer
  ActualLen      - Actual Transfered Len 
  DataToggle     - Data Toggle
  TimeOut        - TimeOut value in milliseconds
  TransferResult - Transfer result
  IsControl      - Is Control Transfer
  IsLow          - Is Low Speed Device
  
Returns:

  EFI_SUCCESS      - The transfer finished with success
  EFI_DEVICE_ERROR - Transfer failed

--*/
{
  UINT32                  ErrTdPos;
  UINTN                   Index;
  UINTN                   Delay;
  UINTN                   ScrollNum;

  ErrTdPos    = 0;
  *Result     = EFI_USB_NOERROR;
  *ActualLen  = 0;
  Delay       = (TimeOut * STALL_1_MS / 50) + 1;
  
  for (Index = 0; Index < Delay; Index++) {
    UhciCheckTdStatus (Td, Result, &ErrTdPos, ActualLen, IsLow);

    //
    // Transfer is OK or some error occured (TD inactive)
    //
    if ((*Result == EFI_USB_NOERROR) || ((*Result & EFI_USB_ERR_NOTEXECUTE) == 0)) {
      break;
    }

    gBS->Stall (50);
  }

  if (*Result != EFI_USB_NOERROR) {
    //
    // Roll the data toggle back to the last success TD.
    // No need to roll toggle for control transfer because
    // it always start a new data toggle sequence for new
    // transfer
    //
    if (!IsControl) {
      ScrollNum = UhciGetNumberOfTd (Td) - ErrTdPos;

      if ((ScrollNum % 2) != 0) {
        *DataToggle ^= 1;
      }
    }

    UHCI_DEBUG ((UHCI_DEBUG_SCHEDULE, "UhciExecuteTransfer failed:"
      "Result:0x%x, Error position: %d,  Data transferred: %d\n",
      *Result, ErrTdPos, *ActualLen));

    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

STATIC
VOID
UhciUpdateAsyncReq (
  IN UHCI_ASYNC_REQUEST  *AsyncReq,
  IN UINT32              Result,
  IN UINT32              ErrTdPos
  )
/*++

Routine Description:

  Update Async Request, QH and TDs

Arguments:

  AsyncReq  - The UHCI asynchronous transfer to update
  Result    - Transfer reslut
  ErrTdPos  - Error TD Position

Returns:

  VOID

--*/
{
  UHCI_QH_SW              *Qh;
  UHCI_TD_SW              *FirstTd;
  UHCI_TD_SW              *Td;
  UINT8                   DataToggle;
  UINT32                  Index;

  Qh          = AsyncReq->QhSw;
  FirstTd     = AsyncReq->FirstTd;
  DataToggle  = 0;

  if (Result == EFI_USB_NOERROR) { 
    //
    // The last transfer succeeds. Then we need to update
    // the Qh and Td for next round of transfer.
    // 1. Update the TD's data toggle
    // 2. Activate all the TDs
    // 3. Link the TD to the queue head again since during
    //    execution, queue head's TD pointer is changed by 
    //    hardware.
    //

    //
    // First find the last data toggle of the last successful
    // transfer. New round of transfer continues it.
    //
    for (Td = FirstTd; Td != NULL; Td = Td->NextTd) {
      DataToggle = (UINT8)Td->TdHw.DataToggle;
    }
    
    AsyncReq->DataToggle = DataToggle;

    //
    // Setup the TD for next round of transfer: continue the data
    // toggle sequence of the last round of transfer and set the
    // TD to active.
    //
    for (Td = FirstTd; Td != NULL; Td = Td->NextTd) {
      DataToggle         ^= 1;
      Td->TdHw.DataToggle = DataToggle;
      Td->TdHw.Status    |= USBTD_ACTIVE;
    }
    
    UhciLinkTdToQh (Qh, FirstTd);
    return ;
  }
  
  //
  // Update the data toggle if transfer met some error. 
  // No need to update TD if TD is still active or received 
  // some NAKs
  //
  if ((Result & USB_ERR_FAIL_MASK) != 0) {
    //
    // Find the first error TD, then update the asynchronous 
    // transfer's data toggle. This the next data toggle to use.
    //
    for (Td = FirstTd, Index = 0; Index < ErrTdPos; Index++) {
      Td = Td->NextTd;
    } 
    
    AsyncReq->DataToggle = (UINT8)Td->TdHw.DataToggle;
  }
}

EFI_STATUS
UhciCreateAsyncReq (
  IN USB_HC_DEV                       *Uhc,
  IN UHCI_QH_SW                       *Qh,
  IN UHCI_TD_SW                       *FirstTd,
  IN UINT8                            DevAddr,
  IN UINT8                            EndPoint,
  IN UINT8                            Toggle,
  IN UINTN                            DataLen,
  IN UINTN                            Interval,
  IN VOID                             *Mapping,
  IN UINT8                            *Data,
  IN EFI_ASYNC_USB_TRANSFER_CALLBACK  Callback,
  IN VOID                             *Context,
  IN BOOLEAN                          IsLow
  )
/*++

Routine Description:

  Create Async Request node, and Link to List 
  
Arguments:

  Uhc         - The UHCI device
  Qh          - The queue head of the transfer
  FirstTd     - First TD of the transfer
  DevAddr     - Device Address
  EndPoint    - EndPoint Address
  Toggle      - Data Toggle
  DataLen     - Data length 
  Interval    - Polling Interval when inserted to frame list
  Mapping     - Mapping value  
  Data        - Data buffer, unmapped
  Callback    - Callback after interrupt transfeer
  Context     - Callback Context passed as function parameter
  IsLow       - Is Low Speed
  
Returns:

  EFI_SUCCESS            - An asynchronous transfer is created
  EFI_INVALID_PARAMETER  - Paremeter is error 
  EFI_OUT_OF_RESOURCES   - Failed because of resource shortage.
  
--*/
{
  UHCI_ASYNC_REQUEST      *AsyncReq;

  AsyncReq = EfiLibAllocatePool (sizeof (UHCI_ASYNC_REQUEST));

  if (AsyncReq == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  //
  // Fill Request field. Data is allocated host memory, not mapped
  //
  AsyncReq->Signature   = UHCI_ASYNC_INT_SIGNATURE;
  AsyncReq->DevAddr     = DevAddr;
  AsyncReq->EndPoint    = EndPoint;
  AsyncReq->DataToggle  = Toggle;
  AsyncReq->DataLen     = DataLen;
  AsyncReq->Interval    = Interval;
  AsyncReq->Mapping     = Mapping;
  AsyncReq->Data        = Data;
  AsyncReq->Callback    = Callback;
  AsyncReq->Context     = Context;
  AsyncReq->QhSw        = Qh;
  AsyncReq->FirstTd     = FirstTd;
  AsyncReq->IsLow       = IsLow;

  //
  // Insert the new interrupt transfer to the head of the list.
  // The interrupt transfer's monitor function scans the whole
  // list from head to tail. The new interrupt transfer MUST be
  // added to the head of the list.
  //
  InsertHeadList (&(Uhc->AsyncIntList), &(AsyncReq->Link));

  return EFI_SUCCESS;
}


STATIC
VOID
UhciFreeAsyncReq (
  IN USB_HC_DEV           *Uhc,
  IN UHCI_ASYNC_REQUEST   *AsyncReq
  )
/*++
Routine Description:

  Free an asynchronous request's resource such as memory
  
Arguments:

  Uhc         - The UHCI device
  AsyncReq    - The asynchronous request to free

Returns:

  None
  
--*/

{
  ASSERT ((Uhc != NULL) && (AsyncReq != NULL));
  
  UhciDestoryTds (Uhc, AsyncReq->FirstTd);
  UhciFreePool   (Uhc, (UINT8 *)AsyncReq->QhSw, sizeof (UHCI_QH_SW));

  if (AsyncReq->Mapping != NULL) {
    Uhc->PciIo->Unmap (Uhc->PciIo, AsyncReq->Mapping);
  }
  
  if (AsyncReq->Data != NULL) {
    gBS->FreePool (AsyncReq->Data);
  }

  gBS->FreePool (AsyncReq);
}

STATIC
VOID
UhciUnlinkAsyncReq (
  IN USB_HC_DEV           *Uhc,
  IN UHCI_ASYNC_REQUEST   *AsyncReq,
  IN BOOLEAN              FreeNow
  )
/*++
Routine Description:

  Unlink an asynchronous request's from UHC's asynchronus list.
  also remove the queue head from the frame list. If FreeNow, 
  release its resource also. Otherwise, add the request to the 
  UHC's recycle list to wait for a while before release the memory.
  Until then, hardware won't hold point to the request.
  
Arguments:

  Uhc         - The UHCI device
  AsyncReq    - The asynchronous request to free
  FreeNow     - If TRUE, free the resource immediately, otherwise
                add the request to recycle wait list.
Returns:

  None
  
--*/

{
  ASSERT ((Uhc != NULL) && (AsyncReq != NULL));

  RemoveEntryList (&(AsyncReq->Link));
  UhciUnlinkQhFromFrameList (Uhc->FrameBase, AsyncReq->QhSw);

  if (FreeNow) {
    UhciFreeAsyncReq (Uhc, AsyncReq);
  } else {
    //
    // To sychronize with hardware, mark the queue head as inactive
    // then add AsyncReq to UHC's recycle list
    //
    AsyncReq->QhSw->QhHw.VerticalLink = QH_VLINK (NULL, TRUE);
    AsyncReq->Recycle = Uhc->RecycleWait;
    Uhc->RecycleWait  = AsyncReq;
  }
}

EFI_STATUS
UhciRemoveAsyncReq (
  IN  USB_HC_DEV          *Uhc,
  IN  UINT8               DevAddr,
  IN  UINT8               EndPoint,
  OUT UINT8               *Toggle
  )
/*++
Routine Description:

  Delete Async Interrupt QH and TDs
  
Arguments:

  Uhc         - The UHCI device
  DevAddr     - Device Address
  EndPoint    - EndPoint Address
  Toggle      - The next data toggle to use

Returns:

  EFI_SUCCESS            - The request is deleted
  EFI_INVALID_PARAMETER  - Paremeter is error 
  EFI_NOT_FOUND         - The asynchronous isn't found

--*/
{
  EFI_STATUS          Status;
  UINT32              Result;
  UHCI_ASYNC_REQUEST  *AsyncReq;
  EFI_LIST_ENTRY      *Link;
  UINT32              ErrTdPos;
  UINTN               ActualLen;
  BOOLEAN             Found;

  Status = EFI_SUCCESS;

  //
  // If no asynchronous interrupt transaction exists
  //
  if (IsListEmpty (&(Uhc->AsyncIntList))) {
    return EFI_SUCCESS;
  }
  
  //
  // Find the asynchronous transfer to this device/endpoint pair
  //
  Found = FALSE;
  Link  = Uhc->AsyncIntList.ForwardLink;

  do {
    AsyncReq  = UHCI_ASYNC_INT_FROM_LINK (Link);
    Link      = Link->ForwardLink;

    if ((AsyncReq->DevAddr == DevAddr) && (AsyncReq->EndPoint == EndPoint)) {
      Found = TRUE;
      break;
    }
    
  } while (Link != &(Uhc->AsyncIntList));

  if (!Found) {
    return EFI_NOT_FOUND;
  }
  
  //
  // Check the result of the async transfer then update it
  // to get the next data toggle to use.
  //
  UhciCheckTdStatus (
    AsyncReq->FirstTd, 
    &Result, 
    &ErrTdPos, 
    &ActualLen, 
    AsyncReq->IsLow
    );
  
  UhciUpdateAsyncReq (AsyncReq, Result, ErrTdPos);
  *Toggle = AsyncReq->DataToggle;

  //
  // Don't release the request now, keep it to synchronize with hardware.
  //
  UhciUnlinkAsyncReq (Uhc, AsyncReq, FALSE);
  return Status;
}

STATIC
VOID
UhciRecycleAsyncReq (
  IN USB_HC_DEV           *Uhc
  )
/*++
Routine Description:

  Recycle the asynchronouse request. When a queue head
  is unlinked from frame list, host controller hardware
  may still hold a cached pointer to it. To synchronize 
  with hardware, the request is released in two steps:
  first it is linked to the UHC's RecycleWait list. At 
  the next time UhciMonitorAsyncReqList is fired, it is
  moved to UHC's Recylelist. Then, at another timer 
  activation, all the requests on Recycle list is freed.
  This guarrantes that each unlink queue head keeps  
  existing for at least 50ms, far enough for the hardware
  to clear its cache.
  
Arguments:

  Uhc         - The UHCI device

Returns:

  None
  
--*/

{
  UHCI_ASYNC_REQUEST      *Req;
  UHCI_ASYNC_REQUEST      *Next;

  Req = Uhc->Recycle;
  
  while (Req != NULL) {
    Next = Req->Recycle;
    UhciFreeAsyncReq (Uhc, Req);    
    Req  = Next;
  }

  Uhc->Recycle     = Uhc->RecycleWait;
  Uhc->RecycleWait = NULL;
}


VOID
UhciFreeAllAsyncReq (
  IN USB_HC_DEV           *Uhc
  )
/*++

Routine Description:

  Release all the asynchronous transfers on the lsit.
  
Arguments:

  Uhc   - The UHCI device

Returns:

  VOID

--*/
{
  EFI_LIST_ENTRY          *Head;
  UHCI_ASYNC_REQUEST      *AsyncReq;

  //
  // Call UhciRecycleAsyncReq twice. The requests on Recycle 
  // will be released at the first call; The requests on 
  // RecycleWait will be released at the second call.
  //
  UhciRecycleAsyncReq (Uhc);
  UhciRecycleAsyncReq (Uhc);
  
  Head = &(Uhc->AsyncIntList);

  if (IsListEmpty (Head)) {
    return;
  }

  while (!IsListEmpty (Head)) {
    AsyncReq  = UHCI_ASYNC_INT_FROM_LINK (Head->ForwardLink);
    UhciUnlinkAsyncReq (Uhc, AsyncReq, TRUE);
  }
}

VOID
UhciMonitorAsyncReqList (
  IN EFI_EVENT            Event,
  IN VOID                 *Context
  )
/*++

  Routine Description:
  
    Interrupt transfer periodic check handler
    
  Arguments:
  
    Event   - The event of the time
    Context - Context of the event, pointer to USB_HC_DEV
    
  Returns:
  
    VOID
    
--*/
{
  UHCI_ASYNC_REQUEST      *AsyncReq;
  EFI_LIST_ENTRY          *Link;
  USB_HC_DEV              *Uhc;
  UINT32                  Result;
  VOID                    *Data;
  UINTN                   ActualLen;
  UINT32                  ErrTdPos;

  Uhc = (USB_HC_DEV *) Context;

  //
  // Recycle the asynchronous requests expired, and promote 
  // requests waiting to be recycled the next time when this 
  // timer expires
  //
  UhciRecycleAsyncReq (Uhc);

  if (IsListEmpty (&(Uhc->AsyncIntList))) {
    return ;
  }

  //
  // This loop must be delete safe
  //
  Link = Uhc->AsyncIntList.ForwardLink;

  do {
    Result    = 0;
    ErrTdPos  = 0;
    ActualLen = 0;
    
    AsyncReq  = UHCI_ASYNC_INT_FROM_LINK (Link);
    Link      = Link->ForwardLink;

    UhciCheckTdStatus (
      AsyncReq->FirstTd,
      &Result,
      &ErrTdPos,
      &ActualLen,
      AsyncReq->IsLow
      );

    if (ActualLen != 0) {
      UHCI_DEBUG ((UHCI_DEBUG_ASYC_INT, "UhciMonitorAsyncReq: actual data "
                                        "length transferred: %d\n", ActualLen));
    }
    
    //
    // check the next transfer if current hasn't finished.
    //
    if ((Result != EFI_USB_NOERROR) && ((Result & USB_ERR_FAIL_MASK) == 0)) {
      continue;
    }
    
    //
    // Copy the data to temporary buffer if there are some 
    // data transferred. We may have zero-length packet
    //
    Data = NULL;

    if (ActualLen != 0) {
      Data = EfiLibAllocatePool (ActualLen);

      if (Data == NULL) {
        return ;
      }

      EfiCopyMem (Data, AsyncReq->FirstTd->Data, ActualLen);
    }
    
    //
    // Now, either transfer is SUCCESS or met errors since
    // we have skipped to next transfer earlier if current
    // transfer is still active. 
    //
    if (Result == EFI_USB_NOERROR) {
      AsyncReq->Callback (Data, ActualLen, AsyncReq->Context, Result);
      UhciUpdateAsyncReq (AsyncReq, Result, ErrTdPos);
    } else {
      //
      // Leave error recovery to its related device driver.
      // A common case of the error recovery is to re-submit
      // the interrupt transfer. When an interrupt transfer
      // is re-submitted, its position in the linked list is
      // changed. It is inserted to the head of the linked
      // list, while this function scans the whole list from
      // head to tail. Thus, the re-submitted interrupt transfer's
      // callback function will not be called again in this round.
      //
      AsyncReq->Callback (NULL, 0, AsyncReq->Context, Result);
    }
    
    if (Data != NULL) {
      gBS->FreePool (Data);
    }
  } while (Link != &(Uhc->AsyncIntList));
}
