/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DebuggerConfiguration.c

Abstract:

  DebuggerConfiguration Protocol

--*/

#include "Tiano.h"
#include "DebuggerConfiguration.h"


EFI_GUID gEfiDebuggerConfigurationProtocolGuid = EFI_DEBUGGER_CONFIGURATION_PROTOCOL_GUID;

EFI_GUID_STRING (&gEfiDebuggerConfigurationProtocolGuid, "Debugger Configuration Protocol", "Debugger Configuration Protocol");

