/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  MemoryAttribute.c
  
Abstract: 
  

Revision History

--*/

#include "CpuDxe.h"
#include "CpuLib.h"
#include "MemoryAttribute.h"

extern EFI_LEGACY_8259_PROTOCOL    *gLegacy8259;
extern EFI_CPU_INTERRUPT_HANDLER   gExternalVectorTable[0x100];

EFI_FIXED_MTRR mFixedMtrrTable[] = {
  { CPU_IA32_MTRR_FIX64K_00000, 0, 0x10000},
  { CPU_IA32_MTRR_FIX16K_80000, 0x80000, 0x4000},
  { CPU_IA32_MTRR_FIX16K_A0000, 0xA0000, 0x4000},
  { CPU_IA32_MTRR_FIX4K_C0000, 0xC0000, 0x1000},
  { CPU_IA32_MTRR_FIX4K_C8000, 0xC8000, 0x1000},
  { CPU_IA32_MTRR_FIX4K_D0000, 0xD0000, 0x1000},
  { CPU_IA32_MTRR_FIX4K_D8000, 0xD8000, 0x1000},
  { CPU_IA32_MTRR_FIX4K_E0000, 0xE0000, 0x1000},
  { CPU_IA32_MTRR_FIX4K_E8000, 0xE8000, 0x1000},
  { CPU_IA32_MTRR_FIX4K_F0000, 0xF0000, 0x1000},
  { CPU_IA32_MTRR_FIX4K_F8000, 0xF8000, 0x1000},
};

EFI_VARIABLE_MTRR mVariableMtrr[6];
UINT32            mUsedMtrr;

typedef struct {
  UINT16  LimitLow;
  UINT16  BaseLow;
  UINT8   BaseMiddle;
  UINT16  Attributes;
  UINT8   BaseHigh;
} SEGMENT_DESCRIPTOR;

typedef struct {
  UINT16  OffsetLow;
  UINT16  SegmentSelector;
  UINT16  Attributes;
  UINT16  OffsetHigh;
} INTERRUPT_GATE_DESCRIPTOR;

INTERRUPT_GATE_DESCRIPTOR mIdtTable[INTERRUPT_VECTOR_NUMBER];

VOID
AsmIdtVector00 (
  VOID
  );

VOID
InitializeInterruptTables (
  VOID
  )
/*++

Routine Description:

  Slick around interrupt routines.

Arguments:

  None

Returns:

  None


--*/
{
  EFI_STATUS                      Status;
  UINT16                          CodeSegment;
  INTERRUPT_GATE_DESCRIPTOR       *IdtEntry;
  UINT8                           *CurrentHandler;
  UINTN                           Index;

  CodeSegment = CpuCodeSegment ();
  
  IdtEntry = mIdtTable;
  CurrentHandler = (UINT8 *)(UINTN)AsmIdtVector00;
  for (Index = 0; Index < INTERRUPT_VECTOR_NUMBER; Index++) {
    IdtEntry[Index].OffsetLow = (UINT16) (UINTN) CurrentHandler;
    IdtEntry[Index].SegmentSelector = CodeSegment;
    IdtEntry[Index].Attributes = INTERRUPT_GATE_ATTRIBUTE;
    //
    // 8e00;
    //
    IdtEntry[Index].OffsetHigh = (UINT16) ((UINTN) CurrentHandler >> 16);

    CurrentHandler += 0x8;
  }
  //
  // Find the Legacy8259 protocol.
  //
  Status = gBS->LocateProtocol (&gEfiLegacy8259ProtocolGuid, NULL, (VOID **) &gLegacy8259);
  ASSERT_EFI_ERROR (Status);

  gLegacy8259->SetMode (gLegacy8259, Efi8259ProtectedMode, NULL, NULL);

  InitializeIdt (
    &(gExternalVectorTable[0]),
    (UINTN *) mIdtTable,
    sizeof (INTERRUPT_GATE_DESCRIPTOR) * INTERRUPT_VECTOR_NUMBER
    );

  return;
}


VOID 
PreMtrrChange (
  VOID
  )
{
  UINT64                      TempQword;

  EfiDisableCache ();

  //
  // Disable Cache MTRR
  //
  TempQword = EfiReadMsr (CPU_CACHE_IA32_MTRR_DEF_TYPE);
  TempQword = TempQword & ~CPU_CACHE_MTRR_VALID & ~CPU_CACHE_FIXED_MTRR_VALID;
  EfiWriteMsr (CPU_CACHE_IA32_MTRR_DEF_TYPE, TempQword);

  return;
}

VOID 
PostMtrrChange (
  VOID
  )
{
  UINT64  TempQword=0;

  //
  // Enable Cache MTRR
  //
  TempQword = EfiReadMsr (CPU_CACHE_IA32_MTRR_DEF_TYPE);
  EfiWriteMsr (CPU_CACHE_IA32_MTRR_DEF_TYPE, TempQword | CPU_CACHE_MTRR_VALID | CPU_CACHE_FIXED_MTRR_VALID);

  EfiEnableCache ();
  return;
}

EFI_STATUS
ProgramFixedMtrr (
  IN  UINT64                    MemoryCacheType,
  IN  UINT64                    *Base,
  IN  UINT64                    *Length
  )
{
  UINT32                      MsrNum;
  UINT32                      ByteShift;
  UINT64                      TempQword;
  UINT64                      OrMask;
  UINT64                      ClearMask;

  TempQword = 0;
  OrMask =  0;
  ClearMask = 0;

  for (MsrNum = 0; MsrNum < EFI_FIXED_MTRR_NUMBER; MsrNum++) {
    if ((*Base >= mFixedMtrrTable[MsrNum].BaseAddress) &&
        (*Base < (mFixedMtrrTable[MsrNum].BaseAddress + 8 * mFixedMtrrTable[MsrNum].Length))) {
      break;
    }
  }
  if (MsrNum == EFI_FIXED_MTRR_NUMBER ) {
    return  EFI_UNSUPPORTED;
  }

  //
  // We found the fixed MTRR to be programmed
  //
  for (ByteShift=0; ByteShift < 8; ByteShift++) {
    if ( *Base == (mFixedMtrrTable[MsrNum].BaseAddress + ByteShift * mFixedMtrrTable[MsrNum].Length)) {
      break;
    }
  }
  if (ByteShift == 8 ) {
    return  EFI_UNSUPPORTED;
  }
  
  for (; ((ByteShift<8) && (*Length >= mFixedMtrrTable[MsrNum].Length));ByteShift++) {
    OrMask |= LShiftU64((UINT64) MemoryCacheType, (UINT32) (ByteShift* 8));
    ClearMask |= LShiftU64((UINT64) 0xFF, (UINT32) (ByteShift * 8));
    *Length -= mFixedMtrrTable[MsrNum].Length;
    *Base += mFixedMtrrTable[MsrNum].Length;
  }
  if (ByteShift < 8 && (*Length != 0)) {
    return EFI_UNSUPPORTED;
  }
  TempQword = EfiReadMsr (mFixedMtrrTable[MsrNum].Msr) & ~ClearMask | OrMask;
  EfiWriteMsr (mFixedMtrrTable[MsrNum].Msr, TempQword);
  return  EFI_SUCCESS;
}

EFI_STATUS
GetMemoryAttribute(
  VOID
  )
{
  UINTN   Index;
  UINT32  MsrNum;
  

  EfiZeroMem (mVariableMtrr, sizeof(EFI_VARIABLE_MTRR) * 6);
  mUsedMtrr = 0;

  for (MsrNum = CPU_CACHE_VARIABLE_MTRR_BASE, Index = 0; 
       ((MsrNum < CPU_CACHE_VARIABLE_MTRR_END) && (Index < 6)); 
       MsrNum +=2) {
    if ((EfiReadMsr(MsrNum+1) & CPU_CACHE_MTRR_VALID) != 0 ) {
      mVariableMtrr[Index].Msr          = MsrNum;
      mVariableMtrr[Index].BaseAddress  = (EfiReadMsr(MsrNum) & CPU_CACHE_VALID_ADDRESS);
      mVariableMtrr[Index].Length       = ((~((EfiReadMsr(MsrNum+1) & CPU_CACHE_VALID_ADDRESS))) & CPU_MSR_VALID_MASK ) + 1;
      mVariableMtrr[Index].Type         = (EfiReadMsr(MsrNum) & 0x0ff);
      mVariableMtrr[Index].Valid        = TRUE;
      mUsedMtrr++;
      Index++;
    }
  }
  return EFI_SUCCESS;
}

BOOLEAN 
CheckMemoryAttributeOverlap (
  IN  EFI_PHYSICAL_ADDRESS      Start,
  IN  EFI_PHYSICAL_ADDRESS      End
  )
{
  UINT32    Index;
  
  for (Index = 0; Index < 6; Index++) {
    if (mVariableMtrr[Index].Valid && 
      !(Start > (mVariableMtrr[Index].BaseAddress + mVariableMtrr[Index].Length - 1) || 
      (End < mVariableMtrr[Index].BaseAddress)) ) {
      
      return TRUE;
    }
  }
  return FALSE;
}

EFI_STATUS
CombineMemoryAttribute (
  IN  UINT64                    Attributes,
  IN  UINT64                    *Base,
  IN  UINT64                    *Length
  )
{
  UINT32  Index;
  UINT64  CombineStart;
  UINT64  CombineEnd;
  UINT64  MtrrEnd;
  UINT64  EndAddress;
//  UINT64  MemoryLength;
  EndAddress = *Base + *Length - 1;
  
  for (Index = 0; Index < 6; Index++) {

    MtrrEnd = mVariableMtrr[Index].BaseAddress + mVariableMtrr[Index].Length - 1;
    if ( !mVariableMtrr[Index].Valid || (*Base > (MtrrEnd) || (EndAddress < mVariableMtrr[Index].BaseAddress)) ) {
      continue;
    }

    //
    // if UC, UC memory type used
    //
    if (Attributes == CPU_CACHE_UNCACHEABLE) {
      return EFI_SUCCESS;
    } 
    
    //
    // Combine same attribute MTRR range
    //
    if (Attributes == mVariableMtrr[Index].Type) {
      //
      // if the Mtrr range contain the request range, return EFI_SUCCESS
      // 
      if (mVariableMtrr[Index].BaseAddress <= *Base && MtrrEnd >= EndAddress) {
          *Length = 0;
          return EFI_SUCCESS;
      }
      
      //
      // invalid this MTRR, and program the combine range
      //
      CombineStart = (*Base) < mVariableMtrr[Index].BaseAddress ? (*Base) : mVariableMtrr[Index].BaseAddress;
      CombineEnd = EndAddress > MtrrEnd ? EndAddress : MtrrEnd;
      
      InvariableMtrr(mVariableMtrr[Index].Msr, Index);
      *Base = CombineStart;
      *Length = CombineEnd - CombineStart + 1;
      EndAddress = CombineEnd;
      continue;
    }
    
    if ((Attributes == CPU_CACHE_WRITETHROUGH && mVariableMtrr[Index].Type == CPU_CACHE_WRITEBACK) || 
        (Attributes == CPU_CACHE_WRITEBACK && mVariableMtrr[Index].Type == CPU_CACHE_WRITETHROUGH)  ||
        (Attributes == CPU_CACHE_WRITETHROUGH && mVariableMtrr[Index].Type == CPU_CACHE_UNCACHEABLE)  ||
        (Attributes == CPU_CACHE_WRITEBACK && mVariableMtrr[Index].Type == CPU_CACHE_UNCACHEABLE) 
        ) {
      continue;
    }
    
    //
    // Other type memory overlap is invalid
    //
    return EFI_ACCESS_DENIED;

  }  

  return EFI_SUCCESS;
}

EFI_STATUS
GetDirection (
  IN  UINT64                    Input,
  IN  UINT32                    *MtrrNumber,
  IN  BOOLEAN                   *Direction
  )
/*++

Routine Description:
    Given the input, check if the number of MTRR is lesser
    if positive or subtractive

Arguments:
    Input - Length of Memory to program MTRR
    MtrrNumber - return needed Mtrr number
    Direction  - TRUE: do positive
                 FALSE: do subtractive
Returns:
    Zero, do positive
    Non-Zero, do subractive
    
--*/
{
  UINT64                        TempQword;
  UINT32                        Positive;
  UINT32                        Subtractive;

  TempQword = Input;
  Positive = 0;
  Subtractive = 0;

  do {
    TempQword -= Power2MaxMemory(TempQword);
    Positive++;

  } while (TempQword != 0);

  TempQword = Power2MaxMemory(LShiftU64(Input, 1)) - Input;
  Subtractive++;
  do {
    TempQword -= Power2MaxMemory(TempQword);
    Subtractive++;

  } while (TempQword != 0);

  if (Positive <= Subtractive) {
    *Direction = TRUE;
    *MtrrNumber = Positive;
  } else {
    *Direction = FALSE;
    *MtrrNumber = Subtractive;
  }
  return EFI_SUCCESS;
}


UINT64
Power2MaxMemory (
  IN UINT64                     MemoryLength
  )
{
  UINT64                    Result;

  if (RShiftU64(MemoryLength, 32)) {
    Result = LShiftU64((UINT64)CpuSetPower2((UINT32) RShiftU64(MemoryLength, 32)), 32);
  } else {
    Result = (UINT64)CpuSetPower2((UINT32)MemoryLength);
  }
  return  Result;
}

EFI_STATUS
InvariableMtrr (
  IN  UINT32    MtrrNumber,
  IN  UINT32    Index
  )
{
  PreMtrrChange ();
  mVariableMtrr[Index].Valid = FALSE;
  EfiWriteMsr (MtrrNumber, 0);
  EfiWriteMsr (MtrrNumber+1, 0);
  mUsedMtrr--;
  PostMtrrChange ();

  return EFI_SUCCESS;
}

EFI_STATUS
ProgramVariableMtrr(
  IN  UINT32                    MtrrNumber,
  IN  EFI_PHYSICAL_ADDRESS      BaseAddress,
  IN  UINT64                    Length,
  IN  UINT64                    MemoryCacheType
  )
{
  UINT64                        TempQword;
  PreMtrrChange();

  //
  // MTRR Physical Base
  //
  TempQword = (BaseAddress & CPU_CACHE_VALID_ADDRESS) | MemoryCacheType;
  EfiWriteMsr (MtrrNumber, TempQword);

  // 
  // MTRR Physical Mask
  //
  TempQword = ~(Length - 1);
  EfiWriteMsr (MtrrNumber+1, (TempQword & CPU_CACHE_VALID_ADDRESS) | CPU_CACHE_MTRR_VALID);

  PostMtrrChange();

  return EFI_SUCCESS;
}


EFI_STATUS
CleanupVariableMtrr (
  VOID
  ) 
{
  BOOLEAN               Cleaned;
  BOOLEAN               EverCleaned;
  UINT32                Index;
  UINT32                Index2;
  BOOLEAN               MtrrModified[6];
  UINT32                MtrrNumber;
  UINT32                MsrNum;
  EFI_PHYSICAL_ADDRESS  BaseAddress;
  UINT64                Length;
  UINT64                TempQword;
  UINT64                Attributes;
  BOOLEAN               Positive;

  for ( Index = 0 ; Index < 6; Index++ ) {
    MtrrModified[Index] = FALSE;
  }

  GetMemoryAttribute ();

  //
  // After the do-while, mVariableMtrr is NO longer the value read from MTRR regisrer!!!
  //
  EverCleaned = FALSE;

  do {
    Cleaned = FALSE;
    
    for ( Index = 0; Index < 6; Index++ ) {
      if  ( mVariableMtrr[Index].Type  == CPU_CACHE_UNCACHEABLE  && mVariableMtrr[Index].Valid ) {
        for ( Index2 = 0; Index2 < 6; Index2++ ) {
          if ( mVariableMtrr[Index2].Type  == CPU_CACHE_WRITEBACK && mVariableMtrr[Index2].Valid) {
            //
            // the Uncacheble just inside the WB and at the edge.
            // if so, we can clean the UC entry and decrease the WB entry
            //
            if (  mVariableMtrr[Index].BaseAddress == mVariableMtrr[Index2].BaseAddress  ) {
                Cleaned = TRUE;
                EverCleaned = TRUE;
                if ( mVariableMtrr[Index].Length >= mVariableMtrr[Index2].Length ) {
                  //
                  // we can invalidate WB entry, since nothing left
                  //
                  InvariableMtrr (mVariableMtrr[Index2].Msr, Index2);
                  mVariableMtrr[Index].BaseAddress = mVariableMtrr[Index].BaseAddress + mVariableMtrr[Index2].Length;
                  mVariableMtrr[Index].Length -= mVariableMtrr[Index2].Length;
                  MtrrModified[Index] = TRUE;
                  if ( mVariableMtrr[Index].Length == 0 ) {
                    InvariableMtrr(mVariableMtrr[Index].Msr, Index);
                  }

                } else {
                  //
                  // we can invalidate UC entry, since nothing left
                  //
                  InvariableMtrr(mVariableMtrr[Index].Msr, Index);
                  mVariableMtrr[Index2].BaseAddress = mVariableMtrr[Index].BaseAddress + mVariableMtrr[Index].Length;
                  mVariableMtrr[Index2].Length -= mVariableMtrr[Index].Length;
                  MtrrModified[Index2] = TRUE;
                }
               
            }


            if (  mVariableMtrr[Index].BaseAddress + mVariableMtrr[Index].Length == mVariableMtrr[Index2].BaseAddress + mVariableMtrr[Index2].Length) {
                Cleaned = TRUE;
                EverCleaned = TRUE;

                if ( mVariableMtrr[Index].Length >= mVariableMtrr[Index2].Length ) {
                  //
                  // we can invalidate WB entry, since nothing left
                  //
                  InvariableMtrr(mVariableMtrr[Index2].Msr, Index2);
                  mVariableMtrr[Index].Length -= mVariableMtrr[Index2].Length;
                  MtrrModified[Index] = TRUE;
                  if ( mVariableMtrr[Index].Length == 0 ) {
                    InvariableMtrr(mVariableMtrr[Index].Msr, Index);
                  }

                } else {
                  //
                  // we can invalidate UC entry, since nothing left
                  //
                  InvariableMtrr(mVariableMtrr[Index].Msr, Index);
                  mVariableMtrr[Index2].Length -= mVariableMtrr[Index].Length;
                  MtrrModified[Index2] = TRUE;
                }
               

            }
          }  // end WB
        }  // end of Index2
      }  // Endof UC
    }  // endof Index

  } while (Cleaned);


  if (!EverCleaned) {
    return EFI_SUCCESS;
  }

  //
  // Begin to program the MTRR again
  //
  for ( Index = 0; Index < 6; Index++ ) {
    if ( MtrrModified[Index] && mVariableMtrr[Index].Valid) {

      //
      // Program the new MTRRR
      //
     TempQword = mVariableMtrr[Index].Length;
     MsrNum = CPU_CACHE_VARIABLE_MTRR_BASE + 2 * Index;
     BaseAddress = mVariableMtrr[Index].BaseAddress;
     Length = mVariableMtrr[Index].Length;
     Attributes = mVariableMtrr[Index].Type;

     if (TempQword == Power2MaxMemory (TempQword)) {
       //
       // if it's two's power
       // no need to request a new mtrr,
       // just program this one
       //
       ProgramVariableMtrr(
                   MsrNum, 
                   BaseAddress, 
                   Length, 
                   Attributes
                   );
     } else {
       GetDirection (TempQword, &MtrrNumber, &Positive);
        //
        // we already has one that can use, so 6+1
        //
        if ((mUsedMtrr + MtrrNumber) > 6+1) {
         return EFI_OUT_OF_RESOURCES;
        }

        if (!Positive) {    
          Length = Power2MaxMemory (LShiftU64(TempQword, 1));
          ProgramVariableMtrr (MsrNum, BaseAddress, Length, Attributes);
          BaseAddress += TempQword;
          TempQword  = Length - TempQword;
          Attributes = CPU_CACHE_UNCACHEABLE;
        }

      do {
        // 
        // Find unused MTRR
        //
        for (MsrNum = CPU_CACHE_VARIABLE_MTRR_BASE; MsrNum < CPU_CACHE_VARIABLE_MTRR_END; MsrNum +=2) {
          if ((EfiReadMsr (MsrNum+1) & CPU_CACHE_MTRR_VALID) == 0 ) {
            break;
          }
        }

        Length = Power2MaxMemory(TempQword);
        ProgramVariableMtrr (MsrNum, BaseAddress, Length, Attributes);
        BaseAddress += Length;
        TempQword -= Length;

      } while (TempQword);

     }  // endof Powerof

    
    }  // endof Modified

  }  // endof for
  
  
  return EFI_SUCCESS;
}

  
EFI_STATUS
CpuSetMemoryAttributes (
  IN EFI_CPU_ARCH_PROTOCOL     *This,
  IN EFI_PHYSICAL_ADDRESS      BaseAddress,
  IN UINT64                    Length,
  IN UINT64                    Attributes
  )
/*++

Routine Description:
  Set memory cacheability attributes for given range of memeory

Arguments:
  This                - Protocol instance structure
  BaseAddress         - Specifies the start address of the memory range
  Length              - Specifies the length of the memory range
  Attributes          - The memory cacheability for the memory range

Returns: 
  EFI_SUCCESS           - If the cacheability of that memory range is set successfully
  EFI_UNSUPPORTED       - If the desired operation cannot be done
  EFI_INVALID_PARAMETER - The input parameter is not correct, such as Length = 0

--*/
{
  EFI_STATUS                Status;
  UINT64                    TempQword;
  UINT32                    MsrNum;
  UINT32                    MtrrNumber;
  BOOLEAN                   Positive;
  BOOLEAN                   OverLap;
  UINTN                     Remainder;

  TempQword = 0;

  switch (Attributes) {
  case EFI_MEMORY_UC:
    Attributes = EFI_CACHE_UNCACHEABLE;
    break;

  case EFI_MEMORY_WC:
    Attributes = EFI_CACHE_WRITECOMBINING;
    break;

  case EFI_MEMORY_WT:
    Attributes = EFI_CACHE_WRITETHROUGH;
    break;

  case EFI_MEMORY_WP:
    Attributes = EFI_CACHE_WRITEPROTECTED;
    break;

  case EFI_MEMORY_WB:
    Attributes = EFI_CACHE_WRITEBACK;
    break;

  default:
    return EFI_UNSUPPORTED;
  }

  //
  // Check if Fixed MTRR
  //
  Status = EFI_SUCCESS;
  while ((BaseAddress < (1 << 20)) && (Length > 0) && Status == EFI_SUCCESS) {
    PreMtrrChange ();
    Status = ProgramFixedMtrr (Attributes, &BaseAddress, &Length);
    PostMtrrChange ();
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (Length == 0) {
    //
    // Just Fixed MTRR. NO need to go through Variable MTRR
    //
    return Status;
  }
  //
  // since mem below 1m will be override by fixed mtrr, we can set it to 0 to save mtrr.
  //
  if (BaseAddress == 0x100000) {
    BaseAddress = 0;
    Length += 0x100000;
  }
  //
  // Check memory base address alignment
  //
  DivU64x32 (BaseAddress, (UINTN) Power2MaxMemory (LShiftU64 (Length, 1)), &Remainder);
  if (Remainder != 0) {
    DivU64x32 (BaseAddress, (UINTN) Power2MaxMemory (Length), &Remainder);
    if (Remainder != 0) {
      return EFI_UNSUPPORTED;
    }
  }
  //
  // Check overlap
  //
  GetMemoryAttribute ();
  OverLap = CheckMemoryAttributeOverlap (BaseAddress, BaseAddress + Length - 1);
  if (OverLap) {
    Status = CombineMemoryAttribute (Attributes, &BaseAddress, &Length);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (Length == 0) {
      //
      // combine successfully
      //
      return EFI_SUCCESS;
    }
  } else {
    if (Attributes == EFI_CACHE_UNCACHEABLE) {
      return EFI_SUCCESS;
    }
  }

  //
  // Program Variable MTRRs
  //
  if (mUsedMtrr >= 6) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Find first unused MTRR
  //
  for (MsrNum = EFI_CACHE_VARIABLE_MTRR_BASE; MsrNum < EFI_CACHE_VARIABLE_MTRR_END; MsrNum += 2) {
    if ((EfiReadMsr (MsrNum + 1) & EFI_CACHE_MTRR_VALID) == 0) {
      break;
    }
  }

  TempQword = Length;

  if (TempQword == Power2MaxMemory (TempQword)) {
    ProgramVariableMtrr (
      MsrNum,
      BaseAddress,
      Length,
      Attributes
      );
  } else {

    GetDirection (TempQword, &MtrrNumber, &Positive);

    if ((mUsedMtrr + MtrrNumber) > 6) {
      return Status;
    }

    if (!Positive) {
      Length = Power2MaxMemory (LShiftU64 (TempQword, 1));
      ProgramVariableMtrr (
        MsrNum,
        BaseAddress,
        Length,
        Attributes
        );
      BaseAddress += TempQword;
      TempQword   = Length - TempQword;
      Attributes  = EFI_CACHE_UNCACHEABLE;
    }

    do {
      //
      // Find unused MTRR
      //
      for (; MsrNum < EFI_CACHE_VARIABLE_MTRR_END; MsrNum += 2) {
        if ((EfiReadMsr (MsrNum + 1) & EFI_CACHE_MTRR_VALID) == 0) {
          break;
        }
      }

      Length = Power2MaxMemory (TempQword);
      ProgramVariableMtrr (
        MsrNum,
        BaseAddress,
        Length,
        Attributes
        );
      BaseAddress += Length;
      TempQword -= Length;

    } while (TempQword);
  }

  return Status;
}

VOID
InitailizeMemoryAttributes (
  VOID
  )
{
}




