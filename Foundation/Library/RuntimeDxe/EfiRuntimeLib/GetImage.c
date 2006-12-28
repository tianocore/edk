/*++

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  GetImage.c

Abstract:

  Image data extraction support for common use.

--*/

#include "Tiano.h"
#include "EfiRuntimeLib.h"
#include "EfiImageFormat.h"

EFI_STATUS
GetImage (
  IN  EFI_GUID           *NameGuid,
  IN  EFI_SECTION_TYPE   SectionType,
  OUT VOID               **Buffer,
  OUT UINTN              *Size
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         HandleCount;
  UINTN                         Index;
  EFI_FIRMWARE_VOLUME_PROTOCOL  *Fv;
  EFI_FV_FILETYPE               FileType;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  UINT32                        AuthenticationStatus;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolumeProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Find desired image in all Fvs
  //
  for (Index = 0; Index < HandleCount; ++Index) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiFirmwareVolumeProtocolGuid,
                    (VOID**)&Fv
                    );

    if (EFI_ERROR (Status)) {
      gBS->FreePool(HandleBuffer);
      return Status;
    }

    //
    // Read desired section content in NameGuid file
    //
    *Buffer     = NULL;
    *Size       = 0;
    Status      = Fv->ReadSection (
                        Fv,
                        NameGuid,
                        SectionType,
                        0,
                        Buffer,
                        Size,
                        &AuthenticationStatus
                        );

    if (EFI_ERROR (Status) && (SectionType == EFI_SECTION_TE)) {
      //
      // Try reading PE32 section, since the TE section does not exist
      //
      *Buffer = NULL;
      *Size   = 0;
      Status  = Fv->ReadSection (
                      Fv,
                      NameGuid,
                      EFI_SECTION_PE32,
                      0,
                      Buffer,
                      Size,
                      &AuthenticationStatus
                      );
    }

    if (EFI_ERROR (Status) && 
        ((SectionType == EFI_SECTION_TE) || (SectionType == EFI_SECTION_PE32))) {
      //
      // Try reading raw file, since the desired section does not exist
      //
      *Buffer = NULL;
      *Size   = 0;
      Status  = Fv->ReadFile (
                      Fv,
                      NameGuid,
                      Buffer,
                      Size,
                      &FileType,
                      &Attributes,
                      &AuthenticationStatus
                      );
    }

    if (!EFI_ERROR (Status)) {
      break;
    }
  }
  gBS->FreePool(HandleBuffer);

  //
  // Not found image
  //
  if (Index == HandleCount) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

