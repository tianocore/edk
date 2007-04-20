/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EbcDemo.c

Abstract:

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"

EFI_DRIVER_ENTRY_POINT (InitializeEbcDriver)

EFI_STATUS
EFIAPI
InitializeEbcDriver (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

Arguments:

  ImageHandle - EFI image handle.
  SystemTable - Pointer to the EFI system table.

Returns:
  Standard EFI status code.

--*/
{
  UINTN     Index;
  UINT8     Array[10];

  for (Index = 0; Index < 40; Index++) {
    Array[Index] = Index;
  }

  return EFI_SUCCESS;
}
