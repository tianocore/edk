/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Metronome.h

Abstract:

  NT Emulation Metronome Architectural Protocol Driver as defined in DXE CIS

--*/

#include "Efi2WinNT.h"
#include "EfiWinNtLib.h"
#include "EfiDriverLib.h"

//
// Produced Protocols
//
#include EFI_ARCH_PROTOCOL_PRODUCER (Metronome)

//
//Period of on tick in 100 nanosecond units
//
#define TICK_PERIOD 10000

//
// Function Prototypes
//

EFI_STATUS
WinNtMetronomeDriverInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

EFI_STATUS
EFIAPI
WinNtMetronomeDriverWaitForTick(
  IN EFI_METRONOME_ARCH_PROTOCOL  *This,
  IN UINT32                       TickNumber
  );
