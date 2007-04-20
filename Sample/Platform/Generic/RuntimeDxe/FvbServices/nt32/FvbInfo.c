/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  FvbInfo.c

Abstract:

  Defines data structure that is the volume header found.These data is intent
  to decouple FVB driver with FV header.

--*/

#include "Tiano.h"
#include "FlashLayout.h"
#include "EfiRuntimeLib.h"
#include EFI_GUID_DEFINITION (SystemNvDataGuid)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeBlock)
#include EFI_GUID_DEFINITION (FirmwareFileSystem)
#include EFI_GUID_DEFINITION (FirmwareFileSystem2)

#define FIRMWARE_BLOCK_SIZE 0x10000

typedef struct {
  UINT64                      FvLength;
  EFI_FIRMWARE_VOLUME_HEADER  FvbInfo;
  //
  // EFI_FV_BLOCK_MAP_ENTRY    ExtraBlockMap[n];//n=0
  //
  EFI_FV_BLOCK_MAP_ENTRY      End[1];
} EFI_FVB_MEDIA_INFO;

#define FVB_MEDIA_BLOCK_SIZE    FIRMWARE_BLOCK_SIZE
#define RECOVERY_BOIS_BLOCK_NUM FIRMWARE_BLOCK_NUMBER
#define SYSTEM_NV_BLOCK_NUM     2

EFI_FVB_MEDIA_INFO  mPlatformFvbMediaInfo[] = {
  //
  // Recovery BOIS FVB
  //
  {
    EFI_WINNT_FIRMWARE_LENGTH,
#if (PI_SPECIFICATION_VERSION < 0x00010000)
    {
      {
        0,
      },  // ZeroVector[16]
      EFI_FIRMWARE_FILE_SYSTEM_GUID,
      FVB_MEDIA_BLOCK_SIZE * RECOVERY_BOIS_BLOCK_NUM,
      EFI_FVH_SIGNATURE,
      EFI_FVB_READ_ENABLED_CAP |
        EFI_FVB_READ_STATUS |
        EFI_FVB_WRITE_ENABLED_CAP |
        EFI_FVB_WRITE_STATUS |
        EFI_FVB_ERASE_POLARITY,
      sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
      0,  // CheckSum
      {
        0,
      },  // Reserved[3]
      1,  // Revision
      {
        RECOVERY_BOIS_BLOCK_NUM,
        FVB_MEDIA_BLOCK_SIZE,
      }
    },
#else
    {
      {
        0,
      },  // ZeroVector[16]
      EFI_FIRMWARE_FILE_SYSTEM2_GUID,
      FVB_MEDIA_BLOCK_SIZE * RECOVERY_BOIS_BLOCK_NUM,
      EFI_FVH_SIGNATURE,
      EFI_FVB_READ_ENABLED_CAP |
        EFI_FVB_READ_STATUS |
        EFI_FVB_WRITE_ENABLED_CAP |
        EFI_FVB_WRITE_STATUS |
        EFI_FVB_ERASE_POLARITY,
      sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
      0,    //CheckSum
      0,    // ExtHeaderOffset
      {0}, //Reserved[1]
      2,    //Revision
      {
        RECOVERY_BOIS_BLOCK_NUM,
        FVB_MEDIA_BLOCK_SIZE,
      }
    },
#endif
    {
      0,
      0
    }
  },
  //
  // Systen NvStorage FVB
  //
  {
    EFI_WINNT_RUNTIME_UPDATABLE_LENGTH + EFI_WINNT_FTW_SPARE_BLOCK_LENGTH,
#if (PI_SPECIFICATION_VERSION < 0x00010000)
    {
      {
        0,
      },  // ZeroVector[16]
      EFI_SYSTEM_NV_DATA_HOB_GUID,
      FVB_MEDIA_BLOCK_SIZE * SYSTEM_NV_BLOCK_NUM,
      EFI_FVH_SIGNATURE,
      EFI_FVB_READ_ENABLED_CAP |
        EFI_FVB_READ_STATUS |
        EFI_FVB_WRITE_ENABLED_CAP |
        EFI_FVB_WRITE_STATUS |
        EFI_FVB_ERASE_POLARITY,
      sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
      0,  // CheckSum
      {
        0,
      },  // Reserved[3]
      1,  // Revision
      {
        SYSTEM_NV_BLOCK_NUM,
        FVB_MEDIA_BLOCK_SIZE,
      }
    },
#else
    {
      {
        0,
      },  // ZeroVector[16]
      EFI_SYSTEM_NV_DATA_HOB_GUID,
      FVB_MEDIA_BLOCK_SIZE * SYSTEM_NV_BLOCK_NUM,
      EFI_FVH_SIGNATURE,
      EFI_FVB_READ_ENABLED_CAP |
        EFI_FVB_READ_STATUS |
        EFI_FVB_WRITE_ENABLED_CAP |
        EFI_FVB_WRITE_STATUS |
        EFI_FVB_ERASE_POLARITY,
      sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
      0,    //CheckSum
      0,    // ExtHeaderOffset
      {0}, //Reserved[1]
      2,    //Revision
      {
        SYSTEM_NV_BLOCK_NUM,
        FVB_MEDIA_BLOCK_SIZE,
      }
    },
#endif
    {
      0,
      0
    }
  }
};

EFI_STATUS
GetFvbInfo (
  IN  UINT64                        FvLength,
  OUT EFI_FIRMWARE_VOLUME_HEADER    **FvbInfo
  )
{
  UINTN Index;

  for (Index = 0; Index < sizeof (mPlatformFvbMediaInfo) / sizeof (EFI_FVB_MEDIA_INFO); Index += 1) {
    if (mPlatformFvbMediaInfo[Index].FvLength == FvLength) {
      *FvbInfo = &mPlatformFvbMediaInfo[Index].FvbInfo;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}
