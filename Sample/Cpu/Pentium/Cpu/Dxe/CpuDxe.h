/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  CpuDxe.h

Abstract:

  Private data structures


--*/
#ifndef _CPU_DXE_H
#define _CPU_DXE_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "CpuIA32.h"
#include "Exception.h"
#include "EfiStatusCode.h"

#include EFI_ARCH_PROTOCOL_DEFINITION (Cpu)
#include EFI_PROTOCOL_DEFINITION (LegacyBiosThunk)
#include EFI_PROTOCOL_CONSUMER (Legacy8259)

#include EFI_GUID_DEFINITION (StatusCode)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)

#define INTERRUPT_VECTOR_NUMBER   256
#define INTERRUPT_GATE_ATTRIBUTE  0x8e00


extern VOID InitializeSelectors ();


typedef struct {
  VOID           *Start;
  UINTN          Size;
  UINTN          FixOffset;
} INTERRUPT_HANDLER_TEMPLATE_MAP;


//
// Function declarations
//

VOID
InitializeInterruptTables (
  VOID
  );


//
// Structures
//
typedef struct _CPU_ARCH_PROTOCOL_PRIVATE {
  EFI_HANDLE                  Handle;

  EFI_CPU_ARCH_PROTOCOL       Cpu;

  //
  // Local Data for CPU interface goes here
  //

} CPU_ARCH_PROTOCOL_PRIVATE;

#define CPU_ARCH_PROTOCOL_PRIVATE_DATA_FROM_THIS(a) \
         CR(a, CPU_ARCH_PROTOCOL_PRIVATE, Cpu, CPU_ARCH_PROT_PRIVATE_SIGNATURE)


//
// Function declarations
//
EFI_STATUS
EFIAPI
InitializeCpu (
  IN EFI_HANDLE                       ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
  );

VOID 
InitializeIdt (
  IN  EFI_CPU_INTERRUPT_HANDLER       *TableStart,
  IN  UINTN                           *IdtTablePtr,
  IN  UINT16                          IdtTableLimit
  );

EFI_STATUS
EFIAPI
CpuFlushCpuDataCache (
  IN EFI_CPU_ARCH_PROTOCOL           *This,
  IN EFI_PHYSICAL_ADDRESS            Start,
  IN UINT64                          Length,
  IN EFI_CPU_FLUSH_TYPE              FlushType
  );

EFI_STATUS
EFIAPI
CpuEnableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL           *This
  );

EFI_STATUS
EFIAPI
CpuDisableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL           *This
  );

EFI_STATUS
EFIAPI
CpuGetInterruptState (
  IN  EFI_CPU_ARCH_PROTOCOL         *This,
  OUT BOOLEAN                       *State
  );

EFI_STATUS
EFIAPI
CpuInit (
  IN EFI_CPU_ARCH_PROTOCOL          *This,
  IN EFI_CPU_INIT_TYPE               InitType
  );

EFI_STATUS
EFIAPI
CpuRegisterInterruptHandler (
  IN EFI_CPU_ARCH_PROTOCOL          *This,
  IN EFI_EXCEPTION_TYPE             InterruptType,
  IN EFI_CPU_INTERRUPT_HANDLER      InterruptHandler
  );

EFI_STATUS
EFIAPI
CpuGetTimerValue (
  IN  EFI_CPU_ARCH_PROTOCOL          *This,
  IN  UINT32                         TimerIndex,
  OUT UINT64                         *TimerValue,
  OUT UINT64                         *TimerPeriod   OPTIONAL
  );
    
EFI_STATUS
EFIAPI
CpuSetMemoryAttributes(
  IN  EFI_CPU_ARCH_PROTOCOL           *This,
  IN  EFI_PHYSICAL_ADDRESS            BaseAddress,
  IN  UINT64                          Length,
  IN  UINT64                          Attributes
  );  

EFI_STATUS
CpuMtrrSetMemoryAttributes (
  IN EFI_CPU_ARCH_PROTOCOL     *This,
  IN EFI_PHYSICAL_ADDRESS      BaseAddress,
  IN UINT64                    Length,
  IN UINT64                    Attributes
  );

VOID
InitailizeMemoryAttributes (
  VOID
  );

BOOLEAN
EFIAPI
CpuLegacyBiosInt86 (
  IN LEGACY_BIOS_THUNK_PROTOCOL       *This,
  IN  UINT8                           BiosInt,
  IN OUT  EFI_IA32_REGISTER_SET       *Regs
  );

BOOLEAN
EFIAPI
CpuLegacyBiosFarCall86 (
  IN LEGACY_BIOS_THUNK_PROTOCOL       *This,
  IN  UINT16                          Segment,
  IN  UINT16                          Offset,
  IN  EFI_IA32_REGISTER_SET           *Regs,
  IN  VOID                            *Stack,
  IN  UINTN                           StackSize
  );

VOID
InitializeBiosIntCaller (
  VOID
  );


#endif
