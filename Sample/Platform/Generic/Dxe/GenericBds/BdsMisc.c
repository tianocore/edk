/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  BdsMisc.c

Abstract:

  Misc BDS library function

--*/

#include "BdsLib.h"

#include EFI_PROTOCOL_DEFINITION (FormBrowser)

#define MAX_STRING_LEN        200

static BOOLEAN   mFeaturerSwitch = TRUE;
static BOOLEAN   mResetRequired = FALSE;

extern UINT16 gPlatformBootTimeOutDefault;

UINT16
BdsLibGetTimeout (
  VOID
  )
/*++

Routine Description:
  
  Return the default value for system Timeout variable.

Arguments:

  None

Returns:
  
  Timeout value.

--*/
{
  UINT16      Timeout;
  UINTN       Size;
  EFI_STATUS  Status;

  //
  // Return Timeout variable or 0xffff if no valid
  // Timeout variable exists.
  //
  Size    = sizeof (UINT16);
  Status  = gRT->GetVariable (L"Timeout", &gEfiGlobalVariableGuid, NULL, &Size, &Timeout);
  if (!EFI_ERROR (Status)) {
    return Timeout;
  }
  //
  // To make the current EFI Automatic-Test activity possible, just add
  // following code to make AutoBoot enabled when this variable is not
  // present.
  // This code should be removed later.
  //
  Timeout = gPlatformBootTimeOutDefault;

  //
  // Notes: Platform should set default variable if non exists on all error cases!!!
  //
  Status = gRT->SetVariable (
                  L"Timeout",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  sizeof (UINT16),
                  &Timeout
                  );
  return Timeout;
}

VOID
BdsLibLoadDrivers (
  IN EFI_LIST_ENTRY               *BdsDriverLists
  )
/*++

Routine Description:
  
  The function will go through the driver optoin link list, load and start
  every driver the driver optoin device path point to.

Arguments:

  BdsDriverLists   - The header of the current driver option link list

Returns:
  
  None

--*/
{
  EFI_STATUS                Status;
  EFI_LIST_ENTRY            *Link;
  BDS_COMMON_OPTION         *Option;
  EFI_HANDLE                ImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL *ImageInfo;
  UINTN                     ExitDataSize;
  CHAR16                    *ExitData;
  BOOLEAN                   ReconnectAll;

  ReconnectAll = FALSE;

  //
  // Process the driver option
  //
  for (Link = BdsDriverLists->ForwardLink; Link != BdsDriverLists; Link = Link->ForwardLink) {
    Option = CR (Link, BDS_COMMON_OPTION, Link, BDS_LOAD_OPTION_SIGNATURE);
    //
    // If a load option is not marked as LOAD_OPTION_ACTIVE,
    // the boot manager will not automatically load the option.
    //
    if (!IS_LOAD_OPTION_TYPE (Option->Attribute, LOAD_OPTION_ACTIVE)) {
      continue;
    }
    //
    // If a driver load option is marked as LOAD_OPTION_FORCE_RECONNECT,
    // then all of the EFI drivers in the system will be disconnected and
    // reconnected after the last driver load option is processed.
    //
    if (IS_LOAD_OPTION_TYPE (Option->Attribute, LOAD_OPTION_FORCE_RECONNECT)) {
      ReconnectAll = TRUE;
    }
    //
    // Make sure the driver path is connected.
    //
    BdsLibConnectDevicePath (Option->DevicePath);

    //
    // Load and start the image that Driver#### describes
    //
    Status = gBS->LoadImage (
                    FALSE,
                    mBdsImageHandle,
                    Option->DevicePath,
                    NULL,
                    0,
                    &ImageHandle
                    );

    if (!EFI_ERROR (Status)) {
      gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, &ImageInfo);

      //
      // Verify whether this image is a driver, if not,
      // exit it and continue to parse next load option
      //
      if (ImageInfo->ImageCodeType != EfiBootServicesCode && ImageInfo->ImageCodeType != EfiRuntimeServicesCode) {
        gBS->Exit (ImageHandle, EFI_INVALID_PARAMETER, 0, NULL);
        continue;
      }

      if (Option->LoadOptionsSize != 0) {
        ImageInfo->LoadOptionsSize  = Option->LoadOptionsSize;
        ImageInfo->LoadOptions      = Option->LoadOptions;
      }
      //
      // Before calling the image, enable the Watchdog Timer for
      // the 5 Minute period
      //
      gBS->SetWatchdogTimer (5 * 60, 0x0000, 0x00, NULL);

      Status = gBS->StartImage (ImageHandle, &ExitDataSize, &ExitData);
      DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Driver Return Status = %r\n", Status));

      //
      // Clear the Watchdog Timer after the image returns
      //
      gBS->SetWatchdogTimer (0x0000, 0x0000, 0x0000, NULL);
    }
  }
  //
  // Process the LOAD_OPTION_FORCE_RECONNECT driver option
  //
  if (ReconnectAll) {
    BdsLibDisconnectAllEfi ();
    BdsLibConnectAll ();
  }

}

UINT16
BdsLibGetFreeOptionNumber (
  IN  CHAR16    *VariableName
  )
/*++

Routine Description:
  Get the Option Number that does not used 
  Try to locate the specific option variable one by one untile find a free number
  
Arguments:
  VariableName - Indicate if the boot#### or driver#### option
  
Returns:
  The Minimal Free Option Number
  
--*/
{
  UINT16        Number;
  UINTN         Index;
  CHAR16        StrTemp[10];
  UINT16        *OptionBuffer;
  UINTN         OptionSize;

  //
  // Try to find the minimum free number from 0, 1, 2, 3....
  //
  Index = 0;
  do {
    if (*VariableName == 'B') {
      SPrint (StrTemp, sizeof (StrTemp), L"Boot%04x", Index);      
    } else {
      SPrint (StrTemp, sizeof (StrTemp), L"Driver%04x", Index);  
    }
    //
    // try if the option number is used
    //
    OptionBuffer = BdsLibGetVariableAndSize (
              StrTemp,
              &gEfiGlobalVariableGuid,
              &OptionSize
              );
    if (OptionBuffer == NULL) {
      break;
    }
    Index++;
  } while (1);
  
  Number = (UINT16) Index;
  return Number;
}

EFI_STATUS
BdsLibRegisterNewOption (
  IN  EFI_LIST_ENTRY                 *BdsOptionList,
  IN  EFI_DEVICE_PATH_PROTOCOL       *DevicePath,
  IN  CHAR16                         *String,
  IN  CHAR16                         *VariableName
  )
/*++

Routine Description:
  
  This function will register the new boot#### or driver#### option base on
  the VariableName. The new registered boot#### or driver#### will be linked
  to BdsOptionList and also update to the VariableName. After the boot#### or
  driver#### updated, the BootOrder or DriverOrder will also be updated.

Arguments:

  BdsOptionList    - The header of the boot#### or driver#### link list
  
  DevicePath       - The device path which the boot####
                     or driver#### option present
                     
  String           - The description of the boot#### or driver####
  
  VariableName     - Indicate if the boot#### or driver#### option

Returns:
  
  EFI_SUCCESS      - The boot#### or driver#### have been success registered
  
  EFI_STATUS       - Return the status of gRT->SetVariable ().

--*/
{
  EFI_STATUS                Status;
  UINTN                     Index;
  UINT16                    MaxOptionNumber;
  UINT16                    RegisterOptionNumber;
  UINT16                    *TempOptionPtr;
  UINTN                     TempOptionSize;
  UINT16                    *OptionOrderPtr;
  VOID                      *OptionPtr;
  UINTN                     OptionSize;
  UINT8                     *TempPtr;
  EFI_DEVICE_PATH_PROTOCOL  *OptionDevicePath;
  CHAR16                    *Description;
  CHAR16                    OptionName[10];
  BOOLEAN                   UpdateBootDevicePath;
  UINT16                    BootOrderEntry; 
  UINTN                     OrderItemNum;
  

  OptionPtr             = NULL;
  OptionSize            = 0;
  TempPtr               = NULL;
  OptionDevicePath      = NULL;
  Description           = NULL;
  MaxOptionNumber       = 0;
  OptionOrderPtr        = NULL;
  UpdateBootDevicePath  = FALSE;
  EfiZeroMem (OptionName, sizeof (OptionName));

  TempOptionSize = 0;
  TempOptionPtr = BdsLibGetVariableAndSize (
                    VariableName,
                    &gEfiGlobalVariableGuid,
                    &TempOptionSize
                    );

  //
  // Compare with current option variable
  //
  for (Index = 0; Index < TempOptionSize / sizeof (UINT16); Index++) {

    if (*VariableName == 'B') {
      SPrint (OptionName, sizeof (OptionName), L"Boot%04x", TempOptionPtr[Index]);
    } else {
      SPrint (OptionName, sizeof (OptionName), L"Driver%04x", TempOptionPtr[Index]);
    }

    OptionPtr = BdsLibGetVariableAndSize (
                  OptionName,
                  &gEfiGlobalVariableGuid,
                  &OptionSize
                  );
    if (OptionPtr == NULL) {
      continue;
    }
    TempPtr = OptionPtr;
    TempPtr += sizeof (UINT32) + sizeof (UINT16);
    Description = (CHAR16 *) TempPtr;
    TempPtr += EfiStrSize ((CHAR16 *) TempPtr);
    OptionDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) TempPtr;

    //
    // Notes: the description may will change base on the GetStringToken
    //
    if (EfiCompareMem (Description, String, EfiStrSize (Description)) == 0) {
      if (EfiCompareMem (OptionDevicePath, DevicePath, EfiDevicePathSize (OptionDevicePath)) == 0) {
        //
        // Got the option, so just return
        //
        gBS->FreePool (OptionPtr);
        gBS->FreePool (TempOptionPtr);
        return EFI_SUCCESS;
      } else {
        //
        // Boot device path changed, need update.
        //
        UpdateBootDevicePath = TRUE;
        break;
      }
    }

    gBS->FreePool (OptionPtr);
  }

  OptionSize          = sizeof (UINT32) + sizeof (UINT16) + EfiStrSize (String) + EfiDevicePathSize (DevicePath);
  OptionPtr           = EfiLibAllocateZeroPool (OptionSize);
  TempPtr             = OptionPtr;
  *(UINT32 *) TempPtr = LOAD_OPTION_ACTIVE;
  TempPtr += sizeof (UINT32);
  *(UINT16 *) TempPtr = (UINT16) EfiDevicePathSize (DevicePath);
  TempPtr += sizeof (UINT16);
  EfiCopyMem (TempPtr, String, EfiStrSize (String));
  TempPtr += EfiStrSize (String);
  EfiCopyMem (TempPtr, DevicePath, EfiDevicePathSize (DevicePath));

  if (UpdateBootDevicePath) {
    //
    // The number in option#### to be updated
    //
    RegisterOptionNumber = TempOptionPtr[Index];
  } else {
    //
    // The new option#### number
    //
    RegisterOptionNumber = BdsLibGetFreeOptionNumber(VariableName);
  }
  
  if (*VariableName == 'B') {
    SPrint (OptionName, sizeof (OptionName), L"Boot%04x", RegisterOptionNumber);
  } else {
    SPrint (OptionName, sizeof (OptionName), L"Driver%04x", RegisterOptionNumber);
  }

  Status = gRT->SetVariable (
                  OptionName,
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  OptionSize,
                  OptionPtr
                  );
  if (EFI_ERROR (Status) || UpdateBootDevicePath) {
    gBS->FreePool (OptionPtr);
    gBS->FreePool (TempOptionPtr);
    return Status;
  }

  gBS->FreePool (OptionPtr);

  //
  // Update the option order variable
  //
  
  //
  // If no BootOrder
  //
  if (TempOptionSize == 0) {
    BootOrderEntry = 0;
    Status = gRT->SetVariable (
                    VariableName,
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    sizeof (UINT16),
                    &BootOrderEntry
                    );
    if (EFI_ERROR (Status)) {
      gBS->FreePool (TempOptionPtr);
      return Status;
    }
    return EFI_SUCCESS;
  }  

  if (UpdateBootDevicePath) {
    //
    // If just update a old option, the new optionorder size not change
    //  
    OrderItemNum = (TempOptionSize / sizeof (UINT16)) ;
    OptionOrderPtr = EfiLibAllocateZeroPool ( OrderItemNum * sizeof (UINT16));
    EfiCopyMem (OptionOrderPtr, TempOptionPtr, OrderItemNum * sizeof (UINT16));
  } else {
    OrderItemNum = (TempOptionSize / sizeof (UINT16)) + 1 ;
    OptionOrderPtr = EfiLibAllocateZeroPool ( OrderItemNum * sizeof (UINT16));
    EfiCopyMem (OptionOrderPtr, TempOptionPtr, (OrderItemNum - 1) * sizeof (UINT16));
  }

  OptionOrderPtr[Index] = RegisterOptionNumber; 
   
  Status = gRT->SetVariable (
                  VariableName,
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  OrderItemNum * sizeof (UINT16),
                  OptionOrderPtr
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (TempOptionPtr);
    gBS->FreePool (OptionOrderPtr);
    return Status;
  }

  gBS->FreePool (TempOptionPtr);
  gBS->FreePool (OptionOrderPtr);

  return EFI_SUCCESS;
}

BDS_COMMON_OPTION *
BdsLibVariableToOption (
  IN OUT EFI_LIST_ENTRY               *BdsCommonOptionList,
  IN  CHAR16                          *VariableName
  )
/*++

Routine Description:

  Build the boot#### or driver#### option from the VariableName, the 
  build boot#### or driver#### will also be linked to BdsCommonOptionList
  
Arguments:

  BdsCommonOptionList - The header of the boot#### or driver#### option link list

  VariableName - EFI Variable name indicate if it is boot#### or driver####

Returns:

  BDS_COMMON_OPTION    - Get the option just been created

  NULL                 - Failed to get the new option

--*/
{
  UINT32                    Attribute;
  UINT16                    FilePathSize;
  UINT8                     *Variable;
  UINT8                     *TempPtr;
  UINTN                     VariableSize;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  BDS_COMMON_OPTION         *Option;
  VOID                      *LoadOptions;
  UINT32                    LoadOptionsSize;
  CHAR16                    *Description;

  //
  // Read the variable. We will never free this data.
  //
  Variable = BdsLibGetVariableAndSize (
              VariableName,
              &gEfiGlobalVariableGuid,
              &VariableSize
              );
  if (Variable == NULL) {
    return NULL;
  }
  //
  // Notes: careful defined the variable of Boot#### or
  // Driver####, consider use some macro to abstract the code
  //
  //
  // Get the option attribute
  //
  TempPtr   = Variable;
  Attribute = *(UINT32 *) Variable;
  TempPtr += sizeof (UINT32);

  //
  // Get the option's device path size
  //
  FilePathSize = *(UINT16 *) TempPtr;
  TempPtr += sizeof (UINT16);

  //
  // Get the option's description string
  //
  Description = (CHAR16 *) TempPtr;

  //
  // Get the option's description string size
  //
  TempPtr += EfiStrSize ((CHAR16 *) TempPtr);

  //
  // Get the option's device path
  //
  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) TempPtr;
  TempPtr += FilePathSize;

  LoadOptions     = TempPtr;
  LoadOptionsSize = (UINT32) (VariableSize - (UINTN) (TempPtr - Variable));

  //
  // The Console variables may have multiple device paths, so make
  // an Entry for each one.
  //
  Option = EfiLibAllocateZeroPool (sizeof (BDS_COMMON_OPTION));
  if (Option == NULL) {
    return NULL;
  }

  Option->Signature   = BDS_LOAD_OPTION_SIGNATURE;
  Option->DevicePath  = EfiLibAllocateZeroPool (EfiDevicePathSize (DevicePath));
  EfiCopyMem (Option->DevicePath, DevicePath, EfiDevicePathSize (DevicePath));
  Option->Attribute   = Attribute;
  Option->Description = EfiLibAllocateZeroPool (EfiStrSize (Description));
  EfiCopyMem (Option->Description, Description, EfiStrSize (Description));
  Option->LoadOptions = EfiLibAllocateZeroPool (LoadOptionsSize);
  EfiCopyMem (Option->LoadOptions, LoadOptions, LoadOptionsSize);
  Option->LoadOptionsSize = LoadOptionsSize;

  //
  // Insert active entry to BdsDeviceList
  //
  if ((Option->Attribute & LOAD_OPTION_ACTIVE) == LOAD_OPTION_ACTIVE) {
    InsertTailList (BdsCommonOptionList, &Option->Link);
    gBS->FreePool (Variable);
    return Option;
  }

  gBS->FreePool (Variable);
  gBS->FreePool (Option);
  return NULL;

}

EFI_STATUS
BdsLibBuildOptionFromVar (
  IN  EFI_LIST_ENTRY                  *BdsCommonOptionList,
  IN  CHAR16                          *VariableName
  )
/*++

Routine Description:

  Process BootOrder, or DriverOrder variables, by calling
  BdsLibVariableToOption () for each UINT16 in the variables.

Arguments:

  BdsCommonOptionList - The header of the option list base on variable
                        VariableName

  VariableName - EFI Variable name indicate the BootOrder or DriverOrder

Returns:

  EFI_SUCCESS - Success create the boot option or driver option list

  EFI_OUT_OF_RESOURCES - Failed to get the boot option or driver option list

--*/
{
  UINT16            *OptionOrder;
  UINTN             OptionOrderSize;
  UINTN             Index;
  BDS_COMMON_OPTION *Option;
  CHAR16            OptionName[20];

  //
  // Zero Buffer in order to get all BOOT#### variables
  //
  EfiZeroMem (OptionName, sizeof (OptionName));

  //
  // Read the BootOrder, or DriverOrder variable.
  //
  OptionOrder = BdsLibGetVariableAndSize (
                  VariableName,
                  &gEfiGlobalVariableGuid,
                  &OptionOrderSize
                  );
  if (OptionOrder == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < OptionOrderSize / sizeof (UINT16); Index++) {
    if (*VariableName == 'B') {
      SPrint (OptionName, sizeof (OptionName), L"Boot%04x", OptionOrder[Index]);
    } else {
      SPrint (OptionName, sizeof (OptionName), L"Driver%04x", OptionOrder[Index]);
    }

    Option              = BdsLibVariableToOption (BdsCommonOptionList, OptionName);
    Option->BootCurrent = OptionOrder[Index];

  }

  gBS->FreePool (OptionOrder);

  return EFI_SUCCESS;
}

EFI_STATUS
BdsLibGetBootMode (
  OUT EFI_BOOT_MODE       *BootMode
  )
/*++

Routine Description:

  Get boot mode by looking up configuration table and parsing HOB list

Arguments:

  BootMode - Boot mode from PEI handoff HOB.

Returns:

  EFI_SUCCESS - Successfully get boot mode
  
  EFI_NOT_FOUND - Can not find the current system boot mode

--*/
{
  VOID        *HobList;
  EFI_STATUS  Status;

  //
  // Get Hob list
  //
  Status = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Hob list not found\n"));
    *BootMode = 0;
    return EFI_NOT_FOUND;
  }

  Status = GetHobBootMode (HobList, BootMode);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

VOID *
BdsLibGetVariableAndSize (
  IN  CHAR16              *Name,
  IN  EFI_GUID            *VendorGuid,
  OUT UINTN               *VariableSize
  )
/*++

Routine Description:

  Read the EFI variable (VendorGuid/Name) and return a dynamically allocated
  buffer, and the size of the buffer. If failure return NULL.

Arguments:

  Name       - String part of EFI variable name

  VendorGuid - GUID part of EFI variable name

  VariableSize - Returns the size of the EFI variable that was read

Returns:

  Dynamically allocated memory that contains a copy of the EFI variable.
  Caller is responsible freeing the buffer.

  NULL - Variable was not read

--*/
{
  EFI_STATUS  Status;
  UINTN       BufferSize;
  VOID        *Buffer;

  Buffer = NULL;

  //
  // Pass in a zero size buffer to find the required buffer size.
  //
  BufferSize  = 0;
  Status      = gRT->GetVariable (Name, VendorGuid, NULL, &BufferSize, Buffer);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    //
    // Allocate the buffer to return
    //
    Buffer = EfiLibAllocateZeroPool (BufferSize);
    if (Buffer == NULL) {
      return NULL;
    }
    //
    // Read variable into the allocated buffer.
    //
    Status = gRT->GetVariable (Name, VendorGuid, NULL, &BufferSize, Buffer);
    if (EFI_ERROR (Status)) {
      BufferSize = 0;
    }
  }

  *VariableSize = BufferSize;
  return Buffer;
}

EFI_DEVICE_PATH_PROTOCOL *
BdsLibDelPartMatchInstance (
  IN     EFI_DEVICE_PATH_PROTOCOL  *Multi,
  IN     EFI_DEVICE_PATH_PROTOCOL  *Single
  )
/*++

Routine Description:

  Delete the instance in Multi which matches partly with Single instance

Arguments:

  Multi        - A pointer to a multi-instance device path data structure.

  Single       - A pointer to a single-instance device path data structure.

Returns:

  This function will remove the device path instances in Multi which partly 
  match with the Single, and return the result device path. If there is no
  remaining device path as a result, this function will return NULL.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *Instance;
  EFI_DEVICE_PATH_PROTOCOL  *NewDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempNewDevicePath;
  UINTN                     InstanceSize;
  UINTN                     SingleDpSize;  
  UINTN                     Size; 
  
  NewDevicePath     = NULL;
  TempNewDevicePath = NULL;

  if (Multi == NULL || Single == NULL) {
    return Multi;
  }
  
  Instance        =  EfiDevicePathInstance (&Multi, &InstanceSize);
  SingleDpSize    =  EfiDevicePathSize (Single) - END_DEVICE_PATH_LENGTH;
  InstanceSize    -= END_DEVICE_PATH_LENGTH;

  while (Instance != NULL) {

    Size = (SingleDpSize < InstanceSize) ? SingleDpSize : InstanceSize;    
        
    if ((EfiCompareMem (Instance, Single, Size) != 0)) {
      //
      // Append the device path instance which does not match with Single
      //
      TempNewDevicePath = NewDevicePath;
      NewDevicePath = EfiAppendDevicePathInstance (NewDevicePath, Instance);
      EfiLibSafeFreePool(TempNewDevicePath);
    }
    EfiLibSafeFreePool(Instance);
    Instance = EfiDevicePathInstance (&Multi, &InstanceSize);
    InstanceSize  -= END_DEVICE_PATH_LENGTH;
  }
  
  return NewDevicePath;
}

BOOLEAN
BdsLibMatchDevicePaths (
  IN  EFI_DEVICE_PATH_PROTOCOL  *Multi,
  IN  EFI_DEVICE_PATH_PROTOCOL  *Single
  )
/*++

Routine Description:

  Function compares a device path data structure to that of all the nodes of a
  second device path instance.

Arguments:

  Multi        - A pointer to a multi-instance device path data structure.

  Single       - A pointer to a single-instance device path data structure.

Returns:

  TRUE   - If the Single is contained within Multi
  
  FALSE  - The Single is not match within Multi
  

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathInst;
  UINTN                     Size;

  if (!Multi || !Single) {
    return FALSE;
  }

  DevicePath      = Multi;
  DevicePathInst  = EfiDevicePathInstance (&DevicePath, &Size);

  //
  // Search for the match of 'Single' in 'Multi'
  //
  while (DevicePathInst != NULL) {
    //
    // If the single device path is found in multiple device paths,
    // return success
    //
    if (EfiCompareMem (Single, DevicePathInst, Size) == 0) {
      gBS->FreePool (DevicePathInst);
      return TRUE;
    }

    gBS->FreePool (DevicePathInst);
    DevicePathInst = EfiDevicePathInstance (&DevicePath, &Size);
  }

  return FALSE;
}

EFI_STATUS
BdsLibOutputStrings (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL   *ConOut,
  ...
  )
/*++

Routine Description:

  This function prints a series of strings.

Arguments:

  ConOut               - Pointer to EFI_SIMPLE_TEXT_OUT_PROTOCOL

  ...                  - A variable argument list containing series of strings,
                         the last string must be NULL.

Returns:

  EFI_SUCCESS          - Success print out the string using ConOut.
  
  EFI_STATUS           - Return the status of the ConOut->OutputString ().

--*/
{
  VA_LIST     args;
  EFI_STATUS  Status;
  CHAR16      *String;

  Status = EFI_SUCCESS;
  VA_START (args, ConOut);

  while (!EFI_ERROR (Status)) {
    //
    // If String is NULL, then it's the end of the list
    //
    String = VA_ARG (args, CHAR16 *);
    if (!String) {
      break;
    }

    Status = ConOut->OutputString (ConOut, String);

    if (EFI_ERROR (Status)) {
      break;
    }
  }

  return Status;
}

//
//  Following are BDS Lib functions which  contain all the code about setup browser reset reminder feature.
//  Setup Browser reset reminder feature is that an reset reminder will be given before user leaves the setup browser  if 
//  user change any option setting which needs a reset to be effective, and  the reset will be applied according to  the user selection.
//

VOID
EnableResetReminderFeature (
  VOID
  )
/*++

Routine Description:
 
  Enable the setup browser reset reminder feature.
  This routine is used in platform tip. If the platform policy need the feature, use the routine to enable it.

Arguments:

  VOID

Returns:

  VOID

--*/
{
  mFeaturerSwitch = TRUE;
} 

VOID
DisableResetReminderFeature (
  VOID
  )
/*++

Routine Description:
 
  Disable the setup browser reset reminder feature.
  This routine is used in platform tip. If the platform policy do not want the feature, use the routine to disable it.
  
Arguments:

  VOID

Returns:

  VOID

--*/
{
  mFeaturerSwitch = FALSE;
} 

VOID
EnableResetRequired (
  VOID
  )
/*++

Routine Description:
 
   Record the info that  a reset is required.
   A  module boolean variable is used to record whether a reset is required. 
  
Arguments:

  VOID

Returns:

  VOID

--*/
{
  mResetRequired = TRUE;
} 

VOID
DisableResetRequired (
  VOID
  )
/*++

Routine Description:

   Record the info that  no reset is required.
   A  module boolean variable is used to record whether a reset is required. 

Arguments:

  VOID

Returns:

  VOID

--*/
{
  mResetRequired = FALSE;
} 

BOOLEAN
IsResetReminderFeatureEnable (
  VOID
  )
/*++

Routine Description:
 
  Check whether platform policy enable the reset reminder feature. The default is enabled.

Arguments:

  VOID

Returns:

  VOID

--*/
{
  return mFeaturerSwitch;
}

BOOLEAN
IsResetRequired (
  VOID
  )
/*++

Routine Description:
 
  Check if  user changed any option setting which needs a system reset to be effective.
  
Arguments:

  VOID

Returns:

  VOID

--*/
{
  return mResetRequired;
}

VOID
SetupResetReminder (
  VOID
  )
/*++

Routine Description:
 
  Check whether a reset is needed, and finish the reset reminder feature.
  If a reset is needed, Popup a menu to notice user, and finish the feature 
  according to the user selection.

Arguments:

  VOID

Returns:

  VOID

--*/
{
  EFI_STATUS                    Status;
  EFI_FORM_BROWSER_PROTOCOL     *Browser;
  EFI_INPUT_KEY                 Key;  
  CHAR16                        *StringBuffer1;
  CHAR16                        *StringBuffer2;    


  //
  //check any reset required change is applied? if yes, reset system
  //
  if (IsResetReminderFeatureEnable ()) {
    if (IsResetRequired ()) {
    
      Status = gBS->LocateProtocol (
                      &gEfiFormBrowserProtocolGuid,
                      NULL,
                      &Browser
                      );      
                      
      StringBuffer1 = EfiLibAllocateZeroPool (MAX_STRING_LEN * sizeof (CHAR16));
      ASSERT (StringBuffer1 != NULL);
      StringBuffer2 = EfiLibAllocateZeroPool (MAX_STRING_LEN * sizeof (CHAR16));
      ASSERT (StringBuffer2 != NULL);      
      EfiStrCpy (StringBuffer1, L"Configuration changed. Reset to apply it Now ? ");
      EfiStrCpy (StringBuffer2, L"Enter (YES)  /   Esc (NO)");      
      //
      // Popup a menu to notice user
      // 
      do {
        Browser->CreatePopUp (2, TRUE, 0, NULL, &Key, StringBuffer1, StringBuffer2);
      } while ((Key.ScanCode != SCAN_ESC) && (Key.UnicodeChar != CHAR_CARRIAGE_RETURN)); 
      
      gBS->FreePool (StringBuffer1);      
      gBS->FreePool (StringBuffer2);            
      //
      // If the user hits the YES Response key, reset
      //
      if ((Key.UnicodeChar == CHAR_CARRIAGE_RETURN)) {
        gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
      }
      gST->ConOut->ClearScreen (gST->ConOut);
    } 
  } 
}

EFI_STATUS
BdsLibGetImageHeader (
  IN  EFI_HANDLE                  Device,
  IN  CHAR16                      *FileName,
  OUT EFI_IMAGE_DOS_HEADER        *DosHeader,
  OUT EFI_IMAGE_FILE_HEADER       *ImageHeader,
  OUT EFI_IMAGE_OPTIONAL_HEADER   *OptionalHeader
  )
/*++

Routine Description:

  Get the headers (dos, image, optional header) from an image

Arguments:
  Device           - SimpleFileSystem device handle
  FileName         - File name for the image
  DosHeader        - Pointer to dos header
  ImageHeader      - Pointer to image header
  OptionalHeader   - Pointer to optional header

Returns:
  EFI_SUCCESS           - Successfully get the machine type.
  EFI_NOT_FOUND         - The file is not found.
  EFI_LOAD_ERROR        - File is not a valid image file.
  
--*/
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *Volume;
  EFI_FILE_HANDLE                  Root;
  EFI_FILE_HANDLE                  ThisFile;
  UINT32                           Signature;
  UINTN                            BufferSize;
  UINT64                           FileSize;
  EFI_FILE_INFO                    *Info;

  Root     = NULL;
  ThisFile = NULL;
  //
  // Handle the file system interface to the device
  //
  Status = gBS->HandleProtocol (
                  Device,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID *) &Volume
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = Volume->OpenVolume (
                     Volume,
                     &Root
                     );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = Root->Open (Root, &ThisFile, FileName, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Get file size
  //
  BufferSize  = SIZE_OF_EFI_FILE_INFO + 200;
  do {
    Info   = NULL;
    Status = gBS->AllocatePool (EfiBootServicesData, BufferSize, &Info);
    if (EFI_ERROR (Status)) {
      goto Done;
    }
    Status = ThisFile->GetInfo (
                         ThisFile,
                         &gEfiFileInfoGuid,
                         &BufferSize,
                         Info
                         );
    if (!EFI_ERROR (Status)) {
      break;
    }
    if (Status != EFI_BUFFER_TOO_SMALL) {
      goto Done;
    }
    gBS->FreePool (Info);
  } while (TRUE);

  FileSize = Info->FileSize;
  gBS->FreePool (Info);

  //
  // Read dos header
  //
  BufferSize = sizeof (EFI_IMAGE_DOS_HEADER);
  Status = ThisFile->Read (ThisFile, &BufferSize, DosHeader);
  if (EFI_ERROR (Status) ||
      BufferSize < sizeof (EFI_IMAGE_DOS_HEADER) ||
      FileSize <= DosHeader->e_lfanew ||
      DosHeader->e_magic != EFI_IMAGE_DOS_SIGNATURE) {
    Status = EFI_LOAD_ERROR;
    goto Done;
  }

  //
  // Move to PE signature
  //
  Status = ThisFile->SetPosition (ThisFile, DosHeader->e_lfanew);
  if (EFI_ERROR (Status)) {
    Status = EFI_LOAD_ERROR;
    goto Done;
  }

  //
  // Read and check PE signature
  //
  BufferSize = sizeof (Signature);
  Status = ThisFile->Read (ThisFile, &BufferSize, &Signature);
  if (EFI_ERROR (Status) ||
      BufferSize < sizeof (Signature) ||
      Signature != EFI_IMAGE_NT_SIGNATURE) {
    Status = EFI_LOAD_ERROR;
    goto Done;
  }

  //
  // Read image header
  //
  BufferSize = EFI_IMAGE_SIZEOF_FILE_HEADER;
  Status = ThisFile->Read (ThisFile, &BufferSize, ImageHeader);
  if (EFI_ERROR (Status) ||
      BufferSize < EFI_IMAGE_SIZEOF_FILE_HEADER) {
    Status = EFI_LOAD_ERROR;
    goto Done;
  }

  //
  // Read optional header
  //
  BufferSize = ImageHeader->SizeOfOptionalHeader;
  Status = ThisFile->Read (ThisFile, &BufferSize, OptionalHeader);
  if (EFI_ERROR (Status) ||
      BufferSize < ImageHeader->SizeOfOptionalHeader) {
    Status = EFI_LOAD_ERROR;
    goto Done;
  }

  //
  // Check PE32 or PE32+ magic
  //  
  if (OptionalHeader->Magic != EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
      OptionalHeader->Magic != EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
    Status = EFI_LOAD_ERROR;
    goto Done;
  }

 Done:
  if (ThisFile != NULL) {
    ThisFile->Close (ThisFile);
  }
  if (Root != NULL) {
    Root->Close (Root);
  }
  return Status;
}

EFI_STATUS
BdsLibGetHiiHandles (
  IN     EFI_HII_PROTOCOL *Hii,
  IN OUT UINT16           *HandleBufferLength,
  OUT    EFI_HII_HANDLE   **HiiHandleBuffer
  )
/*++

Routine Description:

  Determines the handles that are currently active in the database.
  It's the caller's responsibility to free handle buffer.

Arguments:

  This                  - A pointer to the EFI_HII_PROTOCOL instance.
  HandleBufferLength    - On input, a pointer to the length of the handle buffer. On output, 
                          the length of the handle buffer that is required for the handles found.
  HiiHandleBuffer       - Pointer to an array of EFI_HII_PROTOCOL instances returned.

Returns:

  EFI_SUCCESS           - Get an array of EFI_HII_PROTOCOL instances successfully.
  EFI_INVALID_PARAMETER - Hii is NULL.
  EFI_NOT_FOUND         - Database not found.
  
--*/
{
  UINT16      TempBufferLength;
  EFI_STATUS  Status;
  
  TempBufferLength = 0;
  
  //
  // Try to find the actual buffer size for HiiHandle Buffer.
  //
  Status = Hii->FindHandles (Hii, &TempBufferLength, *HiiHandleBuffer);
  
  if (Status == EFI_BUFFER_TOO_SMALL) {
      *HiiHandleBuffer = EfiLibAllocateZeroPool (TempBufferLength);
      Status = Hii->FindHandles (Hii, &TempBufferLength, *HiiHandleBuffer);
      //
      // we should not fail here.
      //
      ASSERT_EFI_ERROR (Status);
  }
  
  *HandleBufferLength = TempBufferLength;
  
  return Status;
  
}