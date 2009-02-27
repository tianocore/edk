/*++
Copyright (c) 2008, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  DriverSample2.c

Abstract:

  This is an example of how a driver retrieve HII data using HII Package List
  Protocol, and how to publish the HII data.

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiHii.h"

#include EFI_PROTOCOL_CONSUMER (HiiDatabase)
#include EFI_PROTOCOL_CONSUMER (HiiPackageList)

EFI_HII_HANDLE  mHiiHandle = NULL;

EFI_DRIVER_ENTRY_POINT (DriverSampleInit)

EFI_STATUS
EFIAPI
DriverSampleInit (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                      Status;
  EFI_HII_DATABASE_PROTOCOL       *HiiDatabase;
  EFI_HII_PACKAGE_LIST_HEADER     *PackageList;

  //
  // Initialize the library and our protocol.
  //
  EfiInitializeDriverLib (ImageHandle, SystemTable);

  //
  // Locate Hii Database protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Retrieve HII package list from ImageHandle
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **) &PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Publish HII package list to HII Database. Here we use ImageHandle as
  // the "Driver Handle" for HiiDatabase->NewPackageList(), since there will
  // be EFI_LOADED_IMAGE_DEVICE_PATH_PROTOCOL installed on the ImageHandle.
  //
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          ImageHandle,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}
