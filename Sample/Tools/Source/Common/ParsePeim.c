/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ParsePeim.c

Abstract:

  This contains code for parsing PEIM files.

--*/

#include "ParsePeim.h"
#include "FvLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Function implementations
//
EFI_STATUS
FfsFindExportTable (
  IN EFI_FFS_FILE_HEADER            *FfsHeader,
  OUT EFI_EXPORT_TABLE_ENTRY_ISA    *ExportTableEntry
  )
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
{
  EFI_PEIM_HEADER_ISA PeimHeader;
  EFI_STATUS          Status;

  //
  // Check for invalid argument
  //
  if (FfsHeader == NULL || ExportTableEntry == NULL) {
    printf ("ERROR: Invalid parameter to FindExportTable.\n");
    return EFI_INVALID_PARAMETER;
  }
  //
  // Retrieve a pointer to the PEIM_HEADER
  //
  Status = FindPeimHeader (FfsHeader, &PeimHeader);
  if (EFI_ERROR (Status)) {
    printf ("ERROR: FindExportTable could not find the PEIM header.\n");
    return EFI_ABORTED;
  }
  //
  // Find the export table in the peim
  //
  Status = PeimFindExportTable (&PeimHeader, ExportTableEntry);

  return Status;
}

EFI_STATUS
PeimFindExportTable (
  IN EFI_PEIM_HEADER_ISA          *PeimHeader,
  OUT EFI_EXPORT_TABLE_ENTRY_ISA  *ExportTableEntry
  )
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
{
  //
  // Check for invalid argument
  //
  if (PeimHeader == NULL || ExportTableEntry == NULL) {
    printf ("ERROR: Invalid parameter to PeimFindExportTable.\n");
    return EFI_INVALID_PARAMETER;
  }
  //
  // Find the export table offset and update the return pointer to the export table.
  //
  switch (PeimHeader->PeimHeader->InstructionSet) {
  case EFI_IA32_INSTRUCTION_SET:
    if (PeimHeader->Ia32PeimHeader->ExportTable == 0) {
      ExportTableEntry->Ia32ExportEntry = 0;
    } else {
      //
      //  Fixed - warning C4133: '=' : incompatible types - from 'struct _EFI_IMPORT_TABLE_ENTRY_IA32 *' to 'struct _EFI_EXPORT_TABLE_ENTRY_IA32 *'
      //
      ExportTableEntry->Ia32ExportEntry = (EFI_EXPORT_TABLE_ENTRY_IA32 *) ((UINTN) PeimHeader->PeimHeader + PeimHeader->Ia32PeimHeader->ExportTable);
    }
    break;

  case EFI_IA64_INSTRUCTION_SET:
    if (PeimHeader->Ia64PeimHeader->ExportTable == 0) {
      ExportTableEntry->Ia64ExportEntry = 0;
    } else {
      //
      //  Fixed - warning C4133: '=' : incompatible types - from 'struct _EFI_IMPORT_TABLE_ENTRY_IA64 *' to 'struct _EFI_EXPORT_TABLE_ENTRY_IA64 *'
      //
      ExportTableEntry->Ia64ExportEntry = (EFI_EXPORT_TABLE_ENTRY_IA64 *) (UINTN) ((UINTN) PeimHeader->PeimHeader + PeimHeader->Ia64PeimHeader->ExportTable);
    }
    break;

  default:
    printf ("ERROR: Unrecognized instruction set in a PEIM_HEADER.\n");
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
FfsFindImportTable (
  IN EFI_FFS_FILE_HEADER            *FfsHeader,
  OUT EFI_IMPORT_TABLE_ENTRY_ISA    *ImportTableEntry
  )
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
{
  EFI_PEIM_HEADER_ISA PeimHeader;
  EFI_STATUS          Status;

  //
  // Check for invalid argument
  //
  if (FfsHeader == NULL || ImportTableEntry == NULL) {
    printf ("ERROR: Invalid parameter to FindImportTable.\n");
    return EFI_INVALID_PARAMETER;
  }
  //
  // Retrieve a pointer to the PEIM_HEADER
  //
  Status = FindPeimHeader (FfsHeader, &PeimHeader);
  if (EFI_ERROR (Status)) {
    printf ("ERROR: FindImportTable could not find the PEIM header.\n");
    return EFI_ABORTED;
  }
  //
  // Find the import table in the peim
  //
  Status = PeimFindImportTable (&PeimHeader, ImportTableEntry);

  return Status;
}

EFI_STATUS
PeimFindImportTable (
  IN EFI_PEIM_HEADER_ISA            *PeimHeader,
  OUT EFI_IMPORT_TABLE_ENTRY_ISA    *ImportTableEntry
  )
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
{
  //
  // Check for invalid argument
  //
  if (PeimHeader == NULL || ImportTableEntry == NULL) {
    printf ("ERROR: Invalid parameter to FindImportTable.\n");
    return EFI_INVALID_PARAMETER;
  }
  //
  // Find the import table offset and update the return pointer to the import table.
  //
  switch (PeimHeader->PeimHeader->InstructionSet) {
  case EFI_IA32_INSTRUCTION_SET:
    if (PeimHeader->Ia32PeimHeader->ImportTable == 0) {
      ImportTableEntry->Ia32ImportEntry = 0;
    } else {
      //
      // original      ImportTableEntry->Ia32ImportEntry = (EFI_IMPORT_TABLE_ENTRY_IA32*) ((UINT32)PeimHeader->PeimHeader + PeimHeader->Ia32PeimHeader->ImportTable);
      //
      ImportTableEntry->Ia32ImportEntry = (EFI_IMPORT_TABLE_ENTRY_IA32 *) ((UINTN) PeimHeader->PeimHeader + PeimHeader->Ia32PeimHeader->ImportTable);
    }
    break;

  case EFI_IA64_INSTRUCTION_SET:
    if (PeimHeader->Ia64PeimHeader->ImportTable == 0) {
      ImportTableEntry->Ia64ImportEntry = 0;
    } else {
      //
      // original       ImportTableEntry->Ia64ImportEntry = (EFI_IMPORT_TABLE_ENTRY_IA64*) (UINTN) ((UINT64)PeimHeader->PeimHeader + PeimHeader->Ia64PeimHeader->ImportTable);
      //
      ImportTableEntry->Ia64ImportEntry = (EFI_IMPORT_TABLE_ENTRY_IA64 *) (UINTN) ((UINTN) PeimHeader->PeimHeader + PeimHeader->Ia64PeimHeader->ImportTable);
    }
    break;

  default:
    printf ("ERROR: Unrecognized instruction set in a PEIM_HEADER.\n");
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
FindPeimHeader (
  IN EFI_FFS_FILE_HEADER  *FfsHeader,
  OUT EFI_PEIM_HEADER_ISA *PeimHeader
  )
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
{
  EFI_FILE_SECTION_POINTER  PicSection;
  EFI_STATUS                Status;

  //
  // Validate input parameters
  //
  if (FfsHeader == NULL || PeimHeader == NULL) {
    printf ("ERROR: Invalid parameter passed to the FindPeimHeader function.\n");
    return EFI_INVALID_PARAMETER;
  }
  //
  // Verify that we have a PEIM file
  //
  if (FfsHeader->Type != EFI_FV_FILETYPE_PEIM) {
    printf ("ERROR: FFS file passed to FindPeimHeader is not a PEIM file.\n");
    return EFI_INVALID_PARAMETER;
  }
  //
  // Find PIC section
  //
  Status = GetSectionByType (FfsHeader, EFI_SECTION_PIC, 1, &PicSection);
  if (Status == EFI_NOT_FOUND) {
    printf ("ERROR: Could not find PIC section, cannot fixup PEIM.\n");
    return Status;
  } else if (EFI_ERROR (Status)) {
    printf ("ERROR: Could not parse the sections in the FFS file, cannot be fixed up.\n");
    return Status;
  }

  PeimHeader->PeimHeader = (EFI_PEIM_HEADER_COMMON *) ((UINT8 *) PicSection.PicSection + sizeof (EFI_PIC_SECTION));

  return EFI_SUCCESS;
}

EFI_STATUS
FindImportTableEntry (
  IN EFI_FFS_FILE_HEADER              *FfsImage,
  IN EFI_GUID                         *ImportGuid,
  OUT EFI_IMPORT_TABLE_ENTRY_ISA      *ImportTableEntry
  )
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
  EFI_ABORTED             Operation aborted.
  EFI_NOT_FOUND           Not found.
--*/
{
  EFI_PEIM_HEADER_ISA PeimHeader;
  EFI_STATUS          Status;

  //
  // Check parameters
  //
  if (FfsImage == NULL || ImportGuid == NULL || ImportTableEntry == NULL) {
    printf ("ERROR: NULL pointer passed to FindImportTableEntry.\n");
    return EFI_INVALID_PARAMETER;
  }
  //
  // Retrieve a pointer to the PEIM_HEADER
  //
  Status = FindPeimHeader (FfsImage, &PeimHeader);
  if (EFI_ERROR (Status)) {
    printf ("ERROR: FindImportTableEntry could not find the PEIM header.\n");
    return EFI_ABORTED;
  }
  //
  // Find the import table offset and update the return pointer to the import table.
  //
  switch (PeimHeader.PeimHeader->InstructionSet) {
  case EFI_IA32_INSTRUCTION_SET:
    //
    // Find the import table
    //
    if (PeimHeader.Ia32PeimHeader->ImportTable == 0) {
      //
      // No import table, return a failure
      //
      return EFI_NOT_FOUND;

    } else {
      //
      // Get the first entry.
      //
      ImportTableEntry->Ia32ImportEntry = (EFI_IMPORT_TABLE_ENTRY_IA32 *) (((UINT8 *) PeimHeader.PeimHeader) + PeimHeader.Ia32PeimHeader->ImportTable);
    }

    while (ImportTableEntry->Ia32ImportEntry != NULL) {
      //
      // If our entry is found, we are done (good)
      //
      if (memcmp (&ImportTableEntry->Ia32ImportEntry->Guid, ImportGuid, sizeof (EFI_GUID)) == 0) {
        return EFI_SUCCESS;
      }
      //
      // If the last entry is found, we are done (bad)
      //
      if (ImportTableEntry->Ia32ImportEntry->Flags & IMPORT_FLAG_LAST_ENTRY) {
        break;
      }
      //
      // Increment our table entry
      //
      ImportTableEntry->Ia32ImportEntry++;;
    }
    //
    // Could not find our entry
    //
    ImportTableEntry->ImportEntry = NULL;
    return EFI_NOT_FOUND;

  case EFI_IA64_INSTRUCTION_SET:
    //
    // Get the import table
    //
    if (PeimHeader.Ia64PeimHeader->ImportTable == 0) {
      //
      // No import table, return an error
      //
      ImportTableEntry->ImportEntry = NULL;
      return EFI_NOT_FOUND;
    } else {
      //
      // Get the first entry in the import table
      //
      ImportTableEntry->Ia64ImportEntry = (EFI_IMPORT_TABLE_ENTRY_IA64 *) (((UINT8 *) PeimHeader.PeimHeader) + PeimHeader.Ia64PeimHeader->ImportTable);
    }

    while (ImportTableEntry->Ia64ImportEntry != NULL) {
      //
      // If our entry is found, we are done (good)
      //
      if (memcmp (&ImportTableEntry->Ia64ImportEntry->Guid, ImportGuid, sizeof (EFI_GUID)) == 0) {
        return EFI_SUCCESS;
      }
      //
      // If the last entry is found, we are done (bad)
      //
      if (ImportTableEntry->Ia64ImportEntry->Flags & IMPORT_FLAG_LAST_ENTRY) {
        break;
      }
      //
      // Increment our table entry
      //
      ImportTableEntry->Ia64ImportEntry++;
    }
    //
    // Could not find our entry
    //
    ImportTableEntry->ImportEntry = NULL;
    return EFI_NOT_FOUND;

  default:
    printf ("ERROR: Unrecognized instruction set in a PEIM_HEADER.\n");
    return EFI_ABORTED;
  }
}

EFI_STATUS
DeterminePeimIsa (
  IN EFI_FFS_FILE_HEADER  *FfsImage,
  OUT UINT16              *InstructionSet
  )
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
{
  EFI_PEIM_HEADER_ISA PeimHeader;

  //
  // Verify input
  //
  if (FfsImage == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Make sure the input file is a PEIM.
  //
  if (FfsImage->Type != EFI_FV_FILETYPE_PEIM) {
    return EFI_ABORTED;
  }
  //
  // Find the PEIM header
  //
  if (EFI_ERROR (FindPeimHeader (FfsImage, &PeimHeader))) {
    return EFI_ABORTED;
  }
  //
  // Return the instruction set
  //
  *InstructionSet = PeimHeader.PeimHeader->InstructionSet;
  return EFI_SUCCESS;
}
