/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PeiApi.h

Abstract:

  Tiano PEI intrinsic definitions. This includes all Pei Services APIs.

  Peims are passed in a pointer to a pointer to the PEI Services table.
  The PEI Services table contains pointers to the PEI services exported
  by the PEI Core.

--*/

#ifndef _PEI_API_H_
#define _PEI_API_H_

#include "PeiHob.h"
#include "EfiFirmwareFileSystem.h"
#include "EfiFirmwareVolumeHeader.h"

//
// Declare forward referenced data structures
//
EFI_FORWARD_DECLARATION (EFI_PEI_NOTIFY_DESCRIPTOR);
EFI_FORWARD_DECLARATION (EFI_PEI_SERVICES);

#include EFI_PPI_DEFINITION (CpuIo)
#include EFI_PPI_DEFINITION (PciCfg)

//
// PEI Specification Revision information
//
#define PEI_SPECIFICATION_MAJOR_REVISION  0
#define PEI_SPECIFICATION_MINOR_REVISION  91

typedef
EFI_STATUS
(EFIAPI *EFI_PEIM_ENTRY_POINT)(IN EFI_FFS_FILE_HEADER       * FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEIM_NOTIFY_ENTRY_POINT) (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  * NotifyDescriptor,
  IN VOID                       *Ppi
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_VERIFICATION) (
  IN UINTN    SectionAddress
  );

//
// PEI Ppi Services List Descriptors
//
#define EFI_PEI_PPI_DESCRIPTOR_PIC              0x00000001
#define EFI_PEI_PPI_DESCRIPTOR_PPI              0x00000010
#define EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK  0x00000020
#define EFI_PEI_PPI_DESCRIPTOR_NOTIFY_DISPATCH  0x00000040
#define EFI_PEI_PPI_DESCRIPTOR_NOTIFY_TYPES     0x00000060
#define EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST   0x80000000

typedef struct {
  UINTN     Flags;
  EFI_GUID  *Guid;
  VOID      *Ppi;
} EFI_PEI_PPI_DESCRIPTOR;

typedef struct _EFI_PEI_NOTIFY_DESCRIPTOR {
  UINTN                       Flags;
  EFI_GUID                    *Guid;
  EFI_PEIM_NOTIFY_ENTRY_POINT Notify;
} EFI_PEI_NOTIFY_DESCRIPTOR;

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_INSTALL_PPI) (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_PEI_PPI_DESCRIPTOR      * PpiList
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_REINSTALL_PPI) (
  IN EFI_PEI_SERVICES                **PeiServices,
  IN EFI_PEI_PPI_DESCRIPTOR          * OldPpi,
  IN EFI_PEI_PPI_DESCRIPTOR          * NewPpi
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_LOCATE_PPI) (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_GUID                    * Guid,
  IN UINTN                       Instance,
  IN OUT EFI_PEI_PPI_DESCRIPTOR  **PpiDescriptor,
  IN OUT VOID                    **Ppi
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_NOTIFY_PPI) (
  IN EFI_PEI_SERVICES                **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR       * NotifyList
  );

//
// EFI PEI Boot Mode Services
//
typedef
EFI_STATUS
(EFIAPI *EFI_PEI_GET_BOOT_MODE) (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN OUT EFI_BOOT_MODE           * BootMode
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_SET_BOOT_MODE) (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_BOOT_MODE               BootMode
  );

//
// PEI HOB Services
//
typedef
EFI_STATUS
(EFIAPI *EFI_PEI_GET_HOB_LIST) (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN OUT VOID                    **HobList
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_CREATE_HOB) (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN UINT16                      Type,
  IN UINT16                      Length,
  IN OUT VOID                    **Hob
  );

//
// FFS Fw Volume support functions
//
typedef
EFI_STATUS
(EFIAPI *EFI_PEI_FFS_FIND_NEXT_VOLUME) (
  IN EFI_PEI_SERVICES                **PeiServices,
  IN UINTN                           Instance,
  IN OUT EFI_FIRMWARE_VOLUME_HEADER  **FwVolHeader
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_FFS_FIND_NEXT_FILE) (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_FV_FILETYPE             SearchType,
  IN EFI_FIRMWARE_VOLUME_HEADER  * FwVolHeader,
  IN OUT EFI_FFS_FILE_HEADER     **FileHeader
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_FFS_FIND_SECTION_DATA) (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_SECTION_TYPE            SectionType,
  IN EFI_FFS_FILE_HEADER         * FfsFileHeader,
  IN OUT VOID                    **SectionData
  );

//
// Memory Services
//
typedef
EFI_STATUS
(EFIAPI *EFI_PEI_INSTALL_PEI_MEMORY) (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PHYSICAL_ADDRESS       MemoryBegin,
  IN UINT64                     MemoryLength
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_ALLOCATE_PAGES) (

  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_MEMORY_TYPE            MemoryType,
  IN UINTN                      Pages,
  IN OUT EFI_PHYSICAL_ADDRESS   * Memory
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_ALLOCATE_POOL) (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN UINTN                      Size,
  OUT VOID                      **Buffer
  );

typedef
VOID
(EFIAPI *EFI_PEI_COPY_MEM) (
  IN VOID                       *Destination,
  IN VOID                       *Source,
  IN UINTN                      Length
  );

typedef
VOID
(EFIAPI *EFI_PEI_SET_MEM) (
  IN VOID                       *Buffer,
  IN UINTN                      Size,
  IN UINT8                      Value
  );

//
// Status Code
//
//
// Definition of Status Code extended data header
//
//  HeaderSize    The size of the architecture. This is specified to enable
//                the future expansion
//
//  Size          The size of the data in bytes. This does not include the size
//                of the header structure.
//
//  Type          A GUID defining the type of the data
//
//
typedef
EFI_STATUS
(EFIAPI *EFI_REPORT_STATUS_CODE) (
  IN EFI_STATUS_CODE_TYPE     Type,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 * CallerId OPTIONAL,
  IN EFI_STATUS_CODE_DATA     * Data OPTIONAL
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_REPORT_STATUS_CODE) (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN EFI_STATUS_CODE_TYPE     Type,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 * CallerId OPTIONAL,
  IN EFI_STATUS_CODE_DATA     * Data OPTIONAL
  );

//
// PEI Reset
//
typedef
EFI_STATUS
(EFIAPI *EFI_PEI_RESET_SYSTEM) (
  IN EFI_PEI_SERVICES   **PeiServices
  );

//
// EFI PEI Services Table
//
#define PEI_SERVICES_SIGNATURE  0x5652455320494550
#define PEI_SERVICES_REVISION   ((PEI_SPECIFICATION_MAJOR_REVISION << 16) | (PEI_SPECIFICATION_MINOR_REVISION))

typedef struct _EFI_PEI_SERVICES {
  EFI_TABLE_HEADER              Hdr;

  //
  // PPI Functions
  //
  EFI_PEI_INSTALL_PPI           InstallPpi;
  EFI_PEI_REINSTALL_PPI         ReInstallPpi;
  EFI_PEI_LOCATE_PPI            LocatePpi;
  EFI_PEI_NOTIFY_PPI            NotifyPpi;

  //
  // Boot Mode Functions
  //
  EFI_PEI_GET_BOOT_MODE         GetBootMode;
  EFI_PEI_SET_BOOT_MODE         SetBootMode;

  //
  // HOB Functions
  //
  EFI_PEI_GET_HOB_LIST          GetHobList;
  EFI_PEI_CREATE_HOB            CreateHob;

  //
  // Filesystem Functions
  //
  EFI_PEI_FFS_FIND_NEXT_VOLUME  FfsFindNextVolume;
  EFI_PEI_FFS_FIND_NEXT_FILE    FfsFindNextFile;
  EFI_PEI_FFS_FIND_SECTION_DATA FfsFindSectionData;

  //
  // Memory Functions
  //
  EFI_PEI_INSTALL_PEI_MEMORY    InstallPeiMemory;
  EFI_PEI_ALLOCATE_PAGES        AllocatePages;
  EFI_PEI_ALLOCATE_POOL         AllocatePool;
  EFI_PEI_COPY_MEM              CopyMem;
  EFI_PEI_SET_MEM               SetMem;

  //
  // Status Code
  //
  EFI_PEI_REPORT_STATUS_CODE    PeiReportStatusCode;

  //
  // Reset
  //
  EFI_PEI_RESET_SYSTEM          PeiResetSystem;

  //
  // Pointer to PPI interface
  //
  PEI_CPU_IO_PPI                *CpuIo;
  PEI_PCI_CFG_PPI               *PciCfg;

} EFI_PEI_SERVICES;

typedef struct {
  UINTN                   BootFirmwareVolume;
  UINTN                   SizeOfCacheAsRam;
  EFI_PEI_PPI_DESCRIPTOR  *DispatchTable;
} EFI_PEI_STARTUP_DESCRIPTOR;

#endif
