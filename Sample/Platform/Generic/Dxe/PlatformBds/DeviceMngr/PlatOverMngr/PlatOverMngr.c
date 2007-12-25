/*++
Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:
  PlatOverMngr.c

Abstract:

  A UI driver to offer a UI interface in device manager to let user configue platform override protocol to
  override the default algorithm for matching drivers to controllers.
  
  The main flow:
  1.  The UI driver dynamicly locate all controller device path.
  2. The UI driver dynamicly locate all drivers which support binding protocol.
  3. The UI driver export and dynamicly update two  menu to let user select the mapping between drivers to controllers.
  4. The UI driver save all the mapping info in NV variables which will be consumed by platform override protocol driver
      to publish the platform override protocol.

--*/

#include "PlatOverMngr.h"
#include "PlatDriOverLib.h"

#define PLAT_OVER_MNGR_GUID \
  { \
    0x8614567d, 0x35be, 0x4415, 0x8d, 0x88, 0xbd, 0x7d, 0xc, 0x9c, 0x70, 0xc0 \
  }


EFI_DRIVER_ENTRY_POINT (PlatOverMngrInit)

STATIC  EFI_GUID      mPlatformOverridesManagerGuid = PLAT_OVER_MNGR_GUID;

STATIC  EFI_LIST_ENTRY  mMappingDataBase = INITIALIZE_LIST_HEAD_VARIABLE (mMappingDataBase);

STATIC  EFI_HANDLE    *mDevicePathHandleBuffer;
STATIC  EFI_HANDLE    *mDriverImageHandleBuffer;
STATIC  EFI_HANDLE    mSelectedCtrDPHandle;
  
STATIC CFG_PROTOCOL_INVOKER_CHOICE mChoice[MAX_CHOICE_NUM];

STATIC UINTN      mSelectedCtrIndex;
STATIC STRING_REF mControllerToken[MAX_CHOICE_NUM];

STATIC UINTN                        mDriverImageHandleCount;
STATIC STRING_REF                   mDriverImageToken[MAX_CHOICE_NUM];  
STATIC STRING_REF                   mDriverImageFilePathToken[MAX_CHOICE_NUM];  
STATIC EFI_LOADED_IMAGE_PROTOCOL    *mDriverImageProtocol[MAX_CHOICE_NUM];  
STATIC EFI_DEVICE_PATH_PROTOCOL     *mControllerDevicePathProtocol[MAX_CHOICE_NUM];  
STATIC UINTN                        mSelectedDriverImageNum;
STATIC UINTN                        mLastSavedDriverImageNum;
STATIC CHAR8                        mLanguage[4];
STATIC CHAR16                       mLang[4];
STATIC UINT8                        mCurrentPage;

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
  EFI_STATUS          Status;
  EFI_HII_PROTOCOL    *Hii;
  EFI_HII_PACKAGES    *PackageList;
  EFI_HII_HANDLE      HiiHandle;
  EFI_HII_UPDATE_DATA *UpdateData;
  EFI_CALLBACK_INFO   *CallbackInfo;
  EFI_HANDLE          Handle;
  UINTN               Index;
  EFI_GUID            PlatOverMngrGuid = PLAT_OVER_MNGR_GUID;

  //
  // Initialize the library and our protocol.
  //
  DxeInitializeDriverLib (ImageHandle, SystemTable);

  //
  // There should only be one HII protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiProtocolGuid,
                  NULL,
                  &Hii
                  );
  if (EFI_ERROR (Status)) {
    return Status ;
  }

  CallbackInfo = EfiLibAllocateZeroPool (sizeof (EFI_CALLBACK_INFO));
  if (CallbackInfo == NULL) {
    return EFI_BAD_BUFFER_SIZE;
  }

  CallbackInfo->Signature = EFI_CALLBACK_INFO_SIGNATURE;
  CallbackInfo->Hii       = Hii;

  //
  // This driver implement a NV write worker function and a callback evaluator
  //
  CallbackInfo->DriverCallback.NvRead   = NULL;
  CallbackInfo->DriverCallback.NvWrite  = PlatOverMngrNvWrite;
  CallbackInfo->DriverCallback.Callback = PlatOverMngrCallback;

  //
  // Install protocol interface
  //
  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEfiFormCallbackProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &CallbackInfo->DriverCallback
                  );

  ASSERT_EFI_ERROR (Status);

  CallbackInfo->CallbackHandle  = Handle;

  PackageList = PreparePackages (2, &PlatOverMngrGuid, VfrBin, PlatOverMngrStrings);
  Status      = Hii->NewPack (Hii, PackageList, &HiiHandle);
  gBS->FreePool (PackageList);

  CallbackInfo->RegisteredHandle = HiiHandle;

  //
  // Allocate space for creation of Buffer
  //
  UpdateData = EfiLibAllocateZeroPool (0x1000);
  ASSERT (UpdateData != NULL);

  //
  // Flag update pending in FormSet
  //
  UpdateData->FormSetUpdate = TRUE;
  //
  // Register CallbackHandle data for FormSet
  //
  UpdateData->FormCallbackHandle = (EFI_PHYSICAL_ADDRESS) (UINTN) CallbackInfo->CallbackHandle;
  UpdateData->FormUpdate  = FALSE;
  UpdateData->FormTitle   = 0;
  UpdateData->DataCount   = 0;

  Hii->UpdateForm (Hii, HiiHandle, (EFI_FORM_LABEL) 0x0, TRUE, UpdateData);
  gBS->FreePool (UpdateData);

  mDriverImageHandleCount = 0;
  mCurrentPage = 0;
  //
  // Clear all the globle variable
  //
  for (Index = 0; Index < MAX_CHOICE_NUM; Index++) {
    mDriverImageToken[Index] = 0;
    mDriverImageFilePathToken[Index] = 0;
    mControllerToken[Index] = 0;
    mDriverImageProtocol[Index] = NULL;
  }
  return EFI_SUCCESS;
}

EFI_STRING
GetString (
  IN  EFI_CALLBACK_INFO   *Private,
  IN  STRING_REF          Token
  )
/*++

Routine Description:
  Extract a string from the driver package, the string is specified by token and current setted langurage.
  The rountine will allocate pool buffer for the wanted string, and it is the caller's responsibility to safe
  free the buffer

Arguments:
  Private - pointer to the callback private structure
  Token - the string token assigned to the string

Returns:
  Pointer to the wanted string if it is found, otherwise return NULL

--*/
{
  EFI_STATUS                    Status;
  UINTN                         StringLen;
  EFI_STRING                    String;

  StringLen = 0x1000;
  String = EfiLibAllocateZeroPool (StringLen);
  ASSERT (String != NULL);

  Status = Private->Hii->GetString (
                          Private->Hii,
                          Private->RegisteredHandle,
                          Token,
                          TRUE,
                          mLang,
                          &StringLen,
                          String
                          );
  if (EFI_ERROR(Status)) {
    gBS->FreePool (String);
    String = NULL;
  }

  return String;
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
EFIAPI
UpdateDeviceSelectPage (
  IN EFI_CALLBACK_INFO                *Private,
  IN UINT16                           KeyValue,
  IN EFI_IFR_DATA_ARRAY               *Data
  )
/*++

Routine Description:
  Prepare the first page to let user select the device controller which need to add mapping drivers
  
Arguments:

  KeyValue -  No use here.
  Data -         EFI_IFR_DATA_ARRAY data.
  Packet-       No use here.

  Returns -    Always successful

--*/
{
  EFI_HII_UPDATE_DATA                       *UpdateData;
  EFI_STATUS                                Status;
  UINTN                                     LangSize;
  UINTN                                     Index;
  UINT8                                     *Location;
  UINTN                                     DevicePathHandleCount;

  CHAR16                                    *NewString;
  STRING_REF                                NewStringToken;
  CHAR16                                    *ControllerName;
  EFI_DEVICE_PATH_PROTOCOL                  *ControllerDevicePath;
  EFI_PCI_IO_PROTOCOL                       *PciIo;
  EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL *BusSpecificDriverOverride;
  
  mCurrentPage = 0x01;
  //
  // Following code will be run if user select 'Refresh' in first page
  // During first page, user will see all currnet controller device path in system,
  // select any device path will go to second page to select its overrides drivers
  //

  LangSize = 0x3;
  Status = gRT->GetVariable (
              L"Lang",
              &gEfiGlobalVariableGuid,
              NULL,
              &LangSize,
              mLanguage
              );
  ASSERT_EFI_ERROR (Status);

  Status = GetCurrentLanguage (mLang);
  ASSERT_EFI_ERROR (Status);
  //
  // Initial the mapping database in memory
  //
  LibFreeMappingDatabase (&mMappingDataBase);
  Status = LibInitOverridesMapping (&mMappingDataBase);
  
  //
  // Clear all the content in the first page
  //
  UpdateData = NULL;
  UpdateData = EfiLibAllocateZeroPool (UPDATE_DATA_SIZE);
  ASSERT (UpdateData != NULL);

  UpdateData->FormSetUpdate       = FALSE;
  UpdateData->FormCallbackHandle  = 0;
  UpdateData->FormUpdate          = FALSE;
  UpdateData->FormTitle           = 0;
  UpdateData->DataCount           = 0xff;
  UpdateData->Data                = NULL;

  Private->Hii->UpdateForm (
                  Private->Hii,
                  Private->RegisteredHandle,
                  (EFI_FORM_LABEL) 0x1234,  // Label 0x1234
                  FALSE,                    // Remove Op-codes (will never remove form/endform)
                  UpdateData                // Significant value is UpdateData->DataCount
                  );
  //
  // When user enter the page at first time, the 'first refresh' string is given to notify user to refresh all the drivers,
  // then the 'first refresh' string will be replaced by the 'refresh' string, and the two strings content are  same after the replacement
  //
  NewStringToken = (STRING_REF) STR_FIRST_REFRESH;
  NewString = GetString (Private, (STRING_REF) STR_REFRESH);
  ASSERT (NewString != NULL);
  Status = Private->Hii->NewString (Private->Hii, mLang, Private->RegisteredHandle, &NewStringToken, NewString);
  ASSERT_EFI_ERROR (Status);
  gBS->FreePool (NewString);

  NewStringToken = (STRING_REF) STR_FIRST_REFRESH_HELP;
  NewString = GetString (Private, (STRING_REF) STR_REFRESH_HELP);
  ASSERT (NewString != NULL);
  Status = Private->Hii->NewString (Private->Hii, mLang, Private->RegisteredHandle, &NewStringToken, NewString);
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
    if (((MyIfrNVData *) Data->NvRamMap)->PciDeviceFilter == 0x01) {
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
    // Create a item for the driver in set options page
    // Clear the Update buffer
    //
    UpdateData->FormSetUpdate       = FALSE;
    UpdateData->FormCallbackHandle  = 0;
    UpdateData->FormUpdate          = FALSE;
    UpdateData->FormTitle           = 0;
    UpdateData->DataCount           = 0;
    Location = (UINT8 *) &UpdateData->Data;
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
    Status = Private->Hii->NewString (Private->Hii, NULL, Private->RegisteredHandle, &NewStringToken, NewString);
    ASSERT_EFI_ERROR (Status);
    gBS->FreePool (NewString);
    //
    // Save the device path string toke for next access use
    //
    mControllerToken[Index] = NewStringToken;
    
    CreateGotoOpCode (
      0x1200,
      NewStringToken,               // Description String Token
      STR_GOTO_HELP_DRIVER,         // Description Help String Token
      EFI_IFR_FLAG_INTERACTIVE,     // Flag designating callback is active
      (UINT16) Index + 0x100, // Callback key value
      Location                      // Buffer to fill with op-code
      );
    //
    // Update the buffer items number and adjust next item address to new one
    //
    UpdateData->DataCount +=1 ;
    Location = Location + ((EFI_IFR_OP_HEADER *) Location)->Length;  
    //
    // Update first page form
    //
    Private->Hii->UpdateForm (
                    Private->Hii,
                    Private->RegisteredHandle,
                    (EFI_FORM_LABEL) 0x1234,
                    TRUE,
                    UpdateData
                    );
    
  } 
    
  gBS->FreePool (UpdateData);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UpdateBindingDriverSelectPage (
  IN EFI_CALLBACK_INFO                *Private,
  IN UINT16                           KeyValue,
  IN EFI_IFR_DATA_ARRAY               *Data
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
  EFI_HII_UPDATE_DATA                       *UpdateData;
  EFI_STATUS                                Status;
  UINTN                                     Index;
  UINT8                                     *Location;

  CHAR16                                    *NewString;
  STRING_REF                                NewStringToken;
  STRING_REF                                NewStringHelpToken;
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

  
  mCurrentPage = 0x02;
  //    
  // Switch the item callback key value to its NO. in mDevicePathHandleBuffer
  //
  mSelectedCtrIndex = KeyValue - 0x100;    
  ASSERT (0 <= mSelectedCtrIndex < MAX_CHOICE_NUM);
  mLastSavedDriverImageNum = 0;
  //
  // Clear all the content in dynamic page
  //
  UpdateData = NULL;
  UpdateData = EfiLibAllocateZeroPool (UPDATE_DATA_SIZE);
  ASSERT (UpdateData != NULL);

  UpdateData->FormSetUpdate       = FALSE;
  UpdateData->FormCallbackHandle  = 0;
  UpdateData->FormUpdate          = FALSE;
  UpdateData->FormTitle           = 0;
  UpdateData->DataCount           = 0xff;
  UpdateData->Data                = NULL;

  Private->Hii->UpdateForm (
                  Private->Hii,
                  Private->RegisteredHandle,
                  (EFI_FORM_LABEL) 0x1200,  // Label 0x1234
                  FALSE,                    // Remove Op-codes (will never remove form/endform)
                  UpdateData                // Significant value is UpdateData->DataCount
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
      ((MyIfrNVData *) Data->NvRamMap)->DriSelection[Index] = 0;
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
      ((MyIfrNVData *) Data->NvRamMap)->DriSelection[Index] = 0;
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
      ((MyIfrNVData *) Data->NvRamMap)->DriSelection[Index] = 0;
      continue;
    }
    
    if (((MyIfrNVData *) Data->NvRamMap)->PciDeviceFilter == 0x01) {
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
          ((MyIfrNVData *) Data->NvRamMap)->DriSelection[Index] = 0;
          continue;
        }  
      } else {
        ((MyIfrNVData *) Data->NvRamMap)->DriSelection[Index] = 0;
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
      DriverName = GetString (Private, (STRING_REF) STR_DRIVER_DEFAULT_NAME);
      ASSERT (DriverName != NULL);
      FreeDriverName = TRUE;  // the DriverName string need to free pool
    }
   
    
    //
    // Step2 Export the driver name string and create check box item in second page
    //
    
    //
    // Clear the Update buffer
    //
    UpdateData->FormSetUpdate       = FALSE;
    UpdateData->FormCallbackHandle  = 0;
    UpdateData->FormUpdate          = FALSE;
    UpdateData->FormTitle           = 0;
    UpdateData->DataCount           = 0;
    Location = (UINT8 *) &UpdateData->Data;
    //
    // First create the driver image name      
    //
    NewString = EfiLibAllocateZeroPool (EfiStrSize (DriverName));
    if (EFI_ERROR (LibCheckMapping (mControllerDevicePathProtocol[mSelectedCtrIndex], TatalFilePath, &mMappingDataBase, NULL, NULL))) {
      ((MyIfrNVData *) Data->NvRamMap)->DriSelection[Index] = 0x00;            
    } else {
      ((MyIfrNVData *) Data->NvRamMap)->DriSelection[Index] = 0x01;
      mLastSavedDriverImageNum++;
    }
    EfiStrCat (NewString, DriverName);
    NewStringToken = mDriverImageToken[Index];
    Status = Private->Hii->NewString (Private->Hii, NULL, Private->RegisteredHandle, &NewStringToken, NewString);
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
    Status = Private->Hii->NewString (Private->Hii, NULL, Private->RegisteredHandle, &NewStringHelpToken, NewString);
    mDriverImageFilePathToken[Index] = NewStringHelpToken;
    ASSERT_EFI_ERROR (Status);
    gBS->FreePool (NewString);
    gBS->FreePool (DriverName);
    
    CreateCheckBoxOpCode (
      (UINT16) (DRIVER_SELECTION_QUESTION_ID + Index),
      (UINT8) 1,
      NewStringToken,                               // Description String Token
      NewStringHelpToken,                           // Description Help String Token
      EFI_IFR_FLAG_INTERACTIVE,                     // Flag designating callback is active
      (UINT16) Index + 0x500,                       // Callback key value
      Location                                      // Buffer to fill with op-code
      );

    UpdateData->DataCount +=1 ;
    Location = Location + ((EFI_IFR_OP_HEADER *) Location)->Length;

    //
    // Update second page form
    //
    Private->Hii->UpdateForm (
                    Private->Hii,
                    Private->RegisteredHandle,
                    (EFI_FORM_LABEL) 0x1200,
                    TRUE,
                    UpdateData
                    );
  }
  gBS->FreePool (UpdateData);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UpdatePrioritySelectPage (
  IN EFI_CALLBACK_INFO                *Private,
  IN UINT16                           KeyValue,
  IN EFI_IFR_DATA_ARRAY               *Data
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
  EFI_HII_UPDATE_DATA                       *UpdateData;
  EFI_STATUS                                Status;
  UINTN                                     Index;
  UINT8                                     *Location;

  
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
  mCurrentPage = 0x03;
  
  UpdateData = NULL;
  UpdateData = EfiLibAllocateZeroPool (UPDATE_DATA_SIZE);
  ASSERT (UpdateData != NULL);
  //
  // Clear all the content in dynamic page
  //
  UpdateData->FormSetUpdate       = FALSE;
  UpdateData->FormCallbackHandle  = 0;
  UpdateData->FormUpdate          = FALSE;
  UpdateData->FormTitle           = 0;
  UpdateData->DataCount           = 0xff;
  UpdateData->Data                = NULL;

  Private->Hii->UpdateForm (
                  Private->Hii,
                  Private->RegisteredHandle,
                  (EFI_FORM_LABEL) 0x1500,  
                  FALSE,                    // Remove Op-codes (will never remove form/endform)
                  UpdateData                // Significant value is UpdateData->DataCount
                  );

  //
  // Clear the Update buffer  
  //
  UpdateData->FormSetUpdate       = FALSE;
  UpdateData->FormCallbackHandle  = 0;
  UpdateData->FormUpdate          = FALSE;
  UpdateData->FormTitle           = 0;
  UpdateData->DataCount           = 0;
  Location = (UINT8 *) &UpdateData->Data;
  //
  // Check how many drivers have been selected
  //
  SelectedDriverImageNum = 0;
  for (Index = 0; Index < mDriverImageHandleCount; Index++) {
    if (((MyIfrNVData *) Data->NvRamMap)->DriSelection[Index] != 0) {
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
    if (((MyIfrNVData *) Data->NvRamMap)->DriSelection[Index] != 0) {
      IfrOptionList[SelectedDriverImageNum].StringToken = mDriverImageToken[Index];
      //
      // Use the NO. in driver binding buffer as value, will use it later
      //
      IfrOptionList[SelectedDriverImageNum].Value = (UINT16) Index + 1; 
      IfrOptionList[SelectedDriverImageNum].OptionString = NULL;
      IfrOptionList[SelectedDriverImageNum].Flags       = EFI_IFR_FLAG_INTERACTIVE|EFI_IFR_FLAG_RESET_REQUIRED;
      IfrOptionList[SelectedDriverImageNum].Key  = (UINT16) (0x500 + Index);

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
  for (Index=0; Index < 100; Index++) {
    ((MyIfrNVData *) Data->NvRamMap)->DriOrder[Index] = 0;
  }
  
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
    ((MyIfrNVData *) Data->NvRamMap)->DriOrder[Index] =(UINT8) IfrOptionList[MinNO].Value;
    TempNO[MinNO] = 101;
  }
  
  CreateOrderedListOpCode (
    (UINT16) DRIVER_ORDER_QUESTION_ID,
    (UINT8) 100,
    mControllerToken[mSelectedCtrIndex],
    mControllerToken[mSelectedCtrIndex],
    IfrOptionList,
    SelectedDriverImageNum,
    Location
    );

  for (Index = 0; Index < SelectedDriverImageNum + 2; Index++) {
    Location = Location + ((EFI_IFR_OP_HEADER *) Location)->Length;
  }
  
  UpdateData->DataCount = (UINT16) (UpdateData->DataCount + SelectedDriverImageNum + 2);
  
  //
  // Update third page form
  //
  Private->Hii->UpdateForm (
                  Private->Hii,
                  Private->RegisteredHandle,
                  (EFI_FORM_LABEL) 0x1500,
                  TRUE,
                  UpdateData
                  );
                  
  gBS->FreePool (IfrOptionList);
  gBS->FreePool (UpdateData);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CommintChanges (
  IN EFI_CALLBACK_INFO                *Private,
  IN UINT16                           KeyValue,
  IN EFI_IFR_DATA_ARRAY               *Data
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
    Index = ((MyIfrNVData *) Data->NvRamMap)->DriOrder[SelectedDriverImageNum] - 1;
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
PlatOverMngrCallback (
  IN EFI_FORM_CALLBACK_PROTOCOL       *This,
  IN UINT16                           KeyValue,
  IN EFI_IFR_DATA_ARRAY               *Data,
  OUT EFI_HII_CALLBACK_PACKET         **Packet
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
  
  Data -         EFI_IFR_DATA_ARRAY data
  Packet-       No use here.

  Returns -    Always successful

--*/
{
  EFI_CALLBACK_INFO                         *Private;
  EFI_STATUS                                Status;
  STRING_REF                                NewStringToken;
  
  Private     = EFI_CALLBACK_INFO_FROM_THIS (This);

  if (KeyValue == 0x1234 || KeyValue == 0x1235) {
    UpdateDeviceSelectPage (Private, KeyValue, Data);
    //
    // Update page title string 
    //
    NewStringToken = (STRING_REF) STR_TITLE;
    Status = Private->Hii->NewString (Private->Hii, mLang, Private->RegisteredHandle, &NewStringToken, L"First, Select the controller by device path");
    ASSERT_EFI_ERROR (Status); 
  }
  
  if ((0x100 <= KeyValue) && (KeyValue < 0x500) || (KeyValue == 0x2000)) {
    if (KeyValue == 0x2000) {
      KeyValue = (UINT16)mSelectedCtrIndex + 0x100;
    }
    UpdateBindingDriverSelectPage (Private, KeyValue, Data);      
    //
    // Update page title string 
    //
    NewStringToken = (STRING_REF) STR_TITLE;
    Status = Private->Hii->NewString (Private->Hii, mLang, Private->RegisteredHandle, &NewStringToken, L"Second, Select drivers for the previous selected controller");
    ASSERT_EFI_ERROR (Status); 
  }

  
  if (KeyValue == 0x1500) {
    UpdatePrioritySelectPage (Private, KeyValue, Data);
    //
    // Update page title string 
    //
    NewStringToken = (STRING_REF) STR_TITLE;
    Status = Private->Hii->NewString (Private->Hii, mLang, Private->RegisteredHandle, &NewStringToken, L"Finally, Set the priority order for the drivers and save them");
    ASSERT_EFI_ERROR (Status); 
  }
  
  if (KeyValue == 0x1800) {
    Status = CommintChanges (Private, KeyValue, Data);
    if (EFI_ERROR (Status)) {
      *Packet = EfiLibAllocateZeroPool (sizeof (EFI_HII_CALLBACK_PACKET) + sizeof ( L"Single Override Info too large, Saving Error!") + 2);
      EfiStrCpy ((*Packet)->String,  L"Single Override Info too large, Saving Error!");
      return EFI_DEVICE_ERROR;
    }
  }
  
  if (KeyValue == 0x1236) {
    //
    // Deletes all environment variable(s) that contain the override mappings info
    //
    LibFreeMappingDatabase (&mMappingDataBase);
    Status = LibSaveOverridesMapping (&mMappingDataBase);    
    UpdateDeviceSelectPage (Private, KeyValue, Data);
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PlatOverMngrNvWrite (
  IN      EFI_FORM_CALLBACK_PROTOCOL       *This,
  IN      CHAR16                           *VariableName,
  IN      EFI_GUID                         *VendorGuid,
  IN      UINT32                           Attributes,
  IN      UINTN                            DataSize,
  IN      VOID                             *Buffer,
  OUT     BOOLEAN                          *ResetRequired
  )
/*++

  Routine Description:
    This callback NV write worker function is to prevent user enter F10 to save 'Setup' variable during the Ui menu.

  Arguments:


  Returns:
    EFI_SUCCESS

--*/
{
  EFI_CALLBACK_INFO                         *Private;
  EFI_IFR_DATA_ARRAY                        Data;
  UINT16                                    KeyValue;
  
  Private     = EFI_CALLBACK_INFO_FROM_THIS (This);
  Data.NvRamMap = Buffer;
  
  if (mCurrentPage == 0x02) {
    KeyValue = 0x1500;
    UpdatePrioritySelectPage (Private, KeyValue, &Data);  
    KeyValue = 0x1800;
    CommintChanges (Private, KeyValue, &Data);
    //
    // Since UpdatePrioritySelectPage will change mCurrentPage,  
    // should ensure the mCurrentPage still indicate the second page here
    //
    mCurrentPage = 0x02;
  }

  if (mCurrentPage == 0x03) {
    KeyValue = 0x1800;
    CommintChanges (Private, KeyValue, &Data);
  }
  return EFI_SUCCESS;
}
