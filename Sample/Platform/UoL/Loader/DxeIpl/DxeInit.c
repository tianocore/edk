/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  DxeInit.c

Abstract:

Revision History:

--*/


#include "Tiano.h"
#include "PeiLib.h"
#include "PeiHob.h"
#include "EfiHobLib.h"
#include "EfiCommonLib.h"
#include "EfiImage.h"
#include "EfiImageFormat.h"

#include EFI_PROTOCOL_CONSUMER (LoadedImage)
#include EFI_PROTOCOL_CONSUMER (DevicePath)
#include EFI_PROTOCOL_CONSUMER (SimpleFileSystem)
#include EFI_PROTOCOL_CONSUMER (FirmwareVolume)
#include EFI_PROTOCOL_CONSUMER (FileInfo)
#include EFI_GUID_DEFINITION   (PeiPeCoffLoader)
#include EFI_GUID_DEFINITION   (MemoryAllocationHob)
#include EFI_GUID_DEFINITION   (DebugImageInfoTable)
#include EFI_GUID_DEFINITION   (MemoryTypeInformation)
#include EFI_GUID_DEFINITION   (SmBios)
#include EFI_GUID_DEFINITION   (Acpi)
#include EFI_GUID_DEFINITION   (Mps)

#include EFI_PROTOCOL_DEFINITION (Decompress)

#include "LegacyTable.h"
#include "HobGeneration.h"
#include "PpisNeededByDxeCore.h"
#include "Debug.h"

//
// --------------------------------------------------------
// Memory Map: (XX=32,64)
// --------------------------------------------------------
// 0x0
//   IVT
// 0x400
//   BDA
// 0x500
//
// 0x7C00
//   BootSector
// 0x10000
//   EfiLdr (loaded)
// 0x20000
//   StartXX.COM (E820 table, Temporary GDT, Temporary IDT)
// 0x21000
//   EfiXX.COM (Temporary Interrupt Handler)
// 0x22000
//   EfiLdr + DxeIpl.Z + DxeMain.Z + BFV.Z
// 0x80000
//   Temporary 4G PageTable for X64
// 0x86000
//   MemoryFreeUnder1M (For legacy driver DMA)
//
// 0x9F800
//   EBDA
// 0xA0000
//   VGA
// 0xC0000
//   OPROM
// 0xE0000
//   FIRMEWARE
// 0x100000 (1M) ------------------------------------------
//   Temporary Stack (1M)
// 0x200000
// 0x100000 + AAA
//   MemoryFreeAbove1M
//
// 0x100000000 - RRR         <- Phit.EfiMemoryBottom -----+
//   Hob                                                  |
// 0x100000000 - SSS         <- Phit.EfiFreeMemoryBottom  |
//                                                        |
// 0x100000000 - TTT         <- Phit.EfiFreeMemoryTop     |
//   MemoryDescriptor (For ACPINVS, ACPIReclaim)          |
// 0x100000000 - UUU                                      |
//   Permament 4G PageTable for IA32                     4M
//   Permament 64G PageTable for X64                      |
// 0x100000000 - VVV                                      |
//   Permament Stack (128K)                               |
// 0x100000000 - WWW         <- Memory Top on Decriptor   |
//   DxeCore                                              |
// 0x100000000 - XXX                                      |
//   DxeIpl                                               |
// 0x100000000 - YYY         <- Phit.EfiMemoryTop --------+
//   BFV
// 0x100000000 - ZZZ         <- Memory Top on E820
//   ACPINVS
//   ACPIReclaim
//   Reserved
// 0x100000000 - ???         <- Memory Top on RealMemory
// 
// 0x100000000 (4G)----------------------------------------
// 0x100000000 + AAA
//   MemoryFreeAbove4G
//
// 0x100000000 + ZZZ
// --------------------------------------------------------
//

VOID
EnterDxeMain (
  IN VOID *StackTop,
  IN VOID *DxeCoreEntryPoint,
  IN VOID *Hob,
  IN VOID *PageTable
  );

VOID
DxeInit (
  IN EFILDRHANDOFF  *Handoff
  )
/*++

  Routine Description:

    This is the entry point after this code has been loaded into memory. 

Arguments:


Returns:

    Calls into EFI Firmware

--*/
{
  VOID                  *StackTop;
  VOID                  *StackBottom;
  VOID                  *PageTableBase;
  VOID                  *MemoryTopOnDescriptor;
  VOID                  *MemoryDescriptor;

  PrepareHobBfv (Handoff->BfvBase, Handoff->BfvSize);

  MemoryTopOnDescriptor = PrepareHobMemory (Handoff->MemDescCount, Handoff->MemDesc);
  StackTop = MemoryTopOnDescriptor;

  StackBottom = PrepareHobStack (StackTop);

  PageTableBase = PrepareHobPageTable (StackBottom);

  MemoryDescriptor = PrepareHobMemoryDescriptor (PageTableBase, Handoff->MemDescCount, Handoff->MemDesc);

  PrepareHobPhit (Handoff->BfvBase, MemoryDescriptor);

  PrepareHobDxeCore (Handoff->DxeCoreEntryPoint, (EFI_PHYSICAL_ADDRESS)(UINTN)Handoff->DxeCoreImageBase, Handoff->DxeCoreImageSize);

  PrepareHobLegacyTable (gHob);
  PreparePpisNeededByDxeCore (gHob);

  CompleteHobGeneration ();

/*
  //
  // Print Hob Info
  //
  ClearScreen();
  PrintString("Hob Info\n");
  PrintString("Phit.EfiMemoryTop = ");   
  PrintValue64(gHob->Phit.EfiMemoryTop);
  PrintString(" Phit.EfiMemoryBottom = ");   
  PrintValue64(gHob->Phit.EfiMemoryBottom);
  PrintString("\n");   
  PrintString("Phit.EfiFreeMemoryTop = ");   
  PrintValue64(gHob->Phit.EfiFreeMemoryTop);
  PrintString(" Phit.EfiFreeMemoryBottom = ");   
  PrintValue64(gHob->Phit.EfiFreeMemoryBottom);
  PrintString("\n");   
  PrintString("Bfv = ");   
  PrintValue64(gHob->Bfv.BaseAddress);
  PrintString(" BfvLength = ");   
  PrintValue64(gHob->Bfv.Length);
  PrintString("\n");   
  PrintString("Stack = ");   
  PrintValue64(gHob->Stack.AllocDescriptor.MemoryBaseAddress);
  PrintString(" StackLength = ");   
  PrintValue64(gHob->Stack.AllocDescriptor.MemoryLength);
  PrintString("\n");   
  PrintString("MemoryFreeUnder1MB = ");   
  PrintValue64(gHob->MemoryFreeUnder1MB.PhysicalStart);
  PrintString(" MemoryFreeUnder1MBLength = ");   
  PrintValue64(gHob->MemoryFreeUnder1MB.ResourceLength);
  PrintString("\n");   
  PrintString("MemoryAbove1MB = ");   
  PrintValue64(gHob->MemoryAbove1MB.PhysicalStart);
  PrintString(" MemoryAbove1MBLength = ");   
  PrintValue64(gHob->MemoryAbove1MB.ResourceLength);
  PrintString("\n");   
  PrintString("MemoryAbove4GB = ");   
  PrintValue64(gHob->MemoryAbove4GB.PhysicalStart);
  PrintString(" MemoryAbove4GBLength = ");   
  PrintValue64(gHob->MemoryAbove4GB.ResourceLength);
  PrintString("\n");   
  PrintString("DxeCore = ");   
  PrintValue64(gHob->DxeCore.MemoryAllocationHeader.MemoryBaseAddress);
  PrintString(" DxeCoreLength = ");   
  PrintValue64(gHob->DxeCore.MemoryAllocationHeader.MemoryLength);
  PrintString("\n");   
  PrintString("MemoryAllocation = ");   
  PrintValue64(gHob->MemoryAllocation.AllocDescriptor.MemoryBaseAddress);
  PrintString(" MemoryLength = ");   
  PrintValue64(gHob->MemoryAllocation.AllocDescriptor.MemoryLength);
  PrintString("\n");   
*/

  ClearScreen();
  PrintString("\n\n\n\n\n\n\n\n\n\n");
  PrintString("                         WELCOME TO EFI WORLD!\n");

  EnterDxeMain (StackTop, Handoff->DxeCoreEntryPoint, gHob, PageTableBase);

  //
  // Should never get here
  //
  EFI_DEADLOOP ();
}

