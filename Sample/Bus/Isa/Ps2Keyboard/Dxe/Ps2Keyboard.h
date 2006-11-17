/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  Ps2Keyboard.h

Abstract:

  PS/2 keyboard driver header file

Revision History

--*/

#ifndef _PS2KEYBOARD_H
#define _PS2KEYBOARD_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiStatusCode.h"
#include "..\..\IsaIoDefinitions.h"

//
// Driver consumed protocol prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (Ps2Policy)
#include EFI_GUID_DEFINITION (GlobalVariable)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)

//
// Driver produced protocol prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)

//
// Driver Private Data
//
#define KEYBOARD_BUFFER_MAX_COUNT         32
#define KEYBOARD_CONSOLE_IN_DEV_SIGNATURE EFI_SIGNATURE_32 ('k', 'k', 'e', 'y')

typedef struct {
  UINTN                               Signature;

  EFI_HANDLE                          Handle;
  EFI_SIMPLE_TEXT_IN_PROTOCOL         ConIn;
  EFI_INTERFACE_DEFINITION_FOR_ISA_IO *IsaIo;

  EFI_EVENT                           TimerEvent;

  UINT32                              DataRegisterAddress;
  UINT32                              StatusRegisterAddress;
  UINT32                              CommandRegisterAddress;

  EFI_INPUT_KEY                       Key;

  BOOLEAN                             Ctrl;
  BOOLEAN                             Alt;
  BOOLEAN                             Shift;
  BOOLEAN                             CapsLock;
  BOOLEAN                             NumLock;
  BOOLEAN                             ScrollLock;

  //
  // Buffer storing key scancodes
  //
  UINT8                               ScancodeBuf[KEYBOARD_BUFFER_MAX_COUNT];
  UINT32                              ScancodeBufStartPos;
  UINT32                              ScancodeBufEndPos;
  UINT32                              ScancodeBufCount;

  //
  // Indicators of the key pressing state, used in detecting Alt+Ctrl+Del
  //
  BOOLEAN                             Ctrled;
  BOOLEAN                             Alted;

  //
  // Error state
  //
  BOOLEAN                             KeyboardErr;

  EFI_UNICODE_STRING_TABLE            *ControllerNameTable;

  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
} KEYBOARD_CONSOLE_IN_DEV;

#define KEYBOARD_CONSOLE_IN_DEV_FROM_THIS(a)  CR (a, KEYBOARD_CONSOLE_IN_DEV, ConIn, KEYBOARD_CONSOLE_IN_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gKeyboardControllerDriver;
extern EFI_COMPONENT_NAME_PROTOCOL  gPs2KeyboardComponentName;

//
// Driver entry point
//
EFI_STATUS
InstallPs2KeyboardDriver (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ImageHandle - GC_TODO: add argument description
  SystemTable - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

#define KEYBOARD_8042_DATA_REGISTER     0x60
#define KEYBOARD_8042_STATUS_REGISTER   0x64
#define KEYBOARD_8042_COMMAND_REGISTER  0x64

#define KEYBOARD_TIMEOUT                65536   // 0.07s
#define KEYBOARD_WAITFORVALUE_TIMEOUT   1000000 // 1s
#define KEYBOARD_BAT_TIMEOUT            4000000 // 4s
#define KEYBOARD_TIMER_INTERVAL         200000  // 0.02s
#define SCANCODE_EXTENDED               0xE0
#define SCANCODE_EXTENDED1              0xE1
#define SCANCODE_CTRL_MAKE              0x1D
#define SCANCODE_CTRL_BREAK             0x9D
#define SCANCODE_ALT_MAKE               0x38
#define SCANCODE_ALT_BREAK              0xB8
#define SCANCODE_LEFT_SHIFT_MAKE        0x2A
#define SCANCODE_LEFT_SHIFT_BREAK       0xAA
#define SCANCODE_RIGHT_SHIFT_MAKE       0x36
#define SCANCODE_RIGHT_SHIFT_BREAK      0xB6
#define SCANCODE_CAPS_LOCK_MAKE         0x3A
#define SCANCODE_NUM_LOCK_MAKE          0x45
#define SCANCODE_SCROLL_LOCK_MAKE       0x46
#define SCANCODE_MAX_MAKE               0x54

//
// Other functions that are used among .c files
//
EFI_STATUS
KeyboardRead (
  IN KEYBOARD_CONSOLE_IN_DEV  *ConsoleIn,
  OUT UINT8                   *Data
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ConsoleIn - GC_TODO: add argument description
  Data      - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
KeyGetchar (
  IN OUT KEYBOARD_CONSOLE_IN_DEV *ConsoleIn
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ConsoleIn - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
InitKeyboard (
  IN OUT KEYBOARD_CONSOLE_IN_DEV *ConsoleIn,
  IN BOOLEAN                     ExtendedVerification
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ConsoleIn             - GC_TODO: add argument description
  ExtendedVerification  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
DisableKeyboard (
  IN KEYBOARD_CONSOLE_IN_DEV *ConsoleIn
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ConsoleIn - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
EFIAPI
KeyboardTimerHandler (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Event   - GC_TODO: add argument description
  Context - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
KeyboardEfiReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  IN  BOOLEAN                      ExtendedVerification
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                  - GC_TODO: add argument description
  ExtendedVerification  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
KeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                *Key
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description
  Key   - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
EFIAPI
KeyboardWaitForKey (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Event   - GC_TODO: add argument description
  Context - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

UINT8
KeyReadStatusRegister (
  IN KEYBOARD_CONSOLE_IN_DEV *ConsoleIn
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ConsoleIn - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;
#endif
