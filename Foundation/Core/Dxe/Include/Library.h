/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DxeCore.h

Abstract:

Revision History

--*/

#ifndef _DXE_LIBRARY_H_
#define _DXE_LIBRARY_H_

typedef struct {
  EFI_TPL     Tpl;
  EFI_TPL     OwnerTpl;
  UINTN       Lock;
} EFI_LOCK;


//
// Macro to initialize the state of a lock when a lock variable is declared
//
#define EFI_INITIALIZE_LOCK_VARIABLE(Tpl) {Tpl,0,0}

VOID
CoreReportProgressCode (
  IN  EFI_STATUS_CODE_VALUE   Value
  );

VOID
CoreReportProgressCodeSpecific (
  IN  EFI_STATUS_CODE_VALUE   Value,
  IN  EFI_HANDLE              Handle
  );

VOID
CoreAcquireLock (
  IN EFI_LOCK *Lock
  );

EFI_STATUS
CoreAcquireLockOrFail (
  IN EFI_LOCK  *Lock
  );

VOID
CoreReleaseLock (
  IN EFI_LOCK *Lock
  );

//
// Device Path functions
//

UINTN
CoreDevicePathSize (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  );

BOOLEAN
CoreIsDevicePathMultiInstance (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  );


EFI_DEVICE_PATH_PROTOCOL *
CoreDuplicateDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL   *DevicePath
  );

EFI_DEVICE_PATH_PROTOCOL *
CoreAppendDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *Src1,
  IN EFI_DEVICE_PATH_PROTOCOL  *Node
  );

VOID *
CoreAllocateBootServicesPool (
  IN  UINTN   AllocationSize
  );

VOID *
CoreAllocateZeroBootServicesPool (
  IN  UINTN   AllocationSize
  );

EFI_STATUS
CoreGetConfigTable (
  IN EFI_GUID *Guid,
  IN OUT VOID **Table
  );

VOID *
CoreAllocateRuntimeCopyPool (
  IN  UINTN   AllocationSize,
  IN  VOID    *Buffer
  );

VOID *
CoreAllocateRuntimePool (
  IN  UINTN   AllocationSize
  );

VOID *
CoreAllocateCopyPool (
  IN  UINTN   AllocationSize,
  IN  VOID    *Buffer
  );

VOID
EfiDebugAssert (
  IN CHAR8    *FileName,
  IN INTN     LineNumber,
  IN CHAR8    *Description
  );

VOID
EfiDebugPrint (
  IN  UINTN ErrorLevel,
  IN  CHAR8 *Format,
  ...
  );

EFI_EVENT
CoreCreateProtocolNotifyEvent (
  IN EFI_GUID             *ProtocolGuid,
  IN EFI_TPL              NotifyTpl,
  IN EFI_EVENT_NOTIFY     NotifyFunction,
  IN VOID                 *NotifyContext,
  OUT VOID                **Registration,
  IN  BOOLEAN             SignalFlag
  );

#endif
