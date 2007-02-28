/*++

Copyright (c) 2004 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Performance.c

Abstract:

  This file include the file which can help to get the system
  performance, all the function will only include if the performance
  switch is set.

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiPrintLib.h"

#ifdef EFI_DXE_PERFORMANCE
#include "EfiImage.h"
#include "Performance.h"

STATIC
VOID
ConvertChar16ToChar8 (
  IN CHAR8      *Dest,
  IN CHAR16     *Src
  )
{
  while (*Src) {
    *Dest++ = (UINT8) (*Src++);
  }

  *Dest = 0;
}

VOID
WriteBootToOsPerformanceData (
  VOID
  )
/*++

Routine Description:

  Allocates a block of memory and writes performance data of booting to OS into it.

Arguments:

  None

Returns:

  None

--*/
{
  EFI_STATUS                Status;
  EFI_CPU_ARCH_PROTOCOL     *Cpu;
  EFI_PERFORMANCE_PROTOCOL  *DrvPerf;
  EFI_PHYSICAL_ADDRESS      mAcpiLowMemoryBase;
  UINT32                    mAcpiLowMemoryLength;
  UINT32                    LimitCount;
  EFI_PERF_HEADER           mPerfHeader;
  EFI_PERF_DATA             mPerfData;
  EFI_GAUGE_DATA            *DumpData;
  EFI_HANDLE                *Handles;
  UINTN                     NoHandles;
  UINT8                     *Ptr;
  UINT8                     *PdbFileName;
  UINT32                    mIndex;
  UINT64                    Ticker;
  UINT64                    Freq;
  UINT32                    Duration;
  UINT64                    CurrentTicker;
  UINT64                    TimerPeriod;

  //
  // Retrive time stamp count as early as possilbe
  //
  Ticker = EfiReadTsc ();

  //
  // Allocate a block of memory that contain performance data to OS
  //
  mAcpiLowMemoryBase = 0xFFFFFFFF;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiReservedMemoryType,
                  4,
                  &mAcpiLowMemoryBase
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }

  mAcpiLowMemoryLength  = EFI_PAGES_TO_SIZE(4);

  Ptr                   = (UINT8 *) ((UINT32) mAcpiLowMemoryBase + sizeof (EFI_PERF_HEADER));
  LimitCount            = (mAcpiLowMemoryLength - sizeof (EFI_PERF_HEADER)) / sizeof (EFI_PERF_DATA);

  //
  // Get performance architecture protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiPerformanceProtocolGuid,
                  NULL,
                  &DrvPerf
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePages (mAcpiLowMemoryBase, 4);
    return ;
  }
  //
  // Initialize performance data structure
  //
  EfiZeroMem (&mPerfHeader, sizeof (EFI_PERF_HEADER));

  //
  // Get CPU frequency
  //
  Status = gBS->LocateProtocol (
                  &gEfiCpuArchProtocolGuid,
                  NULL,
                  &Cpu
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePages (mAcpiLowMemoryBase, 4);
    return ;
  }
  //
  // Get Cpu Frequency
  //
  Status = Cpu->GetTimerValue (Cpu, 0, &(CurrentTicker), &TimerPeriod);
  if (EFI_ERROR (Status)) {
    gBS->FreePages (mAcpiLowMemoryBase, 4);
    return ;
  }

  Freq                = DivU64x32 (1000000000000, (UINTN) TimerPeriod, NULL);

  mPerfHeader.CpuFreq = Freq;

  //
  // Record BDS raw performance data
  //
  mPerfHeader.BDSRaw = Ticker;

  //
  // Put Detailed performance data into memory
  //
  Handles = NULL;
  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &NoHandles,
                  &Handles
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePages (mAcpiLowMemoryBase, 4);
    return ;
  }
  //
  // Get DXE drivers performance
  //
  for (mIndex = 0; mIndex < NoHandles; mIndex++) {

    Ticker = 0;
    PdbFileName = NULL;
    DumpData = DrvPerf->GetGauge (
                          DrvPerf,    // Context
                          NULL,       // Handle
                          NULL,       // Token
                          NULL,       // Host
                          NULL        // PrecGauge
                          );
    while (DumpData) {
      if (DumpData->Handle == Handles[mIndex]) {
      	PdbFileName = &(DumpData->PdbFileName[0]);
      	if (DumpData->StartTick < DumpData->EndTick) {
      	  Ticker += (DumpData->EndTick - DumpData->StartTick);
      	}
      }

      DumpData = DrvPerf->GetGauge (
                            DrvPerf,  // Context
                            NULL,     // Handle
                            NULL,     // Token
                            NULL,     // Host
                            DumpData  // PrecGauge
                            );
    }

    Duration = (UINT32) DivU64x32 (
                          Ticker,
                          (UINT32) Freq,
                          NULL
                          );

    if (Duration > 0) {
      EfiZeroMem (&mPerfData, sizeof (EFI_PERF_DATA));

      if (PdbFileName != NULL) {
        EfiAsciiStrCpy (mPerfData.Token, PdbFileName);
      }
      mPerfData.Duration = Duration;

      EfiCopyMem (Ptr, &mPerfData, sizeof (EFI_PERF_DATA));
      Ptr += sizeof (EFI_PERF_DATA);

      mPerfHeader.Count++;
      if (mPerfHeader.Count == LimitCount) {
        goto Done;
      }
    }
  }

  gBS->FreePool (Handles);

  //
  // Get inserted performance data
  //
  DumpData = DrvPerf->GetGauge (
                        DrvPerf,      // Context
                        NULL,         // Handle
                        NULL,         // Token
                        NULL,         // Host
                        NULL          // PrecGauge
                        );
  while (DumpData) {
    if ((DumpData->Handle) || (DumpData->StartTick > DumpData->EndTick)) {
      DumpData = DrvPerf->GetGauge (
                            DrvPerf,  // Context
                            NULL,     // Handle
                            NULL,     // Token
                            NULL,     // Host
                            DumpData  // PrecGauge
                            );
      continue;
    }

    EfiZeroMem (&mPerfData, sizeof (EFI_PERF_DATA));

    ConvertChar16ToChar8 ((UINT8 *) mPerfData.Token, DumpData->Token);
    mPerfData.Duration = (UINT32) DivU64x32 (
                                    DumpData->EndTick - DumpData->StartTick,
                                    (UINT32) Freq,
                                    NULL
                                    );

    EfiCopyMem (Ptr, &mPerfData, sizeof (EFI_PERF_DATA));
    Ptr += sizeof (EFI_PERF_DATA);

    mPerfHeader.Count++;
    if (mPerfHeader.Count == LimitCount) {
      goto Done;
    }

    DumpData = DrvPerf->GetGauge (
                          DrvPerf,    // Context
                          NULL,       // Handle
                          NULL,       // Token
                          NULL,       // Host
                          DumpData    // PrecGauge
                          );
  }

Done:

  mPerfHeader.Signiture = 0x66726550;

  //
  // Put performance data to memory
  //
  EfiCopyMem (
    (UINTN *) (UINTN) mAcpiLowMemoryBase,
    &mPerfHeader,
    sizeof (EFI_PERF_HEADER)
    );

  gRT->SetVariable (
        L"PerfDataMemAddr",
        &gEfiGenericVariableGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        sizeof (UINT32),
        (VOID *) &mAcpiLowMemoryBase
        );

  return ;
}
#endif
