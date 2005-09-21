/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  StatusCode.h

Abstract:

  Status code Architectural Protocol as defined in the DXE CIS

  This code abstracts Status Code reporting.

  The StatusCode () Tiano service is added to the EFI system table and the 
  EFI_STATUS_CODE_ARCH_PROTOCOL_GUID protocol is registered with a NULL 
  pointer.

  No CRC of the EFI system table is required, as it is done in the DXE core.

--*/

#ifndef _ARCH_PROTOCOL_STATUS_CODE_H__
#define _ARCH_PROTOCOL_STATUS_CODE_H__

//
// Global ID for the Status Code Architectural Protocol
//
#define EFI_STATUS_CODE_ARCH_PROTOCOL_GUID \
  { 0xd98e3ea3, 0x6f39, 0x4be4, 0x82, 0xce, 0x5a, 0x89, 0xc, 0xcb, 0x2c, 0x95 }

extern EFI_GUID gEfiStatusCodeArchProtocolGuid;

#endif
