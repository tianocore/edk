/*++

Copyright (c) 2006, Intel Corporation                                                         
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

#include "Tiano.h"
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

BOOLEAN
GetNullPointerProtectionState (
  UINT8 *PageTable
  )
{
  X64_PAGE_TABLE_ENTRY_4K                       *PageTableEntry4KB;

  PageTableEntry4KB = (X64_PAGE_TABLE_ENTRY_4K *)((UINTN)PageTable + EFI_PAGE_NUMBER_2M * EFI_PAGE_SIZE_4K);
  return (PageTableEntry4KB->Bits.Present == 0) ? TRUE : FALSE;
}

VOID
EnableNullPointerProtection (
  UINT8 *PageTable
  )
{
  X64_PAGE_TABLE_ENTRY_4K                       *PageTableEntry4KB;

  PageTableEntry4KB = (X64_PAGE_TABLE_ENTRY_4K *)((UINTN)PageTable + EFI_PAGE_NUMBER_2M * EFI_PAGE_SIZE_4K);
  PageTableEntry4KB->Bits.Present = 0;

  return ;
}

VOID
DisableNullPointerProtection (
  UINT8 *PageTable
  )
{
  X64_PAGE_TABLE_ENTRY_4K                       *PageTableEntry4KB;

  PageTableEntry4KB = (X64_PAGE_TABLE_ENTRY_4K *)((UINTN)PageTable + EFI_PAGE_NUMBER_2M * EFI_PAGE_SIZE_4K);
  PageTableEntry4KB->Bits.Present = 1;

  return ;
}

