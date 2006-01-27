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

#include "Go64.h"

VOID *
GetFileFromWhereWeLoaded (
  IN  CHAR16      *InputFilename,
  OUT UINTN       *BfvLength
  )
/*++

Routine Description:
  Open the file InputFilename from the same location that this driver was 
  loaded from. 

Arguments:
  InputFilename - File name to load
  BfvLength     - Length of file that was loaded

Returns:
  Pointer to file loaded into memory or NULL

--*/
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
  // Make FileName point to the directory we loaded this 
  // driver from and load a file named x64.FV.
  //
  
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
  // Figure out how big the file info is and allocate a buffer for it
  //
  Size = 1;
  FileInfo = AllocateTempPages (Size);
  Size *= EFI_PAGE_SIZE;
  if (FileInfo != NULL) {
    Status = FileHandle->GetInfo (FileHandle, &gEfiFileInfoGuid, &Size, FileInfo);
    if (!EFI_ERROR (Status)) {  
      Buffer = AllocateTempPages (EFI_SIZE_TO_PAGES ((UINTN)FileInfo->FileSize));
      if (Buffer != NULL) {
        *BfvLength = (UINTN)FileInfo->FileSize;

        //
        // Read in the file 
        //
        Status = FileHandle->Read (FileHandle, BfvLength, Buffer);
        FileHandle->Close (FileHandle);
        if (!EFI_ERROR (Status)) {
          return Buffer;      
        }
      }
    }
  }

  //
  // FileInfo is a memory leak. 
  //

  return NULL;
}


VOID *
FindFileInFv (
  IN  EFI_HANDLE        *Bfv,
  IN  EFI_SECTION_TYPE  Type,
  IN  EFI_GUID          *FileName,
  OUT UINTN             *ImageSize
  )
/*++

Routine Description:
  Find a file in the FV and extract it and return it

Arguments:
  Bfv       - Boot Firmware Volume
  Type      - Type of file to seach for
  FileName  - Filename in FV
  ImageSize - Size of image returned

Returns:
  Pointer to memory buffer of ImageSize extrated from FV. 

--*/
{
  EFI_STATUS                      Status;
  EFI_FIRMWARE_VOLUME_PROTOCOL    *Fv;
  VOID                            *Buffer;
  VOID                            *TempBuffer;
  UINT32                          AuthenticationStatus;

  //
  // Find the right FV protocol
  //
  Status = gBS->HandleProtocol (Bfv, &gEfiFirmwareVolumeProtocolGuid, &Fv);
  if (EFI_ERROR (Status)) {
    return NULL;
  }
  
  //
  // Read the file type and section name from the FV
  //
  *ImageSize = 0;
  Buffer = NULL;
  Status = Fv->ReadSection (
                Fv, 
                FileName,
                Type,
                0,
                &Buffer,
                ImageSize,
                &AuthenticationStatus
                );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  TempBuffer = AllocateTempPages (EFI_SIZE_TO_PAGES (*ImageSize));
  if (TempBuffer == NULL) {
    return NULL;
  }

  EfiCommonLibCopyMem(TempBuffer, Buffer, *ImageSize);
  gBS->FreePool (Buffer);

  return TempBuffer;
}


EFI_STATUS
FindDxeCoreFileName (
  IN  EFI_HANDLE        *Bfv,
  OUT EFI_GUID          *FileName
  )
/*++

Routine Description:
  Find DXE core in Boot Firmware Volume 

Arguments:
  Bfv      - Boot Firmware Volume
  FileName - Returned filename of DXE core

Returns:
  EFI_SUCCESS - DXE Core found

--*/
{
  EFI_STATUS                    Status;
  EFI_FIRMWARE_VOLUME_PROTOCOL  *Fv;
  EFI_FV_FILETYPE               FoundType;
  EFI_FV_FILE_ATTRIBUTES        FileAttributes;
  UINTN                         Key;
  UINTN                         Size;

  //
  // Find the right FV protocol
  //
  Status = gBS->HandleProtocol (Bfv, &gEfiFirmwareVolumeProtocolGuid, &Fv);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  Key = 0;
  do {
    FoundType = EFI_FV_FILETYPE_DXE_CORE;
    Status = Fv->GetNextFile (
                  Fv,
                  &Key,
                  &FoundType,
                  FileName,
                  &FileAttributes,
                  &Size
                  );
    if (!EFI_ERROR (Status)) {
      return EFI_SUCCESS;
    }
  } while (!EFI_ERROR (Status));

  return Status;
}

