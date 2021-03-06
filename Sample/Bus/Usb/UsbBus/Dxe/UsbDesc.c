/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:

    UsbDesc.c

  Abstract:

    Manage Usb Descriptor List

  Revision History

--*/

#include "UsbBus.h"

STATIC
VOID
UsbFreeInterfaceDesc (
  IN USB_INTERFACE_SETTING  *Setting
  )
/*++

Routine Description:

  Free the interface setting descriptor

Arguments:

  Setting - The descriptor to free

Returns:

  None

--*/
{
  USB_ENDPOINT_DESC       *Ep;
  UINTN                   Index;

  if (Setting->Endpoints != NULL) {
    //
    // Each interface setting may have several endpoints, free them first.
    //
    for (Index = 0; Index < Setting->Desc.NumEndpoints; Index++) {
      Ep = Setting->Endpoints[Index];

      if (Ep != NULL) {
        gBS->FreePool (Ep);
      }
    }

    gBS->FreePool (Setting->Endpoints);
  }

  gBS->FreePool (Setting);
}


STATIC
VOID
UsbFreeConfigDesc (
  IN USB_CONFIG_DESC      *Config
  )
/*++

Routine Description:

  Free a configuration descriptor with its interface
  descriptors. It may be initialized partially

Arguments:

  Config  - The configuration descriptor to free

Returns:

  None

--*/
{
  USB_INTERFACE_DESC      *Interface;
  UINTN                   Index;
  UINTN                   SetIndex;

  if (Config->Interfaces != NULL) {
    //
    // A configuration may have several interfaces, free the interface
    //
    for (Index = 0; Index < Config->Desc.NumInterfaces; Index++) {
      Interface = Config->Interfaces[Index];

      if (Interface == NULL) {
        continue;
      }

      //
      // Each interface may have several settings, free the settings
      //
      for (SetIndex = 0; SetIndex < Interface->NumOfSetting; SetIndex++) {
        if (Interface->Settings[SetIndex] != NULL) {
          UsbFreeInterfaceDesc (Interface->Settings[SetIndex]);
        }
      }

      gBS->FreePool (Interface);
    }

    gBS->FreePool (Config->Interfaces);
  }

  gBS->FreePool (Config);

}


VOID
UsbFreeDevDesc (
  IN USB_DEVICE_DESC      *DevDesc
  )
/*++

Routine Description:

  Free a device descriptor with its configurations

Arguments:

  DevDesc - The device descriptor

Returns:

  None

--*/
{
  UINTN                   Index;

  if (DevDesc->Configs != NULL) {
    for (Index = 0; Index < DevDesc->Desc.NumConfigurations; Index++) {
      if (DevDesc->Configs[Index] != NULL) {
        UsbFreeConfigDesc (DevDesc->Configs[Index]);
      }
    }

    gBS->FreePool (DevDesc->Configs);
  }

  gBS->FreePool (DevDesc);
}

STATIC
VOID *
UsbCreateDesc (
  IN  UINT8               *DescBuf,
  IN  INTN                Len,
  IN  UINT8               Type,
  OUT INTN                *Consumed
  )
/*++

Routine Description:

  Create a descriptor

Arguments:

  DescBuf  - The buffer of raw descriptor 
  Len      - The lenght of the raw descriptor buffer
  Type     - The type of descriptor to create
  Consumed - Number of bytes consumed
  
Returns:

  Created descriptor or NULL

--*/
{
  USB_DESC_HEAD           *Head;
  INTN                    DescLen;
  INTN                    CtrlLen;
  INTN                    Offset;
  VOID                    *Desc;

  DescLen   = 0;
  CtrlLen   = 0;
  *Consumed = 0;
  
  switch (Type) {
  case USB_DESC_TYPE_DEVICE:
    DescLen = sizeof (EFI_USB_DEVICE_DESCRIPTOR);
    CtrlLen = sizeof (USB_DEVICE_DESC);
    break;

  case USB_DESC_TYPE_CONFIG:
    DescLen = sizeof (EFI_USB_CONFIG_DESCRIPTOR);
    CtrlLen = sizeof (USB_CONFIG_DESC);
    break;

  case USB_DESC_TYPE_INTERFACE:
    DescLen = sizeof (EFI_USB_INTERFACE_DESCRIPTOR);
    CtrlLen = sizeof (USB_INTERFACE_SETTING);
    break;

  case USB_DESC_TYPE_ENDPOINT:
    DescLen = sizeof (EFI_USB_ENDPOINT_DESCRIPTOR);
    CtrlLen = sizeof (USB_ENDPOINT_DESC);
    break;
  }

  //
  // All the descriptor has a common LTV (Length, Type, Value)
  // format. Skip the descriptor that isn't of this Type
  //
  Offset = 0;
  Head   = (USB_DESC_HEAD*)DescBuf;
  
  while ((Offset < Len) && (Head->Type != Type)) {
    Offset += Head->Len;
    Head    = (USB_DESC_HEAD*)(DescBuf + Offset);
  }
  
  if ((Len <= Offset)      || (Len < Offset + DescLen) ||
      (Head->Type != Type) || (Head->Len != DescLen)) {   
    USB_ERROR (("UsbCreateDesc: met mal-format descriptor\n"));   
    return NULL;
  }

  Desc = EfiLibAllocateZeroPool (CtrlLen);

  if (Desc == NULL) {
    return NULL;
  }

  EfiCopyMem (Desc, Head, DescLen);
  *Consumed = Offset + Head->Len;
  
  return Desc;
}

STATIC
USB_INTERFACE_SETTING *
UsbParseInterfaceDesc (
  IN  UINT8               *DescBuf,
  IN  INTN                Len,
  OUT INTN                *Consumed
  )
/*++

Routine Description:

  Parse an interface desciptor and its endpoints

Arguments:

  DescBuf   - The buffer of raw descriptor 
  Len       - The lenght of the raw descriptor buffer
  Consumed  - The number of raw descriptor consumed

Returns:

  The create interface setting or NULL if failed

--*/
{
  USB_INTERFACE_SETTING   *Setting;
  USB_ENDPOINT_DESC       *Ep;
  UINTN                   Index;
  UINTN                   NumEp;
  INTN                    Used;
  INTN                    Offset;
  
  *Consumed = 0;
  Setting   = UsbCreateDesc (DescBuf, Len, USB_DESC_TYPE_INTERFACE, &Used);

  if (Setting == NULL) {
    USB_ERROR (("UsbParseInterfaceDesc: failed to create interface descriptor\n"));
    return NULL;
  }

  Offset = Used;

  //
  // Create an arry to hold the interface's endpoints
  //
  NumEp  = Setting->Desc.NumEndpoints;

  USB_DEBUG (("UsbParseInterfaceDesc: interface %d(setting %d) has %d endpoints\n",
              Setting->Desc.InterfaceNumber, Setting->Desc.AlternateSetting, NumEp));

  if (NumEp == 0) {
    goto ON_EXIT;
  }
  
  Setting->Endpoints  = EfiLibAllocateZeroPool (sizeof (USB_ENDPOINT_DESC *) * NumEp);

  if (Setting->Endpoints == NULL) {
    goto ON_ERROR;
  }

  //
  // Create the endpoints for this interface
  //
  for (Index = 0; Index < NumEp; Index++) {
    Ep = UsbCreateDesc (DescBuf + Offset, Len - Offset, USB_DESC_TYPE_ENDPOINT, &Used);

    if (Ep == NULL) {
      USB_ERROR (("UsbParseInterfaceDesc: failed to create endpoint(index %d)\n", Index));
      goto ON_ERROR;
    }

    Setting->Endpoints[Index]  = Ep;
    Offset                    += Used;
  }


ON_EXIT:
  *Consumed = Offset;
  return Setting;

ON_ERROR:
  UsbFreeInterfaceDesc (Setting);
  return NULL;
}

STATIC
USB_CONFIG_DESC *
UsbParseConfigDesc (
  IN UINT8                *DescBuf,
  IN INTN                 Len
  )
/*++

Routine Description:

  Parse the configuration descriptor and its interfaces.

Arguments:

  DescBuf   - The buffer of raw descriptor 
  Len       - The lenght of the raw descriptor buffer

Returns:

  The created configuration descriptor

--*/
{
  USB_CONFIG_DESC         *Config;
  USB_INTERFACE_SETTING   *Setting;
  USB_INTERFACE_DESC      *Interface;
  UINTN                   Index;
  UINTN                   NumIf;
  INTN                    Consumed;

  ASSERT (DescBuf != NULL);

  Config = UsbCreateDesc (DescBuf, Len, USB_DESC_TYPE_CONFIG, &Consumed);

  if (Config == NULL) {
    return NULL;
  }
  
  //
  // Initialize an array of setting for the configuration's interfaces.
  //
  NumIf               = Config->Desc.NumInterfaces;
  Config->Interfaces  = EfiLibAllocateZeroPool (sizeof (USB_INTERFACE_DESC *) * NumIf);

  if (Config->Interfaces == NULL) {
    goto ON_ERROR;
  }

  USB_DEBUG (("UsbParseConfigDesc: config %d has %d interfaces\n", 
                Config->Desc.ConfigurationValue, NumIf));
  
  for (Index = 0; Index < NumIf; Index++) {
    Interface = EfiLibAllocateZeroPool (sizeof (USB_INTERFACE_DESC));

    if (Interface == NULL) {
      goto ON_ERROR;
    }

    Config->Interfaces[Index] = Interface;
  }
  
  //
  // If a configuration has several interfaces, these interfaces are
  // numbered from zero to n. If a interface has several settings,
  // these settings are also number from zero to m. The interface
  // setting must be organized as |interface 0, setting 0|interface 0
  // setting 1|interface 1, setting 0|interface 2, setting 0|. Check
  // USB2.0 spec, page 267.
  //
  DescBuf += Consumed;
  Len     -= Consumed;

  while (Len > 0) {
    Setting = UsbParseInterfaceDesc (DescBuf, Len, &Consumed);

    if ((Setting == NULL)) {
      USB_ERROR (("UsbParseConfigDesc: failed to parse interface setting\n"));
      goto ON_ERROR;
      
    } else if (Setting->Desc.InterfaceNumber >= NumIf) {
      USB_ERROR (("UsbParseConfigDesc: mal-formated interface descriptor\n"));

      UsbFreeInterfaceDesc (Setting);
      goto ON_ERROR;
    }
    
    //
    // Insert the descriptor to the corresponding set.
    //
    Interface = Config->Interfaces[Setting->Desc.InterfaceNumber];

    if (Interface->NumOfSetting >= USB_MAX_INTERFACE_SETTING) {
      goto ON_ERROR;
    }

    Interface->Settings[Interface->NumOfSetting] = Setting;
    Interface->NumOfSetting++;

    DescBuf += Consumed;
    Len     -= Consumed;
  }

  return Config;

ON_ERROR:
  UsbFreeConfigDesc (Config);
  return NULL;
}


EFI_STATUS
UsbCtrlRequest (
  IN USB_DEVICE             *UsbDev,
  IN EFI_USB_DATA_DIRECTION Direction,
  IN UINTN                  Type, 
  IN UINTN                  Target,
  IN UINTN                  Request,
  IN UINT16                 Value,
  IN UINT16                 Index,
  IN OUT VOID               *Buf,
  IN UINTN                  Length
  )
/*++

Routine Description:

  USB standard control transfer support routine. This
  function is used by USB device. It is possible that
  the device's interfaces are still waiting to be 
  enumerated.

Arguments:

  UsbDev    - The usb device 
  Direction - The direction of data transfer
  Type      - Standard / class specific / vendor specific
  Target    - The receiving target
  Request   - Which request
  Value     - The wValue parameter of the request
  Index     - The wIndex parameter of the request
  Buf       - The buffer to receive data into / transmit from
  Length    - The length of the buffer

Returns:

  EFI_SUCCESS      - The control request is executed
  EFI_DEVICE_ERROR - Failed to execute the control transfer

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;
  EFI_STATUS              Status;
  UINT32                  Result;
  UINTN                   Len;

  ASSERT ((UsbDev != NULL) && (UsbDev->Bus != NULL));

  DevReq.RequestType  = USB_REQUEST_TYPE (Direction, Type, Target);
  DevReq.Request      = (UINT8) Request;
  DevReq.Value        = Value;
  DevReq.Index        = Index;
  DevReq.Length       = (UINT16) Length;

  Len                 = Length;
  Status = UsbHcControlTransfer (
             UsbDev->Bus,
             UsbDev->Address,
             UsbDev->Speed,
             UsbDev->MaxPacket0,
             &DevReq,
             Direction,
             Buf,
             &Len,
             USB_GENERAL_DEVICE_REQUEST_TIMEOUT,
             &UsbDev->Translator,
             &Result
             );

  return Status;
}


STATIC
EFI_STATUS
UsbCtrlGetDesc (
  IN  USB_DEVICE          *UsbDev,
  IN  UINTN               DescType,
  IN  UINTN               DescIndex,
  IN  UINT16              LangId, 
  OUT VOID                *Buf,
  IN  UINTN               Length
  )
/*++

Routine Description:

  Get the standard descriptors. 

Arguments:

  UsbDev    - The USB device to read descriptor from
  DescType  - The type of descriptor to read
  DescIndex - The index of descriptor to read
  LangId    - Language ID, only used to get string, otherwise set it to 0
  Buf       - The buffer to hold the descriptor read
  Length    - The length of the buffer

Returns:

  EFI_SUCCESS - The descriptor is read OK
  Others      - Failed to retrieve the descriptor

--*/
{
  EFI_STATUS              Status;

  Status = UsbCtrlRequest (
             UsbDev,
             EfiUsbDataIn,
             USB_REQ_TYPE_STANDARD,
             USB_TARGET_DEVICE,
             USB_REQ_GET_DESCRIPTOR,
             (UINT16) ((DescType << 8) | DescIndex),
             LangId,
             Buf,
             Length
             );

  return Status;
}


EFI_STATUS
UsbGetMaxPacketSize0 (
  IN USB_DEVICE           *UsbDev
  )
/*++

Routine Description:

  Return the max packet size for endpoint zero. This function
  is the first function called to get descriptors during bus
  enumeration.

Arguments:

  UsbDev  - The usb device 

Returns:

  EFI_SUCCESS       - The max packet size of endpoint zero is retrieved
  EFI_DEVICE_ERROR  - Failed to retrieve it
  
--*/
{
  EFI_USB_DEVICE_DESCRIPTOR DevDesc;
  EFI_STATUS                Status;
  UINTN                     Index;

  
  //
  // Get the first 8 bytes of the device descriptor which contains
  // max packet size for endpoint 0, which is at least 8.
  //
  UsbDev->MaxPacket0 = 8;
 
  for (Index = 0; Index < 3; Index++) {
    Status = UsbCtrlGetDesc (UsbDev, USB_DESC_TYPE_DEVICE, 0, 0, &DevDesc, 8);

    if (!EFI_ERROR (Status)) {
      UsbDev->MaxPacket0 = DevDesc.MaxPacketSize0;
      return EFI_SUCCESS;
    }

    gBS->Stall (USB_RETRY_MAX_PACK_SIZE_STALL);
  }
  
  return EFI_DEVICE_ERROR;
}


STATIC
EFI_STATUS
UsbGetDevDesc (
  IN USB_DEVICE           *UsbDev
  )
/*++

Routine Description:

  Get the device descriptor for the device.

Arguments:

  UsbDev  - The Usb device to retrieve descriptor from

Returns:

  EFI_SUCCESS          - The device descriptor is returned
  EFI_OUT_OF_RESOURCES - Failed to allocate memory

--*/
{
  USB_DEVICE_DESC         *DevDesc;
  EFI_STATUS              Status;

  DevDesc = EfiLibAllocateZeroPool (sizeof (USB_DEVICE_DESC));

  if (DevDesc == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status  = UsbCtrlGetDesc (
              UsbDev, 
              USB_DESC_TYPE_DEVICE, 
              0, 
              0, 
              DevDesc, 
              sizeof (EFI_USB_DEVICE_DESCRIPTOR)
              );

  if (EFI_ERROR (Status)) {
    gBS->FreePool (DevDesc);
  } else {
    UsbDev->DevDesc = DevDesc;
  }

  return Status;
}


EFI_USB_STRING_DESCRIPTOR *
UsbGetOneString (
  IN     USB_DEVICE       *UsbDev,
  IN     UINT8            Index,
  IN     UINT16           LangId  
  )
/*++

Routine Description:

  Retrieve the indexed string for the language. It requires two
  steps to get a string, first to get the string's length. Then
  the string itself.

Arguments:

  UsbDev      - The usb device
  Index       - The index the string to retrieve
  LangId      - Language ID

Returns:

  The created string descriptor or NULL

--*/
{
  EFI_USB_STRING_DESCRIPTOR Desc;
  EFI_STATUS                Status;
  UINT8                     *Buf;

  //
  // First get two bytes which contains the string length.
  //
  Status = UsbCtrlGetDesc (UsbDev, USB_DESC_TYPE_STRING, Index, LangId, &Desc, 2);

  if (EFI_ERROR (Status)) {
    return NULL;
  }
  
  Buf = EfiLibAllocateZeroPool (Desc.Length);

  if (Buf == NULL) {
    return NULL;
  }
  
  Status = UsbCtrlGetDesc (
             UsbDev, 
             USB_DESC_TYPE_STRING, 
             Index, 
             LangId, 
             Buf, 
             Desc.Length
             );

  if (EFI_ERROR (Status)) {
    gBS->FreePool (Buf);
    return NULL;
  }

  return (EFI_USB_STRING_DESCRIPTOR *) Buf;
}


STATIC
EFI_STATUS
UsbBuildLangTable (
  IN USB_DEVICE           *UsbDev
  )
/*++

Routine Description:

  Build the language ID table for string descriptors

Arguments:

  UsbDev  - The Usb device

Returns:

  EFI_UNSUPPORTED - This device doesn't support string table

--*/
{
  EFI_USB_STRING_DESCRIPTOR *Desc;
  EFI_STATUS                Status;
  UINTN                     Index;
  UINTN                     Max;
  UINT16                    *Point;

  //
  // The string of language ID zero returns the supported languages
  //
  Desc = UsbGetOneString (UsbDev, 0, 0);

  if (Desc == NULL) {
    return EFI_UNSUPPORTED;
  }

  if (Desc->Length < 4) {
    Status = EFI_UNSUPPORTED;
    goto ON_EXIT;
  }

  Status = EFI_SUCCESS;

  Max   = (Desc->Length - 2) / 2;
  Max   = (Max < USB_MAX_LANG_ID ? Max : USB_MAX_LANG_ID);

  Point = Desc->String;
  for (Index = 0; Index < Max; Index++) {
    UsbDev->LangId[Index] = *Point;
    Point++;
  }

  UsbDev->TotalLangId = (UINT16)Max;

ON_EXIT:
  gBS->FreePool (Desc);
  return Status;
}


STATIC
EFI_USB_CONFIG_DESCRIPTOR *
UsbGetOneConfig (
  IN USB_DEVICE           *UsbDev,
  IN UINT8                Index
  )
/*++

Routine Description:

  Retrieve the indexed configure for the device. USB device
  returns the configuration togetther with the interfaces for
  this configuration. Configuration descriptor is also of
  variable length

Arguments:

  UsbDev  - The Usb interface
  Index   - The index of the configuration

Returns:

  The created configuration descriptor

--*/
{
  EFI_USB_CONFIG_DESCRIPTOR Desc;
  EFI_STATUS                Status;
  VOID                      *Buf;

  //
  // First get four bytes which contains the total length
  // for this configuration.
  //
  Status = UsbCtrlGetDesc (UsbDev, USB_DESC_TYPE_CONFIG, Index, 0, &Desc, 4);

  if (EFI_ERROR (Status)) {
    USB_ERROR (("UsbGetOneConfig: failed to get descript length(%d) %r\n", 
                Status, Desc.TotalLength));
    
    return NULL;
  }

  USB_DEBUG (("UsbGetOneConfig: total length is %d\n", Desc.TotalLength));

  Buf = EfiLibAllocateZeroPool (Desc.TotalLength);
  
  if (Buf == NULL) {
    return NULL;
  }
  
  Status = UsbCtrlGetDesc (UsbDev, USB_DESC_TYPE_CONFIG, Index, 0, Buf, Desc.TotalLength);

  if (EFI_ERROR (Status)) {
    USB_ERROR (("UsbGetOneConfig: failed to get full descript %r\n", Status));

    gBS->FreePool (Buf);
    return NULL;
  }

  return Buf;
}


EFI_STATUS
UsbBuildDescTable (
  IN USB_DEVICE           *UsbDev
  )
/*++

Routine Description:

  Build the whole array of descriptors. This function must
  be called after UsbGetMaxPacketSize0 returns the max packet
  size correctly for endpoint 0.

Arguments:

  UsbDev  - The Usb device

Returns:

  EFI_SUCCESS          - The descriptor table is build
  EFI_OUT_OF_RESOURCES - Failed to allocate resource for the descriptor

--*/
{
  EFI_USB_CONFIG_DESCRIPTOR *Config;
  USB_DEVICE_DESC           *DevDesc;
  USB_CONFIG_DESC           *ConfigDesc;
  UINT8                     NumConfig;
  EFI_STATUS                Status;
  UINT8                     Index;

  //
  // Get the device descriptor, then allocate the configure
  // descriptor pointer array to hold configurations.
  //
  Status = UsbGetDevDesc (UsbDev);

  if (EFI_ERROR (Status)) {
    USB_ERROR (("UsbBuildDescTable: failed to get device descriptor - %r\n", Status));
    return Status;
  }

  DevDesc          = UsbDev->DevDesc;
  NumConfig        = DevDesc->Desc.NumConfigurations;
  DevDesc->Configs = EfiLibAllocateZeroPool (NumConfig * sizeof (USB_CONFIG_DESC *));

  if (DevDesc->Configs == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  USB_DEBUG (("UsbBuildDescTable: device has %d configures\n", NumConfig));
  
  //
  // Read each configurations, then parse them
  //
  for (Index = 0; Index < NumConfig; Index++) {
    Config = UsbGetOneConfig (UsbDev, Index);

    if (Config == NULL) {
      USB_ERROR (("UsbBuildDescTable: failed to get configure (index %d)\n", Index));

      //
      // If we can get the default descriptor, it is likely that the
      // device is still operational.
      //
      if (Index == 0) {
        return EFI_DEVICE_ERROR;
      }

      break;
    }

    ConfigDesc = UsbParseConfigDesc ((UINT8 *) Config, Config->TotalLength);

    gBS->FreePool (Config);

    if (ConfigDesc == NULL) {
      USB_ERROR (("UsbBuildDescTable: failed to parse configure (index %d)\n", Index));

      //
      // If we can get the default descriptor, it is likely that the
      // device is still operational.
      //
      if (Index == 0) {
        return EFI_DEVICE_ERROR;
      } 

      break;
    }

    DevDesc->Configs[Index] = ConfigDesc;
  }

  //
  // Don't return error even this function failed because
  // it is possible for the device to not support strings.
  //
  Status = UsbBuildLangTable (UsbDev);

  if (EFI_ERROR (Status)) {
    USB_DEBUG (("UsbBuildDescTable: get language ID table %r\n", Status));
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
UsbSetAddress (
  IN USB_DEVICE           *UsbDev,
  IN UINT8                Address
  )
/*++

Routine Description:

  Set the device's address. 

Arguments:

  UsbDev  - The device to set address to
  Address - The address to set

Returns:

  EFI_SUCCESS - The device is set to the address
  Others      - Failed to set the device address

--*/
{
  EFI_STATUS              Status;

  Status = UsbCtrlRequest (
             UsbDev,
             EfiUsbNoData,
             USB_REQ_TYPE_STANDARD,
             USB_TARGET_DEVICE,
             USB_REQ_SET_ADDRESS,
             Address,
             0,
             NULL,
             0
             );

  return Status;
}

EFI_STATUS
UsbSetConfig (
  IN USB_DEVICE           *UsbDev,
  IN UINT8                ConfigIndex
  )
/*++

Routine Description:

  Set the device's configuration. This function changes
  the device's internal state. UsbSelectConfig changes
  the Usb bus's internal state.
  

Arguments:

  UsbDev      - The USB device to set configure to
  ConfigIndex - The configure index to set

Returns:

  EFI_SUCCESS - The device is configured now
  Others      - Failed to set the device configure

--*/
{
  EFI_STATUS              Status;

  Status = UsbCtrlRequest (
             UsbDev,
             EfiUsbNoData,
             USB_REQ_TYPE_STANDARD,
             USB_TARGET_DEVICE,
             USB_REQ_SET_CONFIG,
             ConfigIndex,
             0,
             NULL,
             0
            );

  return Status;
}


EFI_STATUS
UsbIoClearFeature (
  IN  EFI_USB_IO_PROTOCOL *UsbIo,
  IN  UINTN               Target,
  IN  UINT16              Feature,
  IN  UINT16              Index
  )
/*++

Routine Description:

  Usb UsbIo interface to clear the feature. This is should
  only be used by HUB which is considered a device driver
  on top of the UsbIo interface.

Arguments:

  UsbIo   - The UsbIo interface
  Target  - The target of the transfer: endpoint/device
  Feature - The feature to clear
  Index   - The wIndex parameter

Returns:

  EFI_SUCCESS - The device feature is cleared
  Others      - Failed to clear the feature

--*/
{
  EFI_USB_DEVICE_REQUEST  DevReq;
  UINT32                  UsbResult;
  EFI_STATUS              Status;

  DevReq.RequestType  = USB_REQUEST_TYPE (EfiUsbNoData, USB_REQ_TYPE_STANDARD, Target);
  DevReq.Request      = USB_REQ_CLEAR_FEATURE;
  DevReq.Value        = Feature;
  DevReq.Index        = Index;
  DevReq.Length       = 0;

  Status = UsbIo->UsbControlTransfer (
                    UsbIo,
                    &DevReq,
                    EfiUsbNoData,
                    USB_CLEAR_FEATURE_REQUEST_TIMEOUT,
                    NULL,
                    0,
                    &UsbResult
                    );

  return Status;
}
