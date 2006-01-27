/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Go64.h

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

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "PeiHob.h"
#include "EfiImage.h"
#include "EfiImageFormat.h"

#include EFI_PROTOCOL_CONSUMER (LoadedImage)
#include EFI_PROTOCOL_CONSUMER (DevicePath)
#include EFI_PROTOCOL_CONSUMER (SimpleFileSystem)
#include EFI_PROTOCOL_CONSUMER (FirmwareVolume)
#include EFI_PROTOCOL_CONSUMER (FileInfo)
#include EFI_GUID_DEFINITION   (PeiPeCoffLoader)
#include EFI_GUID_DEFINITION   (MemoryAllocationHob)
#include EFI_GUID_DEFINITION   (DebugImageInfoTable)
#include EFI_GUID_DEFINITION   (MemoryTypeInformation)
#include EFI_GUID_DEFINITION   (SmBios)
#include EFI_GUID_DEFINITION   (Acpi)


VOID *
GetFileFromWhereWeLoaded (
  IN  CHAR16      *InputFile,
  OUT UINTN       *BfvLength
  );

UINTN
GetMemoryFromMap (
  OUT EFI_PHYSICAL_ADDRESS  *Base
  );

EFI_PHYSICAL_ADDRESS
AllocateZeroedHobPages (
  IN  UINTN   NumberOfPages
  );

EFI_PHYSICAL_ADDRESS
CreateIdentityMappingPageTables (
  IN UINT32 NumberOfProcessorPhysicalAddressBits
  );

VOID
Go64CopyMem (
  IN VOID   *Destination,
  IN VOID   *Source,
  IN UINTN  Length
  );

VOID *
FindFileInFv (
  IN  EFI_HANDLE  *Bfv,
  IN  UINT8       Type,
  IN  EFI_GUID    *FileName,
  OUT UINTN       *ImageSize
  );

EFI_STATUS
FindDxeCoreFileName (
  IN  EFI_HANDLE        *Bfv,
  OUT EFI_GUID          *FileName
  );

EFI_PHYSICAL_ADDRESS
Loadx64PeImage (
  IN OUT  VOID                  **Image, 
  IN OUT  UINTN                 *ImageSize
  );

VOID
SwitchStacks (
  VOID  *EntryPoint,
  UINTN Parameter1,
  UINTN Parameter2,
  VOID  *NewStack
  );

VOID
ActivateLongMode (
  IN  EFI_PHYSICAL_ADDRESS  PageTables,  
  IN  EFI_PHYSICAL_ADDRESS  HobStart,
  IN  EFI_PHYSICAL_ADDRESS  Stack,
  IN  EFI_PHYSICAL_ADDRESS  CodeEntryPoint1,
  IN  EFI_PHYSICAL_ADDRESS  CodeEntryPoint2
  );
  
VOID
LoadGo64Gdt();
#define CONSUMED_MEMORY  0x400000   // Firmware Volume + Stack + HOBs + Page Tables

#pragma pack(1)

typedef struct {
  EFI_HOB_GUID_TYPE             Hob;
  EFI_MEMORY_TYPE_INFORMATION   Info[10];
} MEMORY_TYPE_INFORMATION_HOB;

typedef struct {
  EFI_HOB_GUID_TYPE             Hob;
  EFI_PHYSICAL_ADDRESS          Table;
} TABLE_HOB;

typedef struct {
  EFI_HOB_HANDOFF_INFO_TABLE        Phit;
  EFI_HOB_FIRMWARE_VOLUME           Bfv;
  EFI_HOB_RESOURCE_DESCRIPTOR       BfvResource;
  EFI_HOB_CPU                       Cpu;
  EFI_HOB_MEMORY_ALLOCATION_STACK   Stack;
  EFI_HOB_MEMORY_ALLOCATION         MemoryAllocation;
  EFI_HOB_RESOURCE_DESCRIPTOR       MemoryFreeUnder1MB;
  EFI_HOB_RESOURCE_DESCRIPTOR       MemoryAbove1MB;
  EFI_HOB_RESOURCE_DESCRIPTOR       MemoryAbove4GB;
  EFI_HOB_MEMORY_ALLOCATION_MODULE  DxeCore;
  MEMORY_TYPE_INFORMATION_HOB       MemoryTypeInfo;
  TABLE_HOB                         Acpi;
  TABLE_HOB                         Smbios;
  EFI_HOB_GENERIC_HEADER            EndOfHobList;
} HOB_TEMPLATE;

#pragma pack()

//
// Define macro to determine if the machine type is supported.
// Returns 0 if the machine is not supported, Not 0 otherwise.
//
#define EFI_IMAGE_MACHINE_TYPE_SUPPORTED(Machine) \
  ((Machine) == EFI_IMAGE_MACHINE_X64 || \
   (Machine) == EFI_IMAGE_MACHINE_IA32  || \
   (Machine) == EFI_IMAGE_MACHINE_EBC)

typedef struct {
  UINT8                             *Buffer;
  UINT64                            Size;
  UINT32                            PeCoffHeaderOffset;
  EFI_IMAGE_OPTIONAL_HEADER64       *OptionalHeader;
  EFI_PHYSICAL_ADDRESS              ImageAddress;
  UINT64                            ImageSize;
  UINT64                            DestinationAddress;
  BOOLEAN                           RelocationsStripped;
  VOID                              *FixupData;
  UINTN                             FixupDataSize;
  UINTN                             SizeOfHeaders;
  UINT32                            SectionAlignment;
  UINT16                            ImageType;
  EFI_PHYSICAL_ADDRESS              EntryPoint;
  UINT32                            DebugDirectoryEntryRva;
} IMAGE_CONTEXT;

EFI_STATUS
PeCoffLoaderInitializeImageContext (
  EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT      *ImageContext,
  VOID                                  *PeImage
  );

EFI_STATUS
EFIAPI
PeCoffLoaderGetImageInfo (
  IN     EFI_PEI_PE_COFF_LOADER_PROTOCOL           *This,
  IN OUT EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT      *ImageContext
  );

EFI_STATUS
EFIAPI
PeCoffLoaderLoadImage (
  IN     EFI_PEI_PE_COFF_LOADER_PROTOCOL           *This,
  IN OUT EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT      *ImageContext
  );

EFI_STATUS
EFIAPI
PeCoffLoaderRelocateImage (
  IN     EFI_PEI_PE_COFF_LOADER_PROTOCOL           *This,
  IN OUT EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT      *ImageContext
  );

EFI_STATUS
EfiGetMtrrs (
  VOID
  );

VOID *
AllocateTempPages (
  IN  UINTN   NumberOfPages
  );

extern EFI_HANDLE gMyImageHandle;
