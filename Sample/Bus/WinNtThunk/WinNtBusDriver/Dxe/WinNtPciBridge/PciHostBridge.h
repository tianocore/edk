/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciHostBridge.h

Abstract:
  
  The Header file of the Pci Host Bridge Driver 

Revision History

--*/

#ifndef _PCI_HOST_BRIDGE_H_
#define _PCI_HOST_BRIDGE_H_

#include "Tiano.h"
#include "Pci22.h"
#include "EfiDriverLib.h"

#include EFI_PROTOCOL_DEFINITION(PciHostBridgeResourceAllocation)

#define HOST_BRIDGE_NUMBER  1

#define PCI_HOST_BRIDGE_SIGNATURE  EFI_SIGNATURE_32('e', 'h', 's', 't')
typedef struct {
  UINTN               Signature;
  EFI_HANDLE          HostBridgeHandle;
  UINTN               RootBridgeNumber;
  EFI_LIST_ENTRY      Head;
  BOOLEAN             ResourceSubmited;  
  BOOLEAN             CanRestarted;  
  
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL        ResAlloc;
} PCI_HOST_BRIDGE_INSTANCE;

#define INSTANCE_FROM_RESOURCE_ALLOCATION_THIS(a) \
  CR(a, PCI_HOST_BRIDGE_INSTANCE, ResAlloc, PCI_HOST_BRIDGE_SIGNATURE)

extern PCI_CONFIG_ACCESS_CF8 gConfigData;

EFI_STATUS
InitializePciHostBridge (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  );

//
//  HostBridge Resource Allocation interface
//
EFI_STATUS
NotifyPhase (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE    Phase
  );

EFI_STATUS
GetNextRootBridge (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN OUT EFI_HANDLE                                   *RootBridgeHandle
  );
  
EFI_STATUS
GetAttributes (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  OUT UINT64                                          *Attributes
  );
  
EFI_STATUS
StartBusEnumeration (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  OUT VOID                                            **Configuration
  );
  
EFI_STATUS
SetBusNumbers (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  IN VOID                                             *Configuration
  );
  
EFI_STATUS
SubmitResources (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  IN VOID                                             *Configuration
  );
  
EFI_STATUS
GetProposedResources (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  OUT VOID                                            **Configuration
  );

EFI_STATUS
PreprocessController (
  IN  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  *This,
  IN  EFI_HANDLE                                        RootBridgeHandle,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS       PciAddress,
  IN  EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE      Phase
  );
  
#endif
