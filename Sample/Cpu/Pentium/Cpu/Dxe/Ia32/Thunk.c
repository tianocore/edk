/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Thunk.c

Abstract:

  Call into 16-bit BIOS code

  BugBug: Thunker does A20 gate. Can we get rid of this code or
          put it into Legacy16 code.

--*/
#include "CpuDxe.h"

extern EFI_LEGACY_8259_PROTOCOL          *gLegacy8259;

UINTN
CallRealThunkCode (
  UINT8               *RealCode,
  UINT8               BiosInt,
  UINT32              CallAddress
  );


//
// Thunk Status Codes
//   (These apply only to errors with the thunk and not to the code that was
//   thunked to.)
//
#define THUNK_OK              0x00
#define THUNK_ERR_A20_UNSUP   0x01
#define THUNK_ERR_A20_FAILED  0x02

#pragma pack (1)

#define LOW_STACK_SIZE      (8 * 1024)  // 8k
#define NUM_REAL_GDT_ENTRIES  5

//
// Define what a processor descriptor looks like
//
typedef struct {
  UINT16  Limit;
  UINT32  Base;
} DESCRIPTOR;

typedef struct {
  UINT16  LimitLow;
  UINT16  BaseLow;
  UINT8   BaseMid;
  UINT8   Attribute;
  UINT8   LimitHi;
  UINT8   BaseHi;
} GDT_ENTRY;

#define NULL_SELECTOR_ARRAY_INDEX             0
#define REAL_CODE_SELECTOR_ARRAY_INDEX        1
#define REAL_DATA_SELECTOR_ARRAY_INDEX        2
#define PROTECTED_CODE_SELECTOR_ARRAY_INDEX   3
#define PROTECTED_DATA_SELECTOR_ARRAY_INDEX   4


GDT_ENTRY gTrapoleanGdt[NUM_REAL_GDT_ENTRIES] = {
  { // Selector 0x00 - NULL
    0,  // LimitLow
    0,  // BaseLow
    0,  // BaseMid
    0,  // Attribute
    0,  // LimitHi
    0   // BaseHi
  },
  { // Selector 0x08 - Real Code 
    0xffff,  // LimitLow
    0x0000,  // BaseLow
    0x00,    // BaseMid
    0x9a,    // Attribute
    0x0f,    // LimitHi
    0x00     // BaseHi
  },
  { // Selector 0x10 - Real Data
    0xffff,  // LimitLow
    0x0000,  // BaseLow
    0x00,    // BaseMid
    0x92,    // Attribute
    0x8f,    // LimitHi
    0x00     // BaseHi
  },
  { // Selector 0x18 - Protected Code
    0xffff,  // LimitLow
    0x0000,  // BaseLow
    0x00,    // BaseMid
    0x9a,    // Attribute
    0xcf,    // LimitHi
    0x00     // BaseHi
  },
  { // Selector 0x20 - Protected Data
    0xffff,  // LimitLow
    0x0000,  // BaseLow
    0x00,    // BaseMid
    0x93,    // Attribute
    0xcf,    // LimitHi
    0x00     // BaseHi
  },
};

typedef struct {
  //
  // Space for the code
  //  The address of Code is also the beginning of the relocated Thunk code
  //
  CHAR8                             CodeBuffer[4096]; // ?

  //
  // Data for the code (cs releative)
  //
  DESCRIPTOR                        GdtDesc;          // Protected mode GDT
  DESCRIPTOR                        IdtDesc;          // Protected mode IDT
  UINT32                            FlatSs;
  UINT32                            FlatEsp;

  UINT32                            RealCodeSelector;  // Low code selector in GDT
  UINT32                            RealDataSelector;  // Low data selector in GDT
  UINT32                            RealStack;
  DESCRIPTOR                        RealModeIdtDesc;

  //
  // real-mode GDT (temporary GDT with two real mode segment descriptors)
  //
  GDT_ENTRY                         RealModeGdt[NUM_REAL_GDT_ENTRIES];
  DESCRIPTOR                        RealModeGdtDesc;

  //
  // A low memory stack
  //
  CHAR8                             Stack[LOW_STACK_SIZE];

} LOW_MEMORY_THUNK;

#pragma pack()

LOW_MEMORY_THUNK  *gIntThunk;

typedef
UINTN
(EFIAPI *CALL_REAL_MODE_THUNK) (
  IN UINTN            CallAddress,
  IN UINT8            BiosInt,
  IN LOW_MEMORY_THUNK *IntThunk
  );


VOID
GetRegisters (
  LOW_MEMORY_THUNK    *IntThunk
  );

VOID
RealModeTemplate (
  OUT UINT32          *CodeStart,
  OUT UINT32          *CodeEnd,
  IN LOW_MEMORY_THUNK *IntThunk
  );


VOID
InitializeBiosIntCaller (
  VOID
  )
/*++

  Routine Description:
    Initialize call infrastructure to 16-bit BIOS code

  Arguments:
    NONE

  Returns:
    NONE
--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  Address;
  UINT32                CodeStart;
  UINT32                CodeEnd;

  //
  // Grab a buffer under 1MB
  //
  Address = 0x00000FFFFF;
  Status  = gBS->AllocatePages (AllocateMaxAddress, EfiBootServicesData, 1 + (sizeof (LOW_MEMORY_THUNK)/4096), &Address);
  ASSERT_EFI_ERROR (Status);

  //
  // Clear the reserved descriptor
  //
  gIntThunk = (LOW_MEMORY_THUNK *)(UINTN)Address;
  EfiZeroMem (gIntThunk, sizeof (LOW_MEMORY_THUNK));

  EfiCopyMem (gIntThunk->RealModeGdt, gTrapoleanGdt, sizeof (gTrapoleanGdt));
  
  //
  // Compute selector value now that the GDT is under 1MB.
  //
  gIntThunk->RealModeGdtDesc.Limit = sizeof (gIntThunk->RealModeGdt) - 1;
  gIntThunk->RealModeGdtDesc.Base  = (UINT32)(UINTN)gIntThunk->RealModeGdt;
  
  gIntThunk->RealModeIdtDesc.Limit = 0xFFFF;
  gIntThunk->RealModeIdtDesc.Base  = 0;

  //
  // Must match gTrapoleanGdt
  //
  gIntThunk->RealCodeSelector       = REAL_CODE_SELECTOR_ARRAY_INDEX * 8;
  gIntThunk->RealDataSelector       = REAL_DATA_SELECTOR_ARRAY_INDEX * 8;

  //
  // Initialize low real-mode code thunk
  //
  RealModeTemplate (&CodeStart, &CodeEnd, gIntThunk);

  //
  // Copy Thunk code bellow 1 MB.
  //
  EfiCopyMem (gIntThunk->CodeBuffer, (VOID *)(UINTN)CodeStart, CodeEnd - CodeStart);
}


BOOLEAN
EFIAPI
CpuLegacyBiosInt86 (
  IN  LEGACY_BIOS_THUNK_PROTOCOL    *This,
  IN  UINT8                         BiosInt,
  IN  EFI_IA32_REGISTER_SET         *Regs
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
{
  UINTN                 Status;
  UINT16                *Stack16;
  EFI_TPL               OriginalTpl;
  CALL_REAL_MODE_THUNK  CallRealModeThunk;

  //
  // Get the current flat GDT, IDT, and SS and store them in gIntThunk.
  //
  GetRegisters (gIntThunk);

  //
  // Clear the error flag; thunk code may set it.
  //
  Regs->X.Flags.Reserved1 = 1;
  Regs->X.Flags.Reserved2 = 0;
  Regs->X.Flags.Reserved3 = 0;
  Regs->X.Flags.Reserved4 = 0;
  Regs->X.Flags.IOPL      = 3;
  Regs->X.Flags.NT        = 0;
  Regs->X.Flags.IF        = 1;
  Regs->X.Flags.TF        = 0;
  Regs->X.Flags.CF        = 0;

  //
  // Copy regs to low memory stack
  //
  Stack16 = (UINT16 *) (gIntThunk->Stack + LOW_STACK_SIZE);
  Stack16 -= sizeof (EFI_IA32_REGISTER_SET) / sizeof (UINT16);
  EfiCopyMem (Stack16, Regs, sizeof (EFI_IA32_REGISTER_SET));

  //
  // Provide low stack esp
  //
  gIntThunk->RealStack = ((UINT32) Stack16) - ((UINT32) gIntThunk);

  //
  // The call to Legacy16 is a critical section to EFI
  //
  OriginalTpl = gBS->RaiseTPL (EFI_TPL_HIGH_LEVEL);

  //
  // Set Legacy16 state. 0x08, 0x70 is legacy 8259 vector bases.
  //
  Status = gLegacy8259->SetMode (gLegacy8259, Efi8259LegacyMode, NULL, NULL);
  ASSERT_EFI_ERROR (Status);

  //
  // Call the real mode thunk code
  //
  CallRealModeThunk = (CALL_REAL_MODE_THUNK)(UINTN)gIntThunk->CodeBuffer;
  Status = CallRealModeThunk (0, BiosInt, gIntThunk);

  //
  // Check for errors with the thunk
  //
  switch (Status) {
  case THUNK_OK:
    break;

  case THUNK_ERR_A20_UNSUP:
  case THUNK_ERR_A20_FAILED:
  default:
    //
    // For all errors, set EFLAGS.CF (used by legacy BIOS to indicate error).
    //
    Regs->X.Flags.CF = 1;
    break;
  }

  //
  // Return the resulting registers
  //
  EfiCopyMem (Regs, Stack16, sizeof (EFI_IA32_REGISTER_SET));

  //
  // Restore protected mode interrupt state
  //
  Status = gLegacy8259->SetMode (gLegacy8259, Efi8259ProtectedMode, NULL, NULL);
  ASSERT_EFI_ERROR (Status);

  //
  // End critical section
  //
  gBS->RestoreTPL (OriginalTpl);

  return (BOOLEAN) (Regs->X.Flags.CF ? TRUE : FALSE);
}


BOOLEAN
EFIAPI
CpuLegacyBiosFarCall86 (
  IN  LEGACY_BIOS_THUNK_PROTOCOL      *This,
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
{
  UINTN                 Status;
  UINT32                CallAddress;
  UINT16                *Stack16;
  EFI_TPL               OriginalTpl;
  CALL_REAL_MODE_THUNK  CallRealModeThunk;

  //
  // Get the current flat GDT and IDT and store them in gIntThunk.
  //
  GetRegisters (gIntThunk);

  CallAddress = (Segment << 16) | Offset;

  //
  // Clear the error flag; thunk code may set it.
  //
  Regs->X.Flags.Reserved1 = 1;
  Regs->X.Flags.Reserved2 = 0;
  Regs->X.Flags.Reserved3 = 0;
  Regs->X.Flags.Reserved4 = 0;
  Regs->X.Flags.IOPL      = 3;
  Regs->X.Flags.NT        = 0;
  Regs->X.Flags.IF        = 1;
  Regs->X.Flags.TF        = 0;
  Regs->X.Flags.CF        = 0;

  Stack16 = (UINT16 *) (gIntThunk->Stack + LOW_STACK_SIZE);
  if (Stack != NULL && StackSize != 0) {
    //
    // Copy Stack to low memory stack
    //
    Stack16 -= StackSize / sizeof (UINT16);
    EfiCopyMem (Stack16, Stack, StackSize);
  }

  //
  // Copy regs to low memory stack
  //
  Stack16 -= sizeof (EFI_IA32_REGISTER_SET) / sizeof (UINT16);
  EfiCopyMem (Stack16, Regs, sizeof (EFI_IA32_REGISTER_SET));

  //
  // Provide low stack esp
  //
  gIntThunk->RealStack = ((UINT32) Stack16) - ((UINT32) gIntThunk);

  //
  // The call to Legacy16 is a critical section to EFI
  //
  OriginalTpl = gBS->RaiseTPL (EFI_TPL_HIGH_LEVEL);

  //
  // Set Legacy16 state. 0x08, 0x70 is legacy 8259 vector bases.
  //
  Status = gLegacy8259->SetMode (gLegacy8259, Efi8259LegacyMode, NULL, NULL);
  ASSERT_EFI_ERROR (Status);

  //
  // Call the real mode thunk code
  //
  CallRealModeThunk = (CALL_REAL_MODE_THUNK)(UINTN)gIntThunk->CodeBuffer;
  Status = CallRealModeThunk (CallAddress, 0, gIntThunk);

  //
  // Check for errors with the thunk
  //
  switch (Status) {
  case THUNK_OK:
    break;

  case THUNK_ERR_A20_UNSUP:
  case THUNK_ERR_A20_FAILED:
  default:
    //
    // For all errors, set EFLAGS.CF (used by legacy BIOS to indicate error).
    //
    Regs->X.Flags.CF = 1;
    break;
  }

  //
  // Return the resulting registers
  //
  EfiCopyMem (Regs, Stack16, sizeof (EFI_IA32_REGISTER_SET));
  Stack16 += sizeof (EFI_IA32_REGISTER_SET) / sizeof (UINT16);

  if (Stack != NULL && StackSize != 0) {
    //
    // Copy low memory stack to Stack
    //
    EfiCopyMem (Stack, Stack16, StackSize);
    Stack16 += StackSize / sizeof (UINT16);
  }

  //
  // Restore protected mode interrupt state
  //
  Status = gLegacy8259->SetMode (gLegacy8259, Efi8259ProtectedMode, NULL, NULL);
  ASSERT_EFI_ERROR (Status);

  //
  // End critical section
  //
  gBS->RestoreTPL (OriginalTpl);

  return (BOOLEAN) (Regs->X.Flags.CF ? TRUE : FALSE);
}



