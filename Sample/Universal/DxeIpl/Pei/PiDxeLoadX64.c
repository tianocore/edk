/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PiDxeLoadX64.c

Abstract:

  Last PEIM.
  Responsibility of this module is to load the DXE Core from a Firmware Volume.

--*/

#include "Tiano.h"
#include "Pei.h"
#include "PiDxeIpl.h"
#include "DxeLoadFunc.h"
#include "PeiLib.h"
#include EFI_PPI_DEFINITION (RecoveryModule)
#include EFI_PPI_DEFINITION (S3Resume)
#include EFI_PROTOCOL_DEFINITION (Decompress)
#include EFI_PROTOCOL_DEFINITION (TianoDecompress)
#include EFI_PROTOCOL_DEFINITION (CustomizedDecompress)


#pragma warning( disable : 4305 )

EFI_PEI_SERVICES                  **gPeiServices;

//
// Interface and GUID for the Decompression APIs shared between PEI and DXE
//
EFI_GUID                          mPeiEfiDecompressProtocolGuid = EFI_DECOMPRESS_PROTOCOL_GUID;

//
// Interface and GUID for the Tiano Decompression APIs shared between PEI and DXE
//
EFI_GUID                          mPeiEfiTianoDecompressProtocolGuid = EFI_TIANO_DECOMPRESS_PROTOCOL_GUID;

//
// Interface and GUID for the user customized Decompression APIs shared between PEI and DXE
//
EFI_GUID                          mPeiEfiCustomizedDecompressProtocolGuid = EFI_CUSTOMIZED_DECOMPRESS_PROTOCOL_GUID;

//
// Interface and GUID for the Instruction Cache Flushing APIs shared between PEI and DXE
//
EFI_GUID                          mPeiEfiPeiFlushInstructionCacheGuid = EFI_PEI_FLUSH_INSTRUCTION_CACHE_GUID;

//
// Interface and GUID for the PE/COFF Loader APIs shared between PEI and DXE
//
EFI_GUID                          mPeiEfiPeiPeCoffLoaderGuid = EFI_PEI_PE_COFF_LOADER_GUID;

//
// Interface and GUID for the setjump()/longjump() APIs shared between PEI and DXE
//
EFI_GUID                          mPeiEfiPeiTransferControlGuid = EFI_PEI_TRANSFER_CONTROL_GUID;

//
// GUID for EM64T
//
#define EFI_PPI_NEEDED_BY_DXE \
  { \
    0x4d37da42, 0x3a0c, 0x4eda, 0xb9, 0xeb, 0xbc, 0x0e, 0x1d, 0xb4, 0x71, 0x3b \
  }
EFI_GUID mPpiNeededByDxeGuid = EFI_PPI_NEEDED_BY_DXE;

#define EFI_BREAK \
  { \
    0x5B60CCFD, 0x1011, 0x4BCF, 0xb7, 0xd1, 0xbb, 0x99, 0xca, 0x96, 0xa6, 0x03 \
  }
EFI_GUID mBreakGuid = EFI_BREAK;


//
// GUID for the Firmware Volume type that PEI supports
//
// EFI_GUID  mPeiFwFileSysTypeGuid = EFI_FIRMWARE_FILE_SYSTEM_GUID;
//
// Module Globals used in the DXE to PEI handoff
// These must be module globals, so the stack can be switched
//
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
;

EFI_STATUS
PeiLoadx64File (
  IN  EFI_PEI_SERVICES                          **PeiServices,
  IN  EFI_PEI_PE_COFF_LOADER_PROTOCOL           *PeiEfiPeiPeCoffLoader,
  IN  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  *PeiEfiPeiFlushInstructionCache,
  IN  EFI_PEI_FILE_HANDLE                       FileHandle,
  OUT EFI_PHYSICAL_ADDRESS                      *ImageAddress,
  OUT UINT64                                    *ImageSize,
  OUT EFI_PHYSICAL_ADDRESS                      *EntryPoint
  )
/*++

Routine Description:

  Loads and relocates a PE/COFF image into memory.

Arguments:

  PeiService                      - General purpose services available to every PEIM.
  PeiEfiPeiPeCoffLoader           - Pointer to a PE COFF loader protocol
  PeiEfiPeiFlushInstructionCache  - Pointer to a flush-instruction-cache protocol so
                                    we can flush the cache after loading
  PeiImage                        - The base address of the PE/COFF file that is to be loaded and relocated
  ImageAddress                    - The base address of the relocated PE/COFF image
  ImageSize                       - The size of the relocated PE/COFF image
  EntryPoint                      - The entry point of the relocated PE/COFF image

Returns:

  EFI_SUCCESS                     - The file was loaded and relocated
  EFI_OUT_OF_RESOURCES            - There was not enough memory to load and relocate the PE/COFF file

--*/  
;

VOID
EfiCommonLibZeroMem (
  IN VOID   *Buffer,
  IN UINTN  Size
  )
/*++

Routine Description:

  Set Buffer to 0 for Size bytes. Bugbug, should be replaced by Pei Lib function

Arguments:

  Buffer  - Memory to set.

  Size    - Number of bytes to set

Returns:

  None

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


static EFI_PEI_PPI_DESCRIPTOR     mPpiSignal = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEndOfPeiSignalPpiGuid,
  NULL
};



EFI_PEIM_ENTRY_POINT (PeimInitializeDxeIpl)

typedef
EFI_STATUS
(EFIAPI *DXE_IPL_ENTRYPOINT) (
  IN EFI_FFS_FILE_HEADER       * FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  );

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
DxeIplFindFileByName (
  IN OUT UINTN              *Instance,
  IN  EFI_GUID              *FileName,
  OUT EFI_PEI_FV_HANDLE     *VolumeHandle,
  OUT EFI_PEI_FILE_HANDLE   *FileHandle
  )
/*++

Routine Description:

   Search a file with its name in all Fv list.

Arguments:

  Instance      - The Fv instance.
  FileName      - File name to search.
  VolumeHandle  - Pointer to Fv which contains the file to search.
  FileHandle    - Pointer to the file to search.
  
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
      Status = PeiLibFfsFindFileByName (FileName, *VolumeHandle, FileHandle);
      if (!EFI_ERROR (Status)) {
        return Status;
      }
    }
    *Instance += 1;
  } while (!EFI_ERROR (VolumeStatus));

  return VolumeStatus;
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
  EFI_STATUS                                                Status;
  EFI_PHYSICAL_ADDRESS                                      TopOfStack;
  EFI_PHYSICAL_ADDRESS                                      BaseOfStack;
  EFI_PHYSICAL_ADDRESS                                      BspStore;
  EFI_GUID                                                  DxeCoreFileName;
  EFI_PHYSICAL_ADDRESS                                      DxeCoreAddress;
  UINT64                                                    DxeCoreSize;
  EFI_PHYSICAL_ADDRESS                                      DxeCoreEntryPoint;
  EFI_PHYSICAL_ADDRESS                                      PpisNeededByDxeAddress;
  UINT64                                                    PpisNeededByDxeSize;
  EFI_PHYSICAL_ADDRESS                                      PpisNeededByDxeEntryPoint;
  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL                  *PeiEfiPeiFlushInstructionCache;
  EFI_PEI_PE_COFF_LOADER_PROTOCOL                           *PeiEfiPeiPeCoffLoader;
  EFI_BOOT_MODE                                             BootMode;
  PEI_RECOVERY_MODULE_INTERFACE                             *PeiRecovery;
  PEI_S3_RESUME_PPI                                         *S3Resume;
  EFI_PHYSICAL_ADDRESS                                      PageTables;
  EFI_PEI_FV_HANDLE                                         VolumeHandle;
  EFI_PEI_FILE_HANDLE                                       DxeCoreFileHandle;
  EFI_PEI_FILE_HANDLE                                       FileHandle;
  UINTN                                                     Instance;
  EFI_PEI_HOB_POINTERS                                      Hob;
  UINT8                                                     SizeOfMemorySpace;


  PEI_PERF_START (PeiServices, L"DxeIpl", NULL, 0);

  TopOfStack  = 0;
  BaseOfStack = 0;
  BspStore    = 0;
  Status      = EFI_SUCCESS;
  PeiEfiPeiPeCoffLoader = NULL;

  //
  // if in S3 Resume, restore configure
  //
  Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
  if (!EFI_ERROR (Status) && (BootMode == BOOT_ON_S3_RESUME)) {
    Status = (*PeiServices)->LocatePpi (
                               PeiServices,
                               &gPeiS3ResumePpiGuid,
                               0,
                               NULL,
                               &S3Resume
                               );
    ASSERT_PEI_ERROR (PeiServices, Status);

    Status = S3Resume->S3RestoreConfig (PeiServices);
    ASSERT_PEI_ERROR (PeiServices, Status);
  }

  //
  // Install the PEI Protocols that are shared between PEI and DXE
  //
  Status                = InstallEfiPeiFlushInstructionCache (&PeiEfiPeiFlushInstructionCache);
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
  // Compute the top of the stack we were allocated. Pre-allocate a 32 bytes
  // for safety (PpisNeededByDxe and DxeCore).
  //
  TopOfStack = BaseOfStack + EFI_SIZE_TO_PAGES (EFI_STACK_SIZE) * EFI_PAGE_SIZE - 32;

  //
  // Add architecture-specifc HOBs (including the BspStore HOB)
  //
  Status = CreateArchSpecificHobs (
             PeiServices,
             &BspStore
             );
  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // See if we are in crisis recovery
  //
  Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
  if (!EFI_ERROR (Status) && (BootMode == BOOT_IN_RECOVERY_MODE)) {
    Status = (*PeiServices)->LocatePpi (
                               PeiServices,
                               &gPeiRecoveryModulePpiGuid,
                               0,
                               NULL,
                               &PeiRecovery
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
  Status = DxeIplFindFirmwareVolumeInstance (&Instance, EFI_FV_FILETYPE_DXE_CORE, &VolumeHandle, &DxeCoreFileHandle);
  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Copy the File GUID of Dxe Core.
  //
  (*PeiServices)->CopyMem (
                    &DxeCoreFileName,
                    &(((EFI_FFS_FILE_HEADER*)DxeCoreFileHandle)->Name),
                    sizeof (EFI_GUID)
                    );
  //
  // Find the PpisNeededByDxe in a Firmware Volume
  //
  Instance = 0;
  Status = DxeIplFindFileByName (
             &Instance,
             &mPpiNeededByDxeGuid,
             &VolumeHandle,
             &FileHandle
             );
  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Load the GDT of Go64. Since the GDT of 32-bit Tiano locates in the BS_DATA \
  // memory, it may be corrupted when copying FV to high-end memory 
  LoadGo64Gdt();

  //
  // Set default memory space addressing to 36 bits
  //
  SizeOfMemorySpace = 36;

  //
  //
  // Get memory space addressing value from CPU Hob
  Status = (*PeiServices)->GetHobList (PeiServices, &Hob.Raw);
  if (!EFI_ERROR (Status)) {
    Hob.Raw = GetHob (EFI_HOB_TYPE_CPU, Hob.Raw);
    if (Hob.Header->HobType == EFI_HOB_TYPE_CPU) {
      SizeOfMemorySpace = Hob.Cpu->SizeOfMemorySpace;
    }
  }

  PageTables = CreateIdentityMappingPageTables (PeiServices, SizeOfMemorySpace);

  InstallEfiPeiPeCoffLoader (PeiServices, &PeiEfiPeiPeCoffLoader, NULL);

  //
  // Load the PpiNeededByDxe from a Firmware Volume
  //
  Status = PeiLoadx64File (
             PeiServices,
             PeiEfiPeiPeCoffLoader,
             PeiEfiPeiFlushInstructionCache,
             FileHandle,
             &PpisNeededByDxeAddress,
             &PpisNeededByDxeSize,
             &PpisNeededByDxeEntryPoint
             );
  ASSERT_PEI_ERROR (PeiServices, Status);


  //
  // Load the DXE Core from a Firmware Volume
  //
  Status = PeiLoadx64File (
             PeiServices,
             PeiEfiPeiPeCoffLoader,
             PeiEfiPeiFlushInstructionCache,
             DxeCoreFileHandle,
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

  Status = (*PeiServices)->InstallPpi (PeiServices, &mPpiSignal);

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Add HOB for the DXE Core
  //

  // Adjust DxeCoreSize to 4K granularity to meet the page size requirements of UEFI
  DxeCoreSize = (DxeCoreSize + EFI_PAGE_MASK) & ~((UINT64)EFI_PAGE_MASK);

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
  (**PeiServices).PeiReportStatusCode (
                    PeiServices,
                    EFI_PROGRESS_CODE,
                    EFI_SOFTWARE_PEI_MODULE | EFI_SW_PEI_CORE_PC_HANDOFF_TO_NEXT,
                    0,
                    NULL,
                    NULL
                    );

  //
  // Go to Long Mode. Interrupts will not get turned on until the CPU AP is loaded.
  // Call x64 drivers passing in single argument, a pointer to the HOBs.
  //
  ActivateLongMode (
    PageTables, 
    (EFI_PHYSICAL_ADDRESS)(UINTN)(HobList.Raw), 
    TopOfStack,
    PpisNeededByDxeEntryPoint,
    DxeCoreEntryPoint
    );

  //
  // If we get here, then the DXE Core returned.  This is an error
  //
  ASSERT_PEI_ERROR (PeiServices, Status);

  return EFI_OUT_OF_RESOURCES;
}

EFI_STATUS
PeiLoadx64File (
  IN  EFI_PEI_SERVICES                          **PeiServices,
  IN  EFI_PEI_PE_COFF_LOADER_PROTOCOL           *PeiEfiPeiPeCoffLoader,
  IN  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  *PeiEfiPeiFlushInstructionCache,
  IN  EFI_PEI_FILE_HANDLE                       FileHandle,
  OUT EFI_PHYSICAL_ADDRESS                      *ImageAddress,
  OUT UINT64                                    *ImageSize,
  OUT EFI_PHYSICAL_ADDRESS                      *EntryPoint
  )
/*++

Routine Description:

  Loads and relocates a PE/COFF image into memory.

Arguments:

  PeiService                      - General purpose services available to every PEIM.
  PeiEfiPeiPeCoffLoader           - Pointer to a PE COFF loader protocol
  PeiEfiPeiFlushInstructionCache  - Pointer to a flush-instruction-cache protocol so
                                    we can flush the cache after loading
  PeiImage                        - The base address of the PE/COFF file that is to be loaded and relocated
  ImageAddress                    - The base address of the relocated PE/COFF image
  ImageSize                       - The size of the relocated PE/COFF image
  EntryPoint                      - The entry point of the relocated PE/COFF image

Returns:

  EFI_SUCCESS                     - The file was loaded and relocated
  EFI_OUT_OF_RESOURCES            - There was not enough memory to load and relocate the PE/COFF file

--*/
{
  EFI_STATUS                            Status;
  EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT  ImageContext;
  EFI_PHYSICAL_ADDRESS                  MemoryBuffer;
  VOID                                  *Pe32Data;
  EFI_TE_IMAGE_HEADER                   *TEImageHeader;

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


  (*PeiServices)->SetMem (
                    &ImageContext,
                    sizeof (ImageContext),
                    0
                    );
  ImageContext.Handle = Pe32Data;
  Status              = GetImageReadFunction (PeiServices, &ImageContext);

  ASSERT_PEI_ERROR (PeiServices, Status);
  
  Status = PeiEfiPeiPeCoffLoader->GetImageInfo (PeiEfiPeiPeCoffLoader, &ImageContext);
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
                             &MemoryBuffer
                             );

  ASSERT_PEI_ERROR (PeiServices, Status);

  ImageContext.ImageAddress = MemoryBuffer;

  //
  // Load the image to our new buffer
  //

  Status = PeiEfiPeiPeCoffLoader->LoadImage (PeiEfiPeiPeCoffLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Relocate the image in our new buffer
  //
  Status = PeiEfiPeiPeCoffLoader->RelocateImage (PeiEfiPeiPeCoffLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Flush the instruction cache so the image data is written before we execute it
  //
  Status = PeiEfiPeiFlushInstructionCache->Flush (
                                             PeiEfiPeiFlushInstructionCache,
                                             ImageContext.ImageAddress,
                                             ImageContext.ImageSize
                                             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *ImageAddress = ImageContext.ImageAddress;
  *ImageSize    = ImageContext.ImageSize;
  *EntryPoint   = ImageContext.EntryPoint;

  return EFI_SUCCESS;
}

EFI_PHYSICAL_ADDRESS
AllocateZeroedHobPages (
  IN  EFI_PEI_SERVICES       **PeiServices,
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
 EFI_STATUS          Status;
#if 0
  
  UINTN                   NumberOfBytes;
  UINT64                  AlignmentOffset;
  EFI_PEI_HOB_POINTERS        Hob;
  

  //
  // EFI pages are 4K by convention. EFI pages are independent of processor page size
  //
  NumberOfBytes = 0x1000 * NumberOfPages;

  //
  // Allocate the EFI pages out of Hob Free Memory Heap.
  // Heap grows down from top of Free Memory. HOB grows up.
  //
  //  Page = gHob->Phit.EfiFreeMemoryTop - NumberOfBytes + 1;
  Status = (*PeiServices)->GetHobList (PeiServices, &Hob.Raw);
  Page = Hob.HandoffInformationTable->EfiFreeMemoryTop -  NumberOfBytes + 1;

  //
  // Make sure page is 4K aligned.
  //
  AlignmentOffset = Page & EFI_PAGE_MASK;
  NumberOfBytes += (UINTN)AlignmentOffset;
  Page -= AlignmentOffset;

  if (Page < Hob.HandoffInformationTable->EfiFreeMemoryBottom) {
    PEI_DEBUG ((PeiServices, EFI_D_ERROR, "Pages Requested %d\n", NumberOfPages));
    ASSERT_PEI_ERROR (PeiServices, FALSE);
    return 0;
  }

  EfiCommonLibZeroMem ((VOID *)(UINTN)Page, NumberOfBytes);

  Hob.HandoffInformationTable->EfiFreeMemoryTop -= NumberOfBytes;

  Status = PeiBuildHobMemoryAllocation (
             PeiServices,
             Page,  // now the bottom of our allocation
             NumberOfBytes,
             NULL,
             4
             );
#endif
    
  //
  // Allocate 128KB for the Stack
  //
  Status = (*PeiServices)->AllocatePages (
                             PeiServices,
                             EfiBootServicesData,
                             NumberOfPages,
                             &Page
                             );
  if (!EFI_ERROR (Status)) {
    EfiCommonLibZeroMem ((VOID *) Page, EFI_PAGES_TO_SIZE (NumberOfPages));
  }
  return Page;
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
  VOID                            *SectionInMemory;

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

  //
  // Copy compressed section to memory before decompess
  //
  Status = (*PeiServices)->AllocatePages (
                             PeiServices,
                             EfiBootServicesData,
                             EFI_SIZE_TO_PAGES(SECTION_SIZE (InputSection)),
                             &OldTopOfMemory);
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  SectionInMemory = (VOID*) (UINTN) OldTopOfMemory;
  (*PeiServices)->CopyMem (SectionInMemory, (VOID *) InputSection, SECTION_SIZE (InputSection));
  Status = DecompressProtocol->GetInfo (
                                DecompressProtocol,
                                (UINT8 *) ((EFI_COMPRESSION_SECTION *) SectionInMemory + 1),
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
                                  (CHAR8 *) ((EFI_COMPRESSION_SECTION *) SectionInMemory + 1),
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

