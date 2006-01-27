/*++
Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  NetworkComboEntry.c

Abstract:

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiPrintLib.h"

EFI_STATUS
EFIAPI
InitializeBCDriver (
  IN EFI_HANDLE                       ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeSnpNiiDriver (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  );
  
EFI_STATUS
EFIAPI
PxeDhcp4DriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );    
  
EFI_DRIVER_ENTRY_POINT (InitializeNetworkWrapperDriver)

EFI_STATUS
InitializeNetworkWrapperDriver (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
/*++

  Routine Description:
    Initialize the base code drivers and install the driver binding

  Arguments:
    ImageHandle         - Driver image handle
    SystemTable         - System service table

  Returns:
    EFI_SUCCESS         - This driver was successfully bound

--*/
{
  EFI_STATUS  Status;
  
  Status = InitializeBCDriver (ImageHandle, SystemTable);
  if (EFI_ERROR (Status)) {
    goto error_done;
  }  
  
  Status = InitializeSnpNiiDriver (ImageHandle, SystemTable);
  if (EFI_ERROR (Status)) {
    goto error_done;
  }  

  Status = PxeDhcp4DriverEntryPoint (ImageHandle, SystemTable);
  if (EFI_ERROR (Status)) {
    goto error_done;
  }  
    
  return EFI_SUCCESS;

error_done:
  return Status;    
}

/* eof - bc.c */
