/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
  EfiJump.h

Abstract:

  This is the Setjump/Longjump pair for an IA32 processor.

--*/

#ifndef _EFI_JUMP_H_
#define _EFI_JUMP_H_

#include EFI_GUID_DEFINITION(PeiTransferControl)


//  NOTE:Set/LongJump needs to have this buffer start
//  at 16 byte boundary. Either fix the structure
//  which call this buffer or fix inside SetJump/LongJump
//  Choosing 1K buffer storage for now 

typedef struct {
  CHAR8       Buffer[1024];   
} EFI_JUMP_BUFFER;


EFI_STATUS
SetJump (
  IN EFI_PEI_TRANSFER_CONTROL_PROTOCOL  *This,
  IN EFI_JUMP_BUFFER  *Jump
  );

EFI_STATUS
LongJump (
  IN EFI_PEI_TRANSFER_CONTROL_PROTOCOL  *This,
  IN EFI_JUMP_BUFFER  *Jump
  );

VOID
RtPioICacheFlush (
    IN  VOID    *StartAddress,
    IN  UINTN   SizeInBytes
    );

#endif
