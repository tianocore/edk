/*++
Copyright (c) 2007 - 2008, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:
  PlatOverMngr.c

Abstract:

  A UI driver to offer a UI interface in device manager to let user configue
  platform override protocol to override the default algorithm for matching 
  drivers to controllers.

  The main flow:
  1. The UI driver dynamicly locate all controller device path.
  2. The UI driver dynamicly locate all drivers which support binding protocol.
  3. The UI driver export and dynamicly update two  menu to let user select the
     mapping between drivers to controllers.
  4. The UI driver save all the mapping info in NV variables which will be consumed
     by platform override protocol driver to publish the platform override protocol.

--*/

#include "PlatOverMngr.h"
#include "PlatDriOverLib.h"

EFI_DRIVER_ENTRY_POINT (PlatOverMngrInit)

STATIC  EFI_GUID      mPlatformOverridesManagerGuid = PLAT_OVER_MNGR_GUID;

STATIC  EFI_LIST_ENTRY  mMappingDataBase = INITIALIZE_LIST_HEAD_VARIABLE (mMappingDataBase);

STATIC  EFI_HANDLE    *mDevicePathHandleBuffer;
STATIC  EFI_HANDLE    *mDriverImageHandleBuffer;
STATIC  EFI_HANDLE    mSelectedCtrDPHandle;

STATIC CFG_PROTOCOL_INVOKER_CHOICE mChoice[MAX_CHOICE_NUM];

STATIC UINTN         mSelectedCtrIndex;
STATIC EFI_STRING_ID mControllerToken[MAX_CHOICE_NUM];

STATIC UINTN                        mDriverImageHandleCount;
STATIC EFI_STRING_ID                mDriverImageToken[MAX_CHOICE_NUM];
STATIC EFI_STRING_ID                mDriverImageFilePathToken[MAX_CHOICE_NUM];
STATIC EFI_LOADED_IMAGE_PROTOCOL    *mDriverImageProtocol[MAX_CHOICE_NUM];
STATIC EFI_DEVICE_PATH_PROTOCOL     *mControllerDevicePathProtocol[MAX_CHOICE_NUM];
STATIC UINTN                        mSelectedDriverImageNum;
STATIC UINTN                        mLastSavedDriverImageNum;
STATIC CHAR8                        mLanguage[RFC_3066_ENTRY_SIZE];
STATIC UINT16                       mCurrentPage;

EFI_STATUS
EFIAPI
PlatOverMngrInit (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
/*++

  Routine Description:
    The driver Entry Point.
    The funciton will export a disk device class formset and its callback function to hii database

  Arguments:
    ImageHandle - EFI_HANDLE
    SystemTable - EFI_SYSTEM_TABLE

  Returns:
    EFI_STATUS

--*/
{
  EFI_STATUS                  Status;
  EFI_HII_DATABASE_PROTOCOL   *HiiDatabase;
  EFI_HII_PACKAGE_LIST_HEADER *PackageList;
  EFI_CALLBACK_INFO           *CallbackInfo;
  EFI_HANDLE                  DriverHandle;

  //
  // Initialize the library and our protocol.
  //
  DxeInitializeDriverLib (ImageHandle, SystemTable);

  //
  // There should only be one HII protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  &HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    return Status ;
  }

  CallbackInfo = EfiLibAllocateZeroPool (sizeof (EFI_CALLBACK_INFO));
  if (CallbackInfo == NULL) {
    return EFI_BAD_BUFFER_SIZE;
  }

  CallbackInfo->Signature = EFI_CALLBACK_INFO_SIGNATURE;
  CallbackInfo->ConfigAccess.ExtractConfig = PlatOverMngrExtractConfig;
  CallbackInfo->ConfigAccess.RouteConfig   = PlatOverMngrRouteConfig;
  CallbackInfo->ConfigAccess.Callback      = PlatOverMngrCallback;

  //
  // Create driver handle used by HII database
  //
  Status = CreateHiiDriverHandle (&DriverHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CallbackInfo->DriverHandle = DriverHandle;

  //
  // Install Config Access protocol to driver handle
  //
  Status = gBS->InstallProtocolInterface (
                  &DriverHandle,
                  &gEfiHiiConfigAccessProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &CallbackInfo->ConfigAccess
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Publish our HII data
  //
  PackageList = PreparePackageList (
                  2,
                  &mPlatformOverridesManagerGuid, 
                  VfrBin, 
                  PlatOverMngrStrings
                  );
  ASSERT (PackageList != NULL);

  Status = HiiDatabase->NewPackageList (
                           HiiDatabase,
                           PackageList,
                           DriverHandle,
                           &CallbackInfo->RegisteredHandle
                           );
  gBS->FreePool (PackageList);

  //
  // Locate ConfigRouting protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  &CallbackInfo->HiiConfigRouting
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Clear all the globle variable
  //
  mDriverImageHandleCount = 0;
  mCurrentPage = 0;
  EfiZeroMem (mDriverImageToken, MAX_CHOICE_NUM * sizeof (EFI_STRING_ID));
  EfiZeroMem (mDriverImageFilePathToken, MAX_CHOICE_NUM * sizeof (EFI_STRING_ID));
  EfiZeroMem (mControllerToken, MAX_CHOICE_NUM * sizeof (EFI_STRING_ID));
  EfiZeroMem (mDriverImageProtocol, MAX_CHOICE_NUM * sizeof (EFI_LOADED_IMAGE_PROTOCOL *));

  return EFI_SUCCESS;
}

UINTN
strncmpa (
  IN CHAR8    *s1,
  IN CHAR8    *s2,
  IN UINTN    len
  )
/*++

Routine Description:

  compare ascii string

Arguments:

  s1  -  The first string
  s2  -  The second string
  len -  The compare string length

Returns:

--*/
{
  ASSERT (s1 != NULL);
  ASSERT (s2 != NULL);

  while (*s1 && len) {
    if (*s1 != *s2) {
      break;
    }

    s1 += 1;
    s2 += 1;
    len -= 1;
  }

  return len ? *s1 -*s2 : 0;
}

UINTN
strcmpa (
  IN CHAR8    *s1,
  IN CHAR8    *s2
  )
/*++

Routine Description:

  Compare ascii string

Arguments:

  s1  -  The first string
  s2  -  The second string

Returns:

--*/
{
  while (*s1) {
    if (*s1 != *s2) {
      break;
    }

    s1 += 1;
    s2 += 1;
  }

  return *s1 -*s2;
}

CHAR8*
strstra (
  IN  CHAR8  *String,
  IN  CHAR8  *StrCharSet
  )
/*++

Routine Description:

  Find a Ascii substring.

Arguments:

  String      - Null-terminated Ascii string to search.
  StrCharSet  - Null-terminated Ascii string to search for.

Returns:
  The address of the first occurrence of the matching Ascii substring if successful, or NULL otherwise.
--*/
{
  CHAR8 *Src;
  CHAR8 *Sub;

  Src = String;
  Sub = StrCharSet;

  while ((*String != '\0') && (*StrCharSet != '\0')) {
    if (*String++ != *StrCharSet++) {
      String = ++Src;
      StrCharSet = Sub;
    }
  }
  if (*StrCharSet == '\0') {
    return Src;
  } else {
    return NULL;
  }
}

CHAR8 *
ConvertComponentName2SupportLanguage (
  IN EFI_COMPONENT_NAME2_PROTOCOL    *ComponentName,
  IN CHAR8                           *Language
  )
/*++

  Routine Description:

    Do some convertion for the ComponentName2 supported language. It do
    the convertion just for english language code currently.

  Arguments:

    ComponentName         - Pointer to the ComponentName2 protocl pointer.
    Language              - The language string.

  Returns:

    Return the duplication of Language if it is not english otherwise return
    the supported english language code.

--*/
{
  CHAR8                              *SupportedLanguages;
  CHAR8                              *LangCode;
  UINTN                              Index;

  LangCode           = NULL;
  SupportedLanguages = NULL;

  //
  // treat all the english language code (en-xx or eng) equally
  //
  if ((strncmpa(Language, "en-", 3) == 0) || (strcmpa(Language, "eng") == 0)) {
    SupportedLanguages = strstra(ComponentName->SupportedLanguages, "en-");
    if (SupportedLanguages == NULL) {
      SupportedLanguages = strstra(ComponentName->SupportedLanguages, "eng");
    }
  }

  //
  // duplicate the Language if it is not english
  //
  if (SupportedLanguages == NULL) {
    SupportedLanguages = Language;
  }

  //
  // duplicate the returned language code.
  //
  if (strstra(SupportedLanguages, "-") != NULL) {
    LangCode = EfiLibAllocateZeroPool(32);
    for(Index = 0; (Index < 31) && (SupportedLanguages[Index] != '\0') && (SupportedLanguages[Index] != ';'); Index++) {
      LangCode[Index] = SupportedLanguages[Index];
    }
    LangCode[Index] = '\0';
  } else {
    LangCode = EfiLibAllocateZeroPool(4);
    for(Index = 0; (Index < 3) && (SupportedLanguages[Index] != '\0'); Index++) {
      LangCode[Index] = SupportedLanguages[Index];
    }
    LangCode[Index] = '\0';
  }
  return LangCode;
}

CHAR16 *
GetComponentName (
  IN EFI_HANDLE                      DriverBindingHandle
  )
/*++

  Routine Description:

    Get the ComponentName or ComponentName2 protocol according to the driver binding handle

  Arguments:

    DriverBindingHandle   - The Handle of DriverBinding

  Returns:
    Pointer into the image name if the image name is found,
    Otherwise a pointer to NULL.


--*/
{
  EFI_STATUS                   Status;
  EFI_COMPONENT_NAME_PROTOCOL  *ComponentName;
  EFI_COMPONENT_NAME2_PROTOCOL *ComponentName2;
  CHAR8                        *SupportedLanguage;
  CHAR16                       *DriverName;

  ComponentName  = NULL;
  ComponentName2 = NULL;
  Status = gBS->OpenProtocol (
                 DriverBindingHandle,
                 &gEfiComponentName2ProtocolGuid,
                 (VOID **) &ComponentName2,
                 NULL,
                 NULL,
                 EFI_OPEN_PROTOCOL_GET_PROTOCOL
                 );
  if (EFI_ERROR(Status)) {
    Status = gBS->OpenProtocol (
                   DriverBindingHandle,
                   &gEfiComponentNameProtocolGuid,
                   (VOID **) &ComponentName,
                   NULL,
                   NULL,
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );
  }

  Status     = EFI_SUCCESS;
  DriverName = NULL;
  if (ComponentName != NULL) {
    if (ComponentName->GetDriverName != NULL) {
      Status = ComponentName->GetDriverName (
                                ComponentName,
                                mLanguage,
                                &DriverName
                                );
    }
  } else if (ComponentName2 != NULL) {
    if (ComponentName2->GetDriverName != NULL) {
      SupportedLanguage = ConvertComponentName2SupportLanguage (ComponentName2, mLanguage);
      Status = ComponentName2->GetDriverName (
                                 ComponentName2,
                                 SupportedLanguage,
                                 &DriverName
                                 );
        gBS->FreePool (SupportedLanguage);
    }
  }
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return DriverName;
}

CHAR16 *
GetImageName (
  EFI_LOADED_IMAGE_PROTOCOL *Image
  )
/*++

Routine Description:
  Get the image name

Arguments:
  Image - Image to search

Returns:
  Pointer into the image name if the image name is found,
  Otherwise a pointer to NULL.

--*/
{
  EFI_STATUS                        Status;
  EFI_DEVICE_PATH_PROTOCOL          *DevPath;
  EFI_DEVICE_PATH_PROTOCOL          *DevPathNode;
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *FvFilePath;
  VOID                              *Buffer;
  UINTN                             BufferSize;
  UINT32                            AuthenticationStatus;
  EFI_GUID                          *NameGuid;
  EFI_FIRMWARE_VOLUME_PROTOCOL      *FV;
  EFI_FIRMWARE_VOLUME2_PROTOCOL     *FV2;

  FV          = NULL;
  FV2         = NULL;
  Buffer      = NULL;
  BufferSize  = 0;

  if (Image->FilePath == NULL) {
    return NULL;
  }

  DevPath     = BdsLibUnpackDevicePath (Image->FilePath);

  if (DevPath == NULL) {
    return NULL;
  }

  DevPathNode = DevPath;

  while (!IsDevicePathEnd (DevPathNode)) {
    //
    // Find the Fv File path
    //
    NameGuid = EfiGetNameGuidFromFwVolDevicePathNode ((MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *)DevPathNode);
    if (NameGuid != NULL) {
      FvFilePath = (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *) DevPathNode;
      Status = gBS->HandleProtocol (
                    Image->DeviceHandle,
                    &gEfiFirmwareVolumeProtocolGuid,
                    &FV
                    );
      if (!EFI_ERROR (Status)) {
        Status = FV->ReadSection (
                      FV,
                      &FvFilePath->NameGuid,
                      EFI_SECTION_USER_INTERFACE,
                      0,
                      &Buffer,
                      &BufferSize,
                      &AuthenticationStatus
                      );
        if (!EFI_ERROR (Status)) {
          break;
        }

        Buffer = NULL;
      } else {
        Status = gBS->HandleProtocol (
                      Image->DeviceHandle,
                      &gEfiFirmwareVolume2ProtocolGuid,
                      &FV2
                      );
        if (!EFI_ERROR (Status)) {
          Status = FV2->ReadSection (
                          FV2,
                          &FvFilePath->NameGuid,
                          EFI_SECTION_USER_INTERFACE,
                          0,
                          &Buffer,
                          &BufferSize,
                          &AuthenticationStatus
                          );
          if (!EFI_ERROR (Status)) {
            break;
          }

          Buffer = NULL;
        }
      }
    }
    //
    // Next device path node
    //
    DevPathNode = NextDevicePathNode (DevPathNode);
  }

    gBS->FreePool (DevPath);
  return Buffer;
}


EFI_STATUS
UpdateDeviceSelectPage (
  IN EFI_CALLBACK_INFO                *Private,
  IN UINT16                           KeyValue,
  IN PLAT_OVER_MNGR_DATA              *FakeNvData
  )
/*++

Routine Description:
  Prepare the first page to let user select the device controller which need to add mapping drivers

Arguments:

  KeyValue -  No use here.
  Packet-       No use here.

  Returns -    Always successful

--*/
{
  EFI_HII_UPDATE_DATA                       UpdateData;
  EFI_STATUS                                Status;
  UINTN                                     LangSize;
  UINTN                                     Index;
  UINTN                                     DevicePathHandleCount;
  CHAR16                                    *NewString;
  EFI_STRING_ID                             NewStringToken;
  CHAR16                                    *ControllerName;
  EFI_DEVICE_PATH_PROTOCOL                  *ControllerDevicePath;
  EFI_PCI_IO_PROTOCOL                       *PciIo;
  EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL *BusSpecificDriverOverride;

  mCurrentPage = FORM_ID_DEVICE;
  //
  // Following code will be run if user select 'Refresh' in first page
  // During first page, user will see all currnet controller device path in system,
  // select any device path will go to second page to select its overrides drivers
  //

  LangSize = RFC_3066_ENTRY_SIZE;
  Status = gRT->GetVariable (
              L"PlatformLang",
              &gEfiGlobalVariableGuid,
              NULL,
              &LangSize,
              mLanguage
              );
  ASSERT_EFI_ERROR (Status);

  //
  // Initial the mapping database in memory
  //
  LibFreeMappingDatabase (&mMappingDataBase);
  Status = LibInitOverridesMapping (&mMappingDataBase);

  //
  // Clear all the content in the first page
  //
  UpdateData.BufferSize = UPDATE_DATA_SIZE;
  UpdateData.Offset = 0;
  UpdateData.Data = EfiLibAllocateZeroPool (UPDATE_DATA_SIZE);
  ASSERT (UpdateData.Data != NULL);
  //
  // Clear first page form
  //
  IfrLibUpdateForm (
    Private->RegisteredHandle,
    &mPlatformOverridesManagerGuid,
    FORM_ID_DEVICE,
    FORM_ID_DEVICE,
    FALSE,
    &UpdateData
    );
    
  //
  // When user enter the page at first time, the 'first refresh' string is given to notify user to refresh all the drivers,
  // then the 'first refresh' string will be replaced by the 'refresh' string, and the two strings content are  same after the replacement
  //
  NewStringToken = STRING_TOKEN (STR_FIRST_REFRESH);
  GetStringFromHandle (Private->RegisteredHandle, STRING_TOKEN (STR_REFRESH), &NewString);
  ASSERT (NewString != NULL);
  Status = IfrLibSetString (Private->RegisteredHandle, NewStringToken, NewString);
  ASSERT_EFI_ERROR (Status);
  gBS->FreePool (NewString);

  NewStringToken = STRING_TOKEN (STR_FIRST_REFRESH_HELP);
  GetStringFromHandle (Private->RegisteredHandle, STRING_TOKEN (STR_REFRESH_HELP), &NewString);
  ASSERT (NewString != NULL);
  Status = IfrLibSetString (Private->RegisteredHandle, NewStringToken, NewString);
  ASSERT_EFI_ERROR (Status);
  gBS->FreePool (NewString);
  //
  // created needed controller device item in first page
  //
  DevicePathHandleCount  = 0;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiDevicePathProtocolGuid,
                  NULL,
                  &DevicePathHandleCount,
                  &mDevicePathHandleBuffer
                  );
  if (EFI_ERROR (Status) || (DevicePathHandleCount == 0)) {
    return EFI_SUCCESS;
  }

  for (Index = 0; Index < DevicePathHandleCount; Index++) {
    if (FakeNvData->PciDeviceFilter == 0x01) {
      //
      // Only care PCI device which contain efi driver in its option rom.
      //

      //
      // Check whether it is a pci device
      //
      ControllerDevicePath = NULL;
      Status = gBS->OpenProtocol (
                      mDevicePathHandleBuffer[Index],
                      &gEfiPciIoProtocolGuid,
                      (VOID **) &PciIo,
                      NULL,
                      NULL,
                      EFI_OPEN_PROTOCOL_GET_PROTOCOL
                      );
      if (EFI_ERROR (Status)) {
        continue;
      }
      //
      // Check whether it contain efi driver in its option rom
      //
      Status = gBS->HandleProtocol(
                       mDevicePathHandleBuffer[Index],
                       &gEfiBusSpecificDriverOverrideProtocolGuid,
                       &BusSpecificDriverOverride
                       );
      if (EFI_ERROR (Status) || BusSpecificDriverOverride == NULL) {
        continue;
      }
    }

    ControllerDevicePath = NULL;
    Status = gBS->OpenProtocol (
                    mDevicePathHandleBuffer[Index],
                    &gEfiDevicePathProtocolGuid,
                    (VOID **) &ControllerDevicePath,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    ASSERT_EFI_ERROR (Status);
    //
    // Save the device path protocol interface
    //
    mControllerDevicePathProtocol[Index] = ControllerDevicePath;

    //
    // Get the driver name
    //
    ControllerName = DevicePathToStr (ControllerDevicePath);

    //
    // Export the driver name string and create item in set options page
    //
    NewString = EfiLibAllocateZeroPool (EfiStrSize (ControllerName) + EfiStrSize (L"--"));
    if (EFI_ERROR (LibCheckMapping (ControllerDevicePath,NULL, &mMappingDataBase, NULL, NULL))) {
      EfiStrCat (NewString, L"--");
    } else {
      EfiStrCat (NewString, L"**");
    }
    EfiStrCat (NewString, ControllerName);

    NewStringToken = mControllerToken[Index];
    if (NewStringToken == 0) {
      Status = IfrLibNewString (Private->RegisteredHandle, &NewStringToken, NewString);
    } else {
      Status = IfrLibSetString (Private->RegisteredHandle, NewStringToken, NewString);
    }
    ASSERT_EFI_ERROR (Status);
    gBS->FreePool (NewString);
    //
    // Save the device path string toke for next access use
    //
    mControllerToken[Index] = NewStringToken;

    CreateGotoOpCode (
      FORM_ID_DRIVER,
      NewStringToken,
      STRING_TOKEN (STR_GOTO_HELP_DRIVER),
      EFI_IFR_FLAG_CALLBACK,
      (UINT16) (Index + KEY_VALUE_DEVICE_OFFSET),
      &UpdateData
      );
  }

  //
  // Update first page form
  //
  IfrLibUpdateForm (
    Private->RegisteredHandle,
    &mPlatformOverridesManagerGuid,
    FORM_ID_DEVICE,
    FORM_ID_DEVICE,
    FALSE,
    &UpdateData
    );

  gBS->FreePool (UpdateData.Data);
  return EFI_SUCCESS;
}

EFI_STATUS
UpdateBindingDriverSelectPage (
  IN EFI_CALLBACK_INFO                *Private,
  IN UINT16                           KeyValue,
  IN PLAT_OVER_MNGR_DATA              *FakeNvData
  )
/*++

Routine Description:
  Prepare to let user select the drivers which need mapping with the device controller selected in first page

Arguments:

  KeyValue - The callback key value of device controller item in first page
  Data -         EFI_IFR_DATA_ARRAY data.
  Packet-       No use here.

  Returns -    Always successful

--*/
{
  EFI_HII_UPDATE_DATA                       UpdateData;
  EFI_STATUS                                Status;
  UINTN                                     Index;

  CHAR16                                    *NewString;
  EFI_STRING_ID                             NewStringToken;
  EFI_STRING_ID                             NewStringHelpToken;
  UINTN                                     DriverImageHandleCount;

  EFI_DRIVER_BINDING_PROTOCOL               *DriverBindingInterface;
  EFI_LOADED_IMAGE_PROTOCOL                 *LoadedImage;
  CHAR16                                    *DriverName;
  BOOLEAN                                   FreeDriverName;

  EFI_DEVICE_PATH_PROTOCOL                  *LoadedImageHandleDevicePath;
  EFI_DEVICE_PATH_PROTOCOL                  *TatalFilePath;
  EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL *BusSpecificDriverOverride;
  EFI_HANDLE                                DriverBindingHandle;
  //
  // If user select a controller item in the first page  the following code will be run.
  // During second page, user will see all currnet driver bind protocol driver, the driver name and its device path will be shown
  //
  //First acquire the list of Loaded Image Protocols, and then when  want the name of the driver, look up all the Driver Binding Protocols
  // and find the first one whose ImageHandle field matches the image handle of the Loaded Image Protocol.
  // then use the Component Name Protocol on the same handle as the first matching Driver Binding Protocol to look up the name of the driver.
  //


  mCurrentPage = FORM_ID_DRIVER;
  //
  // Switch the item callback key value to its NO. in mDevicePathHandleBuffer
  //
  mSelectedCtrIndex = KeyValue - 0x100;
  ASSERT (0 <= mSelectedCtrIndex < MAX_CHOICE_NUM);
  mLastSavedDriverImageNum = 0;
  //
  // Clear all the content in dynamic page
  //
  UpdateData.BufferSize = UPDATE_DATA_SIZE;
  UpdateData.Offset = 0;
  UpdateData.Data = EfiLibAllocateZeroPool (UPDATE_DATA_SIZE);
  ASSERT (UpdateData.Data != NULL);
  //
  // Clear second page form
  //
  IfrLibUpdateForm (
    Private->RegisteredHandle,
    &mPlatformOverridesManagerGuid,
    FORM_ID_DRIVER,
    FORM_ID_DRIVER,
    FALSE,
    &UpdateData
    );
    
  //
  // Show all driver which support loaded image protocol in second page
  //
  DriverImageHandleCount  = 0;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiLoadedImageProtocolGuid,
                  NULL,
                  &DriverImageHandleCount,
                  &mDriverImageHandleBuffer
                  );
  if (EFI_ERROR (Status) || (DriverImageHandleCount == 0)) {
    return EFI_NOT_FOUND;
  }

  mDriverImageHandleCount = DriverImageHandleCount;
  for (Index = 0; Index < DriverImageHandleCount; Index++) {
    //
    // Step1: Get the driver image total file path for help string and the driver name.
    //

    //
    // Find driver's Loaded Image protocol
    //
    LoadedImage =NULL;
    Status = gBS->OpenProtocol (
                    mDriverImageHandleBuffer[Index],
                    &gEfiLoadedImageProtocolGuid,
                    (VOID **) &LoadedImage,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      FakeNvData->DriSelection[Index] = 0x00;
      continue;
    }
    mDriverImageProtocol[Index] = LoadedImage;
    //
    // Find its related driver binding protocol
    //
    DriverBindingInterface = NULL;
    DriverBindingHandle = NULL;
    DriverBindingInterface = LibGetBindingProtocolFromImageHandle (
                                mDriverImageHandleBuffer[Index],
                                &DriverBindingHandle
                                );
    if (DriverBindingInterface == NULL) {
      FakeNvData->DriSelection[Index] = 0x00;
      continue;
    }

    //
    // Get the driver image total file path for help string
    //
    LoadedImageHandleDevicePath = NULL;
    Status = gBS->HandleProtocol (
                        LoadedImage->DeviceHandle,
                        &gEfiDevicePathProtocolGuid,
                        &LoadedImageHandleDevicePath
                        );
    if (EFI_ERROR (Status)) {
      FakeNvData->DriSelection[Index] = 0x00;
      continue;
    }

    if (FakeNvData->PciDeviceFilter == 0x01) {
      //
      // only care the driver which is in a Pci device option rom,
      // and the driver's LoadedImage->DeviceHandle must point to a pci device which has efi option rom
      //
      if (!EFI_ERROR (Status)) {
        Status = gBS->HandleProtocol(
                         LoadedImage->DeviceHandle,
                         &gEfiBusSpecificDriverOverrideProtocolGuid,
                         &BusSpecificDriverOverride
                         );
        if (EFI_ERROR (Status) || BusSpecificDriverOverride == NULL) {
          FakeNvData->DriSelection[Index] = 0x00;
          continue;
        }
      } else {
        FakeNvData->DriSelection[Index] = 0x00;        
        continue;
      }
    }

    TatalFilePath = NULL;
    TatalFilePath = EfiAppendDevicePath (LoadedImageHandleDevicePath, LoadedImage->FilePath);

    //
    // For driver name, try to get its component name, if fail, get its image name,
    // if also fail, give a default name.
    //
    FreeDriverName = FALSE;
    DriverName = GetComponentName (DriverBindingHandle);
    if (DriverName == NULL) {
      //
      // get its image name
      //
      DriverName = GetImageName (LoadedImage);
    }
    if (DriverName == NULL) {
      //
      // give a default name
      //
      GetStringFromHandle (Private->RegisteredHandle, STRING_TOKEN (STR_DRIVER_DEFAULT_NAME), &DriverName);
      ASSERT (DriverName != NULL);
      FreeDriverName = TRUE;  // the DriverName string need to free pool
    }


    //
    // Step2 Export the driver name string and create check box item in second page
    //

    //
    // First create the driver image name
    //
    NewString = EfiLibAllocateZeroPool (EfiStrSize (DriverName));
    if (EFI_ERROR (LibCheckMapping (mControllerDevicePathProtocol[mSelectedCtrIndex], TatalFilePath, &mMappingDataBase, NULL, NULL))) {
      FakeNvData->DriSelection[Index] = 0x00;
    } else {
      FakeNvData->DriSelection[Index] = 0x01;
      mLastSavedDriverImageNum++;
    }
    EfiStrCat (NewString, DriverName);
    NewStringToken = mDriverImageToken[Index];
    if (NewStringToken == 0) {
      Status = IfrLibNewString (Private->RegisteredHandle, &NewStringToken, NewString);
    } else {
      Status = IfrLibSetString (Private->RegisteredHandle, NewStringToken, NewString);
    }
    mDriverImageToken[Index] = NewStringToken;
    ASSERT_EFI_ERROR (Status);
    gBS->FreePool (NewString);
    if (FreeDriverName) {
      gBS->FreePool (DriverName);
    }

    //
    // Second create the driver image device path as item help string
    //
    if (TatalFilePath == NULL) {
      DriverName = EfiLibAllocateZeroPool (EfiStrSize (L"This driver device path and file path are both NULL! Maybe it is a Option Rom driver. Can not save it!"));
      EfiStrCat (DriverName, L"This driver device path and file path are both NULL! Maybe it is a Option Rom driver. Can not save it!");
    } else {
      DriverName = DevicePathToStr (TatalFilePath);
    }
    NewString = EfiLibAllocateZeroPool (EfiStrSize (DriverName));
    EfiStrCat (NewString, DriverName);
    NewStringHelpToken = mDriverImageFilePathToken[Index];
    if (NewStringHelpToken == 0) {
      Status = IfrLibNewString (Private->RegisteredHandle, &NewStringHelpToken, NewString);
    } else {
      Status = IfrLibSetString (Private->RegisteredHandle, NewStringHelpToken, NewString);
    }
    mDriverImageFilePathToken[Index] = NewStringHelpToken;
    ASSERT_EFI_ERROR (Status);
    gBS->FreePool (NewString);
    gBS->FreePool (DriverName);

    CreateCheckBoxOpCode (
      (UINT16) (DRIVER_SELECTION_QUESTION_ID + Index),
      VARSTORE_ID_PLAT_OVER_MNGR,
      (UINT16) (DRIVER_SELECTION_VAR_OFFSET + Index),
      NewStringToken,
      NewStringHelpToken,
      0,
      0,
      &UpdateData
      );
  }

  //
  // Update second page form
  //
  IfrLibUpdateForm (
    Private->RegisteredHandle,
    &mPlatformOverridesManagerGuid,
    FORM_ID_DRIVER,
    FORM_ID_DRIVER,
    FALSE,
    &UpdateData
    );

  gBS->FreePool (UpdateData.Data);
  return EFI_SUCCESS;
}

EFI_STATUS
UpdatePrioritySelectPage (
  IN EFI_CALLBACK_INFO                *Private,
  IN UINT16                           KeyValue,
  IN PLAT_OVER_MNGR_DATA              *FakeNvData
  )
/*++

Routine Description:
  Prepare to let user select the priority order of the drivers which are selected in second page

Arguments:

  KeyValue - No use here.
  Data -         EFI_IFR_DATA_ARRAY data.
  Packet-       No use here.

  Returns -    Always successful

--*/
{
  EFI_HII_UPDATE_DATA                       UpdateData;
  EFI_STATUS                                Status;
  UINTN                                     Index;


  EFI_DEVICE_PATH_PROTOCOL                  *LoadedImageHandleDevicePath;
  EFI_DEVICE_PATH_PROTOCOL                  *TatalFilePath;

  IFR_OPTION                                *IfrOptionList;
  UINTN                                     SelectedDriverImageNum;
  UINT32                                    DriverImageNO;
  UINTN                                     MinNO;
  UINTN                                     Index1;
  UINTN                                     TempNO[100];

  //
  //  Following code will be run if user select 'order ... priority' item in second page
  // Prepare third page.  In third page, user will order the  drivers priority which are selected in second page
  //
  mCurrentPage = FORM_ID_ORDER;

  UpdateData.BufferSize = UPDATE_DATA_SIZE;
  UpdateData.Offset = 0;
  UpdateData.Data = EfiLibAllocateZeroPool (UPDATE_DATA_SIZE);
  ASSERT (UpdateData.Data != NULL);
  //
  // Clear third page form
  //
  IfrLibUpdateForm (
    Private->RegisteredHandle,
    &mPlatformOverridesManagerGuid,
    FORM_ID_ORDER,
    FORM_ID_ORDER,
    FALSE,
    &UpdateData
    );
    
  //
  // Check how many drivers have been selected
  //
  SelectedDriverImageNum = 0;
  for (Index = 0; Index < mDriverImageHandleCount; Index++) {
    if (FakeNvData->DriSelection[Index] != 0) {
      SelectedDriverImageNum ++;
    }
  }

  mSelectedDriverImageNum = SelectedDriverImageNum;
  if (SelectedDriverImageNum == 0) {
    return EFI_SUCCESS;
  }

  IfrOptionList = EfiLibAllocateZeroPool (0x200);
  ASSERT_EFI_ERROR (IfrOptionList != NULL);
  //
  // Create order list for those selected drivers
  //
  SelectedDriverImageNum = 0;
  for (Index = 0; Index < mDriverImageHandleCount; Index++) {
    if (FakeNvData->DriSelection[Index] != 0) {
      IfrOptionList[SelectedDriverImageNum].StringToken = mDriverImageToken[Index];
      //
      // Use the NO. in driver binding buffer as value, will use it later
      //
      IfrOptionList[SelectedDriverImageNum].Value.u8 = (UINT8) (Index + 1);
      IfrOptionList[SelectedDriverImageNum].Flags = 0;

      //
      // Get the driver image total file path
      //
      LoadedImageHandleDevicePath = NULL;
      Status = gBS->HandleProtocol (
                          mDriverImageProtocol[Index]->DeviceHandle,
                          &gEfiDevicePathProtocolGuid,
                          &LoadedImageHandleDevicePath
                          );
      TatalFilePath = NULL;
      TatalFilePath = EfiAppendDevicePath (LoadedImageHandleDevicePath, mDriverImageProtocol[Index]->FilePath);
      ASSERT (TatalFilePath != NULL);
      //
      // Check the driver DriverImage's order number in mapping database
      //
      DriverImageNO = 0;
      LibCheckMapping (
              mControllerDevicePathProtocol[mSelectedCtrIndex],
              TatalFilePath,
              &mMappingDataBase,
              NULL,
              &DriverImageNO
              );
      if (DriverImageNO == 0) {
        DriverImageNO = (UINT32) mLastSavedDriverImageNum + 1;
        mLastSavedDriverImageNum++;
      }
      TempNO[SelectedDriverImageNum] = DriverImageNO;
      SelectedDriverImageNum ++;
    }
  }

  ASSERT (SelectedDriverImageNum == mSelectedDriverImageNum);
  //
  // NvRamMap Must be clear firstly
  //
  EfiZeroMem (FakeNvData->DriOrder, 100);

  //
  // Order the selected drivers according to the info already in mapping database
  // the less order number in mapping database the less order number in NvRamMap
  //
  for (Index=0; Index < SelectedDriverImageNum; Index++) {
    //
    // Find the minimal order number in TempNO array,  its index in TempNO is same as IfrOptionList array
    //
    MinNO = 0;
    for (Index1=0; Index1 < SelectedDriverImageNum; Index1++) {
      if (TempNO[Index1] < TempNO[MinNO]) {
        MinNO = Index1;
      }
    }
    //
    // the IfrOptionList[MinNO].Value = the driver NO. in driver binding buffer
    //
    FakeNvData->DriOrder[Index] =IfrOptionList[MinNO].Value.u8;
    TempNO[MinNO] = 101;
  }

  CreateOrderedListOpCode (
    DRIVER_ORDER_QUESTION_ID,
    VARSTORE_ID_PLAT_OVER_MNGR,
    DRIVER_ORDER_VAR_OFFSET,
    mControllerToken[mSelectedCtrIndex],
    mControllerToken[mSelectedCtrIndex],
    EFI_IFR_FLAG_RESET_REQUIRED,
    0,
    EFI_IFR_NUMERIC_SIZE_1,
    100,
    IfrOptionList,
    SelectedDriverImageNum,
    &UpdateData
    );

  //
  // Update third page form
  //
  IfrLibUpdateForm (
    Private->RegisteredHandle,
    &mPlatformOverridesManagerGuid,
    FORM_ID_ORDER,
    FORM_ID_ORDER,
    FALSE,
    &UpdateData
    );

  gBS->FreePool (IfrOptionList);
  gBS->FreePool (UpdateData.Data);
  return EFI_SUCCESS;
}

EFI_STATUS
CommintChanges (
  IN EFI_CALLBACK_INFO                *Private,
  IN UINT16                           KeyValue,
  IN PLAT_OVER_MNGR_DATA              *FakeNvData
  )
/*++

Routine Description:

  Save the save the mapping database to NV variable

Arguments:

  KeyValue - No use here.
  Data -         EFI_IFR_DATA_ARRAY data.
  Packet-       No use here.

  Returns -    Always successful

--*/
{
  EFI_STATUS                                Status;
  UINTN                                     Index;
  UINTN                                     SelectedDriverImageNum;
  EFI_DEVICE_PATH_PROTOCOL                  *LoadedImageHandleDevicePath;
  EFI_DEVICE_PATH_PROTOCOL                  *TatalFilePath;
  //
  //  Following code will be run if user select 'commint changes' in third page
  //  user enter 'Commit Changes' to save the mapping database
  //
  LibDeleteDriverImage (mControllerDevicePathProtocol[mSelectedCtrIndex], NULL, &mMappingDataBase);
  for (SelectedDriverImageNum = 0; SelectedDriverImageNum < mSelectedDriverImageNum; SelectedDriverImageNum++) {
    //
    // DriOrder[SelectedDriverImageNum] = the driver NO. in driver binding buffer
    //
    Index = FakeNvData->DriOrder[SelectedDriverImageNum] - 1;
    //
    // Get the driver image total file path
    //
    LoadedImageHandleDevicePath = NULL;
    Status = gBS->HandleProtocol (
                        mDriverImageProtocol[Index]->DeviceHandle,
                        &gEfiDevicePathProtocolGuid,
                        &LoadedImageHandleDevicePath
                        );
    ASSERT (!EFI_ERROR (Status));
    TatalFilePath = EfiAppendDevicePath (LoadedImageHandleDevicePath, mDriverImageProtocol[Index]->FilePath);
    ASSERT (TatalFilePath != NULL);
    LibInsertDriverImage (
            mControllerDevicePathProtocol[mSelectedCtrIndex],
            TatalFilePath,
            &mMappingDataBase,
            (UINT32)SelectedDriverImageNum + 1
            );
  }
  Status = LibSaveOverridesMapping (&mMappingDataBase);

  return Status;
}

EFI_STATUS
EFIAPI
PlatOverMngrExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
/*++

  Routine Description:
    This function allows a caller to extract the current configuration for one
    or more named elements from the target driver.

  Arguments:
    This       - Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
    Request    - A null-terminated Unicode string in <ConfigRequest> format.
    Progress   - On return, points to a character in the Request string.
                 Points to the string's null terminator if request was successful.
                 Points to the most recent '&' before the first failing name/value
                 pair (or the beginning of the string if the failure is in the
                 first name/value pair) if the request was not successful.
    Results    - A null-terminated Unicode string in <ConfigAltResp> format which
                 has all values filled in for the names in the Request string.
                 String to be allocated by the called function.

  Returns:
    EFI_SUCCESS           - The Results is filled with the requested values.
    EFI_OUT_OF_RESOURCES  - Not enough memory to store the results.
    EFI_INVALID_PARAMETER - Request is NULL, illegal syntax, or unknown name.
    EFI_NOT_FOUND         - Routing data doesn't match any storage in this driver.

--*/
{
  EFI_STATUS                       Status;
  EFI_CALLBACK_INFO                *Private;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;

  Private = EFI_CALLBACK_INFO_FROM_THIS (This);
  HiiConfigRouting = Private->HiiConfigRouting;

  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  Status = HiiConfigRouting->BlockToConfig (
                                HiiConfigRouting,
                                Request,
                                (UINT8 *) &Private->FakeNvData,
                                sizeof (PLAT_OVER_MNGR_DATA),
                                Results,
                                Progress
                                );
  return Status;
}

EFI_STATUS
EFIAPI
PlatOverMngrRouteConfig (
  IN  EFI_HII_CONFIG_ACCESS_PROTOCOL         *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
/*++

  Routine Description:
    This function processes the results of changes in configuration.

  Arguments:
    This          - Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
    Configuration - A null-terminated Unicode string in <ConfigResp> format.
    Progress      - A pointer to a string filled in with the offset of the most
                    recent '&' before the first failing name/value pair (or the
                    beginning of the string if the failure is in the first
                    name/value pair) or the terminating NULL if all was successful.

  Returns:
    EFI_SUCCESS           - The Results is processed successfully.
    EFI_INVALID_PARAMETER - Configuration is NULL.
    EFI_NOT_FOUND         - Routing data doesn't match any storage in this driver.

--*/
{
  EFI_CALLBACK_INFO                         *Private;
  EFI_STATUS                                Status;
  UINT16                                    KeyValue;
  UINTN                                     BufferSize;
  PLAT_OVER_MNGR_DATA                       *FakeNvData;

  Private     = EFI_CALLBACK_INFO_FROM_THIS (This);

  FakeNvData = &Private->FakeNvData;
  BufferSize = sizeof (PLAT_OVER_MNGR_DATA);
  Status = GetBrowserData (NULL, NULL, &BufferSize, (UINT8 *) FakeNvData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (mCurrentPage == FORM_ID_DRIVER) {
    KeyValue = KEY_VALUE_DRIVER_GOTO_ORDER;
    UpdatePrioritySelectPage (Private, KeyValue, FakeNvData);
    KeyValue = KEY_VALUE_ORDER_SAVE_AND_EXIT;
    CommintChanges (Private, KeyValue, FakeNvData);
    //
    // Since UpdatePrioritySelectPage will change mCurrentPage,
    // should ensure the mCurrentPage still indicate the second page here
    //
    mCurrentPage = FORM_ID_DRIVER;
  }

  if (mCurrentPage == FORM_ID_ORDER) {
    KeyValue = KEY_VALUE_ORDER_SAVE_AND_EXIT;
    CommintChanges (Private, KeyValue, FakeNvData);
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PlatOverMngrCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        KeyValue,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
/*++

Routine Description:

  This is the function that is called to provide results data to the driver.  This data
  consists of a unique key which is used to identify what data is either being passed back
  or being asked for.

Arguments:

  KeyValue - A unique Goto OpCode callback value which record user's selection

  0x100 <= KeyValue <0x500    : user select a controller item in the first page;
  KeyValue == 0x1234               : user select 'Refresh' in first page, or user select 'Go to Previous Menu' in second page
  KeyValue == 0x1235               : user select 'Pci device filter' in first page
  KeyValue == 0x1500               : user select 'order ... priority' item in second page
  KeyValue == 0x1800               : user select 'commint changes' in third page
  KeyValue == 0x2000              : user select 'Go to Previous Menu' in third page

  Returns -    Always successful

--*/
{
  EFI_CALLBACK_INFO                         *Private;
  EFI_STATUS                                Status;
  EFI_STRING_ID                             NewStringToken;
  UINTN                                     BufferSize;
  PLAT_OVER_MNGR_DATA                       *FakeNvData;
  EFI_INPUT_KEY                             Key; 
  
  Private = EFI_CALLBACK_INFO_FROM_THIS (This);

  FakeNvData = &Private->FakeNvData;
  BufferSize = sizeof (PLAT_OVER_MNGR_DATA);
  Status = GetBrowserData (NULL, NULL, &BufferSize, (UINT8 *) FakeNvData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (KeyValue == KEY_VALUE_DEVICE_REFRESH ||
      KeyValue == KEY_VALUE_DEVICE_FILTER ||
      KeyValue == KEY_VALUE_DRIVER_GOTO_PREVIOUS
      ) {
    UpdateDeviceSelectPage (Private, KeyValue, FakeNvData);
    //
    // Update page title string
    //
    NewStringToken = STRING_TOKEN (STR_TITLE);
    Status = IfrLibSetString (Private->RegisteredHandle, NewStringToken, L"First, Select the controller by device path");
    ASSERT_EFI_ERROR (Status);
  }

  if (((KEY_VALUE_DEVICE_OFFSET <= KeyValue) && (KeyValue < KEY_VALUE_DEVICE_MAX)) || (KeyValue == KEY_VALUE_ORDER_GOTO_PREVIOUS)) {
    if (KeyValue == KEY_VALUE_ORDER_GOTO_PREVIOUS) {
      KeyValue = (EFI_QUESTION_ID) mSelectedCtrIndex + 0x100;
    }
    UpdateBindingDriverSelectPage (Private, KeyValue, FakeNvData);    
    //
    // Update page title string
    //
    NewStringToken = STRING_TOKEN (STR_TITLE);
    Status = IfrLibSetString (Private->RegisteredHandle, NewStringToken, L"Second, Select drivers for the previous selected controller");
    ASSERT_EFI_ERROR (Status);
  }

  if (KeyValue == KEY_VALUE_DRIVER_GOTO_ORDER) {
    UpdatePrioritySelectPage (Private, KeyValue, FakeNvData);
    //
    // Update page title string
    //
    NewStringToken = STRING_TOKEN (STR_TITLE);
    Status = IfrLibSetString (Private->RegisteredHandle, NewStringToken, L"Finally, Set the priority order for the drivers and save them");
    ASSERT_EFI_ERROR (Status);
  }

  if (KeyValue == KEY_VALUE_ORDER_SAVE_AND_EXIT) {
    Status = CommintChanges (Private, KeyValue, FakeNvData);
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_SUBMIT;
    if (EFI_ERROR (Status)) {
      IfrLibCreatePopUp (1, &Key, L"Single Override Info too large, Saving Error!");
      return EFI_DEVICE_ERROR;
    }
  }

  if (KeyValue == KEY_VALUE_DEVICE_CLEAR) {
    //
    // Deletes all environment variable(s) that contain the override mappings info
    //
    LibFreeMappingDatabase (&mMappingDataBase);
    Status = LibSaveOverridesMapping (&mMappingDataBase);    
    UpdateDeviceSelectPage (Private, KeyValue, FakeNvData);
  }
  //
  // Pass changed uncommitted data back to Form Browser
  //
  BufferSize = sizeof (PLAT_OVER_MNGR_DATA);
  Status = SetBrowserData (NULL, NULL, BufferSize, (UINT8 *) FakeNvData, NULL);

  return EFI_SUCCESS;
}
