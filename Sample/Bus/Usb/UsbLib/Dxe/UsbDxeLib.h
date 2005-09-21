/*++

Copyright (c) 2004, Intel Corporation                                                         
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
// define the timeout time as 3ms
//
#define TIMEOUT_VALUE 3 * 1000

//
// Get Device Descriptor
//
EFI_STATUS
UsbGetDescriptor (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Value,
  IN  UINT16                  Index,
  IN  UINT16                  DescriptorLength,
  OUT VOID                    *Descriptor,
  OUT UINT32                  *Status
  );

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
  );

//
// Get device Interface
//
EFI_STATUS
UsbGetDeviceInterface (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Index,
  OUT UINT8                   *AltSetting,
  OUT UINT32                  *Status
  );

//
// Set device interface
//
EFI_STATUS
UsbSetDeviceInterface (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  InterfaceNo,
  IN  UINT16                  AltSetting,
  OUT UINT32                  *Status
  );

//
// Get device configuration
//
EFI_STATUS
UsbGetDeviceConfiguration (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  OUT UINT8                   *ConfigValue,
  OUT UINT32                  *Status
  );

//
// Set device configuration
//
EFI_STATUS
UsbSetDeviceConfiguration (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT16                  Value,
  OUT UINT32                  *Status
  );

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
  );

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
  );

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
  );

//
// The following APIs are not basic library, but they are common used.
//
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
