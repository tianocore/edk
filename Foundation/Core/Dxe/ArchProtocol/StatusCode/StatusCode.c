/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  StatusCode.c

Abstract:

  Status code Architectural Protocol as defined in EFI 2.0

  This code abstracts Status Code reporting.

--*/

#include "Tiano.h"
#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)

EFI_GUID gEfiStatusCodeArchProtocolGuid = EFI_STATUS_CODE_ARCH_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiStatusCodeArchProtocolGuid, "Status Code", "Status Code Arch Protocol");
