/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PeiCore.h

Abstract:

  Definition of Pei Core Structures and Services

Revision History

--*/

#ifndef _PEICORE_H_
#define _PEICORE_H_

#include "EfiImage.h"
#include "Pei.h"
#include "PeiHob.h"

#ifdef EFI64
#include "SalApi.h"
#endif

#include "EfiCommonLib.h"

#include EFI_PPI_DEFINITION (FindFv)
#include EFI_PPI_DEFINITION (DxeIpl)
#include EFI_PPI_DEFINITION (StatusCode)
#include EFI_PPI_DEFINITION (Security)


#ifdef  EFI_NT_EMULATOR
#include EFI_PPI_DEFINITION (NtPeiLoadFile)
#endif

//
//Build private HOB to PEI core to transfer old NEM-range data to new NEM-range
//
#define EFI_PEI_CORE_PRIVATE_GUID  \
  {0xd641a0f5, 0xcb7c, 0x4846, 0xa3, 0x80, 0x1d, 0x01, 0xb4, 0xd9, 0xe3, 0xb9}

//
// Pei Core private data structures
//
typedef union _PEI_PPI_LIST_POINTERS {
  EFI_PEI_PPI_DESCRIPTOR    *Ppi;
  EFI_PEI_NOTIFY_DESCRIPTOR *Notify;
  VOID                  *Raw;
} PEI_PPI_LIST_POINTERS;

#define PEI_STACK_SIZE 0x20000

#define MAX_PPI_DESCRIPTORS 64

typedef struct {
  INTN                    PpiListEnd;
  INTN                    NotifyListEnd;
  INTN                    DispatchListEnd;
  INTN                    LastDispatchedInstall;
  INTN                    LastDispatchedNotify;
  PEI_PPI_LIST_POINTERS   PpiListPtrs[MAX_PPI_DESCRIPTORS];
} PEI_PPI_DATABASE;

typedef struct {
  UINT8                       CurrentPeim;
  UINT8                       CurrentFv;
  UINT32                      DispatchedPeimBitMap;
  UINT32                      PreviousPeimBitMap;
  EFI_FFS_FILE_HEADER         *CurrentPeimAddress;
  EFI_FIRMWARE_VOLUME_HEADER  *CurrentFvAddress;
  EFI_FIRMWARE_VOLUME_HEADER  *BootFvAddress;
  EFI_FIND_FV_PPI             *FindFv;
} PEI_CORE_DISPATCH_DATA;


//
// Pei Core private data structure instance
//

#define PEI_CORE_HANDLE_SIGNATURE  EFI_SIGNATURE_32('P','e','i','C')

typedef struct{
  UINTN                              Signature;
  EFI_PEI_SERVICES                   *PS;     // Point to ServiceTableShadow
  PEI_PPI_DATABASE                   PpiData;
  PEI_CORE_DISPATCH_DATA             DispatchData;
  EFI_PEI_HOB_POINTERS               HobList;
  BOOLEAN                            SwitchStackSignal;
  BOOLEAN                            PeiMemoryInstalled;
  EFI_PHYSICAL_ADDRESS               StackBase;
  UINT64                             StackSize;
  VOID                               *BottomOfCarHeap;
  VOID                               *TopOfCarHeap;
  VOID                               *CpuIo;
  PEI_SECURITY_PPI                   *PrivateSecurityPpi;
  EFI_PEI_SERVICES                   ServiceTableShadow;

PEI_DEBUG_CODE (
  UINTN                              SizeOfCacheAsRam;
  VOID                               *MaxTopOfCarHeap;
)
 } PEI_CORE_INSTANCE;

//
// Pei Core Instance Data Macros
//

#define PEI_CORE_INSTANCE_FROM_PS_THIS(a) \
  PEI_CR(a, PEI_CORE_INSTANCE, PS, PEI_CORE_HANDLE_SIGNATURE)

typedef
EFI_STATUS
(EFIAPI *PEI_MAIN_ENTRY_POINT) (
    IN EFI_PEI_STARTUP_DESCRIPTOR  *PeiStartupDescriptor
  );

typedef
EFI_STATUS
(EFIAPI *PEI_CORE_ENTRY_POINT) (
   IN EFI_PEI_STARTUP_DESCRIPTOR  *PeiStartupDescriptor,
   IN PEI_CORE_INSTANCE           *OldCoreData
   );

//
// Union of temporarily used function pointers (to save stack space)
//
typedef union {
  PEI_CORE_ENTRY_POINT         PeiCore;
  EFI_PEIM_ENTRY_POINT         PeimEntry;
  EFI_PEIM_NOTIFY_ENTRY_POINT  PeimNotifyEntry;
  EFI_DXE_IPL_PPI              *DxeIpl;
  EFI_PEI_PPI_DESCRIPTOR       *PpiDescriptor;
  EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor;
  VOID                         *Raw;
} PEI_CORE_TEMP_POINTERS;


//
// Main PEI entry
//
EFI_STATUS
PeiMain (
  IN EFI_PEI_STARTUP_DESCRIPTOR  *PeiStartupDescriptor
  );

EFI_STATUS
PeiCore (
  IN EFI_PEI_STARTUP_DESCRIPTOR  *PeiStartupDescriptor,
  IN PEI_CORE_INSTANCE           *OldCoreData
  );

//
// Dispatcher support functions
//

EFI_STATUS
PeimDispatchReadiness (
  IN EFI_PEI_SERVICES   **PeiServices,
  IN VOID               *DependencyExpression,
  IN OUT BOOLEAN        *Runnable
  );

EFI_STATUS
PeiDispatcher (
  IN EFI_PEI_STARTUP_DESCRIPTOR  *PeiStartupDescriptor,
  IN PEI_CORE_INSTANCE           *PrivateData,
  IN PEI_CORE_DISPATCH_DATA      *DispatchData
  ) ;

VOID
InitializeDispatcherData (
  IN EFI_PEI_SERVICES             **PeiServices,
  IN PEI_CORE_INSTANCE            *OldCoreData,
  IN EFI_PEI_STARTUP_DESCRIPTOR   *PeiStartupDescriptor
  );


EFI_STATUS
FindNextPeim (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_FIRMWARE_VOLUME_HEADER  *FwVolHeader,
  IN OUT EFI_FFS_FILE_HEADER     **PeimFileHeader
  );

BOOLEAN
Dispatched (
  IN UINT8  CurrentPeim,
  IN UINT32 DispatchedPeimBitMap
  );

VOID
SetDispatched (
  IN EFI_PEI_SERVICES   **PeiServices,
  IN UINT8              CurrentPeim,
  OUT UINT32            *DispatchedPeimBitMap
  );

BOOLEAN
DepexSatisfied (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN  VOID             *CurrentPeimAddress
  );

VOID
SwitchCoreStacks (
  IN VOID  *EntryPoint,
  IN UINTN Parameter1,
  IN UINTN Parameter2,
  IN VOID  *NewStack
  );

#ifdef EFI64
  //
  // In Ipf we should make special changes for the PHIT pointers to support
  // recovery boot in cache mode.
  //
#define  SWITCH_TO_CACHE_MODE(CoreData)  SwitchToCacheMode(CoreData)
#define  CACHE_MODE_ADDRESS_MASK         0x7FFFFFFFFFFFFFFF
VOID
SwitchToCacheMode (
  IN PEI_CORE_INSTANCE           *CoreData
);

#else

#define  SWITCH_TO_CACHE_MODE(CoreData)

#endif

//
// PPI support functions
//
VOID
InitializePpiServices (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CORE_INSTANCE   *OldCoreData
  );

VOID
ConvertPpiPointers (
  IN EFI_PEI_SERVICES              **PeiServices,
  IN EFI_HOB_HANDOFF_INFO_TABLE    *OldHandOffHob,
  IN EFI_HOB_HANDOFF_INFO_TABLE    *NewHandOffHob
  );

EFI_STATUS
EFIAPI
PeiInstallPpi (
  IN EFI_PEI_SERVICES        **PeiServices,
  IN EFI_PEI_PPI_DESCRIPTOR  *PpiList
  );

EFI_STATUS
EFIAPI
PeiReInstallPpi (
  IN EFI_PEI_SERVICES        **PeiServices,
  IN EFI_PEI_PPI_DESCRIPTOR  *OldPpi,
  IN EFI_PEI_PPI_DESCRIPTOR  *NewPpi
  );

EFI_STATUS
EFIAPI
PeiLocatePpi (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_GUID                    *Guid,
  IN UINTN                       Instance,
  IN OUT EFI_PEI_PPI_DESCRIPTOR  **PpiDescriptor,
  IN OUT VOID                    **Ppi
  );

EFI_STATUS
EFIAPI
PeiNotifyPpi (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyList
  );

VOID
ProcessNotifyList (
  IN EFI_PEI_SERVICES    **PeiServices
  );

VOID
DispatchNotify (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN UINTN               NotifyType,
  IN INTN                InstallStartIndex,
  IN INTN                InstallStopIndex,
  IN INTN                NotifyStartIndex,
  IN INTN                NotifyStopIndex
  );

//
// Boot mode support functions
//
EFI_STATUS
EFIAPI
PeiGetBootMode (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN OUT EFI_BOOT_MODE *BootMode
  );

EFI_STATUS
EFIAPI
PeiSetBootMode (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN EFI_BOOT_MODE     BootMode
  );

//
// Security support functions
//
VOID
InitializeSecurityServices (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN PEI_CORE_INSTANCE *OldCoreData
  );

EFI_STATUS
VerifyFv (
  IN EFI_FIRMWARE_VOLUME_HEADER  *CurrentFvAddress
  );

EFI_STATUS
VerifyPeim (
  IN EFI_PEI_SERVICES     **PeiServices,
  IN EFI_FFS_FILE_HEADER  *CurrentPeimAddress
  );

EFI_STATUS
EFIAPI
PeiGetHobList (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN OUT VOID          **HobList
  );

EFI_STATUS
EFIAPI
PeiCreateHob (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN UINT16            Type,
  IN UINT16            Length,
  IN OUT VOID          **Hob
  );

EFI_STATUS
PeiCoreBuildHobHandoffInfoTable (
  IN EFI_BOOT_MODE         BootMode,
  IN EFI_PHYSICAL_ADDRESS  MemoryBegin,
  IN UINT64                MemoryLength
  );


//
// FFS Fw Volume support functions
//
EFI_STATUS
EFIAPI
PeiFfsFindNextFile (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN UINT8                       SearchType,
  IN EFI_FIRMWARE_VOLUME_HEADER  *FwVolHeader,
  IN OUT EFI_FFS_FILE_HEADER     **FileHeader
  );

EFI_STATUS
EFIAPI
PeiFfsFindSectionData (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_SECTION_TYPE            SectionType,
  IN EFI_FFS_FILE_HEADER         *FfsFileHeader,
  IN OUT VOID                    **SectionData
  );

EFI_STATUS
EFIAPI
PeiFvFindNextVolume (
  IN EFI_PEI_SERVICES                **PeiServices,
  IN UINTN                           Instance,
  IN OUT EFI_FIRMWARE_VOLUME_HEADER  **FwVolHeader
  );

//
// Memory support functions
//
VOID
InitializeMemoryServices (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_PEI_STARTUP_DESCRIPTOR  *PeiStartupDescriptor,
  IN PEI_CORE_INSTANCE           *OldCoreData
  );

EFI_STATUS
EFIAPI
PeiInstallPeiMemory (
  IN EFI_PEI_SERVICES      **PeiServices,
  IN EFI_PHYSICAL_ADDRESS  MemoryBegin,
  IN UINT64                MemoryLength
  );

EFI_STATUS
EFIAPI
PeiAllocatePages (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_MEMORY_TYPE            MemoryType,
  IN UINTN                      Pages,
  OUT EFI_PHYSICAL_ADDRESS      *Memory
  );

EFI_STATUS
EFIAPI
PeiAllocatePool (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN UINTN                      Size,
  OUT VOID                      **Buffer
  );

VOID
EFIAPI
PeiCoreCopyMem (
  IN VOID                       *Destination,
  IN VOID                       *Source,
  IN UINTN                      Length
  );

VOID
EFIAPI
PeiCoreSetMem (
  IN VOID                       *Buffer,
  IN UINTN                      Size,
  IN UINT8                      Value
  );

EFI_STATUS
PeiLoadImage (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_FFS_FILE_HEADER         *PeimFileHeader,
  OUT VOID                       **EntryPoint
  );


EFI_STATUS
EFIAPI
PeiReportStatusCode (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN EFI_STATUS_CODE_TYPE     CodeType,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 *CallerId,
  IN EFI_STATUS_CODE_DATA     *Data OPTIONAL
  );


EFI_STATUS
EFIAPI
PeiCoreResetSystem (
  IN EFI_PEI_SERVICES   **PeiServices
  );

#endif
