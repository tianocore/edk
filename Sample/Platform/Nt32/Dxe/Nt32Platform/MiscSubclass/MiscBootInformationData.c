/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MiscBootInformationData.c
  
Abstract: 

  This driver parses the mMiscSubclassDataTable structure and reports
  any generated data to the DataHub.

--*/

#include "MiscSubclassDriver.h"

//
// Static (possibly build generated) Bios Vendor data.
//
MISC_SUBCLASS_TABLE_DATA(EFI_MISC_BOOT_INFORMATION_STATUS, BootInformationStatus) = {
  EfiBootInformationStatusNoError,  // BootInformationStatus
  0                                 // BootInformationData
};

/* eof - MiscBootInformationData.c */
