/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  IpfCf9Reset.c

Abstract:

--*/

#include "Cf9Reset.h"

VOID
EFIAPI
IpfCf9ResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL
  );

SAL_RETURN_REGS
Cf9ResetEsalServicesClassCommonEntry (
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

Returns:

  SAL_RETURN_REGS

--*/
// TODO:    Arg6 - add argument and description to function comment
// TODO:    Arg7 - add argument and description to function comment
// TODO:    Arg8 - add argument and description to function comment
// TODO:    ExtendedSalProc - add argument and description to function comment
// TODO:    VirtualMode - add argument and description to function comment
// TODO:    Global - add argument and description to function comment
{
  SAL_RETURN_REGS ReturnVal;

  switch (FunctionId) {
  case ResetSystem:
    IpfCf9ResetSystem (Arg2, Arg3, (UINTN) Arg4, (VOID *) Arg5);
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
InitializeCf9Reset (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Initialize the state information for the Timer Architectural Protocol

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
  EfiInitializeRuntimeDriverLib (ImageHandle, SystemTable, NULL);

  RegisterEsalClass (
    &gEfiExtendedSalResetServicesProtocolGuid,
    NULL,
    Cf9ResetEsalServicesClassCommonEntry,
    ResetSystem,
    NULL
    );

  return EFI_SUCCESS;
}

VOID
CapsuleReset (
  IN UINTN CapsuleDataPtr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  CapsuleDataPtr  - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  //
  // BUGBUG -- implement for IPF
  //
}

VOID
EFIAPI
IpfCf9ResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL
  )
/*++

Routine Description:

  Reset the system on IPF based platform. We only implement only cold reset here
  because warm reset on IPF is PAL_INIT entry, which is a different entry point
  from PAL_RESET (cold). The current IPF PEI core doesn't implement PAL_INIT 
  boot path and just put a dead loop there. Once this boot path is developed, we
  should remove this function and just use the common one "Cf9ResetSystem"

Arguments:
  
    ResetType - warm or cold
    ResetStatus - possible cause of reset
    DataSize - Size of ResetData in bytes
    ResetData - Optional Unicode string
    For details, see efiapi.h

Returns:
  Does not return if the reset takes place.
  EFI_INVALID_PARAMETER   If ResetType is invalid.

--*/
{
  UINT8   InitialData;
  UINT8   OutputData;
  UINT32  PM_BASE_Addr;
  UINT32  AcpiCntl_Addr;
  UINT64  Address;
  UINT32  PmBase;
  UINT32  Gpe0Enable;
  UINT32  PmCntl;
  UINT16  PwrSts;
  UINT8   AcpiCntl;

  switch (ResetType) {
  //
  // All reset types go to cold reset.
  //
  case EfiResetWarm:
#if ((EFI_SPECIFICATION_VERSION < 0x00020000) && (TIANO_RELEASE_VERSION != 0))
  case EfiResetUpdate:
#endif    
  case EfiResetCold:

    InitialData = HARDSTARTSTATE;
    OutputData  = HARDRESET;
    break;

  case EfiResetShutdown:

    AcpiCntl_Addr = 0x80000000 | (BUS_NUMBER << 16) | (LPC_DEVICE_NUMBER << 11) | (PM_FUNCTION_NUMBER << 8) | ACPI_CNTL;

    IoWrite32 (0xcf8, AcpiCntl_Addr);

    //
    // Firstly, ACPI decode must be enabled
    //
    AcpiCntl = IoRead8 (0xcfc);

    if ((AcpiCntl & 0x10) == 0) {
      AcpiCntl |= 0x10;
      IoWrite8 (0xcfc, AcpiCntl);
    }

    PM_BASE_Addr = 0x80000000 | (BUS_NUMBER << 16) | (LPC_DEVICE_NUMBER << 11) | (PM_FUNCTION_NUMBER << 8) | PM_BASE;

    IoWrite32 (0xcf8, PM_BASE_Addr);

    PmBase = (IoRead32 (0xcfc) & 0xFFFC);

    //
    // Then, GPE0_EN should be disabled to
    // avoid any GPI waking up the system from S5
    //
    Gpe0Enable  = 0;
    Address     = PmBase + GPE0_EN;
    IoWrite32 (Address, Gpe0Enable);

    //
    // Secondly, PwrSts register must be cleared
    //
    // Write a "1" to bit[8] of power button status register at
    // (PM_BASE + PM1_STS_OFFSET) to clear this bit
    //
    PwrSts  = 0x0100;
    Address = PmBase + PM1_STS_OFFSET;
    IoWrite16 (Address, PwrSts);

    //
    // Finally, transform system into S5 sleep state
    //
    Address = PmBase + PM1_CNT_OFFSET;
    PmCntl  = IoRead32 (Address);

    PmCntl  = (PmCntl & 0xffffc3ff) | 0x3c00;
    IoWrite32 (Address, PmCntl);
    return ;

  default:
    return ;
  }

  IoWrite8 (0xcf9, InitialData);
  IoWrite8 (0xcf9, OutputData);

  //
  // Given we should have reset getting here would be bad
  //
  ASSERT (FALSE);
}
