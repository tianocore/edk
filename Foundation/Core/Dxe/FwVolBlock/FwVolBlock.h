/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  FwVolBlock.c

Abstract:

  Firmware Volume Block protocol..  Consumes FV hobs and creates
  appropriate block protocols.

  Also consumes NT_NON_MM_FV envinronment variable and produces appropriate
  block protocols fro them also... (this is TBD)

--*/

#ifndef _FWVOL_BLOCK_H_
#define _FWVOL_BLOCK_H_

#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeBlock)
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include "Peihob.h"
#include "EfiHobLib.h"
#include "DxeCore.h"

#include EFI_GUID_DEFINITION (Hob)



#define FVB_DEVICE_SIGNATURE       EFI_SIGNATURE_32('_','F','V','B')

typedef struct {
  UINTN                       Base;
  UINTN                       Length;
} LBA_CACHE;

typedef struct {
  MEMMAP_DEVICE_PATH          MemMapDevPath;
  EFI_DEVICE_PATH_PROTOCOL    EndDevPath;
} FV_DEVICE_PATH;


typedef struct {
  UINTN                                 Signature;
  EFI_HANDLE                            Handle;
  FV_DEVICE_PATH                        DevicePath;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL    FwVolBlockInstance;
  UINTN                                 NumBlocks;
  LBA_CACHE                             *LbaCache;
  UINT32                                FvbAttributes;
  EFI_PHYSICAL_ADDRESS                  BaseAddress;
} EFI_FW_VOL_BLOCK_DEVICE;

#define FVB_DEVICE_FROM_THIS(a) \
  CR(a, EFI_FW_VOL_BLOCK_DEVICE, FwVolBlockInstance, FVB_DEVICE_SIGNATURE)



EFI_STATUS
FwVolBlockDriverInit (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );


EFI_STATUS
EFIAPI
FwVolBlockGetAttributes (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL   *This,
  OUT EFI_FVB_ATTRIBUTES                          *Attributes
  );


EFI_STATUS
EFIAPI
FwVolBlockSetAttributes (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL   *This,
  OUT EFI_FVB_ATTRIBUTES                          *Attributes
  );


EFI_STATUS
EFIAPI
FwVolBlockEraseBlock (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL    *This,
  ...
  );


EFI_STATUS
EFIAPI
FwVolBlockReadBlock (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL   *This,
  IN EFI_LBA                                      Lba,
  IN UINTN                                        Offset,
  IN OUT UINTN                                    *NumBytes,
  IN UINT8                                        *Buffer
  );

  
EFI_STATUS
EFIAPI
FwVolBlockWriteBlock (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL    *This,
  IN EFI_LBA                              Lba,
  IN UINTN                                Offset,
  IN OUT UINTN                            *NumBytes,
  IN UINT8                                *Buffer
  );

    
EFI_STATUS
EFIAPI
FwVolBlockGetPhysicalAddress (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  *This,
  OUT EFI_PHYSICAL_ADDRESS                        *Address
  );


EFI_STATUS
EFIAPI
FwVolBlockGetBlockSize (
  IN EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL   *This,
  IN EFI_LBA                             Lba,
  OUT UINTN                              *BlockSize,
  OUT UINTN                              *NumberOfBlocks
  );

EFI_STATUS
FwVolBlockDriverInit (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
ProduceFVBProtocolOnBuffer (
  IN EFI_PHYSICAL_ADDRESS   BaseAddress,
  IN UINT64                 Length,
  IN EFI_HANDLE             ParentHandle,
  OUT EFI_HANDLE            *FvProtocolHandle  OPTIONAL
  );

#endif
