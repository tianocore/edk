/*++
Copyright (c) 2004 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    EfiKey.h

Abstract:

    Header file for USB Keyboard Driver's Data Structures

Revision History
--*/
#ifndef _USB_KB_H
#define _USB_KB_H

#include "Tiano.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (UsbIo)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (ComponentName2)
#include EFI_GUID_DEFINITION (HotPlugDevice)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)
#include EFI_PROTOCOL_DEFINITION (SimpleTextInputEx)

#include "usb.h"
#include "UsbDxeLib.h"

#define MAX_KEY_ALLOWED     32

#define HZ                  1000 * 1000 * 10
#define USBKBD_REPEAT_DELAY ((HZ) / 2)
#define USBKBD_REPEAT_RATE  ((HZ) / 50)

#define CLASS_HID           3
#define SUBCLASS_BOOT       1
#define PROTOCOL_KEYBOARD   1

#define BOOT_PROTOCOL       0
#define REPORT_PROTOCOL     1

typedef struct {
  UINT8 Down;
  UINT8 KeyCode;
} USB_KEY;

typedef struct {
  USB_KEY buffer[MAX_KEY_ALLOWED + 1];
  UINT8   bHead;
  UINT8   bTail;
} USB_KB_BUFFER;

#define USB_KB_DEV_SIGNATURE  EFI_SIGNATURE_32 ('u', 'k', 'b', 'd')

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#define USB_KB_CONSOLE_IN_EX_NOTIFY_SIGNATURE EFI_SIGNATURE_32 ('u', 'k', 'b', 'x')

typedef struct _KEYBOARD_CONSOLE_IN_EX_NOTIFY {
  UINTN                                 Signature;
  EFI_HANDLE                            NotifyHandle;
  EFI_KEY_DATA                          KeyData;
  EFI_KEY_NOTIFY_FUNCTION               KeyNotificationFn;
  EFI_LIST_ENTRY                        NotifyEntry;
} KEYBOARD_CONSOLE_IN_EX_NOTIFY;

#define USB_NS_KEY_SIGNATURE  EFI_SIGNATURE_32 ('u', 'n', 's', 'k')

typedef struct {
  UINTN                         Signature;
  EFI_LIST_ENTRY                Link;

  //
  // The number of EFI_NS_KEY_MODIFIER children definitions
  //
  UINTN                         KeyCount;

  //
  // NsKey[0] : Non-spacing key
  // NsKey[1] ~ NsKey[KeyCount] : Physical keys
  //
  EFI_KEY_DESCRIPTOR            *NsKey;
} USB_NS_KEY;

#define USB_NS_KEY_FORM_FROM_LINK(a)  CR (a, USB_NS_KEY, Link, USB_NS_KEY_SIGNATURE)

#endif
typedef struct {
  UINTN                         Signature;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  EFI_EVENT                     DelayedRecoveryEvent;
  EFI_SIMPLE_TEXT_IN_PROTOCOL   SimpleInput;
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL SimpleInputEx;
#endif    
  EFI_USB_IO_PROTOCOL           *UsbIo;

  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   IntEndpointDescriptor;

  USB_KB_BUFFER                 KeyboardBuffer;
  UINT8                         CtrlOn;
  UINT8                         AltOn;
  UINT8                         ShiftOn;
  UINT8                         NumLockOn;
  UINT8                         CapsOn;
  UINT8                         ScrollOn;
  UINT8                         LastKeyCodeArray[8];
  UINT8                         CurKeyChar;

  UINT8                         RepeatKey;
  EFI_EVENT                     RepeatTimer;

  EFI_UNICODE_STRING_TABLE      *ControllerNameTable;
  
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  UINT8                         LeftCtrlOn;
  UINT8                         LeftAltOn;
  UINT8                         LeftShiftOn;
  UINT8                         LeftLogoOn;
  UINT8                         RightCtrlOn;
  UINT8                         RightAltOn;
  UINT8                         RightShiftOn;
  UINT8                         RightLogoOn;  
  UINT8                         MenuKeyOn;
  UINT8                         SysReqOn;
  UINT8                         AltGrOn;

  EFI_KEY_STATE                 KeyState;
  //
  // Notification function list
  //
  EFI_LIST_ENTRY                NotifyList;

  //
  // Non-spacing key list
  //
  EFI_LIST_ENTRY                NsKeyList;
  USB_NS_KEY                    *CurrentNsKey;
  EFI_KEY_DESCRIPTOR            *KeyConvertionTable;
  EFI_EVENT                     KeyboardLayoutEvent;
#endif
} USB_KB_DEV;

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gUsbKeyboardDriverBinding;
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
extern EFI_COMPONENT_NAME2_PROTOCOL gUsbKeyboardComponentName;
#else
extern EFI_COMPONENT_NAME_PROTOCOL  gUsbKeyboardComponentName;
#endif
extern EFI_GUID                     gEfiUsbKeyboardDriverGuid;

VOID
KbdReportStatusCode (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath,
  IN EFI_STATUS_CODE_TYPE      CodeType,
  IN EFI_STATUS_CODE_VALUE     Value
  );

#define USB_KB_DEV_FROM_THIS(a) \
    CR(a, USB_KB_DEV, SimpleInput, USB_KB_DEV_SIGNATURE)
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#define TEXT_INPUT_EX_USB_KB_DEV_FROM_THIS(a) \
    CR(a, USB_KB_DEV, SimpleInputEx, USB_KB_DEV_SIGNATURE)

#endif

#define MOD_CONTROL_L           0x01
#define MOD_CONTROL_R           0x10
#define MOD_SHIFT_L             0x02
#define MOD_SHIFT_R             0x20
#define MOD_ALT_L               0x04
#define MOD_ALT_R               0x40
#define MOD_WIN_L               0x08
#define MOD_WIN_R               0x80

typedef struct {
  UINT8 Mask;
  UINT8 Key;
} KB_MODIFIER;

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#define USB_KEYCODE_MAX_MAKE      0x62
#else
#define USB_KEYCODE_MAX_MAKE      0x64
#endif

#define USBKBD_VALID_KEYCODE(key) ((UINT8) (key) > 3)

typedef struct {
  UINT8 NumLock : 1;
  UINT8 CapsLock : 1;
  UINT8 ScrollLock : 1;
  UINT8 Resrvd : 5;
} LED_MAP;

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
//
// Simple Text Input Ex protocol functions
//

EFI_STATUS
EFIAPI
USBKeyboardResetEx (
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
USBKeyboardReadKeyStrokeEx (
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
USBKeyboardSetState (
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
USBKeyboardRegisterKeyNotify (
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
USBKeyboardUnregisterKeyNotify (
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

#endif


#endif
