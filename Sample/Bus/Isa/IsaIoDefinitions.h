/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  IsaIoDefinitions.h
  
Abstract:
  
  The header file to choose the EFI_ISA_IO_PROTOCOL 
  or EFI_LIGHT_ISA_IO_PROTOCOL.
  
Revision History:

--*/

#ifndef _EFI_ISA_IO_DEFINITIONS_H
#define _EFI_ISA_IO_DEFINITIONS_H

#ifndef SIZE_REDUCTION_ISA_COMBINED

#include EFI_PROTOCOL_DEFINITION (IsaIo)
#define EFI_INTERFACE_DEFINITION_FOR_ISA_IO EFI_ISA_IO_PROTOCOL
#define  EFI_ISA_IO_PROTOCOL_VERSION &gEfiIsaIoProtocolGuid
#define EFI_ISA_IO_OPERATION_TOKEN   EfiIsaIoOperationBusMasterWrite

#else

#include EFI_PROTOCOL_DEFINITION (LightIsaIo)
#define EFI_INTERFACE_DEFINITION_FOR_ISA_IO EFI_LIGHT_ISA_IO_PROTOCOL
#define  EFI_ISA_IO_PROTOCOL_VERSION &gEfiLightIsaIoProtocolGuid
#define EFI_ISA_IO_OPERATION_TOKEN   EfiIsaIoOperationSlaveWrite  
#define ADD_SERIAL_NAME(x, y)

#endif


#endif
