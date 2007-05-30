/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  UhciDebug.h

Abstract:

  This file contains the definination for host controller debug support routines

Revision History

--*/

#ifndef _EFI_UHCI_DEBUG_H_
#define _EFI_UHCI_DEBUG_H_

//
// DEBUG support
//
//
// USB generic definition
//
#define USB_DEBUG_FORCE_OUTPUT  (UINTN) (1 << 0)
#define USB_DEBUG_PROMT         (UINTN) (1 << 1)

//
// Data Structure
//
#define UHCI_DEBUG_QH       (UINTN) (1 << 2)
#define UHCI_DEBUG_TD       (UINTN) (1 << 3)
#define UHCI_DEBUG_BUFFER   (UINTN) (1 << 4)
#define UHCI_DEBUG_PORT_STS (UINTN) (1 << 5)

//
// Function
//
#define UHCI_DEBUG_CTL      (UINTN) (1 << 10)
#define UHCI_DEBUG_BULK     (UINTN) (1 << 11)
#define UHCI_DEBUG_SYC_INT  (UINTN) (1 << 12)
#define UHCI_DEBUG_ASYC_INT (UINTN) (1 << 13)
#define UHCI_DEBUG_SCHEDULE (UINTN) (1 << 14)

//
// USB generic definition
//
#define USB_DEBUG_RESERVED1 (UINTN) (1 << 29)
#define USB_DEBUG_RESERVED2 (UINTN) (1 << 30)
#define USB_DEBUG_ERROR     (UINTN) (1 << 31)
#define USB_DEBUG_ALL       (UINTN) (-1)

VOID
UhciDebug (
  IN  UINTN         Mask,
  IN  CHAR8         *Format,
  ...
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Mask    - GC_TODO: add argument description
  Format  - GC_TODO: add argument description
  ...     - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
UhciDumpBuffer (
  IN UINT8         *Buffer,
  IN UINT32        Length
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Buffer  - GC_TODO: add argument description
  Length  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
UhciDumpPortsStatus (
  IN USB_HC_DEV    *HcDev,
  IN UINT8         PortNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  HcDev       - GC_TODO: add argument description
  PortNumber  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
UhciDumpQh (
  IN UHCI_QH_SW    *QhSw
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  QhSw  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
UhciDumpTds (
  IN UHCI_TD_SW    *TdSw,
  IN BOOLEAN       IsCur
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  TdSw  - GC_TODO: add argument description
  IsCur - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

#ifdef EFI_DEBUG
  #define UHCI_DEBUG(arg)            UhciDebug arg
  #define UHCI_DUMP_PORT_STAT(arg)   UhciDumpPortsStatus arg
  #define UHCI_DUMP_BUFFER(arg)      UhciDumpBuffer arg
  #define UHCI_DUMP_QH(arg)          UhciDumpQh arg
  #define UHCI_DUMP_TDS(arg)         UhciDumpTds arg
#else
  #define UHCI_DEBUG(arg)
  #define UHCI_DUMP_PORT_STAT(arg)
  #define UHCI_DUMP_BUFFER(arg)
  #define UHCI_DUMP_QH(arg)
  #define UHCI_DUMP_TDS(arg)
#endif

#endif
