/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  UhciDebug.c

Abstract:

  This file provides the information dump support for Uhci when in debug mode. 
  You can dynamically adjust the debug level by changing variable gEHCDebugLevel 
  and gEHCErrorLevel.

Revision History

--*/

#include "Uhci.h"
#include "UhciDebug.h"

#ifdef EFI_DEBUG

UINTN gUhciDebugLevel = USB_DEBUG_ERROR | USB_DEBUG_FORCE_OUTPUT | USB_DEBUG_PROMT;

EFI_STATUS
EFIAPI
UhciGetRootHubPortStatus (
  IN  EFI_USB_HC_PROTOCOL     *This,
  IN  UINT8                   PortNumber,
  OUT EFI_USB_PORT_STATUS     *PortStatus
  );

VOID
UhciDebug (
  IN  UINTN         Mask,
  IN  CHAR8         *Format,
  ...
  )
/*++

Routine Description:

  Debug print interface for UHCI

Arguments:

  Mask    - Level to control debug print
  Format  - String to use for the print, followed by print arguments

Returns:

  None

--*/
{
  VA_LIST Marker;

  if (Mask & gUhciDebugLevel) {
    VA_START (Marker, Format);
    EfiDebugVPrint (EFI_D_ERROR, Format, Marker);
    VA_END (Marker);
  } else {
    VA_START (Marker, Format);
    EfiDebugVPrint (EFI_D_INFO, Format, Marker);
    VA_END (Marker);
  }
}

VOID
UhciDumpBuffer (
  IN UINT8   *Buffer,
  IN UINT32  Length
  )
/*++

Routine Description:

  Dump the content of data buffer

Arguments:

  Buffer  - Start pointer of data buffer
  Length  - Length of data buffer

Returns: 

  None
  
--*/

{
  UINT32  Index;

  Index = 0;
  UhciDebug (UHCI_DEBUG_BUFFER, "&BUFFER = 0x%x, Length = %d\n", Buffer, Length);
  while (Index < Length) {
    UhciDebug (UHCI_DEBUG_BUFFER, "0x%x ", *(Buffer + Index));
    Index++;
    if (Index == Length) {
      break;
    }

    if (Index % 0x10 == 0) {
      UhciDebug (UHCI_DEBUG_BUFFER, "\n");
    }
  }

}

VOID
UhciDumpPortsStatus (
  IN USB_HC_DEV    *HcDev,
  IN UINT8         PortNumber
  )
/*++

Routine Description:

  Dump the content of port status register

Arguments:

  HcDev       - Host controller structure
  PortNumber  - Index of port

Returns:

  None

--*/
{
  EFI_USB_PORT_STATUS PortStatus;

  UhciGetRootHubPortStatus (&(HcDev->UsbHc), PortNumber, &PortStatus);
  UhciDebug (
    UHCI_DEBUG_PORT_STS,
    "PortStatus[ChangeStatus|Status] = [0x%x|0x%x]\n",
    PortNumber,
    PortStatus.PortChangeStatus,
    PortStatus.PortStatus
    );

}

VOID
UhciDumpQh (
  IN UHCI_QH_SW    *QhSw
  )
/*++

Routine Description:

  Dump the content of QH structure

Arguments:

  QhSw  - Pointer to software QH structure

Returns:

  None

--*/
{
  UhciDebug (UHCI_DEBUG_QH, "&QhSw = 0x%x\n", QhSw);
  UhciDebug (UHCI_DEBUG_QH, "QhSw.NextQh = 0x%x\n", QhSw->NextQh);
  UhciDebug (UHCI_DEBUG_QH, "QhSw.TDs = 0x%x\n", QhSw->TDs);
  UhciDebug (UHCI_DEBUG_QH, "QhSw.QhHw:\n");
  UhciDebug (UHCI_DEBUG_QH, "Horizontal Link: %x \n", QhSw->QhHw.HorizonLink);
  UhciDebug (UHCI_DEBUG_QH, "Vertical Link: %x\n", QhSw->QhHw.VerticalLink);
}

VOID
UhciDumpTds (
  IN UHCI_TD_SW    *TdSw,
  IN BOOLEAN       IsCur
  )
/*++

Routine Description:

  Dump the content of TD structure.

Arguments:

  TdSw  - Pointer to software TD structure
  IsCur - Whether dump the whole list, or only dump the current TD

Returns:

  None

--*/
{
  UHCI_TD_SW  *CurTdSw;

  CurTdSw = TdSw;
  while (CurTdSw != NULL && !IsCur) {
    UhciDebug (UHCI_DEBUG_TD, "&TdSw = 0x%x\n", CurTdSw);
    UhciDebug (UHCI_DEBUG_TD, "TdSw.NextTd = 0x%x\n", CurTdSw->NextTd);
    UhciDebug (UHCI_DEBUG_TD, "TdSw.DataLen = 0x%x\n", CurTdSw->DataLen);
    UhciDebug (UHCI_DEBUG_TD, "TdSw.Data = 0x%x\n", CurTdSw->Data);
    UhciDebug (UHCI_DEBUG_TD, "TdSw.TdHw:\n");
    UhciDebug (UHCI_DEBUG_TD, "[Next LinkPointer] = [0x%x]\n", CurTdSw->TdHw.NextLink);
    UhciDebug (
      UHCI_DEBUG_TD,
      "[SPD|C_ERR|LS|ISO|IOC|Status|ActLen] = [0x%x|0x%x|0x%x|0x%x|0x%x|0x%x|0x%x]\n",
      CurTdSw->TdHw.ShortPacket,
      CurTdSw->TdHw.ErrorCount,
      CurTdSw->TdHw.LowSpeed,
      CurTdSw->TdHw.IsIsoch,
      CurTdSw->TdHw.IntOnCpl,
      TdSw->TdHw.Status,
      TdSw->TdHw.ActualLen
      );
    UhciDebug (
      UHCI_DEBUG_TD,
      "[MaxLen|DataToggle|EndPt|PID] = [0x%x|0x%x|0x%x|0x%x]\n",
      CurTdSw->TdHw.MaxPacketLen,
      CurTdSw->TdHw.DataToggle,
      CurTdSw->TdHw.EndPoint,
      CurTdSw->TdHw.PidCode
      );
    UhciDebug (UHCI_DEBUG_TD, "[BufferPointer] = [0x%x]\n", CurTdSw->TdHw.DataBuffer);

    CurTdSw = CurTdSw->NextTd;
  }

}

#endif
