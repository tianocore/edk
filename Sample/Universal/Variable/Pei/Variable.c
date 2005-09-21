/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Variable.c

Abstract:

  Framework PEIM to provide the Variable functionality

--*/

#include "Tiano.h"
#include "Pei.h"
#include "PeiLib.h"
#include "Variable.h"
#include "EfiHobLib.h"

//
// Module globals
//
static PEI_READ_ONLY_VARIABLE_PPI mVariablePpi = {
  PeiGetVariable,
  PeiGetNextVariableName
};

static EFI_PEI_PPI_DESCRIPTOR     mPpiListVariable = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiReadOnlyVariablePpiGuid,
  &mVariablePpi
};

EFI_GUID                          gEfiVariableIndexTableGuid = EFI_VARIABLE_INDEX_TABLE_GUID;

EFI_PEIM_ENTRY_POINT (PeimInitializeVariableServices)

EFI_STATUS
EFIAPI
PeimInitializeVariableServices (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Provide the functionality of the variable services.

Arguments:
  
  FfsHeadher  - The FFS file header
  PeiServices - General purpose services available to every PEIM.

Returns:

  Status -  EFI_SUCCESS if the interface could be successfully
            installed

--*/
{
  //
  // Publish the variable capability to other modules
  //
  return (**PeiServices).InstallPpi (PeiServices, &mPpiListVariable);

}

INTN
StrCmp (
  IN CHAR16   *s1,
  IN CHAR16   *s2
  )
/*++

Routine Description:

  This function compares two strings

Arguments:
  s1 - The first string for comparison
  s2 - The second string for comparison

Returns:
   
  0 - Two strings are the same
  Others - Two strings are not the same

--*/
{
  while (*s1) {
    if (*s1 != *s2) {
      break;
    }

    s1 += 1;
    s2 += 1;
  }

  return *s1 -*s2;
}

VARIABLE_HEADER *
GetNextVariablePtr (
  IN VARIABLE_HEADER  *Variable
  )
/*++

Routine Description:

  This code checks if variable header is valid or not.

Arguments:
  Variable       Pointer to the Variable Header.

Returns:
  TRUE            Variable header is valid.
  FALSE           Variable header is not valid.

--*/
{
  return (VARIABLE_HEADER *) ((UINTN) GET_VARIABLE_DATA_PTR (Variable) + Variable->DataSize + GET_PAD_SIZE (Variable->DataSize));
}

BOOLEAN
EFIAPI
IsValidVariableHeader (
  IN  VARIABLE_HEADER   *Variable
  )
/*++

Routine Description:

  This code checks if variable header is valid or not.

Arguments:
  Variable              Pointer to the Variable Header.

Returns:
  TRUE            Variable header is valid.
  FALSE           Variable header is not valid.

--*/
{
  if (Variable == NULL ||
      Variable->StartId != VARIABLE_DATA ||
      (sizeof (VARIABLE_HEADER) + Variable->DataSize + Variable->NameSize) > MAX_VARIABLE_SIZE
      ) {
    return FALSE;
  }

  return TRUE;
}

VARIABLE_STORE_STATUS
EFIAPI
GetVariableStoreStatus (
  IN VARIABLE_STORE_HEADER *VarStoreHeader
  )
/*++

Routine Description:

  This code gets the pointer to the variable name.

Arguments:

  VarStoreHeader  Pointer to the Variable Store Header.

Returns:

  EfiRaw        Variable store is raw
  EfiValid      Variable store is valid
  EfiInvalid    Variable store is invalid

--*/
{
  if (VarStoreHeader->Signature == VARIABLE_STORE_SIGNATURE &&
      VarStoreHeader->Format == VARIABLE_STORE_FORMATTED &&
      VarStoreHeader->State == VARIABLE_STORE_HEALTHY
      ) {

    return EfiValid;
  }

  if (VarStoreHeader->Signature == 0xffffffff &&
      VarStoreHeader->Size == 0xffffffff &&
      VarStoreHeader->Format == 0xff &&
      VarStoreHeader->State == 0xff
      ) {

    return EfiRaw;
  } else {
    return EfiInvalid;
  }
}

EFI_STATUS
GetFirstGuidHob (
  IN     VOID      **HobStart,
  IN     EFI_GUID  * Guid,
  OUT    VOID      **Buffer,
  OUT    UINTN     *BufferSize OPTIONAL
  )
/*++

Routine Description:

  This code get the first GUID Hob.

Arguments:

  HobStart    - A pointer to the start of the hobs.
  Guid        - A pointer to the EFI_GUID structure.
  Buffer      - A pointer to the buffer.
  BufferSize  - A pointer to the Buffer's size.

Returns:

  EFI_SUCCESS    - Get it successfully
  EFI_NOT_FOUND  - Fail to find it
 
--*/
{
  EFI_STATUS            Status;
  EFI_PEI_HOB_POINTERS  GuidHob;

  GuidHob.Raw = *HobStart;

  for (Status = EFI_NOT_FOUND; EFI_ERROR (Status);) {

    if (END_OF_HOB_LIST (GuidHob)) {
      return EFI_NOT_FOUND;
    }

    if (GuidHob.Header->HobType == EFI_HOB_TYPE_GUID_EXTENSION) {
      if (((INT32 *) Guid)[0] == ((INT32 *) &GuidHob.Guid->Name)[0] &&
          ((INT32 *) Guid)[1] == ((INT32 *) &GuidHob.Guid->Name)[1] &&
          ((INT32 *) Guid)[2] == ((INT32 *) &GuidHob.Guid->Name)[2] &&
          ((INT32 *) Guid)[3] == ((INT32 *) &GuidHob.Guid->Name)[3]
          ) {
        Status  = EFI_SUCCESS;
        *Buffer = (VOID *) ((UINT8 *) (&GuidHob.Guid->Name) + sizeof (EFI_GUID));
        if (BufferSize != NULL) {
          *BufferSize = GuidHob.Header->HobLength - sizeof (EFI_HOB_GUID_TYPE);
        }
      }
    }

    GuidHob.Raw = GET_NEXT_HOB (GuidHob);
  }

  return Status;
}

EFI_STATUS
CompareWithValidVariable (
  IN  VARIABLE_HEADER         *Variable,
  IN  CHAR16                  *VariableName,
  IN  EFI_GUID                *VendorGuid,
  OUT VARIABLE_POINTER_TRACK  *PtrTrack
  )
/*++

Routine Description:

  This function compares a variable with variable entries in database

Arguments:

  Variable       - Pointer to the variable in our database
  VariableName   - Name of the variable to compare to 'Variable'
  VendorGuid     - GUID of the variable to compare to 'Variable'
  PtrTrack       - Variable Track Pointer structure that contains
                   Variable Information.

Returns:

  EFI_SUCCESS    - Found match variable
  EFI_NOT_FOUND  - Variable not found
 
--*/
{
  if (VariableName[0] == 0) {
    PtrTrack->CurrPtr = Variable;
    return EFI_SUCCESS;
  } else {
    //
    // Don't use CompareGuid function here for performance reasons.
    // Instead we compare the GUID a UINT32 at a time and branch
    // on the first failed comparison.
    //
    if ((((INT32 *) VendorGuid)[0] == ((INT32 *) &Variable->VendorGuid)[0]) &&
        (((INT32 *) VendorGuid)[1] == ((INT32 *) &Variable->VendorGuid)[1]) &&
        (((INT32 *) VendorGuid)[2] == ((INT32 *) &Variable->VendorGuid)[2]) &&
        (((INT32 *) VendorGuid)[3] == ((INT32 *) &Variable->VendorGuid)[3])
        ) {
      if (!StrCmp (VariableName, GET_VARIABLE_NAME_PTR (Variable))) {
        PtrTrack->CurrPtr = Variable;
        return EFI_SUCCESS;
      }
    }
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
FindVariable (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN  CHAR16                  *VariableName,
  IN  EFI_GUID                *VendorGuid,
  OUT VARIABLE_POINTER_TRACK  *PtrTrack
  )
/*++

Routine Description:

  This code finds variable in storage blocks (Non-Volatile)

Arguments:

  PeiServices    - General purpose services available to every PEIM.
  VariableName   - Name of the variable to be found
  VendorGuid     - Vendor GUID to be found.
  PtrTrack       - Variable Track Pointer structure that contains
                   Variable Information.

Returns:

  EFI_SUCCESS      - Variable found successfully
  EFI_NOT_FOUND    - Variable not found
  EFI_INVALID_PARAMETER  - Invalid variable name

--*/
{
  PEI_FLASH_MAP_PPI       *FlashMapPpi;
  EFI_FLASH_SUBAREA_ENTRY *VariableStoreEntry;
  UINT32                  NumEntries;

  VARIABLE_STORE_HEADER   *VariableStoreHeader;
  VARIABLE_HEADER         *Variable;

  EFI_STATUS              Status;

  EFI_PEI_HOB_POINTERS    Hob;
  VARIABLE_HEADER         *MaxIndex;
  VARIABLE_INDEX_TABLE    *IndexTable;
  UINT32                  Count;

  if (VariableName != 0 && VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // First, Let's go through index table in HOB
  //
  Status = (*PeiServices)->GetHobList (PeiServices, &(Hob.Raw));

  ASSERT_PEI_ERROR (PeiServices, Status);

  //
  // No Variable Address equals zero, so 0 as initial value is safe.
  //
  MaxIndex = 0;

  if (EFI_ERROR (GetFirstGuidHob (&(Hob.Raw), &gEfiVariableIndexTableGuid, &IndexTable, NULL))) {
    PeiBuildHobGuid (PeiServices, &gEfiVariableIndexTableGuid, sizeof (VARIABLE_INDEX_TABLE), &(Hob.Raw));
    IndexTable              = (VARIABLE_INDEX_TABLE *) ((UINT8 *) &Hob.Guid->Name + sizeof (EFI_GUID));

    IndexTable->Length      = 0;
    IndexTable->StartPtr    = NULL;
    IndexTable->EndPtr      = NULL;
    IndexTable->GoneThrough = 0;
  } else {
    for (Count = 0; Count < IndexTable->Length; Count++)
    {
#if ALIGNMENT <= 1
      MaxIndex = (VARIABLE_HEADER *) (IndexTable->Index[Count] + ((UINT32) IndexTable->StartPtr & 0xFFFF0000));
#else
#if ALIGNMENT >= 4
          MaxIndex = (VARIABLE_HEADER *) (UINTN) ((((UINT32)IndexTable->Index[Count]) << 2) + ((UINT32)(UINTN)IndexTable->StartPtr & 0xFFFC0000) );       
#endif
#endif
      if (CompareWithValidVariable (MaxIndex, VariableName, VendorGuid, PtrTrack) == EFI_SUCCESS) {
        PtrTrack->StartPtr  = IndexTable->StartPtr;
        PtrTrack->EndPtr    = IndexTable->EndPtr;

        return EFI_SUCCESS;
      }
    }

    if (IndexTable->GoneThrough) {
      return EFI_NOT_FOUND;
    }
  }
  //
  // If not found in HOB, then let's start from the MaxIndex we've found.
  //
  if (MaxIndex != NULL) {
    Variable = GetNextVariablePtr (MaxIndex);
  } else {
    if (IndexTable->StartPtr || IndexTable->EndPtr) {
      Variable = IndexTable->StartPtr;
    } else {
      //
      // Locate FlashMap PPI
      //
      Status = (**PeiServices).LocatePpi (
                                PeiServices,
                                &gPeiFlashMapPpiGuid,
                                0,
                                NULL,
                                &FlashMapPpi
                                );
      ASSERT_PEI_ERROR (PeiServices, Status);

      //
      // Get flash area info for variables
      //
      Status = FlashMapPpi->GetAreaInfo (
                              PeiServices,
                              FlashMapPpi,
                              EFI_FLASH_AREA_EFI_VARIABLES,
                              NULL,
                              &NumEntries,
                              &VariableStoreEntry
                              );

      //
      //  Currently only one non-volatile variable store is supported
      //
      if (NumEntries != 1) {
        return EFI_UNSUPPORTED;
      }

      VariableStoreHeader = (VARIABLE_STORE_HEADER *) (UINTN) (VariableStoreEntry->Base);

      if (GetVariableStoreStatus (VariableStoreHeader) != EfiValid) {
        return EFI_UNSUPPORTED;
      }

      if (~VariableStoreHeader->Size == 0) {
        return EFI_NOT_FOUND;
      }
      //
      // Find the variable by walk through non-volatile variable store
      //
      IndexTable->StartPtr  = (VARIABLE_HEADER *) (VariableStoreHeader + 1);
      IndexTable->EndPtr    = (VARIABLE_HEADER *) ((UINTN) VariableStoreHeader + VariableStoreHeader->Size);

      //
      // Start Pointers for the variable.
      // Actual Data Pointer where data can be written.
      //
      Variable = IndexTable->StartPtr;
    }
  }
  //
  // Find the variable by walk through non-volatile variable store
  //
  PtrTrack->StartPtr  = IndexTable->StartPtr;
  PtrTrack->EndPtr    = IndexTable->EndPtr;

  while (IsValidVariableHeader (Variable) && (Variable <= IndexTable->EndPtr)) {
    if (Variable->State == VAR_ADDED) {
      //
      // Record Variable in VariableIndex HOB
      //
      if (IndexTable->Length < VARIABLE_INDEX_TABLE_VOLUME)
      {
#if ALIGNMENT <= 1
        IndexTable->Index[IndexTable->Length++] = (UINT16) (UINT32) Variable;
#else
#if ALIGNMENT >= 4
            IndexTable->Index[IndexTable->Length++] = (UINT16) (((UINT32)(UINTN) Variable) >> 2);
#endif
#endif
      }

      if (CompareWithValidVariable (Variable, VariableName, VendorGuid, PtrTrack) == EFI_SUCCESS) {
        return EFI_SUCCESS;
      }
    }

    Variable = GetNextVariablePtr (Variable);
  }
  //
  // If gone through the VariableStore, that means we never find in Firmware any more.
  //
  if (IndexTable->Length < VARIABLE_INDEX_TABLE_VOLUME) {
    IndexTable->GoneThrough = 1;
  }

  PtrTrack->CurrPtr = NULL;

  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
PeiGetVariable (
  IN EFI_PEI_SERVICES             **PeiServices,
  IN CHAR16                       *VariableName,
  IN EFI_GUID                     * VendorGuid,
  OUT UINT32                      *Attributes OPTIONAL,
  IN OUT UINTN                    *DataSize,
  OUT VOID                        *Data
  )
/*++

Routine Description:

  Provide the read variable functionality of the variable services.

Arguments:

  PeiServices - General purpose services available to every PEIM.

  VariableName     - The variable name

  VendorGuid       - The vendor's GUID

  Attributes       - Pointer to the attribute

  DataSize         - Size of data

  Data             - Pointer to data

Returns:

  EFI_SUCCESS           - The interface could be successfully installed

  EFI_NOT_FOUND         - The variable could not be discovered

  EFI_BUFFER_TOO_SMALL  - The caller buffer is not large enough

--*/
{
  VARIABLE_POINTER_TRACK  Variable;
  UINTN                   VarDataSize;
  EFI_STATUS              Status;

  if (VariableName == NULL || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Find existing variable
  //
  Status = FindVariable (PeiServices, VariableName, VendorGuid, &Variable);

  if (Variable.CurrPtr == NULL || Status != EFI_SUCCESS) {
    return Status;
  }
  //
  // Get data size
  //
  VarDataSize = Variable.CurrPtr->DataSize;
  if (*DataSize >= VarDataSize) {
    (*PeiServices)->CopyMem (Data, GET_VARIABLE_DATA_PTR (Variable.CurrPtr), VarDataSize);

    if (Attributes != NULL) {
      *Attributes = Variable.CurrPtr->Attributes;
    }

    *DataSize = VarDataSize;
    return EFI_SUCCESS;
  } else {
    *DataSize = VarDataSize;
    return EFI_BUFFER_TOO_SMALL;
  }
}

EFI_STATUS
EFIAPI
PeiGetNextVariableName (
  IN EFI_PEI_SERVICES             **PeiServices,
  IN OUT UINTN                    *VariableNameSize,
  IN OUT CHAR16                   *VariableName,
  IN OUT EFI_GUID                 *VendorGuid
  )
/*++

Routine Description:

  Provide the get next variable functionality of the variable services.

Arguments:

  PeiServices        - General purpose services available to every PEIM.
  VariabvleNameSize  - The variable name's size.
  VariableName       - A pointer to the variable's name.
  VendorGuid         - A pointer to the EFI_GUID structure.

  VariableNameSize - Size of the variable name

  VariableName     - The variable name

  VendorGuid       - The vendor's GUID

Returns:

  EFI_SUCCESS - The interface could be successfully installed

  EFI_NOT_FOUND - The variable could not be discovered

--*/
{
  VARIABLE_POINTER_TRACK  Variable;
  UINTN                   VarNameSize;
  EFI_STATUS              Status;

  if (VariableName == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = FindVariable (PeiServices, VariableName, VendorGuid, &Variable);

  if (Variable.CurrPtr == NULL || Status != EFI_SUCCESS) {
    return Status;
  }

  if (VariableName[0] != 0) {
    //
    // If variable name is not NULL, get next variable
    //
    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
  }

  while (!(Variable.CurrPtr >= Variable.EndPtr || Variable.CurrPtr == NULL)) {
    if (IsValidVariableHeader (Variable.CurrPtr)) {
      if (Variable.CurrPtr->State == VAR_ADDED) {
        VarNameSize = (UINTN) Variable.CurrPtr->NameSize;
        if (VarNameSize <= *VariableNameSize) {
          (*PeiServices)->CopyMem (VariableName, GET_VARIABLE_NAME_PTR (Variable.CurrPtr), VarNameSize);

          (*PeiServices)->CopyMem (VendorGuid, &Variable.CurrPtr->VendorGuid, sizeof (EFI_GUID));

          Status = EFI_SUCCESS;
        } else {
          Status = EFI_BUFFER_TOO_SMALL;
        }

        *VariableNameSize = VarNameSize;
        return Status;
        //
        // Variable is found
        //
      } else {
        Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
      }
    } else {
      break;
    }
  }

  return EFI_NOT_FOUND;
}
