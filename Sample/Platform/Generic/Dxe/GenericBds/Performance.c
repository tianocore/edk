/*++

Copyright 2004, Intel Corporation                                                         
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
    *Dest ++ = (UINT8) (*Src ++);
  }
  *Dest = 0;
}

STATIC
VOID
GetShortPdbFileName (
  CHAR8  *PdbFileName,
  CHAR8  *GaugeString
  )
/*++

Routine Description:
  
Arguments:

Returns:

--*/
{
  UINTN  Index;
  UINTN  Index1;
  UINTN  StartIndex;
  UINTN  EndIndex;

  if (PdbFileName == NULL) {
    EfiAsciiStrCpy (GaugeString, " ");
  } else {
    StartIndex = 0;
    for (EndIndex = 0; PdbFileName[EndIndex] != 0; EndIndex++);
    
    for (Index = 0; PdbFileName[Index] != 0; Index++) {
      if (PdbFileName[Index] == '\\') {
        StartIndex = Index+1;
      }
      if (PdbFileName[Index] == '.') {
        EndIndex = Index;
      }
    }

    Index1 = 0;
    for (Index = StartIndex; Index < EndIndex; Index++) {
      GaugeString[Index1] = PdbFileName[Index];
      Index1 ++;
      if (Index1 == EFI_PERF_TOKEN_LENGTH - 1) {
        break;
      }
    }
    GaugeString[Index1] = 0;
  }
  
  return;
}

STATIC
CHAR8 *
GetPdbPath (
  VOID *ImageBase
  )
/*++

Routine Description:

  Located PDB path name in PE image

Arguments:

  ImageBase - base of PE to search

Returns:

  Pointer into image at offset of PDB file name if PDB file name is found,
  Otherwise a pointer to an empty string.

--*/
{
  CHAR8                           *PdbPath;
  UINT32                          DirCount;
  EFI_IMAGE_DOS_HEADER            *DosHdr;
  EFI_IMAGE_NT_HEADERS            *NtHdr;
  EFI_IMAGE_OPTIONAL_HEADER       *OptionalHdr;
  EFI_IMAGE_DATA_DIRECTORY        *DirectoryEntry;
  EFI_IMAGE_DEBUG_DIRECTORY_ENTRY *DebugEntry;
  VOID                            *CodeViewEntryPointer;

  CodeViewEntryPointer = NULL;
  PdbPath = NULL;
  DosHdr = ImageBase;

  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    NtHdr = (EFI_IMAGE_NT_HEADERS *)((UINT8 *)DosHdr + DosHdr->e_lfanew);
    OptionalHdr = (VOID *) &NtHdr->OptionalHeader;
    DirectoryEntry = (EFI_IMAGE_DATA_DIRECTORY *)&(OptionalHdr->DataDirectory\
                                             [EFI_IMAGE_DIRECTORY_ENTRY_DEBUG]);
    if (DirectoryEntry->VirtualAddress != 0) {
      for (DirCount = 0;
           (DirCount < DirectoryEntry->Size /
                            sizeof (EFI_IMAGE_DEBUG_DIRECTORY_ENTRY))
                                         && CodeViewEntryPointer == NULL;
                                                                   DirCount++) {
        DebugEntry = (EFI_IMAGE_DEBUG_DIRECTORY_ENTRY *) (
                      DirectoryEntry->VirtualAddress +
                      (UINTN) ImageBase +
                      DirCount * sizeof (EFI_IMAGE_DEBUG_DIRECTORY_ENTRY)
                      );
        if (DebugEntry->Type == EFI_IMAGE_DEBUG_TYPE_CODEVIEW) {
          CodeViewEntryPointer = (VOID *) ((UINTN) DebugEntry->RVA +
                                                             (UINTN) ImageBase);
          switch (* (UINT32 *) CodeViewEntryPointer) {
            case CODEVIEW_SIGNATURE_NB10:
              PdbPath = (CHAR8 *) CodeViewEntryPointer +
                                   sizeof (EFI_IMAGE_DEBUG_CODEVIEW_NB10_ENTRY);
              break;
            case CODEVIEW_SIGNATURE_RSDS:
              PdbPath = (CHAR8 *) CodeViewEntryPointer +
                                   sizeof (EFI_IMAGE_DEBUG_CODEVIEW_RSDS_ENTRY);
              break;
            default:
              break;
          }
        }
      }
    }
  }
  return (PdbPath);
}

STATIC
VOID
GetNameFromHandle (
  IN  EFI_HANDLE     Handle,
  OUT CHAR8          *GaugeString
  )
{
  EFI_STATUS                   Status;
  EFI_LOADED_IMAGE_PROTOCOL    *Image;
  CHAR8                        *PdbFileName;
  EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding;
  
  EfiAsciiStrCpy (GaugeString, " ");

  //
  //Get handle name from image protocol
  //
  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiLoadedImageProtocolGuid,
                  &Image
                  );

  if (EFI_ERROR (Status)) {
    Status = gBS->OpenProtocol (
                    Handle,
                    &gEfiDriverBindingProtocolGuid,
                    (VOID **)&DriverBinding,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      return;
    }
    //
    //Get handle name from image protocol
    //
    Status = gBS->HandleProtocol (
                    DriverBinding->ImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    &Image
                    );
  }

  PdbFileName = GetPdbPath (Image->ImageBase);

  if (PdbFileName != NULL) {
    GetShortPdbFileName (PdbFileName, GaugeString);
  }

  return;
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
  EFI_STATUS                        Status;
  EFI_CPU_ARCH_PROTOCOL             *Cpu ;
  EFI_PERFORMANCE_PROTOCOL          *DrvPerf;
  EFI_PHYSICAL_ADDRESS              mAcpiLowMemoryBase;
  UINT32                            mAcpiLowMemoryLength;
  UINT32                            LimitCount;
  EFI_PERF_HEADER                   mPerfHeader;
  EFI_PERF_DATA                     mPerfData;
  EFI_GAUGE_DATA                    *DumpData;
  EFI_HANDLE                        *Handles;
  UINTN                             NoHandles;
  CHAR8                             GaugeString[EFI_PERF_TOKEN_LENGTH];
  UINT8                             *Ptr;
  UINT32                            mIndex;
  UINT64                            Ticker;
  UINT64                            Freq;
  UINT32                            Duration;
  UINT64                            CurrentTicker;
  UINT64                            TimerPeriod;

  //
  //Retrive time stamp count as early as possilbe
  //
  Ticker = EfiReadTsc();

  //
  //Allocate a block of memory that contain performance data to OS
  //
  Status = gBS->AllocatePages (
                  AllocateAnyPages,
                  EfiACPIReclaimMemory, 
                  4, 
                  &mAcpiLowMemoryBase
                  );
  if (EFI_ERROR(Status)) {
    return ;
  }

  mAcpiLowMemoryLength = 0x1000;
  
  Ptr = (UINT8*)((UINT32)mAcpiLowMemoryBase + sizeof (EFI_PERF_HEADER));
  LimitCount = (mAcpiLowMemoryLength - sizeof (EFI_PERF_HEADER)) / sizeof (EFI_PERF_DATA);
  
  //
  //Get performance architecture protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiPerformanceProtocolGuid,
                  NULL,
                  &DrvPerf
                  );
  if (EFI_ERROR(Status)) {
    gBS->FreePages (mAcpiLowMemoryBase, 1);
    return;
  }

  //
  //Initialize performance data structure
  //
  EfiZeroMem (&mPerfHeader, sizeof (EFI_PERF_HEADER));
  
  //
  //Get CPU frequency
  //
  Status = gBS->LocateProtocol (
                  &gEfiCpuArchProtocolGuid,
                  NULL,
                  &Cpu
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePages (mAcpiLowMemoryBase, 1);
    return;
  }


  //
  //Get Cpu Frequency
  //
  Status = Cpu -> GetTimerValue (Cpu, 0, &(CurrentTicker), &TimerPeriod) ;
  if (EFI_ERROR(Status)) {
    gBS->FreePages (mAcpiLowMemoryBase, 1);
    return;
  }

  Freq = DivU64x32 (1000000000000, (UINTN)TimerPeriod , NULL) ;

  mPerfHeader.CpuFreq = Freq;

  //
  //Record BDS raw performance data
  //
  mPerfHeader.BDSRaw = Ticker;
  
  //
  //Put Detailed performance data into memory
  //
  Handles = NULL;
  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &NoHandles,
                  &Handles
                  );
  if (EFI_ERROR(Status)) {
    gBS->FreePages (mAcpiLowMemoryBase, 1);
    return;
   }

  //
  //Get DXE drivers performance
  //
  for (mIndex = 0; mIndex < NoHandles; mIndex++) {
    
    Ticker = 0;

    DumpData = DrvPerf->GetGauge (
                          DrvPerf,     //Context
                          NULL,        //Handle
                          NULL,        //Token
                          NULL,        //Host
                          NULL         //PrecGauge
                          );
    while (DumpData) {
      if ((DumpData->Handle == Handles[mIndex]) &&\
          (DumpData->StartTick < DumpData->EndTick)) {
        Ticker += (DumpData->EndTick - DumpData->StartTick);
      }

      DumpData = DrvPerf->GetGauge (
                            DrvPerf,   //Context
                            NULL,        //Handle
                            NULL,        //Token
                            NULL,        //Host
                            DumpData     //PrecGauge
                            );
    }

    Duration = (UINT32) DivU64x32 (
                          Ticker,
                          (UINT32) Freq,
                          NULL
                          );
                                   
    if (Duration > 0) {
      EfiZeroMem (&mPerfData, sizeof (EFI_PERF_DATA));

      GetNameFromHandle (Handles[mIndex], GaugeString);

      EfiAsciiStrCpy (mPerfData.Token, GaugeString);
      mPerfData.Duration = Duration;

      EfiCopyMem (Ptr, &mPerfData, sizeof (EFI_PERF_DATA));
      Ptr += sizeof (EFI_PERF_DATA);

      mPerfHeader.Count ++;
      if (mPerfHeader.Count == LimitCount) {
        goto Done;
      }
    }
  }
  gBS->FreePool (Handles);
  
  //
  //Get inserted performance data
  //
  DumpData = DrvPerf->GetGauge (
                        DrvPerf,   //Context
                        NULL,        //Handle
                        NULL,        //Token
                        NULL,        //Host
                        NULL         //PrecGauge
                        );
  while (DumpData) {
    if ((DumpData->Handle) || (DumpData->StartTick > DumpData->EndTick)) {
      DumpData = DrvPerf->GetGauge (
                            DrvPerf,   //Context
                            NULL,        //Handle
                            NULL,        //Token
                            NULL,        //Host
                            DumpData     //PrecGauge
                            );
      continue;
    }

    EfiZeroMem (&mPerfData, sizeof (EFI_PERF_DATA));

    ConvertChar16ToChar8 ((UINT8*)mPerfData.Token, DumpData->Token);
    mPerfData.Duration = (UINT32) DivU64x32 (
                                    DumpData->EndTick - DumpData->StartTick,
                                    (UINT32) Freq,
                                    NULL
                                    );

    EfiCopyMem (Ptr, &mPerfData, sizeof (EFI_PERF_DATA));
    Ptr += sizeof (EFI_PERF_DATA);

    mPerfHeader.Count ++;
    if (mPerfHeader.Count == LimitCount) {
      goto Done;
    }

    DumpData = DrvPerf->GetGauge (
                          DrvPerf,   //Context
                          NULL,        //Handle
                          NULL,        //Token
                          NULL,        //Host
                          DumpData     //PrecGauge
                          );
  }

Done:

  ClearDebugRegisters ();
  
  mPerfHeader.Signiture = 0x66726550;

  //
  //Put performance data to memory
  //
  EfiCopyMem (
    (UINT32*)(UINT32)mAcpiLowMemoryBase,
    &mPerfHeader,
    sizeof (EFI_PERF_HEADER)
    ); 
  
  gRT->SetVariable (
         L"PerfDataMemAddr", 
         &gEfiGlobalVariableGuid,
         EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
         sizeof (UINT32), 
         (VOID*)&mAcpiLowMemoryBase
         );
  
  return;
}
#endif
