/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciEnumerator.h
  
Abstract:

  PCI Bus Driver

Revision History

--*/

#ifndef _EFI_PCI_ENUMERATOR_H
#define _EFI_PCI_ENUMERATOR_H

#include "PciResourceSupport.h"
#include EFI_PROTOCOL_DEFINITION(PciHostBridgeResourceAllocation)


EFI_STATUS
PciEnumerator (
  IN EFI_HANDLE                    Controller
);

EFI_STATUS
PciRootBridgeEnumerator (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  *PciResAlloc,
  IN PCI_IO_DEVICE                                     *RootBridgeDev
);

EFI_STATUS
ProcessOptionRom (
  IN PCI_IO_DEVICE *Bridge,
  IN UINT64        RomBase,
  IN UINT64        MaxLength
);

EFI_STATUS
PciAssignBusNumber (
  IN PCI_IO_DEVICE                      *Bridge,  
  IN UINT8                              StartBusNumber,
  OUT UINT8                             *SubBusNumber
);

EFI_STATUS
DetermineRootBridgeAttributes (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *PciResAlloc,
  IN PCI_IO_DEVICE                                    *RootBridgeDev
);

UINT64 
GetMaxOptionRomSize(
  IN PCI_IO_DEVICE   *Bridge
);

EFI_STATUS 
PciHostBridgeDeviceAttribute (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *PciResAlloc 
);

EFI_STATUS 
GetResourceAllocationStatus (
  VOID        *AcpiConfig,
  OUT UINT64  *IoResStatus,
  OUT UINT64  *Mem32ResStatus,
  OUT UINT64  *PMem32ResStatus,
  OUT UINT64  *Mem64ResStatus,
  OUT UINT64  *PMem64ResStatus
);

EFI_STATUS 
RejectPciDevice(
  IN PCI_IO_DEVICE       *PciDevice
);

BOOLEAN
IsRejectiveDevice(
IN  PCI_RESOURCE_NODE   *PciResNode
);

PCI_RESOURCE_NODE *
GetLargerConsumerDevice(
  IN  PCI_RESOURCE_NODE   *PciResNode1,
  IN  PCI_RESOURCE_NODE   *PciResNode2
);

PCI_RESOURCE_NODE *
GetMaxResourceConsumerDevice (
  IN  PCI_RESOURCE_NODE   *ResPool
);

EFI_STATUS 
PciHostBridgeAdjustAllocation (
  IN  PCI_RESOURCE_NODE   *IoPool,
  IN  PCI_RESOURCE_NODE   *Mem32Pool,
  IN  PCI_RESOURCE_NODE   *PMem32Pool,
  IN  PCI_RESOURCE_NODE   *Mem64Pool,
  IN  PCI_RESOURCE_NODE   *PMem64Pool,  
  IN  UINT64  IoResStatus,
  IN  UINT64  Mem32ResStatus,
  IN  UINT64  PMem32ResStatus,
  IN  UINT64  Mem64ResStatus,
  IN  UINT64  PMem64ResStatus
);


EFI_STATUS
ConstructAcpiResourceRequestor (
  IN PCI_IO_DEVICE      *Bridge,
  IN PCI_RESOURCE_NODE  *IoNode,
  IN PCI_RESOURCE_NODE  *Mem32Node,
  IN PCI_RESOURCE_NODE  *PMem32Node,
  IN PCI_RESOURCE_NODE  *Mem64Node,
  IN PCI_RESOURCE_NODE  *PMem64Node,
  OUT VOID              **pConfig
);

EFI_STATUS 
GetResourceBase (
  IN VOID     *pConfig,
  OUT UINT64  *IoBase,
  OUT UINT64  *Mem32Base,
  OUT UINT64  *PMem32Base,
  OUT UINT64  *Mem64Base,
  OUT UINT64  *PMem64Base
);

EFI_STATUS
PciBridgeEnumerator (
  IN PCI_IO_DEVICE                                     *BridgeDev
);

EFI_STATUS 
PciBridgeResourceAllocator (
  IN PCI_IO_DEVICE  *Bridge
);

EFI_STATUS 
GetResourceBaseFromBridge (
  IN  PCI_IO_DEVICE *Bridge,
  OUT UINT64  *IoBase,
  OUT UINT64  *Mem32Base,
  OUT UINT64  *PMem32Base,
  OUT UINT64  *Mem64Base,
  OUT UINT64  *PMem64Base
);

EFI_STATUS 
PciHostBridgeP2CProcess (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *PciResAlloc   
);

EFI_STATUS
NotifyPhase (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *PciResAlloc,
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE Phase
);

EFI_STATUS
PreprocessController (
  IN PCI_IO_DEVICE                      *Bridge,  
  IN UINT8                              Bus,
  IN UINT8                              Device,
  IN UINT8                              Func,
  IN EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE   Phase
);


EFI_STATUS
PciHotPlugRequestNotify (
 IN EFI_PCI_HOTPLUG_REQUEST_PROTOCOL *This,
 IN EFI_PCI_HOTPLUG_OPERATION        Operation,
 IN EFI_HANDLE                       Controller,
 IN EFI_DEVICE_PATH_PROTOCOL         *RemainingDevicePath OPTIONAL,
 IN OUT UINT8                        *NumberOfChildren,
 IN OUT EFI_HANDLE                   *ChildHandleBuffer
);

BOOLEAN
SearchHostBridgeHandle(
  IN EFI_HANDLE RootBridgeHandle
);

EFI_STATUS
AddHostBridgeEnumerator(
  IN EFI_HANDLE HostBridgeHandle
);

#endif