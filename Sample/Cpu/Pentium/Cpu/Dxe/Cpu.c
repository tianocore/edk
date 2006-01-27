/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Cpu.c

Abstract:
  Architectural Protocol Driver as defined in EFI 2.0.

--*/

#include "CpuDxe.h"

VOID	
InitializeExternalVectorTablePtr(
	EFI_CPU_INTERRUPT_HANDLER* VectorTable
);	
//
// Global Variables
//

EFI_CPU_INTERRUPT_HANDLER   gExternalVectorTable[0x100]={0};

BOOLEAN                     mInterruptState = FALSE;
EFI_LEGACY_8259_PROTOCOL    *gLegacy8259 = NULL;

//
// The Cpu Architectural Protocol that this Driver produces
//
EFI_HANDLE              mHandle = NULL;
EFI_CPU_ARCH_PROTOCOL   mCpu = {
  CpuFlushCpuDataCache,
  CpuEnableInterrupt,
  CpuDisableInterrupt,
  CpuGetInterruptState,
  CpuInit,
  CpuRegisterInterruptHandler,
  CpuGetTimerValue,
  CpuSetMemoryAttributes,
  1,          // NumberOfTimers
  4,          // DmaBufferAlignment
};

LEGACY_BIOS_THUNK_PROTOCOL  mLegacyBiosThunk = {
  CpuLegacyBiosInt86,
  CpuLegacyBiosFarCall86
};



EFI_STATUS
EFIAPI
CpuFlushCpuDataCache (
  IN EFI_CPU_ARCH_PROTOCOL           *This,
  IN EFI_PHYSICAL_ADDRESS            Start,
  IN UINT64                          Length,
  IN EFI_CPU_FLUSH_TYPE              FlushType
  )
/*++

Routine Description:
  Flush CPU data cache. If the instruction cache is fully coherent
  with all DMA operations then function can just return EFI_SUCCESS.

Arguments:
  This                - Protocol instance structure
  Start               - Physical address to start flushing from.
  Length              - Number of bytes to flush. Round up to chipset 
                         granularity.
  FlushType           - Specifies the type of flush operation to perform.

Returns: 

  EFI_SUCCESS           - If cache was flushed
  EFI_UNSUPPORTED       - If flush type is not supported.
  EFI_DEVICE_ERROR      - If requested range could not be flushed.

--*/
{
  if (FlushType == EfiCpuFlushTypeWriteBackInvalidate) {
    EfiWbinvd ();
    return EFI_SUCCESS;
  } else if (FlushType == EfiCpuFlushTypeInvalidate) {
    EfiInvd ();
    return EFI_SUCCESS;
  } else {
    return EFI_UNSUPPORTED;
  }
}


EFI_STATUS
EFIAPI
CpuEnableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL          *This
  )
/*++

Routine Description:
  Enables CPU interrupts.

Arguments:
  This                - Protocol instance structure

Returns: 
  EFI_SUCCESS           - If interrupts were enabled in the CPU
  EFI_DEVICE_ERROR      - If interrupts could not be enabled on the CPU.

--*/
{
  //
  // BugBug: Interrupts broken
  //
  EfiEnableInterrupts (); 

  mInterruptState  = TRUE;
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
CpuDisableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL          *This
  )
/*++

Routine Description:
  Disables CPU interrupts.

Arguments:
  This                - Protocol instance structure

Returns: 
  EFI_SUCCESS           - If interrupts were disabled in the CPU.
  EFI_DEVICE_ERROR      - If interrupts could not be disabled on the CPU.

--*/
{
  EfiDisableInterrupts ();
  
  mInterruptState = FALSE;
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
CpuGetInterruptState (
  IN  EFI_CPU_ARCH_PROTOCOL         *This,
  OUT BOOLEAN                       *State
  )
/*++

Routine Description:
  Return the state of interrupts.

Arguments:
  This                - Protocol instance structure
  State               - Pointer to the CPU's current interrupt state

Returns: 
  EFI_SUCCESS           - If interrupts were disabled in the CPU.
  EFI_INVALID_PARAMETER - State is NULL.
  
--*/
{
  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *State = mInterruptState;
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
CpuInit (
  IN EFI_CPU_ARCH_PROTOCOL           *This,
  IN EFI_CPU_INIT_TYPE               InitType
  )

/*++

Routine Description:
  Generates an INIT to the CPU

Arguments:
  This                - Protocol instance structure
  InitType            - Type of CPU INIT to perform

Returns: 
  EFI_SUCCESS           - If CPU INIT occurred. This value should never be
        seen.
  EFI_DEVICE_ERROR      - If CPU INIT failed.
  EFI_NOT_SUPPORTED     - Requested type of CPU INIT not supported.

--*/
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
CpuRegisterInterruptHandler (
  IN EFI_CPU_ARCH_PROTOCOL          *This,
  IN EFI_EXCEPTION_TYPE             InterruptType,
  IN EFI_CPU_INTERRUPT_HANDLER      InterruptHandler
  )
/*++

Routine Description:
  Registers a function to be called from the CPU interrupt handler.

Arguments:
  This                - Protocol instance structure
  InterruptType       - Defines which interrupt to hook. IA-32 valid range
                         is 0x00 through 0xFF
  InterruptHandler    - A pointer to a function of type 
      EFI_CPU_INTERRUPT_HANDLER that is called when a
      processor interrupt occurs. A null pointer
      is an error condition.

Returns: 
  EFI_SUCCESS           - If handler installed or uninstalled.
  EFI_ALREADY_STARTED   - InterruptHandler is not NULL, and a handler for 
        InterruptType was previously installed
  EFI_INVALID_PARAMETER - InterruptHandler is NULL, and a handler for 
        InterruptType was not previously installed.
  EFI_UNSUPPORTED       - The interrupt specified by InterruptType is not
        supported.

--*/
{
  if (InterruptType < 0 || InterruptType > 0xff) {
    return EFI_UNSUPPORTED;
  }
  if (InterruptHandler == NULL && gExternalVectorTable[InterruptType] == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (InterruptHandler != NULL && gExternalVectorTable[InterruptType] != NULL) {
    return EFI_ALREADY_STARTED;
  }
  
  gExternalVectorTable[InterruptType] = InterruptHandler;
  return EFI_SUCCESS;
}



EFI_STATUS
EFIAPI
CpuGetTimerValue (
  IN  EFI_CPU_ARCH_PROTOCOL          *This,
  IN  UINT32                         TimerIndex,
  OUT UINT64                         *TimerValue,
  OUT UINT64                         *TimerPeriod   OPTIONAL
  )
/*++

Routine Description:
  Returns a timer value from one of the CPU's internal timers. There is no
  inherent time interval between ticks but is a function of the CPU 
  frequency.

Arguments:
  This                - Protocol instance structure
  TimerIndex          - Specifies which CPU timer ie requested
  TimerValue          - Pointer to the returned timer value
  TimerPeriod         - 

Returns: 
  EFI_SUCCESS           - If the CPU timer count was returned.
  EFI_UNSUPPORTED       - If the CPU does not have any readable timers
  EFI_DEVICE_ERROR      - If an error occurred reading the timer.
  EFI_INVALID_PARAMETER - TimerIndex is not valid

--*/
{  
  if (TimerValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (TimerIndex == 0) {
    *TimerValue = EfiReadTsc ();
    if (TimerPeriod != NULL) {
      //
      // BugBug: Hard coded. Don't know how to do this generically
      //
      *TimerPeriod = 1000000000;
    }
    return EFI_SUCCESS;
  }
  return EFI_INVALID_PARAMETER;
}



EFI_DRIVER_ENTRY_POINT (InitializeCpu)

EFI_STATUS
EFIAPI
InitializeCpu (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Initialize the state information for the CPU Architectural Protocol

Arguments:
  ImageHandle of the loaded driver
  Pointer to the System Table

Returns:
  EFI_SUCCESS           - thread can be successfully created
  EFI_OUT_OF_RESOURCES  - cannot allocate protocol data structure
  EFI_DEVICE_ERROR      - cannot create the thread

--*/
{
  EFI_STATUS                  Status;

  EfiInitializeDriverLib (ImageHandle, SystemTable);
  
  //
  // Allocate space to convert 2MB page tables to 4K tables.
  //  This can not be done a call time as the TPL level will
  //  not be correct. 
  //
  InitailizeMemoryAttributes ();

  
  InitializeExternalVectorTablePtr(gExternalVectorTable);
  //
  // Initialize the Interrupt Descriptor Table
  //
  InitializeInterruptTables ();
  
  //
  // Initialize the Global Descriptor Table
  //
  InitializeSelectors ();

  //
  // Initialize Exception Handlers
  //
  InitializeException (&mCpu);
  //
  // Find the Legacy8259 protocol.
  //
  Status = gBS->LocateProtocol (&gEfiLegacy8259ProtocolGuid, NULL, (VOID **) &gLegacy8259);
  ASSERT_EFI_ERROR (Status);
  //
  // BugBug: Not debugged yet
  //
  InitializeBiosIntCaller ();

  //
  // Install CPU Architectural Protocol and the thunk protocol
  //
  mHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mHandle,
                  &gEfiCpuArchProtocolGuid,       &mCpu,
                  &gLegacyBiosThunkProtocolGuid,  &mLegacyBiosThunk,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

