/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EbcSupport.c

Abstract:

  This module contains EBC support routines that are customized based on
  the target processor.

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"

//
// To support the EFI debug support protocol
//
#include EFI_PROTOCOL_DEFINITION (Ebc)
#include EFI_PROTOCOL_DEFINITION (DebugSupport)

#include "EbcInt.h"
#include "EbcExecute.h"

//
// NOTE: This is the stack size allocated for the interpreter
//       when it executes an EBC image. The requirements can change
//       based on whether or not a debugger is present, and other
//       platform-specific configurations.
//
#define VM_STACK_SIZE   (1024 * 4)
#define EBC_THUNK_SIZE  32

STATIC
UINT64
EbcInterpret (
  UINTN      Arg1,
  UINTN      Arg2,
  UINTN      Arg3,
  UINTN      Arg4,
  UINTN      Arg5,
  UINTN      Arg6,
  UINTN      Arg7,
  UINTN      Arg8
  )
/*++

Routine Description:

  Begin executing an EBC image. The address of the entry point is passed
  in via a processor register, so we'll need to make a call to get the
  value.
  
Arguments:

  None. Since we're called from a fixed up thunk (which we want to keep
  small), our only so-called argument is the EBC entry point passed in
  to us in a processor register.

Returns:

  The value returned by the EBC application we're going to run.
  
--*/
{
  //
  // Create a new VM context on the stack
  //
  VM_CONTEXT  VmContext;
  UINTN       Addr;

  //
  // Get the EBC entry point from the processor register.
  //
  Addr = EbcLLGetEbcEntryPoint ();

  //
  // Now clear out our context
  //
  EfiZeroMem ((VOID *) &VmContext, sizeof (VM_CONTEXT));

  //
  // Set the VM instruction pointer to the correct location in memory.
  //
  VmContext.Ip = (VMIP) Addr;

  //
  // Initialize the stack pointer for the EBC. Get the current system stack
  // pointer and adjust it down by the max needed for the interpreter.
  //
  Addr            = EbcLLGetStackPointer ();

  VmContext.R[0]  = (UINT64) Addr;
  VmContext.R[0] -= VM_STACK_SIZE;

  //
  // Align the stack on a natural boundary
  //
  VmContext.R[0] &= ~(sizeof (UINTN) - 1);

  //
  // Put a magic value in the stack gap, then adjust down again
  //
  *(UINTN *) (UINTN) (VmContext.R[0]) = (UINTN) VM_STACK_KEY_VALUE;
  VmContext.StackMagicPtr             = (UINTN *) (UINTN) VmContext.R[0];
  VmContext.R[0] -= sizeof (UINTN);

  //
  // For IA32, this is where we say our return address is
  //
  VmContext.StackRetAddr  = (UINT64) VmContext.R[0];
  VmContext.LowStackTop   = (UINTN) VmContext.R[0];

  //
  // We need to keep track of where the EBC stack starts. This way, if the EBC
  // accesses any stack variables above its initial stack setting, then we know
  // it's accessing variables passed into it, which means the data is on the
  // VM's stack.
  // When we're called, on the stack (high to low) we have the parameters, the
  // return address, then the saved ebp. Save the pointer to the return address.
  // EBC code knows that's there, so should look above it for function parameters.
  // The offset is the size of locals (VMContext + Addr + saved ebp).
  // Note that the interpreter assumes there is a 16 bytes of return address on
  // the stack too, so adjust accordingly.
  //  VmContext.HighStackBottom = (UINTN)(Addr + sizeof (VmContext) + sizeof (Addr));
  //
  VmContext.HighStackBottom = (UINTN) &Arg1 - 16;
  //
  // Begin executing the EBC code
  //
  EbcExecute (&VmContext);

  //
  // Return the value in R[7] unless there was an error
  //
  return (UINT64) VmContext.R[7];
}

STATIC
UINT64
ExecuteEbcImageEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Begin executing an EBC image. The address of the entry point is passed
  in via a processor register, so we'll need to make a call to get the
  value.
  
Arguments:

  ImageHandle   - image handle for the EBC application we're executing
  SystemTable   - standard system table passed into an driver's entry point

Returns:

  The value returned by the EBC application we're going to run.

--*/
{
  //
  // Create a new VM context on the stack
  //
  VM_CONTEXT  VmContext;
  UINTN       Addr;

  //
  // Get the EBC entry point from the processor register. Make sure you don't
  // call any functions before this or you could mess up the register the
  // entry point is passed in.
  //
  Addr = EbcLLGetEbcEntryPoint ();

  //
  //  Print(L"*** Thunked into EBC entry point - ImageHandle = 0x%X\n", (UINTN)ImageHandle);
  //  Print(L"EBC entry point is 0x%X\n", (UINT32)(UINTN)Addr);
  //
  // Now clear out our context
  //
  EfiZeroMem ((VOID *) &VmContext, sizeof (VM_CONTEXT));

  //
  // Save the image handle so we can track the thunks created for this image
  //
  VmContext.ImageHandle = ImageHandle;
  VmContext.SystemTable = SystemTable;

  //
  // Set the VM instruction pointer to the correct location in memory.
  //
  VmContext.Ip = (VMIP) Addr;

  //
  // Initialize the stack pointer for the EBC. Get the current system stack
  // pointer and adjust it down by the max needed for the interpreter.
  //
  Addr            = EbcLLGetStackPointer ();
  VmContext.R[0]  = (UINT64) Addr;
  VmContext.R[0] -= VM_STACK_SIZE;
  //
  // Put a magic value in the stack gap, then adjust down again
  //
  *(UINTN *) (UINTN) (VmContext.R[0]) = (UINTN) VM_STACK_KEY_VALUE;
  VmContext.StackMagicPtr             = (UINTN *) (UINTN) VmContext.R[0];
  VmContext.R[0] -= sizeof (UINTN);

  //
  // Align the stack on a natural boundary
  //  VmContext.R[0] &= ~(sizeof(UINTN) - 1);
  //
  VmContext.StackRetAddr  = (UINT64) VmContext.R[0];
  VmContext.LowStackTop   = (UINTN) VmContext.R[0];
  //
  // VM pushes 16-bytes for return address. Simulate that here.
  //
  VmContext.HighStackBottom = (UINTN) &ImageHandle - 16;

  //
  // Begin executing the EBC code
  //
  EbcExecute (&VmContext);

  //
  // Return the value in R[7] unless there was an error
  //
  return (UINT64) VmContext.R[7];
}

EFI_STATUS
EbcCreateThunks (
  IN EFI_HANDLE           ImageHandle,
  IN VOID                 *EbcEntryPoint,
  OUT VOID                **Thunk,
  IN  UINT32              Flags
  )
/*++

Routine Description:

  Create an IA32 thunk for the given EBC entry point.
  
Arguments:

  ImageHandle     - Handle of image for which this thunk is being created
  EbcEntryPoint   - Address of the EBC code that the thunk is to call
  Thunk           - Returned thunk we create here

Returns:

  Standard EFI status.
  
--*/
{
  UINT8       *Ptr;
  UINT8       *ThunkBase;
  UINT32      I;
  UINT32      Addr;
  INT32       Size;
  INT32       ThunkSize;
  EFI_STATUS  Status;

  //
  // Check alignment of pointer to EBC code
  //
  if ((UINT32) (UINTN) EbcEntryPoint & 0x01) {
    return EFI_INVALID_PARAMETER;
  }

  Size      = EBC_THUNK_SIZE;
  ThunkSize = Size;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  Size,
                  (VOID *) &Ptr
                  );
  if (Status != EFI_SUCCESS) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  //  Print(L"Allocate TH: 0x%X\n", (UINT32)Ptr);
  //
  // Save the start address so we can add a pointer to it to a list later.
  //
  ThunkBase = Ptr;

  //
  // Give them the address of our buffer we're going to fix up
  //
  *Thunk = (VOID *) Ptr;

  //
  // Add code bytes to load up a processor register with the EBC entry point.
  // mov eax, 0xaa55aa55  => B8 55 AA 55 AA
  // The first 8 bytes of the thunk entry is the address of the EBC
  // entry point.
  //
  *Ptr = 0xB8;
  Ptr++;
  Size--;
  Addr = (UINT32) EbcEntryPoint;
  for (I = 0; I < sizeof (Addr); I++) {
    *Ptr = (UINT8) (UINTN) Addr;
    Addr >>= 8;
    Ptr++;
    Size--;
  }
  //
  // Stick in a load of ecx with the address of appropriate VM function.
  //  mov ecx 12345678h  => 0xB9 0x78 0x56 0x34 0x12
  //
  if (Flags & FLAG_THUNK_ENTRY_POINT) {
    Addr = (UINT32) (UINTN) ExecuteEbcImageEntryPoint;
  } else {
    Addr = (UINT32) (UINTN) EbcInterpret;
  }

  //
  // MOV ecx
  //
  *Ptr = 0xB9;
  Ptr++;
  Size--;
  for (I = 0; I < sizeof (Addr); I++) {
    *Ptr = (UINT8) Addr;
    Addr >>= 8;
    Ptr++;
    Size--;
  }
  //
  // Stick in jump opcode bytes for jmp ecx => 0xFF 0xE1
  //
  *Ptr = 0xFF;
  Ptr++;
  Size--;
  *Ptr = 0xE1;
  Size--;

  //
  // Double check that our defined size is ok (application error)
  //
  if (Size < 0) {
    ASSERT (FALSE);
    return EFI_BUFFER_TOO_SMALL;
  }
  //
  // Add the thunk to the list for this image. Do this last since the add
  // function flushes the cache for us.
  //
  EbcAddImageThunk (ImageHandle, (VOID *) ThunkBase, ThunkSize);

  return EFI_SUCCESS;
}
