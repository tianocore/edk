/*++

Copyright (c) 2004 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  BootManager.h

Abstract:

  The platform boot manager reference implement

Revision History

--*/

#ifndef _EFI_BOOT_MANAGER_H
#define _EFI_BOOT_MANAGER_H

#include "Tiano.h"
#include "Bds.h"
#include "BdsLib.h"
#include "BdsPlatform.h"
#include "UefiIfrLibrary.h"
#include "FrontPage.h"

//
// These are defined as the same with vfr file
//
#define BOOT_MANAGER_FORMSET_GUID \
  { \
    0x847bc3fe, 0xb974, 0x446d, 0x94, 0x49, 0x5a, 0xd5, 0x41, 0x2e, 0x99, 0x3b \
  }

#define BOOT_MANAGER_FORM_ID     0x1000

#define LABEL_BOOT_OPTION        0x00

//
// These are the VFR compiler generated data representing our VFR data.
//
extern UINT8 BootManagerVfrBin[];

#define BOOT_MANAGER_CALLBACK_DATA_SIGNATURE  EFI_SIGNATURE_32 ('B', 'M', 'C', 'B')

typedef struct {
  UINTN                           Signature;

  //
  // HII relative handles
  //
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HANDLE                      DriverHandle;

  //
  // Produced protocols
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL   ConfigAccess;
} BOOT_MANAGER_CALLBACK_DATA;

EFI_STATUS
EFIAPI
BootManagerCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
;

EFI_STATUS
InitializeBootManager (
  VOID
  )
;

VOID
CallBootManager (
  VOID
  )
;

#endif
