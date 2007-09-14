/*++

Copyright (c) 2007, Intel Corporation                                                     
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Hotkey.h

Abstract:

  Provides a way for 3rd party applications to register themselves for launch by the
  Boot Manager based on hot key

Revision History

--*/

#ifndef _HOTKEY_H
#define _HOTKEY_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "BdsPlatform.h"
#include "BdsLib.h"

#include EFI_PROTOCOL_DEFINITION (SimpleTextInputEx)


typedef struct _EFI_KEY_OPTION {
  UINT32         KeyOptions;
  UINT32         BootOptionCrc;
  UINT16         BootOption;
//EFI_INPUT_KEY  Keys[];
} EFI_KEY_OPTION;

#define EFI_KEY_OPTION_SHIFT     0x00000001
#define EFI_KEY_OPTION_CONTROL   0x00000002
#define EFI_KEY_OPTION_ALT       0x00000004
#define EFI_KEY_OPTION_LOGO      0x00000008
#define EFI_KEY_OPTION_MENU      0x00000010
#define EFI_KEY_OPTION_SYSREQ    0x00000020
#define EFI_KEY_CODE_COUNT       0x00000300

#define GET_KEY_CODE_COUNT(KeyOptions)      (((KeyOptions) & EFI_KEY_CODE_COUNT) >> 8)

#define BDS_HOTKEY_OPTION_SIGNATURE EFI_SIGNATURE_32 ('B', 'd', 'K', 'O')
typedef struct {
  UINTN                     Signature;
  EFI_LIST_ENTRY            Link;

  EFI_HANDLE                NotifyHandle;
  UINT16                    BootOptionNumber;
  UINT8                     CodeCount;
  UINT8                     WaitingKey;
  EFI_KEY_DATA              KeyData[3];
} BDS_HOTKEY_OPTION;

#define BDS_HOTKEY_OPTION_FROM_LINK(a) CR (a, BDS_HOTKEY_OPTION, Link, BDS_HOTKEY_OPTION_SIGNATURE)

#define VarKeyOrder       L"KeyOrder"

EFI_STATUS
RegisterHotkey (
  IN EFI_KEY_OPTION     *KeyOption,
  OUT UINT16            *KeyOptionNumber
)
/*++

Routine Description:

  Create Key#### for the given hotkey.

Arguments:

  KeyOption             - The Hot Key Option to be added.
  KeyOptionNumber       - The key option number for Key#### (optional).

Returns:

  EFI_SUCCESS           - Register hotkey successfully.
  EFI_INVALID_PARAMETER - The hotkey option is invalid.

--*/
;

EFI_STATUS
UnregisterHotkey (
  IN UINT16             KeyOptionNumber
)
/*++

Routine Description:

  Delete Key#### for the given Key Option number.

Arguments:

  KeyOptionNumber       - Key option number for Key####

Returns:

  EFI_SUCCESS           - Unregister hotkey successfully.
  EFI_NOT_FOUND         - No Key#### is found for the given Key Option number.

--*/
;


EFI_STATUS
InitializeHotkeyService (
  VOID
  )
/*++

Routine Description:

  Process all the "Key####" variables, associate Hotkeys with corresponding Boot Options.

Arguments:

  None

Returns:

  EFI_SUCCESS   - Hotkey services successfully initialized.

--*/
;

#endif
