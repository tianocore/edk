/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CapsuleService.c

Abstract:

  Capsule Runtime Service.

--*/

#include "Capsule.h"
#include "CapsuleService.h"
#include EFI_GUID_DEFINITION(Capsule)

#define SUPPORT_MAX_SIZE               (10*1024*1024)
STATIC EFI_GUID mEfiCapsuleHeaderGuid = EFI_CAPSULE_GUID;


EFI_STATUS
EFIAPI
UpdateCapsule (
  IN UEFI_CAPSULE_HEADER     **CapsuleHeaderArray,
  IN UINTN                   CapsuleCount,
  IN EFI_PHYSICAL_ADDRESS    ScatterGatherList OPTIONAL
  )
/*++

Routine Description:

  This code finds if the capsule needs reset to update, if no, update immediately.

Arguments:

  CapsuleHeaderArray             A array of pointers to capsule headers passed in
  CapsuleCount                   The number of capsule
  ScatterGatherList              Physical address of datablock list points to capsule
  
Returns:

  EFI STATUS
  EFI_SUCCESS                    Valid capsule was passed.If CAPSULE_FLAG_PERSIT_ACROSS_RESET is
                                 not set, the capsule has been successfully processed by the firmware.
                                 If it set, the ScattlerGatherList is successfully to be set.
  EFI_INVALID_PARAMETER          CapsuleCount is less than 1,CapsuleGuid is not supported.
  EFI_DEVICE_ERROR               Failed to SetVariable or AllocatePool or ProcessFirmwareVolume. 
  
--*/
{
  UINTN                     DataSize;
  UINTN                     CapsuleSize;
  UINTN                     ArrayNumber;
  VOID                      *AllocatedBuffer;
  UINT8                     *BufferPtr;
  EFI_STATUS                Status;
  EFI_HANDLE                FvHandle;
  UEFI_CAPSULE_HEADER       *CapsuleHeader;

  if (CapsuleCount < 1) {
    return EFI_INVALID_PARAMETER;
  }

  DataSize        = 0;
  BufferPtr       = NULL;
  AllocatedBuffer = NULL;
  CapsuleHeader   = NULL;

  //
  //now just support EFI_CAPSULE_GUID
  //
  for (ArrayNumber = 0; ArrayNumber < CapsuleCount; ArrayNumber++) {
    CapsuleHeader = CapsuleHeaderArray[ArrayNumber];
    if (!EfiCompareGuid (&CapsuleHeader->CapsuleGuid, &mEfiCapsuleHeaderGuid)) {
      return EFI_INVALID_PARAMETER;
    }   
    DataSize += CapsuleHeader->CapsuleImageSize - CapsuleHeader->HeaderSize;
  }

  CapsuleHeader = CapsuleHeaderArray[0];

  if ((CapsuleHeader->Flags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET) != 0) {
    if (ScatterGatherList == 0) {
      return EFI_INVALID_PARAMETER;
    } else {
      Status = EfiSetVariable (
                 EFI_CAPSULE_VARIABLE_NAME,  
                 &gEfiCapsuleVendorGuid,     
                 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,  
                 sizeof (UINTN), 
                 (VOID *) &ScatterGatherList 
                 );
      if (Status != EFI_SUCCESS) { 
        return EFI_DEVICE_ERROR;
      }
    }
    return EFI_SUCCESS;
  }
  
  //
  //the rest occurs in the condition of non-reset mode
  //
  if (EfiAtRuntime ()) { 
    return EFI_INVALID_PARAMETER;
  }

  //
  //in the boottime,concatenate split capsules into a signle big capsule
  //
  Status = gBS->AllocatePool (EfiBootServicesData, DataSize, &AllocatedBuffer);

  if (Status != EFI_SUCCESS) {
    goto Done;
  }
  
  BufferPtr = AllocatedBuffer;

  for (ArrayNumber = 0; ArrayNumber < CapsuleCount ; ArrayNumber++) {
    CapsuleHeader = CapsuleHeaderArray[ArrayNumber];
    CapsuleSize = CapsuleHeader->CapsuleImageSize - CapsuleHeader->HeaderSize;
    gBS->CopyMem (BufferPtr, (UINT8*)CapsuleHeader+ CapsuleHeader->HeaderSize, CapsuleSize);
    BufferPtr += CapsuleSize;
  }

  //
  //call DXE service ProcessFirmwareVolume to process FV
  //
  Status = gDS->ProcessFirmwareVolume (AllocatedBuffer, DataSize, &FvHandle);
  if (Status != EFI_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }

  gDS->Dispatch ();

  return EFI_SUCCESS;

Done:
  if (AllocatedBuffer != NULL) {
    gBS->FreePool (AllocatedBuffer);
  }     
  return EFI_DEVICE_ERROR;
}


  
EFI_STATUS
EFIAPI
QueryCapsuleCapabilities(
  IN  UEFI_CAPSULE_HEADER  **CapsuleHeaderArray,
  IN  UINTN                CapsuleCount,
  OUT UINT64               *MaxiumCapsuleSize,
  OUT EFI_RESET_TYPE       *ResetType
  )
/*++

Routine Description:

  This code is query about capsule capability.

Arguments:

  CapsuleHeaderArray              A array of pointers to capsule headers passed in
  CapsuleCount                    The number of capsule
  MaxiumCapsuleSize               Max capsule size is supported
  ResetType                       Reset type the capsule indicates, if reset is not needed,return EfiResetCold.
                                  If reset is needed, return EfiResetWarm.

Returns:

  EFI STATUS
  EFI_SUCCESS                     Valid answer returned
  EFI_INVALID_PARAMETER           MaxiumCapsuleSize is NULL,ResetType is NULL.CapsuleCount is less than 1,CapsuleGuid is not supported.
  EFI_UNSUPPORTED                 The capsule type is not supported.

--*/
{
  UINTN                     ArrayNumber;
  UEFI_CAPSULE_HEADER       *CapsuleHeader;

  if (CapsuleCount < 1) {
    return EFI_INVALID_PARAMETER;
  }

  if ((MaxiumCapsuleSize == NULL) ||(ResetType == NULL)) {
    return EFI_INVALID_PARAMETER;
  }  

  CapsuleHeader = NULL;

  //
  //now just support EFI_CAPSULE_GUID
  //
  for (ArrayNumber = 0; ArrayNumber < CapsuleCount; ArrayNumber++) {
    CapsuleHeader = CapsuleHeaderArray[ArrayNumber];
    if (!EfiCompareGuid (&CapsuleHeader->CapsuleGuid, &mEfiCapsuleHeaderGuid)) {
      return EFI_UNSUPPORTED;
    }   
  }

  *MaxiumCapsuleSize = SUPPORT_MAX_SIZE;

  CapsuleHeader = CapsuleHeaderArray[0];  
  if ((CapsuleHeader->Flags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET) != 0) {
    *ResetType = EfiResetWarm;
  } else {
    *ResetType = EfiResetCold;
  }  
  return EFI_SUCCESS;
} 


