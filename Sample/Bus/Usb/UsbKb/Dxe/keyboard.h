/*++
Copyright 2004, Intel Corporation                                                         
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
++*/

// TODO: fix comment to end with --*/
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "efikey.h"
#include "UsbDxeLib.h"

BOOLEAN
IsUSBKeyboard (
  IN  EFI_USB_IO_PROTOCOL       *UsbIo
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
InitUSBKeyboard (
  IN USB_KB_DEV   *UsbKeyboardDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbKeyboardDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
KeyboardHandler (
  IN  VOID          *Data,
  IN  UINTN         DataLength,
  IN  VOID          *Context,
  IN  UINT32        Result
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Data        - TODO: add argument description
  DataLength  - TODO: add argument description
  Context     - TODO: add argument description
  Result      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
USBKeyboardRecoveryHandler (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Event   - TODO: add argument description
  Context - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
USBParseKey (
  IN OUT  USB_KB_DEV  *UsbKeyboardDevice,
  OUT     UINT8       *KeyChar
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbKeyboardDevice - TODO: add argument description
  KeyChar           - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
USBKeyCodeToEFIScanCode (
  IN  USB_KB_DEV      *UsbKeyboardDevice,
  IN  UINT8           KeyChar,
  OUT EFI_INPUT_KEY   *Key
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbKeyboardDevice - TODO: add argument description
  KeyChar           - TODO: add argument description
  Key               - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
InitUSBKeyBuffer (
  IN OUT  USB_KB_BUFFER   *KeyboardBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  KeyboardBuffer  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsUSBKeyboardBufferEmpty (
  IN  USB_KB_BUFFER   KeyboardBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  KeyboardBuffer  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsUSBKeyboardBufferFull (
  IN  USB_KB_BUFFER   KeyboardBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  KeyboardBuffer  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
InsertKeyCode (
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  IN      UINT8         Key,
  IN      UINT8         Down
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  KeyboardBuffer  - TODO: add argument description
  Key             - TODO: add argument description
  Down            - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
RemoveKeyCode (
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  OUT     USB_KEY       *UsbKey
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  KeyboardBuffer  - TODO: add argument description
  UsbKey          - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
USBKeyboardRepeatHandler (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Event   - TODO: add argument description
  Context - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
SetKeyLED (
  IN  USB_KB_DEV    *UsbKeyboardDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbKeyboardDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
