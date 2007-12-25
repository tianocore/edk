/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiKey.c
    
Abstract:

  USB Keyboard Driver

Revision History

--*/

#include "efikey.h"
#include "keyboard.h"

//
// Prototypes
// Driver model protocol interface
//
EFI_STATUS
EFIAPI
USBKeyboardDriverBindingEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
USBKeyboardDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
USBKeyboardDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
USBKeyboardDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

//
// Simple Text In Protocol Interface
//
STATIC
EFI_STATUS
EFIAPI
USBKeyboardReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  IN  BOOLEAN                      ExtendedVerification
  );

STATIC
EFI_STATUS
EFIAPI
USBKeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL   *This,
  OUT EFI_INPUT_KEY                 *Key
  );

STATIC
VOID
EFIAPI
USBKeyboardWaitForKey (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );

//
//  Helper functions
//
STATIC
EFI_STATUS
USBKeyboardCheckForKey (
  IN  USB_KB_DEV      *UsbKeyboardDevice
  );

EFI_GUID  gEfiUsbKeyboardDriverGuid = {
  0xa05f5f78, 0xfb3, 0x4d10, 0x90, 0x90, 0xac, 0x4, 0x6e, 0xeb, 0x7c, 0x3c
};

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
STATIC
EFI_STATUS
KbdFreeNotifyList (
  IN OUT EFI_LIST_ENTRY       *ListHead
  );  
STATIC
BOOLEAN
IsKeyRegistered (
  IN EFI_KEY_DATA  *RegsiteredData,
  IN EFI_KEY_DATA  *InputData
  );

EFI_GUID gSimpleTextInExNotifyGuid = { \
  0x856f2def, 0x4e93, 0x4d6b, 0x94, 0xce, 0x1c, 0xfe, 0x47, 0x1, 0x3e, 0xa5 \
};
#endif

//
// USB Keyboard Driver Global Variables
//
EFI_DRIVER_BINDING_PROTOCOL gUsbKeyboardDriverBinding = {
  USBKeyboardDriverBindingSupported,
  USBKeyboardDriverBindingStart,
  USBKeyboardDriverBindingStop,
  0xa,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT (USBKeyboardDriverBindingEntryPoint)

EFI_STATUS
EFIAPI
USBKeyboardDriverBindingEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++
  
  Routine Description:
    Driver Entry Point.
        
  Arguments:
    ImageHandle - EFI_HANDLE
    SystemTable - EFI_SYSTEM_TABLE
  Returns:
    EFI_STATUS
  
--*/       
{
  return EfiLibInstallAllDriverProtocols (
          ImageHandle,
          SystemTable,
          &gUsbKeyboardDriverBinding,
          ImageHandle,
          &gUsbKeyboardComponentName,
          NULL,
          NULL
          );
}


EFI_STATUS
EFIAPI
USBKeyboardDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Supported.
    
  Arguments:
    This          - EFI_DRIVER_BINDING_PROTOCOL
    Controller    - Controller handle
    RemainingDevicePath - EFI_DEVICE_PATH_PROTOCOL 
  Returns:
    EFI_STATUS
  
--*/ 
{
  EFI_STATUS          OpenStatus;
  EFI_USB_IO_PROTOCOL *UsbIo;
  EFI_STATUS          Status;

  //
  // Check if USB_IO protocol is attached on the controller handle.
  //
  OpenStatus = gBS->OpenProtocol (
                      Controller,
                      &gEfiUsbIoProtocolGuid,
                      &UsbIo,
                      This->DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_BY_DRIVER
                      );
  if (EFI_ERROR (OpenStatus)) {
    return OpenStatus;
  }
   
  //
  // Use the USB I/O protocol interface to check whether the Controller is
  // the Keyboard controller that can be managed by this driver.
  //
  Status = EFI_SUCCESS;

  if (!IsUSBKeyboard (UsbIo)) {
    Status = EFI_UNSUPPORTED;
  }

  gBS->CloseProtocol (
        Controller,
        &gEfiUsbIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

EFI_STATUS
EFIAPI
USBKeyboardDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Start.
  
  Arguments:
    This       - EFI_DRIVER_BINDING_PROTOCOL
    Controller - Controller handle
    RemainingDevicePath - EFI_DEVICE_PATH_PROTOCOL
  Returns:
    EFI_SUCCESS          - Success
    EFI_OUT_OF_RESOURCES - Can't allocate memory
    EFI_UNSUPPORTED      - The Start routine fail
--*/       
{ 
  EFI_STATUS                    Status;
  EFI_USB_IO_PROTOCOL           *UsbIo;
  USB_KB_DEV                    *UsbKeyboardDevice;
  UINT8                         EndpointNumber;
  EFI_USB_ENDPOINT_DESCRIPTOR   EndpointDescriptor;
  UINT8                         Index;
  UINT8                         EndpointAddr;
  UINT8                         PollingInterval;
  UINT8                         PacketSize;
  BOOLEAN                       Found;
  
  UsbKeyboardDevice = NULL;
  Found             = FALSE;

  //
  // Open USB_IO Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUsbIoProtocolGuid,
                  &UsbIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  UsbKeyboardDevice = EfiLibAllocateZeroPool (sizeof (USB_KB_DEV));
  if (UsbKeyboardDevice == NULL) {
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Get the Device Path Protocol on Controller's handle
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &UsbKeyboardDevice->DevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return Status;
  }
  //
  // Report that the usb keyboard is being enabled
  //
  KbdReportStatusCode (
    UsbKeyboardDevice->DevicePath,
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_ENABLE)
    );

  //
  // This is pretty close to keyboard detection, so log progress
  //
  KbdReportStatusCode (
    UsbKeyboardDevice->DevicePath,
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_PRESENCE_DETECT)
    );

  //
  // Initialize UsbKeyboardDevice
  //
  UsbKeyboardDevice->UsbIo = UsbIo;

  //
  // Get interface & endpoint descriptor
  //
  UsbIo->UsbGetInterfaceDescriptor (
          UsbIo,
          &UsbKeyboardDevice->InterfaceDescriptor
          );

  EndpointNumber = UsbKeyboardDevice->InterfaceDescriptor.NumEndpoints;

  for (Index = 0; Index < EndpointNumber; Index++) {

    UsbIo->UsbGetEndpointDescriptor (
            UsbIo,
            Index,
            &EndpointDescriptor
            );

    if ((EndpointDescriptor.Attributes & 0x03) == 0x03) {
      //
      // We only care interrupt endpoint here
      //
      UsbKeyboardDevice->IntEndpointDescriptor  = EndpointDescriptor;
      Found = TRUE;
    }
  }

  if (!Found) {
    //
    // No interrupt endpoint found, then return unsupported.
    //
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_UNSUPPORTED;
  }

  UsbKeyboardDevice->Signature                  = USB_KB_DEV_SIGNATURE;
  UsbKeyboardDevice->SimpleInput.Reset          = USBKeyboardReset;
  UsbKeyboardDevice->SimpleInput.ReadKeyStroke  = USBKeyboardReadKeyStroke;

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#ifndef DISABLE_CONSOLE_EX
  UsbKeyboardDevice->SimpleInputEx.Reset               = USBKeyboardResetEx;
  UsbKeyboardDevice->SimpleInputEx.ReadKeyStrokeEx     = USBKeyboardReadKeyStrokeEx;
  UsbKeyboardDevice->SimpleInputEx.SetState            = USBKeyboardSetState;
  UsbKeyboardDevice->SimpleInputEx.RegisterKeyNotify   = USBKeyboardRegisterKeyNotify;
  UsbKeyboardDevice->SimpleInputEx.UnregisterKeyNotify = USBKeyboardUnregisterKeyNotify; 
  
  InitializeListHead (&UsbKeyboardDevice->NotifyList);
  
  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_WAIT,
                  EFI_TPL_NOTIFY,
                  USBKeyboardWaitForKey,
                  UsbKeyboardDevice,
                  &(UsbKeyboardDevice->SimpleInputEx.WaitForKeyEx)
                  );

  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }
#endif  // DISABLE_CONSOLE_EX
#endif

  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_WAIT,
                  EFI_TPL_NOTIFY,
                  USBKeyboardWaitForKey,
                  UsbKeyboardDevice,
                  &(UsbKeyboardDevice->SimpleInput.WaitForKey)
                  );

  if (EFI_ERROR (Status)) {
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return Status;
  }
       
  //
  // Install simple txt in protocol interface
  // for the usb keyboard device.
  // Usb keyboard is a hot plug device, and expected to work immediately
  // when plugging into system, so a HotPlugDeviceGuid is installed onto
  // the usb keyboard device handle, to distinguish it from other conventional
  // console devices.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  &UsbKeyboardDevice->SimpleInput,
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#ifndef DISABLE_CONSOLE_EX
                  &gEfiSimpleTextInputExProtocolGuid,
                  &UsbKeyboardDevice->SimpleInputEx,
#endif  // DISABLE_CONSOLE_EX
#endif                  
                  &gEfiHotPlugDeviceGuid,
                  NULL,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (UsbKeyboardDevice->SimpleInput.WaitForKey);
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return Status;
  }

  //
  // Reset USB Keyboard Device
  //
  Status = UsbKeyboardDevice->SimpleInput.Reset (
                                            &UsbKeyboardDevice->SimpleInput,
                                            TRUE
                                            );
  if (EFI_ERROR (Status)) {
    gBS->UninstallMultipleProtocolInterfaces (
           Controller,
           &gEfiSimpleTextInProtocolGuid,
           &UsbKeyboardDevice->SimpleInput,
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#ifndef DISABLE_CONSOLE_EX
           &gEfiSimpleTextInputExProtocolGuid,
           &UsbKeyboardDevice->SimpleInputEx,
#endif  // DISABLE_CONSOLE_EX
#endif                            
           &gEfiHotPlugDeviceGuid,
           NULL,
           NULL
           );
    gBS->CloseEvent (UsbKeyboardDevice->SimpleInput.WaitForKey);
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return Status;
  }
  //
  // submit async interrupt transfer
  //
  EndpointAddr    = UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress;
  PollingInterval = UsbKeyboardDevice->IntEndpointDescriptor.Interval;
  PacketSize      = (UINT8) (UsbKeyboardDevice->IntEndpointDescriptor.MaxPacketSize);

  Status = UsbIo->UsbAsyncInterruptTransfer (
                    UsbIo,
                    EndpointAddr,
                    TRUE,
                    PollingInterval,
                    PacketSize,
                    KeyboardHandler,
                    UsbKeyboardDevice
                    );

  if (EFI_ERROR (Status)) {

    gBS->UninstallMultipleProtocolInterfaces (
           Controller,
           &gEfiSimpleTextInProtocolGuid,
           &UsbKeyboardDevice->SimpleInput,
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#ifndef DISABLE_CONSOLE_EX
           &gEfiSimpleTextInputExProtocolGuid,
           &UsbKeyboardDevice->SimpleInputEx,
#endif  // DISABLE_CONSOLE_EX
#endif                                      
           &gEfiHotPlugDeviceGuid,
           NULL,
           NULL
           );
    gBS->CloseEvent (UsbKeyboardDevice->SimpleInput.WaitForKey);
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return Status;
  }

  UsbKeyboardDevice->ControllerNameTable = NULL;
  EfiLibAddUnicodeString (
    LANGUAGE_CODE_ENGLISH,
    gUsbKeyboardComponentName.SupportedLanguages,
    &UsbKeyboardDevice->ControllerNameTable,
    L"Generic Usb Keyboard"
    );

  return EFI_SUCCESS;

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#ifndef DISABLE_CONSOLE_EX
ErrorExit:
  if (UsbKeyboardDevice != NULL) {
    if (UsbKeyboardDevice->SimpleInput.WaitForKey != NULL) {
      gBS->CloseEvent (UsbKeyboardDevice->SimpleInput.WaitForKey);
    }
    if (UsbKeyboardDevice->SimpleInputEx.WaitForKeyEx != NULL) {
      gBS->CloseEvent (UsbKeyboardDevice->SimpleInputEx.WaitForKeyEx);
    }
    KbdFreeNotifyList (&UsbKeyboardDevice->NotifyList);    
    EfiLibSafeFreePool (UsbKeyboardDevice);
    UsbKeyboardDevice = NULL;
  }
  gBS->CloseProtocol (
         Controller,
         &gEfiUsbIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );
  return Status;
#endif  // DISABLE_CONSOLE_EX
#endif      

}


EFI_STATUS
EFIAPI
USBKeyboardDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
/*++
  
  Routine Description:
    Stop.
  
  Arguments:
    This              - EFI_DRIVER_BINDING_PROTOCOL
    Controller        - Controller handle
    NumberOfChildren  - Child handle number
    ChildHandleBuffer - Child handle buffer 
  Returns:
    EFI_SUCCESS       - Success
    EFI_UNSUPPORTED   - Can't support 
--*/       
{
  EFI_STATUS                  Status;
  EFI_SIMPLE_TEXT_IN_PROTOCOL *SimpleInput;
  USB_KB_DEV                  *UsbKeyboardDevice;
  EFI_USB_IO_PROTOCOL         *UsbIo;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  &SimpleInput,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#ifndef DISABLE_CONSOLE_EX
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextInputExProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }
#endif  // DISABLE_CONSOLE_EX
#endif
  //
  // Get USB_KB_DEV instance.
  //
  UsbKeyboardDevice = USB_KB_DEV_FROM_THIS (SimpleInput);

  gBS->CloseProtocol (
        Controller,
        &gEfiSimpleTextInProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  UsbIo = UsbKeyboardDevice->UsbIo;
  //
  // Uninstall the Asyn Interrupt Transfer from this device
  // will disable the key data input from this device
  //
  KbdReportStatusCode (
    UsbKeyboardDevice->DevicePath,
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_DISABLE)
    );

  //
  // Destroy asynchronous interrupt transfer
  //
  UsbKeyboardDevice->UsbIo->UsbAsyncInterruptTransfer (
                              UsbKeyboardDevice->UsbIo,
                              UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
                              FALSE,
                              UsbKeyboardDevice->IntEndpointDescriptor.Interval,
                              0,
                              NULL,
                              NULL
                              );

  gBS->CloseProtocol (
        Controller,
        &gEfiUsbIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  &UsbKeyboardDevice->SimpleInput,
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#ifndef DISABLE_CONSOLE_EX
                  &gEfiSimpleTextInputExProtocolGuid,
                  &UsbKeyboardDevice->SimpleInputEx,
#endif  // DISABLE_CONSOLE_EX
#endif
                  &gEfiHotPlugDeviceGuid,
                  NULL,
                  NULL
                  );
  //
  // free all the resources.
  //
  gBS->CloseEvent (UsbKeyboardDevice->RepeatTimer);
  gBS->CloseEvent (UsbKeyboardDevice->DelayedRecoveryEvent);
  gBS->CloseEvent ((UsbKeyboardDevice->SimpleInput).WaitForKey);
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#ifndef DISABLE_CONSOLE_EX
  gBS->CloseEvent (UsbKeyboardDevice->SimpleInputEx.WaitForKeyEx);  
  KbdFreeNotifyList (&UsbKeyboardDevice->NotifyList);    
#endif  // DISABLE_CONSOLE_EX
#endif    
  
  if (UsbKeyboardDevice->ControllerNameTable != NULL) {
    EfiLibFreeUnicodeStringTable (UsbKeyboardDevice->ControllerNameTable);
  }

  gBS->FreePool (UsbKeyboardDevice);

  return Status;

}

STATIC
EFI_STATUS
USBKeyboardReadKeyStrokeWorker (
  IN  USB_KB_DEV                        *UsbKeyboardDevice,
  OUT EFI_KEY_DATA                      *KeyData
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can 
    be used to test for existance of a keystroke via WaitForEvent () call.

  Arguments:
    UsbKeyboardDevice     - Usb keyboard private structure.
    KeyData               - A pointer to a buffer that is filled in with the keystroke 
                            state data for the key that was pressed.

  Returns:
    EFI_SUCCESS           - The keystroke information was returned.
    EFI_NOT_READY         - There was no keystroke data availiable.
    EFI_DEVICE_ERROR      - The keystroke information was not returned due to 
                            hardware errors.
    EFI_INVALID_PARAMETER - KeyData is NULL.                        

--*/
{

  EFI_STATUS                        Status;
  UINT8                             KeyChar;  
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#ifndef DISABLE_CONSOLE_EX
  EFI_LIST_ENTRY                    *Link;
  KEYBOARD_CONSOLE_IN_EX_NOTIFY     *CurrentNotify;  
  EFI_KEY_DATA                      OriginalKeyData;
#endif  // DISABLE_CONSOLE_EX
#endif  

  if (KeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // if there is no saved ASCII byte, fetch it
  // by calling USBKeyboardCheckForKey().
  //
  if (UsbKeyboardDevice->CurKeyChar == 0) {
    Status = USBKeyboardCheckForKey (UsbKeyboardDevice);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  KeyData->Key.UnicodeChar = 0;
  KeyData->Key.ScanCode    = SCAN_NULL;

  KeyChar = UsbKeyboardDevice->CurKeyChar;

  UsbKeyboardDevice->CurKeyChar = 0;

  //
  // Translate saved ASCII byte into EFI_INPUT_KEY
  //
  Status = USBKeyCodeToEFIScanCode (UsbKeyboardDevice, KeyChar, &KeyData->Key);
  if (EFI_ERROR (Status)) {
    return Status;
  }

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#ifndef DISABLE_CONSOLE_EX
  EfiCopyMem (&KeyData->KeyState, &UsbKeyboardDevice->KeyState, sizeof (KeyData->KeyState));
  
  UsbKeyboardDevice->KeyState.KeyShiftState  = EFI_SHIFT_STATE_VALID;
  UsbKeyboardDevice->KeyState.KeyToggleState = EFI_TOGGLE_STATE_VALID;

  //
  //Switch the control value to their original characters. In USBKeyCodeToEFIScanCode() the  CTRL-Alpha characters have been switched to 
  // their corresponding control value (ctrl-a = 0x0001 through ctrl-Z = 0x001A), here switch them back for notification function.
  //
  EfiCopyMem (&OriginalKeyData, KeyData, sizeof (EFI_KEY_DATA));
  if (UsbKeyboardDevice->CtrlOn) {
    if (OriginalKeyData.Key.UnicodeChar >= 0x01 && OriginalKeyData.Key.UnicodeChar <= 0x1A) {
      if (UsbKeyboardDevice->CapsOn) {
        OriginalKeyData.Key.UnicodeChar = OriginalKeyData.Key.UnicodeChar + 'A' - 1;
      } else {
        OriginalKeyData.Key.UnicodeChar = OriginalKeyData.Key.UnicodeChar + 'a' - 1;
      } 
    }
  }
  
  //
  // Invoke notification functions if exist
  //
  for (Link = UsbKeyboardDevice->NotifyList.ForwardLink; Link != &UsbKeyboardDevice->NotifyList; Link = Link->ForwardLink) {
    CurrentNotify = CR (
                      Link, 
                      KEYBOARD_CONSOLE_IN_EX_NOTIFY, 
                      NotifyEntry, 
                      USB_KB_CONSOLE_IN_EX_NOTIFY_SIGNATURE
                      );
    if (IsKeyRegistered (&CurrentNotify->KeyData, &OriginalKeyData)) { 
      CurrentNotify->KeyNotificationFn (&OriginalKeyData);
    }
  }
#endif  // DISABLE_CONSOLE_EX
#endif

  return EFI_SUCCESS;
  
}
EFI_STATUS
EFIAPI
USBKeyboardReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL   *This,
  IN  BOOLEAN                       ExtendedVerification
  )
/*++

  Routine Description:
    Implements EFI_SIMPLE_TEXT_IN_PROTOCOL.Reset() function.
  
  Arguments:
    This      The EFI_SIMPLE_TEXT_IN_PROTOCOL instance.
    ExtendedVerification
              Indicates that the driver may perform a more exhaustive
              verification operation of the device during reset.              
    
  Returns:  
    EFI_SUCCESS      - Success
    EFI_DEVICE_ERROR - Hardware Error
--*/      
{
  EFI_STATUS          Status;
  USB_KB_DEV          *UsbKeyboardDevice;
  EFI_USB_IO_PROTOCOL *UsbIo;

  UsbKeyboardDevice = USB_KB_DEV_FROM_THIS (This);

  UsbIo             = UsbKeyboardDevice->UsbIo;

  KbdReportStatusCode (
    UsbKeyboardDevice->DevicePath,
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_RESET)
    );

  //
  // Non Exhaustive reset:
  // only reset private data structures.
  //
  if (!ExtendedVerification) {
    //
    // Clear the key buffer of this Usb keyboard
    //
    KbdReportStatusCode (
      UsbKeyboardDevice->DevicePath,
      EFI_PROGRESS_CODE,
      (EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_CLEAR_BUFFER)
      );

    InitUSBKeyBuffer (&(UsbKeyboardDevice->KeyboardBuffer));
    UsbKeyboardDevice->CurKeyChar = 0;
    return EFI_SUCCESS;
  }
  
  //
  // Exhaustive reset
  //
  Status                        = InitUSBKeyboard (UsbKeyboardDevice);
  UsbKeyboardDevice->CurKeyChar = 0;
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
USBKeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL   *This,
  OUT EFI_INPUT_KEY                 *Key
  )
/*++

  Routine Description:
    Implements EFI_SIMPLE_TEXT_IN_PROTOCOL.ReadKeyStroke() function.
  
  Arguments:
    This     The EFI_SIMPLE_TEXT_IN_PROTOCOL instance.
    Key      A pointer to a buffer that is filled in with the keystroke
             information for the key that was pressed.
    
  Returns:  
    EFI_SUCCESS - Success
--*/       
{
  USB_KB_DEV   *UsbKeyboardDevice;
  EFI_STATUS   Status;
  EFI_KEY_DATA KeyData;

  UsbKeyboardDevice = USB_KB_DEV_FROM_THIS (This);

  Status = USBKeyboardReadKeyStrokeWorker (UsbKeyboardDevice, &KeyData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  EfiCopyMem (Key, &KeyData.Key, sizeof (EFI_INPUT_KEY));

  return EFI_SUCCESS;

}

STATIC
VOID
EFIAPI
USBKeyboardWaitForKey (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  )
/*++

  Routine Description:
    Handler function for WaitForKey event.    
  
  Arguments:
    Event        Event to be signaled when a key is pressed.
    Context      Points to USB_KB_DEV instance.
    
  Returns:  
    VOID
--*/       
{
  USB_KB_DEV  *UsbKeyboardDevice;

  UsbKeyboardDevice = (USB_KB_DEV *) Context;

  if (UsbKeyboardDevice->CurKeyChar == 0) {

    if (EFI_ERROR (USBKeyboardCheckForKey (UsbKeyboardDevice))) {
      return ;
    }
  }
  //
  // If has key pending, signal the event.
  //
  gBS->SignalEvent (Event);
}


STATIC
EFI_STATUS
USBKeyboardCheckForKey (
  IN  USB_KB_DEV    *UsbKeyboardDevice
  )
/*++

  Routine Description:
    Check whether there is key pending.
  
  Arguments:
    UsbKeyboardDevice    The USB_KB_DEV instance.
    
  Returns:  
    EFI_SUCCESS  - Success
--*/       
{
  EFI_STATUS  Status;
  UINT8       KeyChar;

  //
  // Fetch raw data from the USB keyboard input,
  // and translate it into ASCII data.
  //
  Status = USBParseKey (UsbKeyboardDevice, &KeyChar);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  UsbKeyboardDevice->CurKeyChar = KeyChar;
  return EFI_SUCCESS;
}

VOID
KbdReportStatusCode (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath,
  IN EFI_STATUS_CODE_TYPE      CodeType,
  IN EFI_STATUS_CODE_VALUE     Value
  )
/*++

  Routine Description:
    Report Status Code in Usb Bot Driver

  Arguments:
    DevicePath  - Use this to get Device Path
    CodeType    - Status Code Type
    CodeValue   - Status Code Value

  Returns:
    None

--*/
{

  ReportStatusCodeWithDevicePath (
    CodeType,
    Value,
    0,
    &gEfiUsbKeyboardDriverGuid,
    DevicePath
    );
}

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
STATIC
EFI_STATUS
KbdFreeNotifyList (
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
  KEYBOARD_CONSOLE_IN_EX_NOTIFY *NotifyNode;

  if (ListHead == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  while (!IsListEmpty (ListHead)) {
    NotifyNode = CR (
                   ListHead->ForwardLink, 
                   KEYBOARD_CONSOLE_IN_EX_NOTIFY, 
                   NotifyEntry, 
                   USB_KB_CONSOLE_IN_EX_NOTIFY_SIGNATURE
                   );
    RemoveEntryList (ListHead->ForwardLink);
    EfiLibSafeFreePool (NotifyNode);
  }
  
  return EFI_SUCCESS;
}

STATIC
BOOLEAN
IsKeyRegistered (
  IN EFI_KEY_DATA  *RegsiteredData,
  IN EFI_KEY_DATA  *InputData
  )
/*++

Routine Description:

Arguments:

  RegsiteredData    - A pointer to a buffer that is filled in with the keystroke 
                      state data for the key that was registered.
  InputData         - A pointer to a buffer that is filled in with the keystroke 
                      state data for the key that was pressed.

Returns:
  TRUE              - Key be pressed matches a registered key.
  FLASE             - Match failed. 
  
--*/
{
  ASSERT (RegsiteredData != NULL && InputData != NULL);
  
  if ((RegsiteredData->Key.ScanCode    != InputData->Key.ScanCode) ||
      (RegsiteredData->Key.UnicodeChar != InputData->Key.UnicodeChar)) {
    return FALSE;  
  }      
  
  //
  // Assume KeyShiftState/KeyToggleState = 0 in Registered key data means these state could be ignored.
  //
  if (RegsiteredData->KeyState.KeyShiftState != 0 &&
      RegsiteredData->KeyState.KeyShiftState != InputData->KeyState.KeyShiftState) {
    return FALSE;    
  }   
  if (RegsiteredData->KeyState.KeyToggleState != 0 &&
      RegsiteredData->KeyState.KeyToggleState != InputData->KeyState.KeyToggleState) {
    return FALSE;    
  }     
  
  return TRUE;

}

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
{
  EFI_STATUS                Status;
  USB_KB_DEV                *UsbKeyboardDevice;
  EFI_TPL                   OldTpl;
  

  UsbKeyboardDevice = TEXT_INPUT_EX_USB_KB_DEV_FROM_THIS (This);

  Status = UsbKeyboardDevice->SimpleInput.Reset (&UsbKeyboardDevice->SimpleInput, ExtendedVerification);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  OldTpl = gBS->RaiseTPL (EFI_TPL_NOTIFY);
  UsbKeyboardDevice->KeyState.KeyShiftState  = EFI_SHIFT_STATE_VALID;
  UsbKeyboardDevice->KeyState.KeyToggleState = EFI_TOGGLE_STATE_VALID;
  gBS->RestoreTPL (OldTpl);

  return EFI_SUCCESS;

}

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
{
  USB_KB_DEV                        *UsbKeyboardDevice;

  if (KeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  UsbKeyboardDevice = TEXT_INPUT_EX_USB_KB_DEV_FROM_THIS (This);

  return USBKeyboardReadKeyStrokeWorker (UsbKeyboardDevice, KeyData);
  
}

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
{
  USB_KB_DEV                        *UsbKeyboardDevice;

  if (KeyToggleState == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  UsbKeyboardDevice = TEXT_INPUT_EX_USB_KB_DEV_FROM_THIS (This);

  if (((UsbKeyboardDevice->KeyState.KeyToggleState & EFI_TOGGLE_STATE_VALID) != EFI_TOGGLE_STATE_VALID) ||
      ((*KeyToggleState & EFI_TOGGLE_STATE_VALID) != EFI_TOGGLE_STATE_VALID)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Update the status light
  //

  UsbKeyboardDevice->ScrollOn   = 0;
  UsbKeyboardDevice->NumLockOn  = 0;
  UsbKeyboardDevice->CapsOn     = 0;
 
  if ((*KeyToggleState & EFI_SCROLL_LOCK_ACTIVE) == EFI_SCROLL_LOCK_ACTIVE) {
    UsbKeyboardDevice->ScrollOn = 1;
  }
  if ((*KeyToggleState & EFI_NUM_LOCK_ACTIVE) == EFI_NUM_LOCK_ACTIVE) {
    UsbKeyboardDevice->NumLockOn = 1;
  }
  if ((*KeyToggleState & EFI_CAPS_LOCK_ACTIVE) == EFI_CAPS_LOCK_ACTIVE) {
    UsbKeyboardDevice->CapsOn = 1;
  }

  SetKeyLED (UsbKeyboardDevice);

  UsbKeyboardDevice->KeyState.KeyToggleState = *KeyToggleState;

  return EFI_SUCCESS;
  
}

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
{
  USB_KB_DEV                        *UsbKeyboardDevice;
  EFI_STATUS                        Status;
  KEYBOARD_CONSOLE_IN_EX_NOTIFY     *NewNotify;
  EFI_LIST_ENTRY                    *Link;
  KEYBOARD_CONSOLE_IN_EX_NOTIFY     *CurrentNotify;  

  if (KeyData == NULL || NotifyHandle == NULL || KeyNotificationFunction == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  UsbKeyboardDevice = TEXT_INPUT_EX_USB_KB_DEV_FROM_THIS (This);

  //
  // Return EFI_SUCCESS if the (KeyData, NotificationFunction) is already registered.
  //
  for (Link = UsbKeyboardDevice->NotifyList.ForwardLink; Link != &UsbKeyboardDevice->NotifyList; Link = Link->ForwardLink) {
    CurrentNotify = CR (
                      Link, 
                      KEYBOARD_CONSOLE_IN_EX_NOTIFY, 
                      NotifyEntry, 
                      USB_KB_CONSOLE_IN_EX_NOTIFY_SIGNATURE
                      );
    if (IsKeyRegistered (&CurrentNotify->KeyData, KeyData)) { 
      if (CurrentNotify->KeyNotificationFn == KeyNotificationFunction) {
        *NotifyHandle = CurrentNotify->NotifyHandle;        
        return EFI_SUCCESS;
      }
    }
  }
  
  //
  // Allocate resource to save the notification function
  //  
  NewNotify = (KEYBOARD_CONSOLE_IN_EX_NOTIFY *) EfiLibAllocateZeroPool (sizeof (KEYBOARD_CONSOLE_IN_EX_NOTIFY));
  if (NewNotify == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  NewNotify->Signature         = USB_KB_CONSOLE_IN_EX_NOTIFY_SIGNATURE;     
  NewNotify->KeyNotificationFn = KeyNotificationFunction;
  EfiCopyMem (&NewNotify->KeyData, KeyData, sizeof (EFI_KEY_DATA));
  InsertTailList (&UsbKeyboardDevice->NotifyList, &NewNotify->NotifyEntry);

  //
  // Use gSimpleTextInExNotifyGuid to get a valid EFI_HANDLE
  //  
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &NewNotify->NotifyHandle,
                  &gSimpleTextInExNotifyGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  
  *NotifyHandle = NewNotify->NotifyHandle;  
  
  return EFI_SUCCESS;
  
}

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
{
  USB_KB_DEV                        *UsbKeyboardDevice;
  EFI_STATUS                        Status;
  KEYBOARD_CONSOLE_IN_EX_NOTIFY     *CurrentNotify;
  EFI_LIST_ENTRY                    *Link;

  if (NotificationHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }  
  
  UsbKeyboardDevice = TEXT_INPUT_EX_USB_KB_DEV_FROM_THIS (This);
  
  Status = gBS->OpenProtocol (
                  NotificationHandle,
                  &gSimpleTextInExNotifyGuid,
                  NULL,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_INVALID_PARAMETER;
  }

  for (Link = UsbKeyboardDevice->NotifyList.ForwardLink; Link != &UsbKeyboardDevice->NotifyList; Link = Link->ForwardLink) {
    CurrentNotify = CR (
                      Link, 
                      KEYBOARD_CONSOLE_IN_EX_NOTIFY, 
                      NotifyEntry, 
                      USB_KB_CONSOLE_IN_EX_NOTIFY_SIGNATURE
                      );       
    if (CurrentNotify->NotifyHandle == NotificationHandle) {
      //
      // Remove the notification function from NotifyList and free resources
      //
      RemoveEntryList (&CurrentNotify->NotifyEntry);      
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      CurrentNotify->NotifyHandle,
                      &gSimpleTextInExNotifyGuid,
                      NULL,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      EfiLibSafeFreePool (CurrentNotify);            
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;  
}

#endif


