/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciRootBridge.h

Abstract:

  Driver for the host to pci bridge (root bridge).


--*/

#ifndef _PCI_ROOT_BRIDGE_H_
#define _PCI_ROOT_BRIDGE_H_

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "ACPI.h"

//
// Driver Consumed Protocol Prototypes
//

#include EFI_ARCH_PROTOCOL_DEFINITION(Metronome)
#include EFI_PROTOCOL_CONSUMER (CpuIo)

//
// Driver Produced Protocol Prototypes
//

#include EFI_PROTOCOL_DEFINITION(DevicePath)
#include EFI_PROTOCOL_DEFINITION(PciRootBridgeIo)

//
// Define resource status constant 
//
#define EFI_RESOURCE_NONEXISTENT   0xFFFFFFFFFFFFFFFF
#define EFI_RESOURCE_LESS          0xFFFFFFFFFFFFFFFE
#define EFI_RESOURCE_SATISFIED     0x0000000000000000


//
// Driver Instance Data Prototypes
//

typedef struct {
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION  Operation;
  UINTN                                      NumberOfBytes;
  UINTN                                      NumberOfPages;
  EFI_PHYSICAL_ADDRESS                       HostAddress;
  EFI_PHYSICAL_ADDRESS                       MappedHostAddress;
} MAP_INFO;

typedef struct {
  ACPI_HID_DEVICE_PATH              AcpiDevicePath;
  EFI_DEVICE_PATH_PROTOCOL          EndDevicePath;
} EFI_PCI_ROOT_BRIDGE_DEVICE_PATH;

typedef struct {
  UINT64          BusBase;
  UINT64          BusLimit;     
  
  UINT64          MemBase;     
  UINT64          MemLimit;    
  
  UINT64          IoBase; 
  UINT64          IoLimit;     
} PCI_ROOT_BRIDGE_RESOURCE_APPETURE;

typedef enum {
  TypeIo = 0,
  TypeMem32,
  TypePMem32,
  TypeMem64,
  TypePMem64,
  TypeBus,
  TypeMax
} PCI_RESOURCE_TYPE;

typedef enum {
  ResNone = 0,
  ResSubmitted,
  ResRequested,
  ResAllocated,
  ResStatusMax
} RES_STATUS;

typedef struct {
  PCI_RESOURCE_TYPE Type;
  UINT64            Base;
  UINT64            Length;
  UINT64            Alignment;
  RES_STATUS        Status;
} PCI_RES_NODE;

#define PCI_ROOT_BRIDGE_SIGNATURE  EFI_SIGNATURE_32('e', '2', 'p', 'b')

typedef struct {
  UINT32                 Signature;
  EFI_LIST_ENTRY         Link;
  EFI_HANDLE             Handle;
  UINT64                 RootBridgeAttrib;
  UINT64                 Attributes;
  UINT64                 Supports;
  
  //
  // Specific for ea815: Bus, I/O, Mem
  //
  PCI_RES_NODE           ResAllocNode[6];
  
  //
  // Addressing for Memory and I/O and Bus arrange
  //
  UINT64                 BusBase;
  UINT64                 MemBase;     
  UINT64                 IoBase; 
  UINT64                 BusLimit;     
  UINT64                 MemLimit;    
  UINT64                 IoLimit;     

  EFI_LOCK               PciLock;
  UINTN                  PciAddress;
  UINTN                  PciData;
  
  EFI_DEVICE_PATH_PROTOCOL                *DevicePath;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL         Io;

} PCI_ROOT_BRIDGE_INSTANCE;


//
// Driver Instance Data Macros
//

#define DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(a) \
  CR(a, PCI_ROOT_BRIDGE_INSTANCE, Io, PCI_ROOT_BRIDGE_SIGNATURE)


#define DRIVER_INSTANCE_FROM_LIST_ENTRY(a) \
  CR(a, PCI_ROOT_BRIDGE_INSTANCE, Link, PCI_ROOT_BRIDGE_SIGNATURE)


EFI_STATUS
RootBridgeConstructor (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL    *Protocol,
  IN EFI_HANDLE                         HostBridgeHandle,
  IN UINT64                             Attri,
  IN PCI_ROOT_BRIDGE_RESOURCE_APPETURE  ResAppeture
  );

//
// Help function
//
UINT64
Power2MaxMemory (
  IN UINT64                     MemoryLength
  );

#endif
