/*++

Copyright (c) 2006 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  IpfReset.c

Abstract:

--*/

#include "Cf9Reset.h"

//
// Don't use directly after virtual address have been registered.
//
static EFI_ACPI_DESCRIPTION      mAcpiDescription;

SAL_RETURN_REGS
ResetEsalServicesClassCommonEntry (
  IN  UINT64                     FunctionId,
  IN  UINT64                     Arg2,
  IN  UINT64                     Arg3,
  IN  UINT64                     Arg4,
  IN  UINT64                     Arg5,
  IN  UINT64                     Arg6,
  IN  UINT64                     Arg7,
  IN  UINT64                     Arg8,
  IN  SAL_EXTENDED_SAL_PROC      ExtendedSalProc,
  IN   BOOLEAN                   VirtualMode,
  IN  VOID                       *Global
  )
/*++

Routine Description:

  Main entry for Extended SAL Reset Services

Arguments:

  FunctionId    Function Id which needed to be called.
  Arg2          EFI_RESET_TYPE, whether WARM of COLD reset
  Arg3          Last EFI_STATUS 
  Arg4          Data Size of UNICODE STRING passed in ARG5
  Arg5          Unicode String which CHAR16*
  Arg6          not used
  Arg7          not used
  Arg8          not used
  ExtendedSalProc ExtendedSalProc
  VirtualMode   Current in virtual mode or not
  Global        Global variable for this module

Returns:

  SAL_RETURN_REGS

--*/
{
  SAL_RETURN_REGS ReturnVal;

  switch (FunctionId) {
  case ResetSystem:
    AcpiResetSystem (Arg2, Arg3, (UINTN) Arg4, (VOID *) Arg5, Global);
    ReturnVal.Status = EFI_SUCCESS;
    break;

  default:
    ReturnVal.Status = EFI_SAL_INVALID_ARGUMENT;
    break;
  }

  return ReturnVal;
}

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
// TODO:    SystemTable - add argument and description to function comment
{
  EFI_ACPI_DESCRIPTION      *AcpiDescription;

  EfiInitializeRuntimeDriverLib (ImageHandle, SystemTable, NULL);

  //
  // Initialize AcpiDescription
  //
  AcpiDescription = GetAcpiDescription (&mAcpiDescription);
  if (AcpiDescription == NULL) {
    return EFI_UNSUPPORTED;
  }

  RegisterEsalClass (
    &gEfiExtendedSalResetServicesProtocolGuid,
    &mAcpiDescription,
    ResetEsalServicesClassCommonEntry,
    ResetSystem,
    NULL
    );

  return EFI_SUCCESS;
}

