/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DxeCore.h

Abstract:

Revision History

--*/

#ifndef _DXECORE_H_
#define _DXECORE_H_

#include EFI_GUID_DEFINITION (PeiFlushInstructionCache)
#include EFI_GUID_DEFINITION (PeiPeCoffLoader)
#include EFI_GUID_DEFINITION (PeiTransferControl)
#include EFI_GUID_DEFINITION (Hob)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)
#include EFI_GUID_DEFINITION (DxeServices)
#include EFI_GUID_DEFINITION (MemoryTypeInformation)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_ARCH_PROTOCOL_DEFINITION (Cpu)
#include EFI_ARCH_PROTOCOL_DEFINITION (Metronome)
#include EFI_ARCH_PROTOCOL_DEFINITION (MonotonicCounter)
#include EFI_ARCH_PROTOCOL_DEFINITION (Timer)
#include EFI_ARCH_PROTOCOL_DEFINITION (Bds)
#include EFI_ARCH_PROTOCOL_DEFINITION (Reset)
#include EFI_ARCH_PROTOCOL_DEFINITION (RealTimeClock)
#include EFI_ARCH_PROTOCOL_DEFINITION (Variable)
#include EFI_ARCH_PROTOCOL_DEFINITION (VariableWrite)
#include EFI_ARCH_PROTOCOL_DEFINITION (WatchdogTimer)
#include EFI_ARCH_PROTOCOL_DEFINITION (Runtime)
#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)
#include EFI_ARCH_PROTOCOL_DEFINITION (Security)
#include EFI_PROTOCOL_DEFINITION (Decompress)
#include EFI_PROTOCOL_DEFINITION (TianoDecompress)
#include EFI_PROTOCOL_DEFINITION (CustomizedDecompress)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolume)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeDispatch)
#include EFI_PROTOCOL_DEFINITION (LoadedImage)
#include "LinkedList.h"
#include "DebugImageInfo.h"
#include "EfiCommonLib.h"
#include "Library.h"
#include "Peihob.h"
#include "EfiHobLib.h"
#include "DebugMask.h"


typedef struct {
  EFI_GUID                    *ProtocolGuid;
  VOID                        **Protocol;
  EFI_EVENT                   Event;
  VOID                        *Registration;
  BOOLEAN                     Present;
} ARCHITECTURAL_PROTOCOL_ENTRY;


//
// DXE Dispatcher Data structures
//

#define KNOWN_HANDLE_SIGNATURE  EFI_SIGNATURE_32('k','n','o','w')
typedef struct {
  UINTN           Signature;
  EFI_LIST_ENTRY  Link;         // mFvHandleList           
  EFI_HANDLE      Handle;
} KNOWN_HANDLE;


#define EFI_CORE_DRIVER_ENTRY_SIGNATURE EFI_SIGNATURE_32('d','r','v','r')
typedef struct {
  UINTN                           Signature;
  EFI_LIST_ENTRY                  Link;             // mDriverList

  EFI_LIST_ENTRY                  ScheduledLink;    // mScheduledQueue

  EFI_HANDLE                      FvHandle;
  EFI_GUID                        FileName;
  EFI_DEVICE_PATH_PROTOCOL        *FvFileDevicePath;
  EFI_FIRMWARE_VOLUME_PROTOCOL    *Fv;

  VOID                            *Depex;
  UINTN                           DepexSize;

  BOOLEAN                         Before;
  BOOLEAN                         After;
  EFI_GUID                        BeforeAfterGuid;

  BOOLEAN                         Dependent;
  BOOLEAN                         Unrequested;
  BOOLEAN                         Scheduled;
  BOOLEAN                         Untrusted;
  BOOLEAN                         Initialized;
  BOOLEAN                         DepexProtocolError;

  EFI_HANDLE                      ImageHandle;

} EFI_CORE_DRIVER_ENTRY;

//
//The data structure of GCD memory map entry
//
#define EFI_GCD_MAP_SIGNATURE  EFI_SIGNATURE_32('g','c','d','m')
typedef struct {
  UINTN                 Signature;
  EFI_LIST_ENTRY        Link;
  EFI_PHYSICAL_ADDRESS  BaseAddress;
  UINT64                EndAddress;
  UINT64                Capabilities;
  UINT64                Attributes;
  EFI_GCD_MEMORY_TYPE   GcdMemoryType;
  EFI_GCD_IO_TYPE       GcdIoType;
  EFI_HANDLE            ImageHandle;
  EFI_HANDLE            DeviceHandle;
} EFI_GCD_MAP_ENTRY;

//
// DXE Core Global Variables
//
extern EFI_SYSTEM_TABLE                         *gST;
extern EFI_BOOT_SERVICES                        *gBS;
extern EFI_RUNTIME_SERVICES                     *gRT;
extern EFI_DXE_SERVICES                         *gDS;
extern EFI_HANDLE                               gDxeCoreImageHandle;

extern EFI_DECOMPRESS_PROTOCOL                  *gEfiDecompress;
extern EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL *gEfiPeiFlushInstructionCache;
extern EFI_PEI_PE_COFF_LOADER_PROTOCOL          *gEfiPeiPeCoffLoader;
extern EFI_PEI_TRANSFER_CONTROL_PROTOCOL        *gEfiPeiTransferControl;

extern EFI_RUNTIME_ARCH_PROTOCOL                *gRuntime;
extern EFI_CPU_ARCH_PROTOCOL                    *gCpu;
extern EFI_WATCHDOG_TIMER_ARCH_PROTOCOL         *gWatchdogTimer;
extern EFI_METRONOME_ARCH_PROTOCOL              *gMetronome;
extern EFI_TIMER_ARCH_PROTOCOL                  *gTimer;
extern EFI_SECURITY_ARCH_PROTOCOL               *gSecurity;
extern EFI_BDS_ARCH_PROTOCOL                    *gBds;

extern EFI_TPL                                  gEfiCurrentTpl;

extern EFI_GUID                                 *gDxeCoreFileName;
extern EFI_LOADED_IMAGE_PROTOCOL                *gDxeCoreLoadedImage;

extern EFI_MEMORY_TYPE_INFORMATION              gMemoryTypeInformation[EfiMaxMemoryType + 1];

//
// Service Initialization Functions
//


VOID
CoreInitializePool (
  VOID
  );

VOID
CoreAddMemoryDescriptor (
  IN EFI_MEMORY_TYPE       Type,
  IN EFI_PHYSICAL_ADDRESS  Start,
  IN UINT64                NumberOfPages,
  IN UINT64                Attribute
  );

VOID
CoreReleaseGcdMemoryLock (
  VOID
  );

VOID
CoreAcquireGcdMemoryLock (
  VOID
  );

EFI_STATUS
CoreInitializeMemoryServices (
  IN VOID                  **HobStart,
  IN EFI_PHYSICAL_ADDRESS  *MemoryBaseAddress,
  IN UINT64                *MemoryLength
  );

EFI_STATUS
CoreInitializeGcdServices (
  IN VOID                  **HobStart,
  IN EFI_PHYSICAL_ADDRESS  MemoryBaseAddress,
  IN UINT64                MemoryLength
  );

EFI_STATUS
CoreInitializeEventServices (
  VOID
  );

EFI_STATUS
CoreShutdownEventServices (
  VOID
  );

EFI_STATUS
CoreInitializeImageServices (
  IN  VOID *HobStart
  );

EFI_STATUS
CoreShutdownImageServices (
  VOID
  );

VOID
CoreNotifyOnArchProtocolInstallation (
  VOID
  );

EFI_STATUS
CoreAllEfiServicesAvailable (
  VOID
  );

VOID
CalculateEfiHdrCrc (
  IN  OUT EFI_TABLE_HEADER    *Hdr
  );

VOID
EFIAPI
CoreTimerTick (
  IN UINT64     Duration
  );

VOID
CoreInitializeDispatcher (
  VOID
  );

BOOLEAN
CoreIsSchedulable (
  IN  EFI_CORE_DRIVER_ENTRY   *DriverEntry  
  );

EFI_STATUS
CorePreProcessDepex (
  IN  EFI_CORE_DRIVER_ENTRY   *DriverEntry  
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreExitBootServices (
  IN EFI_HANDLE   ImageHandle,
  IN UINTN        MapKey
  );

EFI_STATUS
CoreTerminateMemoryMap (
  IN UINTN        MapKey
  );

VOID
CoreNotifySignalList (
  IN UINTN        SignalType
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreInstallConfigurationTable (
  IN EFI_GUID         *Guid,
  IN VOID             *Table
  );

EFI_BOOTSERVICE
EFI_TPL
EFIAPI
CoreRaiseTpl (
  IN EFI_TPL  NewTpl
  );

EFI_BOOTSERVICE
VOID
EFIAPI
CoreRestoreTpl (
  IN EFI_TPL  NewTpl
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreStall (
  IN UINTN            Microseconds
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreSetWatchdogTimer (
  IN UINTN            Timeout,
  IN UINT64           WatchdogCode,
  IN UINTN            DataSize,
  IN CHAR16           *WatchdogData   OPTIONAL
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreInstallProtocolInterface (
  IN OUT EFI_HANDLE     *UserHandle,
  IN EFI_GUID           *Protocol,
  IN EFI_INTERFACE_TYPE InterfaceType,
  IN VOID               *Interface
  );

EFI_STATUS
CoreInstallProtocolInterfaceNotify (
  IN OUT EFI_HANDLE     *UserHandle,
  IN EFI_GUID           *Protocol,
  IN EFI_INTERFACE_TYPE InterfaceType,
  IN VOID               *Interface,
  IN BOOLEAN            Notify
  );

EFI_BOOTSERVICE11
EFI_STATUS
EFIAPI
CoreInstallMultipleProtocolInterfaces (
  IN OUT EFI_HANDLE           *Handle,
  ...
  );

EFI_BOOTSERVICE11
EFI_STATUS
EFIAPI
CoreUninstallMultipleProtocolInterfaces (
  IN EFI_HANDLE           Handle,
  ...
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreReinstallProtocolInterface (
  IN EFI_HANDLE     UserHandle,
  IN EFI_GUID       *Protocol,
  IN VOID           *OldInterface,
  IN VOID           *NewInterface
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreUninstallProtocolInterface (
  IN EFI_HANDLE       UserHandle,
  IN EFI_GUID         *Protocol,
  IN VOID             *Interface
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreHandleProtocol (
  IN  EFI_HANDLE       UserHandle,
  IN  EFI_GUID         *Protocol,
  OUT VOID             **Interface
  );

EFI_BOOTSERVICE11
EFI_STATUS
EFIAPI
CoreOpenProtocol (
  IN  EFI_HANDLE                UserHandle,
  IN  EFI_GUID                  *Protocol,
  OUT VOID                      **Interface OPTIONAL,
  IN  EFI_HANDLE                ImageHandle,
  IN  EFI_HANDLE                ControllerHandle,
  IN  UINT32                    Attributes
  );

EFI_BOOTSERVICE11
EFI_STATUS
EFIAPI
CoreOpenProtocolInformation (
  IN  EFI_HANDLE                          UserHandle,
  IN  EFI_GUID                            *Protocol,
  OUT EFI_OPEN_PROTOCOL_INFORMATION_ENTRY **EntryBuffer,
  OUT UINTN                               *EntryCount
  );

EFI_BOOTSERVICE11
EFI_STATUS
EFIAPI
CoreCloseProtocol (
  IN  EFI_HANDLE                UserHandle,
  IN  EFI_GUID                  *Protocol,
  IN  EFI_HANDLE                ImageHandle,
  IN  EFI_HANDLE                ControllerHandle
  );

EFI_BOOTSERVICE11
EFI_STATUS
EFIAPI
CoreProtocolsPerHandle (
  IN  EFI_HANDLE       UserHandle,
  OUT EFI_GUID         ***ProtocolBuffer,
  OUT UINTN            *ProtocolBufferCount
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreRegisterProtocolNotify (
  IN  EFI_GUID       *Protocol,
  IN  EFI_EVENT      Event,
  OUT VOID           **Registration
  );
  
EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreLocateHandle (
  IN     EFI_LOCATE_SEARCH_TYPE         SearchType,
  IN     EFI_GUID                       *Protocol OPTIONAL,
  IN     VOID                           *SearchKey OPTIONAL,
  IN OUT UINTN                          *BufferSize,
  OUT    EFI_HANDLE                     *Buffer
  );
  
EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreLocateDevicePath (
  IN     EFI_GUID                       *Protocol,
  IN OUT EFI_DEVICE_PATH_PROTOCOL       **FilePath,
  OUT    EFI_HANDLE                     *Device
  );

EFI_BOOTSERVICE11 
EFI_STATUS
EFIAPI
CoreLocateHandleBuffer (
  IN     EFI_LOCATE_SEARCH_TYPE         SearchType,
  IN     EFI_GUID                       *Protocol OPTIONAL,
  IN     VOID                           *SearchKey OPTIONAL,
  IN OUT UINTN                          *NumberHandles,
  OUT    EFI_HANDLE                     **Buffer
  );

EFI_BOOTSERVICE11 
EFI_STATUS
EFIAPI
CoreLocateProtocol (
  IN    EFI_GUID  *Protocol,
  IN    VOID      *Registration OPTIONAL,
  OUT   VOID      **Interface
  );

UINT64
CoreGetHandleDatabaseKey (
  VOID
  );

VOID
CoreConnectHandlesByKey (
  UINT64  Key
  );

EFI_BOOTSERVICE11
EFI_STATUS 
EFIAPI
CoreConnectController (
  IN  EFI_HANDLE                ControllerHandle,
  IN  EFI_HANDLE                *DriverImageHandle    OPTIONAL,
  IN  EFI_DEVICE_PATH_PROTOCOL  *RemainingDevicePath  OPTIONAL,
  IN  BOOLEAN                   Recursive
  );

EFI_BOOTSERVICE11
EFI_STATUS 
EFIAPI
CoreDisconnectController (
  IN EFI_HANDLE  ControllerHandle,
  IN EFI_HANDLE  DriverImageHandle  OPTIONAL,
  IN EFI_HANDLE  ChildHandle        OPTIONAL
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreAllocatePages (
  IN      EFI_ALLOCATE_TYPE       Type,
  IN      EFI_MEMORY_TYPE         MemoryType,
  IN      UINTN                   NumberOfPages,
  IN OUT  EFI_PHYSICAL_ADDRESS    *Memory
  );

EFI_BOOTSERVICE
EFI_STATUS 
EFIAPI
CoreFreePages (
  IN EFI_PHYSICAL_ADDRESS   Memory,
  IN UINTN                  NumberOfPages
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreGetMemoryMap (
  IN OUT UINTN                       *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR       *Desc,
  OUT    UINTN                       *MapKey,
  OUT    UINTN                       *DescriptorSize,
  OUT    UINT32                      *DescriptorVersion
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreAllocatePool (
  IN   EFI_MEMORY_TYPE  PoolType,
  IN   UINTN            Size,
  OUT  VOID             **Buffer
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreFreePool (
  IN VOID      *Buffer
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreLoadImage (
  IN  BOOLEAN                    BootPolicy,
  IN  EFI_HANDLE                 ParentImageHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL   *FilePath,
  IN  VOID                       *SourceBuffer   OPTIONAL,
  IN  UINTN                      SourceSize,
  OUT EFI_HANDLE                 *ImageHandle
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreUnloadImage (
  IN EFI_HANDLE  ImageHandle
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreStartImage (
  IN  EFI_HANDLE  ImageHandle,
  OUT UINTN       *ExitDataSize,
  OUT CHAR16      **ExitData  OPTIONAL
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreExit (
  IN EFI_HANDLE  ImageHandle,
  IN EFI_STATUS  Status,
  IN UINTN       ExitDataSize,
  IN CHAR16      *ExitData  OPTIONAL
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreCreateEvent (
  IN  UINT32               Type,
  IN  EFI_TPL              NotifyTpl,
  IN  EFI_EVENT_NOTIFY     NotifyFunction,
  IN  VOID                 *NotifyContext,
  OUT EFI_EVENT            *pEvent
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreSetTimer (
  IN EFI_EVENT            Event,
  IN EFI_TIMER_DELAY      Type,
  IN UINT64               TriggerTime
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreSignalEvent (
  IN EFI_EVENT            Event
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreWaitForEvent (
  IN  UINTN        NumberOfEvents,
  IN  EFI_EVENT    *UserEvents,
  OUT UINTN        *UserIndex
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreCloseEvent (
  IN EFI_EVENT            Event
  );

EFI_BOOTSERVICE
EFI_STATUS
EFIAPI
CoreCheckEvent (
  IN EFI_EVENT            Event
  );

EFI_STATUS
CoreAddMemorySpace (
  IN EFI_GCD_MEMORY_TYPE   GcdMemoryType,
  IN EFI_PHYSICAL_ADDRESS  BaseAddress,
  IN UINT64                Length,
  IN UINT64                Capabilities
  );

EFI_STATUS
CoreAllocateMemorySpace (
  IN     EFI_GCD_ALLOCATE_TYPE  GcdAllocateType,
  IN     EFI_GCD_MEMORY_TYPE    GcdMemoryType,
  IN     UINTN                  Alignment,
  IN     UINT64                 Length,
  IN OUT EFI_PHYSICAL_ADDRESS   *BaseAddress,
  IN     EFI_HANDLE             ImageHandle,
  IN     EFI_HANDLE             DeviceHandle OPTIONAL
  );

EFI_STATUS
CoreFreeMemorySpace (
  IN EFI_PHYSICAL_ADDRESS  BaseAddress,
  IN UINT64                Length
  );

EFI_STATUS
CoreRemoveMemorySpace (
  IN EFI_PHYSICAL_ADDRESS  BaseAddress,
  IN UINT64                Length
  );

EFI_STATUS
CoreGetMemorySpaceDescriptor (
  IN  EFI_PHYSICAL_ADDRESS             BaseAddress,
  OUT EFI_GCD_MEMORY_SPACE_DESCRIPTOR  *Descriptor
  );

EFI_STATUS
CoreSetMemorySpaceAttributes (
  IN EFI_PHYSICAL_ADDRESS  BaseAddress,
  IN UINT64                Length,
  IN UINT64                Attributes
  );

EFI_STATUS
CoreGetMemorySpaceMap (
  OUT UINTN                            *NumberOfDescriptors,
  OUT EFI_GCD_MEMORY_SPACE_DESCRIPTOR  **MemorySpaceMap
  );

EFI_STATUS
CoreAddIoSpace (
  IN EFI_GCD_IO_TYPE       GcdIoType,
  IN EFI_PHYSICAL_ADDRESS  BaseAddress,
  IN UINT64                Length
  );

EFI_STATUS
CoreAllocateIoSpace (
  IN     EFI_GCD_ALLOCATE_TYPE  GcdAllocateType,
  IN     EFI_GCD_IO_TYPE        GcdIoType,
  IN     UINTN                  Alignment,
  IN     UINT64                 Length,
  IN OUT EFI_PHYSICAL_ADDRESS   *BaseAddress,
  IN     EFI_HANDLE             ImageHandle,
  IN     EFI_HANDLE             DeviceHandle OPTIONAL
  );

EFI_STATUS
CoreFreeIoSpace (
  IN EFI_PHYSICAL_ADDRESS  BaseAddress,
  IN UINT64                Length
  );

EFI_STATUS
CoreRemoveIoSpace (
  IN EFI_PHYSICAL_ADDRESS  BaseAddress,
  IN UINT64                Length
  );

EFI_STATUS
CoreGetIoSpaceDescriptor (
  IN  EFI_PHYSICAL_ADDRESS         BaseAddress,
  OUT EFI_GCD_IO_SPACE_DESCRIPTOR  *Descriptor
  );

EFI_STATUS
CoreGetIoSpaceMap (
  OUT UINTN                        *NumberOfDescriptors,
  OUT EFI_GCD_IO_SPACE_DESCRIPTOR  **IoSpaceMap
  );

EFI_STATUS
CoreDispatcher (
  VOID
  );

EFI_STATUS
CoreSchedule (
  IN  EFI_HANDLE  FirmwareVolumeHandle,
  IN  EFI_GUID    *DriverName
  );

EFI_STATUS
CoreTrust (
  IN  EFI_HANDLE  FirmwareVolumeHandle,
  IN  EFI_GUID    *DriverName
  );

BOOLEAN
CoreGrowBuffer (
  IN OUT EFI_STATUS       *Status,
  IN OUT VOID             **Buffer,
  IN     UINTN            BufferSize
  );

EFI_STATUS
EFIAPI
FwVolDriverInit (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeSectionExtraction (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  );

EFI_STATUS
CoreProcessFirmwareVolume (
  IN  VOID                         *FvHeader,
  IN  UINTN                        Size, 
  OUT EFI_HANDLE                   *FVProtocolHandle
  );

//
//Functions used during debug buils
//
DEBUG_CODE (
  VOID
  CoreDisplayMissingArchProtocols (
    VOID
    );
  VOID
  CoreDisplayDiscoveredNotDispatched (
    VOID
    );
)
#endif
