/*++

Copyright (c) 2007 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  IScsiConfig.h

Abstract:


--*/

#ifndef _ISCSI_CONFIG_H_
#define _ISCSI_CONFIG_H_

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiPrintLib.h"

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#include "UefiIfrLibrary.h"
#include EFI_PROTOCOL_DEFINITION (HiiDatabase)
#include EFI_PROTOCOL_DEFINITION (HiiConfigAccess)
#else
#include "IfrLibrary.h"
#include EFI_PROTOCOL_DEFINITION (Hii)
#include EFI_PROTOCOL_DEFINITION (FormBrowser)
#include EFI_PROTOCOL_DEFINITION (FormCallback)
#endif

#include "NetLib.h"
#include "IScsiConfigNVDataStruc.h"

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include "IScsiStrDefs.h"

extern UINT8  IScsiConfigVfrBin[];
extern UINT8  IScsiStrings[];

#define ISCSI_INITATOR_NAME_VAR_NAME        L"I_NAME"

#define ISCSI_CONFIG_VAR_ATTR               (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE)

#define ISCSI_FORM_CALLBACK_INFO_SIGNATURE  EFI_SIGNATURE_32 ('I', 'f', 'c', 'i')

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
#define ISCSI_FORM_CALLBACK_INFO_FROM_FORM_CALLBACK(Callback) \
  CR ( \
  Callback, \
  ISCSI_FORM_CALLBACK_INFO, \
  ConfigAccess, \
  ISCSI_FORM_CALLBACK_INFO_SIGNATURE \
  )
#else
#define ISCSI_FORM_CALLBACK_INFO_FROM_FORM_CALLBACK(Callback) \
  CR ( \
  Callback, \
  ISCSI_FORM_CALLBACK_INFO, \
  FormCallback, \
  ISCSI_FORM_CALLBACK_INFO_SIGNATURE \
  )
#endif

#pragma pack(1)

typedef struct _ISCSI_MAC_INFO {
  EFI_MAC_ADDRESS Mac;
  UINT8           Len;
} ISCSI_MAC_INFO;

typedef struct _ISCSI_DEVICE_LIST {
  UINT8           NumDevice;
  ISCSI_MAC_INFO  MacInfo[1];
} ISCSI_DEVICE_LIST;

#pragma pack()

typedef struct _ISCSI_CONFIG_FORM_ENTRY {
  NET_LIST_ENTRY                Link;
  EFI_HANDLE                    Controller;
  CHAR16                        MacString[95];
  STRING_REF                    PortTitleToken;
  STRING_REF                    PortTitleHelpToken;

  ISCSI_SESSION_CONFIG_NVDATA   SessionConfigData;
  ISCSI_CHAP_AUTH_CONFIG_NVDATA AuthConfigData;
} ISCSI_CONFIG_FORM_ENTRY;

typedef struct _ISCSI_FORM_CALLBACK_INFO {
  UINT32                           Signature;
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  EFI_HANDLE                       DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL   ConfigAccess;
  EFI_HII_DATABASE_PROTOCOL        *HiiDatabase;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *ConfigRouting;
#else
  EFI_HANDLE                       CallbackHandle;
  EFI_FORM_CALLBACK_PROTOCOL       FormCallback;
  EFI_HII_PROTOCOL                 *Hii;
#endif
  UINT16                           *KeyList;
  VOID                             *FormBuffer;
  EFI_HII_HANDLE                   RegisteredHandle;
  ISCSI_CONFIG_FORM_ENTRY          *Current;
} ISCSI_FORM_CALLBACK_INFO;

EFI_STATUS
IScsiConfigUpdateForm (
  IN EFI_HANDLE  DriverBindingHandle,
  IN EFI_HANDLE  Controller,
  IN BOOLEAN     AddForm
  );

EFI_STATUS
IScsiConfigFormInit (
  IN EFI_HANDLE  DriverBindingHandle
  );

EFI_STATUS
IScsiConfigFormUnload (
  IN EFI_HANDLE  DriverBindingHandle
  );

#endif
