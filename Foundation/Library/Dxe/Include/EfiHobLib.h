/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    EfiHobLib.h

Abstract:

 
--*/

#ifndef _EFI_HOB_LIB_H_
#define _EFI_HOB_LIB_H_

#include "PeiHob.h"

VOID *
GetHob (
  IN UINT16  Type,
  IN VOID    *HobStart
  );

UINTN
GetHobListSize (
  IN VOID  *HobStart
  ); 

UINT32
GetHobVersion (
  IN VOID  *HobStart
  );

EFI_STATUS
GetHobBootMode (
  IN  VOID           *HobStart,
  OUT EFI_BOOT_MODE  *BootMode
  );

EFI_STATUS
GetCpuHobInfo (
  IN  VOID   *HobStart,
  OUT UINT8  *SizeOfMemorySpace,
  OUT UINT8  *SizeOfIoSpace
  );

EFI_STATUS
GetDxeCoreHobInfo (
  IN  VOID                  *HobStart,
  OUT EFI_PHYSICAL_ADDRESS  *BaseAddress,
  OUT UINT64                *Length,
  OUT VOID                  **EntryPoint,
  OUT EFI_GUID              **FileName
  );

EFI_STATUS
GetNextFirmwareVolumeHob (
  IN OUT VOID                  **HobStart,
  OUT    EFI_PHYSICAL_ADDRESS  *BaseAddress,
  OUT    UINT64                *Length
  );

EFI_STATUS
GetNextGuidHob (
  IN OUT VOID      **HobStart,
  IN     EFI_GUID  *Guid,
  OUT    VOID      **Buffer,
  OUT    UINTN     *BufferSize  OPTIONAL
  );

EFI_STATUS
GetPalEntryHobInfo (
  IN  VOID                  *HobStart,
  OUT EFI_PHYSICAL_ADDRESS  *PalEntry
  );

EFI_STATUS
GetIoPortSpaceAddressHobInfo (
  IN  VOID                  *HobStart,
  OUT EFI_PHYSICAL_ADDRESS  *IoPortSpaceAddress
  );

#endif
