/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ImageRead.h

Abstract:

--*/

#ifndef _IMAGEREAD_H
#define _IMAGEREAD_H
EFI_STATUS
GetImageReadFunction (
  IN      EFI_PEI_SERVICES                      **PeiServices,
  IN      EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
;

EFI_STATUS
PeiImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINTN   *ReadSize,
  OUT    VOID    *Buffer
  )
;

EFI_STATUS
InstallEfiPeiTransferControl (
  IN OUT EFI_PEI_TRANSFER_CONTROL_PROTOCOL **This
  )
;

EFI_STATUS
InstallEfiPeiFlushInstructionCache (
  IN OUT EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  **This
  )
;

#endif