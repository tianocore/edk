/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciEnumeratorSupport.h
  
Abstract:

  PCI Bus Driver

Revision History

--*/

#ifndef _EFI_PCI_ENUMERATOR_SUPPORT_H
#define _EFI_PCI_ENUMERATOR_SUPPORT_H



EFI_STATUS 
PciDevicePresent(
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo,
  PCI_TYPE00                          *Pci,
  UINT8                               Bus,
  UINT8                               Device,
  UINT8                               Func
);

EFI_STATUS
PciPciDeviceInfoCollector (
  IN PCI_IO_DEVICE                      *Bridge,
  UINT8                                 StartBusNumber
);

EFI_STATUS
PciSearchDevice (
  IN PCI_IO_DEVICE                      *Bridge,
  PCI_TYPE00                            *Pci,
  UINT8                                 Bus,
  UINT8                                 Device,
  UINT8                                 Func,
  PCI_IO_DEVICE                         **PciDevice
);

PCI_IO_DEVICE*
GatherDeviceInfo (
  IN PCI_IO_DEVICE                    *Bridge,
  IN PCI_TYPE00                       *Pci,
  UINT8                               Bus,
  UINT8                               Device,
  UINT8                               Func
);

PCI_IO_DEVICE*
GatherPpbInfo (
  IN PCI_IO_DEVICE                    *Bridge,
  IN PCI_TYPE00                       *Pci,
  UINT8                               Bus,
  UINT8                               Device,
  UINT8                               Func
);

PCI_IO_DEVICE*
GatherP2CInfo (
  IN PCI_IO_DEVICE                    *Bridge,
  IN PCI_TYPE00                       *Pci,
  UINT8                               Bus,
  UINT8                               Device,
  UINT8                               Func
);

EFI_DEVICE_PATH_PROTOCOL *
CreatePciDevicePath(
  IN  EFI_DEVICE_PATH_PROTOCOL *ParentDevicePath,
  IN  PCI_IO_DEVICE            *PciIoDevice
);

EFI_STATUS 
BarExisted (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINTN         Offset,
  OUT UINT32       *BarLengthValue,
  OUT UINT32       *OriginalBarValue
);

EFI_STATUS 
PciTestSupportedAttribute (
  IN PCI_IO_DEVICE                      *PciIoDevice,
  IN UINT16                             *Command,
  IN UINT16                             *BridgeControl,
  IN UINT16                             *OldCommand,
  IN UINT16                             *OldBridgeControl
);

EFI_STATUS 
PciSetDeviceAttribute (
  IN PCI_IO_DEVICE                      *PciIoDevice,
  IN UINT16                             Command,
  IN UINT16                             BridgeControl,
  IN UINTN                              Option
);

EFI_STATUS 
GetFastBackToBackSupport (
  IN PCI_IO_DEVICE                      *PciIoDevice,
  IN UINT8                              StatusIndex
);

EFI_STATUS 
DetermineDeviceAttribute (
  IN PCI_IO_DEVICE                      *PciIoDevice
);

EFI_STATUS 
UpdatePciInfo (
  IN PCI_IO_DEVICE  *PciIoDevice
);

VOID  
SetNewAlign(
  IN UINT64 *Alignment,
  IN UINT64 NewAlignment
);

UINTN
PciParseBar (
  IN PCI_IO_DEVICE  *PciIoDevice,
  IN UINTN          Offset,
  IN UINTN          BarIndex
);

EFI_STATUS 
InitializePciDevice (
  IN PCI_IO_DEVICE *PciIoDevice 
);

EFI_STATUS 
InitializePpb (
  IN PCI_IO_DEVICE *PciIoDevice 
);

EFI_STATUS 
InitializeP2C (
  IN PCI_IO_DEVICE *PciIoDevice 
); 

PCI_IO_DEVICE* 
CreatePciIoDevice (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo,
  IN PCI_TYPE00                       *Pci,
  UINT8                               Bus,
  UINT8                               Device,
  UINT8                               Func
);

EFI_STATUS
PciEnumeratorLight (
  IN EFI_HANDLE                    Controller
);

EFI_STATUS
PciGetBusRange (
  IN     EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  **Descriptors,
  OUT    UINT16                             *MinBus, 
  OUT    UINT16                             *MaxBus,
  OUT    UINT16                             *BusRange
);

EFI_STATUS
StartManagingRootBridge (
  IN PCI_IO_DEVICE *RootBridgeDev
);

BOOLEAN
IsPciDeviceRejected (
  IN PCI_IO_DEVICE *PciIoDevice
);


#endif

