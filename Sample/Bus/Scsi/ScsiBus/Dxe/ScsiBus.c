/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    scsibus.c
    
Abstract: 

    SCSI Bus Driver

Revision History
--*/

#include "scsibus.h"
#include "ScsiLib.h"


EFI_STATUS
EFIAPI
SCSIBusDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
SCSIBusDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
SCSIBusDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  );

EFI_DRIVER_BINDING_PROTOCOL gSCSIBusDriverBinding = {
  SCSIBusDriverBindingSupported,
  SCSIBusDriverBindingStart,
  SCSIBusDriverBindingStop,
  0x10,
  NULL,
  NULL
};


//
// The ScsiBusProtocol is just used to locate ScsiBusDev
// structure in the SCSIBusDriverBindingStop(). Then we can
// Close all opened protocols and release this structure.
//
STATIC EFI_GUID  mScsiBusProtocolGuid = EFI_SCSI_BUS_PROTOCOL_GUID;


EFI_DRIVER_ENTRY_POINT (ScsiBusControllerDriverEntryPoint)

EFI_STATUS
EFIAPI
ScsiBusControllerDriverEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Entry point for EFI drivers.

Arguments:

  ImageHandle - EFI_HANDLE
  SystemTable - EFI_SYSTEM_TABLE

Returns:

  EFI_SUCCESS
  Others 
--*/
{
  EFI_STATUS  Status;

  //
  // Initialize the EFI Library
  //
  Status = EfiLibInstallAllDriverProtocols (
             ImageHandle,
             SystemTable,
             &gSCSIBusDriverBinding,
             ImageHandle,
             &gScsiBusComponentName,
             NULL,
             NULL
             );
  return Status;
}

EFI_STATUS
EFIAPI
SCSIBusDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

  Test to see if this driver supports ControllerHandle. Any ControllerHandle
  that has ExtScsiPassThruProtocol/ScsiPassThruProtocol installed will be supported.

Arguments:

  This                - Protocol instance pointer.
  Controller          - Handle of device to test
  RemainingDevicePath - Not used

Returns:

  EFI_SUCCESS         - This driver supports this device.
  EFI_UNSUPPORTED     - This driver does not support this device.

--*/

{
  EFI_STATUS  Status;

  //
  // If RemainingDevicePath is not NULL, it should verify that the first device
  // path node in RemainingDevicePath is an ATAPI Device path node.
  //
  if (RemainingDevicePath != NULL) {
    if ((RemainingDevicePath->Type != MESSAGING_DEVICE_PATH) ||
        (RemainingDevicePath->SubType != MSG_ATAPI_DP) ||
        (DevicePathNodeLength (RemainingDevicePath) != sizeof(ATAPI_DEVICE_PATH))) {
      return EFI_UNSUPPORTED;
    }
  }
  //
  // Check for the existence of Extended SCSI Pass Thru Protocol and SCSI Pass Thru Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiExtScsiPassThruProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    Status = gBS->OpenProtocol (
                    Controller,
                    &gEfiScsiPassThruProtocolGuid,
                    NULL,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      Status = EFI_UNSUPPORTED;
    }
  }
  return Status;
}

EFI_STATUS
EFIAPI
SCSIBusDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:
  Starting the SCSI Bus Driver

Arguments:
  This                - Protocol instance pointer.
  Controller          - Handle of device to test
  RemainingDevicePath - Not used

Returns:
  EFI_SUCCESS         - This driver supports this device.
  EFI_UNSUPPORTED     - This driver does not support this device.
  EFI_DEVICE_ERROR    - This driver cannot be started due to device Error

--*/
{
  EFI_STATUS                  Status;
  UINT32                      StartPun;
  UINT64                      StartLun;
  UINT32                      Pun;
  UINT64                      Lun;
  BOOLEAN                     ScanOtherPuns;
  SCSI_BUS_DEVICE             *ScsiBusDev;
  UINT8                       **TargetId;
  UINT8                       Target[4];

  TargetId = NULL;
  //
  // Allocate SCSI_BUS_DEVICE structure
  //
  ScsiBusDev = NULL;
  ScsiBusDev = EfiLibAllocateZeroPool (sizeof (SCSI_BUS_DEVICE));
  if (ScsiBusDev == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  StartPun  = 0;
  StartLun  = 0;
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &(ScsiBusDev->DevicePath),
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    gBS->FreePool (ScsiBusDev);
    return Status;
  }

  //
  // First consume Extended SCSI Pass Thru protocol, if fail, then consume
  // SCSI Pass Thru protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiExtScsiPassThruProtocolGuid,
                  (VOID **) &(ScsiBusDev->ExtScsiInterface),
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    Status = gBS->OpenProtocol (
                    Controller,
                    &gEfiScsiPassThruProtocolGuid,
                    (VOID **) &(ScsiBusDev->ScsiInterface),
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_BY_DRIVER
                    );
    if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
      gBS->CloseProtocol (
             Controller,
             &gEfiDevicePathProtocolGuid,
             This->DriverBindingHandle,
             Controller
             );
      gBS->FreePool (ScsiBusDev);
      return Status;
    } 
    DEBUG ((EFI_D_INFO, "Open Scsi Pass Thrugh Protocol\n"));
    ScsiBusDev->ExtScsiSupport  = FALSE;
  } else {
    DEBUG ((EFI_D_INFO, "Open Extended Scsi Pass Thrugh Protocol\n"));
    ScsiBusDev->ExtScsiSupport  = TRUE;
  }

  ScsiBusDev->Signature = SCSI_BUS_DEVICE_SIGNATURE;
  //
  // Attach EFI_SCSI_BUS_PROTOCOL to controller handle
  //
  Status = gBS->InstallProtocolInterface (
                  &Controller,
                  &mScsiBusProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &ScsiBusDev->BusIdentify
                  );

  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
           Controller,
           &gEfiDevicePathProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );
    if (ScsiBusDev->ExtScsiSupport) {
      gBS->CloseProtocol (
             Controller,
             &gEfiExtScsiPassThruProtocolGuid,
             This->DriverBindingHandle,
             Controller
             );
    } else {
      gBS->CloseProtocol (
             Controller,
             &gEfiScsiPassThruProtocolGuid,
             This->DriverBindingHandle,
             Controller
             );
    }
    gBS->FreePool (ScsiBusDev);
    return Status;
  }

  if (RemainingDevicePath == NULL) {
    StartPun  = 0xFFFFFFFF;
    StartLun  = 0;
  } else {
    if (ScsiBusDev->ExtScsiSupport) {
      *(UINT32*)Target = StartPun;
      *TargetId = Target;
      ScsiBusDev->ExtScsiInterface->GetTargetLun (ScsiBusDev->ExtScsiInterface, RemainingDevicePath, TargetId, &StartLun);  
      StartPun = *(UINT32*)Target;
    } else {
      ScsiBusDev->ScsiInterface->GetTargetLun (ScsiBusDev->ScsiInterface, RemainingDevicePath, &StartPun, &StartLun);
    }
  }

  for (Pun = StartPun, ScanOtherPuns = TRUE; ScanOtherPuns;) {

    if (StartPun == 0xFFFFFFFF) {
      //
      // Remaining Device Path is NULL, scan all the possible Puns in the
      // SCSI Channel.
      //
      if (ScsiBusDev->ExtScsiSupport) {
        *(UINT32*)Target = Pun;
        *TargetId = Target;
        Status = ScsiBusDev->ExtScsiInterface->GetNextTargetLun (ScsiBusDev->ExtScsiInterface, TargetId, &Lun);
        Pun = *(UINT32*)Target;
      } else {
        Status = ScsiBusDev->ScsiInterface->GetNextDevice (ScsiBusDev->ScsiInterface, &Pun, &Lun);
      }
      if (EFI_ERROR (Status)) {
        //
        // no legal Pun and Lun found any more
        //
        break;
      }
    } else {
      //
      // Remaining Device Path is not NULL, only scan the specified Pun.
      //
      Pun           = StartPun;
      Lun           = StartLun;
      ScanOtherPuns = FALSE;
    }
    //
    // Avoid creating handle for the host adapter.
    //
    if (ScsiBusDev->ExtScsiSupport) {
      if (Pun == ScsiBusDev->ExtScsiInterface->Mode->AdapterId) {
        continue;
      }
    } else {
      if (Pun == ScsiBusDev->ScsiInterface->Mode->AdapterId) {
        continue;
      }
    }
    //
    // Scan for the scsi device, if it attaches to the scsi bus,
    // then create handle and install scsi i/o protocol.
    //
    Status = ScsiScanCreateDevice (This, Controller, Pun, Lun, ScsiBusDev);
  }
  return Status;
}

EFI_STATUS
EFIAPI
SCSIBusDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
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
  Others
--*/
{
  EFI_STATUS                  Status;
  BOOLEAN                     AllChildrenStopped;
  UINTN                       Index;
  EFI_SCSI_IO_PROTOCOL        *ScsiIo;
  SCSI_IO_DEV                 *ScsiIoDevice;
  VOID                        *ScsiPassThru;
  EFI_SCSI_BUS_PROTOCOL       *Scsidentifier;
  SCSI_BUS_DEVICE             *ScsiBusDev;

  if (NumberOfChildren == 0) {
    //
    // Get the SCSI_BUS_DEVICE
    //
    Status = gBS->OpenProtocol (
                    Controller,
                    &mScsiBusProtocolGuid,
                    (VOID **) &Scsidentifier,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
  
    if (EFI_ERROR (Status)) {
      return EFI_DEVICE_ERROR;
    }

    ScsiBusDev = SCSI_BUS_CONTROLLER_DEVICE_FROM_THIS (Scsidentifier);

    //
    // Uninstall SCSI Bus Protocol
    //
    gBS->UninstallProtocolInterface (
           Controller,
           &mScsiBusProtocolGuid,
           &ScsiBusDev->BusIdentify
           );
    
    //
    // Close the bus driver
    //
    if (ScsiBusDev->ExtScsiSupport) {
      gBS->CloseProtocol (
             Controller,
             &gEfiExtScsiPassThruProtocolGuid,
             This->DriverBindingHandle,
             Controller
             );
    } else {
      gBS->CloseProtocol (
             Controller,
             &gEfiScsiPassThruProtocolGuid,
             This->DriverBindingHandle,
             Controller
             );
    }

    gBS->CloseProtocol (
           Controller,
           &gEfiDevicePathProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );
    gBS->FreePool (ScsiBusDev);
    return EFI_SUCCESS;
  }

  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {

    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index],
                    &gEfiScsiIoProtocolGuid,
                    (VOID **) &ScsiIo,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
      continue;
    }

    ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (ScsiIo);
    //
    // Close the child handle
    //
    if (ScsiIoDevice->ExtScsiSupport) {
      Status = gBS->CloseProtocol (
                      Controller,
                      &gEfiExtScsiPassThruProtocolGuid,
                      This->DriverBindingHandle,
                      ChildHandleBuffer[Index]
                      );

    } else {
      Status = gBS->CloseProtocol (
                      Controller,
                      &gEfiScsiPassThruProtocolGuid,
                      This->DriverBindingHandle,
                      ChildHandleBuffer[Index]
                      );
    }

    Status = gBS->UninstallMultipleProtocolInterfaces (
                    ChildHandleBuffer[Index],
                    &gEfiDevicePathProtocolGuid,
                    ScsiIoDevice->DevicePath,
                    &gEfiScsiIoProtocolGuid,
                    &ScsiIoDevice->ScsiIo,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
      if (ScsiIoDevice->ExtScsiSupport) {
        gBS->OpenProtocol (
               Controller,
               &gEfiExtScsiPassThruProtocolGuid,
               &(EFI_EXT_SCSI_PASS_THRU_PROTOCOL*)ScsiPassThru,
               This->DriverBindingHandle,
               ChildHandleBuffer[Index],
               EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
               );
      } else {
        gBS->OpenProtocol (
               Controller,
               &gEfiScsiPassThruProtocolGuid,
               &(EFI_SCSI_PASS_THRU_PROTOCOL*)ScsiPassThru,
               This->DriverBindingHandle,
               ChildHandleBuffer[Index],
               EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
               );
      }
    } else {
      gBS->FreePool (ScsiIoDevice);
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ScsiGetDeviceType (
  IN  EFI_SCSI_IO_PROTOCOL     *This,
  OUT UINT8                    *DeviceType
  )
/*++

Routine Description:

  Retrieves the device type information of the SCSI Controller.
    
Arguments:

  This                  - Protocol instance pointer.
  DeviceType            - A pointer to the device type information
                            retrieved from the SCSI Controller. 

Returns:

  EFI_SUCCESS           - Retrieves the device type information successfully.
  EFI_INVALID_PARAMETER - The DeviceType is NULL.
  
--*/
{
  SCSI_IO_DEV *ScsiIoDevice;

  if (DeviceType == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ScsiIoDevice  = SCSI_IO_DEV_FROM_THIS (This);
  *DeviceType   = ScsiIoDevice->ScsiDeviceType;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ScsiGetDeviceLocation (
  IN  EFI_SCSI_IO_PROTOCOL    *This,
  IN OUT UINT8                **Target,
  OUT UINT64                  *Lun
  )
/*++

Routine Description:

  Retrieves the device location in the SCSI channel.
    
Arguments:

  This                  - Protocol instance pointer.
  Target                - A pointer to the Target Array which represents ID of a SCSI device 
                          on the SCSI channel. 
  Lun                   - A pointer to the LUN of the SCSI device on 
                          the SCSI channel.

Returns:

  EFI_SUCCESS           - Retrieves the device location successfully.
  EFI_INVALID_PARAMETER - The Target or Lun is NULL.

--*/
{
  SCSI_IO_DEV *ScsiIoDevice;

  if (Target == NULL || Lun == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (This);

  (*Target)[0] = (UINT8) (ScsiIoDevice->Pun & 0xff);
  (*Target)[1] = (UINT8) ((ScsiIoDevice->Pun >> 8) & 0xff);
  (*Target)[2] = (UINT8) ((ScsiIoDevice->Pun >> 16) & 0xff);
  (*Target)[3] = (UINT8) ((ScsiIoDevice->Pun >> 24) & 0xff);

  *Lun         = ScsiIoDevice->Lun;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ScsiResetBus (
  IN  EFI_SCSI_IO_PROTOCOL     *This
  )
/*++

Routine Description:

  Resets the SCSI Bus that the SCSI Controller is attached to.
    
Arguments:

  This                  - Protocol instance pointer.

Returns:

  EFI_SUCCESS           - The SCSI bus is reset successfully.
  EFI_DEVICE_ERROR      - Errors encountered when resetting the SCSI bus.
  EFI_UNSUPPORTED       - The bus reset operation is not supported by the
                          SCSI Host Controller.
  EFI_TIMEOUT           - A timeout occurred while attempting to reset 
                          the SCSI bus.
--*/
{
  SCSI_IO_DEV *ScsiIoDevice;

  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (This);

  if (ScsiIoDevice->ExtScsiSupport){
    return ScsiIoDevice->ExtScsiPassThru->ResetChannel (ScsiIoDevice->ExtScsiPassThru);
  } else {
    return ScsiIoDevice->ScsiPassThru->ResetChannel (ScsiIoDevice->ScsiPassThru);
  }
}

EFI_STATUS
EFIAPI
ScsiResetDevice (
  IN  EFI_SCSI_IO_PROTOCOL     *This
  )
/*++

Routine Description:

  Resets the SCSI Controller that the device handle specifies.
    
Arguments:

  This                  - Protocol instance pointer.
    
Returns:

  EFI_SUCCESS           - Reset the SCSI controller successfully.
  EFI_DEVICE_ERROR      - Errors are encountered when resetting the
                          SCSI Controller.
  EFI_UNSUPPORTED       - The SCSI bus does not support a device 
                          reset operation.
  EFI_TIMEOUT           - A timeout occurred while attempting to 
                          reset the SCSI Controller.
--*/
{
  SCSI_IO_DEV  *ScsiIoDevice;
  UINT8        Target[4];

  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (This);
  *(UINT32*)Target = ScsiIoDevice->Pun;

  if (ScsiIoDevice->ExtScsiSupport) {
    return ScsiIoDevice->ExtScsiPassThru->ResetTargetLun (
                                        ScsiIoDevice->ExtScsiPassThru,
                                        Target,
                                        ScsiIoDevice->Lun
                                          );
  } else {
    return ScsiIoDevice->ScsiPassThru->ResetTarget (
                                          ScsiIoDevice->ScsiPassThru,
                                          ScsiIoDevice->Pun,
                                          ScsiIoDevice->Lun
                                            );
  }
}

EFI_STATUS
EFIAPI
ScsiExecuteSCSICommand (
  IN  EFI_SCSI_IO_PROTOCOL                         *This,
  IN OUT  EFI_SCSI_IO_SCSI_REQUEST_PACKET          *Packet,
  IN EFI_EVENT                                     Event  OPTIONAL
  )
/*++

Routine Description:

  Sends a SCSI Request Packet to the SCSI Controller for execution.
    
Arguments:

  This                  - Protocol instance pointer.
  Packet                - The SCSI request packet to send to the SCSI 
                          Controller specified by the device handle.
  Event                 - If the SCSI bus where the SCSI device is attached
                          does not support non-blocking I/O, then Event is 
                          ignored, and blocking I/O is performed.  
                          If Event is NULL, then blocking I/O is performed.
                          If Event is not NULL and non-blocking I/O is 
                          supported, then non-blocking I/O is performed,
                          and Event will be signaled when the SCSI Request
                          Packet completes.
Returns:

  EFI_SUCCESS           - The SCSI Request Packet was sent by the host 
                          successfully, and TransferLength bytes were 
                          transferred to/from DataBuffer.See 
                          HostAdapterStatus, TargetStatus, 
                          SenseDataLength, and SenseData in that order
                          for additional status information.
  EFI_WARN_BUFFER_TOO_SMALL - The SCSI Request Packet was executed, 
                          but the entire DataBuffer could not be transferred.
                          The actual number of bytes transferred is returned
                          in TransferLength. See HostAdapterStatus, 
                          TargetStatus, SenseDataLength, and SenseData in 
                          that order for additional status information.
  EFI_NOT_READY         - The SCSI Request Packet could not be sent because 
                          there are too many SCSI Command Packets already 
                          queued.The caller may retry again later.
  EFI_DEVICE_ERROR      - A device error occurred while attempting to send 
                          the SCSI Request Packet. See HostAdapterStatus, 
                          TargetStatus, SenseDataLength, and SenseData in 
                          that order for additional status information.
  EFI_INVALID_PARAMETER - The contents of CommandPacket are invalid.  
                          The SCSI Request Packet was not sent, so no 
                          additional status information is available.
  EFI_UNSUPPORTED       - The command described by the SCSI Request Packet
                          is not supported by the SCSI initiator(i.e., SCSI 
                          Host Controller). The SCSI Request Packet was not
                          sent, so no additional status information is 
                          available.
  EFI_TIMEOUT           - A timeout occurred while waiting for the SCSI 
                          Request Packet to execute. See HostAdapterStatus,
                          TargetStatus, SenseDataLength, and SenseData in 
                          that order for additional status information.
--*/
{
  SCSI_IO_DEV                                 *ScsiIoDevice;
  EFI_STATUS                                  Status;
  UINT8                                       Target[4];
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET      *RequestPacket;
  EFI_EXT_SCSI_PASS_THRU_SCSI_REQUEST_PACKET  *ExtRequestPacket;

  if (Packet == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ScsiIoDevice  = SCSI_IO_DEV_FROM_THIS (This);
  *(UINT32*)Target = ScsiIoDevice->Pun;
  
  if (ScsiIoDevice->ExtScsiSupport) {
    ExtRequestPacket = (EFI_EXT_SCSI_PASS_THRU_SCSI_REQUEST_PACKET *) Packet;
    Status = ScsiIoDevice->ExtScsiPassThru->PassThru (
                                          ScsiIoDevice->ExtScsiPassThru,
                                          Target,
                                          ScsiIoDevice->Lun,
                                          ExtRequestPacket,
                                          Event
                                          );
  } else {
    RequestPacket = (EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET *) Packet;
    Status = ScsiIoDevice->ScsiPassThru->PassThru (
                                          ScsiIoDevice->ScsiPassThru,
                                          ScsiIoDevice->Pun,
                                          ScsiIoDevice->Lun,
                                          RequestPacket,
                                          Event
                                          );
 }
  
  return Status;
}

EFI_STATUS
ScsiScanCreateDevice (
  EFI_DRIVER_BINDING_PROTOCOL   *This,
  EFI_HANDLE                    Controller,
  UINT32                        Pun,
  UINT64                        Lun,
  SCSI_BUS_DEVICE               *ScsiBusDev
  )
/*++

Routine Description:

  Scan SCSI Bus to discover the device, and attach ScsiIoProtocol to it.

Arguments:

  This              - Protocol instance pointer
  Controller        - Controller handle
  Pun               - The Pun of the SCSI device on the SCSI channel.
  Lun               - The Lun of the SCSI device on the SCSI channel.
  ScsiBusDev        - The pointer of SCSI_BUS_DEVICE

Returns:

  EFI_SUCCESS       - Successfully to discover the device and attach ScsiIoProtocol to it.
  EFI_OUT_OF_RESOURCES - Fail to discover the device.

--*/
{
  UINT8                     Target[4];
  EFI_STATUS                Status;
  SCSI_IO_DEV               *ScsiIoDevice;
  EFI_DEVICE_PATH_PROTOCOL  *ScsiDevicePath;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (SCSI_IO_DEV),
                  (VOID **) &ScsiIoDevice
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  EfiZeroMem (ScsiIoDevice, sizeof (SCSI_IO_DEV));

  ScsiIoDevice->Signature                 = SCSI_IO_DEV_SIGNATURE;
  ScsiIoDevice->Pun                       = Pun;
  ScsiIoDevice->Lun                       = Lun;

  if (ScsiBusDev->ExtScsiSupport) {
    ScsiIoDevice->ExtScsiPassThru         = ScsiBusDev->ExtScsiInterface;
    ScsiIoDevice->ExtScsiSupport          = TRUE;
  } else {
    ScsiIoDevice->ScsiPassThru            = ScsiBusDev->ScsiInterface;
    ScsiIoDevice->ExtScsiSupport          = FALSE;
  }

  ScsiIoDevice->ScsiIo.GetDeviceType      = ScsiGetDeviceType;
  ScsiIoDevice->ScsiIo.GetDeviceLocation  = ScsiGetDeviceLocation;
  ScsiIoDevice->ScsiIo.ResetBus           = ScsiResetBus;
  ScsiIoDevice->ScsiIo.ResetDevice        = ScsiResetDevice;
  ScsiIoDevice->ScsiIo.ExecuteSCSICommand = ScsiExecuteSCSICommand;

  if (!DiscoverScsiDevice (ScsiIoDevice)) {
    gBS->FreePool (ScsiIoDevice);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Set Device Path
  //
  *(UINT32*)Target = ScsiIoDevice->Pun;
  if (ScsiIoDevice->ExtScsiSupport){
    Status = ScsiIoDevice->ExtScsiPassThru->BuildDevicePath (
                                          ScsiIoDevice->ExtScsiPassThru,
                                          Target,
                                          ScsiIoDevice->Lun,
                                          &ScsiDevicePath
                                          );
    if (Status == EFI_OUT_OF_RESOURCES) {
      gBS->FreePool (ScsiIoDevice);
      return Status;
    }
  } else {
    Status = ScsiIoDevice->ScsiPassThru->BuildDevicePath (
                                          ScsiIoDevice->ScsiPassThru,
                                          ScsiIoDevice->Pun,
                                          ScsiIoDevice->Lun,
                                          &ScsiDevicePath
                                          );
    if (Status == EFI_OUT_OF_RESOURCES) {
      gBS->FreePool (ScsiIoDevice);
      return Status;
    }
  }
  
  ScsiIoDevice->DevicePath = EfiAppendDevicePathNode (
                              ScsiBusDev->DevicePath,
                              ScsiDevicePath
                              );
  //
  // The memory space for ScsiDevicePath is allocated in
  // ScsiPassThru->BuildDevicePath() function; It is no longer used
  // after EfiAppendDevicePathNode,so free the memory it occupies.
  //
  gBS->FreePool (ScsiDevicePath);

  if (ScsiIoDevice->DevicePath == NULL) {
    gBS->FreePool (ScsiIoDevice);
    return EFI_OUT_OF_RESOURCES;
  }
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ScsiIoDevice->Handle,
                  &gEfiDevicePathProtocolGuid,
                  ScsiIoDevice->DevicePath,
                  &gEfiScsiIoProtocolGuid,
                  &ScsiIoDevice->ScsiIo,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (ScsiIoDevice);
    return EFI_OUT_OF_RESOURCES;
  } else {
    if (ScsiBusDev->ExtScsiSupport) {
      gBS->OpenProtocol (
            Controller,
            &gEfiExtScsiPassThruProtocolGuid,
            (VOID **) &(ScsiBusDev->ExtScsiInterface),
            This->DriverBindingHandle,
            ScsiIoDevice->Handle,
            EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
            );
     } else {
      gBS->OpenProtocol (
            Controller,
            &gEfiScsiPassThruProtocolGuid,
            (VOID **) &(ScsiBusDev->ScsiInterface),
            This->DriverBindingHandle,
            ScsiIoDevice->Handle,
            EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
            );
     }
  }
  return EFI_SUCCESS;
}

BOOLEAN
DiscoverScsiDevice (
  SCSI_IO_DEV   *ScsiIoDevice
  )
/*++

Routine Description:

  Discovery SCSI Device

Arguments:

  ScsiIoDevice    - The pointer of SCSI_IO_DEV

Returns:

  TRUE            - Find SCSI Device and verify it.
  FALSE           - Unable to find SCSI Device.  

--*/
{
  EFI_STATUS            Status;
  UINT32                InquiryDataLength;
  UINT8                 SenseDataLength;
  UINT8                 HostAdapterStatus;
  UINT8                 TargetStatus;
  EFI_SCSI_SENSE_DATA   SenseData;
  EFI_SCSI_INQUIRY_DATA InquiryData;

  
  HostAdapterStatus = 0;
  TargetStatus      = 0;
  //
  // Using Inquiry command to scan for the device
  //
  InquiryDataLength = sizeof (EFI_SCSI_INQUIRY_DATA);
  SenseDataLength   = sizeof (EFI_SCSI_SENSE_DATA);

  Status = SubmitInquiryCommand (
            &ScsiIoDevice->ScsiIo,
            EfiScsiStallSeconds (1),
            (VOID *) &SenseData,
            &SenseDataLength,
            &HostAdapterStatus,
            &TargetStatus,
            (VOID *) &InquiryData,
            &InquiryDataLength,
            FALSE
            );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }
  //
  // Retrieved inquiry data successfully
  //
  if ((InquiryData.Peripheral_Qualifier != 0) &&
      (InquiryData.Peripheral_Qualifier != 3)) {
    return FALSE;
  }

  if (InquiryData.Peripheral_Qualifier == 3) {
    if (InquiryData.Peripheral_Type != 0x1f) {
      return FALSE;
    }
  }

  if (0x1e >= InquiryData.Peripheral_Type >= 0xa) {
    return FALSE;
  }
  
  //
  // valid device type and peripheral qualifier combination.
  //
  ScsiIoDevice->ScsiDeviceType  = InquiryData.Peripheral_Type;
  ScsiIoDevice->RemovableDevice = InquiryData.RMB;
  if (InquiryData.Version == 0) {
    ScsiIoDevice->ScsiVersion = 0;
  } else {
    //
    // ANSI-approved version
    //
    ScsiIoDevice->ScsiVersion = (UINT8) (InquiryData.Version & 0x03);
  }

  return TRUE;
}
