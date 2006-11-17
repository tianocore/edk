/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  SerialEntry.c
  
Abstract:

  The Entry Point of Serial Driver.

Revision History:

--*/

#include "Serial.h"

EFI_DRIVER_ENTRY_POINT (SerialControllerDriverEntryPoint)
//
// ISA Bus Driver Support Functions
//
EFI_STATUS
EFIAPI
SerialControllerDriverEntryPoint (
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
  return INSTALL_ALL_DRIVER_PROTOCOLS (
           ImageHandle,
           SystemTable,
           &gSerialControllerDriver,
           ImageHandle,
           &gIsaSerialComponentName,
           NULL,
           NULL
           );
}
