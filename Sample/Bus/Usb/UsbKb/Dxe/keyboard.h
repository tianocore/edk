/*++
Copyright (c) 2004 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    Keyboard.h

Abstract:

    Function prototype for USB Keyboard Driver

Revision History
--*/

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "efikey.h"
#include "UsbDxeLib.h"

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#include "EfiHii.h"
#include "UefiIfrLibrary.h"

#define USB_KEYBOARD_LAYOUT_PACKAGE_GUID \
  { \
    0xc0f3b43, 0x44de, 0x4907, { 0xb4, 0x78, 0x22, 0x5f, 0x6f, 0x62, 0x89, 0xdc } \
  }

#define USB_KEYBOARD_LAYOUT_KEY_GUID \
  { \
    0x3a4d7a7c, 0x18a, 0x4b42, { 0x81, 0xb3, 0xdc, 0x10, 0xe3, 0xb5, 0x91, 0xbd } \
  }

#define USB_KEYBOARD_KEY_COUNT            104

#define USB_KEYBOARD_LANGUAGE_STR_LEN     5         // RFC4646 Language Code: "en-US"
#define USB_KEYBOARD_DESCRIPTION_STR_LEN  (16 + 1)  // Description: "English Keyboard"

#pragma pack (1)
typedef struct {
  //
  // This 4-bytes total array length is required by PreparePackageList()
  //
  UINT32                 Length;

  //
  // Keyboard Layout package definition
  //
  EFI_HII_PACKAGE_HEADER PackageHeader;
  UINT16                 LayoutCount;

  //
  // EFI_HII_KEYBOARD_LAYOUT
  //
  UINT16                 LayoutLength;
  EFI_GUID               Guid;
  UINT32                 LayoutDescriptorStringOffset;
  UINT8                  DescriptorCount;
  EFI_KEY_DESCRIPTOR     KeyDescriptor[USB_KEYBOARD_KEY_COUNT];
  UINT16                 DescriptionCount;
  CHAR16                 Language[USB_KEYBOARD_LANGUAGE_STR_LEN];
  CHAR16                 Space;
  CHAR16                 DescriptionString[USB_KEYBOARD_DESCRIPTION_STR_LEN];
} USB_KEYBOARD_LAYOUT_PACK_BIN;
#pragma pack()

#endif

BOOLEAN
IsUSBKeyboard (
  IN  EFI_USB_IO_PROTOCOL       *UsbIo
  );

EFI_STATUS
InitUSBKeyboard (
  IN USB_KB_DEV   *UsbKeyboardDevice
  );

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
EFI_STATUS
InitKeyboardLayout (
  IN USB_KB_DEV   *UsbKeyboardDevice
  );

VOID
ReleaseKeyboardLayoutResources (
  USB_KB_DEV  *UsbKeyboardDevice
  );
#endif

EFI_STATUS
EFIAPI
KeyboardHandler (
  IN  VOID          *Data,
  IN  UINTN         DataLength,
  IN  VOID          *Context,
  IN  UINT32        Result
  );

VOID
EFIAPI
USBKeyboardRecoveryHandler (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  );

EFI_STATUS
USBParseKey (
  IN OUT  USB_KB_DEV  *UsbKeyboardDevice,
  OUT     UINT8       *KeyChar
  );

EFI_STATUS
USBKeyCodeToEFIScanCode (
  IN  USB_KB_DEV      *UsbKeyboardDevice,
  IN  UINT8           KeyChar,
  OUT EFI_INPUT_KEY   *Key
  );

EFI_STATUS
InitUSBKeyBuffer (
  IN OUT  USB_KB_BUFFER   *KeyboardBuffer
  );

BOOLEAN
IsUSBKeyboardBufferEmpty (
  IN  USB_KB_BUFFER   KeyboardBuffer
  );

BOOLEAN
IsUSBKeyboardBufferFull (
  IN  USB_KB_BUFFER   KeyboardBuffer
  );

EFI_STATUS
InsertKeyCode (
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  IN      UINT8         Key,
  IN      UINT8         Down
  );

EFI_STATUS
RemoveKeyCode (
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  OUT     USB_KEY       *UsbKey
  );

VOID
EFIAPI
USBKeyboardRepeatHandler (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  );

EFI_STATUS
SetKeyLED (
  IN  USB_KB_DEV    *UsbKeyboardDevice
  );

#endif
