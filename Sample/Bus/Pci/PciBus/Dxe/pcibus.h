/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciBus.h
  
Abstract:

  PCI Bus Driver

Revision History

--*/

#ifndef _EFI_PCI_BUS_H
#define _EFI_PCI_BUS_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiImage.h"
#include "pci.h"
#include "acpi.h"
#include "linkedlist.h"
#include "EfiCompNameSupport.h"
#include "ComponentName.h"

#include EFI_GUID_DEFINITION (PciHotplugDevice)

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciRootBridgeIo)
#include EFI_PROTOCOL_DEFINITION (Decompress)
#include EFI_PROTOCOL_DEFINITION (LoadedImage)
#include EFI_PROTOCOL_DEFINITION (UgaIo)
#include EFI_PROTOCOL_DEFINITION (PciPlatform)

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (BusSpecificDriverOverride)
#include EFI_PROTOCOL_DEFINITION (PciHotPlugRequest)
#include EFI_PROTOCOL_DEFINITION (IncompatiblePciDeviceSupport)
#include EFI_PROTOCOL_DEFINITION (PciHotPlugInit)

//
// Driver Consumed Protocols and GUIDs
//
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)

//
// Driver Produced Protocol Prototypes
//

#define VGABASE1  0x3B0
#define VGALIMIT1 0x3BB

#define VGABASE2  0x3C0
#define VGALIMIT2 0x3DF

#define ISABASE   0x100
#define ISALIMIT  0x3FF

typedef enum {
  PciBarTypeUnknown = 0,
  PciBarTypeIo16,
  PciBarTypeIo32,
  PciBarTypeMem32,
  PciBarTypePMem32,
  PciBarTypeMem64,
  PciBarTypePMem64,
  PciBarTypeIo,
  PciBarTypeMem,
  PciBarTypeMaxType
} PCI_BAR_TYPE;

typedef struct {
  UINT64        BaseAddress;
  UINT64        Length;
  UINT64        Alignment;
  PCI_BAR_TYPE  BarType;
  BOOLEAN       Prefetchable;
  UINT8         MemType;
  UINT8         Offset;
} PCI_BAR;

#define PPB_BAR_0                             0
#define PPB_BAR_1                             1
#define PPB_IO_RANGE                          2
#define PPB_MEM32_RANGE                       3
#define PPB_PMEM32_RANGE                      4
#define PPB_PMEM64_RANGE                      5
#define PPB_MEM64_RANGE                       0xFF

#define P2C_BAR_0                             0
#define P2C_MEM_1                             1
#define P2C_MEM_2                             2
#define P2C_IO_1                              3
#define P2C_IO_2                              4

#define PCI_IO_DEVICE_SIGNATURE               EFI_SIGNATURE_32 ('p', 'c', 'i', 'o')

#define EFI_BRIDGE_IO32_DECODE_SUPPORTED      0x0001
#define EFI_BRIDGE_PMEM32_DECODE_SUPPORTED    0x0002
#define EFI_BRIDGE_PMEM64_DECODE_SUPPORTED    0x0004
#define EFI_BRIDGE_IO16_DECODE_SUPPORTED      0x0008
#define EFI_BRIDGE_PMEM_MEM_COMBINE_SUPPORTED 0x0010
#define EFI_BRIDGE_MEM64_DECODE_SUPPORTED     0x0020
#define EFI_BRIDGE_MEM32_DECODE_SUPPORTED     0x0040

#define PCI_MAX_HOST_BRIDGE_NUM               0x0010
//
// Define resource status constant
//
#define EFI_RESOURCE_NONEXISTENT  0xFFFFFFFFFFFFFFFF
#define EFI_RESOURCE_LESS         0xFFFFFFFFFFFFFFFE
#define EFI_RESOURCE_SATISFIED    0x0000000000000000

//
// Define option for attribute
//
#define EFI_SET_SUPPORTS    0
#define EFI_SET_ATTRIBUTES  1

typedef struct _PCI_IO_DEVICE {
  UINT32                                    Signature;
  EFI_HANDLE                                Handle;
  EFI_PCI_IO_PROTOCOL                       PciIo;
  EFI_LIST_ENTRY                            Link;

  EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL PciDriverOverride;
  EFI_DEVICE_PATH_PROTOCOL                  *DevicePath;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL           *PciRootBridgeIo;

  //
  // PCI configuration space header type
  //
  PCI_TYPE00                                Pci;

  //
  // Bus number, Device number, Function number
  //
  UINT8                                     BusNumber;
  UINT8                                     DeviceNumber;
  UINT8                                     FunctionNumber;

  //
  // BAR for this PCI Device
  //
  PCI_BAR                                   PciBar[PCI_MAX_BAR];

  //
  // The bridge device this pci device is subject to
  //
  struct _PCI_IO_DEVICE                     *Parent;

  //
  // A linked list for children Pci Device if it is bridge device
  //
  EFI_LIST_ENTRY                            ChildList;

  //
  // TURE if the PCI bus driver creates the handle for this PCI device
  //
  BOOLEAN                                   Registered;

  //
  // TRUE if the PCI bus driver successfully allocates the resource required by
  // this PCI device
  //
  BOOLEAN                                   Allocated;

  //
  // The attribute this PCI device currently set
  //
  UINT64                                    Attributes;

  //
  // The attributes this PCI device actually supports
  //
  UINT64                                    Supports;

  //
  // The resource decode the bridge supports
  //
  UINT32                                    Decodes;

  //
  // The OptionRom Size
  //
  UINT64                                    RomSize;

  //
  // The OptionRom Size
  //
  UINT64                                    RomBase;

  //
  // TRUE if all OpROM (in device or in platform specific position) have been processed
  //
  BOOLEAN                                   AllOpRomProcessed;

  //
  // TRUE if there is any EFI driver in the OptionRom
  //
  BOOLEAN                                   BusOverride;

  //
  //  A list tracking reserved resource on a bridge device
  //
  EFI_LIST_ENTRY                            ReservedResourceList;

  //
  // A list tracking image handle of platform specific overriding driver
  //
  EFI_LIST_ENTRY                            OptionRomDriverList;

  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR         *ResourcePaddingDescriptors;
  EFI_HPC_PADDING_ATTRIBUTES                PaddingAttributes;

  BOOLEAN                                   IsPciExp;

} PCI_IO_DEVICE;


#define PCI_IO_DEVICE_FROM_PCI_IO_THIS(a) \
  CR (a, PCI_IO_DEVICE, PciIo, PCI_IO_DEVICE_SIGNATURE)

#define PCI_IO_DEVICE_FROM_PCI_DRIVER_OVERRIDE_THIS(a) \
  CR (a, PCI_IO_DEVICE, PciDriverOverride, PCI_IO_DEVICE_SIGNATURE)

#define PCI_IO_DEVICE_FROM_LINK(a) \
  CR (a, PCI_IO_DEVICE, Link, PCI_IO_DEVICE_SIGNATURE)

//
// Global Variables
//
extern EFI_INCOMPATIBLE_PCI_DEVICE_SUPPORT_PROTOCOL *gEfiIncompatiblePciDeviceSupport;
extern EFI_DRIVER_BINDING_PROTOCOL                  gPciBusDriverBinding;
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
extern EFI_COMPONENT_NAME2_PROTOCOL                 gPciBusComponentName;
#else
extern EFI_COMPONENT_NAME_PROTOCOL                  gPciBusComponentName;
#endif
extern EFI_LIST_ENTRY                               gPciDevicePool;
extern BOOLEAN                                      gFullEnumeration;
extern UINTN                                        gPciHostBridgeNumber;
extern EFI_HANDLE                                   gPciHostBrigeHandles[PCI_MAX_HOST_BRIDGE_NUM];
extern UINT64                                       gAllOne;
extern UINT64                                       gAllZero;

extern EFI_PCI_PLATFORM_PROTOCOL                    *gPciPlatformProtocol;

#include "PciIo.h"
#include "PciCommand.h"
#include "PciDeviceSupport.h"
#include "PciEnumerator.h"
#include "PciEnumeratorSupport.h"
#include "PciDriverOverride.h"
#include "PciRomTable.h"
#include "PciOptionRomSupport.h"
#include "PciPowerManagement.h"
#include "PciHotPlugSupport.h"
#include "PciLib.h"

#endif
