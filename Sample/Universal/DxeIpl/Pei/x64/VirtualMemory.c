/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  VirtualMemory.c
  
Abstract:

  x64 Virtual Memory Management Services in the form of an IA-32 driver.  
  Used to establish a 1:1 Virtual to Physical Mapping that is required to
  enter Long Mode (x64 64-bit mode).

  While we make a 1:1 mapping (identity mapping) for all physical pages 
  we still need to use the MTRR's to ensure that the cachability attirbutes
  for all memory regions is correct.

  The basic idea is to use 2MB page table entries where ever possible. If
  more granularity of cachability is required then 4K page tables are used.

  References:
    1) IA-32 Intel(R) Atchitecture Software Developer's Manual Volume 1:Basic Architecture, Intel
    2) IA-32 Intel(R) Atchitecture Software Developer's Manual Volume 2:Instruction Set Reference, Intel
    3) IA-32 Intel(R) Atchitecture Software Developer's Manual Volume 3:System Programmer's Guide, Intel
  
--*/  

#include "Pei.h"
#include "DxeIpl.h"
#include "CpuIA32.h"
#include "VirtualMemory.h"
#include "DxeLoadFunc.h"

EFI_PHYSICAL_ADDRESS
CreateIdentityMappingPageTables (
  IN EFI_PEI_SERVICES      **PeiServices,
  IN UINT32                NumberOfProcessorPhysicalAddressBits
  )
/*++

Routine Description:

  Allocates and fills in the Page Directory and Page Table Entries to
  establish a 1:1 Virtual to Physical mapping.

Arguments:

  NumberOfProcessorPhysicalAddressBits - Number of processor address bits to use.
                                         Limits the number of page table entries 
                                         to the physical address space.

Returns:
  EFI_OUT_OF_RESOURCES  There are not enough resources to allocate the Page Tables

  EFI_SUCCESS           The 1:1 Virtual to Physical identity mapping was created

--*/
{  
  EFI_PHYSICAL_ADDRESS                          PageAddress;
  UINTN                                         NumberOfPml4EntriesNeeded;
  UINTN                                         NumberOfPdpEntriesNeeded;
  UINTN                                         IndexOfPml4Entries;
  UINTN                                         IndexOfPdpEntries;
  UINTN                                         IndexOfPageDirectoryEntries;
  PAGE_MAP_AND_DIRECTORY_POINTER                *PageMapLevel4Entry;
  PAGE_MAP_AND_DIRECTORY_POINTER                *PageMap;
  PAGE_MAP_AND_DIRECTORY_POINTER                *PageDirectoryPointerEntry;
  PAGE_TABLE_ENTRY                              *PageDirectoryEntry;

  //
  //  Page Table structure 4 level 4K, 3 level 2MB.
  //
  //                   PageMapLevel4Entry        : bits 47-39
  //                   PageDirectoryPointerEntry : bits 38-30
  //  Page Table 2MB : PageDirectoryEntry2M      : bits 29-21
  //  Page Table 4K  : PageDirectoryEntry4K      : bits 29 - 21
  //                   PageTableEntry            : bits 20 - 12
  //
  // Strategy is to map every thing in the processor address space using 
  //  2MB pages.
  //

  //
  // By architecture only one PageMapLevel4 exists - so lets allocate storgage for it.
  // 
  PageMap = PageMapLevel4Entry = (PAGE_MAP_AND_DIRECTORY_POINTER *)(UINTN)AllocateZeroedHobPages (PeiServices, 1);
  ASSERT_PEI_ERROR (PeiServices, PageMap != NULL);
  PageAddress = 0;

  //
  // The number of page-map Level-4 Offset entries is based on the number of 
  // physical address bits. Less than equal to 38 bits only takes one entry.
  // 512 entries represents 48 address bits. 
  //
  if (NumberOfProcessorPhysicalAddressBits <= 39) {
    NumberOfPml4EntriesNeeded = 1;
    NumberOfPdpEntriesNeeded  = 1 << (NumberOfProcessorPhysicalAddressBits - 30);
  } else {
    NumberOfPml4EntriesNeeded = 1 << (NumberOfProcessorPhysicalAddressBits - 39);
    NumberOfPdpEntriesNeeded  = 512;
  }

  for (IndexOfPml4Entries = 0; IndexOfPml4Entries < NumberOfPml4EntriesNeeded; IndexOfPml4Entries++, PageMapLevel4Entry++) {
    //
    // Each PML4 entry points to a page of Page Directory Pointer entires.
    //  So lets allocate space for them and fill them in in the IndexOfPdpEntries loop.
    //  
    PageDirectoryPointerEntry = (PAGE_MAP_AND_DIRECTORY_POINTER *)(UINTN)AllocateZeroedHobPages (PeiServices, 1);
    ASSERT_PEI_ERROR (PeiServices, PageDirectoryPointerEntry != NULL);

    //
    // Make a PML4 Entry
    //
    PageMapLevel4Entry->Uint64 = (UINT64)(UINTN)PageDirectoryPointerEntry;
    PageMapLevel4Entry->Bits.ReadWrite = 1;
    PageMapLevel4Entry->Bits.Present = 1;

    for (IndexOfPdpEntries = 0; IndexOfPdpEntries < 512; IndexOfPdpEntries++, PageDirectoryPointerEntry++) {
      //
      // Each Directory Pointer entries points to a page of Page Directory entires.
      //  So lets allocate space for them and fill them in in the IndexOfPageDirectoryEntries loop.
      //       
      PageDirectoryEntry = (PAGE_TABLE_ENTRY *)(UINTN)AllocateZeroedHobPages (PeiServices, 1);
      ASSERT_PEI_ERROR (PeiServices, PageDirectoryEntry != NULL);

      //
      // Fill in a Page Directory Pointer Entries
      //
      PageDirectoryPointerEntry->Uint64 = (UINT64)(UINTN)PageDirectoryEntry;
      PageDirectoryPointerEntry->Bits.ReadWrite = 1;
      PageDirectoryPointerEntry->Bits.Present = 1;

      for (IndexOfPageDirectoryEntries = 0; IndexOfPageDirectoryEntries < 512; IndexOfPageDirectoryEntries++, PageDirectoryEntry++, PageAddress += 0x200000) {
        //
        // Fill in the Page Directory entries
        //
        PageDirectoryEntry->Uint64 = (UINT64)PageAddress;
        PageDirectoryEntry->Bits.ReadWrite = 1;
        PageDirectoryEntry->Bits.Present = 1;
        PageDirectoryEntry->Bits.MustBe1 = 1;
      }
    }
  }

  //
  // For the PML4 entries we are not using fill in a null entry.
  //  for now we just copy the first entry.
  //
  for (; IndexOfPml4Entries < 512; IndexOfPml4Entries++, PageMapLevel4Entry++) {
    (*PeiServices)->CopyMem (
                  PageMapLevel4Entry,
                  PageMap,
                  sizeof (PAGE_MAP_AND_DIRECTORY_POINTER)
                  );
  }

  return (EFI_PHYSICAL_ADDRESS)PageMap;
}

