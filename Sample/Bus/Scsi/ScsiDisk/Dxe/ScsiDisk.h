/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ScsiDisk.h

Abstract:
  
  Header file for SCSI Disk Driver.

--*/

#ifndef _SCSI_DISK_H
#define _SCSI_DISK_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiPrintLib.h"
#include "scsi.h"
#include "ScsiLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (ScsiIo)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (BlockIo)
#include EFI_PROTOCOL_DEFINITION (ComponentName)

#define IsDeviceFixed(a)  (a)->FixedDevice ? 1:0

#define SCSI_DISK_DEV_SIGNATURE   EFI_SIGNATURE_32('s','c','d','k')

typedef struct {
  UINT32          Signature;
  
  EFI_HANDLE      Handle;
  
  EFI_BLOCK_IO_PROTOCOL     BlkIo;
  EFI_BLOCK_IO_MEDIA        BlkIoMedia;
  EFI_SCSI_IO_PROTOCOL      *ScsiIo;
  UINT8                     DeviceType;
  BOOLEAN                   FixedDevice;
  UINT16                    Reserved;
  
  EFI_SCSI_SENSE_DATA       *SenseData;
  UINTN                     SenseDataNumber;
  EFI_SCSI_INQUIRY_DATA     InquiryData;
  
  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;
  
} SCSI_DISK_DEV;

#define SCSI_DISK_DEV_FROM_THIS(a) CR(a, SCSI_DISK_DEV,BlkIo, SCSI_DISK_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gScsiDiskDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gScsiDiskComponentName;
//
// action code used in detect media process
//
#define ACTION_NO_ACTION            0x00
#define ACTION_READ_CAPACITY        0x01
#define ACTION_RETRY_COMMAND_LATER  0x02

EFI_STATUS
ScsiDiskReset (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  BOOLEAN                 ExtendedVerification
  );
  
EFI_STATUS
ScsiDiskReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  OUT VOID                    *Buffer
  );
  
EFI_STATUS
ScsiDiskWriteBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  IN  VOID                    *Buffer
  );
  
EFI_STATUS
ScsiDiskFlushBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This
  );
  
EFI_STATUS
ScsiDiskDetectMedia (
  SCSI_DISK_DEV   *ScsiDiskDevice,
  BOOLEAN         MustReadCap,
  BOOLEAN         *MediaChange
  );
  
EFI_STATUS
ScsiDiskTestUnitReady (
  SCSI_DISK_DEV   *ScsiDiskDevice,
  BOOLEAN         *NeedRetry,
  VOID            **SenseDataArray,
  UINTN           *NumberOfSenseKeys
  );
  
EFI_STATUS
DetectMediaParsingSenseKeys (
  SCSI_DISK_DEV           *ScsiDiskDevice,
  EFI_SCSI_SENSE_DATA     *SenseData,
  UINTN                   NumberOfSenseKeys,
  UINTN                   *Action
  );
  
EFI_STATUS
ScsiDiskReadCapacity (
  SCSI_DISK_DEV   *ScsiDiskDevice,
  BOOLEAN         *NeedRetry,
  VOID            **SenseDataArray,
  UINTN           *NumberOfSenseKeys
  );
  
EFI_STATUS
CheckHostAdapterStatus (
  UINT8   HostAdapterStatus
  );
  
EFI_STATUS
CheckTargetStatus (
  UINT8   TargetStatus
  );
  
EFI_STATUS
ScsiDiskRequestSenseKeys (
  SCSI_DISK_DEV           *ScsiDiskDevice,
  BOOLEAN                 *NeedRetry,
  EFI_SCSI_SENSE_DATA     **SenseDataArray,
  UINTN                   *NumberOfSenseKeys,
  BOOLEAN                 AskResetIfError
  );

EFI_STATUS
ScsiDiskInquiryDevice (
  SCSI_DISK_DEV   *ScsiDiskDevice,
  BOOLEAN         *NeedRetry
  );

VOID
ParseInquiryData (
  SCSI_DISK_DEV   *ScsiDiskDevice
  );
      
EFI_STATUS
ScsiDiskReadSectors (
  SCSI_DISK_DEV     *ScsiDiskDevice,
  VOID              *Buffer,
  EFI_LBA           Lba,
  UINTN             NumberOfBlocks
  );
  
EFI_STATUS
ScsiDiskWriteSectors (
  SCSI_DISK_DEV     *ScsiDiskDevice,
  VOID              *Buffer,
  EFI_LBA           Lba,
  UINTN             NumberOfBlocks
  );

EFI_STATUS
ScsiDiskRead10 (
  SCSI_DISK_DEV         *ScsiDiskDevice,
  BOOLEAN               *NeedRetry,
  EFI_SCSI_SENSE_DATA   **SenseDataArray,
  UINTN                 *NumberOfSenseKeys,
  UINT64                Timeout,
  UINT8                 *DataBuffer,
  UINT32                *DataLength,
  UINT32                StartLba,
  UINT32                SectorSize
  );

EFI_STATUS
ScsiDiskWrite10 (
  SCSI_DISK_DEV         *ScsiDiskDevice,
  BOOLEAN               *NeedRetry,
  EFI_SCSI_SENSE_DATA   **SenseDataArray,
  UINTN                 *NumberOfSenseKeys,
  UINT64                Timeout,
  UINT8                 *DataBuffer,
  UINT32                *DataLength,
  UINT32                StartLba,
  UINT32                SectorSize
  );
    
VOID
GetMediaInfo (
  SCSI_DISK_DEV                 *ScsiDiskDevice,
  EFI_SCSI_DISK_CAPACITY_DATA   *Capacity
  );

BOOLEAN
ScsiDiskIsNoMedia(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  );

BOOLEAN
ScsiDiskIsMediaError(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  );


BOOLEAN
ScsiDiskIsHardwareError(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  );
  
BOOLEAN
ScsiDiskIsMediaChange(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  );


BOOLEAN
ScsiDiskIsResetBefore(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  );


BOOLEAN
ScsiDiskIsDriveReady(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts,
  OUT BOOLEAN               *NeedRetry
  );

BOOLEAN
ScsiDiskHaveSenseKey(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  );

VOID
ReleaseScsiDiskDeviceResources (
  IN  SCSI_DISK_DEV   *ScsiDiskDevice
  );
  
#endif
