/*++
Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  GetInfo.c

Abstract:
  
  Some library routines to get drivers related infos


--*/
#include "GetInfo.h"

EFI_STATUS
GetDeviceHandlesManagedByDriver (
  IN  EFI_HANDLE  DriverBindingHandle,
  OUT UINTN       *ControllerHandleCount,
  OUT EFI_HANDLE  **ControllerHandleBuffer
  )
/*++

Routine Description:
  Get all device handles which are being opened by a specific driver.
  The rountine will allocate pool buffer for the found device handles, and it is the caller's responsibility to safe
  free the buffer
  
Arguments:
  DriverBindingHandle - the handle of a driver which contains the binding protocol
  ControllerHandleCount - the number of available device handles returned in ControllerHandleBuffer
  ControllerHandleBuffer - a pointer to the buffer to return the array of device handles
  
Returns:
  EFI_STATUS
  If returned status is not succeful or find no available device , the *ControllerHandleBuffer will be NULL
  
--*/
{
  UINTN                               HandleCount;
  EFI_HANDLE                          *HandleBuffer;
  BOOLEAN                             *HandleBufferMap;
  EFI_STATUS                          Status;
  UINTN                               HandleIndex;
  UINTN                               AvailableIndex;
  EFI_GUID                            **ProtocolGuidArray;
  UINTN                               ArrayCount;
  UINTN                               ProtocolIndex;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenInfo;
  UINTN                               OpenInfoCount;
  UINTN                               OpenInfoIndex;
  
  *ControllerHandleCount  = 0;
  *ControllerHandleBuffer = NULL;
  HandleCount = 0;
  HandleBuffer = NULL;  
  
  if (DriverBindingHandle == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto Error;
  }
  
  //
  // Retrieve the list of all handles from the handle database
  //
  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  
  //
  //Create a map for HandleBuffer. If a handle in HandleBuffer is the wanted device handle, its map item is true.
  //
  HandleBufferMap = EfiLibAllocateZeroPool (sizeof (BOOLEAN) * HandleCount);
  ASSERT (HandleBufferMap != NULL);
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    HandleBufferMap[HandleIndex] = FALSE;
  }
  
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    //
    // Check if it is a device handle
    //
    Status = gBS->OpenProtocol (
                    HandleBuffer[HandleIndex],
                    &gEfiDevicePathProtocolGuid,
                    NULL,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }
    //
    // Retrieve the list of all the protocols on each handle
    //
    Status = gBS->ProtocolsPerHandle (
                    HandleBuffer[HandleIndex],
                    &ProtocolGuidArray,
                    &ArrayCount
                    );
                  
    if (!EFI_ERROR (Status)) {
      for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
        //
        // Retrieve the list of agents that have opened each protocol
        //
        Status = gBS->OpenProtocolInformation (
                        HandleBuffer[HandleIndex],
                        ProtocolGuidArray[ProtocolIndex],
                        &OpenInfo,
                        &OpenInfoCount
                        );
        if (!EFI_ERROR (Status)) {
          for (OpenInfoIndex = 0; OpenInfoIndex < OpenInfoCount; OpenInfoIndex++) {
            if (OpenInfo[OpenInfoIndex].AgentHandle == DriverBindingHandle) {
              if ((OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER)
                  == EFI_OPEN_PROTOCOL_BY_DRIVER
                 ){
                //
                // HandleBufferMap[HandleIndex] is the wanted device handle, find it in the handlebuffer
                // A bus driver maybe open a Controller with BY_DRIVER attribute for different protocol  many times, 
                //
                HandleBufferMap[HandleIndex] = TRUE;
              }
            }
          }
          gBS->FreePool (OpenInfo);
        }
      }
      gBS->FreePool (ProtocolGuidArray);
    }
  }
  //
  // count how many device handles are found
  //
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    if (HandleBufferMap[HandleIndex]) {
      (*ControllerHandleCount)++;
    }
  }
  
  if (*ControllerHandleCount > 0) {
    //
    // Copy the found device handle to returned buffer
    //
    *ControllerHandleBuffer = EfiLibAllocateZeroPool (sizeof (EFI_HANDLE) * (*ControllerHandleCount));
    ASSERT (*ControllerHandleBuffer != NULL);
    for (HandleIndex = 0, AvailableIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
      if (HandleBufferMap[HandleIndex]) {
        (*ControllerHandleBuffer)[AvailableIndex] = HandleBuffer[HandleIndex];
        AvailableIndex++;
      }
    }
  }
  
  if (HandleBuffer != NULL) {
    gBS->FreePool (HandleBuffer);
  }
  
  return EFI_SUCCESS;

Error:

  if (HandleBuffer != NULL) {
    gBS->FreePool (HandleBuffer);
  }

  return Status;
}

EFI_STATUS
GetChildDeviceHandlesManagedByDriver (
  IN  EFI_HANDLE  DriverBindingHandle,
  IN  EFI_HANDLE  ControllerHandle,
  OUT UINTN       *ChildControllerHandleCount,
  OUT EFI_HANDLE  **ChildControllerHandleBuffer
  )
/*++

Routine Description:
  Get all child device handles which are being opened by a specific driver.
  The rountine will allocate pool buffer for the found child device handles, and it is the caller's responsibility to safe
  free the buffer
  
Arguments:
  DriverBindingHandle - the handle of a driver which contains the binding protocol
  ControllerHandle - the device controller handle be opened by its child device 
  ChildControllerHandleCount - the number of available device handles returned in ControllerHandleBuffer
  ChildControllerHandleBuffer - a pointer to the buffer to return the array of child device handles
  
Returns:
  EFI_STATUS
  If returned status is not succeful or find no available device , the *ChildControllerHandleBuffer will be NULL
  
--*/

{
  UINTN                               HandleCount;
  EFI_HANDLE                          *HandleBuffer;
  BOOLEAN                             *HandleBufferMap;
  EFI_STATUS                          Status;
  UINTN                               HandleIndex;
  UINTN                               AvailableIndex;
  EFI_GUID                            **ProtocolGuidArray;
  UINTN                               ArrayCount;
  UINTN                               ProtocolIndex;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenInfo;
  UINTN                               OpenInfoCount;
  UINTN                               OpenInfoIndex;
  
  *ChildControllerHandleCount  = 0;
  *ChildControllerHandleBuffer = NULL;
  HandleCount = 0;
  HandleBuffer = NULL;  
  
  if ((DriverBindingHandle == NULL) || (ControllerHandle == NULL)) {
    Status = EFI_INVALID_PARAMETER;
    goto Error;
  }
  
  //
  // Retrieve the list of all handles from the handle database
  //
  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  //
  //Create a map for HandleBuffer. If a handle in HandleBuffer is the wanted device handle, its map item is true.
  //
  HandleBufferMap = EfiLibAllocateZeroPool (sizeof (BOOLEAN) * HandleCount);
  ASSERT (HandleBufferMap != NULL);
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    HandleBufferMap[HandleIndex] = FALSE;
  }
  
  //
  // Retrieve the list of all the protocols on each handle
  //
  Status = gBS->ProtocolsPerHandle (
                  ControllerHandle,
                  &ProtocolGuidArray,
                  &ArrayCount
                  );
                
  if (!EFI_ERROR (Status)) {
    for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
      //
      // Retrieve the list of agents that have opened each protocol
      //
      Status = gBS->OpenProtocolInformation (
                      ControllerHandle,
                      ProtocolGuidArray[ProtocolIndex],
                      &OpenInfo,
                      &OpenInfoCount
                      );
      if (!EFI_ERROR (Status)) {
        for (OpenInfoIndex = 0; OpenInfoIndex < OpenInfoCount; OpenInfoIndex++) {
          if (OpenInfo[OpenInfoIndex].AgentHandle == DriverBindingHandle) {
            if ((OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER)
                  == EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
               ) {
              //
              // OpenInfo[OpenInfoIndex].ControllerHandle is the wanted child device handle, find it in the handlebuffer
              // A bus driver maybe open a Controller with BY_CHILD_CONTROLLER attribute for different protocol many times, 
              //
              for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
                if (OpenInfo[OpenInfoIndex].ControllerHandle == HandleBuffer[HandleIndex]) {
                  HandleBufferMap[HandleIndex] = TRUE;
                }
              }
            }
          }
        }
        gBS->FreePool (OpenInfo);
      }
    }
    gBS->FreePool (ProtocolGuidArray);
  }

  //
  // count how many device handles are found
  //
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    if (HandleBufferMap[HandleIndex]) {
      (*ChildControllerHandleCount)++;
    }
  }
  
  if (*ChildControllerHandleCount > 0) {
    //
    // Copy the found device handle to returned buffer
    //
    *ChildControllerHandleBuffer = EfiLibAllocateZeroPool (sizeof (EFI_HANDLE) * (*ChildControllerHandleCount));
    ASSERT (*ChildControllerHandleBuffer != NULL);
    for (HandleIndex = 0, AvailableIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
      if (HandleBufferMap[HandleIndex]) {
        (*ChildControllerHandleBuffer)[AvailableIndex] = HandleBuffer[HandleIndex];
        AvailableIndex++;
      }
    }
  }
  
  if (HandleBuffer != NULL) {
    gBS->FreePool (HandleBuffer);
  }
  
  return EFI_SUCCESS;

Error:

  if (HandleBuffer != NULL) {
    gBS->FreePool (HandleBuffer);
  }

  return Status;
}

CHAR16 *
GetImageName (
  IN  EFI_LOADED_IMAGE_PROTOCOL *Image
  )
/*++

Routine Description:
  Read the FV UI section to get the image name 

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
  EFI_FIRMWARE_VOLUME_PROTOCOL      *FV;
  VOID                              *Buffer;
  UINTN                             BufferSize;
  UINT32                            AuthenticationStatus;
  EFI_GUID                          *NameGuid;

  FV          = NULL;
  Buffer      = NULL;
  BufferSize  = 0;

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

VOID
AlignmentItem (
  IN        CHAR16   *ControllerHandleName,
  IN        CHAR16   *PrefixString,
  IN OUT    CHAR16   **NewString
  )
/*++

Routine Description:
  Do controller item string swap and alignment if needed. The alignment is the length of PrefixString.
  Because controller device path is too long sometime and cannot be presented in one line,
  and the browser automatic swap will break the necessary alignment, so do some additional 
  process  for the problem.
  
Arguments:
 ControllerHandleName - a pointer to the controller real device path string
 PrefixString- a pointer to the prefix string
 NewString - a pointer to the string which will be presented by the browser 

Returns:
  None
  
--*/
{

  CHAR16                                    *PadString;
  UINTN                                     PtrIndex;
  UINTN                                     IndexOffset;
  UINTN                                     Width;
  CHAR16                                    *NewStringSwapped;
  UINTN                                     SwapLineNum;
  UINTN                                     SwapLine;

  //
  // Register the device name string and create item in set options page
  //
  *NewString = EfiLibAllocateZeroPool (EfiStrSize (ControllerHandleName) + EfiStrSize (PrefixString));
  EfiStrCat (*NewString, PrefixString);
  EfiStrCat (*NewString, ControllerHandleName);
  //
  // Add pad chars into the string  for string swap in set options page to solve following two issue:
  // Issue1: Our form browser will do the string swap according to the spaces in the string, so
  //              if a string is too long and without space in it, the string position will be ugly. 
  //              The driver need add some spaces to control the swap in the long and no space string.
  // Issue2:  If browser find a space to do swap, it will not show any other space directly followed it in swap position.  
  //                So if you want to use the space to do the alignment in swapped new line, you need add another char(e.g '.')  in it.
  // e.g. if the item max lenth is 5, then the following string need add some space and '.' to get right presentation 
  // '12345678901234567890' --------> '      12345 .    67890 .    12345 .    67890' , and presentation is below
  // |     12345
  // |.    67890
  // |.    12345
  // |.    67890
  //
  if ((EfiStrLen (*NewString)/SWAP_LENGTH > 0) && (EfiStrLen (PrefixString) > 0)) {
    //
    // Prepare the pad string according to the PrefixString length
    // the pad string is string of ' ', except of the NO2 charater which is '.'
    //
    PadString = EfiLibAllocateZeroPool (EfiStrSize (PrefixString) + 2);
    for (PtrIndex = 0; PtrIndex < (EfiStrLen (PrefixString) + 1); PtrIndex++) {
      PadString[PtrIndex] = ' ';
    }
    PadString[1] = '.';  
    
    Width = SWAP_LENGTH - EfiStrLen (PrefixString);
    SwapLineNum = EfiStrLen (ControllerHandleName)/Width;
    
    NewStringSwapped = EfiLibAllocateZeroPool (EfiStrSize (ControllerHandleName) + 
                       EfiStrSize (PrefixString) + 
                       SwapLineNum * EfiStrSize (PadString));   
    ASSERT (NewStringSwapped != NULL);
    
    IndexOffset = 0;
    EfiStrCpy (NewStringSwapped, PrefixString);
    IndexOffset += EfiStrLen (PrefixString);    
    
    for (SwapLine = 0; SwapLine < SwapLineNum; SwapLine++) {
      EfiStrnCpy (&NewStringSwapped[IndexOffset], 
                  &ControllerHandleName[SwapLine * Width],
                  Width
                 );
      IndexOffset += Width;
      
      EfiStrnCpy (&NewStringSwapped[IndexOffset], 
                  PadString,
                  EfiStrLen (PadString)
                 );
      IndexOffset += EfiStrLen (PadString);
    }
    EfiStrCat (NewStringSwapped, &ControllerHandleName[SwapLine * Width]);
    gBS->FreePool (PadString);
    gBS->FreePool (*NewString);
    *NewString = NewStringSwapped;
  }
  return;
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
  CHAR16                        Lang[4];
  
  StringLen = 0x1000;
  String = EfiLibAllocateZeroPool (StringLen);
  ASSERT (String != NULL);
  
  Status = GetCurrentLanguage (Lang);
  ASSERT_EFI_ERROR (Status);
  
  Status = Private->Hii->GetString (
                          Private->Hii, 
                          Private->RegisteredHandle, 
                          Token, 
                          TRUE, 
                          Lang,
                          &StringLen,
                          String
                          );
  if (EFI_ERROR(Status)) {
    gBS->FreePool (String);
    String = NULL;
  }
  
  return String;
}