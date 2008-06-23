/*++

Copyright (c) 2004 - 2008, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Keyboard.c

Abstract:

  Helper functions for USB Keyboard Driver

Revision History

--*/

#include "keyboard.h"
#include "hid.h"

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)

#ifndef DISABLE_DEFAULT_USB_KEYBOARD_LAYOUT
//
// Static English keyboard layout
// Format:<efi key>, <unicode without shift>, <unicode with shift>, <Modifier>, <AffectedAttribute>
//
STATIC
UINT8 KeyboardLayoutTable[USB_KEYCODE_MAX_MAKE + 8][5] = {
  EfiKeyC1,         'a',      'A',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x04
  EfiKeyB5,         'b',      'B',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x05
  EfiKeyB3,         'c',      'C',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x06
  EfiKeyC3,         'd',      'D',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x07
  EfiKeyD3,         'e',      'E',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x08
  EfiKeyC4,         'f',      'F',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x09
  EfiKeyC5,         'g',      'G',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x0A
  EfiKeyC6,         'h',      'H',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x0B
  EfiKeyD8,         'i',      'I',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x0C
  EfiKeyC7,         'j',      'J',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x0D
  EfiKeyC8,         'k',      'K',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x0E
  EfiKeyC9,         'l',      'L',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x0F
  EfiKeyB7,         'm',      'M',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x10
  EfiKeyB6,         'n',      'N',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x11
  EfiKeyD9,         'o',      'O',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x12
  EfiKeyD10,        'p',      'P',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x13
  EfiKeyD1,         'q',      'Q',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x14
  EfiKeyD4,         'r',      'R',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x15
  EfiKeyC2,         's',      'S',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x16
  EfiKeyD5,         't',      'T',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x17
  EfiKeyD7,         'u',      'U',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x18
  EfiKeyB4,         'v',      'V',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x19
  EfiKeyD2,         'w',      'W',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x1A
  EfiKeyB2,         'x',      'X',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x1B
  EfiKeyD6,         'y',      'Y',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x1C
  EfiKeyB1,         'z',      'Z',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_CAPS_LOCK,   // 0x1D
  EfiKeyE1,         '1',      '!',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x1E
  EfiKeyE2,         '2',      '@',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x1F
  EfiKeyE3,         '3',      '#',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x20
  EfiKeyE4,         '4',      '$',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x21
  EfiKeyE5,         '5',      '%',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x22
  EfiKeyE6,         '6',      '^',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x23
  EfiKeyE7,         '7',      '&',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x24
  EfiKeyE8,         '8',      '*',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x25
  EfiKeyE9,         '9',      '(',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x26
  EfiKeyE10,        '0',      ')',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x27
  EfiKeyEnter,      0x0d,     0x0d,  EFI_NULL_MODIFIER,   0,                                // 0x28   Enter
  EfiKeyEsc,        0x1b,     0x1b,  EFI_NULL_MODIFIER,   0,                                // 0x29   Esc
  EfiKeyBackSpace,  0x08,     0x08,  EFI_NULL_MODIFIER,   0,                                // 0x2A   Backspace
  EfiKeyTab,        0x09,     0x09,  EFI_NULL_MODIFIER,   0,                                // 0x2B   Tab
  EfiKeySpaceBar,   ' ',      ' ',   EFI_NULL_MODIFIER,   0,                                // 0x2C   Spacebar
  EfiKeyE11,        '-',      '_',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x2D
  EfiKeyE12,        '=',      '+',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x2E
  EfiKeyD11,        '[',      '{',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x2F
  EfiKeyD12,        ']',      '}',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x30
  EfiKeyD13,        '\\',     '|',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x31
  EfiKeyC12,        '\\',     '|',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x32  Keyboard Non-US # and ~
  EfiKeyC10,        ';',      ':',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x33
  EfiKeyC11,        '\'',     '"',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x34
  EfiKeyE0,         '`',      '~',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x35  Keyboard Grave Accent and Tlide
  EfiKeyB8,         ',',      '<',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x36
  EfiKeyB9,         '.',      '>',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x37
  EfiKeyB10,        '/',      '?',   EFI_NULL_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT,   // 0x38
  EfiKeyCapsLock,   0x00,     0x00,  EFI_CAPS_LOCK_MODIFIER,            0,                  // 0x39   CapsLock
  EfiKeyF1,         0x00,     0x00,  EFI_FUNCTION_KEY_ONE_MODIFIER,     0,                  // 0x3A
  EfiKeyF2,         0x00,     0x00,  EFI_FUNCTION_KEY_TWO_MODIFIER,     0,                  // 0x3B
  EfiKeyF3,         0x00,     0x00,  EFI_FUNCTION_KEY_THREE_MODIFIER,   0,                  // 0x3C
  EfiKeyF4,         0x00,     0x00,  EFI_FUNCTION_KEY_FOUR_MODIFIER,    0,                  // 0x3D
  EfiKeyF5,         0x00,     0x00,  EFI_FUNCTION_KEY_FIVE_MODIFIER,    0,                  // 0x3E
  EfiKeyF6,         0x00,     0x00,  EFI_FUNCTION_KEY_SIX_MODIFIER,     0,                  // 0x3F
  EfiKeyF7,         0x00,     0x00,  EFI_FUNCTION_KEY_SEVEN_MODIFIER,   0,                  // 0x40
  EfiKeyF8,         0x00,     0x00,  EFI_FUNCTION_KEY_EIGHT_MODIFIER,   0,                  // 0x41
  EfiKeyF9,         0x00,     0x00,  EFI_FUNCTION_KEY_NINE_MODIFIER,    0,                  // 0x42
  EfiKeyF10,        0x00,     0x00,  EFI_FUNCTION_KEY_TEN_MODIFIER,     0,                  // 0x43
  EfiKeyF11,        0x00,     0x00,  EFI_FUNCTION_KEY_ELEVEN_MODIFIER,  0,                  // 0x44   F11
  EfiKeyF12,        0x00,     0x00,  EFI_FUNCTION_KEY_TWELVE_MODIFIER,  0,                  // 0x45   F12
  EfiKeyPrint,      0x00,     0x00,  EFI_PRINT_MODIFIER,                0,                  // 0x46   PrintScreen
  EfiKeySLck,       0x00,     0x00,  EFI_SCROLL_LOCK_MODIFIER,          0,                  // 0x47   Scroll Lock
  EfiKeyPause,      0x00,     0x00,  EFI_PAUSE_MODIFIER,                0,                  // 0x48   Pause
  EfiKeyIns,        0x00,     0x00,  EFI_INSERT_MODIFIER,               0,                  // 0x49
  EfiKeyHome,       0x00,     0x00,  EFI_HOME_MODIFIER,                 0,                  // 0x4A
  EfiKeyPgUp,       0x00,     0x00,  EFI_PAGE_UP_MODIFIER,              0,                  // 0x4B
  EfiKeyDel,        0x00,     0x00,  EFI_DELETE_MODIFIER,               0,                  // 0x4C
  EfiKeyEnd,        0x00,     0x00,  EFI_END_MODIFIER,                  0,                  // 0x4D
  EfiKeyPgDn,       0x00,     0x00,  EFI_PAGE_DOWN_MODIFIER,            0,                  // 0x4E
  EfiKeyRightArrow, 0x00,     0x00,  EFI_RIGHT_ARROW_MODIFIER,          0,                  // 0x4F
  EfiKeyLeftArrow,  0x00,     0x00,  EFI_LEFT_ARROW_MODIFIER,           0,                  // 0x50
  EfiKeyDownArrow,  0x00,     0x00,  EFI_DOWN_ARROW_MODIFIER,           0,                  // 0x51
  EfiKeyUpArrow,    0x00,     0x00,  EFI_UP_ARROW_MODIFIER,             0,                  // 0x52
  EfiKeyNLck,       0x00,     0x00,  EFI_NUM_LOCK_MODIFIER,             0,                  // 0x53   NumLock
  EfiKeySlash,      '/',      '/',   EFI_NULL_MODIFIER,                 0,                  // 0x54
  EfiKeyAsterisk,   '*',      '*',   EFI_NULL_MODIFIER,                 0,                  // 0x55
  EfiKeyMinus,      '-',      '-',   EFI_NULL_MODIFIER,                 0,                  // 0x56
  EfiKeyPlus,       '+',      '+',   EFI_NULL_MODIFIER,                 0,                  // 0x57
  EfiKeyEnter,      0x0d,     0x0d,  EFI_NULL_MODIFIER,                 0,                  // 0x58
  EfiKeyOne,        '1',      '1',   EFI_END_MODIFIER,         EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK,   // 0x59
  EfiKeyTwo,        '2',      '2',   EFI_DOWN_ARROW_MODIFIER,  EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK,   // 0x5A
  EfiKeyThree,      '3',      '3',   EFI_PAGE_DOWN_MODIFIER,   EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK,   // 0x5B
  EfiKeyFour,       '4',      '4',   EFI_LEFT_ARROW_MODIFIER,  EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK,   // 0x5C
  EfiKeyFive,       '5',      '5',   EFI_NULL_MODIFIER,        EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK,   // 0x5D
  EfiKeySix,        '6',      '6',   EFI_RIGHT_ARROW_MODIFIER, EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK,   // 0x5E
  EfiKeySeven,      '7',      '7',   EFI_HOME_MODIFIER,        EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK,   // 0x5F
  EfiKeyEight,      '8',      '8',   EFI_UP_ARROW_MODIFIER,    EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK,   // 0x60
  EfiKeyNine,       '9',      '9',   EFI_PAGE_UP_MODIFIER,     EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK,   // 0x61
  EfiKeyZero,       '0',      '0',   EFI_INSERT_MODIFIER,      EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK,   // 0x62
  EfiKeyPeriod,     '.',      '.',   EFI_DELETE_MODIFIER,      EFI_AFFECTED_BY_STANDARD_SHIFT | EFI_AFFECTED_BY_NUM_LOCK,   // 0x63
  EfiKeyB0,         '\\',     '|',   EFI_NULL_MODIFIER,        EFI_AFFECTED_BY_STANDARD_SHIFT, // 0x64 Keyboard Non-US \ and |
  EfiKeyA4,         0x00,     0x00,  EFI_MENU_MODIFIER,        0,                              // 0x65 Keyboard Application

  EfiKeyLCtrl,      0,        0,     EFI_LEFT_CONTROL_MODIFIER,    0,  // 0xe0
  EfiKeyLShift,     0,        0,     EFI_LEFT_SHIFT_MODIFIER,      0,  // 0xe1
  EfiKeyLAlt,       0,        0,     EFI_LEFT_ALT_MODIFIER,        0,  // 0xe2
  EfiKeyA0,         0,        0,     EFI_LEFT_LOGO_MODIFIER,       0,  // 0xe3
  EfiKeyRCtrl,      0,        0,     EFI_RIGHT_CONTROL_MODIFIER,   0,  // 0xe4
  EfiKeyRShift,     0,        0,     EFI_RIGHT_SHIFT_MODIFIER,     0,  // 0xe5
  EfiKeyA2,         0,        0,     EFI_RIGHT_ALT_MODIFIER,       0,  // 0xe6
  EfiKeyA3,         0,        0,     EFI_RIGHT_LOGO_MODIFIER,      0,  // 0xe7
};

VOID
LoadDefaultKeyboardLayout (
  IN USB_KB_DEV                 *UsbKeyboardDevice
  )
/*++

  Routine Description:
    Initialize KeyConvertionTable by using default keyboard layout.

  Arguments:
    UsbKeyboardDevice    The USB_KB_DEV instance.

  Returns:
    None.

--*/
{
  UINTN               Index;
  EFI_KEY_DESCRIPTOR  *KeyDescriptor;

  //
  // Construct KeyConvertionTable by default keyboard layout
  //
  KeyDescriptor = &UsbKeyboardDevice->KeyConvertionTable[0];

  for (Index = 0; Index < (USB_KEYCODE_MAX_MAKE + 8); Index++) {
    KeyDescriptor->Key                 = (EFI_KEY) KeyboardLayoutTable[Index][0];
    KeyDescriptor->Unicode             = KeyboardLayoutTable[Index][1];
    KeyDescriptor->ShiftedUnicode      = KeyboardLayoutTable[Index][2];
    KeyDescriptor->AltGrUnicode        = 0;
    KeyDescriptor->ShiftedAltGrUnicode = 0;
    KeyDescriptor->Modifier            = KeyboardLayoutTable[Index][3];
    KeyDescriptor->AffectedAttribute   = KeyboardLayoutTable[Index][4];

    KeyDescriptor++;
  }
}
#endif  //DISABLE_DEFAULT_USB_KEYBOARD_LAYOUT



//
// EFI_KEY to USB Scan Code convertion table
//
STATIC
UINT8 UsbScanCodeConvertionTable[] = {
  0xe0,  //  EfiKeyLCtrl
  0xe3,  //  EfiKeyA0
  0xe2,  //  EfiKeyLAlt
  0x2c,  //  EfiKeySpaceBar
  0xe6,  //  EfiKeyA2
  0xe7,  //  EfiKeyA3
  0x65,  //  EfiKeyA4
  0xe4,  //  EfiKeyRCtrl
  0x50,  //  EfiKeyLeftArrow
  0x51,  //  EfiKeyDownArrow
  0x4F,  //  EfiKeyRightArrow
  0x62,  //  EfiKeyZero
  0x63,  //  EfiKeyPeriod
  0x28,  //  EfiKeyEnter
  0xe1,  //  EfiKeyLShift
  0x64,  //  EfiKeyB0
  0x1D,  //  EfiKeyB1
  0x1B,  //  EfiKeyB2
  0x06,  //  EfiKeyB3
  0x19,  //  EfiKeyB4
  0x05,  //  EfiKeyB5
  0x11,  //  EfiKeyB6
  0x10,  //  EfiKeyB7
  0x36,  //  EfiKeyB8
  0x37,  //  EfiKeyB9
  0x38,  //  EfiKeyB10
  0xe5,  //  EfiKeyRShift
  0x52,  //  EfiKeyUpArrow
  0x59,  //  EfiKeyOne
  0x5A,  //  EfiKeyTwo
  0x5B,  //  EfiKeyThree
  0x39,  //  EfiKeyCapsLock
  0x04,  //  EfiKeyC1
  0x16,  //  EfiKeyC2
  0x07,  //  EfiKeyC3
  0x09,  //  EfiKeyC4
  0x0A,  //  EfiKeyC5
  0x0B,  //  EfiKeyC6
  0x0D,  //  EfiKeyC7
  0x0E,  //  EfiKeyC8
  0x0F,  //  EfiKeyC9
  0x33,  //  EfiKeyC10
  0x34,  //  EfiKeyC11
  0x32,  //  EfiKeyC12
  0x5C,  //  EfiKeyFour
  0x5D,  //  EfiKeyFive
  0x5E,  //  EfiKeySix
  0x57,  //  EfiKeyPlus
  0x2B,  //  EfiKeyTab
  0x14,  //  EfiKeyD1
  0x1A,  //  EfiKeyD2
  0x08,  //  EfiKeyD3
  0x15,  //  EfiKeyD4
  0x17,  //  EfiKeyD5
  0x1C,  //  EfiKeyD6
  0x18,  //  EfiKeyD7
  0x0C,  //  EfiKeyD8
  0x12,  //  EfiKeyD9
  0x13,  //  EfiKeyD10
  0x2F,  //  EfiKeyD11
  0x30,  //  EfiKeyD12
  0x31,  //  EfiKeyD13
  0x4C,  //  EfiKeyDel
  0x4D,  //  EfiKeyEnd
  0x4E,  //  EfiKeyPgDn
  0x5F,  //  EfiKeySeven
  0x60,  //  EfiKeyEight
  0x61,  //  EfiKeyNine
  0x35,  //  EfiKeyE0
  0x1E,  //  EfiKeyE1
  0x1F,  //  EfiKeyE2
  0x20,  //  EfiKeyE3
  0x21,  //  EfiKeyE4
  0x22,  //  EfiKeyE5
  0x23,  //  EfiKeyE6
  0x24,  //  EfiKeyE7
  0x25,  //  EfiKeyE8
  0x26,  //  EfiKeyE9
  0x27,  //  EfiKeyE10
  0x2D,  //  EfiKeyE11
  0x2E,  //  EfiKeyE12
  0x2A,  //  EfiKeyBackSpace
  0x49,  //  EfiKeyIns
  0x4A,  //  EfiKeyHome
  0x4B,  //  EfiKeyPgUp
  0x53,  //  EfiKeyNLck
  0x54,  //  EfiKeySlash
  0x55,  //  EfiKeyAsterisk
  0x56,  //  EfiKeyMinus
  0x29,  //  EfiKeyEsc
  0x3A,  //  EfiKeyF1
  0x3B,  //  EfiKeyF2
  0x3C,  //  EfiKeyF3
  0x3D,  //  EfiKeyF4
  0x3E,  //  EfiKeyF5
  0x3F,  //  EfiKeyF6
  0x40,  //  EfiKeyF7
  0x41,  //  EfiKeyF8
  0x42,  //  EfiKeyF9
  0x43,  //  EfiKeyF10
  0x44,  //  EfiKeyF11
  0x45,  //  EfiKeyF12
  0x46,  //  EfiKeyPrint
  0x47,  //  EfiKeySLck
  0x48   //  EfiKeyPause
};

//
// Keyboard Layout Modifier to EFI Scan Code convertion table
//
STATIC
UINT8 EfiScanCodeConvertionTable[] = {
  SCAN_NULL,       // EFI_NULL_MODIFIER
  SCAN_NULL,       // EFI_LEFT_CONTROL_MODIFIER
  SCAN_NULL,       // EFI_RIGHT_CONTROL_MODIFIER
  SCAN_NULL,       // EFI_LEFT_ALT_MODIFIER
  SCAN_NULL,       // EFI_RIGHT_ALT_MODIFIER
  SCAN_NULL,       // EFI_ALT_GR_MODIFIER
  SCAN_INSERT,     // EFI_INSERT_MODIFIER
  SCAN_DELETE,     // EFI_DELETE_MODIFIER
  SCAN_PAGE_DOWN,  // EFI_PAGE_DOWN_MODIFIER
  SCAN_PAGE_UP,    // EFI_PAGE_UP_MODIFIER
  SCAN_HOME,       // EFI_HOME_MODIFIER
  SCAN_END,        // EFI_END_MODIFIER
  SCAN_NULL,       // EFI_LEFT_SHIFT_MODIFIER
  SCAN_NULL,       // EFI_RIGHT_SHIFT_MODIFIER
  SCAN_NULL,       // EFI_CAPS_LOCK_MODIFIER
  SCAN_NULL,       // EFI_NUM_LOCK_MODIFIER
  SCAN_LEFT,       // EFI_LEFT_ARROW_MODIFIER
  SCAN_RIGHT,      // EFI_RIGHT_ARROW_MODIFIER
  SCAN_DOWN,       // EFI_DOWN_ARROW_MODIFIER
  SCAN_UP,         // EFI_UP_ARROW_MODIFIER
  SCAN_NULL,       // EFI_NS_KEY_MODIFIER
  SCAN_NULL,       // EFI_NS_KEY_DEPENDENCY_MODIFIER
  SCAN_F1,         // EFI_FUNCTION_KEY_ONE_MODIFIER
  SCAN_F2,         // EFI_FUNCTION_KEY_TWO_MODIFIER
  SCAN_F3,         // EFI_FUNCTION_KEY_THREE_MODIFIER
  SCAN_F4,         // EFI_FUNCTION_KEY_FOUR_MODIFIER
  SCAN_F5,         // EFI_FUNCTION_KEY_FIVE_MODIFIER
  SCAN_F6,         // EFI_FUNCTION_KEY_SIX_MODIFIER
  SCAN_F7,         // EFI_FUNCTION_KEY_SEVEN_MODIFIER
  SCAN_F8,         // EFI_FUNCTION_KEY_EIGHT_MODIFIER
  SCAN_F9,         // EFI_FUNCTION_KEY_NINE_MODIFIER
  SCAN_F10,        // EFI_FUNCTION_KEY_TEN_MODIFIER
  SCAN_F11,        // EFI_FUNCTION_KEY_ELEVEN_MODIFIER
  SCAN_F12,        // EFI_FUNCTION_KEY_TWELVE_MODIFIER
};

EFI_GUID  mKeyboardLayoutEventGuid = EFI_HII_SET_KEYBOARD_LAYOUT_EVENT_GUID;

#else   // EFI_SPECIFICATION_VERSION < 0x0002000A

//
// USB Key Code to Efi key mapping table
// Format:<efi scan code>, <unicode without shift>, <unicode with shift>
//
STATIC
UINT8 KeyConvertionTable[USB_KEYCODE_MAX_MAKE][3] = {
    SCAN_NULL,      'a',      'A',      // 0x04
    SCAN_NULL,      'b',      'B',      // 0x05
    SCAN_NULL,      'c',      'C',      // 0x06
    SCAN_NULL,      'd',      'D',      // 0x07
    SCAN_NULL,      'e',      'E',      // 0x08
    SCAN_NULL,      'f',      'F',      // 0x09
    SCAN_NULL,      'g',      'G',      // 0x0A
    SCAN_NULL,      'h',      'H',      // 0x0B
    SCAN_NULL,      'i',      'I',      // 0x0C
    SCAN_NULL,      'j',      'J',      // 0x0D
    SCAN_NULL,      'k',      'K',      // 0x0E
    SCAN_NULL,      'l',      'L',      // 0x0F
    SCAN_NULL,      'm',      'M',      // 0x10
    SCAN_NULL,      'n',      'N',      // 0x11
    SCAN_NULL,      'o',      'O',      // 0x12
    SCAN_NULL,      'p',      'P',      // 0x13
    SCAN_NULL,      'q',      'Q',      // 0x14
    SCAN_NULL,      'r',      'R',      // 0x15
    SCAN_NULL,      's',      'S',      // 0x16
    SCAN_NULL,      't',      'T',      // 0x17
    SCAN_NULL,      'u',      'U',      // 0x18
    SCAN_NULL,      'v',      'V',      // 0x19
    SCAN_NULL,      'w',      'W',      // 0x1A
    SCAN_NULL,      'x',      'X',      // 0x1B
    SCAN_NULL,      'y',      'Y',      // 0x1C
    SCAN_NULL,      'z',      'Z',      // 0x1D
    SCAN_NULL,      '1',      '!',      // 0x1E
    SCAN_NULL,      '2',      '@',      // 0x1F
    SCAN_NULL,      '3',      '#',      // 0x20
    SCAN_NULL,      '4',      '$',      // 0x21
    SCAN_NULL,      '5',      '%',      // 0x22
    SCAN_NULL,      '6',      '^',      // 0x23
    SCAN_NULL,      '7',      '&',      // 0x24
    SCAN_NULL,      '8',      '*',      // 0x25
    SCAN_NULL,      '9',      '(',      // 0x26
    SCAN_NULL,      '0',      ')',      // 0x27
    SCAN_NULL,      0x0d,     0x0d,     // 0x28   Enter
    SCAN_ESC,       0x00,     0x00,     // 0x29   Esc
    SCAN_NULL,      0x08,     0x08,     // 0x2A   Backspace
    SCAN_NULL,      0x09,     0x09,     // 0x2B   Tab
    SCAN_NULL,      ' ',      ' ',      // 0x2C   Spacebar
    SCAN_NULL,      '-',      '_',      // 0x2D
    SCAN_NULL,      '=',      '+',      // 0x2E
    SCAN_NULL,      '[',      '{',      // 0x2F
    SCAN_NULL,      ']',      '}',      // 0x30
    SCAN_NULL,      '\\',     '|',      // 0x31
    SCAN_NULL,      '\\',     '|',      // 0x32  Keyboard US \ and |
    SCAN_NULL,      ';',      ':',      // 0x33
    SCAN_NULL,      '\'',     '"',      // 0x34
    SCAN_NULL,      '`',      '~',      // 0x35  Keyboard Grave Accent and Tlide
    SCAN_NULL,      ',',      '<',      // 0x36
    SCAN_NULL,      '.',      '>',      // 0x37
    SCAN_NULL,      '/',      '?',      // 0x38
    SCAN_NULL,      0x00,     0x00,     // 0x39   CapsLock
    SCAN_F1,        0x00,     0x00,     // 0x3A
    SCAN_F2,        0x00,     0x00,     // 0x3B
    SCAN_F3,        0x00,     0x00,     // 0x3C  
    SCAN_F4,        0x00,     0x00,     // 0x3D  
    SCAN_F5,        0x00,     0x00,     // 0x3E
    SCAN_F6,        0x00,     0x00,     // 0x3F
    SCAN_F7,        0x00,     0x00,     // 0x40
    SCAN_F8,        0x00,     0x00,     // 0x41
    SCAN_F9,        0x00,     0x00,     // 0x42
    SCAN_F10,       0x00,     0x00,     // 0x43
    SCAN_F11,       0x00,     0x00,     // 0x44   F11
    SCAN_F12,       0x00,     0x00,     // 0x45   F12
    SCAN_NULL,      0x00,     0x00,     // 0x46   PrintScreen
    SCAN_NULL,      0x00,     0x00,     // 0x47   Scroll Lock
    SCAN_NULL,      0x00,     0x00,     // 0x48   Pause
    SCAN_INSERT,    0x00,     0x00,     // 0x49
    SCAN_HOME,      0x00,     0x00,     // 0x4A
    SCAN_PAGE_UP,   0x00,     0x00,     // 0x4B
    SCAN_DELETE,    0x00,     0x00,     // 0x4C
    SCAN_END,       0x00,     0x00,     // 0x4D
    SCAN_PAGE_DOWN, 0x00,     0x00,     // 0x4E
    SCAN_RIGHT,     0x00,     0x00,     // 0x4F
    SCAN_LEFT,      0x00,     0x00,     // 0x50
    SCAN_DOWN,      0x00,     0x00,     // 0x51
    SCAN_UP,        0x00,     0x00,     // 0x52
    SCAN_NULL,      0x00,     0x00,     // 0x53   NumLock
    SCAN_NULL,      '/',      '/',      // 0x54
    SCAN_NULL,      '*',      '*',      // 0x55
    SCAN_NULL,      '-',      '-',      // 0x56
    SCAN_NULL,      '+',      '+',      // 0x57
    SCAN_NULL,      0x0d,     0x0d,     // 0x58
    SCAN_END,       '1',      '1',      // 0x59
    SCAN_DOWN,      '2',      '2',      // 0x5A
    SCAN_PAGE_DOWN, '3',      '3',      // 0x5B
    SCAN_LEFT,      '4',      '4',      // 0x5C
    SCAN_NULL,      '5',      '5',      // 0x5D
    SCAN_RIGHT,     '6',      '6',      // 0x5E
    SCAN_HOME,      '7',      '7',      // 0x5F
    SCAN_UP,        '8',      '8',      // 0x60
    SCAN_PAGE_UP,   '9',      '9',      // 0x61
    SCAN_INSERT,    '0',      '0',      // 0x62
    SCAN_DELETE,    '.',      '.',      // 0x63
    SCAN_NULL,      '\\',     '|',      // 0x64 Keyboard Non-US \ and |
    SCAN_NULL,      0x00,     0x00,     // 0x65 Keyboard Application
    SCAN_NULL,      0x00,     0x00,     // 0x66 Keyboard Power
    SCAN_NULL,      '=' ,     '='       // 0x67 Keypad =
};

#endif

STATIC KB_MODIFIER  KB_Mod[8] = {
  { MOD_CONTROL_L,  0xe0 }, // 11100000
  { MOD_CONTROL_R,  0xe4 }, // 11100100
  { MOD_SHIFT_L,    0xe1 }, // 11100001
  { MOD_SHIFT_R,    0xe5 }, // 11100101
  { MOD_ALT_L,      0xe2 }, // 11100010
  { MOD_ALT_R,      0xe6 }, // 11100110
  { MOD_WIN_L,      0xe3 }, // 11100011
  { MOD_WIN_R,      0xe7 }, // 11100111
};

BOOLEAN
IsUSBKeyboard (
  IN  EFI_USB_IO_PROTOCOL       *UsbIo
  )
/*++

  Routine Description:
    Uses USB I/O to check whether the device is a USB Keyboard device.

  Arguments:
    UsbIo:    Points to a USB I/O protocol instance.

  Returns:

--*/
{
  EFI_STATUS                    Status;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDescriptor;

  //
  // Get the Default interface descriptor, currently we
  // assume it is interface 1
  //
  Status = UsbIo->UsbGetInterfaceDescriptor (
                    UsbIo,
                    &InterfaceDescriptor
                    );

  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (InterfaceDescriptor.InterfaceClass == CLASS_HID &&
      InterfaceDescriptor.InterfaceSubClass == SUBCLASS_BOOT &&
      InterfaceDescriptor.InterfaceProtocol == PROTOCOL_KEYBOARD
      ) {

    return TRUE;
  }

  return FALSE;
}

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)

EFI_HII_KEYBOARD_LAYOUT *
GetCurrentKeyboardLayout (
  VOID
  )
/*++

  Routine Description:
    Get current keyboard layout from HII database.

  Arguments:
    None.

  Returns:
    Pointer to EFI_HII_KEYBOARD_LAYOUT.

--*/
{
  EFI_STATUS                Status;
  EFI_HII_DATABASE_PROTOCOL *HiiDatabase;
  EFI_HII_KEYBOARD_LAYOUT   *KeyboardLayout;
  UINT16                    Length;

  //
  // Locate Hii database protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  &HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Get current keyboard layout from HII database
  //
  Length = 0;
  KeyboardLayout = NULL;
  Status = HiiDatabase->GetKeyboardLayout (
                          HiiDatabase,
                          NULL,
                          &Length,
                          KeyboardLayout
                          );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    KeyboardLayout = EfiLibAllocatePool (Length);
    ASSERT (KeyboardLayout != NULL);

    Status = HiiDatabase->GetKeyboardLayout (
                            HiiDatabase,
                            NULL,
                            &Length,
                            KeyboardLayout
                            );
    if (EFI_ERROR (Status)) {
      gBS->FreePool (KeyboardLayout);
      KeyboardLayout = NULL;
    }
  }

  return KeyboardLayout;
}

EFI_KEY_DESCRIPTOR *
GetKeyDescriptor (
  IN USB_KB_DEV        *UsbKeyboardDevice,
  IN UINT8             ScanCode
  )
/*++

  Routine Description:
    Find Key Descriptor in KeyConvertionTable given its scan code.

  Arguments:
    UsbKeyboardDevice  -  The USB_KB_DEV instance.
    ScanCode           -  USB scan code.

  Returns:
    The Key descriptor in KeyConvertionTable.

--*/
{
  UINT8  Index;

  if (((ScanCode > 0x65) && (ScanCode < 0xe0)) || (ScanCode > 0xe7)) {
    return NULL;
  }

  if (ScanCode <= 0x65) {
    Index = ScanCode - 4;
  } else {
    Index = ScanCode - 0xe0 + USB_KEYCODE_MAX_MAKE;
  }

  return &UsbKeyboardDevice->KeyConvertionTable[Index];
}

USB_NS_KEY *
FindUsbNsKey (
  IN USB_KB_DEV          *UsbKeyboardDevice,
  IN EFI_KEY_DESCRIPTOR  *KeyDescriptor
  )
/*++

  Routine Description:
    Find Non-Spacing key for given KeyDescriptor.

  Arguments:
    UsbKeyboardDevice  -  The USB_KB_DEV instance.
    KeyDescriptor      -  Key descriptor.

  Returns:
    The Non-Spacing key.

--*/
{
  EFI_LIST_ENTRY  *Link;
  USB_NS_KEY      *UsbNsKey;

  Link = GetFirstNode (&UsbKeyboardDevice->NsKeyList);
  while (!IsNull (&UsbKeyboardDevice->NsKeyList, Link)) {
    UsbNsKey = USB_NS_KEY_FORM_FROM_LINK (Link);

    if (UsbNsKey->NsKey[0].Key == KeyDescriptor->Key) {
      return UsbNsKey;
    }

    Link = GetNextNode (&UsbKeyboardDevice->NsKeyList, Link);
  }

  return NULL;
}

EFI_KEY_DESCRIPTOR *
FindPhysicalKey (
  IN USB_NS_KEY          *UsbNsKey,
  IN EFI_KEY_DESCRIPTOR  *KeyDescriptor
  )
/*++

  Routine Description:
    Find physical key definition for a given Key stroke.

  Arguments:
    UsbNsKey        -  The Non-Spacing key information.
    KeyDescriptor   -  The key stroke.

  Returns:
    The physical key definition.

--*/
{
  UINTN               Index;
  EFI_KEY_DESCRIPTOR  *PhysicalKey;

  PhysicalKey = &UsbNsKey->NsKey[1];
  for (Index = 0; Index < UsbNsKey->KeyCount; Index++) {
    if (KeyDescriptor->Key == PhysicalKey->Key) {
      return PhysicalKey;
    }

    PhysicalKey++;
  }

  //
  // No children definition matched, return original key
  //
  return KeyDescriptor;
}

VOID
EFIAPI
SetKeyboardLayoutEvent (
  EFI_EVENT                  Event,
  VOID                       *Context
  )
/*++

  Routine Description:
    The notification function for SET_KEYBOARD_LAYOUT_EVENT.

  Arguments:

  Returns:

--*/
{
  USB_KB_DEV                *UsbKeyboardDevice;
  EFI_HII_KEYBOARD_LAYOUT   *KeyboardLayout;
  EFI_KEY_DESCRIPTOR        TempKey;
  EFI_KEY_DESCRIPTOR        *KeyDescriptor;
  EFI_KEY_DESCRIPTOR        *TableEntry;
  EFI_KEY_DESCRIPTOR        *NsKey;
  USB_NS_KEY                *UsbNsKey;
  UINTN                     Index;
  UINTN                     Index2;
  UINTN                     KeyCount;
  UINT8                     ScanCode;

  UsbKeyboardDevice = (USB_KB_DEV *) Context;

  //
  // Try to get current Keyboard Layout from HII database
  //
  KeyboardLayout = GetCurrentKeyboardLayout ();
  if (KeyboardLayout == NULL) {
    return;
  }

  //
  // Allocate resource for KeyConvertionTable
  //
  ReleaseKeyboardLayoutResources (UsbKeyboardDevice);
  UsbKeyboardDevice->KeyConvertionTable = EfiLibAllocateZeroPool ((USB_KEYCODE_MAX_MAKE + 8) * sizeof (EFI_KEY_DESCRIPTOR));
  ASSERT (UsbKeyboardDevice->KeyConvertionTable != NULL);

  KeyDescriptor = (EFI_KEY_DESCRIPTOR *) (((UINT8 *) KeyboardLayout) + sizeof (EFI_HII_KEYBOARD_LAYOUT));
  for (Index = 0; Index < KeyboardLayout->DescriptorCount; Index++) {
    //
    // Copy from HII keyboard layout package binary for alignment
    //
    EfiCopyMem (&TempKey, KeyDescriptor, sizeof (EFI_KEY_DESCRIPTOR));

    //
    // Fill the key into KeyConvertionTable (which use USB Scan Code as index)
    //
    ScanCode = UsbScanCodeConvertionTable [(UINT8) (TempKey.Key)];
    TableEntry = GetKeyDescriptor (UsbKeyboardDevice, ScanCode);
    EfiCopyMem (TableEntry, KeyDescriptor, sizeof (EFI_KEY_DESCRIPTOR));

    if (TempKey.Modifier == EFI_NS_KEY_MODIFIER) {
      //
      // Non-spacing key
      //
      UsbNsKey = EfiLibAllocatePool (sizeof (USB_NS_KEY));
      ASSERT (UsbNsKey != NULL);

      //
      // Search for sequential children physical key definitions
      //
      KeyCount = 0;
      NsKey = KeyDescriptor + 1;
      for (Index2 = Index + 1; Index2 < KeyboardLayout->DescriptorCount; Index2++) {
        EfiCopyMem (&TempKey, NsKey, sizeof (EFI_KEY_DESCRIPTOR));
        if (TempKey.Modifier & EFI_NS_KEY_DEPENDENCY_MODIFIER) {
          KeyCount++;
        } else {
          break;
        }
        NsKey++;
      }

      UsbNsKey->Signature = USB_NS_KEY_SIGNATURE;
      UsbNsKey->KeyCount = KeyCount;
      UsbNsKey->NsKey = EfiLibAllocateCopyPool (
                          (KeyCount + 1) * sizeof (EFI_KEY_DESCRIPTOR),
                          KeyDescriptor
                          );
      InsertTailList (&UsbKeyboardDevice->NsKeyList, &UsbNsKey->Link);

      //
      // Skip over the child physical keys
      //
      Index += KeyCount;
      KeyDescriptor += KeyCount;
    }

    KeyDescriptor++;
  }

  //
  // There are two EfiKeyEnter, duplicate its Key Descriptor
  //
  TableEntry = GetKeyDescriptor (UsbKeyboardDevice, 0x58);
  KeyDescriptor = GetKeyDescriptor (UsbKeyboardDevice, 0x28);
  EfiCopyMem (TableEntry, KeyDescriptor, sizeof (EFI_KEY_DESCRIPTOR));

  gBS->FreePool (KeyboardLayout);
}

VOID
ReleaseKeyboardLayoutResources (
  IN USB_KB_DEV              *UsbKeyboardDevice
  )
/*++

  Routine Description:
    Destroy resources for Keyboard layout.

  Arguments:
    UsbKeyboardDevice  -  The USB_KB_DEV instance.

  Returns:
    None.

--*/
{
  USB_NS_KEY      *UsbNsKey;
  EFI_LIST_ENTRY  *Link;

  EfiLibSafeFreePool (UsbKeyboardDevice->KeyConvertionTable);
  UsbKeyboardDevice->KeyConvertionTable = NULL;

  while (!IsListEmpty (&UsbKeyboardDevice->NsKeyList)) {
    Link = GetFirstNode (&UsbKeyboardDevice->NsKeyList);
    UsbNsKey = USB_NS_KEY_FORM_FROM_LINK (Link);
    RemoveEntryList (&UsbNsKey->Link);

    gBS->FreePool (UsbNsKey->NsKey);
    gBS->FreePool (UsbNsKey);
  }
}

EFI_STATUS
InitKeyboardLayout (
  IN USB_KB_DEV   *UsbKeyboardDevice
  )
/*++

  Routine Description:
    Initialize USB Keyboard layout.

  Arguments:
    UsbKeyboardDevice    The USB_KB_DEV instance.

  Returns:
    EFI_SUCCESS  - Success
    Other        - Keyboard layout initial failed.
--*/
{
  EFI_HII_KEYBOARD_LAYOUT   *KeyboardLayout;
  EFI_STATUS                Status;

  UsbKeyboardDevice->KeyConvertionTable = EfiLibAllocateZeroPool ((USB_KEYCODE_MAX_MAKE + 8) * sizeof (EFI_KEY_DESCRIPTOR));
  ASSERT (UsbKeyboardDevice->KeyConvertionTable != NULL);

  InitializeListHead (&UsbKeyboardDevice->NsKeyList);
  UsbKeyboardDevice->CurrentNsKey = NULL;
  UsbKeyboardDevice->KeyboardLayoutEvent = NULL;

  //
  // Register SET_KEYBOARD_LAYOUT_EVENT notification
  //
  Status = gBS->CreateEventEx (
                  EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_NOTIFY,
                  SetKeyboardLayoutEvent,
                  UsbKeyboardDevice,
                  &mKeyboardLayoutEventGuid,
                  &UsbKeyboardDevice->KeyboardLayoutEvent
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Try to get current keyboard layout from HII database
  //
  KeyboardLayout = GetCurrentKeyboardLayout ();
  if (KeyboardLayout != NULL) {
    //
    // Force to initialize the keyboard layout
    //
    gBS->SignalEvent (UsbKeyboardDevice->KeyboardLayoutEvent);
  } else {
#ifndef DISABLE_DEFAULT_USB_KEYBOARD_LAYOUT
    //
    // Fail to get keyboard layout from HII database,
    // use default keyboard layout
    //
    LoadDefaultKeyboardLayout (UsbKeyboardDevice);
#else
    return EFI_NOT_READY;
#endif
  }

  return EFI_SUCCESS;
}

#endif  // EFI_SPECIFICATION_VERSION >= 0x0002000A

EFI_STATUS
InitUSBKeyboard (
  IN USB_KB_DEV   *UsbKeyboardDevice
  )
/*++

  Routine Description:
    Initialize USB Keyboard device and all private data structures.

  Arguments:
    UsbKeyboardDevice    The USB_KB_DEV instance.

  Returns:
    EFI_SUCCESS      - Success
    EFI_DEVICE_ERROR - Hardware Error
--*/
{
  UINT8               ConfigValue;
  UINT8               Protocol;
  UINT8               ReportId;
  UINT8               Duration;
  EFI_STATUS          Status;
  UINT32              TransferResult;
  EFI_USB_IO_PROTOCOL *UsbIo;

  UsbIo = UsbKeyboardDevice->UsbIo;

  KbdReportStatusCode (
    UsbKeyboardDevice->DevicePath,
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_SELF_TEST)
    );

  InitUSBKeyBuffer (&(UsbKeyboardDevice->KeyboardBuffer));

  //
  // default configurations
  //
  ConfigValue = 0x01;

  //
  // Uses default configuration to configure the USB Keyboard device.
  //
  Status = UsbSetConfiguration (
            UsbKeyboardDevice->UsbIo,
            (UINT16) ConfigValue,
            &TransferResult
            );
  if (EFI_ERROR (Status)) {
    //
    // If configuration could not be set here, it means
    // the keyboard interface has some errors and could
    // not be initialized
    //
    KbdReportStatusCode (
      UsbKeyboardDevice->DevicePath,
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      (EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_INTERFACE_ERROR)
      );

    return EFI_DEVICE_ERROR;
  }

  UsbGetProtocolRequest (
    UsbKeyboardDevice->UsbIo,
    UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
    &Protocol
    );
  //
  // Sets boot protocol for the USB Keyboard.
  // This driver only supports boot protocol.
  // !!BugBug: How about the device that does not support boot protocol?
  //
  if (Protocol != BOOT_PROTOCOL) {
    UsbSetProtocolRequest (
      UsbKeyboardDevice->UsbIo,
      UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
      BOOT_PROTOCOL
      );
  }
  //
  // the duration is indefinite, so the endpoint will inhibit reporting forever,
  // and only reporting when a change is detected in the report data.
  //

  //
  // idle value for all report ID
  //
  ReportId = 0;
  //
  // idle forever until there is a key pressed and released.
  //
  Duration = 0;
  UsbSetIdleRequest (
    UsbKeyboardDevice->UsbIo,
    UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
    ReportId,
    Duration
    );

  UsbKeyboardDevice->CtrlOn     = 0;
  UsbKeyboardDevice->AltOn      = 0;
  UsbKeyboardDevice->ShiftOn    = 0;
  UsbKeyboardDevice->NumLockOn  = 0;
  UsbKeyboardDevice->CapsOn     = 0;
  UsbKeyboardDevice->ScrollOn   = 0;

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#ifndef DISABLE_CONSOLE_EX
  UsbKeyboardDevice->LeftCtrlOn   = 0;
  UsbKeyboardDevice->LeftAltOn    = 0;
  UsbKeyboardDevice->LeftShiftOn  = 0;
  UsbKeyboardDevice->LeftLogoOn   = 0;
  UsbKeyboardDevice->RightCtrlOn  = 0;
  UsbKeyboardDevice->RightAltOn   = 0;
  UsbKeyboardDevice->RightShiftOn = 0;
  UsbKeyboardDevice->RightLogoOn  = 0;
  UsbKeyboardDevice->MenuKeyOn    = 0;
  UsbKeyboardDevice->SysReqOn     = 0;
#endif  // DISABLE_CONSOLE_EX

  UsbKeyboardDevice->AltGrOn      = 0;

  UsbKeyboardDevice->CurrentNsKey = NULL;
#endif

  //
  // Sync the initial state of lights
  //
  SetKeyLED (UsbKeyboardDevice);

  EfiZeroMem (UsbKeyboardDevice->LastKeyCodeArray, sizeof (UINT8) * 8);

  //
  // Set a timer for repeat keys' generation.
  //
  if (UsbKeyboardDevice->RepeatTimer) {
    gBS->CloseEvent (UsbKeyboardDevice->RepeatTimer);
    UsbKeyboardDevice->RepeatTimer = 0;
  }

  Status = gBS->CreateEvent (
                  EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_NOTIFY,
                  USBKeyboardRepeatHandler,
                  UsbKeyboardDevice,
                  &UsbKeyboardDevice->RepeatTimer
                  );

  if (UsbKeyboardDevice->DelayedRecoveryEvent) {
    gBS->CloseEvent (UsbKeyboardDevice->DelayedRecoveryEvent);
    UsbKeyboardDevice->DelayedRecoveryEvent = 0;
  }

  Status = gBS->CreateEvent (
                  EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_NOTIFY,
                  USBKeyboardRecoveryHandler,
                  UsbKeyboardDevice,
                  &UsbKeyboardDevice->DelayedRecoveryEvent
                  );

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
KeyboardHandler (
  IN  VOID          *Data,
  IN  UINTN         DataLength,
  IN  VOID          *Context,
  IN  UINT32        Result
  )
/*++

  Routine Description:
    Handler function for USB Keyboard's asynchronous interrupt transfer.

  Arguments:
    Data       A pointer to a buffer that is filled with key data which is
               retrieved via asynchronous interrupt transfer.
    DataLength Indicates the size of the data buffer.
    Context    Pointing to USB_KB_DEV instance.
    Result     Indicates the result of the asynchronous interrupt transfer.

  Returns:
    EFI_SUCCESS      - Success
    EFI_DEVICE_ERROR - Hardware Error
--*/
{
  USB_KB_DEV          *UsbKeyboardDevice;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               *CurKeyCodeBuffer;
  UINT8               *OldKeyCodeBuffer;
  UINT8               CurModifierMap;
  UINT8               OldModifierMap;
  UINT8               Index;
  UINT8               Index2;
  BOOLEAN             Down;
  EFI_STATUS          Status;
  BOOLEAN             KeyRelease;
  BOOLEAN             KeyPress;
  UINT8               SavedTail;
  USB_KEY             UsbKey;
  UINT8               NewRepeatKey;
  UINT32              UsbStatus;
  UINT8               *DataPtr;
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  EFI_KEY_DESCRIPTOR  *KeyDescriptor;
#endif

  ASSERT (Context);

  NewRepeatKey      = 0;
  DataPtr           = (UINT8 *) Data;
  UsbKeyboardDevice = (USB_KB_DEV *) Context;
  UsbIo             = UsbKeyboardDevice->UsbIo;

  //
  // Analyzes the Result and performs corresponding action.
  //
  if (Result != EFI_USB_NOERROR) {
    //
    // Some errors happen during the process
    //
    KbdReportStatusCode (
      UsbKeyboardDevice->DevicePath,
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      (EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_INPUT_ERROR)
      );

    //
    // stop the repeat key generation if any
    //
    UsbKeyboardDevice->RepeatKey = 0;

    gBS->SetTimer (
          UsbKeyboardDevice->RepeatTimer,
          TimerCancel,
          USBKBD_REPEAT_RATE
          );

    if ((Result & EFI_USB_ERR_STALL) == EFI_USB_ERR_STALL) {
      UsbClearEndpointHalt (
        UsbIo,
        UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
        &UsbStatus
        );
    }

    //
    // Delete & Submit this interrupt again
    //

    Status = UsbIo->UsbAsyncInterruptTransfer (
                      UsbIo,
                      UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
                      FALSE,
                      0,
                      0,
                      NULL,
                      NULL
                      );

    gBS->SetTimer (
          UsbKeyboardDevice->DelayedRecoveryEvent,
          TimerRelative,
          EFI_USB_INTERRUPT_DELAY
          );

    return EFI_DEVICE_ERROR;
  }

  if (DataLength == 0 || Data == NULL) {
    return EFI_SUCCESS;
  }

  CurKeyCodeBuffer  = (UINT8 *) Data;
  OldKeyCodeBuffer  = UsbKeyboardDevice->LastKeyCodeArray;

  //
  // checks for new key stroke.
  // if no new key got, return immediately.
  //
  for (Index = 0; Index < 8; Index++) {
    if (OldKeyCodeBuffer[Index] != CurKeyCodeBuffer[Index]) {
      break;
    }
  }

  if (Index == 8) {
    return EFI_SUCCESS;
  }

  //
  // Parse the modifier key
  //
  CurModifierMap  = CurKeyCodeBuffer[0];
  OldModifierMap  = OldKeyCodeBuffer[0];

  //
  // handle modifier key's pressing or releasing situation.
  //
  for (Index = 0; Index < 8; Index++) {

    if ((CurModifierMap & KB_Mod[Index].Mask) != (OldModifierMap & KB_Mod[Index].Mask)) {
      //
      // if current modifier key is up, then
      // CurModifierMap & KB_Mod[Index].Mask = 0;
      // otherwize it is a non-zero value.
      // Inserts the pressed modifier key into key buffer.
      //
      Down = (UINT8) (CurModifierMap & KB_Mod[Index].Mask);
      InsertKeyCode (&(UsbKeyboardDevice->KeyboardBuffer), KB_Mod[Index].Key, Down);
    }
  }

  //
  // handle normal key's releasing situation
  //
  KeyRelease = FALSE;
  for (Index = 2; Index < 8; Index++) {

    if (!USBKBD_VALID_KEYCODE (OldKeyCodeBuffer[Index])) {
      continue;
    }

    KeyRelease = TRUE;
    for (Index2 = 2; Index2 < 8; Index2++) {

      if (!USBKBD_VALID_KEYCODE (CurKeyCodeBuffer[Index2])) {
        continue;
      }

      if (OldKeyCodeBuffer[Index] == CurKeyCodeBuffer[Index2]) {
        KeyRelease = FALSE;
        break;
      }
    }

    if (KeyRelease) {
      InsertKeyCode (
        &(UsbKeyboardDevice->KeyboardBuffer),
        OldKeyCodeBuffer[Index],
        0
        );
      //
      // the original reapeat key is released.
      //
      if (OldKeyCodeBuffer[Index] == UsbKeyboardDevice->RepeatKey) {
        UsbKeyboardDevice->RepeatKey = 0;
      }
    }
  }

  //
  // original repeat key is released, cancel the repeat timer
  //
  if (UsbKeyboardDevice->RepeatKey == 0) {
    gBS->SetTimer (
          UsbKeyboardDevice->RepeatTimer,
          TimerCancel,
          USBKBD_REPEAT_RATE
          );
  }

  //
  // handle normal key's pressing situation
  //
  KeyPress = FALSE;
  for (Index = 2; Index < 8; Index++) {

    if (!USBKBD_VALID_KEYCODE (CurKeyCodeBuffer[Index])) {
      continue;
    }

    KeyPress = TRUE;
    for (Index2 = 2; Index2 < 8; Index2++) {

      if (!USBKBD_VALID_KEYCODE (OldKeyCodeBuffer[Index2])) {
        continue;
      }

      if (CurKeyCodeBuffer[Index] == OldKeyCodeBuffer[Index2]) {
        KeyPress = FALSE;
        break;
      }
    }

    if (KeyPress) {
      InsertKeyCode (&(UsbKeyboardDevice->KeyboardBuffer), CurKeyCodeBuffer[Index], 1);
      //
      // NumLock pressed or CapsLock pressed
      //
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
      KeyDescriptor = GetKeyDescriptor (UsbKeyboardDevice, CurKeyCodeBuffer[Index]);
      if (KeyDescriptor->Modifier == EFI_NUM_LOCK_MODIFIER || KeyDescriptor->Modifier == EFI_CAPS_LOCK_MODIFIER) {
#else
      if (CurKeyCodeBuffer[Index] == 0x53 || CurKeyCodeBuffer[Index] == 0x39) {
#endif
        UsbKeyboardDevice->RepeatKey = 0;
      } else {
        NewRepeatKey = CurKeyCodeBuffer[Index];
        //
        // do not repeat the original repeated key
        //
        UsbKeyboardDevice->RepeatKey = 0;
      }
    }
  }

  //
  // Update LastKeycodeArray[] buffer in the
  // Usb Keyboard Device data structure.
  //
  for (Index = 0; Index < 8; Index++) {
    UsbKeyboardDevice->LastKeyCodeArray[Index] = CurKeyCodeBuffer[Index];
  }

  //
  // pre-process KeyboardBuffer, pop out the ctrl,alt,del key in sequence
  // and judge whether it will invoke reset event.
  //
  SavedTail = UsbKeyboardDevice->KeyboardBuffer.bTail;
  Index     = UsbKeyboardDevice->KeyboardBuffer.bHead;
  while (Index != SavedTail) {
    RemoveKeyCode (&(UsbKeyboardDevice->KeyboardBuffer), &UsbKey);

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
    KeyDescriptor = GetKeyDescriptor (UsbKeyboardDevice, UsbKey.KeyCode);

    switch (KeyDescriptor->Modifier) {

    case EFI_LEFT_CONTROL_MODIFIER:
    case EFI_RIGHT_CONTROL_MODIFIER:
      if (UsbKey.Down) {
        UsbKeyboardDevice->CtrlOn = 1;
      } else {
        UsbKeyboardDevice->CtrlOn = 0;
      }
      break;

    case EFI_LEFT_ALT_MODIFIER:
    case EFI_RIGHT_ALT_MODIFIER:
      if (UsbKey.Down) {
        UsbKeyboardDevice->AltOn = 1;
      } else {
        UsbKeyboardDevice->AltOn = 0;
      }
      break;

    case EFI_ALT_GR_MODIFIER:
      if (UsbKey.Down) {
        UsbKeyboardDevice->AltGrOn = 1;
      } else {
        UsbKeyboardDevice->AltGrOn = 0;
      }
      break;

    //
    // Del Key Code
    //
    case EFI_DELETE_MODIFIER:
      if (UsbKey.Down) {
        if (UsbKeyboardDevice->CtrlOn && UsbKeyboardDevice->AltOn) {
          gRT->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, NULL);
        }
      }
      break;

    default:
      break;
    }
#else
    switch (UsbKey.KeyCode) {

    case 0xe0:
    case 0xe4:
      if (UsbKey.Down) {
        UsbKeyboardDevice->CtrlOn = 1;
      } else {
        UsbKeyboardDevice->CtrlOn = 0;
      }
      break;

    case 0xe2:
    case 0xe6:
      if (UsbKey.Down) {
        UsbKeyboardDevice->AltOn = 1;
      } else {
        UsbKeyboardDevice->AltOn = 0;
      }
      break;

    //
    // Del Key Code
    //
    case 0x4c:
    case 0x63:
      if (UsbKey.Down) {
        if (UsbKeyboardDevice->CtrlOn && UsbKeyboardDevice->AltOn) {
          gRT->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, NULL);
        }
      }
      break;

    default:
      break;
    }
#endif

    //
    // insert the key back to the buffer.
    // so the key sequence will not be destroyed.
    //
    InsertKeyCode (
      &(UsbKeyboardDevice->KeyboardBuffer),
      UsbKey.KeyCode,
      UsbKey.Down
      );
    Index = UsbKeyboardDevice->KeyboardBuffer.bHead;

  }
  //
  // If have new key pressed, update the RepeatKey value, and set the
  // timer to repeate delay timer
  //
  if (NewRepeatKey != 0) {
    //
    // sets trigger time to "Repeat Delay Time",
    // to trigger the repeat timer when the key is hold long
    // enough time.
    //
    gBS->SetTimer (
          UsbKeyboardDevice->RepeatTimer,
          TimerRelative,
          USBKBD_REPEAT_DELAY
          );
    UsbKeyboardDevice->RepeatKey = NewRepeatKey;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
USBParseKey (
  IN OUT  USB_KB_DEV  *UsbKeyboardDevice,
  OUT     UINT8       *KeyChar
  )
/*++

  Routine Description:
    Retrieves a key character after parsing the raw data in keyboard buffer.

  Arguments:
    UsbKeyboardDevice    The USB_KB_DEV instance.
    KeyChar              Points to the Key character after key parsing.

  Returns:
    EFI_SUCCESS   - Success
    EFI_NOT_READY - Device is not ready
--*/
{
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  USB_KEY             UsbKey;
  EFI_KEY_DESCRIPTOR  *KeyDescriptor;

  *KeyChar = 0;

  while (!IsUSBKeyboardBufferEmpty (UsbKeyboardDevice->KeyboardBuffer)) {
    //
    // pops one raw data off.
    //
    RemoveKeyCode (&(UsbKeyboardDevice->KeyboardBuffer), &UsbKey);

    KeyDescriptor = GetKeyDescriptor (UsbKeyboardDevice, UsbKey.KeyCode);
    if (!UsbKey.Down) {
      switch (KeyDescriptor->Modifier) {

      //
      // CTRL release
      //
      case EFI_LEFT_CONTROL_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
        UsbKeyboardDevice->LeftCtrlOn = 0;
        UsbKeyboardDevice->CtrlOn = 0;
        break;
#endif  // DISABLE_CONSOLE_EX
      case EFI_RIGHT_CONTROL_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
        UsbKeyboardDevice->RightCtrlOn = 0;
#endif  // DISABLE_CONSOLE_EX
        UsbKeyboardDevice->CtrlOn = 0;
        break;

      //
      // Shift release
      //
      case EFI_LEFT_SHIFT_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
        UsbKeyboardDevice->LeftShiftOn = 0;
        UsbKeyboardDevice->ShiftOn = 0;
        break;
#endif  // DISABLE_CONSOLE_EX
      case EFI_RIGHT_SHIFT_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
        UsbKeyboardDevice->RightShiftOn = 0;
#endif  // DISABLE_CONSOLE_EX
        UsbKeyboardDevice->ShiftOn = 0;
        break;

      //
      // Alt release
      //
      case EFI_LEFT_ALT_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
        UsbKeyboardDevice->LeftAltOn = 0;
        UsbKeyboardDevice->AltOn = 0;
        break;
#endif  // DISABLE_CONSOLE_EX
      case EFI_RIGHT_ALT_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
        UsbKeyboardDevice->RightAltOn = 0;
#endif  // DISABLE_CONSOLE_EX
        UsbKeyboardDevice->AltOn = 0;
        break;

#ifndef DISABLE_CONSOLE_EX
      //
      // Left Logo release
      //
      case EFI_LEFT_LOGO_MODIFIER:
        UsbKeyboardDevice->LeftLogoOn = 0;
        break;

      //
      // Right Logo release
      //
      case EFI_RIGHT_LOGO_MODIFIER:
        UsbKeyboardDevice->RightLogoOn = 0;
        break;

      //
      // Menu key release
      //
      case EFI_MENU_MODIFIER:
        UsbKeyboardDevice->MenuKeyOn = 0;
        break;

      //
      // SysReq release
      //
      case EFI_PRINT_MODIFIER:
      case EFI_SYS_REQUEST_MODIFIER:
        UsbKeyboardDevice->SysReqOn = 0;
        break;
#endif  // DISABLE_CONSOLE_EX

      //
      // AltGr release
      //
      case EFI_ALT_GR_MODIFIER:
        UsbKeyboardDevice->AltGrOn = 0;
        break;

      default:
        break;
      }

      continue;
    }

    //
    // Analyzes key pressing situation
    //
    switch (KeyDescriptor->Modifier) {

    //
    // CTRL press
    //
    case EFI_LEFT_CONTROL_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
      UsbKeyboardDevice->LeftCtrlOn = 1;
      UsbKeyboardDevice->CtrlOn = 1;
      continue;
      break;
#endif  // DISABLE_CONSOLE_EX
    case EFI_RIGHT_CONTROL_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
      UsbKeyboardDevice->RightCtrlOn = 1;
#endif  // DISABLE_CONSOLE_EX
      UsbKeyboardDevice->CtrlOn = 1;
      continue;
      break;

    //
    // Shift press
    //
    case EFI_LEFT_SHIFT_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
      UsbKeyboardDevice->LeftShiftOn = 1;
      UsbKeyboardDevice->ShiftOn = 1;
      continue;
      break;
#endif  // DISABLE_CONSOLE_EX
    case EFI_RIGHT_SHIFT_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
      UsbKeyboardDevice->RightShiftOn = 1;
#endif  // DISABLE_CONSOLE_EX
      UsbKeyboardDevice->ShiftOn = 1;
      continue;
      break;

    //
    // Alt press
    //
    case EFI_LEFT_ALT_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
      UsbKeyboardDevice->LeftAltOn = 1;
      UsbKeyboardDevice->AltOn = 1;
      continue;
      break;
#endif  // DISABLE_CONSOLE_EX
    case EFI_RIGHT_ALT_MODIFIER:
#ifndef DISABLE_CONSOLE_EX
      UsbKeyboardDevice->RightAltOn = 1;
#endif  // DISABLE_CONSOLE_EX
      UsbKeyboardDevice->AltOn = 1;
      continue;
      break;

#ifndef DISABLE_CONSOLE_EX
    //
    // Left Logo press
    //
    case EFI_LEFT_LOGO_MODIFIER:
      UsbKeyboardDevice->LeftLogoOn = 1;
      break;

    //
    // Right Logo press
    //
    case EFI_RIGHT_LOGO_MODIFIER:
      UsbKeyboardDevice->RightLogoOn = 1;
      break;

    //
    // Menu key press
    //
    case EFI_MENU_MODIFIER:
      UsbKeyboardDevice->MenuKeyOn = 1;
      break;

    //
    // SysReq press
    //
    case EFI_PRINT_MODIFIER:
    case EFI_SYS_REQUEST_MODIFIER:
      UsbKeyboardDevice->SysReqOn = 1;
      continue;
      break;
#endif  // DISABLE_CONSOLE_EX

    //
    // AltGr press
    //
    case EFI_ALT_GR_MODIFIER:
      UsbKeyboardDevice->AltGrOn = 1;
      break;

    case EFI_NUM_LOCK_MODIFIER:
      UsbKeyboardDevice->NumLockOn ^= 1;
      //
      // Turn on the NumLock light on KB
      //
      SetKeyLED (UsbKeyboardDevice);
      continue;
      break;

    case EFI_CAPS_LOCK_MODIFIER:
      UsbKeyboardDevice->CapsOn ^= 1;
      //
      // Turn on the CapsLock light on KB
      //
      SetKeyLED (UsbKeyboardDevice);
      continue;
      break;

    case EFI_SCROLL_LOCK_MODIFIER:
      UsbKeyboardDevice->ScrollOn ^= 1;
      //
      // Turn on the ScrollLock light on KB
      //
      SetKeyLED (UsbKeyboardDevice);
      continue;
      break;

    //
    // F11,F12,PrintScreen,Pause/Break
    // could not be retrieved via SimpleTxtInEx protocol
    //
    case EFI_FUNCTION_KEY_ELEVEN_MODIFIER:
    case EFI_FUNCTION_KEY_TWELVE_MODIFIER:
    case EFI_PAUSE_MODIFIER:
    case EFI_BREAK_MODIFIER:
      //
      // fall through
      //
      continue;
      break;

    default:
      break;
    }

    //
    // When encountered Del Key...
    //
    if (KeyDescriptor->Modifier == EFI_DELETE_MODIFIER) {
      if (UsbKeyboardDevice->CtrlOn && UsbKeyboardDevice->AltOn) {
        gRT->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, NULL);
      }
    }

    *KeyChar = UsbKey.KeyCode;
    return EFI_SUCCESS;
  }

  return EFI_NOT_READY;

#else  // EFI_SPECIFICATION_VERSION < 0x0002000A

  USB_KEY UsbKey;

  *KeyChar = 0;

  while (!IsUSBKeyboardBufferEmpty (UsbKeyboardDevice->KeyboardBuffer)) {
    //
    // pops one raw data off.
    //
    RemoveKeyCode (&(UsbKeyboardDevice->KeyboardBuffer), &UsbKey);

    if (!UsbKey.Down) {
      switch (UsbKey.KeyCode) {

      //
      // CTRL release
      //
      case 0xe0:
      case 0xe4:
        UsbKeyboardDevice->CtrlOn = 0;
        break;

      //
      // Shift release
      //
      case 0xe1:
      case 0xe5:
        UsbKeyboardDevice->ShiftOn = 0;
        break;

      //
      // Alt release
      //
      case 0xe2:
      case 0xe6:
        UsbKeyboardDevice->AltOn = 0;
        break;

      default:
        break;
      }

      continue;
    }
    
    //
    // Analyzes key pressing situation
    //
    switch (UsbKey.KeyCode) {

    //
    // CTRL press
    //
    case 0xe0:
    case 0xe4:
      UsbKeyboardDevice->CtrlOn = 1;
      continue;
      break;

    //
    // Shift press
    //
    case 0xe1:
    case 0xe5:
      UsbKeyboardDevice->ShiftOn = 1;
      continue;
      break;

    //
    // Alt press
    //
    case 0xe2:
    case 0xe6:
      UsbKeyboardDevice->AltOn = 1;
      continue;
      break;

    case 0xe3:
    case 0xe7:
      continue;
      break;

    case 0x53:
      UsbKeyboardDevice->NumLockOn ^= 1;
      //
      // Turn on the NumLock light on KB
      //
      SetKeyLED (UsbKeyboardDevice);
      continue;
      break;

    case 0x39:
      UsbKeyboardDevice->CapsOn ^= 1;
      //
      // Turn on the CapsLock light on KB
      //
      SetKeyLED (UsbKeyboardDevice);
      continue;
      break;
    
    case 0x47:
      UsbKeyboardDevice->ScrollOn ^= 1;  
      //
      // Turn on the ScrollLock light on KB
      //
      SetKeyLED (UsbKeyboardDevice);
      continue;
      break;

    //
    // PrintScreen,Pause,Application,Power
    // keys are not valid EFI key
    //

    case 0x46:
    //
    // fall through
    //
    case 0x48:
    //
    // fall through
    //
    case 0x65:
    //
    // fall through
    //
    case 0x66:
    //
    // fall through
    //
      continue;
      break;

    default:
      break;
    }
    
    //
    // When encountered Del Key...
    //
    if (UsbKey.KeyCode == 0x4c || UsbKey.KeyCode == 0x63) {
      if (UsbKeyboardDevice->CtrlOn && UsbKeyboardDevice->AltOn) {
        gRT->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, NULL);
      }
    }

    *KeyChar = UsbKey.KeyCode;
    return EFI_SUCCESS;
  }

  return EFI_NOT_READY;

#endif
}


EFI_STATUS
USBKeyCodeToEFIScanCode (
  IN  USB_KB_DEV      *UsbKeyboardDevice,
  IN  UINT8           KeyChar,
  OUT EFI_INPUT_KEY   *Key
  )
/*++

  Routine Description:
    Converts USB Keyboard code to EFI Scan Code.

  Arguments:
    UsbKeyboardDevice    The USB_KB_DEV instance.
    KeyChar              Indicates the key code that will be interpreted.
    Key                  A pointer to a buffer that is filled in with
                         the keystroke information for the key that
                         was pressed.
  Returns:
    EFI_NOT_READY - Device is not ready
    EFI_SUCCESS   - Success
--*/
{
  UINT8               Index;
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  EFI_KEY_DESCRIPTOR  *KeyDescriptor;
#endif

  if (!USBKBD_VALID_KEYCODE (KeyChar)) {
    return EFI_NOT_READY;
  }

  //
  // valid USB Key Code starts from 4
  //
  Index = (UINT8) (KeyChar - 4);

  if (Index >= USB_KEYCODE_MAX_MAKE) {
    return EFI_NOT_READY;
  }

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  KeyDescriptor = GetKeyDescriptor (UsbKeyboardDevice, KeyChar);

  //
  // Check for Non-spacing key
  //
  if (KeyDescriptor->Modifier == EFI_NS_KEY_MODIFIER) {
    UsbKeyboardDevice->CurrentNsKey = FindUsbNsKey (UsbKeyboardDevice, KeyDescriptor);
    return EFI_NOT_READY;
  }

  //
  // Check whether this keystroke follows a Non-spacing key
  //
  if (UsbKeyboardDevice->CurrentNsKey != NULL) {
    KeyDescriptor = FindPhysicalKey (UsbKeyboardDevice->CurrentNsKey, KeyDescriptor);
    UsbKeyboardDevice->CurrentNsKey = NULL;
  }

  Key->ScanCode = EfiScanCodeConvertionTable[KeyDescriptor->Modifier];
  Key->UnicodeChar = KeyDescriptor->Unicode;

  if (KeyDescriptor->AffectedAttribute & EFI_AFFECTED_BY_STANDARD_SHIFT) {
    if (UsbKeyboardDevice->ShiftOn) {
      Key->UnicodeChar = KeyDescriptor->ShiftedUnicode;

      //
      // Need not return associated shift state if a class of printable characters that
      // are normally adjusted by shift modifiers. e.g. Shift Key + 'f' key = 'F'
      //
      if (KeyDescriptor->AffectedAttribute & EFI_AFFECTED_BY_CAPS_LOCK) {
        UsbKeyboardDevice->LeftShiftOn = 0;
        UsbKeyboardDevice->RightShiftOn = 0;
      }

      if (UsbKeyboardDevice->AltGrOn) {
        Key->UnicodeChar = KeyDescriptor->ShiftedAltGrUnicode;
      }
    } else {
      //
      // Shift off
      //
      Key->UnicodeChar = KeyDescriptor->Unicode;

      if (UsbKeyboardDevice->AltGrOn) {
        Key->UnicodeChar = KeyDescriptor->AltGrUnicode;
      }
    }
  }

  if (KeyDescriptor->AffectedAttribute & EFI_AFFECTED_BY_CAPS_LOCK) {
    if (UsbKeyboardDevice->CapsOn) {

      if (Key->UnicodeChar == KeyDescriptor->Unicode) {

        Key->UnicodeChar = KeyDescriptor->ShiftedUnicode;

      } else if (Key->UnicodeChar == KeyDescriptor->ShiftedUnicode) {

        Key->UnicodeChar = KeyDescriptor->Unicode;

      }
    }
  }

  //
  // Translate the CTRL-Alpha characters to their corresponding control value  (ctrl-a = 0x0001 through ctrl-Z = 0x001A)
  //
  if (UsbKeyboardDevice->CtrlOn) {
    if (Key->UnicodeChar >= 'a' && Key->UnicodeChar <= 'z') {
      Key->UnicodeChar = Key->UnicodeChar - 'a' + 1;
    } else if (Key->UnicodeChar >= 'A' && Key->UnicodeChar <= 'Z') {
      Key->UnicodeChar = Key->UnicodeChar - 'A' + 1;
    }
  }

  if (KeyDescriptor->AffectedAttribute & EFI_AFFECTED_BY_NUM_LOCK) {

    if (UsbKeyboardDevice->NumLockOn && !UsbKeyboardDevice->ShiftOn) {

      Key->ScanCode = SCAN_NULL;

    } else {
      Key->UnicodeChar = 0x00;
    }
  }

  //
  // Translate Unicode 0x1B (ESC) to EFI Scan Code
  //
  if (Key->UnicodeChar == 0x1B && Key->ScanCode == SCAN_NULL) {
    Key->ScanCode = SCAN_ESC;
    Key->UnicodeChar = 0x00;
  }

  if (Key->UnicodeChar == 0 && Key->ScanCode == SCAN_NULL) {
    return EFI_NOT_READY;
  }


#ifndef DISABLE_CONSOLE_EX
  //
  // Save Shift/Toggle state
  //
  if (UsbKeyboardDevice->LeftCtrlOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_LEFT_CONTROL_PRESSED;
  }
  if (UsbKeyboardDevice->RightCtrlOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_RIGHT_CONTROL_PRESSED;
  }
  if (UsbKeyboardDevice->LeftAltOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_LEFT_ALT_PRESSED;
  }
  if (UsbKeyboardDevice->RightAltOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_RIGHT_ALT_PRESSED;
  }
  if (UsbKeyboardDevice->LeftShiftOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_LEFT_SHIFT_PRESSED;
  }
  if (UsbKeyboardDevice->RightShiftOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_RIGHT_SHIFT_PRESSED;
  }
  if (UsbKeyboardDevice->LeftLogoOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_LEFT_LOGO_PRESSED;
  }
  if (UsbKeyboardDevice->RightLogoOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_RIGHT_LOGO_PRESSED;
  }
  if (UsbKeyboardDevice->MenuKeyOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_MENU_KEY_PRESSED;
  }
  if (UsbKeyboardDevice->SysReqOn == 1) {
    UsbKeyboardDevice->KeyState.KeyShiftState |= EFI_SYS_REQ_PRESSED;
  }

  if (UsbKeyboardDevice->ScrollOn == 1) {
    UsbKeyboardDevice->KeyState.KeyToggleState |= EFI_SCROLL_LOCK_ACTIVE;
  }
  if (UsbKeyboardDevice->NumLockOn == 1) {
    UsbKeyboardDevice->KeyState.KeyToggleState |= EFI_NUM_LOCK_ACTIVE;
  }
  if (UsbKeyboardDevice->CapsOn == 1) {
    UsbKeyboardDevice->KeyState.KeyToggleState |= EFI_CAPS_LOCK_ACTIVE;
  }
#endif  // DISABLE_CONSOLE_EX

#else   // EFI_SPECIFICATION_VERSION < 0x0002000A

  Key->ScanCode = KeyConvertionTable[Index][0];

  if (UsbKeyboardDevice->ShiftOn) {

    Key->UnicodeChar = KeyConvertionTable[Index][2];

  } else {

    Key->UnicodeChar = KeyConvertionTable[Index][1];
  }

  if (UsbKeyboardDevice->CapsOn) {

    if (Key->UnicodeChar >= 'a' && Key->UnicodeChar <= 'z') {

      Key->UnicodeChar = KeyConvertionTable[Index][2];

    } else if (Key->UnicodeChar >= 'A' && Key->UnicodeChar <= 'Z') {

      Key->UnicodeChar = KeyConvertionTable[Index][1];

    }
  }
  //
  // Translate the CTRL-Alpha characters to their corresponding control value  (ctrl-a = 0x0001 through ctrl-Z = 0x001A)
  //
  if (UsbKeyboardDevice->CtrlOn) {
    if (Key->UnicodeChar >= 'a' && Key->UnicodeChar <= 'z') {
      Key->UnicodeChar = Key->UnicodeChar - 'a' + 1;
    } else if (Key->UnicodeChar >= 'A' && Key->UnicodeChar <= 'Z') {
      Key->UnicodeChar = Key->UnicodeChar - 'A' + 1;
    }
  }
  
  
  if (KeyChar >= 0x59 && KeyChar <= 0x63) {

    if (UsbKeyboardDevice->NumLockOn && !UsbKeyboardDevice->ShiftOn) {

      Key->ScanCode = SCAN_NULL;

    } else {

      Key->UnicodeChar = 0x00;
    }
  }

  if (Key->UnicodeChar == 0 && Key->ScanCode == SCAN_NULL) {
    return EFI_NOT_READY;
  }
#endif

  return EFI_SUCCESS;

}


EFI_STATUS
InitUSBKeyBuffer (
  IN OUT  USB_KB_BUFFER   *KeyboardBuffer
  )
/*++

  Routine Description:
    Resets USB Keyboard Buffer.

  Arguments:
    KeyboardBuffer - Points to the USB Keyboard Buffer.

  Returns:
    EFI_SUCCESS - Success
--*/
{
  EfiZeroMem (KeyboardBuffer, sizeof (USB_KB_BUFFER));

  KeyboardBuffer->bHead = KeyboardBuffer->bTail;

  return EFI_SUCCESS;
}

BOOLEAN
IsUSBKeyboardBufferEmpty (
  IN  USB_KB_BUFFER   KeyboardBuffer
  )
/*++

  Routine Description:
    Check whether USB Keyboard buffer is empty.

  Arguments:
    KeyboardBuffer - USB Keyboard Buffer.

  Returns:

--*/
{
  //
  // meet FIFO empty condition
  //
  return (BOOLEAN) (KeyboardBuffer.bHead == KeyboardBuffer.bTail);
}


BOOLEAN
IsUSBKeyboardBufferFull (
  IN  USB_KB_BUFFER   KeyboardBuffer
  )
/*++

  Routine Description:
    Check whether USB Keyboard buffer is full.

  Arguments:
    KeyboardBuffer - USB Keyboard Buffer.

  Returns:

--*/
{
  return (BOOLEAN)(((KeyboardBuffer.bTail + 1) % (MAX_KEY_ALLOWED + 1)) ==
                                                        KeyboardBuffer.bHead);
}


EFI_STATUS
InsertKeyCode (
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  IN      UINT8         Key,
  IN      UINT8         Down
  )
/*++

  Routine Description:
    Inserts a key code into keyboard buffer.

  Arguments:
    KeyboardBuffer - Points to the USB Keyboard Buffer.
    Key            - Key code
    Down           - Special key
  Returns:
    EFI_SUCCESS - Success
--*/
{
  USB_KEY UsbKey;

  //
  // if keyboard buffer is full, throw the
  // first key out of the keyboard buffer.
  //
  if (IsUSBKeyboardBufferFull (*KeyboardBuffer)) {
    RemoveKeyCode (KeyboardBuffer, &UsbKey);
  }

  KeyboardBuffer->buffer[KeyboardBuffer->bTail].KeyCode = Key;
  KeyboardBuffer->buffer[KeyboardBuffer->bTail].Down    = Down;

  //
  // adjust the tail pointer of the FIFO keyboard buffer.
  //
  KeyboardBuffer->bTail = (UINT8) ((KeyboardBuffer->bTail + 1) % (MAX_KEY_ALLOWED + 1));

  return EFI_SUCCESS;
}

EFI_STATUS
RemoveKeyCode (
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  OUT     USB_KEY       *UsbKey
  )
/*++

  Routine Description:
    Pops a key code off from keyboard buffer.

  Arguments:
    KeyboardBuffer -  Points to the USB Keyboard Buffer.
    UsbKey         -  Points to the buffer that contains a usb key code.

  Returns:
    EFI_SUCCESS      - Success
    EFI_DEVICE_ERROR - Hardware Error
--*/
{
  if (IsUSBKeyboardBufferEmpty (*KeyboardBuffer)) {
    return EFI_DEVICE_ERROR;
  }

  UsbKey->KeyCode = KeyboardBuffer->buffer[KeyboardBuffer->bHead].KeyCode;
  UsbKey->Down    = KeyboardBuffer->buffer[KeyboardBuffer->bHead].Down;

  //
  // adjust the head pointer of the FIFO keyboard buffer.
  //
  KeyboardBuffer->bHead = (UINT8) ((KeyboardBuffer->bHead + 1) % (MAX_KEY_ALLOWED + 1));

  return EFI_SUCCESS;
}

EFI_STATUS
SetKeyLED (
  IN  USB_KB_DEV    *UsbKeyboardDevice
  )
/*++

  Routine Description:
    Sets USB Keyboard LED state.

  Arguments:
    UsbKeyboardDevice - The USB_KB_DEV instance.

  Returns:
    EFI_SUCCESS - Success
--*/
{
  LED_MAP Led;
  UINT8   ReportId;

  //
  // Set each field in Led map.
  //
  Led.NumLock    = (UINT8) UsbKeyboardDevice->NumLockOn;
  Led.CapsLock   = (UINT8) UsbKeyboardDevice->CapsOn;
  Led.ScrollLock = (UINT8) UsbKeyboardDevice->ScrollOn;
  Led.Resrvd     = 0;

  ReportId       = 0;
  //
  // call Set Report Request to lighten the LED.
  //
  UsbSetReportRequest (
    UsbKeyboardDevice->UsbIo,
    UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
    ReportId,
    HID_OUTPUT_REPORT,
    1,
    (CHAR8 *) &Led
    );

  return EFI_SUCCESS;
}

VOID
EFIAPI
USBKeyboardRepeatHandler (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
/*++

  Routine Description:
    Timer handler for Repeat Key timer.

  Arguments:
    Event   - The Repeat Key event.
    Context - Points to the USB_KB_DEV instance.

  Returns:

--*/
{
  USB_KB_DEV  *UsbKeyboardDevice;

  UsbKeyboardDevice = (USB_KB_DEV *) Context;

  //
  // Do nothing when there is no repeat key.
  //
  if (UsbKeyboardDevice->RepeatKey != 0) {
    //
    // Inserts one Repeat key into keyboard buffer,
    //
    InsertKeyCode (
      &(UsbKeyboardDevice->KeyboardBuffer),
      UsbKeyboardDevice->RepeatKey,
      1
      );

    //
    // set repeate rate for repeat key generation.
    //
    gBS->SetTimer (
          UsbKeyboardDevice->RepeatTimer,
          TimerRelative,
          USBKBD_REPEAT_RATE
          );

  }
}

VOID
EFIAPI
USBKeyboardRecoveryHandler (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
/*++

  Routine Description:
    Timer handler for Delayed Recovery timer.

  Arguments:
    Event   -  The Delayed Recovery event.
    Context -  Points to the USB_KB_DEV instance.

  Returns:

--*/
{

  USB_KB_DEV          *UsbKeyboardDevice;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               PacketSize;

  UsbKeyboardDevice = (USB_KB_DEV *) Context;

  UsbIo             = UsbKeyboardDevice->UsbIo;

  PacketSize        = (UINT8) (UsbKeyboardDevice->IntEndpointDescriptor.MaxPacketSize);

  UsbIo->UsbAsyncInterruptTransfer (
          UsbIo,
          UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
          TRUE,
          UsbKeyboardDevice->IntEndpointDescriptor.Interval,
          PacketSize,
          KeyboardHandler,
          UsbKeyboardDevice
          );
}
