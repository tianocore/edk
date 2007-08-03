/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:

    UsbUtility.c

  Abstract:

    Wrapper function for usb host controller interface

  Revision History

--*/

#include "UsbBus.h"

EFI_STATUS
UsbHcGetCapability (
  IN  USB_BUS             *UsbBus,
  OUT UINT8               *MaxSpeed,
  OUT UINT8               *NumOfPort,
  OUT UINT8               *Is64BitCapable
  )
/*++

Routine Description:

  Get the capability of the host controller

Arguments:

  UsbBus          - The usb driver
  MaxSpeed        - The maximum speed this host controller supports
  NumOfPort       - The number of the root hub port
  Is64BitCapable  - Whether this controller support 64 bit addressing

Returns:

  EFI_SUCCESS  - The host controller capability is returned
  Others       - Failed to retrieve the host controller capability.

--*/
{
  EFI_STATUS              Status;

  if (UsbBus->Usb2Hc != NULL) {
    Status = UsbBus->Usb2Hc->GetCapability (
                              UsbBus->Usb2Hc,
                              MaxSpeed,
                              NumOfPort,
                              Is64BitCapable
                              );
    
  } else {
    Status = UsbBus->UsbHc->GetRootHubPortNumber (UsbBus->UsbHc, NumOfPort);

    *MaxSpeed       = EFI_USB_SPEED_FULL;
    *Is64BitCapable = (UINT8) FALSE;
  }

  return Status;
}

EFI_STATUS
UsbHcReset (
  IN USB_BUS              *UsbBus,
  IN UINT16               Attributes
  )
/*++

Routine Description:

  Reset the host controller

Arguments:

  UsbBus      - The usb bus driver
  Attributes  - The reset type, only global reset is used by this driver

Returns:

  GC_TODO: add return values

--*/
{
  EFI_STATUS              Status;

  if (UsbBus->Usb2Hc != NULL) {
    Status = UsbBus->Usb2Hc->Reset (UsbBus->Usb2Hc, Attributes);
  } else {
    Status = UsbBus->UsbHc->Reset (UsbBus->UsbHc, Attributes);
  }

  return Status;
}

EFI_STATUS
UsbHcGetState (
  IN  USB_BUS             *UsbBus,
  OUT EFI_USB_HC_STATE    *State
  )
/*++

Routine Description:

  Get the current operation state of the host controller

Arguments:

  UsbBus  - The USB bus driver
  State   - The host controller operation state

Returns:

  EFI_SUCCESS - The operation state is returned in State
  Others      - Failed to get the host controller state

--*/
{
  EFI_STATUS              Status;

  if (UsbBus->Usb2Hc != NULL) {
    Status = UsbBus->Usb2Hc->GetState (UsbBus->Usb2Hc, State);
  } else {
    Status = UsbBus->UsbHc->GetState (UsbBus->UsbHc, State);
  }

  return Status;
}

EFI_STATUS
UsbHcSetState (
  IN USB_BUS              *UsbBus,
  IN EFI_USB_HC_STATE     State
  )
/*++

Routine Description:

  Set the host controller operation state

Arguments:

  UsbBus  - The USB bus driver
  State   - The state to set

Returns:

  EFI_SUCCESS - The host controller is now working at State
  Others      - Failed to set operation state

--*/
{
  EFI_STATUS              Status;

  if (UsbBus->Usb2Hc != NULL) {
    Status = UsbBus->Usb2Hc->SetState (UsbBus->Usb2Hc, State);
  } else {
    Status = UsbBus->UsbHc->SetState (UsbBus->UsbHc, State);
  }

  return Status;
}

EFI_STATUS
UsbHcGetRootHubPortStatus (
  IN  USB_BUS             *UsbBus,
  IN  UINT8               PortIndex,
  OUT EFI_USB_PORT_STATUS *PortStatus
  )
/*++

Routine Description:

  Get the root hub port state
  
Arguments:

  UsbBus      - The USB bus driver
  PortIndex   - The index of port 
  PortStatus  - The variable to save port state

Returns:

  EFI_SUCCESS - The root port state is returned in 
  Others      - Failed to get the root hub port state
  
--*/
{
  EFI_STATUS              Status;

  if (UsbBus->Usb2Hc != NULL) {
    Status = UsbBus->Usb2Hc->GetRootHubPortStatus (UsbBus->Usb2Hc, PortIndex, PortStatus);
  } else {
    Status = UsbBus->UsbHc->GetRootHubPortStatus (UsbBus->UsbHc, PortIndex, PortStatus);
  }

  return Status;
}

EFI_STATUS
UsbHcSetRootHubPortFeature (
  IN USB_BUS              *UsbBus,
  IN UINT8                PortIndex,
  IN EFI_USB_PORT_FEATURE Feature
  )
/*++

Routine Description:

  Set the root hub port feature

Arguments:

  UsbBus      - The USB bus driver
  PortIndex   - The port index
  Feature     - The port feature to set
  
Returns:

  EFI_SUCCESS - The port feature is set
  Others      - Failed to set port feature

--*/
{
  EFI_STATUS              Status;


  if (UsbBus->Usb2Hc != NULL) {
    Status = UsbBus->Usb2Hc->SetRootHubPortFeature (UsbBus->Usb2Hc, PortIndex, Feature);
  } else {
    Status = UsbBus->UsbHc->SetRootHubPortFeature (UsbBus->UsbHc, PortIndex, Feature);
  }

  return Status;
}

EFI_STATUS
UsbHcClearRootHubPortFeature (
  IN USB_BUS              *UsbBus,
  IN UINT8                PortIndex,
  IN EFI_USB_PORT_FEATURE Feature
  )
/*++

Routine Description:

  Clear the root hub port feature

Arguments:

  UsbBus      - The USB bus driver
  PortIndex   - The port index
  Feature     - The port feature to clear

Returns:

  EFI_SUCCESS - The port feature is clear
  Others      - Failed to clear port feature

--*/
{
  EFI_STATUS              Status;

  if (UsbBus->Usb2Hc != NULL) {
    Status = UsbBus->Usb2Hc->ClearRootHubPortFeature (UsbBus->Usb2Hc, PortIndex, Feature);
  } else {
    Status = UsbBus->UsbHc->ClearRootHubPortFeature (UsbBus->UsbHc, PortIndex, Feature);
  }

  return Status;
}

EFI_STATUS
UsbHcControlTransfer (
  IN  USB_BUS                             *UsbBus,
  IN  UINT8                               DevAddr,
  IN  UINT8                               DevSpeed,
  IN  UINTN                               MaxPacket,
  IN  EFI_USB_DEVICE_REQUEST              *Request,
  IN  EFI_USB_DATA_DIRECTION              Direction,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *UsbResult
  )
/*++

Routine Description:

  Execute a control transfer to the device

Arguments:

  UsbBus     - The USB bus driver
  DevAddr    - The device address
  DevSpeed   - The device speed
  MaxPacket  - Maximum packet size of endpoint 0
  Request    - The control transfer request
  Direction  - The direction of data stage
  Data       - The buffer holding data
  DataLength - The length of the data
  TimeOut    - Timeout (in ms) to wait until timeout
  Translator - The transaction translator for low/full speed device
  UsbResult  - The result of transfer

Returns:

  EFI_SUCCESS - The control transfer finished without error
  Others      - The control transfer failed, reason returned in UsbReslt

--*/
{
  EFI_STATUS              Status;
  BOOLEAN                 IsSlowDevice;

  if (UsbBus->Usb2Hc != NULL) {
    Status = UsbBus->Usb2Hc->ControlTransfer (
                               UsbBus->Usb2Hc,
                               DevAddr,
                               DevSpeed,
                               MaxPacket,
                               Request,
                               Direction,
                               Data,
                               DataLength,
                               TimeOut,
                               Translator,
                               UsbResult
                               );
    
  } else {
    IsSlowDevice = (BOOLEAN)(EFI_USB_SPEED_LOW == DevSpeed);
    Status = UsbBus->UsbHc->ControlTransfer (
                              UsbBus->UsbHc,
                              DevAddr,
                              IsSlowDevice,
                              (UINT8) MaxPacket,
                              Request,
                              Direction,
                              Data,
                              DataLength,
                              TimeOut,
                              UsbResult
                              );
  }

  return Status;
}

EFI_STATUS
UsbHcBulkTransfer (
  IN  USB_BUS                             *UsbBus,
  IN  UINT8                               DevAddr,
  IN  UINT8                               EpAddr,
  IN  UINT8                               DevSpeed,
  IN  UINTN                               MaxPacket,
  IN  UINT8                               BufferNum,
  IN  OUT VOID                            *Data[EFI_USB_MAX_BULK_BUFFER_NUM],
  IN  OUT UINTN                           *DataLength,
  IN  OUT UINT8                           *DataToggle,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *UsbResult
  )
/*++

Routine Description:

  Execute a bulk transfer to the device's endpoint

Arguments:

  UsbBus      - The USB bus driver
  DevAddr     - The target device address
  EpAddr      - The target endpoint address, with direction encoded in bit 7
  DevSpeed    - The device's speed
  MaxPacket   - The endpoint's max packet size
  BufferNum   - The number of data buffer
  Data        - Array of pointers to data buffer 
  DataLength  - The length of data buffer
  DataToggle  - On input, the initial data toggle to use, also 
                return the next toggle on output.
  TimeOut     - The time to wait until timeout
  Translator  - The transaction translator for low/full speed device
  UsbResult   - The result of USB execution

Returns:

  EFI_SUCCESS - The bulk transfer is finished without error
  Others      - Failed to execute bulk transfer, result in UsbResult

--*/
{
  EFI_STATUS              Status;

  if (UsbBus->Usb2Hc != NULL) {
    Status = UsbBus->Usb2Hc->BulkTransfer (
                               UsbBus->Usb2Hc,
                               DevAddr,
                               EpAddr,
                               DevSpeed,
                               MaxPacket,
                               BufferNum,
                               Data,
                               DataLength,
                               DataToggle,
                               TimeOut,
                               Translator,
                               UsbResult
                               );
  } else {
    Status = UsbBus->UsbHc->BulkTransfer (
                              UsbBus->UsbHc,
                              DevAddr,
                              EpAddr,
                              (UINT8) MaxPacket,
                              *Data,
                              DataLength,
                              DataToggle,
                              TimeOut,
                              UsbResult
                              );
  }

  return Status;
}

EFI_STATUS
UsbHcAsyncInterruptTransfer (
  IN  USB_BUS                             *UsbBus,
  IN  UINT8                               DevAddr,
  IN  UINT8                               EpAddr,
  IN  UINT8                               DevSpeed,
  IN  UINTN                               MaxPacket,
  IN  BOOLEAN                             IsNewTransfer,
  IN OUT UINT8                            *DataToggle,
  IN  UINTN                               PollingInterval,
  IN  UINTN                               DataLength,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  IN  EFI_ASYNC_USB_TRANSFER_CALLBACK     Callback,
  IN  VOID                                *Context OPTIONAL
  )
/*++

Routine Description:

  Queue or cancel an asynchronous interrupt transfer

Arguments:
  UsbBus          - The USB bus driver
  DevAddr         - The target device address
  EpAddr          - The target endpoint address, with direction encoded in bit 7
  DevSpeed        - The device's speed
  MaxPacket       - The endpoint's max packet size
  IsNewTransfer   - Whether this is a new request. If not, cancel the old request
  DataToggle      - Data toggle to use on input, next toggle on output
  PollingInterval - The interval to poll the interrupt transfer (in ms)
  DataLength      - The length of periodical data receive
  Translator      - The transaction translator for low/full speed device
  Callback        - Function to call when data is received
  Context         - The context to the callback

Returns:

  EFI_SUCCESS     - The asynchronous transfer is queued 
  Others          - Failed to queue the transfer

--*/
{
  EFI_STATUS              Status;
  BOOLEAN                 IsSlowDevice;

  if (UsbBus->Usb2Hc != NULL) {
    Status = UsbBus->Usb2Hc->AsyncInterruptTransfer (
                               UsbBus->Usb2Hc,
                               DevAddr,
                               EpAddr,
                               DevSpeed,
                               MaxPacket,
                               IsNewTransfer,
                               DataToggle,
                               PollingInterval,
                               DataLength,
                               Translator,
                               Callback,
                               Context
                               );
  } else {
    IsSlowDevice = (BOOLEAN)(EFI_USB_SPEED_LOW == DevSpeed);
    
    Status = UsbBus->UsbHc->AsyncInterruptTransfer (
                              UsbBus->UsbHc,
                              DevAddr,
                              EpAddr,
                              IsSlowDevice,
                              (UINT8) MaxPacket,
                              IsNewTransfer,
                              DataToggle,
                              PollingInterval,
                              DataLength,
                              Callback,
                              Context
                              );
  }

  return Status;
}

EFI_STATUS
UsbHcSyncInterruptTransfer (
  IN  USB_BUS                             *UsbBus,
  IN  UINT8                               DevAddr,
  IN  UINT8                               EpAddr,
  IN  UINT8                               DevSpeed,
  IN  UINTN                               MaxPacket,
  IN OUT VOID                             *Data,
  IN OUT UINTN                            *DataLength,
  IN OUT UINT8                            *DataToggle,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *UsbResult
  )
/*++

Routine Description:

  Execute a synchronous interrupt transfer to the target endpoint

Arguments:

  UsbBus      - The USB bus driver
  DevAddr     - The target device address
  EpAddr      - The target endpoint address, with direction encoded in bit 7
  DevSpeed    - The device's speed
  MaxPacket   - The endpoint's max packet size
  Data        - Pointer to data buffer 
  DataLength  - The length of data buffer
  DataToggle  - On input, the initial data toggle to use, also 
                return the next toggle on output.
  TimeOut     - The time to wait until timeout
  Translator  - The transaction translator for low/full speed device
  UsbResult   - The result of USB execution


Returns:

  EFI_SUCCESS - The synchronous interrupt transfer is OK
  Others      - Failed to execute the synchronous interrupt transfer
  
--*/
{
  EFI_STATUS              Status;
  BOOLEAN                 IsSlowDevice;

  if (UsbBus->Usb2Hc != NULL) {
    Status = UsbBus->Usb2Hc->SyncInterruptTransfer (
                               UsbBus->Usb2Hc,
                               DevAddr,
                               EpAddr,
                               DevSpeed,
                               MaxPacket,
                               Data,
                               DataLength,
                               DataToggle,
                               TimeOut,
                               Translator,
                               UsbResult
                               );
  } else {
    IsSlowDevice = (EFI_USB_SPEED_LOW == DevSpeed) ? TRUE : FALSE;
    Status = UsbBus->UsbHc->SyncInterruptTransfer (
                              UsbBus->UsbHc,
                              DevAddr,
                              EpAddr,
                              IsSlowDevice,
                              (UINT8) MaxPacket,
                              Data,
                              DataLength,
                              DataToggle,
                              TimeOut,
                              UsbResult
                              );
  }

  return Status;
}

EFI_STATUS
UsbHcIsochronousTransfer (
  IN  USB_BUS                             *UsbBus,
  IN  UINT8                               DevAddr,
  IN  UINT8                               EpAddr,
  IN  UINT8                               DevSpeed,
  IN  UINTN                               MaxPacket,
  IN  UINT8                               BufferNum,
  IN  OUT VOID                            *Data[EFI_USB_MAX_ISO_BUFFER_NUM],
  IN  UINTN                               DataLength,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *UsbResult
  )
/*++

Routine Description:

  Execute a synchronous Isochronous USB transfer

Arguments:
  UsbBus      - The USB bus driver
  DevAddr     - The target device address
  EpAddr      - The target endpoint address, with direction encoded in bit 7
  DevSpeed    - The device's speed
  MaxPacket   - The endpoint's max packet size
  BufferNum   - The number of data buffer
  Data        - Array of pointers to data buffer 
  DataLength  - The length of data buffer
  Translator  - The transaction translator for low/full speed device
  UsbResult   - The result of USB execution

Returns:

  EFI_UNSUPPORTED - The isochronous transfer isn't supported now

--*/
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
UsbHcAsyncIsochronousTransfer (
  IN  USB_BUS                             *UsbBus,
  IN  UINT8                               DevAddr,
  IN  UINT8                               EpAddr,
  IN  UINT8                               DevSpeed,
  IN  UINTN                               MaxPacket,
  IN  UINT8                               BufferNum,
  IN OUT VOID                             *Data[EFI_USB_MAX_ISO_BUFFER_NUM],
  IN  UINTN                               DataLength,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  IN  EFI_ASYNC_USB_TRANSFER_CALLBACK     Callback,
  IN  VOID                                *Context
  )
/*++

Routine Description:

  Queue an asynchronous isochronous transfer

Arguments:

  UsbBus      - The USB bus driver
  DevAddr     - The target device address
  EpAddr      - The target endpoint address, with direction encoded in bit 7
  DevSpeed    - The device's speed
  MaxPacket   - The endpoint's max packet size
  BufferNum   - The number of data buffer
  Data        - Array of pointers to data buffer 
  DataLength  - The length of data buffer
  Translator  - The transaction translator for low/full speed device
  Callback    - The function to call when data is transferred
  Context     - The context to the callback function

Returns:

  EFI_UNSUPPORTED - The asynchronous isochronous transfer isn't supported 

--*/
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
UsbOpenHostProtoByChild (
  IN USB_BUS              *Bus,
  IN EFI_HANDLE           Child
  )
/*++

Routine Description:

  Open the USB host controller protocol BY_CHILD

Arguments:

  Bus   - The USB bus driver
  Child - The child handle

Returns:

  The open protocol return

--*/
{
  EFI_USB_HC_PROTOCOL     *UsbHc;
  EFI_USB2_HC_PROTOCOL    *Usb2Hc;
  EFI_STATUS              Status;

  if (Bus->Usb2Hc != NULL) {
    Status = gBS->OpenProtocol (
                    Bus->HostHandle,
                    &gEfiUsb2HcProtocolGuid,
                    &Usb2Hc,
                    mUsbBusDriverBinding.DriverBindingHandle,
                    Child,
                    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                    );

  } else {
    Status = gBS->OpenProtocol (
                    Bus->HostHandle,
                    &gEfiUsbHcProtocolGuid,
                    &UsbHc,
                    mUsbBusDriverBinding.DriverBindingHandle,
                    Child,
                    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                    );
  }

  return Status;
}

VOID
UsbCloseHostProtoByChild (
  IN USB_BUS              *Bus,
  IN EFI_HANDLE           Child
  )
/*++

Routine Description:

  Close the USB host controller protocol BY_CHILD

Arguments:

  Bus   - The USB bus driver
  Child - The child handle

Returns:

  None

--*/
{
  if (Bus->Usb2Hc != NULL) {
    gBS->CloseProtocol (
           Bus->HostHandle,
           &gEfiUsb2HcProtocolGuid,
           mUsbBusDriverBinding.DriverBindingHandle,
           Child
           );

  } else {
    gBS->CloseProtocol (
           Bus->HostHandle,
           &gEfiUsbHcProtocolGuid,
           mUsbBusDriverBinding.DriverBindingHandle,
           Child
           );
  }
}


EFI_TPL
UsbGetCurrentTpl (
  VOID
  )
/*++

Routine Description:

  return the current TPL, copied from the EDKII glue lib. 

Arguments:

  VOID

Returns:

  Current TPL

--*/
{
  EFI_TPL                 Tpl;

  Tpl = gBS->RaiseTPL (EFI_TPL_HIGH_LEVEL); 
  gBS->RestoreTPL (Tpl);

  return Tpl;
}


#ifdef EFI_DEBUG
VOID
UsbDebug (
  IN  CHAR8               *Format,
  ...
  )
/*++

Routine Description:

  USB's debug output function.

Arguments:

  Format  - The format parameters to the print
  ...     - The variable length parameters after format

Returns:

  None

--*/  
{
  VA_LIST                 Marker;

  VA_START (Marker, Format);
  EfiDebugVPrint (EFI_D_INFO, Format, Marker);
  VA_END (Marker);
}


VOID
UsbError (
  IN  CHAR8               *Format,
  ...
  )
/*++

Routine Description:

  USB's error output function.

Arguments:

  Format  - The format parameters to the print
  ...     - The variable length parameters after format

Returns:

  None

--*/  
{
  VA_LIST                 Marker;

  VA_START (Marker, Format);
  EfiDebugVPrint (EFI_D_ERROR, Format, Marker);
  VA_END (Marker);
}

#endif
