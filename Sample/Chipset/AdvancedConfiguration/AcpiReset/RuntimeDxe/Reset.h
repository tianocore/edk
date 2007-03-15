/*++

Copyright (c) 2006 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Reset.h

Abstract:

  some definitions for reset

--*/

#ifndef _ACPI_RESET_H
#define _ACPI_RESET_H

#include "Tiano.h"
#include "EfiRuntimeLib.h"
#include "EfiCommonLib.h"
#include "EfiHobLib.h"

//
// Driver Consumes GUIDs
//
#include EFI_GUID_DEFINITION (Hob)
#include EFI_GUID_DEFINITION (AcpiDescription)

EFI_STATUS
EFIAPI
InitializeReset (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Initialize the state information for the Reset Architectural Protocol

Arguments:

  ImageHandle of the loaded driver
  Pointer to the System Table

Returns:

  Status

  EFI_SUCCESS           - thread can be successfully created
  EFI_OUT_OF_RESOURCES  - cannot allocate protocol data structure
  EFI_DEVICE_ERROR      - cannot create the timer service

--*/
;

VOID
EFIAPI
AcpiResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData, OPTIONAL
  IN EFI_ACPI_DESCRIPTION *AcpiDescription
  )
/*++

Routine Description:

  Reset the system.

Arguments:
  
    ResetType - warm or cold
    ResetStatus - possible cause of reset
    DataSize - Size of ResetData in bytes
    ResetData - Optional Unicode string
    AcpiDescription - Global variable to record reset info

Returns:
  Does not return if the reset takes place.

--*/
;

EFI_ACPI_DESCRIPTION *
GetAcpiDescription (
  IN EFI_ACPI_DESCRIPTION *AcpiDescription
  );

#endif
