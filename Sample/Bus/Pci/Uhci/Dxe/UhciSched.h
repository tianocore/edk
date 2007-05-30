/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  UhciSched.h

Abstract:

  The definition for EHCI register operation routines.

Revision History

--*/

#ifndef _EFI_UHCI_SCHED_H_
#define _EFI_UHCI_SCHED_H_

enum {
  UHCI_ASYNC_INT_SIGNATURE = EFI_SIGNATURE_32 ('u', 'h', 'c', 'a'),

  //
  // The failure mask for USB transfer return status. If any of
  // these bit is set, the transfer failed. EFI_USB_ERR_NOEXECUTE
  // and EFI_USB_ERR_NAK are not considered as error condition:
  // the transfer is still going on.
  //
  USB_ERR_FAIL_MASK = EFI_USB_ERR_STALL   | EFI_USB_ERR_BUFFER | 
                      EFI_USB_ERR_BABBLE  | EFI_USB_ERR_CRC    |
                      EFI_USB_ERR_TIMEOUT | EFI_USB_ERR_BITSTUFF |
                      EFI_USB_ERR_SYSTEM,

};

EFI_FORWARD_DECLARATION (UHCI_ASYNC_REQUEST);

//
// Structure used to manager the asynchronous interrupt transfers.
//
typedef struct _UHCI_ASYNC_REQUEST{
  UINTN                           Signature;
  EFI_LIST_ENTRY                  Link;
  UHCI_ASYNC_REQUEST              *Recycle;
  
  //
  // Endpoint attributes
  //
  UINT8                           DevAddr;
  UINT8                           EndPoint;
  BOOLEAN                         IsLow;
  UINTN                           Interval;
  UINT8                           DataToggle; // The last data toggle of the TD

  //
  // Data and UHC structures
  //
  UHCI_QH_SW                      *QhSw;
  UHCI_TD_SW                      *FirstTd;
  UINT8                           *Data;      // Allocated host memory, not mapped memory
  UINTN                           DataLen;
  VOID                            *Mapping;

  //
  // User callback and its context
  //
  EFI_ASYNC_USB_TRANSFER_CALLBACK Callback;
  VOID                            *Context;
} UHCI_ASYNC_REQUEST;

#define UHCI_ASYNC_INT_FROM_LINK(a) \
          CR (a, UHCI_ASYNC_REQUEST, Link, UHCI_ASYNC_INT_SIGNATURE)

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

#endif
