/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PiDxeLoad.c

Abstract:

  Last PEIM.
  Responsibility of this module is to load the DXE Core from a Firmware Volume.

--*/

#include "Tiano.h"
#include "Pei.h"
#include "PiDxeIpl.h"
#include "EfiHobLib.h"
#include "PeiLib.h"
#include EFI_PPI_DEFINITION (RecoveryModule)
#include EFI_PPI_DEFINITION (S3Resume)
#include EFI_PROTOCOL_DEFINITION (Decompress)
#include EFI_PROTOCOL_DEFINITION (TianoDecompress)
#include EFI_PROTOCOL_DEFINITION (CustomizedDecompress)


UINT32  mScratchSize = 0;
VOID    *mScratchBuffer = NULL;

//
// Module Globals used in the DXE to PEI handoff
// These must be module globals, so the stack can be switched
//
static EFI_DXE_IPL_PPI mDxeIplPpi = {
  DxeLoadCore
};

static EFI_PEI_DECOMPRESS_PPI mDxeIplDecompressPpi = {
  DxeIplDecompress
};

static EFI_PEI_PPI_DESCRIPTOR     mPpiList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI),
    &gEfiDxeIplPpiGuid,
    &mDxeIplPpi
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiDecompressPpiGuid,
    &mDxeIplDecompressPpi
  }
};


static EFI_PEI_PPI_DESCRIPTOR     mEndOfPeiSignalPpi = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEndOfPeiSignalPpiGuid,
  NULL
};

//
// Interface and GUID for the Decompression APIs shared between PEI and DXE
//
EFI_GUID   mPeiEfiDecompressProtocolGuid = EFI_DECOMPRESS_PROTOCOL_GUID;

//
// Interface and GUID for the Tiano Decompression APIs shared between PEI and DXE
//
EFI_GUID   mPeiEfiTianoDecompressProtocolGuid = EFI_TIANO_DECOMPRESS_PROTOCOL_GUID;

//
// Interface and GUID for the user customized Decompression APIs shared between PEI and DXE
//
EFI_GUID   mPeiEfiCustomizedDecompressProtocolGuid = EFI_CUSTOMIZED_DECOMPRESS_PROTOCOL_GUID;

//
// Interface and GUID for the Instruction Cache Flushing APIs shared between PEI and DXE
//
EFI_GUID   mPeiEfiPeiFlushInstructionCacheGuid = EFI_PEI_FLUSH_INSTRUCTION_CACHE_GUID;

//
// Interface and GUID for the PE/COFF Loader APIs shared between PEI and DXE
//
EFI_GUID    mPeiEfiPeiPeCoffLoaderGuid = EFI_PEI_PE_COFF_LOADER_GUID;

//
// Interface and GUID for the setjump()/longjump() APIs shared between PEI and DXE
//
EFI_GUID    mPeiEfiPeiTransferControlGuid = EFI_PEI_TRANSFER_CONTROL_GUID;



EFI_STATUS
EFIAPI
PeiLoadFile (
  IN  VOID                                      *Pe32Data,
  OUT EFI_PHYSICAL_ADDRESS                      *ImageAddressArg,
  OUT UINT64                                    *ImageSizeArg,
  OUT EFI_PHYSICAL_ADDRESS                      *EntryPoint
  )
/*++

Routine Description:

  Routine for loading file image.

Arguments:

  FileHandle        - Handle of file to load.
  ImageAddressArg   - Pointer to the image address.
  ImageSizeArg      - Size of image size.
  EntryPoint        - Pointer to entry point of specified image file for output.

Returns:

  Status - EFI_SUCCESS    - Image is successfully loaded.
           EFI_NOT_FOUND  - Fail to locate necessary PPI
           Others         - Fail to load file.

--*/  
;

EFI_STATUS
EFIAPI
PeimInitializeDxeIpl (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Initializes the Dxe Ipl PPI

Arguments:

  FfsHeader   - Pointer to FFS file header
  PeiServices - General purpose services available to every PEIM.

Returns:

  EFI_SUCCESS

--*/  
;

STATIC
EFI_STATUS
GetFvAlignment (
  IN    EFI_FIRMWARE_VOLUME_HEADER   *FvHeader,
  OUT   UINT32                      *FvAlignment
  );

STATIC
VOID *
EFIAPI
AllocateAlignedPages (
  IN EFI_PEI_SERVICES       **PeiServices,
  IN UINTN                  Pages,
  IN UINTN                  Alignment
  );




EFI_PEIM_ENTRY_POINT (PeimInitializeDxeIpl)


EFI_STATUS
EFIAPI
PeimInitializeDxeIpl (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Initializes the Dxe Ipl PPI

Arguments:

  FfsHeader   - Pointer to FFS file header
  PeiServices - General purpose services available to every PEIM.

Returns:

  EFI_SUCCESS

--*/
{
  #ifdef EFI_NT_EMULATOR
   SetPeiServicesTablePointer(PeiServices);
  #endif
  //
  // Install DxeIpl PPI
  //
  return (*PeiServices)->InstallPpi (PeiServices, &mPpiList[0]);
}

EFI_STATUS
DxeIplFindFirmwareVolumeInstance (
  IN OUT UINTN              *Instance,
  IN  EFI_FV_FILETYPE       SeachType,
  OUT EFI_PEI_FV_HANDLE     *VolumeHandle,
  OUT EFI_PEI_FILE_HANDLE   *FileHandle
  )
/*++

Routine Description:

  Find the First Volume that contains the first FileType.

Arguments:

  Instance     - The Fv instance.
  SeachType    - The type of file to search.
  VolumeHandle - Pointer to Fv which contains the file to search. 
  FileHandle   - Pointer to FFS file to search.

Returns:

  EFI_STATUS

--*/  
{
  EFI_STATUS  Status;
  EFI_STATUS  VolumeStatus;
  EFI_PEI_SERVICES      **PeiServices;

  PeiServices = GetPeiServicesTablePointer(); 

  do {
    VolumeStatus = PeiLibFfsFindNextVolume (*Instance, VolumeHandle);
    if (!EFI_ERROR (VolumeStatus)) {
      *FileHandle = NULL;
      Status = PeiLibFfsFindNextFile (SeachType, *VolumeHandle, FileHandle);
      if (!EFI_ERROR (Status)) {
        return Status;
      }
    }
    *Instance += 1;
  } while (!EFI_ERROR (VolumeStatus));

  return VolumeStatus;
}


EFI_STATUS
DxeIplAddEncapsulatedFirmwareVolumes (
  VOID
  )
/*++

Routine Description:

   Add any encapsulated FV's

Arguments:

  NONE
  
Returns:
  EFI_STATUS
  
--*/  
{
  EFI_STATUS                  Status;
  EFI_STATUS                  VolumeStatus;
  UINTN                       Index;
  EFI_FV_INFO                 VolumeInfo; 
  EFI_PEI_SERVICES            **PeiServices;
  EFI_PEI_FV_HANDLE           VolumeHandle;
  EFI_PEI_FILE_HANDLE         FileHandle;
  UINT32                      SectionLength;
  EFI_FIRMWARE_VOLUME_HEADER  *FvHeader;
  EFI_FIRMWARE_VOLUME_IMAGE_SECTION *SectionHeader;
  VOID                        *DstBuffer;
  UINT32                       FvAlignment;
  EFI_PEI_FV_INFO_PPI_PRIVATE  *FvInfoInstance;
  EFI_PEI_PPI_DESCRIPTOR      *mPpiList;

  Status = EFI_NOT_FOUND;
  Index  = 0;
  PeiServices = GetPeiServicesTablePointer();

  do {
    VolumeStatus = DxeIplFindFirmwareVolumeInstance (
                    &Index, EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE, &VolumeHandle, &FileHandle
                    );
    if (!EFI_ERROR (VolumeStatus)) {
      Status =PeiLibFfsFindSectionData (
                EFI_SECTION_FIRMWARE_VOLUME_IMAGE, 
                (EFI_FFS_FILE_HEADER *)FileHandle, 
                (VOID **)&FvHeader
                );
      if (!EFI_ERROR (Status)) {
        if (FvHeader->Signature == EFI_FVH_SIGNATURE) {
          //
          // Because FvLength in FvHeader is UINT64 type, 
          // so FvHeader must meed at least 8 bytes alignment.
          // If current FvImage base address doesn't meet its alignment,
          // we need to reload this FvImage to another correct memory address.
          //
          Status = GetFvAlignment(FvHeader, &FvAlignment); 
          if (EFI_ERROR(Status)) {
            return Status;
          }
          if (((UINTN) FvHeader % FvAlignment) != 0) {
            SectionHeader = (EFI_FIRMWARE_VOLUME_IMAGE_SECTION*)((UINTN)FvHeader - sizeof(EFI_FIRMWARE_VOLUME_IMAGE_SECTION));
            SectionLength =  *(UINT32 *)SectionHeader->CommonHeader.Size & 0x00FFFFFF;
            
            DstBuffer = AllocateAlignedPages (PeiServices, EFI_SIZE_TO_PAGES ((UINTN) SectionLength - sizeof (EFI_COMMON_SECTION_HEADER)), FvAlignment);
            if (DstBuffer == NULL) {
              return EFI_OUT_OF_RESOURCES;
            }
            (*PeiServices)->CopyMem (DstBuffer, FvHeader, (UINTN) SectionLength - sizeof (EFI_COMMON_SECTION_HEADER));
            FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *) DstBuffer;  
          }

          //
          // This new Firmware Volume comes from a firmware file within a firmware volume.
          // Record the original Firmware Volume Name.
          //
          PeiLibFfsGetVolumeInfo(&VolumeHandle, &VolumeInfo);

          //
          // Prepare to install FirmwareVolumeInfo PPI to expose new FV to PeiCore.
          //
          Status = (*PeiServices)->AllocatePool (
                            PeiServices,
                            sizeof (EFI_PEI_FV_INFO_PPI_PRIVATE),
                            &FvInfoInstance
                            );
          ASSERT_PEI_ERROR (PeiServices, Status);

          mPpiList = &FvInfoInstance->PpiList;
          (*PeiServices)->CopyMem (
                            &FvInfoInstance->FvInfoPpi.FvFormat,
                            &(FvHeader->FileSystemGuid),
                            sizeof (EFI_GUID)
                            );

          FvInfoInstance->FvInfoPpi.FvInfo = (VOID*)FvHeader;
          FvInfoInstance->FvInfoPpi.FvInfoSize = (UINT32)FvHeader->FvLength;
          FvInfoInstance->FvInfoPpi.ParentFvName = (VOID*)&FvInfoInstance->ParentFvName;
          FvInfoInstance->FvInfoPpi.ParentFileName = (VOID*)&FvInfoInstance->ParentFileName;          

          (*PeiServices)->CopyMem (
                            &FvInfoInstance->ParentFvName,
                            &(VolumeInfo.FvName),
                            sizeof (EFI_GUID)
                            );
          (*PeiServices)->CopyMem (
                            &FvInfoInstance->ParentFileName,
                            &(((EFI_FFS_FILE_HEADER*)FileHandle)->Name),
                            sizeof (EFI_GUID)
                            );
          
          //
          // Install FirmwareVolumeInfo PPI to export new Firmware Volume to Core.
          //
          mPpiList->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
          mPpiList->Guid  = &gEfiFirmwareVolumeInfoPpiGuid;
          mPpiList->Ppi   = &FvInfoInstance->FvInfoPpi;
          Status          = (**PeiServices).InstallPpi (PeiServices, mPpiList);
          ASSERT_PEI_ERROR (PeiServices, Status);

          //
          // Makes the encapsulated volume show up in DXE phase to skip processing of
          // encapsulated file again.
          //
          BuildFvHob2 (
            (EFI_PHYSICAL_ADDRESS)(UINTN)FvHeader,
            FvHeader->FvLength, 
            &VolumeInfo.FvName,
            &(((EFI_FFS_FILE_HEADER *)FileHandle)->Name)
            );
          return Status;
        }
      }
    }
  } while (!EFI_ERROR (VolumeStatus));
  
  return Status;
}


EFI_STATUS
EFIAPI
DxeLoadCore (
  IN EFI_DXE_IPL_PPI       *This,
  IN EFI_PEI_SERVICES      **PeiServices,
  IN EFI_PEI_HOB_POINTERS  HobList
  )
/*++

Routine Description:

  Main entry point to last PEIM

Arguments:
  This         - Entry point for DXE IPL PPI
  PeiServices  - General purpose services available to every PEIM.
  HobList      - Address to the Pei HOB list

Returns:

  EFI_SUCCESS          - DEX core was successfully loaded.
  EFI_OUT_OF_RESOURCES - There are not enough resources to load DXE core.

--*/
{
  EFI_STATUS                                Status;
  EFI_PHYSICAL_ADDRESS                      TopOfStack;
  EFI_PHYSICAL_ADDRESS                      BaseOfStack;
  EFI_PHYSICAL_ADDRESS                      BspStore;  
  VOID                                      *Interface;
  EFI_GUID                                  DxeCoreFileName;
  EFI_PHYSICAL_ADDRESS                      DxeCoreAddress;
  UINT64                                    DxeCoreSize;
  EFI_PHYSICAL_ADDRESS                      DxeCoreEntryPoint;
  EFI_BOOT_MODE                             BootMode;
  PEI_RECOVERY_MODULE_INTERFACE             *PeiRecovery;
  PEI_S3_RESUME_PPI                         *S3Resume;
  EFI_DECOMPRESS_PROTOCOL                   *PeiEfiDecompress;
  EFI_TIANO_DECOMPRESS_PROTOCOL             *PeiEfiTianoDecompress;
  EFI_CUSTOMIZED_DECOMPRESS_PROTOCOL        *PeiEfiCustomizedDecompress;
  EFI_PEI_TRANSFER_CONTROL_PROTOCOL         *PeiEfiPeiTransferControl;
  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  *PeiEfiPeiFlushInstructionCache;
  EFI_PEI_FV_HANDLE                         VolumeHandle;
  EFI_PEI_FILE_HANDLE                       FileHandle;
  UINTN                                     Instance;

  
  PEI_PERF_START (PeiServices, L"DxeIpl", NULL, 0);

  TopOfStack  = 0;
  BaseOfStack = 0;
  BspStore = 0;

  //
  // if in S3 Resume, restore configure
  //
  Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
  if (!EFI_ERROR (Status) && (BootMode == BOOT_ON_S3_RESUME)) {
    Status = PeiServicesLocatePpi (
               &gPeiS3ResumePpiGuid,
               0,
               NULL,
               (VOID **)&S3Resume
               );
    ASSERT_PEI_ERROR (PeiServices, Status);

    Status = S3Resume->S3RestoreConfig (PeiServices);
    //
    // S2RestorConfig should transfer back to OS resume vector
    //
    ASSERT_PEI_ERROR (PeiServices, Status);
  }

  Status = EFI_SUCCESS;

  //
  // Get the EFI decompress functions for possible usage
  //
  Status = InstallEfiDecompress (&PeiEfiDecompress);

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Get the Tiano decompress functions for possible usage
  //
  Status = InstallTianoDecompress (&PeiEfiTianoDecompress);

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Get the user Customized decompress functions for possible usage
  //
  Status = InstallCustomizedDecompress (&PeiEfiCustomizedDecompress);

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Install the PEI Protocols that are shared between PEI and DXE
  //
  Status = InstallEfiPeiFlushInstructionCache (&PeiEfiPeiFlushInstructionCache);

  ASSERT_PEI_ERROR (PeiServices, Status);


  Status = InstallEfiPeiTransferControl (&PeiEfiPeiTransferControl);

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Allocate 128KB for the Stack
  //
  Status = (*PeiServices)->AllocatePages (
                            PeiServices,
                            EfiBootServicesData,
                            EFI_SIZE_TO_PAGES (EFI_STACK_SIZE),
                            &BaseOfStack
                            );

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Compute the top of the stack we were allocated. Pre-allocate a UINTN
  // for safety.
  //
  TopOfStack = BaseOfStack + EFI_SIZE_TO_PAGES (EFI_STACK_SIZE) * EFI_PAGE_SIZE - sizeof (UINTN);
  //
  // Add architecture-specifc HOBs (including the BspStore HOB)
  //
  Status = CreateArchSpecificHobs (
            PeiServices,
            &BspStore
            );

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Add HOB for the EFI Decompress Protocol
  //
  Interface = (VOID *) PeiEfiDecompress;

  Status = PeiBuildHobGuidData (
            PeiServices,
            &mPeiEfiDecompressProtocolGuid,
            &Interface,
            sizeof (VOID *)
            );

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Add HOB for the Tiano Decompress Protocol
  //
  Interface = (VOID *) PeiEfiTianoDecompress;

  Status = PeiBuildHobGuidData (
            PeiServices,
            &mPeiEfiTianoDecompressProtocolGuid,
            &Interface,
            sizeof (VOID *)
            );

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Add HOB for the user customized Decompress Protocol
  //
  Interface = (VOID *) PeiEfiCustomizedDecompress;

  Status = PeiBuildHobGuidData (
            PeiServices,
            &mPeiEfiCustomizedDecompressProtocolGuid,
            &Interface,
            sizeof (VOID *)
            );

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Add HOB for the Flush Instruction Cache Protocol
  //
  Interface = (VOID *) PeiEfiPeiFlushInstructionCache;

  Status = PeiBuildHobGuidData (
            PeiServices,
            &mPeiEfiPeiFlushInstructionCacheGuid,
            &Interface,
            sizeof (VOID *)
            );

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Add HOB for the PE/COFF Loader Protocol
  //
  InstallEfiPeiPeCoffLoader (PeiServices, &(EFI_PEI_PE_COFF_LOADER_PROTOCOL*)Interface, NULL);
  Status = PeiBuildHobGuidData (
            PeiServices,
            &mPeiEfiPeiPeCoffLoaderGuid,
            &Interface,
            sizeof (VOID *)
            );

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Add HOB for the Transfer Control Protocol
  //
  Interface = (VOID *) PeiEfiPeiTransferControl;

  Status = PeiBuildHobGuidData (
            PeiServices,
            &mPeiEfiPeiTransferControlGuid,
            &Interface,
            sizeof (VOID *)
            );

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // See if we are in crisis recovery
  //
  if ((BootMode == BOOT_IN_RECOVERY_MODE)) {

    Status = PeiServicesLocatePpi (
               &gPeiRecoveryModulePpiGuid,
               0,
               NULL,
               (VOID **)&PeiRecovery
               );
    ASSERT_PEI_ERROR (PeiServices, Status);
    
    Status = PeiRecovery->LoadRecoveryCapsule (PeiServices, PeiRecovery);
    ASSERT_PEI_ERROR (PeiServices, Status);

  }
  
  //
  // If any FV contains an encapsulated FV extract that FV
  //
  DxeIplAddEncapsulatedFirmwareVolumes ();

  //
  // Look in all the FVs present in PEI and find the DXE Core
  //
  Instance = 0;
  Status = DxeIplFindFirmwareVolumeInstance (&Instance, EFI_FV_FILETYPE_DXE_CORE, &VolumeHandle, &FileHandle);
  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Copy the File GUID of Dxe Core.
  //
  (*PeiServices)->CopyMem (
                    &DxeCoreFileName,
                    &(((EFI_FFS_FILE_HEADER*)FileHandle)->Name),
                    sizeof (EFI_GUID)
                    );
  //
  // Load the DXE Core from a Firmware Volume
  //
  Status = PeiLoadFile (
            FileHandle,
            &DxeCoreAddress,
            &DxeCoreSize,
            &DxeCoreEntryPoint
            );

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Transfer control to the DXE Core
  // The handoff state is simply a pointer to the HOB list
  //
  PEI_PERF_END (PeiServices, L"DxeIpl", NULL, 0);
  Status = (*PeiServices)->InstallPpi (PeiServices,&mEndOfPeiSignalPpi);
  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Add HOB for the DXE Core
  //
  Status = PeiBuildHobModule (
            PeiServices,
            &DxeCoreFileName,
            DxeCoreAddress,
            DxeCoreSize,
            DxeCoreEntryPoint
            );
  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Report Status Code EFI_SW_PEI_PC_HANDOFF_TO_NEXT
  //
  PEI_REPORT_STATUS_CODE (
    PeiServices,
    EFI_PROGRESS_CODE,
    EFI_SOFTWARE_PEI_MODULE | EFI_SW_PEI_CORE_PC_HANDOFF_TO_NEXT,
    0,
    NULL,
    NULL
    );

  PEI_DEBUG ((PeiServices, EFI_D_INFO, "DXE Core Entry\n"));
  SwitchStacks (
    (VOID *) (UINTN) DxeCoreEntryPoint,
    (UINTN) (HobList.Raw),
    (VOID *) (UINTN) TopOfStack,
    (VOID *) (UINTN) BspStore
    );

  //
  // We should never get here! We should be running the DXE Core
  //
  ASSERT_PEI_ERROR (PeiServices, Status);

  return EFI_OUT_OF_RESOURCES;
}

EFI_STATUS
EFIAPI
PeiLoadFile (
  IN  EFI_PEI_FILE_HANDLE                       FileHandle,
  OUT EFI_PHYSICAL_ADDRESS                      *ImageAddressArg,
  OUT UINT64                                    *ImageSizeArg,
  OUT EFI_PHYSICAL_ADDRESS                      *EntryPoint
  )
/*++

Routine Description:

  Routine for loading file image.

Arguments:

  FileHandle        - Handle of file to load.
  ImageAddressArg   - Pointer to the image address.
  ImageSizeArg      - Size of image size.
  EntryPoint        - Pointer to entry point of specified image file for output.

Returns:

  Status - EFI_SUCCESS    - Image is successfully loaded.
           EFI_NOT_FOUND  - Fail to locate necessary PPI
           Others         - Fail to load file.

--*/
{
  EFI_STATUS                        Status;
  EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT ImageContext;
  EFI_PHYSICAL_ADDRESS              ImageAddress;
  UINT64                            ImageSize;
  EFI_PEI_PE_COFF_LOADER_PROTOCOL   *PeCoffLoader;
  EFI_PEI_SERVICES                  **PeiServices;
  EFI_TE_IMAGE_HEADER               *TEImageHeader;
  VOID                              *Pe32Data;

  PeiServices = GetPeiServicesTablePointer(); 
  *EntryPoint          = 0;
  ImageAddress         = 0;
  ImageSize            = 0;

  //
  // Try to find PE32 or a TE section.
  //
  Status = (*PeiServices)->FfsFindSectionData (
            PeiServices,
            EFI_SECTION_PE32,
            &FileHandle,
            &Pe32Data
            );
  if (EFI_ERROR (Status)) {
    Status = (*PeiServices)->FfsFindSectionData (
               PeiServices,
               EFI_SECTION_TE,
               &FileHandle,
               (VOID **)&TEImageHeader
               );
    Pe32Data = TEImageHeader;
  }
  
  if (EFI_ERROR (Status)) {
    //
    // NO image types we support so exit.
    //
    return Status;
  }
  
  //
  // Load, relocate, and run image in memory
  //
  ZeroMem (&ImageContext, sizeof (ImageContext));
  ImageContext.Handle = Pe32Data;
  ImageContext.ImageRead = PeiImageRead;
  InstallEfiPeiPeCoffLoader (PeiServices, &PeCoffLoader, NULL);
  Status = PeCoffLoader->GetImageInfo (PeCoffLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Allocate Memory for the image
  //
  Status = (*PeiServices)->AllocatePages (
            PeiServices, 
            EfiBootServicesData, 
            EFI_SIZE_TO_PAGES ((UINT32) ImageContext.ImageSize), 
            &ImageContext.ImageAddress
            );
  ASSERT_PEI_ERROR (PeiServices,Status);
  
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
  

  if (ImageAddressArg != NULL) {
    *ImageAddressArg = ImageAddress;
  }

  if (ImageSizeArg != NULL) {
    *ImageSizeArg = ImageSize;
  }
  
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
DxeIplDecompress (
  IN  CONST EFI_PEI_DECOMPRESS_PPI          *This,
  IN  CONST EFI_COMPRESSION_SECTION         *InputSection,
  OUT VOID                                  **OutputBuffer,
  OUT UINTN                                 *OutputSize
  )
/*++

Routine Description:

  Routine for decompress a image.

Arguments:

  This              - Pointer to EFI_PEI_DECOMPRESS_PPI.
  InputSection      - Pointer to compressed image.
  OutputBuffer      - Pointer to the decompressed image address.
  OutputSize        - Size of decompressed image size.

Returns:

  EFI_STATUS

--*/  
{
  EFI_STATUS                      Status;
  UINT32                          CompressedSize;
  EFI_PEI_SERVICES                **PeiServices;
  UINT8                           *DstBuffer;
  UINT8                           *ScratchBuffer;
  UINT32                          DstBufferSize;
  UINTN                           ScratchBufferSize;
  EFI_PHYSICAL_ADDRESS            OldTopOfMemory;
  EFI_TIANO_DECOMPRESS_PROTOCOL   *DecompressProtocol;

  DstBufferSize     = 0;
  ScratchBufferSize = 0;
  DecompressProtocol  = NULL;

  PeiServices = GetPeiServicesTablePointer();
  CompressedSize = SECTION_SIZE (InputSection) - sizeof (EFI_COMPRESSION_SECTION);
  //
  // Get the Tiano decompress functions for possible usage
  //
  if (InputSection->CompressionType == EFI_STANDARD_COMPRESSION) {
    Status = InstallTianoDecompress (&DecompressProtocol);
  } else {
  //
  // Get the user Customized decompress functions for possible usage
  //
    Status = InstallCustomizedDecompress ((EFI_CUSTOMIZED_DECOMPRESS_PROTOCOL **)&DecompressProtocol);
  }
  
  ASSERT_PEI_ERROR (PeiServices, Status);
  Status = (*PeiServices)->AllocatePages (
                            PeiServices,
                            EfiBootServicesData,
                            1,  // EFI_PAGE_SIZE,
                            &OldTopOfMemory
                            );
  
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
 
  DstBuffer         = (UINT8 *) (UINTN) (OldTopOfMemory);
  ScratchBuffer     = (UINT8 *) (UINTN) (OldTopOfMemory);
  Status = DecompressProtocol->GetInfo (
                                DecompressProtocol,
                                (UINT8 *) ((EFI_COMPRESSION_SECTION *) InputSection + 1),
                                (UINT32) CompressedSize,
                                (UINT32 *) &DstBufferSize,
                                (UINT32 *) &ScratchBufferSize
                                );
  
  if (EFI_SUCCESS == Status) {
    //
    // This is a compression set, expand it
    //
    Status = (*PeiServices)->AllocatePages (
                              PeiServices,
                              EfiBootServicesData,
                              EFI_SIZE_TO_PAGES (ScratchBufferSize),
                              &OldTopOfMemory
                              );
  
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  
    ScratchBuffer = (UINT8 *) (UINTN) (OldTopOfMemory);
  
    //
    // Allocate destination buffer
    //
    Status = (*PeiServices)->AllocatePages (
                              PeiServices,
                              EfiBootServicesData,
                              EFI_SIZE_TO_PAGES (DstBufferSize),
                              &OldTopOfMemory
                              );
  
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  
    DstBuffer = (UINT8 *) (UINTN) (OldTopOfMemory);
  
    //
    // Call decompress function
    //
    Status = DecompressProtocol->Decompress (
                                  DecompressProtocol,
                                  (CHAR8 *) ((EFI_COMPRESSION_SECTION *) InputSection + 1),
                                  (UINT32) CompressedSize,
                                  DstBuffer,
                                  (UINT32) DstBufferSize,
                                  ScratchBuffer,
                                  (UINT32) ScratchBufferSize
                                  );
  }
  
  if (EFI_ERROR (Status)) {
    //
    // Decompress failed
    //
    return EFI_NOT_FOUND;
  }
  
  *OutputBuffer = DstBuffer;
  *OutputSize = DstBufferSize;
  return Status;
}


STATIC
EFI_STATUS
GetFvAlignment (
  IN    EFI_FIRMWARE_VOLUME_HEADER   *FvHeader,
  OUT   UINT32                      *FvAlignment
  )
{
  //
  // Because FvLength in FvHeader is UINT64 type, 
  // so FvHeader must meed at least 8 bytes alignment.
  // Get the appropriate alignment requirement.
  // 
  if ((FvHeader->Attributes & EFI_FVB2_ALIGNMENT) < EFI_FVB2_ALIGNMENT_8) {
    return EFI_UNSUPPORTED;
  }
  
   *FvAlignment = 1 << ((FvHeader->Attributes & EFI_FVB2_ALIGNMENT) >> 16);
   return EFI_SUCCESS;
}


STATIC
VOID *
EFIAPI
AllocateAlignedPages (
  IN EFI_PEI_SERVICES       **PeiServices,
  IN UINTN                  Pages,
  IN UINTN                  Alignment
  )
{
  VOID                  *Memory;
  UINTN                 AlignmentMask;
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  BufferAddress; 

  //
  // Alignment must be a power of two or zero.
  //
  PEI_ASSERT (PeiServices, (Alignment & (Alignment - 1)) == 0);

  if (Pages == 0) {
    return NULL;
  }
  //
  // Make sure that Pages plus EFI_SIZE_TO_PAGES (Alignment) does not overflow.
  //
  PEI_ASSERT (PeiServices,Pages <= (EFI_MAX_ADDRESS - EFI_SIZE_TO_PAGES (Alignment)));
  //
  // We would rather waste some memory to save PEI code size.
  //
  if ((Pages + EFI_SIZE_TO_PAGES (Alignment)) == 0) {
    return NULL;
  }

  Status = ((*PeiServices)->AllocatePages) (PeiServices, EfiBootServicesData, (Pages + EFI_SIZE_TO_PAGES (Alignment)), &BufferAddress);

  if (EFI_ERROR (Status)) {
    BufferAddress = 0;
  }
  Memory = (VOID *) (UINTN) BufferAddress;

  if (Alignment == 0) {
    AlignmentMask = Alignment;
  } else {
    AlignmentMask = Alignment - 1;  
  }
  return (VOID *) (UINTN) (((UINTN) Memory + AlignmentMask) & ~AlignmentMask);
}


