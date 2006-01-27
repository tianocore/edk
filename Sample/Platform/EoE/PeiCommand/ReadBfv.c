/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  ReadBfv.c

Abstract:
  This is an application that invokes the FormConfiguration protoclol
  causing the Setup to run. This is only temporary till the
  Setup will be hooked correctly under the BDS' UI phase.


Revision History:

--*/

#include "PeiCommand.h"
#include "EfiDriverLib.h"



VOID *
GetFileFromWhereWeLoaded (
  IN  CHAR16      *InputFilename,
  OUT UINTN       *BfvLength
  )
{
  EFI_STATUS                      Status;
  EFI_LOADED_IMAGE_PROTOCOL       *LoadedImage;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Vol;  
  FILEPATH_DEVICE_PATH            *FileDevicePath;
  EFI_FILE_HANDLE                 RootFs;
  EFI_FILE_HANDLE                 FileHandle;
  CHAR16                          FileName[100];
  EFI_FILE_INFO                   *FileInfo;
  VOID                            *Buffer;
  UINTN                           i;
  UINTN                           Size;
  EFI_DEVICE_PATH_PROTOCOL        *DevPath;

  //
  // Get the LoadedImage protocol so we can figure out where this driver was
  //  loaded from
  //
  Status = gBS->HandleProtocol (gMyImageHandle, &gEfiLoadedImageProtocolGuid, &LoadedImage);
  if (EFI_ERROR (Status)) {
    return NULL;
  }
  
  //
  // Find the FileDevicepath
  //
  DevPath = LoadedImage->FilePath;
  FileDevicePath = NULL;
  while (!IsDevicePathEndType (DevPath)) {
    if ((DevicePathType (DevPath) == MEDIA_DEVICE_PATH) &&
        (DevicePathSubType (DevPath) == MEDIA_FILEPATH_DP)){
      FileDevicePath = (FILEPATH_DEVICE_PATH *)DevPath;
    }
    DevPath = NextDevicePathNode (DevPath);
  }
  
  if (FileDevicePath == NULL) {
    //
    // Could not find the file to load.
    // BugBug: Need to add FV support
    //
    return NULL;
  }

  EfiStrCpy (FileName, FileDevicePath->PathName);


  for (i = EfiStrLen (FileName); i>0 && FileName[i] != '/'; i--);
  if (FileName[i-1] == '\\') {
    i--;
  }
  FileName[i]=0;
  EfiStrCat (&FileName[i], InputFilename);

  //
  // Open The File that contains the FV with the x64 code
  //
  Status = gBS->HandleProtocol (LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, &Vol);

  Status = Vol->OpenVolume (Vol, &RootFs);

  Status = RootFs->Open (RootFs, &FileHandle, FileName, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Figure out how big the file is and allocate a buffer for it
  //
  FileInfo = EfiLibAllocatePool (0x1000);
  Size = 0x1000;
  Status = FileHandle->GetInfo (FileHandle, &gEfiFileInfoGuid, &Size, FileInfo);

  Buffer = EfiLibAllocatePool ((UINTN)FileInfo->FileSize);
  *BfvLength = (UINTN)FileInfo->FileSize;

  gBS->FreePool (FileInfo);

  //
  // Read in the file (x64.FV)
  //
  Status = FileHandle->Read (FileHandle, BfvLength, Buffer);

  FileHandle->Close (FileHandle);

  if (EFI_ERROR (Status)) {
    return NULL;
  } else {
    return Buffer;
  }
}


