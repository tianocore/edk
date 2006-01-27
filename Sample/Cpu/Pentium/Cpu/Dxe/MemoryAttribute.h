/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:  
    MemoryAttribute.h
  
Abstract:
  
 
Revision History:

--*/

#ifndef _EFI_MEMORY_ATTRIB_H
#define _EFI_MEMORY_ATTRIB_H

typedef struct {
  UINT32    Msr;
  UINT32    BaseAddress;
  UINT32    Length;
} EFI_FIXED_MTRR;


typedef struct {
  UINT64    BaseAddress;
  UINT64    Length;
  UINT64    Type;
  UINT32    Msr;
  BOOLEAN   Valid;
} EFI_VARIABLE_MTRR;

#define EFI_FIXED_MTRR_NUMBER           11

extern  UINT32            mUsedMtrr;

EFI_STATUS
ProgramFixedMtrr (
  IN  UINT64                    MemoryCacheType,
  IN  UINT64                    *Base,
  IN  UINT64                    *Length
  );

VOID PreMtrrChange(
  VOID
  );

VOID PostMtrrChange(
  VOID
  );

EFI_STATUS 
GetMemoryAttribute(
  VOID
  );

BOOLEAN 
CheckMemoryAttributeOverlap (
  IN  EFI_PHYSICAL_ADDRESS      Start,
  IN  EFI_PHYSICAL_ADDRESS      End
  );

EFI_STATUS
CombineMemoryAttribute (
  IN  UINT64                    Attribute,
  IN  UINT64                    *Base,
  IN  UINT64                    *Length
  );

EFI_STATUS
GetDirection (
  IN  UINT64                    Input,
  IN  UINT32                    *MtrrNumber,
  IN  BOOLEAN                   *Direction
  );

UINT64
Power2MaxMemory (
  IN UINT64                     MemoryLength
  );

EFI_STATUS
InvariableMtrr (
  IN  UINT32                    MtrrNumber,
  IN  UINT32                    Index
  );

EFI_STATUS
ProgramVariableMtrr(
  IN  UINT32                    MtrrNumber,
  IN  EFI_PHYSICAL_ADDRESS      BaseAddress,
  IN  UINT64                    Length,
  IN  UINT64                    MemoryCacheType
  );


EFI_STATUS
CleanupVariableMtrr (
  VOID
  );

#endif
