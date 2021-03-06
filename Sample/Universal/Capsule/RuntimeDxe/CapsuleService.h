/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CapsuleService.h

Abstract:

  Capsule Runtime Service

--*/

#ifndef  _CAPSULE_RUNTIME_H_
#define  _CAPSULE_RUNTIME_H_

#include "Tiano.h"
#include "EfiCapsule.h"
#include "EfiRuntimeLib.h"


EFI_STATUS
EFIAPI
UpdateCapsule (
  IN EFI_CAPSULE_HEADER      **CapsuleHeaderArray,
  IN UINTN                   CapsuleCount,
  IN EFI_PHYSICAL_ADDRESS    ScatterGatherList OPTIONAL
  );

EFI_STATUS
EFIAPI
QueryCapsuleCapabilities (
  IN  EFI_CAPSULE_HEADER   **CapsuleHeaderArray,
  IN  UINTN                CapsuleCount,
  OUT UINT64               *MaxiumCapsuleSize,
  OUT EFI_RESET_TYPE       *ResetType
  );

EFI_STATUS
EFIAPI
CheckCapsuleGuid (
  IN EFI_CAPSULE_HEADER     **CapsuleHeaderArray,
  IN UINTN                  CapsuleCount
  );

EFI_STATUS
EFIAPI    
LaunchCapsule (
  IN EFI_CAPSULE_HEADER      **CapsuleHeaderArray,
  IN UINTN                   CapsuleCount,
  OUT BOOLEAN                *NeedReset
  );


BOOLEAN
EFIAPI
SupportUpdateCapsuleRest (
  VOID
  );

VOID
EFIAPI
SupportCapsuleSize (
  IN OUT UINT32 *MaxSizePopulate,
  IN OUT UINT32 *MaxSizeNonPopulate
  );


#endif

