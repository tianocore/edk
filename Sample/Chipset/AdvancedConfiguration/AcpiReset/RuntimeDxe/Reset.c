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

EFI_ACPI_DESCRIPTION      mAcpiDescription;

VOID
EFIAPI
AcpiResetSystem (
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
  UINT8   DevFunc;
  UINT8   Register;

  switch (ResetType) {
  case EfiResetWarm:
  case EfiResetCold:
  case EfiResetShutdown:

    switch (mAcpiDescription.RESET_REG.AddressSpaceId) {
    case ACPI_ADDRESS_ID_IO:
      IoWrite8 (mAcpiDescription.RESET_REG.Address, mAcpiDescription.RESET_VALUE);
      break;
    case ACPI_ADDRESS_ID_MEMORY:
      MemWrite8 (mAcpiDescription.RESET_REG.Address, mAcpiDescription.RESET_VALUE);
      break;
    case ACPI_ADDRESS_ID_PCI:
      DevFunc = (UINT8) (RShiftU64 (mAcpiDescription.RESET_REG.Address, 16) & 0x7) |
                (UINT8) ((RShiftU64 (mAcpiDescription.RESET_REG.Address, 32) & 0x1F) << 3);
      Register = (UINT8)mAcpiDescription.RESET_REG.Address;
      PciWrite8 (0, 0, DevFunc, Register, mAcpiDescription.RESET_VALUE);
      break;
    }
    break;

  default:
    return ;
  }

  //
  // Given we should have reset getting here would be bad
  //
  ASSERT (FALSE);
}

EFI_ACPI_DESCRIPTION *
GetAcpiDescription (
  VOID
  )
{
  VOID                                     *HobList;
  EFI_STATUS                               Status;
  EFI_ACPI_DESCRIPTION                     *AcpiDescription;
  UINTN                                    BufferSize;

  //
  // Get Hob List from configuration table
  //
  Status = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Get AcpiDescription Hob
  //
  AcpiDescription = NULL;
  Status = GetNextGuidHob (&HobList, &gEfiAcpiDescriptionGuid, &AcpiDescription, &BufferSize);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Do basic validation
  //
  if ((AcpiDescription->RESET_REG.Address == 0) ||
      ((AcpiDescription->RESET_REG.AddressSpaceId != ACPI_ADDRESS_ID_IO) &&
       (AcpiDescription->RESET_REG.AddressSpaceId != ACPI_ADDRESS_ID_MEMORY) &&
       (AcpiDescription->RESET_REG.AddressSpaceId != ACPI_ADDRESS_ID_PCI))) {
    return NULL;
  }

  DEBUG ((EFI_D_ERROR, "ACPI Reset Base - %lx\n", AcpiDescription->RESET_REG.Address));
  DEBUG ((EFI_D_ERROR, "ACPI Reset Value - %02x\n", (UINTN)AcpiDescription->RESET_VALUE));

  //
  // Copy it to Runtime Memory
  //
  EfiCommonLibCopyMem (&mAcpiDescription, AcpiDescription, sizeof(mAcpiDescription));
  
  return AcpiDescription;
}

