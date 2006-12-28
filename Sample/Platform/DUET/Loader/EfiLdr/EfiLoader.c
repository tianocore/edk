/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  EfiLoader.c

Abstract:

Revision History:

--*/

#include "Tiano.h"
#include EFI_PROTOCOL_DEFINITION (LoadedImage)
#include "EfiCommonLib.h"
#include "PeiLib.h"

#define INT15_E820_AddressRangeMemory   1
#define INT15_E820_AddressRangeReserved 2
#define INT15_E820_AddressRangeACPI     3
#define INT15_E820_AddressRangeNVS      4

#define EFI_FIRMWARE_BASE_ADDRESS  0x00200000

#define EFI_DECOMPRESSED_BUFFER_ADDRESS 0x00600000

#define EFI_MAX_MEMORY_DESCRIPTORS 64

#define LOADED_IMAGE_SIGNATURE     EFI_SIGNATURE_32('l','d','r','i')

typedef struct {
  UINTN                       Signature;
  CHAR16                      *Name;          // Displayable name
  UINTN                       Type;

  BOOLEAN                     Started;        // If entrypoint has been called
  VOID                        *StartImageContext;

  EFI_IMAGE_ENTRY_POINT       EntryPoint;     // The image's entry point
  EFI_LOADED_IMAGE_PROTOCOL   Info;           // loaded image protocol

  // 
  EFI_PHYSICAL_ADDRESS        ImageBasePage;  // Location in memory
  UINTN                       NoPages;        // Number of pages 
  UINT8                       *ImageBase;     // As a char pointer
  UINT8                       *ImageEof;      // End of memory image

  // relocate info
  UINT8                       *ImageAdjust;   // Bias for reloc calculations
  UINTN                       StackAddress;
  UINT8                       *FixupData;     //  Original fixup data
} EFILDR_LOADED_IMAGE;

#pragma pack(4)
typedef struct {          
  UINT64       BaseAddress;
  UINT64       Length;
  UINT32       Type;
} BIOS_MEMORY_MAP_ENTRY;
#pragma pack()

typedef struct {          
  UINT32                MemoryMapSize;
  BIOS_MEMORY_MAP_ENTRY MemoryMapEntry[1];
} BIOS_MEMORY_MAP;

#include "EfiLdrHandoff.h"

#include "PeLoader.h"
#include "Support.h"
#include "Debug.h"

EFI_STATUS
EFIAPI
TianoGetInfo (
  IN      EFI_TIANO_DECOMPRESS_PROTOCOL *This,
  IN      VOID                          *Source,
  IN      UINT32                        SrcSize,
  OUT     UINT32                        *DstSize,
  OUT     UINT32                        *ScratchSize
  );

EFI_STATUS
EFIAPI
TianoDecompress (
  IN      EFI_TIANO_DECOMPRESS_PROTOCOL *This,
  IN      VOID                          *Source,
  IN      UINT32                        SrcSize,
  IN OUT  VOID                          *Destination,
  IN      UINT32                        DstSize,
  IN OUT  VOID                          *Scratch,
  IN      UINT32                        ScratchSize
  );

EFILDR_LOADED_IMAGE    DxeCoreImage;
EFILDR_LOADED_IMAGE    DxeIplImage;

typedef
VOID
(* EFI_MAIN_ENTRYPOINT) (
    IN EFILDRHANDOFF  *Handoff
    );

VOID
EfiLoader (
  UINT32    BiosMemoryMapBaseAddress
  )
{
  BIOS_MEMORY_MAP       *BiosMemoryMap;    
  EFILDR_HEADER         *EFILDRHeader;
  EFILDR_IMAGE          *EFILDRImage;
  EFI_MEMORY_DESCRIPTOR EfiMemoryDescriptor[EFI_MAX_MEMORY_DESCRIPTORS];
  EFI_STATUS            Status;
  UINTN                 NumberOfMemoryMapEntries;
  UINT32                DestinationSize;
  UINT32                ScratchSize;
  UINTN                 BfvPageNumber;
  UINTN                 BfvBase;
  EFI_MAIN_ENTRYPOINT   EfiMainEntrypoint;
  static EFILDRHANDOFF  Handoff;

PrintHeader ('A');

  ClearScreen();
  PrintString("EFI Loader\n");

//  PrintString("&BiosMemoryMapBaseAddress = ");   
//  PrintValue64 ((UINT64)(&BiosMemoryMapBaseAddress));
//  PrintString("  BiosMemoryMapBaseAddress = ");   
//  PrintValue(BiosMemoryMapBaseAddress);
//  PrintString("\n");

  //
  // Add all EfiConventionalMemory descriptors to the table.  If there are partial pages, then
  // round the start address up to the next page, and round the length down to a page boundry.
  //
  BiosMemoryMap = (BIOS_MEMORY_MAP *)(UINTN)(BiosMemoryMapBaseAddress);
  NumberOfMemoryMapEntries = 0;
  GenMemoryMap (&NumberOfMemoryMapEntries, EfiMemoryDescriptor, BiosMemoryMap);

  //
  // Get information on where the image is in memory
  //

  EFILDRHeader = (EFILDR_HEADER *)(UINTN)(EFILDR_HEADER_ADDRESS);
  EFILDRImage  = (EFILDR_IMAGE *)(UINTN)(EFILDR_HEADER_ADDRESS + sizeof(EFILDR_HEADER));

PrintHeader ('D');

  //
  // Point to the 4th image (Bfv)
  //
    
  EFILDRImage += 3;

  //
  // Decompress the image
  //

  Status = TianoGetInfo (
             NULL, 
             (VOID *)(UINTN)(EFILDR_HEADER_ADDRESS + EFILDRImage->Offset),
             EFILDRImage->Length,
             &DestinationSize, 
             &ScratchSize
             );
  if (EFI_ERROR (Status)) {
    EFI_DEADLOOP();
  }

  Status = TianoDecompress (
             NULL, 
             (VOID *)(UINTN)(EFILDR_HEADER_ADDRESS + EFILDRImage->Offset),
             EFILDRImage->Length,
             (VOID *)(UINTN)EFI_DECOMPRESSED_BUFFER_ADDRESS,
             DestinationSize, 
             (VOID *)(UINTN)((EFI_DECOMPRESSED_BUFFER_ADDRESS + DestinationSize + 0x1000) & 0xfffff000),
             ScratchSize
             );
  if (EFI_ERROR (Status)) {
    EFI_DEADLOOP();
  }

  BfvPageNumber = EFI_SIZE_TO_PAGES (DestinationSize);
  BfvBase = (UINTN) FindSpace (BfvPageNumber, &NumberOfMemoryMapEntries, EfiMemoryDescriptor, EfiRuntimeServicesData, EFI_MEMORY_WB);
  if (BfvBase == 0) {
    EFI_DEADLOOP();
  }
  EfiCommonLibZeroMem ((VOID *)(UINTN)BfvBase, BfvPageNumber * EFI_PAGE_SIZE);
  EfiCommonLibCopyMem ((VOID *)(UINTN)BfvBase, (VOID *)(UINTN)EFI_DECOMPRESSED_BUFFER_ADDRESS, DestinationSize);

PrintHeader ('B');

  //
  // Point to the 2nd image (DxeIpl)
  //
    
  EFILDRImage -= 2;

  //
  // Decompress the image
  //

  Status = TianoGetInfo (
             NULL, 
             (VOID *)(UINTN)(EFILDR_HEADER_ADDRESS + EFILDRImage->Offset),
             EFILDRImage->Length,
             &DestinationSize, 
             &ScratchSize
             );
  if (EFI_ERROR (Status)) {
    EFI_DEADLOOP();
  }

  Status = TianoDecompress (
             NULL, 
             (VOID *)(UINTN)(EFILDR_HEADER_ADDRESS + EFILDRImage->Offset),
             EFILDRImage->Length,
             (VOID *)(UINTN)EFI_DECOMPRESSED_BUFFER_ADDRESS,
             DestinationSize, 
             (VOID *)(UINTN)((EFI_DECOMPRESSED_BUFFER_ADDRESS + DestinationSize + 0x1000) & 0xfffff000),
             ScratchSize
             );
  if (EFI_ERROR (Status)) {
    EFI_DEADLOOP();
  }

  //
  // Load and relocate the EFI PE/COFF Firmware Image 
  //
  Status = EfiLdrPeCoffLoadPeImage (
             (VOID *)(UINTN)(EFI_DECOMPRESSED_BUFFER_ADDRESS), 
             &DxeIplImage, 
             &NumberOfMemoryMapEntries, 
             EfiMemoryDescriptor
             );
  if (EFI_ERROR (Status)) {
    EFI_DEADLOOP();
  }

//  PrintString("Image.NoPages = ");   
//  PrintValue(Image.NoPages);
//  PrintString("\n");

PrintHeader ('C');

  //
  // Point to the 3rd image (DxeMain)
  //
    
  EFILDRImage++;

  //
  // Decompress the image
  //

  Status = TianoGetInfo (
             NULL, 
             (VOID *)(UINTN)(EFILDR_HEADER_ADDRESS + EFILDRImage->Offset),
             EFILDRImage->Length,
             &DestinationSize, 
             &ScratchSize
             );
  if (EFI_ERROR (Status)) {
    EFI_DEADLOOP();
  }

  Status = TianoDecompress (
             NULL, 
             (VOID *)(UINTN)(EFILDR_HEADER_ADDRESS + EFILDRImage->Offset),
             EFILDRImage->Length,
             (VOID *)(UINTN)EFI_DECOMPRESSED_BUFFER_ADDRESS,
             DestinationSize, 
             (VOID *)(UINTN)((EFI_DECOMPRESSED_BUFFER_ADDRESS + DestinationSize + 0x1000) & 0xfffff000),
             ScratchSize
             );
  if (EFI_ERROR (Status)) {
    EFI_DEADLOOP();
  }

  //
  // Load and relocate the EFI PE/COFF Firmware Image 
  //
  Status = EfiLdrPeCoffLoadPeImage (
             (VOID *)(UINTN)(EFI_DECOMPRESSED_BUFFER_ADDRESS), 
             &DxeCoreImage, 
             &NumberOfMemoryMapEntries, 
             EfiMemoryDescriptor
             );
  if (EFI_ERROR (Status)) {
    EFI_DEADLOOP();
  }

PrintHeader ('E');

  //
  // Display the table of memory descriptors.
  //

//  PrintString("\nEFI Memory Descriptors\n");   
/*
  {
  UINTN Index;
  for (Index = 0; Index < NumberOfMemoryMapEntries; Index++) {
    PrintString("Type = ");   
    PrintValue(EfiMemoryDescriptor[Index].Type);
    PrintString("  Start = ");   
    PrintValue((UINT32)(EfiMemoryDescriptor[Index].PhysicalStart));
    PrintString("  NumberOfPages = ");   
    PrintValue((UINT32)(EfiMemoryDescriptor[Index].NumberOfPages));
    PrintString("\n");
  }
  }
*/

  //
  // Jump to EFI Firmware
  //

  if (DxeIplImage.EntryPoint != NULL) {

    Handoff.MemDescCount      = NumberOfMemoryMapEntries;
    Handoff.MemDesc           = EfiMemoryDescriptor;
    Handoff.BfvBase           = (VOID *)(UINTN)BfvBase;
    Handoff.BfvSize           = BfvPageNumber * EFI_PAGE_SIZE;
    Handoff.DxeIplImageBase   = (VOID *)(UINTN)DxeIplImage.ImageBasePage;
    Handoff.DxeIplImageSize   = DxeIplImage.NoPages * EFI_PAGE_SIZE;
    Handoff.DxeCoreImageBase  = (VOID *)(UINTN)DxeCoreImage.ImageBasePage;
    Handoff.DxeCoreImageSize  = DxeCoreImage.NoPages * EFI_PAGE_SIZE;
    Handoff.DxeCoreEntryPoint = (VOID *)(UINTN)DxeCoreImage.EntryPoint;

    EfiMainEntrypoint = (EFI_MAIN_ENTRYPOINT)(UINTN)DxeIplImage.EntryPoint;
    EfiMainEntrypoint (&Handoff);
  }

PrintHeader ('F');

  //
  // There was a problem loading the image, so HALT the system.
  //

  EFI_DEADLOOP();
}

