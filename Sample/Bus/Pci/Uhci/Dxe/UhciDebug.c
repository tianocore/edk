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

UINTN mUhciDebugMask = USB_DEBUG_FORCE_OUTPUT;

VOID
UhciDebug (
  IN  CHAR8               *Format,
  ...
  )
/*++

Routine Description:

  Debug debug print interface for UHCI

Arguments:

  Format  - String to use for the print, followed by print arguments

Returns:

  None

--*/
{
  VA_LIST                 Marker;

  VA_START (Marker, Format);
  EfiDebugVPrint (EFI_D_INFO, Format, Marker);
  VA_END (Marker); 
}

VOID
UhciError (
  IN  CHAR8               *Format,
  ...
  )
/*++

Routine Description:

  Debug error print interface for UHCI

Arguments:

  Format  - String to use for the print, followed by print arguments

Returns:

  None

--*/
{
  VA_LIST                 Marker;

  VA_START (Marker, Format);
  EfiDebugVPrint (EFI_D_ERROR, Format, Marker);
  VA_END (Marker); 
}


VOID
UhciDebugPrint (
  IN  UINTN               Level,
  IN  CHAR8               *Format,
  ...
  )
/*++

Routine Description:

  Debug print interface for UHCI

Arguments:

  Level   - Level to control debug print
  Format  - String to use for the print, followed by print arguments

Returns:

  None

--*/
{
  VA_LIST                 Marker;

  VA_START (Marker, Format);

  if (Level & mUhciDebugMask) {
    if (mUhciDebugMask & USB_DEBUG_FORCE_OUTPUT) {
      EfiDebugVPrint (EFI_D_ERROR, Format, Marker);
    } else {
      EfiDebugVPrint (EFI_D_INFO, Format, Marker);
    }
  }

  VA_END (Marker);
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
  UINTN                   Level;

  Level = UHCI_DEBUG_QH;
  
  UhciDebugPrint (Level, "&QhSw @ 0x%x\n", QhSw);
  UhciDebugPrint (Level, "QhSw.NextQh    - 0x%x\n", QhSw->NextQh);
  UhciDebugPrint (Level, "QhSw.TDs       - 0x%x\n", QhSw->TDs);
  UhciDebugPrint (Level, "QhSw.QhHw:\n");
  UhciDebugPrint (Level, " Horizon  Link - %x\n", QhSw->QhHw.HorizonLink);
  UhciDebugPrint (Level, " Vertical Link - %x\n\n", QhSw->QhHw.VerticalLink);
}

VOID
UhciDumpTds (
  IN UHCI_TD_SW           *TdSw
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
  UHCI_TD_SW              *CurTdSw;
  UINTN                   Level;

  Level   = UHCI_DEBUG_TD;
  CurTdSw = TdSw;

  while (CurTdSw != NULL) {
    UhciDebugPrint (Level, "TdSw @ 0x%x\n",           CurTdSw);
    UhciDebugPrint (Level, "TdSw.NextTd   - 0x%x\n",  CurTdSw->NextTd);
    UhciDebugPrint (Level, "TdSw.DataLen  - %d\n",    CurTdSw->DataLen);
    UhciDebugPrint (Level, "TdSw.Data     - 0x%x\n",  CurTdSw->Data);
    UhciDebugPrint (Level, "TdHw:\n");
    UhciDebugPrint (Level, " NextLink     - 0x%x\n",  CurTdSw->TdHw.NextLink);
    UhciDebugPrint (Level, " ActualLen    - %d\n",    CurTdSw->TdHw.ActualLen);
    UhciDebugPrint (Level, " Status       - 0x%x\n",  CurTdSw->TdHw.Status);
    UhciDebugPrint (Level, " IOC          - %d\n",    CurTdSw->TdHw.IntOnCpl);
    UhciDebugPrint (Level, " IsIsoCh      - %d\n",    CurTdSw->TdHw.IsIsoch);
    UhciDebugPrint (Level, " LowSpeed     - %d\n",    CurTdSw->TdHw.LowSpeed);
    UhciDebugPrint (Level, " ErrorCount   - %d\n",    CurTdSw->TdHw.ErrorCount);
    UhciDebugPrint (Level, " ShortPacket  - %d\n",    CurTdSw->TdHw.ShortPacket);
    UhciDebugPrint (Level, " PidCode      - 0x%x\n",  CurTdSw->TdHw.PidCode);
    UhciDebugPrint (Level, " DevAddr      - %d\n",    CurTdSw->TdHw.DeviceAddr);
    UhciDebugPrint (Level, " EndPoint     - %d\n",    CurTdSw->TdHw.EndPoint);
    UhciDebugPrint (Level, " DataToggle   - %d\n",    CurTdSw->TdHw.DataToggle);
    UhciDebugPrint (Level, " MaxPacketLen - %d\n",    CurTdSw->TdHw.MaxPacketLen);
    UhciDebugPrint (Level, " DataBuffer   - 0x%x\n\n",CurTdSw->TdHw.DataBuffer);
 
    CurTdSw = CurTdSw->NextTd;
  }
}

#endif
