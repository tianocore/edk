/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MemoryServices.c

Abstract:

  EFI PEI Core memory services

--*/

#include "Tiano.h"
#include "PeiApi.h"
#include "PeiCore.h"
#include "PeiLib.h"  

VOID
InitializeMemoryServices (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_PEI_STARTUP_DESCRIPTOR  *PeiStartupDescriptor,
  IN PEI_CORE_INSTANCE           *OldCoreData
  )
/*++

Routine Description:

  Initialize the memory services.

Arguments:

  PeiServices          - The PEI core services table.
  PeiStartupDescriptor - Information and services provided by SEC phase.
  OldCoreData          - Pointer to the PEI Core data.
                         NULL if being run in non-permament memory mode.

Returns:

  None

--*/
{
  PEI_CORE_INSTANCE                    *PrivateData;
  UINT64                               SizeOfCarHeap;

  PrivateData = PEI_CORE_INSTANCE_FROM_PS_THIS (PeiServices);
  PrivateData->SwitchStackSignal = FALSE;

  if (OldCoreData == NULL) {

    PrivateData->PeiMemoryInstalled = FALSE;

    PrivateData->BottomOfCarHeap = (VOID *) (((UINTN)(VOID *)(&PrivateData))
                                   & (~((PeiStartupDescriptor->SizeOfCacheAsRam) - 1))); 
    PrivateData->TopOfCarHeap = (VOID *)((UINTN)(PrivateData->BottomOfCarHeap) + PeiStartupDescriptor->SizeOfCacheAsRam);
    //
    // SizeOfCarHeap is 1/2 (arbitrary) of CacheAsRam Size.
    //
    SizeOfCarHeap = (UINT64) PeiStartupDescriptor->SizeOfCacheAsRam;
    SizeOfCarHeap = RShiftU64 (SizeOfCarHeap, 1);
 
    PEI_DEBUG_CODE (
      {
        PrivateData->SizeOfCacheAsRam = PeiStartupDescriptor->SizeOfCacheAsRam;
        PrivateData->MaxTopOfCarHeap  = (VOID *) ((UINTN) PrivateData->BottomOfCarHeap + (UINTN) SizeOfCarHeap);
      }
    )

    PrivateData->HobList.Raw = PrivateData->BottomOfCarHeap;
    
    PeiCoreBuildHobHandoffInfoTable (
      BOOT_WITH_FULL_CONFIGURATION,
      (EFI_PHYSICAL_ADDRESS) (UINTN) PrivateData->BottomOfCarHeap,
      (UINTN) SizeOfCarHeap
      );
    //
    // Copy PeiServices from ROM to Cache in PrivateData
    //
    PeiCoreCopyMem (&(PrivateData->ServiceTableShadow), *PeiServices, sizeof (EFI_PEI_SERVICES));

    //
    // Set PS to point to ServiceTableShadow in Cache
    //
    PrivateData->PS = &(PrivateData->ServiceTableShadow);
  } else {
  //                                                                    
  // Set PS to point to ServiceTableShadow in Cache one time after the  
  // stack switched to main memory                                      
  //                                                                    
  PrivateData->PS = &(PrivateData->ServiceTableShadow);                 
}                                                                       

  return;
}

EFI_STATUS
EFIAPI
PeiInstallPeiMemory (
  IN EFI_PEI_SERVICES      **PeiServices,
  IN EFI_PHYSICAL_ADDRESS  MemoryBegin,
  IN UINT64                MemoryLength
  )
/*++

Routine Description:

  Install the permanent memory is now available.
  Creates HOB (PHIT and Stack).

Arguments:

  PeiServices   - The PEI core services table.
  MemoryBegin   - Start of memory address.
  MemoryLength  - Length of memory.

Returns:

  Status  - EFI_SUCCESS
            
--*/
{
  PEI_CORE_INSTANCE                     *PrivateData;
  EFI_HOB_HANDOFF_INFO_TABLE            *OldHandOffHob;
  EFI_HOB_HANDOFF_INFO_TABLE            *NewHandOffHob;
  UINT64                                PeiStackSize;
  UINT64                                EfiFreeMemorySize;
  EFI_PHYSICAL_ADDRESS                  PhysicalAddressOfOldHob;
  
  PrivateData = PEI_CORE_INSTANCE_FROM_PS_THIS (PeiServices);

  PrivateData->SwitchStackSignal = TRUE;
  PrivateData->PeiMemoryInstalled = TRUE;

  PrivateData->StackBase = MemoryBegin;
  
  PeiStackSize = RShiftU64 (MemoryLength, 1);
  if (PEI_STACK_SIZE > PeiStackSize) {
    PrivateData->StackSize = PeiStackSize;
  } else {
    PrivateData->StackSize = PEI_STACK_SIZE;
  }

  OldHandOffHob = PrivateData->HobList.HandoffInformationTable;

  PrivateData->HobList.Raw = (VOID *)((UINTN)(MemoryBegin + PrivateData->StackSize));
  NewHandOffHob = PrivateData->HobList.HandoffInformationTable;
  PhysicalAddressOfOldHob = (EFI_PHYSICAL_ADDRESS) (UINTN) OldHandOffHob;

  EfiFreeMemorySize = OldHandOffHob->EfiFreeMemoryBottom - PhysicalAddressOfOldHob;
  
  PeiCoreCopyMem (
    NewHandOffHob,
    OldHandOffHob,
    (UINTN)EfiFreeMemorySize
    );

  NewHandOffHob->EfiMemoryTop     = MemoryBegin + MemoryLength;
  NewHandOffHob->EfiFreeMemoryTop = NewHandOffHob->EfiMemoryTop;
  NewHandOffHob->EfiMemoryBottom  = MemoryBegin;
  
  NewHandOffHob->EfiFreeMemoryBottom = (UINTN)NewHandOffHob + EfiFreeMemorySize;                                     
                                       
  NewHandOffHob->EfiEndOfHobList     = (UINTN)NewHandOffHob +
                                       (OldHandOffHob->EfiEndOfHobList -
                                        PhysicalAddressOfOldHob);

  ConvertPpiPointers (PeiServices, OldHandOffHob, NewHandOffHob);

  PeiBuildHobStack (PeiServices, PrivateData->StackBase, PrivateData->StackSize);

  PEI_DEBUG_CODE (
    {
      PEI_DEBUG ((PeiServices, EFI_D_INFO, "HOBLIST address before memory init = 0x%08x\n", OldHandOffHob));
      PEI_DEBUG ((PeiServices, EFI_D_INFO, "HOBLIST address after memory init = 0x%08x\n", NewHandOffHob));
    }
  )  

  return EFI_SUCCESS;   
}


EFI_STATUS
EFIAPI
PeiAllocatePages (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_MEMORY_TYPE            MemoryType,
  IN UINTN                      Pages,
  OUT EFI_PHYSICAL_ADDRESS      *Memory
  )
/*++

Routine Description:

  Memory allocation service on permanent memory, 
  not usable prior to the memory installation.

Arguments:

  PeiServices - The PEI core services table.
  MemoryType  - Type of memory to allocate.
  Pages       - Number of pages to allocate.
  Memory      - Pointer of memory allocated.

Returns:

  Status - EFI_SUCCESS              The allocation was successful
           EFI_INVALID_PARAMETER    Only AllocateAnyAddress is supported.
           EFI_NOT_AVAILABLE_YET    Called with permanent memory not available
           EFI_OUT_OF_RESOURCES     There is not enough HOB heap to satisfy the requirement
                                    to allocate the number of pages.

--*/
{
  PEI_CORE_INSTANCE                       *PrivateData;
  EFI_PEI_HOB_POINTERS                    Hob;
  EFI_PHYSICAL_ADDRESS                    Offset;
  EFI_STATUS                              Status;

  PrivateData = PEI_CORE_INSTANCE_FROM_PS_THIS (PeiServices);

  //
  // Check if Hob already available
  //
  if (!PrivateData->PeiMemoryInstalled) {
    return EFI_NOT_AVAILABLE_YET;
  }

  Hob.Raw = PrivateData->HobList.Raw;

  //
  // Check to see if on 4k boundary
  //
  Offset = Hob.HandoffInformationTable->EfiFreeMemoryTop & 0xFFF;

  //
  // If not aligned, make the allocation aligned.
  //
  if (Offset != 0) {
    Hob.HandoffInformationTable->EfiFreeMemoryTop -= Offset;
  }

  //
  // Verify that there is sufficient memory to satisfy the allocation
  //
  if (Hob.HandoffInformationTable->EfiFreeMemoryTop - ((Pages * EFI_PAGE_SIZE) + sizeof (EFI_HOB_MEMORY_ALLOCATION)) < 
      Hob.HandoffInformationTable->EfiFreeMemoryBottom) {
    PEI_DEBUG_CODE (
      {
        PEI_DEBUG ((PeiServices, EFI_D_ERROR, "PeiAllocatePages fail: Pages - 0x%08x\n", Pages));
        PEI_DEBUG ((PeiServices, EFI_D_ERROR, "  FreeMemoryTop    - 0x%08x\n", (UINTN)Hob.HandoffInformationTable->EfiFreeMemoryTop));
        PEI_DEBUG ((PeiServices, EFI_D_ERROR, "  FreeMemoryBottom - 0x%08x\n", (UINTN)Hob.HandoffInformationTable->EfiFreeMemoryBottom));
      }
    )
    return  EFI_OUT_OF_RESOURCES;
  } else {
    //
    // Update the PHIT to reflect the memory usage
    //
    Hob.HandoffInformationTable->EfiFreeMemoryTop -= Pages * EFI_PAGE_SIZE;

    //
    // Update the value for the caller
    //
    *Memory = Hob.HandoffInformationTable->EfiFreeMemoryTop;

    //
    // Create a memory allocation HOB.
    //
    Status = PeiBuildHobMemoryAllocation (
               PeiServices,
               Hob.HandoffInformationTable->EfiFreeMemoryTop,
               Pages * EFI_PAGE_SIZE + Offset,
               NULL,
               MemoryType
               );

    return Status;
  }
}


EFI_STATUS
EFIAPI
PeiAllocatePool (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN UINTN                      Size,
  OUT VOID                      **Buffer
  )
/*++

Routine Description:

  Memory allocation service on the CAR.  

Arguments:

  PeiServices - The PEI core services table.

  Size        - Amount of memory required

  Buffer      - Address of pointer to the buffer

Returns:

  Status - EFI_SUCCESS              The allocation was successful
           EFI_OUT_OF_RESOURCES     There is not enough heap to satisfy the requirement
                                    to allocate the requested size.
                                    
--*/
{
  EFI_STATUS               Status;
  EFI_HOB_MEMORY_POOL      *Hob;


  Status = PeiCreateHob (
             PeiServices,
             EFI_HOB_TYPE_PEI_MEMORY_POOL,
             (UINT16)(sizeof (EFI_HOB_MEMORY_POOL) + Size),
             &Hob
             );
  *Buffer = Hob+1;  


  return Status;
}

VOID
EFIAPI
PeiCoreCopyMem (
  IN VOID                       *Destination,
  IN VOID                       *Source,
  IN UINTN                      Length
  )
/*++

Routine Description:

  Copy Length bytes from Source to Destination.

Arguments:

  Destination - Target of copy

  Source      - Place to copy from

  Length      - Number of bytes to copy

Returns:

  None

--*/
{
  EfiCommonLibCopyMem (Destination, Source, Length);
}

VOID
EFIAPI
PeiCoreSetMem (
  IN VOID                       *Buffer,
  IN UINTN                      Size,
  IN UINT8                      Value
  )
/*++

Routine Description:

  Set Size bytes at Buffer address with Value

Arguments:

  Buffer      - Target of set 

  Size        - Amount of memory to set

  Value       - Value to place in memory

Returns:

  None

--*/
{
  EfiCommonLibSetMem (Buffer, Size, Value);
}

