/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Variable.h
   
Abstract:

  Tiano PEIM to provide the variable functionality

--*/

#ifndef _PEI_VARIABLE_H
#define _PEI_VARIABLE_H

#include "EfiVariable.h"
#include "VarMachine.h"

#include EFI_PPI_DEFINITION (FlashMap)
#include EFI_PPI_PRODUCER (Variable)
#include EFI_PPI_PRODUCER (Variable2)
//
// Define GET_PAD_SIZE to optimize compiler
//
#if ((ALIGNMENT == 0) || (ALIGNMENT == 1))
#define GET_PAD_SIZE(a) (0)
#else
#define GET_PAD_SIZE(a) (((~a) + 1) & (ALIGNMENT - 1))
#endif

#define GET_VARIABLE_NAME_PTR(a)  (CHAR16 *) ((UINTN) (a) + sizeof (VARIABLE_HEADER))

#define GET_VARIABLE_DATA_PTR(a) \
  (UINT8 *) ((UINTN) GET_VARIABLE_NAME_PTR (a) + (a)->NameSize + GET_PAD_SIZE ((a)->NameSize))

typedef struct {
  VARIABLE_HEADER *CurrPtr;
  VARIABLE_HEADER *EndPtr;
  VARIABLE_HEADER *StartPtr;
} VARIABLE_POINTER_TRACK;

#define VARIABLE_INDEX_TABLE_VOLUME 122

#define EFI_VARIABLE_INDEX_TABLE_GUID \
  { \
    0x8cfdb8c8, 0xd6b2, 0x40f3, 0x8e, 0x97, 0x02, 0x30, 0x7c, 0xc9, 0x8b, 0x7c \
  }

typedef struct {
  UINT16          Length;
  UINT16          GoneThrough;
  VARIABLE_HEADER *EndPtr;
  VARIABLE_HEADER *StartPtr;
  UINT16          Index[VARIABLE_INDEX_TABLE_VOLUME];
} VARIABLE_INDEX_TABLE;

extern EFI_GUID gEfiVariableIndexTableGuid;

//
// Functions
//
EFI_STATUS
EFIAPI
PeimInitializeVariableServices (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  FfsHeader   - TODO: add argument description
  PeiServices - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#if (PI_SPECIFICATION_VERSION < 0x00010000)

EFI_STATUS
EFIAPI
PeiGetVariable (
  IN EFI_PEI_SERVICES             **PeiServices,
  IN CHAR16                       *VariableName,
  IN EFI_GUID                     * VendorGuid,
  OUT UINT32                      *Attributes OPTIONAL,
  IN OUT UINTN                    *DataSize,
  OUT VOID                        *Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PeiServices   - TODO: add argument description
  VariableName  - TODO: add argument description
  VendorGuid    - TODO: add argument description
  Attributes    - TODO: add argument description
  DataSize      - TODO: add argument description
  Data          - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
PeiGetNextVariableName (
  IN EFI_PEI_SERVICES             **PeiServices,
  IN OUT UINTN                    *VariableNameSize,
  IN OUT CHAR16                   *VariableName,
  IN OUT EFI_GUID                 *VendorGuid
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PeiServices       - TODO: add argument description
  VariableNameSize  - TODO: add argument description
  VariableName      - TODO: add argument description
  VendorGuid        - TODO: add argument description

Returns:

  TODO: add return values

--*/
;
#else

EFI_STATUS
EFIAPI
PeiGetVariable (
  IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI    *This,
  IN CONST CHAR16                             *VariableName,
  IN CONST EFI_GUID                           *VariableGuid,
  OUT UINT32                                  *Attributes,
  IN OUT UINTN                                *DataSize,
  OUT VOID                                    *Data
  )
/*++

Routine Description:

  Provide the read variable functionality of the variable services.

Arguments:

  This             - Pointer to EFI_PEI_READ_ONLY_VARIABLE2_PPI.

  VariableName     - The variable name

  VendorGuid       - The vendor's GUID

  Attributes       - Pointer to the attribute

  DataSize         - Size of data

  Data             - Pointer to data

Returns:

  EFI_SUCCESS           - The interface could be successfully installed

  EFI_NOT_FOUND         - The variable could not be discovered

  EFI_BUFFER_TOO_SMALL  - The caller buffer is not large enough

--*/
;

EFI_STATUS
EFIAPI
PeiGetNextVariableName (
  IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI    *This,
  IN OUT UINTN                                *VariableNameSize,
  IN OUT CHAR16                               *VariableName,
  IN OUT EFI_GUID                             *VariableGuid
  )
/*++

Routine Description:

  Provide the get next variable functionality of the variable services.

Arguments:

  This             - Pointer to EFI_PEI_READ_ONLY_VARIABLE2_PPI.
  VariabvleNameSize  - The variable name's size.
  VariableName       - A pointer to the variable's name.
  VendorGuid         - A pointer to the EFI_GUID structure.

  VariableNameSize - Size of the variable name

  VariableName     - The variable name

  VendorGuid       - The vendor's GUID

Returns:

  EFI_SUCCESS - The interface could be successfully installed

  EFI_NOT_FOUND - The variable could not be discovered

--*/
;


#endif
#endif // _PEI_VARIABLE_H
