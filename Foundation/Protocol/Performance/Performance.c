/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    Performance.c

Abstract:

  Load File protocol as defined in the EFI 1.0 specification.

  Load file protocol exists to supports the addition of new boot devices, 
  and to support booting from devices that do not map well to file system. 
  Network boot is done via a LoadFile protocol.

  EFI 1.0 can boot from any device that produces a LoadFile protocol.

--*/

#include "Tiano.h"
#include EFI_PROTOCOL_DEFINITION (Performance)

EFI_GUID gEfiPerformanceProtocolGuid = EFI_PERFORMANCE_PROTOCOL_GUID;

EFI_GUID_STRING (&gEfiPerformanceProtocolGuid, "EFI Performance Protocol", "EFI 1.0  Performance Protocol");
