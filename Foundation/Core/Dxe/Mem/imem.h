/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  imem.h

Abstract:

  Head file to imem.h


Revision History

--*/

#ifndef _IMEM_H_
#define _IMEM_H_

#include "Tiano.h"
#include "DxeCore.h"
#include "Processor.h"

//
// MEMORY_MAP_ENTRY
//

#define MEMORY_MAP_SIGNATURE   EFI_SIGNATURE_32('m','m','a','p')
typedef struct {
  UINTN           Signature;
  EFI_LIST_ENTRY  Link;
  BOOLEAN         FromPool;

  EFI_MEMORY_TYPE Type;
  UINT64          Start;
  UINT64          End;

  UINT64          VirtualStart;
  UINT64          Attribute;
} MEMORY_MAP;

//
// Internal prototypes
//

VOID *
CoreAllocatePoolPages (
  IN EFI_MEMORY_TYPE   PoolType,
  IN UINTN             NumberOfPages,
  IN UINTN             Alignment
  );


VOID
CoreFreePoolPages (
  IN EFI_PHYSICAL_ADDRESS   Memory,
  IN UINTN                  NumberOfPages
  );


VOID *
CoreAllocatePoolI (
  IN EFI_MEMORY_TYPE  PoolType,
  IN UINTN            Size
  );


EFI_STATUS
CoreFreePoolI (
  IN VOID           *Buffer
  );


VOID
CoreAcquireMemoryLock (
  VOID
  );

VOID
CoreReleaseMemoryLock (
  VOID
  );


//
// Internal Global data
//

extern EFI_LOCK           gMemoryLock; 
extern EFI_LIST_ENTRY     gMemoryMap;
extern MEMORY_MAP         *gMemoryLastConvert;
extern EFI_LIST_ENTRY     mGcdMemorySpaceMap;
#endif
