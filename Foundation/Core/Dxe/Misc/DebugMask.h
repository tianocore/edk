/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
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
  )
/*++

Routine Description:

  Install debug mask protocol on an image handle.

Arguments:

  ImageHandle     - Image handle which debug mask protocol will install on

Returns:

  EFI_INVALID_PARAMETER   - Invalid image handle
  
  EFI_OUT_OF_RESOURCES    - No enough buffer could be allocated
  
  EFI_SUCCESS             - Debug mask protocol successfully installed

--*/
;
  
//
// Internal DebugMask Procotol Install/Uninstall Function
//
EFI_STATUS
InstallCoreDebugMaskProtocol (
  IN EFI_HANDLE      ImageHandle
  )
/*++

Routine Description:

  Install debug mask protocol on Dxe Core.

Arguments:

  ImageHandle     - Image handle which debug mask protocol will install on

Returns:

  EFI_INVALID_PARAMETER   - Invalid image handle
  
  EFI_OUT_OF_RESOURCES    - No enough buffer could be allocated
  
  EFI_SUCCESS             - Debug mask protocol successfully installed

--*/
;
  
EFI_STATUS
UninstallDebugMaskProtocol (
  IN EFI_HANDLE      ImageHandle
  )
/*++

Routine Description:

  Uninstall debug mask protocol on an image handle.

Arguments:

  ImageHandle     - Image handle which debug mask protocol will uninstall on

Returns:

  EFI_INVALID_PARAMETER   - Invalid image handle
  
  EFI_SUCCESS             - Debug mask protocol successfully uninstalled

--*/
;

#endif
