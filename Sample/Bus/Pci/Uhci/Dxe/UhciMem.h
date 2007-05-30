/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  UhciMem.h
    
Abstract: 

  The definition for UHCI memory operation routines.

Revision History

--*/

#ifndef _EFI_UHCI_MEM_H_
#define _EFI_UHCI_MEM_H_

#define NORMAL_MEMORY_BLOCK_UNIT_IN_PAGES 1

typedef struct _MEMORY_MANAGE_HEADER {
  UINT8                         *BitArrayPtr;
  UINTN                         BitArraySizeInBytes;
  UINT8                         *MemoryBlockPtr;
  UINTN                         MemoryBlockSizeInBytes;
  VOID                          *Mapping;
  struct _MEMORY_MANAGE_HEADER  *Next;
} MEMORY_MANAGE_HEADER;

EFI_STATUS
InitializeMemoryManagement (
  IN USB_HC_DEV     *HcDev
  )
/*++

Routine Description:

  Initialize Memory Management

Arguments:

  HcDev - USB_HC_DEV

Returns:

  EFI_SUCCESS -  Success
--*/
;

EFI_STATUS
UhciAllocatePool (
  IN  USB_HC_DEV     *HcDev,
  OUT UINT8          **Pool,
  IN  UINTN          AllocSize
  )
/*++

Routine Description:

  Uhci Allocate Pool

Arguments:

  HcDev     - USB_HC_DEV
  Pool      - Place to store pointer to the memory buffer
  AllocSize - Alloc Size

Returns:

  EFI_SUCCESS - Success

--*/
;

VOID
UhciFreePool (
  IN USB_HC_DEV     *HcDev,
  IN UINT8          *Pool,
  IN UINTN          AllocSize
  )
/*++

Routine Description:

  Uhci Free Pool

Arguments:

  HcDev     - USB_HC_DEV
  Pool      - Pool to free
  AllocSize - Pool size

Returns:

  VOID

--*/
;

EFI_STATUS
DelMemoryManagement (
  IN USB_HC_DEV     *HcDev
  )
/*++

Routine Description:

  Delete Memory Management

Arguments:

  HcDev - USB_HC_DEV

Returns:

  EFI_SUCCESS - Success

--*/
;

#endif
