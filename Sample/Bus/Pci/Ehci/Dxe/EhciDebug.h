/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  EhciDebug.h

Abstract:

  This file contains the definination for host controller debug support routines

Revision History
--*/

#ifndef _EFI_EHCI_DEBUG_H_
#define _EFI_EHCI_DEBUG_H_

enum {
  USB_DEBUG_FORCE_OUTPUT  = (UINTN)(1 << 0),

  EHC_DEBUG_QH            = (UINTN)(1 << 8),
  EHC_DEBUG_QTD           = (UINTN)(1 << 9),
  EHC_DEBUG_BUF           = (UINTN)(1 << 10),
};

VOID
EhciDebugPrint (
  IN  UINTN               Level,
  IN  CHAR8               *Format,
  ...
  )
/*++

Routine Description:

  EHCI's debug output function. It determines whether
  to output by the mask and level

Arguments:

  Level   - The output level
  Format  - The format parameters to the print
  ...     - The variable length parameters after format

Returns:

  None

--*/
;

VOID
EhcDebug (
  IN  CHAR8               *Format,
  ...
  )
/*++

Routine Description:

  EHCI's debug output function. It determines whether
  to output by the mask and level

Arguments:

  Format  - The format parameters to the print
  ...     - The variable length parameters after format

Returns:

  None

--*/
;


VOID
EhcError (
  IN  CHAR8               *Format,
  ...
  )
/*++

Routine Description:

  EHCI's error output function. It determines whether
  to output by the mask and level

Arguments:

  Format  - The format parameters to the print
  ...     - The variable length parameters after format

Returns:

  None

--*/
;


VOID
EhcDumpQtd (
  IN EHC_QTD              *Qtd,
  IN UINT8                *Msg
  )
/*++

Routine Description:

  Dump the fields of a QTD

Arguments:

  Qtd - The QTD to dump
  Msg - The message to print before the dump

Returns:

  None

--*/
;


VOID
EhcDumpQh (
  IN EHC_QH               *Qh,
  IN UINT8                *Msg,
  IN BOOLEAN              DumpBuf
  )
/*++

Routine Description:

  Dump the queue head

Arguments:

  Qh      - The queue head to dump
  Msg     - The message to print before the dump
  DumpBuf - Whether to dump the memory buffer of the associated QTD

Returns:

  None

--*/
;


VOID
EhcDumpBuf (
  IN UINT8                *Buf,
  IN UINTN                Len
  )
/*++

Routine Description:

  Dump the buffer in the form of hex

Arguments:

  Buf - The buffer to dump
  Len - The length of buffer

Returns:

  None

--*/
;

#ifdef EFI_DEBUG
  #define EHC_DEBUG(arg)                  EhcDebug    arg
  #define EHC_ERROR(arg)                  EhcError    arg
  #define EHC_DUMP_QH(arg)                EhcDumpQh   arg
  #define EHC_DUMP_QTD(arg)               EhcDumpQtd  arg
  #define EHC_DUMP_BUF(arg)               EhcDumpBuf  arg
#else
  #define EHC_DEBUG(arg)
  #define EHC_ERROR(arg)                  
  #define EHC_DUMP_QH(arg)
  #define EHC_DUMP_QTD(arg)
  #define EHC_DUMP_BUF(arg)
#endif

#endif
