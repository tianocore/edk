/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ParsePeim.h

Abstract:

  These functions assist in parsing PEIM files.

--*/

#ifndef _EFI_PARSE_PEIM_H
#define _EFI_PARSE_PEIM_H

#include "Efi2WinNt.h"
#include "EfiFirmwareFileSystem.h"
#include "CorePeim.h"
#include "PeiCommon.h"

//
// Function declarations
//
EFI_STATUS
FindPeimHeader (
  IN EFI_FFS_FILE_HEADER          *FfsHeader,
  OUT EFI_PEIM_HEADER_ISA         *PeimHeader
  )
;

/*++

Routine Description:

  Locates the PEIM_HEADER within a PEIM file.

Arguments:

  FfsHeader       Pointer to an FFS file.
  PeimHeader      Storage for the pointer to the PEIM_HEADER.

Returns:

  EFI_SUCCESS             PEIM_HEADER found and returned.
  EFI_INVALID_PARAMETER   One of the input parameters was NULL.

--*/
EFI_STATUS
FfsFindExportTable (
  IN EFI_FFS_FILE_HEADER            *FfsHeader,
  OUT EFI_EXPORT_TABLE_ENTRY_ISA    *ExportTableEntry
  )
;

/*++

Routine Description:

  Given an FFS file of type PEIM, it returns a pointer to the first entry in 
  the export table, or NULL if no export table exists in this PEIM.

Arguments:

  FfsHeader         Pointer to the FFS file of type PEIM.
  ExportTableEntry  Pointer to storage for the return value.

Returns:

  EFI_SUCCESS             The export table entry points to the first element
                          of the export table or NULL if there is no table.
  EFI_INVALID_PARAMETER   One of the input parameters was a NULL pointer.
  EFI_ABORTED             Error parsing the file.

--*/
EFI_STATUS
PeimFindExportTable (
  IN EFI_PEIM_HEADER_ISA          *PeimHeader,
  OUT EFI_EXPORT_TABLE_ENTRY_ISA  *ExportTableEntry
  )
;

/*++

Routine Description:

  Given a PEIM file, it returns a pointer to the first entry in 
  the export table, or NULL if no export table exists in this PEIM.

Arguments:

  PeimHeader        Pointer to the PEIM file.
  ExportTableEntry  Pointer to storage for the return value.

Returns:

  EFI_SUCCESS             The export table entry points to the first element
                          of the export table or NULL if there is no table.
  EFI_INVALID_PARAMETER   One of the input parameters was a NULL pointer.
  EFI_ABORTED             Error parsing the file.

--*/
EFI_STATUS
FfsFindImportTable (
  IN EFI_FFS_FILE_HEADER            *FfsHeader,
  OUT EFI_IMPORT_TABLE_ENTRY_ISA    *ImportTableEntry
  )
;

/*++

Routine Description:

  Given an FFS file of type PEIM, it returns a pointer to the first entry in 
  the import table, or NULL if no import table exists in this PEIM.

Arguments:

  FfsHeader         Pointer to the FFS file of type PEIM.
  ImportTableEntry  Pointer to storage for the return value.

Returns:

  EFI_SUCCESS             The import table entry points to the first element
                          of the import table or NULL if there is no table.
  EFI_INVALID_PARAMETER   One of the input parameters was a NULL pointer.
  EFI_ABORTED             Error parsing the file.

--*/
EFI_STATUS
PeimFindImportTable (
  IN EFI_PEIM_HEADER_ISA            *PeimHeader,
  OUT EFI_IMPORT_TABLE_ENTRY_ISA    *ImportTableEntry
  )
;

/*++

Routine Description:

  Given a PEIM file, it returns a pointer to the first entry in 
  the import table, or NULL if no import table exists in this PEIM.

Arguments:

  PeimHeader        Pointer to the PEIM file.
  ImportTableEntry  Pointer to storage for the return value.

Returns:

  EFI_SUCCESS             The import table entry points to the first element
                          of the import table or NULL if there is no table.
  EFI_INVALID_PARAMETER   One of the input parameters was a NULL pointer.
  EFI_ABORTED             Error parsing the file.

--*/
EFI_STATUS
FindImportTableEntry (
  IN EFI_FFS_FILE_HEADER          *FfsImage,
  IN EFI_GUID                     *ImportGuid,
  OUT EFI_IMPORT_TABLE_ENTRY_ISA  *ImportTableEntry
  )
;

/*++

Routine Description:

  This function searches a FFS file for a specific import table entry.

Arguments:

  FfsImage            The file so search.
  ImportGuid          The GUID to look for.
  ImportTableEntry    The return pointer.

Returns:

  EFI_SUCCESS             The function completed successfully.
  EFI_INVALID_PARAMETER   One of the input parameters was NULL.

--*/
EFI_STATUS
DeterminePeimIsa (
  IN EFI_FFS_FILE_HEADER          *FfsImage,
  OUT UINT16                      *InstructionSet
  )
;

/*++

Routine Description:

  This function determines the instruction set of a PEIM FFS file.
  The return value is the EFI instruction set, currently 
  EFI_IA32_INSTRUCTION_SET or EFI_IA64_INSTRUCTION_SET.

Arguments:

  FfsImage          The FFS file to examine.
  InstructionSet    The output ISA

Returns:

  EFI_SUCCESS 
  EFI_INVALID_PARAMETER   One of the input parameters was NULL.
  EFI_ABORTED             Not a valid FFS or PEIM.

--*/
#endif
