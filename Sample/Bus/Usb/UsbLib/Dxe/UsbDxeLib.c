/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

 Module Name:

    UsbDxeLib.c

 Abstract:

   Common Dxe Libarary  for USB

 Revision History

--*/

#include "UsbDxeLib.h"

//
// Get Device Descriptor
//
EFI_STATUS
UsbGetDescriptor (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Value,
  IN  UINT16                  Index,
  IN  UINT16                  DescriptorLength,
  IN  VOID                    *Descriptor,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo             - TODO: add argument description
  Value             - TODO: add argument description
  Index             - TODO: add argument description
  DescriptorLength  - TODO: add argument description
  Descriptor        - TODO: add argument description
  Status            - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;
  
  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiZeroMem (&DevReq, sizeof (EFI_USB_DEVICE_REQUEST));

  DevReq.RequestType  = USB_DEV_GET_DESCRIPTOR_REQ_TYPE;
  DevReq.Request      = USB_DEV_GET_DESCRIPTOR;
  DevReq.Value        = Value;
  DevReq.Index        = Index;
  DevReq.Length       = DescriptorLength;

  return UsbIo->UsbControlTransfer (
                  UsbIo,
                  &DevReq,
                  EfiUsbDataIn,
                  TIMEOUT_VALUE,
                  Descriptor,
                  DescriptorLength,
                  Status
                  );
}
//
// Set Device Descriptor
//
EFI_STATUS
UsbSetDescriptor (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Value,
  IN  UINT16                  Index,
  IN  UINT16                  DescriptorLength,
  IN  VOID                    *Descriptor,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo             - TODO: add argument description
  Value             - TODO: add argument description
  Index             - TODO: add argument description
  DescriptorLength  - TODO: add argument description
  Descriptor        - TODO: add argument description
  Status            - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;

  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiZeroMem (&DevReq, sizeof (EFI_USB_DEVICE_REQUEST));

  DevReq.RequestType  = USB_DEV_SET_DESCRIPTOR_REQ_TYPE;
  DevReq.Request      = USB_DEV_SET_DESCRIPTOR;
  DevReq.Value        = Value;
  DevReq.Index        = Index;
  DevReq.Length       = DescriptorLength;

  return UsbIo->UsbControlTransfer (
                  UsbIo,
                  &DevReq,
                  EfiUsbDataOut,
                  TIMEOUT_VALUE,
                  Descriptor,
                  DescriptorLength,
                  Status
                  );
}
//
// Set Address
//
EFI_STATUS
UsbSetDeviceAddress (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  AddressValue,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo         - TODO: add argument description
  AddressValue  - TODO: add argument description
  Status        - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;

  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiZeroMem (&DevReq, sizeof (EFI_USB_DEVICE_REQUEST));

  DevReq.RequestType  = USB_DEV_SET_ADDRESS_REQ_TYPE;
  DevReq.Request      = USB_DEV_SET_ADDRESS;
  DevReq.Value        = AddressValue;
  DevReq.Index        = 0;
  DevReq.Length       = 0;

  return UsbIo->UsbControlTransfer (
                  UsbIo,
                  &DevReq,
                  EfiUsbNoData,
                  TIMEOUT_VALUE,
                  NULL,
                  0,
                  Status
                  );
}
//
// Get device Interface
//
EFI_STATUS
UsbGetDeviceInterface (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Index,
  OUT UINT8                   *AltSetting,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo       - TODO: add argument description
  Index       - TODO: add argument description
  AltSetting  - TODO: add argument description
  Status      - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;

  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiZeroMem (&DevReq, sizeof (EFI_USB_DEVICE_REQUEST));

  DevReq.RequestType  = USB_DEV_GET_INTERFACE_REQ_TYPE;
  DevReq.Request      = USB_DEV_GET_INTERFACE;
  DevReq.Value        = 0;
  DevReq.Index        = Index;
  DevReq.Length       = 1;

  return UsbIo->UsbControlTransfer (
                  UsbIo,
                  &DevReq,
                  EfiUsbDataIn,
                  TIMEOUT_VALUE,
                  AltSetting,
                  1,
                  Status
                  );
}
//
// Set device interface
//
EFI_STATUS
UsbSetDeviceInterface (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  InterfaceNo,
  IN  UINT16                  AltSetting,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo       - TODO: add argument description
  InterfaceNo - TODO: add argument description
  AltSetting  - TODO: add argument description
  Status      - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;

  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiZeroMem (&DevReq, sizeof (EFI_USB_DEVICE_REQUEST));

  DevReq.RequestType  = USB_DEV_SET_INTERFACE_REQ_TYPE;
  DevReq.Request      = USB_DEV_SET_INTERFACE;
  DevReq.Value        = AltSetting;
  DevReq.Index        = InterfaceNo;
  DevReq.Length       = 0;

  return UsbIo->UsbControlTransfer (
                  UsbIo,
                  &DevReq,
                  EfiUsbNoData,
                  TIMEOUT_VALUE,
                  NULL,
                  0,
                  Status
                  );
}
//
// Get device configuration
//
EFI_STATUS
UsbGetDeviceConfiguration (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  OUT UINT8                   *ConfigValue,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo       - TODO: add argument description
  ConfigValue - TODO: add argument description
  Status      - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;

  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiZeroMem (&DevReq, sizeof (EFI_USB_DEVICE_REQUEST));

  DevReq.RequestType  = USB_DEV_GET_CONFIGURATION_REQ_TYPE;
  DevReq.Request      = USB_DEV_GET_CONFIGURATION;
  DevReq.Value        = 0;
  DevReq.Index        = 0;
  DevReq.Length       = 1;

  return UsbIo->UsbControlTransfer (
                  UsbIo,
                  &DevReq,
                  EfiUsbDataIn,
                  TIMEOUT_VALUE,
                  ConfigValue,
                  1,
                  Status
                  );
}
//
// Set device configuration
//
EFI_STATUS
UsbSetDeviceConfiguration (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Value,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo   - TODO: add argument description
  Value   - TODO: add argument description
  Status  - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;
 
  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiZeroMem (&DevReq, sizeof (EFI_USB_DEVICE_REQUEST));

  DevReq.RequestType  = USB_DEV_SET_CONFIGURATION_REQ_TYPE;
  DevReq.Request      = USB_DEV_SET_CONFIGURATION;
  DevReq.Value        = Value;
  DevReq.Index        = 0;
  DevReq.Length       = 0;

  return UsbIo->UsbControlTransfer (
                  UsbIo,
                  &DevReq,
                  EfiUsbNoData,
                  TIMEOUT_VALUE,
                  NULL,
                  0,
                  Status
                  );
}
//
//  Set Device Feature
//
EFI_STATUS
UsbSetDeviceFeature (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  EFI_USB_RECIPIENT       Recipient,
  IN  UINT16                  Value,
  IN  UINT16                  Target,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo     - TODO: add argument description
  Recipient - TODO: add argument description
  Value     - TODO: add argument description
  Target    - TODO: add argument description
  Status    - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;

  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiZeroMem (&DevReq, sizeof (EFI_USB_DEVICE_REQUEST));

  switch (Recipient) {
  case EfiUsbDevice:
    DevReq.RequestType = 0x00;
    break;

  case EfiUsbInterface:
    DevReq.RequestType = 0x01;
    break;

  case EfiUsbEndpoint:
    DevReq.RequestType = 0x02;
    break;
  }

  //
  // Fill device request, see USB1.1 spec
  //
  DevReq.Request  = USB_DEV_SET_FEATURE;
  DevReq.Value    = Value;
  DevReq.Index    = Target;
  DevReq.Length   = 0;

  return UsbIo->UsbControlTransfer (
                  UsbIo,
                  &DevReq,
                  EfiUsbNoData,
                  TIMEOUT_VALUE,
                  NULL,
                  0,
                  Status
                  );
}

//
// Clear Device Feature
//
EFI_STATUS
UsbClearDeviceFeature (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  EFI_USB_RECIPIENT       Recipient,
  IN  UINT16                  Value,
  IN  UINT16                  Target,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo     - TODO: add argument description
  Recipient - TODO: add argument description
  Value     - TODO: add argument description
  Target    - TODO: add argument description
  Status    - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;

  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiZeroMem (&DevReq, sizeof (EFI_USB_DEVICE_REQUEST));

  switch (Recipient) {
  case EfiUsbDevice:
    DevReq.RequestType = 0x00;
    break;

  case EfiUsbInterface:
    DevReq.RequestType = 0x01;
    break;

  case EfiUsbEndpoint:
    DevReq.RequestType = 0x02;
    break;
  }

  //
  // Fill device request, see USB1.1 spec
  //
  DevReq.Request  = USB_DEV_CLEAR_FEATURE;
  DevReq.Value    = Value;
  DevReq.Index    = Target;
  DevReq.Length   = 0;

  return UsbIo->UsbControlTransfer (
                  UsbIo,
                  &DevReq,
                  EfiUsbNoData,
                  TIMEOUT_VALUE,
                  NULL,
                  0,
                  Status
                  );
}

//
//  Get Device Status
//
EFI_STATUS
UsbGetDeviceStatus (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  EFI_USB_RECIPIENT       Recipient,
  IN  UINT16                  Target,
  OUT UINT16                  *DevStatus,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo     - TODO: add argument description
  Recipient - TODO: add argument description
  Target    - TODO: add argument description
  DevStatus - TODO: add argument description
  Status    - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;

  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiZeroMem (&DevReq, sizeof (EFI_USB_DEVICE_REQUEST));

  switch (Recipient) {
  case EfiUsbDevice:
    DevReq.RequestType = 0x80;
    break;

  case EfiUsbInterface:
    DevReq.RequestType = 0x81;
    break;

  case EfiUsbEndpoint:
    DevReq.RequestType = 0x82;
    break;
  }

  //
  // Fill device request, see USB1.1 spec
  //
  DevReq.Request  = USB_DEV_GET_STATUS;
  DevReq.Value    = 0;
  DevReq.Index    = Target;
  DevReq.Length   = 2;

  return UsbIo->UsbControlTransfer (
                  UsbIo,
                  &DevReq,
                  EfiUsbDataIn,
                  TIMEOUT_VALUE,
                  DevStatus,
                  2,
                  Status
                  );
}

//
// Usb Get String
//
EFI_STATUS
UsbGetString (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  LangID,
  IN  UINT8                   Index,
  IN  VOID                    *Buf,
  IN  UINTN                   BufSize,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo   - TODO: add argument description
  LangID  - TODO: add argument description
  Index   - TODO: add argument description
  Buf     - TODO: add argument description
  BufSize - TODO: add argument description
  Status  - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value

--*/
{
  UINT16  Value;

  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Fill value, see USB1.1 spec
  //
  Value = (UINT16) ((USB_DT_STRING << 8) | Index);

  return UsbGetDescriptor (
          UsbIo,
          Value,
          LangID,
          (UINT16) BufSize,
          Buf,
          Status
          );
}

//
// Clear endpoint stall
//
EFI_STATUS
UsbClearEndpointHalt (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT8                   EndpointNo,
  OUT UINT32                  *Status
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbIo       - TODO: add argument description
  EndpointNo  - TODO: add argument description
  Status      - TODO: add argument description

Returns:

  EFI_NOT_FOUND - TODO: Add description for return value
  EFI_DEVICE_ERROR - TODO: Add description for return value
  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  EFI_STATUS                    Result;
  EFI_USB_ENDPOINT_DESCRIPTOR   EndpointDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   *DevEndpointDescriptor;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDescriptor;
  UINT8                         Index;

  EfiZeroMem (&EndpointDescriptor, sizeof (EFI_USB_ENDPOINT_DESCRIPTOR));
  //
  // First seach the endpoint descriptor for that endpoint addr
  //
  Result = UsbIo->UsbGetInterfaceDescriptor (
                    UsbIo,
                    &InterfaceDescriptor
                    );
  if (EFI_ERROR (Result)) {
    return Result;
  }

  for (Index = 0; Index < InterfaceDescriptor.NumEndpoints; Index++) {
    Result = UsbIo->UsbGetEndpointDescriptor (
                      UsbIo,
                      Index,
                      &EndpointDescriptor
                      );
    if (EFI_ERROR (Result)) {
      continue;
    }

    if (EndpointDescriptor.EndpointAddress == EndpointNo) {
      break;
    }
  }

  if (Index == InterfaceDescriptor.NumEndpoints) {
    //
    // No such endpoint
    //
    return EFI_NOT_FOUND;
  }

  Result = UsbClearDeviceFeature (
            UsbIo,
            EfiUsbEndpoint,
            EfiUsbEndpointHalt,
            EndpointDescriptor.EndpointAddress,
            Status
            );

  if (EFI_ERROR (Result)) {
    return Result;
  }

  //
  // Set the corresponding endpoint toggle to 0, see declaration of
  // ENDPOINT_DESC_LIST_ENTRY
  // How to implement???
  //
  DevEndpointDescriptor = GetEndpointDescriptor (
                            UsbIo,
                            EndpointNo
                            );
  if (DevEndpointDescriptor == NULL) {
    return EFI_DEVICE_ERROR;
  }

  *((UINT16 *) ((UINT8 *) DevEndpointDescriptor - 2)) = 0;

  return EFI_SUCCESS;
}

//
// Move from usbio.c
//
EFI_STATUS
UsbGetDeviceDescriptor (
  IN  EFI_USB_IO_PROTOCOL           *This,
  OUT EFI_USB_DEVICE_DESCRIPTOR     **DeviceDescriptor
  )
/*++

  Routine Description:
    Get Usb Device Descriptor

  Arguments:
    This                -   Calling Context
    DeviceDescriptor    -   Pointer to the device descriptor stored in the
                            Controller device.

  Return Value:
    EFI_NOT_FOUND
    EFI_SUCCESS

--*/
// TODO: function comment is missing 'Returns:'
// TODO:    EFI_NOT_FOUND - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  USB_IO_DEVICE             *UsbIoDev;
  USB_IO_CONTROLLER_DEVICE  *UsbIoController;

  UsbIoController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS (This);
  UsbIoDev        = UsbIoController->UsbDevice;

  if (!UsbIoDev->IsConfigured) {
    return EFI_NOT_FOUND;
  }

  //
  // The device descriptor is in the UsbIoDev structure if it is
  // correctly configured.
  //
  *DeviceDescriptor = &UsbIoDev->DeviceDescriptor;

  return EFI_SUCCESS;
}

EFI_STATUS
UsbGetActiveConfigDescriptor (
  IN  EFI_USB_IO_PROTOCOL           *This,
  OUT EFI_USB_CONFIG_DESCRIPTOR     **ConfigurationDescriptor
  )
/*++

  Routine Description:
    Get Usb Active Configuration Descriptor

  Arguments:
    This                      -   Calling Context
    ConfigurationDescriptor   -   Pointer to the Configuration descriptor
                                  stored in the Controller device.

  Returns:
    EFI_SUCCESS
    EFI_NOT_FOUND

--*/
{
  USB_IO_DEVICE             *UsbIoDev;
  USB_IO_CONTROLLER_DEVICE  *UsbIoController;

  UsbIoController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS (This);
  UsbIoDev        = UsbIoController->UsbDevice;

  if (!UsbIoDev->IsConfigured) {
    return EFI_NOT_FOUND;
  }

  //
  // The active configuration descriptor is in the UsbIoDev structure
  // if it is correctly configured.
  //
  *ConfigurationDescriptor = &(UsbIoDev->ActiveConfig->CongfigDescriptor);

  return EFI_SUCCESS;
}

EFI_STATUS
UsbGetInterfaceDescriptor (
  IN  EFI_USB_IO_PROTOCOL              *This,
  OUT EFI_USB_INTERFACE_DESCRIPTOR     **InterfaceDescriptor
  )
/*++

  Routine Description:
    Get Usb Controller Interface Descriptor

  Arguments:
    This                  -   Calling Context
    InterfaceDescriptor   -   Pointer to the Interface descriptor
                              stored in the Controller device.

  Returns:
    EFI_SUCCESS
    EFI_NOT_FOUND

--*/
{
  USB_IO_DEVICE             *UsbIoDev;
  EFI_LIST_ENTRY            *InterfaceListHead;
  CONFIG_DESC_LIST_ENTRY    *ConfigListEntry;
  INTERFACE_DESC_LIST_ENTRY *InterfaceListEntry;
  EFI_USB_CONFIG_DESCRIPTOR *ConfigurationDescriptor;
  EFI_STATUS                Status;
  USB_IO_CONTROLLER_DEVICE  *UsbIoController;
  UINT8                     InterfaceNum;

  //
  // First get the active configuration descriptor
  //
  Status = UsbGetActiveConfigDescriptor (
            This,
            &ConfigurationDescriptor
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  UsbIoController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS (This);
  UsbIoDev        = UsbIoController->UsbDevice;

  if (!UsbIoDev->IsConfigured) {
    return EFI_NOT_FOUND;
  }

  InterfaceNum        = UsbIoController->InterfaceNumber;

  ConfigListEntry     = (CONFIG_DESC_LIST_ENTRY *) ((UINT8 *) ConfigurationDescriptor - sizeof (EFI_LIST_ENTRY));

  InterfaceListHead   = (EFI_LIST_ENTRY *) (&ConfigListEntry->InterfaceDescListHead);
  InterfaceListEntry  = (INTERFACE_DESC_LIST_ENTRY *) (InterfaceListHead->ForwardLink);

  //
  // Loop all interface descriptor to get match one.
  //
  while (InterfaceListEntry != (INTERFACE_DESC_LIST_ENTRY *) InterfaceListHead) {
    if ((InterfaceListEntry->InterfaceDescriptor).InterfaceNumber == InterfaceNum) {
      *InterfaceDescriptor = &InterfaceListEntry->InterfaceDescriptor;
      return EFI_SUCCESS;
    }

    InterfaceListEntry = (INTERFACE_DESC_LIST_ENTRY *) ((EFI_LIST_ENTRY *) InterfaceListEntry)->ForwardLink;
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
UsbGetEndpointDescriptor (
  IN  EFI_USB_IO_PROTOCOL             *This,
  IN  UINT8                           EndpointIndex,
  OUT EFI_USB_ENDPOINT_DESCRIPTOR     **EndpointDescriptor
  )
/*++

  Routine Description:
    Get a given endpoint descriptor

  Arguments:
    This                  -   Calling Context
    EndpointIndex         -   Zero-based index within current interface.
    EndpointDescriptor    -   Pointer to the Endpoint descriptor
                              stored in the Controller device.

  Returns:
    EFI_SUCCESS
    EFI_NOT_FOUND
    EFI_INVALID_PARAMETER

--*/
{
  USB_IO_DEVICE                 *UsbIoDev;
  EFI_LIST_ENTRY                *EndpointListHead;
  INTERFACE_DESC_LIST_ENTRY     *InterfaceListEntry;
  ENDPOINT_DESC_LIST_ENTRY      *EndpointListEntry;
  USB_IO_CONTROLLER_DEVICE      *UsbIoController;
  EFI_STATUS                    Status;
  EFI_USB_INTERFACE_DESCRIPTOR  *InterfaceDescriptor;

  UsbIoController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS (This);
  UsbIoDev        = UsbIoController->UsbDevice;

  if (!UsbIoDev->IsConfigured) {
    return EFI_NOT_FOUND;
  }

  //
  // First get the interface descriptor
  //
  Status = UsbGetInterfaceDescriptor (
            This,
            &InterfaceDescriptor
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (EndpointIndex >= InterfaceDescriptor->NumEndpoints) {
    return EFI_INVALID_PARAMETER;
  }

  InterfaceListEntry  = (INTERFACE_DESC_LIST_ENTRY *) ((UINT8 *) InterfaceDescriptor - sizeof (EFI_LIST_ENTRY));

  EndpointListHead    = (EFI_LIST_ENTRY *) (&InterfaceListEntry->EndpointDescListHead);
  EndpointListEntry   = (ENDPOINT_DESC_LIST_ENTRY *) (EndpointListHead->ForwardLink);

  //
  // Loop all endpoint descriptor to get match one.
  //
  while (EndpointIndex != 0) {
    EndpointListEntry = (ENDPOINT_DESC_LIST_ENTRY *) ((EndpointListEntry->Link).ForwardLink);
    EndpointIndex--;
  }

  *EndpointDescriptor = &EndpointListEntry->EndpointDescriptor;

  return EFI_SUCCESS;
}

EFI_USB_ENDPOINT_DESCRIPTOR *
GetEndpointDescriptor (
  IN EFI_USB_IO_PROTOCOL     *UsbIo,
  IN UINT8                   EndpointAddr
  )
/*++

  Routine Description:
    Get the endpoint descriptor according to the endpoint address.

  Parameters:
    UsbIo         -     EFI_USB_IO_PROTOCOL instance.
    EndpointAddr  -     Given Endpoint address.

  Return Value:

++*/
// TODO: function comment should end with '--*/'
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    UsbIo - add argument and description to function comment
// TODO:    EndpointAddr - add argument and description to function comment
{
  EFI_USB_INTERFACE_DESCRIPTOR  *InterfaceDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR   *EndpointDescriptor;
  UINT8                         Index;
  EFI_STATUS                    Status;

  //
  // First get the interface descriptor
  //
  Status = UsbGetInterfaceDescriptor (
            UsbIo,
            &InterfaceDescriptor
            );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Loop all endpoint descriptor to get match one.
  //
  for (Index = 0; Index < InterfaceDescriptor->NumEndpoints; Index++) {
    Status = UsbGetEndpointDescriptor (
              UsbIo,
              Index,
              &EndpointDescriptor
              );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (EndpointDescriptor->EndpointAddress == EndpointAddr) {
      return EndpointDescriptor;
    }
  }

  return NULL;
}

VOID
GetDeviceEndPointMaxPacketLength (
  IN  USB_IO_CONTROLLER_DEVICE     *UsbIoController,
  IN  UINT8                        EndpointAddr,
  OUT UINT8                        *MaxPacketLength
  )
/*++

  Routine Description:
    Get the Max Packet Length of the speified Endpoint.

  Parameters:
    UsbIoController   -     Given Usb Controller device.
    EndpointAddr      -     Given Endpoint address.
    MaxPacketLength   -     The max packet length of that endpoint

  Return Value:
    N/A

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    UsbIoController - add argument and description to function comment
// TODO:    EndpointAddr - add argument and description to function comment
// TODO:    MaxPacketLength - add argument and description to function comment
{
  EFI_USB_IO_PROTOCOL         *UsbIo;
  EFI_USB_ENDPOINT_DESCRIPTOR *EndpointDesc;

  *MaxPacketLength  = 0;
  UsbIo             = &UsbIoController->UsbIo;

  //
  // Get the endpoint descriptor
  //
  EndpointDesc = GetEndpointDescriptor (UsbIo, EndpointAddr);
  if (EndpointDesc == NULL) {
    return ;
  }

  *MaxPacketLength = (UINT8) (EndpointDesc->MaxPacketSize);

  return ;
}

VOID
GetDataToggleBit (
  IN  USB_IO_CONTROLLER_DEVICE     *UsbIoController,
  IN  UINT8                        EndpointAddr,
  OUT UINT8                        *DataToggle
  )
/*++

  Routine Description:
    Get the datatoggle of a specified endpoint.

  Parameters:
    UsbIoController   -     Given Usb Controller device.
    EndpointAddr      -     Given Endpoint address.
    DataToggle        -     The current data toggle of that endpoint

  Return Value:
    N/A

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    UsbIoController - add argument and description to function comment
// TODO:    EndpointAddr - add argument and description to function comment
// TODO:    DataToggle - add argument and description to function comment
{
  EFI_USB_IO_PROTOCOL         *UsbIo;
  EFI_USB_ENDPOINT_DESCRIPTOR *EndpointDesc;
  ENDPOINT_DESC_LIST_ENTRY    *EndpointListEntry;

  *DataToggle = 0;
  UsbIo       = &UsbIoController->UsbIo;

  //
  // Get the endpoint descriptor
  //
  EndpointDesc = GetEndpointDescriptor (UsbIo, EndpointAddr);
  if (EndpointDesc == NULL) {
    return ;
  }

  EndpointListEntry =
  (ENDPOINT_DESC_LIST_ENTRY *) ((UINT8 *) EndpointDesc - sizeof (UINT16) - sizeof (EFI_LIST_ENTRY));

  *DataToggle       = (UINT8) (EndpointListEntry->Toggle);
  return ;
}

VOID
SetDataToggleBit (
  IN USB_IO_CONTROLLER_DEVICE     *UsbIoController,
  IN UINT8                        EndpointAddr,
  IN UINT8                        DataToggle
  )
/*++

  Routine Description:
    Set the datatoggle of a specified endpoint

  Parameters:
    UsbIoController   -     Given Usb Controller device.
    EndpointAddr      -     Given Endpoint address.
    DataToggle        -     The current data toggle of that endpoint to be set

  Return Value:
    N/A

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    UsbIoController - add argument and description to function comment
// TODO:    EndpointAddr - add argument and description to function comment
// TODO:    DataToggle - add argument and description to function comment
{
  EFI_USB_IO_PROTOCOL         *UsbIo;
  EFI_USB_ENDPOINT_DESCRIPTOR *EndpointDesc;
  ENDPOINT_DESC_LIST_ENTRY    *EndpointListEntry;

  UsbIo = &UsbIoController->UsbIo;

  //
  // Get the endpoint descriptor
  //
  EndpointDesc = GetEndpointDescriptor (UsbIo, EndpointAddr);
  if (EndpointDesc == NULL) {
    return ;
  }

  EndpointListEntry         =
  (ENDPOINT_DESC_LIST_ENTRY *) ((UINT8 *) EndpointDesc - sizeof (UINT16) - sizeof (EFI_LIST_ENTRY));

  EndpointListEntry->Toggle = DataToggle;
  return ;
}
