/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Event.c

Abstract:

  Support for Event lib fucntions.

--*/

#include "Tiano.h"
#include "EfiRuntimeLib.h"

EFI_EVENT
RtEfiLibCreateProtocolNotifyEvent (
  IN EFI_GUID             *ProtocolGuid,
  IN EFI_TPL              NotifyTpl,
  IN EFI_EVENT_NOTIFY     NotifyFunction,
  IN VOID                 *NotifyContext,
  OUT VOID                *Registration
  )
/*++

Routine Description:

  Create a protocol notification event and return it.

Arguments:

  ProtocolGuid    - Protocol to register notification event on.

  NotifyTpl       - Maximum TPL to single the NotifyFunction.

  NotifyFunction  - EFI notification routine.

  NotifyContext   - Context passed into Event when it is created.

  Registration    - Registration key returned from RegisterProtocolNotify().

Returns: 

  The EFI_EVENT that has been registered to be signaled when a ProtocolGuid 
  is added to the system.

--*/
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  //
  // Create the event
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL,
                  NotifyTpl,
                  NotifyFunction,
                  NotifyContext,
                  &Event
                  );
  ASSERT (!EFI_ERROR (Status));

  //
  // Register for protocol notifactions on this event
  //
  Status = gBS->RegisterProtocolNotify (
                  ProtocolGuid,
                  Event,
                  Registration
                  );

  ASSERT (!EFI_ERROR (Status));

  //
  // Kick the event so we will perform an initial pass of
  // current installed drivers
  //
  gBS->SignalEvent (Event);
  return Event;
}

EFI_STATUS
EfiLibGetSystemConfigurationTable (
  IN EFI_GUID *TableGuid,
  IN OUT VOID **Table
  )
/*++

Routine Description:
  
  Return the EFI 1.0 System Tabl entry with TableGuid

Arguments:

  TableGuid - Name of entry to return in the system table
  Table     - Pointer in EFI system table associated with TableGuid

Returns: 

  EFI_SUCCESS - Table returned;
  EFI_NOT_FOUND - TableGuid not in EFI system table

--*/
{
  UINTN Index;

  for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
    if (EfiCompareGuid (TableGuid, &(gST->ConfigurationTable[Index].VendorGuid))) {
      *Table = gST->ConfigurationTable[Index].VendorTable;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
EfiConvertList (
  IN UINTN                DebugDisposition,
  IN OUT EFI_LIST_ENTRY   *ListHead
  )
/*++

Routine Description:

  Conver the standard Lib double linked list to a virtual mapping.

Arguments:

  DebugDisposition - Argument to EfiConvertPointer (EFI 1.0 API)

  ListHead         - Head of linked list to convert

Returns: 

  EFI_SUCCESS

--*/
{
  EFI_LIST_ENTRY  *Link;
  EFI_LIST_ENTRY  *NextLink;

  //
  // Convert all the ForwardLink & BackLink pointers in the list
  //
  Link = ListHead;
  do {
    NextLink = Link->ForwardLink;

    EfiConvertPointer (
      Link->ForwardLink == ListHead ? DebugDisposition : 0,
      (VOID **) &Link->ForwardLink
      );

    EfiConvertPointer (
      Link->BackLink == ListHead ? DebugDisposition : 0,
      (VOID **) &Link->BackLink
      );

    Link = NextLink;
  } while (Link != ListHead);
  return EFI_SUCCESS;
}
