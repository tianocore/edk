/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  Ps2KeyboardEntry.c

Abstract:

  The Entry Point of Ps2Keyboard Driver
  
--*/

#include "Ps2Keyboard.h"

//
// Driver entry point
//
EFI_DRIVER_ENTRY_POINT (InstallPs2KeyboardDriver)

EFI_STATUS
EFIAPI
InstallPs2KeyboardDriver (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++
  
  Routine Description:
  
    Driver entry point
  
  Arguments:
  
    ImageHandle           - Driver image handle
    SystemTable           - Pointer to common system table
  
  Returns:
  
    EFI_SUCCESS           - Driver Binding Protocol has been successfully installed.
    EFI_INVALID_PARAMETER - Invalid Parameter exists in installing protocol.
  
--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle,
           SystemTable,
           &gKeyboardControllerDriver,
           ImageHandle,
           &gPs2KeyboardComponentName,
           NULL,
           NULL
           );
}
