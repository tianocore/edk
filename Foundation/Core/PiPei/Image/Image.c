/*++

Copyright (c) 2004 - 2009, Intel Corporation                                                         
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
#include "PeiApi.h"
#include "EfiImage.h"
#include EFI_PROTOCOL_DEFINITION (CustomizedDecompress)

#include EFI_PPI_DEFINITION (LoadFile2)
#include EFI_GUID_DEFINITION (PeiPeCoffLoader)
#include EFI_PPI_DEFINITION (SectionExtraction)


EFI_STATUS
EFIAPI
PeiImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINTN   *ReadSize,
  OUT    VOID    *Buffer
  )
/*++

Routine Description:

  Support routine for the PE/COFF Loader that reads a buffer from a PE/COFF file

Arguments:

  FileHandle - The handle to the PE/COFF file
  FileOffset - The offset, in bytes, into the file to read
  ReadSize   - The number of bytes to read from the file starting at FileOffset
  Buffer     - A pointer to the buffer to read the data into.

Returns:

  EFI_SUCCESS - ReadSize bytes of data were read into Buffer from the PE/COFF file starting at FileOffset

--*/  
;

EFI_STATUS
PeiLoadImageLoadImage (
  IN     EFI_PEI_SERVICES             **PeiServices,
  IN     EFI_PEI_FILE_HANDLE          FileHandle,
  OUT    EFI_PHYSICAL_ADDRESS         *ImageAddressArg,  OPTIONAL
  OUT    UINT64                       *ImageSizeArg,     OPTIONAL
  OUT    EFI_PHYSICAL_ADDRESS         *EntryPoint,
  OUT    UINT32                       *AuthenticationState
  )
/*++

Routine Description:

  Routine for loading file image.

Arguments:

  PeiServices          - The PEI core services table.
  FileHandle           - Pointer to the FFS file header of the image.
  ImageAddressArg      - Pointer to PE/TE image.
  ImageSizeArg         - Size of PE/TE image.
  EntryPoint           - Pointer to entry point of specified image file for output.
  AuthenticationState  - Pointer to attestation authentication state of image.

Returns:

  Status - EFI_SUCCESS    - Image is successfully loaded.
           EFI_NOT_FOUND  - Fail to locate necessary PPI
           Others         - Fail to load file.

--*/
;

EFI_STATUS
EFIAPI
PeiLoadImageLoadImageWrapper (
  IN     CONST EFI_PEI_LOAD_FILE_PPI  *This,
  IN     EFI_PEI_FILE_HANDLE          FileHandle,
  OUT    EFI_PHYSICAL_ADDRESS         *ImageAddressArg,  OPTIONAL
  OUT    UINT64                       *ImageSizeArg,     OPTIONAL
  OUT    EFI_PHYSICAL_ADDRESS         *EntryPoint,
  OUT    UINT32                       *AuthenticationState
  )
/*++

Routine Description:

  The wrapper function of PeiLoadImageLoadImage().

Arguments:

  This                 - Pointer to EFI_PEI_LOAD_FILE_PPI.
  PeiServices          - The PEI core services table.
  FileHandle           - Pointer to the FFS file header of the image.
  ImageAddressArg      - Pointer to PE/TE image.
  ImageSizeArg         - Size of PE/TE image.
  EntryPoint           - Pointer to entry point of specified image file for output.
  AuthenticationState  - Pointer to attestation authentication state of image.

Returns:

  EFI_STATUS.
  
--*/ 
;


EFI_STATUS
EFIAPI
PeCoffLoaderGetEntryPoint (
  IN  VOID  *Pe32Data,
  OUT VOID  **EntryPoint
  )
/*++

Routine Description:

  Routine for get PE image entry point.

Arguments:

  Pe32Data    - Pointer to PE image.
  EntryPoint  - Pointer to entry point of PE image.

Returns:

  EFI_SUCCESS.
  
--*/    
;

EFI_IMAGE_NT_HEADERS*
EFIAPI
GetPeHeader (
  IN  EFI_PEI_SERVICES         **PeiServices,
  IN  VOID                     *Pe32Data
  )
/*++

Routine Description:

  Routine for get PE image header.

Arguments:
  PeiServices - The PEI core services table.
  Pe32Data    - Pointer to PE image.

Returns:

  Pointer to PE header.
  
--*/  
;

static EFI_PEI_LOAD_FILE_PPI   mPeiLoadImagePpi = {
  PeiLoadImageLoadImageWrapper
};


EFI_PEI_PPI_DESCRIPTOR     gPpiLoadFilePpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiLoadFile2PpiGuid,
  &mPeiLoadImagePpi
};

EFI_STATUS
EFIAPI
PeiImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINTN   *ReadSize,
  OUT    VOID    *Buffer
  )
/*++

Routine Description:

  Support routine for the PE/COFF Loader that reads a buffer from a PE/COFF file

Arguments:

  FileHandle - The handle to the PE/COFF file
  FileOffset - The offset, in bytes, into the file to read
  ReadSize   - The number of bytes to read from the file starting at FileOffset
  Buffer     - A pointer to the buffer to read the data into.

Returns:

  EFI_SUCCESS - ReadSize bytes of data were read into Buffer from the PE/COFF file starting at FileOffset

--*/
{
  CHAR8 *Destination8;
  CHAR8 *Source8;
  UINTN Length;

  Destination8  = Buffer;
  Source8       = (CHAR8 *) ((UINTN) FileHandle + FileOffset);
  Length        = *ReadSize;
  while (Length--) {
    *(Destination8++) = *(Source8++);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetImageReadFunction (
  IN      EFI_PEI_SERVICES                      **PeiServices,
  IN      EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
/*++

Routine Description:

  Support routine to return the Image Read

Arguments:

  PeiServices   - PEI Services Table

  ImageContext  - The context of the image being loaded

Returns:

  EFI_SUCCESS - If Image function location is found

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  MemoryBuffer;

  Status = (*PeiServices)->AllocatePages (
                            PeiServices,
                            EfiBootServicesData,
                            0x400 / EFI_PAGE_SIZE + 1,
                            &MemoryBuffer
                            );
  ASSERT_PEI_ERROR (PeiServices, Status);

  (*PeiServices)->CopyMem (
                    (VOID *) (UINTN) MemoryBuffer,
                    (VOID *) (UINTN) PeiImageRead,
                    0x400
                    );

  ImageContext->ImageRead = (EFI_PEI_PE_COFF_LOADER_READ_FILE) (UINTN) MemoryBuffer;

  return Status;
}

EFI_STATUS
PeiLoadImageLoadImage (
  IN     EFI_PEI_SERVICES             **PeiServices,
  IN     EFI_PEI_FILE_HANDLE          FileHandle,
  OUT    EFI_PHYSICAL_ADDRESS         *ImageAddressArg,  OPTIONAL
  OUT    UINT64                       *ImageSizeArg,     OPTIONAL
  OUT    EFI_PHYSICAL_ADDRESS         *EntryPoint,
  OUT    UINT32                       *AuthenticationState
  )
/*++

Routine Description:

  Routine for loading file image.

Arguments:

  PeiServices          - The PEI core services table.
  FileHandle           - Pointer to the FFS file header of the image.
  ImageAddressArg      - Pointer to PE/TE image.
  ImageSizeArg         - Size of PE/TE image.
  EntryPoint           - Pointer to entry point of specified image file for output.
  AuthenticationState  - Pointer to attestation authentication state of image.

Returns:

  Status - EFI_SUCCESS    - Image is successfully loaded.
           EFI_NOT_FOUND  - Fail to locate necessary PPI
           Others         - Fail to load file.

--*/
{
  EFI_STATUS                        Status;
  PEI_CORE_INSTANCE                 *Private;
  VOID                              *Pe32Data;
  EFI_TE_IMAGE_HEADER               *TEImageHeader;
  EFI_PHYSICAL_ADDRESS              ImageAddress;
  UINT64                            ImageSize;
  EFI_IMAGE_NT_HEADERS              *PeHdr;
  EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT ImageContext;
  EFI_PEI_PE_COFF_LOADER_PROTOCOL   *PeCoffLoader;
  
#ifdef EFI_NT_EMULATOR
  EFI_PEI_PPI_DESCRIPTOR      *PpiDescriptor;
  NT_PEI_LOAD_FILE_PPI        *PeiNtService;
#else
  VOID                        *EntryPointPtr;
#endif

  Private = PEI_CORE_INSTANCE_FROM_PS_THIS (PeiServices);

  *EntryPoint          = 0;
  *AuthenticationState = 0;
  ImageAddress         = 0;
  ImageSize            = 0;
  PeHdr                = NULL;

  //
  // Try to find a TE or PE32 section.
  //
  Status = PeiFfsFindSectionData (
             PeiServices,
             EFI_SECTION_TE,
             FileHandle,
             (VOID **)&TEImageHeader
             );
  Pe32Data = TEImageHeader;
  if (EFI_ERROR (Status)) {
    Status = PeiFfsFindSectionData (
              PeiServices,
              EFI_SECTION_PE32,
              FileHandle,
              &Pe32Data
              );
  }
  
  if (EFI_ERROR (Status)) {
    //
    // NO image types we support so exit.
    //
    return Status;
  }

  //
  // Check image header machine type. If not supported, skip it.
  //
  if (TEImageHeader != NULL) {
    if (!EFI_IMAGE_MACHINE_TYPE_SUPPORTED (TEImageHeader->Machine) &&
        !EFI_IMAGE_MACHINE_CROSS_TYPE_SUPPORTED (TEImageHeader->Machine)) {
      return EFI_UNSUPPORTED;
    }
  } else {
    //
    // Get PE32 Header to check machine type.
    //
    PeHdr = GetPeHeader(PeiServices, Pe32Data);
    if (!EFI_IMAGE_MACHINE_TYPE_SUPPORTED (PeHdr->FileHeader.Machine) &&
        !EFI_IMAGE_MACHINE_CROSS_TYPE_SUPPORTED (PeHdr->FileHeader.Machine)) {
      return EFI_UNSUPPORTED;
    }
  }

  if (Private->PeiMemoryInstalled && 
      (Private->HobList.HandoffInformationTable->BootMode != BOOT_ON_S3_RESUME)) {
    //
    // Load, relocate, and run image in memory
    //
    ZeroMem (&ImageContext, sizeof (ImageContext));
    ImageContext.Handle = Pe32Data;
    if (Private->ImageReadFile == NULL) {
      GetImageReadFunction (PeiServices, &ImageContext);
      Private->ImageReadFile = ImageContext.ImageRead;
    } else {
      ImageContext.ImageRead = Private->ImageReadFile;
    }
    PeCoffLoader = Private->PeCoffLoader;
    Status = PeCoffLoader->GetImageInfo (PeCoffLoader, &ImageContext);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // Allocate Memory for the image
    //
    Status = PeiAllocatePages (
              PeiServices, 
              EfiBootServicesData, 
              EFI_SIZE_TO_PAGES ((UINT32) ImageContext.ImageSize), 
              &ImageContext.ImageAddress
              );
    ASSERT_PEI_ERROR (PeiServices,Status);

    if (ImageContext.IsTeImage &&
        TEImageHeader->Machine == EFI_IMAGE_MACHINE_IA64) {
      ImageContext.ImageAddress = ImageContext.ImageAddress + 
                                    TEImageHeader->StrippedSize -
                                    sizeof (EFI_TE_IMAGE_HEADER);
    }

    //
    // Load the image to our new buffer
    //
    Status = PeCoffLoader->LoadImage (PeCoffLoader, &ImageContext);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Relocate the image in our new buffer
    //
    Status = PeCoffLoader->RelocateImage (PeCoffLoader, &ImageContext);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Flush the instruction cache so the image data is written before we execute it
    //
    InvalidateInstructionCacheRange ((VOID *)(UINTN)ImageContext.ImageAddress, (UINTN)ImageContext.ImageSize);

    ImageAddress  = ImageContext.ImageAddress;
    ImageSize     = ImageContext.ImageSize;
    *EntryPoint   = ImageContext.EntryPoint;
  }
#ifndef  EFI_NT_EMULATOR 
  else if (TEImageHeader != NULL) {
    //
    // Get entry point from the TE image header
    //
    ImageAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)TEImageHeader;
    ImageSize    = 0;
    *EntryPoint  = (EFI_PHYSICAL_ADDRESS)((UINTN) TEImageHeader + sizeof (EFI_TE_IMAGE_HEADER) +
                    TEImageHeader->AddressOfEntryPoint - TEImageHeader->StrippedSize);
  } else {
    //
    // Retrieve the entry point from the PE/COFF image header
    //
    ImageAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)Pe32Data;
    ImageSize    = 0;
    Status = PeCoffLoaderGetEntryPoint (Pe32Data, &EntryPointPtr);
    *EntryPoint = (EFI_PHYSICAL_ADDRESS)(UINTN)EntryPointPtr;
    if (EFI_ERROR (Status)) {
      return EFI_NOT_FOUND;
    }
  }
#else
  else {
    if (TEImageHeader == NULL) {
      PeHdr = GetPeHeader(PeiServices,Pe32Data);
    }
    
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
                            EntryPoint
                            );
    
    if (EFI_ERROR (Status)) {
      if (TEImageHeader != NULL) {
        *EntryPoint = (EFI_PHYSICAL_ADDRESS)((UINTN) TEImageHeader + sizeof (EFI_TE_IMAGE_HEADER) +
                      TEImageHeader->AddressOfEntryPoint - TEImageHeader->StrippedSize);
      } else {
        *EntryPoint = (EFI_PHYSICAL_ADDRESS) ((UINTN) Pe32Data + (UINTN) (PeHdr->OptionalHeader.AddressOfEntryPoint & 0x0ffffffff));
      }
    }
  }  
#endif


  if (ImageAddressArg != NULL) {
    *ImageAddressArg = ImageAddress;
  }

  if (ImageSizeArg != NULL) {
    *ImageSizeArg = ImageSize;
  }
  
  //
  // Print debug message: Loading PEIM at 0x12345678 EntryPoint=0x12345688 Driver.efi
  //
  PEI_DEBUG ((PeiServices,EFI_D_INFO | EFI_D_LOAD, "Loading PEIM at 0x%08x EntryPoint=0x%08x ", Pe32Data, *EntryPoint));
  PEI_DEBUG_CODE (
  {
    EFI_IMAGE_DATA_DIRECTORY         *DirectoryEntry;
    EFI_IMAGE_DEBUG_DIRECTORY_ENTRY  *DebugEntry;
    UINTN DirCount;
    UINTN Index;
    UINTN Index1;
    BOOLEAN FileNameFound;
    CHAR8 *AsciiString;
    CHAR8 AsciiBuffer[512];
    VOID *CodeViewEntryPointer;
    INTN TEImageAdjust;
    EFI_IMAGE_DOS_HEADER  *DosHeader;
    EFI_IMAGE_NT_HEADERS  *PeHeader;
  
    DosHeader = (EFI_IMAGE_DOS_HEADER *)Pe32Data;
      if (DosHeader->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
        //
        // DOS image header is present, so read the PE header after the DOS image header
        //
        PeHeader = (EFI_IMAGE_NT_HEADERS *) ((UINTN) Pe32Data + (UINTN) ((DosHeader->e_lfanew) & 0x0ffff));
      } else {
        //
        // DOS image header is not present, so PE header is at the image base
        //
        PeHeader = (EFI_IMAGE_NT_HEADERS *) Pe32Data;
      }
  
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
        if (PeHeader->OptionalHeader.NumberOfRvaAndSizes > EFI_IMAGE_DIRECTORY_ENTRY_DEBUG) {
          DirectoryEntry = (EFI_IMAGE_DATA_DIRECTORY *) &(PeHeader->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_DEBUG]);
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
                  PEI_DEBUG ((PeiServices,EFI_D_INFO | EFI_D_LOAD, "%a.efi", AsciiBuffer));
                  break;
                }
              }
            }
          }
        }
      }
   }
  )

  PEI_DEBUG ((PeiServices,EFI_D_INFO | EFI_D_LOAD, "\n"));

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
PeiLoadImageLoadImageWrapper (
  IN     CONST EFI_PEI_LOAD_FILE_PPI  *This,
  IN     EFI_PEI_FILE_HANDLE          FileHandle,
  OUT    EFI_PHYSICAL_ADDRESS         *ImageAddressArg,  OPTIONAL
  OUT    UINT64                       *ImageSizeArg,     OPTIONAL
  OUT    EFI_PHYSICAL_ADDRESS         *EntryPoint,
  OUT    UINT32                       *AuthenticationState
  )
/*++

Routine Description:

  The wrapper function of PeiLoadImageLoadImage().

Arguments:

  This                 - Pointer to EFI_PEI_LOAD_FILE_PPI.
  PeiServices          - The PEI core services table.
  FileHandle           - Pointer to the FFS file header of the image.
  ImageAddressArg      - Pointer to PE/TE image.
  ImageSizeArg         - Size of PE/TE image.
  EntryPoint           - Pointer to entry point of specified image file for output.
  AuthenticationState  - Pointer to attestation authentication state of image.

Returns:

  EFI_STATUS.
  
--*/      
{
  return PeiLoadImageLoadImage (
           GetPeiServicesTablePointer (),
           FileHandle,
           ImageAddressArg,
           ImageSizeArg,
           EntryPoint,
           AuthenticationState
           );
}

EFI_STATUS
PeiLoadImage (
  IN     EFI_PEI_SERVICES             **PeiServices,
  IN     EFI_PEI_FILE_HANDLE          FileHandle,
  OUT    EFI_PHYSICAL_ADDRESS         *EntryPoint,
  OUT    UINT32                       *AuthenticationState
  )
/*++

Routine Description:

  Routine for load image file.

Arguments:

  PeiServices          - The PEI core services table.
  FileHandle           - Pointer to the FFS file header of the image.
  EntryPoint           - Pointer to entry point of specified image file for output.
  AuthenticationState  - Pointer to attestation authentication state of image.

Returns:

  Status - EFI_SUCCESS    - Image is successfully loaded.
           EFI_NOT_FOUND  - Fail to locate necessary PPI
           Others         - Fail to load file.
  
--*/    
{
  EFI_STATUS              PpiStatus;
  EFI_STATUS              Status;
  UINTN                   Index;
  EFI_PEI_LOAD_FILE_PPI   *LoadFile;
  EFI_PHYSICAL_ADDRESS    ImageAddress;
  UINT64                  ImageSize;
  UINT16                  Magic;
  UINT16                  Machine;
  EFI_IMAGE_NT_HEADERS    *PeHdr;
  EFI_TE_IMAGE_HEADER     *TeHdr;
  
  //
  // If any instances of PEI_LOAD_FILE_PPI are installed, they are called.
  // one at a time, until one reports EFI_SUCCESS.
  //
  Index = 0;
  do {
    PpiStatus = PeiLocatePpi (
                  PeiServices,
                  &gEfiLoadFile2PpiGuid,
                  Index,
                  NULL,
                  (VOID **)&LoadFile
                  );
    if (!EFI_ERROR (PpiStatus)) {
      Status = LoadFile->LoadFile (
                          LoadFile, 
                          FileHandle, 
                          &ImageAddress, 
                          &ImageSize,
                          EntryPoint,
                          AuthenticationState
                          );
      if (!EFI_ERROR (Status)) {
        //
        // Only support image machine type that is the same as PEI core.
        // Get the image's machine type to check if supported.
        //
        Magic = *(UINT16 *)(UINTN) ImageAddress;
        if (Magic == EFI_TE_IMAGE_HEADER_SIGNATURE) {
          TeHdr   = (EFI_TE_IMAGE_HEADER *)(UINTN) ImageAddress;
          Machine = TeHdr->Machine;
        } else {
          if (Magic == EFI_IMAGE_DOS_SIGNATURE) {
            PeHdr = (EFI_IMAGE_NT_HEADERS *)((UINTN) ImageAddress + (UINTN) ((((EFI_IMAGE_DOS_HEADER *)(UINTN) ImageAddress)->e_lfanew) & 0x0ffff));
          } else {
            PeHdr = (EFI_IMAGE_NT_HEADERS *)(UINTN) ImageAddress;
          }
          Machine = PeHdr->FileHeader.Machine;
        }
        if (Machine == EFI_IMAGE_MACHINE_TYPE) {
          return EFI_SUCCESS;
        } else {
          return EFI_UNSUPPORTED;
        }
      }
    }
    Index++;
  } while (!EFI_ERROR (PpiStatus));

  return PpiStatus;
}


VOID
InitializeImageServices (
  IN  PEI_CORE_INSTANCE   *PrivateData,
  IN  PEI_CORE_INSTANCE   *OldCoreData
  )
/*++

Routine Description:

  Regitser PeCoffLoader to PeiCore PrivateData. And install
  Pei Load File PPI.

Arguments:

  PrivateData    - Pointer to PEI_CORE_INSTANCE.
  OldCoreData    - Pointer to PEI_CORE_INSTANCE.

Returns:

  NONE.
  
--*/      
{
  EFI_PEI_PE_COFF_LOADER_PROTOCOL    *PeiEfiPeiPeCoffLoader;

  InstallEfiPeiPeCoffLoader (&PrivateData->PS, &PeiEfiPeiPeCoffLoader, NULL);
  PrivateData->PeCoffLoader = PeiEfiPeiPeCoffLoader;
  
  if (OldCoreData == NULL) {
    //
    // The first time we are XIP (running from FLASH). We need to remember the
    // FLASH address so we can reinstall the memory version that runs faster
    //
    PrivateData->XipLoadFile = &gPpiLoadFilePpiList;
    PeiInstallPpi (&PrivateData->PS,  PrivateData->XipLoadFile);
  } else {
    //
    // 2nd time we are running from memory so replace the XIP version with the 
    // new memory version. 
    //
    PeiReInstallPpi (&PrivateData->PS, PrivateData->XipLoadFile, &gPpiLoadFilePpiList); 
  }
}


EFI_STATUS
EFIAPI
PeCoffLoaderGetEntryPoint (
  IN  VOID  *Pe32Data,
  OUT VOID  **EntryPoint
  )
/*++

Routine Description:

  Routine for get PE image entry point.

Arguments:

  Pe32Data    - Pointer to PE image.
  EntryPoint  - Pointer to entry point of PE image.

Returns:

  EFI_SUCCESS.
  
--*/    
{
  EFI_IMAGE_NT_HEADERS     *PeHdr;
  EFI_PEI_SERVICES         **PeiServices;
  
  PeHdr = NULL;
  PeiServices = GetPeiServicesTablePointer();
  
  PEI_ASSERT (PeiServices, (Pe32Data != NULL));
  PEI_ASSERT (PeiServices,(EntryPoint != NULL));

  PeHdr = GetPeHeader(PeiServices, Pe32Data);

  //
  // Calculate the entry point relative to the start of the image. 
  // AddressOfEntryPoint is common for PE32 & PE32+
  //
  *EntryPoint = (VOID *) ((UINTN) Pe32Data + (UINTN) (PeHdr->OptionalHeader.AddressOfEntryPoint & 0x0ffffffff));
  return EFI_SUCCESS;
}

EFI_IMAGE_NT_HEADERS*
EFIAPI
GetPeHeader (
  IN  EFI_PEI_SERVICES         **PeiServices,
  IN  VOID                     *Pe32Data
  )
/*++

Routine Description:

  Routine for get PE image header.

Arguments:
  PeiServices - The PEI core services table.
  Pe32Data    - Pointer to PE image.

Returns:

  Pointer to PE header.
  
--*/  
{
  EFI_IMAGE_DOS_HEADER           *DosHeader;
  EFI_IMAGE_NT_HEADERS           *PeHdr;
  
  PeHdr       = NULL;
  PEI_ASSERT (PeiServices, (Pe32Data != NULL));

  DosHeader = (EFI_IMAGE_DOS_HEADER *)Pe32Data;
  if (DosHeader->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    //
    // DOS image header is present, so read the PE header after the DOS image header.
    //
    PeHdr = (EFI_IMAGE_NT_HEADERS *) ((UINTN) Pe32Data + (UINTN) ((((EFI_IMAGE_DOS_HEADER *) Pe32Data)->e_lfanew) & 0x0ffff));
  } else {
    //
    // DOS image header is not present, so PE header is at the image base.
    //
    PeHdr = (EFI_IMAGE_NT_HEADERS *) Pe32Data;
  }
  return PeHdr;
}

