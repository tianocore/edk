/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Performance.h

Abstract:

  This file included the performance relete function header and 
  definition.

--*/

#ifndef _PERF_H_
#define _PERF_H_

#include "BmMachine.h"

#include EFI_ARCH_PROTOCOL_CONSUMER (Cpu)
#include EFI_PROTOCOL_DEFINITION (Performance)
#include EFI_PROTOCOL_DEFINITION (LoadedImage)

#include EFI_GUID_DEFINITION (Acpi)
#include EFI_GUID_DEFINITION (GenericVariable)

#define EFI_PERF_PEI_ENTRY_MAX_NUM  50

typedef struct {
  CHAR8   Token[EFI_PERF_PDBFILENAME_LENGTH];
  UINT32  Duration;
} EFI_PERF_DATA;

typedef struct {
  UINT64        BootToOs;
  UINT64        S3Resume;
  UINT32        S3EntryNum;
  EFI_PERF_DATA S3Entry[EFI_PERF_PEI_ENTRY_MAX_NUM];
  UINT64        CpuFreq;
  UINT64        BDSRaw;
  UINT32        Count;
  UINT32        Signiture;
} EFI_PERF_HEADER;

VOID
WriteBootToOsPerformanceData (
  VOID
  );

VOID
ClearDebugRegisters (
  VOID
  );

#endif
