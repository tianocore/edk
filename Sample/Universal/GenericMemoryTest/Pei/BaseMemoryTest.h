/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  BaseMemoryTest.h
   
Abstract:

  EFI 2.0 PEIM to provide a PEI memory test service.

--*/

#ifndef _PEI_BASE_MEMORY_TEST_H_
#define _PEI_BASE_MEMORY_TEST_H_

#include "Tiano.h"
#include "Pei.h"
#include EFI_PPI_DEFINITION (CpuIo)
#include EFI_PPI_DEFINITION (BaseMemoryTest)

//Some global define
#define  COVER_SPAN           0x40000
#define  TEST_PATTERN         0x5A5A5A5A

EFI_STATUS
EFIAPI
PeiBaseMemoryTestInit(
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  );

EFI_STATUS
BaseMemoryTest (
  IN  EFI_PEI_SERVICES                   **PeiServices,
  IN PEI_BASE_MEMORY_TEST_PPI   *This, 
  IN  EFI_PHYSICAL_ADDRESS               BeginAddress,
  IN  UINT64                             MemoryLength,
  IN  PEI_MEMORY_TEST_OP                 Operation,
  OUT EFI_PHYSICAL_ADDRESS               *ErrorAddress
  );

#endif
