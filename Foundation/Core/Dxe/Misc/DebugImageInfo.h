/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DebugImageInfo.h
    
Abstract:

  Support functions for managing debug image info table when loading and unloading
  images.

--*/

#ifndef __DEBUG_IMAGE_INFO_H__
#define __DEBUG_IMAGE_INFO_H__

#include EFI_PROTOCOL_DEFINITION(LoadedImage)
#include EFI_GUID_DEFINITION(DebugImageInfoTable)

#define FOUR_MEG_PAGES  0x400  
#define FOUR_MEG_MASK   ((FOUR_MEG_PAGES * EFI_PAGE_SIZE) - 1)

#define EFI_DEBUG_TABLE_ENTRY_SIZE       (sizeof (VOID *))

VOID
CoreInitializeDebugImageInfoTable (
  VOID
  );

VOID
CoreUpdateDebugTableCrc32 (
  VOID
  );

VOID
CoreNewDebugImageInfoEntry (
  UINTN ImageInfoType,
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage,
  EFI_HANDLE ImageHandle
  );

VOID
CoreRemoveDebugImageInfoEntry (
  EFI_HANDLE ImageHandle
  );

#endif
