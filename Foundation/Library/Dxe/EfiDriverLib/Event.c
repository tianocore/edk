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
#include "EfiDriverLib.h"

EFI_EVENT
EfiLibCreateProtocolNotifyEvent (
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
EfiLibNamedEventListen (
  IN EFI_GUID             * Name,
  IN EFI_TPL              NotifyTpl,
  IN EFI_EVENT_NOTIFY     NotifyFunction,
  IN VOID                 *NotifyContext,
  OUT VOID                *Registration OPTIONAL
  )
/*++

Routine Description:
  Listenes to signals on the name.
  EfiLibNamedEventSignal() signals the event.

  NOTE: For now, the named listening/signalling is implemented
  on a protocol interface being installed and uninstalled.
  In the future, this maybe implemented based on a dedicated mechanism.

Arguments:
  Name            - Name to register the listener on.
  NotifyTpl       - Maximum TPL to singnal the NotifyFunction.
  NotifyFunction  - The listener routine.
  NotifyContext   - Context passed into the listener routine.
  Registration    - Registration key returned from RegisterProtocolNotify().
                    It could be NULL.

Returns:
  EFI_SUCCESS if successful.

--*/
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;
  VOID        *RegistrationLocal;

  //
  // Create event
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL,
                  NotifyTpl,
                  NotifyFunction,
                  NotifyContext,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // The Registration is not optional to RegisterProtocolNotify().
  // To make it optional to EfiLibNamedEventListen(), may need to substitute with a local.
  //
  if (Registration != NULL) {
    RegistrationLocal = Registration;
  } else {
    RegistrationLocal = &RegistrationLocal;
  }

  //
  // Register for an installation of protocol interface
  //

  Status = gBS->RegisterProtocolNotify (
                  Name,
                  Event,
                  RegistrationLocal
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
EfiLibNamedEventSignal (
  IN EFI_GUID            *Name
  )
/*++

Routine Description:
  Signals a named event. All registered listeners will run.
  The listeners should register using EfiLibNamedEventListen() function.

  NOTE: For now, the named listening/signalling is implemented
  on a protocol interface being installed and uninstalled.
  In the future, this maybe implemented based on a dedicated mechanism.

Arguments:
  Name - Name to perform the signaling on. The name is a GUID.

Returns:
  EFI_SUCCESS if successfull.

--*/
{
  EFI_STATUS  Status;
  EFI_HANDLE  Handle;

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  Name,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->UninstallProtocolInterface (
                  Handle,
                  Name,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
