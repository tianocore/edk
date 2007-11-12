/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    PlatformDriOverride.h
    
Abstract:

--*/

#ifndef PLATFORM_DRI_OVERRIDE_H_
#define PLATFORM_DRI_OVERRIDE_H_

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "PlatDriOverLib.h"

//
// Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_GUID_DEFINITION (GlobalVariable)

//
// Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (PlatformDriverOverride)

STATIC
EFI_STATUS
EFIAPI
GetDriver (
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL              * This,
  IN     EFI_HANDLE                                     ControllerHandle,
  IN OUT EFI_HANDLE                                     * DriverImageHandle
  );
  
STATIC
EFI_STATUS
EFIAPI
GetDriverPath (
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL              * This,
  IN     EFI_HANDLE                                     ControllerHandle,
  IN OUT EFI_DEVICE_PATH_PROTOCOL                       **DriverImagePath
  );
  
STATIC
EFI_STATUS
EFIAPI
DriverLoaded (
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL          * This,
  IN EFI_HANDLE                                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL                       * DriverImagePath,
  IN EFI_HANDLE                                     DriverImageHandle
  );
#endif
