/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
  CustomizedDecompress.h

Abstract:

  Header file for Customized decompression routine
  
--*/
#ifndef _CUSTOMIZED_DECOMPRESS_LIB_H_
#define _CUSTOMIZED_DECOMPRESS_LIB_H_

#include EFI_PROTOCOL_DEFINITION(CustomizedDecompress)

EFI_STATUS
EFIAPI
CustomizedGetInfo (
  IN EFI_CUSTOMIZED_DECOMPRESS_PROTOCOL     *This,
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  OUT     UINT32  *DstSize,
  OUT     UINT32  *ScratchSize
  );
  
EFI_STATUS
EFIAPI
CustomizedDecompress (
  IN EFI_CUSTOMIZED_DECOMPRESS_PROTOCOL     *This,
  IN      VOID    *Source,
  IN      UINT32  SrcSize,
  IN OUT  VOID    *Destination,
  IN      UINT32  DstSize,
  IN OUT  VOID   *Scratch,
  IN      UINT32  ScratchSize
  );

EFI_STATUS
InstallCustomizedDecompress(
  EFI_CUSTOMIZED_DECOMPRESS_PROTOCOL  **This
);
#endif