/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PeHotRelocateEx.c

Abstract:

  Stub that handles X64 specific relocation types

--*/

#include "Runtime.h"

EFI_STATUS
PeHotRelocateImageEx (
  IN     UINT16  *Reloc,
  IN OUT CHAR8   *Fixup,
  IN OUT CHAR8   **FixupData,
  IN     UINT64  Adjust
  )
/*++

Routine Description:

  Performs X64 specific relocation fixup

Arguments:

  Reloc      - Pointer to the relocation record
  Fixup      - Pointer to the address to fix up
  FixupData  - Pointer to a buffer to log the fixups
  Adjust     - The offset to adjust the fixup

Returns:

  EFI_SUCCESS     - The relocation item fixed up
  EFI_UNSUPPORTED - The relocation item is of unsupported relocation type
  
--*/
{
  UINT64      *F64;

  switch ((*Reloc) >> 12) {
    case EFI_IMAGE_REL_BASED_DIR64:
      F64 = (UINT64 *) Fixup;
      *FixupData = ALIGN_POINTER(*FixupData, sizeof(UINT64));
      if (*(UINT64 *)(*FixupData) == *F64) {
        *F64  = *F64 + (UINT64) Adjust;
      }

      *FixupData = *FixupData + sizeof(UINT64);
      break;

    default:
      return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

