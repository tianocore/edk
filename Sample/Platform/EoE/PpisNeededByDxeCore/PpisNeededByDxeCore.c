/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  PpisNeededByDxeCore.c

Abstract:
  
Revision History:

--*/

#include "Tiano.h"
#include "PeiLib.h"
#include "PeiHob.h"
#include "EfiHobLib.h"
#include "EfiCommonLib.h"
#include "SerialStatusCode.h"

#include EFI_PROTOCOL_DEFINITION (Decompress)
#include EFI_PROTOCOL_DEFINITION (TianoDecompress)
#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)
#include EFI_GUID_DEFINITION (PeiFlushInstructionCache)
#include EFI_GUID_DEFINITION (PeiPeCoffLoader)
#include EFI_GUID_DEFINITION (PeiTransferControl)


#define SIZE_OF_GUID_HOB_WITH_POINTER (sizeof (EFI_HOB_GUID_TYPE) + sizeof (VOID *))

EFI_STATUS
InstallEfiPeiTransferControl (
  IN OUT EFI_PEI_TRANSFER_CONTROL_PROTOCOL **This
  );

EFI_STATUS
InstallEfiPeiFlushInstructionCache (
  IN OUT EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  **This
  );



EFI_HOB_HANDOFF_INFO_TABLE    *gPhit;


BOOLEAN
UpdateProtocolHob (
  IN  EFI_GUID    *Guid,
  IN  VOID        *Interface
  )
/*++

Routine Description:
  
Arguments:
  
Returns:

--*/
{
  VOID                  *HobStart;
  EFI_PEI_HOB_POINTERS  GuidHob;
  UINTN                 *InterfaceAddress;

  HobStart = gPhit + 1;
  while (TRUE) {

    GuidHob.Raw = HobStart;
    if (END_OF_HOB_LIST (GuidHob)) {
      return FALSE;
    }

    if (GuidHob.Header->HobType == EFI_HOB_TYPE_GUID_EXTENSION) {
      if (EfiCompareGuid (Guid, &GuidHob.Guid->Name)) {
        InterfaceAddress = (UINTN *)(UINTN)GuidHob.Guid++;
        *InterfaceAddress = (UINTN)Interface;
        return TRUE;
      }
    }

    HobStart = GET_NEXT_HOB (GuidHob);
  }
}


EFI_STATUS
BuildProtocolHob (
  IN  EFI_GUID    *Guid,
  IN  VOID        *Interface
  )
{
  EFI_HOB_GUID_TYPE  *Hob;
  UINTN              *Pointer;

  if (UpdateProtocolHob (Guid, Interface)) {
    //
    // If this HOB type already exists update it and return.
    //
    return EFI_SUCCESS;
  }

  //
  // This protocol hob did not exist so add one.
  //

  if (gPhit->EfiFreeMemoryTop - gPhit->EfiFreeMemoryBottom < SIZE_OF_GUID_HOB_WITH_POINTER) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Shift the End HOB to the right locaiton
  //
  EfiCommonLibCopyMem (
    (VOID *)(UINTN)(gPhit->EfiEndOfHobList + SIZE_OF_GUID_HOB_WITH_POINTER),
    (VOID *)(UINTN)(gPhit->EfiEndOfHobList),
    sizeof (EFI_HOB_GENERIC_HEADER)
    );

  Hob = (EFI_HOB_GUID_TYPE *)(UINTN)gPhit->EfiEndOfHobList;
  Hob->Header.HobType   = EFI_HOB_TYPE_GUID_EXTENSION;
  Hob->Header.HobLength = SIZE_OF_GUID_HOB_WITH_POINTER;
  Hob->Header.Reserved  = 0;
  
  EfiCommonLibCopyMem (&Hob->Name, Guid, sizeof (EFI_GUID));

  Pointer = (UINTN *) (++Hob); 
  *Pointer = ((UINTN)Interface);

  //
  // Grow Hob List from the bottom of the free memory list
  //
  gPhit->EfiEndOfHobList += SIZE_OF_GUID_HOB_WITH_POINTER;
  gPhit->EfiFreeMemoryBottom += SIZE_OF_GUID_HOB_WITH_POINTER;

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
FakePeiEntry (
  IN  VOID  *HobStart
  )
/*++

Routine Description:

  This routine adds the PPI/Protocol Hobs that are consumed by the DXE Core.
  Normally these come from PEI, but since our PEI was 32-bit we need an
  alternate source. That is this driver.

  This driver does not consume PEI or DXE services and thus updates the 
  Phit (HOB list) directly

Arguments:

  HobStart - Pointer to the beginning of the HOB List from PEI

Returns:

  This function should after it has add it's HOBs

--*/
{
  EFI_PEI_PE_COFF_LOADER_PROTOCOL           *PeCoffLoader;
  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  *FlushInstructionCache;
  EFI_PEI_TRANSFER_CONTROL_PROTOCOL         *TransferControl;
  EFI_DECOMPRESS_PROTOCOL                   *EfiDecompress;
  EFI_TIANO_DECOMPRESS_PROTOCOL             *TianoDecompress;
  EFI_REPORT_STATUS_CODE                    ReportStatusCode;

  gPhit = HobStart;

  InstallEfiPeiFlushInstructionCache (&FlushInstructionCache);
  BuildProtocolHob (&gEfiPeiFlushInstructionCacheGuid, FlushInstructionCache);

  InstallEfiPeiTransferControl (&TransferControl);
  BuildProtocolHob (&gEfiPeiTransferControlGuid, TransferControl);

  InstallEfiPeiPeCoffLoader (NULL, &PeCoffLoader, NULL);
  BuildProtocolHob (&gEfiPeiPeCoffLoaderGuid, PeCoffLoader);

  InstallEfiDecompress (&EfiDecompress);
  BuildProtocolHob (&gEfiDecompressProtocolGuid, EfiDecompress);

  InstallTianoDecompress (&TianoDecompress);
  BuildProtocolHob (&gEfiTianoDecompressProtocolGuid, TianoDecompress);

  InstallSerialStatusCode (&ReportStatusCode);
  BuildProtocolHob (&gEfiStatusCodeArchProtocolGuid, (VOID *)(UINTN)ReportStatusCode);

  return EFI_SUCCESS;
}


