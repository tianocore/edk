/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  FSVariable.h
  
Abstract:

--*/

#ifndef _FS_VARIABLE_H
#define _FS_VARIABLE_H

//
// Statements that include other header files
//
#include "Tiano.h"
#include "EfiVariable.h"
#include "VariableStorage.h"
#include "VarMachine.h"
#include EFI_GUID_DEFINITION (GlobalVariable)

#define VAR_V_MASK                  (0)
#define VAR_NV_MASK                 (1 << 31)
#define VARIABLE_STORE_SIZE         (64 * 1024)
#define VARIABLE_SCRATCH_SIZE       (4 * 1024)
#define VARIABLE_RECLAIM_THRESHOLD  (1024)

//
// Define GET_PAD_SIZE to optimize compiler
//
#if ((ALIGNMENT == 0) || (ALIGNMENT == 1))
#define GET_PAD_SIZE(a) (0)
#else
#define GET_PAD_SIZE(a) (((~a) + 1) & (ALIGNMENT - 1))
#endif

#define GET_VARIABLE_NAME_PTR(a)  (CHAR16 *) ((UINTN) (a) + sizeof (VARIABLE_HEADER))

typedef enum {
  Physical,
  Virtual
} VARIABLE_POINTER_TYPE;

typedef enum {
  NonVolatile,
  Volatile,
  MaxType
} VARIABLE_STORAGE_TYPE;

typedef struct {
  VARIABLE_HEADER         *CurrPtr;
  VARIABLE_HEADER         *EndPtr;
  VARIABLE_HEADER         *StartPtr;
  VARIABLE_STORAGE_TYPE   Type;
} VARIABLE_POINTER_TRACK;

#define VARIABLE_MEMBER_OFFSET(Member, StartOffset) \
        ( sizeof (VARIABLE_STORE_HEADER) + (StartOffset) + \
          (UINTN) ((UINT8 *) &((VARIABLE_HEADER*) 0)->Member - (UINT8 *) &((VARIABLE_HEADER*) 0)->StartId) \
        )


typedef struct {
  VARIABLE_STORAGE   *VariableStore[MaxType];
  VOID               *VariableBase[MaxType];
  UINTN              LastVariableOffset[MaxType];
  VOID               *Scratch;
} VARIABLE_GLOBAL;

//
// Functions
//

EFI_STATUS
EFIAPI
VariableServiceInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
;

VOID
EFIAPI
VariableClassAddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
;

EFI_STATUS
EFIAPI
GetVariable (
  IN      CHAR16            *VariableName,
  IN      EFI_GUID          *VendorGuid,
  OUT     UINT32            *Attributes OPTIONAL,
  IN OUT  UINTN             *DataSize,
  OUT     VOID              *Data
  )
;

EFI_STATUS
EFIAPI
GetNextVariableName (
  IN OUT  UINTN             *VariableNameSize,
  IN OUT  CHAR16            *VariableName,
  IN OUT  EFI_GUID          *VendorGuid
  )
;

EFI_STATUS
EFIAPI
SetVariable (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes,
  IN UINTN                   DataSize,
  IN VOID                    *Data
  )
;

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
EFI_STATUS
EFIAPI
QueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
;
#endif // EFI_SPECIFICATION_VERSION >= 0x00020000

#endif
