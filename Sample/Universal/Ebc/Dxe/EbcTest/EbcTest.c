/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  EbcTest.c

Abstract:

  EBC Debugger Test Driver

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"

//
// Natural integer initialization
//
UINTN   TestVariable1 = 4;

//
// Function declarations
//

EFI_STATUS
TestSubRoutine1 (
  IN UINTN        Arg1,
  IN UINTN        Arg2
  );

EFI_STATUS
TestSubRoutine2 (
  IN UINTN        Arg1,
  IN UINTN        *Arg2
  );

//
// Function implementations
//

VOID
EFIAPI
TestSignalEventFunc (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  if (Context != NULL) {
    *(UINTN *)Context = 5;
  }
}

VOID
EFIAPI
TestTimerEventFunc (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  if (Context != NULL) {
    *(UINTN *)Context = 6;
  }
}

//
// Driver entry point
//

EFI_DRIVER_ENTRY_POINT (InitializeEbcDriver)

EFI_STATUS
EFIAPI
InitializeEbcDriver (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_STATUS  (*FuncPtr) (IN UINTN Arg1, IN UINTN Arg2);
  EFI_EVENT   Event1;
  EFI_EVENT   Event2;
  UINTN       Index;

  EfiInitializeDriverLib (ImageHandle, SystemTable);

  //
  // EBC-to-Native invoke
  //
  SystemTable->ConOut->OutputString (
                         SystemTable->ConOut,
                         L"Hello EBC Test!\n\r"
                         );

  //
  // EBC-to-EBC invoke
  //
  TestSubRoutine1 (1, 3);
  TestSubRoutine2 (1, &TestVariable1);

  //
  // EBC-to-Native invoke
  //
  SystemTable->ConOut->OutputString (
                         SystemTable->ConOut,
                         L"Test String!\n\r"
                         );

  //
  // EBC-to-EBC Function Pointer invoke
  //
  FuncPtr = TestSubRoutine1;
  FuncPtr (1, 5);

  //
  // Event and Native-to-EBC invoke
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_CALLBACK,
                  TestSignalEventFunc,
                  &TestVariable1,
                  &Event1
                  );
  if (!EFI_ERROR (Status)) {
    gBS->SignalEvent (Event1);
  }

  //
  // Timer and Native-to-EBC invoke
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_CALLBACK,
                  TestTimerEventFunc,
                  &TestVariable1,
                  &Event2
                  );
  if (!EFI_ERROR (Status)) {
    gBS->SetTimer (
           Event2,
           TimerRelative,
           10000000              // 1 second
           );
  }

  //
  // EBC-to-Native invoke
  //
  SystemTable->ConOut->OutputString (
                         SystemTable->ConOut,
                         L"Goodbye EBC Test!\n\r"
                         );

  return EFI_SUCCESS;
}
