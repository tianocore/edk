/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  NtAutoscan.h

Abstract:

  WinNt Autoscan PPI as defined in EFI 2.0

--*/

#ifndef _NT_PEI_AUTOSCAN_H_
#define _NT_PEI_AUTOSCAN_H_

#include "Tiano.h"
#include "PeiHob.h"

#define PEI_AUTOSCAN_PRIVATE_GUID \
  { 0xdce384d, 0x7c, 0x4ba5, 0x94, 0xbd, 0xf, 0x6e, 0xb6, 0x4d, 0x2a, 0xa9 }

typedef
EFI_STATUS
(EFIAPI *PEI_NT_AUTOSCAN) (
  IN OUT UINT64                *MemorySize,
  IN OUT EFI_PHYSICAL_ADDRESS  *MemoryBase
  );

EFI_FORWARD_DECLARATION (PEI_NT_AUTOSCAN_CALLBACK_PROTOCOL);

typedef struct _PEI_NT_AUTOSCAN_CALLBACK_PROTOCOL {
  //
  //  OK, so now load all of the stuff that was formerly GLOBAL in the
  //  SecMain utility.  This stuff was only consumed by this protocol.
  //  This protocol thing needs to be declared, but members can be privately
  //  scoped.  
  //
  PEI_NT_AUTOSCAN  NtAutoScan;
} PEI_NT_AUTOSCAN_CALLBACK_PROTOCOL;

extern EFI_GUID gPeiAutoScanGuid;

#endif
