/*++ 

Copyright 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  GenPage.c
  
Abstract:

--*/

#include <stdio.h>
#include <stdlib.h>
#include "VirtualMemory.h"

void
memset (void *, char, long);

#define EFI_PAGE_BASE_OFFSET_IN_LDR 0x70000
#define EFI_PAGE_BASE_ADDRESS       (EFI_PAGE_BASE_OFFSET_IN_LDR + 0x20000)

  //
  // Create 4G page table
  // 4K page table memory:
  // PML4 (47:39)  :    1 entry
  // PDPTE (38:30) :    4 entry
  // PDE (29:21)   :  512 entries
  // PTE (20:12)   :  512 entries (4K)
  //
#define EFI_MAX_ENTRY_NUM     512

#define EFI_PML4_ENTRY_NUM    1
#define EFI_PDPTE_ENTRY_NUM   4
#define EFI_PDE_ENTRY_NUM     EFI_MAX_ENTRY_NUM
#define EFI_PTE_ENTRY_NUM     EFI_MAX_ENTRY_NUM

#define EFI_PML4_PAGE_NUM     1
#define EFI_PDPTE_PAGE_NUM    EFI_PML4_ENTRY_NUM
#define EFI_PDE_PAGE_NUM      (EFI_PML4_ENTRY_NUM * EFI_PDPTE_ENTRY_NUM)
#define EFI_PTE_PAGE_NUM      (EFI_PML4_ENTRY_NUM * EFI_PDPTE_ENTRY_NUM * EFI_PDE_ENTRY_NUM)

#define EFI_PAGE_NUMBER       (EFI_PML4_PAGE_NUM + EFI_PDPTE_PAGE_NUM + EFI_PDE_PAGE_NUM)

#define EFI_PAGE_SIZE_4K      0x1000
#define EFI_PAGE_SIZE_2M      0x200000

#define CONVERT_BIN_PAGE_ADDRESS(a)  (a - PageTable + EFI_PAGE_BASE_ADDRESS)

/*
 * this function is used to create 4G PAE 2M pagetable
 */
void *
CreateIdentityMappingPageTables (
  void
  )
{
  UINT64                                        PageAddress;
  UINT8                                         *PageTable;
  UINT8                                         *TempPageTable;
  int                                           PML4Index;
  int                                           PDPTEIndex;
  int                                           PDEIndex;
  X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K     *PageMapLevel4Entry;
  X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K     *PageDirectoryPointerEntry;
  X64_PAGE_TABLE_ENTRY_2M                       *PageDirectoryEntry2MB;

  PageTable = (void *)malloc (EFI_PAGE_NUMBER * EFI_PAGE_SIZE_4K);
  memset (PageTable, 0, (EFI_PAGE_NUMBER * EFI_PAGE_SIZE_4K));
  TempPageTable = PageTable;

  PageAddress = 0;

  //
  //  Page Table structure 4 level 4K, 3 level 2MB.
  //
  //                   PageMapLevel4Entry        : bits 47-39
  //                   PageDirectoryPointerEntry : bits 38-30
  //  Page Table 2MB : PageDirectoryEntry2M      : bits 29-21
  //  Page Table 4K  : PageDirectoryEntry4K      : bits 29-21
  //                   PageTableEntry            : bits 20-12
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
    PageMapLevel4Entry->Uint64 = (UINT64)(UINT32)(CONVERT_BIN_PAGE_ADDRESS (TempPageTable));
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
      PageDirectoryPointerEntry->Uint64 = (UINT64)(UINT32)(CONVERT_BIN_PAGE_ADDRESS (TempPageTable));
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

  return PageTable;
}

int
GenBinPage (
  void *BaseMemory,
  char *NoPageFileName,
  char *PageFileName
  )
{
  FILE  *PageFile;
  FILE  *NoPageFile;
  UINT8 Data;
  unsigned long FileSize;

  //
  // Open files
  //
  PageFile = fopen (PageFileName, "w+b");
  if (PageFile == NULL) {
    printf ("GenBinPage: Could not open file %s\n", PageFileName);
    return -1;
  }

  NoPageFile = fopen (NoPageFileName, "r+b");
  if (NoPageFile == NULL) {
    printf ("GenBinPage: Could not open file %s\n", NoPageFileName);
    fclose (PageFile);
    return -1;
  }

  //
  // Check size - should not be great than EFI_PAGE_BASE_OFFSET_IN_LDR
  //
  fseek (NoPageFile, 0, SEEK_END);
  FileSize = ftell (NoPageFile);
  fseek (NoPageFile, 0, SEEK_SET);
  if (FileSize > EFI_PAGE_BASE_OFFSET_IN_LDR) {
    printf ("GenBinPage: file size too large - 0x%x\n", FileSize);
    fclose (PageFile);
    fclose (NoPageFile);
    return -1;
  }

  //
  // Write data
  //
  while (fread (&Data, sizeof(UINT8), 1, NoPageFile)) {
    fwrite (&Data, sizeof(UINT8), 1, PageFile);
  }

  //
  // Write PageTable
  //
  fseek (PageFile, EFI_PAGE_BASE_OFFSET_IN_LDR, SEEK_SET);
  fwrite (BaseMemory, (EFI_PAGE_NUMBER * EFI_PAGE_SIZE_4K), 1, PageFile);

  //
  // Close files
  //
  fclose (PageFile);
  fclose (NoPageFile);

  return 0;
}

int
main (
  int argc,
  char **argv
  )
{
  void *BaseMemory;
  int  result;

  if (argc != 3) {
    printf ("arg error: %d\n", argc);
    return ;
  }

  BaseMemory = CreateIdentityMappingPageTables ();

  result = GenBinPage (BaseMemory, argv[1], argv[2]);
  if (result < 0) {
    exit (-1);
  }

  return 0;
}

