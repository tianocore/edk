/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LegacyBiosThunk.h
    
Abstract:

  The EFI Legacy BIOS Protocol is used to abstract legacy Option ROM usage
  under EFI and Legacy OS boot.

  Note: The names for EFI_IA32_REGISTER_SET elements were picked to follow 
  well known naming conventions.

  Thunk - A thunk is a transition from one processor mode to another. A Thunk
          is a transition from native EFI mode to 16-bit mode. A reverse thunk
          would be a transition from 16-bit mode to native EFI mode.


  Note: Note: Note: Note: Note: Note: Note:

  You most likely should not use this protocol! Find the EFI way to solve the
  problem to make your code portable

  Note: Note: Note: Note: Note: Note: Note:

Revision History

--*/

#ifndef __LEGACY_BIOS_THUNK_H__
#define __LEGACY_BIOS_THUNK_H__

#define LEGACY_BIOS_THUNK_PROTOCOL_GUID \
  { 0x4c51a7ba, 0x7195, 0x442d, 0x87, 0x92, 0xbe, 0xea, 0x6e, 0x2f, 0xf6, 0xec }

EFI_FORWARD_DECLARATION (LEGACY_BIOS_THUNK_PROTOCOL);

//
// Convert from address (_Adr) to Segment:Offset 16-bit form
//
#define EFI_SEGMENT(_Adr)     (UINT16) ((UINT16) (((UINTN) (_Adr)) >> 4) & 0xf000)
#define EFI_OFFSET(_Adr)      (UINT16) (((UINT16) ((UINTN) (_Adr))) & 0xffff)
#define BYTE_GRANULARITY      0x01
#define WORD_GRANULARITY      0x02
#define DWORD_GRANULARITY     0x04
#define QWORD_GRANULARITY     0x08
#define PARAGRAPH_GRANULARITY 0x10

#define CARRY_FLAG            0x01

typedef struct {
  UINT16  CF : 1;
  UINT16  Reserved1 : 1;
  UINT16  PF : 1;
  UINT16  Reserved2 : 1;
  UINT16  AF : 1;
  UINT16  Reserved3 : 1;
  UINT16  ZF : 1;
  UINT16  SF : 1;
  UINT16  TF : 1;
  UINT16  IF : 1;
  UINT16  DF : 1;
  UINT16  OF : 1;
  UINT16  IOPL : 2;
  UINT16  NT : 1;
  UINT16  Reserved4 : 1;
} EFI_FLAGS_REG;

typedef struct {
  UINT16        AX;
  UINT16        BX;
  UINT16        CX;
  UINT16        DX;
  UINT16        SI;
  UINT16        DI;
  EFI_FLAGS_REG Flags;
  UINT16        ES;
  UINT16        CS;
  UINT16        SS;
  UINT16        DS;

  UINT16        BP;
} EFI_WORD_REGS;

typedef struct {
  UINT8 AL;
  UINT8 AH;
  UINT8 BL;
  UINT8 BH;
  UINT8 CL;
  UINT8 CH;
  UINT8 DL;
  UINT8 DH;
} EFI_BYTE_REGS;

typedef union {
  EFI_WORD_REGS X;
  EFI_BYTE_REGS H;
} EFI_IA32_REGISTER_SET;


typedef
BOOLEAN
(EFIAPI *LEGACY_BIOS_THUNK_INT86) (
  IN LEGACY_BIOS_THUNK_PROTOCOL       *This,
  IN  UINT8                           BiosInt,
  IN OUT  EFI_IA32_REGISTER_SET       *Regs
  )
/*++

  Routine Description:
    Thunk to 16-bit real mode and execute a software interrupt with a vector 
    of BiosInt. Regs will contain the 16-bit register context on entry and 
    exit.

  Arguments:
    This    - Protocol instance pointer.
    BiosInt - Processor interrupt vector to invoke
    Reg     - Register contexted passed into (and returned) from thunk to 
              16-bit mode

  Returns:
    FALSE   - Thunk completed, and there were no BIOS errors in the target code.
              See Regs for status.
    TRUE    - There was a BIOS erro in the target code.

--*/
;

typedef
BOOLEAN
(EFIAPI *LEGACY_BIOS_THUNK_FARCALL86) (
  IN LEGACY_BIOS_THUNK_PROTOCOL       *This,
  IN  UINT16                          Segment,
  IN  UINT16                          Offset,
  IN  EFI_IA32_REGISTER_SET           *Regs,
  IN  VOID                            *Stack,
  IN  UINTN                           StackSize
  )
/*++

  Routine Description:
    Thunk to 16-bit real mode and call Segment:Offset. Regs will contain the 
    16-bit register context on entry and exit. Arguments can be passed on 
    the Stack argument

  Arguments:
    This      - Protocol instance pointer.
    Segment   - Segemnt of 16-bit mode call
    Offset    - Offset of 16-bit mdoe call
    Reg       - Register contexted passed into (and returned) from thunk to 
                16-bit mode
    Stack     - Caller allocated stack used to pass arguments
    StackSize - Size of Stack in bytes

  Returns:
    FALSE     - Thunk completed, and there were no BIOS errors in the target code.
                See Regs for status.
    TRUE      - There was a BIOS erro in the target code.

--*/
;



typedef struct _LEGACY_BIOS_THUNK_PROTOCOL {
  LEGACY_BIOS_THUNK_INT86       Int86;
  LEGACY_BIOS_THUNK_FARCALL86   FarCall86;
} LEGACY_BIOS_THUNK_PROTOCOL;

extern EFI_GUID gLegacyBiosThunkProtocolGuid;

#endif
