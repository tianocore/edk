/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  FlashMap.c
   
Abstract:

  EFI 2.0 PEIM to build GUIDed HOBs for platform specific flash map

--*/

#include "Tiano.h"
#include "Pei.h"
#include "PeiLib.h"

#include "FlashLayout.h"

#include EFI_GUID_DEFINITION (FlashMapHob)
#include EFI_PPI_DEFINITION (FlashMap)
#include EFI_PPI_DEFINITION (NtFwh)

#include EFI_GUID_DEFINITION (SystemNvDataGuid)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeBlock)
#include EFI_GUID_DEFINITION (FirmwareFileSystem)

EFI_GUID  mFvBlockGuid = EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL_GUID;
EFI_GUID  mFfsGuid = EFI_FIRMWARE_FILE_SYSTEM_GUID;
EFI_GUID  mSystemDataGuid = EFI_SYSTEM_NV_DATA_HOB_GUID;

EFI_STATUS
GetAreaInfo (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN PEI_FLASH_MAP_PPI   *This,
  IN  EFI_FLASH_AREA_TYPE         AreaType,
  IN  EFI_GUID                    *AreaTypeGuid,
  OUT UINT32                      *NumEntries,
  OUT EFI_FLASH_SUBAREA_ENTRY     **Entries
  );

EFI_STATUS
EFIAPI
MemoryDiscoveredPpiNotifyCallback (
  IN EFI_PEI_SERVICES       **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                   *Ppi
  );

//
// Module globals
//
static PEI_FLASH_MAP_PPI mFlashMapPpi = {
  GetAreaInfo
};

static EFI_PEI_PPI_DESCRIPTOR mPpiListFlashMap = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiFlashMapPpiGuid,
  &mFlashMapPpi
};

static  EFI_FLASH_AREA_DATA mFlashAreaData[] = {
  // Variable area
  { EFI_VARIABLE_STORE_OFFSET,
    EFI_VARIABLE_STORE_LENGTH,
    EFI_FLASH_AREA_SUBFV | EFI_FLASH_AREA_MEMMAPPED_FV,
    EFI_FLASH_AREA_EFI_VARIABLES },

  // FTW spare (backup) block
  { EFI_WINNT_FTW_SPARE_BLOCK_OFFSET,
    EFI_WINNT_FTW_SPARE_BLOCK_LENGTH,
    EFI_FLASH_AREA_SUBFV | EFI_FLASH_AREA_MEMMAPPED_FV,
    EFI_FLASH_AREA_FTW_BACKUP },

  // FTW private working (state) area
  { EFI_FTW_WORKING_OFFSET,
    EFI_FTW_WORKING_LENGTH,
    EFI_FLASH_AREA_SUBFV | EFI_FLASH_AREA_MEMMAPPED_FV,
    EFI_FLASH_AREA_FTW_STATE },

  // Recovery FV
  { EFI_WINNT_FIRMWARE_OFFSET,
    EFI_WINNT_FIRMWARE_LENGTH,
    EFI_FLASH_AREA_FV | EFI_FLASH_AREA_MEMMAPPED_FV,
    EFI_FLASH_AREA_RECOVERY_BIOS },

  // System Non-Volatile Storage FV
  { EFI_WINNT_RUNTIME_UPDATABLE_OFFSET,
    EFI_WINNT_RUNTIME_UPDATABLE_LENGTH + EFI_WINNT_FTW_SPARE_BLOCK_LENGTH,
    EFI_FLASH_AREA_FV | EFI_FLASH_AREA_MEMMAPPED_FV,
    EFI_FLASH_AREA_GUID_DEFINED },
};

EFI_STATUS
EFIAPI
PeimInitializeFlashMap (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  );
  
EFI_PEIM_ENTRY_POINT (PeimInitializeFlashMap);
  
EFI_STATUS
EFIAPI
PeimInitializeFlashMap (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Build GUIDed HOBs for platform specific flash map
  
Arguments:
  
  FfsHeader   - A pointer to the EFI_FFS_FILE_HEADER structure.
  PeiServices - General purpose services available to every PEIM.
    
Returns:

  EFI_STATUS

--*/
{
  EFI_STATUS                    Status;

  PEI_NT_FWH_CALLBACK_PROTOCOL  *PeiNtService;
  EFI_PEI_PPI_DESCRIPTOR            *PpiDescriptor;

  UINT64                        FwhSize;
  EFI_PHYSICAL_ADDRESS          FwhBase;

  UINTN                         NumOfHobData;
  UINTN                         Index;
  
  EFI_FLASH_AREA_HOB_DATA       FlashHobData;
    
  //
  // Install FlashMap PPI
  //
  Status = (**PeiServices).InstallPpi (PeiServices, &mPpiListFlashMap);
  ASSERT_PEI_ERROR (PeiServices, Status);

  PEI_DEBUG ((PeiServices, EFI_D_ERROR, "Flash Map PEIM Loaded\n"));

  //
  // Build GUIDed HOBs for platform specific flash map
  //
  Status =  (**PeiServices).LocatePpi ( 
                              PeiServices,
                              &gPeiFwhInformationGuid,     // GUID
                              0,                           // INSTANCE
                              &PpiDescriptor,              // EFI_PEI_PPI_DESCRIPTOR
                              &PeiNtService                // PPI
                              );
  ASSERT_PEI_ERROR (PeiServices, Status);  

  Status = PeiNtService->NtFwh (&FwhSize, &FwhBase);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get number of types
  //
  NumOfHobData = sizeof (mFlashAreaData) / sizeof (EFI_FLASH_AREA_DATA);
  
  //
  // Build flash area entries as GUIDed HOBs.
  //
  for (Index = 0; Index < NumOfHobData; Index++) {
    (*PeiServices)->SetMem (&FlashHobData, sizeof (EFI_FLASH_AREA_HOB_DATA), 0);

    FlashHobData.AreaType               = mFlashAreaData[Index].AreaType;
    FlashHobData.NumberOfEntries        = 1;
    FlashHobData.SubAreaData.Attributes = mFlashAreaData[Index].Attributes;
    FlashHobData.SubAreaData.Base       = FwhBase + (EFI_PHYSICAL_ADDRESS)(UINTN)mFlashAreaData[Index].Base;
    FlashHobData.SubAreaData.Length     = (EFI_PHYSICAL_ADDRESS)(UINTN)mFlashAreaData[Index].Length;

    switch (FlashHobData.AreaType) {
      case EFI_FLASH_AREA_RECOVERY_BIOS:
      case EFI_FLASH_AREA_MAIN_BIOS:
        (*PeiServices)->CopyMem (
                          &FlashHobData.AreaTypeGuid,
                          &mFfsGuid,
                          sizeof (EFI_GUID)
                          );
        (*PeiServices)->CopyMem (
                          &FlashHobData.SubAreaData.FileSystem,
                          &mFvBlockGuid,
                          sizeof (EFI_GUID)
                          );
        break;
      case EFI_FLASH_AREA_GUID_DEFINED:
        (*PeiServices)->CopyMem (
                          &FlashHobData.AreaTypeGuid,
                          &mSystemDataGuid,
                          sizeof (EFI_GUID)
                          );
        (*PeiServices)->CopyMem (
                          &FlashHobData.SubAreaData.FileSystem,
                          &mFvBlockGuid,
                          sizeof (EFI_GUID)
                          );
        break;
      default:
        break;
    }

    Status =  PeiBuildHobGuidData (
                PeiServices,
                &gEfiFlashMapHobGuid,
                &FlashHobData,
                sizeof (EFI_FLASH_AREA_HOB_DATA)
                );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
GetAreaInfo (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN PEI_FLASH_MAP_PPI   *This,
  IN  EFI_FLASH_AREA_TYPE         AreaType,
  IN  EFI_GUID                    *AreaTypeGuid,
  OUT UINT32                      *NumEntries,
  OUT EFI_FLASH_SUBAREA_ENTRY     **Entries
  )
/*++

  Routine Description:
    
    Implementation of Flash Map PPI
    
--*/
{
  EFI_STATUS        Status;
  EFI_PEI_HOB_POINTERS  Hob;
  EFI_HOB_FLASH_MAP_ENTRY_TYPE *FlashMapEntry;

  Status = (*PeiServices)->GetHobList (PeiServices, &Hob.Raw);
  while (!END_OF_HOB_LIST (Hob)) {
    if (Hob.Header->HobType == EFI_HOB_TYPE_GUID_EXTENSION &&
      CompareGuid ( &Hob.Guid->Name, &gEfiFlashMapHobGuid)) {
      FlashMapEntry = (EFI_HOB_FLASH_MAP_ENTRY_TYPE *)Hob.Raw;
      if (AreaType == FlashMapEntry->AreaType) {
        if (AreaType == EFI_FLASH_AREA_GUID_DEFINED) {
          if (!CompareGuid (AreaTypeGuid, &FlashMapEntry->AreaTypeGuid)) {
            continue;
          }
        }
        *NumEntries = FlashMapEntry->NumEntries;
        *Entries = FlashMapEntry->Entries;
        return EFI_SUCCESS;
      }
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }
 
  return EFI_NOT_FOUND;
}


