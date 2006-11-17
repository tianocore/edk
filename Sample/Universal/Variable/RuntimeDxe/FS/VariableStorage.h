/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    VariableStorage.h

Abstract:

    handles variable store/reads with memory and file

Revision History

--*/
#ifndef _VARIABLE_STORAGE_H_
#define _VARIABLE_STORAGE_H_
#include "EfiVariable.h"

#define VAR_DEFAULT_VALUE           (0xff)
#define VAR_DEFAULT_VALUE_16        EFI_SIGNATURE_16 (VAR_DEFAULT_VALUE, VAR_DEFAULT_VALUE)
#define VAR_DEFAULT_VALUE_32        EFI_SIGNATURE_32 (VAR_DEFAULT_VALUE, VAR_DEFAULT_VALUE, \
                                                      VAR_DEFAULT_VALUE, VAR_DEFAULT_VALUE)

#include EFI_PROTOCOL_DEFINITION (SimpleFileSystem)
#include EFI_PROTOCOL_DEFINITION (FileSystemInfo)

EFI_FORWARD_DECLARATION (VARIABLE_STORAGE);

typedef struct _VS_FILE_INFO {
  UINT8                     *FileData;      // local buffer for reading acceleration

  EFI_FILE                  *File;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;    // device having storage file
} VS_FILE_INFO;

typedef struct _VS_MEM_INFO {
  UINT8                     *MemData;
} VS_MEM_INFO;

EFI_STATUS
FileStorageConstructor (
  OUT VARIABLE_STORAGE          **VarStore,
  IN UINT32                     Attributes,
  IN UINTN                      Size,
  IN EFI_HANDLE                 Handle
  );

EFI_STATUS
MemStorageConstructor (
  OUT VARIABLE_STORAGE          **VarStore,  
  IN  UINT32                    Attributes,
  IN  UINTN                     Size
  );


typedef
VOID
(EFIAPI *DESTRUCT_STORE) (
  IN VARIABLE_STORAGE   *This
  );

typedef
EFI_STATUS
(EFIAPI *ERASE_STORE) (
  IN VARIABLE_STORAGE   *This
  );

typedef
EFI_STATUS
(EFIAPI *WRITE_STORE) (
  IN VARIABLE_STORAGE   *This,
  IN UINTN              Offset,
  IN UINTN              BufferSize,
  IN VOID               *Buffer
  );

typedef struct _VARIABLE_STORAGE {

  //
  // Functions to access the storage
  //
  ERASE_STORE           Erase;
  WRITE_STORE           Write;

  //
  // Functions to destroy storage. e.g.: free allocated memory
  //
  DESTRUCT_STORE        Destruct;

} VARIABLE_STORAGE;


typedef struct _VS_DEV {
  UINT32             Signature;
  VARIABLE_STORAGE   VarStore;
  //
  // Attributes and size
  //
  UINT32             Attributes;
  UINTN              Size;
  
  union {
    UINT8            *Data;
    VS_FILE_INFO     FileInfo;
    VS_MEM_INFO      MemInfo;
  } Info;

} VS_DEV;

#define DEV_FROM_THIS(a)        CR (a, VS_DEV, VarStore, VARIABLE_STORE_SIGNATURE)

#define VAR_DATA_PTR(a)         ((a)->Info.Data)
#define VAR_FILE_DEVICEPATH(a)  ((a)->Info.FileInfo.DevicePath)
#define VAR_FILE_FILEHANDLE(a)  ((a)->Info.FileInfo.File)


#endif
