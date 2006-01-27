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
#include "VirtualMemory.h"

extern EFI_LEGACY_8259_PROTOCOL          *gLegacy8259;



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
#define NUM_REAL_GDT_ENTRIES  8

//
// Define what a processor descriptor looks like
// This data structure must be kept in sync with ASM STRUCT in Thunk.inc
//
typedef struct {
  UINT16  Limit;
  UINT64  Base;
} DESCRIPTOR64;

typedef struct {
  UINT16  Limit;
  UINT32  Base;
} DESCRIPTOR32;


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
#define SPARE_SELECTOR_ARRAY_INDEX            5
#define X64_DATA_SELECTOR_ARRAY_INDEX         6
#define X64_CODE_SELECTOR_ARRAY_INDEX         7


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
  { // Selector 0x28 - spare segment
    0x0000,  // LimitLow
    0x0000,  // BaseLow
    0x00,    // BaseMid
    0x00,    // Attribute
    0x00,    // LimitHi
    0x00     // BaseHi
  },
  { // Selector 0x30 - DATA64 SEL
    0xffff,  // LimitLow
    0x0000,  // BaseLow
    0x00,    // BaseMid
    0x92,    // Attribute
    0xcf,    // LimitHi
    0x00     // BaseHi
  },
  { // Selector 0x38 - Code64 SEL
    0xffff,  // LimitLow
    0x0000,  // BaseLow
    0x00,    // BaseMid
    0x9a,    // Attribute
    0xaf,    // LimitHi
    0x00     // BaseHi
  }
};


//
// This data structure must be kept in sync with ASM STRUCT in Thunk.inc
//  It needs to be a data structure as it must be under 1MB to make it back
//  from real mode
typedef struct {
  //
  // Space for the code
  //  The address of Code is also the beginning of the relocated Thunk code
  //
  CHAR8                             CodeBuffer[4096]; // ?

  //
  // Data for the code (cs releative)
  //
  DESCRIPTOR64                      x64GdtDesc;          // Protected mode GDT
  DESCRIPTOR64                      x64IdtDesc;          // Protected mode IDT
  UINTN                             x64Ss;
  UINTN                             x64Esp;

  UINTN                             RealStack;
  DESCRIPTOR32                      RealModeIdtDesc;
  DESCRIPTOR32                      RealModeGdtDesc;
  //
  // real-mode GDT (temporary GDT with two real mode segment descriptors)
  //
  GDT_ENTRY                         RealModeGdt[NUM_REAL_GDT_ENTRIES];

  UINT64                            PageMapLevel4;
  
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

UINTN
RealModeTemplate (
  OUT UINTN             RealSegment,
  OUT UINTN             *CodeEnd,
  IN  LOW_MEMORY_THUNK  *IntThunk,
  IN  UINT32            TrampoleanCr4
  );

VOID
IdentityMap1MB (
  OUT LOW_MEMORY_THUNK  **IntThunk,
  OUT UINT32            *TrampoleanCr4
  )
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  Address;
  X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K   *PageMapLevel4Entry;
  X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K   *PageDirectoryPointerEntry;
  X64_PAGE_TABLE_ENTRY_2M                     *PageDirectoryEntry2MB;
                
 
  //
  // Grab a buffer under 1MB for Thunk. Real mode code has to run under 1MB
  //
  Address = 0x00000FFFFF;
  Status  = gBS->AllocatePages (AllocateMaxAddress, EfiBootServicesData, 1 + (sizeof (LOW_MEMORY_THUNK)/4096), &Address);
  ASSERT_EFI_ERROR (Status);

  //
  // Clear the buffer
  //
  *IntThunk = (LOW_MEMORY_THUNK *)(UINTN)Address;
  EfiZeroMem (*IntThunk, sizeof (LOW_MEMORY_THUNK));
  
  //
  // Allocate memory under 4GB for the x64 page tables. Current page tables are above 4GB we 
  //  need an extra set of page tables bellow 4GB to get back into x64 mode. 
  //  Using 2MB page three levels (3 4K pages) can map the real mode < 1MB space.
  //
  Address = 0xFFFFFFFFF;
  Status  = gBS->AllocatePages (AllocateMaxAddress, EfiBootServicesData, 3, &Address);
  ASSERT_EFI_ERROR (Status);

  EfiZeroMem ((VOID *)Address, 4096 * 3);

  *TrampoleanCr4 = (UINT32)Address;
  
  // Page 1 = Page Map Level 4 Entry
  PageMapLevel4Entry        = (X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K *)Address;
  
  // Page 2 = PageDirectory Pointer Table
  PageDirectoryPointerEntry = (X64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K *)(Address + 4096);
  
  // Page 3 = Page Directory Table Entry
  PageDirectoryEntry2MB     = (X64_PAGE_TABLE_ENTRY_2M *)(Address + 8192);

  PageMapLevel4Entry->Uint64 = (UINT64)PageDirectoryPointerEntry;
  PageMapLevel4Entry->Bits.ReadWrite = 1;
  PageMapLevel4Entry->Bits.Present = 1;

  PageDirectoryPointerEntry->Uint64 = (UINT64)PageDirectoryEntry2MB;
  PageDirectoryPointerEntry->Bits.ReadWrite = 1;
  PageDirectoryPointerEntry->Bits.Present = 1;

  //
  // Build the page from 0 - 2MB 
  //
  PageDirectoryEntry2MB->Uint64 = (UINT64)0;
  PageDirectoryEntry2MB->Bits.ReadWrite = 1;
  PageDirectoryEntry2MB->Bits.Present = 1;
  PageDirectoryEntry2MB->Bits.MustBe1 = 1;

  return;
}


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
  UINTN                 CodeStart;
  UINTN                 CodeEnd;
  UINTN                 RealSegment;
  UINT32                TrampoleanCr4;
  GDT_ENTRY             *RealCodeGdt;
  GDT_ENTRY             *RealDataGdt;

  IdentityMap1MB (&gIntThunk, &TrampoleanCr4);

  //
  // Real mode is Seg:Offset. Make RealSegmentSeg:0 equal to gIntThunk
  //
  RealSegment = (UINTN)(gIntThunk) >> 4;

  EfiCopyMem (gIntThunk->RealModeGdt, gTrapoleanGdt, sizeof (gTrapoleanGdt));
  
  //
  // Real mode CS & DS need to be relative to the start of the buffer.
  //  This is required to make the crack work. Crack - is the transition
  //  from one mode to another. At least this is the rumor
  //
  RealCodeGdt = &gIntThunk->RealModeGdt[REAL_CODE_SELECTOR_ARRAY_INDEX];
  RealCodeGdt->BaseHi  = (UINT8)((UINTN)(gIntThunk) >> 24) & 0xff;
  RealCodeGdt->BaseMid = (UINT8)((UINTN)(gIntThunk) >> 16) & 0xff;
  RealCodeGdt->BaseLow = (UINT16)((UINTN)(gIntThunk) & 0xffff);

  RealDataGdt = &gIntThunk->RealModeGdt[REAL_DATA_SELECTOR_ARRAY_INDEX];
  RealDataGdt->BaseHi  = (UINT8)((UINTN)(gIntThunk) >> 24) & 0xff;
  RealDataGdt->BaseMid = (UINT8)((UINTN)(gIntThunk) >> 16) & 0xff;
  RealDataGdt->BaseLow = (UINT16)((UINTN)(gIntThunk) & 0xffff);

  //
  // Compute selector value now that the GDT is under 1MB.
  //
  gIntThunk->RealModeGdtDesc.Limit = sizeof (gIntThunk->RealModeGdt) - 1;
  gIntThunk->RealModeGdtDesc.Base  = (UINT32)(UINTN)gIntThunk->RealModeGdt;
  
  gIntThunk->RealModeIdtDesc.Limit = 0x3FF;
  gIntThunk->RealModeIdtDesc.Base  = 0;

  //
  // Initialize low real-mode code thunk
  //
  CodeStart = RealModeTemplate (RealSegment, &CodeEnd, gIntThunk, TrampoleanCr4);

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

  Stack16 = (UINT16 *) (gIntThunk->Stack + LOW_STACK_SIZE);

  //
  // Copy regs to low memory stack
  //
  Stack16 -= sizeof (EFI_IA32_REGISTER_SET) / sizeof (UINT16);
  EfiCopyMem (Stack16, Regs, sizeof (EFI_IA32_REGISTER_SET));
// DEBUG((EFI_D_ERROR|EFI_D_INFO, "stack16:  *** %x ***, *** %x ****\n", Stack16, Regs -> H.AH));
  //
  // Provide low stack esp
  //
  gIntThunk->RealStack = ((UINTN) Stack16) - ((UINTN) gIntThunk);

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
  case 0:
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
  UINTN                 CallAddress;
  UINT16                *Stack16;
  EFI_TPL               OriginalTpl;
  CALL_REAL_MODE_THUNK  CallRealModeThunk;
  //
  // Get the current flat GDT and IDT and store them in gIntThunk.
  //
  GetRegisters (gIntThunk);

  CallAddress = (Segment << 16) | Offset;

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
  // Clear the error flag; thunk code may set it.
  //
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
  gIntThunk->RealStack = ((UINTN)Stack16) - ((UINTN)gIntThunk);

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


