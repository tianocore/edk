/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    scsibus.h

Abstract:

    Header file for SCSI Bus Driver.

Revision History
++*/

// TODO: fix comment to end with --*/
#ifndef _SCSI_BUS_H
#define _SCSI_BUS_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiPrintLib.h"
#include "scsi.h"
#include "ScsiLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ScsiPassThru)
#include EFI_PROTOCOL_DEFINITION (ScsiPassThruExt)
#include EFI_PROTOCOL_DEFINITION (DevicePath)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (ScsiIo)
#include EFI_PROTOCOL_DEFINITION (ComponentName)

//
// 1000 * 1000 * 10
//
#define ONE_SECOND_TIMER      10000000  

#define SCSI_IO_DEV_SIGNATURE EFI_SIGNATURE_32 ('s', 'c', 'i', 'o')

typedef struct {
  UINT32                             Signature;
  EFI_HANDLE                         Handle;
  EFI_SCSI_IO_PROTOCOL               ScsiIo;
  EFI_DEVICE_PATH_PROTOCOL           *DevicePath;
  BOOLEAN                            ExtScsiSupport; 
  EFI_SCSI_PASS_THRU_PROTOCOL        *ScsiPassThru;
  EFI_EXT_SCSI_PASS_THRU_PROTOCOL    *ExtScsiPassThru;
  UINT32                             Pun;
  UINT64                             Lun;
  UINT8                              ScsiDeviceType;
  UINT8                              ScsiVersion;
  BOOLEAN                            RemovableDevice;
} SCSI_IO_DEV;

#define SCSI_IO_DEV_FROM_THIS(a)  CR (a, SCSI_IO_DEV, ScsiIo, SCSI_IO_DEV_SIGNATURE)

//
// SCSI Bus Controller device strcuture
//
#define EFI_SCSI_BUS_PROTOCOL_GUID \
  { \
    0x5261213D, 0x3A3D, 0x441E, 0xB3, 0xAF, 0x21, 0xD3, 0xF7, 0xA4, 0xCA, 0x17 \
  }

typedef struct _EFI_SCSI_BUS_PROTOCOL {
  UINT64  Reserved;
} EFI_SCSI_BUS_PROTOCOL;

#define SCSI_BUS_DEVICE_SIGNATURE  EFI_SIGNATURE_32 ('s', 'c', 's', 'i')


typedef struct _SCSI_BUS_DEVICE {
  UINTN                                 Signature;
  EFI_SCSI_BUS_PROTOCOL                 BusIdentify;
  BOOLEAN                               ExtScsiSupport; 
  EFI_SCSI_PASS_THRU_PROTOCOL           *ScsiInterface;
  EFI_EXT_SCSI_PASS_THRU_PROTOCOL       *ExtScsiInterface;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
} SCSI_BUS_DEVICE;

#define SCSI_BUS_CONTROLLER_DEVICE_FROM_THIS(a)  CR (a, SCSI_BUS_DEVICE, BusIdentify, SCSI_BUS_DEVICE_SIGNATURE)


//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gScsiBusDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gScsiBusComponentName;

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
;

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
;

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
;

EFI_STATUS
EFIAPI
ScsiExecuteSCSICommand (
  IN  EFI_SCSI_IO_PROTOCOL                 *This,
  IN OUT  EFI_SCSI_IO_SCSI_REQUEST_PACKET  *CommandPacket,
  IN  EFI_EVENT                            Event
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
;

EFI_STATUS
ScsiScanCreateDevice (
  EFI_DRIVER_BINDING_PROTOCOL   *This,
  EFI_HANDLE                    Controller,
  UINT32                        Pun,
  UINT64                        Lun,
  SCSI_BUS_DEVICE              *ScsiBusDev
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
;

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
;
#endif
