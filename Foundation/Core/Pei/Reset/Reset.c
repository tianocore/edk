/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Reset.c

Abstract:

  Pei Core Reset System Support

Revision History

--*/

#include "Tiano.h"
#include "PeiCore.h"
#include "PeiLib.h"  

#include EFI_PPI_DEFINITION (Reset)

EFI_STATUS
EFIAPI
PeiCoreResetSystem (
  IN EFI_PEI_SERVICES         **PeiServices
  )
/*++

Routine Description:

  Core version of the Reset System

Arguments:

  PeiServices - The PEI core services table.

Returns:

  Status  - EFI_NOT_AVAILABLE_YET. PPI not available yet.
          - EFI_DEVICE_ERROR.   Did not reset system.
          
  Otherwise, resets the system. 

--*/
{
  EFI_STATUS        Status;
  PEI_RESET_PPI     *ResetPpi;

  Status = PeiLocatePpi (
             PeiServices,
             &gPeiResetPpiGuid,         
             0,                         
             NULL,                      
             &ResetPpi                  
             );

  //
  // LocatePpi returns EFI_NOT_FOUND on error
  //
  if (!EFI_ERROR (Status)) {
    return ResetPpi->ResetSystem (PeiServices);
  } 
  return  EFI_NOT_AVAILABLE_YET;
}

