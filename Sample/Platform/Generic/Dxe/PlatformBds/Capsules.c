/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Capsules.c

Abstract:

  BDS routines to handle capsules.

--*/

#include "Tiano.h"
#include "PeiHob.h"
#include "EfiFlashMap.h"
#include "EfiFirmwareVolumeHeader.h"
#include "EfiDriverLib.h" // needed for gDS
#include "EfiHobLib.h"    // needed for GetNextGuidHob
#include "EfiCapsule.h"
#include "Capsule.h"

#include EFI_PROTOCOL_DEFINITION (CpuIO)

#include EFI_GUID_DEFINITION (FlashMapHob)
#include EFI_GUID_DEFINITION (Hob)

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
STATIC EFI_GUID mEfiCapsuleHeaderGuid = EFI_CAPSULE_GUID;
#endif

STATIC
EFI_STATUS
GetNextCapsuleVolumeHob (
  IN OUT VOID                  **HobStart,
  OUT    EFI_PHYSICAL_ADDRESS  *BaseAddress,
  OUT    UINT64                *Length
  );

VOID
BdsLockFv (
  IN EFI_CPU_IO_PROTOCOL          *CpuIo,
  IN EFI_FLASH_SUBAREA_ENTRY      *FlashEntry
  );

VOID
BdsLockFv (
  IN EFI_CPU_IO_PROTOCOL          *CpuIo,
  IN EFI_FLASH_SUBAREA_ENTRY      *FlashEntry
  )
{
  EFI_FV_BLOCK_MAP_ENTRY      *BlockMap;
  EFI_FIRMWARE_VOLUME_HEADER  *FvHeader;
  UINT64                      BaseAddress;
  UINT8                       Data;
  UINT32                      BlockLength;
  UINTN                       Index;

  BaseAddress = FlashEntry->Base - 0x400000 + 2;
  FvHeader    = (EFI_FIRMWARE_VOLUME_HEADER *) ((UINTN) (FlashEntry->Base));
  BlockMap    = &(FvHeader->FvBlockMap[0]);

  while ((BlockMap->NumBlocks != 0) && (BlockMap->BlockLength != 0)) {
    BlockLength = BlockMap->BlockLength;
    for (Index = 0; Index < BlockMap->NumBlocks; Index++) {
      CpuIo->Mem.Read (
                  CpuIo,
                  EfiCpuIoWidthUint8,
                  BaseAddress,
                  1,
                  &Data
                  );
      Data = (UINT8) (Data | 0x3);
      CpuIo->Mem.Write (
                  CpuIo,
                  EfiCpuIoWidthUint8,
                  BaseAddress,
                  1,
                  &Data
                  );
      BaseAddress += BlockLength;
    }

    BlockMap++;
  }
}

VOID
BdsLockNonUpdatableFlash (
  VOID
  )
{
  EFI_FLASH_MAP_ENTRY_DATA  *FlashMapEntryData;
  VOID                      *HobList;
  VOID                      *Buffer;
  EFI_STATUS                Status;
  EFI_CPU_IO_PROTOCOL       *CpuIo;

  Status = gBS->LocateProtocol (&gEfiCpuIoProtocolGuid, NULL, &CpuIo);
  ASSERT_EFI_ERROR (Status);

  Status = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);
  ASSERT_EFI_ERROR (Status);

  for (;;) {
    Status = GetNextGuidHob (&HobList, &gEfiFlashMapHobGuid, &Buffer, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }

    FlashMapEntryData = (EFI_FLASH_MAP_ENTRY_DATA *) Buffer;

    //
    // Get the variable store area
    //
    if ((FlashMapEntryData->AreaType == EFI_FLASH_AREA_RECOVERY_BIOS) ||
        (FlashMapEntryData->AreaType == EFI_FLASH_AREA_MAIN_BIOS)
        ) {
      BdsLockFv (CpuIo, &(FlashMapEntryData->Entries[0]));
    }
  }

  return ;
}

EFI_STATUS
ProcessCapsules (
  EFI_BOOT_MODE BootMode
  )
/*++

Routine Description:

  This routine is called to see if there are any capsules we need to process.
  If the boot mode is not UPDATE, then we do nothing. Otherwise find the
  capsule HOBS and produce firmware volumes for them via the DXE service.
  Then call the dispatcher to dispatch drivers from them. Finally, check
  the status of the updates.

Arguments:

  BootMode - the current boot mode

Returns:
  
  EFI_INVALID_PARAMETER - boot mode is not correct for an update

Note:
 
 This function should be called by BDS in case we need to do some
 sort of processing even if there is no capsule to process. We
 need to do this if an earlier update went awry and we need to
 clear the capsule variable so on the next reset PEI does not see it and 
 think there is a capsule available.

--*/
{
  EFI_STATUS                  Status;
  EFI_HOB_HANDOFF_INFO_TABLE  *HobList;
  EFI_PHYSICAL_ADDRESS        BaseAddress;
  UINT64                      Length;
  EFI_STATUS                  HobStatus;
  EFI_FIRMWARE_VOLUME_HEADER  *FwVolHeader;
  EFI_HANDLE                  FvProtocolHandle;
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  EFI_CAPSULE_HEADER          *CapsuleHeader;
  EFI_PHYSICAL_ADDRESS        BodyBaseAddress;
  UINT32                      Size; 
  UINT8                       CapsuleNumber;
  UINT8                       CapsuleTotalNumber;
  EFI_CAPSULE_TABLE           *CapsuleTable; 
  VOID                        *AddDataPtr;
  UINT32                      *DataPtr;
  UINT32                      *BeginPtr;
  UINT32                      Index;
  UINT32                      PopulateIndex;

  PopulateIndex = 0;
  CapsuleNumber = 0;
  CapsuleTotalNumber = 0;
  AddDataPtr   =  NULL;
  DataPtr      =  NULL;
  BeginPtr     =  NULL;
#endif

  //
  // We don't do anything else if the boot mode is not flash-update
  //
  if (BootMode != BOOT_ON_FLASH_UPDATE) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Get the HOB list so we can determine the boot mode, and (if update mode)
  // look for capsule hobs.
  //
  Status = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, (VOID *) &HobList);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EFI_SUCCESS;
  //
  // If flash update mode, then walk the hobs to find capsules and produce
  // FVB protocols for them.
  //
  if ((HobList->Header.HobType == EFI_HOB_TYPE_HANDOFF) && (HobList->BootMode == BOOT_ON_FLASH_UPDATE)) {
    //
    // Only one capsule HOB allowed.
    //
    HobStatus = GetNextCapsuleVolumeHob (&HobList, &BaseAddress, &Length);
    if (EFI_ERROR (HobStatus)) {
      //
      // We didn't find a hob, so had no errors.
      //
      BdsLockNonUpdatableFlash ();
      return EFI_SUCCESS;
    }
    //
    // Now walk the capsule and call the core to process each
    // firmware volume in it.
    //
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
    AddDataPtr = (VOID *)((UINTN)BaseAddress + (UINTN)Length);
    AddDataPtr = (UINT8 *) (((UINTN) AddDataPtr + sizeof(UINT32) - 1) &~ (UINTN) (sizeof (UINT32) - 1));
    DataPtr = (UINT32*)AddDataPtr;
    CapsuleTotalNumber = (UINT8)*DataPtr++;
    BeginPtr = DataPtr;

    //
    //Check the capsule flags,if contains CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE, install 
    //capsuleTable to configure table with EFI_CAPSULE_GUID
    //
    DataPtr = BeginPtr;
    for (Index = 0; Index < CapsuleTotalNumber; Index++) {
      CapsuleHeader = (EFI_CAPSULE_HEADER*)((UINTN)BaseAddress +  (UINT32)(UINTN)*DataPtr++);
      if ((CapsuleHeader->Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) != 0) {
         CapsuleNumber ++;
      }
    }

    Size = sizeof(EFI_CAPSULE_TABLE) + (CapsuleNumber -1)* sizeof(VOID*);  
    Status  = gBS->AllocatePool (EfiRuntimeServicesData, Size, (VOID **) &CapsuleTable);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    DataPtr = BeginPtr;
    for (Index = 0; Index < CapsuleTotalNumber; Index++) {
      CapsuleHeader = (EFI_CAPSULE_HEADER*)((UINTN)BaseAddress +  (UINT32)(UINTN)*DataPtr++);
      if ((CapsuleHeader->Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) != 0) {
        CapsuleTable->CapsulePtr[PopulateIndex++] = (VOID*)CapsuleHeader;
      }
    }
    CapsuleTable->CapsuleArrayNumber =  PopulateIndex;
    Status = gBS->InstallConfigurationTable (&mEfiCapsuleHeaderGuid, (VOID*)CapsuleTable);
    ASSERT_EFI_ERROR (Status);

    DataPtr = BeginPtr;
    for (Index = 0; Index < CapsuleTotalNumber; Index++) {
      CapsuleHeader = (EFI_CAPSULE_HEADER*)((UINTN)BaseAddress + (UINT32)(UINTN)*DataPtr++);
      if ((CapsuleHeader->Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) == 0) {
        //
        //Skip the capsule header, move to the Firware Volume
        //
        BodyBaseAddress = (EFI_PHYSICAL_ADDRESS)CapsuleHeader + CapsuleHeader->OffsetToCapsuleBody;
        Length = CapsuleHeader->CapsuleImageSize - CapsuleHeader->OffsetToCapsuleBody;

        while (Length != 0) {
          //
          // Point to the next firmware volume header, and then
          // call the DXE service to process it.
          //
          FwVolHeader = (EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) BodyBaseAddress;
          if (FwVolHeader->FvLength > Length) {
            //
            // Notes: need to stuff this status somewhere so that the
            // error can be detected at OS runtime
            //
            Status = EFI_VOLUME_CORRUPTED;
            break;
          }
  
          Status = gDS->ProcessFirmwareVolume (
                        (VOID *) (UINTN) BodyBaseAddress,
                        (UINTN) FwVolHeader->FvLength,
                        &FvProtocolHandle
                        );
          if (EFI_ERROR (Status)) {
            break;
          }
          //
          // Call the dispatcher to dispatch any drivers from the produced firmware volume
          //
          gDS->Dispatch ();
          //
          // On to the next FV in the capsule
          //
          Length -= FwVolHeader->FvLength;
          BodyBaseAddress = (EFI_PHYSICAL_ADDRESS) ((UINTN) BodyBaseAddress + FwVolHeader->FvLength);
          //
          // Notes: when capsule spec is finalized, if the requirement is made to
          // have each FV in a capsule aligned, then we will need to align the
          // BaseAddress and Length here.
          //
        }
      }   
    } 
#else
    while (Length != 0) {
      //
      // Point to the next firmware volume header, and then
      // call the DXE service to process it.
      //
      FwVolHeader = (EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) BaseAddress;
      if (FwVolHeader->FvLength > Length) {
        //
        // Notes: need to stuff this status somewhere so that the
        // error can be detected at OS runtime
        //
        Status = EFI_VOLUME_CORRUPTED;
        break;
      }

      Status = gDS->ProcessFirmwareVolume (
                      (VOID *) (UINTN) BaseAddress,
                      (UINTN) FwVolHeader->FvLength,
                      &FvProtocolHandle
                      );
      if (EFI_ERROR (Status)) {
        break;
      }
      //
      // Call the dispatcher to dispatch any drivers from the produced firmware volume
      //
      gDS->Dispatch ();
      //
      // On to the next FV in the capsule
      //
      Length -= FwVolHeader->FvLength;
      BaseAddress = (EFI_PHYSICAL_ADDRESS) ((UINTN) BaseAddress + FwVolHeader->FvLength);
      //
      // Notes: when capsule spec is finalized, if the requirement is made to
      // have each FV in a capsule aligned, then we will need to align the
      // BaseAddress and Length here.
      //
    }
#endif
  }
  BdsLockNonUpdatableFlash ();

  return Status;
}

STATIC
EFI_STATUS
GetNextCapsuleVolumeHob (
  IN OUT VOID                  **HobStart,
  OUT    EFI_PHYSICAL_ADDRESS  *BaseAddress,
  OUT    UINT64                *Length
  )
/*++

Routine Description:
  Find the next Capsule volume HOB

Arguments:
  HobStart    - start of HOBs
  BaseAddress - returned base address of capsule volume
  Length      - length of capsule volume pointed to by BaseAddress

Returns:
  EFI_SUCCESS     - one found
  EFI_NOT_FOUND   - did not find one

--*/
{
  EFI_PEI_HOB_POINTERS  Hob;

  Hob.Raw = *HobStart;
  if (END_OF_HOB_LIST (Hob)) {
    return EFI_NOT_FOUND;
  }

  Hob.Raw = GetHob (EFI_HOB_TYPE_CV, *HobStart);
  if (Hob.Header->HobType != EFI_HOB_TYPE_CV) {
    return EFI_NOT_FOUND;
  }

  *BaseAddress  = Hob.CapsuleVolume->BaseAddress;
  *Length       = Hob.CapsuleVolume->Length;

  *HobStart     = GET_NEXT_HOB (Hob);

  return EFI_SUCCESS;
}
