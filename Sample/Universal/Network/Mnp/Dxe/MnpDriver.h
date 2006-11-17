/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MnpDriver.h

Abstract:

--*/

#ifndef _MNP_DRIVER_H_
#define _MNP_DRIVER_H_

#include "NetLib.h"
#include "NetBuffer.h"
#include "MnpDebug.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_CONSUMER (SimpleNetwork)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_PRODUCER (DriverBinding)
#include EFI_PROTOCOL_PRODUCER (ComponentName)
#include EFI_PROTOCOL_PRODUCER (ManagedNetwork)

//
// Required Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gMnpDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gMnpComponentName;

#define MNP_SERVICE_DATA_SIGNATURE  EFI_SIGNATURE_32 ('M', 'n', 'p', 'S')

typedef struct _MNP_SERVICE_DATA {
  UINT32                        Signature;

  EFI_HANDLE                    ControllerHandle;

  EFI_SERVICE_BINDING_PROTOCOL  ServiceBinding;
  EFI_SIMPLE_NETWORK_PROTOCOL   *Snp;

  UINT32                        Mtu;

  NET_LIST_ENTRY                ChildrenList;
  NET_LOCK                      ChildrenListLock;
  UINTN                         ChildrenNumber;
  UINTN                         ConfiguredChildrenNumber;

  NET_LIST_ENTRY                GroupAddressList;
  UINT32                        GroupAddressCount;
  NET_LOCK                      GroupAddressLock;

  NET_LOCK                      TxLock;
  EFI_EVENT                     TxTimeoutEvent;

  NET_BUF_QUEUE                 FreeNbufQue;
  INTN                          NbufCnt;

  EFI_EVENT                     PollTimer;
  BOOLEAN                       EnableSystemPoll;

  EFI_EVENT                     TimeoutCheckTimer;

  UINT32                        UnicastCount;
  UINT32                        BroadcastCount;
  UINT32                        MulticastCount;
  UINT32                        PromiscuousCount;

  //
  // The size of the data buffer in the MNP_PACKET_BUFFER used to
  // store a packet.
  //
  UINT32                        BufferLength;
  NET_BUF                       *RxNbufCache;
  UINT8                         *TxBuf;
} MNP_SERVICE_DATA;

#define MNP_SERVICE_DATA_FROM_THIS(a) \
  CR ( \
  (a), \
  MNP_SERVICE_DATA, \
  ServiceBinding, \
  MNP_SERVICE_DATA_SIGNATURE \
  )

EFI_STATUS
EFIAPI
MnpDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     * RemainingDevicePath OPTIONAL
  );

EFI_STATUS
EFIAPI
MnpDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     * RemainingDevicePath OPTIONAL
  );

EFI_STATUS
EFIAPI
MnpDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  );

EFI_STATUS
EFIAPI
MnpServiceBindingCreateChild (
  IN EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                    *ChildHandle
  );

EFI_STATUS
EFIAPI
MnpServiceBindingDestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                    ChildHandle
  );

#endif
