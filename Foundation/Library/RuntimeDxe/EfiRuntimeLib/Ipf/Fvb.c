/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    Fvb.c

Abstract:

  Light weight lib to support EFI 2.0 Firmware Volume Block 
  protocol abstraction at runtime.

  All these functions convert EFI_EXTENDED_SAL_FV_BLOCK_SERVICES_PROTOCOL_GUID
  class function to the Runtime Lib function. There is a 1 to 1 mapping.

  If you are using any of these lib functions.you must first call FvbInitialize ().

--*/

#include "Tiano.h"
#include "EfiRuntimeLib.h"
#include EFI_PROTOCOL_DEFINITION (ExtendedSalGuid)
#include "SalApi.h"


EFI_STATUS
EfiFvbInitialize (
  VOID
  )
/*++

Routine Description:
  Initialize globals and register Fvb Protocol notification function.

Arguments:
  None

Returns: 
  EFI_SUCCESS

--*/
{
  return EFI_SUCCESS;
}


//
// The following functions wrap Fvb protocol in the Runtime Lib functions.
// The Instance translates into Fvb instance. The Fvb order defined by HOBs and
// thus the sequence of FVB protocol addition define Instance.
//
// EfiFvbInitialize () must be called before any of the following functions
// must be called.
//


EFI_STATUS
EfiFvbReadBlock (
  IN UINTN                                        Instance,
  IN EFI_LBA                                      Lba,
  IN UINTN                                        Offset,
  IN OUT UINTN                                    *NumBytes,
  IN UINT8                                        *Buffer
  )
{
  EFI_GUID Guid = EFI_EXTENDED_SAL_FV_BLOCK_SERVICES_PROTOCOL_GUID;

  return EfiCallEsalService (&Guid, Read, Instance, Lba , Offset, (UINT64) NumBytes, (UINT64) Buffer, 0, 0).Status;
}

EFI_STATUS
EfiFvbWriteBlock (
  IN UINTN                                        Instance,
  IN EFI_LBA                                      Lba,
  IN UINTN                                        Offset,
  IN OUT UINTN                                    *NumBytes,
  IN UINT8                                        *Buffer
  )
{
  EFI_GUID Guid = EFI_EXTENDED_SAL_FV_BLOCK_SERVICES_PROTOCOL_GUID;

  return EfiCallEsalService (&Guid, Write, Instance, Lba , Offset, (UINT64) NumBytes, (UINT64) Buffer, 0, 0).Status;
}

EFI_STATUS
EfiFvbEraseBlock (
  IN UINTN                                Instance,
  IN UINTN                                Lba
  )
{
  EFI_GUID Guid = EFI_EXTENDED_SAL_FV_BLOCK_SERVICES_PROTOCOL_GUID;

  return EfiCallEsalService (&Guid, EraseBlock, Instance, Lba , 0, 0, 0, 0, 0).Status;
}

EFI_STATUS
EfiFvbGetVolumeAttributes (
  IN UINTN                                Instance,
  OUT EFI_FVB_ATTRIBUTES                  *Attributes
  )
{
  EFI_GUID Guid = EFI_EXTENDED_SAL_FV_BLOCK_SERVICES_PROTOCOL_GUID;

  return EfiCallEsalService (&Guid, SetVolumeAttributes, Instance, (UINT64)Attributes , 0, 0, 0, 0, 0).Status;
}

EFI_STATUS
EfiFvbSetVolumeAttributes (
  IN UINTN                                Instance,
  IN EFI_FVB_ATTRIBUTES                   Attributes
  )
{
  EFI_GUID Guid = EFI_EXTENDED_SAL_FV_BLOCK_SERVICES_PROTOCOL_GUID;

  return EfiCallEsalService (&Guid, SetVolumeAttributes, Instance, (UINT64)Attributes , 0, 0, 0, 0, 0).Status;
}

EFI_STATUS
EfiFvbGetPhysicalAddress (
  IN UINTN                                Instance,
  OUT EFI_PHYSICAL_ADDRESS                *BaseAddress  
  )
{
  EFI_GUID Guid = EFI_EXTENDED_SAL_FV_BLOCK_SERVICES_PROTOCOL_GUID;

  return EfiCallEsalService (&Guid, GetPhysicalAddress, Instance, (UINT64) BaseAddress , 0, 0, 0, 0, 0).Status;
}

EFI_STATUS
EfiFvbGetBlockSize (
  IN UINTN                                Instance,
  IN EFI_LBA                              Lba,
  OUT UINTN                               *BlockSize,
  OUT UINTN                               *NumOfBlocks
  )
{
  EFI_GUID Guid = EFI_EXTENDED_SAL_FV_BLOCK_SERVICES_PROTOCOL_GUID;

  return EfiCallEsalService (&Guid, GetBlockSize, Instance, Lba, (UINT64) BlockSize, (UINT64) NumOfBlocks, 0, 0, 0).Status;
}

EFI_STATUS
EfiFvbEraseCustomBlockRange (
  IN UINTN                                Instance,
  IN EFI_LBA                              StartLba,
  IN UINTN                                OffsetStartLba,
  IN EFI_LBA                              LastLba,
  IN UINTN                                OffsetLastLba
  )
{
  EFI_GUID Guid = EFI_EXTENDED_SAL_FV_BLOCK_SERVICES_PROTOCOL_GUID;

  return EfiCallEsalService (&Guid, EraseCustomBlockRange, Instance, StartLba , OffsetStartLba, LastLba, OffsetLastLba, 0, 0).Status;
}







