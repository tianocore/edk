/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    plDebugSupport.h

Abstract:

    IA32 specific debug support macros, typedefs and prototypes.

Revision History

--*/

#ifndef _PLDEBUG_SUPPORT_H
#define _PLDEBUG_SUPPORT_H

#include "Tiano.h"
#include EFI_PROTOCOL_DEFINITION (DebugSupport)

#define NUM_IDT_ENTRIES                 0x78
#define SYSTEM_TIMER_VECTOR             0x68
#define VECTOR_ENTRY_PAGES              1
#define CopyDescriptor(Dest, Src)       EfiCopyMem ((Dest), (Src), sizeof (DESCRIPTOR))
#define ZeroDescriptor(Dest)            CopyDescriptor ((Dest), &NullDesc)
#define ReadIdt(Vector, Dest)           CopyDescriptor ((Dest), &((GetIdtr())[(Vector)]))
#define WriteIdt(Vector, Src)           CopyDescriptor (&((GetIdtr())[(Vector)]), (Src))
#define CompareDescriptor(Desc1, Desc2) EfiCompareMem ((Desc1), (Desc2), sizeof (DESCRIPTOR))
#define EFI_ISA                         IsaIa32
#define FF_FXSR                         (1 << 24)

typedef UINT64 DESCRIPTOR;

typedef struct {
  DESCRIPTOR                OrigDesc;
  VOID                      (*OrigVector)(VOID);
  DESCRIPTOR                NewDesc;
  VOID                      (*StubEntry) (VOID);
  VOID                      (*RegisteredCallback) ();
} IDT_ENTRY;

extern EFI_SYSTEM_CONTEXT   SystemContext;
extern UINT8                InterruptEntryStub[];
extern UINT32               StubSize;
extern VOID                 (*OrigVector) (VOID);

VOID
CommonIdtEntry (
  VOID
  );

BOOLEAN
FxStorSupport (
  VOID
  );

DESCRIPTOR *
GetIdtr (
  VOID
  );

VOID
Vect2Desc (
  DESCRIPTOR * DestDesc,
  VOID (*Vector) (VOID)
  );

BOOLEAN
WriteInterruptFlag (
  BOOLEAN NewState
  );

EFI_STATUS
plInitializeDebugSupportDriver (
  VOID
  );

EFI_STATUS
plUnloadDebugSupportDriver (
  IN EFI_HANDLE                       ImageHandle
  );

//
// DebugSupport protocol member functions
//
EFI_STATUS
GetMaximumProcessorIndex (
  IN EFI_DEBUG_SUPPORT_PROTOCOL       *This,
  OUT UINTN                           *MaxProcessorIndex
  );
  
EFI_STATUS
RegisterPeriodicCallback (
  IN EFI_DEBUG_SUPPORT_PROTOCOL       *This,
  IN UINTN                            ProcessorIndex,
  IN EFI_PERIODIC_CALLBACK            PeriodicCallback
  );

EFI_STATUS
RegisterExceptionCallback (
  IN EFI_DEBUG_SUPPORT_PROTOCOL       *This,
  IN UINTN                            ProcessorIndex,
  IN EFI_EXCEPTION_CALLBACK           NewCallback,
  IN EFI_EXCEPTION_TYPE               ExceptionType
  );

EFI_STATUS
InvalidateInstructionCache (
  IN EFI_DEBUG_SUPPORT_PROTOCOL       *This,
  IN UINTN                            ProcessorIndex,
  IN VOID                             *Start,
  IN UINT64                           Length
  );

#endif
