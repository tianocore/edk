/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PeiHobLib.h

Abstract:


--*/

#ifndef _EFI_PEI_HOB_LIB_H_
#define _EFI_PEI_HOB_LIB_H_

#include "PeiApi.h"   // EFI_PEI_SERVICES definition

#define EFI_STACK_SIZE          0x20000
#define EFI_BSP_STORE_SIZE      0x4000


EFI_STATUS
BuildHobHandoffInfoTable (
  IN  VOID                    *HobStart,
  IN  UINT16                  Version,
  IN  EFI_BOOT_MODE           BootMode,
  IN  EFI_PHYSICAL_ADDRESS    EfiMemoryTop,
  IN  EFI_PHYSICAL_ADDRESS    EfiMemoryBottom,
  IN  EFI_PHYSICAL_ADDRESS    EfiFreeMemoryTop,
  IN  EFI_PHYSICAL_ADDRESS    EfiFreeMemoryBottom
 );


EFI_STATUS
BuildHobModule (
  IN VOID                   *HobStart,
  IN EFI_GUID               *ModuleName,
  IN EFI_PHYSICAL_ADDRESS   Module,
  IN UINT64                 ModuleLength,
  IN EFI_PHYSICAL_ADDRESS   EntryPoint
  );

EFI_STATUS
BuildHobResourceDescriptor (
  IN VOID                        *HobStart,
  IN EFI_RESOURCE_TYPE           ResourceType,
  IN EFI_RESOURCE_ATTRIBUTE_TYPE ResourceAttribute,
  IN EFI_PHYSICAL_ADDRESS        PhysicalStart,
  IN UINT64                      NumberOfBytes
  );


EFI_STATUS
BuildHobGuidType (
  IN VOID                        *HobStart,
  IN EFI_GUID                    *Guid,
  IN VOID                        *Buffer,
  IN UINTN                       BufferSize
  );

EFI_STATUS
BuildHobFvDescriptor (
  IN VOID                        *HobStart,
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length
  );

EFI_STATUS
BuildHobCpu (
  IN VOID                        *HobStart,
  IN UINT8                       SizeOfMemorySpace,
  IN UINT8                       SizeOfIoSpace
  );

EFI_STATUS
BuildHobStack (
  IN VOID                        *HobStart,
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length
  );

EFI_STATUS
BuildHobBspStore (
  IN VOID                        *HobStart,
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length,
  IN EFI_MEMORY_TYPE             MemoryType
  );

EFI_STATUS
BuildMemoryAllocationHob (
  IN VOID                        *HobStart,
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length,
  IN EFI_GUID                    *Name,
  IN EFI_MEMORY_TYPE             MemoryType
  );

#endif







 