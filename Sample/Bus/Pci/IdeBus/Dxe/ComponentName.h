/*++
Copyright 2004, Intel Corporation                                                         
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
++*/

#ifndef _IDE_BUS_COMPONENT_NAME_H
#define _IDE_BUS_COMPONENT_NAME_H

#include "Tiano.h"

#ifndef EFI_SIZE_REDUCTION_APPLIED

#include EFI_PROTOCOL_DEFINITION (ComponentName)

#define ADD_NAME(x)                      AddName ((x));
                                         
extern EFI_COMPONENT_NAME_PROTOCOL       gIDEBusComponentName;

#else

#define ADD_NAME(x)

#endif

//
// EFI Component Name Functions
//
EFI_STATUS
EFIAPI
IDEBusComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  );

EFI_STATUS
EFIAPI
IDEBusComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  EFI_HANDLE                   ChildHandle        OPTIONAL,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **ControllerName
  );

VOID AddName (
  IN  IDE_BLK_IO_DEV               *IdeBlkIoDevicePtr
  );

#endif


