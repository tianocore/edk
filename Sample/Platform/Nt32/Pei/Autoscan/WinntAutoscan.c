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

static EFI_PEI_PPI_DESCRIPTOR mPpiListMemoryDiscovered = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiMemoryDiscoveredPpiGuid,
  NULL
};

EFI_STATUS
EFIAPI
PeimInitializeWinNtAutoScan (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  );

EFI_PEIM_ENTRY_POINT (PeimInitializeWinNtAutoScan)

EFI_STATUS
EFIAPI
PeimInitializeWinNtAutoScan (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:
  Perform a call-back into the SEC simulator to get a memory value

Arguments:
  FfsHeader   - General purpose data available to every PEIM
  PeiServices - General purpose services available to every PEIM.
    
Returns:
  None

--*/
{
  EFI_STATUS                  Status;
  EFI_PEI_PPI_DESCRIPTOR      *PpiDescriptor;
  PEI_NT_AUTOSCAN_PPI         *PeiNtService;
  UINT64                      MemorySize;
  EFI_PHYSICAL_ADDRESS        MemoryBase;
  PEI_BASE_MEMORY_TEST_PPI    *MemoryTestPpi;
  EFI_PHYSICAL_ADDRESS        ErrorAddress;
  UINTN                       Index;
  EFI_RESOURCE_ATTRIBUTE_TYPE Attributes;

  //
  // Get the PEI NT Autoscan PPI
  //
  Status = (**PeiServices).LocatePpi (
                            PeiServices,
                            &gPeiNtAutoScanPpiGuid, // GUID
                            0,                      // INSTANCE
                            &PpiDescriptor,         // EFI_PEI_PPI_DESCRIPTOR
                            &PeiNtService           // PPI
                            );
  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // Get the Memory Test PPI
  //
  Status = (**PeiServices).LocatePpi (
                            PeiServices,
                            &gPeiBaseMemoryTestPpiGuid,
                            0,
                            NULL,
                            &MemoryTestPpi
                            );
  ASSERT_PEI_ERROR (PeiServices, Status);

  Index = 0;
  do {
    Status = PeiNtService->NtAutoScan (Index, &MemoryBase, &MemorySize);
    if (!EFI_ERROR (Status)) {
      Attributes =
        (
          EFI_RESOURCE_ATTRIBUTE_PRESENT |
          EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
          EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
          EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
          EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
          EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE
        );

      if (Index == 0) {
        //
        // For the first area register it as PEI tested memory
        //
        Status = MemoryTestPpi->BaseMemoryTest (
                                  PeiServices,
                                  MemoryTestPpi,
                                  MemoryBase,
                                  MemorySize,
                                  Quick,
                                  &ErrorAddress
                                  );
        ASSERT_PEI_ERROR (PeiServices, Status);

        //
        // Register the "tested" memory with the PEI Core
        //
        Status = (**PeiServices).InstallPeiMemory (PeiServices, MemoryBase, MemorySize);
        ASSERT_PEI_ERROR (PeiServices, Status);

        Attributes |= EFI_RESOURCE_ATTRIBUTE_TESTED;
      }

      Status = PeiBuildHobResourceDescriptor (
                PeiServices,
                EFI_RESOURCE_SYSTEM_MEMORY,
                Attributes,
                MemoryBase,
                MemorySize
                );
      ASSERT_PEI_ERROR (PeiServices, Status);
    }

    Index++;
  } while (!EFI_ERROR (Status));

  Status = (**PeiServices).InstallPpi (PeiServices, &mPpiListMemoryDiscovered);

  //
  // Build the CPU hob with 36-bit addressing and 16-bits of IO space.
  //
  Status = PeiBuildHobCpu (PeiServices, 36, 16);

  return Status;
}
