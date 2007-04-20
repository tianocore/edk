/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DxeIpl.h

Abstract:

--*/

#ifndef _PEI_DXEIPL_H_
#define _PEI_DXEIPL_H_

#include "PeiHob.h"
#include "PeiLib.h"
#include "EfiFirmwareVolumeHeader.h"
#include "EfiFirmwareFileSystem.h"
#include "pei.h"

//
// Imported Interfaces
//
#include EFI_GUID_DEFINITION (PeiTransferControl)
#include EFI_GUID_DEFINITION (FirmwareFileSystem2)
#include EFI_PPI_DEFINITION (RecoveryModule)
#include EFI_PPI_DEFINITION (S3Resume)
#include EFI_PPI_DEFINITION (SectionExtraction)
#include EFI_PPI_DEFINITION (Security2)
#include EFI_PPI_DEFINITION (PeiInMemory)
#include EFI_PPI_DEFINITION (LoadFile2)

//
// Exported Interfaces
//
#include EFI_PPI_DEFINITION (DxeIpl)
#include EFI_PPI_DEFINITION (EndOfPeiSignal)
#include EFI_PROTOCOL_DEFINITION (Decompress)
#include EFI_PROTOCOL_DEFINITION (TianoDecompress)
#include EFI_PROTOCOL_DEFINITION (CustomizedDecompress)
#include EFI_GUID_DEFINITION (PeiFlushInstructionCache)
#include EFI_GUID_DEFINITION (PeiPeCoffLoader)

#include "ImageRead.h"
#include "peihoblib.h"

#define STACK_SIZE      0x20000
#define BSP_STORE_SIZE  0x4000

#define PEI_DXEIPL_STACK_SIZE 0x10000


typedef struct _EFI_PEI_FV_INFO_PPI_PRIVATE {
  EFI_PEI_PPI_DESCRIPTOR                  PpiList;
  EFI_PEI_FIRMWARE_VOLUME_INFO_PPI        FvInfoPpi;
  EFI_GUID                                ParentFvName;
  EFI_GUID                                ParentFileName;
} EFI_PEI_FV_INFO_PPI_PRIVATE;


EFI_STATUS
InstallEfiDecompress (
  EFI_DECOMPRESS_PROTOCOL  **This
  )
;

EFI_STATUS
InstallTianoDecompress (
  EFI_TIANO_DECOMPRESS_PROTOCOL  **This
  )
;

EFI_STATUS
InstallCustomizedDecompress (
  EFI_CUSTOMIZED_DECOMPRESS_PROTOCOL  **This
  )
;

EFI_STATUS
EFIAPI
DxeIplDecompress (
  IN  CONST EFI_PEI_DECOMPRESS_PPI           *This,
  IN  CONST EFI_COMPRESSION_SECTION          *InputSection,
  OUT VOID                                   **OutputBuffer,
  OUT UINTN                                  *OutputSize
  )
;

VOID
SwitchStacks (
  VOID  *EntryPoint,
  UINTN Parameter,
  VOID  *NewStack,
  VOID  *NewBsp
  )
;

EFI_STATUS
EFIAPI
PeiLoadFile (
  IN  EFI_PEI_FILE_HANDLE                       FileHandle,
  OUT EFI_PHYSICAL_ADDRESS                      *ImageAddressArg,
  OUT UINT64                                    *ImageSizeArg,
  OUT EFI_PHYSICAL_ADDRESS                      *EntryPoint
  )
;

EFI_STATUS
EFIAPI
DxeLoadCore (
  IN EFI_DXE_IPL_PPI       *This,
  IN EFI_PEI_SERVICES      **PeiServices,
  IN EFI_PEI_HOB_POINTERS  HobList
  )
;

EFI_STATUS
PeCoffLoaderGetImageInfo (
  IN     EFI_PEI_PE_COFF_LOADER_PROTOCOL       *This,
  IN OUT EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
;

EFI_STATUS
PeCoffLoaderRelocateImage (
  IN     EFI_PEI_PE_COFF_LOADER_PROTOCOL       *This,
  IN OUT EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
;

EFI_STATUS
PeCoffLoaderLoadImage (
  IN     EFI_PEI_PE_COFF_LOADER_PROTOCOL       *This,
  IN OUT EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
;

EFI_STATUS
CreateArchSpecificHobs (
  IN  EFI_PEI_SERVICES          **PeiServices,
  OUT EFI_PHYSICAL_ADDRESS      *BspStore
  )
;

#endif
