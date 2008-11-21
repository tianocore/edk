/*++

Copyright (c) 2007 - 2008, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  DriverSample.h

Abstract:


Revision History

--*/

#ifndef _DRIVER_SAMPLE_H
#define _DRIVER_SAMPLE_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "UefiIfrLibrary.h"
#include "TianoHii.h"
#include "EfiPrintLib.h"

#include EFI_PROTOCOL_CONSUMER (HiiDatabase)
#include EFI_PROTOCOL_CONSUMER (HiiString)
#include EFI_PROTOCOL_CONSUMER (HiiConfigRouting)
#include EFI_PROTOCOL_CONSUMER (FormBrowser2)

#include EFI_PROTOCOL_PRODUCER (HiiConfigAccess)

#include "NVDataStruc.h"

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include "DriverSampleStrDefs.h"

//
// This is the generated <AltResp> for defaults defined in VFR
//
extern UINT8 VfrMyIfrNVDataDefault0000[];
extern UINT8 VfrMyIfrNVDataDefault0001[];
extern UINT8 VfrMyIfrNVDataBlockName[];

//
// This is the generated IFR binary data for each formset defined in VFR.
// This data array is ready to be used as input of PreparePackageList() to
// create a packagelist (which contains Form packages, String packages, etc).
//
extern UINT8  VfrBin[];
extern UINT8  InventoryBin[];

//
// This is the generated String package data for all .UNI files.
// This data array is ready to be used as input of PreparePackageList() to
// create a packagelist (which contains Form packages, String packages, etc).
//
extern UINT8  DriverSampleStrings[];

#define VAR_OFFSET(Field)    \
  ((UINT16) ((UINTN) &(((DRIVER_SAMPLE_CONFIGURATION *) 0)->Field)))

#define DYNAMIC_ONE_OF_VAR_OFFSET        VAR_OFFSET (DynamicOneof)
#define DYNAMIC_ORDERED_LIST_VAR_OFFSET  VAR_OFFSET (DynamicOrderedList)


#define DRIVER_SAMPLE_PRIVATE_SIGNATURE EFI_SIGNATURE_32 ('D', 'S', 'p', 's')

typedef struct {
  UINTN                            Signature;

  EFI_HANDLE                       DriverHandle[2];
  EFI_HII_HANDLE                   HiiHandle[2];
  DRIVER_SAMPLE_CONFIGURATION      Configuration;
  UINT8                            PasswordState;

  //
  // Consumed protocol
  //
  EFI_HII_DATABASE_PROTOCOL        *HiiDatabase;
  EFI_HII_STRING_PROTOCOL          *HiiString;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;
  EFI_FORM_BROWSER2_PROTOCOL       *FormBrowser2;

  //
  // Produced protocol
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL   ConfigAccess;
} DRIVER_SAMPLE_PRIVATE_DATA;

#define DRIVER_SAMPLE_PRIVATE_FROM_THIS(a)  CR (a, DRIVER_SAMPLE_PRIVATE_DATA, ConfigAccess, DRIVER_SAMPLE_PRIVATE_SIGNATURE)

#endif
