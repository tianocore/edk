/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  TcgEfiClient.h

Abstract:

  This file contains function prototypes for DXE core to measure whatever need
  to be measured as stipulated by TCG EFI platform spec.
  Please refer to https://www.trustedcomputinggroup.org/specs/PCClient/

--*/

#ifndef _TCG_EFI_CLIENT_H_
#define _TCG_EFI_CLIENT_H_

#include <Tiano.h>

extern
EFI_STATUS
EFIAPI
CoreMeasurePeImage (
  IN      BOOLEAN                   BootPolicy,
  IN      EFI_PHYSICAL_ADDRESS      ImageBase,
  IN      UINTN                     ImageLength,
  IN      UINTN                     LinkTimeBase,
  IN      UINT16                    ImageType,
  IN      EFI_HANDLE                DeviceHandle,
  IN      EFI_DEVICE_PATH_PROTOCOL  *FilePath
  );

#define EFI_CALLING_EFI_APPLICATION         \
  "Calling EFI Application from Boot Option"
#define EFI_RETURNING_FROM_EFI_APPLICATOIN  \
  "Returning from EFI Application from Boot Option"
#define EFI_EXIT_BOOT_SERVICES_INVOCATION   \
  "Exit Boot Services Invocation"
#define EFI_EXIT_BOOT_SERVICES_FAILED       \
  "Exit Boot Services Returned with Failure"
#define EFI_EXIT_BOOT_SERVICES_SUCCEEDED    \
  "Exit Boot Services Returned with Success"

extern
EFI_STATUS
EFIAPI
CoreMeasureAction (
  IN      CHAR8                     *ActionString
  );

#endif  // _TCG_EFI_CLIENT_H_
