/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Cf9Reset.h

Abstract:

  some definitions for reset

--*/

#ifndef _Cf9_RESET_H
#define _Cf9_RESET_H

#include "Tiano.h"
#include "EfiRuntimeLib.h"

//
// Reset control register values
//
#define FULLRESET       (1 << 3)
#define HARDRESET       0x06
#define SOFTRESET       0x04
#define HARDSTARTSTATE  0x02
#define SOFTSTARTSTATE  0x0

//
// Power Button Reset driver uses LPC PCI function which resides on bus:0
// device:31 and function:0 of ICH6
//
#define BUS_NUMBER          0x00
#define LPC_DEVICE_NUMBER   0x1f
#define PM_FUNCTION_NUMBER  0x00

//
// ICH6 Power Button Reset driver consumed registers
//
#define PM_BASE         0x40
#define PM1_STS_OFFSET  0x00
#define PM1_CNT_OFFSET  0x04

#define GPE0_EN         0x2C

#define ACPI_CNTL       0x44

EFI_STATUS
EFIAPI
InitializeCf9Reset (
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

VOID
EFIAPI
Cf9ResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ResetType   - TODO: add argument description
  ResetStatus - TODO: add argument description
  DataSize    - TODO: add argument description
  ResetData   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
CapsuleReset (
  IN UINTN  CapsuleDataPtr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  CapsuleDataPtr  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;
#endif
