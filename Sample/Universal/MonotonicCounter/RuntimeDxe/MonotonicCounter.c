/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MonotonicCounter.c

Abstract:

  Produced the Monotonic Counter Services as defined in the DXE CIS

Revision History:

--*/

#include "MonotonicCounter.h"

//
// The Monotonic Counter Handle
//
EFI_HANDLE  mMonotonicCounterHandle = NULL;

//
// The current Monotonic count value
//
UINT64      mEfiMtc;

//
// Boolean flag that is FALSE before ExitBootServices() and TRUE after ExitBootServices()
//
BOOLEAN     mEfiAtRuntime = FALSE;

//
// Event to use when ExitBootServices() is called
//
EFI_EVENT   mMonotonicCounterNotifyEvent;

//
// Event to use when SetVirtualAddressMap() is called
//
EFI_EVENT   mMonotonicCounterSetVirtualAddressMapNotifyEvent;

//
// Event to use to update the Mtc's high part when wrapping
//
EFI_EVENT   mEfiMtcEvent;

//
// EfiMtcName - Variable name of the MTC value
//
CHAR16      *mEfiMtcName = L"MTC";

//
// EfiMtcGuid - Guid of the MTC value
//
EFI_GUID    mEfiMtcGuid = { 0xeb704011, 0x1402, 0x11d3, 0x8e, 0x77, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b };

//
// Worker functions
//
EFI_BOOTSERVICE
EFI_STATUS
MonotonicCounterDriverGetNextMonotonicCount (
  OUT UINT64  *Count
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_TPL OldTpl;

  //
  // Can not be called after ExitBootServices()
  //
  if (mEfiAtRuntime) {
    return EFI_UNSUPPORTED;
  }
  //
  // Check input parameters
  //
  if (Count == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Update the monotonic counter with a lock
  //
  OldTpl  = gBS->RaiseTPL (EFI_TPL_HIGH_LEVEL);
  *Count  = mEfiMtc;
  mEfiMtc++;
  gBS->RestoreTPL (OldTpl);

  //
  // If the MSB bit of the low part toggled, then signal that the high
  // part needs updated now
  //
  if ((((UINT32) mEfiMtc) ^ ((UINT32) *Count)) & 0x80000000) {
    gBS->SignalEvent (mEfiMtcEvent);
  }

  return EFI_SUCCESS;
}

EFI_RUNTIMESERVICE
EFI_STATUS
MonotonicCounterDriverGetNextHighMonotonicCount (
  OUT UINT32  *HighCount
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS  Status;
  EFI_TPL     OldTpl;

  //
  // Check input parameters
  //
  if (HighCount == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!mEfiAtRuntime) {
    //
    // Use a lock if called before ExitBootServices()
    //
    OldTpl      = gBS->RaiseTPL (EFI_TPL_HIGH_LEVEL);
    *HighCount  = (UINT32) RShiftU64 (mEfiMtc, 32) + 1;
    mEfiMtc     = LShiftU64 (*HighCount, 32);
    gBS->RestoreTPL (OldTpl);
  } else {
    *HighCount  = (UINT32) RShiftU64 (mEfiMtc, 32) + 1;
    mEfiMtc     = LShiftU64 (*HighCount, 32);
  }
  //
  // Update the NvRam store to match the new high part
  //
  Status = gRT->SetVariable (
                  mEfiMtcName,
                  &mEfiMtcGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof (UINT32),
                  HighCount
                  );

  return Status;
}

EFI_STATUS
EFIAPI
EfiMtcEventHandler (
  IN EFI_EVENT                Event,
  IN VOID                     *Context
  )
/*++

Routine Description:

  Monotonic count event handler.  This handler updates the high monotonic count.

Arguments:

  Event         The event to handle
  Context       The event context

Returns:

  EFI_SUCCESS       The event has been handled properly 
  EFI_NOT_FOUND     An error occurred updating the variable.

--*/
{
  UINT32  HighCount;

  return MonotonicCounterDriverGetNextHighMonotonicCount (&HighCount);
}

VOID
EFIAPI
MonotonicCounterDriverExitBootServices (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  mEfiAtRuntime = TRUE;
}

VOID
EFIAPI
MonotonicCounterDriverSetVirtualAddressMap (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  gRT->ConvertPointer (EFI_OPTIONAL_POINTER, (VOID **) &gRT);
}

EFI_DRIVER_ENTRY_POINT (MonotonicCounterDriverInitialize)

EFI_STATUS
MonotonicCounterDriverInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:

--*/
{
  EFI_STATUS  Status;
  UINT32      HighCount;
  UINTN       BufferSize;

  EfiInitializeDriverLib (ImageHandle, SystemTable);

  //
  // Make sure the Monotonic Counter Architectural Protocol is not already installed in the system
  //
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEfiMonotonicCounterArchProtocolGuid);

  //
  // Register our ExitBootServices notify function
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_SIGNAL_EXIT_BOOT_SERVICES,
                  EFI_TPL_NOTIFY,
                  MonotonicCounterDriverExitBootServices,
                  NULL,
                  &mMonotonicCounterNotifyEvent
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register our SetVirtualAddressMap notify function
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                  EFI_TPL_NOTIFY,
                  MonotonicCounterDriverSetVirtualAddressMap,
                  NULL,
                  &mMonotonicCounterSetVirtualAddressMapNotifyEvent
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize event to handle overflows
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_CALLBACK,
                  EfiMtcEventHandler,
                  NULL,
                  &mEfiMtcEvent
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Read the last high part
  //
  BufferSize = sizeof (UINT32);
  Status = gRT->GetVariable (
                  mEfiMtcName,
                  &mEfiMtcGuid,
                  NULL,
                  &BufferSize,
                  &HighCount
                  );
  if (EFI_ERROR (Status)) {
    HighCount = 0;
  }
  //
  // Set the current value
  //
  mEfiMtc = LShiftU64 (HighCount, 32);

  //
  // Increment the upper 32 bits for this boot
  // Continue even if it fails.  It will only fail if the variable services are
  // not functional.
  //
  Status = MonotonicCounterDriverGetNextHighMonotonicCount (&HighCount);

  //
  // Fill in the EFI Boot Services and EFI Runtime Services Monotonic Counter Fields
  //
  gBS->GetNextMonotonicCount      = MonotonicCounterDriverGetNextMonotonicCount;
  gRT->GetNextHighMonotonicCount  = MonotonicCounterDriverGetNextHighMonotonicCount;

  //
  // Install the Monotonic Counter Architctural Protocol onto a new handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mMonotonicCounterHandle,
                  &gEfiMonotonicCounterArchProtocolGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
