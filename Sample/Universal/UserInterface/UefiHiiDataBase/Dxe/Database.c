/*++

Copyright (c) 2007 - 2010, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    Database.c
    
Abstract:

    Implementation for EFI_HII_DATABASE_PROTOCOL.

Revision History

--*/

#include "HiiDatabase.h"

//
// Global variables
//
STATIC EFI_GUID mHiiDatabaseNotifyGuid = HII_DATABASE_NOTIFY_GUID;

STATIC
EFI_STATUS
GenerateHiiDatabaseRecord (
  IN  HII_DATABASE_PRIVATE_DATA *Private,
  OUT HII_DATABASE_RECORD       **DatabaseNode
  )
/*++

  Routine Description:
    This function generates a HII_DATABASE_RECORD node and adds into hii database. 
    
  Arguments:          
    Private           - hii database private structure 
    DatabaseRecord    - HII_DATABASE_RECORD node which is used to store a package list
    
  Returns:
    EFI_SUCCESS            - A database record is generated successfully.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new database contents.
    EFI_INVALID_PARAMETER  - Private is NULL or DatabaseRecord is NULL.
     
--*/  
  
{
  HII_DATABASE_RECORD                *DatabaseRecord;
  HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList;
  HII_HANDLE                         *HiiHandle;
  
  if (Private == NULL || DatabaseNode == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  DatabaseRecord = (HII_DATABASE_RECORD *) EfiLibAllocateZeroPool (sizeof (HII_DATABASE_RECORD));
  if (DatabaseRecord == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  DatabaseRecord->Signature = HII_DATABASE_RECORD_SIGNATURE;

  DatabaseRecord->PackageList = EfiLibAllocateZeroPool (sizeof (HII_DATABASE_PACKAGE_LIST_INSTANCE));
  if (DatabaseRecord->PackageList == NULL) {
    EfiLibSafeFreePool (DatabaseRecord);
    return EFI_OUT_OF_RESOURCES;
  }

  PackageList = DatabaseRecord->PackageList;
  
  InitializeListHead (&PackageList->GuidPkgHdr);
  InitializeListHead (&PackageList->FormPkgHdr);
  InitializeListHead (&PackageList->KeyboardLayoutHdr);
  InitializeListHead (&PackageList->StringPkgHdr);
  InitializeListHead (&PackageList->FontPkgHdr);
  InitializeListHead (&PackageList->SimpleFontPkgHdr);  
  PackageList->ImagePkg      = NULL;
  PackageList->DevicePathPkg = NULL;
    
  //
  // Create a new hii handle
  //
  HiiHandle = (HII_HANDLE *) EfiLibAllocateZeroPool (sizeof (HII_HANDLE));
  if (HiiHandle == NULL) {
    EfiLibSafeFreePool (DatabaseRecord->PackageList);    
    EfiLibSafeFreePool (DatabaseRecord);
    return EFI_OUT_OF_RESOURCES;
  }
  HiiHandle->Signature = HII_HANDLE_SIGNATURE;
  //
  // Backup the number of Hii handles
  //
  Private->HiiHandleCount++;
  HiiHandle->Key = Private->HiiHandleCount;
  //
  // Insert the handle to hii handle list of the whole database.
  //
  InsertTailList (&Private->HiiHandleList, &HiiHandle->Handle);

  DatabaseRecord->Handle = (EFI_HII_HANDLE) HiiHandle;  

  //
  // Insert the Package List node to Package List link of the whole database.
  //
  InsertTailList (&Private->DatabaseList, &DatabaseRecord->DatabaseEntry);

  *DatabaseNode = DatabaseRecord;

  return EFI_SUCCESS;
}

BOOLEAN
IsHiiHandleValid (
  EFI_HII_HANDLE Handle
  )
/*++

  Routine Description:
    This function checks whether a handle is a valid EFI_HII_HANDLE
    
  Arguments:          
    Handle        - Pointer to a EFI_HII_HANDLE
    
  Returns:
    TRUE          - Valid
    FALSE         - Invalid
    
--*/  
  
{
  HII_HANDLE    *HiiHandle;  

  HiiHandle = (HII_HANDLE *) Handle;
  
  if (HiiHandle == NULL) {
    return FALSE;
  }

  if (HiiHandle->Signature != HII_HANDLE_SIGNATURE) {
    return FALSE;
  }

  return TRUE;
}

EFI_STATUS
InvokeRegisteredFunction (
  IN HII_DATABASE_PRIVATE_DATA    *Private, 
  IN EFI_HII_DATABASE_NOTIFY_TYPE NotifyType,
  IN VOID                         *PackageInstance,
  IN UINT8                        PackageType,
  IN EFI_HII_HANDLE               Handle
  )
/*++

  Routine Description:
    This function invokes the matching registered function.
    
  Arguments:          
    Private           - HII Database driver private structure.
    NotifyType        - The type of change concerning the database.
    PackageInstance   - Points to the package referred to by the notification.
    PackageType       - Package type
    Handle            - The handle of the package list which contains the specified package.
    
  Returns:
    EFI_SUCCESS            - Already checked all registered function and invoked 
                             if matched.
    EFI_INVALID_PARAMETER  - Any input parameter is not valid.
     
--*/
{
  HII_DATABASE_NOTIFY             *Notify;
  EFI_LIST_ENTRY                  *Link;
  EFI_HII_PACKAGE_HEADER          *Package;
  UINT8                           *Buffer;
  UINT32                          BufferSize;
  UINT32                          HeaderSize;
  UINT32                          ImageBlockSize;
  UINT32                          PaletteInfoSize;
 
  if (Private == NULL || (NotifyType & 0xF) == 0 || PackageInstance == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (Private->Signature != HII_DATABASE_PRIVATE_DATA_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }
  if (!IsHiiHandleValid (Handle)) {
    return EFI_INVALID_PARAMETER;
  }

  Buffer  = NULL;
  Package = NULL;

  //
  // Convert the incoming package from hii database storage format to UEFI
  // storage format. e.g. HII_GUID_PACKAGE_INSTANCE to EFI_HII_GUID_PACKAGE_HDR.
  //
  switch (PackageType) {
  case EFI_HII_PACKAGE_TYPE_GUID:
    Package = (EFI_HII_PACKAGE_HEADER *) (((HII_GUID_PACKAGE_INSTANCE *) PackageInstance)->GuidPkg);
    break;
    
  case EFI_HII_PACKAGE_FORMS:
    BufferSize = ((HII_IFR_PACKAGE_INSTANCE *) PackageInstance)->FormPkgHdr.Length;
    Buffer = (UINT8 *) EfiLibAllocateZeroPool (BufferSize);
    ASSERT (Buffer != NULL);
    EfiCopyMem (
      Buffer, 
      &((HII_IFR_PACKAGE_INSTANCE *) PackageInstance)->FormPkgHdr,
      sizeof (EFI_HII_PACKAGE_HEADER)
      );
    EfiCopyMem (
      Buffer + sizeof (EFI_HII_PACKAGE_HEADER),
      ((HII_IFR_PACKAGE_INSTANCE *) PackageInstance)->IfrData,
      BufferSize - sizeof (EFI_HII_PACKAGE_HEADER)
      );
    Package = (EFI_HII_PACKAGE_HEADER *) Buffer;
    break;
    
  case EFI_HII_PACKAGE_KEYBOARD_LAYOUT:
    Package = (EFI_HII_PACKAGE_HEADER *) (((HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE *) PackageInstance)->KeyboardPkg);
    break;
    
  case EFI_HII_PACKAGE_STRINGS:
    BufferSize = ((HII_STRING_PACKAGE_INSTANCE *) PackageInstance)->StringPkgHdr->Header.Length;
    HeaderSize = ((HII_STRING_PACKAGE_INSTANCE *) PackageInstance)->StringPkgHdr->HdrSize;
    Buffer = (UINT8 *) EfiLibAllocateZeroPool (BufferSize);
    ASSERT (Buffer != NULL);
    EfiCopyMem (
      Buffer,
      ((HII_STRING_PACKAGE_INSTANCE *) PackageInstance)->StringPkgHdr,
      HeaderSize
      );
    EfiCopyMem (
      Buffer + HeaderSize,
      ((HII_STRING_PACKAGE_INSTANCE *) PackageInstance)->StringBlock,
      BufferSize - HeaderSize
      );
    Package = (EFI_HII_PACKAGE_HEADER *) Buffer;
    break;

  case EFI_HII_PACKAGE_FONTS:
    BufferSize = ((HII_FONT_PACKAGE_INSTANCE *) PackageInstance)->FontPkgHdr->Header.Length;
    HeaderSize = ((HII_FONT_PACKAGE_INSTANCE *) PackageInstance)->FontPkgHdr->HdrSize;
    Buffer = (UINT8 *) EfiLibAllocateZeroPool (BufferSize);
    ASSERT (Buffer != NULL);
    EfiCopyMem (
      Buffer,
      ((HII_FONT_PACKAGE_INSTANCE *) PackageInstance)->FontPkgHdr,
      HeaderSize
      );
    EfiCopyMem (
      Buffer + HeaderSize,
      ((HII_FONT_PACKAGE_INSTANCE *) PackageInstance)->GlyphBlock,
      BufferSize - HeaderSize
      );
    Package = (EFI_HII_PACKAGE_HEADER *) Buffer;
    break;
    
  case EFI_HII_PACKAGE_IMAGES:
    BufferSize = ((HII_IMAGE_PACKAGE_INSTANCE *) PackageInstance)->ImagePkgHdr.Header.Length;
    HeaderSize = sizeof (EFI_HII_IMAGE_PACKAGE_HDR);
    Buffer = (UINT8 *) EfiLibAllocateZeroPool (BufferSize);
    ASSERT (Buffer != NULL);
    
    EfiCopyMem (
      Buffer,
      &((HII_IMAGE_PACKAGE_INSTANCE *) PackageInstance)->ImagePkgHdr,
      HeaderSize
      );
    EfiCopyMem (
      Buffer + sizeof (EFI_HII_PACKAGE_HEADER),
      &HeaderSize,
      sizeof (UINT32)      
      );
    
    ImageBlockSize = ((HII_IMAGE_PACKAGE_INSTANCE *) PackageInstance)->ImageBlockSize;
    if (ImageBlockSize != 0) {
      EfiCopyMem (
        Buffer + HeaderSize,
        ((HII_IMAGE_PACKAGE_INSTANCE *) PackageInstance)->ImageBlock,
        ImageBlockSize        
        );
    }
    
    PaletteInfoSize = ((HII_IMAGE_PACKAGE_INSTANCE *) PackageInstance)->PaletteInfoSize;
    if (PaletteInfoSize != 0) {
      EfiCopyMem (
        Buffer + HeaderSize + ImageBlockSize,
        ((HII_IMAGE_PACKAGE_INSTANCE *) PackageInstance)->PaletteBlock,
        PaletteInfoSize
        );
      HeaderSize += ImageBlockSize;
      EfiCopyMem (
        Buffer + sizeof (EFI_HII_PACKAGE_HEADER) + sizeof (UINT32),
        &HeaderSize,
        sizeof (UINT32)
        );
    }    
    Package = (EFI_HII_PACKAGE_HEADER *) Buffer;  
    break;

  case EFI_HII_PACKAGE_SIMPLE_FONTS:
    BufferSize = ((HII_SIMPLE_FONT_PACKAGE_INSTANCE *) PackageInstance)->SimpleFontPkgHdr->Header.Length;
    Buffer = (UINT8 *) EfiLibAllocateZeroPool (BufferSize);
    ASSERT (Buffer != NULL);
    EfiCopyMem (
      Buffer, 
      ((HII_SIMPLE_FONT_PACKAGE_INSTANCE *) PackageInstance)->SimpleFontPkgHdr,
      BufferSize
      );
    Package = (EFI_HII_PACKAGE_HEADER *) Buffer;
    break;

  case EFI_HII_PACKAGE_DEVICE_PATH:
    Package = (EFI_HII_PACKAGE_HEADER *) PackageInstance;
    break;
    
  default:
    return EFI_INVALID_PARAMETER;
  }
  
  for (Link = Private->DatabaseNotifyList.ForwardLink; 
       Link != &Private->DatabaseNotifyList; 
       Link = Link->ForwardLink
      ) {
    Notify = CR (Link, HII_DATABASE_NOTIFY, DatabaseNotifyEntry, HII_DATABASE_NOTIFY_SIGNATURE);
    if (Notify->NotifyType == NotifyType && Notify->PackageType == PackageType) {
      //
      // Check in case PackageGuid is not NULL when Package is GUID package
      //
      if (PackageType != EFI_HII_PACKAGE_TYPE_GUID) {
        Notify->PackageGuid = NULL;
      }
      //
      // Status of Registered Function is unknown so did not check it
      //
      Notify->PackageNotifyFn (
        Notify->PackageType,
        Notify->PackageGuid,
        Package,
        Handle,
        NotifyType
        );
    }
  }

  EfiLibSafeFreePool (Buffer);
  Buffer = NULL;
  
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
InsertGuidPackage (
  IN     VOID                               *PackageHdr,
  IN     EFI_HII_DATABASE_NOTIFY_TYPE       NotifyType,  
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  OUT    HII_GUID_PACKAGE_INSTANCE          **Package  
  )
/*++

  Routine Description:
    This function insert a GUID package to a package list node.
    
  Arguments:          
    PackageHdr        - Pointer to a buffer stored with GUID package information.
    NotifyType        - The type of change concerning the database.    
    PackageList       - Pointer to a package list which will be inserted to.
    Package           - Created GUID pacakge
    
  Returns:
    EFI_SUCCESS            - Guid Package is inserted successfully.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new Guid package.
    EFI_INVALID_PARAMETER  - PackageHdr is NULL or PackageList is NULL.
     
--*/
{
  HII_GUID_PACKAGE_INSTANCE            *GuidPackage;
  EFI_HII_PACKAGE_HEADER               PackageHeader;

  if (PackageHdr == NULL || PackageList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiCopyMem (&PackageHeader, PackageHdr, sizeof (EFI_HII_PACKAGE_HEADER));
  
  //
  // Create a GUID package node
  //
  GuidPackage = (HII_GUID_PACKAGE_INSTANCE *) EfiLibAllocateZeroPool (sizeof (HII_GUID_PACKAGE_INSTANCE));
  if (GuidPackage == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  GuidPackage->GuidPkg = (UINT8 *) EfiLibAllocateZeroPool (PackageHeader.Length);
  if (GuidPackage->GuidPkg == NULL) {
    EfiLibSafeFreePool (GuidPackage);
    return EFI_OUT_OF_RESOURCES;
  }
  
  GuidPackage->Signature = HII_GUID_PACKAGE_SIGNATURE;
  EfiCopyMem (GuidPackage->GuidPkg, PackageHdr, PackageHeader.Length);
  InsertTailList (&PackageList->GuidPkgHdr, &GuidPackage->GuidEntry);
  *Package = GuidPackage;

  if (NotifyType == EFI_HII_DATABASE_NOTIFY_ADD_PACK) {
    PackageList->PackageListHdr.PackageLength += PackageHeader.Length;
  }
  
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ExportGuidPackages (
  IN HII_DATABASE_PRIVATE_DATA          *Private,
  IN EFI_HII_HANDLE                     Handle,
  IN HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  IN UINTN                              UsedSize,  
  IN UINTN                              BufferSize,
  IN OUT VOID                           *Buffer,
  IN OUT UINTN                          *ResultSize
  )
/*++

  Routine Description:
    This function exports GUID packages to a buffer.
    
  Arguments:          
    Private           - Hii database private structure.
    Handle            - Identification of a package list.
    PackageList       - Pointer to a package list which will be exported.
    UsedSize          - The length of buffer be used.
    BufferSize        - Length of the Buffer.
    Buffer            - Allocated space for storing exported data.
    ResultSize        - The size of the already exported content of 
                        this package list.
    
  Returns:
    EFI_SUCCESS            - Guid Packages are exported successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is invalid.
     
--*/ 
{
  HII_GUID_PACKAGE_INSTANCE            *GuidPackage;
  EFI_LIST_ENTRY                       *Link;
  UINTN                                PackageLength;
  EFI_HII_PACKAGE_HEADER               PackageHeader;
  EFI_STATUS                           Status;

  if (PackageList == NULL || ResultSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize > 0 && Buffer == NULL ) {
    return EFI_INVALID_PARAMETER;
  }

  PackageLength = 0;
  Status        = EFI_SUCCESS;
  
  for (Link = PackageList->GuidPkgHdr.ForwardLink; Link != &PackageList->GuidPkgHdr; Link = Link->ForwardLink) {
    GuidPackage = CR (Link, HII_GUID_PACKAGE_INSTANCE, GuidEntry, HII_GUID_PACKAGE_SIGNATURE);
    EfiCopyMem (&PackageHeader, GuidPackage->GuidPkg, sizeof (EFI_HII_PACKAGE_HEADER));
    PackageLength += PackageHeader.Length;
    if (PackageLength + *ResultSize + UsedSize <= BufferSize) {
      Status = InvokeRegisteredFunction (
                 Private, 
                 EFI_HII_DATABASE_NOTIFY_EXPORT_PACK,
                 (VOID *) GuidPackage,
                 EFI_HII_PACKAGE_TYPE_GUID,
                 Handle
                 );
      ASSERT_EFI_ERROR (Status);
      EfiCopyMem (Buffer, GuidPackage->GuidPkg, PackageHeader.Length);
      Buffer = (UINT8 *) Buffer + PackageHeader.Length;
    }    
  }
  
  *ResultSize += PackageLength;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
RemoveGuidPackages (
  IN     HII_DATABASE_PRIVATE_DATA          *Private, 
  IN     EFI_HII_HANDLE                     Handle,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList  
  )
/*++

  Routine Description:
    This function deletes all GUID packages from a package list node.
    
  Arguments:          
    Private           - Hii database private data.
    Handle            - Handle of the package list which contains the to be 
                        removed GUID packages.
    PackageList       - Pointer to a package list that contains removing packages.
    
  Returns:
    EFI_SUCCESS            - GUID Package(s) is deleted successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is not valid.
     
--*/   
{
  EFI_LIST_ENTRY                       *ListHead;
  HII_GUID_PACKAGE_INSTANCE            *Package;
  EFI_STATUS                           Status;
  EFI_HII_PACKAGE_HEADER               PackageHeader;

  ListHead = &PackageList->GuidPkgHdr;
  
  while (!IsListEmpty (ListHead)) {
    Package = CR (
                ListHead->ForwardLink, 
                HII_GUID_PACKAGE_INSTANCE, 
                GuidEntry, 
                HII_GUID_PACKAGE_SIGNATURE
                );
    Status = InvokeRegisteredFunction (
               Private,
               EFI_HII_DATABASE_NOTIFY_REMOVE_PACK,
               (VOID *) Package,
               EFI_HII_PACKAGE_TYPE_GUID,
               Handle
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    RemoveEntryList (&Package->GuidEntry);
    EfiCopyMem (&PackageHeader, Package->GuidPkg, sizeof (EFI_HII_PACKAGE_HEADER));
    PackageList->PackageListHdr.PackageLength -= PackageHeader.Length;
    EfiLibSafeFreePool (Package->GuidPkg);
    EfiLibSafeFreePool (Package);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
InsertFormPackage (
  IN     VOID                               *PackageHdr,
  IN     EFI_HII_DATABASE_NOTIFY_TYPE       NotifyType,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  OUT    HII_IFR_PACKAGE_INSTANCE           **Package
  )
/*++

  Routine Description:
    This function insert a Form package to a package list node.
    
  Arguments:          
    PackageHdr        - Pointer to a buffer stored with Form package information.
    NotifyType        - The type of change concerning the database.        
    PackageList       - Pointer to a package list which will be inserted to.
    Package           - Created Form package
    
  Returns:
    EFI_SUCCESS            - Form Package is inserted successfully.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new Form package.
    EFI_INVALID_PARAMETER  - PackageHdr is NULL or PackageList is NULL.
     
--*/  
{
  HII_IFR_PACKAGE_INSTANCE *FormPackage;
  EFI_HII_PACKAGE_HEADER   PackageHeader;

  if (PackageHdr == NULL || PackageList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get the length of the package, including package header itself
  //
  EfiCopyMem (&PackageHeader, PackageHdr, sizeof (EFI_HII_PACKAGE_HEADER));
  
  //
  // Create a Form package node
  //
  FormPackage = (HII_IFR_PACKAGE_INSTANCE *) EfiLibAllocateZeroPool (sizeof (HII_IFR_PACKAGE_INSTANCE));
  if (FormPackage == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  FormPackage->IfrData = (UINT8 *) EfiLibAllocateZeroPool (PackageHeader.Length - sizeof (EFI_HII_PACKAGE_HEADER));
  if (FormPackage->IfrData == NULL) {
    EfiLibSafeFreePool (FormPackage);
    return EFI_OUT_OF_RESOURCES;
  }
  
  FormPackage->Signature = HII_IFR_PACKAGE_SIGNATURE;
  //
  // Copy Package Header
  //
  EfiCopyMem (&FormPackage->FormPkgHdr, &PackageHeader, sizeof (EFI_HII_PACKAGE_HEADER));

  //
  // Copy Ifr contents
  //
  EfiCopyMem (
    FormPackage->IfrData, 
    (UINT8 *) PackageHdr + sizeof (EFI_HII_PACKAGE_HEADER), 
    PackageHeader.Length - sizeof (EFI_HII_PACKAGE_HEADER)
    );
  
  InsertTailList (&PackageList->FormPkgHdr, &FormPackage->IfrEntry);
  *Package = FormPackage;

  if (NotifyType == EFI_HII_DATABASE_NOTIFY_ADD_PACK) {
    PackageList->PackageListHdr.PackageLength += FormPackage->FormPkgHdr.Length;
  }
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ExportFormPackages (
  IN HII_DATABASE_PRIVATE_DATA          *Private,
  IN EFI_HII_HANDLE                     Handle,
  IN HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  IN UINTN                              UsedSize,    
  IN UINTN                              BufferSize,
  IN OUT VOID                           *Buffer,
  IN OUT UINTN                          *ResultSize
  )
/*++

  Routine Description:
    This function exports Form packages to a buffer.
    
  Arguments:          
    Private           - Hii database private structure.
    Handle            - Identification of a package list.
    PackageList       - Pointer to a package list which will be exported.
    UsedSize          - The length of buffer be used.
    BufferSize        - Length of the Buffer.
    Buffer            - Allocated space for storing exported data.
    ResultSize        - The size of the already exported content of 
                        this package list.
    
  Returns:
    EFI_SUCCESS            - Form Packages are exported successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is invalid.
     
--*/
{
  HII_IFR_PACKAGE_INSTANCE *FormPackage;
  UINTN                    PackageLength;
  EFI_LIST_ENTRY           *Link; 
  EFI_STATUS               Status;

  if (Private == NULL || PackageList == NULL || ResultSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize > 0 && Buffer == NULL ) {
    return EFI_INVALID_PARAMETER;
  }

  PackageLength = 0;
  Status        = EFI_SUCCESS;
  
  //
  // Export Form packages.
  //
  for (Link = PackageList->FormPkgHdr.ForwardLink; Link != &PackageList->FormPkgHdr; Link = Link->ForwardLink) {
    FormPackage = CR (Link, HII_IFR_PACKAGE_INSTANCE, IfrEntry, HII_IFR_PACKAGE_SIGNATURE);
    PackageLength += FormPackage->FormPkgHdr.Length;
    if (PackageLength + *ResultSize + UsedSize <= BufferSize) {
      //
      // Invoke registered notification if exists
      //
      Status = InvokeRegisteredFunction (
                 Private, 
                 EFI_HII_DATABASE_NOTIFY_EXPORT_PACK,
                 (VOID *) FormPackage,
                 EFI_HII_PACKAGE_FORMS,
                 Handle
                 );
      ASSERT_EFI_ERROR (Status);
      //
      // Copy the Form package content.
      //
      EfiCopyMem (Buffer, (VOID *) (&FormPackage->FormPkgHdr), sizeof (EFI_HII_PACKAGE_HEADER));
      Buffer = (UINT8 *) Buffer + sizeof (EFI_HII_PACKAGE_HEADER);
      EfiCopyMem (
        Buffer, 
        (VOID *) FormPackage->IfrData, 
        FormPackage->FormPkgHdr.Length - sizeof (EFI_HII_PACKAGE_HEADER)
        );
      Buffer = (UINT8 *) Buffer + FormPackage->FormPkgHdr.Length - sizeof (EFI_HII_PACKAGE_HEADER);
    }
  }

  *ResultSize += PackageLength;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
RemoveFormPackages (
  IN     HII_DATABASE_PRIVATE_DATA          *Private, 
  IN     EFI_HII_HANDLE                     Handle,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList
  )
/*++

  Routine Description:
    This function deletes all Form packages from a package list node.
    
  Arguments:          
    Private           - Hii database private data.
    Handle            - Handle of the package list which contains the to be 
                        removed Form packages.
    PackageList       - Pointer to a package list that contains removing packages.                        
                        
  Returns:
    EFI_SUCCESS            - Form Package(s) is deleted successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is not valid.
     
--*/   
{
  EFI_LIST_ENTRY                  *ListHead;
  HII_IFR_PACKAGE_INSTANCE        *Package;
  EFI_STATUS                      Status;

  ListHead = &PackageList->FormPkgHdr;
    
  while (!IsListEmpty (ListHead)) {
    Package = CR (
                ListHead->ForwardLink, 
                HII_IFR_PACKAGE_INSTANCE, 
                IfrEntry, 
                HII_IFR_PACKAGE_SIGNATURE
                );
    Status = InvokeRegisteredFunction (
               Private,
               EFI_HII_DATABASE_NOTIFY_REMOVE_PACK,
               (VOID *) Package,
               EFI_HII_PACKAGE_FORMS,
               Handle
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    RemoveEntryList (&Package->IfrEntry);
    PackageList->PackageListHdr.PackageLength -= Package->FormPkgHdr.Length;
    EfiLibSafeFreePool (Package->IfrData);
    EfiLibSafeFreePool (Package);
    
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
InsertStringPackage (
  IN     HII_DATABASE_PRIVATE_DATA          *Private,
  IN     VOID                               *PackageHdr,
  IN     EFI_HII_DATABASE_NOTIFY_TYPE       NotifyType,    
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  OUT    HII_STRING_PACKAGE_INSTANCE        **Package
  )
/*++

  Routine Description:
    This function insert a String package to a package list node.
    
  Arguments:          
    Private           - Hii database private structure.    
    PackageHdr        - Pointer to a buffer stored with String package information.
    NotifyType        - The type of change concerning the database.        
    PackageList       - Pointer to a package list which will be inserted to.
    Package           - Created String package
    
  Returns:
    EFI_SUCCESS            - String Package is inserted successfully.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new String package.
    EFI_INVALID_PARAMETER  - PackageHdr is NULL or PackageList is NULL.
    EFI_UNSUPPORTED        - A string package with the same language already exists
                             in current package list.
     
--*/
{
  HII_STRING_PACKAGE_INSTANCE *StringPackage;
  UINT32                      HeaderSize;
  EFI_STATUS                  Status;
  EFI_HII_PACKAGE_HEADER      PackageHeader;
  CHAR8                       *Language;
  UINT32                      LanguageSize;
  EFI_LIST_ENTRY              *Link;

  if (Private == NULL || PackageHdr == NULL || PackageList == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (Private->Signature != HII_DATABASE_PRIVATE_DATA_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  EfiCopyMem (&PackageHeader, PackageHdr, sizeof (EFI_HII_PACKAGE_HEADER));
  EfiCopyMem (&HeaderSize, (UINT8 *) PackageHdr + sizeof (EFI_HII_PACKAGE_HEADER), sizeof (UINT32));

  //
  // It is illegal to have two string packages with same language within one packagelist
  // since the stringid will be duplicate if so. Check it to avoid this potential issue.
  //
  LanguageSize = HeaderSize - sizeof (EFI_HII_STRING_PACKAGE_HDR) + sizeof (CHAR8);
  Language = (CHAR8 *) EfiLibAllocateZeroPool (LanguageSize);
  if (Language == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  EfiAsciiStrCpy (Language, (UINT8 *) PackageHdr + HeaderSize - LanguageSize);
  for (Link = PackageList->StringPkgHdr.ForwardLink; Link != &PackageList->StringPkgHdr; Link = Link->ForwardLink) {
    StringPackage = CR (Link, HII_STRING_PACKAGE_INSTANCE, StringEntry, HII_STRING_PACKAGE_SIGNATURE);
    if (EfiLibCompareLanguage (Language, StringPackage->StringPkgHdr->Language)) {
      EfiLibSafeFreePool (Language);
      return EFI_UNSUPPORTED;
    }    
  }  
  EfiLibSafeFreePool (Language);
  
  //
  // Create a String package node
  //
  StringPackage = (HII_STRING_PACKAGE_INSTANCE *) EfiLibAllocateZeroPool (sizeof (HII_STRING_PACKAGE_INSTANCE));
  if (StringPackage == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }  

  StringPackage->StringPkgHdr = (EFI_HII_STRING_PACKAGE_HDR *) EfiLibAllocateZeroPool (HeaderSize);
  if (StringPackage->StringPkgHdr == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  
  StringPackage->StringBlock = (UINT8 *) EfiLibAllocateZeroPool (PackageHeader.Length - HeaderSize);
  if (StringPackage->StringBlock == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  
  StringPackage->Signature = HII_STRING_PACKAGE_SIGNATURE;
  StringPackage->FontId    = 0;
  InitializeListHead (&StringPackage->FontInfoList);
  
  //
  // Copy the String package header.
  //
  EfiCopyMem (StringPackage->StringPkgHdr, PackageHdr, HeaderSize);

  //
  // Copy the String blocks
  //
  EfiCopyMem (
    StringPackage->StringBlock, 
    (UINT8 *) PackageHdr + HeaderSize,
    PackageHeader.Length - HeaderSize
    );

  //
  // Collect all font block info
  //
  Status = FindStringBlock (Private, StringPackage, (EFI_STRING_ID) (-1), NULL, NULL, NULL, &StringPackage->MaxStringId);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Insert to String package array
  //
  InsertTailList (&PackageList->StringPkgHdr, &StringPackage->StringEntry); 
  *Package = StringPackage;

  if (NotifyType == EFI_HII_DATABASE_NOTIFY_ADD_PACK) {
    PackageList->PackageListHdr.PackageLength += StringPackage->StringPkgHdr->Header.Length;
  }
  
  return EFI_SUCCESS;

Error:
  EfiLibSafeFreePool (StringPackage->StringBlock);
  EfiLibSafeFreePool (StringPackage->StringPkgHdr);
  EfiLibSafeFreePool (StringPackage);  
  
  return Status;  
}

EFI_STATUS
AdjustStringPackage (
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList
)
/*++

  Routine Description:
    Adjust all string packages in a single package list to have the same max string ID.

  Arguments:          
    PackageList       - Pointer to a package list which will be inserted to.
    
  Returns:
    EFI_SUCCESS       - Adjust all string packages successfully.
    others            - Can't adjust string packges.

--*/
{
  EFI_LIST_ENTRY              *Link;
  HII_STRING_PACKAGE_INSTANCE *StringPackage;
  UINT32                      Skip2BlockSize;
  UINT32                      OldBlockSize;
  UINT8                       *StringBlock;
  UINT8                       *BlockPtr;
  EFI_STRING_ID               MaxStringId;
  UINT16                      SkipCount;

  MaxStringId = 0;
  for (Link = PackageList->StringPkgHdr.ForwardLink;
       Link != &PackageList->StringPkgHdr;
       Link = Link->ForwardLink
      ) {
    StringPackage = CR (Link, HII_STRING_PACKAGE_INSTANCE, StringEntry, HII_STRING_PACKAGE_SIGNATURE);
    if (MaxStringId < StringPackage->MaxStringId) {
      MaxStringId = StringPackage->MaxStringId;
    }
  }

  for (Link = PackageList->StringPkgHdr.ForwardLink;
       Link != &PackageList->StringPkgHdr;
       Link = Link->ForwardLink
      ) {
    StringPackage = CR (Link, HII_STRING_PACKAGE_INSTANCE, StringEntry, HII_STRING_PACKAGE_SIGNATURE);
    if (StringPackage->MaxStringId < MaxStringId) {
      OldBlockSize = StringPackage->StringPkgHdr->Header.Length - StringPackage->StringPkgHdr->HdrSize;
      //
      // Create SKIP2 EFI_HII_SIBT_SKIP2_BLOCKs to reserve the missing string IDs.
      //
      SkipCount      = (UINT16) (MaxStringId - StringPackage->MaxStringId);
      Skip2BlockSize = (UINT32) sizeof (EFI_HII_SIBT_SKIP2_BLOCK);

      StringBlock = (UINT8 *) EfiLibAllocateZeroPool (OldBlockSize + Skip2BlockSize);
      if (StringBlock == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      //
      // Copy original string blocks, except the EFI_HII_SIBT_END.
      //
      EfiCopyMem (StringBlock, StringPackage->StringBlock, OldBlockSize - sizeof (EFI_HII_SIBT_END_BLOCK));
      //
      // Create SKIP2 EFI_HII_SIBT_SKIP2_BLOCK blocks
      //
      BlockPtr  = StringBlock + OldBlockSize - sizeof (EFI_HII_SIBT_END_BLOCK);
      *BlockPtr = EFI_HII_SIBT_SKIP2;
      EfiCopyMem (BlockPtr + 1, &SkipCount, sizeof (UINT16));
      BlockPtr  += sizeof (EFI_HII_SIBT_SKIP2_BLOCK);

      //
      // Append a EFI_HII_SIBT_END block to the end.
      //
      *BlockPtr = EFI_HII_SIBT_END;
      gBS->FreePool (StringPackage->StringBlock);
      StringPackage->StringBlock = StringBlock;
      StringPackage->StringPkgHdr->Header.Length += Skip2BlockSize;
      PackageList->PackageListHdr.PackageLength += Skip2BlockSize;
      StringPackage->MaxStringId = MaxStringId;
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ExportStringPackages (
  IN HII_DATABASE_PRIVATE_DATA          *Private,
  IN EFI_HII_HANDLE                     Handle,
  IN HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  IN UINTN                              UsedSize,
  IN UINTN                              BufferSize,
  IN OUT VOID                           *Buffer,
  IN OUT UINTN                          *ResultSize
  )
/*++

  Routine Description:
    This function exports String packages to a buffer.
    
  Arguments:          
    Private           - Hii database private structure.
    Handle            - Identification of a package list.
    PackageList       - Pointer to a package list which will be exported.
    UsedSize          - The length of buffer be used.    
    BufferSize        - Length of the Buffer.
    Buffer            - Allocated space for storing exported data.
    ResultSize        - The size of the already exported content of 
                        this package list.
    
  Returns:
    EFI_SUCCESS            - String Packages are exported successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is invalid.
     
--*/
{
  EFI_LIST_ENTRY              *Link;
  UINTN                       PackageLength;
  EFI_STATUS                  Status;
  HII_STRING_PACKAGE_INSTANCE *StringPackage;
  
  if (Private == NULL || PackageList == NULL || ResultSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize > 0 && Buffer == NULL ) {
    return EFI_INVALID_PARAMETER;
  }
  
  PackageLength = 0;
  Status        = EFI_SUCCESS;
  
  for (Link = PackageList->StringPkgHdr.ForwardLink; Link != &PackageList->StringPkgHdr; Link = Link->ForwardLink) {
    StringPackage = CR (Link, HII_STRING_PACKAGE_INSTANCE, StringEntry, HII_STRING_PACKAGE_SIGNATURE);
    PackageLength += StringPackage->StringPkgHdr->Header.Length;
    if (PackageLength + *ResultSize + UsedSize <= BufferSize) {
      //
      // Invoke registered notification function with EXPORT_PACK notify type
      //
      Status = InvokeRegisteredFunction (
                 Private, 
                 EFI_HII_DATABASE_NOTIFY_EXPORT_PACK,
                 (VOID *) StringPackage,
                 EFI_HII_PACKAGE_STRINGS,
                 Handle
                 );
      ASSERT_EFI_ERROR (Status);
      //
      // Copy String package header
      //      
      EfiCopyMem (Buffer, StringPackage->StringPkgHdr, StringPackage->StringPkgHdr->HdrSize);
      Buffer = (UINT8 *) Buffer + StringPackage->StringPkgHdr->HdrSize;

      //
      // Copy String blocks information
      //
      EfiCopyMem (
        Buffer,
        StringPackage->StringBlock, 
        StringPackage->StringPkgHdr->Header.Length - StringPackage->StringPkgHdr->HdrSize        
        );
      Buffer = (UINT8 *) Buffer + StringPackage->StringPkgHdr->Header.Length - StringPackage->StringPkgHdr->HdrSize;
    }    
  }
  
  *ResultSize += PackageLength;
  return EFI_SUCCESS; 
}

STATIC
EFI_STATUS
RemoveStringPackages (
  IN     HII_DATABASE_PRIVATE_DATA          *Private, 
  IN     EFI_HII_HANDLE                     Handle,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList
  )
/*++

  Routine Description:
    This function deletes all String packages from a package list node.
    
  Arguments:          
    Private           - Hii database private data.
    Handle            - Handle of the package list which contains the to be 
                        removed String packages.
    PackageList       - Pointer to a package list that contains removing packages.
    
  Returns:
    EFI_SUCCESS            - String Package(s) is deleted successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is not valid.
     
--*/   
{
  EFI_LIST_ENTRY                  *ListHead;
  HII_STRING_PACKAGE_INSTANCE     *Package;
  HII_FONT_INFO                   *FontInfo;
  EFI_STATUS                      Status;

  ListHead = &PackageList->StringPkgHdr;
  
  while (!IsListEmpty (ListHead)) {
    Package = CR (
                ListHead->ForwardLink, 
                HII_STRING_PACKAGE_INSTANCE, 
                StringEntry, 
                HII_STRING_PACKAGE_SIGNATURE
                );
    Status = InvokeRegisteredFunction (
               Private,
               EFI_HII_DATABASE_NOTIFY_REMOVE_PACK,
               (VOID *) Package,
               EFI_HII_PACKAGE_STRINGS,
               Handle
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    RemoveEntryList (&Package->StringEntry);
    PackageList->PackageListHdr.PackageLength -= Package->StringPkgHdr->Header.Length;
    EfiLibSafeFreePool (Package->StringBlock);
    EfiLibSafeFreePool (Package->StringPkgHdr);
    //
    // Delete font information
    //
    while (!IsListEmpty (&Package->FontInfoList)) {
      FontInfo = CR (
                   Package->FontInfoList.ForwardLink,
                   HII_FONT_INFO,
                   Entry,
                   HII_FONT_INFO_SIGNATURE
                   );
      RemoveEntryList (&FontInfo->Entry);
      EfiLibSafeFreePool (FontInfo);
    }    
    
    EfiLibSafeFreePool (Package);    
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
InsertFontPackage (
  IN     HII_DATABASE_PRIVATE_DATA          *Private,
  IN     VOID                               *PackageHdr,
  IN     EFI_HII_DATABASE_NOTIFY_TYPE       NotifyType,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  OUT    HII_FONT_PACKAGE_INSTANCE          **Package
  )
/*++

  Routine Description:
    This function insert a Font package to a package list node.
    
  Arguments:      
    Private           - Hii database private structure.  
    PackageHdr        - Pointer to a buffer stored with Font package information.
    NotifyType        - The type of change concerning the database.
    PackageList       - Pointer to a package list which will be inserted to.
    Package           - Created Font package
    
  Returns:
    EFI_SUCCESS            - Font Package is inserted successfully.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new Font package.
    EFI_INVALID_PARAMETER  - PackageHdr is NULL or PackageList is NULL.
    EFI_UNSUPPORTED        - A font package with same EFI_FONT_INFO already exists
                             in current hii database.
     
--*/
{
  HII_FONT_PACKAGE_INSTANCE *FontPackage;
  EFI_HII_FONT_PACKAGE_HDR  *FontPkgHdr;
  UINT32                    HeaderSize;
  EFI_STATUS                Status;
  EFI_HII_PACKAGE_HEADER    PackageHeader;
  EFI_FONT_INFO             *FontInfo;
  UINT32                    FontInfoSize; 
  HII_GLOBAL_FONT_INFO      *GlobalFont;

  if (Private == NULL || PackageHdr == NULL || PackageList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiCopyMem (&PackageHeader, PackageHdr, sizeof (EFI_HII_PACKAGE_HEADER));
  EfiCopyMem (&HeaderSize, (UINT8 *) PackageHdr + sizeof (EFI_HII_PACKAGE_HEADER), sizeof (UINT32));

  FontInfo    = NULL;
  FontPackage = NULL;
  GlobalFont  = NULL;

  //
  // It is illegal to have two font packages with same EFI_FONT_INFO within hii
  // database. EFI_FONT_INFO (FontName, FontSize, FontStyle) describes font's 
  // attributes and identify a font uniquely.
  //
  FontPkgHdr = (EFI_HII_FONT_PACKAGE_HDR *) EfiLibAllocateZeroPool (HeaderSize);
  if (FontPkgHdr == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  EfiCopyMem (FontPkgHdr, PackageHdr, HeaderSize);  
  
  FontInfoSize = sizeof (EFI_FONT_INFO) + HeaderSize - sizeof (EFI_HII_FONT_PACKAGE_HDR);
  FontInfo = (EFI_FONT_INFO *) EfiLibAllocateZeroPool (FontInfoSize);
  if (FontInfo == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  FontInfo->FontStyle = FontPkgHdr->FontStyle;
  FontInfo->FontSize  = FontPkgHdr->Cell.Height;
  EfiStrCpy (FontInfo->FontName, FontPkgHdr->FontFamily);

  if (IsFontInfoExisted (Private, FontInfo, NULL, NULL, NULL)) {
    Status = EFI_UNSUPPORTED;
    goto Error;
  }
  
  //
  // Create a Font package node
  //
  FontPackage = (HII_FONT_PACKAGE_INSTANCE *) EfiLibAllocateZeroPool (sizeof (HII_FONT_PACKAGE_INSTANCE));
  if (FontPackage == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  FontPackage->Signature  = HII_FONT_PACKAGE_SIGNATURE;
  FontPackage->FontPkgHdr = FontPkgHdr;
  InitializeListHead (&FontPackage->GlyphInfoList);
  
  FontPackage->GlyphBlock = (UINT8 *) EfiLibAllocateZeroPool (PackageHeader.Length - HeaderSize);
  if (FontPackage->GlyphBlock == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }  
  EfiCopyMem (FontPackage->GlyphBlock, (UINT8 *) PackageHdr + HeaderSize, PackageHeader.Length - HeaderSize);

  //
  // Collect all default character cell information and backup in GlyphInfoList.
  //
  Status = FindGlyphBlock (FontPackage, (CHAR16) (-1), NULL, NULL, NULL);
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // This font package describes an unique EFI_FONT_INFO. Backup it in global 
  // font info list.
  //
  GlobalFont = (HII_GLOBAL_FONT_INFO *) EfiLibAllocateZeroPool (sizeof (HII_GLOBAL_FONT_INFO));
  if (GlobalFont == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  GlobalFont->Signature    = HII_GLOBAL_FONT_INFO_SIGNATURE;
  GlobalFont->FontPackage  = FontPackage;
  GlobalFont->FontInfoSize = FontInfoSize;
  GlobalFont->FontInfo     = FontInfo;
  InsertTailList (&Private->FontInfoList, &GlobalFont->Entry);

  //
  // Insert this font package to Font package array
  //
  InsertTailList (&PackageList->FontPkgHdr, &FontPackage->FontEntry);
  *Package = FontPackage;

  if (NotifyType == EFI_HII_DATABASE_NOTIFY_ADD_PACK) {
    PackageList->PackageListHdr.PackageLength += FontPackage->FontPkgHdr->Header.Length;
  }

  return EFI_SUCCESS;

Error:
  EfiLibSafeFreePool (FontPkgHdr);
  EfiLibSafeFreePool (FontInfo); 
  EfiLibSafeFreePool (FontPackage->GlyphBlock);
  EfiLibSafeFreePool (FontPackage);
  EfiLibSafeFreePool (GlobalFont);   
  
  return Status;  
}

STATIC
EFI_STATUS
ExportFontPackages (
  IN HII_DATABASE_PRIVATE_DATA          *Private,
  IN EFI_HII_HANDLE                     Handle,
  IN HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  IN UINTN                              UsedSize,
  IN UINTN                              BufferSize,
  IN OUT VOID                           *Buffer,
  IN OUT UINTN                          *ResultSize
  )
/*++

  Routine Description:
    This function exports Font packages to a buffer.
    
  Arguments:          
    Private           - Hii database private structure.
    Handle            - Identification of a package list.
    PackageList       - Pointer to a package list which will be exported.
    UsedSize          - The length of buffer be used.        
    BufferSize        - Length of the Buffer.
    Buffer            - Allocated space for storing exported data.
    ResultSize        - The size of the already exported content of 
                        this package list.
    
  Returns:
    EFI_SUCCESS            - Font Packages are exported successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is invalid.
     
--*/
{
  EFI_LIST_ENTRY              *Link;
  UINTN                       PackageLength;
  EFI_STATUS                  Status;
  HII_FONT_PACKAGE_INSTANCE   *Package;
  

  if (Private == NULL || PackageList == NULL || ResultSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize > 0 && Buffer == NULL ) {
    return EFI_INVALID_PARAMETER;
  }
  
  PackageLength = 0;
  Status        = EFI_SUCCESS;
  
  for (Link = PackageList->FontPkgHdr.ForwardLink; Link != &PackageList->FontPkgHdr; Link = Link->ForwardLink) {
    Package = CR (Link, HII_FONT_PACKAGE_INSTANCE, FontEntry, HII_FONT_PACKAGE_SIGNATURE);
    PackageLength += Package->FontPkgHdr->Header.Length;
    if (PackageLength + *ResultSize + UsedSize <= BufferSize) {
      //
      // Invoke registered notification function with EXPORT_PACK notify type
      //
      Status = InvokeRegisteredFunction (
                 Private, 
                 EFI_HII_DATABASE_NOTIFY_EXPORT_PACK,
                 (VOID *) Package,
                 EFI_HII_PACKAGE_FONTS,
                 Handle
                 );
      ASSERT_EFI_ERROR (Status);
      //
      // Copy Font package header
      //      
      EfiCopyMem (Buffer, Package->FontPkgHdr, Package->FontPkgHdr->HdrSize);
      Buffer = (UINT8 *) Buffer + Package->FontPkgHdr->HdrSize;

      //
      // Copy Glyph blocks information
      //
      EfiCopyMem (
        Buffer,
        Package->GlyphBlock, 
        Package->FontPkgHdr->Header.Length - Package->FontPkgHdr->HdrSize 
        );
      Buffer = (UINT8 *) Buffer + Package->FontPkgHdr->Header.Length - Package->FontPkgHdr->HdrSize;
    }    
  }
  
  *ResultSize += PackageLength;
  return EFI_SUCCESS; 
}

STATIC
EFI_STATUS
RemoveFontPackages (
  IN     HII_DATABASE_PRIVATE_DATA          *Private, 
  IN     EFI_HII_HANDLE                     Handle,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList
  )
/*++

  Routine Description:
    This function deletes all Font packages from a package list node.
    
  Arguments:          
    Private           - Hii database private data.
    Handle            - Handle of the package list which contains the to be 
                        removed Font packages.
    PackageList       - Pointer to a package list that contains removing packages.
    
  Returns:
    EFI_SUCCESS            - Font Package(s) is deleted successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is not valid.
     
--*/   
{
  EFI_LIST_ENTRY                  *ListHead;
  HII_FONT_PACKAGE_INSTANCE       *Package;
  EFI_STATUS                      Status;
  HII_GLYPH_INFO                  *GlyphInfo;
  EFI_LIST_ENTRY                  *Link;
  HII_GLOBAL_FONT_INFO            *GlobalFont;

  ListHead = &PackageList->FontPkgHdr;
  
  while (!IsListEmpty (ListHead)) {
    Package = CR (
                ListHead->ForwardLink, 
                HII_FONT_PACKAGE_INSTANCE, 
                FontEntry, 
                HII_FONT_PACKAGE_SIGNATURE
                );
    Status = InvokeRegisteredFunction (
               Private,
               EFI_HII_DATABASE_NOTIFY_REMOVE_PACK,
               (VOID *) Package,
               EFI_HII_PACKAGE_FONTS,
               Handle
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    RemoveEntryList (&Package->FontEntry);
    PackageList->PackageListHdr.PackageLength -= Package->FontPkgHdr->Header.Length;
    EfiLibSafeFreePool (Package->GlyphBlock);
    EfiLibSafeFreePool (Package->FontPkgHdr);
    //
    // Delete default character cell information
    //
    while (!IsListEmpty (&Package->GlyphInfoList)) {
      GlyphInfo = CR (
                    Package->GlyphInfoList.ForwardLink,
                    HII_GLYPH_INFO,
                    Entry,
                    HII_GLYPH_INFO_SIGNATURE
                    );
      RemoveEntryList (&GlyphInfo->Entry);
      EfiLibSafeFreePool (GlyphInfo);
    }    

    //
    // Remove corresponding global font info
    //
    for (Link = Private->FontInfoList.ForwardLink; Link != &Private->FontInfoList; Link = Link->ForwardLink) {
      GlobalFont = CR (Link, HII_GLOBAL_FONT_INFO, Entry, HII_GLOBAL_FONT_INFO_SIGNATURE);
      if (GlobalFont->FontPackage == Package) {
        RemoveEntryList (&GlobalFont->Entry);
        EfiLibSafeFreePool (GlobalFont->FontInfo);
        EfiLibSafeFreePool (GlobalFont);
        break;
      }
    }
       
    EfiLibSafeFreePool (Package);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
InsertImagePackage (
  IN     VOID                               *PackageHdr,
  IN     EFI_HII_DATABASE_NOTIFY_TYPE       NotifyType,    
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  OUT    HII_IMAGE_PACKAGE_INSTANCE         **Package
  )
/*++

  Routine Description:
    This function insert a Image package to a package list node.
    
  Arguments:          
    PackageHdr        - Pointer to a buffer stored with Image package information.
    NotifyType        - The type of change concerning the database.
    PackageList       - Pointer to a package list which will be inserted to.
    Package           - Created Image package
    
  Returns:
    EFI_SUCCESS            - Image Package is inserted successfully.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new Image package.
    EFI_INVALID_PARAMETER  - PackageHdr is NULL or PackageList is NULL.
     
--*/
{
  HII_IMAGE_PACKAGE_INSTANCE        *ImagePackage;
  UINT32                            PaletteSize;
  UINT32                            ImageSize;
  UINT16                            Index;
  EFI_HII_IMAGE_PALETTE_INFO_HEADER *PaletteHdr;
  EFI_HII_IMAGE_PALETTE_INFO        *PaletteInfo;
  UINT32                            PaletteInfoOffset;
  UINT32                            ImageInfoOffset;
  UINT16                            CurrentSize;

  if (PackageHdr == NULL || PackageList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Less than one image package is allowed in one package list.
  //
  if (PackageList->ImagePkg != NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Create a Image package node
  //
  ImagePackage = (HII_IMAGE_PACKAGE_INSTANCE *) EfiLibAllocateZeroPool (sizeof (HII_IMAGE_PACKAGE_INSTANCE));
  if (ImagePackage == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Copy the Image package header.
  //
  EfiCopyMem (&ImagePackage->ImagePkgHdr, PackageHdr, sizeof (EFI_HII_IMAGE_PACKAGE_HDR));

  PaletteInfoOffset = ImagePackage->ImagePkgHdr.PaletteInfoOffset; 
  ImageInfoOffset   = ImagePackage->ImagePkgHdr.ImageInfoOffset; 
  
  //
  // If PaletteInfoOffset is zero, there are no palettes in this image package.
  //
  PaletteSize                = 0;
  ImagePackage->PaletteBlock = NULL;
  if (PaletteInfoOffset != 0) {
    PaletteHdr  = (EFI_HII_IMAGE_PALETTE_INFO_HEADER *) ((UINT8 *) PackageHdr + PaletteInfoOffset);
    PaletteSize = sizeof (EFI_HII_IMAGE_PALETTE_INFO_HEADER);
    PaletteInfo = (EFI_HII_IMAGE_PALETTE_INFO *) ((UINT8 *) PaletteHdr + PaletteSize);

    for (Index = 0; Index < PaletteHdr->PaletteCount; Index++) {    
      EfiCopyMem (&CurrentSize, PaletteInfo, sizeof (UINT16));
      CurrentSize += sizeof (UINT16);
      PaletteSize += (UINT32) CurrentSize;
      PaletteInfo = (EFI_HII_IMAGE_PALETTE_INFO *) ((UINT8 *) PaletteInfo + CurrentSize);
    }

    ImagePackage->PaletteBlock = (UINT8 *) EfiLibAllocateZeroPool (PaletteSize);
    if (ImagePackage->PaletteBlock == NULL) {
      EfiLibSafeFreePool (ImagePackage);
      return EFI_OUT_OF_RESOURCES;
    }
    EfiCopyMem (
      ImagePackage->PaletteBlock,
      (UINT8 *) PackageHdr + PaletteInfoOffset,
      PaletteSize
      );
  }

  //
  // If ImageInfoOffset is zero, there are no images in this package.
  //
  ImageSize                = 0;
  ImagePackage->ImageBlock = NULL;  
  if (ImageInfoOffset != 0) {
    ImageSize = ImagePackage->ImagePkgHdr.Header.Length - 
                sizeof (EFI_HII_IMAGE_PACKAGE_HDR) - PaletteSize;
    ImagePackage->ImageBlock = (UINT8 *) EfiLibAllocateZeroPool (ImageSize);
    if (ImagePackage->ImageBlock == NULL) {
      EfiLibSafeFreePool (ImagePackage->PaletteBlock);
      EfiLibSafeFreePool (ImagePackage);
      return EFI_OUT_OF_RESOURCES;
    }
    EfiCopyMem (
      ImagePackage->ImageBlock, 
      (UINT8 *) PackageHdr + ImageInfoOffset,
      ImageSize
      );
  }
    
  ImagePackage->ImageBlockSize  = ImageSize;
  ImagePackage->PaletteInfoSize = PaletteSize;
  PackageList->ImagePkg         = ImagePackage;
  *Package                      = ImagePackage;

  if (NotifyType == EFI_HII_DATABASE_NOTIFY_ADD_PACK) {
    PackageList->PackageListHdr.PackageLength += ImagePackage->ImagePkgHdr.Header.Length;
  }
  
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ExportImagePackages (
  IN HII_DATABASE_PRIVATE_DATA          *Private,
  IN EFI_HII_HANDLE                     Handle,
  IN HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  IN UINTN                              UsedSize,
  IN UINTN                              BufferSize,
  IN OUT VOID                           *Buffer,
  IN OUT UINTN                          *ResultSize
  )
/*++

  Routine Description:
    This function exports Image packages to a buffer.
    
  Arguments:          
    Private           - Hii database private structure.
    Handle            - Identification of a package list.
    PackageList       - Pointer to a package list which will be exported.
    UsedSize          - The length of buffer be used.
    BufferSize        - Length of the Buffer.
    Buffer            - Allocated space for storing exported data.
    ResultSize        - The size of the already exported content of 
                        this package list.
    
  Returns:
    EFI_SUCCESS            - Image Packages are exported successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is invalid.
     
--*/
{
  UINTN                       PackageLength;
  EFI_STATUS                  Status;
  HII_IMAGE_PACKAGE_INSTANCE  *Package;  
  

  if (Private == NULL || PackageList == NULL || ResultSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize > 0 && Buffer == NULL ) {
    return EFI_INVALID_PARAMETER;
  }
  
  Package = PackageList->ImagePkg;

  if (Package == NULL) {
    return EFI_SUCCESS;
  }
  
  PackageLength = Package->ImagePkgHdr.Header.Length;

  if (PackageLength + *ResultSize + UsedSize <= BufferSize) {
    //
    // Invoke registered notification function with EXPORT_PACK notify type
    //
    Status = InvokeRegisteredFunction (
               Private, 
               EFI_HII_DATABASE_NOTIFY_EXPORT_PACK,
               (VOID *) Package,
               EFI_HII_PACKAGE_IMAGES,
               Handle
               );
    ASSERT_EFI_ERROR (Status);     
    ASSERT (Package->ImagePkgHdr.Header.Length == 
            sizeof (EFI_HII_IMAGE_PACKAGE_HDR) + Package->ImageBlockSize + Package->PaletteInfoSize);      
    //
    // Copy Image package header, 
    // then justify the offset for image info and palette info in the header.
    //      
    EfiCopyMem (Buffer, &Package->ImagePkgHdr, sizeof (EFI_HII_IMAGE_PACKAGE_HDR));
    Buffer = (UINT8 *) Buffer + sizeof (EFI_HII_IMAGE_PACKAGE_HDR);

    //
    // Copy Image blocks information
    //
    if (Package->ImageBlockSize != 0) {
      EfiCopyMem (Buffer, Package->ImageBlock, Package->ImageBlockSize);
      Buffer = (UINT8 *) Buffer + Package->ImageBlockSize;
    }
    //
    // Copy Palette information
    //
    if (Package->PaletteInfoSize != 0) {
      EfiCopyMem (Buffer, Package->PaletteBlock, Package->PaletteInfoSize);
      Buffer = (UINT8 *) Buffer + Package->PaletteInfoSize;
    }
  }    
    
  *ResultSize += PackageLength;
  return EFI_SUCCESS; 
}

STATIC
EFI_STATUS
RemoveImagePackages (
  IN     HII_DATABASE_PRIVATE_DATA          *Private,
  IN     EFI_HII_HANDLE                     Handle,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList
  )
/*++

  Routine Description:
    This function deletes Image package from a package list node.
    
  Arguments:          
    Private           - Hii database private data.
    Handle            - Handle of the package list which contains the to be 
                        removed Image packages.
    PackageList       - Package List which contains the to be 
                        removed Image package.
    
  Returns:
    EFI_SUCCESS            - Image Package(s) is deleted successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is not valid.
     
--*/   
{
  HII_IMAGE_PACKAGE_INSTANCE      *Package;
  EFI_STATUS                      Status;
  
  Package = PackageList->ImagePkg;

  //
  // Image package does not exist, return directly.
  //
  if (Package == NULL) {
    return EFI_SUCCESS;
  }
  
  Status = InvokeRegisteredFunction (
             Private,
             EFI_HII_DATABASE_NOTIFY_REMOVE_PACK,
             (VOID *) Package,
             EFI_HII_PACKAGE_IMAGES,
             Handle
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  PackageList->PackageListHdr.PackageLength -= Package->ImagePkgHdr.Header.Length;
  
  EfiLibSafeFreePool (Package->ImageBlock);
  EfiLibSafeFreePool (Package->PaletteBlock);
  EfiLibSafeFreePool (Package);
  
  PackageList->ImagePkg = NULL;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
InsertSimpleFontPackage (
  IN     VOID                               *PackageHdr,
  IN     EFI_HII_DATABASE_NOTIFY_TYPE       NotifyType,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  OUT HII_SIMPLE_FONT_PACKAGE_INSTANCE      **Package
  )
/*++

  Routine Description:
    This function insert a Simple Font package to a package list node.
    
  Arguments:          
    PackageHdr        - Pointer to a buffer stored with Simple Font package information.
    NotifyType        - The type of change concerning the database.
    PackageList       - Pointer to a package list which will be inserted to.
    Package           - Created Simple Font package
    
  Returns:
    EFI_SUCCESS            - Simple Font Package is inserted successfully.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new Simple Font package.
    EFI_INVALID_PARAMETER  - PackageHdr is NULL or PackageList is NULL.
     
--*/
{
  HII_SIMPLE_FONT_PACKAGE_INSTANCE *SimpleFontPackage;
  EFI_STATUS                       Status;
  EFI_HII_PACKAGE_HEADER           Header;

  if (PackageHdr == NULL || PackageList == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Create a Simple Font package node
  //
  SimpleFontPackage = EfiLibAllocateZeroPool (sizeof (HII_SIMPLE_FONT_PACKAGE_INSTANCE));
  if (SimpleFontPackage == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  SimpleFontPackage->Signature = HII_S_FONT_PACKAGE_SIGNATURE;

  //
  // Copy the Simple Font package.
  //  
  EfiCopyMem (&Header, PackageHdr, sizeof (EFI_HII_PACKAGE_HEADER));
  
  SimpleFontPackage->SimpleFontPkgHdr = EfiLibAllocateZeroPool (Header.Length);
  if (SimpleFontPackage->SimpleFontPkgHdr == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }

  EfiCopyMem (SimpleFontPackage->SimpleFontPkgHdr, PackageHdr, Header.Length);
    
  //
  // Insert to Simple Font package array
  //
  InsertTailList (&PackageList->SimpleFontPkgHdr, &SimpleFontPackage->SimpleFontEntry);  
  *Package = SimpleFontPackage;

  if (NotifyType == EFI_HII_DATABASE_NOTIFY_ADD_PACK) {
    PackageList->PackageListHdr.PackageLength += Header.Length;
  }
  
  return EFI_SUCCESS;

Error:
  
  EfiLibSafeFreePool (SimpleFontPackage->SimpleFontPkgHdr);
  EfiLibSafeFreePool (SimpleFontPackage);
  return Status;
}

STATIC
EFI_STATUS
ExportSimpleFontPackages (
  IN HII_DATABASE_PRIVATE_DATA          *Private,
  IN EFI_HII_HANDLE                     Handle,
  IN HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  IN UINTN                              UsedSize,
  IN UINTN                              BufferSize,
  IN OUT VOID                           *Buffer,
  IN OUT UINTN                          *ResultSize
  )
/*++

  Routine Description:
    This function exports SimpleFont packages to a buffer.
    
  Arguments:          
    Private           - Hii database private structure.
    Handle            - Identification of a package list.
    PackageList       - Pointer to a package list which will be exported.
    UsedSize          - The length of buffer be used.
    BufferSize        - Length of the Buffer.
    Buffer            - Allocated space for storing exported data.
    ResultSize        - The size of the already exported content of 
                        this package list.
    
  Returns:
    EFI_SUCCESS            - SimpleFont Packages are exported successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is invalid.
     
--*/
{
  EFI_LIST_ENTRY                    *Link;
  UINTN                             PackageLength;
  EFI_STATUS                        Status;
  HII_SIMPLE_FONT_PACKAGE_INSTANCE  *Package;  
  
  if (Private == NULL || PackageList == NULL || ResultSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize > 0 && Buffer == NULL ) {
    return EFI_INVALID_PARAMETER;
  }
  
  PackageLength = 0;
  Status        = EFI_SUCCESS;
  
  for (Link = PackageList->SimpleFontPkgHdr.ForwardLink; Link != &PackageList->SimpleFontPkgHdr; Link = Link->ForwardLink) {
    Package = CR (Link, HII_SIMPLE_FONT_PACKAGE_INSTANCE, SimpleFontEntry, HII_S_FONT_PACKAGE_SIGNATURE);
    PackageLength += Package->SimpleFontPkgHdr->Header.Length;
    if (PackageLength + *ResultSize + UsedSize <= BufferSize) {
      //
      // Invoke registered notification function with EXPORT_PACK notify type
      //
      Status = InvokeRegisteredFunction (
                 Private, 
                 EFI_HII_DATABASE_NOTIFY_EXPORT_PACK,
                 (VOID *) Package,
                 EFI_HII_PACKAGE_SIMPLE_FONTS,
                 Handle
                 );
      ASSERT_EFI_ERROR (Status);
      
      //
      // Copy SimpleFont package
      //      
      EfiCopyMem (Buffer, Package->SimpleFontPkgHdr, Package->SimpleFontPkgHdr->Header.Length);
      Buffer = (UINT8 *) Buffer + Package->SimpleFontPkgHdr->Header.Length;
    }    
  }
  
  *ResultSize += PackageLength;
  return EFI_SUCCESS; 
}

STATIC
EFI_STATUS
RemoveSimpleFontPackages (
  IN     HII_DATABASE_PRIVATE_DATA          *Private, 
  IN     EFI_HII_HANDLE                     Handle,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList
  )
/*++

  Routine Description:
    This function deletes all Simple Font packages from a package list node.
    
  Arguments:          
    Private           - Hii database private data.
    Handle            - Handle of the package list which contains the to be 
                        removed Simple Font packages.
    PackageList       - Pointer to a package list that contains removing packages.
    
  Returns:
    EFI_SUCCESS            - Simple Font Package(s) is deleted successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is not valid.
     
--*/   
{
  EFI_LIST_ENTRY                   *ListHead;
  HII_SIMPLE_FONT_PACKAGE_INSTANCE *Package;
  EFI_STATUS                       Status;

  ListHead = &PackageList->SimpleFontPkgHdr;
  
  while (!IsListEmpty (ListHead)) {
    Package = CR (
                ListHead->ForwardLink, 
                HII_SIMPLE_FONT_PACKAGE_INSTANCE, 
                SimpleFontEntry, 
                HII_S_FONT_PACKAGE_SIGNATURE
                );
    Status = InvokeRegisteredFunction (
               Private,
               EFI_HII_DATABASE_NOTIFY_REMOVE_PACK,
               (VOID *) Package,
               EFI_HII_PACKAGE_SIMPLE_FONTS,
               Handle
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    RemoveEntryList (&Package->SimpleFontEntry);
    PackageList->PackageListHdr.PackageLength -= Package->SimpleFontPkgHdr->Header.Length;
    EfiLibSafeFreePool (Package->SimpleFontPkgHdr);
    EfiLibSafeFreePool (Package);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
InsertDevicePathPackage (
  IN     EFI_DEVICE_PATH_PROTOCOL           *DevicePath,
  IN     EFI_HII_DATABASE_NOTIFY_TYPE       NotifyType,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList
  )
/*++

  Routine Description:
    This function insert a Device path package to a package list node.
    
  Arguments:          
    DevicePath        - Pointer to a EFI_DEVICE_PATH_PROTOCOL protocol instance
    NotifyType        - The type of change concerning the database.
    PackageList       - Pointer to a package list which will be inserted to.
    
  Returns:
    EFI_SUCCESS            - Device path Package is inserted successfully.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new Device path package.
    EFI_INVALID_PARAMETER  - DevicePath is NULL or PackageList is NULL.
     
--*/    
{
  UINT32                           PackageLength;
  EFI_HII_PACKAGE_HEADER           Header;
  
  if (DevicePath == NULL || PackageList == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Less than one device path package is allowed in one package list.
  //
  if (PackageList->DevicePathPkg != NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PackageLength = (UINT32) EfiDevicePathSize (DevicePath) + sizeof (EFI_HII_PACKAGE_HEADER);
  PackageList->DevicePathPkg = (UINT8 *) EfiLibAllocateZeroPool (PackageLength);
  if (PackageList->DevicePathPkg == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Header.Length = PackageLength;
  Header.Type   = EFI_HII_PACKAGE_DEVICE_PATH;
  EfiCopyMem (PackageList->DevicePathPkg, &Header, sizeof (EFI_HII_PACKAGE_HEADER));
  EfiCopyMem (
    PackageList->DevicePathPkg + sizeof (EFI_HII_PACKAGE_HEADER),
    DevicePath,
    PackageLength - sizeof (EFI_HII_PACKAGE_HEADER)
    );

  //
  // Since Device Path package is created by NewPackageList, either NEW_PACK
  // or ADD_PACK should increase the length of package list.
  //
  PackageList->PackageListHdr.PackageLength += PackageLength;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ExportDevicePathPackage (
  IN HII_DATABASE_PRIVATE_DATA          *Private,
  IN EFI_HII_HANDLE                     Handle,
  IN HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  IN UINTN                              UsedSize,
  IN UINTN                              BufferSize,
  IN OUT VOID                           *Buffer,
  IN OUT UINTN                          *ResultSize
  )
/*++

  Routine Description:
    This function exports device path package to a buffer.
    
  Arguments:          
    Private           - Hii database private structure.
    Handle            - Identification of a package list.
    PackageList       - Pointer to a package list which will be exported.
    UsedSize          - The length of buffer be used.
    BufferSize        - Length of the Buffer.
    Buffer            - Allocated space for storing exported data.
    ResultSize        - The size of the already exported content of 
                        this package list.
    
  Returns:
    EFI_SUCCESS            - Device path Package is exported successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is invalid.
     
--*/  
{
  EFI_STATUS                       Status;
  UINT8                            *Package;
  EFI_HII_PACKAGE_HEADER           Header;  

  if (Private == NULL || PackageList == NULL || ResultSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (BufferSize > 0 && Buffer == NULL ) {
    return EFI_INVALID_PARAMETER;
  }
  
  Package = PackageList->DevicePathPkg;

  if (Package == NULL) {
    return EFI_SUCCESS;
  }
    
  EfiCopyMem (&Header, Package, sizeof (EFI_HII_PACKAGE_HEADER));

  if (Header.Length + *ResultSize + UsedSize <= BufferSize) {
    //
    // Invoke registered notification function with EXPORT_PACK notify type
    //
    Status = InvokeRegisteredFunction (
               Private, 
               EFI_HII_DATABASE_NOTIFY_EXPORT_PACK,
               (VOID *) Package,
               EFI_HII_PACKAGE_DEVICE_PATH,
               Handle
               );
    ASSERT_EFI_ERROR (Status);

    //
    // Copy Device path package
    //
    EfiCopyMem (Buffer, Package, Header.Length);
  }
    
  *ResultSize += Header.Length;
  return EFI_SUCCESS; 
}

STATIC
EFI_STATUS
RemoveDevicePathPackage (
  IN     HII_DATABASE_PRIVATE_DATA          *Private, 
  IN     EFI_HII_HANDLE                     Handle,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList
  )
/*++

  Routine Description:
    This function deletes Device Path package from a package list node.
    
  Arguments:          
    Private           - Hii database private data.
    Handle            - Handle of the package list.
    PackageList       - Package List which contains the to be 
                        removed Device Path package.    
    
  Returns:
    EFI_SUCCESS            - Device Path Package is deleted successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is not valid.
     
--*/   
{ 
  EFI_STATUS                       Status;
  UINT8                            *Package;
  EFI_HII_PACKAGE_HEADER           Header;

  Package = PackageList->DevicePathPkg;

  //
  // No device path, return directly.
  //
  if (Package == NULL) {
    return EFI_SUCCESS;
  }
  
  Status = InvokeRegisteredFunction (
             Private,
             EFI_HII_DATABASE_NOTIFY_REMOVE_PACK,
             (VOID *) Package,
             EFI_HII_PACKAGE_DEVICE_PATH,
             Handle
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  EfiCopyMem (&Header, Package, sizeof (EFI_HII_PACKAGE_HEADER));
  PackageList->PackageListHdr.PackageLength -= Header.Length;

  EfiLibSafeFreePool (Package);

  PackageList->DevicePathPkg = NULL;
 
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
AddDevicePathPackage (
  IN HII_DATABASE_PRIVATE_DATA        *Private,
  IN EFI_HII_DATABASE_NOTIFY_TYPE     NotifyType,
  IN EFI_DEVICE_PATH_PROTOCOL         *DevicePath,
  IN OUT HII_DATABASE_RECORD          *DatabaseRecord
  )
/*++

  Routine Description:
    This function will insert a device path package to package list firstly then
    invoke notification functions if any.
    
  Arguments:      
    Private           - Hii database private structure.
    NotifyType        - The type of change concerning the database.
    DevicePath        - Pointer to a EFI_DEVICE_PATH_PROTOCOL protocol instance    
    DatabaseRecord    - Pointer to a database record contains 
                        a package list which will be inserted to.
    
  Returns:
    EFI_SUCCESS            - Device path Package is inserted successfully.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new Device path package.
    EFI_INVALID_PARAMETER  - DevicePath is NULL or PackageList is NULL.
     
--*/    
{
  EFI_STATUS                          Status; 

  if (DevicePath == NULL) {
    return EFI_SUCCESS;
  }
  
  ASSERT (Private != NULL);
  ASSERT (DatabaseRecord != NULL);
  
  //
  // Create a device path package and insert to packagelist
  //
  Status = InsertDevicePathPackage (
               DevicePath,
               NotifyType,
               DatabaseRecord->PackageList
               );
  if (EFI_ERROR (Status)) {
    return Status;
  }      
  
  return InvokeRegisteredFunction (
            Private,
            NotifyType,
            (VOID *) DatabaseRecord->PackageList->DevicePathPkg,
            EFI_HII_PACKAGE_DEVICE_PATH,
            DatabaseRecord->Handle
            );
}

STATIC
EFI_STATUS
InsertKeyboardLayoutPackage (
  IN     VOID                               *PackageHdr,
  IN     EFI_HII_DATABASE_NOTIFY_TYPE       NotifyType,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  OUT HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE  **Package
  )
/*++

  Routine Description:
    This function insert a Keyboard Layout package to a package list node.
    
  Arguments:          
    PackageHdr        - Pointer to a buffer stored with Keyboard Layout package information.
    NotifyType        - The type of change concerning the database.
    PackageList       - Pointer to a package list which will be inserted to.
    Package           - Created Keyboard Layout package
    
  Returns:
    EFI_SUCCESS            - Keyboard Layout Package is inserted successfully.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new Keyboard Layout package.
    EFI_INVALID_PARAMETER  - PackageHdr is NULL or PackageList is NULL.
     
--*/  
{
  HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE *KeyboardLayoutPackage;
  EFI_HII_PACKAGE_HEADER               PackageHeader;
  EFI_STATUS                           Status;

  if (PackageHdr == NULL || PackageList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  EfiCopyMem (&PackageHeader, PackageHdr, sizeof (EFI_HII_PACKAGE_HEADER));
  
  //
  // Create a Keyboard Layout package node
  //
  KeyboardLayoutPackage = EfiLibAllocateZeroPool (sizeof (HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE));
  if (KeyboardLayoutPackage == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  KeyboardLayoutPackage->Signature = HII_KB_LAYOUT_PACKAGE_SIGNATURE;

  KeyboardLayoutPackage->KeyboardPkg = (UINT8 *) EfiLibAllocateZeroPool (PackageHeader.Length);
  if (KeyboardLayoutPackage->KeyboardPkg == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }

  EfiCopyMem (KeyboardLayoutPackage->KeyboardPkg, PackageHdr, PackageHeader.Length);
  InsertTailList (&PackageList->KeyboardLayoutHdr, &KeyboardLayoutPackage->KeyboardEntry);

  *Package = KeyboardLayoutPackage;

  if (NotifyType == EFI_HII_DATABASE_NOTIFY_ADD_PACK) {
    PackageList->PackageListHdr.PackageLength += PackageHeader.Length;
  }
  
  return EFI_SUCCESS;

Error:  
  EfiLibSafeFreePool (KeyboardLayoutPackage->KeyboardPkg);
  EfiLibSafeFreePool (KeyboardLayoutPackage);
  
  return Status;
}

STATIC
EFI_STATUS
ExportKeyboardLayoutPackages (
  IN HII_DATABASE_PRIVATE_DATA          *Private,
  IN EFI_HII_HANDLE                     Handle,
  IN HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  IN UINTN                              UsedSize,
  IN UINTN                              BufferSize,
  IN OUT VOID                           *Buffer,
  IN OUT UINTN                          *ResultSize
  )
/*++

  Routine Description:
    This function exports Keyboard Layout packages to a buffer.
    
  Arguments:          
    Private           - Hii database private structure.
    Handle            - Identification of a package list.
    PackageList       - Pointer to a package list which will be exported.
    UsedSize          - The length of buffer be used.
    BufferSize        - Length of the Buffer.
    Buffer            - Allocated space for storing exported data.
    ResultSize        - The size of the already exported content of 
                        this package list.
    
  Returns:
    EFI_SUCCESS            - Keyboard Layout Packages are exported successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is invalid.
     
--*/
{
  EFI_LIST_ENTRY                       *Link;
  UINTN                                PackageLength;
  EFI_STATUS                           Status;
  HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE *Package;  
  EFI_HII_PACKAGE_HEADER               PackageHeader;

  if (Private == NULL || PackageList == NULL || ResultSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize > 0 && Buffer == NULL ) {
    return EFI_INVALID_PARAMETER;
  }
  
  PackageLength = 0;
  Status        = EFI_SUCCESS;
  
  for (Link = PackageList->KeyboardLayoutHdr.ForwardLink; Link != &PackageList->KeyboardLayoutHdr; Link = Link->ForwardLink) {
    Package = CR (Link, HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE, KeyboardEntry, HII_KB_LAYOUT_PACKAGE_SIGNATURE);
    EfiCopyMem (&PackageHeader, Package->KeyboardPkg, sizeof (EFI_HII_PACKAGE_HEADER));
    PackageLength += PackageHeader.Length;
    if (PackageLength + *ResultSize + UsedSize <= BufferSize) {
      //
      // Invoke registered notification function with EXPORT_PACK notify type
      //
      Status = InvokeRegisteredFunction (
                 Private, 
                 EFI_HII_DATABASE_NOTIFY_EXPORT_PACK,
                 (EFI_HII_PACKAGE_HEADER *) Package,
                 EFI_HII_PACKAGE_KEYBOARD_LAYOUT,
                 Handle
                 );
      ASSERT_EFI_ERROR (Status);
                  
      //
      // Copy Keyboard Layout package 
      //      
      EfiCopyMem (Buffer, Package->KeyboardPkg, PackageHeader.Length);
      Buffer = (UINT8 *) Buffer + PackageHeader.Length;
    }    
  }
  
  *ResultSize += PackageLength;
  return EFI_SUCCESS; 
}

STATIC
EFI_STATUS
RemoveKeyboardLayoutPackages (
  IN     HII_DATABASE_PRIVATE_DATA          *Private,
  IN     EFI_HII_HANDLE                     Handle,
  IN OUT HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList
  )
/*++

  Routine Description:
    This function deletes all Keyboard Layout packages from a package list node.
    
  Arguments:          
    Private           - Hii database private data.
    Handle            - Handle of the package list which contains the to be 
                        removed Keyboard Layout packages.
    PackageList       - Pointer to a package list that contains removing packages.
    
  Returns:
    EFI_SUCCESS            - Keyboard Layout Package(s) is deleted successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is not valid.
     
--*/
{
  EFI_LIST_ENTRY                       *ListHead;
  HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE *Package;
  EFI_HII_PACKAGE_HEADER               PackageHeader;
  EFI_STATUS                           Status;
  
  ListHead = &PackageList->KeyboardLayoutHdr;
  
  while (!IsListEmpty (ListHead)) {
    Package = CR (
                ListHead->ForwardLink, 
                HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE, 
                KeyboardEntry, 
                HII_KB_LAYOUT_PACKAGE_SIGNATURE
                );
    Status = InvokeRegisteredFunction (
               Private,
               EFI_HII_DATABASE_NOTIFY_REMOVE_PACK,
               (VOID *) Package,
               EFI_HII_PACKAGE_KEYBOARD_LAYOUT,
               Handle
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    
    RemoveEntryList (&Package->KeyboardEntry);
    EfiCopyMem (&PackageHeader, Package->KeyboardPkg, sizeof (EFI_HII_PACKAGE_HEADER));
    PackageList->PackageListHdr.PackageLength -= PackageHeader.Length;
    EfiLibSafeFreePool (Package->KeyboardPkg);
    EfiLibSafeFreePool (Package);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
AddPackages (
  IN HII_DATABASE_PRIVATE_DATA         *Private,
  IN EFI_HII_DATABASE_NOTIFY_TYPE      NotifyType,
  IN CONST EFI_HII_PACKAGE_LIST_HEADER *PackageList,
  IN OUT   HII_DATABASE_RECORD         *DatabaseRecord
  )
/*++

  Routine Description:
    This function will insert a package list to hii database firstly then
    invoke notification functions if any. It is the worker function of 
    HiiNewPackageList and HiiUpdatePackageList.
    
  Arguments:      
    Private           - Hii database private structure.
    NotifyType        - The type of change concerning the database.
    PackageList       - Pointer to a package list.
    DatabaseRecord    - Pointer to a database record contains 
                        a package list instance which will be inserted to.
    
  Returns:
    EFI_SUCCESS            - All incoming packages are inserted to current database.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new Device path package.
    EFI_INVALID_PARAMETER  - Any input parameter is invalid.
     
--*/    
{
  EFI_STATUS                           Status;
  HII_GUID_PACKAGE_INSTANCE            *GuidPackage;
  HII_IFR_PACKAGE_INSTANCE             *FormPackage;
  HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE *KeyboardLayoutPackage;
  HII_STRING_PACKAGE_INSTANCE          *StringPackage;
  HII_FONT_PACKAGE_INSTANCE            *FontPackage;
  HII_SIMPLE_FONT_PACKAGE_INSTANCE     *SimpleFontPackage;
  HII_IMAGE_PACKAGE_INSTANCE           *ImagePackage;
  EFI_HII_PACKAGE_HEADER               *PackageHdrPtr;
  EFI_HII_PACKAGE_HEADER               PackageHeader;
  UINT32                               OldPackageListLen;
  BOOLEAN                              StringPkgIsAdd;

  //
  // Initialize Variables
  //
  StringPkgIsAdd = FALSE;
  FontPackage = NULL;
  
  //
  // Process the package list header
  //
  OldPackageListLen = DatabaseRecord->PackageList->PackageListHdr.PackageLength;
  EfiCopyMem (
    &DatabaseRecord->PackageList->PackageListHdr,
    (VOID *) PackageList,
    sizeof (EFI_HII_PACKAGE_LIST_HEADER)
    );
  if (NotifyType == EFI_HII_DATABASE_NOTIFY_ADD_PACK) {
    DatabaseRecord->PackageList->PackageListHdr.PackageLength = OldPackageListLen;
  }

  PackageHdrPtr = (EFI_HII_PACKAGE_HEADER *) ((UINT8 *) PackageList + sizeof (EFI_HII_PACKAGE_LIST_HEADER));
  EfiCopyMem (&PackageHeader, PackageHdrPtr, sizeof (EFI_HII_PACKAGE_HEADER));

  Status = EFI_SUCCESS;

  while (PackageHeader.Type != EFI_HII_PACKAGE_END) {
    switch (PackageHeader.Type) {
    case EFI_HII_PACKAGE_TYPE_GUID:
      Status = InsertGuidPackage (
                 PackageHdrPtr, 
                 NotifyType,
                 DatabaseRecord->PackageList,
                 &GuidPackage
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = InvokeRegisteredFunction (
                 Private, 
                 NotifyType, 
                 (VOID *) GuidPackage,
                 (UINT8) (PackageHeader.Type),
                 DatabaseRecord->Handle
                 );
      break;
    case EFI_HII_PACKAGE_FORMS:      
      Status = InsertFormPackage (
                 PackageHdrPtr, 
                 NotifyType,
                 DatabaseRecord->PackageList,
                 &FormPackage
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = InvokeRegisteredFunction (
                 Private, 
                 NotifyType, 
                 (VOID *) FormPackage,
                 (UINT8) (PackageHeader.Type),
                 DatabaseRecord->Handle
                 );
      break;      
    case EFI_HII_PACKAGE_KEYBOARD_LAYOUT:
      Status = InsertKeyboardLayoutPackage (
                 PackageHdrPtr,
                 NotifyType,
                 DatabaseRecord->PackageList,
                 &KeyboardLayoutPackage
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = InvokeRegisteredFunction (
                 Private, 
                 NotifyType, 
                 (VOID *) KeyboardLayoutPackage,
                 (UINT8) (PackageHeader.Type),
                 DatabaseRecord->Handle
                 );
      break;
    case EFI_HII_PACKAGE_STRINGS:
      Status = InsertStringPackage (
                 Private,
                 PackageHdrPtr, 
                 NotifyType,
                 DatabaseRecord->PackageList,
                 &StringPackage
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = InvokeRegisteredFunction (
                 Private, 
                 NotifyType, 
                 (VOID *) StringPackage,
                 (UINT8) (PackageHeader.Type),
                 DatabaseRecord->Handle
                 );      
      StringPkgIsAdd = TRUE;
      break;      
    case EFI_HII_PACKAGE_FONTS:
      Status = InsertFontPackage (
                 Private,
                 PackageHdrPtr,
                 NotifyType,
                 DatabaseRecord->PackageList,
                 &FontPackage
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = InvokeRegisteredFunction (
                 Private, 
                 NotifyType, 
                 (VOID *) FontPackage,
                 (UINT8) (PackageHeader.Type),
                 DatabaseRecord->Handle
                 );
      break;      
    case EFI_HII_PACKAGE_IMAGES:
      Status = InsertImagePackage (
                 PackageHdrPtr,
                 NotifyType,
                 DatabaseRecord->PackageList,
                 &ImagePackage
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = InvokeRegisteredFunction (
                 Private, 
                 NotifyType, 
                 (VOID *) ImagePackage,
                 (UINT8) (PackageHeader.Type),
                 DatabaseRecord->Handle
                 );
      break;      
    case EFI_HII_PACKAGE_SIMPLE_FONTS:
      Status = InsertSimpleFontPackage (
                 PackageHdrPtr,
                 NotifyType,
                 DatabaseRecord->PackageList,
                 &SimpleFontPackage
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = InvokeRegisteredFunction (
                 Private, 
                 NotifyType, 
                 (VOID *) SimpleFontPackage,
                 (UINT8) (PackageHeader.Type),
                 DatabaseRecord->Handle
                 );
      break;      
    case EFI_HII_PACKAGE_DEVICE_PATH:
      Status = AddDevicePathPackage (
                 Private, 
                 NotifyType,
                 (EFI_DEVICE_PATH_PROTOCOL *) ((UINT8 *) PackageHdrPtr + sizeof (EFI_HII_PACKAGE_HEADER)),
                 DatabaseRecord
                 );
      break;
    default:
      break;
    }

    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // goto header of next package 
    //    
    PackageHdrPtr = (EFI_HII_PACKAGE_HEADER *) ((UINT8 *) PackageHdrPtr + PackageHeader.Length);
    EfiCopyMem (&PackageHeader, PackageHdrPtr, sizeof (EFI_HII_PACKAGE_HEADER));
  }

  //
  // Adjust String Package to make sure all string packages have the same max string ID.
  //
  if (!EFI_ERROR (Status) && StringPkgIsAdd) {
    Status = AdjustStringPackage (DatabaseRecord->PackageList);
  }

  return Status;
}

STATIC
EFI_STATUS  
ExportPackageList (
  IN HII_DATABASE_PRIVATE_DATA          *Private,
  IN EFI_HII_HANDLE                     Handle,
  IN HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList,
  IN OUT UINTN                          *UsedSize,
  IN UINTN                              BufferSize,
  OUT EFI_HII_PACKAGE_LIST_HEADER       *Buffer    
  )
/*++

  Routine Description:
    This function exports a package list to a buffer. It is the worker function 
    of HiiExportPackageList.
    
  Arguments:          
    Private           - Hii database private structure.
    Handle            - Identification of a package list.
    PackageList       - Pointer to a package list which will be exported.
    UsedSize          - The length of buffer has been used by exporting 
                        package lists when Handle is NULL.
    BufferSize        - Length of the Buffer.
    Buffer            - Allocated space for storing exported data.
    
  Returns:
    EFI_SUCCESS            - Keyboard Layout Packages are exported successfully.
    EFI_INVALID_PARAMETER  - Any input parameter is invalid.
     
--*/  
  
{
  EFI_STATUS                          Status;
  UINTN                               ResultSize;
  EFI_HII_PACKAGE_HEADER              EndofPackageList;

  ASSERT (Private != NULL || PackageList != NULL || UsedSize != NULL);
  ASSERT (Private->Signature == HII_DATABASE_PRIVATE_DATA_SIGNATURE);
  ASSERT (IsHiiHandleValid (Handle));

  if (BufferSize > 0 && Buffer == NULL ) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Copy the package list header
  // ResultSize indicates the length of the exported bytes of this package list
  //
  ResultSize = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  if (ResultSize + *UsedSize <= BufferSize) {
    EfiCopyMem ((VOID *) Buffer, PackageList, ResultSize);
  }
  //
  // Copy the packages and invoke EXPORT_PACK notify functions if exists.
  //
  Status = ExportGuidPackages (
             Private,
             Handle,
             PackageList, 
             *UsedSize,
             BufferSize, 
             (VOID *) ((UINT8 *) Buffer + ResultSize), 
             &ResultSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  } 
  Status = ExportFormPackages (
             Private,
             Handle,
             PackageList,
             *UsedSize,
             BufferSize,
             (VOID *) ((UINT8 *) Buffer + ResultSize),
             &ResultSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  } 
  Status = ExportKeyboardLayoutPackages (
             Private,
             Handle,
             PackageList,
             *UsedSize,
             BufferSize,
             (VOID *) ((UINT8 *) Buffer + ResultSize),
             &ResultSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }  
  Status = ExportStringPackages (
             Private,
             Handle,
             PackageList,
             *UsedSize,
             BufferSize,
             (VOID *) ((UINT8 *) Buffer + ResultSize),
             &ResultSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  } 
  Status = ExportFontPackages (
             Private,
             Handle,
             PackageList,
             *UsedSize,
             BufferSize,
             (VOID *) ((UINT8 *) Buffer + ResultSize),
             &ResultSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  } 
  Status = ExportImagePackages (
             Private,
             Handle,
             PackageList,
             *UsedSize,
             BufferSize,
             (VOID *) ((UINT8 *) Buffer + ResultSize),
             &ResultSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = ExportSimpleFontPackages (
             Private,
             Handle,
             PackageList,
             *UsedSize,
             BufferSize,
             (VOID *) ((UINT8 *) Buffer + ResultSize),
             &ResultSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = ExportDevicePathPackage (
             Private,
             Handle,
             PackageList,
             *UsedSize,
             BufferSize,
             (VOID *) ((UINT8 *) Buffer + ResultSize),
             &ResultSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  } 
  //
  // Append the package list end.
  //
  EndofPackageList.Length = sizeof (EFI_HII_PACKAGE_HEADER);
  EndofPackageList.Type   = EFI_HII_PACKAGE_END;
  if (ResultSize + *UsedSize + sizeof (EFI_HII_PACKAGE_HEADER) <= BufferSize) {
    EfiCopyMem (
      (VOID *) ((UINT8 *) Buffer + ResultSize),
      (VOID *) &EndofPackageList, 
      sizeof (EFI_HII_PACKAGE_HEADER)
      );        
  }   

  *UsedSize += ResultSize + sizeof (EFI_HII_PACKAGE_HEADER);
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI 
HiiNewPackageList (
  IN CONST EFI_HII_DATABASE_PROTOCOL    *This,
  IN CONST EFI_HII_PACKAGE_LIST_HEADER  *PackageList,
  IN CONST EFI_HANDLE                   DriverHandle,
  OUT EFI_HII_HANDLE                    *Handle
  )
/*++

  Routine Description:
    This function adds the packages in the package list to the database and returns a handle. If there is a
    EFI_DEVICE_PATH_PROTOCOL associated with the DriverHandle, then this function will                     
    create a package of type EFI_PACKAGE_TYPE_DEVICE_PATH and add it to the package list.                      
    
  Arguments:          
    This              - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
    PackageList       - A pointer to an EFI_HII_PACKAGE_LIST_HEADER structure.
    DriverHandle      - Associate the package list with this EFI handle.    
    Handle            - A pointer to the EFI_HII_HANDLE instance.
    
  Returns:
    EFI_SUCCESS            - The package list associated with the Handle
                             was added to the HII database.
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new database contents.
    EFI_INVALID_PARAMETER  - PackageList is NULL or Handle is NULL.
    EFI_INVALID_PARAMETER  - PackageListGuid already exists in database.
     
--*/  
{
  EFI_STATUS                          Status;
  HII_DATABASE_PRIVATE_DATA           *Private;
  HII_DATABASE_RECORD                 *DatabaseRecord;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
  EFI_LIST_ENTRY                      *Link;
  EFI_GUID                            PackageListGuid;

  if (This == NULL || PackageList == NULL || Handle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS (This);
  EfiCopyMem (&PackageListGuid, (VOID *) PackageList, sizeof (EFI_GUID));

  //
  // Check the Package list GUID to guarantee this GUID is unique in database.
  //
  for (Link = Private->DatabaseList.ForwardLink; Link != &Private->DatabaseList; Link = Link->ForwardLink) {
    DatabaseRecord = CR (Link, HII_DATABASE_RECORD, DatabaseEntry, HII_DATABASE_RECORD_SIGNATURE);
    if (EfiCompareGuid (
          &(DatabaseRecord->PackageList->PackageListHdr.PackageListGuid),
          &PackageListGuid) &&
        DatabaseRecord->DriverHandle == DriverHandle) {
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Build a PackageList node
  //
  Status = GenerateHiiDatabaseRecord (Private, &DatabaseRecord);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Fill in information of the created Package List node
  // according to incoming package list.
  //
  Status = AddPackages (Private, EFI_HII_DATABASE_NOTIFY_NEW_PACK, PackageList, DatabaseRecord);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  DatabaseRecord->DriverHandle = DriverHandle;
  
  //
  // Create a Device path package and add into the package list if exists.
  //  
  Status = gBS->HandleProtocol (
                  DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &DevicePath
                  );
  if (!EFI_ERROR (Status)) {
    Status = AddDevicePathPackage (Private, EFI_HII_DATABASE_NOTIFY_NEW_PACK, DevicePath, DatabaseRecord);
    ASSERT_EFI_ERROR (Status);
  }   
  
  *Handle = DatabaseRecord->Handle;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI 
HiiRemovePackageList (
  IN CONST EFI_HII_DATABASE_PROTOCOL    *This,
  IN EFI_HII_HANDLE                     Handle
  )
/*++

  Routine Description:
    This function removes the package list that is associated with a handle Handle 
    from the HII database. Before removing the package, any registered functions 
    with the notification type REMOVE_PACK and the same package type will be called.
    
  Arguments:          
    This              - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
    Handle            - The handle that was registered to the data that is requested 
                        for removal.
    
  Returns:
    EFI_SUCCESS            - The data associated with the Handle was removed from 
                             the HII database.
    EFI_NOT_FOUND          - The specified Handle is not in database.
     
--*/
{
  EFI_STATUS                          Status;
  HII_DATABASE_PRIVATE_DATA           *Private;
  EFI_LIST_ENTRY                      *Link;
  HII_DATABASE_RECORD                 *Node;
  HII_DATABASE_PACKAGE_LIST_INSTANCE  *PackageList;
  HII_HANDLE                          *HiiHandle;
  
  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsHiiHandleValid (Handle)) {
    return EFI_NOT_FOUND;
  }

  Private = HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS (This);

  //
  // Get the packagelist to be removed.
  //
  for (Link = Private->DatabaseList.ForwardLink; Link != &Private->DatabaseList; Link = Link->ForwardLink) {
    Node = CR (Link, HII_DATABASE_RECORD, DatabaseEntry, HII_DATABASE_RECORD_SIGNATURE);
    if (Node->Handle == Handle) {
      PackageList = (HII_DATABASE_PACKAGE_LIST_INSTANCE *) (Node->PackageList);
      ASSERT (PackageList != NULL);
      
      //
      // Call registered functions with REMOVE_PACK before removing packages 
      // then remove them.
      //
      Status = RemoveGuidPackages (Private, Handle, PackageList);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = RemoveFormPackages (Private, Handle, PackageList);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = RemoveKeyboardLayoutPackages (Private, Handle, PackageList);
      if (EFI_ERROR (Status)) {
        return Status;
      }      
      Status = RemoveStringPackages (Private, Handle, PackageList);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = RemoveFontPackages (Private, Handle, PackageList);
      if (EFI_ERROR (Status)) {
        return Status;
      }      
      Status = RemoveImagePackages (Private, Handle, PackageList);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = RemoveSimpleFontPackages (Private, Handle, PackageList);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      Status = RemoveDevicePathPackage (Private, Handle, PackageList);
      if (EFI_ERROR (Status)) {
        return Status;
      }      
      
      //
      // Free resources of the package list
      //
      RemoveEntryList (&Node->DatabaseEntry);

      HiiHandle = (HII_HANDLE *) Handle;
      RemoveEntryList (&HiiHandle->Handle);
      Private->HiiHandleCount--;
      ASSERT (Private->HiiHandleCount >= 0);

      HiiHandle->Signature = 0;
      EfiLibSafeFreePool (HiiHandle);
      EfiLibSafeFreePool (Node->PackageList);
      EfiLibSafeFreePool (Node);
      
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
HiiUpdatePackageList (
  IN CONST EFI_HII_DATABASE_PROTOCOL    *This,
  IN EFI_HII_HANDLE                     Handle,
  IN CONST EFI_HII_PACKAGE_LIST_HEADER  *PackageList
  )
/*++

  Routine Description:
    This function updates the existing package list (which has the specified Handle) 
    in the HII databases, using the new package list specified by PackageList.
    
  Arguments:          
    This              - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
    Handle            - The handle that was registered to the data that is 
                        requested to be updated.
    PackageList       - A pointer to an EFI_HII_PACKAGE_LIST_HEADER package.
    
  Returns:
    EFI_SUCCESS            - The HII database was successfully updated.
    EFI_OUT_OF_RESOURCES   - Unable to allocate enough memory for the updated database.
    EFI_INVALID_PARAMETER  - PackageList was NULL.
    EFI_NOT_FOUND          - The specified Handle is not in database.
    
--*/
{
  EFI_STATUS                          Status;
  HII_DATABASE_PRIVATE_DATA           *Private;
  EFI_LIST_ENTRY                      *Link;
  HII_DATABASE_RECORD                 *Node;
  EFI_HII_PACKAGE_HEADER              *PackageHdrPtr;
  HII_DATABASE_PACKAGE_LIST_INSTANCE  *OldPackageList;
  EFI_HII_PACKAGE_HEADER              PackageHeader;

  if (This == NULL || PackageList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsHiiHandleValid (Handle)) {
    return EFI_NOT_FOUND;
  }

  Private = HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS (This);

  PackageHdrPtr = (EFI_HII_PACKAGE_HEADER *) ((UINT8 *) PackageList + sizeof (EFI_HII_PACKAGE_LIST_HEADER));

  Status = EFI_SUCCESS;
  
  //
  // Get original packagelist to be updated
  //
  for (Link = Private->DatabaseList.ForwardLink; Link != &Private->DatabaseList; Link = Link->ForwardLink) {
    Node = CR (Link, HII_DATABASE_RECORD, DatabaseEntry, HII_DATABASE_RECORD_SIGNATURE);
    if (Node->Handle == Handle) {
      OldPackageList = Node->PackageList;
      //
      // Remove the package if its type matches one of the package types which is
      // contained in the new package list.
      //
      EfiCopyMem (&PackageHeader, PackageHdrPtr, sizeof (EFI_HII_PACKAGE_HEADER));
      while (PackageHeader.Type != EFI_HII_PACKAGE_END) {
        switch (PackageHeader.Type) {
        case EFI_HII_PACKAGE_TYPE_GUID:
          Status = RemoveGuidPackages (Private, Handle, OldPackageList);
          break;
        case EFI_HII_PACKAGE_FORMS:
          Status = RemoveFormPackages (Private, Handle, OldPackageList);
          break;
        case EFI_HII_PACKAGE_KEYBOARD_LAYOUT:
          Status = RemoveKeyboardLayoutPackages (Private, Handle, OldPackageList);
          break;
        case EFI_HII_PACKAGE_STRINGS:
          Status = RemoveStringPackages (Private, Handle, OldPackageList);
          break;
        case EFI_HII_PACKAGE_FONTS:
          Status = RemoveFontPackages (Private, Handle, OldPackageList);
          break;
        case EFI_HII_PACKAGE_IMAGES:
          Status = RemoveImagePackages (Private, Handle, OldPackageList);
          break;
        case EFI_HII_PACKAGE_SIMPLE_FONTS:
          Status = RemoveSimpleFontPackages (Private, Handle, OldPackageList);
          break;
        case EFI_HII_PACKAGE_DEVICE_PATH:
          Status = RemoveDevicePathPackage (Private, Handle, OldPackageList);
          break;
        }
        
        if (EFI_ERROR (Status)) {
          return Status;
        }
        
        PackageHdrPtr = (EFI_HII_PACKAGE_HEADER *) ((UINT8 *) PackageHdrPtr + PackageHeader.Length);
        EfiCopyMem (&PackageHeader, PackageHdrPtr, sizeof (EFI_HII_PACKAGE_HEADER));
      }

      //
      // Add all of the packages within the new package list
      //      
      return AddPackages (Private, EFI_HII_DATABASE_NOTIFY_ADD_PACK, PackageList, Node);
    }
  }
  
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
HiiListPackageLists (
  IN  CONST EFI_HII_DATABASE_PROTOCOL   *This,
  IN  UINT8                             PackageType,
  IN  CONST EFI_GUID                    *PackageGuid,
  IN  OUT UINTN                         *HandleBufferLength,
  OUT EFI_HII_HANDLE                    *Handle
  )
/*++

  Routine Description:
    This function returns a list of the package handles of the specified type 
    that are currently active in the database. The pseudo-type 
    EFI_HII_PACKAGE_TYPE_ALL will cause all package handles to be listed.
    
  Arguments:          
    This               - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.    
    PackageType        - Specifies the package type of the packages to list or
                         EFI_HII_PACKAGE_TYPE_ALL for all packages to be listed.
    PackageGuid        - If PackageType is EFI_HII_PACKAGE_TYPE_GUID, then this 
                         is the pointer to the GUID which must match the Guid
                         field of EFI_HII_GUID_PACKAGE_GUID_HDR. Otherwise, 
                         it must be NULL.                
    HandleBufferLength - On input, a pointer to the length of the handle buffer. 
                         On output, the length of the handle buffer that is
                         required for the handles found.
    Handle             - An array of EFI_HII_HANDLE instances returned.
        
  Returns:
    EFI_SUCCESS            - The matching handles are outputed successfully.
                             HandleBufferLength is updated with the actual length.
    EFI_BUFFER_TO_SMALL    - The HandleBufferLength parameter indicates that
                             Handle is too small to support the number of handles.
                             HandleBufferLength is updated with a value that will 
                             enable the data to fit.
    EFI_NOT_FOUND          - No matching handle could not be found in database.
    EFI_INVALID_PARAMETER  - Handle or HandleBufferLength was NULL.
    EFI_INVALID_PARAMETER  - PackageType is not a EFI_HII_PACKAGE_TYPE_GUID but
                             PackageGuid is not NULL, PackageType is a EFI_HII_
                             PACKAGE_TYPE_GUID but PackageGuid is NULL.
     
--*/
{
  HII_GUID_PACKAGE_INSTANCE           *GuidPackage;
  HII_DATABASE_PRIVATE_DATA           *Private;
  HII_DATABASE_RECORD                 *Node;
  EFI_LIST_ENTRY                      *Link;
  BOOLEAN                             Matched;
  HII_HANDLE                          **Result;
  UINTN                               ResultSize;
  HII_DATABASE_PACKAGE_LIST_INSTANCE  *PackageList;
  EFI_LIST_ENTRY                      *Link1;
  
  //
  // Check input parameters
  //
  if (This == NULL || HandleBufferLength == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (*HandleBufferLength > 0 && Handle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if ((PackageType == EFI_HII_PACKAGE_TYPE_GUID && PackageGuid == NULL) ||
      (PackageType != EFI_HII_PACKAGE_TYPE_GUID && PackageGuid != NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Private    = HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS (This);
  Matched    = FALSE;
  Result     = (HII_HANDLE **) Handle;
  ResultSize = 0;
  
  for (Link = Private->DatabaseList.ForwardLink; Link != &Private->DatabaseList; Link = Link->ForwardLink) {
    Node = CR (Link, HII_DATABASE_RECORD, DatabaseEntry, HII_DATABASE_RECORD_SIGNATURE);
    PackageList = (HII_DATABASE_PACKAGE_LIST_INSTANCE *) (Node->PackageList);
    switch (PackageType) {
      case EFI_HII_PACKAGE_TYPE_GUID:
        for (Link1 = PackageList->GuidPkgHdr.ForwardLink; Link1 != &PackageList->GuidPkgHdr; Link1 = Link1->ForwardLink) {          
          GuidPackage = CR (Link1, HII_GUID_PACKAGE_INSTANCE, GuidEntry, HII_GUID_PACKAGE_SIGNATURE);
          if (EfiCompareGuid (
                (EFI_GUID *) PackageGuid, 
                (EFI_GUID *) (GuidPackage->GuidPkg + sizeof (EFI_HII_PACKAGE_HEADER))
                )) {
            Matched = TRUE;
            break;
          }
        }
        break;
      case EFI_HII_PACKAGE_FORMS:
        if (!IsListEmpty (&PackageList->FormPkgHdr)) {
          Matched = TRUE;
        }
        break;
      case EFI_HII_PACKAGE_KEYBOARD_LAYOUT:
        if (!IsListEmpty (&PackageList->KeyboardLayoutHdr)) {
          Matched = TRUE;
        }
        break;     
      case EFI_HII_PACKAGE_STRINGS:
        if (!IsListEmpty (&PackageList->StringPkgHdr)) {
          Matched = TRUE;
        }
        break;
      case EFI_HII_PACKAGE_FONTS:
        if (!IsListEmpty (&PackageList->FontPkgHdr)) {
          Matched = TRUE;
        }
        break;
      case EFI_HII_PACKAGE_IMAGES:
        if (PackageList->ImagePkg != NULL) {
          Matched = TRUE;
        }
        break;
      case EFI_HII_PACKAGE_SIMPLE_FONTS:
        if (!IsListEmpty (&PackageList->SimpleFontPkgHdr)) {
          Matched = TRUE;
        }
        break;
      case EFI_HII_PACKAGE_DEVICE_PATH:
        if (PackageList->DevicePathPkg != NULL) {
          Matched = TRUE;
        }
        break;   
        //
        // Pesudo-type EFI_HII_PACKAGE_TYPE_ALL will cause all package handles 
        // to be listed.
        //
      case EFI_HII_PACKAGE_TYPE_ALL:
        Matched = TRUE;
        break;
      default:
        break;
    }

    //
    // This active package list has the specified package type, list it.
    //
    if (Matched) {
      ResultSize += sizeof (EFI_HII_HANDLE);
      if (ResultSize <= *HandleBufferLength) {
        *Result++ = Node->Handle;
      }
    }
    Matched = FALSE;
  }

  if (ResultSize == 0) {
    return EFI_NOT_FOUND;
  }

  if (*HandleBufferLength < ResultSize) {
    *HandleBufferLength = ResultSize;
    return EFI_BUFFER_TOO_SMALL;
  }

  *HandleBufferLength = ResultSize;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HiiExportPackageLists (
  IN  CONST EFI_HII_DATABASE_PROTOCOL   *This,
  IN  EFI_HII_HANDLE                    Handle,
  IN  OUT UINTN                         *BufferSize,
  OUT EFI_HII_PACKAGE_LIST_HEADER       *Buffer
  )
/*++

  Routine Description:
    This function will export one or all package lists in the database to a buffer. 
    For each package list exported, this function will call functions registered 
    with EXPORT_PACK and then copy the package list to the buffer.    
    
  Arguments:          
    This               - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
    Handle             - An EFI_HII_HANDLE that corresponds to the desired package
                         list in the HII database to export or NULL to indicate 
                         all package lists should be exported.
    BufferSize         - On input, a pointer to the length of the buffer. 
                         On output, the length of the buffer that is required for
                         the exported data.
    Buffer             - A pointer to a buffer that will contain the results of 
                         the export function.
                         
  Returns:
    EFI_SUCCESS            - Package exported.
    EFI_BUFFER_TO_SMALL    - The HandleBufferLength parameter indicates that Handle
                             is too small to support the number of handles.     
                             HandleBufferLength is updated with a value that will
                             enable the data to fit.
    EFI_NOT_FOUND          - The specifiecd Handle could not be found in the current
                             database.
    EFI_INVALID_PARAMETER  - Handle or Buffer or BufferSize was NULL.
     
--*/
{
  EFI_LIST_ENTRY                      *Link;
  EFI_STATUS                          Status;
  HII_DATABASE_PRIVATE_DATA           *Private;
  HII_DATABASE_RECORD                 *Node;
  UINTN                               UsedSize;
  
  if (This == NULL || BufferSize == NULL || Handle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (*BufferSize > 0 && Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (!IsHiiHandleValid (Handle)) {
    return EFI_NOT_FOUND;
  }
  
  Private  = HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS (This);
  UsedSize = 0;
  
  for (Link = Private->DatabaseList.ForwardLink; Link != &Private->DatabaseList; Link = Link->ForwardLink) {
    Node = CR (Link, HII_DATABASE_RECORD, DatabaseEntry, HII_DATABASE_RECORD_SIGNATURE);
    if (Handle == NULL) {
      //
      // Export all package lists in current hii database.
      // 
      Status = ExportPackageList (
                 Private, 
                 Node->Handle, 
                 (HII_DATABASE_PACKAGE_LIST_INSTANCE *) (Node->PackageList), 
                 &UsedSize, 
                 *BufferSize, 
                 (EFI_HII_PACKAGE_LIST_HEADER *)((UINT8 *) Buffer + UsedSize)
                 );
      ASSERT_EFI_ERROR (Status);    
    }
    else if (Handle != NULL && Node->Handle == Handle) {
      Status = ExportPackageList (
                 Private,
                 Handle,
                 (HII_DATABASE_PACKAGE_LIST_INSTANCE *) (Node->PackageList), 
                 &UsedSize, 
                 *BufferSize,
                 Buffer                 
                 );
      ASSERT_EFI_ERROR (Status);
      if (*BufferSize < UsedSize) {
        *BufferSize = UsedSize;
        return EFI_BUFFER_TOO_SMALL;
      }
      return EFI_SUCCESS;
    }
  }  

  if (Handle == NULL && UsedSize != 0) {
    if (*BufferSize < UsedSize) {
      *BufferSize = UsedSize;
      return EFI_BUFFER_TOO_SMALL;
    }
    return EFI_SUCCESS;
  }
  
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
HiiRegisterPackageNotify (
  IN  CONST EFI_HII_DATABASE_PROTOCOL   *This,
  IN  UINT8                             PackageType,
  IN  CONST EFI_GUID                    *PackageGuid,
  IN  CONST EFI_HII_DATABASE_NOTIFY     PackageNotifyFn,
  IN  EFI_HII_DATABASE_NOTIFY_TYPE      NotifyType,
  OUT EFI_HANDLE                        *NotifyHandle
  )
/*++

  Routine Description:
    This function registers a function which will be called when specified actions related to packages of
    the specified type occur in the HII database. By registering a function, other HII-related drivers are
    notified when specific package types are added, removed or updated in the HII database.
    Each driver or application which registers a notification should use
    EFI_HII_DATABASE_PROTOCOL.UnregisterPackageNotify() before exiting. 
    
  Arguments:          
    This               - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.    
    PackageType        - Specifies the package type of the packages to list or
                         EFI_HII_PACKAGE_TYPE_ALL for all packages to be listed.    
    PackageGuid        - If PackageType is EFI_HII_PACKAGE_TYPE_GUID, then this is the pointer to
                         the GUID which must match the Guid field of                             
                         EFI_HII_GUID_PACKAGE_GUID_HDR. Otherwise, it must be NULL.          
    PackageNotifyFn    - Points to the function to be called when the event specified by                           
                         NotificationType occurs.
    NotifyType         - Describes the types of notification which this function will be receiving.                     
    NotifyHandle       - Points to the unique handle assigned to the registered notification. Can be used in
                         EFI_HII_DATABASE_PROTOCOL.UnregisterPackageNotify() to stop notifications.                                                                     
                         
  Returns:
    EFI_SUCCESS            - Notification registered successfully.    
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary data structures    
    EFI_INVALID_PARAMETER  - NotifyHandle is NULL.
    EFI_INVALID_PARAMETER  - PackageGuid is not NULL when PackageType is not
                             EFI_HII_PACKAGE_TYPE_GUID.                     
    EFI_INVALID_PARAMETER  - PackageGuid is NULL when PackageType is EFI_HII_PACKAGE_TYPE_GUID.
     
--*/  
{
  HII_DATABASE_PRIVATE_DATA           *Private;
  HII_DATABASE_NOTIFY                 *Notify;
  EFI_STATUS                          Status;
  
  if (This == NULL || NotifyHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if ((PackageType == EFI_HII_PACKAGE_TYPE_GUID && PackageGuid == NULL) ||
      (PackageType != EFI_HII_PACKAGE_TYPE_GUID && PackageGuid != NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Private = HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS (This);

  //
  // Allocate a notification node
  //
  Notify = (HII_DATABASE_NOTIFY *) EfiLibAllocateZeroPool (sizeof (HII_DATABASE_NOTIFY));
  if (Notify == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Generate a notify handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Notify->NotifyHandle,
                  &mHiiDatabaseNotifyGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  
  //
  // Fill in the information to the notification node
  //
  Notify->Signature       = HII_DATABASE_NOTIFY_SIGNATURE;
  Notify->PackageType     = PackageType;
  Notify->PackageGuid     = (EFI_GUID *) PackageGuid;
  Notify->PackageNotifyFn = (EFI_HII_DATABASE_NOTIFY) PackageNotifyFn;
  Notify->NotifyType      = NotifyType;

  InsertTailList (&Private->DatabaseNotifyList, &Notify->DatabaseNotifyEntry);
  *NotifyHandle = Notify->NotifyHandle;
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI 
HiiUnregisterPackageNotify (
  IN CONST EFI_HII_DATABASE_PROTOCOL    *This,
  IN EFI_HANDLE                         NotificationHandle
  )
/*++

  Routine Description:
    Removes the specified HII database package-related notification.
    
  Arguments:          
    This               - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.        
    NotifyHandle       - The handle of the notification function being unregistered.                         
                         
  Returns:
    EFI_SUCCESS            - Notification is unregistered successfully.    
    EFI_NOT_FOUND          - The incoming notification handle does not exist 
                             in current hii database.
     
--*/  
{
  HII_DATABASE_PRIVATE_DATA           *Private;
  HII_DATABASE_NOTIFY                 *Notify;
  EFI_LIST_ENTRY                      *Link;
  EFI_STATUS                          Status;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (NotificationHandle == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = gBS->OpenProtocol (
                  NotificationHandle,
                  &mHiiDatabaseNotifyGuid,
                  NULL,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  Private = HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS (This);

  for (Link = Private->DatabaseNotifyList.ForwardLink; Link != &Private->DatabaseNotifyList; Link = Link->ForwardLink) {
    Notify = CR (Link, HII_DATABASE_NOTIFY, DatabaseNotifyEntry, HII_DATABASE_NOTIFY_SIGNATURE);
    if (Notify->NotifyHandle == NotificationHandle) {
      //
      // Remove the matching notification node
      //
      RemoveEntryList (&Notify->DatabaseNotifyEntry);
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      Notify->NotifyHandle,
                      &mHiiDatabaseNotifyGuid,
                      NULL,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      EfiLibSafeFreePool (Notify);
      Notify = NULL;

      return EFI_SUCCESS;
    }
  }
  
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI 
HiiFindKeyboardLayouts (
  IN  EFI_HII_DATABASE_PROTOCOL         *This,
  IN  OUT UINT16                        *KeyGuidBufferLength,
  OUT EFI_GUID                          *KeyGuidBuffer
  )
/*++

  Routine Description:
    This routine retrieves an array of GUID values for each keyboard layout that
    was previously registered in the system.
    
  Arguments:          
    This                - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
    KeyGuidBufferLength - On input, a pointer to the length of the keyboard GUID 
                          buffer. On output, the length of the handle buffer 
                          that is required for the handles found.
    KeyGuidBuffer       - An array of keyboard layout GUID instances returned.
    
  Returns:
    EFI_SUCCESS            - KeyGuidBuffer was updated successfully.
    EFI_BUFFER_TOO_SMALL   - The KeyGuidBufferLength parameter indicates   
                             that KeyGuidBuffer is too small to support the
                             number of GUIDs. KeyGuidBufferLength is       
                             updated with a value that will enable the data to fit.
    EFI_INVALID_PARAMETER  - The KeyGuidBuffer or KeyGuidBufferLength was NULL.
    EFI_NOT_FOUND          - There was no keyboard layout.

--*/
{
  HII_DATABASE_PRIVATE_DATA            *Private;
  HII_DATABASE_RECORD                  *Node;
  HII_DATABASE_PACKAGE_LIST_INSTANCE   *PackageList;
  EFI_LIST_ENTRY                       *Link;
  EFI_LIST_ENTRY                       *Link1;
  UINT16                               ResultSize;
  UINTN                                Index;
  UINT16                               LayoutCount;
  UINT16                               LayoutLength;
  UINT8                                *Layout;
  HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE *Package;

  if (This == NULL || KeyGuidBufferLength == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*KeyGuidBufferLength > 0 && KeyGuidBuffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  Private     = HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS (This);
  ResultSize  = 0;

  //
  // Search all package lists in whole database to retrieve keyboard layout.
  //
  for (Link = Private->DatabaseList.ForwardLink; Link != &Private->DatabaseList; Link = Link->ForwardLink) {
    Node = CR (Link, HII_DATABASE_RECORD, DatabaseEntry, HII_DATABASE_RECORD_SIGNATURE);
    PackageList = Node->PackageList;
    for (Link1 = PackageList->KeyboardLayoutHdr.ForwardLink; 
         Link1 != &PackageList->KeyboardLayoutHdr; 
         Link1 = Link1->ForwardLink
        ) {
      //
      // Find out all Keyboard Layout packages in this package list.
      //        
      Package = CR (
                  Link1, 
                  HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE,
                  KeyboardEntry,
                  HII_KB_LAYOUT_PACKAGE_SIGNATURE
                  );
      Layout = (UINT8 *) Package->KeyboardPkg + sizeof (EFI_HII_PACKAGE_HEADER) + sizeof (UINT16);
      EfiCopyMem (
        &LayoutCount, 
        (UINT8 *) Package->KeyboardPkg + sizeof (EFI_HII_PACKAGE_HEADER), 
        sizeof (UINT16)
        );
      for (Index = 0; Index < LayoutCount; Index++) {
        ResultSize += sizeof (EFI_GUID);
        if (ResultSize <= *KeyGuidBufferLength) {
          EfiCopyMem (KeyGuidBuffer + (ResultSize / sizeof (EFI_GUID) - 1), Layout + sizeof (UINT16), sizeof (EFI_GUID));
          EfiCopyMem (&LayoutLength, Layout, sizeof (UINT16));
          Layout = Layout + LayoutLength;
        }
      }
    }
  }

  if (ResultSize == 0) {
    return EFI_NOT_FOUND;
  }

  if (*KeyGuidBufferLength < ResultSize) {
    *KeyGuidBufferLength = ResultSize;
    return EFI_BUFFER_TOO_SMALL;
  }

  *KeyGuidBufferLength = ResultSize;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI 
HiiGetKeyboardLayout (
  IN  EFI_HII_DATABASE_PROTOCOL         *This,
  IN  EFI_GUID                          *KeyGuid,
  IN OUT UINT16                         *KeyboardLayoutLength,
  OUT EFI_HII_KEYBOARD_LAYOUT           *KeyboardLayout
  )
/*++

  Routine Description:
    This routine retrieves the requested keyboard layout. The layout is a physical description of the keys
    on a keyboard and the character(s) that are associated with a particular set of key strokes.          
    
  Arguments:          
    This                 - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.            
    KeyGuid              - A pointer to the unique ID associated with a given keyboard layout. If KeyGuid is
                           NULL then the current layout will be retrieved.             
    KeyboardLayoutLength - On input, a pointer to the length of the KeyboardLayout buffer. 
                           On output, the length of the data placed into KeyboardLayout.                         
    KeyboardLayout       - A pointer to a buffer containing the retrieved keyboard layout.                                           
    
  Returns:
    EFI_SUCCESS            - The keyboard layout was retrieved successfully.
    EFI_NOT_FOUND          - The requested keyboard layout was not found.
    EFI_INVALID_PARAMETER  - The KeyboardLayout or KeyboardLayoutLength was NULL.
    EFI_BUFFER_TOO_SMALL   - The KeyboardLayoutLength parameter indicates   
                             that KeyboardLayout is too small to support the
                             requested keyboard layout. KeyboardLayoutLength is       
                             updated with a value that will enable the data to fit.
     
--*/    
{
  HII_DATABASE_PRIVATE_DATA            *Private;
  HII_DATABASE_RECORD                  *Node;
  HII_DATABASE_PACKAGE_LIST_INSTANCE   *PackageList;
  EFI_LIST_ENTRY                       *Link;
  EFI_LIST_ENTRY                       *Link1;
  UINTN                                Index;
  UINT8                                *Layout;
  UINT16                               LayoutCount;
  UINT16                               LayoutLength;
  HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE *Package;

  if (This == NULL || KeyboardLayoutLength == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (*KeyboardLayoutLength > 0 && KeyboardLayout == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS (This);
  //
  // Retrieve the current keyboard layout.
  //
  if (KeyGuid == NULL) {
    if (Private->CurrentLayout == NULL) {
      return EFI_NOT_FOUND;
    }      
    EfiCopyMem (&LayoutLength, Private->CurrentLayout, sizeof (UINT16));
    if (*KeyboardLayoutLength < LayoutLength) {
      *KeyboardLayoutLength = LayoutLength;
      return EFI_BUFFER_TOO_SMALL;
    }
    EfiCopyMem (KeyboardLayout, Private->CurrentLayout, LayoutLength);
    return EFI_SUCCESS;
  }

  for (Link = Private->DatabaseList.ForwardLink; Link != &Private->DatabaseList; Link = Link->ForwardLink) {
    Node = CR (Link, HII_DATABASE_RECORD, DatabaseEntry, HII_DATABASE_RECORD_SIGNATURE);
    PackageList = (HII_DATABASE_PACKAGE_LIST_INSTANCE *) (Node->PackageList);
    for (Link1 = PackageList->KeyboardLayoutHdr.ForwardLink; 
         Link1 != &PackageList->KeyboardLayoutHdr; 
         Link1 = Link1->ForwardLink
        ) {
      Package = CR (
                  Link1,
                  HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE,
                  KeyboardEntry,
                  HII_KB_LAYOUT_PACKAGE_SIGNATURE
                  );
      
      Layout = (UINT8 *) Package->KeyboardPkg + 
               sizeof (EFI_HII_PACKAGE_HEADER) + sizeof (UINT16);
      EfiCopyMem (&LayoutCount, Layout - sizeof (UINT16), sizeof (UINT16));
      for (Index = 0; Index < LayoutCount; Index++) {
        EfiCopyMem (&LayoutLength, Layout, sizeof (UINT16));
        if (EfiCompareMem (Layout + sizeof (UINT16), KeyGuid, sizeof (EFI_GUID)) == 0) {
          if (LayoutLength <= *KeyboardLayoutLength) {
            EfiCopyMem (KeyboardLayout, Layout, LayoutLength);
            return EFI_SUCCESS;
          } else {
            *KeyboardLayoutLength = LayoutLength;
            return EFI_BUFFER_TOO_SMALL;
          }          
        }
        Layout = Layout + LayoutLength;
      }      
    }
  }  
  
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI 
HiiSetKeyboardLayout (
  IN EFI_HII_DATABASE_PROTOCOL          *This,
  IN EFI_GUID                           *KeyGuid
  )
/*++

  Routine Description:
    This routine sets the default keyboard layout to the one referenced by KeyGuid. When this routine  
    is called, an event will be signaled of the EFI_HII_SET_KEYBOARD_LAYOUT_EVENT_GUID                 
    group type. This is so that agents which are sensitive to the current keyboard layout being changed
    can be notified of this change.                                                                    
    
  Arguments:          
    This                - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.            
    KeyGuid             - A pointer to the unique ID associated with a given keyboard layout.                                       
    
  Returns:
    EFI_SUCCESS            - The current keyboard layout was successfully set.
    EFI_NOT_FOUND          - The referenced keyboard layout was not found, so action was taken.                                                     
    EFI_INVALID_PARAMETER  - The KeyGuid was NULL.
     
--*/    
{
  HII_DATABASE_PRIVATE_DATA            *Private;
  EFI_HII_KEYBOARD_LAYOUT              *KeyboardLayout;
  UINT16                               KeyboardLayoutLength;
  EFI_STATUS                           Status;

  if (This == NULL || KeyGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS (This);

  //
  // The specified GUID equals the current keyboard layout GUID,
  // return directly.
  //
  if (EfiCompareGuid (&Private->CurrentLayoutGuid, KeyGuid)) {
    return EFI_SUCCESS;
  }

  //
  // Try to find the incoming keyboard layout data in current database.
  //
  KeyboardLayoutLength = 0;
  KeyboardLayout       = NULL;
  Status = HiiGetKeyboardLayout (This, KeyGuid, &KeyboardLayoutLength, KeyboardLayout);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return Status;
  }

  KeyboardLayout = (EFI_HII_KEYBOARD_LAYOUT *) EfiLibAllocateZeroPool (KeyboardLayoutLength);
  ASSERT (KeyboardLayout != NULL);
  Status = HiiGetKeyboardLayout (This, KeyGuid, &KeyboardLayoutLength, KeyboardLayout);
  ASSERT_EFI_ERROR (Status);
  
  //
  // Backup current keyboard layout.
  //
  EfiCopyMem (&Private->CurrentLayoutGuid, KeyGuid, sizeof (EFI_GUID));
  EfiLibSafeFreePool(Private->CurrentLayout);
  Private->CurrentLayout = KeyboardLayout;
  
  //
  // Signal EFI_HII_SET_KEYBOARD_LAYOUT_EVENT_GUID group to notify
  // current keyboard layout is changed.
  //
  Status = gBS->SignalEvent (gHiiKeyboardLayoutChanged);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HiiGetPackageListHandle (
  IN  EFI_HII_DATABASE_PROTOCOL         *This,
  IN  EFI_HII_HANDLE                    PackageListHandle,
  OUT EFI_HANDLE                        *DriverHandle
  )
/*++

  Routine Description:
    Return the EFI handle associated with a package list.
    
  Arguments:          
    This                - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.            
    PackageListHandle   - An EFI_HII_HANDLE that corresponds to the desired package list in the
                          HIIdatabase.                                                         
    DriverHandle        - On return, contains the EFI_HANDLE which was registered with the package list in
                          NewPackageList().                                                               
                          
  Returns:
    EFI_SUCCESS            - The DriverHandle was returned successfully.    
    EFI_INVALID_PARAMETER  - The PackageListHandle was not valid or DriverHandle was NULL.
    EFI_NOT_FOUND          - This PackageList handle can not be found in current database.
     
--*/    
{
  HII_DATABASE_PRIVATE_DATA           *Private;  
  HII_DATABASE_RECORD                 *Node;
  EFI_LIST_ENTRY                      *Link;
  
  if (This == NULL || DriverHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsHiiHandleValid (PackageListHandle)) {
    return EFI_INVALID_PARAMETER;
  }

  Private = HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS (This);
  
  for (Link = Private->DatabaseList.ForwardLink; Link != &Private->DatabaseList; Link = Link->ForwardLink) {
    Node = CR (Link, HII_DATABASE_RECORD, DatabaseEntry, HII_DATABASE_RECORD_SIGNATURE);
    if (Node->Handle == PackageListHandle) {
      *DriverHandle = Node->DriverHandle;
      return EFI_SUCCESS;
    }
  }
  
  return EFI_NOT_FOUND;
}

