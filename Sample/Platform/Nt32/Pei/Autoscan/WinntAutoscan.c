/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  WinNtAutoscan.c

Abstract:

  Tiano PEIM to abstract memory auto-scan in a Windows NT environment.

Revision History

--*/
#include "Tiano.h"
#include "Pei.h"
#include "PeiLib.h"

#include EFI_PPI_DEFINITION (NtAutoScan)
#include EFI_PPI_DEFINITION (MemoryDiscovered)
#include EFI_PPI_DEFINITION (BaseMemoryTest)

static
EFI_PEI_PPI_DESCRIPTOR mPpiListMemoryDiscovered = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiMemoryDiscoveredPpiGuid,
  NULL
};

EFI_STATUS
PeimInitializeWinNtAutoScan (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  );

#define  PEI_MEMORY_SIZE          0x4000000  
#define  PEI_CORE_MEMORY_SIZE     (PEI_MEMORY_SIZE / 2)

EFI_PEIM_ENTRY_POINT(PeimInitializeWinNtAutoScan);

EFI_STATUS
PeimInitializeWinNtAutoScan (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Perform a call-back into the SEC simulator to get a memory value

Arguments:

  PeiServices - General purpose services available to every PEIM.
    
Returns:

  None

--*/
{
  EFI_STATUS                          Status;
  EFI_PEI_PPI_DESCRIPTOR                  *PpiDescriptor;
  PEI_NT_AUTOSCAN_CALLBACK_PROTOCOL   *PeiNtService;
  UINT64                              MemorySize;
  EFI_PHYSICAL_ADDRESS                MemoryBase;
  EFI_BOOT_MODE                       BootMode;
  PEI_BASE_MEMORY_TEST_PPI            *MemoryTestPpi;
  EFI_PHYSICAL_ADDRESS                ErrorAddress;

  Status =  (**PeiServices).LocatePpi ( PeiServices,
                                        &gPeiAutoScanGuid,    // GUID
                                        0,                    // INSTANCE
                                        &PpiDescriptor,       // EFI_PEI_PPI_DESCRIPTOR
                                        &PeiNtService         // PPI
                                        );
  ASSERT_PEI_ERROR (PeiServices, Status);  

  MemorySize = PEI_MEMORY_SIZE;

  Status = PeiNtService->NtAutoScan ( &MemorySize,
                                      &MemoryBase
                                    );

  ASSERT_PEI_ERROR (PeiServices, Status);  

  Status = (**PeiServices).GetBootMode (PeiServices, &BootMode); //EFI_BREAKPOINT();
  //
  // Test the "discovered" memory
  // Herein, discovered is something known a priori.  We should
  // go back to using the environment variable at some point.
  //
  Status =  (**PeiServices).LocatePpi (
                              PeiServices,
                              &gPeiBaseMemoryTestPpiGuid,
                              0,
                              NULL,
                              &MemoryTestPpi
                              );

  ASSERT_PEI_ERROR (PeiServices, Status);

  Status = MemoryTestPpi->BaseMemoryTest (
                              PeiServices,
                              MemoryTestPpi,
                              (MemoryBase + MemorySize - PEI_CORE_MEMORY_SIZE),
                              PEI_CORE_MEMORY_SIZE,
                              Quick,
                              &ErrorAddress
                              );

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Register the "tested" memory with the PEI Core
  //

  Status =  (**PeiServices).InstallPeiMemory (
                               PeiServices,
                               (MemoryBase + MemorySize - PEI_CORE_MEMORY_SIZE),
                               PEI_CORE_MEMORY_SIZE
                               );

  ASSERT_PEI_ERROR (PeiServices, Status);  

  //
  // Describe all of the memory for the subsequent phase of execution
  //

  Status =  PeiBuildHobResourceDescriptor (
               PeiServices,
               EFI_RESOURCE_SYSTEM_MEMORY,
               (EFI_RESOURCE_ATTRIBUTE_PRESENT                |
               EFI_RESOURCE_ATTRIBUTE_INITIALIZED             |
               EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             |
               EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       |
               EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
               EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),
               MemoryBase,                 
               (MemorySize - PEI_CORE_MEMORY_SIZE)
               );

  ASSERT_PEI_ERROR (PeiServices, Status);

  Status =  PeiBuildHobResourceDescriptor (
               PeiServices,
               EFI_RESOURCE_SYSTEM_MEMORY,
               (EFI_RESOURCE_ATTRIBUTE_PRESENT                |
               EFI_RESOURCE_ATTRIBUTE_INITIALIZED             |
               EFI_RESOURCE_ATTRIBUTE_TESTED                  |
               EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             |
               EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       |
               EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
               EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE),
               (MemoryBase + MemorySize - PEI_CORE_MEMORY_SIZE),
               PEI_CORE_MEMORY_SIZE
               );

  ASSERT_PEI_ERROR (PeiServices, Status);

//  Status = (**PeiServices).InstallPpi (PeiServices, &mPpiListMemoryDiscovered);
//
  Status = PeiBuildHobCpu (PeiServices, 36, 16);

  return Status;
}

