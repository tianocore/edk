/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    PciCommand.h
  
Abstract:

  PCI Bus Driver

Revision History

--*/


#ifndef _EFI_PCI_COMMAND_H
#define _EFI_PCI_COMMAND_H

#include "pci22.h"

#define EFI_GET_REGISTER      1
#define EFI_SET_REGISTER      2  
#define EFI_ENABLE_REGISTER   3
#define EFI_DISABLE_REGISTER  4

EFI_STATUS 
PciOperateRegister (
  IN  PCI_IO_DEVICE *PciIoDevice,
  IN  UINT16        Command,  
  IN  UINT8         Offset,
  IN  UINT8         Operation,
  OUT UINT16       *PtrCommand
);

BOOLEAN
PciCapabilitySupport (
  IN PCI_IO_DEVICE  *PciIoDevice
);

EFI_STATUS
LocateCapabilityRegBlock (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT8          CapId,
  OUT UINT8        *Offset,
  OUT UINT8        *NextRegBlock
);


#define PciReadCommandRegister(a,b) \
        PciOperateRegister (a,0, PCI_COMMAND_OFFSET, EFI_GET_REGISTER, b)

#define PciSetCommandRegister(a,b) \
        PciOperateRegister (a,b, PCI_COMMAND_OFFSET, EFI_SET_REGISTER, NULL)
        
#define PciEnableCommandRegister(a,b) \
        PciOperateRegister (a,b, PCI_COMMAND_OFFSET, EFI_ENABLE_REGISTER, NULL)
        
#define PciDisableCommandRegister(a,b) \
        PciOperateRegister (a,b, PCI_COMMAND_OFFSET, EFI_DISABLE_REGISTER, NULL)

#define PciReadBridgeControlRegister(a,b) \
        PciOperateRegister (a,0, PCI_BRIDGE_CONTROL_REGISTER_OFFSET, EFI_GET_REGISTER, b)
        
#define PciSetBridgeControlRegister(a,b) \
        PciOperateRegister (a,b, PCI_BRIDGE_CONTROL_REGISTER_OFFSET, EFI_SET_REGISTER, NULL)

#define PciEnableBridgeControlRegister(a,b) \
        PciOperateRegister (a,b, PCI_BRIDGE_CONTROL_REGISTER_OFFSET, EFI_ENABLE_REGISTER, NULL)
        
#define PciDisableBridgeControlRegister(a,b) \
        PciOperateRegister (a,b, PCI_BRIDGE_CONTROL_REGISTER_OFFSET, EFI_DISABLE_REGISTER, NULL)

#endif

