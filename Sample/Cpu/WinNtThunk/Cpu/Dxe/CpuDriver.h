/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CpuDriver.h

Abstract:

  NT Emulation Architectural Protocol Driver as defined in EFI 2.0.

--*/

#ifndef _CPU_ARCHITECTURAL_PROTOCOL_DRIVER_H_
#define _CPU_ARCHITECTURAL_PROTOCOL_DRIVER_H_

extern UINT8 STRING_ARRAY_NAME[];

//
// Internal Data Structures
//

#define CPU_ARCH_PROT_PRIVATE_SIGNATURE   EFI_SIGNATURE_32('c','a','p','d')

typedef struct {
  UINTN                       Signature;
  EFI_HANDLE                  Handle;

  EFI_CPU_ARCH_PROTOCOL       Cpu;

  //
  // Local Data for CPU interface goes here
  //
  CRITICAL_SECTION            NtCriticalSection;
  BOOLEAN                     InterruptState;

} CPU_ARCH_PROTOCOL_PRIVATE;

#define CPU_ARCH_PROTOCOL_PRIVATE_DATA_FROM_THIS(a) \
         CR(a, CPU_ARCH_PROTOCOL_PRIVATE, Cpu, CPU_ARCH_PROT_PRIVATE_SIGNATURE)


#endif
