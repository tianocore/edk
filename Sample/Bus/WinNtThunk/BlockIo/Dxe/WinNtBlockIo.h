/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  WinNtBlockIo.h

Abstract:

  Produce block IO abstractions for real devices on your PC using Win32 APIs.
  The configuration of what devices to mount or emulate comes from NT 
  environment variables. The variables must be visible to the Microsoft* 
  Developer Studio for them to work.

  * Other names and brands may be claimed as the property of others.

--*/

#ifndef _WIN_NT_BLOCK_IO_H_
#define _WIN_NT_BLOCK_IO_H_

#include "EfiWinNt.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (WinNtIo)

//
// Driver Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (DriverConfiguration)
#include EFI_PROTOCOL_DEFINITION (DriverDiagnostics)
#include EFI_PROTOCOL_DEFINITION (BlockIo)

#define FILENAME_BUFFER_SIZE  80

//
//Language supported for driverconfiguration protocol
//
#define LANGUAGESUPPORTED "eng"

typedef enum {
    EfiWinNtVirtualDisks,
    EfiWinNtPhysicalDisks,
    EifWinNtMaxTypeDisks
} WIN_NT_RAW_DISK_DEVICE_TYPE;

#define WIN_NT_BLOCK_IO_PRIVATE_SIGNATURE   EFI_SIGNATURE_32('N','T','b','k')
typedef struct {
    UINTN                              Signature;
                                      
    EFI_LOCK                           Lock;
                                      
    CHAR16                             Filename[FILENAME_BUFFER_SIZE];
    UINTN                              ReadMode;
    UINTN                              ShareMode;
    UINTN                              OpenMode;
                                      
    HANDLE                             NtHandle;
    WIN_NT_RAW_DISK_DEVICE_TYPE        DeviceType;
                                      
    UINT64                             LastBlock;
    UINTN                              BlockSize;
    UINT64                             NumberOfBlocks;
                                      
    EFI_HANDLE                         EfiHandle;
    EFI_BLOCK_IO_PROTOCOL              BlockIo;
    EFI_BLOCK_IO_MEDIA                 Media;

    EFI_UNICODE_STRING_TABLE           *ControllerNameTable;

    EFI_WIN_NT_THUNK_PROTOCOL          *WinNtThunk;

} WIN_NT_BLOCK_IO_PRIVATE;

#define WIN_NT_BLOCK_IO_PRIVATE_DATA_FROM_THIS(a)  \
         CR(a, WIN_NT_BLOCK_IO_PRIVATE, BlockIo, WIN_NT_BLOCK_IO_PRIVATE_SIGNATURE)

#define LIST_BUFFER_SIZE  512

//
// Block I/O Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL       gWinNtBlockIoDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL       gWinNtBlockIoComponentName;
extern EFI_DRIVER_CONFIGURATION_PROTOCOL gWinNtBlockIoDriverConfiguration;
extern EFI_DRIVER_DIAGNOSTICS_PROTOCOL   gWinNtBlockIoDriverDiagnostics;

//
// EFI Driver Binding Functions
//
EFI_STATUS
WinNtBlockIoDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  );

EFI_STATUS
WinNtBlockIoDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  );

EFI_STATUS
WinNtBlockIoDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  );

//
// Block IO protocol member functions
//
STATIC
EFI_STATUS
EFIAPI
WinNtBlockIoReadBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSize,  
  OUT VOID                  *Buffer
  );

STATIC
EFI_STATUS
EFIAPI
WinNtBlockIoWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSize,
  IN VOID                   *Buffer
  );

STATIC
EFI_STATUS
EFIAPI
WinNtBlockIoFlushBlocks(
  IN EFI_BLOCK_IO_PROTOCOL  *This
  );

STATIC
EFI_STATUS
EFIAPI
WinNtBlockIoResetBlock(
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN BOOLEAN                ExtendedVerification
  );

//
// Private Worker functions
//

STATIC
EFI_STATUS
WinNtBlockIoCreateMapping (
  IN EFI_WIN_NT_IO_PROTOCOL  *WinNtIo,
  IN EFI_HANDLE                         EfiDeviceHandle,
  IN CHAR16                             *Filename,
  IN BOOLEAN                            ReadOnly,
  IN BOOLEAN                            RemovableMedia,
  IN UINTN                              NumberOfBlocks,
  IN UINTN                              BlockSize,
  IN WIN_NT_RAW_DISK_DEVICE_TYPE        DeviceType
  );

STATIC
EFI_STATUS
WinNtBlockIoReadWriteCommon (
  IN  WIN_NT_BLOCK_IO_PRIVATE *Private,
  IN UINT32                   MediaId,
  IN EFI_LBA                  Lba,
  IN UINTN                    BufferSize,
  IN VOID                     *Buffer,
  IN CHAR8                    *CallerName 
  );

STATIC
EFI_STATUS
WinNtBlockIoError (
  IN WIN_NT_BLOCK_IO_PRIVATE      *Private
  );

STATIC
EFI_STATUS
WinNtBlockIoOpenDevice (
  WIN_NT_BLOCK_IO_PRIVATE         *Private
  );

STATIC
CHAR16 *
GetNextElementPastTerminator (
  IN  CHAR16  *EnvironmentVariable,
  IN  CHAR16  Terminator
  );

EFI_STATUS
InitializeWinNtBlockIo (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
SetFilePointer64 (
  IN  WIN_NT_BLOCK_IO_PRIVATE   *Private,
  IN  INT64                     DistanceToMove,
  OUT UINT64                     *NewFilePointer,
  IN  DWORD                     MoveMethod
  );

UINTN
Atoi (
  CHAR16  *String
  );

#endif
