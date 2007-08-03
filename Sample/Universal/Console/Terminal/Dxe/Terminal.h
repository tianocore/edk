/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  terminal.h

Abstract:

  
Revision History

--*/

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiStatusCode.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (SerialIo)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (ComponentName2)
#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#include EFI_PROTOCOL_DEFINITION (SimpleTextInputEx)
#endif
#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)
#include EFI_GUID_DEFINITION (HotPlugDevice)

#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)

//
//  Driver Consumed Guid
//
#include EFI_GUID_DEFINITION (PcAnsi)
#include EFI_GUID_DEFINITION (GlobalVariable)

#define RAW_FIFO_MAX_NUMBER 256
#define FIFO_MAX_NUMBER     128

typedef struct {
  UINT8 Head;
  UINT8 Tail;
  UINT8 Data[RAW_FIFO_MAX_NUMBER + 1];
} RAW_DATA_FIFO;

typedef struct {
  UINT8   Head;
  UINT8   Tail;
  UINT16  Data[FIFO_MAX_NUMBER + 1];
} UNICODE_FIFO;

typedef struct {
  UINT8         Head;
  UINT8         Tail;
  EFI_INPUT_KEY Data[FIFO_MAX_NUMBER + 1];
} EFI_KEY_FIFO;

#define TERMINAL_DEV_SIGNATURE  EFI_SIGNATURE_32 ('t', 'm', 'n', 'l')

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#define SIMPLE_TEXTIN_EX_NOTIFY_GUID \
  { \
    0x856f2def, 0x4e93, 0x4d6b, 0x94, 0xce, 0x1c, 0xfe, 0x47, 0x1, 0x3e, 0xa5 \
  }

#define TERMINAL_CONSOLE_IN_EX_NOTIFY_SIGNATURE EFI_SIGNATURE_32 ('t', 'm', 'e', 'n')

typedef struct _TERMINAL_CONSOLE_IN_EX_NOTIFY {
  UINTN                                 Signature;
  EFI_HANDLE                            NotifyHandle;
  EFI_KEY_DATA                          KeyData;
  EFI_KEY_NOTIFY_FUNCTION               KeyNotificationFn;
  EFI_LIST_ENTRY                        NotifyEntry;
} TERMINAL_CONSOLE_IN_EX_NOTIFY;

#endif

typedef struct {
  UINTN                         Signature;
  EFI_HANDLE                    Handle;
  UINT8                         TerminalType;
  EFI_SERIAL_IO_PROTOCOL        *SerialIo;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  VENDOR_DEVICE_PATH            Node;
  EFI_SIMPLE_TEXT_IN_PROTOCOL   SimpleInput;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  SimpleTextOutput;
  EFI_SIMPLE_TEXT_OUTPUT_MODE   SimpleTextOutputMode;
  UINTN                         SerialInTimeOut;
  RAW_DATA_FIFO                 RawFiFo;
  UNICODE_FIFO                  UnicodeFiFo;
  EFI_KEY_FIFO                  EfiKeyFiFo;
  EFI_UNICODE_STRING_TABLE      *ControllerNameTable;
  EFI_EVENT                     TwoSecondTimeOut;
  UINT32                        InputState;
  UINT32                        ResetState;

  //
  // Esc could not be output to the screen by user,
  // but the terminal driver need to output it to
  // the terminal emulation software to send control sequence.
  // This boolean is used by the terminal driver only
  // to indicate whether the Esc could be sent or not.
  //
  BOOLEAN                       OutputEscChar;

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL SimpleInputEx;
  EFI_LIST_ENTRY                    NotifyList;
#endif
  
} TERMINAL_DEV;

#define INPUT_STATE_DEFAULT               0x00
#define INPUT_STATE_ESC                   0x01
#define INPUT_STATE_CSI                   0x02
#define INPUT_STATE_LEFTOPENBRACKET       0x04
#define INPUT_STATE_O                     0x08
#define INPUT_STATE_2                     0x10

#define RESET_STATE_DEFAULT               0x00
#define RESET_STATE_ESC_R                 0x01
#define RESET_STATE_ESC_R_ESC_r           0x02

#define TERMINAL_CON_IN_DEV_FROM_THIS(a)  CR (a, TERMINAL_DEV, SimpleInput, TERMINAL_DEV_SIGNATURE)
#define TERMINAL_CON_OUT_DEV_FROM_THIS(a) CR (a, TERMINAL_DEV, SimpleTextOutput, TERMINAL_DEV_SIGNATURE)
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#define TERMINAL_CON_IN_EX_DEV_FROM_THIS(a)  CR (a, TERMINAL_DEV, SimpleInputEx, TERMINAL_DEV_SIGNATURE)
#endif

typedef union {
  UINT8 Utf8_1;
  UINT8 Utf8_2[2];
  UINT8 Utf8_3[3];
} UTF8_CHAR;

#define PcAnsiType                0
#define VT100Type                 1
#define VT100PlusType             2
#define VTUTF8Type                3

#define LEFTOPENBRACKET           0x5b  // '['
#define ACAP                      0x41
#define BCAP                      0x42
#define CCAP                      0x43
#define DCAP                      0x44

#define MODE0_COLUMN_COUNT        80
#define MODE0_ROW_COUNT           25

#define MODE1_COLUMN_COUNT        100
#define MODE1_ROW_COUNT           31

#define BACKSPACE                 8
#define ESC                       27
#define CSI                       0x9B
#define DEL                       127
#define BRIGHT_CONTROL_OFFSET     2
#define FOREGROUND_CONTROL_OFFSET 6
#define BACKGROUND_CONTROL_OFFSET 11
#define ROW_OFFSET                2
#define COLUMN_OFFSET             5

typedef struct {
  CHAR16  Unicode;
  CHAR8   PcAnsi;
  CHAR8   Ascii;
} UNICODE_TO_CHAR;

#define VarConsoleInpDev  L"ConInDev"
#define VarConsoleOutDev  L"ConOutDev"
#define VarErrorOutDev    L"ErrOutDev"

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gTerminalDriverBinding;
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
extern EFI_COMPONENT_NAME2_PROTOCOL  gTerminalComponentName;
#else
extern EFI_COMPONENT_NAME_PROTOCOL   gTerminalComponentName;
#endif

//
// Prototypes
//
EFI_STATUS
EFIAPI
InitializeTerminal (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
;

EFI_STATUS
EFIAPI
TerminalConInReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL    *This,
  IN  BOOLEAN                        ExtendedVerification
  )
;

EFI_STATUS
EFIAPI
TerminalConInReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                *Key
  )
;

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)

VOID
EFIAPI
TerminalConInWaitForKeyEx (
  IN  EFI_EVENT       Event,
  IN  VOID            *Context
  )
;  

//
// Simple Text Input Ex protocol prototypes
//

EFI_STATUS
EFIAPI
TerminalConInResetEx (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN BOOLEAN                            ExtendedVerification
  )
/*++

  Routine Description:
    Reset the input device and optionaly run diagnostics

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCESS           - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning properly and could 
                            not be reset.

--*/
;

EFI_STATUS
EFIAPI
TerminalConInReadKeyStrokeEx (
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  OUT EFI_KEY_DATA                      *KeyData
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can 
    be used to test for existance of a keystroke via WaitForEvent () call.

  Arguments:
    This       - Protocol instance pointer.
    KeyData    - A pointer to a buffer that is filled in with the keystroke 
                 state data for the key that was pressed.

  Returns:
    EFI_SUCCESS           - The keystroke information was returned.
    EFI_NOT_READY         - There was no keystroke data availiable.
    EFI_DEVICE_ERROR      - The keystroke information was not returned due to 
                            hardware errors.
    EFI_INVALID_PARAMETER - KeyData is NULL.                        

--*/
;

EFI_STATUS
EFIAPI
TerminalConInSetState (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_TOGGLE_STATE               *KeyToggleState
  )
/*++

  Routine Description:
    Set certain state for the input device.

  Arguments:
    This                  - Protocol instance pointer.
    KeyToggleState        - A pointer to the EFI_KEY_TOGGLE_STATE to set the 
                            state for the input device.
                          
  Returns:                
    EFI_SUCCESS           - The device state was set successfully.
    EFI_DEVICE_ERROR      - The device is not functioning correctly and could 
                            not have the setting adjusted.
    EFI_UNSUPPORTED       - The device does not have the ability to set its state.
    EFI_INVALID_PARAMETER - KeyToggleState is NULL.                       

--*/   
;

EFI_STATUS
EFIAPI
TerminalConInRegisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_DATA                       *KeyData,
  IN EFI_KEY_NOTIFY_FUNCTION            KeyNotificationFunction,
  OUT EFI_HANDLE                        *NotifyHandle
  )
/*++

  Routine Description:
    Register a notification function for a particular keystroke for the input device.

  Arguments:
    This                    - Protocol instance pointer.
    KeyData                 - A pointer to a buffer that is filled in with the keystroke 
                              information data for the key that was pressed.
    KeyNotificationFunction - Points to the function to be called when the key 
                              sequence is typed specified by KeyData.                        
    NotifyHandle            - Points to the unique handle assigned to the registered notification.                          

  Returns:
    EFI_SUCCESS             - The notification function was registered successfully.
    EFI_OUT_OF_RESOURCES    - Unable to allocate resources for necesssary data structures.
    EFI_INVALID_PARAMETER   - KeyData or NotifyHandle is NULL.                       
                              
--*/   
;

EFI_STATUS
EFIAPI
TerminalConInUnregisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_HANDLE                         NotificationHandle
  )
/*++

  Routine Description:
    Remove a registered notification function from a particular keystroke.

  Arguments:
    This                    - Protocol instance pointer.    
    NotificationHandle      - The handle of the notification function being unregistered.

  Returns:
    EFI_SUCCESS             - The notification function was unregistered successfully.
    EFI_INVALID_PARAMETER   - The NotificationHandle is invalid.
    EFI_NOT_FOUND           - Can not find the matching entry in database.  
                              
--*/   
;

extern EFI_GUID gSimpleTextInExNotifyGuid;

#endif

VOID
EFIAPI
TerminalConInWaitForKey (
  IN  EFI_EVENT     Event,
  IN  VOID          *Context
  )
;

EFI_STATUS
EFIAPI
TerminalConOutReset (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL    *This,
  IN  BOOLEAN                         ExtendedVerification
  )
;

EFI_STATUS
EFIAPI
TerminalConOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        *WString
  )
;

EFI_STATUS
EFIAPI
TerminalConOutTestString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  CHAR16                        *WString
  )
;

EFI_STATUS
EFIAPI
TerminalConOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         ModeNumber,
  OUT UINTN                         *Columns,
  OUT UINTN                         *Rows
  )
;

EFI_STATUS
EFIAPI
TerminalConOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         ModeNumber
  )
;

EFI_STATUS
EFIAPI
TerminalConOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         Attribute
  )
;

EFI_STATUS
EFIAPI
TerminalConOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This
  )
;

EFI_STATUS
EFIAPI
TerminalConOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  UINTN                         Column,
  IN  UINTN                         Row
  )
;

EFI_STATUS
EFIAPI
TerminalConOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *This,
  IN  BOOLEAN                       Visible
  )
;

//
// internal functions
//
EFI_STATUS
TerminalConInCheckForKey (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This
  )
;

VOID
TerminalUpdateConsoleDevVariable (
  IN CHAR16                    *VariableName,
  IN EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath
  )
;

VOID
TerminalRemoveConsoleDevVariable (
  IN CHAR16                    *VariableName,
  IN EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath
  )
;

VOID                                *
TerminalGetVariableAndSize (
  IN  CHAR16              *Name,
  IN  EFI_GUID            *VendorGuid,
  OUT UINTN               *VariableSize
  )
;

EFI_STATUS
SetTerminalDevicePath (
  IN  UINT8                       TerminalType,
  IN  EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath,
  OUT EFI_DEVICE_PATH_PROTOCOL    **TerminalDevicePath
  )
;

VOID
InitializeRawFiFo (
  IN  TERMINAL_DEV  *TerminalDevice
  )
;

VOID
InitializeUnicodeFiFo (
  IN  TERMINAL_DEV  *TerminalDevice
  )
;

VOID
InitializeEfiKeyFiFo (
  IN  TERMINAL_DEV  *TerminalDevice
  )
;

EFI_STATUS
GetOneKeyFromSerial (
  EFI_SERIAL_IO_PROTOCOL  *SerialIo,
  UINT8                   *Input
  )
;

BOOLEAN
RawFiFoInsertOneKey (
  TERMINAL_DEV  *TerminalDevice,
  UINT8         Input
  )
;

BOOLEAN
RawFiFoRemoveOneKey (
  TERMINAL_DEV  *TerminalDevice,
  UINT8         *Output
  )
;

BOOLEAN
IsRawFiFoEmpty (
  TERMINAL_DEV  *TerminalDevice
  )
;

BOOLEAN
IsRawFiFoFull (
  TERMINAL_DEV  *TerminalDevice
  )
;

BOOLEAN
EfiKeyFiFoInsertOneKey (
  TERMINAL_DEV      *TerminalDevice,
  EFI_INPUT_KEY     Key
  )
;

BOOLEAN
EfiKeyFiFoRemoveOneKey (
  TERMINAL_DEV  *TerminalDevice,
  EFI_INPUT_KEY *Output
  )
;

BOOLEAN
IsEfiKeyFiFoEmpty (
  TERMINAL_DEV  *TerminalDevice
  )
;

BOOLEAN
IsEfiKeyFiFoFull (
  TERMINAL_DEV  *TerminalDevice
  )
;

BOOLEAN
UnicodeFiFoInsertOneKey (
  TERMINAL_DEV      *TerminalDevice,
  UINT16            Input
  )
;

BOOLEAN
UnicodeFiFoRemoveOneKey (
  TERMINAL_DEV  *TerminalDevice,
  UINT16        *Output
  )
;

BOOLEAN
IsUnicodeFiFoEmpty (
  TERMINAL_DEV  *TerminalDevice
  )
;

BOOLEAN
IsUnicodeFiFoFull (
  TERMINAL_DEV  *TerminalDevice
  )
;

UINT8
UnicodeFiFoGetKeyCount (
  TERMINAL_DEV    *TerminalDevice
  )
;

VOID
TranslateRawDataToEfiKey (
  IN  TERMINAL_DEV      *TerminalDevice
  )
;

//
// internal functions for PC ANSI
//
VOID
AnsiRawDataToUnicode (
  IN  TERMINAL_DEV    *PcAnsiDevice
  )
;

VOID
UnicodeToEfiKey (
  IN  TERMINAL_DEV    *PcAnsiDevice
  )
;

EFI_STATUS
AnsiTestString (
  IN  TERMINAL_DEV    *TerminalDevice,
  IN  CHAR16          *WString
  )
;

//
// internal functions for VT100
//
EFI_STATUS
VT100TestString (
  IN  TERMINAL_DEV    *VT100Device,
  IN  CHAR16          *WString
  )
;

//
// internal functions for VT100Plus
//
EFI_STATUS
VT100PlusTestString (
  IN  TERMINAL_DEV    *TerminalDevice,
  IN  CHAR16          *WString
  )
;

//
// internal functions for VTUTF8
//
VOID
VTUTF8RawDataToUnicode (
  IN  TERMINAL_DEV    *VtUtf8Device
  )
;

EFI_STATUS
VTUTF8TestString (
  IN  TERMINAL_DEV    *TerminalDevice,
  IN  CHAR16          *WString
  )
;

VOID
UnicodeToUtf8 (
  IN  CHAR16      Unicode,
  OUT UTF8_CHAR   *Utf8Char,
  OUT UINT8       *ValidBytes
  )
;

VOID
GetOneValidUtf8Char (
  IN  TERMINAL_DEV      *Utf8Device,
  OUT UTF8_CHAR         *Utf8Char,
  OUT UINT8             *ValidBytes
  )
;

VOID
Utf8ToUnicode (
  IN  UTF8_CHAR       Utf8Char,
  IN  UINT8           ValidBytes,
  OUT CHAR16          *UnicodeChar
  )
;

//
// functions for boxdraw unicode
//
BOOLEAN
TerminalIsValidTextGraphics (
  IN  CHAR16  Graphic,
  OUT CHAR8   *PcAnsi, OPTIONAL
  OUT CHAR8   *Ascii OPTIONAL
  )
;

BOOLEAN
TerminalIsValidAscii (
  IN  CHAR16  Ascii
  )
;

BOOLEAN
TerminalIsValidEfiCntlChar (
  IN  CHAR16  CharC
  )
;

#endif
