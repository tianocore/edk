/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:

    UsbMouse.h

  Abstract:

--*/

#ifndef _USB_MOUSE_H
#define _USB_MOUSE_H

#include "usb.h"
#include "usbbus.h"
#include EFI_PROTOCOL_DEFINITION (UsbIo)
#include EFI_PROTOCOL_DEFINITION (SimplePointer)
#include EFI_PROTOCOL_DEFINITION (ComponentName)

#define CLASS_HID               3
#define SUBCLASS_BOOT           1
#define PROTOCOL_MOUSE          2

#define BOOT_PROTOCOL           0
#define REPORT_PROTOCOL         1

#define USB_MOUSE_DEV_SIGNATURE EFI_SIGNATURE_32 ('u', 'm', 'o', 'u')

typedef struct {
  BOOLEAN ButtonDetected;
  UINT8   ButtonMinIndex;
  UINT8   ButtonMaxIndex;
  UINT8   Reserved;
} PRIVATE_DATA;

typedef struct {
  UINTN                         Signature;
  EFI_EVENT                     DelayedRecoveryEvent;
  EFI_USB_IO_PROTOCOL           *UsbIo;
  EFI_USB_INTERFACE_DESCRIPTOR  *InterfaceDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   *IntEndpointDescriptor;
  UINT8                         NumberOfButtons;
  INT32                         XLogicMax;
  INT32                         XLogicMin;
  INT32                         YLogicMax;
  INT32                         YLogicMin;
  EFI_SIMPLE_POINTER_PROTOCOL   SimplePointerProtocol;
  EFI_SIMPLE_POINTER_STATE      State;
  EFI_SIMPLE_POINTER_MODE       Mode;
  BOOLEAN                       StateChanged;
  PRIVATE_DATA                  PrivateData;
  EFI_UNICODE_STRING_TABLE      *ControllerNameTable;
} USB_MOUSE_DEV;

#define USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL(a) \
    CR(a, USB_MOUSE_DEV, SimplePointerProtocol, USB_MOUSE_DEV_SIGNATURE)

VOID
USBMouseRecoveryHandler (
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

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gUsbMouseDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gUsbMouseComponentName;
extern EFI_GUID                     gEfiUsbMouseDriverGuid;

VOID
MouseReportStatusCode (
  IN  EFI_USB_IO_PROTOCOL       *UsbIo,
  IN  EFI_STATUS_CODE_TYPE      CodeType,
  IN  EFI_STATUS_CODE_VALUE     Value
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo     - TODO: add argument description
  CodeType  - TODO: add argument description
  Value     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
