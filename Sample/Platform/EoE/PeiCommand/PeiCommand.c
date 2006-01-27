/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  PeiCommand.c

Abstract:
  This is a 32-bit Driver that is used to boot strap an x64 (64-bit)
  DxeCore. This application replaces the DxeIpl PEIM. The idea is to be able
  to bootstrap an x64 DxeCore from 32-bit DxeCore (shell prompt).

  This driver is designed to be loaded from disk. This driver will load an
  FV from the same location directoy on the disk that this driver was loaded 
  from. The name of the FV file is x64.FV. x64.FV contains a single PEIM
  that produces the HOB based servics required to make the DxeCore function.
  The rest of the x64.FV is a copy of DXE and the shell in x64 mode. This
  allow a 32-bit IA-32 system to soft load into a x64 environment.

  The Go64 driver allocates some memory and makes a fake set of HOBs to emulate
  what would happen in PEI.

Revision History:

--*/

#include "PeiCommand.h"

VOID
GetMemoryBellow1MB (
  IN OUT    EFI_PHYSICAL_ADDRESS  *Address,
  IN OUT    UINT64                *ResourceLength
  );

EFI_HANDLE    gMyImageHandle;

HOB_TEMPLATE  gHobTemplate = {
  { // Phit
    { // Header
      EFI_HOB_TYPE_HANDOFF,                 // HobType
      sizeof (EFI_HOB_HANDOFF_INFO_TABLE),  // HobLength
      0                                     // Reserved
    },
    EFI_HOB_HANDOFF_TABLE_VERSION,        // Version
    BOOT_WITH_MINIMAL_CONFIGURATION,      // BootMode
    0,                                    // EfiMemoryTop
    0,                                    // EfiMemoryBottom
    0,                                    // EfiFreeMemoryTop
    0,                                    // EfiFreeMemoryBottom
    0                                     // EfiEndOfHobList
  }, 
  { // Bfv
    {
      EFI_HOB_TYPE_FV, // HobType
      sizeof (EFI_HOB_FIRMWARE_VOLUME), // HobLength
      0                                 // Reserved
    },
    0,                                  // BaseAddress
    0                                   // Length
  },
  { // BfvResource
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,                      // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR), // HobLength
      0                                     // Reserved
    },
    {
      0                                    // Owner Guid
    },
    EFI_RESOURCE_FIRMWARE_DEVICE,          // ResourceType
    (EFI_RESOURCE_ATTRIBUTE_PRESENT    |
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
     EFI_RESOURCE_ATTRIBUTE_TESTED |
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),  // ResourceAttribute
    0,                                              // PhysicalStart
    0                                               // ResourceLength
  },
  { // Cpu
    { // Header
      EFI_HOB_TYPE_CPU,     // HobType
      sizeof (EFI_HOB_CPU), // HobLength
      0                     // Reserved
    },
    52,                     // SizeOfMemorySpace - Architecture Max
    16,                     // SizeOfIoSpace,
    {
      0, 0, 0, 0, 0, 0      // Reserved[6]
    }
  },
  { // MemoryResourceConsumed for HOB's & early stack
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,     // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR), // HobLength
      0                                     // Reserved
    },
    {
      0                                    // Owner Guid
    },
    EFI_RESOURCE_SYSTEM_MEMORY,            // ResourceType
    (EFI_RESOURCE_ATTRIBUTE_PRESENT                 |
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED             |
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | 
     EFI_RESOURCE_ATTRIBUTE_TESTED                  |
     EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),     
    0x0,                                            // PhysicalStart
    0                                               // ResourceLength
  },
  { // MemoryResourceFree for unused memory that DXE core will claim
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,     // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR), // HobLength
      0                                     // Reserved
    },
    {
      0                                    // Owner Guid
    },
    EFI_RESOURCE_SYSTEM_MEMORY,            // ResourceType
    (EFI_RESOURCE_ATTRIBUTE_PRESENT                 |
     EFI_RESOURCE_ATTRIBUTE_TESTED                  |
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED             |
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),     
    0x0,                                            // PhysicalStart
    0                                               // ResourceLength
  },
  { // MemoryUnder1MB for unused memory
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,     // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR), // HobLength
      0                                     // Reserved
    },
    {
      0                                    // Owner Guid
    },
    EFI_RESOURCE_SYSTEM_MEMORY,            // ResourceType
    (EFI_RESOURCE_ATTRIBUTE_PRESENT                 |
     EFI_RESOURCE_ATTRIBUTE_TESTED                  |
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED             |
     EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | 
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),     
    0x0,                                            // PhysicalStart
    0                                               // ResourceLength
  },
  {   // Memory Allocation for DXE Core
    {   // header
      EFI_HOB_TYPE_MEMORY_ALLOCATION,                 // Hob type
      sizeof (EFI_HOB_MEMORY_ALLOCATION_MODULE),      // Hob size
      0                                               // reserved
    },
    {
      {
        0, //EFI_HOB_MEMORY_ALLOC_MODULE_GUID              // Guid
      },
      0x0,                                            // EFI_PHYSICAL_ADDRESS  MemoryBaseAddress;
      0x0,                                            // UINT64                MemoryLength;
      EfiBootServicesCode,                            // EFI_MEMORY_TYPE       MemoryType;  
      {
        0, 0, 0, 0                                    // UINT8                 Reserved[4]; 
      },
    },
    {
      0, //EFI_HOB_MEMORY_ALLOC_MODULE_GUID            //  BugBug - Should be Fv File name at some point
    },
    0x0                                           //  EFI_PHYSICAL_ADDRESS of EntryPoint;
  },
  { // EndOfHobList
    EFI_HOB_TYPE_END_OF_HOB_LIST,     // HobType
    sizeof (EFI_HOB_GENERIC_HEADER),  // HobLength
    0                                 // Reserved
  }
};

HOB_TEMPLATE *gHob;


//
// FILE_GUID from PpisNeededByDxeCore.inf
//
EFI_GUID mPpisNeededByDxeCoreGuid = {
  0x4d37da42, 0x3a0c, 0x4eda, { 0xb9, 0xeb, 0xbc, 0xe, 0x1d, 0xb4, 0x71, 0x3b }
};




EFI_DRIVER_ENTRY_POINT (PeiCommandInit)

EFI_STATUS
EFIAPI
PeiCommandInit (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
{
  EFI_STATUS                  Status;
  EFI_PHYSICAL_ADDRESS        Base;
  UINTN                       BaseSizeInBytes;
  VOID                        *Bfv;
  UINTN                       BfvLength;
  EFI_PHYSICAL_ADDRESS        Stack;
  HOB_ONLY_ENTRY              DxeCoreEntryPoint;
  HOB_ONLY_ENTRY              PpisNeededByDxeIplEntryPoint;
  VOID                        *Image;
  UINT32                      ImageSize;
  VOID                        *Image2;
  UINT32                      ImageSize2;

  DxeInitializeDriverLib (ImageHandle, SystemTable);

  gMyImageHandle = ImageHandle;

  //
  // Go find x64.FV, the FV with all the x64 code on the disk
  //
  Bfv = GetFileFromWhereWeLoaded (L"FvMain32.FV", &BfvLength);
  if (Bfv == NULL) {
    //
    // The Bfv File should be located in the same directory as this driver.
    // If we can't find it exit now
    //
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  Status = FfsFindPe32Section (Bfv, EFI_FV_FILETYPE_DRIVER, &mPpisNeededByDxeCoreGuid, &Image, &ImageSize);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  Status = FfsFindPe32Section (Bfv, EFI_FV_FILETYPE_DXE_CORE, NULL, &Image2, &ImageSize2);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  //
  //
  //
  BaseSizeInBytes = GetMemoryFromMap (&Base);
  if ((BaseSizeInBytes == 0) || (BaseSizeInBytes < CONSUMED_MEMORY)) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  //
  // Define Base to be the allocation
  //
  gHobTemplate.Phit.EfiMemoryTop = Base + CONSUMED_MEMORY - 1;
  gHobTemplate.Phit.EfiMemoryBottom = Base;
  gHobTemplate.Phit.EfiFreeMemoryTop = gHobTemplate.Phit.EfiMemoryTop;
  gHobTemplate.Phit.EfiFreeMemoryBottom = Base + sizeof (HOB_TEMPLATE);
  gHobTemplate.Phit.EfiEndOfHobList = gHobTemplate.Phit.EfiFreeMemoryBottom - sizeof (EFI_HOB_GENERIC_HEADER);

  //
  // BFV is allocated from the HOB heap
  //
  gHobTemplate.Bfv.BaseAddress = (EFI_PHYSICAL_ADDRESS)AllocateZeroedHobPages (EFI_SIZE_TO_PAGES (BfvLength));
  gHobTemplate.Bfv.Length = BfvLength;

  gHobTemplate.BfvResource.PhysicalStart = gHobTemplate.Bfv.BaseAddress;
  gHobTemplate.BfvResource.ResourceLength = gHobTemplate.Bfv.Length;

  //
  // All memory mapped by the resource descriptor
  //
  gHobTemplate.MemoryResourceConsumed.PhysicalStart  = Base;
  gHobTemplate.MemoryResourceConsumed.ResourceLength = CONSUMED_MEMORY;

  gHobTemplate.MemoryResourceFree.PhysicalStart  = Base + CONSUMED_MEMORY;
  gHobTemplate.MemoryResourceFree.ResourceLength = BaseSizeInBytes - CONSUMED_MEMORY;

  GetMemoryBellow1MB (
    &gHobTemplate.MemoryUnder1MB.PhysicalStart,
    &gHobTemplate.MemoryUnder1MB.ResourceLength
    );


  //
  // Copy HOBs to memory. Use gHob to remember where it was so we can allocate memory
  // latter for page tables.
  //
  gHob = (HOB_TEMPLATE *)(UINTN)(Base);
  EfiCopyMem (gHob, &gHobTemplate, sizeof (HOB_TEMPLATE));

  //
  // Copy Boot Firmware Volume to memory
  //
  EfiCopyMem ((VOID *)(UINTN)(gHobTemplate.Bfv.BaseAddress), Bfv, BfvLength);

  //
  // Allocate 128K of stack
  //
  Stack = (EFI_PHYSICAL_ADDRESS)AllocateZeroedHobPages (32);
  Stack = Stack + 32 * 0x1000 - 16;

  //
  // Load the images we found earlier in the FV
  //
  PpisNeededByDxeIplEntryPoint = LoadPeImage (Image, ImageSize);
  DxeCoreEntryPoint = LoadPeImage (Image2, ImageSize2);

  //
  // Fill in module alloc hob
  //
  gHob->DxeCore.MemoryAllocationHeader.Name = gEfiHobMemeryAllocModuleGuid;
  gHob->DxeCore.MemoryAllocationHeader.MemoryBaseAddress = (EFI_PHYSICAL_ADDRESS) Image2;
  gHob->DxeCore.MemoryAllocationHeader.MemoryLength = ImageSize2;
  gHob->DxeCore.EntryPoint = (EFI_PHYSICAL_ADDRESS)(UINTN)DxeCoreEntryPoint;
  gHob->DxeCore.ModuleName = gEfiHobMemeryAllocModuleGuid;

  Status = PpisNeededByDxeIplEntryPoint (gHob);
  ASSERT_EFI_ERROR (Status);

  DxeCoreEntryPoint (gHob);

  //
  // We should never return
  //
  ASSERT (FALSE);

  return EFI_DEVICE_ERROR;
}





EFI_PHYSICAL_ADDRESS
AllocateZeroedHobPages (
  IN  UINTN   NumberOfPages
  )
{
  EFI_PHYSICAL_ADDRESS    Page;
  UINTN                   NumberOfBytes;
  UINT64                  AlignmentOffset;

  //
  // EFI pages are 4K by convention. EFI pages are independent of processor page size
  //
  NumberOfBytes = 0x1000 * NumberOfPages;

  //
  // Allocate the EFI pages out of Hob Free Memory Heap.
  // Heap grows down from top of Free Memory. HOB grows up.
  //
  Page = gHob->Phit.EfiFreeMemoryTop - NumberOfBytes + 1;

  //
  // Make sure page is 4K aligned.
  //
  AlignmentOffset = Page & EFI_PAGE_MASK;
  NumberOfBytes += (UINTN)AlignmentOffset;
  Page -= AlignmentOffset;

  if (Page < gHob->Phit.EfiFreeMemoryBottom) {
    DEBUG ((EFI_D_ERROR, "Pages Requested %d\n", NumberOfPages));
    ASSERT (FALSE);
    return 0;
  }

  EfiCommonLibZeroMem ((VOID *)(UINTN)Page, NumberOfBytes);

  gHob->Phit.EfiFreeMemoryTop -= NumberOfBytes;

  return Page;
}



UINTN
GetMemoryFromMap (
  OUT EFI_PHYSICAL_ADDRESS  *Base
  )
{
  EFI_STATUS              Status;
  
  //
  // Grab contiguous memory from 1MB to the first gap.
  //  That's the memory map we will pass into the x64 code
  //

  //
  // Allocate 80 MBytes of contiguous memory
  //
  Status = gBS->AllocatePages (AllocateAnyPages, EfiLoaderData, 0x5000, Base);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return 0;
  }


  //
  // Convert pages to bytes. 
  //
  return 0x5000 * 4 * 1024;
}


VOID
GetMemoryBellow1MB (
  IN OUT    EFI_PHYSICAL_ADDRESS  *Address,
  IN OUT    UINT64                *ResourceLength
  )
{
  EFI_STATUS  Status;

  //
  // Allocate pages < 1 MB - 128 K bytes (32 * 4K) worth
  //
  *Address = 0x100000;
  Status = gBS->AllocatePages (AllocateMaxAddress, EfiLoaderData, 0x20, Address);
  ASSERT_EFI_ERROR (Status);

  *ResourceLength = 0x20 * 4 * 1024;
}


HOB_ONLY_ENTRY
LoadPeImage (
  IN  VOID  *PeImage,
  IN  UINTN PeImageSize
  )
{
  EFI_STATUS                                Status;
  EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT      ImageContext;

  //
  // Initialize ImageContext for PeImage
  //
  PeCoffLoaderInitializeImageContext (&ImageContext, PeImage);

  //
  // Get the size of the Image into the ImageContext
  //
  PeCoffLoaderGetImageInfo (NULL, &ImageContext);
  
  //
  // Allocate Memory for the image from our made up HOBs
  //
  ImageContext.ImageAddress = AllocateZeroedHobPages (EFI_SIZE_TO_PAGES ((UINT32)ImageContext.ImageSize));
  if (ImageContext.ImageAddress == 0) {
    return 0;
  }

  Status = PeCoffLoaderLoadImage (NULL, &ImageContext);
  if (EFI_ERROR (Status)) {
    return 0;
  }

  Status = PeCoffLoaderRelocateImage (NULL, &ImageContext);
  if (EFI_ERROR (Status)) {
    return 0;
  }

  //
  // BugBug: We would flush the Instruction Cache here to follow architecture.
  // x64 and IA-32 parts do not require one so I left it out.
  //

  return (HOB_ONLY_ENTRY)(UINTN)ImageContext.EntryPoint;
}

