/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  reclaim.h
  
Abstract:
  
  Definitions for non-volatile variable store garbage collection

Revision History

--*/

#ifndef _VAR_RECLAIM_H
#define _VAR_RECLAIM_H

#include "Tiano.h"
#include "EfiVariable.h"
#include "EfiRuntimeLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_ARCH_PROTOCOL_DEFINITION (Variable)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeBlock)
#include EFI_PROTOCOL_DEFINITION (FaultTolerantWriteLite)

//
// Functions
//
EFI_STATUS
GetFvbHandleByAddress (
  IN  EFI_PHYSICAL_ADDRESS   VariableStoreBase,
  OUT EFI_HANDLE             *FvbHandle
  )
;

EFI_STATUS
FtwVariableSpace (
  IN EFI_PHYSICAL_ADDRESS   VariableBaseAddress,
  IN UINT8                  *Buffer,
  IN UINTN                  BufferSize
  )
;

#endif
