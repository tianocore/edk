/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DebugMask.h
    
Abstract:


--*/

#ifndef __DEBUG_MASK_INFO_H__
#define __DEBUG_MASK_INFO_H__

#include EFI_PROTOCOL_CONSUMER (DebugMask)

//
// local type definitions
//
#define DEBUGMASK_PRIVATE_DATA_SIGNATURE  EFI_SIGNATURE_32('D','M','S','K')

//
// Private structure used by driver
//
typedef struct {
  UINT32                            Signature;
  UINTN                             ImageDebugMask;
  EFI_DEBUG_MASK_PROTOCOL           DebugMaskInterface;
}DEBUG_MASK_PRIVATE_DATA;

#define DEBUG_MASK_PRIVATE_DATA_FROM_THIS(a) \
  CR(a, DEBUG_MASK_PRIVATE_DATA, DebugMaskInterface, DEBUGMASK_PRIVATE_DATA_SIGNATURE)

//
// Internal DebugMask Procotol Install/Uninstall Function
//
EFI_STATUS
InstallDebugMaskProtocol (
  IN EFI_HANDLE      ImageHandle
  );
  
EFI_STATUS
UninstallDebugMaskProtocol (
  IN EFI_HANDLE      ImageHandle
  );

#endif
