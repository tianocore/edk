/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

 NtFwh.c

Abstract:

  NT FWH Information PPI GUID as defined in EFI 2.0

--*/

#include "Tiano.h"
#include "PeiBind.h"
#include "PeiApi.h"
#include EFI_PPI_DEFINITION(NtFwh)

EFI_GUID gPeiFwhInformationGuid = PEI_NT_FWH_PRIVATE_GUID;

EFI_GUID_STRING(&gPeiFwhInformationGuid, "NtFwh", "NT PEI FWH INFO PPI");
