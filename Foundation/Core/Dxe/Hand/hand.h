/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  hand.h

Abstract:

  EFI internal protocol definitions



Revision History

--*/

#ifndef  _HAND_H_
#define  _HAND_H_

#include "Tiano.h"
#include "DxeCore.h"

//
// IHANDLE - contains a list of protocol handles
//

#define EFI_HANDLE_SIGNATURE            EFI_SIGNATURE_32('h','n','d','l')
typedef struct {
  UINTN               Signature;
  EFI_LIST_ENTRY      AllHandles;     // All handles list of IHANDLE
  EFI_LIST_ENTRY      Protocols;      // List of PROTOCOL_INTERFACE's for this handle
  UINTN               LocateRequest;  // 
  UINT64              Key;            // The Handle Database Key value when this handle was last created or modified
} IHANDLE;

#define ASSERT_IS_HANDLE(a)  ASSERT((a)->Signature == EFI_HANDLE_SIGNATURE)


//
// PROTOCOL_ENTRY - each different protocol has 1 entry in the protocol 
// database.  Each handler that supports this protocol is listed, along
// with a list of registered notifies.
//

#define PROTOCOL_ENTRY_SIGNATURE        EFI_SIGNATURE_32('p','r','t','e')
typedef struct {
  UINTN               Signature;
  EFI_LIST_ENTRY      AllEntries;             // All entries
  EFI_GUID            ProtocolID;             // ID of the protocol
  EFI_LIST_ENTRY      Protocols;              // All protocol interfaces
  EFI_LIST_ENTRY      Notify;                 // Registerd notification handlers
} PROTOCOL_ENTRY;

//
// PROTOCOL_INTERFACE - each protocol installed on a handle is tracked
// with a protocol interface structure
//

#define PROTOCOL_INTERFACE_SIGNATURE  EFI_SIGNATURE_32('p','i','f','c')
typedef struct {
  UINTN                       Signature;
  EFI_HANDLE                  Handle;     // Back pointer
  EFI_LIST_ENTRY              Link;       // Link on IHANDLE.Protocols
  EFI_LIST_ENTRY              ByProtocol; // Link on PROTOCOL_ENTRY.Protocols
  PROTOCOL_ENTRY              *Protocol;  // The protocol ID
  VOID                        *Interface; // The interface value
                                          
  EFI_LIST_ENTRY              OpenList;       // OPEN_PROTOCOL_DATA list.
  UINTN                       OpenListCount;  
  
  EFI_HANDLE                  ControllerHandle;

} PROTOCOL_INTERFACE;

#define OPEN_PROTOCOL_DATA_SIGNATURE  EFI_SIGNATURE_32('p','o','d','l')

typedef struct {
  UINTN                       Signature;
  EFI_LIST_ENTRY              Link;

  EFI_HANDLE                  AgentHandle;
  EFI_HANDLE                  ControllerHandle;
  UINT32                      Attributes;
  UINT32                      OpenCount;
} OPEN_PROTOCOL_DATA;


//
// PROTOCOL_NOTIFY - used for each register notification for a protocol
//

#define PROTOCOL_NOTIFY_SIGNATURE       EFI_SIGNATURE_32('p','r','t','n')
typedef struct {
  UINTN               Signature;
  PROTOCOL_ENTRY      *Protocol;
  EFI_LIST_ENTRY      Link;                   // All notifications for this protocol
  EFI_EVENT           Event;                  // Event to notify
  EFI_LIST_ENTRY      *Position;              // Last position notified
} PROTOCOL_NOTIFY;

//
// Internal prototypes
//


PROTOCOL_ENTRY  *
CoreFindProtocolEntry (
  IN EFI_GUID     *Protocol,
  IN BOOLEAN      Create
  );

VOID
CoreNotifyProtocolEntry (
  IN PROTOCOL_ENTRY       *ProtEntry
  );

PROTOCOL_INTERFACE *
CoreFindProtocolInterface (
  IN IHANDLE              *Handle,
  IN EFI_GUID             *Protocol,
  IN VOID                 *Interface
  );

PROTOCOL_INTERFACE *
CoreRemoveInterfaceFromProtocol (
  IN IHANDLE              *Handle,
  IN EFI_GUID             *Protocol,
  IN VOID                 *Interface
  );

EFI_STATUS
CoreUnregisterProtocolNotify (
  IN EFI_EVENT            Event
  );

EFI_STATUS
CoreDisconnectControllersUsingProtocolInterface (
  IN EFI_HANDLE           UserHandle,
  IN PROTOCOL_INTERFACE   *Prot
  );

VOID
CoreAcquireProtocolLock (
  VOID
  );

VOID
CoreReleaseProtocolLock (
  VOID
  );

EFI_STATUS
CoreValidateHandle (
  IN  EFI_HANDLE                UserHandle
  );

//
// Externs
//

extern EFI_LOCK         gProtocolDatabaseLock;
extern EFI_LIST_ENTRY   gHandleList;
extern UINT64           gHandleDatabaseKey;

#endif
