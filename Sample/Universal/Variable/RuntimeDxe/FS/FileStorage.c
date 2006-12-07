/*++

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

    FileStorage.c

Abstract:

    handles variable store/reads on file

Revision History

--*/
#include "Tiano.h"
#include "VariableStorage.h"
#include "EfiRuntimeLib.h"

#include EFI_PROTOCOL_DEFINITION (FileInfo)
#include EFI_PROTOCOL_DEFINITION (BlockIo)

CHAR16 mEfivar[]    = L"Efivar.bin";
//
// Variable storage hot plug is supported but there are still some restrictions:
// After plugging the storage back,
// 1. Still use memory as NV if newly plugged storage's MediaId is not same as the original one
// 2. Still use memory as NV if there are some update operation during storage is unplugged.
//
UINT32  mMediaId;
BOOLEAN mContentDiff = FALSE;
//
// Prototypes
//

STATIC
EFI_FILE_INFO *
FileInfo (
  IN EFI_FILE_HANDLE      FHand
  );

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
  IN VARIABLE_STORAGE     *This
  );

STATIC
EFI_STATUS
VarWriteStore (
  IN VARIABLE_STORAGE     *This,
  IN UINTN                Offset,
  IN UINTN                BufferSize,
  IN VOID                 *Buffer
  );

STATIC
EFI_STATUS
OpenStore (
  IN  EFI_DEVICE_PATH_PROTOCOL  *Device,
  IN  CHAR16                    *FilePathName,
  IN  UINT64                    OpenMode,
  OUT EFI_FILE                  **File,
  OUT UINT32                    *MediaId        OPTIONAL
  );

STATIC
EFI_STATUS
OpenVarStore (
  IN VS_DEV               *This,
  IN UINTN                Offset
  );

STATIC
EFI_STATUS
ReadStore (
  IN VS_DEV               *Dev
  );


//
// Implementation below:
//

STATIC
VOID
FileStorageDestructor (
  IN VARIABLE_STORAGE     *VarStore
  )
{
  VS_DEV    *Dev;
  Dev = DEV_FROM_THIS (VarStore);

  gBS->FreePool (VAR_DATA_PTR (Dev));
  gBS->FreePool (VAR_FILE_DEVICEPATH (Dev));
  gBS->FreePool (Dev);
}

EFI_STATUS
FileStorageConstructor (
  OUT VARIABLE_STORAGE    **VarStore,  
  IN  UINT32              Attributes,
  IN  UINTN               Size,
  IN  EFI_HANDLE          Handle
  )
{
  VS_DEV                    *Dev;
  EFI_STATUS                Status;
  EFI_EVENT                 ChgVMEvent;
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;
  EFI_FILE_INFO             *Info;
  EFI_FILE                  *File;
  BOOLEAN                   CreateNew;

  Info      = NULL;
  CreateNew = FALSE;
  *VarStore = NULL;

  Status = gBS->AllocatePool (EfiRuntimeServicesData, sizeof(VS_DEV), &Dev);
  ASSERT_EFI_ERROR (Status);
  EfiZeroMem (Dev, sizeof(VS_DEV));

  Dev->Signature  = VARIABLE_STORE_SIGNATURE;
  Dev->Size       = Size;
  Dev->Attributes = Attributes;

  //
  // Allocate space for local buffer
  //
  Status = gBS->AllocatePool (EfiRuntimeServicesData, Size, &VAR_DATA_PTR(Dev));
  ASSERT_EFI_ERROR (Status);

  //
  // Check devices that starts with the BootStoreDevice device path
  //
  VAR_FILE_FILEHANDLE (Dev) = NULL;

  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiBlockIoProtocolGuid, // BlockIo should be supported if it supports SimpleFileSystem
                  (VOID*)&BlkIo
                  );

  if (EFI_ERROR (Status)) {
    goto ErrHandle;
  }
  if (!BlkIo->Media->MediaPresent) {
    DEBUG ((EFI_D_ERROR, "FileStorageConstructor: Media not present!\n"));
    Status = EFI_NO_MEDIA;
    goto ErrHandle;
  }
  if (BlkIo->Media->ReadOnly) {
    DEBUG ((EFI_D_ERROR, "FileStorageConstructor: Media is read-only!\n"));
    Status = EFI_ACCESS_DENIED;
    goto ErrHandle;
  }

  //
  // Get the device path of this device
  //

  VAR_FILE_DEVICEPATH (Dev) = RtEfiDuplicateDevicePath (RtEfiDevicePathFromHandle (Handle));
  if (VAR_FILE_DEVICEPATH (Dev) == NULL) {
    DEBUG ((EFI_D_ERROR, "FileStorageConstructor: Device Path from Handle failed: %X!\n", Handle));
    Status = EFI_LOAD_ERROR;
    goto ErrHandle;
  }

  //
  // Check \efi\boot\ directory
  //
  Status = OpenStore (VAR_FILE_DEVICEPATH (Dev), L"\\efi\\boot", EFI_FILE_MODE_READ, &File, NULL);
  if (EFI_ERROR (Status)) {
    goto ErrHandle;
  }
  File->Close (File);

  //
  // Check \Efivar.bin file
  //
  Status = OpenStore (VAR_FILE_DEVICEPATH (Dev), mEfivar, EFI_FILE_MODE_READ, &File, &mMediaId);

  if (EFI_ERROR (Status)) {
    //
    // Create \Efivar.bin, and init with default value, e.g.: 0xff
    //
    Status = OpenStore (
               VAR_FILE_DEVICEPATH (Dev), 
               mEfivar, 
               EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 
               &File,
               NULL
               );
    if (!EFI_ERROR (Status)) {
      Status = VarEraseStore (&Dev->VarStore);
      if (EFI_ERROR (Status)) {
        goto ErrHandle;
      }
      CreateNew = TRUE;
    }

    if (EFI_ERROR (Status)) {
      goto ErrHandle;
    }
  }

  VAR_FILE_FILEHANDLE (Dev) = File;

  //
  // If file's size is not equal to our desired size, there MUST be some mistakes.
  // The better way here is *NOT* to automatic re-create the file with correct size,
  //   but to report such error to user.
  //
  Info = FileInfo (VAR_FILE_FILEHANDLE (Dev));
  ASSERT ((Info != NULL) && (Info->FileSize == Size));
  if ((Info == NULL) || (Info->FileSize != Size)) {
    Status = EFI_LOAD_ERROR;
    goto ErrHandle;
  }
  gBS->FreePool (Info);
  Info = NULL;

  //
  // Read total content into local buffer
  //
  if (!CreateNew) {
    Status = ReadStore (Dev);
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      goto ErrHandle;
    }
  }

  DEBUG ((EFI_D_ERROR, "FileStorageConstructor: added, Size - 0x%x\n", Size));

  Status = gBS->CreateEvent (
                  EFI_EVENT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                  EFI_TPL_NOTIFY,
                  OnVirtualAddressChange,
                  Dev,
                  &ChgVMEvent
                  );
  ASSERT_EFI_ERROR (Status);


  Dev->VarStore.Erase    = VarEraseStore;
  Dev->VarStore.Write    = VarWriteStore;
  Dev->VarStore.Destruct = FileStorageDestructor;

  *VarStore = &Dev->VarStore;
  return EFI_SUCCESS;

ErrHandle:
  if (Info != NULL) {
    gBS->FreePool (Info);
  }
  gBS->FreePool (VAR_DATA_PTR (Dev));
  gBS->FreePool (Dev);
  return Status;
}


STATIC
EFI_STATUS
VarEraseStore(
  IN VARIABLE_STORAGE   *This
  )
{
  EFI_STATUS              Status;
  VS_DEV                  *Dev;

  Status = EFI_SUCCESS;
  Dev    = DEV_FROM_THIS(This);

  EfiSetMem (VAR_DATA_PTR (Dev), Dev->Size, VAR_DEFAULT_VALUE);

  if (!mContentDiff && !EfiAtRuntime ()) {
    //
    // If not in the runtime period, write through to file also
    //
    Status = OpenVarStore (Dev, 0);

    if (EFI_ERROR (Status)) {
      mContentDiff = TRUE;
    } else {
      Status = VAR_FILE_FILEHANDLE (Dev)->Write (VAR_FILE_FILEHANDLE (Dev), &Dev->Size, VAR_DATA_PTR (Dev));
      if (!EFI_ERROR (Status)) {
        VAR_FILE_FILEHANDLE (Dev)->Flush (VAR_FILE_FILEHANDLE (Dev));
      }
      VAR_FILE_FILEHANDLE (Dev)->Close (VAR_FILE_FILEHANDLE (Dev));
    }
    VAR_FILE_FILEHANDLE (Dev) = NULL;
  }
  
  return Status;
}

STATIC
EFI_STATUS
VarWriteStore (
  IN VARIABLE_STORAGE     *This,
  IN UINTN                Offset,
  IN UINTN                BufferSize,
  IN VOID                 *Buffer
  )
{
  EFI_STATUS              Status;
  VS_DEV                  *Dev;

  Status = EFI_SUCCESS;
  Dev    = DEV_FROM_THIS(This);

  ASSERT (Buffer != NULL);
  ASSERT (Offset + BufferSize <= Dev->Size);

  //
  // For better performance
  //
  if (VAR_DATA_PTR (Dev) + Offset != Buffer) {
    EfiCopyMem (VAR_DATA_PTR (Dev) + Offset, Buffer, BufferSize);
  }

  if (!mContentDiff && !EfiAtRuntime ()) {

    Status = OpenVarStore (Dev, Offset);

    if (EFI_ERROR (Status)) {
      mContentDiff = TRUE;
    } else {
      Status = VAR_FILE_FILEHANDLE (Dev)->Write (VAR_FILE_FILEHANDLE (Dev), &BufferSize, Buffer);
      if (!EFI_ERROR (Status)) {
        VAR_FILE_FILEHANDLE (Dev)->Flush (VAR_FILE_FILEHANDLE (Dev));
      }
      VAR_FILE_FILEHANDLE (Dev)->Close (VAR_FILE_FILEHANDLE (Dev));
    }

    VAR_FILE_FILEHANDLE (Dev) = NULL;
  }
  return Status;
}

STATIC
VOID
EFIAPI
OnVirtualAddressChange (
  IN EFI_EVENT            Event,
  IN VOID                 *Context
  )
{
  VS_DEV  *Dev;

  Dev = Context;

  EfiConvertPointer (EFI_INTERNAL_POINTER, &VAR_DATA_PTR (Dev));
  EfiConvertPointer (EFI_INTERNAL_POINTER, &Dev->VarStore.Erase);
  EfiConvertPointer (EFI_INTERNAL_POINTER, &Dev->VarStore.Write);
  EfiConvertPointer (EFI_INTERNAL_POINTER, &Dev->VarStore.Destruct);
}

STATIC
EFI_STATUS
OpenStore (
  IN  EFI_DEVICE_PATH_PROTOCOL  *Device,
  IN  CHAR16                    *FilePathName,
  IN  UINT64                    OpenMode,
  OUT EFI_FILE                  **File,
  OUT UINT32                    *MediaId        OPTIONAL
  )
{
  EFI_HANDLE                        Handle;
  EFI_FILE_HANDLE                   Root;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Volume;
  EFI_BLOCK_IO_PROTOCOL             *BlockIo;
  EFI_STATUS                        Status;

  *File = NULL;

  Status = gBS->LocateDevicePath (
                  &gEfiSimpleFileSystemProtocolGuid,
                  &Device, 
                  &Handle
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  &Volume
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get Media ID if neccessary
  //
  if (MediaId != NULL) {
    Status = gBS->HandleProtocol (
                    Handle,
                    &gEfiBlockIoProtocolGuid,
                    &BlockIo
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    *MediaId = BlockIo->Media->MediaId;
  }
  //
  // Open the root directory of the volume
  //
  Root = NULL;
  Status = Volume->OpenVolume (
                     Volume,
                     &Root
                     );
  ASSERT_EFI_ERROR (Status);
  ASSERT (Root != NULL);

  //
  // Open file
  //
  Status = Root->Open (
                   Root,
                   File,
                   FilePathName,
                   OpenMode,
                   0
                   );
  if (EFI_ERROR (Status)) {
    *File = NULL;
  }

  //
  // Close the Root directory
  //
  Root->Close (Root);
  return Status;
}

STATIC
EFI_STATUS
OpenVarStore (
  IN VS_DEV   *Dev,
  IN UINTN    Offset
  )
{
  EFI_STATUS  Status;
  UINT32      MediaId;      

  ASSERT (!EfiAtRuntime ());
  ASSERT (VAR_FILE_DEVICEPATH (Dev) != NULL);
  //
  // If the boot store file isn't open, open it now
  //
  if (VAR_FILE_FILEHANDLE (Dev) == NULL) {
    Status = OpenStore (
               VAR_FILE_DEVICEPATH (Dev), 
               mEfivar, 
               EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 
               &VAR_FILE_FILEHANDLE (Dev),
               &MediaId
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if (MediaId != mMediaId) {
      return EFI_MEDIA_CHANGED;
    }
  }

  //
  // Set the offset pointer
  //
  Status = VAR_FILE_FILEHANDLE (Dev)->SetPosition (VAR_FILE_FILEHANDLE (Dev), Offset);

  return Status;
}


STATIC
EFI_STATUS
ReadStore (
  IN VS_DEV               *Dev
  )
{
  EFI_STATUS Status;

  Status = OpenVarStore (Dev, 0);

  if (!EFI_ERROR(Status)) {
    Status = VAR_FILE_FILEHANDLE (Dev)->Read (VAR_FILE_FILEHANDLE (Dev), &Dev->Size, VAR_DATA_PTR (Dev));
    VAR_FILE_FILEHANDLE (Dev)->Close (VAR_FILE_FILEHANDLE (Dev));
  }

  VAR_FILE_FILEHANDLE (Dev) = NULL;

  return Status;
}

STATIC
EFI_FILE_INFO *
FileInfo (
  IN EFI_FILE_HANDLE      FHand
  )
/*++

Routine Description:

  Function gets the file information from an open file descriptor, and stores it 
  in a buffer allocated from pool.

Arguments:

  Fhand         - A file handle

Returns:
  
  A pointer to a buffer with file information or NULL is returned

--*/
{
  EFI_STATUS            Status;
  EFI_FILE_INFO         *Buffer;
  UINTN                 BufferSize;
  
  ASSERT (FHand != NULL);
  
  //
  // Initialize for GrowBuffer loop
  //
  Buffer      = NULL;
  BufferSize  = 0;
  Status = FHand->GetInfo (
                    FHand,
                    &gEfiFileInfoGuid,
                    &BufferSize,
                    Buffer
                    );
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);

  Status = gBS->AllocatePool (EfiBootServicesData, BufferSize, &Buffer);
  ASSERT_EFI_ERROR (Status);

  Status = FHand->GetInfo (
                    FHand,
                    &gEfiFileInfoGuid,
                    &BufferSize,
                    Buffer
                    );
  ASSERT_EFI_ERROR (Status);
  return Buffer;
}

