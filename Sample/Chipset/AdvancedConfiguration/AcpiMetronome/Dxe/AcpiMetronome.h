/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             


Module Name:

  AcpiMetronome.h

Abstract:

  Driver implementing the EFI 2.0 metronome protocol using the ACPI timer.

--*/

#ifndef _ACPI_METRONOME_H
#define _ACPI_METRONOME_H

//
// Statements that include other files
//
#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiHobLib.h"

//
// Driver Consumes GUIDs
//
#include EFI_GUID_DEFINITION (Hob)
#include EFI_GUID_DEFINITION (AcpiDescription)

//
// Consumed protocols
//
#include EFI_PROTOCOL_DEPENDENCY (CpuIo)

//
// Produced protocols
//
#include EFI_ARCH_PROTOCOL_PRODUCER (Metronome)

//
// Private definitions
//
#define TICK_PERIOD                   300   // 30 uS

//
// 8254 counter initialization definitions
//
#define TIMER1_CONTROL_PORT 0x43
#define TIMER1_COUNT_PORT   0x41
#define LOAD_COUNTER1_LSB   0x54
#define COUNTER1_COUNT      0x12

//
// Function Prototypes
//
EFI_STATUS
EFIAPI
WaitForTick (
  IN EFI_METRONOME_ARCH_PROTOCOL  *This,
  IN UINT32                       TickNumber
  )
/*++

Routine Description:

  Waits for the TickNumber of ticks from a known platform time source.

Arguments:

  This                Pointer to the protocol instance.
  TickNumber          Tick Number to be waited

Returns: 

  EFI_SUCCESS         If number of ticks occurred.
  EFI_NOT_FOUND       Could not locate CPU IO protocol

--*/
;

UINT32
GetAcpiTick (
  VOID
  )
/*++

Routine Description:

  Get the current ACPI counter's value

Arguments:

  None

Returns: 

  The value of the counter

--*/
;

VOID
WriteIo8 (
  UINT16  Port,
  UINT8   Data
  )
/*++

Routine Description:

  Write an 8 bit value to an I/O port

Arguments:

  Port - IO Port
  Data - Data in IO Port

Returns: 

  None.

--*/
;
#endif
