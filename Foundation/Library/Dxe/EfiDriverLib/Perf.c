/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Perf.c

Abstract:

  Support for Performance primatives. 

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"
#include EFI_PROTOCOL_DEFINITION (Performance)
#include EFI_GUID_DEFINITION (Hob)
#include EFI_GUID_DEFINITION (PeiPerformanceHob)
#include "linkedlist.h"
#include "EfiHobLib.h"

EFI_STATUS
GetTimerValue (
  OUT UINT64    *TimerValue
  );

EFI_STATUS
GetPeiPerformance (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN UINT64               Ticker
  );

#define EFI_PERFORMANCE_DATA_SIGNATURE  EFI_SIGNATURE_32 ('P', 'E', 'D', 'A')

typedef struct {
  UINT32          Signature;
  EFI_LIST_ENTRY  Link;
  EFI_GAUGE_DATA  GaugeData;
} EFI_PERF_DATA_LIST;

#define GAUGE_DATA_FROM_LINK(_link)  \
            CR(_link, EFI_PERF_DATA_LIST, Link, EFI_PERFORMANCE_DATA_SIGNATURE)

#define GAUGE_DATA_FROM_GAUGE(_GaugeData)  \
            CR(_GaugeData, EFI_PERF_DATA_LIST, GaugeData, EFI_PERFORMANCE_DATA_SIGNATURE)

#define EFI_PERFORMANCE_SIGNATURE         EFI_SIGNATURE_32 ('P', 'E', 'R', 'F')

//
// Performance protocol instance data structure
//
typedef struct {
  UINTN                     Signature;
  EFI_HANDLE                Handle;
  EFI_PERFORMANCE_PROTOCOL  Perf;
  UINT8                     Phase;
} EFI_PERFORMANCE_INSTANCE;

//
// Performace protocol instance containing record macro
//

#define EFI_PERFORMANCE_FROM_THIS(a) \
  CR(a, EFI_PERFORMANCE_INSTANCE, Perf, EFI_PERFORMANCE_SIGNATURE)

EFI_LIST_ENTRY  mPerfDataHead = INITIALIZE_LIST_HEAD_VARIABLE(mPerfDataHead);

EFI_PERF_DATA_LIST *
CreateDataNode (
  IN EFI_HANDLE       Handle,
  IN UINT16           *Token,
  IN UINT16           *Host
  )
/*++

Routine Description:

  Create a EFI_PERF_DATA_LIST data node.

Arguments:

  Handle  - Handle of gauge data
  Token   - Token of gauge data
  Host    - Host of gauge data

Returns:

  Pointer to a data node created.

--*/
{
  EFI_PERF_DATA_LIST  *Node;

  //
  // Al\ a new image structure
  //
  Node = EfiLibAllocateZeroPool (sizeof (EFI_PERF_DATA_LIST));
  if (Node != NULL) {

    Node->Signature         = EFI_PERFORMANCE_DATA_SIGNATURE;

    Node->GaugeData.Handle  = Handle;

    if (Token != NULL) {
      EfiStrCpy ((Node->GaugeData).Token, Token);
    }

    if (Host != NULL) {
      EfiStrCpy ((Node->GaugeData).Host, Host);
    }
  }

  return Node;
}


EFI_PERF_DATA_LIST *
GetDataNode (
  IN EFI_HANDLE        Handle,
  IN UINT16            *Token,
  IN UINT16            *Host,
  IN EFI_GUID          *GuidName,
  IN EFI_GAUGE_DATA    *PrevGauge
  )
/*++

Routine Description:

  Search gauge node list to find one node with matched handle, token, host and Guid name.

Arguments:

  Handle    - Handle to match
  Token     - Token to match
  Host      - Host to match
  GuidName  - Guid name to match
  PrevGauge - Start node, start from list head if NULL

Returns:

  Return pointer to the node found, NULL if not found.

--*/
{
  EFI_PERF_DATA_LIST  *Node;
  EFI_PERF_DATA_LIST  *Temp;
  EFI_PERF_DATA_LIST  *Temp2;
  EFI_LIST_ENTRY      *CurrentLink;
  EFI_GUID            NullGuid = EFI_NULL_GUID;

  Node      = NULL;
  Temp      = NULL;
  Temp2     = NULL;

  if (PrevGauge == NULL) {
    CurrentLink = mPerfDataHead.ForwardLink;
  } else {
    Temp2       = GAUGE_DATA_FROM_GAUGE (PrevGauge);
    CurrentLink = (Temp2->Link).ForwardLink;
  }

  while (CurrentLink && CurrentLink != &mPerfDataHead) {
    Node = GAUGE_DATA_FROM_LINK (CurrentLink);

    if (Handle == 0 && Token == NULL && Host == NULL && GuidName == NULL) {
      return Node;
    }

    if (Handle != (Node->GaugeData).Handle) {
      CurrentLink = CurrentLink->ForwardLink;
      continue;
    }

    if (GuidName == NULL && !EfiCompareGuid (&((Node->GaugeData).GuidName), &NullGuid)) {
      CurrentLink = CurrentLink->ForwardLink;
      continue;
    }

    if (GuidName && !EfiCompareGuid (&((Node->GaugeData).GuidName), GuidName)) {
      CurrentLink = CurrentLink->ForwardLink;
      continue;
    }

    if (Token == NULL && EfiStrCmp (Node->GaugeData.Token, L"")) {
      CurrentLink = CurrentLink->ForwardLink;
      continue;
    }

    if (Token && EfiStrCmp (Node->GaugeData.Token, Token)) {
      CurrentLink = CurrentLink->ForwardLink;
      continue;
    }

    if (Host == NULL && EfiStrCmp (Node->GaugeData.Host, L"")) {
      CurrentLink = CurrentLink->ForwardLink;
      continue;
    }

    if (Host && EfiStrCmp (Node->GaugeData.Host, Host)) {
      CurrentLink = CurrentLink->ForwardLink;
      continue;
    }

    Temp = Node;
    break;
  }

  return Temp;
}


EFI_STATUS
StartGauge (
  IN EFI_PERFORMANCE_PROTOCOL         *This,
  IN EFI_HANDLE                       Handle,
  IN UINT16                           *Token,
  IN UINT16                           *Host,
  IN UINT64                           Ticker
  )
/*++

Routine Description:

  Create a guage data node and initialized it.

Arguments:

  This    - Calling context
  Handle  - Handle of gauge data
  Token   - Token of gauge data
  Host    - Host of gauge data
  Ticker  - Set gauge data's StartTick. If 0, StartTick is current timer.

Returns:

  EFI_SUCCESS     - Successfully create and initialized a guage data node.
  EFI_OUT_OF_RESOURCES  - No enough resource to create a guage data node.

--*/
{
  EFI_PERFORMANCE_INSTANCE  *PerfInstance;
  EFI_PERF_DATA_LIST        *Node;
  UINT64                    TimerValue;

  TimerValue    = 0;
  PerfInstance  = EFI_PERFORMANCE_FROM_THIS (This);

  Node          = CreateDataNode (Handle, Token, Host);
  if (!Node) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (Ticker != 0) {
    TimerValue = Ticker;
  } else {
    GetTimerValue (&TimerValue);
  }

  Node->GaugeData.StartTick = TimerValue;

  if (!EfiStrCmp (Token, DXE_TOK)) {
    PerfInstance->Phase = DXE_PHASE;
  }

  if (!EfiStrCmp (Token, SHELL_TOK)) {
    PerfInstance->Phase = SHELL_PHASE;
  }

  Node->GaugeData.Phase = PerfInstance->Phase;

  InsertTailList (&mPerfDataHead, &(Node->Link));

  return EFI_SUCCESS;
}


EFI_STATUS
EndGauge (
  IN EFI_PERFORMANCE_PROTOCOL         *This,
  IN EFI_HANDLE                       Handle,
  IN UINT16                           *Token,
  IN UINT16                           *Host,
  IN UINT64                           Ticker
  )
/*++

Routine Description:

  End all unfinished gauge data node that match specified handle, token and host.

Arguments:

  This    - Calling context
  Handle  - Handle to stop
  Token   - Token to stop
  Host    - Host to stop
  Ticker  - End tick, if 0 then get current timer

Returns:

  EFI_NOT_FOUND - Node not found
  EFI_SUCCESS - Gauge data node successfully ended.

--*/
{
  EFI_PERFORMANCE_INSTANCE  *PerfInstance;
  EFI_PERF_DATA_LIST        *Node;
  UINT64                    TimerValue;

  TimerValue    = 0;
  PerfInstance  = EFI_PERFORMANCE_FROM_THIS (This);

  Node          = GetDataNode (Handle, Token, Host, NULL, NULL);
  if (!Node) {
    return EFI_NOT_FOUND;
  }

  while (Node->GaugeData.EndTick != 0) {
    Node = GetDataNode (Handle, Token, Host, NULL, &(Node->GaugeData));
    if (!Node) {
      return EFI_NOT_FOUND;
    }
  }

  if (Ticker != 0) {
    TimerValue = Ticker;
  } else {
    GetTimerValue (&TimerValue);
  }

  Node->GaugeData.EndTick = TimerValue;

  return EFI_SUCCESS;
}


EFI_GAUGE_DATA *
GetGauge (
  IN EFI_PERFORMANCE_PROTOCOL         *This,
  IN EFI_HANDLE                       Handle,
  IN UINT16                           *Token,
  IN UINT16                           *Host,
  IN EFI_GAUGE_DATA                   *PrevGauge
  )
/*++

Routine Description:
  Get gauge.

Arguments:
  This        - A pointer to the EFI_PERFORMANCE_PROTOCOL.
  Handle      - A pointer of a efi handle.
  Token       - A pointer to the token.
  Host        - A pointer to the host.
  PrevGauge   - A pointer to the EFI_GAUGE_DATA structure.


Returns:
  Status code.

--*/
{
  EFI_PERFORMANCE_INSTANCE  *PerfInstance;
  EFI_PERF_DATA_LIST        *Node;

  PerfInstance  = EFI_PERFORMANCE_FROM_THIS (This);

  Node          = GetDataNode (Handle, Token, Host, NULL, PrevGauge);
  if (Node != NULL) {
    return &(Node->GaugeData);
  } else {
    return NULL;
  }
}

//
// Driver entry point
//
EFI_STATUS
InitializePerformanceInfrastructure (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN UINT64               Ticker
  )
/*++

Routine Description:

  Install gEfiPerformanceProtocolGuid protocol and transfer PEI performance to gauge data nodes.

Arguments:

  ImageHandle - Standard driver entry point parameter
  SystemTable - Standard driver entry point parameter
  Ticker      - End tick for PEI performance

Returns:

  EFI_OUT_OF_RESOURCES - No enough buffer to allocate
  EFI_SUCCESS - Protocol installed.

--*/
{
  EFI_STATUS                Status;
  EFI_PERFORMANCE_INSTANCE  *PerfInstance;

  //
  // Allocate a new image structure
  //
  PerfInstance = EfiLibAllocateZeroPool (sizeof (EFI_PERFORMANCE_INSTANCE));
  if (PerfInstance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  PerfInstance->Signature       = EFI_PERFORMANCE_SIGNATURE;
  PerfInstance->Perf.StartGauge = StartGauge;
  PerfInstance->Perf.EndGauge   = EndGauge;
  PerfInstance->Perf.GetGauge   = GetGauge;

  //
  // Install the protocol interfaces
  //
  Status = gBS->InstallProtocolInterface (
                  &PerfInstance->Handle,
                  &gEfiPerformanceProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &PerfInstance->Perf
                  );

  if (!EFI_ERROR (Status)) {
    GetPeiPerformance (ImageHandle, SystemTable, Ticker);
  }

  return EFI_SUCCESS;
}


EFI_STATUS
StartMeasure (
  EFI_HANDLE          Handle,
  IN UINT16           *Token,
  IN UINT16           *Host,
  IN UINT64           Ticker
  )
/*++

Routine Description:

  Start to gauge on a specified handle, token and host, with Ticker as start tick.

Arguments:

  Handle  - Handle to measure
  Token   - Token to measure
  Host    - Host to measure
  Ticker  - Ticker as start tick

Returns:

  Status code.

--*/
{
  EFI_STATUS                Status;
  EFI_PERFORMANCE_PROTOCOL  *Perf;

  Status = gBS->LocateProtocol (&gEfiPerformanceProtocolGuid, NULL, (VOID **) &Perf);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Perf->StartGauge (Perf, Handle, Token, Host, Ticker);

}


EFI_STATUS
EndMeasure (
  EFI_HANDLE          Handle,
  IN UINT16           *Token,
  IN UINT16           *Host,
  IN UINT64           Ticker
  )
/*++

Routine Description:

  End gauging on a specified handle, token and host, with Ticker as end tick.

Arguments:

  Handle  - Handle to stop
  Token   - Token to stop
  Host    - Host to stop
  Ticker  - Ticker as end tick

Returns:

  Status code.

--*/
{
  EFI_STATUS                Status;
  EFI_PERFORMANCE_PROTOCOL  *Perf;

  Status = gBS->LocateProtocol (&gEfiPerformanceProtocolGuid, NULL, (VOID **) &Perf);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  return (Perf->EndGauge( Perf, Handle, Token, Host, Ticker)) ;
}


EFI_STATUS
UpdateMeasure (
  EFI_HANDLE         Handle,
  IN UINT16          *Token,
  IN UINT16          *Host,
  EFI_HANDLE         HandleNew,
  IN UINT16          *TokenNew,
  IN UINT16          *HostNew
  )
/*++

Routine Description:
  Update measure.

Arguments:
  Handle      - A pointer of an efi handle.
  Token       - A pointer to the token.
  Host        - A pointer to the host.
  HandleNew   - A pointer of an new efi handle.
  TokenNew    - A pointer to the new token.
  HostNew     - A pointer to the new host.

Returns:
  Status code.

  EFI_NOT_FOUND       - The speicified gauge data node not found.
  
  EFI_SUCCESS         - Update successfully.

--*/
{
  EFI_STATUS                Status;
  EFI_GAUGE_DATA            *GaugeData;
  EFI_PERFORMANCE_PROTOCOL  *Perf;

  Status = gBS->LocateProtocol (&gEfiPerformanceProtocolGuid, NULL, (VOID **) &Perf);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  GaugeData = Perf->GetGauge (Perf, Handle, Token, Host, NULL);
  if (!GaugeData) {
    return EFI_NOT_FOUND;
  }

  GaugeData->Handle = HandleNew;
  if (HostNew != NULL) {
    EfiStrCpy (GaugeData->Host, HostNew);
  } else {
    EfiStrCpy (GaugeData->Host, L"");
  }

  if (TokenNew != NULL) {
    EfiStrCpy (GaugeData->Token, TokenNew);
  } else {
    EfiStrCpy (GaugeData->Token, L"");
  }

  return EFI_SUCCESS;
}


EFI_STATUS
GetPeiPerformance (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN UINT64               Ticker
  )
/*++

Routine Description:

  Transfer PEI performance data to gauge data node.

Arguments:

  ImageHandle - Standard entry point parameter
  SystemTable - Standard entry point parameter
  Ticker      - Start tick

Returns:

  EFI_OUT_OF_RESOURCES - No enough resource to create data node.
  EFI_SUCCESS - Transfer done successfully.

--*/
{
  EFI_STATUS                        Status;
  VOID                              *HobList;
  EFI_HOB_GUID_DATA_PERFORMANCE_LOG *LogHob;
  PEI_PERFORMANCE_MEASURE_LOG_ENTRY *LogEntry;
  UINT32                            Index;
  EFI_PERF_DATA_LIST                *Node;
  UINT64                            TimerValue;

  Node = CreateDataNode (0, PEI_TOK, NULL);
  if (!Node) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (Ticker != 0) {
    TimerValue = Ticker;
  } else {
    GetTimerValue (&TimerValue);
  }
  (Node->GaugeData).EndTick = TimerValue;

  InsertTailList (&mPerfDataHead, &(Node->Link));

  EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);
  do {
    Status = GetNextGuidHob (&HobList, &gEfiPeiPerformanceHobGuid, (VOID **) &LogHob, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }

    for (Index = 0; Index < LogHob->NumberOfEntries; Index++) {
      LogEntry  = &(LogHob->Log[Index]);
      Node      = CreateDataNode (0, LogEntry->DescriptionString, NULL);
      if (!Node) {
        return EFI_OUT_OF_RESOURCES;
      }
      (Node->GaugeData).StartTick = LogEntry->StartTimeCount;

      EfiCopyMem (&(Node->GaugeData.GuidName), &LogEntry->Name, sizeof (EFI_GUID));

      InsertTailList (&mPerfDataHead, &(Node->Link));

      (Node->GaugeData).EndTick = LogEntry->StopTimeCount;
    }
  } while (!EFI_ERROR (Status));

  return EFI_SUCCESS;
}
