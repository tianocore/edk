/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Image.h

Abstract:

Revision History

--*/
#include "Tiano.h"
#include EFI_PROTOCOL_PRODUCER (LoadedImage)
#include EFI_PROTOCOL_PRODUCER (LoadPe32Image)
#include EFI_PROTOCOL_CONSUMER (Ebc)
#include EFI_PROTOCOL_CONSUMER (SimpleFileSystem)
#include EFI_PROTOCOL_CONSUMER (FileInfo)
#include EFI_PROTOCOL_CONSUMER (LoadFile)
#include EFI_PROTOCOL_CONSUMER (FirmwareVolume)

#include "LinkedList.h"
#include "DxeCore.h"

#define LOADED_IMAGE_PRIVATE_DATA_SIGNATURE   EFI_SIGNATURE_32('l','d','r','i')

typedef struct {
    UINTN                       Signature;
    EFI_HANDLE                  Handle;         // Image handle
    UINTN                       Type;           // Image type

    BOOLEAN                     Started;        // If entrypoint has been called

    EFI_IMAGE_ENTRY_POINT       EntryPoint;     // The image's entry point
    EFI_LOADED_IMAGE_PROTOCOL   Info;           // loaded image protocol

    EFI_PHYSICAL_ADDRESS        ImageBasePage;  // Location in memory
    UINTN                       NumberOfPages;  // Number of pages 

    CHAR8                       *FixupData;     // Original fixup data

    EFI_TPL                     Tpl;            // Tpl of started image
    EFI_STATUS                  Status;         // Status returned by started image

    UINTN                       ExitDataSize;   // Size of ExitData from started image
    VOID                        *ExitData;      // Pointer to exit data from started image
    VOID                        *JumpContext;   // Pointer to buffer for context save/retore
    UINT16                      Machine;        // Machine type from PE image

    EFI_EBC_PROTOCOL            *Ebc;           // EBC Protocol pointer

    BOOLEAN                     RuntimeFixupValid; // True if RT image needs fixup
    VOID                        *RuntimeFixup;     // Copy of fixup data;
    EFI_LIST_ENTRY              Link;              // List of RT LOADED_IMAGE_PRIVATE_DATA

    EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT  ImageContext;     // PeCoffLoader ImageContext

} LOADED_IMAGE_PRIVATE_DATA;

#define LOADED_IMAGE_PRIVATE_DATA_FROM_THIS(a) \
          CR(a, LOADED_IMAGE_PRIVATE_DATA, Info, LOADED_IMAGE_PRIVATE_DATA_SIGNATURE)



#define LOAD_PE32_IMAGE_PRIVATE_DATA_SIGNATURE  EFI_SIGNATURE_32('l','p','e','i') 

typedef struct {
    UINTN                       Signature;
    EFI_HANDLE                  Handle;         // Image handle
    EFI_PE32_IMAGE_PROTOCOL     Pe32Image;
} LOAD_PE32_IMAGE_PRIVATE_DATA;

#define LOAD_PE32_IMAGE_PRIVATE_DATA_FROM_THIS(a) \
          CR(a, LOAD_PE32_IMAGE_PRIVATE_DATA, Pe32Image, LOAD_PE32_IMAGE_PRIVATE_DATA_SIGNATURE)



//
// Private Data Types
//
#define IMAGE_FILE_HANDLE_SIGNATURE       EFI_SIGNATURE_32('i','m','g','f')
typedef struct {
  UINTN               Signature;
  BOOLEAN             FreeBuffer;
  VOID                *Source;
  UINTN               SourceSize;
} IMAGE_FILE_HANDLE;


//
// Abstractions for reading image contents
//

EFI_STATUS
CoreOpenImageFile (
  IN BOOLEAN                        BootPolicy,
  IN VOID                           *SourceBuffer   OPTIONAL,
  IN UINTN                          SourceSize,
  IN OUT EFI_DEVICE_PATH_PROTOCOL   *FilePath,
  OUT EFI_HANDLE                    *DeviceHandle,
  IN IMAGE_FILE_HANDLE              *ImageFileHandle,
  OUT UINT32                        *AuthenticationStatus
  );


EFI_STATUS
EFIAPI
CoreReadImageFile (
  IN     VOID     *UserHandle,
  IN     UINTN    Offset,
  IN OUT UINTN    *ReadSize,
  OUT     VOID    *Buffer
  );

VOID
EFIAPI
CoreCloseImageFile (
  IN IMAGE_FILE_HANDLE *ImageFileHandle
  );

//
// Image processing worker functions
//
EFI_STATUS
CoreDevicePathToInterface (
  IN EFI_GUID                     *Protocol,
  IN OUT EFI_DEVICE_PATH_PROTOCOL **FilePath,
  OUT VOID                        **Interface,
  OUT EFI_HANDLE                  *Handle
  );

STATIC
EFI_STATUS
CoreLoadPeImage (
  IN  VOID                       *Pe32Handle,
  IN  LOADED_IMAGE_PRIVATE_DATA  *Image,
  IN  EFI_PHYSICAL_ADDRESS       DstBuffer   OPTIONAL,
  OUT EFI_PHYSICAL_ADDRESS       *EntryPoint  OPTIONAL,
  IN  UINT32                     Attribute
  );

LOADED_IMAGE_PRIVATE_DATA *
CoreLoadedImageInfo (
  IN EFI_HANDLE  ImageHandle
  );

VOID
CoreUnloadAndCloseImage (
  IN LOADED_IMAGE_PRIVATE_DATA  *Image,
  IN BOOLEAN                    FreePage
  );


//
// Exported Image functions
//

EFI_STATUS
EFIAPI
CoreLoadImageEx (
  IN EFI_PE32_IMAGE_PROTOCOL           *This,
  IN  EFI_HANDLE                       ParentImageHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL         *FilePath,
  IN  VOID                             *SourceBuffer       OPTIONAL,
  IN  UINTN                            SourceSize,
  IN  EFI_PHYSICAL_ADDRESS             DstBuffer           OPTIONAL,
  OUT UINTN                            *NumberOfPages      OPTIONAL,
  OUT EFI_HANDLE                       *ImageHandle,
  OUT EFI_PHYSICAL_ADDRESS             *EntryPoint         OPTIONAL,
  IN  UINT32                           Attribute
  );

EFI_STATUS
EFIAPI
CoreUnloadImageEx (
  IN EFI_PE32_IMAGE_PROTOCOL            *This,
  IN EFI_HANDLE                         ImageHandle
  );
