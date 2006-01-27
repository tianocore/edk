/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Cf9Reset.c

Abstract:

  Reset Architectural Protocol implementation

--*/

#include "Cf9Reset.h"

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

  Reset the system.

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
  case EfiResetWarm:
    InitialData = SOFTSTARTSTATE;
    OutputData  = SOFTRESET;
    break;

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

