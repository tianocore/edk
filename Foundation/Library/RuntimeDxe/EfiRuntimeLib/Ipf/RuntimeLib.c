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

  Light weight lib to support EFI 2.0 Sal drivers.

--*/

#include "Tiano.h"
#include "EfiRuntimeLib.h"
#include EFI_PROTOCOL_DEFINITION (ExtendedSalBootService)
#include EFI_PROTOCOL_DEFINITION (ExtendedSalGuid)
#include "IpfDefines.h"
#include "SalApi.h"

//
// Worker functions in EsalLib.s
//

SAL_RETURN_REGS
GetEsalEntryPoint (
  VOID
  );

SAL_RETURN_REGS
SetEsalPhysicalEntryPoint (
  IN  UINT64  EntryPoint,
  IN  UINT64  Gp
  );

SAL_RETURN_REGS
SetEsalVirtualEntryPoint (
  IN  UINT64  EntryPoint,
  IN  UINT64  Gp
  );

VOID
SalFlushCache (
  IN EFI_PHYSICAL_ADDRESS  Start,
  IN UINT64                Length
  );


//
// Module Globals. It's not valid to use these after the 
// EfiRuntimeLibVirtualNotifyEvent has fired.
//
static EFI_EVENT                            mEfiVirtualNotifyEvent;
static EFI_RUNTIME_SERVICES                 *mRT;
static EFI_PLABEL                           mPlabel;
static EXTENDED_SAL_BOOT_SERVICE_PROTOCOL   *mEsalBootService;
static BOOLEAN                              mRuntimeLibInitialized = FALSE;



VOID
EFIAPI
EfiRuntimeLibVirtualNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
/*++

Routine Description:

  Fixup internal data so that EFI and SAL can be call in virtual mode.
  Call the passed in Child Notify event and convert any pointers in 
  lib to virtual mode.

Arguments:

  (Standard EFI notify event - EFI_EVENT_NOTIFY)

Returns: 

  None

--*/
{
  EFI_EVENT_NOTIFY  ChildNotify;

  if (Context != NULL) {
    //
    // Call child event
    //
    ChildNotify = (EFI_EVENT_NOTIFY)(UINTN)Context;
    ChildNotify (Event, NULL);
  }

  mRT->ConvertPointer (EFI_INTERNAL_POINTER, (VOID **) &mPlabel.EntryPoint);
  mRT->ConvertPointer (EFI_INTERNAL_POINTER | EFI_IPF_GP_POINTER, (VOID **) &mPlabel.GP);
  
  SetEsalVirtualEntryPoint (mPlabel.EntryPoint, mPlabel.GP);

  //
  // Clear out BootService globals
  //
  gBS = NULL;
  gST = NULL;
  mRT = NULL;

  //
  // Pointers don't work you must use a direct lib call
  //
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

  GoVirtualChildEvent - Caller can register a virtual notification event.

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 

  EFI_STATUS always returns EFI_SUCCESS

--*/
{
  EFI_STATUS                          Status;
  EFI_PLABEL                          *Plabel;

  if (mRuntimeLibInitialized) {
    return EFI_ALREADY_STARTED;
  }

  mRuntimeLibInitialized = TRUE;

  gST = SystemTable;
  gBS = SystemTable->BootServices;
  mRT = SystemTable->RuntimeServices;
  Status = EfiLibGetSystemConfigurationTable (&gEfiDxeServicesTableGuid, (VOID **)&gDS);
  ASSERT_EFI_ERROR (Status);
 
  //
  // The protocol contains a function pointer, which is an indirect procedure call.
  // An indirect procedure call goes through a plabel, and pointer to a function is
  // a pointer to a plabel. To implement indirect procedure calls that can work in
  // both physical and virtual mode, two plabels are required (one physical and one
  // virtual). So lets grap the physical PLABEL for the EsalEntryPoint and store it
  // away. We cache it in a module global, so we can register the vitrual version.
  //
  Status = gBS->LocateProtocol (&gEfiExtendedSalBootServiceProtocolGuid, NULL, &mEsalBootService);
  ASSERT_EFI_ERROR (Status);

  Plabel = (EFI_PLABEL *)(UINTN)mEsalBootService->ExtendedSalProc;

  mPlabel.EntryPoint = Plabel->EntryPoint;
  mPlabel.GP = Plabel->GP;

  SetEsalPhysicalEntryPoint (mPlabel.EntryPoint, mPlabel.GP);

  //
  // Create a Virtual address change notification event. Pass in the callers
  // GoVirtualChildEvent so it's get passed to the event as contex.
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_SIGNAL_VIRTUAL_ADDRESS_CHANGE, 
                  EFI_TPL_NOTIFY,
                  EfiRuntimeLibVirtualNotifyEvent,
                  (VOID *)GoVirtualChildEvent,
                  &mEfiVirtualNotifyEvent
                  );
  ASSERT_EFI_ERROR (Status);  

  return EFI_SUCCESS;
}



EFI_STATUS
RegisterEsalFunction (
  IN  UINT64                                    FunctionId,
  IN  EFI_GUID                                  *ClassGuid,
  IN  SAL_INTERNAL_EXTENDED_SAL_PROC            Function,
  IN  VOID                                      *ModuleGlobal
  )
/*++

Routine Description:

  Register ESAL Class Function and it's asociated global.
  This function is boot service only!

Arguments:
  FunctionId    - ID of function to register
  ClassGuid     - GUID of function class 
  Function      - Function to register under ClassGuid/FunctionId pair
  ModuleGlobal  - Module global for Function.

Returns: 
  EFI_SUCCESS - If ClassGuid/FunctionId Function was registered.

--*/
{
  return mEsalBootService->AddExtendedSalProc (
                            mEsalBootService, ClassGuid, FunctionId, Function, ModuleGlobal
                            );
}


EFI_STATUS
RegisterEsalClass (
  IN  EFI_GUID                                  *ClassGuid,
  IN  VOID                                      *ModuleGlobal,
  ...
  )
/*++

Routine Description:

  Register ESAL Class and it's asociated global.
  This function is boot service only!

Arguments:
  ClassGuid     - GUID of function class 
  ModuleGlobal  - Module global for Function.
  ..            - SAL_INTERNAL_EXTENDED_SAL_PROC and FunctionId pairs. NULL 
                  indicates the end of the list.

Returns: 
  EFI_SUCCESS - All members of ClassGuid registered

--*/
{ 
  VA_LIST                         Args;
  EFI_STATUS                      Status;
  SAL_INTERNAL_EXTENDED_SAL_PROC  Function;
  UINT64                          FunctionId;
  EFI_HANDLE                      NewHandle;

  VA_START (Args, ModuleGlobal);

  Status = EFI_SUCCESS;
  while (!EFI_ERROR (Status)) {
    Function = (SAL_INTERNAL_EXTENDED_SAL_PROC) VA_ARG (Args, SAL_INTERNAL_EXTENDED_SAL_PROC);
    if (Function == NULL) {
      break;
    }

    FunctionId = VA_ARG (Args, UINT64);

    Status = RegisterEsalFunction (FunctionId, ClassGuid, Function, ModuleGlobal);
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }

  NewHandle = NULL;
  return gBS->InstallProtocolInterface (
                &NewHandle, ClassGuid, EFI_NATIVE_INTERFACE, NULL
                );
}


SAL_RETURN_REGS
EfiCallEsalService (
  IN  EFI_GUID                                      *ClassGuid,
  IN  UINT64                                        FunctionId,
  IN  UINT64                                        Arg2,
  IN  UINT64                                        Arg3,
  IN  UINT64                                        Arg4,
  IN  UINT64                                        Arg5,
  IN  UINT64                                        Arg6,
  IN  UINT64                                        Arg7,
  IN  UINT64                                        Arg8
  ) 
/*++

Routine Description:

  Call module that is not linked direclty to this module. This code is IP 
  relative and hides the binding issues of virtual or physical calling. The
  function that gets dispatched has extra arguments that include the registered
  module global and a boolean flag to indicate if the system is in virutal mode.

Arguments:
  ClassGuid   - GUID of function
  FunctionId  - Function in ClassGuid to call
  Arg2        - Argument 2 ClassGuid/FunctionId defined
  Arg3        - Argument 3 ClassGuid/FunctionId defined
  Arg4        - Argument 4 ClassGuid/FunctionId defined
  Arg5        - Argument 5 ClassGuid/FunctionId defined
  Arg6        - Argument 6 ClassGuid/FunctionId defined
  Arg7        - Argument 7 ClassGuid/FunctionId defined
  Arg8        - Argument 8 ClassGuid/FunctionId defined

Returns: 
  Status of ClassGuid/FuncitonId

--*/
{
  SAL_RETURN_REGS         ReturnReg;
  SAL_EXTENDED_SAL_PROC   EsalProc;

  ReturnReg = GetEsalEntryPoint ();
  if (ReturnReg.Status != EFI_SAL_SUCCESS) {
    return ReturnReg;
  }

  if (ReturnReg.r11 & PSR_IT_MASK) {
    //
    // Virtual mode plabel to entry point
    //
    EsalProc = (SAL_EXTENDED_SAL_PROC)ReturnReg.r10;
  } else {
    //
    // Physical mode plabel to entry point
    //
    EsalProc = (SAL_EXTENDED_SAL_PROC)ReturnReg.r9;
  }

  return EsalProc (
          ClassGuid, FunctionId, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8
          );
}



EFI_STATUS
EfiConvertPointer (
  IN UINTN                     DebugDisposition,
  IN OUT VOID                  *Address
  )
{
  return mRT->ConvertPointer (DebugDisposition, Address);
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
  EFI_GUID Guid = EFI_EXTENDED_SAL_VIRTUAL_SERVICES_PROTOCOL_GUID;
  SAL_RETURN_REGS         ReturnReg;

  ReturnReg = EfiCallEsalService (&Guid, IsVirtual, 0 , 0 , 0 , 0 , 0 , 0 , 0);

  return (BOOLEAN)(ReturnReg.r9 == 1);
}


BOOLEAN
EfiAtRuntime (
  VOID
  ) 
/*++

Routine Description:
  Return TRUE if ExitBootService () has been called

Arguments:
  NONE

Returns: 
  TRUE - If ExitBootService () has been called

--*/
{
  EFI_GUID Guid = EFI_EXTENDED_SAL_VIRTUAL_SERVICES_PROTOCOL_GUID;
  SAL_RETURN_REGS         ReturnReg;

  ReturnReg = EfiCallEsalService (&Guid, IsEfiRuntime, 0 , 0 , 0 , 0 , 0 , 0 , 0);

  return (BOOLEAN)(ReturnReg.r9 == 1);
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
  EFI_GUID Guid = EFI_EXTENDED_SAL_STATUS_CODE_SERVICES_PROTOCOL_GUID;
  SAL_RETURN_REGS         ReturnReg;

  ReturnReg = EfiCallEsalService (
                &Guid, StatusCode, (UINT64)CodeType , (UINT64) Value , 
                (UINT64) Instance , (UINT64) CallerId , (UINT64)Data , 0 , 0
                );

  return (EFI_STATUS)ReturnReg.Status;
}



//
//  Sal Reset Driver Class
//
VOID
EfiResetSystem (
  IN EFI_RESET_TYPE     ResetType,
  IN EFI_STATUS         ResetStatus,
  IN UINTN              DataSize,
  IN CHAR16             *ResetData
  )
{
  EFI_GUID Guid = EFI_EXTENDED_SAL_RESET_SERVICES_PROTOCOL_GUID;

  EfiCallEsalService (
    &Guid, ResetSystem, (UINT64)ResetType , (UINT64) ResetStatus , 
    (UINT64) DataSize , (UINT64) ResetData , 0 , 0 , 0
    );
}

//
//  Sal MTC Driver Class
//

EFI_STATUS
EfiGetNextHighMonotonicCount (
  OUT UINT32      *HighCount
  )
{
  SAL_RETURN_REGS         ReturnReg;

  EFI_GUID Guid = EFI_EXTENDED_SAL_MTC_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, GetNextHighMonotonicCount, (UINT64)HighCount , 0, 0, 0, 0, 0, 0);
  return (EFI_STATUS)ReturnReg.Status;
}

//
// Sal Variable Driver Class
//

EFI_STATUS
EfiGetVariable (
  IN CHAR16                       *VariableName,
  IN EFI_GUID                     *VendorGuid,
  OUT UINT32                      *Attributes OPTIONAL,
  IN OUT UINTN                    *DataSize,
  OUT VOID                        *Data
  )
{
  SAL_RETURN_REGS         ReturnReg;
  EFI_GUID Guid = EFI_EXTENDED_SAL_VARIABLE_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, EsalGetVariable, (UINT64)VariableName, (UINT64)VendorGuid, (UINT64)Attributes, (UINT64)DataSize, (UINT64)Data, 0, 0);
  return (EFI_STATUS)ReturnReg.Status;
}


EFI_STATUS
EfiGetNextVariableName (
  IN OUT UINTN                    *VariableNameSize,
  IN OUT CHAR16                   *VariableName,
  IN OUT EFI_GUID                 *VendorGuid
  )
{
  SAL_RETURN_REGS         ReturnReg;
  EFI_GUID Guid = EFI_EXTENDED_SAL_VARIABLE_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, EsalGetNextVariableName, (UINT64)VariableNameSize, (UINT64)VariableName, (UINT64)VendorGuid, 0, 0, 0, 0);
  return (EFI_STATUS)ReturnReg.Status;
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
  SAL_RETURN_REGS         ReturnReg;
  EFI_GUID Guid = EFI_EXTENDED_SAL_VARIABLE_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, EsalSetVariable, (UINT64)VariableName, (UINT64)VendorGuid, (UINT64)Attributes, (UINT64)DataSize, (UINT64)Data, 0, 0);
  return (EFI_STATUS)ReturnReg.Status;
}


//
//  Sal RTC Driver Class.
//

EFI_STATUS
EfiGetTime (
  OUT EFI_TIME              *Time,
  OUT EFI_TIME_CAPABILITIES *Capabilities 
  )
{
  SAL_RETURN_REGS         ReturnReg;
  EFI_GUID Guid = EFI_EXTENDED_SAL_RTC_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, GetTime, (UINT64) Time , (UINT64) Capabilities, 0, 0, 0, 0, 0);
  return ReturnReg.Status;
}


EFI_STATUS
EfiSetTime (
  OUT EFI_TIME              *Time
  )
{
  SAL_RETURN_REGS         ReturnReg;

  EFI_GUID Guid = EFI_EXTENDED_SAL_RTC_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, SetTime, (UINT64) Time , 0, 0, 0, 0, 0, 0);
  return ReturnReg.Status;
}


EFI_STATUS
EfiGetWakeupTime (
  OUT BOOLEAN       *Enabled,
  OUT BOOLEAN       *Pending,
  OUT EFI_TIME      *Time
  )
{
  SAL_RETURN_REGS         ReturnReg;

  EFI_GUID Guid = EFI_EXTENDED_SAL_RTC_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, GetWakeupTime, (UINT64) Enabled , (UINT64) Pending, (UINT64) Time, 0, 0, 0, 0);
  return ReturnReg.Status;
}


EFI_STATUS
EfiSetWakeupTime (
  IN BOOLEAN        Enable,
  IN EFI_TIME       *Time
  )
{
  SAL_RETURN_REGS         ReturnReg;

  EFI_GUID Guid = EFI_EXTENDED_SAL_RTC_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, SetWakeupTime, (UINT64) Enable , (UINT64) Time, 0, 0, 0, 0, 0);
  return ReturnReg.Status;
}



//
//  Base IO Services
//

EFI_STATUS
EfiIoRead (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                     Address,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  )
{

  SAL_RETURN_REGS         ReturnReg;

  EFI_GUID Guid = EFI_EXTENDED_SAL_BASE_IO_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, IoRead, (UINT64) Width , Address, Count, (UINT64) Buffer, 0, 0, 0);

  return ReturnReg.Status;
  
}

EFI_STATUS
EfiIoWrite (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                     Address,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  )
{

  SAL_RETURN_REGS         ReturnReg;

  EFI_GUID Guid = EFI_EXTENDED_SAL_BASE_IO_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, IoWrite, (UINT64) Width , Address, Count, (UINT64) Buffer, 0, 0, 0);

  return ReturnReg.Status;
  
}

EFI_STATUS
EfiMemRead (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                     Address,
  IN     UINTN                      Count,
  IN  OUT VOID                      *Buffer
  )
{

  SAL_RETURN_REGS         ReturnReg;

  EFI_GUID Guid = EFI_EXTENDED_SAL_BASE_IO_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, MemRead, (UINT64) Width , Address, Count, (UINT64) Buffer, 0, 0, 0);

  return ReturnReg.Status;
  
}

EFI_STATUS
EfiMemWrite (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                     Address,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
  )
{

  SAL_RETURN_REGS         ReturnReg;

  EFI_GUID Guid = EFI_EXTENDED_SAL_BASE_IO_SERVICES_PROTOCOL_GUID;

  ReturnReg = EfiCallEsalService (&Guid, MemWrite, (UINT64) Width , Address, Count, (UINT64) Buffer, 0, 0, 0);

  return ReturnReg.Status;
  
}


#define EFI_PCI_ADDRESS_IPF(_seg, _bus, _devfunc, _reg) \
  ( ((_seg) << 24) | ((_bus) << 16) | ((_devfunc) << 8) | (_reg) ) & 0xFFFFFFFF

//
//  PCI Class Functions
//
UINT8
PciRead8 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register
  )
{
  EFI_GUID        Guid = EFI_EXTENDED_SAL_PCI_SERVICES_PROTOCOL_GUID;
  UINT64          Address;
  SAL_RETURN_REGS Return;
  
  Address = EFI_PCI_ADDRESS_IPF (Segment, Bus, DevFunc, Register);
  Return = EfiCallEsalService (&Guid, SalPciConfigRead, Address, 1, 0, 0, 0, 0, 0);

  return (UINT8)Return.r9;
}


UINT16
PciRead16 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register
  )
{
  EFI_GUID        Guid = EFI_EXTENDED_SAL_PCI_SERVICES_PROTOCOL_GUID;
  UINT64          Address;
  SAL_RETURN_REGS Return;
  
  Address = EFI_PCI_ADDRESS_IPF (Segment, Bus, DevFunc, Register);
  Return = EfiCallEsalService (&Guid, SalPciConfigRead, Address, 2, 0, 0, 0, 0, 0);

  return (UINT16)Return.r9;
}


UINT32
PciRead32 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register
  )
{
  EFI_GUID        Guid = EFI_EXTENDED_SAL_PCI_SERVICES_PROTOCOL_GUID;
  UINT64          Address;
  SAL_RETURN_REGS Return;
  
  Address = EFI_PCI_ADDRESS_IPF (Segment, Bus, DevFunc, Register);
  Return = EfiCallEsalService (&Guid, SalPciConfigRead, Address, 4, 0, 0, 0, 0, 0);

  return (UINT32)Return.r9;
}


VOID
PciWrite8 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register,
  UINT8   Data
  )
{
  EFI_GUID        Guid = EFI_EXTENDED_SAL_PCI_SERVICES_PROTOCOL_GUID;
  UINT64          Address;
  
  Address = EFI_PCI_ADDRESS_IPF (Segment, Bus, DevFunc, Register);
  EfiCallEsalService (&Guid, SalPciConfigWrite, Address, 1, Data, 0, 0, 0, 0);
}


VOID
PciWrite16 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register,
  UINT16  Data
  )
{
  EFI_GUID        Guid = EFI_EXTENDED_SAL_PCI_SERVICES_PROTOCOL_GUID;
  UINT64          Address;
  
  Address = EFI_PCI_ADDRESS_IPF (Segment, Bus, DevFunc, Register);
  EfiCallEsalService (&Guid, SalPciConfigWrite, Address, 2, Data, 0, 0, 0, 0);
}

VOID
PciWrite32 (
  UINT8   Segment,
  UINT8   Bus,
  UINT8   DevFunc,
  UINT8   Register,
  UINT32  Data
  )
{
  EFI_GUID        Guid = EFI_EXTENDED_SAL_PCI_SERVICES_PROTOCOL_GUID;
  UINT64          Address;
  
  Address = EFI_PCI_ADDRESS_IPF (Segment, Bus, DevFunc, Register);
  EfiCallEsalService (&Guid, SalPciConfigWrite, Address, 4, Data, 0, 0, 0, 0);
}

//
// Stall class functions
//
VOID
EfiStall (
  IN  UINTN   Microseconds
  )
{
  EFI_GUID        Guid = EFI_EXTENDED_SAL_STALL_SERVICES_PROTOCOL_GUID;

  if (EfiAtRuntime ()) {
    EfiCallEsalService (&Guid, Stall, Microseconds, 4, 0, 0, 0, 0, 0);
  } else {
    gBS->Stall (Microseconds);
  }
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
  SalFlushCache (Start, Length);
  return  EFI_SUCCESS; 
}



