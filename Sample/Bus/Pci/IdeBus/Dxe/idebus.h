/*++
Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    idebus.h

Abstract:

    Header file for IDE Bus Driver.

Revision History
++*/

#ifndef _IDE_BUS_H
#define _IDE_BUS_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "pci22.h"
#include "idedata.h"
#include "EfiCompNameSupport.h"

//
// Driver Consumed Protocols and GUIDs
//
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)

#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (IdeControllerInit)



//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (BlockIo)
#include EFI_PROTOCOL_DEFINITION (DiskInfo)

#define MAX_IDE_DEVICE 4
#define MAX_IDE_CHANNELS 2
#define MAX_IDE_DRIVES 2

typedef struct {
  BOOLEAN  HaveScannedDevice[MAX_IDE_DEVICE];
  BOOLEAN  DeviceFound[MAX_IDE_DEVICE]; 
  BOOLEAN  DeviceProcessed[MAX_IDE_DEVICE]; 
} IDE_BUS_DRIVER_PRIVATE_DATA;

#define IDE_BLK_IO_DEV_SIGNATURE   EFI_SIGNATURE_32('i','b','i','d')

typedef struct {
    UINT32                        Signature;
    
    EFI_HANDLE                    Handle;
    EFI_BLOCK_IO_PROTOCOL         BlkIo;
    EFI_BLOCK_IO_MEDIA            BlkMedia;
    EFI_DISK_INFO_PROTOCOL        DiskInfo;
    EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
    EFI_PCI_IO_PROTOCOL           *PciIo;
    IDE_BUS_DRIVER_PRIVATE_DATA   *IdeBusDriverPrivateData;

    //
    // Local Data for IDE interface goes here
    //
    EFI_IDE_CHANNEL               Channel;
    EFI_IDE_DEVICE                Device;
    UINT16                        Lun;
    IDE_DEVICE_TYPE               Type;
    
    IDE_BASE_REGISTERS            *IoPort;
    UINT16                        AtapiError;

    INQUIRY_DATA                  *pInquiryData; 
    EFI_IDENTIFY_DATA             *pIdData;
    ATA_PIO_MODE                  PioMode;
    ATA_UDMA_MODE                 UDma_Mode;
    CHAR8                         ModelName[41];
    REQUEST_SENSE_DATA            *SenseData;
    UINT8                         SenseDataNumber;
    UINT8                         *Cache;

    EFI_UNICODE_STRING_TABLE      *ControllerNameTable;
} IDE_BLK_IO_DEV;

#include "ComponentName.h"

#define IDE_BLOCK_IO_DEV_FROM_THIS(a) CR(a, IDE_BLK_IO_DEV, BlkIo, IDE_BLK_IO_DEV_SIGNATURE)
#define IDE_BLOCK_IO_DEV_FROM_DISK_INFO_THIS(a) CR(a, IDE_BLK_IO_DEV, DiskInfo, IDE_BLK_IO_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_GUID                          gIDEBusDriverGuid;
extern EFI_DRIVER_BINDING_PROTOCOL       gIDEBusDriverBinding;

#include "ide.h"

//
// Prototypes
// Driver model protocol interface
//
EFI_STATUS
IDEBusControllerDriverEntryPoint(
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  );

EFI_STATUS
IDEBusDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
IDEBusDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
IDEBusDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  UINTN                       NumberOfChildren,
  IN  EFI_HANDLE                  *ChildHandleBuffer
  );

  
//
// Block I/O Protocol Interface
//
EFI_STATUS
IDEBlkIoReset(
  IN  EFI_BLOCK_IO_PROTOCOL       *This,
  IN  BOOLEAN                     ExtendedVerification
  );
  
EFI_STATUS
IDEBlkIoReadBlocks(
  IN  EFI_BLOCK_IO_PROTOCOL       *This,
  IN  UINT32                      MediaId,
  IN  EFI_LBA                     LBA,
  IN  UINTN                       BufferSize,
  OUT VOID                        *Buffer
  );
  
EFI_STATUS
IDEBlkIoWriteBlocks(
  IN  EFI_BLOCK_IO_PROTOCOL       *This,
  IN  UINT32                      MediaId,
  IN  EFI_LBA                     LBA,
  IN  UINTN                       BufferSize,
  IN  VOID                        *Buffer
  );
  
EFI_STATUS
IDEBlkIoFlushBlocks(
  IN  EFI_BLOCK_IO_PROTOCOL       *This
  );
  
EFI_STATUS  
IDERegisterDecodeEnableorDisable(
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  IN  BOOLEAN                     Enable
  );  

EFI_STATUS
IDEDiskInfoInquiry (
  IN EFI_DISK_INFO_PROTOCOL       *This,
  IN OUT VOID                     *InquiryData,
  IN OUT UINT32                   *IntquiryDataSize
  );

EFI_STATUS
IDEDiskInfoIdentify (
  IN EFI_DISK_INFO_PROTOCOL       *This,
  IN OUT VOID                     *IdentifyData,
  IN OUT UINT32                   *IdentifyDataSize
  );

EFI_STATUS
IDEDiskInfoSenseData (
  IN EFI_DISK_INFO_PROTOCOL       *This,
  IN OUT VOID                     *SenseData,
  IN OUT UINT32                   *SenseDataSize,
  OUT UINT8                       *SenseDataNumber
  );

EFI_STATUS
IDEDiskInfoWhichIde (
  IN EFI_DISK_INFO_PROTOCOL       *This,
  OUT UINT32                      *IdeChannel,
  OUT UINT32                      *IdeDevice
  );

#endif
