/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    EfiSmmDriverLib.h

Abstract:

  Light weight lib to support EFI Smm drivers.

--*/

#ifndef _EFI_SMM_DRIVER_LIB_H_
#define _EFI_SMM_DRIVER_LIB_H_

#include "Tiano.h"
#include EFI_PROTOCOL_DEFINITION (SmmBase)
#include EFI_PROTOCOL_DEFINITION (SmmStatusCode)
//
// Driver Lib Globals.
//
extern EFI_BOOT_SERVICES         *gBS;
extern EFI_SYSTEM_TABLE          *gST;
extern EFI_RUNTIME_SERVICES      *gRT;
extern EFI_SMM_BASE_PROTOCOL     *gSMM;
extern EFI_SMM_STATUS_CODE_PROTOCOL *mSmmDebug;
extern UINTN               gErrorLevel;

EFI_STATUS
EfiInitializeSmmDriverLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN OUT BOOLEAN          *InSmm
  );

VOID
EfiDebugAssert (
  IN CHAR8    *FileName,
  IN INTN     LineNumber,
  IN CHAR8    *Description
  );
VOID

EfiDebugVPrint (
  IN  UINTN   ErrorLevel,
  IN  CHAR8   *Format,
  IN  VA_LIST Marker
  );

VOID
EfiDebugPrint (
  IN  UINTN                   ErrorLevel,
  IN  CHAR8                   *Format,
  ...
  );



#endif
