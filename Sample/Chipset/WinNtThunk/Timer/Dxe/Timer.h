/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Timer.h

Abstract:

  NT Emulation Architectural Protocol Driver as defined in EFI 2.0.
  This Timer module uses an NT Thread to simulate the timer-tick driven
  timer service.

--*/

#ifndef _TIMER_H_
#define _TIMER_H_

#include "Efi2WinNT.h"
#include "EfiWinNtLib.h"
#include "EfiDriverLib.h"

//
// Consumed Protocols
//
#include EFI_ARCH_PROTOCOL_CONSUMER (Cpu)

//
// Produced Protocols
//
#include EFI_ARCH_PROTOCOL_PRODUCER (Timer)

//
// Legal timer value range in 100 ns units
//
#define TIMER_MINIMUM_VALUE          0
#define TIMER_MAXIMUM_VALUE          (0x100000000 - 1)

//
// Default timer value in 100 ns units (10 ms)
//
#define DEFAULT_TIMER_TICK_DURATION  100000

//
// Function Prototypes
//

EFI_STATUS
EFIAPI
WinNtTimerDriverInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

EFI_STATUS
EFIAPI
WinNtTimerDriverRegisterHandler (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  IN EFI_TIMER_NOTIFY         NotifyFunction
  );

EFI_STATUS
EFIAPI
WinNtTimerDriverSetTimerPeriod (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  IN UINT64                   TimerPeriod
  );

EFI_STATUS
EFIAPI
WinNtTimerDriverGetTimerPeriod (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  OUT UINT64                   *TimerPeriod
  );

EFI_STATUS
EFIAPI
WinNtTimerDriverGenerateSoftInterrupt (
  IN EFI_TIMER_ARCH_PROTOCOL  *This
  );
  
#endif
