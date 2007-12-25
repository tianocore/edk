/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Uhci.c
    
Abstract: 

  The UHCI driver model and HC protocol routines.    

Revision History

--*/

#include "Uhci.h"

STATIC
EFI_STATUS
EFIAPI
UhciReset (
  IN EFI_USB_HC_PROTOCOL     *This,
  IN UINT16                  Attributes
  )
/*++
  
  Routine Description:
    Provides software reset for the USB host controller.
  
  Arguments:
    This      : A pointer to the EFI_USB_HC_PROTOCOL instance.  
    Attributes: A bit mask of the reset operation to perform. 
  
  Returns:
    EFI_SUCCESS           : The reset operation succeeded.
    EFI_INVALID_PARAMETER : Attributes is not valid.
    EFI_DEVICE_ERROR      : An error was encountered while attempting 
                            to perform the reset operation.      
--*/
{
  USB_HC_DEV          *Uhc;
  EFI_TPL             OldTpl;

  OldTpl  = gBS->RaiseTPL (UHCI_TPL);
  Uhc     = UHC_FROM_USB_HC_PROTO (This);

  switch (Attributes) {
  case EFI_USB_HC_RESET_GLOBAL:
    //
    // Stop schedule and set the Global Reset bit in the command register
    //
    UhciStopHc (Uhc, UHC_GENERIC_TIMEOUT);
    UhciSetRegBit (Uhc->PciIo, USBCMD_OFFSET, USBCMD_GRESET);

    gBS->Stall (UHC_ROOT_PORT_RESET_STALL);

    //
    // Clear the Global Reset bit to zero.
    //
    UhciClearRegBit (Uhc->PciIo, USBCMD_OFFSET, USBCMD_GRESET);

    gBS->Stall (UHC_ROOT_PORT_RECOVERY_STALL);
    break;

  case EFI_USB_HC_RESET_HOST_CONTROLLER:
    //
    // Stop schedule and set Host Controller Reset bit to 1
    //
    UhciStopHc (Uhc, UHC_GENERIC_TIMEOUT);
    UhciSetRegBit (Uhc->PciIo, USBCMD_OFFSET, USBCMD_HCRESET);

    gBS->Stall (UHC_ROOT_PORT_RECOVERY_STALL);
    break;

  default:
    goto ON_INVAILD_PARAMETER;
  }
  
  //
  // Delete all old transactions on the USB bus, then
  // reinitialize the frame list
  //
  UhciFreeAllAsyncReq (Uhc);
  UhciDestoryFrameList (Uhc);
  UhciInitFrameList (Uhc);

  gBS->RestoreTPL (OldTpl);
  
  return EFI_SUCCESS;

ON_INVAILD_PARAMETER:
  
  gBS->RestoreTPL (OldTpl);
  
  return EFI_INVALID_PARAMETER;
}

STATIC
EFI_STATUS
EFIAPI
UhciGetState (
  IN  EFI_USB_HC_PROTOCOL     *This,
  OUT EFI_USB_HC_STATE        *State
  )
/*++
  
  Routine Description:
    Retrieves current state of the USB host controller.
  
  Arguments:
    This    :  A pointer to the EFI_USB_HC_PROTOCOL instance.
    State   :  A pointer to the EFI_USB_HC_STATE data structure that 
               indicates current state of the USB host controller.            
  Returns:
    EFI_SUCCESS           : State was returned
    EFI_INVALID_PARAMETER : State is NULL.
    EFI_DEVICE_ERROR      : An error was encountered
            
--*/
{
  USB_HC_DEV          *Uhc;
  UINT16              UsbSts;
  UINT16              UsbCmd;

  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Uhc     = UHC_FROM_USB_HC_PROTO (This);

  UsbCmd  = UhciReadReg (Uhc->PciIo, USBCMD_OFFSET);
  UsbSts  = UhciReadReg (Uhc->PciIo, USBSTS_OFFSET);

  if (UsbCmd & USBCMD_EGSM) {
    *State = EfiUsbHcStateSuspend;

  } else if ((UsbSts & USBSTS_HCH) != 0) {
    *State = EfiUsbHcStateHalt;

  } else {
    *State = EfiUsbHcStateOperational;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
UhciSetState (
  IN EFI_USB_HC_PROTOCOL     *This,
  IN EFI_USB_HC_STATE        State
  )
/*++
  
  Routine Description:
    Sets the USB host controller to a specific state.
  
  Arguments:
    This     : A pointer to the EFI_USB_HC_PROTOCOL instance.
    State    : Indicates the state of the host controller that will be set.
  
  Returns:
    EFI_SUCCESS           : The USB host controller was successfully set
    EFI_INVALID_PARAMETER : State is invalid.
    EFI_DEVICE_ERROR      : Failed to set the state specified 
          
--*/
{
  EFI_USB_HC_STATE    CurState;
  USB_HC_DEV          *Uhc;
  EFI_TPL             OldTpl;
  EFI_STATUS          Status;
  UINT16              UsbCmd;

  Uhc     = UHC_FROM_USB_HC_PROTO (This);
  Status  = UhciGetState (This, &CurState);

  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  if (CurState == State) {
    return EFI_SUCCESS;
  }

  Status  = EFI_SUCCESS;
  OldTpl  = gBS->RaiseTPL (UHCI_TPL);

  switch (State) {
  case EfiUsbHcStateHalt:
    Status = UhciStopHc (Uhc, UHC_GENERIC_TIMEOUT);
    break;

  case EfiUsbHcStateOperational:
    UsbCmd = UhciReadReg (Uhc->PciIo, USBCMD_OFFSET);

    if (CurState == EfiUsbHcStateHalt) {
      //
      // Set Run/Stop bit to 1, also set the bandwidht reclamation
      // point to 64 bytes
      //
      UsbCmd |= USBCMD_RS | USBCMD_MAXP;
      UhciWriteReg (Uhc->PciIo, USBCMD_OFFSET, UsbCmd);

    } else if (CurState == EfiUsbHcStateSuspend) {
      //
      // If FGR(Force Global Resume) bit is 0, set it
      //
      if ((UsbCmd & USBCMD_FGR) == 0) {
        UsbCmd |= USBCMD_FGR;
        UhciWriteReg (Uhc->PciIo, USBCMD_OFFSET, UsbCmd);
      }
      
      //
      // wait 20ms to let resume complete (20ms is specified by UHCI spec)
      //
      gBS->Stall (UHC_FORCE_GLOBAL_RESUME_STALL);

      //
      // Write FGR bit to 0 and EGSM(Enter Global Suspend Mode) bit to 0
      //
      UsbCmd &= ~USBCMD_FGR;
      UsbCmd &= ~USBCMD_EGSM;
      UsbCmd |= USBCMD_RS;
      UhciWriteReg (Uhc->PciIo, USBCMD_OFFSET, UsbCmd);
    }

    break;

  case EfiUsbHcStateSuspend:
    Status = UhciSetState (This, EfiUsbHcStateHalt);

    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto ON_EXIT;
    }
    
    //
    // Set Enter Global Suspend Mode bit to 1.
    //
    UsbCmd = UhciReadReg (Uhc->PciIo, USBCMD_OFFSET);
    UsbCmd |= USBCMD_EGSM;
    UhciWriteReg (Uhc->PciIo, USBCMD_OFFSET, UsbCmd);
    break;

  default:
    Status = EFI_INVALID_PARAMETER;
    break;
  }

ON_EXIT:
  gBS->RestoreTPL (OldTpl);
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
UhciGetRootHubPortNumber (
  IN  EFI_USB_HC_PROTOCOL     *This,
  OUT UINT8                   *PortNumber
  )
/*++
  
  Routine Description:
  
    Retrieves the number of root hub ports.
    
  Arguments:
  
    This       : A pointer to the EFI_USB_HC_PROTOCOL instance.
    PortNumber : A pointer to the number of the root hub ports.
  
  Returns:
  
    EFI_SUCCESS           : The port number was retrieved successfully.
    EFI_INVALID_PARAMETER : PortNumber is NULL.
    EFI_DEVICE_ERROR      : An error was encountered 
          
--*/
{
  USB_HC_DEV          *Uhc;
  UINT32              Offset;
  UINT16              PortSC;
  UINT32              Index;

  Uhc = UHC_FROM_USB_HC_PROTO (This);

  if (PortNumber == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *PortNumber = 0;

  for (Index = 0; Index < USB_MAX_ROOTHUB_PORT; Index++) {
    Offset  = USBPORTSC_OFFSET + Index * 2;
    PortSC  = UhciReadReg (Uhc->PciIo, Offset);

    //
    // Port status's bit 7 is reserved and always returns 1 if
    // the port number is valid. Intel's UHCI (in EHCI controller)
    // returns 0 in this bit if port number is invalid. Also, if 
    // PciIo IoRead returns error, 0xFFFF is returned to caller.
    //
    if (((PortSC & 0x80) != 0) && (PortSC != 0xFFFF)) {
      (*PortNumber)++;
    }
  }

  Uhc->RootPorts = *PortNumber;

  UHCI_DEBUG (("UhciGetRootHubPortNumber: %d ports\n", Uhc->RootPorts));
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UhciGetRootHubPortStatus (
  IN  EFI_USB_HC_PROTOCOL     *This,
  IN  UINT8                   PortNumber,
  OUT EFI_USB_PORT_STATUS     *PortStatus
  )
/*++
  
  Routine Description:
  
    Retrieves the current status of a USB root hub port.
  
  Arguments:
  
    This        : A pointer to the EFI_USB_HC_PROTOCOL.
    PortNumber  : Specifies the root hub port. This value is zero-based.
    PortStatus  : A pointer to the current port status bits and port status change bits.  
  
  Returns:
  
    EFI_SUCCESS           : The port status was returned in PortStatus.
    EFI_INVALID_PARAMETER : PortNumber is invalid. 
    EFI_DEVICE_ERROR      : Can't read register      
    
--*/
{
  USB_HC_DEV          *Uhc;
  UINT32              Offset;
  UINT16              PortSC;

  Uhc = UHC_FROM_USB_HC_PROTO (This);

  if (PortStatus == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (PortNumber >= Uhc->RootPorts) {
    return EFI_INVALID_PARAMETER;
  }

  Offset                        = USBPORTSC_OFFSET + PortNumber * 2;
  PortStatus->PortStatus        = 0;
  PortStatus->PortChangeStatus  = 0;

  PortSC                        = UhciReadReg (Uhc->PciIo, Offset);

  if (PortSC & USBPORTSC_CCS) {
    PortStatus->PortStatus |= USB_PORT_STAT_CONNECTION;
  }

  if (PortSC & USBPORTSC_PED) {
    PortStatus->PortStatus |= USB_PORT_STAT_ENABLE;
  }

  if (PortSC & USBPORTSC_SUSP) {
    UHCI_DEBUG (("UhciGetRootHubPortStatus: port %d is suspended\n", PortNumber));
    PortStatus->PortStatus |= USB_PORT_STAT_SUSPEND;
  }

  if (PortSC & USBPORTSC_PR) {
    PortStatus->PortStatus |= USB_PORT_STAT_RESET;
  }

  if (PortSC & USBPORTSC_LSDA) {
    PortStatus->PortStatus |= USB_PORT_STAT_LOW_SPEED;
  }
  
  //
  // CHC will always return one in port owner bit
  //
  PortStatus->PortStatus |= USB_PORT_STAT_OWNER;

  if (PortSC & USBPORTSC_CSC) {
    PortStatus->PortChangeStatus |= USB_PORT_STAT_C_CONNECTION;
  }

  if (PortSC & USBPORTSC_PEDC) {
    PortStatus->PortChangeStatus |= USB_PORT_STAT_C_ENABLE;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
UhciSetRootHubPortFeature (
  IN  EFI_USB_HC_PROTOCOL     *This,
  IN  UINT8                   PortNumber,
  IN  EFI_USB_PORT_FEATURE    PortFeature
  )
/*++
  
  Routine Description:
  
    Sets a feature for the specified root hub port.
  
  Arguments:
  
    This        : A pointer to the EFI_USB_HC_PROTOCOL.
    PortNumber  : Specifies the root hub port whose feature 
                  is requested to be set.
    PortFeature : Indicates the feature selector associated 
                  with the feature set request. 
  
  Returns:
  
    EFI_SUCCESS           : The feature was set for the port.
    EFI_INVALID_PARAMETER : PortNumber is invalid or PortFeature is invalid.
    EFI_DEVICE_ERROR      : Can't read register
        
--*/
{
  USB_HC_DEV          *Uhc;
  EFI_TPL             OldTpl;
  UINT32              Offset;
  UINT16              PortSC;
  UINT16              Command;

  Uhc = UHC_FROM_USB_HC_PROTO (This);

  if (PortNumber >= Uhc->RootPorts) {
    return EFI_INVALID_PARAMETER;
  }

  Offset  = USBPORTSC_OFFSET + PortNumber * 2;

  OldTpl  = gBS->RaiseTPL (UHCI_TPL);
  PortSC  = UhciReadReg (Uhc->PciIo, Offset);

  switch (PortFeature) {
  case EfiUsbPortSuspend:
    Command = UhciReadReg (Uhc->PciIo, USBCMD_OFFSET);
    if (!(Command & USBCMD_EGSM)) {
      //
      // if global suspend is not active, can set port suspend
      //
      PortSC &= 0xfff5;
      PortSC |= USBPORTSC_SUSP;
    }
    break;

  case EfiUsbPortReset:
    PortSC &= 0xfff5;
    PortSC |= USBPORTSC_PR;
    break;

  case EfiUsbPortPower:
    //
    // No action
    //
    break;

  case EfiUsbPortEnable:
    PortSC &= 0xfff5;
    PortSC |= USBPORTSC_PED;
    break;

  default:
    gBS->RestoreTPL (OldTpl);
    return EFI_INVALID_PARAMETER;
  }

  UhciWriteReg (Uhc->PciIo, Offset, PortSC);
  gBS->RestoreTPL (OldTpl);
  
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
UhciClearRootHubPortFeature (
  IN  EFI_USB_HC_PROTOCOL     *This,
  IN  UINT8                   PortNumber,
  IN  EFI_USB_PORT_FEATURE    PortFeature
  )
/*++
  
  Routine Description:
  
    Clears a feature for the specified root hub port.
  
  Arguments:
  
    This        : A pointer to the EFI_USB_HC_PROTOCOL instance.
    PortNumber  : Specifies the root hub port whose feature 
                  is requested to be cleared.
    PortFeature : Indicates the feature selector associated with the 
                  feature clear request.
                  
  Returns:
  
    EFI_SUCCESS           : The feature was cleared for the port.
    EFI_INVALID_PARAMETER : PortNumber is invalid or PortFeature is invalid.
    EFI_DEVICE_ERROR      : Can't read register
        
--*/
{
  USB_HC_DEV          *Uhc;
  EFI_TPL             OldTpl;
  UINT32              Offset;
  UINT16              PortSC;

  Uhc = UHC_FROM_USB_HC_PROTO (This);

  if (PortNumber >= Uhc->RootPorts) {
    return EFI_INVALID_PARAMETER;
  }

  Offset  = USBPORTSC_OFFSET + PortNumber * 2;

  OldTpl  = gBS->RaiseTPL (UHCI_TPL);
  PortSC  = UhciReadReg (Uhc->PciIo, Offset);

  switch (PortFeature) {
  case EfiUsbPortEnable:
    PortSC &= 0xfff5;
    PortSC &= ~USBPORTSC_PED;
    break;

  case EfiUsbPortSuspend:
    //
    // Cause a resume on the specified port if in suspend mode.
    //
    PortSC &= 0xfff5;
    PortSC &= ~USBPORTSC_SUSP;
    break;

  case EfiUsbPortPower:
    //
    // No action
    //
    break;

  case EfiUsbPortReset:
    PortSC &= 0xfff5;
    PortSC &= ~USBPORTSC_PR;
    break;

  case EfiUsbPortConnectChange:
    PortSC &= 0xfff5;
    PortSC |= USBPORTSC_CSC;
    break;

  case EfiUsbPortEnableChange:
    PortSC &= 0xfff5;
    PortSC |= USBPORTSC_PEDC;
    break;

  case EfiUsbPortSuspendChange:
    //
    // Root hub does not support this
    //
    break;

  case EfiUsbPortOverCurrentChange:
    //
    // Root hub does not support this
    //
    break;

  case EfiUsbPortResetChange:
    //
    // Root hub does not support this
    //
    break;

  default:
    gBS->RestoreTPL (OldTpl);
    return EFI_INVALID_PARAMETER;
  }

  UhciWriteReg (Uhc->PciIo, Offset, PortSC);
  gBS->RestoreTPL (OldTpl);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
UhciControlTransfer (
  IN       EFI_USB_HC_PROTOCOL        *This,
  IN       UINT8                      DeviceAddress,
  IN       BOOLEAN                    IsSlowDevice,
  IN       UINT8                      MaximumPacketLength,
  IN       EFI_USB_DEVICE_REQUEST     *Request,
  IN       EFI_USB_DATA_DIRECTION     TransferDirection,
  IN OUT   VOID                       *Data,              OPTIONAL
  IN OUT   UINTN                      *DataLength,        OPTIONAL
  IN       UINTN                      TimeOut,
  OUT      UINT32                     *TransferResult
  )
/*++
  
  Routine Description:
  
    Submits control transfer to a target USB device.
  
  Arguments:
    
    This                : A pointer to the EFI_USB_HC_PROTOCOL instance.
    DeviceAddress       : Usb device address
    IsSlowDevice        : Whether the device is of slow speed or full speed
    MaximumPacketLength : maximum packet size of the default control endpoint
    Request             : USB device request to send
    TransferDirection   : Specifies the data direction for the transfer.
    Data                : Data buffer to transmit from or receive into 
    DataLength          : Number of bytes of the data
    TimeOut             : Maximum time, in microseconds
    TransferResult      : Return result in this
    
  Returns:
    EFI_SUCCESS           : Transfer was completed successfully.
    EFI_OUT_OF_RESOURCES  : Failed due to a lack of resources.
    EFI_INVALID_PARAMETER : Some parameters are invalid.
    EFI_TIMEOUT           : Failed due to timeout.
    EFI_DEVICE_ERROR      : Failed due to host controller or device error. 
--*/
{
  USB_HC_DEV              *Uhc;
  UHCI_TD_SW              *TDs;
  EFI_TPL                 OldTpl;
  EFI_STATUS              Status;
  UHCI_QH_RESULT          QhResult;
  UINT8                   PktId;
  UINT8                   *RequestPhy;
  VOID                    *RequestMap;
  UINT8                   *DataPhy;
  VOID                    *DataMap;

  Uhc         = UHC_FROM_USB_HC_PROTO (This);
  TDs         = NULL;
  DataPhy     = NULL;
  DataMap     = NULL;
  RequestPhy  = NULL;
  RequestMap  = NULL;

  //
  // Parameters Checking
  //
  if (Request == NULL || TransferResult == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsSlowDevice && (MaximumPacketLength != 8)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((MaximumPacketLength != 8) &&  (MaximumPacketLength != 16) &&
      (MaximumPacketLength != 32) && (MaximumPacketLength != 64)) {
      
    return EFI_INVALID_PARAMETER;
  }

  if ((TransferDirection != EfiUsbNoData) && (DataLength == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *TransferResult = EFI_USB_ERR_SYSTEM;
  Status          = EFI_DEVICE_ERROR;

  //
  // If errors exist that cause host controller halt,
  // clear status then return EFI_DEVICE_ERROR.
  //
  UhciAckAllInterrupt (Uhc);

  if (!UhciIsHcWorking (Uhc->PciIo)) {
    return EFI_DEVICE_ERROR;
  }

  OldTpl = gBS->RaiseTPL (UHCI_TPL);

  //
  // Map the Request and data for bus master access, 
  // then create a list of TD for this transfer
  //
  Status = UhciMapUserRequest (Uhc, Request, &RequestPhy, &RequestMap);

  if (EFI_ERROR (Status)) {
    goto ON_EXIT;
  }

  Status = UhciMapUserData (Uhc, TransferDirection, Data, DataLength, &PktId, &DataPhy, &DataMap);

  if (EFI_ERROR (Status)) {
    Uhc->PciIo->Unmap (Uhc->PciIo, RequestMap);
    goto ON_EXIT;
  }
  
  TDs = UhciCreateCtrlTds (
          Uhc,
          DeviceAddress,
          PktId,
          RequestPhy,
          DataPhy,
          *DataLength,
          MaximumPacketLength,
          IsSlowDevice
          );

  if (TDs == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto UNMAP_DATA;
  }

  //
  // According to the speed of the end point, link 
  // the TD to corrosponding queue head, then check
  // the execution result
  //
  UhciLinkTdToQh (Uhc->CtrlQh, TDs);
  Status = UhciExecuteTransfer (Uhc, Uhc->CtrlQh, TDs, TimeOut, IsSlowDevice, &QhResult);
  UhciUnlinkTdFromQh (Uhc->CtrlQh, TDs);

  Uhc->PciIo->Flush (Uhc->PciIo);

  *TransferResult = QhResult.Result;
 
  if (DataLength != NULL) {
    *DataLength = QhResult.Complete;
  }
  
  UhciDestoryTds (Uhc, TDs);

UNMAP_DATA:
  Uhc->PciIo->Unmap (Uhc->PciIo, DataMap);
  Uhc->PciIo->Unmap (Uhc->PciIo, RequestMap);

ON_EXIT:
  gBS->RestoreTPL (OldTpl);
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
UhciBulkTransfer (
  IN       EFI_USB_HC_PROTOCOL     *This,
  IN       UINT8                   DeviceAddress,
  IN       UINT8                   EndPointAddress,
  IN       UINT8                   MaximumPacketLength,
  IN OUT   VOID                    *Data,
  IN OUT   UINTN                   *DataLength,
  IN OUT   UINT8                   *DataToggle,
  IN       UINTN                   TimeOut,
  OUT      UINT32                  *TransferResult
  )
/*++
  
  Routine Description:
  
    Submits bulk transfer to a bulk endpoint of a USB device.
    
  Arguments:
    
    This                :A pointer to the EFI_USB_HC_PROTOCOL instance.
    DeviceAddress       : Usb device address
    EndPointAddress     : Endpoint number and endpoint direction
    MaximumPacketLength : Maximum packet size of the target endpoint
    Data                : Data buffer to transmit from or receive into
    DataLength          : Length of the data buffer
    DataToggle          : On input, data toggle to use, on output, the next toggle
    TimeOut             : Indicates the maximum time
    TransferResult      : Variable to receive the transfer result
    
  Returns:
    EFI_SUCCESS           : The bulk transfer was completed successfully.
    EFI_OUT_OF_RESOURCES  : Failed due to lack of resource.
    EFI_INVALID_PARAMETER : Some parameters are invalid.
    EFI_TIMEOUT           : Failed due to timeout.
    EFI_DEVICE_ERROR      : Failed due to host controller or device error.

--*/
{
  EFI_USB_DATA_DIRECTION  Direction;
  EFI_TPL                 OldTpl;
  USB_HC_DEV              *Uhc;
  UHCI_TD_SW              *TDs;
  UHCI_QH_SW              *BulkQh;
  UHCI_QH_RESULT          QhResult;
  EFI_STATUS              Status;
  UINT8                   PktId;
  UINT8                   *DataPhy;
  VOID                    *DataMap;

  Uhc     = UHC_FROM_USB_HC_PROTO (This);
  DataPhy = NULL;
  DataMap = NULL;

  if ((DataLength == NULL) || (Data == NULL) || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (*DataLength == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 1) && (*DataToggle != 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((MaximumPacketLength != 8) && (MaximumPacketLength != 16) && 
      (MaximumPacketLength != 32) && (MaximumPacketLength != 64)) {
    return EFI_INVALID_PARAMETER;
  }
  
  *TransferResult = EFI_USB_ERR_SYSTEM;
  Status          = EFI_OUT_OF_RESOURCES;

  //
  // If has errors that cause host controller halt,
  // then return EFI_DEVICE_ERROR directly.
  //
  UhciAckAllInterrupt (Uhc);

  if (!UhciIsHcWorking (Uhc->PciIo)) {
    return EFI_DEVICE_ERROR;
  }

  OldTpl = gBS->RaiseTPL (UHCI_TPL);

  //
  // Map the source data buffer for bus master access,
  // then create a list of TDs
  //
  if (EndPointAddress & 0x80) {
    Direction = EfiUsbDataIn;
  } else {
    Direction = EfiUsbDataOut;
  }
  
  Status = UhciMapUserData (Uhc, Direction, Data, DataLength, &PktId, &DataPhy, &DataMap);

  if (EFI_ERROR (Status)) {
    goto ON_EXIT;
  }

  Status = EFI_OUT_OF_RESOURCES;
  TDs    = UhciCreateBulkOrIntTds (
             Uhc,
             DeviceAddress,
             EndPointAddress,
             PktId,
             DataPhy,
             *DataLength,
             DataToggle,
             MaximumPacketLength,
             FALSE
             );

  if (TDs == NULL) { 
    Uhc->PciIo->Unmap (Uhc->PciIo, DataMap);
    goto ON_EXIT;
  }

 
  //
  // Link the TDs to bulk queue head. According to the platfore
  // defintion of UHCI_NO_BW_RECLAMATION, BulkQh is either configured
  // to do full speed bandwidth reclamation or not.
  //
  BulkQh = Uhc->BulkQh;

  UhciLinkTdToQh (BulkQh, TDs);
  Status = UhciExecuteTransfer (Uhc, BulkQh, TDs, TimeOut, FALSE, &QhResult);
  UhciUnlinkTdFromQh (BulkQh, TDs);
 
  Uhc->PciIo->Flush (Uhc->PciIo);

  *TransferResult = QhResult.Result;
  *DataToggle     = QhResult.NextToggle;
  *DataLength     = QhResult.Complete;

  UhciDestoryTds (Uhc, TDs);
  Uhc->PciIo->Unmap (Uhc->PciIo, DataMap);

ON_EXIT:
  gBS->RestoreTPL (OldTpl);
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
UhciAsyncInterruptTransfer (
  IN     EFI_USB_HC_PROTOCOL                * This,
  IN     UINT8                              DeviceAddress,
  IN     UINT8                              EndPointAddress,
  IN     BOOLEAN                            IsSlowDevice,
  IN     UINT8                              MaximumPacketLength,
  IN     BOOLEAN                            IsNewTransfer,
  IN OUT UINT8                              *DataToggle,
  IN     UINTN                              PollingInterval,    OPTIONAL
  IN     UINTN                              DataLength,         OPTIONAL
  IN     EFI_ASYNC_USB_TRANSFER_CALLBACK    CallBackFunction,   OPTIONAL
  IN     VOID                               *Context OPTIONAL
  )
/*++
  
  Routine Description:
  
    Submits an asynchronous interrupt transfer to an interrupt endpoint of a USB device.
  
  Arguments:
    
    This                : A pointer to the EFI_USB_HC_PROTOCOL instance.
    DeviceAddress       : Target device address
    EndPointAddress     : Endpoint number with direction
    IsSlowDevice        : Whether the target device is slow device or full-speed device.                
    MaximumPacketLength : Maximum packet size of the target endpoint
    IsNewTransfer       : If TRUE, submit a new async interrupt transfer, otherwise
                          cancel an existed one 
    DataToggle          : On input, the data toggle to use; On output, next data toggle
    PollingInterval     : Interrupt poll rate in milliseconds              
    DataLength          : Length of data to receive
    CallBackFunction    : Function to call periodically
    Context             : User context
  
  Returns:
  
    EFI_SUCCESS           : Request is submitted or cancelled
    EFI_INVALID_PARAMETER : Some parameters are invalid.
    EFI_OUT_OF_RESOURCES  : Failed due to a lack of resources.  
    EFI_DEVICE_ERROR      : Failed to due to device error
        
--*/
{
  USB_HC_DEV          *Uhc;
  UHCI_QH_SW          *Qh;
  UHCI_TD_SW          *IntTds;
  EFI_TPL             OldTpl;
  EFI_STATUS          Status;
  UINT8               *DataPtr;
  UINT8               *DataPhy;
  VOID                *DataMap;
  UINT8               PktId;

  Uhc       = UHC_FROM_USB_HC_PROTO (This);
  Qh        = NULL;
  IntTds    = NULL;
  DataPtr   = NULL;
  DataPhy   = NULL;
  DataMap   = NULL;

  if ((EndPointAddress & 0x80) == 0) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Delete Async interrupt transfer request
  //
  if (!IsNewTransfer) {
    OldTpl = gBS->RaiseTPL (UHCI_TPL);
    Status = UhciRemoveAsyncReq (Uhc, DeviceAddress, EndPointAddress, DataToggle);

    gBS->RestoreTPL (OldTpl);
    return Status;
  }

  if (PollingInterval < 1 || PollingInterval > 255) {
    return EFI_INVALID_PARAMETER;
  }

  if (DataLength == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 1) && (*DataToggle != 0)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If has errors that cause host controller halt,
  // then return EFI_DEVICE_ERROR directly.
  //
  UhciAckAllInterrupt (Uhc);

  if (!UhciIsHcWorking (Uhc->PciIo)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Allocate and map source data buffer for bus master access.
  //
  DataPtr = EfiLibAllocatePool (DataLength);

  if (DataPtr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  OldTpl = gBS->RaiseTPL (UHCI_TPL);
  
  //
  // Map the user data then create a queue head and 
  // list of TD for it.
  //
  Status = UhciMapUserData (
             Uhc, 
             EfiUsbDataIn, 
             DataPtr, 
             &DataLength, 
             &PktId, 
             &DataPhy, 
             &DataMap
             );

  if (EFI_ERROR (Status)) {
    goto FREE_DATA;
  }
  
  Qh = UhciCreateQh (Uhc, PollingInterval);

  if (Qh == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto UNMAP_DATA;
  }

  IntTds = UhciCreateBulkOrIntTds (
             Uhc,
             DeviceAddress,
             EndPointAddress,
             PktId,
             DataPhy,
             DataLength,
             DataToggle,
             MaximumPacketLength,
             IsSlowDevice
             );

  if (IntTds == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto DESTORY_QH;
  }
  
  UhciLinkTdToQh (Qh, IntTds);

  //
  // Save QH-TD structures to async Interrupt transfer list,
  // for monitor interrupt transfer execution routine use.
  //  
  Status = UhciCreateAsyncReq (
             Uhc,
             Qh,
             IntTds,
             DeviceAddress,
             EndPointAddress,
             DataLength,
             PollingInterval,
             DataMap,
             DataPtr,
             CallBackFunction,
             Context,
             IsSlowDevice
             );

  if (EFI_ERROR (Status)) {
    goto DESTORY_QH;
  }

  UhciLinkQhToFrameList (Uhc->FrameBase, Qh);
  
  gBS->RestoreTPL (OldTpl);
  return EFI_SUCCESS;

DESTORY_QH:
  UsbHcFreeMem (Uhc->MemPool, Qh, sizeof (UHCI_QH_SW));

UNMAP_DATA:
  Uhc->PciIo->Unmap (Uhc->PciIo, DataMap);

FREE_DATA:
  gBS->FreePool (DataPtr);
  Uhc->PciIo->Flush (Uhc->PciIo);

  gBS->RestoreTPL (OldTpl);
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
UhciSyncInterruptTransfer (
  IN       EFI_USB_HC_PROTOCOL     *This,
  IN       UINT8                   DeviceAddress,
  IN       UINT8                   EndPointAddress,
  IN       BOOLEAN                 IsSlowDevice,
  IN       UINT8                   MaximumPacketLength,
  IN OUT   VOID                    *Data,
  IN OUT   UINTN                   *DataLength,
  IN OUT   UINT8                   *DataToggle,
  IN       UINTN                   TimeOut,
  OUT      UINT32                  *TransferResult
  )
/*++
  
  Routine Description:
  
    Submits synchronous interrupt transfer to an interrupt endpoint of a USB device.
  
  Arguments:
    
    This                : A pointer to the EFI_USB_HC_PROTOCOL instance.
    DeviceAddress       : Device address of the target USB device
    EndPointAddress     : Endpoint number and direction        
    IsSlowDevice        : Whether the target device is of slow speed or full speed
    MaximumPacketLength : Maximum packet size of target endpoint 
    Data                : Data to transmit or receive 
    DataLength          : On input, data length to transmit or buffer size.
                          On output, the number of bytes transferred.                
    DataToggle          : On input, data toggle to use; On output, next data toggle
    TimeOut             : Maximum time, in microseconds, transfer is allowed to complete.               
    TransferResult      : Variable to receive transfer result
    
  Returns:
    EFI_SUCCESS           : Transfer was completed successfully.
    EFI_OUT_OF_RESOURCES  : Failed due to lack of resource.
    EFI_INVALID_PARAMETER : Some parameters are invalid.
    EFI_TIMEOUT           : Failed due to timeout.
    EFI_DEVICE_ERROR      : Failed due to host controller or device error

--*/
{
  EFI_STATUS          Status;
  USB_HC_DEV          *Uhc;
  UHCI_TD_SW          *TDs;
  UHCI_QH_RESULT      QhResult;
  EFI_TPL             OldTpl;
  UINT8               *DataPhy;
  VOID                *DataMap;
  UINT8               PktId;

  Uhc     = UHC_FROM_USB_HC_PROTO (This);
  DataPhy = NULL;
  DataMap = NULL;
  TDs     = NULL;

  if ((DataLength == NULL) || (Data == NULL) || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((EndPointAddress & 0x80) == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 1) && (*DataToggle != 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataLength == 0) || (MaximumPacketLength > 64)) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsSlowDevice && (MaximumPacketLength > 8)) {
    return EFI_INVALID_PARAMETER;
  }

  *TransferResult = EFI_USB_ERR_SYSTEM;
  Status          = EFI_DEVICE_ERROR;
  

  UhciAckAllInterrupt (Uhc);

  if (!UhciIsHcWorking (Uhc->PciIo)) {
    return Status;
  }

  OldTpl = gBS->RaiseTPL (UHCI_TPL);

  //
  // Map the source data buffer for bus master access.
  // Create Tds list, then link it to the UHC's interrupt list
  //
  Status = UhciMapUserData (
             Uhc,
             EfiUsbDataIn,
             Data,
             DataLength,
             &PktId,
             &DataPhy,
             &DataMap
             );

  if (EFI_ERROR (Status)) {
    goto ON_EXIT;
  }
  
  TDs = UhciCreateBulkOrIntTds (
          Uhc,
          DeviceAddress,
          EndPointAddress,
          PktId,
          DataPhy,
          *DataLength,
          DataToggle,
          MaximumPacketLength,
          IsSlowDevice
          );

  if (TDs == NULL) {
    Uhc->PciIo->Unmap (Uhc->PciIo, DataMap);

    Status = EFI_OUT_OF_RESOURCES;
    goto ON_EXIT;
  }


  UhciLinkTdToQh (Uhc->SyncIntQh, TDs);

  Status = UhciExecuteTransfer (Uhc, Uhc->SyncIntQh, TDs, TimeOut, IsSlowDevice, &QhResult);
  
  UhciUnlinkTdFromQh (Uhc->SyncIntQh, TDs);
  Uhc->PciIo->Flush (Uhc->PciIo);
  
  *TransferResult = QhResult.Result;
  *DataToggle     = QhResult.NextToggle;
  *DataLength     = QhResult.Complete;

  UhciDestoryTds (Uhc, TDs);
  Uhc->PciIo->Unmap (Uhc->PciIo, DataMap);

ON_EXIT:
  gBS->RestoreTPL (OldTpl);
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
UhciIsochronousTransfer (
  IN       EFI_USB_HC_PROTOCOL     *This,
  IN       UINT8                   DeviceAddress,
  IN       UINT8                   EndPointAddress,
  IN       UINT8                   MaximumPacketLength,
  IN OUT   VOID                    *Data,
  IN       UINTN                   DataLength,
  OUT      UINT32                  *TransferResult
  )
/*++
  
  Routine Description:
  
    Submits isochronous transfer to a target USB device.
  
  Arguments:
    
    This                : A pointer to the EFI_USB_HC_PROTOCOL instance.
    DeviceAddress       : Target device address
    EndPointAddress     : End point address withdirection
    MaximumPacketLength : Maximum packet size of the endpoint
    Data                : Data to transmit or receive
    DataLength          : Bytes of the data
    TransferResult      : Variable to receive the result
    
  Returns:
  
    EFI_UNSUPPORTED 

--*/
{
  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
UhciAsyncIsochronousTransfer (
  IN       EFI_USB_HC_PROTOCOL                 * This,
  IN       UINT8                               DeviceAddress,
  IN       UINT8                               EndPointAddress,
  IN       UINT8                               MaximumPacketLength,
  IN OUT   VOID                                *Data,
  IN       UINTN                               DataLength,
  IN       EFI_ASYNC_USB_TRANSFER_CALLBACK     IsochronousCallBack,
  IN       VOID                                *Context OPTIONAL
  )
/*++
  
  Routine Description:
  
    Submits Async isochronous transfer to a target USB device.
  
  Arguments:
    
    This                : A pointer to the EFI_USB_HC_PROTOCOL instance.
    DeviceAddress       : Target device address
    EndPointAddress     : End point address withdirection
    MaximumPacketLength : Maximum packet size of the endpoint
    Data                : Data to transmit or receive
    IsochronousCallBack : Function to call when the transfer completes
    Context             : User context
                    
  Returns:
  
    EFI_UNSUPPORTED 

--*/

{
  return EFI_UNSUPPORTED;
}


STATIC
EFI_STATUS
EFIAPI
Uhci2Reset (
  IN EFI_USB2_HC_PROTOCOL   *This,
  IN UINT16                 Attributes
  )
/*++

  Routine Description:
  
    Provides software reset for the USB host controller according to UEFI 2.0 spec.

  Arguments:
  
    This           - A pointer to the EFI_USB2_HC_PROTOCOL instance.
    Attributes     - A bit mask of the reset operation to perform. 
                       See below for a list of the supported bit mask values.
  Returns:
  
    EFI_SUCCESS           : The reset operation succeeded.
    EFI_INVALID_PARAMETER : Attributes is not valid.
    EFI_UNSUPPORTED       : This type of reset is not currently supported
    EFI_DEVICE_ERROR      : Other errors
--*/
{
  USB_HC_DEV          *UhciDev;

  UhciDev = UHC_FROM_USB2_HC_PROTO (This);

  if ((Attributes == EFI_USB_HC_RESET_GLOBAL_WITH_DEBUG) || 
      (Attributes == EFI_USB_HC_RESET_HOST_WITH_DEBUG)) {
    return EFI_UNSUPPORTED;
  }

  return UhciReset (&UhciDev->UsbHc, Attributes);
}

STATIC
EFI_STATUS
EFIAPI
Uhci2GetState (
  IN  EFI_USB2_HC_PROTOCOL   *This,
  OUT EFI_USB_HC_STATE       *State
  )
/*++
  
  Routine Description:
  
    Retrieves current state of the USB host controller according to UEFI 2.0 spec.
  
  Arguments:
    
    This     - A pointer to the EFI_USB_HC_PROTOCOL instance.
    State    - Variable to receive current device state
 
  Returns:
  
    EFI_SUCCESS           : The state is returned
    EFI_INVALID_PARAMETER : State is not valid.
    EFI_DEVICE_ERROR      : Other errors
            
--*/
{
  USB_HC_DEV          *Uhc;

  Uhc = UHC_FROM_USB2_HC_PROTO (This);
  return UhciGetState (&Uhc->UsbHc, State);
}

STATIC
EFI_STATUS
EFIAPI
Uhci2SetState (
  IN EFI_USB2_HC_PROTOCOL    *This,
  IN EFI_USB_HC_STATE        State
  )
/*++
  
  Routine Description:
  
    Sets the USB host controller to a specific state according to UEFI 2.0 spec.
  
  Arguments:
    
    This     - A pointer to the EFI_USB_HC_PROTOCOL instance.
    State    - Indicates the state of the host controller that will be set.
  
  Returns:
  
    EFI_SUCCESS           : Host controller was successfully placed in the state 
    EFI_INVALID_PARAMETER : State is invalid.
    EFI_DEVICE_ERROR      : Failed to set the state
          
--*/
{
  USB_HC_DEV          *Uhc;

  Uhc = UHC_FROM_USB2_HC_PROTO (This);
  return UhciSetState (&Uhc->UsbHc, State);
}

STATIC
EFI_STATUS
EFIAPI
Uhci2GetCapability (
  IN  EFI_USB2_HC_PROTOCOL  *This,
  OUT UINT8                 *MaxSpeed,
  OUT UINT8                 *PortNumber,
  OUT UINT8                 *Is64BitCapable
  )
/*++

  Routine Description:
  
    Retrieves capabilities of USB host controller according to UEFI 2.0 spec.

  Arguments:
  
    This           - A pointer to the EFI_USB2_HC_PROTOCOL instance 
    MaxSpeed       - A pointer to the max speed USB host controller supports.
    PortNumber     - A pointer to the number of root hub ports.
    Is64BitCapable - A pointer to an integer to show whether USB host controller
                     supports 64-bit memory addressing.
  Returns:
  
    EFI_SUCCESS           : capabilities were retrieved successfully.
    EFI_INVALID_PARAMETER : MaxSpeed or PortNumber or Is64BitCapable is NULL.
    EFI_DEVICE_ERROR      : An error was encountered
        
--*/
{
  USB_HC_DEV          *Uhc;

  Uhc = UHC_FROM_USB2_HC_PROTO (This);

  if ((NULL == MaxSpeed) || (NULL == PortNumber) || (NULL == Is64BitCapable)) {
    return EFI_INVALID_PARAMETER;
  }

  *MaxSpeed       = EFI_USB_SPEED_FULL;
  *Is64BitCapable = (UINT8) FALSE;

  return UhciGetRootHubPortNumber (&Uhc->UsbHc, PortNumber);
}

STATIC
EFI_STATUS
EFIAPI
Uhci2GetRootHubPortStatus (
  IN  EFI_USB2_HC_PROTOCOL   *This,
  IN  UINT8                  PortNumber,
  OUT EFI_USB_PORT_STATUS    *PortStatus
  )
/*++
  
  Routine Description:
  
    Retrieves the current status of a USB root hub port according to UEFI 2.0 spec.
  
  Arguments:
  
    This        - A pointer to the EFI_USB2_HC_PROTOCOL.
    PortNumber  - The port to get status
    PortStatus  - A pointer to the current port status bits and 
                  port status change bits.  
  
  Returns:
  
    EFI_SUCCESS           : status of the USB root hub port was returned in PortStatus.
    EFI_INVALID_PARAMETER : PortNumber is invalid. 
    EFI_DEVICE_ERROR      : Can't read register  
    
--*/
{
  USB_HC_DEV          *Uhc;

  Uhc = UHC_FROM_USB2_HC_PROTO (This);

  return UhciGetRootHubPortStatus (&Uhc->UsbHc, PortNumber, PortStatus);
}

STATIC
EFI_STATUS
EFIAPI
Uhci2SetRootHubPortFeature (
  IN EFI_USB2_HC_PROTOCOL    *This,
  IN UINT8                   PortNumber,
  IN EFI_USB_PORT_FEATURE    PortFeature
  )
/*++
  
  Routine Description:
  
    Sets a feature for the specified root hub port according to UEFI 2.0 spec.
  
  Arguments:
  
    This        - A pointer to the EFI_USB2_HC_PROTOCOL.
    PortNumber  - Specifies the root hub port whose feature 
                  is requested to be set.
    PortFeature - Indicates the feature selector associated 
                  with the feature set request. 
  
  Returns:
  
    EFI_SUCCESS           : PortFeature was set for the root port
    EFI_INVALID_PARAMETER : PortNumber is invalid or PortFeature is invalid.
    EFI_DEVICE_ERROR      : Can't read register
        
--*/
{
  USB_HC_DEV          *Uhc;

  Uhc = UHC_FROM_USB2_HC_PROTO (This);

  return UhciSetRootHubPortFeature (&Uhc->UsbHc, PortNumber, PortFeature);
}

STATIC
EFI_STATUS
EFIAPI
Uhci2ClearRootHubPortFeature (
  IN EFI_USB2_HC_PROTOCOL    *This,
  IN UINT8                   PortNumber,
  IN EFI_USB_PORT_FEATURE    PortFeature
  )
/*++
  
  Routine Description:
  
    Clears a feature for the specified root hub port according to Uefi 2.0 spec.
  
  Arguments:
  
    This        - A pointer to the EFI_USB2_HC_PROTOCOL instance.
    PortNumber  - Specifies the root hub port whose feature 
                  is requested to be cleared.
    PortFeature - Indicates the feature selector associated with the 
                  feature clear request.
                  
  Returns:
  
    EFI_SUCCESS           : PortFeature was cleared for the USB root hub port
    EFI_INVALID_PARAMETER : PortNumber is invalid or PortFeature is invalid.
    EFI_DEVICE_ERROR      : Can't read register
        
--*/
{
  USB_HC_DEV          *Uhc;

  Uhc = UHC_FROM_USB2_HC_PROTO (This);

  return UhciClearRootHubPortFeature (&Uhc->UsbHc, PortNumber, PortFeature);
}

STATIC
EFI_STATUS
EFIAPI
Uhci2ControlTransfer (
  IN     EFI_USB2_HC_PROTOCOL                 *This,
  IN     UINT8                                DeviceAddress,
  IN     UINT8                                DeviceSpeed,
  IN     UINTN                                MaximumPacketLength,
  IN     EFI_USB_DEVICE_REQUEST               *Request,
  IN     EFI_USB_DATA_DIRECTION               TransferDirection,
  IN OUT VOID                                 *Data,
  IN OUT UINTN                                *DataLength,
  IN     UINTN                                TimeOut,
  IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR   *Translator,
  OUT    UINT32                               *TransferResult
  )
/*++
  
  Routine Description:
  
    Submits control transfer to a target USB device accroding to UEFI 2.0 spec..
  
  Arguments:
    
    This                : A pointer to the EFI_USB2_HC_PROTOCOL instance.
    DeviceAddress       : Target device address          
    DeviceSpeed         : Device speed
    MaximumPacketLength : Maximum packet size of the target endpoint
    Request             : USB device request to send 
    TransferDirection   : Data direction of the Data stage in control transfer
    Data                : Data to transmit/receive in data stage
    DataLength          : Length of the data
    TimeOut             : Maximum time, in microseconds, for transfer to complete.
    TransferResult      : Variable to receive the transfer result
                    
  Returns:
  
    EFI_SUCCESS           : The control transfer was completed successfully.
    EFI_OUT_OF_RESOURCES  : Failed due to lack of resource. 
    EFI_INVALID_PARAMETER : Some parameters are invalid.
    EFI_TIMEOUT           : Failed due to timeout.
    EFI_DEVICE_ERROR      : Failed due to host controller or device error.

--*/
{
  USB_HC_DEV          *Uhc;
  BOOLEAN             IsSlow;

  Uhc     = UHC_FROM_USB2_HC_PROTO (This);
  IsSlow  = (EFI_USB_SPEED_LOW == DeviceSpeed) ? TRUE : FALSE;

  return UhciControlTransfer (
           &Uhc->UsbHc,
           DeviceAddress,
           IsSlow,
           (UINT8) MaximumPacketLength,
           Request,
           TransferDirection,
           Data,
           DataLength,
           TimeOut,
           TransferResult
           );
}

STATIC
EFI_STATUS
EFIAPI
Uhci2BulkTransfer (
  IN     EFI_USB2_HC_PROTOCOL               *This,
  IN     UINT8                              DeviceAddress,
  IN     UINT8                              EndPointAddress,
  IN     UINT8                              DeviceSpeed,
  IN     UINTN                              MaximumPacketLength,
  IN     UINT8                              DataBuffersNumber,
  IN OUT VOID                               *Data[EFI_USB_MAX_BULK_BUFFER_NUM],
  IN OUT UINTN                              *DataLength,
  IN OUT UINT8                              *DataToggle,
  IN     UINTN                              TimeOut,
  IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
  OUT    UINT32                             *TransferResult
  )
/*++
  
  Routine Description:
  
    Submits bulk transfer to a bulk endpoint of a USB device
    
  Arguments:
    
    This                : A pointer to the EFI_USB2_HC_PROTOCOL instance.
    DeviceAddress       : Target device address          
    EndPointAddress     : Endpoint number and direction             
    DeviceSpeed         : Device speed
    MaximumPacketLength : Maximum packet size of the target endpoint
    DataBuffersNumber   : Number of data buffers prepared for the transfer.
    Data                : Array of pointers to the buffers of data        
    DataLength          : On input, size of the data buffer, On output, 
                          actually transferred data size.             
    DataToggle          : On input, data toggle to use; On output, next data toggle
    Translator          : A pointr to the transaction translator data.
    TimeOut             : Maximum time out, in microseconds       
    TransferResult      : Variable to receive transfer result
    
  Returns:
    EFI_SUCCESS           : The bulk transfer was completed successfully.
    EFI_OUT_OF_RESOURCES  : Failed due to lack of resource. 
    EFI_INVALID_PARAMETER : Some parameters are invalid.
    EFI_TIMEOUT           : Failed due to timeout.
    EFI_DEVICE_ERROR      : Failed due to host controller or device error.

--*/
{
  USB_HC_DEV          *Uhc;

  Uhc = UHC_FROM_USB2_HC_PROTO (This);

  if (Data == NULL || DeviceSpeed == EFI_USB_SPEED_LOW) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // For full-speed bulk transfers only the data pointed by Data[0] shall be used
  //
  return UhciBulkTransfer (
           &Uhc->UsbHc,
           DeviceAddress,
           EndPointAddress,
           (UINT8) MaximumPacketLength,
           *Data,
           DataLength,
           DataToggle,
           TimeOut,
           TransferResult
           );
}

STATIC
EFI_STATUS
EFIAPI
Uhci2AsyncInterruptTransfer (
  IN     EFI_USB2_HC_PROTOCOL               *This,
  IN     UINT8                              DeviceAddress,
  IN     UINT8                              EndPointAddress,
  IN     UINT8                              DeviceSpeed,
  IN     UINTN                              MaximumPacketLength,
  IN     BOOLEAN                            IsNewTransfer,
  IN OUT UINT8                              *DataToggle,
  IN     UINTN                              PollingInterval,
  IN     UINTN                              DataLength,
  IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
  IN     EFI_ASYNC_USB_TRANSFER_CALLBACK    CallBackFunction,
  IN     VOID                               *Context
  )
/*++
  
  Routine Description:
  
    Submits an asynchronous interrupt transfer to an 
    interrupt endpoint of a USB device according to UEFI 2.0 spec.
  
  Arguments:
    
    This                : A pointer to the EFI_USB2_HC_PROTOCOL instance.
    DeviceAddress       : Target device address          
    EndPointAddress     : Endpoint number and direction             
    DeviceSpeed         : Device speed
    MaximumPacketLength : Maximum packet size of the target endpoint
    IsNewTransfer       : If TRUE, submit a new transfer, if FALSE cancel old transfer       
    DataToggle          : On input, data toggle to use; On output, next data toggle
    PollingInterval     : Interrupt poll rate in milliseconds              
    DataLength          : On input, size of the data buffer, On output, 
                          actually transferred data size.             
    Translator          : A pointr to the transaction translator data.
    CallBackFunction    : Function to call periodically
    Context             : User context
  
  Returns:
  
    EFI_SUCCESS           : Transfer was submitted
    EFI_INVALID_PARAMETER : Some parameters are invalid.
    EFI_OUT_OF_RESOURCES  : Failed due to a lack of resources.  
    EFI_DEVICE_ERROR      : Can't read register
        
--*/
{
  USB_HC_DEV          *Uhc;
  BOOLEAN             IsSlow;

  Uhc     = UHC_FROM_USB2_HC_PROTO (This);
  IsSlow  = (EFI_USB_SPEED_LOW == DeviceSpeed) ? TRUE : FALSE;

  return UhciAsyncInterruptTransfer (
           &Uhc->UsbHc,
           DeviceAddress,
           EndPointAddress,
           IsSlow,
           (UINT8) MaximumPacketLength,
           IsNewTransfer,
           DataToggle,
           PollingInterval,
           DataLength,
           CallBackFunction,
           Context
           );
}

STATIC
EFI_STATUS
EFIAPI
Uhci2SyncInterruptTransfer (
  IN     EFI_USB2_HC_PROTOCOL                      *This,
  IN     UINT8                                     DeviceAddress,
  IN     UINT8                                     EndPointAddress,
  IN     UINT8                                     DeviceSpeed,
  IN     UINTN                                     MaximumPacketLength,
  IN OUT VOID                                      *Data,
  IN OUT UINTN                                     *DataLength,
  IN OUT UINT8                                     *DataToggle,
  IN     UINTN                                     TimeOut,
  IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR        *Translator,
  OUT    UINT32                                    *TransferResult
  )
/*++
  
  Routine Description:
  
    Submits synchronous interrupt transfer to an interrupt endpoint 
    of a USB device according to UEFI 2.0 spec.
  
  Arguments:
    
    This                : A pointer to the EFI_USB2_HC_PROTOCOL instance.
    DeviceAddress       : Target device address          
    EndPointAddress     : Endpoint number and direction             
    DeviceSpeed         : Device speed
    MaximumPacketLength : Maximum packet size of the target endpoint
    DataBuffersNumber   : Number of data buffers prepared for the transfer.
    Data                : Array of pointers to the buffers of data        
    DataLength          : On input, size of the data buffer, On output, 
                          actually transferred data size.             
    DataToggle          : On input, data toggle to use; On output, next data toggle
    TimeOut             : Maximum time out, in microseconds       
    Translator          : A pointr to the transaction translator data.
    TransferResult      : Variable to receive transfer result

  Returns:
    EFI_SUCCESS           : The transfer was completed successfully.
    EFI_OUT_OF_RESOURCES  : Failed due to lack of resource. 
    EFI_INVALID_PARAMETER : Some parameters are invalid.
    EFI_TIMEOUT           : Failed due to timeout.
    EFI_DEVICE_ERROR      : Failed due to host controller or device error.
        
--*/
{
  USB_HC_DEV          *Uhc;
  BOOLEAN             IsSlow;

  if (DeviceSpeed == EFI_USB_SPEED_HIGH) {
    return EFI_INVALID_PARAMETER;
  }

  Uhc     = UHC_FROM_USB2_HC_PROTO (This);
  IsSlow  = (EFI_USB_SPEED_LOW == DeviceSpeed) ? TRUE : FALSE;

  return UhciSyncInterruptTransfer (
           &Uhc->UsbHc,
           DeviceAddress,
           EndPointAddress,
           IsSlow,
           (UINT8) MaximumPacketLength,
           Data,
           DataLength,
           DataToggle,
           TimeOut,
           TransferResult
           );
}

STATIC
EFI_STATUS
EFIAPI
Uhci2IsochronousTransfer (
  IN     EFI_USB2_HC_PROTOCOL               *This,
  IN     UINT8                              DeviceAddress,
  IN     UINT8                              EndPointAddress,
  IN     UINT8                              DeviceSpeed,
  IN     UINTN                              MaximumPacketLength,
  IN     UINT8                              DataBuffersNumber,
  IN OUT VOID                               *Data[EFI_USB_MAX_ISO_BUFFER_NUM],
  IN     UINTN                              DataLength,
  IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
  OUT    UINT32                             *TransferResult
  )
/*++
  
  Routine Description:
  
    Submits isochronous transfer to a target USB device according to UEFI 2.0 spec.
  
  Arguments:
    
    This                : A pointer to the EFI_USB2_HC_PROTOCOL instance.
    DeviceAddress       : Target device address          
    EndPointAddress     : Endpoint number and direction             
    DeviceSpeed         : Device speed
    MaximumPacketLength : Maximum packet size of the target endpoint
    DataBuffersNumber   : Number of data buffers prepared for the transfer.
    Data                : Array of pointers to the buffers of data        
    DataLength          : On input, size of the data buffer, On output, 
                          actually transferred data size.             
    Translator          : A pointr to the transaction translator data.
    TransferResult      : Variable to receive transfer result
                      
  Returns:
  
    EFI_UNSUPPORTED 

--*/
{
  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
Uhci2AsyncIsochronousTransfer (
  IN     EFI_USB2_HC_PROTOCOL                *This,
  IN     UINT8                               DeviceAddress,
  IN     UINT8                               EndPointAddress,
  IN     UINT8                               DeviceSpeed,
  IN     UINTN                               MaximumPacketLength,
  IN     UINT8                               DataBuffersNumber,
  IN OUT VOID                                *Data[EFI_USB_MAX_ISO_BUFFER_NUM],
  IN     UINTN                               DataLength,
  IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  IN     EFI_ASYNC_USB_TRANSFER_CALLBACK     IsochronousCallBack,
  IN     VOID                                *Context
  )
/*++
  
  Routine Description:
  
    Submits Async isochronous transfer to a target USB device according to UEFI 2.0 spec.
  
  Arguments:
    
    This                : A pointer to the EFI_USB2_HC_PROTOCOL instance.
    DeviceAddress       : Target device address          
    EndPointAddress     : Endpoint number and direction             
    DeviceSpeed         : Device speed
    MaximumPacketLength : Maximum packet size of the target endpoint
    DataBuffersNumber   : Number of data buffers prepared for the transfer.
    Data                : Array of pointers to the buffers of data        
    Translator          : A pointr to the transaction translator data.
    IsochronousCallBack : Function to call when the transfer complete
    Context             : Pass to the call back function as parameter
                    
  Returns:
  
    EFI_UNSUPPORTED 

--*/
{
  return EFI_UNSUPPORTED;
}

EFI_DRIVER_ENTRY_POINT (UhciDriverEntryPoint)

EFI_STATUS
EFIAPI
UhciDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

  Routine Description:
  
    Entry point for EFI drivers.

  Arguments:
  
    ImageHandle - EFI_HANDLE
    SystemTable - EFI_SYSTEM_TABLE
    
  Returns:
  
    EFI_SUCCESS : Driver is successfully loaded
    Others      : Failed
    
--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle,
           SystemTable,
           &gUhciDriverBinding,
           ImageHandle,
           &gUhciComponentName,
           NULL,
           NULL
           );
}

EFI_STATUS
EFIAPI
UhciDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN EFI_HANDLE                      Controller,
  IN EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

  Routine Description:
  
    Test to see if this driver supports ControllerHandle. Any 
    ControllerHandle that has UsbHcProtocol installed will be supported.

  Arguments:
  
    This                - Protocol instance pointer.
    Controller          - Handle of device to test
    RemainingDevicePath - Not used

  Returns:
  
    EFI_SUCCESS         : This driver supports this device.
    EFI_UNSUPPORTED     : This driver does not support this device.

--*/
{
  EFI_STATUS            OpenStatus;
  EFI_STATUS            Status;
  EFI_PCI_IO_PROTOCOL   *PciIo;
  USB_CLASSC            UsbClassCReg;

  //
  // Test whether there is PCI IO Protocol attached on the controller handle.
  //
  OpenStatus = gBS->OpenProtocol (
                      Controller,
                      &gEfiPciIoProtocolGuid,
                      &PciIo,
                      This->DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_BY_DRIVER
                      );

  if (EFI_ERROR (OpenStatus)) {
    return OpenStatus;
  }

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        CLASSC_OFFSET,
                        sizeof (USB_CLASSC) / sizeof (UINT8),
                        &UsbClassCReg
                        );

  if (EFI_ERROR (Status)) {
    Status = EFI_UNSUPPORTED;
    goto ON_EXIT;
  }
  
  //
  // Test whether the controller belongs to UHCI type
  //
  if ((UsbClassCReg.BaseCode != PCI_CLASS_SERIAL) ||
      (UsbClassCReg.SubClassCode != PCI_CLASS_SERIAL_USB) ||
      (UsbClassCReg.PI != PCI_CLASSC_PI_UHCI)
      ) {

    Status = EFI_UNSUPPORTED;
  }

ON_EXIT:
  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  return Status;

}

STATIC
USB_HC_DEV *
UhciAllocateDev (
  IN EFI_PCI_IO_PROTOCOL    *PciIo
  )
/*++

Routine Description:

  Allocate and initialize the empty UHCI device 

Arguments:

  PciIo - The PCIIO to use

Returns:

  Allocated UHCI device

--*/
{
  USB_HC_DEV  *Uhc;
  EFI_STATUS  Status;

  Uhc = EfiLibAllocateZeroPool (sizeof (USB_HC_DEV));

  if (Uhc == NULL) {
    return NULL;
  }
  
  //
  // This driver supports both USB_HC_PROTOCOL and USB2_HC_PROTOCOL.
  // USB_HC_PROTOCOL is for EFI 1.1 backward compability.
  //
  Uhc->Signature                        = USB_HC_DEV_SIGNATURE;
  Uhc->UsbHc.Reset                      = UhciReset;
  Uhc->UsbHc.GetState                   = UhciGetState;
  Uhc->UsbHc.SetState                   = UhciSetState;
  Uhc->UsbHc.ControlTransfer            = UhciControlTransfer;
  Uhc->UsbHc.BulkTransfer               = UhciBulkTransfer;
  Uhc->UsbHc.AsyncInterruptTransfer     = UhciAsyncInterruptTransfer;
  Uhc->UsbHc.SyncInterruptTransfer      = UhciSyncInterruptTransfer;
  Uhc->UsbHc.IsochronousTransfer        = UhciIsochronousTransfer;
  Uhc->UsbHc.AsyncIsochronousTransfer   = UhciAsyncIsochronousTransfer;
  Uhc->UsbHc.GetRootHubPortNumber       = UhciGetRootHubPortNumber;
  Uhc->UsbHc.GetRootHubPortStatus       = UhciGetRootHubPortStatus;
  Uhc->UsbHc.SetRootHubPortFeature      = UhciSetRootHubPortFeature;
  Uhc->UsbHc.ClearRootHubPortFeature    = UhciClearRootHubPortFeature;
  Uhc->UsbHc.MajorRevision              = 0x1;
  Uhc->UsbHc.MinorRevision              = 0x1;

  Uhc->Usb2Hc.GetCapability             = Uhci2GetCapability;
  Uhc->Usb2Hc.Reset                     = Uhci2Reset;
  Uhc->Usb2Hc.GetState                  = Uhci2GetState;
  Uhc->Usb2Hc.SetState                  = Uhci2SetState;
  Uhc->Usb2Hc.ControlTransfer           = Uhci2ControlTransfer;
  Uhc->Usb2Hc.BulkTransfer              = Uhci2BulkTransfer;
  Uhc->Usb2Hc.AsyncInterruptTransfer    = Uhci2AsyncInterruptTransfer;
  Uhc->Usb2Hc.SyncInterruptTransfer     = Uhci2SyncInterruptTransfer;
  Uhc->Usb2Hc.IsochronousTransfer       = Uhci2IsochronousTransfer;
  Uhc->Usb2Hc.AsyncIsochronousTransfer  = Uhci2AsyncIsochronousTransfer;
  Uhc->Usb2Hc.GetRootHubPortStatus      = Uhci2GetRootHubPortStatus;
  Uhc->Usb2Hc.SetRootHubPortFeature     = Uhci2SetRootHubPortFeature;
  Uhc->Usb2Hc.ClearRootHubPortFeature   = Uhci2ClearRootHubPortFeature;
  Uhc->Usb2Hc.MajorRevision             = 0x1;
  Uhc->Usb2Hc.MinorRevision             = 0x1;
  
  Uhc->PciIo   = PciIo;
  Uhc->MemPool = UsbHcInitMemPool (PciIo, TRUE, 0);
  
  if (Uhc->MemPool == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ON_ERROR;
  }

  InitializeListHead (&Uhc->AsyncIntList);
  
  Status = gBS->CreateEvent (
                  EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_CALLBACK,
                  UhciMonitorAsyncReqList,
                  Uhc,
                  &Uhc->AsyncIntMonitor
                  );

  if (EFI_ERROR (Status)) {
    UsbHcFreeMemPool (Uhc->MemPool);
    goto ON_ERROR;
  }

  return Uhc;

ON_ERROR:
  gBS->FreePool (Uhc);
  return NULL;
}

STATIC
VOID
UhciFreeDev (
  IN USB_HC_DEV           *Uhc
  )
/*++

Routine Description:

  Free the UHCI device and release its associated resources

Arguments:

  Uhc - The UHCI device to release

Returns:

  None

--*/
{
  if (Uhc->AsyncIntMonitor != NULL) {
    gBS->CloseEvent (Uhc->AsyncIntMonitor);
  }

  if (Uhc->MemPool != NULL) {
    UsbHcFreeMemPool (Uhc->MemPool);
  }

  if (Uhc->CtrlNameTable) {
    EfiLibFreeUnicodeStringTable (Uhc->CtrlNameTable);
  }

  gBS->FreePool (Uhc);
}

STATIC
VOID
UhciCleanDevUp (
  IN  EFI_HANDLE          Controller,
  IN  EFI_USB_HC_PROTOCOL *This
  )
/*++
  Routine Description:
  
    Uninstall all Uhci Interface
    
  Arguments:
  
    Controller        - Controller handle
    This              - Protocol instance pointer.
    
  Returns:
  
    VOID
    
--*/
{
  USB_HC_DEV          *Uhc;

  //
  // Uninstall the USB_HC and USB_HC2 protocol, then disable the controller
  //
  Uhc = UHC_FROM_USB_HC_PROTO (This);
  UhciStopHc (Uhc, UHC_GENERIC_TIMEOUT);

  gBS->UninstallProtocolInterface (
        Controller,
        &gEfiUsbHcProtocolGuid,
        &Uhc->UsbHc
        );

  gBS->UninstallProtocolInterface (
        Controller,
        &gEfiUsb2HcProtocolGuid,
        &Uhc->Usb2Hc
        );
  
  UhciFreeAllAsyncReq (Uhc);
  UhciDestoryFrameList (Uhc);

  Uhc->PciIo->Attributes (
                Uhc->PciIo,
                EfiPciIoAttributeOperationDisable,
                EFI_PCI_DEVICE_ENABLE,
                NULL
                );

  UhciFreeDev (Uhc);
}

EFI_STATUS
EFIAPI
UhciDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN EFI_HANDLE                      Controller,
  IN EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

  Routine Description:
  
    Starting the Usb UHCI Driver

  Arguments:
  
    This                - Protocol instance pointer.
    Controller          - Handle of device to test
    RemainingDevicePath - Not used

  Returns:
  
    EFI_SUCCESS         - This driver supports this device.
    EFI_UNSUPPORTED     - This driver does not support this device.
    EFI_DEVICE_ERROR    - This driver cannot be started due to device Error
    EFI_OUT_OF_RESOURCES- Failed due to resource shortage

--*/
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  USB_HC_DEV          *Uhc;

  //
  // Open PCIIO, then enable the EHC device and turn off emulation
  //
  Uhc = NULL;
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  UhciTurnOffUsbEmulation (PciIo);

  Status = PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationEnable,
                    EFI_PCI_DEVICE_ENABLE,
                    NULL
                    );

  if (EFI_ERROR (Status)) {
    goto CLOSE_PCIIO;
  }

  Uhc = UhciAllocateDev (PciIo);

  if (Uhc == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto CLOSE_PCIIO;
  }
  
  //
  // Stop schedule and set the Global Reset bit in the command register
  //
  UhciStopHc (Uhc, UHC_GENERIC_TIMEOUT);
  UhciSetRegBit (Uhc->PciIo, USBCMD_OFFSET, USBCMD_GRESET);
  gBS->Stall (UHC_ROOT_PORT_RESET_STALL);
  //
  // Clear the Global Reset bit to zero.
  //
  UhciClearRegBit (Uhc->PciIo, USBCMD_OFFSET, USBCMD_GRESET);
  gBS->Stall (UHC_ROOT_PORT_RECOVERY_STALL);
  
  //
  // Allocate and Init Host Controller's Frame List Entry
  //
  Status = UhciInitFrameList (Uhc);

  if (EFI_ERROR (Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto FREE_UHC;
  }

  Status = gBS->SetTimer (
                  Uhc->AsyncIntMonitor,
                  TimerPeriodic,
                  UHC_ASYNC_POLL_INTERVAL
                  );

  if (EFI_ERROR (Status)) {
    goto FREE_UHC;
  }
  
  //
  // Install both USB_HC_PROTOCOL and USB2_HC_PROTOCOL
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiUsbHcProtocolGuid,
                  &Uhc->UsbHc,
                  &gEfiUsb2HcProtocolGuid,
                  &Uhc->Usb2Hc,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    goto FREE_UHC;
  }
 
  //
  // Install the component name protocol
  //
  Uhc->CtrlNameTable = NULL;

  EfiLibAddUnicodeString (
    LANGUAGE_CODE_ENGLISH,
    gUhciComponentName.SupportedLanguages,
    &Uhc->CtrlNameTable,
    L"Usb Universal Host Controller"
    );

  //
  // Start the UHCI hardware, also set its reclamation point to 64 bytes
  //
  UhciWriteReg (Uhc->PciIo, USBCMD_OFFSET, USBCMD_RS | USBCMD_MAXP);

  return EFI_SUCCESS;

FREE_UHC:
  UhciFreeDev (Uhc);

CLOSE_PCIIO:
  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

EFI_STATUS
EFIAPI
UhciDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN EFI_HANDLE                      Controller,
  IN UINTN                           NumberOfChildren,
  IN EFI_HANDLE                      *ChildHandleBuffer
  )
/*++

  Routine Description:
  
    Stop this driver on ControllerHandle. Support stoping any child handles
    created by this driver.

  Arguments:
  
    This              - Protocol instance pointer.
    Controller        - Handle of device to stop driver on
    NumberOfChildren  - Number of Children in the ChildHandleBuffer
    ChildHandleBuffer - List of handles for the children we need to stop.

  Returns:
  
    EFI_SUCCESS
    others

--*/
{
  EFI_USB_HC_PROTOCOL   *UsbHc;
  EFI_USB2_HC_PROTOCOL  *Usb2Hc;
  EFI_STATUS            Status;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUsbHcProtocolGuid,
                  &UsbHc,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  //
  // Test whether the Controller handler passed in is a valid
  // Usb controller handle that should be supported, if not,
  // return the error status directly
  //
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUsb2HcProtocolGuid,
                  &Usb2Hc,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  //
  // Test whether the Controller handler passed in is a valid
  // Usb controller handle that should be supported, if not,
  // return the error status directly
  //
  if (EFI_ERROR (Status)) {
    return Status;
  }

  UhciCleanDevUp (Controller, UsbHc);

  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return EFI_SUCCESS;
}

EFI_DRIVER_BINDING_PROTOCOL gUhciDriverBinding = {
  UhciDriverBindingSupported,
  UhciDriverBindingStart,
  UhciDriverBindingStop,
  0x20,
  NULL,
  NULL
};
