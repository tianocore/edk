/*++

Copyright (c) 2004 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciOptionRomSupport.c
  
Abstract:

  PCI Bus Driver

Revision History

--*/

#include "PciBus.h"
#include "PciResourceSupport.h"

EFI_STATUS
LocalLoadFile2 (
  IN PCI_IO_DEVICE            *PciIoDevice,
  IN EFI_DEVICE_PATH_PROTOCOL *FilePath,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer      OPTIONAL
  )
/*++

Routine Description:

  Load the EFI Image from Option ROM

Arguments:
  
  PciIoDevice - PCI IO Device
  FilePath    - The file path of the EFI Image
  BufferSize  - On input the size of Buffer in bytes. On output with a return 
                code of EFI_SUCCESS, the amount of data transferred to Buffer. 
                On output with a return code of EFI_BUFFER_TOO_SMALL, 
                the size of Buffer required to retrieve the requested file. 
  Buffer      - The memory buffer to transfer the file to. If Buffer is NULL, 
                then no the size of the requested file is returned in BufferSize.


Returns:

  EFI_SUCCESS           - The file was loaded. 
  EFI_UNSUPPORTED       - BootPolicy is TRUE.
  EFI_BUFFER_TOO_SMALL  - The BufferSize is too small to read the current directory entry.
                          BufferSize has been updated with the size needed to complete the request.
--*/

{
  EFI_STATUS                                Status;
  MEDIA_RELATIVE_OFFSET_RANGE_DEVICE_PATH   *EfiOpRomImageNode;
  EFI_PCI_EXPANSION_ROM_HEADER              *EfiRomHeader;
  PCI_DATA_STRUCTURE                        *Pcir;
  UINT32                                    ImageSize;
  UINT8                                     *ImageBuffer;
  UINT32                                    ImageLength;
  UINT32                                    DestinationSize;
  UINT32                                    ScratchSize;
  VOID                                      *Scratch;
  EFI_DECOMPRESS_PROTOCOL                   *Decompress;

  EfiOpRomImageNode = (MEDIA_RELATIVE_OFFSET_RANGE_DEVICE_PATH *) FilePath;
  if ((EfiOpRomImageNode == NULL) ||
      (DevicePathType (FilePath) != MEDIA_DEVICE_PATH) ||
      (DevicePathSubType (FilePath) != MEDIA_RELATIVE_OFFSET_RANGE_DP) ||
      (DevicePathNodeLength (FilePath) != sizeof (MEDIA_RELATIVE_OFFSET_RANGE_DEVICE_PATH)) ||
      (!IsDevicePathEnd (NextDevicePathNode (FilePath))) ||
      (EfiOpRomImageNode->StartingOffset > EfiOpRomImageNode->EndingOffset) ||
      (EfiOpRomImageNode->EndingOffset >= PciIoDevice->RomSize) ||
      (BufferSize == NULL)
      ) {
    return EFI_INVALID_PARAMETER;
  }
  
  EfiRomHeader = (EFI_PCI_EXPANSION_ROM_HEADER *) (
      (UINT8 *) PciIoDevice->PciIo.RomImage + EfiOpRomImageNode->StartingOffset
      );
  if (EfiRomHeader->Signature != PCI_EXPANSION_ROM_HEADER_SIGNATURE) {
    return EFI_NOT_FOUND;
  }

  
  Pcir = (PCI_DATA_STRUCTURE *) ((UINT8 *) EfiRomHeader + EfiRomHeader->PcirOffset);

  
  if ((Pcir->CodeType == PCI_CODE_TYPE_EFI_IMAGE) && 
      (EfiRomHeader->EfiSignature == EFI_PCI_EXPANSION_ROM_HEADER_EFISIGNATURE) &&
      ((EfiRomHeader->EfiSubsystem == EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER) ||
       (EfiRomHeader->EfiSubsystem == EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER)) &&
      (EfiRomHeader->CompressionType <= EFI_PCI_EXPANSION_ROM_HEADER_COMPRESSED)
       ) {

    ImageSize               = (UINT32) EfiRomHeader->InitializationSize * 512;
    
    ImageBuffer             = (UINT8 *) EfiRomHeader + EfiRomHeader->EfiImageHeaderOffset;
    ImageLength             = ImageSize - EfiRomHeader->EfiImageHeaderOffset;

    if (EfiRomHeader->CompressionType != EFI_PCI_EXPANSION_ROM_HEADER_COMPRESSED) {
      //
      // Uncompressed: Copy the EFI Image directly to user's buffer
      //
      if (Buffer == NULL || *BufferSize < ImageLength) {
        *BufferSize = ImageLength;
        return EFI_BUFFER_TOO_SMALL;
      }

      *BufferSize = ImageLength;
      EfiCopyMem (Buffer, ImageBuffer, ImageLength);
      return EFI_SUCCESS;

    } else {
      //
      // Compressed: Uncompress before copying
      //
      Status = gBS->LocateProtocol (&gEfiDecompressProtocolGuid, NULL, &Decompress);
      if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
      }
      Status = Decompress->GetInfo (
                             Decompress,
                             ImageBuffer,
                             ImageLength,
                             &DestinationSize,
                             &ScratchSize
                             );
      if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
      } 
      
      if (Buffer == NULL || *BufferSize < DestinationSize) {
        *BufferSize = DestinationSize;
        return EFI_BUFFER_TOO_SMALL;
      }      

      *BufferSize = DestinationSize;
      Scratch = EfiLibAllocatePool (ScratchSize);
      if (Scratch == NULL) {
        return EFI_DEVICE_ERROR;
      }
      
      Status = Decompress->Decompress (
                             Decompress,
                             ImageBuffer,
                             ImageLength,
                             Buffer,
                             DestinationSize,
                             Scratch,
                             ScratchSize
                             );
      gBS->FreePool (Scratch);
      
      if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
      }
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
VOID
InitializePciLoadFile2 (
  PCI_IO_DEVICE       *PciIoDevice
  )
/*++

Routine Description:

  Initialize a PCI LoadFile2 instance

Arguments:
  
  PciIoDevice - PCI IO Device

Returns:
  None

--*/
{
  PciIoDevice->LoadFile2.LoadFile = LoadFile2;
}

EFI_STATUS
EFIAPI
LoadFile2 (
  IN EFI_LOAD_FILE2_PROTOCOL  *This,
  IN EFI_DEVICE_PATH_PROTOCOL *FilePath,
  IN BOOLEAN                  BootPolicy,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer      OPTIONAL
  )
/*++

Routine Description:
  Causes the driver to load a specified file.

Arguments:
  
  This       - Indicates a pointer to the calling context.
  FilePath   - The device specific path of the file to load.
  BootPolicy - Should always be FALSE.
  BufferSize - On input the size of Buffer in bytes. On output with a return 
               code of EFI_SUCCESS, the amount of data transferred to Buffer. 
               On output with a return code of EFI_BUFFER_TOO_SMALL, 
               the size of Buffer required to retrieve the requested file. 
  Buffer     - The memory buffer to transfer the file to. If Buffer is NULL, 
               then no the size of the requested file is returned in BufferSize.

Returns:
  
--*/
{
  PCI_IO_DEVICE                             *PciIoDevice;

  if (BootPolicy) {
    return EFI_UNSUPPORTED;
  }
  PciIoDevice = PCI_IO_DEVICE_FROM_LOAD_FILE2_THIS (This);

  return LocalLoadFile2 (
           PciIoDevice,
           FilePath,
           BufferSize,
           Buffer
           );
}
#endif

EFI_STATUS
GetOpRomInfo (
  IN PCI_IO_DEVICE    *PciIoDevice
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
// TODO:    PciIoDevice - add argument and description to function comment
// TODO:    EFI_NOT_FOUND - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  UINT8                           RomBarIndex;
  UINT32                          AllOnes;
  UINT64                          Address;
  EFI_STATUS                      Status;
  UINT8                           Bus;
  UINT8                           Device;
  UINT8                           Function;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;

  Bus             = PciIoDevice->BusNumber;
  Device          = PciIoDevice->DeviceNumber;
  Function        = PciIoDevice->FunctionNumber;

  PciRootBridgeIo = PciIoDevice->PciRootBridgeIo;

  //
  // offset is 0x30 if is not ppb
  //

  //
  // 0x30
  //
  RomBarIndex = PCI_DEVICE_ROMBAR;

  if (IS_PCI_BRIDGE (&PciIoDevice->Pci)) {
    //
    // if is ppb
    //

    //
    // 0x38
    //
    RomBarIndex = PCI_BRIDGE_ROMBAR;
  }
  //
  // the bit0 is 0 to prevent the enabling of the Rom address decoder
  //
  AllOnes = 0xfffffffe;
  Address = EFI_PCI_ADDRESS (Bus, Device, Function, RomBarIndex);

  Status = PciRootBridgeIo->Pci.Write (
                                  PciRootBridgeIo,
                                  EfiPciWidthUint32,
                                  Address,
                                  1,
                                  &AllOnes
                                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // read back
  //
  Status = PciRootBridgeIo->Pci.Read (
                                  PciRootBridgeIo,
                                  EfiPciWidthUint32,
                                  Address,
                                  1,
                                  &AllOnes
                                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Bits [1, 10] are reserved
  //
  AllOnes &= 0xFFFFF800;
  if ((AllOnes == 0) || (AllOnes == 0xFFFFF800)) {
    return EFI_NOT_FOUND;
  }

  PciIoDevice->RomSize = (UINT64) ((~AllOnes) + 1);
  return EFI_SUCCESS;
}

BOOLEAN
ContainEfiImage (
  IN VOID            *RomImage,
  IN UINT64          RomSize
  )  
/*++

Routine Description:
  Check if the RomImage contains EFI Images.

--*/
{
  PCI_EXPANSION_ROM_HEADER  *RomHeader;
  PCI_DATA_STRUCTURE        *RomPcir;
  BOOLEAN                   FirstCheck;

  FirstCheck = TRUE;
  RomHeader  = RomImage;
  
  while ((UINT8 *) RomHeader < (UINT8 *) RomImage + RomSize) {
    if (RomHeader->Signature != PCI_EXPANSION_ROM_HEADER_SIGNATURE) {
      if (FirstCheck) {
        return FALSE;
      } else {
        RomHeader = (PCI_EXPANSION_ROM_HEADER *) ((UINT8 *) RomHeader + 512);
        continue;
      }
    }

    FirstCheck = FALSE;
    RomPcir    = (PCI_DATA_STRUCTURE *) ((UINT8 *) RomHeader + RomHeader->PcirOffset);
    
    if (RomPcir->CodeType == PCI_CODE_TYPE_EFI_IMAGE) {
      return TRUE;
    }

    RomHeader = (PCI_EXPANSION_ROM_HEADER *) ((UINT8 *) RomHeader + RomPcir->Length * 512);
  }

  return FALSE;
}

EFI_STATUS
LoadOpRomImage (
  IN PCI_IO_DEVICE   *PciDevice,
  IN UINT64          RomBase
  )
/*++

Routine Description:
 
    Load option rom image for specified PCI device

Arguments:

Returns:

--*/
// TODO:    PciDevice - add argument and description to function comment
// TODO:    RomBase - add argument and description to function comment
// TODO:    EFI_OUT_OF_RESOURCES - add return value to function comment
// TODO:    EFI_OUT_OF_RESOURCES - add return value to function comment
// TODO:    EFI_OUT_OF_RESOURCES - add return value to function comment
{
  UINT8                     RomBarIndex;
  UINT8                     Indicator;
  UINT16                    OffsetPcir;
  UINT32                    RomBarOffset;
  UINT32                    RomBar;
  UINT64                    Temp;
  EFI_STATUS                retStatus;
  BOOLEAN                   FirstCheck;
  UINT8                     *Image;
  PCI_EXPANSION_ROM_HEADER  *RomHeader;
  PCI_DATA_STRUCTURE        *RomPcir;
  UINT64                    RomSize;
  UINT64                    RomImageSize;
  UINT8                     *RomInMemory;
  UINT8                     CodeType;

  RomSize       = PciDevice->RomSize;

  Indicator     = 0;
  RomImageSize  = 0;
  RomInMemory   = NULL;
  Temp          = 0;
  CodeType      = 0xFF;

  //
  // Get the RomBarIndex
  //

  //
  // 0x30
  //
  RomBarIndex = PCI_DEVICE_ROMBAR;
  if (IS_PCI_BRIDGE (&(PciDevice->Pci))) {
    //
    // if is ppb
    //

    //
    // 0x38
    //
    RomBarIndex = PCI_BRIDGE_ROMBAR;
  }
  //
  // Allocate memory for Rom header and PCIR
  //
  RomHeader = EfiLibAllocatePool (sizeof (PCI_EXPANSION_ROM_HEADER));
  if (RomHeader == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  RomPcir = EfiLibAllocatePool (sizeof (PCI_DATA_STRUCTURE));
  if (RomPcir == NULL) {
    gBS->FreePool (RomHeader);
    return EFI_OUT_OF_RESOURCES;
  }

  RomBar = (UINT32) RomBase;

  //
  // Enable RomBar
  //
  RomDecode (PciDevice, RomBarIndex, RomBar, TRUE);

  RomBarOffset  = RomBar;
  retStatus     = EFI_NOT_FOUND;
  FirstCheck    = TRUE;

  do {
    PciDevice->PciRootBridgeIo->Mem.Read (
                                      PciDevice->PciRootBridgeIo,
                                      EfiPciWidthUint8,
                                      RomBarOffset,
                                      sizeof (PCI_EXPANSION_ROM_HEADER),
                                      (UINT8 *) RomHeader
                                      );

    if (RomHeader->Signature != PCI_EXPANSION_ROM_HEADER_SIGNATURE) {
      RomBarOffset = RomBarOffset + 512;
      if (FirstCheck) {
        break;
      } else {
        RomImageSize = RomImageSize + 512;
        continue;
      }
    }

    FirstCheck  = FALSE;
    OffsetPcir  = RomHeader->PcirOffset;
    PciDevice->PciRootBridgeIo->Mem.Read (
                                      PciDevice->PciRootBridgeIo,
                                      EfiPciWidthUint8,
                                      RomBarOffset + OffsetPcir,
                                      sizeof (PCI_DATA_STRUCTURE),
                                      (UINT8 *) RomPcir
                                      );
    if (RomPcir->CodeType == PCI_CODE_TYPE_PCAT_IMAGE) {
      CodeType = PCI_CODE_TYPE_PCAT_IMAGE;
    }
    Indicator     = RomPcir->Indicator;
    RomImageSize  = RomImageSize + RomPcir->ImageLength * 512;
    RomBarOffset  = RomBarOffset + RomPcir->ImageLength * 512;
  } while (((Indicator & 0x80) == 0x00) && ((RomBarOffset - RomBar) < RomSize));

  //
  // Some Legacy Cards do not report the correct ImageLength so used the maximum
  // of the legacy length and the PCIR Image Length
  //
  if (CodeType == PCI_CODE_TYPE_PCAT_IMAGE) {
    RomImageSize = EFI_MAX(RomImageSize, (((EFI_LEGACY_EXPANSION_ROM_HEADER *)RomHeader)->Size512 * 512));
  }

  if (RomImageSize > 0) {
    retStatus = EFI_SUCCESS;
    Image     = EfiLibAllocatePool ((UINT32) RomImageSize);
    if (Image == NULL) {
      RomDecode (PciDevice, RomBarIndex, RomBar, FALSE);
      gBS->FreePool (RomHeader);
      gBS->FreePool (RomPcir);
      return EFI_OUT_OF_RESOURCES;
    }
    
    //
    // Copy Rom image into memory
    //
    PciDevice->PciRootBridgeIo->Mem.Read (
                                      PciDevice->PciRootBridgeIo,
                                      EfiPciWidthUint8,
                                      RomBar,
                                      (UINT32) RomImageSize,
                                      Image
                                      );
    RomInMemory = Image;
  }

  RomDecode (PciDevice, RomBarIndex, RomBar, FALSE);

  PciDevice->PciIo.RomSize  = RomImageSize;
  PciDevice->PciIo.RomImage = RomInMemory;

  //
  // For OpROM read from PCI device:
  //   Add the Rom Image to internal database for later PCI light enumeration
  //
  PciRomAddImageMapping (
    NULL,
    PciDevice->PciRootBridgeIo->SegmentNumber,
    PciDevice->BusNumber,
    PciDevice->DeviceNumber,
    PciDevice->FunctionNumber,
    (UINT64) (UINTN) PciDevice->PciIo.RomImage,
    PciDevice->PciIo.RomSize
    );

  //
  // Free allocated memory
  //
  gBS->FreePool (RomHeader);
  gBS->FreePool (RomPcir);

  return retStatus;
}

EFI_STATUS
RomDecode (
  IN PCI_IO_DEVICE   *PciDevice,
  IN UINT8           RomBarIndex,
  IN UINT32          RomBar,
  IN BOOLEAN         Enable
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
// TODO:    PciDevice - add argument and description to function comment
// TODO:    RomBarIndex - add argument and description to function comment
// TODO:    RomBar - add argument and description to function comment
// TODO:    Enable - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  UINT32              Value32;
  UINT32              Offset;
  EFI_PCI_IO_PROTOCOL *PciIo;

  PciIo = &PciDevice->PciIo;
  if (Enable) {
    //
    // Clear all bars
    //
    for (Offset = 0x10; Offset <= 0x24; Offset += sizeof (UINT32)) {
      PciIo->Pci.Write (PciIo, EfiPciIoWidthUint32, Offset, 1, &gAllZero);
    }
    
    //
    // set the Rom base address: now is hardcode
    // enable its decoder
    //
    Value32 = RomBar | 0x1;
    PciIo->Pci.Write (
                PciIo,
                EfiPciWidthUint32,
                RomBarIndex,
                1,
                &Value32
                );

    //
    // Programe all upstream bridge
    //
    ProgrameUpstreamBridgeForRom(PciDevice, RomBar, TRUE);

    //
    // Setting the memory space bit in the function's command register
    //
    PciEnableCommandRegister(PciDevice, EFI_PCI_COMMAND_MEMORY_SPACE);

  } else {
    
    //
    // disable command register decode to memory
    //
    PciDisableCommandRegister(PciDevice, EFI_PCI_COMMAND_MEMORY_SPACE);

    //
    // Destroy the programmed bar in all the upstream bridge.
    //
    ProgrameUpstreamBridgeForRom(PciDevice, RomBar, FALSE);

    //
    // disable rom decode
    //
    Value32 = 0xFFFFFFFE;
    PciIo->Pci.Write (
                PciIo,
                EfiPciWidthUint32,
                RomBarIndex,
                1,
                &Value32
                );

  }

  return EFI_SUCCESS;

}

EFI_STATUS
ProcessOpRomImage (
  PCI_IO_DEVICE   *PciDevice
  )
/*++

Routine Description:

  Process the oprom image.
  
Arguments:
  PciDevice       A pointer to a pci device.

Returns:

  EFI Status.
  
--*/
{
  UINT8                         Indicator;
  UINT32                        ImageSize;
  VOID                          *RomBar;
  UINT8                         *RomBarOffset;
  EFI_HANDLE                    ImageHandle;
  EFI_STATUS                    Status;
  EFI_STATUS                    retStatus;
  BOOLEAN                       FirstCheck;
  EFI_PCI_EXPANSION_ROM_HEADER  *EfiRomHeader;
  PCI_DATA_STRUCTURE            *Pcir;
  EFI_DEVICE_PATH_PROTOCOL      *EfiOpRomImageDevicePath;
#if (EFI_SPECIFICATION_VERSION < 0x0002000A)
  EFI_DEVICE_PATH_PROTOCOL      *EfiOpRomImageFilePath;
#endif
  MEDIA_RELATIVE_OFFSET_RANGE_DEVICE_PATH 
                                EfiOpRomImageNode;
  VOID                          *Buffer;
  UINTN                         BufferSize;
  
  Indicator = 0;
  //
  // Get the Address of the Rom image
  //
  RomBar        = PciDevice->PciIo.RomImage;
  RomBarOffset  = (UINT8 *) RomBar;
  retStatus     = EFI_NOT_FOUND;
  FirstCheck    = TRUE;

  do {
    EfiRomHeader = (EFI_PCI_EXPANSION_ROM_HEADER *) RomBarOffset;
    if (EfiRomHeader->Signature != PCI_EXPANSION_ROM_HEADER_SIGNATURE) {
      RomBarOffset += 512;
      if (FirstCheck) {
        break;
      } else {
        continue;
      }
    }

    FirstCheck  = FALSE;
    Pcir        = (PCI_DATA_STRUCTURE *) (RomBarOffset + EfiRomHeader->PcirOffset);
    ImageSize   = (UINT32) (Pcir->ImageLength * 512);
    Indicator   = Pcir->Indicator;

    //
    // Create Pci Option Rom Image device path header
    //
    EfiOpRomImageNode.Header.Type     = MEDIA_DEVICE_PATH;
    EfiOpRomImageNode.Header.SubType  = MEDIA_RELATIVE_OFFSET_RANGE_DP;
    SetDevicePathNodeLength (&EfiOpRomImageNode.Header, sizeof (EfiOpRomImageNode));
    EfiOpRomImageNode.StartingOffset  = (UINTN) RomBarOffset - (UINTN) RomBar;
    EfiOpRomImageNode.EndingOffset    = (UINTN) RomBarOffset + ImageSize - 1 - (UINTN) RomBar;

    EfiOpRomImageDevicePath = EfiAppendDevicePathNode (PciDevice->DevicePath, &EfiOpRomImageNode.Header);
    ASSERT (EfiOpRomImageDevicePath != NULL);

    //
    // load image and start image
    //

    BufferSize  = 0;
    Buffer      = NULL;
    Status      = EFI_SUCCESS;
    ImageHandle = NULL;
#if (EFI_SPECIFICATION_VERSION < 0x0002000A)
    //
    // Let EfiOpRomImageFilePath point to the start of Relative Offset Range.
    //
    EfiOpRomImageFilePath = (EFI_DEVICE_PATH_PROTOCOL *) (
                              (UINT8 *) EfiOpRomImageDevicePath + 
                              EfiDevicePathSize (PciDevice->DevicePath) - 
                              sizeof (EFI_DEVICE_PATH_PROTOCOL)
                              );
    
    //
    // read image to buffer firstly before call LoadImage 
    //   because pre-UEFI 2.10 core doesn't support LoadFile2.
    //
    Status = LocalLoadFile2 (
               PciDevice,
               EfiOpRomImageFilePath,
               &BufferSize,
               Buffer
               );
    if (Status == EFI_BUFFER_TOO_SMALL) {
      Buffer = EfiLibAllocatePool (BufferSize);
      ASSERT (Buffer != NULL);
      Status = LocalLoadFile2 (
                 PciDevice,
                 EfiOpRomImageFilePath,
                 &BufferSize,
                 Buffer
                 );
    }
#endif

    if (!EFI_ERROR (Status)) {
      Status = gBS->LoadImage (
                      FALSE,
                      gPciBusDriverBinding.DriverBindingHandle,
                      EfiOpRomImageDevicePath,
                      Buffer,
                      BufferSize,
                      &ImageHandle
                      );
    }

    gBS->FreePool (EfiOpRomImageDevicePath);

    if (!EFI_ERROR (Status)) {
      Status = gBS->StartImage (ImageHandle, NULL, NULL);
      if (!EFI_ERROR (Status)) {
        AddDriver (PciDevice, ImageHandle);
        PciRomAddImageMapping (
          ImageHandle,
          PciDevice->PciRootBridgeIo->SegmentNumber,
          PciDevice->BusNumber,
          PciDevice->DeviceNumber,
          PciDevice->FunctionNumber,
          (UINT64) (UINTN) PciDevice->PciIo.RomImage,
          PciDevice->PciIo.RomSize
          );
        retStatus = EFI_SUCCESS;
      }
    }

    RomBarOffset += ImageSize;
  } while (((Indicator & 0x80) == 0x00) && ((UINTN) (RomBarOffset - (UINT8 *) RomBar) < PciDevice->RomSize));

  return retStatus;

}
