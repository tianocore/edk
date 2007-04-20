/*++

Copyright (c) 2006 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Paging.c

Abstract:

Revision History:

--*/

#include "HobGeneration.h"
#include "VirtualMemory.h"

#define EFI_PAGE_SIZE_4K      0x1000
#define EFI_PAGE_SIZE_2M      0x200000

//
// Create 64G 2M-page table
// PML4 (47:39)  :    1 entry
// PDPTE (38:30) :   64 entry
// PDE (29:21)   :  512 entries
//
#define EFI_MAX_ENTRY_NUM     512

#define EFI_PML4_ENTRY_NUM    1
#define EFI_PDPTE_ENTRY_NUM   64
#define EFI_PDE_ENTRY_NUM     EFI_MAX_ENTRY_NUM

#define EFI_PML4_PAGE_NUM     1
#define EFI_PDPTE_PAGE_NUM    EFI_PML4_ENTRY_NUM
#define EFI_PDE_PAGE_NUM      (EFI_PML4_ENTRY_NUM * EFI_PDPTE_ENTRY_NUM)

#define EFI_PAGE_NUMBER_DIR   (EFI_PML4_PAGE_NUM + EFI_PDPTE_PAGE_NUM)
#define EFI_PAGE_NUMBER_2M    (EFI_PAGE_NUMBER_DIR + EFI_PDE_PAGE_NUM)

//
// Create 2M 4K-page table
// PTE (20:12)   :  512 entries
//
#define EFI_PTE_ENTRY_NUM     EFI_MAX_ENTRY_NUM
#define EFI_PTE_PAGE_NUM      1

#define EFI_PAGE_NUMBER_4K    (EFI_PTE_PAGE_NUM)

#define EFI_PAGE_NUMBER       (EFI_PAGE_NUMBER_2M + EFI_PAGE_NUMBER_4K)

VOID
EnableNullPointerProtection (
  UINT8 *PageTable
  )
{
  X64_PAGE_TABLE_ENTRY_4K                       *PageTableEntry4KB;

  PageTableEntry4KB = (X64_PAGE_TABLE_ENTRY_4K *)((UINTN)PageTable + EFI_PAGE_NUMBER_2M * EFI_PAGE_SIZE_4K);

  //
  // Fill in the Page Table entries
  // Mark 0~4K as not present
  //
  PageTableEntry4KB->Bits.Present = 0;

  return ;
}

VOID
X64Create4KPageTables (
  UINT8 *PageTable
  )
{
  UINT64                                        PageAddress;
  UINTN                                         PTEIndex;
  X64_PAGE_DIRECTORY_ENTRY_4K                   *PageDirectoryEntry4KB;
  X64_PAGE_TABLE_ENTRY_4K                       *PageTableEntry4KB;

  PageAddress = 0;

  //
  //  Page Table structure 4 level 4K.
  //
  //                   PageMapLevel4Entry        : bits 47-39
  //                   PageDirectoryPointerEntry : bits 38-30
  //  Page Table 4K  : PageDirectoryEntry4K      : bits 29-21
  //                   PageTableEntry            : bits 20-12
  //

  PageTableEntry4KB = (X64_PAGE_TABLE_ENTRY_4K *)((UINTN)PageTable + EFI_PAGE_NUMBER_2M * EFI_PAGE_SIZE_4K);
  PageDirectoryEntry4KB = (X64_PAGE_DIRECTORY_ENTRY_4K *)((UINTN)PageTable + EFI_PAGE_NUMBER_DIR * EFI_PAGE_SIZE_4K);

  PageDirectoryEntry4KB->Uint64 = (UINT64)(UINTN)PageTableEntry4KB;
  PageDirectoryEntry4KB->Bits.ReadWrite = 1;
  PageDirectoryEntry4KB->Bits.Present = 1;
  PageDirectoryEntry4KB->Bits.MustBeZero = 0;

  for (PTEIndex = 0; PTEIndex < EFI_PTE_ENTRY_NUM; PTEIndex++, PageTableEntry4KB++) {
    //
    // Fill in the Page Table entries
    //
    PageTableEntry4KB->Uint64 = (UINT64)PageAddress;
    PageTableEntry4KB->Bits.ReadWrite = 1;
    PageTableEntry4KB->Bits.Present = 1;

    PageAddress += EFI_PAGE_SIZE_4K;
  }

  return ;
}

VOID
X64Create2MPageTables (
  UINT8 *PageTable
  )
{
  UINT64                                        PageAddress;
  UINT8                                         *TempPageTable;
  UINTN                                         PML4Index;
  UINTN                                         PDPTEIndex;
  UINTN                                         PDEIndex;
  X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K     *PageMapLevel4Entry;
  X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K     *PageDirectoryPointerEntry;
  X64_PAGE_TABLE_ENTRY_2M                       *PageDirectoryEntry2MB;

  TempPageTable = PageTable;

  PageAddress = 0;

  //
  //  Page Table structure 3 level 2MB.
  //
  //                   PageMapLevel4Entry        : bits 47-39
  //                   PageDirectoryPointerEntry : bits 38-30
  //  Page Table 2MB : PageDirectoryEntry2M      : bits 29-21
  //

  PageMapLevel4Entry = (X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K *)TempPageTable;

  for (PML4Index = 0; PML4Index < EFI_PML4_ENTRY_NUM; PML4Index++, PageMapLevel4Entry++) {
    //
    // Each PML4 entry points to a page of Page Directory Pointer entires.
    //  
    TempPageTable += EFI_PAGE_SIZE_4K;
    PageDirectoryPointerEntry = (X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K *)TempPageTable;

    //
    // Make a PML4 Entry
    //
    PageMapLevel4Entry->Uint64 = (UINT64)(UINTN)(TempPageTable);
    PageMapLevel4Entry->Bits.ReadWrite = 1;
    PageMapLevel4Entry->Bits.Present = 1;

    for (PDPTEIndex = 0; PDPTEIndex < EFI_PDPTE_ENTRY_NUM; PDPTEIndex++, PageDirectoryPointerEntry++) {
      //
      // Each Directory Pointer entries points to a page of Page Directory entires.
      //       
      TempPageTable += EFI_PAGE_SIZE_4K;
      PageDirectoryEntry2MB = (X64_PAGE_TABLE_ENTRY_2M *)TempPageTable;

      //
      // Fill in a Page Directory Pointer Entries
      //
      PageDirectoryPointerEntry->Uint64 = (UINT64)(UINTN)(TempPageTable);
      PageDirectoryPointerEntry->Bits.ReadWrite = 1;
      PageDirectoryPointerEntry->Bits.Present = 1;

      for (PDEIndex = 0; PDEIndex < EFI_PDE_ENTRY_NUM; PDEIndex++, PageDirectoryEntry2MB++) {
        //
        // Fill in the Page Directory entries
        //
        PageDirectoryEntry2MB->Uint64 = (UINT64)PageAddress;
        PageDirectoryEntry2MB->Bits.ReadWrite = 1;
        PageDirectoryEntry2MB->Bits.Present = 1;
        PageDirectoryEntry2MB->Bits.MustBe1 = 1;

        PageAddress += EFI_PAGE_SIZE_2M;
      }
    }
  }

  return ;
}

VOID *
PreparePageTable (
  VOID *PageNumberTop
  )
/*++
Description:
  Generate pagetable below PageNumberTop, 
  and return the bottom address of pagetable for putting other things later.
--*/
{
  VOID *PageNumberBase;

  PageNumberBase = (VOID *)((UINTN)PageNumberTop - EFI_PAGE_NUMBER * EFI_PAGE_SIZE_4K);
  EfiCommonLibZeroMem (PageNumberBase, EFI_PAGE_NUMBER * EFI_PAGE_SIZE_4K);

  X64Create2MPageTables (PageNumberBase);
  X64Create4KPageTables (PageNumberBase);
  //
  // Not enable NULL Pointer Protection if using INTX call
  //
//  EnableNullPointerProtection (PageNumberBase);

  return PageNumberBase;
}
