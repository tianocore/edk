/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SalDriverLib.h
  
Abstract:

  SAL Driver Lib

Revision History

--*/

#ifndef _SAL_DRIVER_LIB_H_
#define _SAL_DRIVER_LIB_H_

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "SalApi.h"

#include EFI_PROTOCOL_DEFINITION(ExtendedSalBootService)
#include EFI_PROTOCOL_DEFINITION(ExtendedSalGuid)

//
//  Assembly Functions
//
SAL_RETURN_REGS
LibGetEsalPhyData (
  );

SAL_RETURN_REGS
LibGetEsalVirtData (
  );

SAL_RETURN_REGS
LibSetEsalPhyData (
  IN  VOID                        *Ptr,
  IN  UINT64                      GP
  );

SAL_RETURN_REGS
LibSetEsalVirtData (
  IN  VOID                        *Ptr,
  IN  UINT64                      GP
  );

SAL_RETURN_REGS
LibGetGlobalPhyData (
  );

SAL_RETURN_REGS
LibGetGlobalVirtData (
  );

SAL_RETURN_REGS
LibSetGlobalPhyData (
  IN  VOID                        *Ptr,
  IN  UINT64                      GP
  );

SAL_RETURN_REGS
LibSetGlobalVirtData (
  IN  VOID                        *Ptr,
  IN  UINT64                      GP
  );

SAL_RETURN_REGS
GetIrrData (
  );

VOID
PrepareApsForHandOverToOS(
  );

VOID
HandOverApsToOS (
  IN UINT64  a1,
  IN UINT64  a2,
  IN UINT64  a3
  );



SAL_RETURN_REGS
GetPsrData (
  );

SAL_RETURN_REGS
GetProcIdData (
  );

VOID
SwitchCpuStack (
  IN  UINT64  NewBsp,
  IN  UINT64  OldBsp
  );

//
//  SAL Reset Class
//

VOID
SalResetSystem (
  IN EFI_RESET_TYPE                ResetType,
  IN EFI_STATUS                    ResetStatus,
  IN UINTN                        DataSize,
  IN CHAR16                        *ResetData
);

//
//  PAL PROC Class
//

SAL_RETURN_REGS
SalPalProc (
  IN  UINT64            Arg1,
  IN  UINT64            Arg2,
  IN  UINT64            Arg3,
  IN  UINT64            Arg4
);

SAL_RETURN_REGS
SalRegisterNewPalEntry (
  IN  BOOLEAN                     PhysicalPalAddress,
  IN  EFI_PHYSICAL_ADDRESS        NewPalAddress
);

SAL_RETURN_REGS
SalGetPalEntryPointer (
  IN  BOOLEAN                     PhysicalPalAddress
);

//
//  SAL MTC Class
//

EFI_STATUS
SalGetNextHighMonotonicCount (
  OUT UINT32                      *HighCount
);

//
//  SAL BASE Class
//

SAL_RETURN_REGS
SalProcSetVectors (
  IN  UINT64                      SalVectorType,
  IN  UINT64                      PhyAddr1,
  IN  UINT64                      Gp1,
  IN  UINT64                      LengthCs1,
  IN  UINT64                      PhyAddr2,
  IN  UINT64                      Gp2,
  IN  UINT64                      LengthCs2
);

SAL_RETURN_REGS
SalProcMcRendez (
);

SAL_RETURN_REGS
SalProcMcSetParams (
  IN  UINT64                      ParamType,
  IN  UINT64                      IntOrMem,
  IN  UINT64                      IntOrMemVal,
  IN  UINT64                      Timeout,
  IN  UINT64                      McaOpt
);

SAL_RETURN_REGS
EsalProcGetVectors (
  IN  UINT64                      VectorType
);

SAL_RETURN_REGS
EsalProcMcGetParams (
  IN  UINT64                      ParamInfoType
);

SAL_RETURN_REGS
EsalProcMcGetMcParams (
);

SAL_RETURN_REGS
EsalProcGetMcCheckinFlags (
  IN  UINT64                      ProcessorUnit
);

//
//  Sal Base Class enums
//

typedef enum {
  McaVector,
  BspInitVector,
  BootRendezVector,
  ApInitVector
} ESAL_GET_VECTOR_TYPE;

//
//  Sal RTC Class
//

EFI_STATUS
SalGetTime (
  OUT EFI_TIME                    *Time,
  OUT EFI_TIME_CAPABILITIES        *Capabilities 
);

EFI_STATUS
SalSetTime (
  OUT EFI_TIME                    *Time
);

EFI_STATUS
SalGetWakeupTime (
  OUT BOOLEAN                      *Enabled,
  OUT BOOLEAN                      *Pending,
  OUT EFI_TIME                    *Time
);

EFI_STATUS
SalSetWakeupTime (
  IN BOOLEAN                      Enable,
  IN EFI_TIME                     *Time      
);

SAL_RETURN_REGS
SalInitializeThreshold (
  IN  VOID                        *ThresholdStruct,
  IN  UINT64                      Count,
  IN  UINT64                      Duration
);

SAL_RETURN_REGS
SalBumpThresholdCount (
  IN  VOID                        *ThresholdStruct,
  IN  UINT64                      Count,
  IN  UINT64                      Duration
);

SAL_RETURN_REGS
SalGetThresholdCount (
  IN  VOID                        *ThresholdStruct,
  IN  UINT64                      Count,
  IN  UINT64                      Duration
);

//
//  Common Lib Function
//

EFI_STATUS
RegisterEsalFunction (
  IN  UINT64                                    FunctionId,
  IN  EFI_GUID                                  *ClassGuid,
  IN  SAL_INTERNAL_EXTENDED_SAL_PROC            Function,
  IN  VOID                                      *ModuleGlobal
  );

EFI_STATUS
EfiInitializeSalDriverLib (
  IN  BOOLEAN   Runtime
  );

//
// MCA PMI INIT Registeration Functions.
//

EFI_STATUS
LibRegisterMcaFunction (
  IN  EFI_SAL_MCA_HANDLER                   McaHandler,
  IN  VOID                                  *ModuleGlobal,
  IN  BOOLEAN                               MakeFirst,
  IN  BOOLEAN                               MakeLast
  );

EFI_STATUS
LibRegisterPmiFunction (
  IN  EFI_SAL_PMI_HANDLER                   PmiHandler,
  IN  VOID                                  *ModuleGlobal,
  IN  BOOLEAN                               MakeFirst,
  IN  BOOLEAN                               MakeLast
  );

EFI_STATUS
LibRegisterInitFunction (
  IN  EFI_SAL_INIT_HANDLER                  InitHandler,
  IN  VOID                                  *ModuleGlobal,
  IN  BOOLEAN                               MakeFirst,
  IN  BOOLEAN                               MakeLast
  );

//
//  Base IO Class Functions
//

EFI_STATUS
ESalIoRead (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                     Address,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
);

EFI_STATUS
ESalIoWrite (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                     Address,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
);

EFI_STATUS
ESalMemRead (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                      Address,
  IN     UINTN                      Count,
  IN  OUT VOID                      *Buffer
);

EFI_STATUS
ESalMemWrite (
  IN     EFI_CPU_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                      Address,
  IN     UINTN                      Count,
  IN OUT VOID                        *Buffer
);

//
//  PCI Class Functions
//

SAL_RETURN_REGS 
SalPCIConfigRead (
  IN  UINT64              Address,
  IN  UINT64              Size
);

SAL_RETURN_REGS 
SalPCIConfigWrite (
  IN  UINT64              Address,
  IN  UINT64              Size,
  IN  UINT64              Value
);

//
//  MP Class Functions
//

SAL_RETURN_REGS
LibMPAddCpuData (
  IN    UINT64      CpuGlobalId,
  IN    BOOLEAN     Enabled,
  IN    UINT64      PalCompatability
  );

SAL_RETURN_REGS
LibMPRemoveCpuData (
  IN    UINT64      CpuGlobalId
  );

SAL_RETURN_REGS
LibMPModifyCpuData (
  IN    UINT64      CpuGlobalId,
  IN    BOOLEAN     Enabled,
  IN    UINT64      PalCompatability
  );

SAL_RETURN_REGS
LibMPGetCpuDataByID (
  IN    UINT64      CpuGlobalId,
  IN    BOOLEAN     IndexByEnabledCpu
  );

SAL_RETURN_REGS
LibMPGetCpuDataByIndex (
  IN    UINT64      Index,
  IN    BOOLEAN     IndexByEnabledCpu
  );

SAL_RETURN_REGS
LibMPSendIpi (
  IN  UINT64                ProcessorNumber,
  IN  UINT64                VectorNumber,
  IN  EFI_DELIVERY_MODE     DeliveryMode,
  IN  BOOLEAN               IRFlag
  );

SAL_RETURN_REGS
LibMpCurrentProcessor (
  IN    BOOLEAN     IndexByEnabledCpu
  );

SAL_RETURN_REGS
LibGetNumProcessors (
  );

SAL_RETURN_REGS
LibMpSaveMinStatePointer (
  IN    UINT64                CpuGlobalId,
  IN    EFI_PHYSICAL_ADDRESS  MinStatePointer
  );

SAL_RETURN_REGS
LibMpRestoreMinStatePointer (
  IN    UINT64                CpuGlobalId
  );
//
//  MCA Class Functions
//

EFI_STATUS
LibMcaGetStateInfo (
  IN  UINT64                                      CpuId,
  OUT EFI_PHYSICAL_ADDRESS                        *StateBufferPointer,
  OUT UINT64                                      *RequiredStateBufferSize
);

EFI_STATUS
LibMcaRegisterCpu (
  IN  UINT64                                      CpuId,
  IN  EFI_PHYSICAL_ADDRESS                        StateBufferAddress
);

//
//  MCA Class Functions
//

EFI_STATUS
LibMcaGetStateInfo (
  IN  UINT64                                      CpuId,
  OUT EFI_PHYSICAL_ADDRESS                        *StateBufferPointer,
  OUT UINT64                                      *RequiredStateBufferSize
  );

EFI_STATUS
LibMcaRegisterCpu (
  IN  UINT64                                      CpuId,
  IN  EFI_PHYSICAL_ADDRESS                        StateBufferAddress
  );

//
// SAL ELOG Functions
//
EFI_STATUS
LibSalGetStateInfo (
  IN  UINT64                                      McaType,
  IN  UINT8                                       *McaBuffer,
  OUT UINTN                                       *Size  
  );

EFI_STATUS
LibSalGetStateInfoSize (
  IN  UINT64                                      McaType,
  OUT UINTN                                       *Size  
  );

EFI_STATUS
LibSalClearStateInfo (
  IN  UINT64                                      McaType
  );

EFI_STATUS
LibEsalGetStateBuffer (
  IN  UINT64                                      McaType,
  OUT UINT8                                       **McaBuffer,
  OUT UINTN                                       *Index
  );

EFI_STATUS
LibEsalSaveStateBuffer (
  IN  UINT64                                      McaType
  );

#endif
