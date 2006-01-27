/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  CpuLib.h

Abstract:

  Library functions that can be called in both PEI and DXE phase

--*/

#ifndef _CPU_LIB_H_
#define _CPU_LIB_H_

#define CPU_CPUID_SIGNATURE                   0x0
#define CPU_CPUID_VERSION_INFO                0x1
#define CPU_CPUID_CACHE_INFO                  0x2
#define CPU_CPUID_SERIAL_NUMBER               0x3
#define CPU_CPUID_EXTENDED_FUNCTION           0x80000000
#define CPU_CPUID_EXTENDED_CPU_SIG            0x80000001
#define CPU_CPUID_BRAND_STRING1               0x80000002
#define CPU_CPUID_BRAND_STRING2               0x80000003
#define CPU_CPUID_BRAND_STRING3               0x80000004

#define CPU_MSR_IA32_PLATFORM_ID              0x17
#define CPU_MSR_IA32_APIC_BASE                0x1B
#define CPU_MSR_EBC_HARD_POWERON              0x2A
#define CPU_MSR_EBC_SOFT_POWERON              0x2B
#define BINIT_DRIVER_DISABLE                  0x40
#define INTERNAL_MCERR_DISABLE                0x20
#define INITIATOR_MCERR_DISABLE               0x10
#define CPU_MSR_EBC_FREQUENCY_ID              0x2C
#define CPU_MSR_IA32_BIOS_UPDT_TRIG           0x79
#define CPU_MSR_IA32_BIOS_SIGN_ID             0x8B
#define CPU_MSR_PSB_CLOCK_STATUS              0xCD
#define CPU_APIC_GLOBAL_ENABLE                0x800
#define CPU_MSR_IA32_MISC_ENABLE              0x1A0
#define LIMIT_CPUID_MAXVAL_ENABLE_BIT         0x00400000
#define AUTOMATIC_THERMAL_CONTROL_ENABLE_BIT  0x00000008
#define COMPATIBLE_FPU_OPCODE_ENABLE_BIT      0x00000004
#define LOGICAL_PROCESSOR_PRIORITY_ENABLE_BIT 0x00000002
#define FAST_STRING_ENABLE_BIT                0x00000001

#define CPU_CACHE_MTRR_CAPABILITY             0xFE
#define CPU_CACHE_VARIABLE_MTRR_BASE          0x200
#define CPU_CACHE_VARIABLE_MTRR_END           0x20F
#define CPU_CACHE_IA32_MTRR_DEF_TYPE          0x2FF
#define CPU_CACHE_VALID_ADDRESS               0xFFFFFF000
#define CPU_CACHE_MTRR_VALID                  0x800
#define CPU_CACHE_FIXED_MTRR_VALID            0x400
#define CPU_MSR_VALID_MASK                    0xFFFFFFFFF

#define CPU_IA32_MTRR_FIX64K_00000            0x250
#define CPU_IA32_MTRR_FIX16K_80000            0x258 
#define CPU_IA32_MTRR_FIX16K_A0000            0x259 
#define CPU_IA32_MTRR_FIX4K_C0000             0x268 
#define CPU_IA32_MTRR_FIX4K_C8000             0x269 
#define CPU_IA32_MTRR_FIX4K_D0000             0x26A 
#define CPU_IA32_MTRR_FIX4K_D8000             0x26B 
#define CPU_IA32_MTRR_FIX4K_E0000             0x26C 
#define CPU_IA32_MTRR_FIX4K_E8000             0x26D 
#define CPU_IA32_MTRR_FIX4K_F0000             0x26E 
#define CPU_IA32_MTRR_FIX4K_F8000             0x26F

#define CPU_IA32_MCG_CAP                      0x179
#define CPU_IA32_MCG_CTL                      0x17B
#define CPU_IA32_MC0_CTL                      0x400
#define CPU_IA32_MC0_STATUS                   0x401

#define CPU_CACHE_UNCACHEABLE                 0
#define CPU_CACHE_WRITECOMBINING              1
#define CPU_CACHE_WRITETHROUGH                4
#define CPU_CACHE_WRITEPROTECTED              5
#define CPU_CACHE_WRITEBACK                   6

typedef struct {
  UINT32  HeaderVersion;
  UINT32  UpdateRevision;
  UINT32  Date;
  UINT32  ProcessorId;
  UINT32  Checksum;
  UINT32  LoaderRevision;
  UINT32  ProcessorFlags;
  UINT32  DataSize;
  UINT32  TotalSize;
  UINT8   Reserved[12];
} CPU_MICROCODE_HEADER;

typedef struct {
  UINT32  ExtSigCount;
  UINT32  ExtChecksum;
  UINT8   Reserved[12];
  UINT32  ProcessorId;
  UINT32  ProcessorFlags;
  UINT32  Checksum;
} CPU_MICROCODE_EXT_HEADER;

typedef struct {
  UINT32  RegEax;
  UINT32  RegEbx;
  UINT32  RegEcx;
  UINT32  RegEdx;
} CPU_CPUID_REGISTER;


UINTN
CpuReadCr0 (
  VOID
  );

VOID
CpuWriteCr0 (
  UINTN   Value
  );

UINTN
CpuReadCr3 (
  VOID
  );

VOID
CpuWriteCr3 (
  UINTN   Value
  );

UINT8
CpuMemRead8 (
  IN  EFI_PHYSICAL_ADDRESS  Address
  );

UINT16
CpuMemRead16 (
  IN  EFI_PHYSICAL_ADDRESS  Address
  );

UINT32
CpuMemRead32 (
  IN  EFI_PHYSICAL_ADDRESS  Address
  );

UINT64
CpuMemRead64 (
  IN  EFI_PHYSICAL_ADDRESS  Address
  );

VOID
CpuMemWrite8 (
  IN  EFI_PHYSICAL_ADDRESS  Address,
  IN  UINT8                 Data
  );

VOID
CpuMemWrite16 (
  IN  EFI_PHYSICAL_ADDRESS  Address,
  IN  UINT16                Data
  );

VOID
CpuMemWrite32 (
  IN  EFI_PHYSICAL_ADDRESS  Address,
  IN  UINT32                Data
  );

VOID
CpuMemWrite64 (
  IN  EFI_PHYSICAL_ADDRESS  Address,
  IN  UINT64  Data
  );

UINTN
CpuSetPower2 (
  IN  UINTN   Input
  );

UINT64
CpuReadTsc (
  VOID
  );

VOID
CpuSwitchStacks (
  IN  UINTN   EntryPoint, 
  IN  UINTN   Parameter,  
  IN  UINTN   NewStack,   
  IN  UINTN   NewBsp      
  );

VOID
CpuSwitchStacks2Args (
  IN  UINTN   EntryPoint, 
  IN  UINTN   Parameter1, 
  IN  UINTN   Parameter2,
  IN  UINTN   NewStack,   
  IN  UINTN   NewBsp      
  );

UINT16
CpuCodeSegment (
  VOID
  );

VOID
CpuBreak (
  VOID
  );

VOID
CpuLoadGlobalDescriptorTable (
  VOID  *Table16ByteAligned
  );

VOID
CpuInitSelectors(
  VOID
  );
 
VOID
CpuLoadInterruptDescriptorTable (
  VOID  *Table16ByteAligned
  );

#endif
