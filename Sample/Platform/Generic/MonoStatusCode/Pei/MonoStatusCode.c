/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MonoStatusCode.c

Abstract:

  PEIM to provide the status code functionality, to aid in system debug.
  It includes output to 0x80 port and/or to serial port.  
  This PEIM is monolithic. Different platform should provide different library.

--*/

#include "MonoStatusCode.h"

//
// Define PEIM entry point
//
EFI_PEIM_ENTRY_POINT (InstallMonoStatusCode)
//
// Module globals
//
EFI_GUID mStatusCodeRuntimeGuid = EFI_STATUS_CODE_RUNTIME_PROTOCOL_GUID;
PEI_STATUS_CODE_PPI     mStatusCodePpi = { PlatformReportStatusCode };

EFI_PEI_PPI_DESCRIPTOR  mPpiListStatusCode = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiStatusCodePpiGuid,
  &mStatusCodePpi
};

//
// Function implemenations
//
EFI_STATUS
EFIAPI
TranslateDxeStatusCodeToPeiStatusCode (
  IN EFI_STATUS_CODE_TYPE     CodeType,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 * CallerId,
  IN EFI_STATUS_CODE_DATA     * Data OPTIONAL
  )
/*++

Routine Description:

  Translate from a DXE status code interface into a PEI-callable
  interface, making the PEI the least common denominator..

Arguments:

  Same as DXE ReportStatusCode RT service
  
Returns:

  None

--*/
{
  return PlatformReportStatusCode (NULL, CodeType, Value, Instance, CallerId, Data);
}

EFI_STATUS
EFIAPI
InitializeDxeReportStatusCode (
  IN EFI_PEI_SERVICES       **PeiServices
  )
/*++

Routine Description:

  Build a hob describing the status code listener that has been installed.
  This will be used by DXE code until a runtime status code listener is 
  installed.

Arguments:

  PeiServices      - General purpose services available to every PEIM.
    
Returns:

  Status -  EFI_SUCCESS if the interface could be successfully
            installed

--*/
{
  EFI_STATUS  Status;
  VOID        *Instance;

  Instance = (VOID *) (UINTN) TranslateDxeStatusCodeToPeiStatusCode;

  Status = PeiBuildHobGuidData (
            PeiServices,
            &mStatusCodeRuntimeGuid,
            &Instance,
            sizeof (VOID *)
            );

  return Status;
}

VOID
EFIAPI
InitializeMonoStatusCode (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Initialize the platform status codes and publish the platform status code 
  PPI.

Arguments:

  FfsHeader   - FV this PEIM was loaded from.
  PeiServices - General purpose services available to every PEIM.
    
Returns:

  Status -  EFI_SUCCESS

--*/
{
  EFI_STATUS  Status;

  //
  // Initialize status code listeners.
  //
  PlatformInitializeStatusCode (FfsHeader, PeiServices);

  //
  // Publish the status code capability to other modules
  //
  Status = (*PeiServices)->InstallPpi (PeiServices, &mPpiListStatusCode);

  ASSERT_PEI_ERROR (PeiServices, Status);

  PEI_DEBUG ((PeiServices, EFI_D_ERROR, "\nMono Status Code PEIM Loaded\n"));

  return ;
}
