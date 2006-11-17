/*++

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

    MemStorage.c

Abstract:

    handles variable store/reads with emulated memory

Revision History

--*/


#include "Tiano.h"
#include "VariableStorage.h"
#include "EfiRuntimeLib.h"
#include "FSVariable.h"

STATIC
VOID
EFIAPI
OnVirtualAddressChange (
  IN EFI_EVENT            Event,
  IN VOID                 *Context
  );

STATIC
EFI_STATUS
VarEraseStore(
  IN VARIABLE_STORAGE   *This
  );

STATIC
EFI_STATUS
VarWriteStore (
  IN VARIABLE_STORAGE   *This,
  IN UINTN                Offset,
  IN UINTN                BufferSize,
  IN VOID                 *Buffer
  );

STATIC
VOID
MemStorageDestructor (
  IN VARIABLE_STORAGE     *VarStore
  )
{
  VS_DEV    *Dev;
  Dev = DEV_FROM_THIS (VarStore);

  gBS->FreePool (VAR_DATA_PTR (Dev));
  gBS->FreePool (Dev);
}

EFI_STATUS
MemStorageConstructor (
  OUT VARIABLE_STORAGE          **VarStore,  
  IN  UINT32                    Attributes,
  IN  UINTN                     Size
  )
{
  EFI_STATUS                  Status;
  EFI_EVENT                   Event;
  VS_DEV                      *Dev;

  Status = gBS->AllocatePool (EfiRuntimeServicesData, sizeof(VS_DEV), &Dev);
  ASSERT_EFI_ERROR (Status);

  EfiZeroMem (Dev, sizeof(VS_DEV));

  Dev->Signature   = VARIABLE_STORE_SIGNATURE;
  Dev->Attributes  = Attributes;
  Dev->Size        = Size;

  Dev->VarStore.Erase    = VarEraseStore;
  Dev->VarStore.Write    = VarWriteStore;
  Dev->VarStore.Destruct = MemStorageDestructor;

  Status = gBS->AllocatePool (EfiRuntimeServicesData, Size, &VAR_DATA_PTR (Dev));
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_ERROR, "%s: Size = 0x%x\n", 
        ((Attributes & VAR_NV_MASK) != 0) ? L"MemStorage Pre allocate for FileStorage" : L"MemStorage",
        Size)
      );
  
  //
  // This a runtime ram device, we need to update any internal
  // pointers for the device if the OS sets the environment into
  // virtual mode
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                  EFI_TPL_NOTIFY,
                  OnVirtualAddressChange,
                  Dev,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);
  *VarStore = &Dev->VarStore;

  return EFI_SUCCESS;
}

STATIC
VOID
EFIAPI
OnVirtualAddressChange (
  IN EFI_EVENT            Event,
  IN VOID                 *Context
  )
{
  VS_DEV                  *Dev;

  Dev = Context;

  EfiConvertPointer (EFI_INTERNAL_POINTER, &VAR_DATA_PTR (Dev));
  EfiConvertPointer (EFI_INTERNAL_POINTER, &Dev->VarStore.Erase);
  EfiConvertPointer (EFI_INTERNAL_POINTER, &Dev->VarStore.Write);
  EfiConvertPointer (EFI_INTERNAL_POINTER, &Dev->VarStore.Destruct);
}

STATIC
EFI_STATUS
VarEraseStore(
  IN VARIABLE_STORAGE   *This
  )
{
  VS_DEV              *Dev;

  Dev = DEV_FROM_THIS(This);
  EfiSetMem (VAR_DATA_PTR (Dev), Dev->Size, VAR_DEFAULT_VALUE);
  return EFI_SUCCESS;
}


STATIC
EFI_STATUS
VarWriteStore (
  IN VARIABLE_STORAGE     *This,
  IN UINTN                Offset,
  IN UINTN                BufferSize,
  IN VOID                 *UserBuffer
  )
{
  VS_DEV              *Dev;

  Dev = DEV_FROM_THIS(This);

  ASSERT (Offset + BufferSize < Dev->Size);

  // For better performance
  if (VAR_DATA_PTR (Dev) + Offset != UserBuffer) {
    EfiCopyMem (VAR_DATA_PTR (Dev) + Offset, UserBuffer, BufferSize);
  }
  return EFI_SUCCESS;
}
