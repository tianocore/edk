/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    PlatDriOverLib.h
    
Abstract:

--*/

#ifndef PLAT_DRI_OVER_LIB_H_
#define PLAT_DRI_OVER_LIB_H_

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "Pci22.h"
#include "EfiVariable.h"

//
// Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_GUID_DEFINITION (GlobalVariable)
#include EFI_PROTOCOL_DEFINITION (LoadedImage)
#include EFI_PROTOCOL_DEFINITION (PciIO)
//
// Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (PlatformDriverOverride)
#include EFI_PROTOCOL_DEFINITION (BusSpecificDriverOverride)
//
//
//
#define PLATFORM_OVERRIDE_ITEM_SIGNATURE      EFI_SIGNATURE_32('p','d','o','i')
 typedef struct _PLATFORM_OVERRIDE_ITEM{
  UINTN                                 Signature;
  EFI_LIST_ENTRY              		      Link;
  UINT32					                      DriverInfoNum;
  EFI_DEVICE_PATH_PROTOCOL      		    *ControllerDevicePath;
  EFI_LIST_ENTRY              		      DriverInfoList;  //DRIVER_IMAGE_INFO List
  EFI_HANDLE				                    LastReturnedImageHandle;
} PLATFORM_OVERRIDE_ITEM;

#define DRIVER_IMAGE_INFO_SIGNATURE           EFI_SIGNATURE_32('p','d','i','i')
typedef struct _DRIVER_IMAGE_INFO{
  UINTN                                 Signature;
  EFI_LIST_ENTRY              		      Link;
  EFI_HANDLE				                    ImageHandle;
  EFI_DEVICE_PATH_PROTOCOL      		    *DriverImagePath;
  BOOLEAN				                        UnLoadable;
  BOOLEAN				                        UnStartable;
} DRIVER_IMAGE_INFO; 

#define DEVICE_PATH_STACK_ITEM_SIGNATURE           EFI_SIGNATURE_32('d','p','s','i')
typedef struct _DEVICE_PATH_STACK_ITEM{
  UINTN                                 Signature;
  EFI_LIST_ENTRY              		      Link;
  EFI_DEVICE_PATH_PROTOCOL      		    *DevicePath;
} DEVICE_PATH_STACK_ITEM; 

EFI_STATUS
EFIAPI
PushDevPathStack (
  IN  EFI_DEVICE_PATH_PROTOCOL    *DevicePath
  );

EFI_STATUS
EFIAPI
PopDevPathStack (
  OUT  EFI_DEVICE_PATH_PROTOCOL    **DevicePath
  );

BOOLEAN
EFIAPI
CheckExistInStack (
  IN  EFI_DEVICE_PATH_PROTOCOL    *DevicePath
  );
  
  
EFI_STATUS
EFIAPI
LibInstallPlatformDriverOverrideProtocol (
  EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL *gPlatformDriverOverride
  );
  
EFI_STATUS
EFIAPI
LibFreeMappingDatabase (
  IN  OUT  EFI_LIST_ENTRY        *MappingDataBase
  );
  
EFI_STATUS
EFIAPI
LibInitOverridesMapping (
  OUT  EFI_LIST_ENTRY        *MappingDataBase
  );
  
EFI_STATUS
EFIAPI
LibSaveOverridesMapping (
  IN  EFI_LIST_ENTRY          *MappingDataBase
  );
  
EFI_STATUS
EFIAPI
LibGetDriverFromMapping (
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL              * This,
  IN     EFI_HANDLE                                     ControllerHandle,
  IN OUT EFI_HANDLE                                     * DriverImageHandle,
  IN     EFI_LIST_ENTRY                                 * MappingDataBase,
  IN     EFI_HANDLE                                     CallerImageHandle
  );
  
EFI_STATUS
EFIAPI
LibDeleteOverridesVariables (
  VOID
  );
  
EFI_STATUS
EFIAPI
UpdateFvFileDevicePath (
  IN  OUT EFI_DEVICE_PATH_PROTOCOL      ** DevicePath,
  IN  EFI_GUID                          *FileGuid,
  IN  EFI_HANDLE                        CallerImageHandle
  );
  
VOID *
GetVariableAndSize (
  IN  CHAR16              *Name,
  IN  EFI_GUID            *VendorGuid,
  OUT UINTN               *VariableSize
  );
  
EFI_STATUS
ConnectDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePathToConnect
  );
  
EFI_STATUS
BdsConnectDeviceByPciClassType (
  UINT8     ClassType,
  UINT8     SubClassCode,
  UINT8     PI,
  BOOLEAN   Recursive
  );
  
EFI_STATUS
EFIAPI
LibCheckMapping (
  IN     EFI_DEVICE_PATH_PROTOCOL                       *ControllerDevicePath,
  IN     EFI_DEVICE_PATH_PROTOCOL                       *DriverImageDevicePath,
  IN     EFI_LIST_ENTRY                                 * MappingDataBase,
  OUT    UINT32                                         *DriverInfoNum,
  OUT    UINT32                                         *DriverImageNO
  );
  
EFI_STATUS
EFIAPI
LibInsertDriverImage (
  IN     EFI_DEVICE_PATH_PROTOCOL                       *ControllerDevicePath,
  IN     EFI_DEVICE_PATH_PROTOCOL                       *DriverImageDevicePath,
  IN     EFI_LIST_ENTRY                                 *MappingDataBase,
  IN     UINT32                                         DriverImageNO
  );
  
EFI_STATUS
EFIAPI
LibDeleteDriverImage (
  IN     EFI_DEVICE_PATH_PROTOCOL                       *ControllerDevicePath,
  IN     EFI_DEVICE_PATH_PROTOCOL                       *DriverImageDevicePath,
  IN     EFI_LIST_ENTRY                                 *MappingDataBase
  );
#endif
