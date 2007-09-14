/*++

Copyright (c) 2006 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  WinNtGopDriver.c

Abstract:

  This file implements the UEFI Device Driver model requirements for GOP

  GOP is short hand for Graphics Output Protocol.

--*/

#include "WinNtGop.h"

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
STATIC
EFI_STATUS
FreeNotifyList (
  IN OUT EFI_LIST_ENTRY       *ListHead
  )
/*++

Routine Description:

Arguments:

  ListHead   - The list head

Returns:

  EFI_SUCCESS           - Free the notify list successfully
  EFI_INVALID_PARAMETER - ListHead is invalid.

--*/
{
  WIN_NT_GOP_SIMPLE_TEXTIN_EX_NOTIFY *NotifyNode;

  if (ListHead == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  while (!IsListEmpty (ListHead)) {
    NotifyNode = CR (
                   ListHead->ForwardLink, 
                   WIN_NT_GOP_SIMPLE_TEXTIN_EX_NOTIFY, 
                   NotifyEntry, 
                   WIN_NT_GOP_SIMPLE_TEXTIN_EX_NOTIFY_SIGNATURE
                   );
    RemoveEntryList (ListHead->ForwardLink);
    EfiLibSafeFreePool (NotifyNode);
  }
  
  return EFI_SUCCESS;
}

EFI_GUID gSimpleTextInExNotifyGuid = SIMPLE_TEXTIN_EX_NOTIFY_GUID;
#endif

EFI_DRIVER_BINDING_PROTOCOL gWinNtGopDriverBinding = {
  WinNtGopDriverBindingSupported,
  WinNtGopDriverBindingStart,
  WinNtGopDriverBindingStop,
  0xa,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT  (WinNtGopInitialize);

EFI_STATUS
EFIAPI
WinNtGopInitialize (
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable
  )
/*++

Routine Description:

  Intialize Win32 windows to act as EFI SimpleTextOut and SimpleTextIn windows. .

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:

  EFI_STATUS

--*/
// TODO:    ImageHandle - add argument and description to function comment
// TODO:    SystemTable - add argument and description to function comment
{
  return EfiLibInstallAllDriverProtocols (
          ImageHandle,
          SystemTable,
          &gWinNtGopDriverBinding,
          ImageHandle,
          &gWinNtGopComponentName,
          NULL,
          NULL
          );
}

EFI_STATUS
EFIAPI
WinNtGopDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Handle - add argument and description to function comment
// TODO:    RemainingDevicePath - add argument and description to function comment
{
  EFI_STATUS              Status;
  EFI_WIN_NT_IO_PROTOCOL  *WinNtIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiWinNtIoProtocolGuid,
                  &WinNtIo,
                  This->DriverBindingHandle,
                  Handle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = WinNtGopSupported (WinNtIo);

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
        Handle,
        &gEfiWinNtIoProtocolGuid,
        This->DriverBindingHandle,
        Handle
        );

  return Status;
}

EFI_STATUS
EFIAPI
WinNtGopDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Handle - add argument and description to function comment
// TODO:    RemainingDevicePath - add argument and description to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
{
  EFI_WIN_NT_IO_PROTOCOL  *WinNtIo;
  EFI_STATUS              Status;
  GOP_PRIVATE_DATA        *Private;

  //
  // Grab the protocols we need
  //
  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiWinNtIoProtocolGuid,
                  &WinNtIo,
                  This->DriverBindingHandle,
                  Handle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate Private context data for SGO inteface.
  //
  Private = NULL;
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (GOP_PRIVATE_DATA),
                  &Private
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Set up context record
  //
  Private->Signature            = GOP_PRIVATE_DATA_SIGNATURE;
  Private->Handle               = Handle;
  Private->WinNtThunk           = WinNtIo->WinNtThunk;

  Private->ControllerNameTable  = NULL;

  EfiLibAddUnicodeString (
    LANGUAGE_CODE_ENGLISH,
    gWinNtGopComponentName.SupportedLanguages,
    &Private->ControllerNameTable,
    WinNtIo->EnvString
    );

  Private->WindowName = WinNtIo->EnvString;

  Status              = WinNtGopConstructor (Private);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Publish the Gop interface to the world
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput,
                  &gEfiSimpleTextInProtocolGuid,
                  &Private->SimpleTextIn,
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
                  &gEfiSimpleTextInputExProtocolGuid,
                  &Private->SimpleTextInEx,
#endif                                    
                  NULL
                  );

Done:
  if (EFI_ERROR (Status)) {

    gBS->CloseProtocol (
          Handle,
          &gEfiWinNtIoProtocolGuid,
          This->DriverBindingHandle,
          Handle
          );

    if (Private != NULL) {
      //
      // On Error Free back private data
      //
      if (Private->ControllerNameTable != NULL) {
        EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);
      }

      if (Private->SimpleTextIn.WaitForKey != NULL) {
        gBS->CloseEvent (Private->SimpleTextIn.WaitForKey);
      }
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
      if (Private->SimpleTextInEx.WaitForKeyEx != NULL) {
        gBS->CloseEvent (Private->SimpleTextInEx.WaitForKeyEx);
      }
      FreeNotifyList (&Private->NotifyList);
#endif

      gBS->FreePool (Private);
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
WinNtGopDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Handle - add argument and description to function comment
// TODO:    NumberOfChildren - add argument and description to function comment
// TODO:    ChildHandleBuffer - add argument and description to function comment
// TODO:    EFI_NOT_STARTED - add return value to function comment
// TODO:    EFI_DEVICE_ERROR - add return value to function comment
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;
  EFI_STATUS                   Status;
  GOP_PRIVATE_DATA             *Private;

  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  &GraphicsOutput,
                  This->DriverBindingHandle,
                  Handle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    //
    // If the GOP interface does not exist the driver is not started
    //
    return EFI_NOT_STARTED;
  }

  //
  // Get our private context information
  //
  Private = GOP_PRIVATE_DATA_FROM_THIS (GraphicsOutput);

  //
  // Remove the SGO interface from the system
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Private->Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput,
                  &gEfiSimpleTextInProtocolGuid,
                  &Private->SimpleTextIn,
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
                  &gEfiSimpleTextInputExProtocolGuid,
                  &Private->SimpleTextInEx,
#endif                                    
                  NULL
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Shutdown the hardware
    //
    Status = WinNtGopDestructor (Private);
    if (EFI_ERROR (Status)) {
      return EFI_DEVICE_ERROR;
    }

    gBS->CloseProtocol (
          Handle,
          &gEfiWinNtIoProtocolGuid,
          This->DriverBindingHandle,
          Handle
          );

    //
    // Free our instance data
    //
    EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);
    Status = gBS->CloseEvent (Private->SimpleTextIn.WaitForKey);
    ASSERT_EFI_ERROR (Status);
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
    Status = gBS->CloseEvent (Private->SimpleTextInEx.WaitForKeyEx);
    ASSERT_EFI_ERROR (Status);
    FreeNotifyList (&Private->NotifyList);
#endif    
    gBS->FreePool (Private);

  }

  return Status;
}

UINTN
Atoi (
  CHAR16  *String
  )
/*++

Routine Description:

  Convert a unicode string to a UINTN

Arguments:

  String - Unicode string.

Returns:

  UINTN of the number represented by String.

--*/
{
  UINTN   Number;
  CHAR16  *Str;

  //
  // skip preceeding white space
  //
  Str = String;
  while ((*Str) && (*Str == ' ' || *Str == '"')) {
    Str++;
  }

  //
  // Convert ot a Number
  //
  Number = 0;
  while (*Str != '\0') {
    if ((*Str >= '0') && (*Str <= '9')) {
      Number = (Number * 10) +*Str - '0';
    } else {
      break;
    }

    Str++;
  }

  return Number;
}
