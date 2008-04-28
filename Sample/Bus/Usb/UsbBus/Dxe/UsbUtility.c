/*++

Copyright (c) 2007 - 2008, Intel Corporation                                                         
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

//
// if RemainingDevicePath== NULL, then all Usb child devices in this bus are wanted.
// Use a shor form Usb class Device Path, which could match any usb device, in WantedUsbIoDPList to indicate all Usb devices 
// are wanted Usb devices
//
STATIC USB_CLASS_FORMAT_DEVICE_PATH mAllUsbClassDevicePath = {
  {
    {
      MESSAGING_DEVICE_PATH,
      MSG_USB_CLASS_DP,
      (UINT8) (sizeof (USB_CLASS_DEVICE_PATH)),
      (UINT8) ((sizeof (USB_CLASS_DEVICE_PATH)) >> 8)
    },
    0xffff, // VendorId 
    0xffff, // ProductId 
    0xff,   // DeviceClass 
    0xff,   // DeviceSubClass
    0xff    // DeviceProtocol
  },

  { 
    END_DEVICE_PATH_TYPE, 
    END_ENTIRE_DEVICE_PATH_SUBTYPE, 
    END_DEVICE_PATH_LENGTH, 
    0
  }
};

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

EFI_DEVICE_PATH_PROTOCOL *
EFIAPI
GetUsbDPFromFullDP (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  )
/*++

Routine Description:
  Create a new device path which only contain the first Usb part of the DevicePath

Arguments:
  DevicePath - A full device path which contain the usb nodes

Returns:
 A new device path which only contain the Usb part of the DevicePath
    
--*/
{ 
  EFI_DEVICE_PATH_PROTOCOL    *UsbDevicePathPtr;
  EFI_DEVICE_PATH_PROTOCOL    *UsbDevicePathBeginPtr;
  EFI_DEVICE_PATH_PROTOCOL    *UsbDevicePathEndPtr;
  UINTN                       Size;
  
  //
  // Get the Usb part first Begin node in full device path
  //
  UsbDevicePathBeginPtr = DevicePath;
  while ( (!EfiIsDevicePathEnd (UsbDevicePathBeginPtr))&&
         ((UsbDevicePathBeginPtr->Type != MESSAGING_DEVICE_PATH) || 
         (UsbDevicePathBeginPtr->SubType != MSG_USB_DP && 
          UsbDevicePathBeginPtr->SubType != MSG_USB_CLASS_DP 
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
          && UsbDevicePathBeginPtr->SubType != MSG_USB_WWID_DP
#endif
          ))) {
          
    UsbDevicePathBeginPtr = NextDevicePathNode(UsbDevicePathBeginPtr);
  }

  //
  // Get the Usb part first End node in full device path
  //
  UsbDevicePathEndPtr = UsbDevicePathBeginPtr;
  while ((!EfiIsDevicePathEnd (UsbDevicePathEndPtr))&&
         (UsbDevicePathEndPtr->Type == MESSAGING_DEVICE_PATH) && 
         (UsbDevicePathEndPtr->SubType == MSG_USB_DP || 
          UsbDevicePathEndPtr->SubType == MSG_USB_CLASS_DP 
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
          || UsbDevicePathEndPtr->SubType == MSG_USB_WWID_DP
#endif
          )) {
          
    UsbDevicePathEndPtr = NextDevicePathNode(UsbDevicePathEndPtr);
  }
  
  Size = EfiDevicePathSize(UsbDevicePathBeginPtr) - EfiDevicePathSize (UsbDevicePathEndPtr);
  if (Size ==0){
    //
    // The passed in DevicePath does not contain the usb nodes
    //
    return NULL;
  }
  
  //
  // Create a new device path which only contain the above Usb part
  //
  UsbDevicePathPtr = EfiLibAllocateZeroPool (Size + sizeof (EFI_DEVICE_PATH_PROTOCOL));
  ASSERT (UsbDevicePathPtr != NULL);
  EfiCopyMem (UsbDevicePathPtr, UsbDevicePathBeginPtr, Size);
  //
  // Append end device path node
  //
  UsbDevicePathEndPtr = (EFI_DEVICE_PATH_PROTOCOL *) ((UINTN) UsbDevicePathPtr + Size);
  SetDevicePathEndNode (UsbDevicePathEndPtr);
  return UsbDevicePathPtr;  
}

BOOLEAN
EFIAPI
SearchUsbDPInList (
  IN EFI_DEVICE_PATH_PROTOCOL     *UsbDP,
  IN EFI_LIST_ENTRY               *UsbIoDPList
  )
/*++

Routine Description:
  Check whether a usb device path is in a DEVICE_PATH_LIST_ITEM list.

Arguments:
  UsbDP            - a usb device path of DEVICE_PATH_LIST_ITEM
  UsbIoDPList - a DEVICE_PATH_LIST_ITEM list

Returns:
  TRUE    - there is a DEVICE_PATH_LIST_ITEM in UsbIoDPList which contains the passed in UsbDP
  FALSE  - there is no DEVICE_PATH_LIST_ITEM in UsbIoDPList which contains the passed in UsbDP
    
--*/
{ 
  EFI_LIST_ENTRY              *ListIndex;
  DEVICE_PATH_LIST_ITEM       *ListItem;
  BOOLEAN                     Found;
  
  //
  // Check that UsbDP and UsbIoDPList are valid
  //
  if ((UsbIoDPList == NULL) || (UsbDP == NULL)) {
    return FALSE;
  }
  
  Found = FALSE;
  ListIndex = UsbIoDPList->ForwardLink;
  while (ListIndex != UsbIoDPList){
    ListItem = CR(ListIndex, DEVICE_PATH_LIST_ITEM, Link, DEVICE_PATH_LIST_ITEM_SIGNATURE);
    //
    // Compare DEVICE_PATH_LIST_ITEM.DevicePath[]
    //
    ASSERT (ListItem->DevicePath != NULL);
    if (EfiDevicePathSize (UsbDP) == EfiDevicePathSize (ListItem->DevicePath)) {
      if (EfiCompareMem (UsbDP, ListItem->DevicePath, EfiDevicePathSize (UsbDP)) == 0) {
        Found = TRUE;
        break;
      }        
    } 
    ListIndex =  ListIndex->ForwardLink;
  }      

  return Found;  
}

EFI_STATUS
EFIAPI
AddUsbDPToList (
  IN EFI_DEVICE_PATH_PROTOCOL     *UsbDP,
  IN EFI_LIST_ENTRY               *UsbIoDPList
  )
/*++

Routine Description:
    Add a usb device path into the DEVICE_PATH_LIST_ITEM list.

Arguments:
  UsbDP            - a usb device path of DEVICE_PATH_LIST_ITEM
  UsbIoDPList - a DEVICE_PATH_LIST_ITEM list

Returns:
    EFI_INVALID_PARAMETER
    EFI_SUCCESS
    
--*/
{ 
  DEVICE_PATH_LIST_ITEM       *ListItem;

  //
  // Check that UsbDP and UsbIoDPList are valid
  //
  if ((UsbIoDPList == NULL) || (UsbDP == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (SearchUsbDPInList (UsbDP, UsbIoDPList)){
    return EFI_SUCCESS;
  }

  //
  // Prepare the usbio device path DEVICE_PATH_LIST_ITEM structure.
  //
  ListItem = EfiLibAllocateZeroPool (sizeof (DEVICE_PATH_LIST_ITEM)); 
  ASSERT (ListItem != NULL);
  ListItem->Signature = DEVICE_PATH_LIST_ITEM_SIGNATURE;
  ListItem->DevicePath = EfiDuplicateDevicePath (UsbDP);  
  
  InsertTailList (UsbIoDPList, &ListItem->Link);

  return EFI_SUCCESS;  
}


BOOLEAN
EFIAPI
MatchUsbClass (
  IN USB_CLASS_DEVICE_PATH      *UsbClassDevicePathPtr,
  IN USB_INTERFACE              *UsbIf
  )
/*++

Routine Description:
  Check whether usb device, whose interface is UsbIf, matches the usb class which indicated by 
  UsbClassDevicePathPtr whose is a short form usb class device path

Arguments:
  UsbClassDevicePathPtr    - a short form usb class device path 
  UsbIf                                 - a usb device interface

Returns:
  TRUE    - the usb device match the usb class
  FALSE  - the usb device does not match the usb class
    
--*/
{
  USB_INTERFACE_DESC            *IfDesc;
  EFI_USB_INTERFACE_DESCRIPTOR  *ActIfDesc;
  EFI_USB_DEVICE_DESCRIPTOR     *DevDesc;

  
  if ((UsbClassDevicePathPtr->Header.Type != MESSAGING_DEVICE_PATH) ||
      (UsbClassDevicePathPtr->Header.SubType != MSG_USB_CLASS_DP)){
    ASSERT (0);
    return FALSE;
  }
      
  IfDesc       = UsbIf->IfDesc;
  ActIfDesc    = &(IfDesc->Settings[IfDesc->ActiveIndex]->Desc);
  DevDesc      = &(UsbIf->Device->DevDesc->Desc);
  
  //
  // If connect class policy, determine whether to create device handle by the five fields 
  // in class device path node. 
  //
  // In addtion, hub interface is always matched for this policy.
  // 
  if ((ActIfDesc->InterfaceClass == USB_HUB_CLASS_CODE) && 
      (ActIfDesc->InterfaceSubClass == USB_HUB_SUBCLASS_CODE)) {
    return TRUE;
  }

  //
  // If vendor id or product id is 0xffff, they will be ignored. 
  //
  if ((UsbClassDevicePathPtr->VendorId == 0xffff || UsbClassDevicePathPtr->VendorId == DevDesc->IdVendor) &&
      (UsbClassDevicePathPtr->ProductId == 0xffff || UsbClassDevicePathPtr->ProductId == DevDesc->IdProduct)) {
      
    //
    // If class or subclass or protocol is 0, the counterparts in interface should be checked.
    //
    if (DevDesc->DeviceClass == 0 || 
        DevDesc->DeviceSubClass == 0 || 
        DevDesc->DeviceProtocol == 0) {
     
      if ((UsbClassDevicePathPtr->DeviceClass == ActIfDesc->InterfaceClass ||
                                          UsbClassDevicePathPtr->DeviceClass == 0xff) && 
          (UsbClassDevicePathPtr->DeviceSubClass == ActIfDesc->InterfaceSubClass ||
                                       UsbClassDevicePathPtr->DeviceSubClass == 0xff) &&
          (UsbClassDevicePathPtr->DeviceProtocol == ActIfDesc->InterfaceProtocol || 
                                       UsbClassDevicePathPtr->DeviceProtocol == 0xff)) {
        return TRUE;
      }
      
    } else if ((UsbClassDevicePathPtr->DeviceClass == DevDesc->DeviceClass ||
                                         UsbClassDevicePathPtr->DeviceClass == 0xff) && 
               (UsbClassDevicePathPtr->DeviceSubClass == DevDesc->DeviceSubClass ||
                                      UsbClassDevicePathPtr->DeviceSubClass == 0xff) &&
               (UsbClassDevicePathPtr->DeviceProtocol == DevDesc->DeviceProtocol ||
                                      UsbClassDevicePathPtr->DeviceProtocol == 0xff)) {

      return TRUE;
    }
  }
  
  return FALSE;
}

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
STATIC
BOOLEAN
MatchUsbWwid (
  IN USB_WWID_DEVICE_PATH       *UsbWWIDDevicePathPtr,
  IN USB_INTERFACE              *UsbIf
  )
/*++

Routine Description:
  Check whether usb device, whose interface is UsbIf, matches the usb WWID requirement which indicated by 
  UsbWWIDDevicePathPtr whose is a short form usb WWID device path

Arguments:
  UsbClassDevicePathPtr    - a short form usb WWID device path 
  UsbIf                                 - a usb device interface

Returns:
  TRUE    - the usb device match the usb WWID requirement 
  FALSE  - the usb device does not match the usb WWID requirement
    
--*/
{
  USB_INTERFACE_DESC            *IfDesc;
  EFI_USB_INTERFACE_DESCRIPTOR  *ActIfDesc;
  EFI_USB_DEVICE_DESCRIPTOR     *DevDesc;
  EFI_USB_STRING_DESCRIPTOR     *StrDesc;
  UINT16                        *SnString;
  
  if ((UsbWWIDDevicePathPtr->Header.Type != MESSAGING_DEVICE_PATH) ||
     (UsbWWIDDevicePathPtr->Header.SubType != MSG_USB_WWID_DP )){
    ASSERT (0);
    return FALSE;
  }
  
  IfDesc       = UsbIf->IfDesc;
  ActIfDesc    = &(IfDesc->Settings[IfDesc->ActiveIndex]->Desc);
  DevDesc      = &(UsbIf->Device->DevDesc->Desc);
  StrDesc      = UsbGetOneString (UsbIf->Device, DevDesc->StrSerialNumber, USB_US_LAND_ID);
  SnString     = (UINT16 *) ((UINT8 *)UsbWWIDDevicePathPtr + 10);
  
  //
  //In addtion, hub interface is always matched for this policy.
  //
  if ((ActIfDesc->InterfaceClass == USB_HUB_CLASS_CODE) && 
      (ActIfDesc->InterfaceSubClass == USB_HUB_SUBCLASS_CODE)) {
    return TRUE;
  }
  //
  // If connect wwid policy, determine the objective device by the serial number of 
  // device descriptor. 
  // Get serial number index from device descriptor, then get serial number by index
  // and land id, compare the serial number with wwid device path node at last
  //
  // BugBug: only check serial number here, should check Interface Number, Device Vendor Id, Device Product Id  in later version
  // 
  if (StrDesc != NULL && !EfiStrnCmp (StrDesc->String, SnString, StrDesc->Length)) { 
    
    return TRUE;
  }

  return FALSE;
}
#endif

EFI_STATUS
EFIAPI
UsbBusFreeUsbDPList (
  IN EFI_LIST_ENTRY      *UsbIoDPList
  )
/*++

Routine Description:
   Free a DEVICE_PATH_LIST_ITEM list

Arguments:
  UsbIoDPList - a DEVICE_PATH_LIST_ITEM list pointer

Returns:
    EFI_INVALID_PARAMETER
    EFI_SUCCESS
    
--*/
{ 
  EFI_LIST_ENTRY              *ListIndex;
  DEVICE_PATH_LIST_ITEM       *ListItem;

  //
  // Check that ControllerHandle is a valid handle
  //
  if (UsbIoDPList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ListIndex = UsbIoDPList->ForwardLink;
  while (ListIndex != UsbIoDPList){
    ListItem = CR(ListIndex, DEVICE_PATH_LIST_ITEM, Link, DEVICE_PATH_LIST_ITEM_SIGNATURE);
    //
    // Free DEVICE_PATH_LIST_ITEM.DevicePath[]
    //
    if (ListItem->DevicePath != NULL){
      EfiLibSafeFreePool(ListItem->DevicePath);     
    }   
    //
    // Free DEVICE_PATH_LIST_ITEM itself
    //
    ListIndex =  ListIndex->ForwardLink;
    RemoveEntryList (&ListItem->Link);
    EfiLibSafeFreePool (ListItem);  
  }      

  InitializeListHead (UsbIoDPList);
  return EFI_SUCCESS;  
}

EFI_STATUS
EFIAPI
UsbBusAddWantedUsbIoDP (
  IN EFI_USB_BUS_PROTOCOL         *UsbBusId,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:
  Store a wanted usb child device info (its Usb part of device path) which is indicated by
  RemainingDevicePath in a Usb bus which  is indicated by UsbBusId

Arguments:
  UsbBusId - point to EFI_USB_BUS_PROTOCOL interface
  RemainingDevicePath - The remaining device patch
  
Returns:
  EFI_SUCCESS  
  EFI_INVALID_PARAMETER
  EFI_OUT_OF_RESOURCES 

--*/
{
  USB_BUS                       *Bus;
  EFI_STATUS                    Status;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePathPtr;
  
  //
  // Check whether remaining device path is valid
  //
  if (RemainingDevicePath != NULL) {
    if ((RemainingDevicePath->Type    != MESSAGING_DEVICE_PATH) ||
        (RemainingDevicePath->SubType != MSG_USB_DP && 
         RemainingDevicePath->SubType != MSG_USB_CLASS_DP 
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
         && RemainingDevicePath->SubType != MSG_USB_WWID_DP
#endif
         )) {
      return EFI_INVALID_PARAMETER;
    }
  }
  
  if (UsbBusId == NULL){
    return EFI_INVALID_PARAMETER;
  }
  
  Bus = USB_BUS_FROM_THIS (UsbBusId);  
  
  if (RemainingDevicePath == NULL) {
    //
    // RemainingDevicePath== NULL means all Usb devices in this bus are wanted.
    // Here use a Usb class Device Path in WantedUsbIoDPList to indicate all Usb devices 
    // are wanted Usb devices
    //
    Status = UsbBusFreeUsbDPList (&Bus->WantedUsbIoDPList);
    ASSERT (!EFI_ERROR (Status));
    DevicePathPtr = EfiDuplicateDevicePath ((EFI_DEVICE_PATH_PROTOCOL *) &mAllUsbClassDevicePath);
  } else {
    //
    // Create new Usb device path according to the usb part in remaining device path
    //
    DevicePathPtr = GetUsbDPFromFullDP (RemainingDevicePath);
  }
  
  ASSERT (DevicePathPtr != NULL); 
  Status = AddUsbDPToList (DevicePathPtr, &Bus->WantedUsbIoDPList);
  ASSERT (!EFI_ERROR (Status));  
  gBS->FreePool (DevicePathPtr);
  return EFI_SUCCESS;
}

BOOLEAN
EFIAPI
UsbBusIsWantedUsbIO (
  IN USB_BUS                 *Bus,
  IN USB_INTERFACE           *UsbIf
  )
/*++

Routine Description:
  Check whether a usb child  device is the wanted device in a bus

Arguments:
  Bus     -   The Usb bus's private data pointer
  UsbIf -  The usb child  device inferface
  
Returns:
  EFI_SUCCESS   
  EFI_INVALID_PARAMETER 
  EFI_OUT_OF_RESOURCES 

--*/
{
  EFI_DEVICE_PATH_PROTOCOL      *DevicePathPtr;
  EFI_LIST_ENTRY                *WantedUsbIoDPListPtr;
  EFI_LIST_ENTRY                *WantedListIndex;
  DEVICE_PATH_LIST_ITEM         *WantedListItem;
  BOOLEAN                       DoConvert;
  
  //
  // Check whether passed in parameters are valid
  //
  if ((UsbIf == NULL) || (Bus == NULL)) {
    return FALSE;
  }
  //
  // Check whether UsbIf is Hub
  //
  if (UsbIf->IsHub) {
    return TRUE;
  }
    
  //
  // Check whether all Usb devices in this bus are wanted
  //
  if (SearchUsbDPInList ((EFI_DEVICE_PATH_PROTOCOL *)&mAllUsbClassDevicePath, &Bus->WantedUsbIoDPList)){
    return TRUE;
  }
  
  //
  // Check whether the Usb device match any item in WantedUsbIoDPList
  //
  WantedUsbIoDPListPtr = &Bus->WantedUsbIoDPList;
  //
  // Create new Usb device path according to the usb part in UsbIo full device path
  //
  DevicePathPtr = GetUsbDPFromFullDP (UsbIf->DevicePath);
  ASSERT (DevicePathPtr != NULL);   
  
  DoConvert = FALSE;
  WantedListIndex = WantedUsbIoDPListPtr->ForwardLink;
  while (WantedListIndex != WantedUsbIoDPListPtr){
    WantedListItem = CR(WantedListIndex, DEVICE_PATH_LIST_ITEM, Link, DEVICE_PATH_LIST_ITEM_SIGNATURE);
    ASSERT (WantedListItem->DevicePath->Type == MESSAGING_DEVICE_PATH);
    switch (WantedListItem->DevicePath->SubType) {
    case MSG_USB_DP:
      if (EfiDevicePathSize (WantedListItem->DevicePath) == EfiDevicePathSize (DevicePathPtr)) {
        if (EfiCompareMem (
              WantedListItem->DevicePath,
              DevicePathPtr,
              EfiDevicePathSize (DevicePathPtr)) == 0
            ) {
          DoConvert = TRUE;
        }
      }
      break;
    case MSG_USB_CLASS_DP:
      if (MatchUsbClass((USB_CLASS_DEVICE_PATH *)WantedListItem->DevicePath, UsbIf)) {
        DoConvert = TRUE;
      }
      break;
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
    case MSG_USB_WWID_DP:
      if (MatchUsbWwid((USB_WWID_DEVICE_PATH *)WantedListItem->DevicePath, UsbIf)) {
        DoConvert = TRUE;
      }
      break;
#endif
    default:
      ASSERT (0);
      break;
    }
    
    if (DoConvert) {
      break;
    }
    
    WantedListIndex =  WantedListIndex->ForwardLink;
  }
  gBS->FreePool (DevicePathPtr);  
  
  //
  // Check whether the new Usb device path is wanted
  //
  if (DoConvert){
    return TRUE;
  } else {
    return FALSE;
  }
}

EFI_STATUS
EFIAPI
UsbBusRecursivelyConnectWantedUsbIo (
  IN EFI_USB_BUS_PROTOCOL         *UsbBusId
  )
/*++

Routine Description:
  Recursively connnect every wanted usb child device to ensure they all fully connected.
  Check all the child Usb IO handles in this bus, recursively connecte if it is wanted usb child device
Arguments:

  UsbBusId - point to EFI_USB_BUS_PROTOCOL interface
  
Returns:

  EFI_SUCCESS
  EFI_INVALID_PARAMETER 
  EFI_OUT_OF_RESOURCES  

--*/
{
  USB_BUS                       *Bus;
  EFI_STATUS                    Status;
  UINTN                         Index;
  EFI_USB_IO_PROTOCOL           *UsbIo;
  USB_INTERFACE                 *UsbIf;
  UINTN                         UsbIoHandleCount;
  EFI_HANDLE                    *UsbIoBuffer;
  EFI_DEVICE_PATH_PROTOCOL      *UsbIoDevicePath;
  
  if (UsbBusId == NULL){
    return EFI_INVALID_PARAMETER;
  }
  
  Bus = USB_BUS_FROM_THIS (UsbBusId);  

  //
  // Get all Usb IO handles in system
  //
  UsbIoHandleCount = 0;
  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiUsbIoProtocolGuid, NULL, &UsbIoHandleCount, &UsbIoBuffer);
  if (Status == EFI_NOT_FOUND || UsbIoHandleCount == 0) {
    return EFI_SUCCESS;
  } 
  ASSERT (!EFI_ERROR (Status));
  
  for (Index = 0; Index < UsbIoHandleCount; Index++) {
    //
    // Check whether the USB IO handle is a child of this bus
    // Note: The usb child handle maybe invalid because of hot plugged out during the loop
    //
    UsbIoDevicePath = NULL;
    Status = gBS->HandleProtocol (UsbIoBuffer[Index], &gEfiDevicePathProtocolGuid, (VOID *) &UsbIoDevicePath);
    if (EFI_ERROR (Status) || UsbIoDevicePath == NULL) {
      continue;
    }
    if (EfiCompareMem (
            UsbIoDevicePath,
            Bus->DevicePath,
            (EfiDevicePathSize (Bus->DevicePath) - sizeof (EFI_DEVICE_PATH_PROTOCOL))
            ) != 0) {
      continue;
    }
    
    //
    // Get the child Usb IO interface
    //
    Status = gBS->HandleProtocol(
                     UsbIoBuffer[Index],  
                     &gEfiUsbIoProtocolGuid, 
                     &UsbIo
                     );
    if (EFI_ERROR (Status)) {
      continue;
    }
    UsbIf   = USB_INTERFACE_FROM_USBIO (UsbIo);
    
    if (UsbBusIsWantedUsbIO (Bus, UsbIf)) {
      if (!UsbIf->IsManaged) {
        //
        // Recursively connect the wanted Usb Io handle
        //
        USB_DEBUG (("UsbConnectDriver: TPL before connect is %d\n", UsbGetCurrentTpl ()));
        Status            = gBS->ConnectController (UsbIf->Handle, NULL, NULL, TRUE);
        UsbIf->IsManaged  = (BOOLEAN)!EFI_ERROR (Status);
        USB_DEBUG (("UsbConnectDriver: TPL after connect is %d\n", UsbGetCurrentTpl()));      
      }
    } 
  }
  
  return EFI_SUCCESS;
}
