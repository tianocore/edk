/*++
Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:

    usbbus.h

  Abstract:

    Header file for USB bus driver Interface

  Revision History



--*/

#ifndef _EFI_USB_BUS_H
#define _EFI_USB_BUS_H

#include EFI_PROTOCOL_DEFINITION(UsbHostController)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)

//
// Define USB DEBUG Value
//
#define EFI_D_USB   EFI_D_ERROR

//
// We define some maximun value here
//
#define USB_MAXCONFIG         8
#define USB_MAXALTSETTING     4
#define USB_MAXINTERFACES     32
#define USB_MAXENDPOINTS      16
#define USB_MAXSTRINGS        16
#define USB_MAXLANID          16
#define USB_MAXCHILDREN       8
#define USB_MAXCONTROLLERS    4

#define USB_IO_CONTROLLER_SIGNATURE   EFI_SIGNATURE_32('u','s','b','d')

typedef struct
{
  EFI_LIST_ENTRY      Link;
  UINT16              StringIndex;
  CHAR16              *String;
} STR_LIST_ENTRY;

typedef struct
{
  EFI_LIST_ENTRY                  Link;
  UINT16                          Toggle;
  EFI_USB_ENDPOINT_DESCRIPTOR     EndpointDescriptor;
} ENDPOINT_DESC_LIST_ENTRY;

typedef struct
{
  EFI_LIST_ENTRY                  Link;
  EFI_USB_INTERFACE_DESCRIPTOR    InterfaceDescriptor;
  EFI_LIST_ENTRY                  EndpointDescListHead;
} INTERFACE_DESC_LIST_ENTRY;

typedef struct
{
  EFI_LIST_ENTRY                  Link;
  EFI_USB_CONFIG_DESCRIPTOR       CongfigDescriptor;
  EFI_LIST_ENTRY                  InterfaceDescListHead;
  UINTN                           ActiveInterface;
} CONFIG_DESC_LIST_ENTRY;

//
// Forward declaring
//
struct usb_io_device;

//
// This is used to form the USB Controller Handle
//
typedef struct usb_io_controller_device
{
  UINTN                               Signature;
  EFI_HANDLE                          Handle;
  EFI_USB_IO_PROTOCOL                 UsbIo;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
  EFI_HANDLE                          HostController;
  UINT8                               CurrentConfigValue;
  UINT8                               InterfaceNumber;
  struct usb_io_device                *UsbDevice;

  BOOLEAN                             IsUsbHub;
  BOOLEAN                             IsManagedByDriver;

  //
  // Fields specified for USB Hub
  //
  EFI_EVENT                           HubNotify;
  UINT8                               HubEndpointAddress;
  UINT8                               StatusChangePort;
  UINT8                               DownstreamPorts;

  UINT8                               ParentPort;
  struct usb_io_controller_device     *Parent;
  struct usb_io_device                *Children[USB_MAXCHILDREN];
} USB_IO_CONTROLLER_DEVICE;

#define USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS(a) \
    CR(a, USB_IO_CONTROLLER_DEVICE, UsbIo, USB_IO_CONTROLLER_SIGNATURE)

//
// This is used to keep the topology of USB bus
//
struct _usb_bus_controller_device;

typedef struct usb_io_device {
  UINT8                               DeviceAddress;
  BOOLEAN                             IsConfigured;
  BOOLEAN                             IsSlowDevice;
  EFI_USB_DEVICE_DESCRIPTOR           DeviceDescriptor;
  EFI_LIST_ENTRY                      ConfigDescListHead;
  CONFIG_DESC_LIST_ENTRY              *ActiveConfig;
  UINT16                              LangID[USB_MAXLANID];

  struct _usb_bus_controller_device   *BusController;

  //
  // Track the controller handle
  //
  UINT8                               NumOfControllers;
  USB_IO_CONTROLLER_DEVICE            *UsbController[USB_MAXCONTROLLERS];

} USB_IO_DEVICE;

//
// Usb Bus Controller device strcuture
//
#define EFI_USB_BUS_PROTOCOL_GUID \
    { 0x2B2F68CC, 0x0CD2, 0x44cf, 0x8E, 0x8B, 0xBB, 0xA2, 0x0B, 0x1B, 0x5B, 0x75 }

typedef struct _EFI_USB_BUS_PROTOCOL
{
  UINT64                  Reserved;
} EFI_USB_BUS_PROTOCOL;

#define USB_BUS_DEVICE_SIGNATURE   EFI_SIGNATURE_32('u','b','u','s')

typedef struct _usb_bus_controller_device
{
  UINTN                         Signature;

  EFI_USB_BUS_PROTOCOL          BusIdentify;
  EFI_USB_HC_PROTOCOL           *UsbHCInterface;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  UINT8                         AddressPool[16];
  USB_IO_DEVICE                 *Root;
} USB_BUS_CONTROLLER_DEVICE;

#define USB_BUS_CONTROLLER_DEVICE_FROM_THIS(a) \
    CR(a, USB_BUS_CONTROLLER_DEVICE, BusIdentify, USB_BUS_DEVICE_SIGNATURE)


//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gUsbBusDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gUsbBusComponentName;
extern EFI_GUID                     gUSBBusDriverGuid;

//
// Usb Device Configuration functions
//
BOOLEAN
IsHub (
  IN USB_IO_CONTROLLER_DEVICE     *Dev
  );

EFI_STATUS
UsbGetStringtable (
  IN  USB_IO_DEVICE     *Dev
  );


EFI_STATUS
UsbGetAllConfigurations (
  IN  USB_IO_DEVICE     *UsbDev
  );


EFI_STATUS
UsbSetConfiguration (
  IN  USB_IO_DEVICE     *Dev,
  IN  UINTN             ConfigurationValue
  );


EFI_STATUS
UsbSetDefaultConfiguration(
  IN  USB_IO_DEVICE     *Dev
  );


//
// Device Deconfiguration functions
//
VOID
UsbDestroyAllConfiguration (
  IN USB_IO_DEVICE     *Dev
  );

EFI_STATUS
DoHubConfig(
  IN USB_IO_CONTROLLER_DEVICE     *HubIoDevice
  );

VOID
GetDeviceEndPointMaxPacketLength (
  IN  USB_IO_CONTROLLER_DEVICE     *UsbIoController,
  IN  UINT8                        EndpointAddr,
  OUT UINT8                        *MaxPacketLength
  );

VOID
GetDataToggleBit (
  IN  USB_IO_CONTROLLER_DEVICE     *UsbIoController,
  IN  UINT8                        EndpointAddr,
  OUT UINT8                        *DataToggle
);

VOID
SetDataToggleBit (
  IN USB_IO_CONTROLLER_DEVICE     *UsbIoController,
  IN UINT8                        EndpointAddr,
  IN UINT8                        DataToggle
);

#endif
