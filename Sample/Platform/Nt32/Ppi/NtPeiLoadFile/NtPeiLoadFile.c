/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

 NtPeiLoadFile.c

Abstract:

  Abstraction for the NT Load Image PPI GUID as defined in EFI 2.0

--*/

#include "Tiano.h"
#include "PeiBind.h"
#include "PeiApi.h"
#include EFI_PPI_DEFINITION(NtPeiLoadFile)

EFI_GUID gPeiLoadFileGuid = PEI_LOAD_FILE_PRIVATE_GUID; 

EFI_GUID_STRING(&gPeiLoadFileGuid, "NtPeiLoadFile", "NT PEI LOAD FILE PPI");
