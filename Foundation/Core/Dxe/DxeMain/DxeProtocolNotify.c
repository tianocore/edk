/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DxeProtocolNotify.c

Abstract:

  This file deals with Architecture Protocol (AP) registration in 
  the Dxe Core. The mArchProtocols[] array represents a list of 
  events that represent the Architectural Protocols.

--*/

#include "Tiano.h"
#include "DxeCore.h"


//
// DXE Core Global Variables for all of the Architectural Protocols.
// If a protocol is installed mArchProtocols[].Present will be TRUE.
//
// CoreNotifyOnArchProtocolInstallation () fills in mArchProtocols[].Event
// and mArchProtocols[].Registration as it creates events for every array
// entry.
//

ARCHITECTURAL_PROTOCOL_ENTRY  mArchProtocols[] = {
  { &gEfiSecurityArchProtocolGuid,         &gSecurity,      NULL, NULL, FALSE},
  { &gEfiCpuArchProtocolGuid,              &gCpu,           NULL, NULL, FALSE},
  { &gEfiMetronomeArchProtocolGuid,        &gMetronome,     NULL, NULL, FALSE},
  { &gEfiTimerArchProtocolGuid,            &gTimer,         NULL, NULL, FALSE},
  { &gEfiBdsArchProtocolGuid,              &gBds,           NULL, NULL, FALSE},
  { &gEfiWatchdogTimerArchProtocolGuid,    &gWatchdogTimer, NULL, NULL, FALSE},
  { &gEfiRuntimeArchProtocolGuid,          &gRuntime,       NULL, NULL, FALSE},
  { &gEfiVariableArchProtocolGuid,         NULL,            NULL, NULL, FALSE},
  { &gEfiVariableWriteArchProtocolGuid,    NULL,            NULL, NULL, FALSE},
  #if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  { &gEfiCapsuleArchProtocolGuid,          NULL,            NULL, NULL, FALSE},
  #endif
  { &gEfiMonotonicCounterArchProtocolGuid, NULL,            NULL, NULL, FALSE},
  { &gEfiResetArchProtocolGuid,            NULL,            NULL, NULL, FALSE},
  { &gEfiStatusCodeRuntimeProtocolGuid,    NULL,            NULL, NULL, FALSE},
  { &gEfiRealTimeClockArchProtocolGuid,    NULL,            NULL, NULL, FALSE},
  NULL
};


EFI_STATUS
CoreAllEfiServicesAvailable (
  VOID
  )
/*++

Routine Description:
  Return TRUE if all AP services are availible.

Arguments:
  NONE

Returns:
  EFI_SUCCESS   - All AP services are available
  EFI_NOT_FOUND - At least one AP service is not available 

--*/
{
  ARCHITECTURAL_PROTOCOL_ENTRY  *Entry;

  for (Entry = mArchProtocols; Entry->ProtocolGuid != NULL; Entry++) {
    if (!Entry->Present) {
      return EFI_NOT_FOUND;
    }
  }

  return EFI_SUCCESS;
}


VOID
EFIAPI
GenericArchProtocolNotify (
  IN EFI_EVENT       Event,
  IN VOID            *Context
  )
/*++

Routine Description:
  Notification event handler registered by CoreNotifyOnArchProtocolInstallation ().
  This notify function is registered for every architectural protocol. This handler
  updates mArchProtocol[] array entry with protocol instance data and sets it's 
  present flag to TRUE. If any constructor is required it is executed. The EFI 
  System Table headers are updated.

Arguments:

  Event   - The Event that is being processed, not used.
  
  Context - Event Context, not used.

Returns:

  None

--*/
{
  EFI_STATUS                      Status;
  ARCHITECTURAL_PROTOCOL_ENTRY    *Entry;
  VOID                            *Protocol;
  BOOLEAN                         Found;
  EFI_LIST_ENTRY                  *Link;
  EFI_LIST_ENTRY                  TempLinkNode;

  Found = FALSE;
  for (Entry = mArchProtocols; Entry->ProtocolGuid != NULL; Entry++) {
 
    Status = CoreLocateProtocol (Entry->ProtocolGuid, Entry->Registration, &Protocol);
    if (EFI_ERROR (Status)) {
      continue;
    } 
    
    Found = TRUE;
    Entry->Present = TRUE;
    
    //
    // Update protocol global variable if one exists. Entry->Protocol points to a global variable
    // if one exists in the DXE core for this Architectural Protocol 
    //
    if (Entry->Protocol != NULL) {
      *(Entry->Protocol) = Protocol;
    }

    if (EfiCompareGuid (Entry->ProtocolGuid, &gEfiTimerArchProtocolGuid)) {
      //
      // Register the Core timer tick handler with the Timer AP
      //
      gTimer->RegisterHandler (gTimer, CoreTimerTick);
    }

    if (EfiCompareGuid (Entry->ProtocolGuid, &gEfiRuntimeArchProtocolGuid)) {
      //
      // When runtime architectural protocol is available, updates CRC32 in the Debug Table
      //
      CoreUpdateDebugTableCrc32 ();

      //
      // Update the Runtime Architectural protocol with the template that the core was
      // using so there would not need to be a dependency on the Runtime AP
      //

      //
      // Copy all the registered Image to new gRuntime protocol
      //
      for (Link = gRuntimeTemplate.ImageHead.ForwardLink; Link != &gRuntimeTemplate.ImageHead; Link = TempLinkNode.ForwardLink) {
        EfiCommonLibCopyMem (&TempLinkNode, Link, sizeof(EFI_LIST_ENTRY));
        InsertTailList (&gRuntime->ImageHead, Link);
      }
      //
      // Copy all the registered Event to new gRuntime protocol
      //
      for (Link = gRuntimeTemplate.EventHead.ForwardLink; Link != &gRuntimeTemplate.EventHead; Link = TempLinkNode.ForwardLink) {
        EfiCommonLibCopyMem (&TempLinkNode, Link, sizeof(EFI_LIST_ENTRY));
        InsertTailList (&gRuntime->EventHead, Link);
      }
      
      //
      // Clean up gRuntimeTemplate
      //
      gRuntimeTemplate.ImageHead.ForwardLink = &gRuntimeTemplate.ImageHead;
      gRuntimeTemplate.ImageHead.BackLink    = &gRuntimeTemplate.ImageHead;
      gRuntimeTemplate.EventHead.ForwardLink = &gRuntimeTemplate.EventHead;
      gRuntimeTemplate.EventHead.BackLink    = &gRuntimeTemplate.EventHead;
    }

    if (EfiCompareGuid (Entry->ProtocolGuid, &gEfiStatusCodeRuntimeProtocolGuid)) {
      //
      // Update StatusCode instance used by DXE core
      //
      gStatusCode = (EFI_STATUS_CODE_PROTOCOL *) Protocol;
    }
  }

  //
  // It's over kill to do them all every time, but it saves a lot of code.
  //
  if (Found) {
    CalculateEfiHdrCrc (&gRT->Hdr);
    CalculateEfiHdrCrc (&gBS->Hdr);
    CalculateEfiHdrCrc (&gST->Hdr);
    CalculateEfiHdrCrc (&gDS->Hdr);
  }
}



VOID
CoreNotifyOnArchProtocolInstallation (
  VOID
  )
/*++

Routine Description:
  Creates an event that is fired everytime a Protocol of a specific type is installed

Arguments:
  NONE

Returns:
  NONE

--*/
{
  EFI_STATUS                      Status;
  ARCHITECTURAL_PROTOCOL_ENTRY    *Entry;

  for (Entry = mArchProtocols; Entry->ProtocolGuid != NULL; Entry++) {
    
    //
    // Create the event
    //
    Status = CoreCreateEvent (
              EFI_EVENT_NOTIFY_SIGNAL,
              EFI_TPL_CALLBACK,
              GenericArchProtocolNotify,
              NULL,
              &Entry->Event
              );
    ASSERT_EFI_ERROR(Status);

    //
    // Register for protocol notifactions on this event
    //
    Status = CoreRegisterProtocolNotify (
              Entry->ProtocolGuid, 
              Entry->Event, 
              &Entry->Registration
              );
    ASSERT_EFI_ERROR(Status);

  }
}

#ifdef EFI_DEBUG
//
// Following is needed to display missing architectural protocols in debug builds
//
typedef struct {
  EFI_GUID                    *ProtocolGuid;
  CHAR16                       *GuidString;
} GUID_TO_STRING_PROTOCOL_ENTRY;

GUID_TO_STRING_PROTOCOL_ENTRY MissingProtocols[] = {
  { &gEfiSecurityArchProtocolGuid,         L"Security"           },
  { &gEfiCpuArchProtocolGuid,              L"CPU"                },
  { &gEfiMetronomeArchProtocolGuid,        L"Metronome"          },
  { &gEfiTimerArchProtocolGuid,            L"Timer"              },
  { &gEfiBdsArchProtocolGuid,              L"Bds"                },
  { &gEfiWatchdogTimerArchProtocolGuid,    L"Watchdog Timer"     },
  { &gEfiRuntimeArchProtocolGuid,          L"Runtime"            },
  { &gEfiVariableArchProtocolGuid,         L"Variable"           },
  { &gEfiVariableWriteArchProtocolGuid,    L"Variable Write"     },
  #if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  { &gEfiCapsuleArchProtocolGuid,          L"Capsule"            },
  #endif
  { &gEfiMonotonicCounterArchProtocolGuid, L"Monotonic Counter"  },
  { &gEfiResetArchProtocolGuid,            L"Reset"              },
  { &gEfiStatusCodeRuntimeProtocolGuid,    L"Status Code"        },
  { &gEfiRealTimeClockArchProtocolGuid,    L"Real Time Clock"    }
};
#endif

DEBUG_CODE (
VOID
CoreDisplayMissingArchProtocols (
  VOID
  )
/*++

Routine Description:
  Displays Architectural protocols that were not loaded and are required for DXE core to function
  Only used in Debug Builds

Arguments:
  NONE

Returns:
  NONE

--*/
{
        

  GUID_TO_STRING_PROTOCOL_ENTRY *MissingEntry;
  
  ARCHITECTURAL_PROTOCOL_ENTRY  *Entry;


  for (Entry = mArchProtocols; Entry->ProtocolGuid != NULL; Entry++) {
    if (!Entry->Present) {
        MissingEntry = MissingProtocols;
  for (MissingEntry = MissingProtocols; TRUE ; MissingEntry++) {
    if (EfiCompareGuid (Entry->ProtocolGuid, MissingEntry->ProtocolGuid)) {
      DEBUG ((EFI_D_ERROR, "\n%s Arch Protocol not present!!\n", MissingEntry->GuidString));
            break;
    }
  }
    }
   
  } 
}
)
