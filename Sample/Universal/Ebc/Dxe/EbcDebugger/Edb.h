/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Edb.h

Abstract:


--*/

#ifndef _EFI_EDB_H_
#define _EFI_EDB_H_

#include "Tiano.h"
#include "EfiCommonLib.h"
#include "EfiDriverLib.h"
#include "EfiPrintLib.h"
#include "EdbCommon.h"

#include EFI_PROTOCOL_DEFINITION (Ebc)
#include EFI_PROTOCOL_DEFINITION (DebugPort)
#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)

#include "EbcInt.h"
#include "EbcExecute.h"

#define EBC_DEBUGGER_MAJOR_VERSION   0
#define EBC_DEBUGGER_MINOR_VERSION   21

#define EFI_DEBUG_RETURN    1
#define EFI_DEBUG_BREAK     2
#define EFI_DEBUG_CONTINUE  3

//
// TBD
//
#define EFI_DEBUGGER_ASSERT(assertion)
// #define EFI_DEBUGGER_ASSERT(assertion) if(!(assertion))  \
//                                         EfiDebugAssert (__FILE__, __LINE__, #assertion)

//
// ASSERT support
//
VOID
EfiDebugAssert (
  IN CHAR8    *FileName,
  IN INTN     LineNumber,
  IN CHAR8    *Description
  );


//
// Function
//
EFI_STATUS
EfiDebuggerEntrypoint (
  IN EFI_HANDLE                     ImageHandle,
  IN EFI_SYSTEM_TABLE               *SystemTable
  );

VOID
EdbExceptionHandler (
  IN EFI_EXCEPTION_TYPE   ExceptionType,
  IN EFI_SYSTEM_CONTEXT   SystemContext
  );

extern EFI_DEBUGGER_PRIVATE_DATA mDebuggerPrivate;

#include "EdbSupport.h"
#include "EdbCommand.h"
#include "EdbDisasm.h"
#include "EdbDisasmSupport.h"
#include "EdbSymbol.h"
#include "EdbHook.h"

#endif
