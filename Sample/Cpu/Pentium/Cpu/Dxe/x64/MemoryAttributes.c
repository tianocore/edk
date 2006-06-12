/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Cpu.c

Abstract:

  x64 version of interrupt table initialization and support
  for setting memory attributes by updating page table structures


--*/

#include "CpuDxe.h"
#include "CpuLib.h"
#include "VirtualMemory.h"

UINT8 *mPageStore     = NULL;
UINTN mPageStoreSize  = 16;
UINTN mPageStoreIndex = 0;

#define ALINE_16BYTE_BOUNDRY  __declspec(align(16)) 

#pragma pack (1)
typedef  struct {
  UINT16              LimitLow;
  UINT16              BaseLow;
  UINT8               BaseMiddle;
  UINT8               Attributes1;
  UINT8			      Attributes2;	
  UINT8               BaseHigh;
} SEGMENT_DESCRIPTOR_X64;

typedef struct {
  UINT16              Limit;
  UINTN               Base;
} PSEUDO_DESCRIPTOR_X64;

#pragma pack()


ALINE_16BYTE_BOUNDRY SEGMENT_DESCRIPTOR_X64 gGdt[] = {
  { // NULL Selector: selector[0]
    0,      // limit 15:0
    0,      // base  15:0
    0,      // base  23:16
    0,      // present, ring 0, data, expand-up writable
    0,      // type & limit 19:16
    0,      // base  31:24
  },
  { // Linear Selector: selector[8]
    0xffff, // limit 15:0
    0,      // base  15:0
    0,      // base  23:16
    0x92,   // present, ring 0, data, expand-up writable
    0xcf,   // type & limit 19:16 
    0,      // base  31:24
  },
  { // Linear code Selector: selector[10]
    0xffff, // limit 15:0
    0,      // base  15:0
    0,      // base  23:16
    0x9a,   // present, ring 0, code, expand-up writable
    0xaf,   // type & limit 19:16   
    0,      // base  31:24
  },
  { // Compatibility mode data Selector: selector[18]
    0xffff, // limit 15:0
    0,      // base  15:0
    0,      // base  23:16
    0x92,   // present, ring 0, code, expand-up writable
    0xcf,   // type & limit 19:16
    0,      // base  31:24
  },
  { // Compatibility code Selector: selector[20]
    0xffff, // limit 15:0
    0,      // base  15:0
    0,      // base  23:16
    0x9a,   // present, ring 0, code, expand-up writable
    0xcf,   // type & limit 19:16
    0,      // base  31:24
  },
  { // Spare3 Selector: selector[28]
    0,      // limit 15:0
    0,      // base  15:0
    0,      // base  23:16
    0,      // present, ring 0, code, expand-up writable
    0,      // type & limit 19:16
    0,      // base  31:24
  },
  { // 64-bit data Selector:selector[30]
    0xffff, // limit 15:0
    0,      // base  15:0
    0,      // base  23:16
    0x92,   // present, ring 0, code, expand-up writable
    0xcf,   // type & limit 19:16
    0,      // base  31:24
  },
  { // 64-bit code Selector: selector[38]
   0xffff,  // limit 15:0
   0,       // base  15:0
   0,       // base  23:16
   0x9a,    // present, ring 0, code, expand-up writable
   0xaf,    // type & limit 19:16
   0,       // base  31:24
  },
  { // Spare3 Selector: selector[40]
    0,      // limit 15:0
    0,      // base  15:0
    0,      // base  23:16
    0,      // present, ring 0, code, expand-up writable
    0,      // type & limit 19:16
    0,      // base  31:24
  }
};

ALINE_16BYTE_BOUNDRY PSEUDO_DESCRIPTOR_X64 gGdtPseudoDescriptor = {
  sizeof (gGdt) - 1,
  (UINTN)gGdt
};


INTERRUPT_GATE_DESCRIPTOR   gIdtTable[INTERRUPT_VECTOR_NUMBER] = { 0 };

ALINE_16BYTE_BOUNDRY PSEUDO_DESCRIPTOR_X64 gLidtPseudoDescriptor = {
  sizeof (gIdtTable) - 1,
  (UINTN)gIdtTable
};


VOID
InitializeSelectors (
  VOID
  )
{
  CpuLoadGlobalDescriptorTable (&gGdtPseudoDescriptor);
}

VOID
AsmIdtVector00 (
  VOID
  );

VOID
InitializeInterruptTables (
  VOID
  )
/*++

Routine Description:

  Slick around interrupt routines.

Arguments:

  None

Returns:

  None


--*/
{
  UINT16                         CodeSegment;
  INTERRUPT_GATE_DESCRIPTOR      *IdtEntry;
  UINT8                          *CurrentHandler;
  UINT32                         Index;
  
  CodeSegment = CpuCodeSegment ();
  
  IdtEntry = gIdtTable;
  CurrentHandler = (UINT8 *)(UINTN)AsmIdtVector00;
  for (Index = 0; Index < INTERRUPT_VECTOR_NUMBER; Index ++) {
    IdtEntry[Index].Offset15To0       = (UINT16)(UINTN)CurrentHandler;
    IdtEntry[Index].SegmentSelector   = CodeSegment;
    IdtEntry[Index].Attributes        = INTERRUPT_GATE_ATTRIBUTE;
    IdtEntry[Index].Offset31To16      = (UINT16)((UINTN)CurrentHandler >> 16);
    IdtEntry[Index].Offset63To32      = (UINT32)((UINTN)CurrentHandler >> 32);
    
    CurrentHandler += 0x8;
  }
      
  CpuLoadInterruptDescriptorTable (&gLidtPseudoDescriptor);

  return;
}


VOID
InitailizeMemoryAttributes (
  VOID
  )
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  Page;

  Status = gBS->AllocatePages (
                 AllocateAnyPages, 
                 EfiBootServicesData, 
                 mPageStoreSize,
                 &Page
                 );
  ASSERT_EFI_ERROR (Status);

  mPageStore = (UINT8 *)(UINTN)Page;

  EfiZeroMem (mPageStore, 0x1000 * mPageStoreSize);
}



VOID  *
AllocateZeroedPage (
  VOID
  )
{
  if (mPageStoreIndex >= mPageStoreSize) {
    //
    // We are out of space
    // 
    return NULL;
  }

  return (VOID *)(UINTN)&mPageStore[0x1000 * mPageStoreIndex++];
}


VOID
Convert2MBPageTo4KPages (  
  IN      EFI_PHYSICAL_ADDRESS        PageAddress,
  IN OUT  X64_PAGE_TABLE_ENTRY        **PageDirectoryToConvert
  )
{
  UINTN                               Index;
  EFI_PHYSICAL_ADDRESS                WorkingAddress;
  X64_PAGE_TABLE_ENTRY_4K             *PageTableEntry;
  X64_PAGE_TABLE_ENTRY                Attributes;

  
  //
  // Save the attributes of the 2MB table
  //
  Attributes.Page2Mb.Uint64 = (*PageDirectoryToConvert)->Page2Mb.Uint64;
  
  //
  // Convert PageDirectoryEntry2MB into a 4K Page Directory
  //
  PageTableEntry = AllocateZeroedPage ();
  (*PageDirectoryToConvert)->Page2Mb.Uint64 = (UINT64)PageTableEntry;
  (*PageDirectoryToConvert)->Page2Mb.Bits.ReadWrite = 1;
  (*PageDirectoryToConvert)->Page2Mb.Bits.Present = 1;
  
  WorkingAddress = PageAddress;
  for (Index = 0; Index < 512; Index++, PageTableEntry++, WorkingAddress += 0x1000) {
    PageTableEntry->Uint64 = (UINT64)WorkingAddress;
    PageTableEntry->Bits.Present = 1;

    //
    // Update the new page to have the same attributes as the 2MB page
    //
    PageTableEntry->Bits.ReadWrite = Attributes.Common.ReadWrite;
    PageTableEntry->Bits.CacheDisabled = Attributes.Common.CacheDisabled;
    PageTableEntry->Bits.WriteThrough  = Attributes.Common.WriteThrough;

    if (WorkingAddress == PageAddress) {
      //
      // Return back the 4K page that matches the Working addresss
      //
      *PageDirectoryToConvert = (X64_PAGE_TABLE_ENTRY *)PageTableEntry;
    }
  }
}


EFI_STATUS
GetCurrentMapping (
  IN  EFI_PHYSICAL_ADDRESS    BaseAddress,
  OUT X64_PAGE_TABLE_ENTRY    **PageTable,
  OUT BOOLEAN                 *Page2MBytes
  )
{
  UINT64                                        Cr3;
  X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K     *PageMapLevel4Entry;
  X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K     *PageDirectoryPointerEntry;
  X64_PAGE_TABLE_ENTRY_2M                       *PageTableEntry2Mb;
  X64_PAGE_DIRECTORY_ENTRY_4K                   *PageDirectoryEntry4k;
  X64_PAGE_TABLE_ENTRY_4K                       *PageTableEntry4k;
  UINTN                                         Pml4Index;
  UINTN                                         PdpIndex;
  UINTN                                         Pde2MbIndex;
  UINTN                                         PteIndex;

  Cr3 = CpuReadCr3 ();
  
  PageMapLevel4Entry = (X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K *)(Cr3 & 0x000ffffffffff000);

  Pml4Index = (UINTN)RShiftU64 (BaseAddress, 39) & 0x1ff;
  if (PageMapLevel4Entry[Pml4Index].Bits.Present == 0) {
    return EFI_NOT_FOUND;
  }
  PageDirectoryPointerEntry = (X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K *)(PageMapLevel4Entry[Pml4Index].Uint64 & 0x000ffffffffff000);
  PdpIndex = (UINTN)RShiftU64 (BaseAddress, 30) & 0x1ff;
  if (PageDirectoryPointerEntry[PdpIndex].Bits.Present == 0) {
    return EFI_NOT_FOUND;
  }
  
  PageTableEntry2Mb = (X64_PAGE_TABLE_ENTRY_2M *)(PageDirectoryPointerEntry[PdpIndex].Uint64 & 0x000ffffffffff000);
  Pde2MbIndex = (UINTN)RShiftU64 (BaseAddress, 21) & 0x1ff;
  if (PageTableEntry2Mb[Pde2MbIndex].Bits.Present == 0) {
    return EFI_NOT_FOUND;
  }

  if (PageTableEntry2Mb[Pde2MbIndex].Bits.MustBe1 == 1) {
    //
    // We found a 2MByte page so lets return it
    //
    *Page2MBytes = TRUE;
    *PageTable = (X64_PAGE_TABLE_ENTRY *)&PageTableEntry2Mb[Pde2MbIndex].Uint64;
    return EFI_SUCCESS;
  }

  //
  // 4K page so keep walking 
  //
  PageDirectoryEntry4k = (X64_PAGE_DIRECTORY_ENTRY_4K *)&PageTableEntry2Mb[Pde2MbIndex].Uint64;

  PageTableEntry4k = (X64_PAGE_TABLE_ENTRY_4K *)(PageDirectoryEntry4k[Pde2MbIndex].Uint64 & 0x000ffffffffff000);
  PteIndex = (UINTN)RShiftU64 (BaseAddress, 12) & 0x1ff;
  if (PageTableEntry4k[PteIndex].Bits.Present == 0) {
    return EFI_NOT_FOUND;
  }

  *Page2MBytes = FALSE;
  *PageTable = (X64_PAGE_TABLE_ENTRY *)&PageTableEntry4k[PteIndex];
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
CpuSetMemoryAttributes (
  IN  EFI_CPU_ARCH_PROTOCOL   *This,
  IN  EFI_PHYSICAL_ADDRESS    BaseAddress,
  IN  UINT64                  Length,
  IN  UINT64                  Attributes
  )
{
  EFI_STATUS                  Status;
  BOOLEAN                     Page2MByte;
  X64_PAGE_TABLE_ENTRY        *PageTableEntry;
  EFI_PHYSICAL_ADDRESS        WorkingAddress;
  UINT64                      WorkingLength;
  UINTN                       Stride;

  if (Length == 0) {
    //
    // Check for invalid parameter
    //
    return EFI_INVALID_PARAMETER;
  }
  
  if ((BaseAddress & 0xffff) != 0 || (Length & 0xffff) != 0 ) {
    //
    // Minimum page table granularity is 4K so BaseAddress and Length must
    // be aligned on 4K boundries 
    //
    return EFI_UNSUPPORTED;
  }

  WorkingLength = Length;
  for (WorkingAddress = BaseAddress; WorkingLength >= 0; WorkingLength -= Stride) {
    Status = GetCurrentMapping (WorkingAddress, &PageTableEntry, &Page2MByte);
    if (EFI_ERROR (Status)) {
      //
      // We make the assumption that all of memory has a 2MB mapping so we 
      // should not get here for a valid request.
      //
      return EFI_UNSUPPORTED;
    }

    if (Page2MByte) {
      //
      // Address is in a 2MB page
      //
      if ((WorkingLength < 0x10000) || ((WorkingAddress & 0x1fffff) != 0)) {
        //
        // Range less than 2 MB or not 2 MB aligned convert the entire 2MB
        // to 4K pages and make PageTableEntry point to the right 4K page
        //
        Convert2MBPageTo4KPages (WorkingAddress, &PageTableEntry);
        Page2MByte = FALSE;
      }
    }

    switch(Attributes) {
      case EFI_MEMORY_UC:
        PageTableEntry->Common.ReadWrite     = 1;
        PageTableEntry->Common.WriteThrough  = 0;
        PageTableEntry->Common.CacheDisabled = 1;
        break;
      case EFI_MEMORY_WC:
        PageTableEntry->Common.ReadWrite     = 1;
        PageTableEntry->Common.WriteThrough  = 1;
        PageTableEntry->Common.CacheDisabled = 1;
        break;
      case EFI_MEMORY_WT:     
        PageTableEntry->Common.ReadWrite     = 1;
        PageTableEntry->Common.WriteThrough  = 1;
        PageTableEntry->Common.CacheDisabled = 0;
        break;
      case EFI_MEMORY_WP:   
        PageTableEntry->Common.ReadWrite     = 0;
        PageTableEntry->Common.WriteThrough  = 0;
        PageTableEntry->Common.CacheDisabled = 0;
        break;
      case EFI_MEMORY_WB:        
        PageTableEntry->Common.ReadWrite     = 1;
        PageTableEntry->Common.WriteThrough  = 0;
        PageTableEntry->Common.CacheDisabled = 0;
        break;

      default:
        return EFI_UNSUPPORTED;
    }
    
    //
    // Flush TBL since page table entries have been updated.
    //
    CpuWriteCr3 (CpuReadCr3 ());

    Stride = (Page2MByte) ? 0x200000 : 0x1000;
  }

  //
  // In case we leave long mode we need to update the MTRR registers
  //
  return EFI_SUCCESS;
}

