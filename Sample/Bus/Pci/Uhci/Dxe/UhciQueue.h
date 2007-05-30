/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  UhciQueue.h

Abstract:

  The definition for UHCI register operation routines.

Revision History

--*/

#ifndef _EFI_UHCI_QUEUE_H_
#define _EFI_UHCI_QUEUE_H_

//
// Macroes used to set various links in UHCI's driver.
// In this UHCI driver, QH's horizontal link always pointers to other QH,
// and its vertical link always pointers to TD. TD's next pointer always
// pointers to other sibling TD. Frame link always pointers to QH because
// ISO transfer isn't supported.
//
// We should use UINT32 to access these pointers to void race conditions
// with hardware.
//
#define QH_HLINK(Pointer, Terminate)  \
        (((UINT32) ((UINTN) (Pointer)) & 0xFFFFFFF0) | 0x02 | ((Terminate) ? 0x01 : 0))

#define QH_VLINK(Pointer, Terminate)  \
        (((UINT32) ((UINTN) (Pointer)) & 0xFFFFFFF0) | ((Terminate) ? 0x01 : 0))

#define TD_LINK(Pointer, VertFirst, Terminate) \
        (((UINT32) ((UINTN) (Pointer)) & 0xFFFFFFF0) | \
         ((VertFirst) ? 0x04 : 0) | ((Terminate) ? 0x01 : 0))

#define LINK_TERMINATED(Link) (((Link) & 0x01) != 0)

#define UHCI_ADDR(QhOrTd)     ((VOID *) (UINTN) ((QhOrTd) & 0xFFFFFFF0))

#pragma pack(1)
//
// Both links in QH has this internal structure:
//   Next pointer: 28, Reserved: 2, NextIsQh: 1, Terminate: 1
// This is the same as frame list entry.
//
typedef struct {
  UINT32              HorizonLink;
  UINT32              VerticalLink;
} UHCI_QH_HW;

//
// Next link in TD has this internal structure:
//   Next pointer: 28, Reserved: 1, Vertical First: 1, NextIsQh: 1, Terminate: 1
//
typedef struct {
  UINT32              NextLink;
  UINT32              ActualLen   : 11;
  UINT32              Reserved1   : 5;
  UINT32              Status      : 8;
  UINT32              IntOnCpl    : 1;
  UINT32              IsIsoch     : 1;
  UINT32              LowSpeed    : 1;
  UINT32              ErrorCount  : 2;
  UINT32              ShortPacket : 1;
  UINT32              Reserved2   : 2;
  UINT32              PidCode     : 8;
  UINT32              DeviceAddr  : 7;
  UINT32              EndPoint    : 4;
  UINT32              DataToggle  : 1;
  UINT32              Reserved3   : 1;
  UINT32              MaxPacketLen: 11;
  UINT32              DataBuffer;
} UHCI_TD_HW;
#pragma pack()

EFI_FORWARD_DECLARATION (UHCI_TD_SW);
EFI_FORWARD_DECLARATION (UHCI_QH_SW);

typedef struct _UHCI_QH_SW {
  UHCI_QH_HW        QhHw;
  UHCI_QH_SW        *NextQh;
  UHCI_TD_SW        *TDs;
  UINTN             Interval;
} UHCI_QH_SW;

typedef struct _UHCI_TD_SW {
  UHCI_TD_HW        TdHw;
  UHCI_TD_SW        *NextTd;
  UINT8             *Data;
  UINT16            DataLen;
} UHCI_TD_SW;

VOID
UhciLinkTdToQh (
  IN UHCI_QH_SW           *Qh,
  IN UHCI_TD_SW           *Td
  )
/*++

Routine Description:

  Link the TD To QH

Arguments:

  Qh  - The queue head for the TD to link to
  Td  - The TD to link
  
Returns:

  VOID

--*/
;

VOID
UhciUnlinkTdFromQh (
  IN UHCI_QH_SW           *Qh,
  IN UHCI_TD_SW           *Td
  )
/*++

Routine Description:

  Unlink TD from the QH

Arguments:

  Qh  - The queue head to unlink from
  Td  - The TD to unlink 
  
Returns:

  VOID

--*/
;

VOID
UhciDestoryTds (
  IN USB_HC_DEV           *Uhc,
  IN UHCI_TD_SW           *FirstTd
  )
/*++
Routine Description:

  Delete a list of TDs
  
Arguments:

  Uhc       - The UHCI device
  FirstTd   - TD link list head

Returns:

  VOID

--*/
;

UHCI_QH_SW *
UhciCreateQh (
  IN  USB_HC_DEV        *Uhc,
  IN  UINTN             Interval
  )
/*++

Routine Description:

  Create an initialize a new queue head

Arguments:

  Uhc       - The UHCI device
  Interval  - The polling interval for the queue
  
Returns:

  The newly created queue header
  
--*/
;

UHCI_TD_SW *
UhciCreateCtrlTds (
  IN USB_HC_DEV           *Uhc,
  IN UINT8                DeviceAddr,
  IN UINT8                DataPktId,
  IN UINT8                *Request,
  IN UINT8                *Data,
  IN UINTN                DataLen,
  IN UINT8                MaxPacket,
  IN BOOLEAN              IsLow
  )
/*++

Routine Description:

  Create Tds list for Control Transfer

Arguments:

  Uhc         - The UHCI device
  DeviceAddr  - The device address
  DataPktId   - Packet Identification of Data Tds
  Request     - A pointer to request structure buffer to transfer
  Data        - A pointer to user data buffer to transfer
  DataLen     - Length of user data to transfer
  MaxPacket   - Maximum packet size for control transfer
  IsLow       - Full speed or low speed
  
Returns:

  The Td list head for the control transfer
  
--*/
;

UHCI_TD_SW *
UhciCreateBulkOrIntTds (
  IN USB_HC_DEV           *Uhc,
  IN UINT8                DevAddr,
  IN UINT8                EndPoint,
  IN UINT8                PktId,
  IN UINT8                *Data,
  IN UINTN                DataLen,
  IN OUT UINT8            *DataToggle,
  IN UINT8                MaxPacket,
  IN BOOLEAN              IsLow
  )
/*++

Routine Description:

  Create Tds list for Bulk/Interrupt Transfer

Arguments:

  Uhc          - USB_HC_DEV
  DevAddr      - Address of Device
  EndPoint     - Endpoint Number
  PktId        - Packet Identification of Data Tds
  Data         - A pointer to user data buffer to transfer
  DataLen      - Length of user data to transfer
  DataToggle   - Data Toggle Pointer
  MaxPacket    - Maximum packet size for Bulk/Interrupt transfer
  IsLow         - Is Low Speed Device
  
Returns:

  The Tds list head for the bulk transfer
  
--*/
;

#endif
