/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  PciCmd.c
  
Abstract:

  PCI Bus Driver

Revision History

--*/


#ifndef _EFI_PCI_COMMAND_H
#define _EFI_PCI_COMMAND_H

EFI_STATUS 
PciReadCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  OUT UINT16       *Command
);

  
EFI_STATUS 
PciSetCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
);

EFI_STATUS 
PciEnableCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
);

EFI_STATUS 
PciDisableCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
);

EFI_STATUS 
PciDisableBridgeControlRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
);


EFI_STATUS 
PciEnableBridgeControlRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
);

EFI_STATUS 
PciReadBridgeControlRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  OUT UINT16       *Command
);

#endif

