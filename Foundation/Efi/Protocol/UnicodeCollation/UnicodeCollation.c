/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  UnicodeCollation.c

Abstract:

  Unicode Collation protocol that follows the EFI 1.0 specification.

--*/

#include "EfiSpec.h"
#include EFI_PROTOCOL_DEFINITION (UnicodeCollation)

EFI_GUID  gEfiUnicodeCollationProtocolGuid = EFI_UNICODE_COLLATION_PROTOCOL_GUID;
EFI_GUID  gEfiUnicodeCollation2ProtocolGuid = EFI_UNICODE_COLLATION2_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiUnicodeCollationProtocolGuid, "Unicode Collation Protocol", "EFI 1.0 Unicode Collation Protocol");
EFI_GUID_STRING(&gEfiUnicodeCollation2ProtocolGuid, "Unicode Collation Protocol", "UEFI 2.10 Unicode Collation Protocol");
