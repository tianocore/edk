/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Reset.c

Abstract:

  Reset Architectural Protocol implementation

--*/

#include "Reset.h"

VOID
EFIAPI
Port92ResetSystem (
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
  UINT8   Data;

  switch (ResetType) {
  case EfiResetWarm:
  case EfiResetCold:
  case EfiResetShutdown:
    Data = IoRead8 (0x92);
    Data |= 1;
    IoWrite8 (0x92, Data);
    break;

  default:
    return ;
  }

  //
  // Given we should have reset getting here would be bad
  //
  ASSERT (FALSE);
}

