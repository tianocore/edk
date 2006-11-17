/*++

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  FSVariable.c

Abstract:

  Provide patch functions for MonotonicCounter service.

--*/
#include "FSVariable.h"
#include "EfiRuntimeLib.h"
//
// Variable need to sync from memory to file storage
//
CHAR16  *mSyncVariable[] = {L"ConIn", L"ConOut", L"ErrOut", L"Lang"};
#define COUNT_OF(x)  (sizeof (x) / sizeof (*x))
UINTN   mSyncVariableDataSize[COUNT_OF (mSyncVariable)];
VOID    *mSyncVariableData[COUNT_OF (mSyncVariable)];
UINT32  mSyncVariableAttributes[COUNT_OF (mSyncVariable)];


//
// TODO: Put additional variable patch function below
//
// ...
STATIC
EFI_STATUS
PatchNextHighMonotonicCount (
  VOID
  )
{
  EFI_STATUS  Status;
  EFI_TPL     OldTpl;
  UINTN       DataSize;
  UINT32      HighCount;
  EFI_GUID    MtcGuid = {
    0xeb704011, 0x1402, 0x11d3, 0x8e, 0x77, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b
  };
  CHAR16      MtcName[] = L"MTC";

  //
  // Check input parameters
  //
  DataSize = sizeof (UINT32);
  Status = GetVariable (
             MtcName,
             &MtcGuid,
             NULL,
             &DataSize,
             &HighCount
             );
  ASSERT (Status == EFI_SUCCESS || Status == EFI_NOT_FOUND);
  if (Status == EFI_NOT_FOUND) {
    HighCount = 0;
  }

  if (!EfiAtRuntime ()) {
    //
    // Use a lock if called before ExitBootServices()
    //
    OldTpl      = gBS->RaiseTPL (EFI_TPL_HIGH_LEVEL);
    HighCount ++;
    gBS->RestoreTPL (OldTpl);
  } else {
    HighCount ++;
  }
  //
  // Update the NvRam store to match the new high part
  //
  Status = SetVariable (
                  MtcName,
                  &MtcGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof (UINT32),
                  &HighCount
                  );

  return Status;
}

VOID
LoadSyncVariable (
  VOID
  )
{
  UINTN       Index;
  EFI_STATUS  Status;

  //
  // Read important variables from previous memory-storage: ConIn, ConOut, ErrOut, Lang
  //
  for (Index = 0; Index < COUNT_OF (mSyncVariable); Index ++) {
    mSyncVariableDataSize[Index] = 0;
    mSyncVariableData[Index]     = NULL;
    Status = GetVariable (
               mSyncVariable[Index],
               &gEfiGlobalVariableGuid,
               NULL,
               &mSyncVariableDataSize[Index],
               NULL
               );
    if (Status == EFI_BUFFER_TOO_SMALL) {
      gBS->AllocatePool (EfiBootServicesData, mSyncVariableDataSize[Index], &mSyncVariableData[Index]);
      ASSERT (mSyncVariableData[Index] != NULL);

      Status = GetVariable (
                 mSyncVariable[Index],
                 &gEfiGlobalVariableGuid,
                 &mSyncVariableAttributes[Index],
                 &mSyncVariableDataSize[Index],
                 mSyncVariableData[Index]
                 );
      ASSERT_EFI_ERROR (Status);
    }
  }
}

VOID
StoreSyncVariable (
  VOID
  )
{
  UINTN       Index;
  EFI_STATUS  Status;

  for (Index = 0; Index < COUNT_OF (mSyncVariable); Index ++) {
    if (mSyncVariableData[Index] == NULL) {
      continue;
    }

    Status = SetVariable (
               mSyncVariable[Index],
               &gEfiGlobalVariableGuid,
               mSyncVariableAttributes[Index],
               mSyncVariableDataSize[Index],
               mSyncVariableData[Index]
               );
    ASSERT_EFI_ERROR (Status);
    gBS->FreePool (mSyncVariableData[Index]);
  }
}

VOID
PatchVariable (
  VOID
  )
{
  //
  // increase MTC
  //
  PatchNextHighMonotonicCount ();

  //
  // TODO: Additional patch operation goes below...
  //
  // Patch...()
}

