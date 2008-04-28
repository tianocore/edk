/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  UhciQueue.c

Abstract:

  The UHCI register operation routines.

Revision History

--*/

#include "Uhci.h"

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
{
  ASSERT ((Qh != NULL) && (Td != NULL));

  Qh->QhHw.VerticalLink = QH_VLINK (Td, FALSE);
  Qh->TDs               = (VOID *) Td;
}

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
{
  ASSERT ((Qh != NULL) && (Td != NULL));

  Qh->QhHw.VerticalLink = QH_VLINK (NULL, TRUE);
  Qh->TDs               = NULL;
}

STATIC
VOID
UhciAppendTd (
  IN UHCI_TD_SW     *PrevTd,
  IN UHCI_TD_SW     *ThisTd
  )
/*++

Routine Description:

  Append a new TD To the previous TD

Arguments:

  PrevTd  - Previous UHCI_TD_SW to be linked to
  ThisTd  - TD to link
  
Returns:

  VOID

--*/
{
  ASSERT ((PrevTd != NULL) && (ThisTd != NULL));

  PrevTd->TdHw.NextLink = TD_LINK (ThisTd, TRUE, FALSE);
  PrevTd->NextTd        = (VOID *) ThisTd;
}

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
{
  UHCI_TD_SW            *NextTd;
  UHCI_TD_SW            *ThisTd;

  NextTd = FirstTd;

  while (NextTd != NULL) {
    ThisTd  = NextTd;
    NextTd  = ThisTd->NextTd;
    UsbHcFreeMem (Uhc->MemPool, ThisTd, sizeof (UHCI_TD_SW));
  }
}

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
{
  UHCI_QH_SW            *Qh;

  Qh = UsbHcAllocateMem (Uhc->MemPool, sizeof (UHCI_QH_SW));
  
  if (Qh == NULL) {
    return NULL;
  }

  Qh->QhHw.HorizonLink  = QH_HLINK (NULL, TRUE);
  Qh->QhHw.VerticalLink = QH_VLINK (NULL, TRUE);
  Qh->Interval          = UhciConvertPollRate(Interval);
  Qh->TDs               = NULL;
  Qh->NextQh            = NULL;

  return Qh;
}

STATIC
UHCI_TD_SW *
UhciCreateTd (
  IN  USB_HC_DEV          *Uhc
  )
/*++

Routine Description:

  Create and intialize a TD

Arguments:

  Uhc   - The UHCI device

Returns:

  The newly allocated and initialized TD
  
--*/
{
  UHCI_TD_SW              *Td;

  Td     = UsbHcAllocateMem (Uhc->MemPool, sizeof (UHCI_TD_SW));
  if (Td == NULL) {
    return NULL;
  }

  Td->TdHw.NextLink = TD_LINK (NULL, FALSE, TRUE);
  Td->NextTd        = NULL;
  Td->Data          = NULL;
  Td->DataLen       = 0;

  return Td;
}

STATIC
UHCI_TD_SW *
UhciCreateSetupTd (
  IN  USB_HC_DEV          *Uhc,
  IN  UINT8               DevAddr,
  IN  UINT8               *Request,
  IN  BOOLEAN             IsLow
  )
/*++

Routine Description:

  Create and initialize a TD for Setup Stage of a control transfer

Arguments:

  Uhc         - The UHCI device 
  DevAddr     - Device address
  Request     - Device request
  IsLow       - Full speed or low speed
  
Returns:

  The created setup Td Pointer

--*/
{
  UHCI_TD_SW              *Td;

  Td = UhciCreateTd (Uhc);

  if (Td == NULL) {
    return NULL;
  }

  Td->TdHw.NextLink     = TD_LINK (NULL, TRUE, TRUE);
  Td->TdHw.ShortPacket  = FALSE;
  Td->TdHw.IsIsoch      = FALSE;
  Td->TdHw.IntOnCpl     = FALSE;
  Td->TdHw.ErrorCount   = 0x03;
  Td->TdHw.Status      |= USBTD_ACTIVE;
  Td->TdHw.DataToggle   = 0;
  Td->TdHw.EndPoint     = 0;
  Td->TdHw.LowSpeed     = IsLow ? 1 : 0;
  Td->TdHw.DeviceAddr   = DevAddr & 0x7F;
  Td->TdHw.MaxPacketLen = (UINT32) (sizeof (EFI_USB_DEVICE_REQUEST) - 1);
  Td->TdHw.PidCode      = SETUP_PACKET_ID;
  Td->TdHw.DataBuffer   = (UINT32) (UINTN) Request;

  Td->Data              = Request;
  Td->DataLen           = sizeof (EFI_USB_DEVICE_REQUEST);

  return Td;
}

STATIC
UHCI_TD_SW *
UhciCreateDataTd (
  IN  USB_HC_DEV          *Uhc,
  IN  UINT8               DevAddr,
  IN  UINT8               Endpoint,
  IN  UINT8               *DataPtr,
  IN  UINTN               Len,
  IN  UINT8               PktId,
  IN  UINT8               Toggle,
  IN  BOOLEAN             IsLow
  )
/*++

Routine Description:

  Create a TD for data

Arguments:

  Uhc         - The UHCI device
  DevAddr     - Device address
  Endpoint    - Endpoint number 
  DataPtr     - Data buffer 
  Len         - Data length
  PktId       - Packet ID
  Toggle      - Data toggle value
  IsLow       - Full speed or low speed
  
Returns:

  Data Td pointer if success, otherwise NUL

--*/
{
  UHCI_TD_SW  *Td;

  //
  // Code as length - 1, and the max valid length is 0x500
  //
  ASSERT (Len <= 0x500);

  Td  = UhciCreateTd (Uhc);
  
  if (Td == NULL) {
    return NULL;
  }

  Td->TdHw.NextLink     = TD_LINK (NULL, TRUE, TRUE);
  Td->TdHw.ShortPacket  = FALSE;
  Td->TdHw.IsIsoch      = FALSE;
  Td->TdHw.IntOnCpl     = FALSE;
  Td->TdHw.ErrorCount   = 0X03;
  Td->TdHw.Status       = USBTD_ACTIVE;
  Td->TdHw.LowSpeed     = IsLow ? 1 : 0;
  Td->TdHw.DataToggle   = Toggle & 0x01;
  Td->TdHw.EndPoint     = Endpoint & 0x0F;
  Td->TdHw.DeviceAddr   = DevAddr & 0x7F;
  Td->TdHw.MaxPacketLen = (UINT32) (Len - 1);
  Td->TdHw.PidCode      = (UINT8) PktId;
  Td->TdHw.DataBuffer   = (UINT32) (UINTN) DataPtr;

  Td->Data              = DataPtr;
  Td->DataLen           = (UINT16) Len;

  return Td;
}

STATIC
UHCI_TD_SW *
UhciCreateStatusTd (
  IN  USB_HC_DEV          *Uhc,
  IN  UINT8               DevAddr,
  IN  UINT8               PktId,
  IN  BOOLEAN             IsLow
  )
/*++

Routine Description:

  Create TD for the Status Stage of control transfer

Arguments:

  Uhc         - The UHCI device
  DevAddr     - Device address
  PktId       - Packet ID
  IsLow       - Full speed or low speed
  
Returns:

  Status Td Pointer

--*/
{
  UHCI_TD_SW              *Td;

  Td = UhciCreateTd (Uhc);

  if (Td == NULL) {
    return NULL;
  }

  Td->TdHw.NextLink     = TD_LINK (NULL, TRUE, TRUE);
  Td->TdHw.ShortPacket  = FALSE;
  Td->TdHw.IsIsoch      = FALSE;
  Td->TdHw.IntOnCpl     = FALSE;
  Td->TdHw.ErrorCount   = 0x03;
  Td->TdHw.Status      |= USBTD_ACTIVE;
  Td->TdHw.MaxPacketLen = 0x7FF;      //0x7FF: there is no data (refer to UHCI spec)
  Td->TdHw.DataToggle   = 1;
  Td->TdHw.EndPoint     = 0;
  Td->TdHw.LowSpeed     = IsLow ? 1 : 0;
  Td->TdHw.DeviceAddr   = DevAddr & 0x7F;
  Td->TdHw.PidCode      = (UINT8) PktId;
  Td->TdHw.DataBuffer   = (UINT32) (UINTN) NULL;

  Td->Data              = NULL;
  Td->DataLen           = 0;

  return Td;
}

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
{
  UHCI_TD_SW                *SetupTd;
  UHCI_TD_SW                *FirstDataTd;
  UHCI_TD_SW                *DataTd;
  UHCI_TD_SW                *PrevDataTd;
  UHCI_TD_SW                *StatusTd;
  UINT8                     DataToggle;
  UINT8                     StatusPktId;
  UINTN                     ThisTdLen;


  DataTd      = NULL;
  SetupTd     = NULL;
  FirstDataTd = NULL;
  PrevDataTd  = NULL;
  StatusTd    = NULL;

  //
  // Create setup packets for the transfer
  //
  SetupTd = UhciCreateSetupTd (Uhc, DeviceAddr, Request, IsLow);

  if (SetupTd == NULL) {
    return NULL;
  }
  
  //
  // Create data packets for the transfer
  //
  DataToggle = 1;

  while (DataLen > 0) {
    //
    // PktSize is the data load size in each Td.
    //
    ThisTdLen = (DataLen > MaxPacket ? MaxPacket : DataLen);

    DataTd = UhciCreateDataTd (
               Uhc,
               DeviceAddr,
               0,
               Data,
               ThisTdLen,
               DataPktId,
               DataToggle,
               IsLow
               );

    if (DataTd == NULL) {
      goto FREE_TD;
    }

    if (FirstDataTd == NULL) {
      FirstDataTd         = DataTd;
      FirstDataTd->NextTd = NULL;
    } else {
      UhciAppendTd (PrevDataTd, DataTd);
    }

    DataToggle ^= 1;
    PrevDataTd = DataTd;
    Data += ThisTdLen;
    DataLen -= ThisTdLen;
  }
  
  //
  // Status packet is on the opposite direction to data packets
  //
  if (OUTPUT_PACKET_ID == DataPktId) {
    StatusPktId = INPUT_PACKET_ID;
  } else {
    StatusPktId = OUTPUT_PACKET_ID;
  }

  StatusTd = UhciCreateStatusTd (Uhc, DeviceAddr, StatusPktId, IsLow);

  if (StatusTd == NULL) {
    goto FREE_TD;
  }
  
  //
  // Link setup Td -> data Tds -> status Td together
  //
  if (FirstDataTd != NULL) {
    UhciAppendTd (SetupTd, FirstDataTd);
    UhciAppendTd (PrevDataTd, StatusTd);
  } else {
    UhciAppendTd (SetupTd, StatusTd);
  }

  return SetupTd;

FREE_TD:
  if (SetupTd != NULL) {
    UhciDestoryTds (Uhc, SetupTd);
  }

  if (FirstDataTd != NULL) {
    UhciDestoryTds (Uhc, FirstDataTd);
  }

  return NULL;
}

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
{
  UHCI_TD_SW              *DataTd;
  UHCI_TD_SW              *FirstDataTd;
  UHCI_TD_SW              *PrevDataTd;
  UINTN                   ThisTdLen;

  DataTd      = NULL;
  FirstDataTd = NULL;
  PrevDataTd  = NULL;

  //
  // Create data packets for the transfer
  //
  while (DataLen > 0) {
    //
    // PktSize is the data load size that each Td.
    //
    ThisTdLen = DataLen;

    if (DataLen > MaxPacket) {
      ThisTdLen = MaxPacket;
    }

    DataTd = UhciCreateDataTd (
               Uhc,
               DevAddr,
               EndPoint,
               Data,
               ThisTdLen,
               PktId,
               *DataToggle,
               IsLow
               );

    if (DataTd == NULL) {
      goto FREE_TD;
    }

    if (PktId == INPUT_PACKET_ID) {
      DataTd->TdHw.ShortPacket = TRUE;
    }

    if (FirstDataTd == NULL) {
      FirstDataTd         = DataTd;
      FirstDataTd->NextTd = NULL;
    } else {
      UhciAppendTd (PrevDataTd, DataTd);
    }

    *DataToggle ^= 1;
    PrevDataTd   = DataTd;
    Data        += ThisTdLen;
    DataLen     -= ThisTdLen;
  }

  return FirstDataTd;

FREE_TD:
  if (FirstDataTd != NULL) {
    UhciDestoryTds (Uhc, FirstDataTd);
  }

  return NULL;
}
