/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PeiLib.h

Abstract:

  PEI Library Functions
 
--*/

#ifndef _PEI_LIB_H_
#define _PEI_LIB_H_

#include "Tiano.h"
#include "Pei.h"
#include "peiHobLib.h"
#include EFI_PROTOCOL_DEFINITION (Decompress)
#include EFI_PROTOCOL_DEFINITION (TianoDecompress)
#include EFI_GUID_DEFINITION (PeiPeCoffLoader)


VOID
PeiCopyMem (
  IN VOID   *Destination,
  IN VOID   *Source,
  IN UINTN  Length
  );

VOID
ZeroMem (
  IN VOID   *Buffer,
  IN UINTN  Size
  );

VOID
CopyMem (
  IN VOID   *Destination,
  IN VOID   *Source,
  IN UINTN  Length
  );

BOOLEAN
CompareGuid (
  IN EFI_GUID     *Guid1,
  IN EFI_GUID     *Guid2
  );


EFI_STATUS
InstallEfiPeiPeCoffLoader(
  IN EFI_PEI_SERVICES                 **PeiServices,
  IN EFI_PEI_PE_COFF_LOADER_PROTOCOL  **This,
  IN EFI_PEI_PPI_DESCRIPTOR               *ThisPpi
  );

EFI_STATUS
InstallEfiDecompress(
  EFI_DECOMPRESS_PROTOCOL  **This
  );

EFI_STATUS
InstallTianoDecompress(
  EFI_TIANO_DECOMPRESS_PROTOCOL  **This
  );
  
VOID
PeiPerfMeasure(
  EFI_PEI_SERVICES              **PeiServices,
  IN UINT16                     *Token,
  IN EFI_FFS_FILE_HEADER        *FileHeader,
  IN BOOLEAN                    EntryExit,
  IN UINT64                     Value
  );

EFI_STATUS
GetTimerValue (
  OUT UINT64    *TimerValue
  );

#ifdef EFI_PEI_PERFORMANCE
  #define PEI_PERF_START(Ps, Token, FileHeader, Value) PeiPerfMeasure(Ps, Token, FileHeader, FALSE, Value)
  #define PEI_PERF_END(Ps, Token, FileHeader, Value) PeiPerfMeasure(Ps, Token, FileHeader, TRUE, Value)
#else
  #define PEI_PERF_START(Ps, Token, FileHeader, Value) 
  #define PEI_PERF_END(Ps, Token, FileHeader, Value) 
#endif



#ifdef EFI_NT_EMULATOR
EFI_STATUS
PeCoffLoaderWinNtLoadAsDll (
  IN  CHAR8  *PdbFileName,
  IN  VOID   **ImageEntryPoint,
  OUT VOID **ModHandle
  );

#endif


//
// hob.c
//

EFI_STATUS
PeiBuildHobModule (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_GUID                    *ModuleName,
  IN EFI_PHYSICAL_ADDRESS        Module,
  IN UINT64                      ModuleLength,
  IN EFI_PHYSICAL_ADDRESS        EntryPoint
  );

EFI_STATUS
PeiBuildHobResourceDescriptor (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_RESOURCE_TYPE           ResourceType,
  IN EFI_RESOURCE_ATTRIBUTE_TYPE ResourceAttribute,
  IN EFI_PHYSICAL_ADDRESS        PhysicalStart,
  IN UINT64                      NumberOfBytes
  );

EFI_STATUS
PeiBuildHobGuid (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_GUID                    *Guid,
  IN UINTN                       DataLength,
  IN OUT VOID                    **Hob
  );

EFI_STATUS
PeiBuildHobGuidData (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_GUID                    *Guid,
  IN VOID                        *Data,
  IN UINTN                       DataLength
  );

EFI_STATUS
PeiBuildHobFv (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length
  );

EFI_STATUS
PeiBuildHobCpu (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN UINT8                       SizeOfMemorySpace,
  IN UINT8                       SizeOfIoSpace
  );

EFI_STATUS
PeiBuildHobStack (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length
  );

EFI_STATUS
PeiBuildHobBspStore (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length,
  IN EFI_MEMORY_TYPE             MemoryType
  );

EFI_STATUS
PeiBuildHobMemoryAllocation (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length,
  IN EFI_GUID                    *Name,
  IN EFI_MEMORY_TYPE             MemoryType
  );


//
// print.c
//

UINTN
AvSPrint (
  OUT CHAR8       *StartOfBuffer,
  IN  UINTN       StrSize,
  IN  CONST CHAR8 *Format,
  IN  VA_LIST     Marker
  );

UINTN
ASPrint (
  OUT CHAR8       *Buffer,
  IN UINTN        BufferSize,
  IN CONST CHAR8  *Format,
  ...
  );


//
// math.c
//

UINT64
MultU64x32 (
  IN  UINT64  Multiplicand,
  IN  UINTN   Multiplier
  );

UINT64
DivU64x32 (
  IN  UINT64  Dividend,
  IN  UINTN   Divisor,
  OUT UINTN   *Remainder  OPTIONAL
  );

UINT64
RShiftU64 (
  IN  UINT64  Operand,
  IN  UINTN   Count
  );

UINT64
LShiftU64 (
  IN  UINT64  Operand,
  IN  UINTN   Count
  );


VOID
RegisterNativeCpuIo (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN VOID                     *CpuIo
);

VOID
GetNativeCpuIo (
  IN EFI_PEI_SERVICES         **PeiServices,
  OUT VOID                    **CpuIo
);

#endif