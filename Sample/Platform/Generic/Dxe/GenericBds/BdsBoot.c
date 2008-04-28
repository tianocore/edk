/*++

Copyright (c) 2004 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  BdsBoot.c

Abstract:

  BDS Lib functions which relate with create or process the boot
  option.

--*/

#include "BdsLib.h"

BOOLEAN mEnumBootDevice = FALSE;

//
// This GUID is used for an EFI Variable that stores the front device pathes
// for a partial device path that starts with the HD node. 
//
EFI_GUID  mHdBootVariablePrivateGuid = { 0xfab7e9e1, 0x39dd, 0x4f2b, { 0x84, 0x8, 0xe2, 0xe, 0x90, 0x6c, 0xb6, 0xde } };


EFI_STATUS
BdsLibDoLegacyBoot (
  IN  BDS_COMMON_OPTION           *Option
  )
/*++

Routine Description:

  Boot the legacy system with the boot option

Arguments:

  Option           - The legacy boot option which have BBS device path

Returns:

  EFI_UNSUPPORTED  - There is no legacybios protocol, do not support
                     legacy boot.

  EFI_STATUS       - Return the status of LegacyBios->LegacyBoot ().

--*/
{
  EFI_STATUS                Status;
  EFI_LEGACY_BIOS_PROTOCOL  *LegacyBios;

  Status = gBS->LocateProtocol (&gEfiLegacyBiosProtocolGuid, NULL, &LegacyBios);
  if (EFI_ERROR (Status)) {
    //
    // If no LegacyBios protocol we do not support legacy boot
    //
    return EFI_UNSUPPORTED;
  }
  //
  // Notes: if we seperate the int 19, then we don't need to refresh BBS
  //
  BdsRefreshBbsTableForBoot (Option);

  //
  // Write boot to OS performance data for Legacy Boot
  //
  WRITE_BOOT_TO_OS_PERFORMANCE_DATA;

  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Legacy Boot: %S\n", Option->Description));
  return LegacyBios->LegacyBoot (
                      LegacyBios,
                      (BBS_BBS_DEVICE_PATH *) Option->DevicePath,
                      Option->LoadOptionsSize,
                      Option->LoadOptions
                      );
}

EFI_STATUS
BdsLibBootViaBootOption (
  IN  BDS_COMMON_OPTION             * Option,
  IN  EFI_DEVICE_PATH_PROTOCOL      * DevicePath,
  OUT UINTN                         *ExitDataSize,
  OUT CHAR16                        **ExitData OPTIONAL
  )
/*++

Routine Description:

  Process the boot option follow the EFI 1.1 specification and
  special treat the legacy boot option with BBS_DEVICE_PATH.

Arguments:

  Option       - The boot option need to be processed

  DevicePath   - The device path which describe where to load
                 the boot image or the legcy BBS device path
                 to boot the legacy OS

  ExitDataSize - Returned directly from gBS->StartImage ()

  ExitData     - Returned directly from gBS->StartImage ()

Returns:

  EFI_SUCCESS   - Status from gBS->StartImage ()

  EFI_NOT_FOUND - If the Device Path is not found in the system

--*/
{
  EFI_STATUS                Status;
  EFI_HANDLE                Handle;
  EFI_HANDLE                ImageHandle;
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  EFI_LOADED_IMAGE_PROTOCOL *ImageInfo;
  EFI_EVENT                 ReadyToBootEvent;
  EFI_DEVICE_PATH_PROTOCOL  *WorkingDevicePath;
  EFI_ACPI_S3_SAVE_PROTOCOL *AcpiS3Save;
  EFI_LIST_ENTRY            TempBootLists;
  //
  // Record the performance data for End of BDS
  //
  PERF_END (0, BDS_TOK, NULL, 0);

  *ExitDataSize = 0;
  *ExitData     = NULL;

  //
  // Notes: put EFI64 ROM Shadow Solution
  //
  EFI64_SHADOW_ALL_LEGACY_ROM ();

  //
  // Notes: this code can be remove after the s3 script table
  // hook on the event EFI_EVENT_SIGNAL_READY_TO_BOOT or
  // EFI_EVENT_SIGNAL_LEGACY_BOOT
  //
  Status = gBS->LocateProtocol (&gEfiAcpiS3SaveGuid, NULL, &AcpiS3Save);
  if (!EFI_ERROR (Status)) {
    AcpiS3Save->S3Save (AcpiS3Save, NULL);
  }
  //
  // If it's Device Path that starts with a hard drive path, append it with the front part to compose a
  // full device path
  //
  WorkingDevicePath = NULL;
  if ((DevicePathType (DevicePath) == MEDIA_DEVICE_PATH) &&
      (DevicePathSubType (DevicePath) == MEDIA_HARDDRIVE_DP)) {
    WorkingDevicePath = BdsExpandPartitionPartialDevicePathToFull (
                          (HARDDRIVE_DEVICE_PATH *)DevicePath
                          );
    if (WorkingDevicePath != NULL) {
      DevicePath = WorkingDevicePath;
    }
  }
  
  //
  // Set Boot Current
  //
  gRT->SetVariable (
        L"BootCurrent",
        &gEfiGlobalVariableGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        sizeof (UINT16),
        &Option->BootCurrent
        );
        
  //
  // Signal the EFI_EVENT_SIGNAL_READY_TO_BOOT event
  //
  Status = EfiCreateEventReadyToBoot (
             EFI_TPL_CALLBACK,
             NULL,
             NULL,
             &ReadyToBootEvent
             );

  if (!EFI_ERROR (Status)) {
    gBS->SignalEvent (ReadyToBootEvent);
    gBS->CloseEvent (ReadyToBootEvent);
  }


  if ((DevicePathType (Option->DevicePath) == BBS_DEVICE_PATH) &&
      (DevicePathSubType (Option->DevicePath) == BBS_BBS_DP)
    ) {
    //
    // Check to see if we should legacy BOOT. If yes then do the legacy boot
    //
    return BdsLibDoLegacyBoot (Option);
  }
  
  //
  // If the boot option point to Internal FV shell, make sure it is valid
  //
  Status = BdsLibUpdateFvFileDevicePath (&DevicePath, &gEfiShellFileGuid);
  if (!EFI_ERROR(Status)) {
    if (Option->DevicePath != NULL) {
      EfiLibSafeFreePool(Option->DevicePath);
    }
    Option->DevicePath  = EfiLibAllocateZeroPool (EfiDevicePathSize (DevicePath));
    EfiCopyMem (Option->DevicePath, DevicePath, EfiDevicePathSize (DevicePath));
    //
    // Update the shell boot option
    //
    InitializeListHead (&TempBootLists);
    BdsLibRegisterNewOption (&TempBootLists, DevicePath, L"EFI Internal Shell", L"BootOrder"); 
    //
    // free the temporary device path created by BdsLibUpdateFvFileDevicePath()
    //
    gBS->FreePool (DevicePath); 
    DevicePath = Option->DevicePath;
  }
  
  //
  // Drop the TPL level from EFI_TPL_DRIVER to EFI_TPL_APPLICATION
  //
  gBS->RestoreTPL (EFI_TPL_APPLICATION);

  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Booting EFI way %S\n", Option->Description));

  Status = gBS->LoadImage (
                  TRUE,
                  mBdsImageHandle,
                  DevicePath,
                  NULL,
                  0,
                  &ImageHandle
                  );
                  
  //
  // If we didn't find an image directly, we need to try as if it is a removable device boot opotion 
  // and load the image according to the default boot behavior for removable device.
  //
  if (EFI_ERROR (Status)) {
    //
    // check if there is a bootable removable media could be found in this device path , 
    // and get the bootable media handle 
    //
    Handle = BdsLibGetBootableHandle(DevicePath);
    if (Handle == NULL) {
       goto Done;
    }
    //
    // Load the default boot file \EFI\BOOT\boot{machinename}.EFI from removable Media
    //  machinename is ia32, ia64, x64, ...
    //
    FilePath = EfiFileDevicePath (Handle, DEFAULT_REMOVABLE_FILE_NAME); 
    if (FilePath) {
      Status = gBS->LoadImage (
                      TRUE,
                      mBdsImageHandle,
                      FilePath,
                      NULL,
                      0,
                      &ImageHandle
                      );
      if (EFI_ERROR (Status)) {
        //
        // The DevicePath failed, and it's not a valid
        // removable media device.
        //
        goto Done;
      }
    }   
  }

  if (EFI_ERROR (Status)) {
    //
    // It there is any error from the Boot attempt exit now.
    //
    goto Done;
  }
  //
  // Provide the image with it's load options
  //
  Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, &ImageInfo);
  ASSERT_EFI_ERROR (Status);

  if (Option->LoadOptionsSize != 0) {
    ImageInfo->LoadOptionsSize  = Option->LoadOptionsSize;
    ImageInfo->LoadOptions      = Option->LoadOptions;
  }
  //
  // Before calling the image, enable the Watchdog Timer for
  // the 5 Minute period
  //
  gBS->SetWatchdogTimer (5 * 60, 0x0000, 0x00, NULL);

  //
  // Write boot to OS performance data for UEFI boot
  //
  WRITE_BOOT_TO_OS_PERFORMANCE_DATA;
  
  Status = gBS->StartImage (ImageHandle, ExitDataSize, ExitData);
  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Image Return Status = %r\n", Status));

  //
  // Clear the Watchdog Timer after the image returns
  //
  gBS->SetWatchdogTimer (0x0000, 0x0000, 0x0000, NULL);

Done:
  //
  // Clear Boot Current
  //
  gRT->SetVariable (
        L"BootCurrent",
        &gEfiGlobalVariableGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        0,
        &Option->BootCurrent
        );

  //
  // Raise the TPL level back to EFI_TPL_DRIVER
  //
  gBS->RaiseTPL (EFI_TPL_DRIVER);

  return Status;
}

EFI_DEVICE_PATH_PROTOCOL *
BdsExpandPartitionPartialDevicePathToFull (
  IN  HARDDRIVE_DEVICE_PATH      *HardDriveDevicePath
  )
/*++

Routine Description:
  Expand a device path that starts with a hard drive media device path node to be a 
  full device path that includes the full hardware path to the device. We need
  to do this so it can be booted. As an optimizaiton the front match (the part point 
  to the partition node. E.g. ACPI() /PCI()/ATA()/Partition() ) is saved in a variable 
  so a connect all is not required on every boot. All successful history device path 
  which point to partition node (the front part) will be saved.
  
Arguments:
  HardDriveDevicePath - EFI Device Path to boot, if it starts with a hard
                        drive media device path.
Returns:
  A Pointer to the full device path.  
  NULL - Cannot find a valid Hard Drive devic path



--*/
{
  EFI_STATUS                Status;
  UINTN                     BlockIoHandleCount;
  EFI_HANDLE                *BlockIoBuffer;
  EFI_DEVICE_PATH_PROTOCOL  *FullDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *BlockIoDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  UINTN                     Index;
  UINTN                     InstanceNum;
  EFI_DEVICE_PATH_PROTOCOL  *CachedDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempNewDevicePath;
  UINTN                     CachedDevicePathSize;
  BOOLEAN                   DeviceExist;
  BOOLEAN                   NeedAdjust;
  EFI_DEVICE_PATH_PROTOCOL  *Instance;
  UINTN                     Size;
  
  FullDevicePath = NULL;
  //
  // Check if there is prestore 'HDDP' variable. 
  // If exist, search the front path which point to partition node in the variable instants.
  // If fail to find or 'HDDP' not exist, reconnect all and search in all system
  //
  CachedDevicePath = BdsLibGetVariableAndSize (
                      L"HDDP",
                      &mHdBootVariablePrivateGuid,  
                      &CachedDevicePathSize
                      );
  if (CachedDevicePath != NULL) {
    TempNewDevicePath = CachedDevicePath;
    DeviceExist = FALSE;
    NeedAdjust = FALSE;
    do {
      //
      // Check every instance of the variable
      // First, check wheather the instance contain the partition node, which is needed for distinguishing  multi 
      // partial partition boot option. Second, check wheather the instance could be connected.
      //
      Instance  = EfiDevicePathInstance (&TempNewDevicePath, &Size);
      if (MatchPartitionDevicePathNode (Instance, HardDriveDevicePath)) {
        //
        // Connect the device path instance, the device path point to hard drive media device path node  
        // e.g. ACPI() /PCI()/ATA()/Partition()
        //
        Status = BdsLibConnectDevicePath (Instance);
        if (!EFI_ERROR (Status)) {
          DeviceExist = TRUE;
          break;
        }
      }
      //
      // Come here means the first instance is not matched
      //
      NeedAdjust = TRUE;
      EfiLibSafeFreePool(Instance);
    } while (TempNewDevicePath != NULL);
  
    if (DeviceExist) {
      //
      // Find the matched device path. 
      // Append the file path infomration from the boot option and return the fully expanded device path.
      //
      DevicePath    = NextDevicePathNode ((EFI_DEVICE_PATH_PROTOCOL *) HardDriveDevicePath);
      FullDevicePath = EfiAppendDevicePath (Instance, DevicePath);

      //
      // Adjust the 'HDDP' instances sequense if the matched one is not first one.
      //
      if (NeedAdjust) {
        //
        // First delete the matched instance.
        //
        TempNewDevicePath = CachedDevicePath;
        CachedDevicePath = BdsLibDelPartMatchInstance ( CachedDevicePath, Instance );
        EfiLibSafeFreePool (TempNewDevicePath); 
        //
        // Second, append the remaining parth after the matched instance
        //
        TempNewDevicePath = CachedDevicePath;
        CachedDevicePath = EfiAppendDevicePathInstance ( Instance, CachedDevicePath );
        EfiLibSafeFreePool (TempNewDevicePath);
        //
        // Save the matching Device Path so we don't need to do a connect all next time
        //
        Status = gRT->SetVariable (
                        L"HDDP",
                        &mHdBootVariablePrivateGuid,
                        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                        EfiDevicePathSize (CachedDevicePath),
                        CachedDevicePath
                        );
      }
      EfiLibSafeFreePool(Instance);
      gBS->FreePool (CachedDevicePath);
      return FullDevicePath;
    }
  }

  //
  // If we get here we fail to find or 'HDDP' not exist, and now we need
  // to seach all devices in the system for a matched partition
  //
  BdsLibConnectAllDriversToAllControllers ();
  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &BlockIoHandleCount, &BlockIoBuffer);
  if (EFI_ERROR (Status) || BlockIoHandleCount == 0) {
    //
    // If there was an error or there are no device handles that support
    // the BLOCK_IO Protocol, then return.
    //
    return NULL;
  }
  //
  // Loop through all the device handles that support the BLOCK_IO Protocol
  //
  for (Index = 0; Index < BlockIoHandleCount; Index++) {

    Status = gBS->HandleProtocol (BlockIoBuffer[Index], &gEfiDevicePathProtocolGuid, (VOID *) &BlockIoDevicePath);
    if (EFI_ERROR (Status) || BlockIoDevicePath == NULL) {
      continue;
    }

    if (MatchPartitionDevicePathNode (BlockIoDevicePath, HardDriveDevicePath)) {
      //
      // Find the matched partition device path
      //
      DevicePath    = NextDevicePathNode ((EFI_DEVICE_PATH_PROTOCOL *) HardDriveDevicePath);
      FullDevicePath = EfiAppendDevicePath (BlockIoDevicePath, DevicePath);

      //
      // Save the matched patition device path in 'HDDP' variable
      //
      if (CachedDevicePath != NULL) {
        //
        // Save the matched patition device path as first instance of 'HDDP' variable
        //        
        if (BdsLibMatchDevicePaths (CachedDevicePath, BlockIoDevicePath)) {
          TempNewDevicePath = CachedDevicePath;
          CachedDevicePath = BdsLibDelPartMatchInstance (CachedDevicePath, BlockIoDevicePath);
          EfiLibSafeFreePool(TempNewDevicePath); 

          TempNewDevicePath = CachedDevicePath;
          CachedDevicePath = EfiAppendDevicePathInstance (BlockIoDevicePath, CachedDevicePath);
          EfiLibSafeFreePool(TempNewDevicePath);
        } else {
          TempNewDevicePath = CachedDevicePath;
          CachedDevicePath = EfiAppendDevicePathInstance (BlockIoDevicePath, CachedDevicePath);
          EfiLibSafeFreePool(TempNewDevicePath);
        }
        //
        // Here limit the device path instance number to 12, which is max number for a system support 3 IDE controller
        // If the user try to boot many OS in different HDs or partitions, in theary, the 'HDDP' variable maybe become larger and larger.
        //
        InstanceNum = 0;
        TempNewDevicePath = CachedDevicePath;
        while (!IsDevicePathEnd (TempNewDevicePath)) {
          TempNewDevicePath = NextDevicePathNode (TempNewDevicePath);
          //
          // Parse one instance
          //
          while (!IsDevicePathEndType (TempNewDevicePath)) {
            TempNewDevicePath = NextDevicePathNode (TempNewDevicePath);
          }
          InstanceNum++;
          //
          // If the CachedDevicePath variable contain too much instance, only remain 12 instances.
          //
          if (InstanceNum >= 12) {
            SetDevicePathEndNode (TempNewDevicePath);
            break;
          }
        }
      } else {
        CachedDevicePath = EfiDuplicateDevicePath (BlockIoDevicePath);
      }
      
      //
      // Save the matching Device Path so we don't need to do a connect all next time
      //
      Status = gRT->SetVariable (
                      L"HDDP",
                      &mHdBootVariablePrivateGuid,
                      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                      EfiDevicePathSize (CachedDevicePath),
                      CachedDevicePath
                      );
                      
      break;
    }
  }
  gBS->FreePool (CachedDevicePath);
  gBS->FreePool (BlockIoBuffer);
  return FullDevicePath;
}

BOOLEAN
MatchPartitionDevicePathNode (
  IN  EFI_DEVICE_PATH_PROTOCOL   *BlockIoDevicePath,
  IN  HARDDRIVE_DEVICE_PATH      *HardDriveDevicePath
  )
/*++

Routine Description:
  Check whether there is a instance in BlockIoDevicePath, which contain multi device path 
  instances, has the same partition node with HardDriveDevicePath device path

Arguments:
  BlockIoDevicePath     - Multi device path instances which need to check
  HardDriveDevicePath - A device path which starts with a hard drive media device path.
  
Returns:
  TRUE  - There is a matched device path instance
  FALSE -There is no matched device path instance


--*/
{
  HARDDRIVE_DEVICE_PATH     *TmpHdPath;
  HARDDRIVE_DEVICE_PATH     *TempPath;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  BOOLEAN                   Match;
  EFI_DEVICE_PATH_PROTOCOL  *BlockIoHdDevicePathNode;
  
  if ((BlockIoDevicePath == NULL) || (HardDriveDevicePath == NULL)) {
    return FALSE;
  }
  //
  // Make PreviousDevicePath == the device path node before the end node
  //
  DevicePath          = BlockIoDevicePath;
  BlockIoHdDevicePathNode = NULL;

  //
  // find the partition device path node
  //
  while (!IsDevicePathEnd (DevicePath)) {
    if ((DevicePathType (DevicePath) == MEDIA_DEVICE_PATH) &&
        (DevicePathSubType (DevicePath) == MEDIA_HARDDRIVE_DP)
        ) {
      BlockIoHdDevicePathNode = DevicePath;
      break;
    }

    DevicePath = NextDevicePathNode (DevicePath);
  }

  if (BlockIoHdDevicePathNode == NULL) {
    return FALSE;
  }
  //
  // See if the harddrive device path in blockio matches the orig Hard Drive Node
  //
  TmpHdPath = (HARDDRIVE_DEVICE_PATH *) BlockIoHdDevicePathNode;
  TempPath  = (HARDDRIVE_DEVICE_PATH *) BdsLibUnpackDevicePath ((EFI_DEVICE_PATH_PROTOCOL *) HardDriveDevicePath);
  Match = FALSE;
  //
  // Check for the match
  //
  if ((TmpHdPath->MBRType == TempPath->MBRType) &&
      (TmpHdPath->SignatureType == TempPath->SignatureType)) {
    switch (TmpHdPath->SignatureType) {
    case SIGNATURE_TYPE_GUID:
      Match = EfiCompareGuid ((EFI_GUID *)TmpHdPath->Signature, (EFI_GUID *)TempPath->Signature);
      break;
    case SIGNATURE_TYPE_MBR:
      Match = (BOOLEAN)(*((UINT32 *)(&(TmpHdPath->Signature[0]))) == *(UINT32 *)(&(TempPath->Signature[0])));
      break;
    default:
      Match = FALSE;
      break;
    }
  }
  
  return Match;
}

EFI_STATUS
BdsLibDeleteOptionFromHandle (
  IN  EFI_HANDLE                 Handle
  )
/*++

Routine Description:

  Delete the boot option associated with the handle passed in

Arguments:

  Handle - The handle which present the device path to create boot option

Returns:

  EFI_SUCCESS           - Delete the boot option success

  EFI_NOT_FOUND         - If the Device Path is not found in the system

  EFI_OUT_OF_RESOURCES  - Lack of memory resource

  Other                 - Error return value from SetVariable()

--*/
{
  UINT16                    *BootOrder;
  UINT8                     *BootOptionVar;
  UINTN                     BootOrderSize;
  UINTN                     BootOptionSize;
  EFI_STATUS                Status;
  UINTN                     Index;
  UINT16                    BootOption[BOOT_OPTION_MAX_CHAR];
  UINTN                     DevicePathSize;
  UINTN                     OptionDevicePathSize;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *OptionDevicePath;
  UINT8                     *TempPtr;
  CHAR16                    *Description;

  Status        = EFI_SUCCESS;
  BootOrder     = NULL;
  BootOrderSize = 0;

  BootOrder = BdsLibGetVariableAndSize (
                L"BootOrder",
                &gEfiGlobalVariableGuid,
                &BootOrderSize
                );
  if (NULL == BootOrder) {
    return EFI_NOT_FOUND;
  }

  DevicePath = EfiDevicePathFromHandle (Handle);
  if (DevicePath == NULL) {
    return EFI_NOT_FOUND;
  }
  DevicePathSize = EfiDevicePathSize (DevicePath);

  Index = 0;
  while (Index < BootOrderSize / sizeof (UINT16)) {
    SPrint (BootOption, sizeof (BootOption), L"Boot%04x", BootOrder[Index]);
    BootOptionVar = BdsLibGetVariableAndSize (
                      BootOption,
                      &gEfiGlobalVariableGuid,
                      &BootOptionSize
                      );
    if (NULL == BootOptionVar) {
      gBS->FreePool (BootOrder);
      return EFI_OUT_OF_RESOURCES;
    }

    TempPtr = BootOptionVar;
    TempPtr += sizeof (UINT32) + sizeof (UINT16);
    Description = (CHAR16 *) TempPtr;
    TempPtr += EfiStrSize ((CHAR16 *) TempPtr);
    OptionDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) TempPtr;
    OptionDevicePathSize = EfiDevicePathSize (OptionDevicePath);

    //
    // Check whether the device path match
    //
    if ((OptionDevicePathSize == DevicePathSize) &&
        (EfiCompareMem (DevicePath, OptionDevicePath, DevicePathSize) == 0)) {
      BdsDeleteBootOption (BootOrder[Index], BootOrder, &BootOrderSize);
      gBS->FreePool (BootOptionVar);
      break;
    }

    gBS->FreePool (BootOptionVar);
    Index++;
  }

  Status = gRT->SetVariable (
                  L"BootOrder",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  BootOrderSize,
                  BootOrder
                  );

  gBS->FreePool (BootOrder);

  return Status;
}

EFI_STATUS
BdsDeleteAllInvalidEfiBootOption (
  VOID
  )
/*++

Routine Description:

  Delete all invalid EFI boot options. The probable invalid boot option could
  be Removable media or Network boot device.

Arguments:

  VOID

Returns:

  EFI_SUCCESS           - Delete all invalid boot option success

  EFI_NOT_FOUND         - Variable "BootOrder" is not found

  EFI_OUT_OF_RESOURCES  - Lack of memory resource

  Other                 - Error return value from SetVariable()

--*/
{
  UINT16                    *BootOrder;
  UINT8                     *BootOptionVar;
  UINTN                     BootOrderSize;
  UINTN                     BootOptionSize;
  EFI_STATUS                Status;
  UINTN                     Index;
  UINTN                     Index2;
  UINT16                    BootOption[BOOT_OPTION_MAX_CHAR];
  UINTN                     OptionDevicePathSize;
  EFI_DEVICE_PATH_PROTOCOL  *OptionDevicePath;
  UINT8                     *TempPtr;
  CHAR16                    *Description;

  Status        = EFI_SUCCESS;
  BootOrder     = NULL;
  BootOrderSize = 0;

  BootOrder = BdsLibGetVariableAndSize (
                L"BootOrder",
                &gEfiGlobalVariableGuid,
                &BootOrderSize
                );
  if (NULL == BootOrder) {
    return EFI_NOT_FOUND;
  }

  Index = 0;
  while (Index < BootOrderSize / sizeof (UINT16)) {
    SPrint (BootOption, sizeof (BootOption), L"Boot%04x", BootOrder[Index]);
    BootOptionVar = BdsLibGetVariableAndSize (
                      BootOption,
                      &gEfiGlobalVariableGuid,
                      &BootOptionSize
                      );
    if (NULL == BootOptionVar) {
      gBS->FreePool (BootOrder);
      return EFI_OUT_OF_RESOURCES;
    }

    TempPtr = BootOptionVar;
    TempPtr += sizeof (UINT32) + sizeof (UINT16);
    Description = (CHAR16 *) TempPtr;
    TempPtr += EfiStrSize ((CHAR16 *) TempPtr);
    OptionDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) TempPtr;
    OptionDevicePathSize = EfiDevicePathSize (OptionDevicePath);

    //
    // Skip legacy boot option (BBS boot device)
    //
    if ((DevicePathType (OptionDevicePath) == BBS_DEVICE_PATH) &&
        (DevicePathSubType (OptionDevicePath) == BBS_BBS_DP)) {
      gBS->FreePool (BootOptionVar);
      Index++;
      continue;
    }

    if (!BdsLibIsValidEFIBootOptDevicePath (OptionDevicePath, FALSE)) {
      //
      // Delete this invalid boot option "Boot####"
      //
      Status = gRT->SetVariable (
                      BootOption,
                      &gEfiGlobalVariableGuid,
                      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                      0,
                      NULL
                      );
      //
      // Mark this boot option in boot order as deleted
      //
      BootOrder[Index] = 0xffff;
    }

    gBS->FreePool (BootOptionVar);
    Index++;
  }

  //
  // Adjust boot order array
  //
  Index2 = 0;
  for (Index = 0; Index < BootOrderSize / sizeof (UINT16); Index++) {
    if (BootOrder[Index] != 0xffff) {
      BootOrder[Index2] = BootOrder[Index];
      Index2 ++;
    }
  }
  Status = gRT->SetVariable (
                  L"BootOrder",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  Index2 * sizeof (UINT16),
                  BootOrder
                  );

  gBS->FreePool (BootOrder);

  return Status;
}

EFI_STATUS
BdsLibEnumerateAllBootOption (
  IN OUT EFI_LIST_ENTRY      *BdsBootOptionList
  )
/*++

Routine Description:

  This function will enumerate all possible boot device in the system,
  it will only excute once of every boot.

Arguments:

  BdsBootOptionList - The header of the link list which indexed all
                      current boot options

Returns:

  EFI_SUCCESS - Finished all the boot device enumerate and create
                the boot option base on that boot device

--*/
{
  EFI_STATUS                    Status;
  UINT16                        FloppyNumber;
  UINT16                        CdromNumber;
  UINT16                        UsbNumber;
  UINT16                        ScsiNumber;
  UINT16                        MiscNumber;
  UINT16                        NonBlockNumber;
  UINTN                         NumberBlockIoHandles;
  EFI_HANDLE                    *BlockIoHandles;
  EFI_BLOCK_IO_PROTOCOL         *BlkIo;
  UINTN                         Index;
  UINTN                         NumberSimpleNetworkHandles;
  EFI_HANDLE                    *SimpleNetworkHandles;
  UINTN                         FvHandleCount;
  EFI_HANDLE                    *FvHandleBuffer;
  EFI_FV_FILETYPE               Type;
  UINTN                         Size;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  UINT32                        AuthenticationStatus;
#if (PI_SPECIFICATION_VERSION < 0x00010000)
  EFI_FIRMWARE_VOLUME_PROTOCOL  *Fv;
#else
  EFI_FIRMWARE_VOLUME2_PROTOCOL  *Fv;
#endif
  EFI_DEVICE_PATH_PROTOCOL     *DevicePath;
  UINTN                         DevicePathType;
  CHAR16                        Buffer[40];
  EFI_HANDLE                    *FileSystemHandles;
  UINTN                         NumberFileSystemHandles;
  BOOLEAN                       NeedDelete;
  EFI_IMAGE_OPTIONAL_HEADER     OptionalHeader;
  EFI_IMAGE_FILE_HEADER         ImageHeader;
  EFI_IMAGE_DOS_HEADER          DosHeader;
  
  FloppyNumber = 0;
  CdromNumber = 0;
  UsbNumber = 0;
  ScsiNumber = 0;
  MiscNumber = 0;
  EfiZeroMem (Buffer, sizeof (Buffer));
  //
  // If the boot device enumerate happened, just get the boot
  // device from the boot order variable
  //
  if (mEnumBootDevice) {
    BdsLibBuildOptionFromVar (BdsBootOptionList, L"BootOrder");
    return EFI_SUCCESS;
  }
  //
  // Notes: this dirty code is to get the legacy boot option from the
  // BBS table and create to variable as the EFI boot option, it should
  // be removed after the CSM can provide legacy boot option directly
  //
  REFRESH_LEGACY_BOOT_OPTIONS;

  //
  // Delete invalid boot option
  //
  BdsDeleteAllInvalidEfiBootOption ();
  //
  // Parse removable media
  //
  gBS->LocateHandleBuffer (
        ByProtocol,
        &gEfiBlockIoProtocolGuid,
        NULL,
        &NumberBlockIoHandles,
        &BlockIoHandles
        );
  for (Index = 0; Index < NumberBlockIoHandles; Index++) {
    Status = gBS->HandleProtocol (
                    BlockIoHandles[Index],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **) &BlkIo
                    );
    if (!EFI_ERROR (Status)) {
      if (!BlkIo->Media->RemovableMedia) {
        //
        // skip the non-removable block devices
        //
        continue;
      }
    }
    DevicePath  = EfiDevicePathFromHandle (BlockIoHandles[Index]);
    DevicePathType = BdsGetBootTypeFromDevicePath (DevicePath);
    
    switch (DevicePathType) {
    case BDS_EFI_ACPI_FLOPPY_BOOT:
      if (FloppyNumber == 0) {
        SPrint (Buffer, sizeof (Buffer), L"EFI Floppy");      
      } else {
        SPrint (Buffer, sizeof (Buffer), L"EFI Floppy %d", FloppyNumber);      
      }
      BdsLibBuildOptionFromHandle (BlockIoHandles[Index], BdsBootOptionList, Buffer);     
      FloppyNumber++; 
      break;
      
    case BDS_EFI_MESSAGE_ATAPI_BOOT:
      if (CdromNumber == 0) {
        SPrint (Buffer, sizeof (Buffer), L"EFI DVD/CDROM");      
      } else {
        SPrint (Buffer, sizeof (Buffer), L"EFI DVD/CDROM %d", CdromNumber);       
      }
      BdsLibBuildOptionFromHandle (BlockIoHandles[Index], BdsBootOptionList, Buffer);    
      CdromNumber++;
      break;
      
    case BDS_EFI_MESSAGE_USB_DEVICE_BOOT:
      if (UsbNumber == 0) {
        SPrint (Buffer, sizeof (Buffer), L"EFI USB Device");      
      } else {
        SPrint (Buffer, sizeof (Buffer), L"EFI USB Device %d", UsbNumber);       
      }  
      BdsLibBuildOptionFromHandle (BlockIoHandles[Index], BdsBootOptionList, Buffer);    
      UsbNumber++;
      break;

    case BDS_EFI_MESSAGE_SCSI_BOOT:
      if (UsbNumber == 0) {
        SPrint (Buffer, sizeof (Buffer), L"EFI SCSI Device");      
      } else {
        SPrint (Buffer, sizeof (Buffer), L"EFI SCSI Device %d", UsbNumber);       
      }  
      BdsLibBuildOptionFromHandle (BlockIoHandles[Index], BdsBootOptionList, Buffer);    
      UsbNumber++;
      break;
      
    case BDS_EFI_MESSAGE_MISC_BOOT:
      if (MiscNumber == 0) {
        SPrint (Buffer, sizeof (Buffer), L"EFI Misc Device");      
      } else {
        SPrint (Buffer, sizeof (Buffer), L"EFI Misc Device %d", MiscNumber);       
      }  
      BdsLibBuildOptionFromHandle (BlockIoHandles[Index], BdsBootOptionList, Buffer);    
      MiscNumber++;
      break;
      
    default:
      break;
    }   
  }
  
  if (NumberBlockIoHandles) {
    gBS->FreePool (BlockIoHandles);
  }
  
  //
  // If there is simple file protocol which does not consume block Io protocol, create a boot option for it here.
  //
  NonBlockNumber = 0;
  gBS->LocateHandleBuffer (
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &NumberFileSystemHandles,
        &FileSystemHandles
        );
  for (Index = 0; Index < NumberFileSystemHandles; Index++) {
    Status = gBS->HandleProtocol (
                    FileSystemHandles[Index],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **) &BlkIo
                    );
     if (!EFI_ERROR (Status)) {
      //
      //  Skip if the file system handle supports a BlkIo protocol,
      //
      continue;
    } 

    //
    // Do the removable Media thing. \EFI\BOOT\boot{machinename}.EFI
    //  machinename is ia32, ia64, x64, ...
    //
    NeedDelete = TRUE;
    Status     = BdsLibGetImageHeader (
                   FileSystemHandles[Index],
                   DEFAULT_REMOVABLE_FILE_NAME,
                   &DosHeader,
                   &ImageHeader,
                   &OptionalHeader
                   );
    if (!EFI_ERROR (Status) &&
        EFI_IMAGE_MACHINE_TYPE_SUPPORTED (ImageHeader.Machine) &&
        OptionalHeader.Subsystem == EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION) {
      NeedDelete = FALSE;
    }

    if (NeedDelete) {
      //
      // No such file or the file is not a EFI application, delete this boot option
      //
      BdsLibDeleteOptionFromHandle (FileSystemHandles[Index]);
    } else {
      if (NonBlockNumber == 0) {
        SPrint (Buffer, sizeof (Buffer), L"EFI Non-Block Boot Device");  
      } else {
        SPrint (Buffer, sizeof (Buffer), L"EFI Non-Block Boot Device %d", NonBlockNumber);       
      }  
      BdsLibBuildOptionFromHandle (FileSystemHandles[Index], BdsBootOptionList, Buffer);
      NonBlockNumber++;
    }
  }
  
  if (NumberFileSystemHandles) {
    gBS->FreePool (FileSystemHandles);
  }
  
  //
  // Parse Network Boot Device
  //
  gBS->LocateHandleBuffer (
        ByProtocol,
        &gEfiSimpleNetworkProtocolGuid,
        NULL,
        &NumberSimpleNetworkHandles,
        &SimpleNetworkHandles
        );
  for (Index = 0; Index < NumberSimpleNetworkHandles; Index++) {
    if (Index == 0) {
      SPrint (Buffer, sizeof (Buffer), L"EFI Network");    
    } else {
      SPrint (Buffer, sizeof (Buffer), L"EFI Network %d", Index);      
    }
    BdsLibBuildOptionFromHandle (SimpleNetworkHandles[Index], BdsBootOptionList, Buffer);
  }

  if (NumberSimpleNetworkHandles) {
    gBS->FreePool (SimpleNetworkHandles);
  }

  //
  // Check if we have on flash shell
  //
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
    //
    // Only care the dispatched FV. If no dispatch protocol on the FV, it is not dispatched, then skip it.
    //
    Status = gBS->HandleProtocol (
                    FvHandleBuffer[Index],
                    &gEfiFirmwareVolumeDispatchProtocolGuid,
                    &Fv
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }
    
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
                  &gEfiShellFileGuid,
                  NULL,
                  &Size,
                  &Type,
                  &Attributes,
                  &AuthenticationStatus
                  );
    if (EFI_ERROR (Status)) {
      //
      // Skip if no shell file in the FV
      //
      continue;
    }
    //
    // Build the shell boot option
    //
    BdsLibBuildOptionFromShell (FvHandleBuffer[Index], BdsBootOptionList);
  }

  if (FvHandleCount) {
    gBS->FreePool (FvHandleBuffer);
  }
  //
  // Make sure every boot only have one time
  // boot device enumerate
  //
  BdsLibBuildOptionFromVar (BdsBootOptionList, L"BootOrder");
  mEnumBootDevice = TRUE;

  return EFI_SUCCESS;
}

VOID
BdsLibBuildOptionFromHandle (
  IN  EFI_HANDLE                 Handle,
  IN  EFI_LIST_ENTRY             *BdsBootOptionList,
  IN  CHAR16                     *String
  )
/*++

Routine Description:

  Build the boot option with the handle parsed in

Arguments:

  Handle - The handle which present the device path to create boot option

  BdsBootOptionList - The header of the link list which indexed all current
                      boot options

Returns:

  VOID

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  DevicePath  = EfiDevicePathFromHandle (Handle);

  //
  // Create and register new boot option
  //
  BdsLibRegisterNewOption (BdsBootOptionList, DevicePath, String, L"BootOrder");
}

VOID
BdsLibBuildOptionFromShell (
  IN EFI_HANDLE                  Handle,
  IN OUT EFI_LIST_ENTRY          *BdsBootOptionList
  )
/*++

Routine Description:

  Build the on flash shell boot option with the handle parsed in

Arguments:

  Handle - The handle which present the device path to create on flash shell
           boot option

  BdsBootOptionList - The header of the link list which indexed all current
                      boot options

Returns:

  None

--*/
{
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH ShellNode;

  DevicePath = EfiDevicePathFromHandle (Handle);

  //
  // Build the shell device path
  //
  EfiInitializeFwVolDevicepathNode (&ShellNode, &gEfiShellFileGuid);
  //
  //ShellNode.Header.Type     = MEDIA_DEVICE_PATH;
  //ShellNode.Header.SubType  = MEDIA_FV_FILEPATH_DP;
  //SetDevicePathNodeLength (&ShellNode.Header, sizeof (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH));
  //EfiCopyMem (&ShellNode.NameGuid, &gEfiShellFileGuid, sizeof (EFI_GUID));
  //
  DevicePath = EfiAppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *) &ShellNode);

  //
  // Create and register the shell boot option
  //
  BdsLibRegisterNewOption (BdsBootOptionList, DevicePath, L"EFI Internal Shell", L"BootOrder");

}

VOID
BdsLibBootNext (
  VOID
  )
/*++

Routine Description:

  Boot from the EFI1.1 spec defined "BootNext" variable

Arguments:

  None

Returns:

  None

--*/
{
  UINT16            *BootNext;
  UINTN             BootNextSize;
  CHAR16            Buffer[20];
  BDS_COMMON_OPTION *BootOption;
  EFI_LIST_ENTRY    TempList;
  UINTN             ExitDataSize;
  CHAR16            *ExitData;

  //
  // Init the boot option name buffer and temp link list
  //
  InitializeListHead (&TempList);
  EfiZeroMem (Buffer, sizeof (Buffer));

  BootNext = BdsLibGetVariableAndSize (
              L"BootNext",
              &gEfiGlobalVariableGuid,
              &BootNextSize
              );

  //
  // Clear the boot next variable first
  //
  if (BootNext != NULL) {
    gRT->SetVariable (
          L"BootNext",
          &gEfiGlobalVariableGuid,
          EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
          0,
          BootNext
          );

    //
    // Start to build the boot option and try to boot
    //
    SPrint (Buffer, sizeof (Buffer), L"Boot%04x", *BootNext);
    BootOption = BdsLibVariableToOption (&TempList, Buffer);
    BdsLibConnectDevicePath (BootOption->DevicePath);
    BdsLibBootViaBootOption (BootOption, BootOption->DevicePath, &ExitDataSize, &ExitData);
  }

}


EFI_HANDLE
BdsLibGetBootableHandle (
  IN  EFI_DEVICE_PATH_PROTOCOL      *DevicePath
  )
/*++

Routine Description:
  Return the bootable media handle.
  First, check the device is connected
  Second, check whether the device path point to a device which support SimpleFileSystemProtocol,
  Third, detect the the default boot file in the Media, and return the removable Media handle.

Arguments:
  DevicePath - Device Path to a  bootable device

Returns:
  If not NULL - The device path points to an EFI bootable Media
  NULL - The media on the DevicePath is not bootable

--*/
{
  EFI_STATUS                      Status;
  EFI_DEVICE_PATH_PROTOCOL        *UpdatedDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        *DupDevicePath;
  EFI_HANDLE                      Handle;
  EFI_BLOCK_IO_PROTOCOL           *BlockIo;
  VOID                            *Buffer;
  EFI_DEVICE_PATH_PROTOCOL        *TempDevicePath;  
  UINTN                           Size;
  UINTN                           TempSize; 
  EFI_HANDLE                      ReturnHandle;
  EFI_HANDLE                      *SimpleFileSystemHandles;
  
  UINTN                           NumberSimpleFileSystemHandles;
  UINTN                           Index;
  EFI_IMAGE_DOS_HEADER            DosHeader;
  EFI_IMAGE_FILE_HEADER           ImageHeader;
  EFI_IMAGE_OPTIONAL_HEADER       OptionalHeader;
  
  UpdatedDevicePath = DevicePath;
  //
  // Check whether the device is connected
  //
  Status = gBS->LocateDevicePath (&gEfiBlockIoProtocolGuid, &UpdatedDevicePath, &Handle);
  if (EFI_ERROR (Status)) {
    //
    // Skip the case that the boot option point to a simple file protocol which does not consume block Io protocol, 
    //
    Status = gBS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid, &UpdatedDevicePath, &Handle);
    if (EFI_ERROR (Status)) {
      //
      // Fail to find the proper BlockIo and simple file protocol, maybe because device not present,  we need to connect it firstly
      //
      UpdatedDevicePath = DevicePath;
      Status = gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &UpdatedDevicePath, &Handle);
      gBS->ConnectController (Handle, NULL, NULL, TRUE);    
    }
  } else {
    //
    // For removable device boot option, its contained device path only point to the removable device handle, 
    // should make sure all its children handles (its child partion or media handles) are created and connected. 
    //
    gBS->ConnectController (Handle, NULL, NULL, TRUE); 
    //
    // Get BlockIo protocal and check removable attribute
    //
    Status = gBS->HandleProtocol (Handle, &gEfiBlockIoProtocolGuid, (VOID **)&BlockIo); 
    //
    // Issue a dummy read to the device to check for media change.
    // When the removable media is changed, any Block IO read/write will
    // cause the BlockIo protocol be reinstalled and EFI_MEDIA_CHANGED is
    // returned. After the Block IO protocol is reinstalled, subsequent
    // Block IO read/write will success.
    //
    Buffer = EfiLibAllocatePool (BlockIo->Media->BlockSize);
    if (Buffer != NULL) {
      BlockIo->ReadBlocks (
               BlockIo,
               BlockIo->Media->MediaId,
               0,
               BlockIo->Media->BlockSize,
               Buffer
               );
      gBS->FreePool (Buffer);      
    }
  }
  
  //
  // Detect the the default boot file from removable Media
  //

  //
  // If fail to get bootable handle specified by a USB boot option, the BDS should try to find other bootable device in the same USB bus
  // Try to locate the USB node device path first, if fail then use its previour PCI node to search
  //
  DupDevicePath = EfiDuplicateDevicePath (DevicePath);
  UpdatedDevicePath = DupDevicePath;
  Status = gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &UpdatedDevicePath, &Handle);
  //
  // if the resulting device path point to a usb node, and the usb node is a dummy node, should only let device path only point to the previous Pci node
  // Acpi()/Pci()/Usb() --> Acpi()/Pci()
  //
  if ((DevicePathType (UpdatedDevicePath) == MESSAGING_DEVICE_PATH) &&
      (DevicePathSubType (UpdatedDevicePath) == MSG_USB_DP)) {
    //
    // Remove the usb node, let the device path only point to PCI node
    //
    SetDevicePathEndNode (UpdatedDevicePath);
    UpdatedDevicePath = DupDevicePath;
  } else {
    UpdatedDevicePath = DevicePath;
  }
  
  //
  // Get the device path size of boot option 
  //
  Size = EfiDevicePathSize(UpdatedDevicePath) - sizeof (EFI_DEVICE_PATH_PROTOCOL); // minus the end node 
  ReturnHandle = NULL;
  gBS->LocateHandleBuffer (
      ByProtocol,
      &gEfiSimpleFileSystemProtocolGuid,
      NULL,
      &NumberSimpleFileSystemHandles,
      &SimpleFileSystemHandles
      );
  for (Index = 0; Index < NumberSimpleFileSystemHandles; Index++) {
    //
    // Get the device path size of SimpleFileSystem handle
    //
    TempDevicePath = EfiDevicePathFromHandle (SimpleFileSystemHandles[Index]);  
    TempSize = EfiDevicePathSize (TempDevicePath)- sizeof (EFI_DEVICE_PATH_PROTOCOL); // minus the end node 
    //
    // Check whether the device path of boot option is part of the  SimpleFileSystem handle's device path
    //
    if (Size <= TempSize && EfiCompareMem (TempDevicePath, UpdatedDevicePath, Size)==0) {
      //
      // Load the default boot file \EFI\BOOT\boot{machinename}.EFI from removable Media
      //  machinename is ia32, ia64, x64, ...
      //
      Status = BdsLibGetImageHeader (
                 SimpleFileSystemHandles[Index],
                 DEFAULT_REMOVABLE_FILE_NAME,
                 &DosHeader,
                 &ImageHeader,
                 &OptionalHeader
                 );
      if (!EFI_ERROR (Status) &&
        EFI_IMAGE_MACHINE_TYPE_SUPPORTED (ImageHeader.Machine) &&
        OptionalHeader.Subsystem == EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION) {
        ReturnHandle = SimpleFileSystemHandles[Index];
        break;
      }
    }
  }
  
  if (DupDevicePath != NULL) {
    EfiLibSafeFreePool(DupDevicePath);  
  }
  if (SimpleFileSystemHandles !=NULL ) {
    gBS->FreePool (SimpleFileSystemHandles);  
  }

  return ReturnHandle;
}



BOOLEAN
BdsLibNetworkBootWithMediaPresent (
  IN  EFI_DEVICE_PATH_PROTOCOL      *DevicePath
  )
/*++

Routine Description:
  Check to see if the network cable is plugged in. If the DevicePath is not 
  connected it will be connected.

Arguments:
  DevicePath - Device Path to check

Returns:
  TRUE - DevicePath points to an Network that is connected 
  FALSE - DevicePath does not point to a bootable network

--*/
{
  EFI_STATUS                      Status;
  EFI_DEVICE_PATH_PROTOCOL        *UpdatedDevicePath;
  EFI_HANDLE                      Handle;
  EFI_SIMPLE_NETWORK_PROTOCOL     *Snp;
  BOOLEAN                         MediaPresent;

  MediaPresent = FALSE;

  UpdatedDevicePath = DevicePath;
  Status = gBS->LocateDevicePath (&gEfiSimpleNetworkProtocolGuid, &UpdatedDevicePath, &Handle);
  if (EFI_ERROR (Status)) {
    //
    // Device not present so see if we need to connect it
    //
    Status = BdsLibConnectDevicePath (DevicePath);
    if (!EFI_ERROR (Status)) {
      //
      // This one should work after we did the connect
      //
      Status = gBS->LocateDevicePath (&gEfiSimpleNetworkProtocolGuid, &UpdatedDevicePath, &Handle);
    }
  }

  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (Handle, &gEfiSimpleNetworkProtocolGuid, (VOID **)&Snp);
    if (!EFI_ERROR (Status)) {
      if (Snp->Mode->MediaPresentSupported) {
        if (Snp->Mode->State == EfiSimpleNetworkInitialized) {
          //
          // In case some one else is using the SNP check to see if it's connected
          //
          MediaPresent = Snp->Mode->MediaPresent;
        } else {
          //
          // No one is using SNP so we need to Start and Initialize so 
          // MediaPresent will be valid.
          //
          Status = Snp->Start (Snp);
          if (!EFI_ERROR (Status)) {
            Status = Snp->Initialize (Snp, 0, 0);
            if (!EFI_ERROR (Status)) {
              MediaPresent = Snp->Mode->MediaPresent;
              Snp->Shutdown (Snp);
            }
            Snp->Stop (Snp);
          }
        }
      } else {
        MediaPresent = TRUE;
      }
    }
  }

  return MediaPresent;
}


UINT32
BdsGetBootTypeFromDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  )
/*++

Routine Description:
  For a bootable Device path, return its boot type
  
Arguments:
  DevicePath - The bootable device Path to check

Returns:
  UINT32 Boot type :
  //
  // If the device path contains any media deviec path node, it is media boot type
  // For the floppy node, handle it as media node
  //
  BDS_EFI_MEDIA_HD_BOOT
  BDS_EFI_MEDIA_CDROM_BOOT
  BDS_EFI_ACPI_FLOPPY_BOOT
  //
  // If the device path not contains any media deviec path node,  and 
  // its last device path node point to a message device path node, it is 
  // a message boot type
  //
  BDS_EFI_MESSAGE_ATAPI_BOOT 
  BDS_EFI_MESSAGE_SCSI_BOOT
  BDS_EFI_MESSAGE_USB_DEVICE_BOOT
  BDS_EFI_MESSAGE_MISC_BOOT  
  //
  // Legacy boot type
  //
  BDS_LEGACY_BBS_BOOT
  //
  // If a EFI Removable BlockIO device path not point to a media and message devie,
  // it is unsupported
  //
  BDS_EFI_UNSUPPORT
--*/
{
  ACPI_HID_DEVICE_PATH          *Acpi;
  EFI_DEVICE_PATH_PROTOCOL      *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL      *LastDeviceNode;

  
  if (NULL == DevicePath) {
    return BDS_EFI_UNSUPPORT;
  }
  
  TempDevicePath = DevicePath;

  while (!IsDevicePathEndType (TempDevicePath)) {
    switch (DevicePathType (TempDevicePath)) {
      case BBS_DEVICE_PATH:
         return BDS_LEGACY_BBS_BOOT; 
      case MEDIA_DEVICE_PATH:
        if (DevicePathSubType (TempDevicePath) == MEDIA_HARDDRIVE_DP) {
          return BDS_EFI_MEDIA_HD_BOOT;         
        } else if (DevicePathSubType (TempDevicePath) == MEDIA_CDROM_DP) {
          return BDS_EFI_MEDIA_CDROM_BOOT;           
        } 
        break; 
      case ACPI_DEVICE_PATH:
        Acpi = (ACPI_HID_DEVICE_PATH *) TempDevicePath;
        if (EISA_ID_TO_NUM (Acpi->HID) == 0x0604) {
          return BDS_EFI_ACPI_FLOPPY_BOOT;
        }
        break;
      case MESSAGING_DEVICE_PATH:
        //
        // if the device path not only point to driver device, it is not a messaging device path.
        //
        LastDeviceNode = NextDevicePathNode (TempDevicePath);
        if (!IsDevicePathEndType (LastDeviceNode)) {
          break;        
        }

        if (DevicePathSubType(TempDevicePath) == MSG_ATAPI_DP) {
          return BDS_EFI_MESSAGE_ATAPI_BOOT;          
        } else if (DevicePathSubType(TempDevicePath) == MSG_USB_DP) {
          return BDS_EFI_MESSAGE_USB_DEVICE_BOOT;          
        } else if (DevicePathSubType(TempDevicePath) == MSG_SCSI_DP) {
          return BDS_EFI_MESSAGE_SCSI_BOOT;       
        }
        return BDS_EFI_MESSAGE_MISC_BOOT; 
      default:
        break;
    }
    TempDevicePath = NextDevicePathNode (TempDevicePath);
  }

  return BDS_EFI_UNSUPPORT;
}

BOOLEAN
BdsLibIsValidEFIBootOptDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevPath,
  IN BOOLEAN                      CheckMedia
  )
/*++

Routine Description:
 
  Check whether the Device path in a boot option point to a valide bootable device,
  And if CheckMedia is true, check the device is ready to boot now.

Arguments:

  DevPath -- the Device path in a boot option
  CheckMedia -- if true, check the device is ready to boot now.
  
Returns:

  TRUE      -- the Device path  is valide
   FALSE   -- the Device path  is invalide .
 
--*/
{
  EFI_STATUS                Status;
  EFI_HANDLE                Handle;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *LastDeviceNode;
  EFI_BLOCK_IO_PROTOCOL     *BlockIo;
  
  TempDevicePath = DevPath;  
  LastDeviceNode = DevPath;
  //
  // Check if it's a valid boot option for network boot device
  // Only check if there is SimpleNetworkProtocol installed. If yes, that means
  // there is the network card there.
  //
  Status = gBS->LocateDevicePath (
                  &gEfiSimpleNetworkProtocolGuid,
                  &TempDevicePath,
                  &Handle
                  );
  if (EFI_ERROR (Status)) {
    //
    // Device not present so see if we need to connect it
    //
    TempDevicePath = DevPath; 
    BdsLibConnectDevicePath (TempDevicePath);
    Status = gBS->LocateDevicePath (
                    &gEfiSimpleNetworkProtocolGuid,
                    &TempDevicePath,
                    &Handle
                    );
  }
  if (!EFI_ERROR (Status)) {
    if (CheckMedia) {
      //
      // Test if it is ready to boot now
      //
      if (BdsLibNetworkBootWithMediaPresent(DevPath)) {
        return TRUE;
      }
    } else {
      return TRUE;    
    }
  }
  
  //
  // If the boot option point to a file, it is a valid EFI boot option,
  // and assume it is ready to boot now
  //
  while (!EfiIsDevicePathEnd (TempDevicePath)) {
     LastDeviceNode = TempDevicePath;
     TempDevicePath = EfiNextDevicePathNode (TempDevicePath);
  }
  if ((DevicePathType (LastDeviceNode) == MEDIA_DEVICE_PATH) &&
    (DevicePathSubType (LastDeviceNode) == MEDIA_FILEPATH_DP)) {
    return TRUE;
  }  
  
  //
  // Check if it's a valid boot option for internal Shell
  //
  if (EfiGetNameGuidFromFwVolDevicePathNode ((MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *) LastDeviceNode) != NULL) {
    //
    // If the boot option point to Internal FV shell, make sure it is valid
    //
    TempDevicePath = DevPath; 
    Status = BdsLibUpdateFvFileDevicePath (&TempDevicePath, &gEfiShellFileGuid);
    if (Status == EFI_ALREADY_STARTED) {
      return TRUE;
    } else {
      if (Status == EFI_SUCCESS) {
        gBS->FreePool (TempDevicePath); 
      }
      return FALSE;
    }
  }
  
  //
  // If the boot option point to a blockIO device, no matter whether or not it is a removeable device, it is a valid EFI boot option
  //
  TempDevicePath = DevPath;  
  Status = gBS->LocateDevicePath (&gEfiBlockIoProtocolGuid, &TempDevicePath, &Handle);
  if (EFI_ERROR (Status)) {
    //
    // Device not present so see if we need to connect it
    //
    Status = BdsLibConnectDevicePath (DevPath);
    if (!EFI_ERROR (Status)) {
      //
      // Try again to get the Block Io protocol after we did the connect
      //
      TempDevicePath = DevPath; 
      Status = gBS->LocateDevicePath (&gEfiBlockIoProtocolGuid, &TempDevicePath, &Handle);
    }
  }
  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (Handle, &gEfiBlockIoProtocolGuid, (VOID **)&BlockIo); 
    if (!EFI_ERROR (Status)) {
      if (CheckMedia) {
        //
        // Test if it is ready to boot now
        //
        if (BdsLibGetBootableHandle (DevPath) != NULL) {
          return TRUE;
        }
      } else {
        return TRUE;    
      }
    }
  } else {
    //
    // if the boot option point to a simple file protocol which does not consume block Io protocol, it is also a valid EFI boot option,
    //
    Status = gBS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid, &TempDevicePath, &Handle);
    if (!EFI_ERROR (Status)) {
      if (CheckMedia) {
        //
        // Test if it is ready to boot now
        //
        if (BdsLibGetBootableHandle (DevPath) != NULL) {
          return TRUE;
        }
      } else {
        return TRUE;    
      }
    }
  }

  return FALSE;
}

EFI_STATUS
EFIAPI
BdsLibUpdateFvFileDevicePath (
  IN  OUT EFI_DEVICE_PATH_PROTOCOL      ** DevicePath,
  IN  EFI_GUID                          *FileGuid
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
  
  if ((DevicePath == NULL) || (*DevicePath == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  if (FileGuid == NULL) {
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
  if (!EfiCompareGuid (GuidPoint, FileGuid)) {
    //
    // If the Fv file is not the input file guid, just return EFI_UNSUPPORTED
    //
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
  // First, try to look for in Bds own FV. Bds and input wanted FV file usually are in the same FV
  //
  FindFvFile = FALSE;
  FoundFvHandle = NULL;
  Status = gBS->HandleProtocol (
             mBdsImageHandle,
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
    FvHandleBuffer = NULL;
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
    
    if (FvHandleBuffer !=NULL ) {
      gBS->FreePool (FvHandleBuffer);  
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
