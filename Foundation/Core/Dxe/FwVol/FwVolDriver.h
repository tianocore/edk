/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  FwVolDriver.h

Abstract:

  Firmware File System protocol. Layers on top of Firmware
  Block protocol to produce a file abstraction of FV based files.

--*/

#ifndef __FWVOL_H
#define __FWVOL_H

#include "Tiano.h"
#include "DxeCore.h"
#include "EfiFirmwareFileSystem.h"
#include "EfiFirmwareVolumeHeader.h"

//
// Consumed protocol
//
#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeBlock)
#include EFI_PROTOCOL_DEFINITION (SectionExtraction)

//
// Consumed GUID
//
#include EFI_GUID_DEFINITION (FirmwareFileSystem)

//
// Produced protocol
//
#include EFI_PROTOCOL_DEFINITION (FirmwareVolume)


//
// Used to track all non-deleted files
//
typedef struct {
  EFI_LIST_ENTRY                  Link;
  EFI_FFS_FILE_HEADER             *FfsHeader;
  UINTN                           StreamHandle;
  EFI_SECTION_EXTRACTION_PROTOCOL *Sep;
} FFS_FILE_LIST_ENTRY;

typedef struct {
  UINTN                                   Signature;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL      *Fvb;
  EFI_HANDLE                              Handle;
  EFI_FIRMWARE_VOLUME_PROTOCOL            Fv;

  EFI_FIRMWARE_VOLUME_HEADER              *FwVolHeader;
  UINT8                                   *CachedFv;
  UINT8                                   *EndOfCachedFv;

  FFS_FILE_LIST_ENTRY                     *LastKey;

  EFI_LIST_ENTRY                          FfsFileListHeader;

  UINT8                                   ErasePolarity;
} FV_DEVICE;

#define FV_DEVICE_FROM_THIS(a) CR(a, FV_DEVICE, Fv, FV_DEVICE_SIGNATURE)


EFI_STATUS
EFIAPI
FvGetVolumeAttributes (
  IN    EFI_FIRMWARE_VOLUME_PROTOCOL   *This,
  OUT   EFI_FV_ATTRIBUTES              *Attributes
  );

EFI_STATUS
EFIAPI
FvSetVolumeAttributes (
  IN     EFI_FIRMWARE_VOLUME_PROTOCOL   *This,
  IN OUT EFI_FV_ATTRIBUTES              *Attributes
  );

EFI_STATUS
EFIAPI
FvGetNextFile (
  IN     EFI_FIRMWARE_VOLUME_PROTOCOL   *This,
  IN OUT VOID                           *Key,
  IN OUT EFI_FV_FILETYPE                *FileType,
  OUT    EFI_GUID                       *NameGuid,
  OUT    EFI_FV_FILE_ATTRIBUTES         *Attributes,
  OUT    UINTN                          *Size
  );


EFI_STATUS
EFIAPI
FvReadFile (
  IN     EFI_FIRMWARE_VOLUME_PROTOCOL   *This,
  IN     EFI_GUID                       *NameGuid,
  IN OUT VOID                           **Buffer,
  IN OUT UINTN                          *BufferSize,
  OUT    EFI_FV_FILETYPE                *FoundType,
  OUT    EFI_FV_FILE_ATTRIBUTES         *FileAttributes,
  OUT    UINT32                         *AuthenticationStatus
  );

EFI_STATUS
EFIAPI
FvReadFileSection (
  IN     EFI_FIRMWARE_VOLUME_PROTOCOL   *This,
  IN     EFI_GUID                       *NameGuid,
  IN     EFI_SECTION_TYPE               SectionType,
  IN     UINTN                          SectionInstance,
  IN OUT VOID                           **Buffer,
  IN OUT UINTN                          *BufferSize,
  OUT    UINT32                         *AuthenticationStatus
  );

EFI_STATUS
EFIAPI
FvWriteFile (
  IN EFI_FIRMWARE_VOLUME_PROTOCOL       *This,
  IN UINT32                             NumberOfFiles,
  IN EFI_FV_WRITE_POLICY                WritePolicy,
  IN EFI_FV_WRITE_FILE_DATA             *FileData
  );


  
//
//Internal functions
//
typedef enum {
  EfiCheckSumUint8    = 0,
  EfiCheckSumUint16   = 1,
  EfiCheckSumUint32   = 2,
  EfiCheckSumUint64   = 3,
  EfiCheckSumMaximum  = 4
} EFI_CHECKSUM_TYPE;


BOOLEAN
IsBufferErased (
  IN UINT8    ErasePolarity,
  IN VOID     *Buffer,
  IN UINTN    BufferSize
  );

EFI_FFS_FILE_STATE 
GetFileState (
  IN UINT8                ErasePolarity,
  IN EFI_FFS_FILE_HEADER  *FfsHeader
  );

VOID
SetFileState (
  IN UINT8                State,
  IN EFI_FFS_FILE_HEADER  *FfsHeader
  );

BOOLEAN
VerifyFvHeaderChecksum (
  IN EFI_FIRMWARE_VOLUME_HEADER *FvHeader
  );
    
BOOLEAN
IsValidFfsHeader (
  IN  UINT8                ErasePolarity,
  IN  EFI_FFS_FILE_HEADER  *FfsHeader,
  OUT EFI_FFS_FILE_STATE   *FileState
  );

BOOLEAN
IsValidFfsFile (
  IN UINT8                ErasePolarity,
  IN EFI_FFS_FILE_HEADER  *FfsHeader
  );

EFI_STATUS
EFIAPI
GetFwVolHeader (
  IN  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL      *Fvb,
  OUT EFI_FIRMWARE_VOLUME_HEADER              **FwVolHeader
  );


EFI_STATUS
FvCheck (
  IN OUT FV_DEVICE  *FvDevice
  );

#endif