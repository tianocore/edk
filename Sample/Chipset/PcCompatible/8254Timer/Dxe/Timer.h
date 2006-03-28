/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             


Module Name:

 Timer.h

Abstract:

  Private data structures

--*/

#ifndef _SMARTTIMER_H_
#define _SMARTTIMER_H_

#include "Tiano.h"
#include "EfiDriverLib.h"

//
// Consumed Protocols
//
#include EFI_ARCH_PROTOCOL_CONSUMER (Cpu)
#include EFI_PROTOCOL_CONSUMER (CpuIo)
#include EFI_PROTOCOL_CONSUMER (Legacy8259)

//
// Produced Protocols
//
#include EFI_ARCH_PROTOCOL_PRODUCER (Timer)

//
// The PCAT 8253/8254 has an input clock at 1.193182 MHz and Timer 0 is
// initialized as a 16 bit free running counter that generates an interrupt(IRQ0)
// each time the counter rolls over.
//
//   65536 counts
// ---------------- * 1,000,000 uS/S = 54925.4 uS = 549254 * 100 ns
//   1,193,182 Hz
//
#define DEFAULT_TIMER_TICK_DURATION 549254
#define TIMER_CONTROL_PORT          0x43
#define TIMER0_COUNT_PORT           0x40

//
// Legacy ACPI timer address
//
#define ACPI_TIME_COUNTER_ADDRESS 0x408

//
// Function Prototypes
//
EFI_STATUS
EFIAPI
TimerDriverInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ImageHandle - TODO: add argument description
  SystemTable - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
TimerDriverRegisterHandler (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  IN EFI_TIMER_NOTIFY         NotifyFunction
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This            - TODO: add argument description
  NotifyFunction  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
TimerDriverSetTimerPeriod (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  IN UINT64                   TimerPeriod
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  TimerPeriod - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
TimerDriverGetTimerPeriod (
  IN EFI_TIMER_ARCH_PROTOCOL   *This,
  OUT UINT64                   *TimerPeriod
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  TimerPeriod - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
TimerDriverGenerateSoftInterrupt (
  IN EFI_TIMER_ARCH_PROTOCOL  *This
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
