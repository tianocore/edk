/*++

Copyright (c) 2005, Intel Corporation                                                         
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
PciPciDeviceInfoCollector (
  IN PCI_IO_DEVICE                    *Bridge,
  IN UINT8                            StartBusNumber
);


EFI_STATUS 
PciDevicePresent(
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo,
  PCI_TYPE00                          *Pci,
  UINT8                               Bus,
  UINT8                               Device,
  UINT8                               Func
);

EFI_STATUS
PciEnumeratorLight (
  IN EFI_HANDLE                       Controller
);

EFI_STATUS
PciGetBusRange (
  IN     EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *Descriptors,
  OUT    UINT16                             *MinBus, 
  OUT    UINT16                             *MaxBus,
  OUT    UINT16                             *BusRange
);

#endif

