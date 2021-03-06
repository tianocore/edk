/*++

Copyright (c) 2006 - 2009, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  FSVariable.c

Abstract:

  Provide support functions for variable services.

--*/

#include "FSVariable.h"
#include "EfiRuntimeLib.h"
#include "EfiFlashMap.h"
#include "EfiHobLib.h"
#include EFI_ARCH_PROTOCOL_DEFINITION (Variable)
#include EFI_ARCH_PROTOCOL_DEFINITION (VariableWrite)
#include EFI_GUID_DEFINITION (Hob)
#include EFI_GUID_DEFINITION (FlashMapHob)

VARIABLE_STORE_HEADER mStoreHeaderTemplate = {
  VARIABLE_STORE_SIGNATURE,
  VOLATILE_VARIABLE_STORE_SIZE,
  VARIABLE_STORE_FORMATTED,
  VARIABLE_STORE_HEALTHY,
  0,
  0
};

//
// Don't use module globals after the SetVirtualAddress map is signaled
//
VARIABLE_GLOBAL  *mGlobal;

STATIC
VOID
EFIAPI
OnVirtualAddressChange (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );

STATIC
VOID
EFIAPI
OnSimpleFileSystemInstall (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );

STATIC
BOOLEAN
IsValidVariableHeader (
  IN  VARIABLE_HEADER   *Variable
  )
/*++

Routine Description:

  This code checks if variable header is valid or not.

Arguments:
  Variable        Pointer to the Variable Header.

Returns:
  TRUE            Variable header is valid.
  FALSE           Variable header is not valid.

--*/
{
  if (Variable == NULL || Variable->StartId != VARIABLE_DATA) {
    return FALSE;
  }
  
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  //
  // Hardware error record variable needs larger size.
  //
  if ((Variable->Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
    if ((sizeof (VARIABLE_HEADER) + Variable->NameSize + Variable->DataSize) > MAX_HARDWARE_ERROR_VARIABLE_SIZE) {
      return FALSE;
    }
  } else
#endif
  if ((sizeof (VARIABLE_HEADER) + Variable->NameSize + Variable->DataSize) > MAX_VARIABLE_SIZE) {
    return FALSE;
  }

  return TRUE;
}

STATIC
VARIABLE_STORE_STATUS
GetVariableStoreStatus (
  IN VARIABLE_STORE_HEADER *VarStoreHeader
  )
/*++

Routine Description:

  This code gets the current status of Variable Store.

Arguments:

  VarStoreHeader  Pointer to the Variable Store Header.

Returns:

  EfiRaw        Variable store status is raw
  EfiValid      Variable store status is valid
  EfiInvalid    Variable store status is invalid

--*/
{
  if ((VarStoreHeader->Signature == mStoreHeaderTemplate.Signature) &&
      (VarStoreHeader->Format == mStoreHeaderTemplate.Format) &&
      (VarStoreHeader->State == mStoreHeaderTemplate.State)
     ) {
    return EfiValid;
  } else if (VarStoreHeader->Signature == VAR_DEFAULT_VALUE_32 &&
           VarStoreHeader->Size == VAR_DEFAULT_VALUE_32 &&
           VarStoreHeader->Format == VAR_DEFAULT_VALUE &&
           VarStoreHeader->State == VAR_DEFAULT_VALUE
          ) {

    return EfiRaw;
  } else {
    return EfiInvalid;
  }
}

STATIC
UINT8 *
GetVariableDataPtr (
  IN  VARIABLE_HEADER   *Variable
  )
/*++

Routine Description:

  This code gets the pointer to the variable data.

Arguments:

  Variable            Pointer to the Variable Header.

Returns:

  UINT8*              Pointer to Variable Data

--*/
{
  //
  // Be careful about pad size for alignment
  //
  return (UINT8 *) ((UINTN) GET_VARIABLE_NAME_PTR (Variable) + Variable->NameSize + GET_PAD_SIZE (Variable->NameSize));
}

STATIC
VARIABLE_HEADER *
GetNextVariablePtr (
  IN  VARIABLE_HEADER   *Variable
  )
/*++

Routine Description:

  This code gets the pointer to the next variable header.

Arguments:

  Variable              Pointer to the Variable Header.

Returns:

  VARIABLE_HEADER*      Pointer to next variable header.

--*/
{
  if (!IsValidVariableHeader (Variable)) {
    return NULL;
  }
  //
  // Be careful about pad size for alignment
  //
  return (VARIABLE_HEADER *) ((UINTN) GetVariableDataPtr (Variable) + Variable->DataSize + GET_PAD_SIZE (Variable->DataSize));
}

STATIC
VARIABLE_HEADER *
GetEndPointer (
  IN VARIABLE_STORE_HEADER       *VarStoreHeader
  )
/*++

Routine Description:

  This code gets the pointer to the last variable memory pointer byte

Arguments:

  VarStoreHeader        Pointer to the Variable Store Header.

Returns:

  VARIABLE_HEADER*      Pointer to last unavailable Variable Header

--*/
{
  //
  // The end of variable store
  //
  return (VARIABLE_HEADER *) ((UINTN) VarStoreHeader + VarStoreHeader->Size);
}

BOOLEAN
ExistNewerVariable (
  IN  VARIABLE_HEADER         *Variable
  )
/*++

Routine Description:

  Check if exist newer variable when doing reclaim

Arguments:

  Variable                    Pointer to start position

Returns:

  TRUE - Exists another variable, which is newer than the current one
  FALSE  - Doesn't exist another vairable which is newer than the current one

--*/
{
  VARIABLE_HEADER       *NextVariable;
  CHAR16                *VariableName;
  EFI_GUID              *VendorGuid;
  
  VendorGuid   = &Variable->VendorGuid;
  VariableName = GET_VARIABLE_NAME_PTR(Variable);
  
  NextVariable = GetNextVariablePtr (Variable);
  while (IsValidVariableHeader (NextVariable)) {
    if ((NextVariable->State == VAR_ADDED) || (NextVariable->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION))) {
      //
      // If match Guid and Name
      //
      if (EfiCompareGuid (VendorGuid, &NextVariable->VendorGuid)) {
         if (EfiCompareMem (VariableName, GET_VARIABLE_NAME_PTR (NextVariable), EfiStrSize (VariableName)) == 0) {
           return TRUE;
         }
       }
    }
    NextVariable = GetNextVariablePtr (NextVariable);
  }
  return FALSE;
}

STATIC
EFI_STATUS
Reclaim (
  IN  VARIABLE_STORAGE_TYPE StorageType,
  IN  VARIABLE_HEADER       *CurrentVariable OPTIONAL
  )
/*++

Routine Description:

  Variable store garbage collection and reclaim operation

Arguments:

  IsVolatile                  The variable store is volatile or not,
                              if it is non-volatile, need FTW
  CurrentVairable             If it is not NULL, it means not to process
                              current variable for Reclaim.

Returns:

  EFI STATUS

--*/
{
  VARIABLE_HEADER       *Variable;
  VARIABLE_HEADER       *NextVariable;
  VARIABLE_STORE_HEADER *VariableStoreHeader;
  UINT8                 *ValidBuffer;
  UINTN                 ValidBufferSize;
  UINTN                 VariableSize;
  UINT8                 *CurrPtr;
  EFI_STATUS            Status;

  VariableStoreHeader = (VARIABLE_STORE_HEADER *) mGlobal->VariableBase[StorageType];

  //
  // Start Pointers for the variable.
  //
  Variable        = (VARIABLE_HEADER *) (VariableStoreHeader + 1);

  
  //
  // To make the reclaim, here we just allocate a memory that equal to the original memory
  //
  ValidBufferSize = sizeof (VARIABLE_STORE_HEADER) + VariableStoreHeader->Size;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  ValidBufferSize,
                  &ValidBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CurrPtr = ValidBuffer;

  //
  // Copy variable store header
  //
  EfiCopyMem (CurrPtr, VariableStoreHeader, sizeof (VARIABLE_STORE_HEADER));
  CurrPtr += sizeof (VARIABLE_STORE_HEADER);

  //
  // Start Pointers for the variable.
  //
  Variable = (VARIABLE_HEADER *) (VariableStoreHeader + 1);

  
  ValidBufferSize = sizeof (VARIABLE_STORE_HEADER);
  while (IsValidVariableHeader (Variable)) {
    NextVariable = GetNextVariablePtr (Variable);
    //
    // State VAR_ADDED or VAR_IN_DELETED_TRANSITION are to kept,
    // The CurrentVariable, is also saved, as SetVariable may fail due to lack of space
    //
    if (Variable->State == VAR_ADDED) {
      VariableSize = (UINTN) NextVariable - (UINTN) Variable;
      EfiCopyMem (CurrPtr, (UINT8 *) Variable, VariableSize);
      ValidBufferSize += VariableSize;
      CurrPtr += VariableSize;
    } else if (Variable->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION)) {
      //
      // As variables that with the same guid and name may exist in NV due to power failure during SetVariable,
      // we will only save the latest valid one
      //
      if (!ExistNewerVariable(Variable)) {
        VariableSize = (UINTN) NextVariable - (UINTN) Variable;
        EfiCopyMem (CurrPtr, (UINT8 *) Variable, VariableSize);
        //
        // If CurrentVariable == Variable, mark as VAR_IN_DELETED_TRANSITION
        //
        if (Variable != CurrentVariable){
          ((VARIABLE_HEADER *)CurrPtr)->State = VAR_ADDED;
        }
        CurrPtr += VariableSize;
        ValidBufferSize += VariableSize;
      }
    }
    Variable = NextVariable;
  }

  //
  // TODO: cannot restore to original state, basic FTW needed
  //
  Status = mGlobal->VariableStore[StorageType]->Erase (
                                                  mGlobal->VariableStore[StorageType]
                                                  );
  Status = mGlobal->VariableStore[StorageType]->Write (
                                                    mGlobal->VariableStore[StorageType],
                                                    0,
                                                    ValidBufferSize,
                                                    ValidBuffer
                                                    );

  // ASSERT_EFI_ERROR (Status);

  mGlobal->LastVariableOffset[StorageType] = ValidBufferSize;
  gBS->FreePool (ValidBuffer);

  return Status;
}

STATIC
EFI_STATUS
FindVariable (
  IN  CHAR16                  *VariableName,
  IN  EFI_GUID                *VendorGuid,
  OUT VARIABLE_POINTER_TRACK  *PtrTrack
  )
/*++

Routine Description:

  This code finds variable in storage blocks (Volatile or Non-Volatile)

Arguments:

  VariableName                Name of the variable to be found
  VendorGuid                  Vendor GUID to be found.
  PtrTrack                    Variable Track Pointer structure that contains
                              Variable Information.
                              Contains the pointer of Variable header.

Returns:

  EFI_INVALID_PARAMETER       - Invalid parameter
  EFI_SUCCESS                 - Find the specified variable
  EFI_NOT_FOUND               - Not found

--*/
{
  VARIABLE_HEADER         *Variable;
  VARIABLE_STORE_HEADER   *VariableStoreHeader;
  UINTN                   Index;
  VARIABLE_HEADER         *InDeleteVariable;
  UINTN                   InDeleteIndex;
  VARIABLE_HEADER         *InDeleteStartPtr;
  VARIABLE_HEADER         *InDeleteEndPtr;

  if (VariableName[0] != 0 && VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  InDeleteVariable = NULL;
  InDeleteIndex    = (UINTN)-1;
  InDeleteStartPtr = NULL;
  InDeleteEndPtr   = NULL;

  for (Index = 0; Index < MaxType; Index ++) {
    //
    // 0: Non-Volatile, 1: Volatile
    //
    VariableStoreHeader = (VARIABLE_STORE_HEADER *) mGlobal->VariableBase[Index];

    //
    // Start Pointers for the variable.
    // Actual Data Pointer where data can be written.
    //
    Variable = (VARIABLE_HEADER *) (VariableStoreHeader + 1);

    //
    // Find the variable by walk through non-volatile and volatile variable store
    //
    PtrTrack->StartPtr = Variable;
    PtrTrack->EndPtr   = GetEndPointer (VariableStoreHeader);

    while (IsValidVariableHeader (Variable) && (Variable < PtrTrack->EndPtr)) {
      if (Variable->State == VAR_ADDED) {
        if (!EfiAtRuntime () || (Variable->Attributes & EFI_VARIABLE_RUNTIME_ACCESS)) {
          if (VariableName[0] == 0) {
            PtrTrack->CurrPtr = Variable;
            PtrTrack->Type    = (VARIABLE_STORAGE_TYPE) Index;
            return EFI_SUCCESS;
          } else {
            if (EfiCompareGuid (VendorGuid, &Variable->VendorGuid)) {
              if (!EfiCompareMem (VariableName, GET_VARIABLE_NAME_PTR (Variable), EfiStrSize (VariableName))) {
                PtrTrack->CurrPtr = Variable;
                PtrTrack->Type    = (VARIABLE_STORAGE_TYPE) Index;
                return EFI_SUCCESS;
              }
            }
          }
        }
      } else if (Variable->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION)) {
        //
        // VAR_IN_DELETED_TRANSITION should also be checked.
        //
        if (!EfiAtRuntime () || (Variable->Attributes & EFI_VARIABLE_RUNTIME_ACCESS)) {
          if (VariableName[0] == 0) {
            InDeleteVariable = Variable;
            InDeleteIndex    = Index;
            InDeleteStartPtr = PtrTrack->StartPtr;
            InDeleteEndPtr   = PtrTrack->EndPtr;
          } else {
            if (EfiCompareGuid (VendorGuid, &Variable->VendorGuid)) {
              if (!EfiCompareMem (VariableName, GET_VARIABLE_NAME_PTR (Variable), EfiStrSize (VariableName))) {
                InDeleteVariable = Variable;
                InDeleteIndex    = Index;
                InDeleteStartPtr = PtrTrack->StartPtr;
                InDeleteEndPtr   = PtrTrack->EndPtr;
              }
            }
          }
        }
      }

      Variable = GetNextVariablePtr (Variable);
    }
    //
    // While (...)
    //
  }
  //
  // for (...)
  //

  //
  // if VAR_IN_DELETED_TRANSITION found, and VAR_ADDED not found,
  // we return it.
  //
  if (InDeleteVariable != NULL) {
    PtrTrack->CurrPtr  = InDeleteVariable;
    PtrTrack->Type     = (VARIABLE_STORAGE_TYPE) InDeleteIndex;
    PtrTrack->StartPtr = InDeleteStartPtr;
    PtrTrack->EndPtr   = InDeleteEndPtr;
    return EFI_SUCCESS;
  }

  PtrTrack->CurrPtr = NULL;
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
GetVariable (
  IN      CHAR16            *VariableName,
  IN      EFI_GUID          *VendorGuid,
  OUT     UINT32            *Attributes OPTIONAL,
  IN OUT  UINTN             *DataSize,
  OUT     VOID              *Data
  )
/*++

Routine Description:

  This code finds variable in storage blocks (Volatile or Non-Volatile)

Arguments:

  VariableName                    Name of Variable to be found
  VendorGuid                      Variable vendor GUID
  Attributes OPTIONAL             Attribute value of the variable found
  DataSize                        Size of Data found. If size is less than the
                                  data, this value contains the required size.
  Data                            Data pointer

Returns:

  EFI STATUS

--*/
{
  VARIABLE_POINTER_TRACK  Variable;
  UINTN                   VarDataSize;
  EFI_STATUS              Status;

  if (VariableName == NULL || VendorGuid == NULL || DataSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Find existing variable
  //
  Status = FindVariable (VariableName, VendorGuid, &Variable);

  if (Variable.CurrPtr == NULL || EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Get data size
  //
  VarDataSize = Variable.CurrPtr->DataSize;
  if (*DataSize >= VarDataSize) {
    if (Data == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    EfiCopyMem (Data, GetVariableDataPtr (Variable.CurrPtr), VarDataSize);

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
GetNextVariableName (
  IN OUT  UINTN             *VariableNameSize,
  IN OUT  CHAR16            *VariableName,
  IN OUT  EFI_GUID          *VendorGuid
  )
/*++

Routine Description:

  This code Finds the Next available variable

Arguments:

  VariableNameSize            Size of the variable
  VariableName                Pointer to variable name
  VendorGuid                  Variable Vendor Guid

Returns:

  EFI STATUS

--*/
{
  VARIABLE_POINTER_TRACK  Variable;
  UINTN                   VarNameSize;
  EFI_STATUS              Status;

  if (VariableNameSize == NULL || VariableName == NULL || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = FindVariable (VariableName, VendorGuid, &Variable);

  if (Variable.CurrPtr == NULL || EFI_ERROR (Status)) {
    return Status;
  }

  if (VariableName[0] != 0) {
    //
    // If variable name is not NULL, get next variable
    //
    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
  }

  while (TRUE) {
    //
    // The order we find variable is: 1). NonVolatile; 2). Volatile
    // If both volatile and non-volatile variable store are parsed,
    // return not found
    //
    if (Variable.CurrPtr >= Variable.EndPtr || Variable.CurrPtr == NULL) {
      if (Variable.Type == Volatile) {
        //
        // Since we met the end of Volatile storage, we have parsed all the stores.
        //
        return EFI_NOT_FOUND;
      }

      //
      // End of NonVolatile, continue to parse Volatile
      //
      Variable.Type = Volatile;
      Variable.StartPtr = (VARIABLE_HEADER *) ((VARIABLE_STORE_HEADER *) mGlobal->VariableBase[Volatile] + 1);
      Variable.EndPtr   = (VARIABLE_HEADER *) GetEndPointer ((VARIABLE_STORE_HEADER *) mGlobal->VariableBase[Volatile]);

      Variable.CurrPtr = Variable.StartPtr;
      if (!IsValidVariableHeader (Variable.CurrPtr)) {
        continue;
      }
    }
    //
    // Variable is found
    //
    if (IsValidVariableHeader (Variable.CurrPtr) &&
        ((Variable.CurrPtr->State == VAR_ADDED) ||
         (Variable.CurrPtr->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION)))) {
      if (!EfiAtRuntime () || (Variable.CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS)) {
        VarNameSize = Variable.CurrPtr->NameSize;
        if (VarNameSize <= *VariableNameSize) {
          EfiCopyMem (
            VariableName,
            GET_VARIABLE_NAME_PTR (Variable.CurrPtr),
            VarNameSize
            );
          EfiCopyMem (
            VendorGuid,
            &Variable.CurrPtr->VendorGuid,
            sizeof (EFI_GUID)
            );
          Status = EFI_SUCCESS;
        } else {
          Status = EFI_BUFFER_TOO_SMALL;
        }

        *VariableNameSize = VarNameSize;
        return Status;
      }
    }

    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
SetVariable (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes,
  IN UINTN                   DataSize,
  IN VOID                    *Data
  )
/*++

Routine Description:

  This code sets variable in storage blocks (Volatile or Non-Volatile)

Arguments:

  VariableName                    Name of Variable to be found
  VendorGuid                      Variable vendor GUID
  Attributes                      Attribute value of the variable found
  DataSize                        Size of Data found. If size is less than the
                                  data, this value contains the required size.
  Data                            Data pointer

Returns:
  
  EFI_INVALID_PARAMETER           - Invalid parameter
  EFI_SUCCESS                     - Set successfully
  EFI_OUT_OF_RESOURCES            - Resource not enough to set variable
  EFI_NOT_FOUND                   - Not found
  EFI_DEVICE_ERROR                - Variable can not be saved due to hardware failure
  EFI_WRITE_PROTECTED             - Variable is read-only

--*/
{
  VARIABLE_POINTER_TRACK  Variable;
  EFI_STATUS              Status;
  VARIABLE_HEADER         *NextVariable;
  UINTN                   VarNameSize;
  UINTN                   VarNameOffset;
  UINTN                   VarDataOffset;
  UINTN                   VarSize;
  UINT8                   State;
  BOOLEAN                 Reclaimed;
  VARIABLE_STORAGE_TYPE   StorageType;

  Reclaimed = FALSE;
  
  //
  // Check input parameters
  // 

  if (VariableName == NULL || VariableName[0] == 0 || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  //  Make sure if runtime bit is set, boot service bit is set also
  //
  if ((Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == EFI_VARIABLE_RUNTIME_ACCESS) {
    return EFI_INVALID_PARAMETER;
  }
  
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  //
  //  The size of the VariableName, including the Unicode Null in bytes plus
  //  the DataSize is limited to maximum size of MAX_HARDWARE_ERROR_VARIABLE_SIZE
  //  bytes for HwErrRec, and MAX_VARIABLE_SIZE bytes for the others.
  //
  if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
    if ((DataSize > MAX_HARDWARE_ERROR_VARIABLE_SIZE) ||                                                       
        (sizeof (VARIABLE_HEADER) + EfiStrSize (VariableName) + DataSize > MAX_HARDWARE_ERROR_VARIABLE_SIZE)) {
      return EFI_INVALID_PARAMETER;
    }    
  } else {
    if ((DataSize > MAX_VARIABLE_SIZE) ||
        (sizeof (VARIABLE_HEADER) + EfiStrSize (VariableName) + DataSize > MAX_VARIABLE_SIZE)) {
      return EFI_INVALID_PARAMETER;
    }  
  }  
#else  
  //
  //  The size of the VariableName, including the Unicode Null in bytes plus
  //  the DataSize is limited to maximum size of MAX_VARIABLE_SIZE bytes.
  //
  if ((DataSize > MAX_VARIABLE_SIZE) ||
      (sizeof (VARIABLE_HEADER) + EfiStrSize (VariableName) + DataSize > MAX_VARIABLE_SIZE)) {
    return EFI_INVALID_PARAMETER;
  }  
#endif
  //
  // Check whether the input variable is already existed
  //

  Status = FindVariable (VariableName, VendorGuid, &Variable);

  if (Status == EFI_SUCCESS && Variable.CurrPtr != NULL) {  
    //
    // Update/Delete existing variable
    //
    
    if (EfiAtRuntime ()) {              
      //
      // If EfiAtRuntime and the variable is Volatile and Runtime Access,  
      // the volatile is ReadOnly, and SetVariable should be aborted and 
      // return EFI_WRITE_PROTECTED.
      //
      if (Variable.Type == Volatile) {
        return EFI_WRITE_PROTECTED;
      }
      //
      // Only variable have NV attribute can be updated/deleted in Runtime
      //
      if (!(Variable.CurrPtr->Attributes & EFI_VARIABLE_NON_VOLATILE)) {
        return EFI_INVALID_PARAMETER;      
      }
    }
    
    //
    // Setting a data variable with no access, or zero DataSize attributes
    // specified causes it to be deleted.
    //
    if (DataSize == 0 || (Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == 0) {
      //
      // Found this variable in storage
      //
      State = Variable.CurrPtr->State;
      State &= VAR_DELETED;

      Status = mGlobal->VariableStore[Variable.Type]->Write (
                                                        mGlobal->VariableStore[Variable.Type],
                                                        VARIABLE_MEMBER_OFFSET (State, (UINTN) Variable.CurrPtr - (UINTN) Variable.StartPtr),
                                                        sizeof (Variable.CurrPtr->State),
                                                        &State
                                                        );
      //
      // NOTE: Write operation at least can write data to memory cache
      //       Discard file writing failure here.
      //
      return EFI_SUCCESS;
    }
    
    //
    // Found this variable in storage
    // If the variable is marked valid and the same data has been passed in
    // then return to the caller immediately.
    //
    if ((Variable.CurrPtr->DataSize == DataSize) &&
        (EfiCompareMem (Data, GetVariableDataPtr (Variable.CurrPtr), DataSize) == 0)
          ) {
      return EFI_SUCCESS;
    } else if ((Variable.CurrPtr->State == VAR_ADDED) ||
               (Variable.CurrPtr->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION))) {
      //
      // Mark the old variable as in delete transition
      //
      State = Variable.CurrPtr->State;
      State &= VAR_IN_DELETED_TRANSITION;

      Status = mGlobal->VariableStore[Variable.Type]->Write (
                                                        mGlobal->VariableStore[Variable.Type],
                                                        VARIABLE_MEMBER_OFFSET (State, (UINTN) Variable.CurrPtr - (UINTN) Variable.StartPtr),
                                                        sizeof (Variable.CurrPtr->State),
                                                        &State
                                                        );
      //
      // NOTE: Write operation at least can write data to memory cache
      //       Discard file writing failure here.
      //
    }
  } else if (Status == EFI_NOT_FOUND) {
    //
    // Create a new variable
    //  
    
    //
    // Make sure we are trying to create a new variable.
    // Setting a data variable with no access, or zero DataSize attributes means to delete it.    
    //
    if (DataSize == 0 || (Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == 0) {
      return EFI_NOT_FOUND;
    }    
    //
    // Only variable have NV|RT attribute can be created in Runtime
    //
    if (EfiAtRuntime () &&
        (!(Attributes & EFI_VARIABLE_RUNTIME_ACCESS) || !(Attributes & EFI_VARIABLE_NON_VOLATILE))) {
      return EFI_INVALID_PARAMETER;
    }        
    
  } else {
    //
    // Status should be EFI_INVALID_PARAMETER here according to return status of FindVariable().
    //
    return Status;
  } 

  //
  // Function part - create a new variable and copy the data.
  // Both update a variable and create a variable will come here.  
  // We can firstly write all the data in memory, then write them to file
  // This can reduce the times of write operation
  //
  
  NextVariable = (VARIABLE_HEADER *) mGlobal->Scratch;

  NextVariable->StartId     = VARIABLE_DATA;
  NextVariable->Attributes  = Attributes;
  NextVariable->State       = VAR_ADDED;
  NextVariable->Reserved    = 0;
  VarNameOffset             = sizeof (VARIABLE_HEADER);
  VarNameSize               = EfiStrSize (VariableName);
  EfiCopyMem (
    (UINT8 *) ((UINTN) NextVariable + VarNameOffset),
    VariableName,
    VarNameSize
    );
  VarDataOffset = VarNameOffset + VarNameSize + GET_PAD_SIZE (VarNameSize);
  EfiCopyMem (
    (UINT8 *) ((UINTN) NextVariable + VarDataOffset),
    Data,
    DataSize
    );
  EfiCopyMem (&NextVariable->VendorGuid, VendorGuid, sizeof (EFI_GUID));
  //
  // There will be pad bytes after Data, the NextVariable->NameSize and
  // NextVariable->DataSize should not include pad size so that variable
  // service can get actual size in GetVariable
  //
  NextVariable->NameSize  = (UINT32)VarNameSize;
  NextVariable->DataSize  = (UINT32)DataSize;

  //
  // The actual size of the variable that stores in storage should
  // include pad size.
  // VarDataOffset: offset from begin of current variable header
  //
  VarSize = VarDataOffset + DataSize + GET_PAD_SIZE (DataSize);

  StorageType = (Attributes & EFI_VARIABLE_NON_VOLATILE) ? NonVolatile : Volatile;

  if ((UINT32) (VarSize + mGlobal->LastVariableOffset[StorageType]) >
      ((VARIABLE_STORE_HEADER *) mGlobal->VariableBase[StorageType])->Size
      ) {
    if ((StorageType == NonVolatile) && EfiAtRuntime ()) {
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Perform garbage collection & reclaim operation
    //
    Status = Reclaim (StorageType, Variable.CurrPtr);
    if (EFI_ERROR (Status)) {
      //
      // Reclaim error
      // we cannot restore to original state, fetal error, report to user
      //
      DEBUG ((EFI_D_ERROR, "FSVariable: Recalim error (fetal error) - %r\n", Status));
      return Status;
    }
    //
    // If still no enough space, return out of resources
    //
    if ((UINT32) (VarSize + mGlobal->LastVariableOffset[StorageType]) >
        ((VARIABLE_STORE_HEADER *) mGlobal->VariableBase[StorageType])->Size
       ) {
      return EFI_OUT_OF_RESOURCES;
    }

    Reclaimed = TRUE;
  }
  Status = mGlobal->VariableStore[StorageType]->Write (
                                                  mGlobal->VariableStore[StorageType],
                                                  mGlobal->LastVariableOffset[StorageType],
                                                  VarSize,
                                                  NextVariable
                                                  );
  //
  // NOTE: Write operation at least can write data to memory cache
  //       Discard file writing failure here.
  //
  mGlobal->LastVariableOffset[StorageType] += VarSize;

  //
  // Mark the old variable as deleted
  //
  if (!Reclaimed && !EFI_ERROR (Status) && Variable.CurrPtr != NULL) {
    State = Variable.CurrPtr->State;
    State &= VAR_DELETED;

    Status = mGlobal->VariableStore[StorageType]->Write (
                                                    mGlobal->VariableStore[StorageType],
                                                    VARIABLE_MEMBER_OFFSET (State, (UINTN) Variable.CurrPtr - (UINTN) Variable.StartPtr),
                                                    sizeof (Variable.CurrPtr->State),
                                                    &State
                                                    );
    //
    // NOTE: Write operation at least can write data to memory cache
    //       Discard file writing failure here.
    //
  }

  return EFI_SUCCESS;
}

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
EFI_STATUS
EFIAPI
QueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
/*++

Routine Description:

  This code returns information about the EFI variables.

Arguments:

  Attributes                      Attributes bitmask to specify the type of variables
                                  on which to return information.
  MaximumVariableStorageSize      Pointer to the maximum size of the storage space available
                                  for the EFI variables associated with the attributes specified.
  RemainingVariableStorageSize    Pointer to the remaining size of the storage space available
                                  for the EFI variables associated with the attributes specified.
  MaximumVariableSize             Pointer to the maximum size of the individual EFI variables
                                  associated with the attributes specified.

Returns:

  EFI STATUS
  EFI_INVALID_PARAMETER           - An invalid combination of attribute bits was supplied.
  EFI_SUCCESS                     - Query successfully.
  EFI_UNSUPPORTED                 - The attribute is not supported on this platform.

--*/
{
  VARIABLE_HEADER        *Variable;
  VARIABLE_HEADER        *NextVariable;
  UINT64                 VariableSize;
  VARIABLE_STORE_HEADER  *VariableStoreHeader;

  if(MaximumVariableStorageSize == NULL || RemainingVariableStorageSize == NULL || MaximumVariableSize == NULL || Attributes == 0) {
    return EFI_INVALID_PARAMETER;
  }
  
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)  
  if((Attributes & (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_HARDWARE_ERROR_RECORD)) == 0) {
    //
    // Make sure the Attributes combination is supported by the platform.
    //
    return EFI_UNSUPPORTED;  
  }  
#else
  if((Attributes & (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS)) == 0) {
    //
    // Make sure the Attributes combination is supported by the platform.
    //
    return EFI_UNSUPPORTED;  
  } 
#endif  
  else if ((Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == EFI_VARIABLE_RUNTIME_ACCESS) {
    //
    // Make sure if runtime bit is set, boot service bit is set also.
    //
    return EFI_INVALID_PARAMETER;
  } else if (EfiAtRuntime () && !(Attributes & EFI_VARIABLE_RUNTIME_ACCESS)) {
    //
    // Make sure RT Attribute is set if we are in Runtime phase.
    //
    return EFI_INVALID_PARAMETER;
  } 
  
  VariableStoreHeader = (VARIABLE_STORE_HEADER *) mGlobal->VariableBase[
                                (Attributes & EFI_VARIABLE_NON_VOLATILE) ? NonVolatile : Volatile
                                ];
  //
  // Now let's fill *MaximumVariableStorageSize *RemainingVariableStorageSize
  // with the storage size (excluding the storage header size).
  //
  *MaximumVariableStorageSize   = VariableStoreHeader->Size - sizeof (VARIABLE_STORE_HEADER);
  *RemainingVariableStorageSize = VariableStoreHeader->Size - sizeof (VARIABLE_STORE_HEADER);

  //
  // Let *MaximumVariableSize be MAX_VARIABLE_SIZE with the exception of the variable header size.
  //
  *MaximumVariableSize = MAX_VARIABLE_SIZE - sizeof (VARIABLE_HEADER);

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  //
  // Harware error record variable needs larger size.
  //
  if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
    *MaximumVariableSize = MAX_HARDWARE_ERROR_VARIABLE_SIZE - sizeof (VARIABLE_HEADER);
  }
#endif
  
  //
  // Point to the starting address of the variables.
  //
  Variable = (VARIABLE_HEADER *) (VariableStoreHeader + 1);

  //
  // Now walk through the related variable store.
  //
  while (IsValidVariableHeader (Variable) && (Variable < GetEndPointer (VariableStoreHeader))) {
    NextVariable = GetNextVariablePtr (Variable);
    VariableSize = (UINT64) (UINTN) NextVariable - (UINT64) (UINTN) Variable;

    if (EfiAtRuntime ()) {
      //
      // we don't take the state of the variables in mind
      // when calculating RemainingVariableStorageSize,
      // since the space occupied by variables not marked with
      // VAR_ADDED is not allowed to be reclaimed in Runtime.
      //
      *RemainingVariableStorageSize -= VariableSize;
    } else {
      //
      // Only care about Variables with State VAR_ADDED,because
      // the space not marked as VAR_ADDED is reclaimable now.
      //
      if ((Variable->State == VAR_ADDED) ||
          (Variable->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION))) {
        *RemainingVariableStorageSize -= VariableSize;
      }
    }

    //
    // Go to the next one
    //
    Variable = NextVariable;
  }
  
  if (*RemainingVariableStorageSize < sizeof (VARIABLE_HEADER)) {
    *MaximumVariableSize = 0;
  } else if ((*RemainingVariableStorageSize - sizeof (VARIABLE_HEADER)) < *MaximumVariableSize) {
    *MaximumVariableSize = *RemainingVariableStorageSize - sizeof (VARIABLE_HEADER);
  }

  return EFI_SUCCESS;
}
#endif

EFI_DRIVER_ENTRY_POINT (VariableServiceInitialize)

EFI_STATUS
EFIAPI
VariableServiceInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  This function does initialization for variable services

Arguments:

  ImageHandle   - The firmware allocated handle for the EFI image.
  SystemTable   - A pointer to the EFI System Table.

Returns:

  Status code.

  EFI_NOT_FOUND     - Variable store area not found.
  EFI_SUCCESS       - Variable services successfully initialized.

--*/
{
  EFI_STATUS                      Status;
  EFI_HANDLE                      NewHandle;
  VS_DEV                          *Dev;
  VOID                            *HobList;
  VARIABLE_HEADER                 *NextVariable;
  VARIABLE_STORE_HEADER           *VariableStoreHeader;
  EFI_FLASH_MAP_FS_ENTRY_DATA     *FlashMapEntryData;
  EFI_FLASH_SUBAREA_ENTRY         VariableStoreEntry;
  VOID                            *Buffer;
  UINT64                          BaseAddress;
  UINT64                          Length;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR GcdDescriptor;

  EfiInitializeRuntimeDriverLib (ImageHandle, SystemTable, OnVirtualAddressChange);
  Status = gBS->AllocatePool (
                  EfiRuntimeServicesData,
                  (UINTN) sizeof (VARIABLE_GLOBAL),
                  &mGlobal
                  );
  if (EFI_ERROR (Status)) {
    goto Shutdown;
  }

  Status = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);

  if (EFI_ERROR (Status)) {
    goto Shutdown;
  }

  
  for (FlashMapEntryData = NULL; ;) {
    Status = GetNextGuidHob (&HobList, &gEfiFlashMapHobGuid, &Buffer, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }
    FlashMapEntryData = (EFI_FLASH_MAP_FS_ENTRY_DATA *) Buffer;

    //
    // Get the variable store area
    //
    if (FlashMapEntryData->AreaType == EFI_FLASH_AREA_EFI_VARIABLES) {
      break;
    }
  }

  if (EFI_ERROR (Status) || FlashMapEntryData == NULL) {
    Status = EFI_NOT_FOUND;
    goto Shutdown;
  }

  VariableStoreEntry = FlashMapEntryData->Entries[0];

  //
  // Mark the variable storage region of the FLASH as RUNTIME
  //
  BaseAddress = VariableStoreEntry.Base & (~EFI_PAGE_MASK);
  Length      = VariableStoreEntry.Length + (VariableStoreEntry.Base - BaseAddress);
  Length      = (Length + EFI_PAGE_SIZE - 1) & (~EFI_PAGE_MASK);
  Status      = gDS->GetMemorySpaceDescriptor (BaseAddress, &GcdDescriptor);
  if (EFI_ERROR (Status)) {
    Status = EFI_UNSUPPORTED;
    goto Shutdown;
  }
  Status = gDS->SetMemorySpaceAttributes (
                  BaseAddress,
                  Length,
                  GcdDescriptor.Attributes | EFI_MEMORY_RUNTIME
                  );
  if (EFI_ERROR (Status)) {
    Status = EFI_UNSUPPORTED;
    goto Shutdown;
  }
  
  Status = FileStorageConstructor (
             &mGlobal->VariableStore[NonVolatile], 
             &mGlobal->GoVirtualChildEvent[NonVolatile],
             VariableStoreEntry.Base,
             (UINT32) VariableStoreEntry.Length,
             FlashMapEntryData->VolumeId,
             FlashMapEntryData->FilePath
             );
  ASSERT_EFI_ERROR (Status);

  //
  // Volatile Storage
  //
  Status = MemStorageConstructor (
             &mGlobal->VariableStore[Volatile],
             &mGlobal->GoVirtualChildEvent[Volatile],
             VOLATILE_VARIABLE_STORE_SIZE
             );
  ASSERT_EFI_ERROR (Status);

  //
  // Scratch
  //
  Status = gBS->AllocatePool (
                  EfiRuntimeServicesData,
                  VARIABLE_SCRATCH_SIZE,
                  &mGlobal->Scratch
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // 1. NV Storage
  //
  Dev = DEV_FROM_THIS (mGlobal->VariableStore[NonVolatile]);
  VariableStoreHeader = (VARIABLE_STORE_HEADER *) VAR_DATA_PTR (Dev);
  if (GetVariableStoreStatus (VariableStoreHeader) == EfiValid) {
    if (~VariableStoreHeader->Size == 0) {
      VariableStoreHeader->Size = (UINT32) VariableStoreEntry.Length;
    }
  }
  //
  // Calculate LastVariableOffset
  //
  NextVariable = (VARIABLE_HEADER *) (VariableStoreHeader + 1);
  while (IsValidVariableHeader (NextVariable)) {
    NextVariable = GetNextVariablePtr (NextVariable);
  }
  mGlobal->LastVariableOffset[NonVolatile] = (UINTN) NextVariable - (UINTN) VariableStoreHeader;
  mGlobal->VariableBase[NonVolatile] = VariableStoreHeader;

  //
  // Reclaim if remaining space is too small
  //
  if ((VariableStoreHeader->Size - mGlobal->LastVariableOffset[NonVolatile]) < VARIABLE_RECLAIM_THRESHOLD) {
    Status = Reclaim (NonVolatile, NULL);
    if (EFI_ERROR (Status)) {
      //
      // Reclaim error
      // we cannot restore to original state
      //
      DEBUG ((EFI_D_ERROR, "FSVariable: Recalim error (fetal error) - %r\n", Status));
      ASSERT_EFI_ERROR (Status);
    }
  }
  
  //
  // 2. Volatile Storage
  //
  Dev = DEV_FROM_THIS (mGlobal->VariableStore[Volatile]);
  VariableStoreHeader = (VARIABLE_STORE_HEADER *) VAR_DATA_PTR (Dev);
  mGlobal->VariableBase[Volatile] = VAR_DATA_PTR (Dev);  
  mGlobal->LastVariableOffset[Volatile] = sizeof (VARIABLE_STORE_HEADER);
  //
  // init store_header & body in memory.
  //
  mGlobal->VariableStore[Volatile]->Erase (mGlobal->VariableStore[Volatile]);
  mGlobal->VariableStore[Volatile]->Write (
                                   mGlobal->VariableStore[Volatile],
                                   0,
                                   sizeof (VARIABLE_STORE_HEADER),
                                   &mStoreHeaderTemplate
                                   );


  SystemTable->RuntimeServices->GetVariable         = GetVariable;
  SystemTable->RuntimeServices->GetNextVariableName = GetNextVariableName;
  SystemTable->RuntimeServices->SetVariable         = SetVariable;

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  SystemTable->RuntimeServices->QueryVariableInfo   = QueryVariableInfo;
#endif

  //
  // Now install the Variable Runtime Architectural Protocol on a new handle
  //
  NewHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &NewHandle,
                  &gEfiVariableArchProtocolGuid,
                  NULL,
                  &gEfiVariableWriteArchProtocolGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;

Shutdown:
  EfiShutdownRuntimeDriverLib ();
  return Status;
}



STATIC
VOID
EFIAPI
OnVirtualAddressChange (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  UINTN Index;

  for (Index = 0; Index < MaxType; Index++) {
    mGlobal->GoVirtualChildEvent[Index] (Event, mGlobal->VariableStore[Index]);
    EfiConvertPointer (EFI_INTERNAL_POINTER, &mGlobal->VariableStore[Index]);
    EfiConvertPointer (EFI_INTERNAL_POINTER, &mGlobal->VariableBase[Index]);
  }
  EfiConvertPointer (EFI_INTERNAL_POINTER, &mGlobal->Scratch);
  EfiConvertPointer (EFI_INTERNAL_POINTER, &mGlobal);
}
