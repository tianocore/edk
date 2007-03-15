/*++

Copyright (c) 2006 - 2007, Intel Corporation                                                         
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
SystemReset (
  IN EFI_ACPI_DESCRIPTION *AcpiDescription
  )
/*++

Routine Description:

  Use ACPI method to reset the sytem. If fail, use legacy 8042 method to reset the system

Arguments:
  
    AcpiDescription - Global variable to record reset info

Returns:
  Does not return if the reset takes place.

--*/
{
  UINT8   DevFunc;
  UINT8   Register;

  if ((AcpiDescription->RESET_REG.Address != 0) &&
      ((AcpiDescription->RESET_REG.AddressSpaceId == ACPI_ADDRESS_ID_IO) ||
       (AcpiDescription->RESET_REG.AddressSpaceId == ACPI_ADDRESS_ID_MEMORY) ||
       (AcpiDescription->RESET_REG.AddressSpaceId == ACPI_ADDRESS_ID_PCI))) {
    //
    // Use ACPI System Reset
    //
    switch (AcpiDescription->RESET_REG.AddressSpaceId) {
    case ACPI_ADDRESS_ID_IO:
      IoWrite8 (AcpiDescription->RESET_REG.Address, AcpiDescription->RESET_VALUE);
      break;
    case ACPI_ADDRESS_ID_MEMORY:
      MemWrite8 (AcpiDescription->RESET_REG.Address, AcpiDescription->RESET_VALUE);
      break;
    case ACPI_ADDRESS_ID_PCI:
      DevFunc = (UINT8) (RShiftU64 (AcpiDescription->RESET_REG.Address, 16) & 0x7) |
                (UINT8) ((RShiftU64 (AcpiDescription->RESET_REG.Address, 32) & 0x1F) << 3);
      Register = (UINT8)AcpiDescription->RESET_REG.Address;
      PciWrite8 (0, 0, DevFunc, Register, AcpiDescription->RESET_VALUE);
      break;
    }
  }

  //
  // If system comes here, means ACPI reset fail, do Legacy System Reset, assume 8042 available
  //
  Register = 0xfe;
  IoWrite8 (0x64, Register);

  //
  // System should reset now
  //

  return ;
}

EFI_STATUS
SystemShutdown (
  IN EFI_ACPI_DESCRIPTION *AcpiDescription
  )
/*++

Routine Description:

  Use ACPI method to shutdown the sytem

Arguments:
  
    AcpiDescription - Global variable to record reset info

Returns:
  Does not return if the shutdown takes place.
  EFI_UNSUPPORTED - if shutdown fails.

--*/
{
  UINT16  Value;

  //
  // 1. Write SLP_TYPa
  //
  if ((AcpiDescription->PM1a_CNT_BLK.Address != 0) && (AcpiDescription->SLP_TYPa != 0)) {
    switch (AcpiDescription->PM1a_CNT_BLK.AddressSpaceId) {
    case ACPI_ADDRESS_ID_IO:
      Value = IoRead16 (AcpiDescription->PM1a_CNT_BLK.Address);
      Value = (Value & 0xc3ff) | 0x2000 | (AcpiDescription->SLP_TYPa << 10);
      IoWrite16 (AcpiDescription->PM1a_CNT_BLK.Address, Value);
      break;
    case ACPI_ADDRESS_ID_MEMORY:
      Value = MemRead16 (AcpiDescription->PM1a_CNT_BLK.Address);
      Value = (Value & 0xc3ff) | 0x2000 | (AcpiDescription->SLP_TYPa << 10);
      MemWrite16 (AcpiDescription->PM1a_CNT_BLK.Address, Value);
      break;
    }
  }

  //
  // 2. Write SLP_TYPb
  //
  if ((AcpiDescription->PM1b_CNT_BLK.Address != 0) && (AcpiDescription->SLP_TYPb != 0)) {
    switch (AcpiDescription->PM1b_CNT_BLK.AddressSpaceId) {
    case ACPI_ADDRESS_ID_IO:
      Value = IoRead16 (AcpiDescription->PM1b_CNT_BLK.Address);
      Value = (Value & 0xc3ff) | 0x2000 | (AcpiDescription->SLP_TYPb << 10);
      IoWrite16 (AcpiDescription->PM1b_CNT_BLK.Address, Value);
      break;
    case ACPI_ADDRESS_ID_MEMORY:
      Value = MemRead16 (AcpiDescription->PM1b_CNT_BLK.Address);
      Value = (Value & 0xc3ff) | 0x2000 | (AcpiDescription->SLP_TYPb << 10);
      MemWrite16 (AcpiDescription->PM1b_CNT_BLK.Address, Value);
      break;
    }
  }

  //
  // Done, if code runs here, mean not shutdown correctly
  //
  return EFI_UNSUPPORTED;
}

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
    For details, see efiapi.h

Returns:
  Does not return if the reset takes place.

--*/
{
  EFI_STATUS  Status;

  switch (ResetType) {
  case EfiResetWarm:
  case EfiResetCold:
    SystemReset (AcpiDescription);
    break;

  case EfiResetShutdown:
    Status = SystemShutdown (AcpiDescription);
    if (EFI_ERROR (Status)) {
      SystemReset (AcpiDescription);
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
  IN EFI_ACPI_DESCRIPTION *AcpiDescription
  )
{
  VOID                                     *HobList;
  EFI_STATUS                               Status;
  EFI_ACPI_DESCRIPTION                     *HobAcpiDescription;
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
  HobAcpiDescription = NULL;
  Status = GetNextGuidHob (&HobList, &gEfiAcpiDescriptionGuid, &HobAcpiDescription, &BufferSize);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Copy it to Runtime Memory
  //
  EfiCommonLibCopyMem (AcpiDescription, HobAcpiDescription, sizeof(*AcpiDescription));

  DEBUG ((EFI_D_ERROR, "ACPI Reset Base - %lx\n", AcpiDescription->RESET_REG.Address));
  DEBUG ((EFI_D_ERROR, "ACPI Reset Value - %02x\n", (UINTN)AcpiDescription->RESET_VALUE));
  DEBUG ((EFI_D_ERROR, "IAPC support - %x\n", (UINTN)(AcpiDescription->IAPC_BOOT_ARCH)));
  
  return AcpiDescription;
}
