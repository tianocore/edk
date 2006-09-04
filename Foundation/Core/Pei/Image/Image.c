/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Image.c

Abstract:

  Pei Core Load Image Support

--*/

#include "Tiano.h"
#include "PeiCore.h"
#include "PeiLib.h"
#include EFI_PPI_DEFINITION (LoadFile)

EFI_STATUS
PeiLoadImage (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN EFI_FFS_FILE_HEADER      *PeimFileHeader,
  OUT VOID                    **EntryPoint
  )
/*++

Routine Description:

  Routine for loading file image.

Arguments:

  PeiServices     - The PEI core services table.
  PeimFileHeader  - Pointer to the FFS file header of the image.
  EntryPoint      - Pointer to entry point of specified image file for output.

Returns:

  Status - EFI_SUCCESS    - Image is successfully loaded.
           EFI_NOT_FOUND  - Fail to locate necessary PPI
           Others         - Fail to load file.

--*/
{
  EFI_STATUS                  Status;
  VOID                        *Pe32Data;
  EFI_IMAGE_NT_HEADERS        *PeHdr;
  EFI_PEI_FV_FILE_LOADER_PPI  *FvLoadFilePpi;
#ifdef EFI_NT_EMULATOR
  EFI_PEI_PPI_DESCRIPTOR      *PpiDescriptor;
  NT_PEI_LOAD_FILE_PPI        *PeiNtService;
#endif
  EFI_PHYSICAL_ADDRESS        ImageAddress;
  UINT64                      ImageSize;
  EFI_PHYSICAL_ADDRESS        ImageEntryPoint;
  EFI_TE_IMAGE_HEADER         *TEImageHeader;

  PEI_DEBUG_CODE (ImageAddress  = 0;)
  
  *EntryPoint   = NULL;
  TEImageHeader = NULL;
  PeHdr         = NULL;
  //
  // Try to find a PE32 section.
  //
  Status = PeiFfsFindSectionData (
            PeiServices,
            EFI_SECTION_PE32,
            PeimFileHeader,
            &Pe32Data
            );
  //
  // If we didn't find a PE32 section, try to find a TE section.
  //
  if (Status != EFI_SUCCESS) {
    Status = PeiFfsFindSectionData (
              PeiServices,
              EFI_SECTION_TE,
              PeimFileHeader,
              (VOID **) &TEImageHeader
              );
    Pe32Data = (VOID *) TEImageHeader;
  }
  if (Status == EFI_SUCCESS) {
    if (TEImageHeader == NULL) {
      if (((EFI_IMAGE_DOS_HEADER *) Pe32Data)->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
        //
        // DOS image header is present, so read the PE header after the DOS image header
        //
        PeHdr = (EFI_IMAGE_NT_HEADERS *) ((UINTN) Pe32Data + (UINTN) ((((EFI_IMAGE_DOS_HEADER *) Pe32Data)->e_lfanew) & 0x0ffff));
      } else {
        //
        // DOS image header is not present, so PE header is at the image base
        //
        PeHdr = (EFI_IMAGE_NT_HEADERS *) Pe32Data;
      }
    }

#ifdef EFI_NT_EMULATOR
    Status = (**PeiServices).LocatePpi (
                              PeiServices,
                              &gNtPeiLoadFileGuid,
                              0,
                              &PpiDescriptor,
                              &PeiNtService
                              );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = PeiNtService->PeiLoadFileService (
                            Pe32Data,
                            &ImageAddress,
                            &ImageSize,
                            (UINT64 *) EntryPoint
                            );

    if (EFI_ERROR (Status)) {
      if (TEImageHeader != NULL) {
      	*EntryPoint = (VOID *)((UINTN) TEImageHeader + sizeof (EFI_TE_IMAGE_HEADER) +
                      TEImageHeader->AddressOfEntryPoint - TEImageHeader->StrippedSize);
      } else {
        *EntryPoint = (VOID *) ((UINTN) Pe32Data + (UINTN) (PeHdr->OptionalHeader.AddressOfEntryPoint & 0x0ffffffff));
      }
    }

#else
    ImageAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) Pe32Data;
    if (TEImageHeader != NULL) {
      *EntryPoint = (VOID *)((UINTN) TEImageHeader + sizeof (EFI_TE_IMAGE_HEADER) +
                    TEImageHeader->AddressOfEntryPoint - TEImageHeader->StrippedSize);
    } else {
      *EntryPoint = (VOID *) ((UINTN) Pe32Data + (UINTN) (PeHdr->OptionalHeader.AddressOfEntryPoint & 0x0ffffffff));
    }
#endif

  } else {
    //
    // There was not a PE32 section, so assume that it's a Compressed section
    // and use the LoadFile
    //
    Status = PeiLocatePpi (
              PeiServices,
              &gPeiFvFileLoaderPpiGuid,
              0,
              NULL,
              &FvLoadFilePpi
              );

    if (!EFI_ERROR (Status)) {
      Status = FvLoadFilePpi->FvLoadFile (
                                FvLoadFilePpi,
                                PeimFileHeader,
                                &ImageAddress,
                                &ImageSize,
                                &ImageEntryPoint
                                );

      if (!EFI_ERROR (Status)) {
        *EntryPoint = (VOID *) ((UINTN) ImageEntryPoint);
        PEI_DEBUG_CODE (
          if (((EFI_IMAGE_DOS_HEADER *) (UINTN) ImageAddress)->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
            //
            // DOS image header is present, so read the PE header after the DOS image header
            //
            PeHdr = (EFI_IMAGE_NT_HEADERS *) ((UINTN) ImageAddress + (UINTN) ((((EFI_IMAGE_DOS_HEADER *) (UINTN) ImageAddress)->e_lfanew) & 0x0ffff));
          } else {
            //
            // DOS image header is not present, so PE header is at the image base
            //
            PeHdr = (EFI_IMAGE_NT_HEADERS *) (UINTN) ImageAddress;
          }
        )
      }
    }
  }

  PEI_DEBUG_CODE (
  {
    EFI_IMAGE_DATA_DIRECTORY * DirectoryEntry;
    EFI_IMAGE_DEBUG_DIRECTORY_ENTRY * DebugEntry;
    UINTN DirCount;
    UINTN Index;
    UINTN Index1;
    BOOLEAN FileNameFound;
    CHAR8 *AsciiString;
    CHAR8 AsciiBuffer[512];
    VOID *CodeViewEntryPointer;
    INTN TEImageAdjust;

    if (Status == EFI_SUCCESS) {
      //
      // Print debug message: Loading PEIM at 0x12345678 EntryPoint=0x12345688 Driver.efi
      //
      PEI_DEBUG ((PeiServices, EFI_D_INFO | EFI_D_LOAD, "Loading PEIM at 0x%08x EntryPoint=0x%08x ", (UINTN) ImageAddress, *EntryPoint));
  
      //
      // Find the codeview info in the image and display the file name
      // being loaded.
      //
      // Per the PE/COFF spec, you can't assume that a given data directory
      // is present in the image. You have to check the NumberOfRvaAndSizes in
      // the optional header to verify a desired directory entry is there.
      //
      DebugEntry      = NULL;
      DirectoryEntry  = NULL;
      TEImageAdjust   = 0;
      if (TEImageHeader == NULL) {
        if (PeHdr->OptionalHeader.NumberOfRvaAndSizes > EFI_IMAGE_DIRECTORY_ENTRY_DEBUG) {
          DirectoryEntry = (EFI_IMAGE_DATA_DIRECTORY *) &(PeHdr->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_DEBUG]);
          DebugEntry = (EFI_IMAGE_DEBUG_DIRECTORY_ENTRY *) ((UINTN) ImageAddress + DirectoryEntry->VirtualAddress);
        }
      } else {
        if (TEImageHeader->DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress != 0) {
          DirectoryEntry  = &TEImageHeader->DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG];
          TEImageAdjust   = sizeof (EFI_TE_IMAGE_HEADER) - TEImageHeader->StrippedSize;
          DebugEntry = (EFI_IMAGE_DEBUG_DIRECTORY_ENTRY *)((UINTN) TEImageHeader +
                        TEImageHeader->DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress +
                        TEImageAdjust);
        }
      }
  
      if (DebugEntry != NULL && DirectoryEntry != NULL) {
        for (DirCount = 0; DirCount < DirectoryEntry->Size; DirCount++, DebugEntry++) {
          if (DebugEntry->Type == EFI_IMAGE_DEBUG_TYPE_CODEVIEW) {
            if (DebugEntry->SizeOfData > 0) {
              CodeViewEntryPointer = (VOID *) ((UINTN) DebugEntry->RVA + (UINTN) ImageAddress + (UINTN)TEImageAdjust);
              switch (* (UINT32 *) CodeViewEntryPointer) {
                case CODEVIEW_SIGNATURE_NB10:
                  AsciiString = (CHAR8 *) CodeViewEntryPointer + sizeof (EFI_IMAGE_DEBUG_CODEVIEW_NB10_ENTRY);
                  break;
  
                case CODEVIEW_SIGNATURE_RSDS:
                  AsciiString = (CHAR8 *) CodeViewEntryPointer + sizeof (EFI_IMAGE_DEBUG_CODEVIEW_RSDS_ENTRY);
                  break;
  
                default:
                  AsciiString = NULL;
                  break;
              }
              if (AsciiString != NULL) {
                FileNameFound = FALSE;
                for (Index = 0, Index1 = 0; AsciiString[Index] != 0; Index++) {
                  if (AsciiString[Index] == '\\') {
                    Index1 = Index;
                    FileNameFound = TRUE;
                  }
                }
  
                if (FileNameFound) {
                  for (Index = Index1 + 1; AsciiString[Index] != '.'; Index++) {
                    AsciiBuffer[Index - (Index1 + 1)] = AsciiString[Index];
                  }
                  AsciiBuffer[Index - (Index1 + 1)] = 0;
                  PEI_DEBUG ((PeiServices, EFI_D_INFO | EFI_D_LOAD, "%a.efi", AsciiBuffer));
                  break;
                }
              }
            }
          }
        }
      }
      PEI_DEBUG ((PeiServices, EFI_D_INFO | EFI_D_LOAD, "\n"));
    }
  }
  )

  return Status;
}
