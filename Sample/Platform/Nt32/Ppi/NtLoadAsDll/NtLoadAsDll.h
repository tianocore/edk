/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

 NtLoadAsDll.h

Abstract:

  Nt service Ppi that is used to load PE32s in the NT emulation environment.

--*/

#ifndef _NT_LOAD_AS_DLL_H_
#define _NT_LOAD_AS_DLL_H_

#include "Tiano.h"

#define EFI_NT_LOAD_AS_DLL_PPI_GUID \
  { 0xccc53f6b, 0xa03a, 0x4ed8, 0x83, 0x9a, 0x3, 0xd9, 0x9c, 0x2, 0xb4, 0xe3 }

EFI_FORWARD_DECLARATION (EFI_NT_LOAD_AS_DLL_PPI);

typedef 
EFI_STATUS
(EFIAPI *EFI_NT_LOAD_AS_DLL) (
  IN CHAR8    *PdbFileName,
  IN VOID     **ImageEntryPoint,
  OUT VOID    **ModHandle
  );

typedef
EFI_STATUS
(EFIAPI *EFI_NT_FREE_LIBRARY) (
  VOID        *ModHandle
  );


typedef struct _EFI_NT_LOAD_AS_DLL_PPI {
  EFI_NT_LOAD_AS_DLL      Entry;
  EFI_NT_FREE_LIBRARY     FreeLibrary;
} EFI_NT_LOAD_AS_DLL_PPI;

extern EFI_GUID gEfiNtLoadAsDllPpiGuid;

#endif
