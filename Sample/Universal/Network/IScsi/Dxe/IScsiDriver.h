/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  IScsiDriver.h

Abstract:

--*/

#ifndef _ISCSI_DRIVER_H_
#define _ISCSI_DRIVER_H_

#include "Tiano.h"
#include EFI_PROTOCOL_CONSUMER (DriverBinding)
#include EFI_PROTOCOL_CONSUMER (ScsiPassThruExt)
#include EFI_PROTOCOL_CONSUMER (DevicePath)
#include EFI_PROTOCOL_CONSUMER (IScsiInitiatorName)
#include EFI_PROTOCOL_CONSUMER (Ip4Config)
#include EFI_PROTOCOL_PRODUCER (ComponentName)
#include EFI_PROTOCOL_PRODUCER (ComponentName2)

#define ISCSI_PRIVATE_GUID \
  { \
    0xfa3cde4c, 0x87c2, 0x427d, 0xae, 0xde, 0x7d, 0xd0, 0x96, 0xc8, 0x8c, 0x58 \
  }

#define ISCSI_INITIATOR_NAME_VAR_NAME L"I_NAME"

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
extern EFI_COMPONENT_NAME2_PROTOCOL       gIScsiComponentName;
#else
extern EFI_COMPONENT_NAME_PROTOCOL        gIScsiComponentName;
#endif

extern EFI_ISCSI_INITIATOR_NAME_PROTOCOL  gIScsiInitiatorName;

extern EFI_GUID                           mIScsiPrivateGuid;

typedef struct _ISCSI_PRIVATE_PROTOCOL {
  UINT32  Reserved;
} ISCSI_PRIVATE_PROTOCOL;

//
// EFI Driver Binding Protocol for iSCSI driver.
//
EFI_STATUS
EFIAPI
IScsiDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  );

EFI_STATUS
EFIAPI
IScsiDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  );

EFI_STATUS
EFIAPI
IScsiDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  );

//
// EFI Component Name Protocol for iSCSI driver.
//
EFI_STATUS
EFIAPI
IScsiComponentNameGetDriverName (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
#else
  IN  EFI_COMPONENT_NAME_PROTOCOL   *This,
#endif
  IN  CHAR8                         *Language,
  OUT CHAR16                        **DriverName
  );

EFI_STATUS
EFIAPI
IScsiComponentNameGetControllerName (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
#else
  IN  EFI_COMPONENT_NAME_PROTOCOL   *This,
#endif
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_HANDLE                    ChildHandle        OPTIONAL,
  IN  CHAR8                         *Language,
  OUT CHAR16                        **ControllerName
  );

//
// EFI iSCSI Initiator Name Protocol for iSCSI driver.
//
EFI_STATUS
EFIAPI
IScsiGetInitiatorName (
  IN     EFI_ISCSI_INITIATOR_NAME_PROTOCOL  *This,
  IN OUT UINTN                              *BufferSize,
  OUT    VOID                               *Buffer
  );

EFI_STATUS
EFIAPI
IScsiSetInitiatorName (
  IN     EFI_ISCSI_INITIATOR_NAME_PROTOCOL  *This,
  IN OUT UINTN                              *BufferSize,
  OUT    VOID                               *Buffer
  );

#endif
