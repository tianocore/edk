/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    RuntimeLib.c

Abstract:

  Light weight lib to support EFI 2.0 drivers.

--*/

#include "Tiano.h"
#include "EfiRuntimeLib.h"
#include EFI_PROTOCOL_DEFINITION (CpuIo)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeBlock)
#include EFI_GUID_DEFINITION(StatusCodeCallerId)

//
// Driver Lib Module Globals
//
static EFI_RUNTIME_SERVICES  *mRT;
static EFI_EVENT             mRuntimeNotifyEvent;
static EFI_EVENT             mEfiVirtualNotifyEvent;
static BOOLEAN               mRuntimeLibInitialized = FALSE;
static BOOLEAN               mEfiGoneVirtual = FALSE;

//
// Runtime Global, but you should use the Lib functions
//
EFI_CPU_IO_PROTOCOL  *gCpuIo;
BOOLEAN               mEfiAtRuntime = FALSE;
FVB_ENTRY             *mFvbEntry;

EFI_STATUS
EfiConvertPointer (
  IN UINTN                     DebugDisposition,
  IN OUT VOID                  *Address
  )
{
  return mRT->ConvertPointer (DebugDisposition, Address);
}

EFI_STATUS
EfiConvertInternalPointer (
  IN OUT VOID                  *Address
  )
{
  return EfiConvertPointer (EFI_INTERNAL_POINTER, Address);
}

VOID
EFIAPI
EfiRuntimeLibFvbVirtualNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  UINTN                                   Index;
  if (mFvbEntry != NULL) {
    for (Index = 0; Index < MAX_FVB_COUNT; Index ++) {
      if (NULL != mFvbEntry[Index].Fvb) {
        EfiConvertInternalPointer ((VOID **) &mFvbEntry[Index].Fvb->GetBlockSize);
        EfiConvertInternalPointer ((VOID **) &mFvbEntry[Index].Fvb->GetPhysicalAddress);
        EfiConvertInternalPointer ((VOID **) &mFvbEntry[Index].Fvb->GetVolumeAttributes);      
        EfiConvertInternalPointer ((VOID **) &mFvbEntry[Index].Fvb->SetVolumeAttributes);      
        EfiConvertInternalPointer ((VOID **) &mFvbEntry[Index].Fvb->Read);      
        EfiConvertInternalPointer ((VOID **) &mFvbEntry[Index].Fvb->Write);      
        EfiConvertInternalPointer ((VOID **) &mFvbEntry[Index].Fvb->EraseBlocks);      
        EfiConvertInternalPointer ((VOID **) &mFvbEntry[Index].Fvb);
      }
      if (NULL != mFvbEntry[Index].FvbExtension) {
        EfiConvertInternalPointer ((VOID **) &mFvbEntry[Index].FvbExtension->EraseFvbCustomBlock);      
        EfiConvertInternalPointer ((VOID **) &mFvbEntry[Index].FvbExtension);      
      }
    }
    EfiConvertInternalPointer ((VOID **) &mFvbEntry);
  } 
}

VOID
EFIAPI
RuntimeDriverExitBootServices (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
/*++

Routine Description:

  Set Runtime Global

Arguments:

  (Standard EFI notify event - EFI_EVENT_NOTIFY)

Returns: 

  None

--*/
{
  mEfiAtRuntime = TRUE;
}


extern BOOLEAN gEfiFvbInitialized;

VOID
EFIAPI
EfiRuntimeLibVirtualNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
/*++

Routine Description:

  Fixup internal data so that EFI can be call in virtual mode.
  Call the passed in Child Notify event and convert any pointers in 
  lib to virtual mode.

Arguments:

  (Standard EFI notify event - EFI_EVENT_NOTIFY)

Returns: 

  None

--*/
{
  EFI_EVENT_NOTIFY  ChildNotifyEventHandler;

  if (Context != NULL) {
    ChildNotifyEventHandler = (EFI_EVENT_NOTIFY)(UINTN)Context;
    ChildNotifyEventHandler (Event, NULL);
  }

  if (gEfiFvbInitialized) {
    EfiRuntimeLibFvbVirtualNotifyEvent (Event, Context);
  }

  //
  // Update global for Runtime Services Table and IO
  //
  EfiConvertInternalPointer ((VOID **) &gCpuIo);
  EfiConvertInternalPointer ((VOID **) &mRT);

  //
  // Clear out BootService globals
  //
  gBS = NULL;
  gST = NULL;
  mEfiGoneVirtual = TRUE;
}



EFI_STATUS
EfiInitializeRuntimeDriverLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN EFI_EVENT_NOTIFY     GoVirtualChildEvent
  )
/*++

Routine Description:

  Intialize runtime Driver Lib if it has not yet been initialized. 

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

  GoVirtualChildEvent - Caller can register a virtual notification event.

Returns: 

  EFI_STATUS always returns EFI_SUCCESS

--*/
{
  EFI_STATUS  Status;

  if (mRuntimeLibInitialized) {
    return EFI_ALREADY_STARTED;
  }

  mRuntimeLibInitialized = TRUE;

  mRT = SystemTable->RuntimeServices;
  
  if ((SystemTable != NULL) && (SystemTable->BootServices != NULL)) {

    gST = SystemTable;
    gBS = SystemTable->BootServices;
    Status = EfiLibGetSystemConfigurationTable (&gEfiDxeServicesTableGuid, (VOID **)&gDS);
    ASSERT_EFI_ERROR (Status);

    Status = gBS->LocateProtocol (&gEfiCpuIoProtocolGuid, NULL, &gCpuIo);

    if (EFI_ERROR (Status)) {
      gCpuIo = NULL;
    }

    //
    // Register our ExitBootServices () notify function
    //
    Status = gBS->CreateEvent (
                  EFI_EVENT_SIGNAL_EXIT_BOOT_SERVICES, 
                  EFI_TPL_NOTIFY,
                  RuntimeDriverExitBootServices,
                  NULL,
                  &mRuntimeNotifyEvent
                  );
    ASSERT_EFI_ERROR  (Status);

    //
    // Register SetVirtualAddressMap () notify function
    //
    Status = gBS->CreateEvent (
                  EFI_EVENT_SIGNAL_VIRTUAL_ADDRESS_CHANGE, 
                  EFI_TPL_NOTIFY,
                  EfiRuntimeLibVirtualNotifyEvent,
                  (VOID *)(UINTN)GoVirtualChildEvent,
                  &mEfiVirtualNotifyEvent
                  );
    ASSERT_EFI_ERROR (Status);  

  }

  return EFI_SUCCESS;
}

EFI_STATUS
EfiInitializeSmmDriverLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Intialize runtime Driver Lib if it has not yet been initialized. 

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

  GoVirtualChildEvent - Caller can register a virtual notification event.

Returns: 

  EFI_STATUS always returns EFI_SUCCESS

--*/
{
  EFI_STATUS  Status;

  if (mRuntimeLibInitialized) {
    return EFI_ALREADY_STARTED;
  }

  mRuntimeLibInitialized = TRUE;

  mRT = SystemTable->RuntimeServices;
  
  if ((SystemTable != NULL) && (SystemTable->BootServices != NULL)) {

    gST = SystemTable;
    gBS = SystemTable->BootServices;

    Status = gBS->LocateProtocol (&gEfiCpuIoProtocolGuid, NULL, &gCpuIo);

    if (EFI_ERROR (Status)) {
      gCpuIo = NULL;
    }
  }

  return EFI_SUCCESS;
}



BOOLEAN
EfiAtRuntime (
  VOID
  ) 
/*++

Routine Description:
  Return TRUE if ExitBootServices () has been called

Arguments:
  NONE

Returns: 
  TRUE - If ExitBootServices () has been called

--*/
{
  return mEfiAtRuntime;
}


BOOLEAN
EfiGoneVirtual (
  VOID
  ) 
/*++

Routine Description:
  Return TRUE if SetVirtualAddressMap () has been called

Arguments:
  NONE

Returns: 
  TRUE - If SetVirtualAddressMap () has been called

--*/
{
  return mEfiGoneVirtual;
}



//
// The following functions hide the mRT local global from the call to 
// runtime service in the EFI system table.
//

EFI_STATUS
EfiGetTime (
  OUT EFI_TIME                    *Time,
  OUT EFI_TIME_CAPABILITIES       *Capabilities 
  )
{
  return mRT->GetTime (Time, Capabilities);
}



EFI_STATUS
EfiSetTime (
  IN EFI_TIME                   *Time
  )
{
  return mRT->SetTime (Time);
}


EFI_STATUS
EfiGetWakeupTime (
  OUT BOOLEAN                     *Enabled,
  OUT BOOLEAN                     *Pending,
  OUT EFI_TIME                    *Time
  )
{
  return mRT->GetWakeupTime (Enabled, Pending, Time);
}


EFI_STATUS
EfiSetWakeupTime (
  IN BOOLEAN                      Enable,
  IN EFI_TIME                    *Time      
  )
{
  return mRT->SetWakeupTime (Enable, Time);
}


EFI_STATUS
EfiGetVariable (
  IN CHAR16                       *VariableName,
  IN EFI_GUID                     *VendorGuid,
  OUT UINT32                      *Attributes OPTIONAL,
  IN OUT UINTN                    *DataSize,
  OUT VOID                        *Data
  )
{
  return mRT->GetVariable (VariableName, VendorGuid, Attributes, DataSize, Data);
}


EFI_STATUS
EfiGetNextVariableName (
  IN OUT UINTN                    *VariableNameSize,
  IN OUT CHAR16                   *VariableName,
  IN OUT EFI_GUID                 *VendorGuid
  )
{
  return mRT->GetNextVariableName (VariableNameSize, VariableName, VendorGuid);
}


EFI_STATUS
EfiSetVariable (
  IN CHAR16                       *VariableName,
  IN EFI_GUID                     *VendorGuid,
  IN UINT32                       Attributes,
  IN UINTN                        DataSize,
  IN VOID                         *Data
  )
{
  return mRT->SetVariable(VariableName, VendorGuid, Attributes, DataSize, Data);
}


EFI_STATUS
EfiGetNextHighMonotonicCount (
  OUT UINT32                      *HighCount
  )
{
  return mRT->GetNextHighMonotonicCount (HighCount);
}


VOID
EfiResetSystem (
  IN EFI_RESET_TYPE               ResetType,
  IN EFI_STATUS                   ResetStatus,
  IN UINTN                        DataSize,
  IN CHAR16                       *ResetData
  )
{
  mRT->ResetSystem (ResetType, ResetStatus, DataSize, ResetData);
}


EFI_STATUS
EfiReportStatusCode (
  IN EFI_STATUS_CODE_TYPE     CodeType,
  IN EFI_STATUS_CODE_VALUE    Value,  
  IN UINT32                   Instance,
  IN EFI_GUID                 *CallerId,
  IN EFI_STATUS_CODE_DATA     *Data OPTIONAL
  )
{
  return mRT->ReportStatusCode (
                CodeType,
                Value,  
                Instance,
                CallerId,
                Data 
                );
}


//
// Cache Flush Routine.
//

EFI_STATUS
EfiCpuFlushCache (
  IN EFI_PHYSICAL_ADDRESS          Start,
  IN UINT64                        Length
  )
{
  return EFI_SUCCESS; 
}

