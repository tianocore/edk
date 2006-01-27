/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Go64.c

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

#include "Go64.h"

//
// A block of allocated pages will be created to contain the stack.
//
#define TEMP_STACK_PAGES 16
#define TEMP_STACK_SIZE ( TEMP_STACK_PAGES << 12 )

//
// This guid must match the file name in the *.inf file for PpisNeededByDxeCore PEIM
//
EFI_GUID mPpisNeededByDxeCoreGuid = { 0x4D37DA42, 0x3A0C, 0x4eda, { 0xB9, 0xEB, 0xBC, 0x0E, 0x1D, 0xB4, 0x71, 0x3B }};

EFI_HANDLE    gMyImageHandle;

HOB_TEMPLATE  gHobTemplate = {
  { // Phit
    {  // Header
      EFI_HOB_TYPE_HANDOFF,                 // HobType
      sizeof (EFI_HOB_HANDOFF_INFO_TABLE),  // HobLength
      0                                     // Reserved
    },
    EFI_HOB_HANDOFF_TABLE_VERSION,          // Version
    BOOT_WITH_FULL_CONFIGURATION,           // BootMode
    0,                                      // EfiMemoryTop
    0,                                      // EfiMemoryBottom
    0,                                      // EfiFreeMemoryTop
    0,                                      // EfiFreeMemoryBottom
    0                                       // EfiEndOfHobList
  }, 
  { // Bfv
    {
      EFI_HOB_TYPE_FV,                      // HobType
      sizeof (EFI_HOB_FIRMWARE_VOLUME),     // HobLength
      0                                     // Reserved
    },
    0,                                      // BaseAddress
    0                                       // Length
  },
  { // BfvResource
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,     // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR), // HobLength
      0                                     // Reserved
    },
    {
      0                                     // Owner Guid
    },
    EFI_RESOURCE_FIRMWARE_DEVICE,           // ResourceType
    (EFI_RESOURCE_ATTRIBUTE_PRESENT    |
     EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
     EFI_RESOURCE_ATTRIBUTE_TESTED |
     EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),  // ResourceAttribute
    0,                                              // PhysicalStart
    0                                               // ResourceLength
  },
  { // Cpu
    { // Header
      EFI_HOB_TYPE_CPU,                     // HobType
      sizeof (EFI_HOB_CPU),                 // HobLength
      0                                     // Reserved
    },
    52,                                     // SizeOfMemorySpace - Architecture Max
    16,                                     // SizeOfIoSpace,
    {
      0, 0, 0, 0, 0, 0                      // Reserved[6]
    }
  },
  {   // Stack HOB
    {   // header
      EFI_HOB_TYPE_MEMORY_ALLOCATION,               // Hob type
      sizeof (EFI_HOB_MEMORY_ALLOCATION_STACK),     // Hob size
      0                                             // reserved
    },
    {
      {
        0, //EFI_HOB_MEMORY_ALLOC_MODULE_GUID       // Name
      },
      0x0,                                          // EFI_PHYSICAL_ADDRESS  MemoryBaseAddress;
      0x0,                                          // UINT64                MemoryLength;
      EfiBootServicesData,                          // EFI_MEMORY_TYPE       MemoryType;  
      0, 0, 0, 0                                    // Reserved              Reserved[4]; 
    }
  },
  { // MemoryAllocation for HOB's & Images
    {
      EFI_HOB_TYPE_MEMORY_ALLOCATION,               // HobType
      sizeof (EFI_HOB_MEMORY_ALLOCATION),           // HobLength
      0                                             // Reserved
    },
    {
      {
        0, //EFI_HOB_MEMORY_ALLOC_MODULE_GUID       // Name
      },
      0x0,                                          // EFI_PHYSICAL_ADDRESS  MemoryBaseAddress;
      0x0,                                          // UINT64                MemoryLength;
      EfiBootServicesData,                          // EFI_MEMORY_TYPE       MemoryType;  
      {
        0, 0, 0, 0                                  // Reserved              Reserved[4]; 
      }
    }
   },
  { // MemoryFreeUnder1MB for unused memory that DXE core will claim
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,             // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR),         // HobLength
      0                                             // Reserved
    },
    {
      0                                             // Owner Guid
    },
    EFI_RESOURCE_SYSTEM_MEMORY,                     // ResourceType
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
  { // MemoryFreeAbove1MB for unused memory that DXE core will claim
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,             // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR),         // HobLength
      0                                             // Reserved
    },
    {
      0                                             // Owner Guid
    },
    EFI_RESOURCE_SYSTEM_MEMORY,                     // ResourceType
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
  { // MemoryFreeAbove4GB for unused memory that DXE core will claim
    {
      EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,             // HobType
      sizeof (EFI_HOB_RESOURCE_DESCRIPTOR),         // HobLength
      0                                             // Reserved
    },
    {
      0                                             // Owner Guid
    },
    EFI_RESOURCE_SYSTEM_MEMORY,                     // ResourceType
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
  {   // Memory Allocation Module for DxeCore
    {   // header
      EFI_HOB_TYPE_GUID_EXTENSION,                  // Hob type
      sizeof (EFI_HOB_MEMORY_ALLOCATION_MODULE),    // Hob size
      0                                             // reserved
    },
    {
      {
        0, //EFI_HOB_MEMORY_ALLOC_MODULE_GUID       // Guid
      },
      0x0,                                          // EFI_PHYSICAL_ADDRESS  MemoryBaseAddress;
      0x0,                                          // UINT64                MemoryLength;
      EfiBootServicesCode,                          // EFI_MEMORY_TYPE       MemoryType;  
      {
        0, 0, 0, 0                                  // UINT8                 Reserved[4]; 
      },
    },
    {
      0, //EFI_HOB_MEMORY_ALLOC_MODULE_GUID         //  BugBug - Should be Fv File name at some point
    },
    0x0                                             //  EFI_PHYSICAL_ADDRESS of EntryPoint;
  },
  { // Memory Map Hints to reduce fragmentation in the memory map
    EFI_HOB_TYPE_GUID_EXTENSION,                    // Hob type
    sizeof (MEMORY_TYPE_INFORMATION_HOB),           // Hob size
    0,                                              // reserved
    EFI_MEMORY_TYPE_INFORMATION_GUID,
    {
      {
        EfiACPIReclaimMemory,
        0x40
      },  // 0x40 pages = 256k for ASL
      {
        EfiACPIMemoryNVS,
        0x80
      },  // 0x80 pages = 512k for S3, SMM, etc
      {
        EfiReservedMemoryType,
        0x04
      },  // 16k for BIOS Reserved
      {
        EfiRuntimeServicesData,
        0x100
      },
      {
        EfiRuntimeServicesCode,
        0x100
      },
      {
        EfiBootServicesCode,
        0x200
      },
      {
        EfiBootServicesData,
        0x200
      },
      {
        EfiLoaderCode,
        0x100
      },
      {
        EfiLoaderData,
        0x100
      },
      {
        EfiMaxMemoryType,
        0
      }
    }
  },
  { // Pointer to ACPI Table
    EFI_HOB_TYPE_GUID_EXTENSION,       // Hob type
    sizeof (TABLE_HOB),                // Hob size
    0,                                 // reserved
    EFI_ACPI_20_TABLE_GUID,
    0
  },
  { // Pointer to SMBIOS Table
    EFI_HOB_TYPE_GUID_EXTENSION,       // Hob type
    sizeof (TABLE_HOB),                // Hob size
    0,                                 // reserved
    EFI_SMBIOS_TABLE_GUID,
    0
  },
  { // EndOfHobList
    EFI_HOB_TYPE_END_OF_HOB_LIST,      // HobType
    sizeof (EFI_HOB_GENERIC_HEADER),   // HobLength
    0                                  // Reserved
  }
};

HOB_TEMPLATE  *gHob = &gHobTemplate;

EFI_STATUS
EFIAPI
Go64Worker (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
;

EFI_GUID mDxeFileName = { 0xb1644c1a, 0xc16a, 0x4c5b, { 0x88, 0xde, 0xea, 0xfb, 0xa9, 0x7e, 0x74, 0xd8 } };

UINT64
Exit32bitBootServicesAndGrabMemory (
  IN  EFI_HANDLE            ImageHandle,
  OUT EFI_PHYSICAL_ADDRESS  *Base,
  OUT EFI_PHYSICAL_ADDRESS  *MemoryAbove4G,
  OUT UINT64                *SizeMemoryAbove4G
  )
/*++

Routine Description:
  Shutdown the EFI services as we are about to launch an alternate EDK 
  environment. This module returns a memory range that is a continous
  range of memory starting some where around 1MB and ending when a
  reserved memory region is found.

Arguments:
  ImageHandle - Handle of this Image (this driver)
  Base        - Returns the address of the memory found by this driver

Returns:
  Size of Base memory range in bytes.
  Zero on error

--*/
{
  EFI_STATUS                  Status;
  EFI_STATUS                  MemMapStatus;
  UINTN                       MapKey;
  UINTN                       BufferSize;
  EFI_MEMORY_DESCRIPTOR       *Buffer;
  EFI_MEMORY_DESCRIPTOR       *Descriptor;
  UINTN                       DescriptorSize;
  UINT32                      DescriptorVersion;
  UINTN                       Index;
  EFI_PHYSICAL_ADDRESS        End;
  EFI_PHYSICAL_ADDRESS        EndMemoryAbove4GB;
  EFI_PHYSICAL_ADDRESS        TempAddress;
  
  //
  // First call to GetMemoryMap will fail, and then tell us how much memory needs to be allocated
  //
  BufferSize = 0;
  Buffer = NULL;
  do {
    MemMapStatus = gBS->GetMemoryMap (&BufferSize, Buffer, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (MemMapStatus == EFI_BUFFER_TOO_SMALL) {
      if (Buffer != NULL) {
        //
        // Free the previous buffer
        //
        gBS->FreePool (Buffer);
      }

      //
      // There are cases where this allocation will change the memory map. 
      //  thats why we need to do the allocate pool in a loop. We also never
      //  free the buffer that works as the memory map has to be the same
      //  when we call ExitBootServices. Freeing the buffer could cause 
      //  ExitBootServices to fail.
      //
      Status = gBS->AllocatePool (
                      EfiLoaderData,
                      BufferSize,
                      (VOID **)&Buffer
                      );
      if (EFI_ERROR (Status)) {
        return 0;
      }
    }
  } while (EFI_ERROR (MemMapStatus));

  //
  // Act like an OS loader and find all the contiguous memory from 1 MB and up. 
  //  Stop when you hit a reserved memory type the OS would not use. 
  //
  End = *Base = 0;
  *MemoryAbove4G = 0;
  *SizeMemoryAbove4G = 0;
  EndMemoryAbove4GB = 0;
  TempAddress = 0;
  for (Index = 0, Descriptor = Buffer; Index < (BufferSize/DescriptorSize); Index++) {
    if (Descriptor->PhysicalStart >= 0x100000) {   
      //
      // Only process regions about 1MB
      //
      if (*Base == 0) {
       	//
       	// Remember the first memory address >= 1MB
       	//
       	*Base = Descriptor->PhysicalStart;
      }
      if ((Descriptor->Type == EfiReservedMemoryType) || 
       		  (Descriptor->Type == EfiRuntimeServicesCode) ||  
         		(Descriptor->Type == EfiRuntimeServicesData) ||
         		(Descriptor->Type >= EfiACPIReclaimMemory) ) {        
         if(End == 0)	{
         	  End = TempAddress; 
         	}       
      }
    } else if (Descriptor->PhysicalStart >= 0x100000000 ) {
      //
      // Look for a contiguous region above 4GB
      //
      if (*MemoryAbove4G == 0) {
        *MemoryAbove4G = Descriptor->PhysicalStart;
      }

      if ((Descriptor->Type == EfiReservedMemoryType) ||
          (Descriptor->Type == EfiRuntimeServicesCode) ||
          (Descriptor->Type == EfiRuntimeServicesData) ||
          (Descriptor->Type >= EfiACPIReclaimMemory) ) { 
          if(EndMemoryAbove4GB == 0) { 	       
           EndMemoryAbove4GB = TempAddress;
          }
        }
    }
    
    TempAddress = Descriptor->PhysicalStart + LShiftU64 (Descriptor->NumberOfPages, EFI_PAGE_SHIFT);
    //
    // Have to use the abstraction to walk the descriptors as the size of descriptors may vary
    //
    Descriptor = NextMemoryDescriptor (Descriptor, DescriptorSize);
  }
      
  //
  // Shutdown the 32-bit EFI implementation
  //
  //DEBUG ((EFI_D_ERROR, "Go64.EFI Exiting Boot Services. Next Message from x64 code\n"));
  Status = gBS->ExitBootServices (ImageHandle, MapKey);
  if (EFI_ERROR (Status)) {
    return 0;
  }
  *SizeMemoryAbove4G = EndMemoryAbove4GB - *MemoryAbove4G;
  return (End - *Base);
}

VOID
Corrupt32bitEfiSystemTablePointer (
  VOID
  )
/*++

Routine Description:
 Corrupt the EFI System Table pointer. This is needed since a debugger may find this
 table. If the debugger finds this table it will think these are the drivers. The
 EoE will create it's own table when it gets launched. 

Arguments:
  None 

Returns:
  None

--*/
{
  UINT8                     *Ptr;
  EFI_SYSTEM_TABLE_POINTER  DebugTable;
  EFI_SYSTEM_TABLE_POINTER  *DebugTablePtr;

  //
  // Find the table on a 4 MB boundry. 
  //
  for (Ptr = (UINT8 *)0xffc00000; Ptr > (UINT8 *)0x0fffff; Ptr -= 0x400000) {
    DebugTablePtr = (EFI_SYSTEM_TABLE_POINTER *)Ptr;
    if (DebugTablePtr->Signature == EFI_SYSTEM_TABLE_SIGNATURE) {
      gBS->CopyMem (&DebugTable, DebugTablePtr, sizeof (EFI_SYSTEM_TABLE_POINTER));
      DebugTable.Crc32 = 0;
      gBS->CalculateCrc32 ((VOID *)&DebugTable, sizeof (EFI_SYSTEM_TABLE_POINTER), &DebugTable.Crc32);
      if (DebugTable.Crc32 == DebugTablePtr->Crc32) {
        //
        // Valid Debug Table found. Destroy it!
        //
        DebugTablePtr->Signature = 0;
      }
    }
  }
}

EFI_STATUS
AllocateMemoryLessThan (
  IN  OUT EFI_PHYSICAL_ADDRESS  *Address,
  IN  OUT UINTN                 *NumberOfPages
  )
/*++

Routine Description:
  Allocate NumberOfBytes bellow the value of address.

Arguments:
  Address       - On input highest address to return. On output address that 
                   was allocated
  NumberOfBytes - Number of bytes to allocate and number allocated

Returns:
  EFI_SUCCESS - Memory Allocated
  other       - Memory not allocated

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhysAddress;

  PhysAddress   = *Address;

  for (;;) {
    Status = gBS->AllocatePages (
                    AllocateMaxAddress, 
                    EfiBootServicesData, 
                    *NumberOfPages,
                    &PhysAddress
                    );
    if (!EFI_ERROR (Status)) {
      *Address = PhysAddress;
      return Status;
    }
    *NumberOfPages = *NumberOfPages - 1;
    if (*NumberOfPages == 0) {
      return EFI_OUT_OF_RESOURCES;
    }
  }  
}


VOID *
AllocateTempPages (
  IN  UINTN   NumberOfPages
  )
/*++

Routine Description:
  Allocate Size bytes of memory from the Temp Pool. The temp pool
  is valid EFI memory that will not last after we load the EoE. The
  Temp memory lets the HOB construction code use teh same high memory
  regions as the EFI implemtnations we loaded from. Thats why the 
  Temp memory comes from memory under 64 MB (much lower than the EFI memory
  that is at the top of memory). If your platform has a small memory foot print
  you should lower the 64MB threashold. 

Arguments:
  NumberOfPages - NumberOfPages to allocat

Returns:

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  Address;

  Address = 0x04000000;
  Status = AllocateMemoryLessThan  (&Address, &NumberOfPages);
  if (EFI_ERROR (Status)) {
    return 0;
  } else {
    return (VOID *)(UINTN)Address;
  }
}


EFI_DRIVER_ENTRY_POINT (Go64Init)

EFI_STATUS
EFIAPI
Go64Init (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:
  EFI Driver that acts like an OS loader. It does an ExitBootServices then
  it loads a firmware volume x64.FV into memory. The code builds page tables
  and goes to long mode (x64). This code enulates the PEI phase and produces
  hobs. An x64 psudo PEIM is loaded from the FV that contains PPI's needed by
  the Dxe Core. This psudo PEIM will add PPI's directly into the simulated
  hob structures. Handoff is made to the x64 DXE core that is read from the 
  FV. On EFI only systems the psudo PEIM and DXE core files are read from disk.

  This driver assumes the x64.FV is in the same location on the media as this
  driver.

  It's a wrapper function just to switch stack and call the real worker Go64Init.

Arguments:
  ImageHandle - Handle of this image
  SystemTable - EFI System Table

Returns:
  This function does not return. 

--*/
{
  UINTN                       Buffer;

  EfiInitializeDriverLib (ImageHandle, SystemTable);

  Buffer = (UINTN) AllocateTempPages (TEMP_STACK_PAGES);

  SwitchStacks (
    (VOID *) (UINTN) Go64Worker,
    (UINTN)  (ImageHandle),
    (UINTN)  (SystemTable),
    (VOID *) (UINTN) (Buffer + TEMP_STACK_SIZE - sizeof (UINTN))
    );
  
  //
  // We should never return.
  //
  EFI_DEADLOOP ();

  return EFI_DEVICE_ERROR;
}

EFI_STATUS
EFIAPI
Go64Worker (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:
  EFI Driver that acts like an OS loader. It does an ExitBootServices then
  it loads a firmware volume x64.FV into memory. The code builds page tables
  and goes to long mode (x64). This code enulates the PEI phase and produces
  hobs. An x64 psudo PEIM is loaded from the FV that contains PPI's needed by
  the Dxe Core. This psudo PEIM will add PPI's directly into the simulated
  hob structures. Handoff is made to the x64 DXE core that is read from the 
  FV. On EFI only systems the psudo PEIM and DXE core files are read from disk.

  This driver assumes the x64.FV is in the same location on the media as this
  driver.

Arguments:
  ImageHandle - Handle of this image
  SystemTable - EFI System Table

Returns:
  on success this driver does an ExitBootServices and does not return.
  On Error an Error is retured. 

--*/
{
  EFI_STATUS                  Status;
  EFI_PHYSICAL_ADDRESS        Base;
  EFI_PHYSICAL_ADDRESS        UsedBase;
  UINTN                       BaseSizeInBytes;
  VOID                        *Bfv;
  UINTN                       BfvLength;
  UINTN                       BfvLengthPageSize;
  EFI_PHYSICAL_ADDRESS        PageTables;
  EFI_PHYSICAL_ADDRESS        Stack;
  EFI_PHYSICAL_ADDRESS        DxeCoreEntryPoint;
  EFI_PHYSICAL_ADDRESS        PpisNeededByDxeIplEntryPoint;
  VOID                        *Image;
  UINTN                       ImageSize;
  VOID                        *Image2;
  UINTN                       ImageSize2;
  EFI_HANDLE                  FvHandle;
  EFI_GUID                    DxeFileName;
  EFI_PHYSICAL_ADDRESS        Address;
  UINTN                       NumberOfPages; 
  UINTN                       NumberOfBytes;
  VOID                        *Table;

  gMyImageHandle = ImageHandle;

  //
  // Go find x64.FV, the FV with all the x64 code on the disk
  //
  Bfv = GetFileFromWhereWeLoaded (L"x64.FV", &BfvLength);
  if (Bfv == NULL) {
    //
    // The Bfv File should be located in the same directory as this driver.
    // If we can't find it exit now
    //
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  //
  // Initialize with a random GUID
  //
  DxeFileName = mDxeFileName;

  Image = Image2 = NULL;
  ImageSize = ImageSize2 = 0;
  if (gDS != NULL) {
    //
    // Use Tiano to give us access to the FV.
    //
    Status = gDS->ProcessFirmwareVolume (Bfv, BfvLength, &FvHandle);
    if (!EFI_ERROR (Status)) {
      Image  = FindFileInFv (FvHandle, EFI_SECTION_PE32, &mPpisNeededByDxeCoreGuid, &ImageSize);

      Status = FindDxeCoreFileName (FvHandle, &DxeFileName);
      if (!EFI_ERROR (Status)) {
        Image2 = FindFileInFv (FvHandle, EFI_SECTION_PE32, &DxeFileName, &ImageSize2);
      }
    }
  }

  if (Image == NULL) {
    //
    // If it's not Tiano or PpisNeededByDxeCore.EFI is not in the FV load it from
    //  the same path as this driver.
    //
    Image = GetFileFromWhereWeLoaded (L"PpisNeededByDxeCore.EFI", &ImageSize);
  }
  ASSERT (Image != NULL);

  if (Image2 == NULL) {
    //
    // If it's not Tiano or DxeMain.EFI is not in the FV load it from
    //  the same path as this driver.
    //
    Image2 = GetFileFromWhereWeLoaded (L"DxeMain.EFI", &ImageSize2);
  }
  ASSERT (Image2 != NULL);
  
  //
  // Make HOB's for Acpi and SMBIOS tables so we can pass them up to EOE
  //
  Status = EfiLibGetSystemConfigurationTable (&gEfiAcpiTableGuid, &Table);
  if (EFI_ERROR (Status)) {
    Status = EfiLibGetSystemConfigurationTable (&gEfiAcpi20TableGuid, &Table);
  }
  gHob->Acpi.Table =  (EFI_PHYSICAL_ADDRESS)Table;

  EfiLibGetSystemConfigurationTable (&gEfiSmbiosTableGuid, &Table);
  gHob->Smbios.Table =  (EFI_PHYSICAL_ADDRESS)Table;
  
  //
  // The EFI Debug Tool can not debug the x64 code as it gets stuck on the IA-32 code.
  //  This function will null out the EFI_SYSTEM_TABLE_POINTER structure from the Tiano
  //  implementation it was loaded from. 
  //
  // If you want to debug IA-32 loaded from the current version of EFI you may need to 
  //  remove this line.
  //
  Corrupt32bitEfiSystemTablePointer ();

  //
  // Retrieve MTRRs
  //
  EfiGetMtrrs ();

  //
  // Allocate some space < 1MB for thunking to real mode.
  // 0x18 pages is 72 KB.
  //
  Address       = 0x100000 - 1;
  NumberOfPages = 0x18;
  Status = AllocateMemoryLessThan (&Address, &NumberOfPages);
  ASSERT_EFI_ERROR (Status);

  gHob->MemoryFreeUnder1MB.PhysicalStart  = Address;
  gHob->MemoryFreeUnder1MB.ResourceLength = NumberOfPages * EFI_PAGE_SIZE;;

  //
  // Go ahead and do an ExitBootServices for the 32-bit EFI.
  //
  BaseSizeInBytes = (UINTN)Exit32bitBootServicesAndGrabMemory (
                            ImageHandle, 
                            &UsedBase,
                            &gHob->MemoryAbove4GB.PhysicalStart,
                            &gHob->MemoryAbove4GB.ResourceLength
                            );
                          
  if ((BaseSizeInBytes == 0) || (BaseSizeInBytes < CONSUMED_MEMORY)) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }
  //
  // Load the GDT of Go64. Since the GDT of 32-bit Tiano locates in the BS_DATA \
  // memory, it may be corrupted when copying FV to high-end memory 
  LoadGo64Gdt();
  
  if (gHob->MemoryAbove4GB.ResourceLength == 0) {
    //
    // If there is no memory above 4GB then change the resource descriptor HOB
    //  into another type. I'm doing this as it's unclear if a resource
    //  descriptor HOB of length zero is valid. Spec does not say it's illegal,
    //  but code in EDK does not seem to handle this case.
    //
    gHob->MemoryAbove4GB.Header.HobType = EFI_HOB_TYPE_UNUSED;
  }

  //
  // Calculate BFV location at top of the memory region.
  // This is like a RAM Disk. Align to page boundry.
  //
  BfvLengthPageSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (BfvLength));
 
  BaseSizeInBytes -= BfvLengthPageSize;
  Base = UsedBase + BaseSizeInBytes;

  gHob->Bfv.BaseAddress = Base;
  gHob->Bfv.Length = BfvLength;

  //
  // Resource descriptor for the FV
  //
  gHob->BfvResource.PhysicalStart = gHob->Bfv.BaseAddress;
  gHob->BfvResource.ResourceLength = gHob->Bfv.Length;

  //
  // Resource descriptor for the memory above 1MB less than 4GB gap (if any)
  //
  gHob->MemoryAbove1MB.PhysicalStart  = UsedBase;
  gHob->MemoryAbove1MB.ResourceLength = BaseSizeInBytes;

  //
  // Use the upper CONSUMED_MEMORY bytes for PEI emulation
  // Base is currently the Base of the BFV
  //
  gHob->Phit.EfiMemoryTop = gHob->Phit.EfiFreeMemoryTop = Base;
  gHob->Phit.EfiMemoryBottom = gHob->Phit.EfiFreeMemoryBottom = Base - CONSUMED_MEMORY;
  
  //
  // Add in size for the hoblist
  //
  gHob->Phit.EfiFreeMemoryBottom += sizeof (HOB_TEMPLATE);
  
  //
  // FreeMemoryBottom points past the end of the HOB_TEMPLATE. EndOfHobList
  //  needs to point to the END HOB. Thats why we back up a HOB header.
  //
  gHob->Phit.EfiEndOfHobList = gHob->Phit.EfiFreeMemoryBottom - sizeof (EFI_HOB_GENERIC_HEADER);


  //
  // Map all the dynamically consumed memory
  //
  gHob->MemoryAllocation.AllocDescriptor.MemoryBaseAddress  = gHob->Phit.EfiFreeMemoryTop;
  gHob->MemoryAllocation.AllocDescriptor.MemoryLength       = 0;

  //
  // Limit to 36 bits of addressing for debug. Should get it from CPU
  //
  PageTables = CreateIdentityMappingPageTables (36);

  //
  // Load the images we found earlier in the FV
  //
  PpisNeededByDxeIplEntryPoint = Loadx64PeImage (&Image, &ImageSize);
  ASSERT (PpisNeededByDxeIplEntryPoint != 0);  

  DxeCoreEntryPoint = Loadx64PeImage (&Image2, &ImageSize2);
  ASSERT (DxeCoreEntryPoint != 0);

  //
  // Fill in module alloc hob
  //
  gHob->DxeCore.MemoryAllocationHeader.Name = gEfiHobMemeryAllocModuleGuid;
  gHob->DxeCore.MemoryAllocationHeader.MemoryBaseAddress = (EFI_PHYSICAL_ADDRESS) Image2;
  gHob->DxeCore.MemoryAllocationHeader.MemoryLength = ImageSize2;
  gHob->DxeCore.EntryPoint = DxeCoreEntryPoint;
  gHob->DxeCore.ModuleName = DxeFileName;

  //
  // Copy Boot Firmware Volume to high memory
  //
  EfiCommonLibCopyMem ((VOID *)(UINTN)(gHob->Bfv.BaseAddress), Bfv, BfvLength);
  //
  // Allocate 128K of stack. 
  //  Don't make any calls to AllocateZeroedHobPages () after the stack 
  //  is allocated as it will break the memory resource hob since we only
  //  have a single HOB it does not support fragmentation. The PEI CIS
  //  states the stack needs it's own resource descriptor.
  //
  gHob->Stack.AllocDescriptor.Name = gEfiHobMemeryAllocStackGuid;

  NumberOfBytes = 32 * EFI_PAGE_SIZE;
  gHob->Stack.AllocDescriptor.MemoryLength = NumberOfBytes;

  //
  // Mark the stack as not being free in the PHIT
  //
  gHob->Phit.EfiFreeMemoryTop -= NumberOfBytes;
  Stack = gHob->Phit.EfiFreeMemoryTop;
  gHob->Stack.AllocDescriptor.MemoryBaseAddress = Stack;

  if (Stack < gHob->Phit.EfiFreeMemoryBottom) {
    //
    // We ran out of memory to allocat the Stack from
    //
    EFI_DEADLOOP ();
  }


  Stack = Stack + 32 * 0x1000 - 16;

  //
  // Copy HOBs to final memory location memory. 
  //
  gHob = (HOB_TEMPLATE *)(UINTN)(gHob->Phit.EfiMemoryBottom);
  EfiCommonLibCopyMem (gHob, &gHobTemplate, sizeof (HOB_TEMPLATE));

  //
  // Go to Long Mode. Interrupts will not get turned on until the CPU AP is loaded.
  // Call x64 drivers passing in single argument, a pointer to the HOBs.
  //
  ActivateLongMode (
    PageTables, 
    (EFI_PHYSICAL_ADDRESS)(UINTN)gHob, 
    Stack,
    PpisNeededByDxeIplEntryPoint,
    DxeCoreEntryPoint
    );
  //
  // We should never return
  //
  EFI_DEADLOOP ();

  return EFI_DEVICE_ERROR;
}



EFI_PHYSICAL_ADDRESS
AllocateZeroedHobPages (
  IN  UINTN   NumberOfPages
  )
/*++

Routine Description:
  Allocate pages from HOBs. Pages are EFI pages and 4K on all architectures in all modes.

Arguments:
  NumberOfPages - Number of EFI pages to allocate from the simulated PEI memory map

Returns:
  Base address of the pages or zero if error.

--*/
{
  EFI_PHYSICAL_ADDRESS    Page;
  UINTN                   NumberOfBytes;

  //
  // EFI pages are 4K by convention. EFI pages are independent of processor page size
  //
  NumberOfBytes = 0x1000 * NumberOfPages;

  if ((gHob->Phit.EfiFreeMemoryTop - NumberOfBytes) < gHob->Phit.EfiFreeMemoryBottom) {
    return 0;
  }

  //
  // Allocate the EFI pages out of Hob Free Memory Heap.
  // Heap grows down from top of Free Memory. HOB grows up.
  //
  gHob->Phit.EfiFreeMemoryTop -= NumberOfBytes;
  Page = gHob->Phit.EfiFreeMemoryTop;

  EfiCommonLibZeroMem ((VOID *)(UINTN)Page, NumberOfBytes);


  //
  // Update the memory descriptor HOB.
  //
  gHob->MemoryAllocation.AllocDescriptor.MemoryBaseAddress  = gHob->Phit.EfiFreeMemoryTop;
  gHob->MemoryAllocation.AllocDescriptor.MemoryLength      += NumberOfBytes;

  return Page;
}


EFI_PHYSICAL_ADDRESS
Loadx64PeImage (
  IN OUT  VOID  **PeImage,
  IN OUT  UINTN *PeImageSize
  )
/*++

Routine Description:
  32-bit code that loads an x64 PE32 image.

Arguments:
  PeImage     - Pointer to x64 PE32 image
  PeImageSize - Size of PeImage

Returns:
  Address of the loaded x64 PE32 image

--*/
{
  EFI_STATUS                                Status;
  EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT      ImageContext;

  //
  // Initialize ImageContext for PeImage
  //
  PeCoffLoaderInitializeImageContext (&ImageContext, *PeImage);

  //
  // Get the size of the Image into the ImageContext
  //
  PeCoffLoaderGetImageInfo (NULL, &ImageContext);
  
  //
  // Allocate Memory for the image from our made up HOBs
  //
  ImageContext.ImageAddress = AllocateZeroedHobPages (EFI_SIZE_TO_PAGES ((UINT32)ImageContext.ImageSize));
  *PeImage = (VOID *)(UINTN)ImageContext.ImageAddress;
  *PeImageSize = (UINTN)ImageContext.ImageSize;
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
  // x64 parts do not require one so I left it out.
  //

  return ImageContext.EntryPoint;
}


