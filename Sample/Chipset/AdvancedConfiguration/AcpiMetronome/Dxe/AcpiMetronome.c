/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             


Module Name:

  AcpiMetronome.c

Abstract:

  Leverage the Acpi timer to implement the Efi Metronome Arch Protocol 
  This driver assume the Acpi timer device has been initialized in PEI and ready to use.
--*/

#include "AcpiMetronome.h"

//
// Handle for the Metronome Architectural Protocol instance produced by this driver
//
EFI_HANDLE                  mMetronomeHandle = NULL;

//
// The Metronome Architectural Protocol instance produced by this driver
//
EFI_METRONOME_ARCH_PROTOCOL mMetronome = {
  WaitForTick,
  TICK_PERIOD // 30 uS
};

//
// The CPU I/O Protocol used to access system hardware
//
STATIC EFI_CPU_IO_PROTOCOL              *mCpuIo = NULL;

EFI_ACPI_DESCRIPTION      *mAcpiDescription;

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

  return AcpiDescription;
}

//
// Driver Entry Point
//
EFI_DRIVER_ENTRY_POINT (InstallAcpiMetronome)

EFI_STATUS
EFIAPI
InstallAcpiMetronome (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:
  
  Install the AcpiMetronome driver.  Loads a Metronome Arch Protocol based
  on the ACPI timer.
  This driver assume the Acpi timer device has been initialized in PEI and ready to use.
  
Arguments:

  ImageHandle     - Handle for the image of this driver
  SystemTable     - Pointer to the EFI System Table

Returns:

  EFI_SUCCESS - Metronome Architectural Protocol Installed

--*/
{
  EFI_STATUS  Status;

  Status = EfiInitializeDriverLib (ImageHandle, SystemTable);
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize AcpiDescription
  //
  mAcpiDescription = GetAcpiDescription ();
  if (mAcpiDescription == NULL) {
    return EFI_UNSUPPORTED;
  }

  if ((mAcpiDescription->PM_TMR_LEN != 4) ||
      (mAcpiDescription->PM_TMR_BLK.Address == 0) ||
      ((mAcpiDescription->PM_TMR_BLK.AddressSpaceId != ACPI_ADDRESS_ID_IO) &&
       (mAcpiDescription->PM_TMR_BLK.AddressSpaceId != ACPI_ADDRESS_ID_MEMORY))) {
    return EFI_UNSUPPORTED;
  }

  DEBUG ((EFI_D_ERROR, "ACPI Timer Base - %lx\n", mAcpiDescription->PM_TMR_BLK.Address));

  //
  // Make sure the Metronome Architectural Protocol is not already installed in the system
  //
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEfiMetronomeArchProtocolGuid);

  //
  // Get the CPU I/O Protocol and PCI Root Bridge I/O Protocol that this driver requires
  //
  Status = gBS->LocateProtocol (&gEfiCpuIoProtocolGuid, NULL, &mCpuIo);
  ASSERT_EFI_ERROR (Status);
 
  //
  // The legacy 8254 counter is a critical legacy hardware for many CSM bins and OSes to implement stall function,
  // so no matter whether we use it in Tiano, we should initialize the 8254 as a timer for compatibility.
  // We could do 8254 initialization somewhere outside this driver,  but for easy to replacy Legacy 
  // Metronome drvier with this ACPI metronome driver,  and not need additional code, we add the 8254 
  // initialization here to promise the 8254 is initialized.
  // Program port 61 timer 1 as refresh timer.  
  //
  WriteIo8 (TIMER1_CONTROL_PORT, LOAD_COUNTER1_LSB);
  WriteIo8 (TIMER1_COUNT_PORT, COUNTER1_COUNT);
  
  //
  // Install on a new handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mMetronomeHandle,
                  &gEfiMetronomeArchProtocolGuid,
                  &mMetronome,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

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
{
  UINT32  Tick;

  switch (mAcpiDescription->PM_TMR_BLK.AddressSpaceId) {
  case ACPI_ADDRESS_ID_IO:
    mCpuIo->Io.Read (mCpuIo, EfiCpuIoWidthUint32, mAcpiDescription->PM_TMR_BLK.Address, 1, &Tick);
    break;
  case ACPI_ADDRESS_ID_MEMORY:
    mCpuIo->Mem.Read (mCpuIo, EfiCpuIoWidthUint32, mAcpiDescription->PM_TMR_BLK.Address, 1, &Tick);
    break;
  default:
    ASSERT (FALSE);
    Tick = 0;
    break;
  }
  
  //
  // Only 23:0 bit is true value
  //
  if (mAcpiDescription->TMR_VAL_EXT == 0) {
    Tick &= 0xffffff;
  }
  return Tick;
}

EFI_STATUS
EFIAPI
WaitForTick (
  IN EFI_METRONOME_ARCH_PROTOCOL  *This,
  IN UINT32                       TickNumber
  )
/*++

Routine Description:

  Waits for the TickNumber of ticks from ACPI timer.
  One Tick delay 30 uS.

Arguments:

  This                        Pointer to the protocol instance.
  TickNumber          Tick Number to be waited

Returns: 

  EFI_SUCCESS         If number of ticks occurred.

--*/
{
  UINT64                Ticks;
  UINTN                 Counts;
  UINTN                 CurrentTick;
  UINTN                 LastTick;
  UINTN                 RemainingTick;
  
  if (TickNumber == 0) {
    return EFI_SUCCESS;
  }
  
  //
  // The timer frequency is 3.579545 MHz, so 1 us corresponds 3.58 clocks, so one one Tick (30 us) corresponds 107.39 clocks
  // 
  CurrentTick  = GetAcpiTick ();
  LastTick = CurrentTick;
  Ticks = DivU64x32 (MultU64x32 (TickNumber, 10739), 100, &RemainingTick) + CurrentTick + 1;
  if (mAcpiDescription->TMR_VAL_EXT == 0) {
    Counts = (UINTN) RShiftU64 (Ticks, 24);
    RemainingTick = (UINTN)((UINT32)Ticks & 0xFFFFFF);
  } else {
    Counts = (UINTN) RShiftU64 (Ticks, 32);
    RemainingTick = (UINTN)((UINT32)Ticks & 0xFFFFFFFF);
  }

  //
  // The loops needed by timer overflow, remaining clocks within one loop
  //
  while (Counts > 0) {
    CurrentTick = GetAcpiTick ();
    //
    // If last tick > current tick, the timer has overfloweded once
    //
    if (CurrentTick < LastTick) {
      Counts --;
    }
    LastTick = CurrentTick;
  }
  
  while ((RemainingTick > CurrentTick) && (LastTick <= CurrentTick) ) {
    LastTick = CurrentTick;
    CurrentTick   = GetAcpiTick ();
  } 
  
  return EFI_SUCCESS;
}

VOID
WriteIo8 (
  UINT16  Port,
  UINT8   Data
  )
/*++

Routine Description:

  Write an 8 bit value to an I/O port and save it to the S3 script

Arguments:

  Port - IO Port
  Data - Data in IO Port

Returns: 

  None.

--*/
{
  mCpuIo->Io.Write (
              mCpuIo,
              EfiCpuIoWidthUint8,
              Port,
              1,
              &Data
              );
}
