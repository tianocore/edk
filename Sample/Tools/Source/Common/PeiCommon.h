/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PeiCommon.h

Abstract:

  Basic architecture specific definitions of PEIM structures.

--*/

#ifndef _EFI_PEI_COMMON_H
#define _EFI_PEI_COMMON_H

#include "Efi2WinNt.h"
#include "CorePeim.h"

#define EFI_IA32_INSTRUCTION_SET    0x00
#define EFI_IA64_INSTRUCTION_SET    0x01

//
// IA32 PEI structures
//

#pragma pack (1)

typedef struct {
  EFI_GUID            Guid;
  EFI_PEIM_VERSION    Version;
  UINT16              InstructionSet;
  UINT32              Flags;
  UINT32              Pass1Entry;
  UINT32              Pass2Entry;
  UINT32              ExportTable;
  UINT32              ImportTable;
  UINT32              RelocationInfo;
  UINT32              AuthenticationInfo;
  UINT32              ExtensionTable;
} EFI_PEIM_HEADER_IA32;

typedef struct {
  PPI_FLAGS       Flags;
  EFI_GUID        Guid;
  UINT32          PpiOffset;
} EFI_EXPORT_TABLE_ENTRY_IA32;

typedef struct {
  PPI_FLAGS       Flags;
  EFI_GUID        Guid;
  UINT32          PhysicalAddr;
} EFI_IMPORT_TABLE_ENTRY_IA32;

//
// Intel?Itanium(TM) PEI structures
//

typedef struct {
  EFI_GUID            Guid;
  EFI_PEIM_VERSION    Version;
  UINT64              InstructionSet;
  UINT64              Flags;
  UINT64              Pass1Entry;
  UINT64              Pass2Entry;
  UINT64              ExportTable;
  UINT64              ImportTable;
  UINT64              RelocationInfo;
  UINT64              AuthenticationInfo;
  UINT64              ExtensionTable;
} EFI_PEIM_HEADER_IA64;

typedef struct {
  PPI_FLAGS       Flags;
  UINT16          Reserved[3];
  EFI_GUID        Guid;
  UINT64          PpiOffset;
} EFI_EXPORT_TABLE_ENTRY_IA64;

typedef struct {
  PPI_FLAGS       Flags;
  UINT16          Reserved[3];
  EFI_GUID        Guid;
  UINT64          PhysicalAddr;
} EFI_IMPORT_TABLE_ENTRY_IA64;

//
// Common between ISAs
//

typedef struct {
  EFI_GUID            Guid;
  EFI_PEIM_VERSION    Version;
  UINT16              InstructionSet;
} EFI_PEIM_HEADER_COMMON;

typedef struct {
  PPI_FLAGS       Flags;
} EFI_EXPORT_TABLE_ENTRY_COMMON;

typedef struct {
  PPI_FLAGS       Flags;
} EFI_IMPORT_TABLE_ENTRY_COMMON;

#pragma pack ()

typedef union _EFI_PEIM_HEADER_ISA {
  EFI_PEIM_HEADER_IA32          *Ia32PeimHeader;
  EFI_PEIM_HEADER_IA64          *Ia64PeimHeader;
  EFI_PEIM_HEADER_COMMON        *PeimHeader;
} EFI_PEIM_HEADER_ISA;

typedef union _EXPORT_TABLE_ENTRY_ISA {
  EFI_EXPORT_TABLE_ENTRY_IA32   *Ia32ExportEntry;
  EFI_EXPORT_TABLE_ENTRY_IA64   *Ia64ExportEntry;
  EFI_EXPORT_TABLE_ENTRY_COMMON *ExportEntry;
} EFI_EXPORT_TABLE_ENTRY_ISA;

typedef union _IMPORT_TABLE_ENTRY_ISA {
  EFI_IMPORT_TABLE_ENTRY_IA32   *Ia32ImportEntry;
  EFI_IMPORT_TABLE_ENTRY_IA64   *Ia64ImportEntry;
  EFI_IMPORT_TABLE_ENTRY_COMMON *ImportEntry;
} EFI_IMPORT_TABLE_ENTRY_ISA;

#endif
