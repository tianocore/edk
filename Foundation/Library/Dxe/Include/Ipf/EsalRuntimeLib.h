/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EsalRuntimeLib.h
  
Abstract:

  SAL Driver Lib

Revision History

--*/

#ifndef _ESAL_RUNTIME_LIB_H_
#define _ESAL_RUNTIME_LIB_H_

#include "SalApi.h"
#include "EfiFirmwareVolumeHeader.h"

#include EFI_PROTOCOL_DEFINITION (ExtendedSalBootService)
#include EFI_PROTOCOL_DEFINITION (ExtendedSalGuid)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeBlock)

VOID
EsalRuntimeLibVirtualNotify (
  VOID
  );

EFI_STATUS
EsalInitializeRuntimeDriverLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable  
  );

SAL_RETURN_REGS
CallEsalService (
  IN  EFI_GUID                                      *ClassGuid,
  IN   UINT64                                        FunctionId,
  IN  UINT64                                        Arg2,
  IN  UINT64                                        Arg3,
  IN  UINT64                                        Arg4,
  IN  UINT64                                        Arg5,
  IN  UINT64                                        Arg6,
  IN  UINT64                                        Arg7,
  IN  UINT64                                        Arg8
  ); 

//
//  Assembly Functions
//

SAL_RETURN_REGS
EsalGetEntryPoint (
  VOID
  );

SAL_RETURN_REGS
EsalSetPhysicalEntryPoint (
  IN  UINT64  EntryPoint,
  IN  UINT64  Gp
  );

SAL_RETURN_REGS
EsalSetVirtualEntryPoint (
  IN  UINT64  EntryPoint,
  IN  UINT64  Gp
  );

SAL_RETURN_REGS
EsalSetPhysicalModuleGlobal (
  IN  VOID    *Global
  );

SAL_RETURN_REGS
EsalSetVirtualModuleGlobal (
  IN  VOID    *Global
  );

SAL_RETURN_REGS
EsalGetModuleGlobal (
  VOID
  );


SAL_RETURN_REGS
GetIrrData (
  VOID
  );

SAL_RETURN_REGS
GetPsrData (
  VOID
  );

SAL_RETURN_REGS
GetProcIdData (
  VOID
  );

VOID
SwitchCpuStack (
  IN  UINT64  NewBsp,
  IN  UINT64  OldBsp
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
  VOID
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
  VOID
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
RegisterEsalClass (
  IN  EFI_GUID                                  *ClassGuid,
  IN  VOID                                      *ModuleGlobal,
  ...
  );

//
//  MP Class Functions
//

SAL_RETURN_REGS
LibMpAddCpuData (
  IN    UINT64      CpuGlobalId,
  IN    BOOLEAN     Enabled,
  IN    UINT64      PalCompatability
  );

SAL_RETURN_REGS
LibMpRemoveCpuData (
  IN    UINT64      CpuGlobalId
  );

SAL_RETURN_REGS
LibMpModifyCpuData (
  IN    UINT64      CpuGlobalId,
  IN    BOOLEAN     Enabled,
  IN    UINT64      PalCompatability
  );

SAL_RETURN_REGS
LibMpGetCpuDataByID (
  IN    UINT64      CpuGlobalId,
  IN    BOOLEAN     IndexByEnabledCpu
  );

SAL_RETURN_REGS
LibMpGetCpuDataByIndex (
  IN    UINT64      Index,
  IN    BOOLEAN     IndexByEnabledCpu
  );

SAL_RETURN_REGS
LibMpSendIpi (
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
// FVB Variables Class
//

EFI_STATUS
EsalReadBlock (
  IN UINTN                                        Instance,
  IN EFI_LBA                                      Lba,
  IN UINTN                                        Offset,
  IN OUT UINTN                                    *NumBytes,
  IN UINT8                                        *Buffer
  );

EFI_STATUS
EsalWriteBlock (
  IN UINTN                                        Instance,
  IN EFI_LBA                                      Lba,
  IN UINTN                                        Offset,
  IN OUT UINTN                                    *NumBytes,
  IN UINT8                                        *Buffer
  );

EFI_STATUS
EsalEraseBlock (
  IN UINTN                                Instance,
  IN UINTN                                Lba
  );

EFI_STATUS
EsalGetVolumeAttributes (
  IN UINTN                                Instance,
  OUT EFI_FVB_ATTRIBUTES                  *Attributes
  );

EFI_STATUS
EsalSetVolumeAttributes (
  IN UINTN                                Instance,
  IN EFI_FVB_ATTRIBUTES                   Attributes
  );

EFI_STATUS
EsalGetPhysicalAddress (
  IN UINTN                                Instance,
  OUT EFI_PHYSICAL_ADDRESS                *Address
  );

EFI_STATUS
EsalGetBlockSize (
  IN UINTN                                Instance,
  IN EFI_LBA                              Lba,
  OUT UINTN                               *BlockSize,
  OUT UINTN                               *NumOfBlocks
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
