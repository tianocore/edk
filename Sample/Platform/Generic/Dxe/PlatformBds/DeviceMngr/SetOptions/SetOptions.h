/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SetOptions.h

Abstract:

  Function prototype for SetOptions driver

--*/

#ifndef _SET_OPTIONS_H
#define _SET_OPTIONS_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "IfrLibrary.h"
#include "EfiPrintLib.h"
#include EFI_PROTOCOL_DEFINITION (Hii)
#include EFI_PROTOCOL_DEFINITION (FormBrowser)
#include EFI_PROTOCOL_DEFINITION (FormCallback)
#include EFI_PROTOCOL_DEFINITION (DevicePathToText)

#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (DriverConfiguration)
#include EFI_PROTOCOL_DEFINITION (DriverDiagnostics)
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (LoadedImage)
#include EFI_PROTOCOL_DEFINITION (DeviceIO)
#include EFI_PROTOCOL_DEFINITION (BlockIO)
#include EFI_PROTOCOL_DEFINITION (DiskIO)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolume)

#define EFI_CALLBACK_INFO_SIGNATURE EFI_SIGNATURE_32 ('C', 'l', 'b', 'k')
#define EFI_CALLBACK_INFO_FROM_THIS(a)  CR (a, EFI_CALLBACK_INFO, DriverCallback, EFI_CALLBACK_INFO_SIGNATURE)
#define MAX_CHOICE_NUM    0x100
#define UPDATE_DATA_SIZE  0x1000
//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include "SetOptionsStrDefs.h"

extern UINT8  VfrBin[];

extern UINT8  SetOptionsStrings[];

typedef struct {
  UINTN                       Signature;
  EFI_HANDLE                  CallbackHandle;
  EFI_FORM_CALLBACK_PROTOCOL  DriverCallback;
  EFI_HII_HANDLE              RegisteredHandle;
  EFI_HII_PROTOCOL            *Hii;
  UINTN                       DummyPad;
} EFI_CALLBACK_INFO;

typedef struct {
  EFI_DRIVER_CONFIGURATION_PROTOCOL         *DriverConfiguration;
  EFI_HANDLE                                DriverImageHandle;
  EFI_HANDLE                                ControllerHandle;
  EFI_HANDLE                                ChildControllerHandle;
  //
  // To avoid created string leak in Hii database, use this token to reuse every token created by the driver
  //
  STRING_REF                                DescriptionToken;
} CFG_PROTOCOL_INVOKER_CHOICE;

EFI_STATUS
EFIAPI
SetOptionsCallback (
  IN EFI_FORM_CALLBACK_PROTOCOL       *This,
  IN UINT16                           KeyValue,
  IN EFI_IFR_DATA_ARRAY               *Data,
  OUT EFI_HII_CALLBACK_PACKET         **Packet
  );

EFI_STATUS
GetDeviceHandlesManagedByDriver (
  IN  EFI_HANDLE  DriverBindingHandle,
  OUT UINTN       *ControllerHandleCount,
  OUT EFI_HANDLE  **ControllerHandleBuffer
  );

EFI_STATUS
GetChildDeviceHandlesManagedByDriver (
  IN  EFI_HANDLE  DriverBindingHandle,
  IN  EFI_HANDLE  ControllerHandle,
  OUT UINTN       *ChildControllerHandleCount,
  OUT EFI_HANDLE  **ChildControllerHandleBuffer
  );
  
EFI_STATUS
ProcessActionRequired (
  IN  EFI_FORM_BROWSER_PROTOCOL                 *FormBrowser,
  IN  EFI_CALLBACK_INFO                         *Private,
  IN  EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  ActionRequired,
  IN  EFI_HANDLE                                DriverImageHandle,
  IN  EFI_HANDLE                                ControllerHandle,
  IN  EFI_HANDLE                                ChildControllerHandle
  );
  
CHAR16 *
GetImageName (
  IN  EFI_LOADED_IMAGE_PROTOCOL *Image
  );
  
CHAR16  *
DevicePathToStr (
  EFI_DEVICE_PATH_PROTOCOL     *DevPath
  );
  
VOID
AlignmentItem (
  IN        CHAR16   *ControllerHandleName,
  IN        CHAR16   *PrefixString,
  IN OUT    CHAR16   **NewString
  );
  
EFI_STRING
GetString (
  IN  EFI_CALLBACK_INFO   *Private,
  IN  STRING_REF          Token
  );
  
#endif
