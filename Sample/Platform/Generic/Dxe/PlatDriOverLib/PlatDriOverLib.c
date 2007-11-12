/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    PlatDriOverLib.c
    
Abstract:

--*/

#include "PlatDriOverLib.h"
#include "EfiPrintLib.h"

STATIC EFI_GUID   mOverrideVariableGuid = { 0x8e3d4ad5, 0xf762, 0x438a, { 0xa1, 0xc1, 0x5b, 0x9f, 0xe6, 0x8c, 0x6b, 0x15 } };
STATIC EFI_LIST_ENTRY  mDevicePathStack = INITIALIZE_LIST_HEAD_VARIABLE (mDevicePathStack);

EFI_STATUS
EFIAPI
LibInstallPlatformDriverOverrideProtocol (
  EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL *gPlatformDriverOverride
  )
/*++

Routine Description:
  Install the Platform Driver Override Protocol, and ensure there is only one Platform Driver Override Protocol
  in the system.

Arguments:
  gPlatformDriverOverride - PlatformDriverOverride protocol interface which needs to be installed

Returns: 
  EFI_ALREADY_STARTED - There has been a Platform Driver Override Protocol in the system, cannot install it again.
  Other - Returned by InstallProtocolInterface
--*/
{
  EFI_HANDLE          Handle;
  EFI_STATUS          Status;
  UINTN               HandleCount;
  EFI_HANDLE          *HandleBuffer;
  
  //
  // There will be only one platform driver override protocol in the system
  // If there is another out there, someone is trying to install us again,
  // Fail that scenario.
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPlatformDriverOverrideProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  //
  // If there was no error, assume there is an installation and return error
  //
  if (!EFI_ERROR (Status)) {
    if (HandleBuffer != NULL) {
      gBS->FreePool (HandleBuffer);
    }
    return EFI_ALREADY_STARTED;
  }
  
  //
  // Install platform driver override protocol
  //
  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEfiPlatformDriverOverrideProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  gPlatformDriverOverride
                  );
  return Status;
}

EFI_STATUS
EFIAPI
LibFreeMappingDatabase (
  IN  OUT  EFI_LIST_ENTRY        *MappingDataBase
  )
/*++

Routine Description:
    Free all the mapping database memory resource and initialize the mapping list entry

Arguments:
    MappingDataBase - Mapping database list entry pointer

Returns:
    EFI_INVALID_PARAMETER -  mapping database list entry is NULL
    EFI_SUCCESS - Free success


--*/
{ 
  EFI_LIST_ENTRY              *OverrideItemListIndex;
  EFI_LIST_ENTRY              *ImageInfoListIndex;
  PLATFORM_OVERRIDE_ITEM      *OverrideItem;
  DRIVER_IMAGE_INFO           *DriverImageInfo;
  
  if (MappingDataBase == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  OverrideItemListIndex = MappingDataBase->ForwardLink;
  while (OverrideItemListIndex != MappingDataBase){
    OverrideItem = CR(OverrideItemListIndex, PLATFORM_OVERRIDE_ITEM, Link, PLATFORM_OVERRIDE_ITEM_SIGNATURE);
    //
    // Free PLATFORM_OVERRIDE_ITEM.ControllerDevicePath[]
    //
    if (OverrideItem->ControllerDevicePath != NULL){
      EfiLibSafeFreePool(OverrideItem->ControllerDevicePath);     
    }   

    ImageInfoListIndex =  OverrideItem->DriverInfoList.ForwardLink; 
    while (ImageInfoListIndex != &OverrideItem->DriverInfoList){
      //
      // Free all DRIVER_IMAGE_INFO.DriverImagePath[]
      //          
      DriverImageInfo = CR(ImageInfoListIndex, DRIVER_IMAGE_INFO, Link, DRIVER_IMAGE_INFO_SIGNATURE);
      if (DriverImageInfo->DriverImagePath != NULL) {
        EfiLibSafeFreePool(DriverImageInfo->DriverImagePath); 
      }
      //
      // Free DRIVER_IMAGE_INFO itself
      //
      ImageInfoListIndex = ImageInfoListIndex->ForwardLink;
      RemoveEntryList (&DriverImageInfo->Link);
      EfiLibSafeFreePool (DriverImageInfo);
    }
    //
    // Free PLATFORM_OVERRIDE_ITEM itself
    //
    OverrideItemListIndex =  OverrideItemListIndex->ForwardLink;
    RemoveEntryList (&OverrideItem->Link);
    EfiLibSafeFreePool (OverrideItem);  
  }      

  InitializeListHead (MappingDataBase);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LibInitOverridesMapping (
  OUT  EFI_LIST_ENTRY        *MappingDataBase
  )
/*++

Routine Description:
     Read the environment variable(s) that contain the override mappings from Controller Device Path to
     a set of Driver Device Paths, and create the mapping database in memory with those variable info.

     VariableLayout{
          //
          // NotEnd indicate whether the variable is the last one, and has no subsequent variable need to load. 
          // Each variable has MaximumVariableSize limitation, so  we maybe need multi variables to store
          // large mapping infos.
          // The variable(s) name rule is PlatDriOver, PlatDriOver1, PlatDriOver2, ....
          //
          UINT32			                                  NotEnd;         
          
          //
          // The entry which contains the mapping that Controller Device Path to a set of Driver Device Paths
          // There are often multi mapping entries in a variable.
          //
          UINT32                                                SIGNATURE;            //EFI_SIGNATURE_32('p','d','o','i')
          UINT32			                                  DriverNum;
          EFI_DEVICE_PATH_PROTOCOL   		  ControllerDevicePath[];
          EFI_DEVICE_PATH_PROTOCOL 		  DriverDevicePath[];
          EFI_DEVICE_PATH_PROTOCOL 		  DriverDevicePath[]; 
          EFI_DEVICE_PATH_PROTOCOL 		  DriverDevicePath[]; 
          ......
          
          UINT32                                                SIGNATURE;
          UINT32			                                  DriverNum;
          EFI_DEVICE_PATH_PROTOCOL   		  ControllerDevicePath[];
          EFI_DEVICE_PATH_PROTOCOL 		  DriverDevicePath[]; 
          EFI_DEVICE_PATH_PROTOCOL 		  DriverDevicePath[]; 
          EFI_DEVICE_PATH_PROTOCOL 		  DriverDevicePath[]; 
          ......
        }  
  

    typedef struct _PLATFORM_OVERRIDE_ITEM{
        UINTN                                            Signature;                  //EFI_SIGNATURE_32('p','d','o','i')
        EFI_LIST_ENTRY              		     Link;
        UINT32					                       DriverInfoNum;
        EFI_DEVICE_PATH_PROTOCOL        *ControllerDevicePath;
        EFI_LIST_ENTRY              		     DriverInfoList;         //DRIVER_IMAGE_INFO List
      } PLATFORM_OVERRIDE_ITEM;

    typedef struct _DRIVER_IMAGE_INFO{
        UINTN                                           Signature;                  //EFI_SIGNATURE_32('p','d','i','i')
        EFI_LIST_ENTRY              		    Link;
        EFI_HANDLE				                 ImageHandle;
        EFI_DEVICE_PATH_PROTOCOL       *DriverImagePath;
        BOOLEAN				                      UnLoadable;
        BOOLEAN				                      UnStartable;
      } DRIVER_IMAGE_INFO; 
     
Arguments:
    MappingDataBase - Mapping database list entry pointer

Returns:
    EFI_INVALID_PARAMETER - MappingDataBase pointer is null
    EFI_NOT_FOUND - Cannot find the 'PlatDriOver' NV variable
    EFI_VOLUME_CORRUPTED  - The found NV variable is corrupted
    EFI_SUCCESS - Create the mapping database in memory successfully

--*/
{ 
  //EFI_STATUS                  Status;
  UINTN                       BufferSize;
  VOID                        *VariableBuffer;
  UINT8                       *VariableIndex;
  UINTN                       VariableNum;
  CHAR16                      OverrideVariableName[40];  
  UINT32                      NotEnd;
  UINT32                      DriverNumber;
  PLATFORM_OVERRIDE_ITEM      *OverrideItem;
  DRIVER_IMAGE_INFO           *DriverImageInfo;
  BOOLEAN                     Corrupted;
  UINT32                      Signature;
  EFI_DEVICE_PATH_PROTOCOL    *ControllerDevicePath;
  EFI_DEVICE_PATH_PROTOCOL    *DriverDevicePath;
  UINTN                       Index;
  
  if (MappingDataBase == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  VariableNum = 0;
  Corrupted = FALSE;
  //
  // Check the environment variable(s) that contain the override mappings .
  //
  VariableBuffer = GetVariableAndSize (L"PlatDriOver", &mOverrideVariableGuid, &BufferSize);
  ASSERT ((UINTN) VariableBuffer % sizeof(UINTN) == 0);
  VariableNum ++;
  if (VariableBuffer == NULL) {
    return EFI_NOT_FOUND;
  }
  
  do {
    VariableIndex = VariableBuffer;
    NotEnd = *(UINT32*) VariableIndex;
    VariableIndex = VariableIndex + sizeof (UINT32);
    while (VariableIndex < ((UINT8 *)VariableBuffer + BufferSize)) {
      OverrideItem = EfiLibAllocateZeroPool (sizeof (PLATFORM_OVERRIDE_ITEM));  
      ASSERT (OverrideItem != NULL);
      OverrideItem->Signature = PLATFORM_OVERRIDE_ITEM_SIGNATURE;
      InitializeListHead (&OverrideItem->DriverInfoList);
      //
      // Check SIGNATURE
      //
      Signature = *(UINT32 *) VariableIndex;
      VariableIndex = VariableIndex + sizeof (UINT32);
      if (Signature != PLATFORM_OVERRIDE_ITEM_SIGNATURE) {
        EfiLibSafeFreePool (OverrideItem);
        Corrupted = TRUE;
        break;
      }
      //
      // Get DriverNum
      //
      DriverNumber = *(UINT32*) VariableIndex;
      OverrideItem->DriverInfoNum = DriverNumber;
      VariableIndex = VariableIndex + sizeof (UINT32);
      //
      // Get ControllerDevicePath[]
      //
      ControllerDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) VariableIndex;
      OverrideItem->ControllerDevicePath = EfiDuplicateDevicePath (ControllerDevicePath);
      VariableIndex = VariableIndex + EfiDevicePathSize (ControllerDevicePath);
      //
      // Align the VariableIndex since the controller device path may not be aligned, refer to the LibSaveOverridesMapping()
      //
      VariableIndex += ((sizeof(UINT32) - ((UINTN) (VariableIndex))) & (sizeof(UINT32) - 1));

      //
      // Get all DriverDevicePath[]
      //
      for (Index = 0; Index < DriverNumber; Index++) {
        DriverImageInfo = EfiLibAllocateZeroPool (sizeof (DRIVER_IMAGE_INFO)); 
        ASSERT (DriverImageInfo != NULL);
        DriverImageInfo->Signature = DRIVER_IMAGE_INFO_SIGNATURE;
      
        DriverDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) VariableIndex;
        DriverImageInfo->DriverImagePath = EfiDuplicateDevicePath (DriverDevicePath);
        VariableIndex = VariableIndex + EfiDevicePathSize (DriverDevicePath);  
        //
        // Align the VariableIndex since the driver image device path may not be aligned, refer to the LibSaveOverridesMapping()
        //
        VariableIndex += ((sizeof(UINT32) - ((UINTN) (VariableIndex))) & (sizeof(UINT32) - 1));

        InsertTailList (&OverrideItem->DriverInfoList, &DriverImageInfo->Link);
      }
      InsertTailList (MappingDataBase, &OverrideItem->Link);
    }  
    
    EfiLibSafeFreePool (VariableBuffer);
    if (Corrupted) {
      LibFreeMappingDatabase (MappingDataBase);
      return EFI_VOLUME_CORRUPTED;
    }
    
    //
    // If has other variable(PlatDriOver1, PlatDriOver2, PlatDriOver3.....), get them. 
    // NotEnd indicate whether current variable is the end variable.
    //
    if (NotEnd != 0) {
      SPrint (OverrideVariableName, sizeof (OverrideVariableName), L"PlatDriOver%d", VariableNum); 
      VariableBuffer = GetVariableAndSize (OverrideVariableName, &mOverrideVariableGuid, &BufferSize);
      ASSERT ((UINTN) VariableBuffer % sizeof(UINTN) == 0);
      VariableNum ++;
      if (VariableBuffer == NULL) {
        LibFreeMappingDatabase (MappingDataBase);
        return EFI_VOLUME_CORRUPTED;
      }
    }
    
  } while (NotEnd != 0);
  
  return EFI_SUCCESS;
}

UINTN
EFIAPI
GetOneItemNeededSize (
  IN  EFI_LIST_ENTRY        *OverrideItemListIndex
  )
/*++

Routine Description:
    Calculate the needed size in NV variable for recording a specific PLATFORM_OVERRIDE_ITEM info 

Arguments:
    OverrideItemListIndex - a list entry point to a specific PLATFORM_OVERRIDE_ITEM

Returns:
    The needed size number


--*/
{ 
  UINTN                       NeededSize;
  PLATFORM_OVERRIDE_ITEM      *OverrideItem;
  EFI_LIST_ENTRY              *ImageInfoListIndex;
  DRIVER_IMAGE_INFO           *DriverImageInfo;
  
  
  NeededSize = 0;
  OverrideItem = CR(OverrideItemListIndex, PLATFORM_OVERRIDE_ITEM, Link, PLATFORM_OVERRIDE_ITEM_SIGNATURE);
  NeededSize += sizeof (UINT32); //UINT32	SIGNATURE;
  NeededSize += sizeof (UINT32); //UINT32	DriverNum;
  NeededSize += EfiDevicePathSize (OverrideItem->ControllerDevicePath); // ControllerDevicePath
  //
  // Align the controller device path
  //
  NeededSize += ((sizeof(UINT32) - ((UINTN) EfiDevicePathSize (OverrideItem->ControllerDevicePath))) \
                  & (sizeof(UINT32) - 1));
  ImageInfoListIndex =  OverrideItem->DriverInfoList.ForwardLink; 
  while (ImageInfoListIndex != &OverrideItem->DriverInfoList){
    DriverImageInfo = CR(ImageInfoListIndex, DRIVER_IMAGE_INFO, Link, DRIVER_IMAGE_INFO_SIGNATURE);
    NeededSize += EfiDevicePathSize (DriverImageInfo->DriverImagePath); //DriverDevicePath
    //
    // Align the driver image device path
    //
    NeededSize += ((sizeof(UINT32) - ((UINTN) EfiDevicePathSize (DriverImageInfo->DriverImagePath))) \
                    & (sizeof(UINT32) - 1));
    ImageInfoListIndex = ImageInfoListIndex->ForwardLink;
  }

  return NeededSize;
}
 

EFI_STATUS
EFIAPI
LibSaveOverridesMapping (
  IN  EFI_LIST_ENTRY          *MappingDataBase
  )
/*++

Routine Description:
    Save the memory mapping database into NV environment variable(s)

Arguments:
    MappingDataBase - Mapping database list entry pointer

Returns:
    EFI_INVALID_PARAMETER - MappingDataBase pointer is null
    EFI_SUCCESS - Save memory mapping database successfully

--*/
{ 
  EFI_STATUS                  Status;
  VOID                        *VariableBuffer;
  UINT8                       *VariableIndex;
  UINTN                       NumIndex;
  CHAR16                      OverrideVariableName[40];  
  UINT32                      NotEnd;
  PLATFORM_OVERRIDE_ITEM      *OverrideItem;
  DRIVER_IMAGE_INFO           *DriverImageInfo;
  EFI_LIST_ENTRY              *OverrideItemListIndex;
  EFI_LIST_ENTRY              *ItemIndex;
  EFI_LIST_ENTRY              *ImageInfoListIndex;
  UINTN                       VariableNeededSize;
  UINTN                       SavedSize;
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  UINT64                      MaximumVariableStorageSize;
  UINT64                      RemainingVariableStorageSize;
#endif
  UINT64                      MaximumVariableSize;
  
  if (MappingDataBase == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (MappingDataBase->ForwardLink == MappingDataBase) {
    Status = LibDeleteOverridesVariables ();
    //ASSERT (!EFI_ERROR(Status));  
    return EFI_SUCCESS;
  }
  DEBUG ((EFI_D_ERROR, "begin QueryVariableInfo \n"));
  //
  // Get the the maximum size of an individual EFI variable in current system
  //
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  gRT->QueryVariableInfo (
          EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
          &MaximumVariableStorageSize,
          &RemainingVariableStorageSize,
          &MaximumVariableSize
          );
#else
  MaximumVariableSize = 0x300;
#endif

  DEBUG ((EFI_D_ERROR, "Steven: MaximumVariableSize =  0x%x\n", MaximumVariableSize));
  NumIndex = 0;
  OverrideItemListIndex = MappingDataBase->ForwardLink;
  while (OverrideItemListIndex != MappingDataBase) {
    //
    // Try to find the most proper variable size which <= MaximumVariableSize, but can contain mapping info as much as possible
    //
    VariableNeededSize = 0;
    VariableNeededSize += sizeof (UINT32); //BOOLEAN	NotEnd; 
    ItemIndex = OverrideItemListIndex;
    NotEnd = FALSE;
    DEBUG ((EFI_D_ERROR, "step1 begin \n"));
    while (ItemIndex != MappingDataBase){ 
      if ((VariableNeededSize + 
           GetOneItemNeededSize (ItemIndex) + 
           sizeof (VARIABLE_HEADER) + 
           EfiStrSize (L"PlatDriOver ")
           ) >= MaximumVariableSize
          ) {
        NotEnd = TRUE;
        break;
      }
      DEBUG ((EFI_D_ERROR, "step1: a item pass \n"));
      VariableNeededSize += GetOneItemNeededSize (ItemIndex);  
      ItemIndex =  ItemIndex->ForwardLink;
    } 
    
    if (NotEnd) {
      if (VariableNeededSize == sizeof (UINT32)) {
        //
        // If an individual EFI variable cannot contain a single Item, return error
        //
        return EFI_OUT_OF_RESOURCES;
      }
    }
    DEBUG ((EFI_D_ERROR, "step1 done \n"));
    //
    // VariableNeededSize is the most proper variable size, allocate variable buffer
    // ItemIndex now points to the next PLATFORM_OVERRIDE_ITEM which is not covered by VariableNeededSize
    //
    VariableBuffer = EfiLibAllocateZeroPool (VariableNeededSize);
    ASSERT ((UINTN) VariableBuffer % sizeof(UINTN) == 0);
    DEBUG ((EFI_D_ERROR, "Allocation done \n"));
    //
    // Fill the variable buffer according to MappingDataBase
    //
    SavedSize = 0;
    VariableIndex = VariableBuffer;
    *(UINT32 *) VariableIndex = NotEnd;    
    VariableIndex += sizeof (UINT32); // pass NoEnd
    SavedSize += sizeof (UINT32);
    //
    // ItemIndex points to the next PLATFORM_OVERRIDE_ITEM which is not covered by VariableNeededSize
    //
    DEBUG ((EFI_D_ERROR, "step2 begin: VariableIndex = 0x%x\n", (UINTN) VariableIndex));
    while (OverrideItemListIndex != ItemIndex){
      *(UINT32 *) VariableIndex = PLATFORM_OVERRIDE_ITEM_SIGNATURE;  
      VariableIndex += sizeof (UINT32); // pass SIGNATURE
      SavedSize += sizeof (UINT32);
      DEBUG ((EFI_D_ERROR, "step2.1 done: VariableIndex = 0x%x\n", (UINTN) VariableIndex));
      
      OverrideItem = CR(OverrideItemListIndex, PLATFORM_OVERRIDE_ITEM, Link, PLATFORM_OVERRIDE_ITEM_SIGNATURE);
      *(UINT32 *) VariableIndex = OverrideItem->DriverInfoNum;
      VariableIndex += sizeof (UINT32); // pass DriverNum
      SavedSize += sizeof (UINT32);
      DEBUG ((EFI_D_ERROR, "step2.2 done: VariableIndex = 0x%x\n", (UINTN) VariableIndex));

      EfiCopyMem (VariableIndex, OverrideItem->ControllerDevicePath, EfiDevicePathSize (OverrideItem->ControllerDevicePath));
      VariableIndex += EfiDevicePathSize (OverrideItem->ControllerDevicePath); // pass ControllerDevicePath
      SavedSize += EfiDevicePathSize (OverrideItem->ControllerDevicePath);
      DEBUG ((EFI_D_ERROR, "step2.3 done: VariableIndex = 0x%x\n", (UINTN) VariableIndex));
      //
      // Align the VariableIndex since the controller device path may not be aligned
      //
      SavedSize += ((sizeof(UINT32) - ((UINTN) (VariableIndex))) & (sizeof(UINT32) - 1));
      VariableIndex += ((sizeof(UINT32) - ((UINTN) (VariableIndex))) & (sizeof(UINT32) - 1));

      ImageInfoListIndex =  OverrideItem->DriverInfoList.ForwardLink; 
      while (ImageInfoListIndex != &OverrideItem->DriverInfoList){
        DriverImageInfo = CR(ImageInfoListIndex, DRIVER_IMAGE_INFO, Link, DRIVER_IMAGE_INFO_SIGNATURE);
        EfiCopyMem (VariableIndex, DriverImageInfo->DriverImagePath, EfiDevicePathSize (DriverImageInfo->DriverImagePath));
        DEBUG ((EFI_D_ERROR, "step2: EfiCopyMem pass \n"));
        VariableIndex += EfiDevicePathSize (DriverImageInfo->DriverImagePath); // pass DriverImageDevicePath
        SavedSize += EfiDevicePathSize (DriverImageInfo->DriverImagePath);
        //
        // Align the VariableIndex since the driver image device path may not be aligned
        //
        SavedSize += ((sizeof(UINT32) - ((UINTN) (VariableIndex))) & (sizeof(UINT32) - 1));
        VariableIndex += ((sizeof(UINT32) - ((UINTN) (VariableIndex))) & (sizeof(UINT32) - 1));
        DEBUG ((EFI_D_ERROR, "step2: EfiCopyMem pass: VariableIndex = 0x%x\n", (UINTN) VariableIndex));
        ImageInfoListIndex = ImageInfoListIndex->ForwardLink;
      }
      DEBUG ((EFI_D_ERROR, "step2: a item pass \n"));
      OverrideItemListIndex =  OverrideItemListIndex->ForwardLink;
      DEBUG ((EFI_D_ERROR, "step2: next item VariableIndex = 0x%x\n", (UINTN) VariableIndex));
    }
    
    DEBUG ((EFI_D_ERROR, "VariableNeededSize = 0x%x \n", VariableNeededSize));
    ASSERT (SavedSize == VariableNeededSize);
    
    if (NumIndex == 0) {
      SPrint (OverrideVariableName, sizeof (OverrideVariableName), L"PlatDriOver"); 
    } else {
      SPrint (OverrideVariableName, sizeof (OverrideVariableName), L"PlatDriOver%d", NumIndex ); 
    }
    
    DEBUG ((EFI_D_ERROR, "begin setvariable \n"));
    Status = gRT->SetVariable (
                    OverrideVariableName,
                    &mOverrideVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    VariableNeededSize,
                    VariableBuffer
                    );
    DEBUG ((EFI_D_ERROR, "Status = 0x%x\n", Status));
    ASSERT (!EFI_ERROR(Status));
    
    NumIndex ++;
    EfiLibSafeFreePool (VariableBuffer);
  }

  return EFI_SUCCESS;  
}

EFI_STATUS
EFIAPI
LibGetDriverFromMapping (
  IN EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL              * This,
  IN     EFI_HANDLE                                     ControllerHandle,
  IN OUT EFI_HANDLE                                     * DriverImageHandle,
  IN     EFI_LIST_ENTRY                                 * MappingDataBase,
  IN     EFI_HANDLE                                     CallerImageHandle
  )
/*++

Routine Description:
    Retrieves the image handle of the platform override driver for a controller in the system from the memory mapping database.

Arguments:
    This                      -  A pointer to the EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL instance.
    ControllerHandle     - The device handle of the controller to check if a driver override exists.
    DriverImageHandle  -  On output, a pointer to the next driver handle. 
                                     Passing in a pointer to NULL, will return the first driver handle for ControllerHandle.
    MappingDataBase    -  MappingDataBase - Mapping database list entry pointer
    CallerImageHandle  -  The caller driver's image handle, for UpdateFvFileDevicePath use.

Returns:
    EFI_INVALID_PARAMETER - The handle specified by ControllerHandle is not a valid handle. 
                                                Or DriverImagePath is not a device path that was returned on a previous call to GetDriverPath().
    EFI_NOT_FOUND - A driver override for ControllerHandle was not found.
    EFI_UNSUPPORTED - The operation is not supported.
    EFI_SUCCESS - The driver override for ControllerHandle was returned in DriverImagePath.
    

--*/
{ 
  EFI_STATUS                  Status;
  EFI_DEVICE_PATH_PROTOCOL    *ControllerDevicePath;
  BOOLEAN                     ControllerFound;
  BOOLEAN                     ImageFound;
  EFI_HANDLE                  *ImageHandleBuffer;
  UINTN                       ImageHandleCount;
  UINTN                       Index;
  EFI_LOADED_IMAGE_PROTOCOL   *LoadedImage;
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding;
  BOOLEAN                     FoundLastReturned;
  PLATFORM_OVERRIDE_ITEM      *OverrideItem;
  DRIVER_IMAGE_INFO           *DriverImageInfo;
  EFI_LIST_ENTRY              *OverrideItemListIndex;
  EFI_LIST_ENTRY              *ImageInfoListIndex;
  EFI_DEVICE_PATH_PROTOCOL    *TempDriverImagePath;
  EFI_HANDLE                  ImageHandle;
  EFI_HANDLE                  Handle;
  EFI_DEVICE_PATH_PROTOCOL    *LoadedImageHandleDevicePath;
  EFI_DEVICE_PATH_PROTOCOL    *TatalFilePath;
  EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL  *BusSpecificDriverOverride;
  //
  // Check that ControllerHandle is a valid handle
  //
  if (ControllerHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  Status = gBS->HandleProtocol (
              ControllerHandle,     
              &gEfiDevicePathProtocolGuid, 
              &ControllerDevicePath
              );
  if (EFI_ERROR (Status) || ControllerDevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }  
  
  //
  // Search ControllerDevicePath in MappingDataBase
  //
  OverrideItem = NULL;
  ControllerFound = FALSE;
  OverrideItemListIndex = MappingDataBase->ForwardLink;
  while (OverrideItemListIndex != MappingDataBase){
    OverrideItem = CR(OverrideItemListIndex, PLATFORM_OVERRIDE_ITEM, Link, PLATFORM_OVERRIDE_ITEM_SIGNATURE);
    if (EfiDevicePathSize (ControllerDevicePath) ==
        EfiDevicePathSize (OverrideItem->ControllerDevicePath)) {
      if (EfiCompareMem (
            ControllerDevicePath, 
            OverrideItem->ControllerDevicePath, 
            EfiDevicePathSize (OverrideItem->ControllerDevicePath)
            ) == 0
          ) {
        ControllerFound = TRUE;
        break;
      }        
        
    }
    OverrideItemListIndex =  OverrideItemListIndex->ForwardLink;
  }
  
  if (!ControllerFound) {
    return EFI_NOT_FOUND;
  }
  DEBUG ((EFI_D_ERROR, "Steven: Controller Found! \n"));
  //
  // Passing in a pointer to NULL, will return the first driver device path for ControllerHandle.
  // Check whether the driverImagePath is not a device path that was returned on a previous call to GetDriverPath().
  //
  if (*DriverImageHandle != NULL) {
    if (*DriverImageHandle != OverrideItem->LastReturnedImageHandle) {
      return EFI_INVALID_PARAMETER;
    }
  }  
  //
  // The GetDriverPath() maybe called recursively, because it use ConnectDevicePath() internally, 
  //  so should check whether there is a dead loop. 
  //  Here use a controller device path stack to record all processed controller device path during a GetDriverPath() call,
  //  and check the  controller device path whether appear again during the  GetDriverPath() call.
  //
  if (CheckExistInStack(OverrideItem->ControllerDevicePath)) {
    //
    // There is a dependecy dead loop if the ControllerDevicePath appear in stack twice 
    //
    return EFI_UNSUPPORTED;
  }
  PushDevPathStack (OverrideItem->ControllerDevicePath);
  
  //
  // Check every override driver, try to load and start them
  //
  ImageInfoListIndex =  OverrideItem->DriverInfoList.ForwardLink; 
  while (ImageInfoListIndex != &OverrideItem->DriverInfoList){
    DriverImageInfo = CR(ImageInfoListIndex, DRIVER_IMAGE_INFO, Link, DRIVER_IMAGE_INFO_SIGNATURE);
    if (DriverImageInfo->ImageHandle == NULL) {
      DEBUG ((EFI_D_ERROR, "Steven: DriverImageInfo->ImageHandle == NULL \n"));
      //
      // Skip if the image is unloadable or unstartable
      //
      if ((!DriverImageInfo->UnLoadable) && ((!DriverImageInfo->UnStartable))) {
        TempDriverImagePath = DriverImageInfo->DriverImagePath;
        //
        // If the image device path contain a FV node, check the Fv file device path is valid. If it is invalid, try to return the valid device path.
        // FV address maybe changes for memory layout adjust from time to time, use this funciton could promise the Fv file device path is right.
        //
        Status = UpdateFvFileDevicePath (&TempDriverImagePath, NULL, CallerImageHandle);
        if (!EFI_ERROR (Status)) {
          EfiLibSafeFreePool(DriverImageInfo->DriverImagePath); 
          DriverImageInfo->DriverImagePath = TempDriverImagePath;
        }  
        //
        // Get all Loaded Image protocol to check whether the driver image has been loaded and started
        //
        ImageFound = FALSE;
        ImageHandleCount  = 0;
        Status = gBS->LocateHandleBuffer (
                        ByProtocol,
                        &gEfiLoadedImageProtocolGuid,
                        NULL,
                        &ImageHandleCount,
                        &ImageHandleBuffer
                        );
        if (EFI_ERROR (Status) || (ImageHandleCount == 0)) {
          return EFI_NOT_FOUND;
        }
        
        for(Index = 0; Index < ImageHandleCount; Index ++) {
          Status = gBS->HandleProtocol (
                          ImageHandleBuffer[Index],
                          &gEfiLoadedImageProtocolGuid,
                          &LoadedImage
                          );
          if (EFI_ERROR (Status)) {
            continue;
          }
          
          //
          // Get the driver image total file path
          //
          LoadedImageHandleDevicePath = NULL;
          Status = gBS->HandleProtocol (
                              LoadedImage->DeviceHandle, 
                              &gEfiDevicePathProtocolGuid, 
                              &LoadedImageHandleDevicePath
                              );
          if (EFI_ERROR (Status)) {
            DEBUG ((EFI_D_ERROR, "Steven:0x%x: LoadedImage->DeviceHandle has wrong DevicePathProtocol \n", Status));
            //
            // Maybe Not all  LoadedImage->DeviceHandle has valid value.  Not continue here currently.
            //
            //continue;
          }
          
          TatalFilePath = EfiAppendDevicePath (LoadedImageHandleDevicePath, LoadedImage->FilePath);
          
          if (EfiDevicePathSize (DriverImageInfo->DriverImagePath) ==
              EfiDevicePathSize (TatalFilePath)) {
            if (EfiCompareMem (
                  DriverImageInfo->DriverImagePath, 
                  TatalFilePath, 
                  EfiDevicePathSize (TatalFilePath)
                  ) == 0
                ) {
              ImageFound = TRUE;
              break;
            }        
          }
        }

        if (ImageFound) {
          DEBUG ((EFI_D_ERROR, "Steven: Needed override Image Found! \n"));
          Status = gBS->HandleProtocol (
                          ImageHandleBuffer[Index],
                          &gEfiDriverBindingProtocolGuid,
                          &DriverBinding
                          );
          if (EFI_ERROR (Status)){
            DEBUG ((EFI_D_ERROR, "Steven: Can not locate the binding protocol!!!!! Status=0x%x\n", Status));
            ASSERT (!EFI_ERROR (Status));
          }
          DriverImageInfo->ImageHandle = ImageHandleBuffer[Index];
        } else {
          DEBUG ((EFI_D_ERROR, "Steven: Not Found Needed override Image, try to load it! \n"));
          //
          // The driver image has not been loaded and started, need try to load and start it now
          // Try to connect all device in the driver image path
          //
          Status = ConnectDevicePath (DriverImageInfo->DriverImagePath);
          //if (!EFI_ERROR (Status)) {
          //DEBUG ((EFI_D_ERROR, "Steven: ConnectDevicePath (DriverImageInfo->DriverImagePath) success! \n"));
          //
          // check whether it points to a PCI Option Rom image, and try to use bus override protocol to get its first option rom image driver
          //
          TempDriverImagePath = DriverImageInfo->DriverImagePath;
          gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &TempDriverImagePath, &Handle);
          //
          // Get the Bus Specific Driver Override Protocol instance on the Controller Handle
          //
          Status = gBS->HandleProtocol(
                           Handle,  
                           &gEfiBusSpecificDriverOverrideProtocolGuid, 
                           &BusSpecificDriverOverride
                           );
          if (!EFI_ERROR (Status) && (BusSpecificDriverOverride != NULL)) {
            ImageHandle = NULL;
            Status = BusSpecificDriverOverride->GetDriver (
                                                  BusSpecificDriverOverride,
                                                  &ImageHandle
                                                  );
            if (!EFI_ERROR (Status)) {
              Status = gBS->HandleProtocol (
                              ImageHandle,
                              &gEfiDriverBindingProtocolGuid,
                              &DriverBinding
                              );
              ASSERT (!EFI_ERROR (Status));
              DriverImageInfo->ImageHandle = ImageHandle; 
            }
          }
          //}
          //
          // Skip if any device cannot be connected now, future passes through GetDriver() may be able to load that driver. 
          // Only file path media or FwVol Device Path Node remain if all device is connected
          //
          TempDriverImagePath = DriverImageInfo->DriverImagePath;
          gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &TempDriverImagePath, &Handle);
          if (((DevicePathType (TempDriverImagePath) == MEDIA_DEVICE_PATH) &&
               (DevicePathSubType (TempDriverImagePath) == MEDIA_FILEPATH_DP)) ||
              (EfiGetNameGuidFromFwVolDevicePathNode ((MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *) TempDriverImagePath) != NULL)
             ) {
            //
            // Try to load the driver
            //
            TempDriverImagePath = DriverImageInfo->DriverImagePath;
            Status = gBS->LoadImage (
                            FALSE,
                            CallerImageHandle,
                            TempDriverImagePath,
                            NULL,
                            0,
                            &ImageHandle
                            );
            if (!EFI_ERROR (Status)) {
              //
              // Try to start the driver
              //
              Status = gBS->StartImage (ImageHandle, NULL, NULL);
              if (EFI_ERROR (Status)){
                DEBUG ((EFI_D_ERROR, "Steven: UnStartable happen! \n"));
                DriverImageInfo->UnStartable = TRUE;
                DriverImageInfo->ImageHandle = NULL;
              } else {
                Status = gBS->HandleProtocol (
                                ImageHandle,
                                &gEfiDriverBindingProtocolGuid,
                                &DriverBinding
                                );
                ASSERT (!EFI_ERROR (Status));
                DriverImageInfo->ImageHandle = ImageHandle;              
              }
            } else {
              DEBUG ((EFI_D_ERROR, "Steven: Unloadable happen! \n"));
              DriverImageInfo->UnLoadable = TRUE;
              DriverImageInfo->ImageHandle = NULL;
            }                      
          }
        }
        EfiLibSafeFreePool (ImageHandleBuffer);       
      }
    }
    ImageInfoListIndex = ImageInfoListIndex->ForwardLink;
  }
  //
  // Finish try to load and start the override driver of a controller, popup the controller's device path
  //
  PopDevPathStack (NULL);
  
  //
  // return the DriverImageHandle for ControllerHandle 
  //
  FoundLastReturned = FALSE;
  ImageInfoListIndex =  OverrideItem->DriverInfoList.ForwardLink; 
  while (ImageInfoListIndex != &OverrideItem->DriverInfoList){
    DriverImageInfo = CR(ImageInfoListIndex, DRIVER_IMAGE_INFO, Link, DRIVER_IMAGE_INFO_SIGNATURE);
    if (DriverImageInfo->ImageHandle != NULL) {
      if ((*DriverImageHandle == NULL) || FoundLastReturned) {
        OverrideItem->LastReturnedImageHandle = DriverImageInfo->ImageHandle;
        *DriverImageHandle = DriverImageInfo->ImageHandle;
        DEBUG ((EFI_D_ERROR, "Steven: return EFI_SUCCESS! \n"));
        return EFI_SUCCESS;
      } else if (*DriverImageHandle == DriverImageInfo->ImageHandle){
        FoundLastReturned = TRUE;
      } 
    }
    ImageInfoListIndex = ImageInfoListIndex->ForwardLink;
  }
  DEBUG ((EFI_D_ERROR, "Steven: return EFI_NOT_FOUND \n"));
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
LibCheckMapping (
  IN     EFI_DEVICE_PATH_PROTOCOL                       *ControllerDevicePath,
  IN     EFI_DEVICE_PATH_PROTOCOL                       *DriverImageDevicePath,
  IN     EFI_LIST_ENTRY                                 * MappingDataBase,
  OUT    UINT32                                         *DriverInfoNum,
  OUT    UINT32                                         *DriverImageNO
  )
/*++

Routine Description:
    Check mapping database whether already has the  mapping info which
    records the input Controller to input DriverImage. 
    If has, the controller's total override driver number and input DriverImage's order number is return.

Arguments:
    ControllerDevicePath - The controller device path need to add a override driver image item
    DriverImageDevicePath - The driver image device path need to be insert 
    MappingDataBase - Mapping database list entry pointer
    DriverInfoNum - the controller's total override driver number 
    DriverImageNO - The inserted order number

Returns:
    EFI_INVALID_PARAMETER
    EFI_NOT_FOUND
    EFI_SUCCESS


--*/
{ 
  EFI_LIST_ENTRY              *OverrideItemListIndex;
  PLATFORM_OVERRIDE_ITEM      *OverrideItem;
  EFI_LIST_ENTRY              *ImageInfoListIndex;
  DRIVER_IMAGE_INFO           *DriverImageInfo;
  BOOLEAN                     Found;
  UINT32                      ImageNO;
  //
  // Check that ControllerHandle is a valid handle
  //
  if (ControllerDevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (MappingDataBase == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Search ControllerDevicePath in MappingDataBase
  //
  Found = FALSE;
  OverrideItem = NULL;
  OverrideItemListIndex = MappingDataBase->ForwardLink;
  while (OverrideItemListIndex != MappingDataBase){
    OverrideItem = CR(OverrideItemListIndex, PLATFORM_OVERRIDE_ITEM, Link, PLATFORM_OVERRIDE_ITEM_SIGNATURE);
    if (EfiDevicePathSize (ControllerDevicePath) ==
        EfiDevicePathSize (OverrideItem->ControllerDevicePath)) {
      if (EfiCompareMem (
            ControllerDevicePath, 
            OverrideItem->ControllerDevicePath, 
            EfiDevicePathSize (OverrideItem->ControllerDevicePath)
            ) == 0
          ) {
        Found = TRUE;
        break;
      }        
    }
    OverrideItemListIndex =  OverrideItemListIndex->ForwardLink;
  }
  
  if (!Found) {
    return EFI_NOT_FOUND;
  }
  
  ASSERT (OverrideItem->DriverInfoNum != 0);
  if (DriverInfoNum != NULL) {
    *DriverInfoNum = OverrideItem->DriverInfoNum;  
  }

  
  if (DriverImageDevicePath == NULL) {
    return EFI_SUCCESS; 
  }
  //
  // return the DriverImageHandle for ControllerHandle 
  //
  ImageNO = 0;
  Found = FALSE;
  ImageInfoListIndex =  OverrideItem->DriverInfoList.ForwardLink; 
  while (ImageInfoListIndex != &OverrideItem->DriverInfoList){
    DriverImageInfo = CR(ImageInfoListIndex, DRIVER_IMAGE_INFO, Link, DRIVER_IMAGE_INFO_SIGNATURE);
    ImageNO++;
    if (EfiDevicePathSize (DriverImageDevicePath) ==
        EfiDevicePathSize (DriverImageInfo->DriverImagePath)) {
      if (EfiCompareMem (
            DriverImageDevicePath, 
            DriverImageInfo->DriverImagePath, 
            EfiDevicePathSize (DriverImageInfo->DriverImagePath)
            ) == 0
          ) {
        Found = TRUE;
        break;
      }        
    }
    ImageInfoListIndex = ImageInfoListIndex->ForwardLink;
  }
  
  if (!Found) {
    return EFI_NOT_FOUND;
  } else {
    if (DriverImageNO != NULL) {
      *DriverImageNO = ImageNO;    
    }
    return EFI_SUCCESS;  
  }

}

EFI_STATUS
EFIAPI
LibInsertDriverImage (
  IN     EFI_DEVICE_PATH_PROTOCOL                       *ControllerDevicePath,
  IN     EFI_DEVICE_PATH_PROTOCOL                       *DriverImageDevicePath,
  IN     EFI_LIST_ENTRY                                 *MappingDataBase,
  IN     UINT32                                         DriverImageNO
  )
/*++

Routine Description:
    Insert a driver image as a controller's override driver into the mapping database. 
    The driver image's order number is indicated by DriverImageNO.

Arguments:
    ControllerDevicePath - The controller device path need to add a override driver image item
    DriverImageDevicePath - The driver image device path need to be insert 
    MappingDataBase - Mapping database list entry pointer
    DriverImageNO - The inserted order number

Returns:
    EFI_INVALID_PARAMETER 
    EFI_ALREADY_STARTED
    EFI_SUCCESS

--*/
{ 
  EFI_STATUS                  Status;
  EFI_LIST_ENTRY              *OverrideItemListIndex;
  PLATFORM_OVERRIDE_ITEM      *OverrideItem;
  EFI_LIST_ENTRY              *ImageInfoListIndex;
  DRIVER_IMAGE_INFO           *DriverImageInfo;
  BOOLEAN                     Found;
  UINT32                      ImageNO;
  //
  // Check that ControllerHandle is a valid handle
  //
  if (ControllerDevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (DriverImageDevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }  
  if (MappingDataBase == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = LibCheckMapping (
            ControllerDevicePath,
            DriverImageDevicePath,
            MappingDataBase,
            NULL,
            NULL
            ); 
  if (Status == EFI_SUCCESS) {
    return EFI_ALREADY_STARTED;
  } 
  
  //
  // Search the input ControllerDevicePath in MappingDataBase
  //
  Found = FALSE;
  OverrideItem = NULL;
  OverrideItemListIndex = MappingDataBase->ForwardLink;
  while (OverrideItemListIndex != MappingDataBase){
    OverrideItem = CR(OverrideItemListIndex, PLATFORM_OVERRIDE_ITEM, Link, PLATFORM_OVERRIDE_ITEM_SIGNATURE);
    if (EfiDevicePathSize (ControllerDevicePath) ==
        EfiDevicePathSize (OverrideItem->ControllerDevicePath)) {
      if (EfiCompareMem (
            ControllerDevicePath, 
            OverrideItem->ControllerDevicePath, 
            EfiDevicePathSize (OverrideItem->ControllerDevicePath)
            ) == 0
          ) {
        Found = TRUE;
        break;
      }        
    }
    OverrideItemListIndex =  OverrideItemListIndex->ForwardLink;
  }
  //
  // If cannot find, this is a new controller item
  // Add the Controller related PLATFORM_OVERRIDE_ITEM structrue in mapping data base
  //
  if (!Found) {
    OverrideItem = EfiLibAllocateZeroPool (sizeof (PLATFORM_OVERRIDE_ITEM));  
    ASSERT (OverrideItem != NULL);
    OverrideItem->Signature = PLATFORM_OVERRIDE_ITEM_SIGNATURE;
    OverrideItem->ControllerDevicePath = EfiDuplicateDevicePath (ControllerDevicePath);
    InitializeListHead (&OverrideItem->DriverInfoList);
    InsertTailList (MappingDataBase, &OverrideItem->Link);
  }
  
  //
  // Prepare the driver image related DRIVER_IMAGE_INFO structure.
  //
  DriverImageInfo = EfiLibAllocateZeroPool (sizeof (DRIVER_IMAGE_INFO)); 
  ASSERT (DriverImageInfo != NULL);
  DriverImageInfo->Signature = DRIVER_IMAGE_INFO_SIGNATURE;
  DriverImageInfo->DriverImagePath = EfiDuplicateDevicePath (DriverImageDevicePath);  
  //
  // Find the driver image wantted order location
  //
  ImageNO = 0;
  Found = FALSE;
  ImageInfoListIndex =  OverrideItem->DriverInfoList.ForwardLink; 
  while (ImageInfoListIndex != &OverrideItem->DriverInfoList){
    if (ImageNO == (DriverImageNO - 1)) {
      //
      // find the wantted order location, insert it
      //    
      InsertTailList (ImageInfoListIndex, &DriverImageInfo->Link);
      OverrideItem->DriverInfoNum ++;
      Found = TRUE;
      break;
    }
    //DriverImageInfo = CR(ImageInfoListIndex, DRIVER_IMAGE_INFO, Link, DRIVER_IMAGE_INFO_SIGNATURE);
    ImageNO++;
    ImageInfoListIndex = ImageInfoListIndex->ForwardLink;
  }  
  
  if (!Found) {
    //
    // if not find the wantted order location, add it as last item of the controller mapping item
    //      
    InsertTailList (&OverrideItem->DriverInfoList, &DriverImageInfo->Link);
    OverrideItem->DriverInfoNum ++;
  }
  
  return EFI_SUCCESS;  
}

EFI_STATUS
EFIAPI
LibDeleteDriverImage (
  IN     EFI_DEVICE_PATH_PROTOCOL                       *ControllerDevicePath,
  IN     EFI_DEVICE_PATH_PROTOCOL                       *DriverImageDevicePath,
  IN     EFI_LIST_ENTRY                                 *MappingDataBase
  )
/*++

Routine Description:
    Delete a controller's override driver from the mapping database. 

Arguments:
    ControllerDevicePath - The controller device path need to add a override driver image item
    DriverImageDevicePath - The driver image device path need to be insert 
    MappingDataBase - Mapping database list entry pointer
    DriverImageNO - The inserted order number

Returns:
    EFI_INVALID_PARAMETER
    EFI_NOT_FOUND
    EFI_SUCCESS
    
--*/
{ 
  EFI_STATUS                  Status;
  EFI_LIST_ENTRY              *OverrideItemListIndex;
  PLATFORM_OVERRIDE_ITEM      *OverrideItem;
  EFI_LIST_ENTRY              *ImageInfoListIndex;
  DRIVER_IMAGE_INFO           *DriverImageInfo;
  BOOLEAN                     Found;
  //
  // Check that ControllerHandle is a valid handle
  //
  if (ControllerDevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (MappingDataBase == NULL) {
    return EFI_INVALID_PARAMETER;
  }

   Status = LibCheckMapping (
              ControllerDevicePath,
              DriverImageDevicePath,
              MappingDataBase,
              NULL,
              NULL
              ); 
  if (Status == EFI_NOT_FOUND) {
    return EFI_NOT_FOUND;
  } 
  
  //
  // Search ControllerDevicePath in MappingDataBase
  //
  Found = FALSE;
  OverrideItem = NULL;
  OverrideItemListIndex = MappingDataBase->ForwardLink;
  while (OverrideItemListIndex != MappingDataBase){
    OverrideItem = CR(OverrideItemListIndex, PLATFORM_OVERRIDE_ITEM, Link, PLATFORM_OVERRIDE_ITEM_SIGNATURE);
    if (EfiDevicePathSize (ControllerDevicePath) ==
        EfiDevicePathSize (OverrideItem->ControllerDevicePath)) {
      if (EfiCompareMem (
            ControllerDevicePath, 
            OverrideItem->ControllerDevicePath, 
            EfiDevicePathSize (OverrideItem->ControllerDevicePath)
            ) == 0
          ) {
        Found = TRUE;
        break;
      }        
    }
    OverrideItemListIndex =  OverrideItemListIndex->ForwardLink;
  }
  
  ASSERT (Found);
  ASSERT (OverrideItem->DriverInfoNum != 0);
  //
  // 
  //
  Found = FALSE;
  ImageInfoListIndex =  OverrideItem->DriverInfoList.ForwardLink; 
  while (ImageInfoListIndex != &OverrideItem->DriverInfoList){
    DriverImageInfo = CR(ImageInfoListIndex, DRIVER_IMAGE_INFO, Link, DRIVER_IMAGE_INFO_SIGNATURE);
    ImageInfoListIndex = ImageInfoListIndex->ForwardLink;
    if (DriverImageDevicePath != NULL) {      
      if (EfiDevicePathSize (DriverImageDevicePath) ==
          EfiDevicePathSize (DriverImageInfo->DriverImagePath)) {
        if (EfiCompareMem (
              DriverImageDevicePath, 
              DriverImageInfo->DriverImagePath, 
              EfiDevicePathSize (DriverImageInfo->DriverImagePath)
              ) == 0
            ) {
          Found = TRUE;
          EfiLibSafeFreePool(DriverImageInfo->DriverImagePath); 
          RemoveEntryList (&DriverImageInfo->Link);
          OverrideItem->DriverInfoNum --;               
          break;
        }   
      }
    } else {
      Found = TRUE;
      EfiLibSafeFreePool(DriverImageInfo->DriverImagePath); 
      RemoveEntryList (&DriverImageInfo->Link);
      OverrideItem->DriverInfoNum --;   
    }
  }  
  
  if (DriverImageDevicePath == NULL) {
    ASSERT (OverrideItem->DriverInfoNum == 0);
  }
  
  if (OverrideItem->DriverInfoNum == 0) {
    EfiLibSafeFreePool(OverrideItem->ControllerDevicePath);   
    RemoveEntryList (&OverrideItem->Link);
    EfiLibSafeFreePool (OverrideItem);  
  }
  
  if (!Found) {
    return EFI_NOT_FOUND;
  }
  
  return EFI_SUCCESS;  
}

EFI_STATUS
EFIAPI
LibDeleteOverridesVariables (
  VOID
  )
/*++

Routine Description:
    Deletes all environment variable(s) that contain the override mappings from Controller Device Path to
     a set of Driver Device Paths.

Arguments:
    None

Returns:
    EFI_SUCCESS

--*/
{
  EFI_STATUS                  Status;
  VOID                        *VariableBuffer;
  UINTN                       VariableNum;
  UINTN                       BufferSize;
  UINTN                       Index;
  CHAR16                      OverrideVariableName[40];  
  
  //
  // Get environment variable(s)  number
  //
  VariableNum =0;
  VariableBuffer = GetVariableAndSize (L"PlatDriOver", &mOverrideVariableGuid, &BufferSize);
  VariableNum ++;
  if (VariableBuffer == NULL) {
    return EFI_NOT_FOUND;
  }
  //
  // Check NotEnd to get all PlatDriOverX variable(s)
  //
  while (*(UINT32*)VariableBuffer) {
    SPrint (OverrideVariableName, sizeof (OverrideVariableName), L"PlatDriOver%d", VariableNum); 
    VariableBuffer = GetVariableAndSize (OverrideVariableName, &mOverrideVariableGuid, &BufferSize);
    VariableNum ++;
    ASSERT (VariableBuffer != NULL);
  }

  Status = gRT->SetVariable (
                  L"PlatDriOver",
                  &mOverrideVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  0,
                  NULL
                  );
  ASSERT (!EFI_ERROR (Status));
  for (Index = 1; Index < VariableNum; Index++) {
    SPrint (OverrideVariableName, sizeof (OverrideVariableName), L"PlatDriOver%d", Index); 
    Status = gRT->SetVariable (
                    OverrideVariableName,
                    &mOverrideVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    0,
                    NULL
                    );  
    ASSERT (!EFI_ERROR (Status));
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PushDevPathStack (
  IN  EFI_DEVICE_PATH_PROTOCOL    *DevicePath
  )
/*++

Routine Description:
    Push a controller device path into a globle device path list

Arguments:
    ControllerDevicePath - The controller device path need to push into stack

Returns:
    EFI_SUCCESS


--*/
{
  DEVICE_PATH_STACK_ITEM  *DevicePathStackItem;
  
  DevicePathStackItem = EfiLibAllocateZeroPool (sizeof (DEVICE_PATH_STACK_ITEM)); 
  ASSERT (DevicePathStackItem != NULL);
  DevicePathStackItem->Signature = DEVICE_PATH_STACK_ITEM_SIGNATURE;
  DevicePathStackItem->DevicePath = EfiDuplicateDevicePath (DevicePath);
  InsertTailList (&mDevicePathStack, &DevicePathStackItem->Link);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PopDevPathStack (
  OUT  EFI_DEVICE_PATH_PROTOCOL    **DevicePath
  )
/*++

Routine Description:
    Pop a controller device path from a globle device path list

Arguments:
    ControllerDevicePath - The controller device path retrieved from stack

Returns:
    EFI_SUCCESS
    EFI_NOT_FOUND

--*/
{
  DEVICE_PATH_STACK_ITEM  *DevicePathStackItem;
  EFI_LIST_ENTRY          *ItemListIndex;

  ItemListIndex = mDevicePathStack.BackLink;
  if (ItemListIndex != &mDevicePathStack){
    DevicePathStackItem = CR(ItemListIndex, DEVICE_PATH_STACK_ITEM, Link, DEVICE_PATH_STACK_ITEM_SIGNATURE);
    if (DevicePath != NULL) {
      *DevicePath = EfiDuplicateDevicePath (DevicePathStackItem->DevicePath);
    }
    EfiLibSafeFreePool (DevicePathStackItem->DevicePath);
    RemoveEntryList (&DevicePathStackItem->Link);
    EfiLibSafeFreePool (DevicePathStackItem);
    return EFI_SUCCESS;
  }
  return EFI_NOT_FOUND;
}

BOOLEAN
EFIAPI
CheckExistInStack (
  IN  EFI_DEVICE_PATH_PROTOCOL    *DevicePath
  )
/*++

Routine Description:
    Check whether a controller device path is in a globle device path list

Arguments:
    ControllerDevicePath - The controller device path need to check

Returns:
    True
    False


--*/
{
  DEVICE_PATH_STACK_ITEM  *DevicePathStackItem;
  EFI_LIST_ENTRY          *ItemListIndex;
  BOOLEAN                 Found;
  
  Found = FALSE;
  ItemListIndex = mDevicePathStack.BackLink;
  while (ItemListIndex != &mDevicePathStack){
    DevicePathStackItem = CR(ItemListIndex, DEVICE_PATH_STACK_ITEM, Link, DEVICE_PATH_STACK_ITEM_SIGNATURE);
    if (EfiDevicePathSize (DevicePath) ==
        EfiDevicePathSize (DevicePathStackItem->DevicePath)) {
      if (EfiCompareMem (
            DevicePath, 
            DevicePathStackItem->DevicePath, 
            EfiDevicePathSize (DevicePathStackItem->DevicePath)
            ) == 0
          ) {
        Found = TRUE;
        break;
      }        
    }
    ItemListIndex = ItemListIndex->BackLink;
  }
  
  return Found;
}

EFI_STATUS
EFIAPI
UpdateFvFileDevicePath (
  IN  OUT EFI_DEVICE_PATH_PROTOCOL      ** DevicePath,
  IN  EFI_GUID                          *FileGuid,
  IN  EFI_HANDLE                        CallerImageHandle
  )
/*++

Routine Description:
   According to a file guild, check a Fv file device path is valid. If it is invalid,
   try to return the valid device path.
   FV address maybe changes for memory layout adjust from time to time, use this funciton 
   could promise the Fv file device path is right.

Arguments:
  DevicePath - on input, the Fv file device path need to check
                    on output, the updated valid Fv file device path
                    
  FileGuid - the Fv file guild
  CallerImageHandle - 
  
Returns:
  EFI_INVALID_PARAMETER - the input DevicePath or FileGuid is invalid parameter
  EFI_UNSUPPORTED - the input DevicePath does not contain Fv file guild at all
  EFI_ALREADY_STARTED - the input DevicePath has pointed to Fv file, it is valid
  EFI_SUCCESS - has successfully updated the invalid DevicePath, and return the updated
                          device path in DevicePath
                
--*/
{
  EFI_DEVICE_PATH_PROTOCOL      *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL      *LastDeviceNode;
  EFI_STATUS                    Status;
  EFI_GUID                      *GuidPoint;
  UINTN                         Index;
  UINTN                         FvHandleCount;
  EFI_HANDLE                    *FvHandleBuffer;
  EFI_FV_FILETYPE               Type;
  UINTN                         Size;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  UINT32                        AuthenticationStatus;
  BOOLEAN                       FindFvFile;
  EFI_LOADED_IMAGE_PROTOCOL     *LoadedImage;
#if (PI_SPECIFICATION_VERSION < 0x00010000)
  EFI_FIRMWARE_VOLUME_PROTOCOL  *Fv;
#else
  EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
#endif
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH FvFileNode;
  EFI_HANDLE                    FoundFvHandle;
  EFI_DEVICE_PATH_PROTOCOL      *NewDevicePath;
  BOOLEAN                       HasFVNode;
  
  if ((DevicePath == NULL) || (*DevicePath == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check whether the device path point to the default the input Fv file
  //
  TempDevicePath = *DevicePath; 
  LastDeviceNode = TempDevicePath;
  while (!EfiIsDevicePathEnd (TempDevicePath)) {
     LastDeviceNode = TempDevicePath;
     TempDevicePath = EfiNextDevicePathNode (TempDevicePath);
  }
  GuidPoint = EfiGetNameGuidFromFwVolDevicePathNode (
                (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *) LastDeviceNode
                );
  if (GuidPoint == NULL) {
    //
    // if this option does not points to a Fv file, just return EFI_UNSUPPORTED
    //
    return EFI_UNSUPPORTED;
  }
  
  if (FileGuid != NULL) {
    if (!EfiCompareGuid (GuidPoint, FileGuid)) {
      //
      // If the Fv file is not the input file guid, just return EFI_UNSUPPORTED
      //
      return EFI_UNSUPPORTED;
    }  
  } else {
    FileGuid = GuidPoint;
  }

  //
  // Check to see if the device path contain memory map node
  //
  TempDevicePath = *DevicePath; 
  HasFVNode = FALSE;
  while (!EfiIsDevicePathEnd (TempDevicePath)) {
    //
    // Use old Device Path
    //
    if (DevicePathType (TempDevicePath) == HARDWARE_DEVICE_PATH &&
        DevicePathSubType (TempDevicePath) == HW_MEMMAP_DP) {
      HasFVNode = TRUE;
      break;
    }
    TempDevicePath = EfiNextDevicePathNode (TempDevicePath);
  }
  
  if (!HasFVNode) {
    return EFI_UNSUPPORTED;
  }
  
  //
  // Check whether the input Fv file device path is valid
  //
  TempDevicePath = *DevicePath; 
  FoundFvHandle = NULL;
  Status = gBS->LocateDevicePath (
                #if (PI_SPECIFICATION_VERSION < 0x00010000)    
                  &gEfiFirmwareVolumeProtocolGuid,
                #else
                  &gEfiFirmwareVolume2ProtocolGuid,
                #endif 
                  &TempDevicePath, 
                  &FoundFvHandle
                  );
  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (
                    FoundFvHandle,
                  #if (PI_SPECIFICATION_VERSION < 0x00010000)    
                    &gEfiFirmwareVolumeProtocolGuid,
                  #else
                    &gEfiFirmwareVolume2ProtocolGuid,
                  #endif
                    (VOID **) &Fv
                    );
    if (!EFI_ERROR (Status)) {
      //
      // Set FV ReadFile Buffer as NULL, only need to check whether input Fv file exist there
      //
      Status = Fv->ReadFile (
                    Fv,
                    FileGuid,
                    NULL,
                    &Size,
                    &Type,
                    &Attributes,
                    &AuthenticationStatus
                    );
      if (!EFI_ERROR (Status)) {
        return EFI_ALREADY_STARTED;
      }
    }
  }
 
  //
  // Look for the input wanted FV file in current FV
  // First, try to look for in Caller own FV. Caller and input wanted FV file usually are in the same FV
  //
  FindFvFile = FALSE;
  FoundFvHandle = NULL;
  Status = gBS->HandleProtocol (
             CallerImageHandle,
             &gEfiLoadedImageProtocolGuid,
             &LoadedImage
             );
  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (
                    LoadedImage->DeviceHandle,
                  #if (PI_SPECIFICATION_VERSION < 0x00010000)    
                    &gEfiFirmwareVolumeProtocolGuid,
                  #else
                    &gEfiFirmwareVolume2ProtocolGuid,
                  #endif
                    (VOID **) &Fv
                    );
    if (!EFI_ERROR (Status)) {
      Status = Fv->ReadFile (
                    Fv,
                    FileGuid,
                    NULL,
                    &Size,
                    &Type,
                    &Attributes,
                    &AuthenticationStatus
                    );
      if (!EFI_ERROR (Status)) {
        FindFvFile = TRUE;
        FoundFvHandle = LoadedImage->DeviceHandle;
      }
    }
  }
  //
  // Second, if fail to find, try to enumerate all FV
  //
  if (!FindFvFile) {
    gBS->LocateHandleBuffer (
          ByProtocol,
       #if (PI_SPECIFICATION_VERSION < 0x00010000)
          &gEfiFirmwareVolumeProtocolGuid,
       #else
          &gEfiFirmwareVolume2ProtocolGuid,
       #endif   
          NULL,
          &FvHandleCount,
          &FvHandleBuffer
          );
    for (Index = 0; Index < FvHandleCount; Index++) {
      gBS->HandleProtocol (
            FvHandleBuffer[Index],
         #if (PI_SPECIFICATION_VERSION < 0x00010000)
            &gEfiFirmwareVolumeProtocolGuid,
         #else
            &gEfiFirmwareVolume2ProtocolGuid,
         #endif   
            (VOID **) &Fv
            );

      Status = Fv->ReadFile (
                    Fv,
                    FileGuid,
                    NULL,
                    &Size,
                    &Type,
                    &Attributes,
                    &AuthenticationStatus
                    );
      if (EFI_ERROR (Status)) {
        //
        // Skip if input Fv file not in the FV
        //
        continue;
      }
      FindFvFile = TRUE;
      FoundFvHandle = FvHandleBuffer[Index];
      break;
    }  
  }

  if (FindFvFile) {
    //
    // Build the shell device path
    //
    NewDevicePath = EfiDevicePathFromHandle (FoundFvHandle);
    EfiInitializeFwVolDevicepathNode (&FvFileNode, FileGuid);
    NewDevicePath = EfiAppendDevicePathNode (NewDevicePath, (EFI_DEVICE_PATH_PROTOCOL *) &FvFileNode);
    *DevicePath = NewDevicePath;    
    return EFI_SUCCESS;
  }
  return EFI_NOT_FOUND;
}

VOID *
EFIAPI
GetVariableAndSize (
  IN  CHAR16              *Name,
  IN  EFI_GUID            *VendorGuid,
  OUT UINTN               *VariableSize
  )
/*++

Routine Description:

  Read the EFI variable (VendorGuid/Name) and return a dynamically allocated
  buffer, and the size of the buffer. If failure return NULL.

Arguments:

  Name       - String part of EFI variable name

  VendorGuid - GUID part of EFI variable name

  VariableSize - Returns the size of the EFI variable that was read

Returns:

  Dynamically allocated memory that contains a copy of the EFI variable.
  Caller is responsible freeing the buffer.

  NULL - Variable was not read

--*/
{
  EFI_STATUS  Status;
  UINTN       BufferSize;
  VOID        *Buffer;

  Buffer = NULL;

  //
  // Pass in a zero size buffer to find the required buffer size.
  //
  BufferSize  = 0;
  Status      = gRT->GetVariable (Name, VendorGuid, NULL, &BufferSize, Buffer);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    //
    // Allocate the buffer to return
    //
    Buffer = EfiLibAllocateZeroPool (BufferSize);
    if (Buffer == NULL) {
      return NULL;
    }
    //
    // Read variable into the allocated buffer.
    //
    Status = gRT->GetVariable (Name, VendorGuid, NULL, &BufferSize, Buffer);
    if (EFI_ERROR (Status)) {
      BufferSize = 0;
    }
  }

  *VariableSize = BufferSize;
  return Buffer;
}

EFI_STATUS
EFIAPI
ConnectDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePathToConnect
  )
/*++

Routine Description:
  This function will create all handles associate with every device
  path node. If the handle associate with one device path node can not
  be created success, then still give one chance to do the dispatch,
  which load the missing drivers if possible.
  
Arguments:

  DevicePathToConnect  - The device path which will be connected, it can
                         be a multi-instance device path

Returns:

  EFI_SUCCESS          - All handles associate with every device path 
                         node have been created
  
  EFI_OUT_OF_RESOURCES - There is no resource to create new handles
  
  EFI_NOT_FOUND        - Create the handle associate with one device 
                         path node failed

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *CopyOfDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *Instance;
  EFI_DEVICE_PATH_PROTOCOL  *RemainingDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *Next;
  EFI_HANDLE                Handle;
  EFI_HANDLE                PreviousHandle;
  UINTN                     Size;

  if (DevicePathToConnect == NULL) {
    return EFI_SUCCESS;
  }

  DevicePath        = EfiDuplicateDevicePath (DevicePathToConnect);
  CopyOfDevicePath  = DevicePath;
  if (DevicePath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  do {
    //
    // The outer loop handles multi instance device paths.
    // Only console variables contain multiple instance device paths.
    //
    // After this call DevicePath points to the next Instance
    //
    Instance  = EfiDevicePathInstance (&DevicePath, &Size);
    Next      = Instance;
    while (!IsDevicePathEndType (Next)) {
      Next = NextDevicePathNode (Next);
    }

    SetDevicePathEndNode (Next);

    //
    // Start the real work of connect with RemainingDevicePath
    //
    PreviousHandle = NULL;
    do {
      //
      // Find the handle that best matches the Device Path. If it is only a
      // partial match the remaining part of the device path is returned in
      // RemainingDevicePath.
      //
      RemainingDevicePath = Instance;
      Status              = gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &RemainingDevicePath, &Handle);

      if (!EFI_ERROR (Status)) {
        if (Handle == PreviousHandle) {
          //
          // If no forward progress is made try invoking the Dispatcher.
          // A new FV may have been added to the system an new drivers
          // may now be found.
          // Status == EFI_SUCCESS means a driver was dispatched
          // Status == EFI_NOT_FOUND means no new drivers were dispatched
          //
          Status = gDS->Dispatch ();
        }

        if (!EFI_ERROR (Status)) {
          PreviousHandle = Handle;
          //
          // Connect all drivers that apply to Handle and RemainingDevicePath,
          // the Recursive flag is FALSE so only one level will be expanded.
          //
          // Do not check the connect status here, if the connect controller fail,
          // then still give the chance to do dispatch, because partial
          // RemainingDevicepath may be in the new FV
          //
          // 1. If the connect fail, RemainingDevicepath and handle will not
          //    change, so next time will do the dispatch, then dispatch's status
          //    will take effect
          // 2. If the connect success, the RemainingDevicepath and handle will
          //    change, then avoid the dispatch, we have chance to continue the
          //    next connection
          //
          gBS->ConnectController (Handle, NULL, RemainingDevicePath, FALSE);
        }
      }
      //
      // Loop until RemainingDevicePath is an empty device path
      //
    } while (!EFI_ERROR (Status) && !IsDevicePathEnd (RemainingDevicePath));

  } while (DevicePath != NULL);

  if (CopyOfDevicePath != NULL) {
    gBS->FreePool (CopyOfDevicePath);
  }
  //
  // All handle with DevicePath exists in the handle database
  //
  return Status;
}

EFI_STATUS
EFIAPI
LocateHandleByPciClassType (
  IN  UINT8       ClassType,
  IN  UINT8       SubClassCode,
  IN  UINT8       PI,
  IN  BOOLEAN     Recursive,
  OUT EFI_HANDLE  **Buffer
  ) 
/*++

Routine Description:
  

Arguments:
  ClassType - 
  Recursive -
  
Returns:
  EFI_SUCCESS       - 
  others            - An error occurred when 

--*/
{
  EFI_STATUS                Status;
  UINTN                     Index;
  UINTN                     HandleCount;
  EFI_HANDLE                *HandleBuffer;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  PCI_TYPE00                Pci;

  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    &PciIo
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (ClassType != 0xFF) {
      if (Pci.Hdr.ClassCode[2] != ClassType) {
        continue;
      }
    }
    
    if (SubClassCode != 0xFF) {
      if (Pci.Hdr.ClassCode[1] != SubClassCode) {
        continue;
      }
    }
    
    if (PI != 0xFF) {
      if (Pci.Hdr.ClassCode[0] != PI) {
        continue;
      }        
    }
    
    gBS->ConnectController (HandleBuffer[Index], NULL, NULL, Recursive);
  }

  if (HandleBuffer != NULL) {
    EfiLibSafeFreePool (HandleBuffer);
  }
  
  return Status;
}