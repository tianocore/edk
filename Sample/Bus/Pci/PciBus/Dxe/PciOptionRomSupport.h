/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciOptionRomSupport.h
  
Abstract:

  PCI Bus Driver

Revision History

--*/


#ifndef _EFI_PCI_OP_ROM_SUPPORT_H
#define _EFI_PCI_OP_ROM_SUPPORT_H

EFI_STATUS
GetOpRomInfo(
  IN PCI_IO_DEVICE    *PciIoDevice
);

EFI_STATUS
LoadOpRomImage(
  IN PCI_IO_DEVICE   *PciDevice,
  IN UINT64          RomBase  
);

EFI_STATUS
RomDecode (
  IN PCI_IO_DEVICE   *PciDevice,
  IN UINT8           RomBarIndex,
  IN UINT32          RomBar,
  IN BOOLEAN         Enable
);

EFI_STATUS
ProcessOpRomImage (
  PCI_IO_DEVICE   *PciDevice
);

#endif