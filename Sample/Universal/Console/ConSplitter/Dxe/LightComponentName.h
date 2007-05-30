/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LightComponentName.h

Abstract:

  Private data structures for the Console Splitter driver

--*/

#ifndef SPLITER_COMPONENT_NAME_H_
#define SPLITER_COMPONENT_NAME_H_

#include "Tiano.h"

#ifndef EFI_SIZE_REDUCTION_APPLIED

#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (ComponentName2)

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
extern EFI_COMPONENT_NAME2_PROTOCOL  gConSplitterConInComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL  gConSplitterConOutComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL  gConSplitterStdErrComponentName;
#else
extern EFI_COMPONENT_NAME_PROTOCOL   gConSplitterConInComponentName;
extern EFI_COMPONENT_NAME_PROTOCOL   gConSplitterConOutComponentName;
extern EFI_COMPONENT_NAME_PROTOCOL   gConSplitterStdErrComponentName;
#endif

//
// EFI Component Name Functions
//
EFI_STATUS
EFIAPI
ConSplitterComponentNameGetDriverName (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN  EFI_COMPONENT_NAME2_PROTOCOL *This,
#else
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
#endif
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  )
;

EFI_STATUS
EFIAPI
ConSplitterConInComponentNameGetControllerName (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN  EFI_COMPONENT_NAME2_PROTOCOL                    *This,
#else
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
#endif
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  )
;

EFI_STATUS
EFIAPI
ConSplitterConOutComponentNameGetControllerName (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN  EFI_COMPONENT_NAME2_PROTOCOL                    *This,
#else
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
#endif
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  )
;

EFI_STATUS
EFIAPI
ConSplitterStdErrComponentNameGetControllerName (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN  EFI_COMPONENT_NAME2_PROTOCOL                    *This,
#else
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
#endif
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  )
;

#endif

#endif
