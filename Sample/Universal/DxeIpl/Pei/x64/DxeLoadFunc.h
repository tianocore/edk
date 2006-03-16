/*++

Copyright 2004 - 2005, Intel Corporation                                                         
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

#ifndef _PEI_DXELOAD_FUNC_H_
#define _PEI_DXELOAD_FUNC_H_

#include "PeiHob.h"
#include "PeiLib.h"
#include "EfiFirmwareVolumeHeader.h"
#include "EfiFirmwareFileSystem.h"
#include "pei.h"

//
// Imported Interfaces
//
#include EFI_GUID_DEFINITION (PeiTransferControl)
#include EFI_GUID_DEFINITION (FirmwareFileSystem)
#include EFI_PPI_DEFINITION (RecoveryModule)
#include EFI_PPI_DEFINITION (S3Resume)
#include EFI_PPI_DEFINITION (SectionExtraction)
#include EFI_PPI_DEFINITION (Security)
#include EFI_PPI_DEFINITION (PeiInMemory)
#include EFI_PPI_DEFINITION (LoadFile)

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



EFI_STATUS
PeiLoadx64File (
  IN  EFI_PEI_SERVICES                          **PeiServices,
  IN  EFI_PEI_PE_COFF_LOADER_PROTOCOL           *PeiEfiPeiPeCoffLoader,
  IN  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  *PeiEfiPeiFlushInstructionCache,
  IN  VOID                                      *Pe32Data,
  IN  EFI_MEMORY_TYPE                           MemoryType,
  OUT EFI_PHYSICAL_ADDRESS                      *ImageAddress,
  OUT UINT64                                    *ImageSize,
  OUT EFI_PHYSICAL_ADDRESS                      *EntryPoint
  )
;


EFI_PHYSICAL_ADDRESS
CreateIdentityMappingPageTables (
  IN EFI_PEI_SERVICES      **PeiServices,
  IN UINT32 NumberOfProcessorPhysicalAddressBits
  )
 ;

 EFI_PHYSICAL_ADDRESS
AllocateZeroedHobPages (
  IN  EFI_PEI_SERVICES       **PeiServices,
  IN  UINTN   NumberOfPages
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

#endif
