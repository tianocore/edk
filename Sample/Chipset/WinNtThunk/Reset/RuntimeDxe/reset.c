/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

 Reset.c

Abstract:

  Reset Architectural Protocol as defined in EFI 2.0 under NT Emulation

--*/

#include "Efi2WinNT.h"
#include "EfiWinNtLib.h"
#include "EfiRuntimeLib.h"
#include EFI_ARCH_PROTOCOL_DEFINITION (Reset)

EFI_STATUS
EFIAPI
InitializeNtReset (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

STATIC
EFI_STATUS
WinNtResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL
  );

EFI_DRIVER_ENTRY_POINT(InitializeNtReset)

EFI_STATUS
EFIAPI
InitializeNtReset (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:


Arguments:

  ImageHandle of the loaded driver
  Pointer to the System Table

Returns:

  Status
--*/
{
  EFI_STATUS Status;
  EFI_HANDLE Handle;

  EfiInitializeRuntimeDriverLib (ImageHandle, SystemTable, NULL);
  EfiInitializeWinNtDriverLib (ImageHandle, SystemTable);

  SystemTable->RuntimeServices->ResetSystem = WinNtResetSystem;

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiResetArchProtocolGuid, NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}


STATIC
EFI_STATUS
WinNtResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL
  )
{
  //
  // BUGBUG Need to kill all console windows later
  //   
  
  //
  // Discard ResetType, always return 0 as exit code
  //
  gWinNt->ExitProcess (0);

  //
  // Should never go here
  //
  return EFI_SUCCESS;
}
