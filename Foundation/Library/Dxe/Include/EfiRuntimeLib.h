/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    EfiRuntimeLib.h

Abstract:

  Light weight lib to support EFI drivers.

--*/

#ifndef _EFI_RUNTIME_LIB_H_
#define _EFI_RUNTIME_LIB_H_
#define MAX_FVB_COUNT     16
#include "EfiStatusCode.h"
#include "EfiCommonLib.h"

#include "LinkedList.h"
#include EFI_GUID_DEFINITION (DxeServices)
#include EFI_PROTOCOL_DEFINITION (CpuIo)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeBlock)
#include EFI_PROTOCOL_DEFINITION (FvbExtension)
#include "ProcDep.h"

typedef struct {
  EFI_HANDLE                            Handle;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL    *Fvb;
  EFI_FVB_EXTENSION_PROTOCOL            *FvbExtension;
} FVB_ENTRY;


//
// Driver Lib Globals.
//
extern EFI_BOOT_SERVICES         *gBS;
extern EFI_SYSTEM_TABLE          *gST;
extern EFI_DXE_SERVICES          *gDS;
extern UINTN                     gRtErrorLevel;
extern FVB_ENTRY                 *mFvbEntry;

VOID
EFIAPI
EfiRuntimeLibFvbVirtualNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );

EFI_STATUS
EfiInitializeRuntimeDriverLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN EFI_EVENT_NOTIFY     RuntimeNotifyEventHandler
  );

EFI_STATUS
EfiInitializeSmmDriverLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EfiLibGetSystemConfigurationTable (
  IN EFI_GUID *TableGuid,
  IN OUT VOID **Table
  );

BOOLEAN
EfiAtRuntime (
  VOID
  );

BOOLEAN
EfiGoneVirtual (
  VOID
  );


EFI_STATUS
EfiLibGetSystemConfigurationTable (
  IN EFI_GUID *TableGuid,
  IN OUT VOID **Table
  );

EFI_EVENT
RtEfiLibCreateProtocolNotifyEvent (
  IN EFI_GUID             *ProtocolGuid,
  IN EFI_TPL              NotifyTpl,
  IN EFI_EVENT_NOTIFY     NotifyFunction,
  IN VOID                 *NotifyContext,
  OUT VOID                *Registration
  );

//
// Lock.c
//

typedef struct {
  EFI_TPL     Tpl;
  EFI_TPL     OwnerTpl;
  UINTN       Lock;
} EFI_LOCK;

VOID
EfiInitializeLock (
  IN OUT EFI_LOCK *Lock,
  IN EFI_TPL      Priority
  );

//
// Macro to initialize the state of a lock when a lock variable is declared
//
#define EFI_INITIALIZE_LOCK_VARIABLE(Tpl) {Tpl,0,0}


VOID
EfiAcquireLock (
  IN EFI_LOCK *Lock
  );

EFI_STATUS
EfiAcquireLockOrFail (
  IN EFI_LOCK  *Lock
  );

VOID
EfiReleaseLock (
  IN EFI_LOCK *Lock
  );

#define EfiCopyMem EfiCommonLibCopyMem
#define EfiSetMem  EfiCommonLibSetMem
#define EfiZeroMem EfiCommonLibZeroMem

INTN
EfiCompareMem (
  IN VOID     *MemOne,
  IN VOID     *MemTwo,
  IN UINTN    Len
  );

//
// Debug.c init
//

EFI_STATUS
EfiDebugAssertInit (
  VOID
  );


//
// Wrapper for EFI runtime functions
//
VOID
EfiResetSystem (
  IN EFI_RESET_TYPE               ResetType,
  IN EFI_STATUS                   ResetStatus,
  IN UINTN                        DataSize,
  IN CHAR16                       *ResetData
  );

EFI_STATUS
EfiGetNextHighMonotonicCount (
  OUT UINT32                      *HighCount
  );

EFI_STATUS
EfiGetTime (
  OUT EFI_TIME                    *Time,
  OUT EFI_TIME_CAPABILITIES       *Capabilities 
  );

EFI_STATUS
EfiSetTime (
  OUT EFI_TIME                    *Time
  );

EFI_STATUS
EfiGetWakeupTime (
  OUT BOOLEAN                     *Enabled,
  OUT BOOLEAN                     *Pending,
  OUT EFI_TIME                    *Time
  );

EFI_STATUS
EfiSetWakeupTime (
  IN BOOLEAN                      Enable,
  IN EFI_TIME                     *Time
  );


EFI_STATUS
EfiGetVariable (
  IN CHAR16                       *VariableName,
  IN EFI_GUID                     *VendorGuid,
  OUT UINT32                      *Attributes OPTIONAL,
  IN OUT UINTN                    *DataSize,
  OUT VOID                        *Data
  );


EFI_STATUS
EfiGetNextVariableName (
  IN OUT UINTN                    *VariableNameSize,
  IN OUT CHAR16                   *VariableName,
  IN OUT EFI_GUID                 *VendorGuid
  );



EFI_STATUS
EfiSetVariable (
  IN CHAR16                       *VariableName,
  IN EFI_GUID                     *VendorGuid,
  IN UINT32                       Attributes,
  IN UINTN                        DataSize,
  IN VOID                         *Data
  );


EFI_STATUS
EfiReportStatusCode (
  IN EFI_STATUS_CODE_TYPE     CodeType,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 *CallerId,
  IN EFI_STATUS_CODE_DATA     *Data OPTIONAL
  );

EFI_STATUS
EfiConvertPointer (
  IN UINTN                     DebugDisposition,
  IN OUT VOID                  *Address
  );

EFI_STATUS
EfiConvertList (
  IN UINTN                DebugDisposition,
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

//
//  Base IO Class Functions
//

EFI_STATUS
EfiIoRead (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                     Address,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  );

UINT8
IoRead8 (
  IN  UINT64    Address
  );

UINT16
IoRead16 (
  IN  UINT64    Address
  );

UINT32
IoRead32 (
  IN  UINT64    Address
  );


EFI_STATUS
EfiIoWrite (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                     Address,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  );

VOID
IoWrite8 (
  IN  UINT64    Address,
  IN  UINT8     Data
  );

VOID
IoWrite16 (
  IN  UINT64    Address,
  IN  UINT16    Data
  );

VOID
IoWrite32 (
  IN  UINT64    Address,
  IN  UINT32    Data
  );

EFI_STATUS
EfiMemRead (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                     Address,
  IN     UINTN                      Count,
  IN  OUT VOID                      *Buffer
  );

UINT32
MemRead32 (
  IN  UINT64    Address
  );

UINT64
MemRead64 (
  IN  UINT64    Address
  );



EFI_STATUS
EfiMemWrite (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                     Address,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  );

VOID
MemWrite32 (
  IN  UINT64    Address,
  IN  UINT32    Data
  );

VOID
EfiMemWrite64 (
  IN  UINT64    Address,
  IN  UINT64    Data
  );

//
//  Platform specific functions
//

UINT8
PciRead8 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register
  );

UINT16
PciRead16 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register
  );

UINT32
PciRead32 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register
  );

VOID
PciWrite8 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register,
  UINT8   Data
  );

VOID
PciWrite16 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register,
  UINT16  Data
  );

VOID
PciWrite32 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register,
  UINT32  Data
  );


VOID
EfiStall (
  IN  UINTN   Microseconds
  );

//
//  FVB Services.
//

EFI_STATUS
EfiFvbInitialize (
  VOID
  );

EFI_STATUS
EfiFvbReadBlock (
  IN UINTN                                        Instance,
  IN EFI_LBA                                      Lba,
  IN UINTN                                        Offset,
  IN OUT UINTN                                    *NumBytes,
  IN UINT8                                        *Buffer
  );

EFI_STATUS
EfiFvbWriteBlock (
  IN UINTN                                        Instance,
  IN EFI_LBA                                      Lba,
  IN UINTN                                        Offset,
  IN OUT UINTN                                    *NumBytes,
  IN UINT8                                        *Buffer
  );

EFI_STATUS
EfiFvbEraseBlock (
  IN UINTN                                Instance,
  IN EFI_LBA                              Lba
  );

EFI_STATUS
EfiFvbGetVolumeAttributes (
  IN UINTN                                Instance,
  OUT EFI_FVB_ATTRIBUTES                  *Attributes
  );

EFI_STATUS
EfiFvbSetVolumeAttributes (
  IN UINTN                                Instance,
  IN EFI_FVB_ATTRIBUTES                   Attributes
  );

EFI_STATUS
EfiFvbGetPhysicalAddress (
  IN UINTN                                Instance,
  OUT EFI_PHYSICAL_ADDRESS                *Address
  );

EFI_STATUS
EfiFvbGetBlockSize (
  IN UINTN                                Instance,
  IN EFI_LBA                              Lba,
  OUT UINTN                               *BlockSize,
  OUT UINTN                               *NumOfBlocks
  );

EFI_STATUS
EfiFvbEraseCustomBlockRange (
  IN UINTN                                Instance,
  IN EFI_LBA                              StartLba,
  IN UINTN                                OffsetStartLba,
  IN EFI_LBA                              LastLba,
  IN UINTN                                OffsetLastLba
  );

EFI_STATUS
EfiCpuFlushCache (
  IN EFI_PHYSICAL_ADDRESS          Start,
  IN UINT64                        Length
  );

#endif
