/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  IsaFloppyEntry.c

Abstract:

  The Entry Point of Isa Floppy Driver

Revision History:

--*/

#include "IsaFloppy.h"

EFI_DRIVER_ENTRY_POINT (FdcControllerDriverEntryPoint)

EFI_STATUS
EFIAPI
FdcControllerDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++
  
  Routine Description:
  
    The entry point of the Floppy driver
  
  Arguments:
  
    ImageHandle         - The incoming image handle
    SystemTable         - The System Table
  
  Returns:
  
    EFI_ALREADY_STARTED - the driver is already started
    EFI_SUCCESS         - the driver load successfully
  
--*/
{
  return INSTALL_ALL_DRIVER_PROTOCOLS (
           ImageHandle,
           SystemTable,
           &gFdcControllerDriver,
           ImageHandle,
           &gIsaFloppyComponentName,
           NULL,
           NULL
           );
}
