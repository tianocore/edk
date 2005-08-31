/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  IfrVariable.c

Abstract:
  Variable/Map manipulations routines

--*/

#include "IfrLibrary.h"

VOID
EfiLibHiiVariablePackGetMap (
  IN    EFI_HII_VARIABLE_PACK       *Pack,  
  OUT   CHAR16                      **Name,  OPTIONAL
  OUT   EFI_GUID                    **Guid,  OPTIONAL
  OUT   UINT16                      *Id,     OPTIONAL
  OUT   VOID                        **Var,   OPTIONAL
  OUT   UINTN                       *Size    OPTIONAL
  )
/*++

Routine Description:

  Extracts a variable form a Pack.

Arguments:

  Pack - List of variables
  Name - Name of the variable/map
  Guid - GUID of the variable/map
  Var  - Pointer to the variable/map
  Size - Size of the variable/map in bytes

Returns: 

  VOID
  
--*/
{
  if (NULL != Name) {
    *Name = (VOID *) (Pack + 1);
  }
    
  if (NULL != Guid) { 
    *Guid = &Pack->VariableGuid;
  }
    
    
  if (NULL != Id) {
    *Id   = Pack->VariableId;
  }
    
  if (NULL != Var) {
    *Var  = (VOID *) ((CHAR8 *) (Pack + 1) + Pack->VariableNameLength);
  }
    
  if (NULL != Size) {
    *Size = Pack->Header.Length - sizeof (*Pack) - Pack->VariableNameLength;
  }
}


UINTN
EfiLibHiiVariablePackListGetMapCnt (
  IN    EFI_HII_VARIABLE_PACK_LIST   *List
  )
  
/*++

Routine Description:

  Finds a count of the variables/maps in the List.

Arguments:

  List - List of variables

Returns: 

  UINTN - The number of map count.

--*/

{
  UINTN   Cnt = 0;
  while (NULL != List) {
    Cnt++;
    List = List->NextVariablePack;
  }
  return Cnt;
}


VOID
EfiLibHiiVariablePackListForEachVar (
  IN    EFI_HII_VARIABLE_PACK_LIST               *List,
  IN    EFI_LIB_HII_VARIABLE_PACK_LIST_CALLBACK  *Callback
  )
/*++

Routine Description:

  Will iterate all variable/maps as appearing 
  in List and for each, it will call the Callback.

Arguments:

  List     - List of variables
  Callback - Routine to be called for each iterated variable.

Returns: 

  VOID
  
--*/

{
  CHAR16    *MapName;
  EFI_GUID  *MapGuid;
  UINT16    MapId;
  VOID      *Map;
  UINTN     MapSize;

  while (NULL != List) {
    EfiLibHiiVariablePackGetMap (List->VariablePack, &MapName, &MapGuid, &MapId, &Map, &MapSize);
    //
    // call the callback
    //
    Callback (MapName, MapGuid, MapId, Map, MapSize); 
    List = List->NextVariablePack;
  }
}


EFI_STATUS
EfiLibHiiVariablePackListGetMapByIdx (
  IN    UINTN                       Idx,  
  IN    EFI_HII_VARIABLE_PACK_LIST  *List,  
  OUT   CHAR16                      **Name,  OPTIONAL
  OUT   EFI_GUID                    **Guid,  OPTIONAL
  OUT   UINT16                      *Id,     OPTIONAL
  OUT   VOID                        **Var,
  OUT   UINTN                       *Size
  )

/*++

Routine Description:

  Finds a variable form List given 
  the order number as appears in the List.

Arguments:

  Idx  - The index of the variable/map to retrieve
  List - List of variables
  Name - Name of the variable/map
  Guid - GUID of the variable/map
  Var  - Pointer to the variable/map
  Size - Size of the variable/map in bytes

Returns:

  EFI_SUCCESS   - Variable is found, OUT parameters are valid
  EFI_NOT_FOUND - Variable is not found, OUT parameters are not valid

--*/
{
  CHAR16     *MapName;
  EFI_GUID   *MapGuid;
  UINT16     MapId;
  VOID       *Map;
  UINTN      MapSize;

  while (NULL != List) {
    EfiLibHiiVariablePackGetMap (List->VariablePack, &MapName, &MapGuid, &MapId, &Map, &MapSize);
    if (0 == Idx--) {
      *Var  = Map;
      *Size = MapSize;

      if (NULL != Name) {
        *Name = MapName;
      }

      if (NULL != Guid) {
        *Guid = MapGuid;
      }
        
      if (NULL != Id) {
        *Id   = MapId;
      }
        
      return EFI_SUCCESS; // Map found
    }
    List = List->NextVariablePack;
  }
  //
  // If here, the map is not found
  //
  return EFI_NOT_FOUND; 
}


EFI_STATUS
EfiLibHiiVariablePackListGetMapById (
  IN    UINT16                        Id,  
  IN    EFI_HII_VARIABLE_PACK_LIST    *List,
  OUT   CHAR16                        **Name,  OPTIONAL
  OUT   EFI_GUID                      **Guid,  OPTIONAL
  OUT   VOID                          **Var,
  OUT   UINTN                         *Size
  )
  
/*++

Routine Description:

  Finds a variable form List given the 
  order number as appears in the List.

Arguments:

  Id   - The ID of the variable/map to retrieve
  List - List of variables
  Name - Name of the variable/map
  Guid - GUID of the variable/map
  Var  - Pointer to the variable/map
  Size - Size of the variable/map in bytes

Returns:

  EFI_SUCCESS   - Variable is found, OUT parameters are valid
  EFI_NOT_FOUND - Variable is not found, OUT parameters are not valid

--*/

{ 
  CHAR16    *MapName;
  EFI_GUID  *MapGuid;
  UINT16    MapId;
  VOID      *Map;
  UINTN     MapSize;

  while (NULL != List) {
    EfiLibHiiVariablePackGetMap (List->VariablePack, &MapName, &MapGuid, &MapId, &Map, &MapSize);
    if (MapId == Id) {
      *Var  = Map;
      *Size = MapSize;
      if (NULL != Name) {
         *Name = MapName;
      }
      if (NULL != Guid) {
        *Guid = MapGuid;
      }
      //
      // Map found
      //
      return EFI_SUCCESS; 
    }
    List = List->NextVariablePack;
  }
  //
  // If here, the map is not found
  //
  return EFI_NOT_FOUND; 
}


EFI_STATUS
EfiLibHiiVariablePackListGetMap (
  IN    EFI_HII_VARIABLE_PACK_LIST   *List,
  IN    CHAR16                       *Name,
  IN    EFI_GUID                     *Guid,
  OUT   UINT16                       *Id,
  OUT   VOID                         **Var, 
  OUT   UINTN                        *Size
  )

/*++

Routine Description:

  Finds a variable form EFI_HII_VARIABLE_PACK_LIST given name and GUID.

Arguments:

  List - List of variables
  Name - Name of the variable/map to be found
  Guid - GUID of the variable/map to be found
  Var  - Pointer to the variable/map found
  Size - Size of the variable/map in bytes found

Returns:

  EFI_SUCCESS   - variable is found, OUT parameters are valid
  EFI_NOT_FOUND - variable is not found, OUT parameters are not valid

--*/

{ 
  VOID      *Map;
  UINTN     MapSize;
  UINT16    MapId;
  CHAR16    *MapName;
  EFI_GUID  *MapGuid;

  while (NULL != List) {
    EfiLibHiiVariablePackGetMap (List->VariablePack, &MapName, &MapGuid, &MapId, &Map, &MapSize);
    if ((0 == EfiStrCmp (Name, MapName)) && EfiCompareGuid (Guid, MapGuid)) {
      *Id   = MapId;
      *Var  = Map;
      *Size = MapSize;
      return EFI_SUCCESS;
    }
    List = List->NextVariablePack;
  }
  //
  // If here, the map is not found
  //
  return EFI_NOT_FOUND;
}
