/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    EfiMgmtModeRuntimeLib.h

Abstract:

  Light weight lib to support EFI drivers.

--*/

#ifndef _EFI_RT_SUPPORT_LIB_H_
#define _EFI_RT_SUPPORT_LIB_H_

#ifndef EFI_LOAD_IMAGE_SMM
  #define EFI_LOAD_DRIVER_SMM  FALSE
#else
  #define EFI_LOAD_DRIVER_SMM  TRUE
#endif

#ifndef EFI_NO_LOAD_IMAGE_RT 
  #define EFI_NO_LOAD_DRIVER_RT  FALSE
#else 
  #define EFI_NO_LOAD_DRIVER_RT  TRUE
#endif  

#include "EfiCommonLib.h"
#include "LinkedList.h"
#include "ProcDep.h"

#include EFI_PROTOCOL_DEFINITION (CpuIo)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeBlock)

//
// Driver Lib Globals.
//
extern EFI_BOOT_SERVICES         *gBS;
extern EFI_SYSTEM_TABLE          *gST;
extern UINTN                     gRtErrorLevel;
extern BOOLEAN                   mEfiLoadDriverSmm;
extern BOOLEAN                   mEfiNoLoadDriverRt;
extern EFI_DEVICE_PATH_PROTOCOL  *mFilePath;

//
// Runtime Memory Allocation/De-Allocation tools (Should be used in Boot Phase only)
//

EFI_STATUS
EfiAllocateRuntimeMemoryPool (
  IN UINTN                          Size,
  OUT VOID                          **Buffer
);

EFI_STATUS
EfiFreeRuntimeMemoryPool (
  IN VOID                          *Buffer
);

EFI_STATUS
EfiLocateProtocolHandleBuffers (
  IN EFI_GUID                       *Protocol,
  IN OUT UINTN                      *NumberHandles,
  OUT EFI_HANDLE                    **Buffer
);

EFI_STATUS
EfiHandleProtocol (
  IN EFI_HANDLE                     Handle,
  IN EFI_GUID                       *Protocol,
  OUT VOID                          **Interface
);

EFI_STATUS
EfiInstallProtocolInterface (
  IN OUT EFI_HANDLE                 *Handle,
  IN EFI_GUID                       *Protocol,
  IN EFI_INTERFACE_TYPE             InterfaceType,
  IN VOID                           *Interface
);

EFI_STATUS
EfiReinstallProtocolInterface (
  IN EFI_HANDLE                     SmmProtocolHandle,
  IN EFI_GUID                       *Protocol,
  IN VOID                           *OldInterface,
  IN VOID                           *NewInterface
);

EFI_STATUS
EfiLocateProtocolInterface (
  EFI_GUID  *Protocol,
  VOID      *Registration, OPTIONAL
  VOID      **Interface
);

EFI_STATUS
UninstallProtocolInterface (
  IN EFI_HANDLE                     SmmProtocolHandle,
  IN EFI_GUID                       *Protocol,
  IN VOID                           *Interface  
);

EFI_STATUS
EfiRegisterProtocolCallback (
  IN  EFI_EVENT_NOTIFY              CallbackFunction,
  IN  VOID                          *Context,
  IN  EFI_GUID                      *ProtocolGuid,
  IN  EFI_TPL                       NotifyTpl,
  OUT VOID                          **Registeration,
  OUT EFI_EVENT                     *Event                
);

EFI_STATUS
EfiSignalProtocolEvent (
  EFI_EVENT                         Event    
);

EFI_STATUS
EfiInstallVendorConfigurationTable (
  IN EFI_GUID                       *Guid,
  IN VOID                           *Table
);

EFI_STATUS
EfiGetVendorConfigurationTable (
  IN EFI_GUID                       *Guid,
  OUT VOID                          **Table
);

EFI_STATUS
EfiInitializeUtilsRuntimeDriverLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN EFI_EVENT_NOTIFY     GoVirtualChildEvent
);

BOOLEAN
EfiInManagementInterrupt (
);

//
// This MACRO initializes the RUNTIME invironment and optionally loads Image to SMM or Non-SMM space
// based upon the presence of build flags EFI_LOAD_DRIVER_SMM and EFI_NO_LOAD_DRIVER_RT.
//
#define EFI_INITIALIZE_RUNTIME_DRIVER_LIB(ImageHandle, SystemTable, GoVirtualChildEvent, FilePath) \
  mEfiLoadDriverSmm = EFI_LOAD_DRIVER_SMM; \
  mEfiNoLoadDriverRt = EFI_NO_LOAD_DRIVER_RT; \
  mFilePath = (EFI_DEVICE_PATH_PROTOCOL*) FilePath; \
  EfiInitializeUtilsRuntimeDriverLib ((EFI_HANDLE) ImageHandle, (EFI_SYSTEM_TABLE*) SystemTable, (EFI_EVENT_NOTIFY) GoVirtualChildEvent); \
  if (!EfiInManagementInterrupt()) { \
    if (mEfiNoLoadDriverRt) { \
      return EFI_SUCCESS; \
    } \
  }  

#endif
