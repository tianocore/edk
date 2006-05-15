/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
    Exception.c
    
Abstract:

    IA-32 Exception Handler.


--*/

#include "CpuDxe.h"


typedef
VOID
(*EFI_INSTALL_EXCEPTION) (
  IN UINT32  InterruptType, 
  IN VOID    *SystemContext
  );

typedef struct {
  UINT32        ErrorMessage;
  UINT8         Interrupt;
} EFI_EXCEPTION_HANDLER;

//
// Global variable to avoid stack adjustment on entry to handlers.
//

//
// Local Table.
//
EFI_EXCEPTION_HANDLER mExceptionTable[] = {
          { EFI_SW_EC_IA32_DIVIDE_ERROR,     INTERRUPT_HANDLER_DIVIDE_ZERO },
          { EFI_SW_EC_IA32_DEBUG,            INTERRUPT_HANDLER_DEBUG },
          { EFI_SW_EC_IA32_NMI,              INTERRUPT_HANDLER_NMI },
          { EFI_SW_EC_IA32_BREAKPOINT,       INTERRUPT_HANDLER_BREAKPOINT },
          { EFI_SW_EC_IA32_OVERFLOW,         INTERRUPT_HANDLER_OVERFLOW },
          { EFI_SW_EC_IA32_BOUND,            INTERRUPT_HANDLER_BOUND }, 
          { EFI_SW_EC_IA32_INVALID_OPCODE,   INTERRUPT_HANDLER_INVALID_OPCODE },
//
// Interrupt 7, 9, 15 not defined in the debug support protocol. Hence no status codes for them!
//
// Yup, we skipped Interrupt 8 (timer)
//
//          { DoubleFaultHandler, INTERRUPT_HANDLER_TIMER },
          { EFI_SW_EC_IA32_INVALID_TSS,      INTERRUPT_HANDLER_INVALID_TSS },        
          { EFI_SW_EC_IA32_SEG_NOT_PRESENT,  INTERRUPT_HANDLER_SEGMENT_NOT_PRESENT },
          { EFI_SW_EC_IA32_STACK_FAULT,      INTERRUPT_HANDLER_STACK_SEGMENT_FAULT },
          { EFI_SW_EC_IA32_GP_FAULT,         INTERRUPT_HANDLER_GP_FAULT },           
          { EFI_SW_EC_IA32_PAGE_FAULT,       INTERRUPT_HANDLER_PAGE_FAULT },         
          { EFI_SW_EC_IA32_FP_ERROR,         INTERRUPT_HANDLER_MATH_FAULT },         
          { EFI_SW_EC_IA32_ALIGNMENT_CHECK,  INTERRUPT_HANDLER_ALIGNMENT_FAULT },    
          { EFI_SW_EC_IA32_MACHINE_CHECK,    INTERRUPT_HANDLER_MACHINE_CHECK },
          { EFI_SW_EC_IA32_SIMD,             INTERRUPT_HANDLER_STREAMING_SIMD }
          };

UINTN     mExceptionNumber = sizeof (mExceptionTable) / sizeof (EFI_EXCEPTION_HANDLER);


typedef struct {
  EFI_STATUS_CODE_DATA    Header;
  CPU_REGISTERS           CpuRegisters;
} CPU_STATUS_CODE_TEMPLATE;

CPU_STATUS_CODE_TEMPLATE  mStatusCodeData  =  {
  {
    sizeof (EFI_STATUS_CODE_DATA),
    sizeof (CPU_REGISTERS),
    EFI_STATUS_CODE_DATA_TYPE_EXCEPTION_HANDLER_GUID
  },
  {
    0
  }
};

VOID
EFIAPI
CommonExceptionHandler (
  IN EFI_EXCEPTION_TYPE   InterruptType, 
  IN EFI_SYSTEM_CONTEXT   SystemContext
  )
{
  UINT32                  ErrorMessage;
  UINT32                  Index;

  EfiCopyMem (&mStatusCodeData.CpuRegisters, SystemContext.SystemContextIa32, sizeof (EFI_SYSTEM_CONTEXT));

  ErrorMessage = EFI_SOFTWARE_DXE_BS_DRIVER;
  for (Index = 0; Index < mExceptionNumber; Index++) {
    if (mExceptionTable[Index].Interrupt == InterruptType) {
      ErrorMessage |= mExceptionTable[Index].ErrorMessage;
      break;
    }
  }
  
  EfiLibReportStatusCode (
    (EFI_ERROR_CODE | EFI_ERROR_UNRECOVERED),
    EFI_SOFTWARE_UNSPECIFIED | ErrorMessage,
    0,
    &gEfiCallerIdGuid,
    (EFI_STATUS_CODE_DATA *)&mStatusCodeData
    );

  //
  // We died so stay here.
  //
  for (;;);
}



EFI_STATUS
InitializeException (
  IN  EFI_CPU_ARCH_PROTOCOL *CpuProtocol
  )
/*++

Routine Description:

  Install the IA-32 Exception Handler.
  The current operation (which likely will change) will uninstall all the
  pertinent exception handlers (0-7, 10-14, 16-19) except for Int8 which the timer
  is currently sitting on (or soon will be).  

  It then installs all the appropriate handlers for each exception.

  The handler then calls gRT->ReportStatusCode with a specific progress code.  The
  progress codes for now start at 0x200 for IA-32 processors. See Status Code
  Specification for details. The Status code Specification uses the enumeration from
  the EFI 1.1 Debug Support Protocol.
Arguments:

Returns:

--*/
{
  EFI_STATUS                      Status;
  UINT32                          Index;

  Status = CpuProtocol->DisableInterrupt (CpuProtocol);

  for (Index = 0; Index < mExceptionNumber; Index++) {
    //
    // Add in our handler.
    //
    Status = CpuProtocol->RegisterInterruptHandler (CpuProtocol, mExceptionTable[Index].Interrupt, CommonExceptionHandler);
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}




