/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
    Exception.h
    
Abstract:

    IA32 Exception Includes.

Revision History

--*/

#ifndef _IA32EXCEPTION_H
#define _IA32EXCEPTION_H


//
// Driver Consumed Protocol Prototypes
//
#include EFI_ARCH_PROTOCOL_DEFINITION (Cpu)

#define INTERRUPT_HANDLER_DIVIDE_ZERO             0x00
#define INTERRUPT_HANDLER_DEBUG                   0x01
#define INTERRUPT_HANDLER_NMI                     0x02
#define INTERRUPT_HANDLER_BREAKPOINT              0x03
#define INTERRUPT_HANDLER_OVERFLOW                0x04
#define INTERRUPT_HANDLER_BOUND                   0x05
#define INTERRUPT_HANDLER_INVALID_OPCODE          0x06
#define INTERRUPT_HANDLER_DEVICE_NOT_AVAILABLE    0x07
#define INTERRUPT_HANDLER_TIMER                   0x08
#define INTERRUPT_HANDLER_COPROCESSOR_OVERRUN     0x09
#define INTERRUPT_HANDLER_INVALID_TSS             0x0A
#define INTERRUPT_HANDLER_SEGMENT_NOT_PRESENT     0x0B
#define INTERRUPT_HANDLER_STACK_SEGMENT_FAULT     0x0C
#define INTERRUPT_HANDLER_GP_FAULT                0x0D
#define INTERRUPT_HANDLER_PAGE_FAULT              0x0E
#define INTERRUPT_HANDLER_RESERVED                0x0F
#define INTERRUPT_HANDLER_MATH_FAULT              0x10
#define INTERRUPT_HANDLER_ALIGNMENT_FAULT         0x11
#define INTERRUPT_HANDLER_MACHINE_CHECK           0x12
#define INTERRUPT_HANDLER_STREAMING_SIMD          0x13

//
// Register Structure Definitions
//
typedef struct {
  UINT32         Eax;
  UINT32         Ebx;
  UINT32         Ecx;
  UINT32         Edx;
  UINT32         Esp;
  UINT32         Ebp;
  UINT16         Cs;
  UINT16         Ds;
  UINT16         Es;
  UINT16         Fs;
  UINT16         Gs;
  UINT16         Ss;
  UINT32         Esi;
  UINT32         Edi;
  UINT32         EFlags;
} CPU_REGISTERS;


VOID
EFIAPI
CommonExceptionHandler (
  IN EFI_EXCEPTION_TYPE   InterruptType, 
  IN EFI_SYSTEM_CONTEXT   SystemContext
  );

EFI_STATUS
InitializeException (
  IN  EFI_CPU_ARCH_PROTOCOL *CpuProtocol
  );


#endif
