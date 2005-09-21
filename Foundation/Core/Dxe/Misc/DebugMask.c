/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DebugMask.c
    
Abstract:


--*/

#include "Tiano.h"
#include "DxeCore.h"
#include "DebugMask.h"

extern UINTN                     gErrorLevel;

EFI_STATUS
GetDebugMask (
  IN EFI_DEBUG_MASK_PROTOCOL      *This,             // Calling context
  IN OUT UINTN                    *CurrentDebugMask  // Ptr to store current debug mask
  )
/*++

Routine Description:
  DebugMask protocol member function.
  Gets the current debug mask for an image, on which this protocol has been installed.
  
Arguments:

  This              - Indicates calling context 
  CurrentDebugMask  - Ptr to store current debug mask

Returns:
  EFI_SUCCESS - Debug mask is retrieved successfully
  EFI_INVALID_PARAMETER - CurrentDebugMask is NULL.
  EFI_UNSUPPORTED - The handle on which this protocol is installed is not an image handle.

--*/
{
  DEBUG_MASK_PRIVATE_DATA        *Private;
  //
  // Check Parameter
  //
  if (CurrentDebugMask == NULL){
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Get Private Data
  //
  Private = DEBUG_MASK_PRIVATE_DATA_FROM_THIS(This);
  *CurrentDebugMask = Private->ImageDebugMask;
  return EFI_SUCCESS;
}
  

EFI_STATUS
SetDebugMask (
  IN  EFI_DEBUG_MASK_PROTOCOL     *This,             // Calling context
  IN  UINTN                       NewDebugMask       // New Debug Mask value to set
  )
/*++

Routine Description:
  DebugMask protocol member function.
  Updates the current debug mask for an image, on which this protocol has been installed.
  
Arguments:

  This          - Calling context
  NewDebugMask  - New Debug Mask value to set

Returns:
  EFI_SUCCESS - Debug mask is updated with the new value successfully
  EFI_UNSUPPORTED - The handle on which this protocol is installed is not an image handle.

--*/
{
  DEBUG_MASK_PRIVATE_DATA        *Private;
  //
  // Set Private Data
  //
  Private = DEBUG_MASK_PRIVATE_DATA_FROM_THIS(This);
  Private->ImageDebugMask = NewDebugMask;
  return EFI_SUCCESS;
}  


EFI_STATUS
InstallDebugMaskProtocol(
  IN EFI_HANDLE ImageHandle
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
{
  EFI_STATUS                 Status;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadImageInterface;
  DEBUG_MASK_PRIVATE_DATA    *DebugMaskPrivate;

  if (ImageHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Check Image Handle
  //
  Status = CoreHandleProtocol (
                  ImageHandle, 
                  &gEfiLoadedImageProtocolGuid, 
                  (VOID*)&LoadImageInterface
                  );
                  
  if (EFI_ERROR (Status)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Create Pool for Private Data
  //
  DebugMaskPrivate = CoreAllocateZeroBootServicesPool (sizeof (DEBUG_MASK_PRIVATE_DATA));
  
  if (DebugMaskPrivate == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Fill in private data structure
  //
  DebugMaskPrivate->Signature  =  DEBUGMASK_PRIVATE_DATA_SIGNATURE;
  DebugMaskPrivate->ImageDebugMask  =  gErrorLevel;
  DebugMaskPrivate->DebugMaskInterface.Revision  =  EFI_DEBUG_MASK_REVISION;
  DebugMaskPrivate->DebugMaskInterface.GetDebugMask  =  GetDebugMask;
  DebugMaskPrivate->DebugMaskInterface.SetDebugMask  =  SetDebugMask;
  //
  // Install Debug Mask Protocol in Image Handle
  //
  Status = CoreInstallProtocolInterface (
             &ImageHandle,
             &gEfiDebugMaskProtocolGuid,
             EFI_NATIVE_INTERFACE,
             &(DebugMaskPrivate->DebugMaskInterface)
             );

  return Status;
}

EFI_STATUS
UninstallDebugMaskProtocol(
  IN EFI_HANDLE ImageHandle
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
{
  EFI_STATUS                 Status;
  EFI_DEBUG_MASK_PROTOCOL    *DebugMaskInterface;
  DEBUG_MASK_PRIVATE_DATA    *DebugMaskPrivate;
  
  if (ImageHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Get Protocol from ImageHandle
  //
  Status = CoreHandleProtocol (
                  ImageHandle, 
                  &gEfiDebugMaskProtocolGuid, 
                  (VOID*)&DebugMaskInterface
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  DebugMaskPrivate = DEBUG_MASK_PRIVATE_DATA_FROM_THIS(DebugMaskInterface);
  //
  // Remove Protocol from ImageHandle
  //                    
  Status = CoreUninstallProtocolInterface (
             ImageHandle,
             &gEfiDebugMaskProtocolGuid,
             (VOID*)DebugMaskInterface
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Free Private Data Pool
  //
  Status = CoreFreePool(DebugMaskPrivate);
  return Status;
}
