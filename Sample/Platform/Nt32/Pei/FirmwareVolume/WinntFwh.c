/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    WinNtFwh.c
    
Abstract:

    EFI 2.0 PEIM to abstract construction of firmware volume in a Windows NT environment.

Revision History

--*/
#include "Tiano.h"
#include "FlashLayout.h"
#include "Pei.h"
#include "PeiLib.h"

#include EFI_PPI_DEFINITION (NtFwh)


EFI_STATUS
PeimInitializeWinNtFwh (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  );

EFI_PEIM_ENTRY_POINT(PeimInitializeWinNtFwh);

EFI_STATUS
PeimInitializeWinNtFwh (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Perform a call-back into the SEC simulator to get address of the Firmware Hub

Arguments:

  PeiServices - General purpose services available to every PEIM.
    
Returns:

  None

--*/
{
  EFI_STATUS                          Status;
  EFI_PEI_PPI_DESCRIPTOR                  *PpiDescriptor;
  PEI_NT_FWH_CALLBACK_PROTOCOL        *PeiNtService;
  UINT64                              FwhSize;
  EFI_PHYSICAL_ADDRESS                FwhBase;

  Status =  (**PeiServices).LocatePpi ( PeiServices,
                                        &gPeiFwhInformationGuid,     // GUID
                                        0,                            // INSTANCE
                                        &PpiDescriptor,               // EFI_PEI_PPI_DESCRIPTOR
                                        &PeiNtService                 // PPI
                                        );
  ASSERT_PEI_ERROR (PeiServices, Status);  

  Status = PeiNtService->NtFwh (&FwhSize, &FwhBase);

  ASSERT_PEI_ERROR (PeiServices, Status);  

  Status =  PeiBuildHobFv (
               PeiServices,
               FwhBase,  //BaseAddress,
               FwhSize   //Length
              );
  ASSERT_PEI_ERROR (PeiServices, Status);

  Status =  PeiBuildHobResourceDescriptor (
               PeiServices,
               EFI_RESOURCE_FIRMWARE_DEVICE,
               (EFI_RESOURCE_ATTRIBUTE_PRESENT    |
               EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
               EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
               FwhBase,
               (FwhSize                           +
               EFI_WINNT_RUNTIME_UPDATABLE_LENGTH +
               EFI_WINNT_FTW_SPARE_BLOCK_LENGTH   )
              );
  ASSERT_PEI_ERROR (PeiServices, Status);

  FwhBase = FwhBase + EFI_WINNT_RUNTIME_UPDATABLE_OFFSET;
  FwhSize = EFI_WINNT_RUNTIME_UPDATABLE_LENGTH + EFI_WINNT_FTW_SPARE_BLOCK_LENGTH;

  Status =  PeiBuildHobFv (
               PeiServices,
               FwhBase,  //BaseAddress,
               FwhSize   //Length
              );
  ASSERT_PEI_ERROR (PeiServices, Status);

  return Status;
}

