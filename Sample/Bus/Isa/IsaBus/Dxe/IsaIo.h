/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  IsaIo.h
  
Abstract:
  
  The special part of header file for EFI_ISA_IO protocol implementation;
  See the common part of header file for EFI_ISA_IO_PROTOCOL and  
  EFI_LIGHT_ISA_IO_PROTOCOL in CommonIsaIo.h.
  
Revision History:

--*/

#ifndef _EFI_ISA_IO_LOCAL_H
#define _EFI_ISA_IO_LOCAL_H

#include "IsaBus.h"
#include "CommonIsaIo.h"

//
// ISA I/O Support Addtional Function Prototypes 
// Common Function Prototypes for both EFI_ISA_IO_PROTOCOL and 
// EFI_LIGHT_ISA_IO_PROTOCOL. 
// Please refer to CommonIsaIo.h
//

EFI_STATUS
EFIAPI
IsaIoMemRead (
  IN     EFI_INTERFACE_DEFINITION_FOR_ISA_IO       *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH                 Width,
  IN     UINT32                                    Offset,
  IN     UINTN                                     Count,
  IN OUT VOID                                      *Buffer
  );


EFI_STATUS
EFIAPI
IsaIoMemWrite (
  IN     EFI_INTERFACE_DEFINITION_FOR_ISA_IO        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH                  Width,
  IN     UINT32                                     Offset,
  IN     UINTN                                      Count,
  IN OUT VOID                                       *Buffer
  );

EFI_STATUS
EFIAPI
IsaIoCopyMem (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO        *This,
  IN EFI_ISA_IO_PROTOCOL_WIDTH                  Width,
  IN UINT32                                     DestOffset,
  IN UINT32                                     SrcOffset,
  IN UINTN                                      Count
  );

EFI_STATUS
EFIAPI
IsaIoAllocateBuffer (
  IN  EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This,
  IN  EFI_ALLOCATE_TYPE                    Type,
  IN  EFI_MEMORY_TYPE                      MemoryType,
  IN  UINTN                                Pages,
  OUT VOID                                 **HostAddress,
  IN  UINT64                               Attributes
  );

EFI_STATUS
EFIAPI
IsaIoFreeBuffer (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This,
  IN UINTN                                Pages,
  IN VOID                                 *HostAddress
  );

//
// Driver Support Global Variables
//
EFI_INTERFACE_DEFINITION_FOR_ISA_IO IsaIoInterface = {
  {    
    IsaIoMemRead,
    IsaIoMemWrite
  },
  {   
    IsaIoIoRead,
    IsaIoIoWrite
  },
  IsaIoCopyMem,
  IsaIoMap,
  IsaIoUnmap,
  IsaIoAllocateBuffer,
  IsaIoFreeBuffer,
  IsaIoFlush,
  NULL,
  0,
  NULL
};
#endif
