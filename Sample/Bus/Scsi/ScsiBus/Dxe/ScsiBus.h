/*++

Copyright 2004, Intel Corporation                                                         
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
#include EFI_PROTOCOL_DEFINITION (DevicePath)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (ScsiIo)
#include EFI_PROTOCOL_DEFINITION (ComponentName)

#define ONE_SECOND_TIMER  10000000    // 1000 * 1000 * 10

#define SCSI_IO_DEV_SIGNATURE   EFI_SIGNATURE_32('s','c','i','o')

typedef struct {
  UINT32                        Signature;

  EFI_HANDLE                    Handle;
  EFI_SCSI_IO_PROTOCOL          ScsiIo;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  EFI_SCSI_PASS_THRU_PROTOCOL   *ScsiPassThru;
  
  UINT32                        Pun;
  UINT64                        Lun;
  UINT8                         ScsiDeviceType;
  UINT8                         ScsiVersion;
  BOOLEAN                       RemovableDevice;
} SCSI_IO_DEV;

#define SCSI_IO_DEV_FROM_THIS(a) CR(a, SCSI_IO_DEV,ScsiIo, SCSI_IO_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gScsiBusDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gScsiBusComponentName;

EFI_STATUS
ScsiGetDeviceType (
  IN  EFI_SCSI_IO_PROTOCOL     *This,
  OUT UINT8                    *DeviceType
  );

EFI_STATUS
ScsiResetBus (
  IN  EFI_SCSI_IO_PROTOCOL     *This
  );

EFI_STATUS
ScsiResetDevice (
  IN  EFI_SCSI_IO_PROTOCOL     *This
  );

EFI_STATUS
ScsiExecuteSCSICommand (
  IN  EFI_SCSI_IO_PROTOCOL                 *This,
  IN OUT  EFI_SCSI_IO_SCSI_REQUEST_PACKET  *CommandPacket,
  IN  EFI_EVENT                            Event
  );    

EFI_STATUS
ScsiScanCreateDevice (
  EFI_DRIVER_BINDING_PROTOCOL   *This,
  EFI_HANDLE                    Controller,  
  UINT32                        Pun,
  UINT64                        Lun,
  EFI_SCSI_PASS_THRU_PROTOCOL   *ScsiPassThru,
  EFI_DEVICE_PATH_PROTOCOL      *ParentDevicePath
  );
  
BOOLEAN
DiscoverScsiDevice (
  SCSI_IO_DEV   *ScsiIoDevice
  );

EFI_STATUS
GetLunList(
  EFI_SCSI_PASS_THRU_PROTOCOL *ScsiPassThru,
  UINT32                      Target,
  UINT64                      **LunArray,
  UINTN                       *NumberOfLuns  
  );
  
EFI_STATUS
ScsiBusSubmitReportLunCommand (
  EFI_SCSI_PASS_THRU_PROTOCOL   *ScsiPassThru,
  UINT32                        Target,
  UINTN                         AllocationLength,
  VOID                          *Buffer,
  EFI_SCSI_SENSE_DATA           *SenseData,
  UINT8                         *SenseDataLength,
  UINT8                         *HostAdapterStatus,
  UINT8                         *TargetStatus
  );  
#endif
