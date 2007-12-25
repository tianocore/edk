/*++
Copyright (c) 2006 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:
  SetOptions.c

Abstract:

  A UI driver to offer a UI interface in device manager to let user use the SetOptions() function in configuration protocol to configure device.
  The main flow:
  1. The UI driver dynamicly locate all drivers which support configuration protocol and its managed device infos.
  2. The UI driver export and dynamicly update a  formset titled 'Set Options' according to the collected infos
  3. Device manager present the formset page to let user select
  4. The UI driver invoke the SetOptions() function in configuration protocol according to user's selection

--*/

#include "SetOptions.h"

EFI_GUID mSetOptionsGuid = {
  0x22afbab1, 0x23b, 0x452d, 0x80, 0xcf, 0x63, 0x75, 0xe0, 0x33, 0x91, 0x2e
};

EFI_DRIVER_ENTRY_POINT (SetOptionsInit)

STATIC CFG_PROTOCOL_INVOKER_CHOICE mChoice[MAX_CHOICE_NUM];

EFI_STATUS
EFIAPI
SetOptionsInit (
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
  EFI_HII_HANDLE              HiiHandle;
  EFI_CALLBACK_INFO           *CallbackInfo;
  UINTN                       ChoiceIndex;

  //
  // Initialize the library and our protocol.
  //
  EfiInitializeDriverLib (ImageHandle, SystemTable);

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

  CallbackInfo->ConfigAccess.ExtractConfig = FakeExtractConfig;
  CallbackInfo->ConfigAccess.RouteConfig   = FakeRouteConfig;
  CallbackInfo->ConfigAccess.Callback      = SetOptionsCallback;

  //
  // Create driver handle used by HII database
  //
  Status = CreateHiiDriverHandle (&CallbackInfo->DriverHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Install Config Access protocol to driver handle
  //
  Status = gBS->InstallProtocolInterface (
                  &CallbackInfo->DriverHandle,
                  &gEfiHiiConfigAccessProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &CallbackInfo->ConfigAccess
                  );
  ASSERT_EFI_ERROR (Status);

  PackageList = PreparePackageList (2, &mSetOptionsGuid, VfrBin, SetOptionsStrings);
  Status      = HiiDatabase->NewPackageList (
                               HiiDatabase,
                               PackageList,
                               CallbackInfo->DriverHandle,
                               &HiiHandle
                               );
  gBS->FreePool (PackageList);

  CallbackInfo->RegisteredHandle = HiiHandle;

  //
  // Clear all the mChoices
  //
  for (ChoiceIndex = 0; ChoiceIndex < MAX_CHOICE_NUM; ChoiceIndex++) {
    mChoice[ChoiceIndex].DriverImageHandle = NULL;
    mChoice[ChoiceIndex].DriverConfiguration = NULL;
    mChoice[ChoiceIndex].ControllerHandle = NULL;
    mChoice[ChoiceIndex].ChildControllerHandle = NULL;
    mChoice[ChoiceIndex].DescriptionToken = 0;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FakeExtractConfig (
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
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
FakeRouteConfig (
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
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SetOptionsCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        KeyValue,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
/*++

  Routine Description:
    This function processes the results of changes in configuration.

  Arguments:
    This          - Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
    Action        - Specifies the type of action taken by the browser.
    KeyValue      - A unique value which is sent to the original exporting driver
                    so that it can identify the type of data to expect.
    Type          - The type of value for the question.
    Value         - A pointer to the data being sent to the original exporting driver.
    ActionRequest - On return, points to the action requested by the callback function.

  Returns:
    EFI_SUCCESS          - The callback successfully handled the action.
    EFI_OUT_OF_RESOURCES - Not enough storage is available to hold the variable and its data.
    EFI_DEVICE_ERROR     - The variable could not be saved.
    EFI_UNSUPPORTED      - The specified Action is not supported by the callback.

--*/
{
  EFI_CALLBACK_INFO                         *Private;
  EFI_HII_UPDATE_DATA                       UpdateData;

  EFI_STATUS                                Status;
  CHAR8                                     OldLanguage[4];
  CHAR8                                     Language[RFC_3066_ENTRY_SIZE];
  UINTN                                     LangSize;

  UINTN                                     Index;
  UINTN                                     ChoiceIndex;

  UINTN                                     DriverImageHandleCount;
  EFI_HANDLE                                *DriverImageHandleBuffer;

  EFI_DRIVER_CONFIGURATION_PROTOCOL         *DriverConfiguration;
  EFI_DRIVER_BINDING_PROTOCOL               *DriverBinding;
  EFI_LOADED_IMAGE_PROTOCOL                 *LoadedImage;
  EFI_COMPONENT_NAME2_PROTOCOL              *ComponentName;

  EFI_DEVICE_PATH_PROTOCOL                  *DevicePath;
  UINTN                                     ControllerHandleIndex;
  CHAR16                                    *ControllerHandleName;
  BOOLEAN                                   FreeControllerHandleName;
  UINTN                                     ControllerHandleCount;
  EFI_HANDLE                                *ControllerHandleBuffer;

  UINTN                                     ChildControllerHandleIndex;
  CHAR16                                    *ChildControllerHandleName;
  BOOLEAN                                   FreeChildControllerHandleName;
  UINTN                                     ChildControllerHandleCount;
  EFI_HANDLE                                *ChildControllerHandleBuffer;

  CHAR16                                    *NewString;
  EFI_STRING_ID                             NewStringToken;
  CHAR16                                    *DriverName;
  BOOLEAN                                   FreeDriverName;

  EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  ActionRequired;


  Private     = EFI_CALLBACK_INFO_FROM_THIS (This);

  //
  // Get current language to set options
  //
  LangSize = RFC_3066_ENTRY_SIZE;
  Status = gRT->GetVariable (
              L"PlatformLang",
              &gEfiGlobalVariableGuid,
              NULL,
              &LangSize,
              Language
              );

  ASSERT_EFI_ERROR (Status);
  ConvertRfc3066LanguageToIso639Language (Language, OldLanguage);

  //
  // Tast1:  Invoke the configuration protocol setoptions function according to user selection.
  // KeyValue between 0x100~0x200 means user select a driver or device item in 'Set Options'  dynamic page
  //
  if ((0x100 <= KeyValue) && (KeyValue <= 0x200)) {

    ChoiceIndex = KeyValue - 0x100;
    ActionRequired = 0;

    //
    // Invoke the user selceted item's SetOptions function with corresponding driver or device handles
    //
    gST->ConOut->ClearScreen (gST->ConOut);
    mChoice[ChoiceIndex].DriverConfiguration->SetOptions (
                                              mChoice[ChoiceIndex].DriverConfiguration,
                                              mChoice[ChoiceIndex].ControllerHandle,
                                              mChoice[ChoiceIndex].ChildControllerHandle,
                                              OldLanguage,
                                              &ActionRequired
                                              );
    gST->ConOut->ClearScreen (gST->ConOut);

    //
    // Notify user the action required after SetOptions function finish, and do the action according to user intent
    //
    Status = ProcessActionRequired (
                Private,
                ActionRequired,
                mChoice[ChoiceIndex].DriverImageHandle,
                mChoice[ChoiceIndex].ControllerHandle,
                mChoice[ChoiceIndex].ChildControllerHandle
                );
    gST->ConOut->ClearScreen (gST->ConOut);

  }

  //
  // Task 2: Collect all the available driver infos and update the formset.
  // The available driver is those drivers which install efi driver configuration protocol
  //

  //
  // Allocate memory to Update form
  //
  DriverImageHandleBuffer = NULL;
  ControllerHandleBuffer = NULL;
  ChildControllerHandleBuffer = NULL;

  UpdateData.BufferSize = UPDATE_DATA_SIZE;
  UpdateData.Offset = 0;
  UpdateData.Data = EfiLibAllocateZeroPool (UPDATE_DATA_SIZE);
  ASSERT (UpdateData.Data != NULL);

  //
  // When user enter the page at first time, the 'first refresh' string is given to notify user to refresh all the drivers,
  // then the 'first refresh' string will be replaced by the 'refresh' string, and the two strings content are  same after the replacement
  //
  NewStringToken = STRING_TOKEN (STR_FIRST_REFRESH);
  NewString = GetString (Private, STRING_TOKEN (STR_REFRESH));
  ASSERT (NewString != NULL);
  Status = IfrLibNewString (Private->RegisteredHandle, &NewStringToken, NewString);
  ASSERT_EFI_ERROR (Status);
  gBS->FreePool (NewString);

  NewStringToken = STRING_TOKEN (STR_FIRST_REFRESH_HELP);
  NewString = GetString (Private, STRING_TOKEN (STR_REFRESH_HELP));
  ASSERT (NewString != NULL);
  Status = IfrLibNewString (Private->RegisteredHandle, &NewStringToken, NewString);
  ASSERT_EFI_ERROR (Status);
  gBS->FreePool (NewString);

  //
  // Get all drivers handles which has configuration protocol
  //
  DriverImageHandleCount  = 0;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiDriverConfigurationProtocolGuid,
                  NULL,
                  &DriverImageHandleCount,
                  &DriverImageHandleBuffer
                  );

  //
  // Scan every driver image handle to get its and its managed device and child device info,
  // including Component name/image name/device handle and so on. Create and associate a item
  // in Set Option page to very driver and its managed device and  child device
  //
  ChoiceIndex = 0; // Item index in Set Option page
  for (Index = 0; (Index < DriverImageHandleCount) && (ChoiceIndex < MAX_CHOICE_NUM); Index++) {
    //
    // step1 : get the driver info
    //
    DriverConfiguration = NULL;
    Status = gBS->OpenProtocol (
                    DriverImageHandleBuffer[Index],
                    &gEfiDriverConfigurationProtocolGuid,
                    (VOID **) &DriverConfiguration,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    ASSERT_EFI_ERROR (Status);

    DriverBinding =NULL;
    Status = gBS->OpenProtocol (
                    DriverImageHandleBuffer[Index],
                    &gEfiDriverBindingProtocolGuid,
                    (VOID **) &DriverBinding,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    LoadedImage = NULL;
    Status = gBS->OpenProtocol (
                    DriverBinding->ImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    (VOID **) &LoadedImage,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      LoadedImage = NULL;
    }

    ComponentName  = NULL;
    Status = gBS->OpenProtocol (
                    DriverBinding->ImageHandle,
                    &gEfiComponentName2ProtocolGuid,
                    (VOID **) &ComponentName,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR(Status)) {
      return EFI_UNSUPPORTED;
    }

    //
    // Get the driver name
    // First try to get its component name, if fail, get its image name then
    //
    Status = EFI_UNSUPPORTED;
    DriverName = NULL;
    FreeDriverName = FALSE;
    if ((ComponentName != NULL) && (ComponentName->GetDriverName != NULL)) {
      Status = ComponentName->GetDriverName (
                                ComponentName,
                                Language,
                                &DriverName
                                );
    }
    if (EFI_ERROR (Status) && (LoadedImage != NULL)) {
      DriverName = GetImageName (LoadedImage);
    }
    if (DriverName == NULL) {
      DriverName = GetString (Private, STRING_TOKEN (STR_DRIVER_DEFAULT_NAME));
      ASSERT (DriverName != NULL);
      FreeDriverName = TRUE;  // the DriverName string need to free pool
    }

    //
    // Export the driver name string and create item in set options page
    //
    NewString = EfiLibAllocateZeroPool (EfiStrSize (DriverName) + EfiStrSize (L" |-"));
    EfiStrCat (NewString, L" |-");
    EfiStrCat (NewString, DriverName);
    NewStringToken = mChoice[ChoiceIndex].DescriptionToken;
    Status = IfrLibNewString (Private->RegisteredHandle, &NewStringToken, NewString);
    ASSERT_EFI_ERROR (Status);
    gBS->FreePool (NewString);
    if (FreeDriverName) {
      gBS->FreePool (DriverName);
    }

    CreateGotoOpCode (
      0x1234,
      NewStringToken,
      STRING_TOKEN (STR_GOTO_HELP_DRIVER),
      EFI_IFR_FLAG_CALLBACK,
      (UINT16) (ChoiceIndex + 0x100),
      &UpdateData
      );

    //
    // Associate callback key with  setoptions function related parameters to used by Task 1
    //
    mChoice[ChoiceIndex].DriverImageHandle = DriverImageHandleBuffer[Index];
    mChoice[ChoiceIndex].DriverConfiguration = DriverConfiguration;
    mChoice[ChoiceIndex].ControllerHandle = NULL;
    mChoice[ChoiceIndex].ChildControllerHandle = NULL;
    mChoice[ChoiceIndex].DescriptionToken = NewStringToken;
    ChoiceIndex++;

    //
    // step2 : get the driver direct managed device info
    //

    //
    // Get the driver all direct managed devices handles
    //
    Status = GetDeviceHandlesManagedByDriver (
              DriverImageHandleBuffer[Index],
              &ControllerHandleCount,           // Get managed devices count
              &ControllerHandleBuffer           // return all handles in buffer
              );
    if (ControllerHandleBuffer == NULL) {
      continue;
    }

    for (ControllerHandleIndex = 0; ControllerHandleIndex < ControllerHandleCount; ControllerHandleIndex++) {
      //
      // Get the  managed device name
      // First try to get its component name, if fail, get its device path then
      // The component name string need not free pool, but the device path string and default string need safe free pool after export the string to Hii database
      //
      FreeControllerHandleName = FALSE;
      ControllerHandleName = NULL;
      Status = EFI_UNSUPPORTED;
      if ((ComponentName != NULL) && (ComponentName->GetControllerName != NULL)) {
        Status = ComponentName->GetControllerName (
                                  ComponentName,
                                  ControllerHandleBuffer[ControllerHandleIndex],
                                  NULL,
                                  Language,
                                  &ControllerHandleName
                                  );
      }
      if (EFI_ERROR (Status)) {
        Status = gBS->OpenProtocol (
                        ControllerHandleBuffer[ControllerHandleIndex],
                        &gEfiDevicePathProtocolGuid,
                        (VOID **) &DevicePath,
                        NULL,
                        NULL,
                        EFI_OPEN_PROTOCOL_GET_PROTOCOL
                        );
        if (!EFI_ERROR (Status)) {
          ControllerHandleName = DevicePathToStr ( DevicePath );
          FreeControllerHandleName = TRUE; // the Controller Handle Name string need to free pool
        }
      }
      if (ControllerHandleName == NULL) {
        ControllerHandleName = GetString (Private, STRING_TOKEN (STR_DRIVER_CONTROLLER_DEFAULT_NAME));
        ASSERT (ControllerHandleName != NULL);
        FreeControllerHandleName = TRUE;  // the Controller Handle Name string need to free pool
      }
      //
      // do some alignment for NewString if needed
      //
      AlignmentItem (ControllerHandleName, L"   |-", &NewString);

      NewStringToken = mChoice[ChoiceIndex].DescriptionToken;
      IfrLibNewString (Private->RegisteredHandle, &NewStringToken, NewString);
      gBS->FreePool (NewString);
      if (FreeControllerHandleName) {
        gBS->FreePool (ControllerHandleName);
      }

      CreateGotoOpCode (
        0x1234,
        NewStringToken,
        STRING_TOKEN (STR_GOTO_HELP_DEVICE),
        EFI_IFR_FLAG_CALLBACK,
        (UINT16) (ChoiceIndex + 0x100),
        &UpdateData
        );

      //
      // Associate callback key with  setoptions function related parameters to used by Task 1
      //
      mChoice[ChoiceIndex].DriverImageHandle = DriverImageHandleBuffer[Index];
      mChoice[ChoiceIndex].DriverConfiguration = DriverConfiguration;
      mChoice[ChoiceIndex].ControllerHandle = ControllerHandleBuffer[ControllerHandleIndex];
      mChoice[ChoiceIndex].ChildControllerHandle = NULL;
      mChoice[ChoiceIndex].DescriptionToken = NewStringToken;
      ChoiceIndex++;

      //
      // step3 : get the infos of child devices created by the driver
      //
      Status = GetChildDeviceHandlesManagedByDriver (
                DriverImageHandleBuffer[Index],
                ControllerHandleBuffer[ControllerHandleIndex],
                &ChildControllerHandleCount,      // Get managed devices count
                &ChildControllerHandleBuffer      // return all handles in buffer
                );

      for (ChildControllerHandleIndex = 0; ChildControllerHandleIndex < ChildControllerHandleCount; ChildControllerHandleIndex++) {
        //
        // Get the  managed child device name
        // First try to get its component name, if fail, get its device path then
        // The component name string need not free pool, but the device path string  need safe free pool after export the string to Hii database
        //
        FreeChildControllerHandleName = FALSE;
        ChildControllerHandleName = NULL;
        Status = EFI_UNSUPPORTED;
        if ((ComponentName != NULL) && (ComponentName->GetDriverName != NULL)) {
          Status = ComponentName->GetControllerName (
                                    ComponentName,
                                    ControllerHandleBuffer[ControllerHandleIndex],
                                    ChildControllerHandleBuffer[ChildControllerHandleIndex],
                                    Language,
                                    &ChildControllerHandleName
                                    );
        }
        if (EFI_ERROR (Status)) {
          Status = gBS->OpenProtocol (
                          ChildControllerHandleBuffer[ChildControllerHandleIndex],
                          &gEfiDevicePathProtocolGuid,
                          (VOID **) &DevicePath,
                          NULL,
                          NULL,
                          EFI_OPEN_PROTOCOL_GET_PROTOCOL
                          );
          if (!EFI_ERROR (Status)) {
            ChildControllerHandleName = DevicePathToStr ( DevicePath );
            FreeChildControllerHandleName = TRUE; // the Child Controller Handle Name string need to free pool
          }
        }
        if (ChildControllerHandleName == NULL) {
          ChildControllerHandleName = GetString (Private, STRING_TOKEN (STR_DRIVER_CHILD_CONTROLLER_DEFAULT_NAME));
          ASSERT (ChildControllerHandleName != NULL);
          FreeChildControllerHandleName = TRUE;  // the Controller Handle Name string need to free pool
        }

        //
        // do some alignment for NewString if needed
        //
        AlignmentItem (ChildControllerHandleName, L"     |--", &NewString);

        NewStringToken = mChoice[ChoiceIndex].DescriptionToken;
        IfrLibNewString (Private->RegisteredHandle, &NewStringToken, NewString);
        gBS->FreePool (NewString);
        if (FreeChildControllerHandleName) {
          gBS->FreePool (ChildControllerHandleName);
        }
        CreateGotoOpCode (
          0x1234,
          NewStringToken,
          STRING_TOKEN (STR_GOTO_HELP_CHILD_DEVICE),
          EFI_IFR_FLAG_CALLBACK,
          (UINT16) (ChoiceIndex + 0x100),
          &UpdateData
          );
        //
        // Associate callback key with  setoptions function related parameters to used by Task 1
        //
        mChoice[ChoiceIndex].DriverImageHandle = DriverImageHandleBuffer[Index];
        mChoice[ChoiceIndex].DriverConfiguration = DriverConfiguration;
        mChoice[ChoiceIndex].ControllerHandle = ControllerHandleBuffer[ControllerHandleIndex];
        mChoice[ChoiceIndex].ChildControllerHandle = ChildControllerHandleBuffer[ChildControllerHandleIndex];
        mChoice[ChoiceIndex].DescriptionToken = NewStringToken;
        ChoiceIndex++;
      }
      if (ChildControllerHandleBuffer != NULL) {
        gBS->FreePool (ChildControllerHandleBuffer);
      }
    }

    //
    // Update Set Option page form
    //
    IfrLibUpdateForm (
      Private->RegisteredHandle,
      &mSetOptionsGuid,
      0x1234,
      0x1234,
      FALSE,
      &UpdateData
      );
    if (ControllerHandleBuffer != NULL) {
      gBS->FreePool (ControllerHandleBuffer);
    }

  }

  gBS->FreePool (UpdateData.Data);

  if (DriverImageHandleBuffer != NULL) {
    gBS->FreePool (DriverImageHandleBuffer);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ProcessActionRequired (
  IN  EFI_CALLBACK_INFO                         *Private,
  IN  EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  ActionRequired,
  IN  EFI_HANDLE                                DriverImageHandle,
  IN  EFI_HANDLE                                ControllerHandle,
  IN  EFI_HANDLE                                ChildControllerHandle
  )
/*++

  Routine Description:
    Notify user the action required after SetOptions function finish, and do the action according to user's intent

  Arguments:
    ActionRequired - SetOPtions function returned data, refer to driver configuration protocol spec
    DriverImageHandle - The driver image handle which support driver configuration protocol
    ControllerHandle - The device controller handle to set option, refer to driver configuration protocol spec
    ChildControllerHandle - The child device controller handle to set option, refer to driver configuration protocol spec

  Returns:
    EFI_STATUS

--*/
{
  CHAR16                        *String;
  CHAR16                        *SelectString;
  UINTN                         Status;
  EFI_INPUT_KEY                 Key;
  EFI_HANDLE                    DriverImageHandleList[2];

  if ((DriverImageHandle == NULL) ||
      ((ControllerHandle == NULL) && (ChildControllerHandle != NULL))
     ) {
    return EFI_INVALID_PARAMETER;
  }

  SelectString = GetString (Private, STRING_TOKEN (STR_ACTION_REQUIRED_ACTION_NOW));
  ASSERT (SelectString != NULL);


  switch (ActionRequired) {
  case EfiDriverConfigurationActionNone:
    gBS->FreePool (SelectString);
    SelectString = GetString (Private, STRING_TOKEN (STR_ACTION_REQUIRED_CONTINUE));
    ASSERT (SelectString != NULL);

    String = GetString (Private, STRING_TOKEN (STR_ACTION_REQUIRED_NONE));
    ASSERT (String != NULL);

    break;

  case EfiDriverConfigurationActionStopController:
    String = GetString (Private, STRING_TOKEN (STR_ACTION_REQUIRED_STOP_CONTROLLER));
    ASSERT (String != NULL);

    break;

  case EfiDriverConfigurationActionRestartController:
    String = GetString (Private, STRING_TOKEN (STR_ACTION_REQUIRED_RESTART_CONTROLLER));
    ASSERT (String != NULL);

    break;

  case EfiDriverConfigurationActionRestartPlatform:
    String = GetString (Private, STRING_TOKEN (STR_ACTION_REQUIRED_RESTART_PLATFORM));
    ASSERT (String != NULL);

    break;

  default:
    gBS->FreePool (SelectString);
    return EFI_INVALID_PARAMETER;
  }
  //
  // Popup a menu to notice user
  //
  do {
    IfrLibCreatePopUp (2, &Key, String, SelectString);
  } while ((Key.ScanCode != SCAN_ESC) && (Key.UnicodeChar != CHAR_CARRIAGE_RETURN));
  gBS->FreePool (SelectString);
  gBS->FreePool (String);

  Status = EFI_SUCCESS;
  if ((Key.UnicodeChar == CHAR_CARRIAGE_RETURN)) {
    switch (ActionRequired) {
    case EfiDriverConfigurationActionStopController:
      //
      // If user select a bus driver item
      //
      if ((ControllerHandle == NULL)) {
        gST->ConOut->ClearScreen (gST->ConOut);
        String = GetString (Private, STRING_TOKEN (STR_ACTION_REQUIRED_STOP_SELECT_ERROR));
        ASSERT (String != NULL);
        IfrLibCreatePopUp (1, &Key, String);
        gBS->FreePool (String);
        break;
      }

      //
      // If user select a device or child device controller item
      //
      if ((ControllerHandle != NULL)) {
        Status = gBS->DisconnectController (ControllerHandle, DriverImageHandle, ChildControllerHandle);
        if (EFI_ERROR(Status)) {
          gST->ConOut->ClearScreen (gST->ConOut);
          String = GetString (Private, STRING_TOKEN (STR_ACTION_REQUIRED_DISCON_CONTRO_ERROR));
          ASSERT (String != NULL);
          IfrLibCreatePopUp (1, &Key, String);
          gBS->FreePool (String);
          break;
        }
      }
      break;

    case EfiDriverConfigurationActionRestartController:
      //
      // If user select a bus driver item
      //
      if ((ControllerHandle == NULL)) {
        gST->ConOut->ClearScreen (gST->ConOut);
        String = GetString (Private, STRING_TOKEN (STR_ACTION_REQUIRED_START_SELECT_ERROR));
        ASSERT (String != NULL);
        IfrLibCreatePopUp (1, &Key, String);
        gBS->FreePool (String);
        break;
      }
      //
      // If user select a device or child device controller item, disconnect the driver from device and all its children first, then
      // connect the driver to all of them.
      // Since the driver must be a driver-model driver, and the managed device or child device should be reconnected successful multi times,
      //  to be simple, merge the restart action for both  device and child device.
      //
      if ((ControllerHandle != NULL)) {
        Status = gBS->DisconnectController (ControllerHandle, DriverImageHandle, NULL);
        if (EFI_ERROR(Status)) {
          gST->ConOut->ClearScreen (gST->ConOut);
          String = GetString (Private, STRING_TOKEN (STR_ACTION_REQUIRED_DISCON_CHILCON_ERROR));
          ASSERT (String != NULL);
          IfrLibCreatePopUp (1, &Key, String);
          gBS->FreePool (String);
          break;
        }

        DriverImageHandleList[0]  = DriverImageHandle;
        DriverImageHandleList[1]  = NULL;
        Status = gBS->ConnectController (ControllerHandle, DriverImageHandleList, NULL, TRUE);
        if (EFI_ERROR(Status)) {
          gST->ConOut->ClearScreen (gST->ConOut);
          String = GetString (Private, STRING_TOKEN (STR_ACTION_REQUIRED_CONNECT_CONTRO_ERROR));
          ASSERT (String != NULL);
          IfrLibCreatePopUp (1, &Key, String);
          gBS->FreePool (String);
          break;
        }
      }

      break;

    case EfiDriverConfigurationActionRestartPlatform:
      gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
      break;

    default:
      break;
    }
  }
  return Status;
}
