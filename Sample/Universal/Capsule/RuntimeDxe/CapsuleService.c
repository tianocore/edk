/*++

Copyright (c) 2006 - 2007, Intel Corporation                                                         
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


EFI_STATUS
EFIAPI
UpdateCapsule (
  IN EFI_CAPSULE_HEADER      **CapsuleHeaderArray,
  IN UINTN                   CapsuleCount,
  IN EFI_PHYSICAL_ADDRESS    ScatterGatherList OPTIONAL
  )
/*++

Routine Description:

  Pass capsules to firmware with both virtual and physical mapping.
  Depending on the intended consumption,the firmware may process the
  capsule immediately. If the payload should persist across a system
  reset, the reset value returned from QueryCapsuleCapabilities() must
  be passed into ResetSystem() and will cause the capsule to be processed
  by firmware as part of the reset process.

Arguments:

  CapsuleHeaderArray             A array of pointers to capsule headers passed in
  CapsuleCount                   The number of capsule
  ScatterGatherList              Physical address of datablock list points to capsule
  
Returns:

  EFI STATUS
  EFI_SUCCESS                    Valid capsule was passed.
  EFI_INVALID_PARAMETER          Invalid capsule was passed. 
  EFI_DEVICE_ERROR               Capsule update was started,but failed due to a device error.
  
--*/
{
  EFI_STATUS   Status;
  BOOLEAN      NeedResetFlag;

  NeedResetFlag   = FALSE;

  //
  // Any capsule is recognized by CapsuleGuid. It is known to every plaform driver.
  //
  Status  = CheckCapsuleGuid (CapsuleHeaderArray, CapsuleCount);
  if (EFI_ERROR(Status)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // To capsules who has no reset flag, launch them immediately. How to treat a 
  // particular capsule is driven by the CapsuleGuid, platform has knowledge 
  // of it.
  //
  Status = LaunchCapsule(CapsuleHeaderArray, CapsuleCount, &NeedResetFlag);
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
  
  //
  // After launching all capsules who has no reset flag, if no more capsules claims
  // for a system reset just return.
  //
  if (!NeedResetFlag) {
    return EFI_SUCCESS;
  }

  //
  // To capsule who has reset flag, firmware will process it after system reset. Patform
  // should guarantee the memory integrity across a reset. It is platform's obligation
  // to use appropriate approach to do a reset, S3, WarmRest and etc. 
  //
  if (!SupportUpdateCapsuleRest()) {
    return EFI_DEVICE_ERROR;
  }

  //
  // ScatterGatherList is only referenced if the capsules are defined to persist across
  // system reset. Set its value into NV storage to let pre-boot driver to pick it up 
  // after coming through a system reset.
  //
  Status = EfiSetVariable (
             EFI_CAPSULE_VARIABLE_NAME,  
             &gEfiCapsuleVendorGuid,     
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,  
             sizeof (UINTN), 
             (VOID *) &ScatterGatherList 
             );
  if (EFI_ERROR(Status)) { 
    return Status;
  }
  return EFI_SUCCESS;
}


  
EFI_STATUS
EFIAPI
QueryCapsuleCapabilities (
  IN  EFI_CAPSULE_HEADER   **CapsuleHeaderArray,
  IN  UINTN                CapsuleCount,
  OUT UINT64               *MaxiumCapsuleSize,
  OUT EFI_RESET_TYPE       *ResetType
  )
/*++

Routine Description:

  The function allows a caller to test to see if a capsule or capsules can be updated
  via UpdateCapsule().The flags values in the capsule header and size of the entire
  capsule is checked.

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
  EFI_CAPSULE_HEADER        *CapsuleHeader;
  UINT32                    MaxSizePopulate;
  UINT32                    MaxSizeNonPopulate;
  EFI_STATUS                Status;
  BOOLEAN                   NeedReset;

  NeedReset = FALSE;
  //
  // Any capsule is recognized by CapsuleGuid. It is known to every plaform driver.
  //
  Status  = CheckCapsuleGuid (CapsuleHeaderArray, CapsuleCount);
  if (EFI_ERROR(Status)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((MaxiumCapsuleSize == NULL) ||(ResetType == NULL)) {
    return EFI_INVALID_PARAMETER;
  }  

  //
  // Every platform has different capability to support capsules, generally it depends on
  // platform memory size, and meanwhile it varies to capsule type.
  //
  SupportCapsuleSize(&MaxSizePopulate,&MaxSizeNonPopulate);

  //
  // Find out whether there is any capsule defined to persist across system reset. 
  //
  for (ArrayNumber = 0; ArrayNumber < CapsuleCount ; ArrayNumber++) {
    CapsuleHeader = CapsuleHeaderArray[ArrayNumber];
    if ((CapsuleHeader->Flags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET) != 0) {
      NeedReset = TRUE;
    }
  }
  
  if (NeedReset) {
    //
    // Capsule needs reset, and if platfrom has no capability to guarantee the 
    // memory integrity across a reset, return unsupported.
    //
    if (!SupportUpdateCapsuleRest()) {
      return EFI_UNSUPPORTED;
    }
    //
    // ResetType returns the type of reset required for the capsule update. Since it
    // is type of EFI_RESET_TYPE, we use EfiResetWarm to indicate reset flag existing
    // in capsule header. Whereas, EfiResetCold indicates no reset flag in capsule header.
    //
    *ResetType = EfiResetWarm;
    *MaxiumCapsuleSize = MaxSizePopulate;    
  } else {
    *ResetType = EfiResetCold;
    *MaxiumCapsuleSize = MaxSizeNonPopulate;
  }  
  return EFI_SUCCESS;
} 
