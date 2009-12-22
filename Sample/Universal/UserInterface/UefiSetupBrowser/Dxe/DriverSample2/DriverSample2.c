/*++
Copyright (c) 2008 - 2009, Intel Corporation
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

//
// HII specific Vendor Device Path definition
//
#pragma pack(1)
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;
#pragma pack()


EFI_HII_HANDLE  mHiiHandle = NULL;
EFI_HANDLE      mDriverHandle = NULL;

HII_VENDOR_DEVICE_PATH  mHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    //
    // {63BA34A0-0F7A-4c41-A58C-6D9FDAEDA6FC}
    //
    { 0x63ba34a0, 0xf7a, 0x4c41, { 0xa5, 0x8c, 0x6d, 0x9f, 0xda, 0xed, 0xa6, 0xfc } }
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    { 
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};


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
  // Create a child handle by installing Vendor Device Path
  //
  Status = gBS->InstallProtocolInterface (
                  &mDriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mHiiVendorDevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Publish HII package list to HII Database.
  //
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          mDriverHandle,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    //
    // Destroy child handle
    //
    gBS->UninstallProtocolInterface (
           mDriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mHiiVendorDevicePath
           );
    return Status;
  }

  return EFI_SUCCESS;
}
