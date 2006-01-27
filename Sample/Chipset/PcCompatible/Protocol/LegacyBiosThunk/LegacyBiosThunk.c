/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LegacyBiosThunk.c
    
Abstract:

  EFI Legacy BIOS Protocol

Revision History

--*/

#include "Tiano.h"

#include EFI_PROTOCOL_DEFINITION (LegacyBiosThunk)

EFI_GUID  gLegacyBiosThunkProtocolGuid = LEGACY_BIOS_THUNK_PROTOCOL_GUID;

EFI_GUID_STRING(&gLegacyBiosThunkProtocolGuid, "Legacy BIOS Thunk Protocol", "Legacy BIOS Thunk Protocol");
