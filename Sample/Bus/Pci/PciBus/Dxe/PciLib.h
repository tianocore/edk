/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciLib.h
  
Abstract:

  PCI Bus Driver Lib header file
  It abstracts some functions that can be different 
  between light PCI bus driver and full PCI bus driver

Revision History

--*/

#ifndef _EFI_PCI_LIB_H
#define _EFI_PCI_LIB_H

VOID InstallHotPlugRequestProtocol(
  IN  EFI_STATUS                    *Status
);

VOID InstallPciHotplugGuid(
  IN  PCI_IO_DEVICE                  *PciIoDevice
);


VOID UninstallPciHotplugGuid(
  IN  PCI_IO_DEVICE                  *PciIoDevice
);

VOID GetBackPcCardBar(
  IN  PCI_IO_DEVICE                  *PciIoDevice
);

EFI_STATUS
RemoveRejectedPciDevices (
  EFI_HANDLE        RootBridgeHandle,
  IN PCI_IO_DEVICE  *Bridge
);

EFI_STATUS 
PciHostBridgeResourceAllocator (  
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *PciResAlloc 
);

EFI_STATUS
PciScanBus (
  IN PCI_IO_DEVICE                      *Bridge,
  IN UINT8                              StartBusNumber,
  OUT UINT8                             *SubBusNumber,
  OUT UINT8                             *PaddedBusRange
);

EFI_STATUS 
PciRootBridgeP2CProcess (
  IN PCI_IO_DEVICE *Bridge 
);

EFI_STATUS 
PciHostBridgeP2CProcess (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *PciResAlloc   
);

EFI_STATUS 
PciHostBridgeEnumerator (  
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  *PciResAlloc 
);

#endif
