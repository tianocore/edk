/*++

Copyright (c) 2004 - 2009, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    EmuVariable.c

Abstract:

Revision History

--*/

#include "Variable.h"

//
// Don't use module globals after the SetVirtualAddress map is signaled
//
ESAL_VARIABLE_GLOBAL  *mVariableModuleGlobal;

VARIABLE_STORE_STATUS
GetVariableStoreStatus (
  IN VARIABLE_STORE_HEADER *VarStoreHeader
  )
/*++

Routine Description:

  This code gets the pointer to the variable name.

Arguments:

  VarStoreHeader  Pointer to the Variable Store Header.

Returns:

  EfiHealthy    Variable store is healthy
  EfiRaw        Variable store is raw
  EfiInvalid    Variable store is invalid

--*/
{
  if (VarStoreHeader->Signature == VARIABLE_STORE_SIGNATURE &&
      VarStoreHeader->Format == VARIABLE_STORE_FORMATTED &&
      VarStoreHeader->State == VARIABLE_STORE_HEALTHY
      ) {

    return EfiValid;
  } else if (VarStoreHeader->Signature == 0xffffffff &&
           VarStoreHeader->Size == 0xffffffff &&
           VarStoreHeader->Format == 0xff &&
           VarStoreHeader->State == 0xff
          ) {

    return EfiRaw;
  } else {
    return EfiInvalid;
  }
}

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
  if (Variable->StartId != VARIABLE_DATA) {
    return NULL;
  }
  //
  // Be careful about pad size for alignment
  //
  return (UINT8 *) ((UINTN) GET_VARIABLE_NAME_PTR (Variable) + Variable->NameSize + GET_PAD_SIZE (Variable->NameSize));
}

VARIABLE_HEADER *
GetNextVariablePtr (
  IN  VARIABLE_HEADER   *Variable
  )
/*++

Routine Description:

  This code gets the pointer to the next variable header.

Arguments:

  Variable                  Pointer to the Variable Header.

Returns:

  VARIABLE_HEADER*      Pointer to next variable header.

--*/
{
  VARIABLE_HEADER *VarHeader;

  if (Variable->StartId != VARIABLE_DATA) {
    return NULL;
  }
  //
  // Be careful about pad size for alignment
  //
  VarHeader = (VARIABLE_HEADER *) (GetVariableDataPtr (Variable) + Variable->DataSize + GET_PAD_SIZE (Variable->DataSize));

  if (VarHeader->StartId != VARIABLE_DATA) {
    return NULL;
  }

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  if ((VarHeader->Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
    if ((sizeof (VARIABLE_HEADER) + VarHeader->DataSize + VarHeader->NameSize) > MAX_HARDWARE_ERROR_VARIABLE_SIZE) {
      return NULL;
    }
  } else
#endif
  if ((sizeof (VARIABLE_HEADER) + VarHeader->DataSize + VarHeader->NameSize) > MAX_VARIABLE_SIZE) {
    return NULL;
  }

  return VarHeader;
}

VARIABLE_HEADER *
GetEndPointer (
  IN VARIABLE_STORE_HEADER       *VolHeader
  )
/*++

Routine Description:

  This code gets the pointer to the last variable memory pointer byte

Arguments:

  Variable                  Pointer to the Variable Header.

Returns:

  VARIABLE_HEADER*      Pointer to last unavailable Variable Header

--*/
{
  //
  // The end of variable store
  //
  return (VARIABLE_HEADER *) ((UINTN) VolHeader + VolHeader->Size);
}

EFI_STATUS
FindVariable (
  IN  CHAR16                  *VariableName,
  IN  EFI_GUID                *VendorGuid,
  OUT VARIABLE_POINTER_TRACK  *PtrTrack,
  IN  VARIABLE_GLOBAL         *Global
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
  Global                      VARIABLE_GLOBAL pointer

Returns:

  EFI STATUS

--*/
{
  VARIABLE_HEADER       *Variable[2];
  VARIABLE_STORE_HEADER *VariableStoreHeader[2];
  UINTN                 Index;

  //
  // 0: Non-Volatile, 1: Volatile
  //
  VariableStoreHeader[0]  = (VARIABLE_STORE_HEADER *) ((UINTN) Global->NonVolatileVariableBase);
  VariableStoreHeader[1]  = (VARIABLE_STORE_HEADER *) ((UINTN) Global->VolatileVariableBase);

  //
  // Start Pointers for the variable.
  // Actual Data Pointer where data can be written.
  //
  Variable[0] = (VARIABLE_HEADER *) (VariableStoreHeader[0] + 1);
  Variable[1] = (VARIABLE_HEADER *) (VariableStoreHeader[1] + 1);

  if (VariableName[0] != 0 && VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Find the variable by walk through non-volatile and volatile variable store
  //
  for (Index = 0; Index < 2; Index++) {
    PtrTrack->StartPtr  = (VARIABLE_HEADER *) (VariableStoreHeader[Index] + 1);
    PtrTrack->EndPtr    = GetEndPointer (VariableStoreHeader[Index]);

    while ((Variable[Index] != NULL) && (Variable[Index] <= GetEndPointer (VariableStoreHeader[Index]))) {
      if (Variable[Index]->StartId == VARIABLE_DATA && Variable[Index]->State == VAR_ADDED) {
        if (!(EfiAtRuntime () && !(Variable[Index]->Attributes & EFI_VARIABLE_RUNTIME_ACCESS))) {
          if (VariableName[0] == 0) {
            PtrTrack->CurrPtr   = Variable[Index];
            PtrTrack->Volatile  = (BOOLEAN) Index;
            return EFI_SUCCESS;
          } else {
            if (EfiCompareGuid (VendorGuid, &Variable[Index]->VendorGuid)) {
              if (!EfiCompareMem (VariableName, GET_VARIABLE_NAME_PTR (Variable[Index]), EfiStrSize (VariableName))) {
                PtrTrack->CurrPtr   = Variable[Index];
                PtrTrack->Volatile  = (BOOLEAN) Index;
                return EFI_SUCCESS;
              }
            }
          }
        }
      }

      Variable[Index] = GetNextVariablePtr (Variable[Index]);
    }
  }
  PtrTrack->CurrPtr = NULL;
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
GetVariable (
  IN      CHAR16            *VariableName,
  IN      EFI_GUID          * VendorGuid,
  OUT     UINT32            *Attributes OPTIONAL,
  IN OUT  UINTN             *DataSize,
  OUT     VOID              *Data,
  IN      VARIABLE_GLOBAL   * Global,
  IN      UINT32            Instance
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
  Global                          Pointer to VARIABLE_GLOBAL structure
  Instance                        Instance of the Firmware Volume.

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
  Status = FindVariable (VariableName, VendorGuid, &Variable, Global);

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
  IN OUT  EFI_GUID          *VendorGuid,
  IN      VARIABLE_GLOBAL   *Global,
  IN      UINT32            Instance
  )
/*++

Routine Description:

  This code Finds the Next available variable

Arguments:

  VariableNameSize            Size of the variable
  VariableName                Pointer to variable name
  VendorGuid                  Variable Vendor Guid
  Global                      VARIABLE_GLOBAL structure pointer.
  Instance                    FV instance

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

  Status = FindVariable (VariableName, VendorGuid, &Variable, Global);

  if (Variable.CurrPtr == NULL || EFI_ERROR (Status)) {
    return Status;
  }

  while (TRUE) {
    if (VariableName[0] != 0) {
      //
      // If variable name is not NULL, get next variable
      //
      Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
    }
    //
    // If both volatile and non-volatile variable store are parsed,
    // return not found
    //
    if (Variable.CurrPtr >= Variable.EndPtr || Variable.CurrPtr == NULL) {
      Variable.Volatile = (BOOLEAN) (Variable.Volatile ^ ((BOOLEAN) 0x1));
      if (Variable.Volatile) {
        Variable.StartPtr = (VARIABLE_HEADER *) ((UINTN) (Global->VolatileVariableBase + sizeof (VARIABLE_STORE_HEADER)));
        Variable.EndPtr = (VARIABLE_HEADER *) GetEndPointer ((VARIABLE_STORE_HEADER *) ((UINTN) Global->VolatileVariableBase));
      } else {
        return EFI_NOT_FOUND;
      }

      Variable.CurrPtr = Variable.StartPtr;
      if (Variable.CurrPtr->StartId != VARIABLE_DATA) {
        continue;
      }
    }
    //
    // Variable is found
    //
    if (Variable.CurrPtr->StartId == VARIABLE_DATA && Variable.CurrPtr->State == VAR_ADDED) {
      if (!(EfiAtRuntime () && !(Variable.CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS))) {
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
  IN VOID                    *Data,
  IN VARIABLE_GLOBAL         *Global,
  IN UINTN                   *VolatileOffset,
  IN UINTN                   *NonVolatileOffset,
  IN UINT32                  Instance
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
  Global                          Pointer to VARIABLE_GLOBAL structure
  VolatileOffset                  The offset of last volatile variable
  NonVolatileOffset               The offset of last non-volatile variable
  Instance                        Instance of the Firmware Volume.

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
  
  Status = FindVariable (VariableName, VendorGuid, &Variable, Global);

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
      if (Variable.Volatile) {
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
      Variable.CurrPtr->State &= VAR_DELETED;
      return EFI_SUCCESS;
    }

    //
    // If the variable is marked valid and the same data has been passed in
    // then return to the caller immediately.
    //
    if (Variable.CurrPtr->DataSize == DataSize &&
        !EfiCompareMem (Data, GetVariableDataPtr (Variable.CurrPtr), DataSize)
          ) {
      return EFI_SUCCESS;
    } else if (Variable.CurrPtr->State == VAR_ADDED) {
      //
      // Mark the old variable as in delete transition
      //
      Variable.CurrPtr->State &= VAR_IN_DELETED_TRANSITION;
    }
    
  } else if (Status == EFI_NOT_FOUND) {

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
  //
  
  VarNameOffset = sizeof (VARIABLE_HEADER);
  VarNameSize   = EfiStrSize (VariableName);
  VarDataOffset = VarNameOffset + VarNameSize + GET_PAD_SIZE (VarNameSize);
  VarSize       = VarDataOffset + DataSize + GET_PAD_SIZE (DataSize);

  if (Attributes & EFI_VARIABLE_NON_VOLATILE) {
    if ((UINT32) (VarSize +*NonVolatileOffset) >
          ((VARIABLE_STORE_HEADER *) ((UINTN) (Global->NonVolatileVariableBase)))->Size
          ) {
      return EFI_OUT_OF_RESOURCES;
    }

    NextVariable        = (VARIABLE_HEADER *) (UINT8 *) (*NonVolatileOffset + (UINTN) Global->NonVolatileVariableBase);
    *NonVolatileOffset  = *NonVolatileOffset + VarSize;
  } else {
    if ((UINT32) (VarSize +*VolatileOffset) >
          ((VARIABLE_STORE_HEADER *) ((UINTN) (Global->VolatileVariableBase)))->Size
          ) {
      return EFI_OUT_OF_RESOURCES;
    }

    NextVariable    = (VARIABLE_HEADER *) (UINT8 *) (*VolatileOffset + (UINTN) Global->VolatileVariableBase);
    *VolatileOffset = *VolatileOffset + VarSize;
  }

  NextVariable->StartId     = VARIABLE_DATA;
  NextVariable->Attributes  = Attributes;
  NextVariable->State       = VAR_ADDED;
  NextVariable->Reserved    = 0;

  //
  // There will be pad bytes after Data, the NextVariable->NameSize and
  // NextVariable->NameSize should not include pad size so that variable
  // service can get actual size in GetVariable
  //
  NextVariable->NameSize  = (UINT32)VarNameSize;
  NextVariable->DataSize  = (UINT32)DataSize;

  EfiCopyMem (&NextVariable->VendorGuid, VendorGuid, sizeof (EFI_GUID));
  EfiCopyMem (
    (UINT8 *) ((UINTN) NextVariable + VarNameOffset),
    VariableName,
    VarNameSize
    );
  EfiCopyMem (
    (UINT8 *) ((UINTN) NextVariable + VarDataOffset),
    Data,
    DataSize
    );

  //
  // Mark the old variable as deleted
  //
  if (!EFI_ERROR (Status)) {
    Variable.CurrPtr->State &= VAR_DELETED;
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
  OUT UINT64                 *MaximumVariableSize,
  IN  VARIABLE_GLOBAL        *Global,
  IN  UINT32                 Instance
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
  Global                          Pointer to VARIABLE_GLOBAL structure.
  Instance                        Instance of the Firmware Volume.

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
    //   Make sure RT Attribute is set if we are in Runtime phase.
    //
    return EFI_INVALID_PARAMETER;
  } 

  if((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0) {
    //
    // Query is Volatile related.
    //
    VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINTN) Global->VolatileVariableBase);    
  } else {
    //
    // Query is Non-Volatile related.
    //
    VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINTN) Global->NonVolatileVariableBase);
  }

  //
  // Now let's fill *MaximumVariableStorageSize *RemainingVariableStorageSize 
  // with the storage size (excluding the storage header size)
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
  while (Variable < GetEndPointer (VariableStoreHeader)) {
    if (Variable->StartId != VARIABLE_DATA) {
      break;
    }

    NextVariable = (VARIABLE_HEADER *) (GetVariableDataPtr (Variable) + Variable->DataSize + GET_PAD_SIZE (Variable->DataSize));
    VariableSize = (UINT64) (UINTN) NextVariable - (UINT64) (UINTN) Variable;

    if (Variable->State == VAR_ADDED) {
      *RemainingVariableStorageSize -= VariableSize;
    }

    //
    // Go to the next one.
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

EFI_STATUS
InitializeVariableStore (
  OUT EFI_PHYSICAL_ADDRESS  *VariableBase,
  OUT UINTN                 *LastVariableOffset
  )
/*++

Routine Description:
  This function initializes variable store

Arguments:

Returns:

--*/
{
  VARIABLE_STORE_HEADER *VariableStore;
  EFI_STATUS            Status;

  //
  // Allocate memory for volatile variable store
  //
  Status = gBS->AllocatePool (
                  EfiRuntimeServicesData,
                  VARIABLE_STORE_SIZE,
                  &VariableStore
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  EfiSetMem (VariableStore, VARIABLE_STORE_SIZE, 0xff);

  //
  // Variable Specific Data
  //
  *VariableBase             = (EFI_PHYSICAL_ADDRESS) (UINTN) VariableStore;
  *LastVariableOffset       = sizeof (VARIABLE_STORE_HEADER);

  VariableStore->Signature  = VARIABLE_STORE_SIGNATURE;
  VariableStore->Size       = VARIABLE_STORE_SIZE;
  VariableStore->Format     = VARIABLE_STORE_FORMATTED;
  VariableStore->State      = VARIABLE_STORE_HEALTHY;
  VariableStore->Reserved   = 0;
  VariableStore->Reserved1  = 0;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VariableCommonInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  This function does common initialization for variable services

Arguments:

Returns:

--*/
{
  EFI_STATUS  Status;

  EfiInitializeRuntimeDriverLib (
    ImageHandle,
    SystemTable,
    VariableClassAddressChangeEvent
    );

  //
  // Allocate memory for mVariableModuleGlobal
  //
  Status = gBS->AllocatePool (
                  EfiRuntimeServicesData,
                  sizeof (ESAL_VARIABLE_GLOBAL),
                  &mVariableModuleGlobal
                  );
  if (EFI_ERROR (Status)) {
    EfiShutdownRuntimeDriverLib ();
    return Status;
  }
  //
  // Intialize volatile variable store
  //
  Status = InitializeVariableStore (
            &mVariableModuleGlobal->VariableBase[Physical].VolatileVariableBase,
            &mVariableModuleGlobal->VolatileLastVariableOffset
            );

  if (EFI_ERROR (Status)) {
    gBS->FreePool (mVariableModuleGlobal);
    EfiShutdownRuntimeDriverLib ();
    return Status;
  }
  //
  // Intialize non volatile variable store
  //
  Status = InitializeVariableStore (
            &mVariableModuleGlobal->VariableBase[Physical].NonVolatileVariableBase,
            &mVariableModuleGlobal->NonVolatileLastVariableOffset
            );
  if (EFI_ERROR (Status)) {
    gBS->FreePool ((VOID*) (UINTN) (mVariableModuleGlobal->VariableBase[Physical].VolatileVariableBase));
    gBS->FreePool (mVariableModuleGlobal);
    EfiShutdownRuntimeDriverLib ();
  }

  return Status;
}
