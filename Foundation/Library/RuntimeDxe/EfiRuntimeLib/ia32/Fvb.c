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

  Firmware Volume Block Protocol Runtime Abstraction

  mFvbEntry is an array of Handle Fvb pairs. The Fvb Lib Instance matches the 
  index in the mFvbEntry array. This should be the same sequence as the FVB's
  were described in the HOB. We have to remember the handle so we can tell if 
  the protocol has been reinstalled and it needs updateing.

  If you are using any of these lib functions.you must first call FvbInitialize ().

Key:
  FVB - Firmware Volume Block

--*/
#include "Tiano.h"
#include "EfiRuntimeLib.h"
#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeBlock)
#include EFI_PROTOCOL_DEFINITION (FvbExtension)

//
// Lib will ASSERT if more FVB devices than this are added to the system.
//
UINTN                       mFvbCount;
VOID                        *mFvbRegistration;
VOID                        *mFvbExtRegistration;
static EFI_EVENT            mEfiFvbVirtualNotifyEvent;
BOOLEAN                     gEfiFvbInitialized = FALSE;

VOID
EFIAPI
FvbNotificationFunction (
  EFI_EVENT       Event,
  VOID            *Context
  )
/*++

Routine Description:
  Update mFvbEntry. Add new entry, or update existing entry if Fvb protocol is
  reinstalled.

Arguments:
  (Standard EFI notify event - EFI_EVENT_NOTIFY)

Returns: 
  None

--*/
{
  EFI_STATUS                            Status;
  UINTN                                 BufferSize;
  EFI_HANDLE                            Handle;
  UINTN                                 Index;
  UINTN                                 UpdateIndex;
  
  while (TRUE) {
    BufferSize = sizeof(Handle);
    Status = gBS->LocateHandle (
                    ByRegisterNotify,
                    &gEfiFirmwareVolumeBlockProtocolGuid,
                    mFvbRegistration,
                    &BufferSize,
                    &Handle
                    );
    if (EFI_ERROR (Status)) {
      //  
      // Exit Path of While Loop....
      //
      break;
    }

    UpdateIndex = MAX_FVB_COUNT;
    for (Index = 0; Index < mFvbCount; Index++) {
      if (mFvbEntry[Index].Handle == Handle) {
        //
        //  If the handle is already in the table just update the protocol
        //
        UpdateIndex = Index;
        break;
      }
    }

    if (UpdateIndex == MAX_FVB_COUNT) {
      //
      // Use the next free slot for a new entry
      //
      UpdateIndex = mFvbCount++;;
      mFvbEntry[UpdateIndex].Handle = Handle;
    }

    //
    // The array does not have enough entries
    //
    ASSERT (UpdateIndex < MAX_FVB_COUNT);

    //
    //  Get the interface pointer and if it's ours, skip it
    //
    Status = gBS->HandleProtocol (Handle, &gEfiFirmwareVolumeBlockProtocolGuid, &mFvbEntry[UpdateIndex].Fvb);        
    ASSERT_EFI_ERROR (Status);

    Status = gBS->HandleProtocol (Handle, &gEfiFvbExtensionProtocolGuid, &mFvbEntry[UpdateIndex].FvbExtension);
    if (Status != EFI_SUCCESS) {
      mFvbEntry[UpdateIndex].FvbExtension = NULL;
    }
  }
}

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
  UINTN         Status;
  mFvbCount = 0;

  Status = gBS->AllocatePool (EfiRuntimeServicesData,
                              (UINTN) sizeof (FVB_ENTRY) * MAX_FVB_COUNT,
                              (VOID *)&mFvbEntry
                              );
                              
  if (EFI_ERROR (Status)) {
    return Status;
  }

  EfiZeroMem (mFvbEntry, sizeof (FVB_ENTRY) * MAX_FVB_COUNT);

  RtEfiLibCreateProtocolNotifyEvent (
    &gEfiFirmwareVolumeBlockProtocolGuid,
    EFI_TPL_CALLBACK,
    FvbNotificationFunction,
    NULL,
    &mFvbRegistration
    );

  //
  // Register SetVirtualAddressMap () notify function
  //
//  Status = gBS->CreateEvent (
//                EFI_EVENT_SIGNAL_VIRTUAL_ADDRESS_CHANGE, 
//                EFI_TPL_NOTIFY,
//                EfiRuntimeLibFvbVirtualNotifyEvent,
//                NULL,
//                &mEfiFvbVirtualNotifyEvent
//                );
//  ASSERT_EFI_ERROR (Status);  
  gEfiFvbInitialized = TRUE;

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
  if (Instance >= mFvbCount) {
    return EFI_INVALID_PARAMETER;
  }
  return mFvbEntry[Instance].Fvb->Read (mFvbEntry[Instance].Fvb, Lba, Offset, NumBytes, Buffer);
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
  if (Instance >= mFvbCount) {
    return EFI_INVALID_PARAMETER;
  }
  return mFvbEntry[Instance].Fvb->Write (mFvbEntry[Instance].Fvb, Lba, Offset, NumBytes, Buffer);
}

EFI_STATUS
EfiFvbEraseBlock (
  IN UINTN                                Instance,
  IN EFI_LBA                              Lba
  )
{
  if (Instance >= mFvbCount) {
    return EFI_INVALID_PARAMETER;
  }
  return mFvbEntry[Instance].Fvb->EraseBlocks (mFvbEntry[Instance].Fvb, Lba, -1);
}

EFI_STATUS
EfiFvbGetVolumeAttributes (
  IN UINTN                                Instance,
  OUT EFI_FVB_ATTRIBUTES                  *Attributes
  )
{
  if (Instance >= mFvbCount) {
    return EFI_INVALID_PARAMETER;
  }
  return mFvbEntry[Instance].Fvb->GetVolumeAttributes (mFvbEntry[Instance].Fvb, Attributes);
}

EFI_STATUS
EfiFvbSetVolumeAttributes (
  IN UINTN                                Instance,
  IN EFI_FVB_ATTRIBUTES                   Attributes
  )
{
  if (Instance >= mFvbCount) {
    return EFI_INVALID_PARAMETER;
  }
  return mFvbEntry[Instance].Fvb->SetVolumeAttributes (mFvbEntry[Instance].Fvb, &Attributes);
}

EFI_STATUS
EfiFvbGetPhysicalAddress (
  IN UINTN                                Instance,
  OUT EFI_PHYSICAL_ADDRESS                *BaseAddress  
  )
{
  if (Instance >= mFvbCount) {
    return EFI_INVALID_PARAMETER;
  }
  return mFvbEntry[Instance].Fvb->GetPhysicalAddress (mFvbEntry[Instance].Fvb, BaseAddress);
}

EFI_STATUS
EfiFvbGetBlockSize (
  IN UINTN                                        Instance,
  IN EFI_LBA                                      Lba,
  OUT UINTN                                       *BlockSize,
  OUT UINTN                                       *NumOfBlocks
  )
{
  if (Instance >= mFvbCount) {
    return EFI_INVALID_PARAMETER;
  }
  return mFvbEntry[Instance].Fvb->GetBlockSize (mFvbEntry[Instance].Fvb, Lba, BlockSize, NumOfBlocks);
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
  if (Instance >= mFvbCount) {
    return EFI_INVALID_PARAMETER;
  }
  
  if ( ! (mFvbEntry[Instance].FvbExtension) ) {
    return EFI_UNSUPPORTED;
  }
  if ( ! (mFvbEntry[Instance].FvbExtension->EraseFvbCustomBlock) ) {
    return EFI_UNSUPPORTED;
  }
  return mFvbEntry[Instance].FvbExtension->EraseFvbCustomBlock (mFvbEntry[Instance].FvbExtension, StartLba, 
                                                                OffsetStartLba, LastLba, OffsetLastLba);
}
