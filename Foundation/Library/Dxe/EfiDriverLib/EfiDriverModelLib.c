/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiDriverModelLib.c

Abstract:

  Light weight lib to support EFI drivers.

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"

EFI_STATUS
EfiLibInstallDriverBinding (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding,
  IN EFI_HANDLE                   DriverBindingHandle
  )
/*++

Routine Description:

  Intialize a driver by installing the Driver Binding Protocol onto the 
  driver's DriverBindingHandle.  This is typically the same as the driver's
  ImageHandle, but it can be different if the driver produces multiple
  DriverBinding Protocols.  This function also initializes the EFI Driver
  Library that initializes the global variables gST, gBS, gRT.

Arguments:

  ImageHandle         - The image handle of the driver

  SystemTable         - The EFI System Table that was passed to the driver's entry point

  DriverBinding       - A Driver Binding Protocol instance that this driver is producing

  DriverBindingHandle - The handle that DriverBinding is to be installe onto.  If this
                        parameter is NULL, then a new handle is created.

Returns: 

  EFI_SUCCESS is DriverBinding is installed onto DriverBindingHandle

  Otherwise, then return status from gBS->InstallProtocolInterface()

--*/
{
  EfiInitializeDriverLib (ImageHandle, SystemTable);

  DriverBinding->ImageHandle          = ImageHandle;

  DriverBinding->DriverBindingHandle  = DriverBindingHandle;

  return gBS->InstallProtocolInterface (
                &DriverBinding->DriverBindingHandle,
                &gEfiDriverBindingProtocolGuid,
                EFI_NATIVE_INTERFACE,
                DriverBinding
                );
}

EFI_STATUS
EfiLibInstallAllDriverProtocols (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   * SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL        * DriverBinding,
  IN EFI_HANDLE                         DriverBindingHandle,
  IN EFI_COMPONENT_NAME_PROTOCOL        * ComponentName, OPTIONAL
  IN EFI_DRIVER_CONFIGURATION_PROTOCOL  * DriverConfiguration, OPTIONAL
  IN EFI_DRIVER_DIAGNOSTICS_PROTOCOL    * DriverDiagnostics OPTIONAL
  )
/*++

Routine Description:

  Intialize a driver by installing the Driver Binding Protocol onto the 
  driver's DriverBindingHandle.  This is typically the same as the driver's
  ImageHandle, but it can be different if the driver produces multiple
  DriverBinding Protocols.  This function also initializes the EFI Driver
  Library that initializes the global variables gST, gBS, gRT.

Arguments:

  ImageHandle         - The image handle of the driver

  SystemTable         - The EFI System Table that was passed to the driver's entry point

  DriverBinding       - A Driver Binding Protocol instance that this driver is producing

  DriverBindingHandle - The handle that DriverBinding is to be installe onto.  If this
                        parameter is NULL, then a new handle is created.

  ComponentName       - A Component Name Protocol instance that this driver is producing

  DriverConfiguration - A Driver Configuration Protocol instance that this driver is producing
  
  DriverDiagnostics   - A Driver Diagnostics Protocol instance that this driver is producing

Returns: 

  EFI_SUCCESS if all the protocols were installed onto DriverBindingHandle

  Otherwise, then return status from gBS->InstallProtocolInterface()

--*/
{
  EFI_STATUS  Status;

  Status = EfiLibInstallDriverBinding (ImageHandle, SystemTable, DriverBinding, DriverBindingHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (ComponentName != NULL) {
    Status = gBS->InstallProtocolInterface (
                    &DriverBinding->DriverBindingHandle,
                    &gEfiComponentNameProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    ComponentName
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (DriverConfiguration != NULL) {
    Status = gBS->InstallProtocolInterface (
                    &DriverBinding->DriverBindingHandle,
                    &gEfiDriverConfigurationProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    DriverConfiguration
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (DriverDiagnostics != NULL) {
    Status = gBS->InstallProtocolInterface (
                    &DriverBinding->DriverBindingHandle,
                    &gEfiDriverDiagnosticsProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    DriverDiagnostics
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}
