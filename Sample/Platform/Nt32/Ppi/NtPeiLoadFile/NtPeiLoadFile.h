/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

 NtPeiLoadFile.h

Abstract:

  WinNt Load File PPI as defined in EFI 2.0

  When the PEI core is done it calls the DXE IPL via this PPI.

--*/

#ifndef _NT_PEI_LOAD_FILE_H_
#define _NT_PEI_LOAD_FILE_H_

#include "Tiano.h"
#include "PeiHob.h"

#define PEI_LOAD_FILE_PRIVATE_GUID \
  { 0xfd0c65eb, 0x405, 0x4cd2, 0x8a, 0xee, 0xf4, 0x0, 0xef, 0x13, 0xba, 0xc2 }

typedef
EFI_STATUS
(EFIAPI *PEI_NT_LOAD_FILE) (
  VOID                  *Pe32Data,
  EFI_PHYSICAL_ADDRESS  *ImageAddress,
  UINT64                *ImageSize,
  EFI_PHYSICAL_ADDRESS  *EntryPoint
  );

EFI_FORWARD_DECLARATION (PEI_NT_CALLBACK_PROTOCOL);

typedef struct _PEI_NT_CALLBACK_PROTOCOL {
  //
  //  OK, so now load all of the stuff that was formerly GLOBAL in the
  //  SecMain utility.  This stuff was only consumed by this protocol.
  //  This protocol thing needs to be declared, but members can be privately
  //  scoped.  
  //
  PEI_NT_LOAD_FILE  PeiLoadFileService;
} PEI_NT_CALLBACK_PROTOCOL;

extern EFI_GUID gPeiLoadFileGuid;

#endif
