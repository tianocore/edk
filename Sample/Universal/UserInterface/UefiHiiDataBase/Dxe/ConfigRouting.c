/*++

Copyright (c) 2007 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    ConfigRouting.c
    
Abstract:

    Implementation for EFI_HII_CONFIG_ROUTING_PROTOCOL.

Revision History

--*/

#include "HiiDatabase.h"

#ifndef DISABLE_UNUSED_HII_PROTOCOLS

STATIC
UINTN
CalculateConfigStringLen (
  IN EFI_STRING                    String
  )
/*++

  Routine Description:
    Calculate the number of Unicode characters of the incoming Configuration string, 
    not including NULL terminator.
    
  Arguments:          
    String                - String in <MultiConfigRequest> or <MultiConfigResp> format.
    
  Returns:              
    The number of Unicode characters.
                             
--*/    
{
  UINTN Length;

  //
  // "GUID=" should be the first element of incoming string.
  //
  ASSERT (String != NULL);
  ASSERT (EfiStrnCmp (String, L"GUID=", EfiStrLen (L"GUID=")) == 0);
  
  Length  = EfiStrLen (L"GUID=");
  String += Length;

  //
  // The beginning of next <ConfigRequest>/<ConfigResp> should be "&GUID=".
  // Will meet '\0' if there is only one <ConfigRequest>/<ConfigResp>.
  // 
  while (*String != 0 && EfiStrnCmp (String, L"&GUID=", EfiStrLen (L"&GUID=")) != 0) {
    Length++;
    String++;
  }

  return Length;
}

STATIC
EFI_STATUS
GetDevicePath (
  IN  EFI_STRING                   String,
  OUT UINT8                        **DevicePath
  )
/*++

  Routine Description:
    Convert the hex UNICODE %02x encoding of a UEFI device path to binary
    from <PathHdr> of <ConfigHdr>.
    
  Arguments:          
    String                 - UEFI configuration string
    DevicePath             - binary of a UEFI device path.
    
  Returns:              
    EFI_INVALID_PARAMETER  - Any incoming parameter is invalid.
    EFI_OUT_OF_RESOURCES   - Lake of resources to store neccesary structures.
    EFI_SUCCESS            - The device path is retrieved and translated to binary format.
                             
--*/    
{
  UINTN      Length;
  EFI_STRING PathHdr;
  EFI_STRING DevicePathString;
  
  if (String == NULL || DevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Find the 'PATH=' of <PathHdr> and skip it.
  //
  for (; (*String != 0 && EfiStrnCmp (String, L"PATH=", EfiStrLen (L"PATH=")) != 0); String++);
  if (*String == 0) {
    return EFI_INVALID_PARAMETER;
  }  
    
  String += EfiStrLen (L"PATH=");
  PathHdr = String;

  //
  // The content between 'PATH=' of <ConfigHdr> and '&' of next element
  // or '\0' (end of configuration string) is the UNICODE %02x bytes encoding 
  // of UEFI device path.   
  //
  for (Length = 0; *String != 0 && *String != L'&'; String++, Length++);
  DevicePathString = (EFI_STRING) EfiLibAllocateZeroPool ((Length + 1) * sizeof (CHAR16));
  if (DevicePathString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  EfiStrnCpy (DevicePathString, PathHdr, Length);
  *(DevicePathString + Length) = 0;  

  //
  // The data in <PathHdr> is encoded as hex UNICODE %02x bytes in the same order
  // as the device path resides in RAM memory.
  // Translate the data into binary.
  //
  Length /= 2;
  *DevicePath = (UINT8 *) EfiLibAllocateZeroPool (Length);
  if (*DevicePath == NULL) {
    EfiLibSafeFreePool (DevicePathString);
    return EFI_OUT_OF_RESOURCES;
  }

  HexStringToBuffer (*DevicePath, &Length, DevicePathString);
  EfiLibSafeFreePool (DevicePathString);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ExportAllStorage (
  IN EFI_HII_DATABASE_PROTOCOL     *HiiDatabase,
  IN OUT EFI_LIST_ENTRY            *StorageListHead
)
/*++
  Routine Description:
    Extract Storage from all Form Packages in current hii database.

  Arguments:
    HiiDatabase            - EFI_HII_DATABASE_PROTOCOL instance.
    StorageListHead        - Storage link List head.

  Returns:
    EFI_NOT_FOUND          - There is no form package in current hii database.
    EFI_INVALID_PARAMETER  - Any parameter is invalid.
    EFI_SUCCESS            - All existing storage is exported.

--*/
{
  EFI_STATUS                   Status;
  UINTN                        BufferSize;
  UINTN                        HandleCount;
  EFI_HII_HANDLE               *HandleBuffer;
  UINTN                        Index;
  UINTN                        Index2;
  EFI_HII_PACKAGE_LIST_HEADER  *HiiPackageList;
  EFI_HII_PACKAGE_HEADER       *Package;
  UINT8                        *OpCodeData;
  UINT8                        Operand;
  UINT32                       Offset;
  HII_FORMSET_STORAGE          *Storage;
  EFI_HII_HANDLE               HiiHandle;
  EFI_HANDLE                   DriverHandle;
  CHAR8                        *AsciiString;
  UINT32                       PackageListLength;
  EFI_HII_PACKAGE_HEADER       PackageHeader;

  //
  // Find the package list which contains Form package.
  //
  BufferSize   = 0;
  HandleBuffer = NULL;
  Status = HiiListPackageLists (
             HiiDatabase, 
             EFI_HII_PACKAGE_FORM, 
             NULL, 
             &BufferSize, 
             HandleBuffer
             );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    HandleBuffer = EfiLibAllocateZeroPool (BufferSize);
    ASSERT (HandleBuffer != NULL);

    Status = HiiListPackageLists (
               HiiDatabase, 
               EFI_HII_PACKAGE_FORM, 
               NULL, 
               &BufferSize, 
               HandleBuffer
               );
  }
  if (EFI_ERROR (Status)) {
    EfiLibSafeFreePool (HandleBuffer);
    return Status;
  }

  HandleCount = BufferSize / sizeof (EFI_HII_HANDLE);
  for (Index = 0; Index < HandleCount; Index++) {
    HiiHandle = HandleBuffer[Index];

    BufferSize     = 0;   
    HiiPackageList = NULL;
    Status = HiiExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      HiiPackageList = EfiLibAllocateZeroPool (BufferSize);
      ASSERT (HiiPackageList != NULL);
      Status = HiiExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
    }
    if (EFI_ERROR (Status)) {
      EfiLibSafeFreePool (HandleBuffer);
      EfiLibSafeFreePool (HiiPackageList);
      return Status;
    }

    //
    // Get Form package from this HII package List
    //
    Offset  = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
    EfiCopyMem (&PackageListLength, &HiiPackageList->PackageLength, sizeof (UINT32));
    Package = NULL;
    EfiZeroMem (&PackageHeader, sizeof (EFI_HII_PACKAGE_HEADER));
    
    while (Offset < PackageListLength) {
      Package = (EFI_HII_PACKAGE_HEADER *) (((UINT8 *) HiiPackageList) + Offset);
      EfiCopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));
      if (PackageHeader.Type == EFI_HII_PACKAGE_FORM) {
        break;
      }
      Offset += PackageHeader.Length;
    }
    if (Offset >= PackageListLength) {
      //
      // Error here: No Form package found in this Package List
      //
      ASSERT (FALSE);
    }

    //
    // Search Storage definition in this Form package
    //
    Offset = sizeof (EFI_HII_PACKAGE_HEADER);
    while (Offset < PackageHeader.Length) {
      OpCodeData = ((UINT8 *) Package) + Offset;
      Offset += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;

      Operand = ((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode;

      if ((Operand == EFI_IFR_VARSTORE_OP) ||
          (Operand == EFI_IFR_VARSTORE_NAME_VALUE_OP) ||
          (Operand == EFI_IFR_VARSTORE_EFI_OP)) {

        Storage = EfiLibAllocateZeroPool (sizeof (HII_FORMSET_STORAGE));
        ASSERT (Storage != NULL);
        InsertTailList (StorageListHead, &Storage->Entry);

        Storage->Signature = HII_FORMSET_STORAGE_SIGNATURE;
        Storage->HiiHandle = HiiHandle;

        Status = HiiGetPackageListHandle (HiiDatabase, HiiHandle, &DriverHandle);
        if (EFI_ERROR (Status)) {
          EfiLibSafeFreePool (HandleBuffer);
          EfiLibSafeFreePool (HiiPackageList);
          EfiLibSafeFreePool (Storage);
          return Status;
        }
        Storage->DriverHandle = DriverHandle;        

        if (Operand == EFI_IFR_VARSTORE_OP) {
          Storage->Type = EFI_HII_VARSTORE_BUFFER;

          EfiCopyMem (&Storage->Guid, &((EFI_IFR_VARSTORE *) OpCodeData)->Guid, sizeof (EFI_GUID));
          EfiCopyMem (&Storage->Size, &((EFI_IFR_VARSTORE *) OpCodeData)->Size, sizeof (UINT16));

          AsciiString = (CHAR8 *) ((EFI_IFR_VARSTORE *) OpCodeData)->Name;
          Storage->Name = EfiLibAllocateZeroPool (EfiAsciiStrSize (AsciiString) * 2);
          ASSERT (Storage->Name != NULL);
          for (Index2 = 0; AsciiString[Index2] != 0; Index2++) {
            Storage->Name[Index2] = (CHAR16) AsciiString[Index2];
          }
          //
          // Append '\0' to the end of the unicode string.
          //
          Storage->Name[Index2] = 0;
        } else if (Operand == EFI_IFR_VARSTORE_NAME_VALUE_OP) {
          Storage->Type = EFI_HII_VARSTORE_NAME_VALUE;

          EfiCopyMem (&Storage->Guid, &((EFI_IFR_VARSTORE_NAME_VALUE *) OpCodeData)->Guid, sizeof (EFI_GUID));
        } else if (Operand == EFI_IFR_VARSTORE_EFI_OP) {
          Storage->Type = EFI_HII_VARSTORE_EFI_VARIABLE;

          EfiCopyMem (&Storage->Guid, &((EFI_IFR_VARSTORE_EFI *) OpCodeData)->Guid, sizeof (EFI_GUID));
        }
      }
    }
    
    EfiLibSafeFreePool (HiiPackageList);
  }

  EfiLibSafeFreePool (HandleBuffer);  
  
  return EFI_SUCCESS;
}

STATIC
VOID
GenerateSubStr (
  IN CONST EFI_STRING              String,
  IN  UINTN                        BufferLen,
  IN  VOID                         *Buffer,
  IN  UINT8                        Flag,
  OUT EFI_STRING                   *SubStr  
  )
/*++
  Routine Description:
    Generate a sub string then output it.

  Arguments:
    String                 - A constant string which is the prefix of the to be
                             generated string, e.g. GUID=
    BufferLen              - The length of the Buffer in bytes.
    Buffer                 - Points to a buffer which will be converted to
                             be the content of the generated string.
    Flag                   - If 1, the buffer contains data for the value of GUID or PATH stored in 
                             UINT8 *; if 2, the buffer contains unicode string for the value of NAME;
                             if 3, the buffer contains other data.
    SubStr                 - Points to the output string. It's caller's responsibility
                             to free this buffer.
                             
  Returns:

--*/
  
{
  UINTN       Length;
  EFI_STRING  Str;
  EFI_STATUS  Status;
  EFI_STRING  StringHeader;

  ASSERT (String != NULL && SubStr != NULL);

  if (Buffer == NULL) {
    *SubStr = EfiLibAllocateCopyPool (EfiStrSize (String), String);
    ASSERT (*SubStr != NULL);
    return ;
  }

  Length = EfiStrLen (String) + BufferLen * 2 + 1 + 1;
  Str = EfiLibAllocateZeroPool (Length * sizeof (CHAR16));
  ASSERT (Str != NULL); 

  EfiStrCpy (Str, String);
  Length = (BufferLen * 2 + 1) * sizeof (CHAR16);

  Status       = EFI_SUCCESS;
  StringHeader = Str + EfiStrLen (String);

  switch (Flag) {
  case 1:
    Status = BufferToHexString (StringHeader, (UINT8 *) Buffer, BufferLen);
    break;
  case 2:
    Status = UnicodeToConfigString (StringHeader, &Length, (CHAR16 *) Buffer);
    break;
  case 3:
    Status = BufToHexString (StringHeader, &Length, (UINT8 *) Buffer, BufferLen);
    //
    // Convert the uppercase to lowercase since <HexAf> is defined in lowercase format.
    //
    ToLower (StringHeader);
    break;
  default:
    break;
  }

  ASSERT_EFI_ERROR (Status);
  EfiStrCat (Str, L"&");

  *SubStr = Str;
}

STATIC
EFI_STATUS
OutputConfigBody (
  IN  EFI_STRING                   String,
  OUT EFI_STRING                   *ConfigBody
  )
/*++
  Routine Description:
    Retrieve the <ConfigBody> from String then output it.

  Arguments:
    String                 - A sub string of a configuration string in 
                             <MultiConfigAltResp> format.
    ConfigBody             - Points to the output string. It's caller's responsibility
                             to free this buffer.                             
  Returns:
    EFI_INVALID_PARAMETER  - There is no form package in current hii database.
    EFI_OUT_OF_RESOURCES   - Not enough memory to finish this operation.
    EFI_SUCCESS            - All existing storage is exported.    

--*/  
{
  EFI_STRING  TmpPtr;
  EFI_STRING  Result;
  UINTN       Length;

  if (String == NULL || ConfigBody == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TmpPtr = EfiStrStr (String, L"GUID=");
  if (TmpPtr == NULL) {
    //
    // It is the last <ConfigResp> of the incoming configuration string.
    //
    Result = EfiLibAllocateCopyPool (EfiStrSize (String), String);
    if (Result == NULL) {
      return EFI_OUT_OF_RESOURCES;
    } else {
      *ConfigBody = Result;
      return EFI_SUCCESS;
    }
  }
  
  Length = TmpPtr - String;
  Result = EfiLibAllocateCopyPool (Length * sizeof (CHAR16), String);
  if (Result == NULL) {
    return EFI_OUT_OF_RESOURCES;     
  }

  *(Result + Length - 1) = 0;
  *ConfigBody = Result;
  return EFI_SUCCESS;  
  
}


#endif

VOID *
ReallocatePool (
  IN VOID                          *OldPool,
  IN UINTN                         OldSize,
  IN UINTN                         NewSize
  )
/*++

Routine Description:
  Adjusts the size of a previously allocated buffer.

Arguments:
  OldPool               - A pointer to the buffer whose size is being adjusted.
  OldSize               - The size of the current buffer.
  NewSize               - The size of the new buffer.

Returns:
  Points to the new buffer
  
--*/
{
  VOID  *NewPool;

  NewPool = NULL;
  if (NewSize) {
    NewPool = EfiLibAllocateZeroPool (NewSize);
  }

  if (OldPool) {
    if (NewPool) {
      EfiCopyMem (NewPool, OldPool, OldSize < NewSize ? OldSize : NewSize);
    }

    gBS->FreePool (OldPool);
  }

  return NewPool;
}

STATIC
EFI_STATUS
AppendToMultiString (
  IN OUT EFI_STRING                *MultiString,
  IN EFI_STRING                    AppendString      
  )
/*++

  Routine Description:
    Append a string to a multi-string format.
    
  Arguments:          
    MultiString            - String in <MultiConfigRequest>, <MultiConfigAltResp>,
                             or <MultiConfigResp>. On input, the buffer length of 
                             this string is MAX_STRING_LENGTH. On output, the 
                             buffer length might be updated.
    AppendString           - NULL-terminated Unicode string.
    
  Returns:              
    EFI_INVALID_PARAMETER  - Any incoming parameter is invalid.
    EFI_SUCCESS            - AppendString is append to the end of MultiString
                             
--*/    
{
  UINTN AppendStringSize;
  UINTN MultiStringSize;
  
  if (MultiString == NULL || *MultiString == NULL || AppendString == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  AppendStringSize = EfiStrSize (AppendString);
  MultiStringSize  = EfiStrSize (*MultiString);

  //
  // Enlarge the buffer each time when length exceeds MAX_STRING_LENGTH.
  //
  if (MultiStringSize + AppendStringSize > MAX_STRING_LENGTH ||
      MultiStringSize > MAX_STRING_LENGTH) {
    *MultiString = (EFI_STRING) ReallocatePool (
                                  (VOID *) (*MultiString), 
                                  MultiStringSize,
                                  MultiStringSize + AppendStringSize
                                  );    
  } 

  //
  // Append the incoming string
  //
  EfiStrCat (*MultiString, AppendString);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetValueOfNumber (
  IN EFI_STRING                    StringPtr,
  OUT UINT8                        **Number,
  OUT UINTN                        *Len
  )
/*++

  Routine Description:
    Get the value of <Number> in <BlockConfig> format, i.e. the value of OFFSET 
    or WIDTH or VALUE.

    <BlockConfig> ::= 'OFFSET='<Number>&'WIDTH='<Number>&'VALUE'=<Number> 
    
  Arguments:          
    StringPtr              - String in <BlockConfig> format and points to the 
                             first character of <Number>.
    Number                 - The output value. Caller takes the responsibility to
                             free memory.
    Len                    - Length of the <Number>, in characters.
    
  Returns:              
    EFI_OUT_OF_RESOURCES   - Insufficient resources to store neccessary structures.
    EFI_SUCCESS            - Value of <Number> is outputted in Number successfully.
                             
--*/      
{
  EFI_STRING               TmpPtr;
  UINTN                    Length;
  EFI_STRING               Str;
  UINT8                    *Buf;
  EFI_STATUS               Status;

  ASSERT (StringPtr != NULL && Number != NULL && Len != NULL);
  ASSERT (*StringPtr != 0);

  Buf = NULL;

  TmpPtr = StringPtr;
  while (*StringPtr != 0 && *StringPtr != L'&') {
    StringPtr++;
  }
  *Len   = StringPtr - TmpPtr;
  Length = *Len + 1;

  Str = (EFI_STRING) EfiLibAllocateZeroPool (Length * sizeof (EFI_STRING));
  if (Str == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }
  EfiCopyMem (Str, TmpPtr, *Len * sizeof (CHAR16));    
  *(Str + *Len) = 0;

  Length = (Length + 1) / 2;
  Buf = (UINT8 *) EfiLibAllocateZeroPool (Length);
  if (Buf == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;    
  }

  Status = HexStringToBuf (Buf, &Length, Str, NULL);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  *Number = Buf;
  Status  = EFI_SUCCESS;
    
Exit:
  EfiLibSafeFreePool (Str);
  return Status;
}

EFI_STATUS
EFIAPI 
HiiConfigRoutingExtractConfig (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL  *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
/*++

  Routine Description:
    This function allows a caller to extract the current configuration 
    for one or more named elements from one or more drivers.
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.
    Request                - A null-terminated Unicode string in <MultiConfigRequest> format.
    Progress               - On return, points to a character in the Request string.
                             Points to the string's null terminator if request was
                             successful. Points to the most recent & before the first
                             failing name / value pair (or the beginning of the string
                             if the failure is in the first name / value pair) if the
                             request was not successful.
    Results                - Null-terminated Unicode string in <MultiConfigAltResp>
                             format which has all values filled in for the names
                             in the Request string. String to be allocated by the
                             called function.                                                                                                         
                        
  Returns:              
    EFI_SUCCESS            - The Results string is filled with the values
                             corresponding to all requested names.       
    EFI_OUT_OF_RESOURCES   - Not enough memory to store the parts of the results
                             that must be stored awaiting possible future       
                             protocols.                                                                      
    EFI_NOT_FOUND          - Routing data doesn't match any known driver.        
                             Progress set to the "G" in "GUID" of the routing 
                             header that doesn't match. Note: There is no        
                             requirement that all routing data be validated before
                             any configuration extraction.                            
    EFI_INVALID_PARAMETER  - For example, passing in a NULL for the Request   
                             parameter would result in this type of error. The
                             Progress parameter is set to NULL.               
                             
    EFI_INVALID_PARAMETER  - Illegal syntax. Progress set to most recent & before
                             the error or the beginning of the string.               
        
    EFI_INVALID_PARAMETER  - Unknown name. Progress points to the & before
                             the name in question.                        
                             
--*/    
{
#ifndef DISABLE_UNUSED_HII_PROTOCOLS

  HII_DATABASE_PRIVATE_DATA           *Private;
  EFI_STRING                          StringPtr;
  EFI_STRING                          ConfigRequest;
  UINTN                               Length;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
  EFI_STATUS                          Status;
  EFI_LIST_ENTRY                      *Link;
  HII_DATABASE_RECORD                 *Database;
  UINT8                               *CurrentDevicePath; 
  EFI_HANDLE                          DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL      *ConfigAccess;
  EFI_STRING                          AccessProgress;
  EFI_STRING                          AccessResults;
  UINTN                               RemainSize;
  EFI_STRING                          TmpPtr;
  
  if (This == NULL || Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Request == NULL) {
    *Progress = NULL;
    return EFI_INVALID_PARAMETER;
  }

  Private   = CONFIG_ROUTING_DATABASE_PRIVATE_DATA_FROM_THIS (This);
  StringPtr = Request;
  *Progress = StringPtr;

  //
  // The first element of <MultiConfigRequest> should be
  // <GuidHdr>, which is in 'GUID='<Guid> syntax.
  //
  if (EfiStrnCmp (StringPtr, L"GUID=", EfiStrLen (L"GUID=")) != 0) {    
    return EFI_INVALID_PARAMETER;
  }

  //
  // Allocate a fix length of memory to store Results. Reallocate memory for 
  // Results if this fix length is insufficient.
  //
  *Results = (EFI_STRING) EfiLibAllocateZeroPool (MAX_STRING_LENGTH);
  if (*Results == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  while (*StringPtr != 0 && EfiStrnCmp (StringPtr, L"GUID=", EfiStrLen (L"GUID=")) == 0) {
    //
    // If parsing error, set Progress to the beginning of the <MultiConfigRequest>
    // or most recent & before the error.
    //
    if (StringPtr == Request) {
      *Progress = StringPtr;
    } else {
      *Progress = StringPtr - 1;
    }
    
    //
    // Process each <ConfigRequest> of <MultiConfigRequest>
    //
    Length = CalculateConfigStringLen (StringPtr);
    ConfigRequest = EfiLibAllocateCopyPool ((Length + 1) * sizeof (CHAR16), StringPtr);
    if (ConfigRequest == NULL) {      
      return EFI_OUT_OF_RESOURCES;
    }
    *(ConfigRequest + Length) = 0;

    //
    // Get the UEFI device path
    //
    Status = GetDevicePath (ConfigRequest, (UINT8 **) &DevicePath);
    if (EFI_ERROR (Status)) {
      EfiLibSafeFreePool (ConfigRequest);
      return Status;
    }

    //
    // Find driver which matches the routing data.
    //
    DriverHandle = NULL;
    for (Link = Private->DatabaseList.ForwardLink;
         Link != &Private->DatabaseList;
         Link = Link->ForwardLink
        ) {
      Database = CR (Link, HII_DATABASE_RECORD, DatabaseEntry, HII_DATABASE_RECORD_SIGNATURE);      
      CurrentDevicePath = Database->PackageList->DevicePathPkg + sizeof (EFI_HII_PACKAGE_HEADER);
      if (CurrentDevicePath != NULL) {
        if (EfiCompareMem (
              DevicePath, 
              CurrentDevicePath, 
              EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) CurrentDevicePath)
              ) == 0) {
          DriverHandle = Database->DriverHandle;
          break;
        }
      }
    }
    
    EfiLibSafeFreePool (DevicePath);
    
    if (DriverHandle == NULL) {
      //
      // Routing data does not match any known driver.
      // Set Progress to the 'G' in "GUID" of the routing header.
      //
      *Progress = StringPtr;
      EfiLibSafeFreePool (ConfigRequest);
      return EFI_NOT_FOUND;
    }

    //
    // Call corresponding ConfigAccess protocol to extract settings 
    //
    Status = gBS->HandleProtocol (
                    DriverHandle,
                    &gEfiHiiConfigAccessProtocolGuid,
                    (VOID *) &ConfigAccess
                    );
    ASSERT_EFI_ERROR (Status);

    Status = ConfigAccess->ExtractConfig (
                             ConfigAccess, 
                             ConfigRequest, 
                             &AccessProgress, 
                             &AccessResults
                             );    
    if (EFI_ERROR (Status)) {
      //
      // AccessProgress indicates the parsing progress on <ConfigRequest>.
      // Map it to the progress on <MultiConfigRequest> then return it.
      //
      RemainSize = EfiStrSize (AccessProgress);
      for (TmpPtr = StringPtr; EfiCompareMem (TmpPtr, AccessProgress, RemainSize) != 0; TmpPtr++);      
      *Progress = TmpPtr;

      EfiLibSafeFreePool (ConfigRequest);      
      return Status;
    }

    //
    // Attach this <ConfigAltResp> to a <MultiConfigAltResp>
    //
    ASSERT (*AccessProgress == 0);
    Status = AppendToMultiString (Results, AccessResults);
    ASSERT_EFI_ERROR (Status);
    EfiLibSafeFreePool (AccessResults);
    AccessResults = NULL;
    EfiLibSafeFreePool (ConfigRequest);
    ConfigRequest = NULL;
    
    //
    // Go to next <ConfigRequest> (skip '&').
    //    
    StringPtr += Length;
    if (*StringPtr == 0) {
      *Progress = StringPtr;
      break;
    }

    StringPtr++;

  }

  return EFI_SUCCESS;  
#else
  return EFI_UNSUPPORTED;
#endif
  
}
  
EFI_STATUS 
EFIAPI 
HiiConfigRoutingExportConfig (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL  *This,
  OUT EFI_STRING                             *Results
  )
/*++

  Routine Description:
    This function allows the caller to request the current configuration for the 
    entirety of the current HII database and returns the data in a 
    null-terminated Unicode string.        
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.    
    Results                - Null-terminated Unicode string in <MultiConfigAltResp>
                             format which has all values filled in for the names
                             in the Request string. String to be allocated by the 
                             called function. De-allocation is up to the caller.                                                        
                        
  Returns:              
    EFI_SUCCESS            - The Results string is filled with the values
                             corresponding to all requested names.       
    EFI_OUT_OF_RESOURCES   - Not enough memory to store the parts of the results
                             that must be stored awaiting possible future       
                             protocols.                                                                      
    EFI_INVALID_PARAMETER  - For example, passing in a NULL for the Results
                             parameter would result in this type of error.    
                             
--*/    
{
#ifndef DISABLE_UNUSED_HII_PROTOCOLS

  EFI_STATUS                          Status;
  HII_DATABASE_PRIVATE_DATA           *Private;
  EFI_LIST_ENTRY                      StorageListHdr; 
  HII_FORMSET_STORAGE                 *Storage;
  EFI_LIST_ENTRY                      *Link;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
  UINTN                               Length;
  EFI_STRING                          PathHdr;
  UINTN                               PathHdrSize;
  EFI_STRING                          ConfigRequest;
  UINTN                               RequestSize;
  EFI_STRING                          StringPtr;
  EFI_HII_CONFIG_ACCESS_PROTOCOL      *ConfigAccess;
  EFI_STRING                          AccessProgress;
  EFI_STRING                          AccessResults;   

  if (This == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = CONFIG_ROUTING_DATABASE_PRIVATE_DATA_FROM_THIS (This);

  InitializeListHead (&StorageListHdr);

  Status = ExportAllStorage (&Private->HiiDatabase, &StorageListHdr);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Allocate a fix length of memory to store Results. Reallocate memory for 
  // Results if this fix length is insufficient.
  //
  *Results = (EFI_STRING) EfiLibAllocateZeroPool (MAX_STRING_LENGTH);
  if (*Results == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Parsing all formset storages.
  //
  for (Link = StorageListHdr.ForwardLink; Link != &StorageListHdr; Link = Link->ForwardLink) {
    Storage = CR (Link, HII_FORMSET_STORAGE, Entry, HII_FORMSET_STORAGE_SIGNATURE);
    //
    // Find the corresponding device path instance
    //
    Status = gBS->HandleProtocol (
                    Storage->DriverHandle,
                    &gEfiDevicePathProtocolGuid,                  
                    (VOID **) &DevicePath
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // Convert the device path binary to hex UNICODE %02x bytes in the same order
    // as the device path resides in RAM memory.
    //
    Length      = EfiDevicePathSize (DevicePath);
    PathHdrSize = (Length * 2 + 1) * sizeof (CHAR16);
    PathHdr     = (EFI_STRING) EfiLibAllocateZeroPool (PathHdrSize);
    if (PathHdr == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    Status = BufferToHexString (PathHdr, (UINT8 *) DevicePath, Length);
    ASSERT_EFI_ERROR (Status);

    //
    // Generate a <ConfigRequest> with one <ConfigHdr> and zero <RequestElement>.
    // It means extract all possible configurations from this specific driver.
    //
    RequestSize   = (EfiStrLen (L"GUID=&NAME=&PATH=") + 32 +  EfiStrLen (Storage->Name) * 4)
                     * sizeof (CHAR16) + PathHdrSize;
    ConfigRequest = (EFI_STRING) EfiLibAllocateZeroPool (RequestSize);
    if (ConfigRequest == NULL) {
      EfiLibSafeFreePool (PathHdr);
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // Add <GuidHdr>
    // <GuidHdr> ::= 'GUID='<Guid>
    // Convert <Guid> in the same order as it resides in RAM memory.
    //
    StringPtr = ConfigRequest;
    EfiStrnCpy (StringPtr, L"GUID=", EfiStrLen (L"GUID="));
    StringPtr += EfiStrLen (L"GUID=");

    Status = BufferToHexString (StringPtr, (UINT8 *) (&Storage->Guid), sizeof (EFI_GUID));
    ASSERT_EFI_ERROR (Status);
    
    StringPtr += 32;
    ASSERT (*StringPtr == 0);
    *StringPtr = L'&';
    StringPtr++;

    //
    // Add <NameHdr>
    // <NameHdr> ::= 'NAME='<String>
    //
    EfiStrnCpy (StringPtr, L"NAME=", EfiStrLen (L"NAME="));
    StringPtr += EfiStrLen (L"NAME=");

    Length = (EfiStrLen (Storage->Name) * 4 + 1) * sizeof (CHAR16);
    Status = UnicodeToConfigString (StringPtr, &Length, Storage->Name);
    ASSERT_EFI_ERROR (Status);
    StringPtr += EfiStrLen (Storage->Name) * 4;
    
    *StringPtr = L'&';
    StringPtr++;

    //
    // Add <PathHdr>
    // <PathHdr> ::= '<PATH=>'<UEFI binary represented as hex UNICODE %02x>
    //
    EfiStrnCpy (StringPtr, L"PATH=", EfiStrLen (L"PATH="));
    StringPtr += EfiStrLen (L"PATH=");
    EfiStrCpy (StringPtr, PathHdr);

    EfiLibSafeFreePool (PathHdr);
    PathHdr = NULL;

    //
    // BUGBUG: The "Implementation note" of ExportConfig() in UEFI spec makes the 
    // code somewhat complex. Let's TBD here whether a <ConfigRequest> or a <ConfigHdr> 
    // is required to call ConfigAccess.ExtractConfig().
    // 
    // Here we use <ConfigHdr> to call ConfigAccess instance. It requires ConfigAccess
    // to handle such kind of "ConfigRequest". It is not supported till now.
    // 
    // Either the ExportConfig will be updated or the ConfigAccess.ExtractConfig() 
    // will be updated as soon as the decision is made.

    //
    // Route the request to corresponding ConfigAccess protocol to extract settings.
    //
    Status = gBS->HandleProtocol (
                    Storage->DriverHandle,
                    &gEfiHiiConfigAccessProtocolGuid,
                    (VOID *) &ConfigAccess
                    );
    ASSERT_EFI_ERROR (Status);

    Status = ConfigAccess->ExtractConfig (
                             ConfigAccess, 
                             ConfigRequest, 
                             &AccessProgress, 
                             &AccessResults
                             );    
    if (EFI_ERROR (Status)) {
      EfiLibSafeFreePool (ConfigRequest);
      EfiLibSafeFreePool (AccessResults);
      return EFI_INVALID_PARAMETER;
    }

    //
    // Attach this <ConfigAltResp> to a <MultiConfigAltResp>
    //
    ASSERT (*AccessProgress == 0);
    Status = AppendToMultiString (Results, AccessResults);
    ASSERT_EFI_ERROR (Status);
    EfiLibSafeFreePool (AccessResults);
    AccessResults = NULL;
    EfiLibSafeFreePool (ConfigRequest);
    ConfigRequest = NULL;
    
  }

  //
  // Free the exported storage resource
  //
  while (!IsListEmpty (&StorageListHdr)) {
    Storage = CR (
                StorageListHdr.ForwardLink, 
                HII_FORMSET_STORAGE, 
                Entry, 
                HII_FORMSET_STORAGE_SIGNATURE
                );
    RemoveEntryList (&Storage->Entry);
    EfiLibSafeFreePool (Storage->Name);
    EfiLibSafeFreePool (Storage);
  }
  
  return EFI_SUCCESS;  
#else
  return EFI_UNSUPPORTED;
#endif
}  

EFI_STATUS
EFIAPI
HiiConfigRoutingRouteConfig (
  IN  EFI_HII_CONFIG_ROUTING_PROTOCOL        *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
/*++

  Routine Description:
    This function processes the results of processing forms and routes it to the 
    appropriate handlers or storage.
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.
    Configuration          - A null-terminated Unicode string in <MulltiConfigResp> format.
    Progress               - A pointer to a string filled in with the offset of
                             the most recent & before the first failing name / value
                             pair (or the beginning of the string if the failure is
                             in the first name / value pair) or the terminating
                             NULL if all was successful.
                        
  Returns:              
    EFI_SUCCESS            - The results have been distributed or are awaiting
                             distribution.                                    
    EFI_OUT_OF_RESOURCES   - Not enough memory to store the parts of the results
                             that must be stored awaiting possible future       
                             protocols.                                                                      
    EFI_INVALID_PARAMETER  - Passing in a NULL for the Configuration
                             parameter would result in this type of error.    
    EFI_NOT_FOUND          - Target for the specified routing data was not found.        
                             
--*/    
{
#ifndef DISABLE_UNUSED_HII_PROTOCOLS

  HII_DATABASE_PRIVATE_DATA           *Private;
  EFI_STRING                          StringPtr;
  EFI_STRING                          ConfigResp;
  UINTN                               Length;
  EFI_STATUS                          Status;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
  EFI_LIST_ENTRY                      *Link;
  HII_DATABASE_RECORD                 *Database;
  UINT8                               *CurrentDevicePath; 
  EFI_HANDLE                          DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL      *ConfigAccess;
  EFI_STRING                          AccessProgress;
  UINTN                               RemainSize;
  EFI_STRING                          TmpPtr;  

  if (This == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Configuration == NULL) {
    *Progress = NULL;
    return EFI_INVALID_PARAMETER;
  }

  Private   = CONFIG_ROUTING_DATABASE_PRIVATE_DATA_FROM_THIS (This);
  StringPtr = Configuration;
  *Progress = StringPtr;

  //
  // The first element of <MultiConfigResp> should be
  // <GuidHdr>, which is in 'GUID='<Guid> syntax.
  //
  if (EfiStrnCmp (StringPtr, L"GUID=", EfiStrLen (L"GUID=")) != 0) {    
    return EFI_INVALID_PARAMETER;
  }

  while (*StringPtr != 0 && EfiStrnCmp (StringPtr, L"GUID=", EfiStrLen (L"GUID=")) == 0) {
    //
    // If parsing error, set Progress to the beginning of the <MultiConfigResp>
    // or most recent & before the error.
    //
    if (StringPtr == Configuration) {
      *Progress = StringPtr;
    } else {
      *Progress = StringPtr - 1;
    }

    //
    // Process each <ConfigResp> of <MultiConfigResp>
    //
    Length = CalculateConfigStringLen (StringPtr);
    ConfigResp = EfiLibAllocateCopyPool ((Length + 1) * sizeof (CHAR16), StringPtr);
    if (ConfigResp == NULL) {      
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Append '\0' to the end of ConfigRequest
    //
    *(ConfigResp + Length) = 0;

    //
    // Get the UEFI device path
    //
    Status = GetDevicePath (ConfigResp, (UINT8 **) &DevicePath);
    if (EFI_ERROR (Status)) {
      EfiLibSafeFreePool (ConfigResp);
      return Status;
    }
    
    //
    // Find driver which matches the routing data.
    //
    DriverHandle = NULL;
    for (Link = Private->DatabaseList.ForwardLink;
         Link != &Private->DatabaseList;
         Link = Link->ForwardLink
        ) {
      Database = CR (Link, HII_DATABASE_RECORD, DatabaseEntry, HII_DATABASE_RECORD_SIGNATURE);      
      CurrentDevicePath = Database->PackageList->DevicePathPkg + sizeof (EFI_HII_PACKAGE_HEADER);
      if (CurrentDevicePath != NULL) {
        if (EfiCompareMem (
              DevicePath, 
              CurrentDevicePath, 
              EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) CurrentDevicePath)
              ) == 0) {
          DriverHandle = Database->DriverHandle;
          break;
        }
      }
    }
    
    EfiLibSafeFreePool (DevicePath);
    
    if (DriverHandle == NULL) {
      //
      // Routing data does not match any known driver.
      // Set Progress to the 'G' in "GUID" of the routing header.
      //
      *Progress = StringPtr;
      EfiLibSafeFreePool (ConfigResp);
      return EFI_NOT_FOUND;
    }

    //
    // Call corresponding ConfigAccess protocol to route settings 
    //
    Status = gBS->HandleProtocol (
                    DriverHandle,
                    &gEfiHiiConfigAccessProtocolGuid,
                    (VOID *) &ConfigAccess
                    );
    ASSERT_EFI_ERROR (Status);

    Status = ConfigAccess->RouteConfig (
                             ConfigAccess, 
                             ConfigResp, 
                             &AccessProgress
                             );    

    if (EFI_ERROR (Status)) {
      //
      // AccessProgress indicates the parsing progress on <ConfigResp>.
      // Map it to the progress on <MultiConfigResp> then return it.
      //
      RemainSize = EfiStrSize (AccessProgress);
      for (TmpPtr = StringPtr; EfiCompareMem (TmpPtr, AccessProgress, RemainSize) != 0; TmpPtr++);      
      *Progress = TmpPtr;

      EfiLibSafeFreePool (ConfigResp);      
      return Status;
    }
    
    EfiLibSafeFreePool (ConfigResp);
    ConfigResp = NULL;
    
    //
    // Go to next <ConfigResp> (skip '&').
    //    
    StringPtr += Length;
    if (*StringPtr == 0) {
      *Progress = StringPtr;
      break;
    }

    StringPtr++;

  } 

  return EFI_SUCCESS;
#else
  return EFI_UNSUPPORTED;
#endif
}  

EFI_STATUS 
EFIAPI
HiiBlockToConfig (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL  *This,
  IN  CONST EFI_STRING                       ConfigRequest,
  IN  CONST UINT8                            *Block,
  IN  CONST UINTN                            BlockSize,
  OUT EFI_STRING                             *Config,
  OUT EFI_STRING                             *Progress
  )
/*++

  Routine Description:
    This helper function is to be called by drivers to map configuration data 
    stored in byte array ("block") formats such as UEFI Variables into current 
    configuration strings.
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.    
    ConfigRequest          - A null-terminated Unicode string in <ConfigRequest> format.
    Block                  - Array of bytes defining the block's configuration.
    BlockSize              - Length in bytes of Block.
    Config                 - Filled-in configuration string. String allocated by 
                             the function. Returned only if call is successful.                                                                               
    Progress               - A pointer to a string filled in with the offset of 
                             the most recent & before the first failing name/value
                             pair (or the beginning of the string if the failure
                             is in the first name / value pair) or the terminating
                             NULL if all was successful.
                        
  Returns:              
    EFI_SUCCESS            - The request succeeded. Progress points to the null
                             terminator at the end of the ConfigRequest        
                             string.                                                                        
    EFI_OUT_OF_RESOURCES   - Not enough memory to allocate Config.    
                             Progress points to the first character of
                             ConfigRequest.                           
    EFI_INVALID_PARAMETER  - Passing in a NULL for the ConfigRequest or      
                             Block parameter would result in this type of    
                             error. Progress points to the first character of
                             ConfigRequest.                                   
    EFI_DEVICE_ERROR       - Block not large enough. Progress undefined.
    EFI_INVALID_PARAMETER  - Encountered non <BlockName> formatted string.         
                             Block is left updated and Progress points at the "&"
                             preceding the first non-<BlockName>.                  
                                                          
--*/      
{
  HII_DATABASE_PRIVATE_DATA           *Private;
  EFI_STRING                          StringPtr;
  UINTN                               Length;
  EFI_STATUS                          Status;
  EFI_STRING                          TmpPtr;
  UINT8                               *TmpBuffer;
  UINTN                               Offset;
  UINTN                               Width;
  UINT8                               *Value;
  EFI_STRING                          ValueStr;
  EFI_STRING                          ConfigElement;

  if (This == NULL || Progress == NULL || Config == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Block == NULL || ConfigRequest == NULL) {
    *Progress = ConfigRequest;
    return EFI_INVALID_PARAMETER;
  }

 
  Private = CONFIG_ROUTING_DATABASE_PRIVATE_DATA_FROM_THIS (This);

  StringPtr     = ConfigRequest;
  ValueStr      = NULL;
  Value         = NULL;
  ConfigElement = NULL; 

  //
  // Allocate a fix length of memory to store Results. Reallocate memory for 
  // Results if this fix length is insufficient.
  //
  *Config = (EFI_STRING) EfiLibAllocateZeroPool (MAX_STRING_LENGTH);
  if (*Config == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Jump <ConfigHdr> 
  //  
  if (EfiStrnCmp (StringPtr, L"GUID=", EfiStrLen (L"GUID=")) != 0) {
    *Progress = StringPtr;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }
  while (*StringPtr != 0 && EfiStrnCmp (StringPtr, L"PATH=", EfiStrLen (L"PATH=")) != 0) {
    StringPtr++;
  }
  if (*StringPtr == 0) {
    *Progress = StringPtr;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }
  while (*StringPtr++ != L'&');

  //
  // Copy <ConfigHdr> and an additional '&' to <ConfigResp>
  //
  Length = StringPtr - ConfigRequest;
  EfiCopyMem (*Config, ConfigRequest, Length * sizeof (CHAR16));
  
  //
  // Parse each <RequestElement> if exists
  // Only <BlockName> format is supported by this help function.
  // <BlockName> ::= 'OFFSET='<Number>&'WIDTH='<Number>
  //
  while (*StringPtr != 0 && EfiStrnCmp (StringPtr, L"OFFSET=", EfiStrLen (L"OFFSET=")) == 0) {
    //
    // Back up the header of one <BlockName>
    //
    TmpPtr = StringPtr;
    
    StringPtr += EfiStrLen (L"OFFSET=");
    //
    // Get Offset
    //
    Status = GetValueOfNumber (StringPtr, &TmpBuffer, &Length);
    if (Status == EFI_OUT_OF_RESOURCES) {
      *Progress = ConfigRequest;
      goto Exit;
    }
    Offset = 0;
    EfiCopyMem (
      &Offset,
      TmpBuffer,
      (((Length + 1) / 2) < sizeof (UINTN)) ? ((Length + 1) / 2) : sizeof (UINTN)
      );
    EfiLibSafeFreePool (TmpBuffer);
    
    StringPtr += Length;
    if (EfiStrnCmp (StringPtr, L"&WIDTH=", EfiStrLen (L"&WIDTH=")) != 0) {
      *Progress = StringPtr - Length - EfiStrLen (L"OFFSET=") - 1;
      Status = EFI_INVALID_PARAMETER;
      goto Exit;
    }
    StringPtr += EfiStrLen (L"&WIDTH=");
    
    //
    // Get Width
    //
    Status = GetValueOfNumber (StringPtr, &TmpBuffer, &Length);
    if (Status == EFI_OUT_OF_RESOURCES) {
      *Progress = ConfigRequest;
      goto Exit;
    }
    Width = 0;
    EfiCopyMem (
      &Width,
      TmpBuffer,
      (((Length + 1) / 2) < sizeof (UINTN)) ? ((Length + 1) / 2) : sizeof (UINTN)
      );
    EfiLibSafeFreePool (TmpBuffer);    

    StringPtr += Length;
    if (*StringPtr != 0 && *StringPtr != L'&') {
      *Progress = StringPtr - Length - EfiStrLen (L"&WIDTH=");      
      Status = EFI_INVALID_PARAMETER;      
      goto Exit;
    }

    //
    // Calculate Value and convert it to hex string.
    //
    if (Offset + Width > BlockSize) {
      *Progress = StringPtr;
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Value = (UINT8 *) EfiLibAllocateZeroPool (Width);
    if (Value == NULL) {
      *Progress = ConfigRequest;
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }

    EfiCopyMem (Value, (UINT8 *) Block + Offset, Width);

    Length = Width * 2 + 1;
    ValueStr = (EFI_STRING) EfiLibAllocateZeroPool (Length  * sizeof (CHAR16));
    if (ValueStr == NULL) {
      *Progress = ConfigRequest;
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }
      
    Status = BufToHexString (ValueStr, &Length, Value, Width);        
    ASSERT_EFI_ERROR (Status);
    ToLower (ValueStr);
    
    EfiLibSafeFreePool (Value);
    Value = NULL;
    
    //
    // Build a ConfigElement
    //
    Length += StringPtr - TmpPtr + 1 + EfiStrLen (L"VALUE=");
    ConfigElement = (EFI_STRING) EfiLibAllocateZeroPool (Length * sizeof (CHAR16));
    if (ConfigElement == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }
    EfiCopyMem (ConfigElement, TmpPtr, (StringPtr - TmpPtr + 1) * sizeof (CHAR16));
    if (*StringPtr == 0) {
      *(ConfigElement + (StringPtr - TmpPtr)) = L'&';
    }
    *(ConfigElement + (StringPtr - TmpPtr) + 1) = 0;
    EfiStrCat (ConfigElement, L"VALUE=");
    EfiStrCat (ConfigElement, ValueStr);

    AppendToMultiString (Config, ConfigElement);
    
    EfiLibSafeFreePool (ConfigElement);    
    EfiLibSafeFreePool (ValueStr);
    ConfigElement = NULL;
    ValueStr = NULL;    

    //
    // If '\0', parsing is finished. Otherwise skip '&' to continue
    //
    if (*StringPtr == 0) {
      break;
    }
    AppendToMultiString (Config, L"&");
    StringPtr++;

  }

  if (*StringPtr != 0) {
    *Progress = StringPtr - 1;    
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  *Progress = StringPtr;
  return EFI_SUCCESS;
  
Exit:
  
  EfiLibSafeFreePool (*Config);
  EfiLibSafeFreePool (ValueStr);
  EfiLibSafeFreePool (Value);
  EfiLibSafeFreePool (ConfigElement);
  
  return Status;

}

EFI_STATUS 
EFIAPI
HiiConfigToBlock (
  IN     CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *This,
  IN     CONST EFI_STRING                      ConfigResp,
  IN OUT UINT8                                 *Block,
  IN OUT UINTN                                 *BlockSize,
  OUT    EFI_STRING                            *Progress
  )
/*++

  Routine Description:
    This helper function is to be called by drivers to map configuration strings 
    to configurations stored in byte array ("block") formats such as UEFI Variables.
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.    
    ConfigResp             - A null-terminated Unicode string in <ConfigResp> format.
    Block                  - A possibly null array of bytes representing the current 
                             block. Only bytes referenced in the ConfigResp string 
                             in the block are modified. If this parameter is null
                             or if the *BlockSize parameter is (on input) shorter than
                             required by the Configuration string, only the BlockSize 
                             parameter is updated and an appropriate status (see below) 
                             is returned.            
                             
    BlockSize              - The length of the Block in units of UINT8. 
                             On input, this is the size of the Block.
                             On output, if successful, contains the index of the 
                             last modified byte in the Block.
                             
    Progress               - On return, points to an element of the ConfigResp 
                             string filled in with the offset of the most recent
                             '&' before the first failing name / value pair (or 
                             the beginning of the string if the failure is in the 
                             first name / value pair) or the terminating NULL if
                             all was successful.

  Returns:  
    EFI_SUCCESS            - The request succeeded. Progress points to the null
                             terminator at the end of the ConfigResp
                             string.                                           
    EFI_OUT_OF_RESOURCES   - Not enough memory to allocate Config.    
                             Progress points to the first character of
                             ConfigResp.                           
    EFI_INVALID_PARAMETER  - Passing in a NULL for the ConfigResp or         
                             Block parameter would result in this type of error.
                             Progress points to the first character of          
                             ConfigResp.                                                                  
    EFI_INVALID_PARAMETER  - Encountered non <BlockName> formatted name /    
                             value pair. Block is left updated and           
                             Progress points at the '&' preceding the first
                             non-<BlockName>.                                
                             
--*/                         
{
  HII_DATABASE_PRIVATE_DATA           *Private;
  EFI_STRING                          StringPtr;
  UINTN                               Length;
  EFI_STATUS                          Status;
  UINT8                               *TmpBuffer;
  UINTN                               Offset;
  UINTN                               Width;
  UINT8                               *Value;
  UINTN                               BufferSize;

  if (This == NULL || BlockSize == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ConfigResp == NULL || Block == NULL) {
    *Progress = ConfigResp;
    return EFI_INVALID_PARAMETER;
  }

  Private = CONFIG_ROUTING_DATABASE_PRIVATE_DATA_FROM_THIS (This);

  StringPtr  = ConfigResp;
  BufferSize = *BlockSize;
  Value      = NULL;
  
  //
  // Jump <ConfigHdr> 
  //  
  if (EfiStrnCmp (StringPtr, L"GUID=", EfiStrLen (L"GUID=")) != 0) {
    *Progress = StringPtr;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }  
  while (*StringPtr != 0 && EfiStrnCmp (StringPtr, L"PATH=", EfiStrLen (L"PATH=")) != 0) {
    StringPtr++;
  }
  if (*StringPtr == 0) {
    *Progress = StringPtr;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }
  while (*StringPtr++ != L'&');

  //
  // Parse each <ConfigElement> if exists
  // Only <BlockConfig> format is supported by this help function.
  // <BlockConfig> ::= 'OFFSET='<Number>&'WIDTH='<Number>&'VALUE='<Number>
  //
  while (*StringPtr != 0 && EfiStrnCmp (StringPtr, L"OFFSET=", EfiStrLen (L"OFFSET=")) == 0) {
    StringPtr += EfiStrLen (L"OFFSET=");
    //
    // Get Offset
    //
    Status = GetValueOfNumber (StringPtr, &TmpBuffer, &Length);
    if (Status == EFI_OUT_OF_RESOURCES) {
      *Progress = ConfigResp;
      goto Exit;
    }
    Offset = 0;
    EfiCopyMem (
      &Offset,
      TmpBuffer,
      (((Length + 1) / 2) < sizeof (UINTN)) ? ((Length + 1) / 2) : sizeof (UINTN)
      );
    EfiLibSafeFreePool (TmpBuffer);    
    
    StringPtr += Length;
    if (EfiStrnCmp (StringPtr, L"&WIDTH=", EfiStrLen (L"&WIDTH=")) != 0) {
      *Progress = StringPtr - Length - EfiStrLen (L"OFFSET=") - 1;
      Status = EFI_INVALID_PARAMETER;
      goto Exit;
    }
    StringPtr += EfiStrLen (L"&WIDTH=");

    //
    // Get Width
    //
    Status = GetValueOfNumber (StringPtr, &TmpBuffer, &Length);
    if (Status == EFI_OUT_OF_RESOURCES) {
      *Progress = ConfigResp;
      goto Exit;
    }
    Width = 0;
    EfiCopyMem (
      &Width,
      TmpBuffer,
      (((Length + 1) / 2) < sizeof (UINTN)) ? ((Length + 1) / 2) : sizeof (UINTN)
      );
    EfiLibSafeFreePool (TmpBuffer);

    StringPtr += Length;
    if (EfiStrnCmp (StringPtr, L"&VALUE=", EfiStrLen (L"&VALUE=")) != 0) {
      *Progress = StringPtr - Length - EfiStrLen (L"&WIDTH=");      
      Status = EFI_INVALID_PARAMETER;      
      goto Exit;
    }
    StringPtr += EfiStrLen (L"&VALUE=");

    //
    // Get Value
    //
    Status = GetValueOfNumber (StringPtr, &Value, &Length);
    if (Status == EFI_OUT_OF_RESOURCES) {
      *Progress = ConfigResp;
      goto Exit;
    }

    StringPtr += Length;
    if (*StringPtr != 0 && *StringPtr != L'&') {
      *Progress = StringPtr - Length - 7;      
      Status = EFI_INVALID_PARAMETER;      
      goto Exit;
    }    

    //
    // Update the Block with configuration info
    //
    
    if (Offset + Width > BufferSize) {
      return EFI_DEVICE_ERROR;
    }

    EfiCopyMem (Block + Offset, Value, Width);
    *BlockSize = Offset + Width - 1;

    EfiLibSafeFreePool (Value);
    Value = NULL;
    
    //
    // If '\0', parsing is finished. Otherwise skip '&' to continue
    //
    if (*StringPtr == 0) {
      break;
    }

    StringPtr++;    
  }  

  if (*StringPtr != 0) {
    *Progress = StringPtr - 1;    
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  *Progress = StringPtr;
  return EFI_SUCCESS;

Exit:
  
  EfiLibSafeFreePool (Value);
  return Status;
}

EFI_STATUS 
EFIAPI
HiiGetAltCfg (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL    *This, 
  IN  CONST EFI_STRING                         Configuration, 
  IN  CONST EFI_GUID                           *Guid, 
  IN  CONST EFI_STRING                         Name, 
  IN  CONST EFI_DEVICE_PATH_PROTOCOL           *DevicePath,  
  IN  CONST UINT16                             *AltCfgId, 
  OUT EFI_STRING                               *AltCfgResp 
  )
/*++

  Routine Description:
    This helper function is to be called by drivers to extract portions of 
    a larger configuration string.
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.    
    Configuration          - A null-terminated Unicode string in <MultiConfigAltResp> format.
    Guid                   - A pointer to the GUID value to search for in the 
                             routing portion of the ConfigResp string when retrieving 
                             the requested data. If Guid is NULL, then all GUID 
                             values will be searched for.
    Name                   - A pointer to the NAME value to search for in the 
                             routing portion of the ConfigResp string when retrieving 
                             the requested data. If Name is NULL, then all Name 
                             values will be searched for.                         
    DevicePath             - A pointer to the PATH value to search for in the 
                             routing portion of the ConfigResp string when retrieving 
                             the requested data. If DevicePath is NULL, then all 
                             DevicePath values will be searched for.             
    AltCfgId               - A pointer to the ALTCFG value to search for in the 
                             routing portion of the ConfigResp string when retrieving 
                             the requested data.  If this parameter is NULL, 
                             then the current setting will be retrieved.
    AltCfgResp             - A pointer to a buffer which will be allocated by the 
                             function which contains the retrieved string as requested.  
                             This buffer is only allocated if the call was successful. 
    
  Returns:              
    EFI_SUCCESS            - The request succeeded. The requested data was extracted 
                             and placed in the newly allocated AltCfgResp buffer.
    EFI_OUT_OF_RESOURCES   - Not enough memory to allocate AltCfgResp.    
    EFI_INVALID_PARAMETER  - Any parameter is invalid.
    EFI_NOT_FOUND          - Target for the specified routing data was not found.
                             
--*/        
{
#ifndef DISABLE_UNUSED_HII_PROTOCOLS

  EFI_STATUS                          Status;
  EFI_STRING                          StringPtr;
  EFI_STRING                          HdrStart = NULL;  
  EFI_STRING                          HdrEnd   = NULL;
  EFI_STRING                          TmpPtr;  
  UINTN                               Length;
  EFI_STRING                          GuidStr  = NULL;
  EFI_STRING                          NameStr  = NULL;
  EFI_STRING                          PathStr  = NULL;  
  EFI_STRING                          AltIdStr = NULL;
  EFI_STRING                          Result   = NULL;  
  BOOLEAN                             GuidFlag = FALSE;
  BOOLEAN                             NameFlag = FALSE;
  BOOLEAN                             PathFlag = FALSE;

  if (This == NULL || Configuration == NULL || AltCfgResp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  StringPtr = Configuration;  
  if (EfiStrnCmp (StringPtr, L"GUID=", EfiStrLen (L"GUID=")) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Generate the sub string for later matching.
  //
  GenerateSubStr (L"GUID=", sizeof (EFI_GUID), (VOID *) Guid, 1, &GuidStr);
  GenerateSubStr (
    L"PATH=", 
    EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) DevicePath), 
    (VOID *) DevicePath,
    1,
    &PathStr
    );   
  if (AltCfgId != NULL) {
    GenerateSubStr (L"ALTCFG=", sizeof (UINT16), (VOID *) AltCfgId, 3, &AltIdStr);  
  }  
  if (Name != NULL) {
    GenerateSubStr (L"NAME=", EfiStrLen (Name) * sizeof (CHAR16), (VOID *) Name, 2, &NameStr);    
  } else {
    GenerateSubStr (L"NAME=", 0, NULL, 2, &NameStr);
  }
     
  while (*StringPtr != 0) {
    //
    // Try to match the GUID
    //
    if (!GuidFlag) {      
      TmpPtr = EfiStrStr (StringPtr, GuidStr);
      if (TmpPtr == NULL) {
        Status = EFI_NOT_FOUND;
        goto Exit;
      }
      HdrStart = TmpPtr;
      
      //
      // Jump to <NameHdr>
      //
      if (Guid != NULL) {
        StringPtr = TmpPtr + EfiStrLen (GuidStr);
      } else {
        StringPtr = EfiStrStr (TmpPtr, L"NAME=");
        if (StringPtr == NULL) {
          Status = EFI_NOT_FOUND;
          goto Exit;
        }        
      }
      GuidFlag = TRUE;
    }

    //
    // Try to match the NAME
    //
    if (GuidFlag && !NameFlag) {
      if (EfiStrnCmp (StringPtr, NameStr, EfiStrLen (NameStr)) != 0) {
        GuidFlag = FALSE;
      } else {
        //
        // Jump to <PathHdr>
        //
        if (Name != NULL) {
          StringPtr += EfiStrLen (NameStr);
        } else {
          StringPtr = EfiStrStr (StringPtr, L"PATH=");
          if (StringPtr == NULL) {
            Status = EFI_NOT_FOUND;
            goto Exit;
          }
        }
        NameFlag = TRUE;
      }      
    }

    //
    // Try to match the DevicePath
    //
    if (GuidFlag && NameFlag && !PathFlag) {
      if (EfiStrnCmp (StringPtr, PathStr, EfiStrLen (PathStr)) != 0) {
        GuidFlag = FALSE;
        NameFlag = FALSE;
      } else {
        //
        // Jump to '&' before <DescHdr> or <ConfigBody>
        //
        if (DevicePath != NULL) {
          StringPtr += EfiStrLen (PathStr);
        } else {
          StringPtr = EfiStrStr (StringPtr, L"&");
          if (StringPtr == NULL) {
            Status = EFI_NOT_FOUND;
            goto Exit;
          }
        }
        PathFlag = TRUE;
        HdrEnd   = ++StringPtr;
      }
    }

    //
    // Try to match the AltCfgId
    //
    if (GuidFlag && NameFlag && PathFlag) {
      if (AltCfgId == NULL) {
        //
        // Return Current Setting when AltCfgId is NULL.
        //
        Status = OutputConfigBody (StringPtr, &Result);
        goto Exit;                        
      }
      //
      // Search the <ConfigAltResp> to get the <AltResp> with AltCfgId.
      //
      if (EfiStrnCmp (StringPtr, AltIdStr, EfiStrLen (AltIdStr)) != 0) {
        GuidFlag = FALSE;
        NameFlag = FALSE;
        PathFlag = FALSE;
      } else {
        Status = OutputConfigBody (StringPtr, &Result);
        goto Exit;
      }      
    }    
  }

  Status = EFI_NOT_FOUND;

Exit:

  if (!EFI_ERROR (Status)) {
    //
    // Copy the <ConfigHdr> and <ConfigBody>
    //
    Length = HdrEnd - HdrStart + EfiStrLen (Result);
    *AltCfgResp = EfiLibAllocateZeroPool (Length * sizeof (CHAR16));
    if (*AltCfgResp == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      EfiStrnCpy (*AltCfgResp, HdrStart, HdrEnd - HdrStart);
      EfiStrCat (*AltCfgResp, Result);
      Status = EFI_SUCCESS;
    }    
  }
  
  EfiLibSafeFreePool (GuidStr);
  EfiLibSafeFreePool (NameStr);
  EfiLibSafeFreePool (PathStr);
  EfiLibSafeFreePool (AltIdStr);
  EfiLibSafeFreePool (Result);

  return Status;

#else
  return EFI_UNSUPPORTED;
#endif

}

