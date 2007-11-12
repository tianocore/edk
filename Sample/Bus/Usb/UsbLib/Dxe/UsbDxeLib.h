/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

 Module Name:

    UsbDxeLib.h

 Abstract:

   Common Dxe Libarary  for USB

 Revision History

--*/

#ifndef _USB_DXE_LIB_H
#define _USB_DXE_LIB_H

#include "Tiano.h"
#include "EfiDriverLib.h"

#include EFI_PROTOCOL_DEFINITION (UsbIo)

//
// Standard device request and request type
// By [Spec-USB20/Chapter-9.4]
//
#define USB_DEV_GET_STATUS                  0x00
#define USB_DEV_GET_STATUS_REQ_TYPE_D       0x80 // Receiver : Device
#define USB_DEV_GET_STATUS_REQ_TYPE_I       0x81 // Receiver : Interface
#define USB_DEV_GET_STATUS_REQ_TYPE_E       0x82 // Receiver : Endpoint

#define USB_DEV_CLEAR_FEATURE               0x01
#define USB_DEV_CLEAR_FEATURE_REQ_TYPE_D    0x00 // Receiver : Device
#define USB_DEV_CLEAR_FEATURE_REQ_TYPE_I    0x01 // Receiver : Interface
#define USB_DEV_CLEAR_FEATURE_REQ_TYPE_E    0x02 // Receiver : Endpoint

#define USB_DEV_SET_FEATURE                 0x03
#define USB_DEV_SET_FEATURE_REQ_TYPE_D      0x00 // Receiver : Device
#define USB_DEV_SET_FEATURE_REQ_TYPE_I      0x01 // Receiver : Interface
#define USB_DEV_SET_FEATURE_REQ_TYPE_E      0x02 // Receiver : Endpoint
 
#define USB_DEV_SET_ADDRESS                 0x05
#define USB_DEV_SET_ADDRESS_REQ_TYPE        0x00

#define USB_DEV_GET_DESCRIPTOR              0x06
#define USB_DEV_GET_DESCRIPTOR_REQ_TYPE     0x80

#define USB_DEV_SET_DESCRIPTOR              0x07
#define USB_DEV_SET_DESCRIPTOR_REQ_TYPE     0x00

#define USB_DEV_GET_CONFIGURATION           0x08
#define USB_DEV_GET_CONFIGURATION_REQ_TYPE  0x80

#define USB_DEV_SET_CONFIGURATION           0x09
#define USB_DEV_SET_CONFIGURATION_REQ_TYPE  0x00

#define USB_DEV_GET_INTERFACE               0x0A
#define USB_DEV_GET_INTERFACE_REQ_TYPE      0x81

#define USB_DEV_SET_INTERFACE               0x0B
#define USB_DEV_SET_INTERFACE_REQ_TYPE      0x01

#define USB_DEV_SYNCH_FRAME                 0x0C
#define USB_DEV_SYNCH_FRAME_REQ_TYPE        0x82

#define TIMEOUT_VALUE 3 * 1000


EFI_STATUS
UsbGetDescriptor (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Value,
  IN  UINT16                  Index,
  IN  UINT16                  DescriptorLength,
  OUT VOID                    *Descriptor,
  OUT UINT32                  *Status
  );

EFI_STATUS
UsbSetDescriptor (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Value,
  IN  UINT16                  Index,
  IN  UINT16                  DescriptorLength,
  IN  VOID                    *Descriptor,
  OUT UINT32                  *Status
  );

EFI_STATUS
UsbGetInterface (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Index,
  OUT UINT8                   *AltSetting,
  OUT UINT32                  *Status
  );

EFI_STATUS
UsbSetInterface (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  InterfaceNo,
  IN  UINT16                  AltSetting,
  OUT UINT32                  *Status
  );

EFI_STATUS
UsbGetConfiguration (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  OUT UINT8                   *ConfigValue,
  OUT UINT32                  *Status
  );

EFI_STATUS
UsbSetConfiguration (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Value,
  OUT UINT32                  *Status
  );

EFI_STATUS
UsbSetFeature (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINTN       Recipient,
  IN  UINT16                  Value,
  IN  UINT16                  Target,
  OUT UINT32                  *Status
  );

EFI_STATUS
UsbClearFeature (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINTN       Recipient,
  IN  UINT16                  Value,
  IN  UINT16                  Target,
  OUT UINT32                  *Status
  );

EFI_STATUS
UsbGetStatus (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINTN       Recipient,
  IN  UINT16                  Target,
  OUT UINT16                  *DevStatus,
  OUT UINT32                  *Status
  );

EFI_STATUS
UsbGetHubDescriptor (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Value,
  IN  UINT16                  Index,
  IN  UINT16                  DescriptorLength,
  OUT VOID                    *Descriptor,
  OUT UINT32                  *Status
  );

//
// Clear endpoint stall
//
EFI_STATUS
UsbClearEndpointHalt (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT8                   EndpointNo,
  OUT UINT32                  *Status
  );

#endif
