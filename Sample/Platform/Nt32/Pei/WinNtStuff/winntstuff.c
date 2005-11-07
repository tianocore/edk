/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    WinNtStuff.c
    
Abstract:

    Tiano PEIM to abstract construction of firmware volume in a Windows NT environment.

Revision History

--*/

#include "Tiano.h"
#include "Pei.h"
#include "PeiLib.h"
#include EFI_PPI_DEFINITION (NtThunk)

#define EFI_PEI_WIN_NT_THUNK_PROTOCOL_GUID \
  { \
    0x58c518b1, 0x76f3, 0x11d4, 0xbc, 0xea, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 \
  }

EFI_STATUS
EFIAPI
PeimInitializeWinNtStuff (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  );

EFI_GUID  mEfiPeiWinNtThunkProtocolGuid = EFI_PEI_WIN_NT_THUNK_PROTOCOL_GUID;

EFI_PEIM_ENTRY_POINT (PeimInitializeWinNtStuff)

EFI_STATUS
EFIAPI
PeimInitializeWinNtStuff (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Perform a call-back into the SEC simulator to get NT Stuff

Arguments:

  PeiServices - General purpose services available to every PEIM.
    
Returns:

  None

--*/
// TODO:    FfsHeader - add argument and description to function comment
{
  EFI_STATUS              Status;
  EFI_PEI_PPI_DESCRIPTOR  *PpiDescriptor;
  PEI_NT_THUNK_PPI        *PeiNtService;
  UINT64                  InterfaceSize;
  EFI_PHYSICAL_ADDRESS    InterfaceBase;

  Status = (**PeiServices).LocatePpi (
                            PeiServices,
                            &gPeiNtThunkPpiGuid,  // GUID
                            0,                    // INSTANCE
                            &PpiDescriptor,       // EFI_PEI_PPI_DESCRIPTOR
                            &PeiNtService         // PPI
                            );
  ASSERT_PEI_ERROR (PeiServices, Status);

  Status = PeiNtService->NtThunk (&InterfaceSize, &InterfaceBase);
  ASSERT_PEI_ERROR (PeiServices, Status);

  Status = PeiBuildHobGuidData (
            PeiServices,
            &mEfiPeiWinNtThunkProtocolGuid,       // Guid
            &InterfaceBase,                       // Buffer
            (UINTN) InterfaceSize                 // BufferSize
            );
  return Status;
}
