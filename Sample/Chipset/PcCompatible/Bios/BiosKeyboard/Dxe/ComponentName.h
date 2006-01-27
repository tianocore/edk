/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  ComponentName.h
    
Abstract: 

Revision History
--*/

#ifndef _BIOS_KEYBOARD_COMPONENT_NAME_H
#define _BIOS_KEYBOARD_COMPONENT_NAME_H

#include "Tiano.h"

#ifndef EFI_SIZE_REDUCTION_APPLIED

#include EFI_PROTOCOL_DEFINITION (ComponentName)

extern EFI_COMPONENT_NAME_PROTOCOL  gBiosKeyboardComponentName;

//
// EFI Component Name Functions
//
EFI_STATUS
EFIAPI
BiosKeyboardComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  Language    - TODO: add argument description
  DriverName  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
BiosKeyboardComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This              - TODO: add argument description
  ControllerHandle  - TODO: add argument description
  ChildHandle       - TODO: add argument description
  Language          - TODO: add argument description
  ControllerName    - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif

#endif
