/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  NtFwh.h

Abstract:

  WinNt FWH PPI as defined in EFI 2.0

--*/

#ifndef _NT_PEI_FWH_H_
#define _NT_PEI_FWH_H_

#include "Tiano.h"
#include "PeiHob.h"

#define PEI_NT_FWH_PRIVATE_GUID \
  { 0x4e76928f, 0x50ad, 0x4334, 0xb0, 0x6b, 0xa8, 0x42, 0x13, 0x10, 0x8a, 0x57 }

typedef
EFI_STATUS
(EFIAPI *PEI_NT_FWH_INFORMATION) (
  IN OUT UINT64                *FwhSize,
  IN OUT EFI_PHYSICAL_ADDRESS  *FwhBase
  );

EFI_FORWARD_DECLARATION (PEI_NT_FWH_CALLBACK_PROTOCOL);

typedef struct _PEI_NT_FWH_CALLBACK_PROTOCOL {
  //
  //  OK, so now load all of the stuff that was formerly GLOBAL in the
  //  SecMain utility.  This stuff was only consumed by this protocol.
  //  This protocol thing needs to be declared, but members can be privately
  //  scoped.  
  //
  PEI_NT_FWH_INFORMATION    NtFwh;
} PEI_NT_FWH_CALLBACK_PROTOCOL;

extern EFI_GUID gPeiFwhInformationGuid;

#endif
