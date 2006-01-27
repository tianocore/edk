/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  PciCommand.c
  
Abstract:

  PCI Bus Driver

Revision History

--*/

#include "Pcibus.h"


EFI_STATUS 
PciReadCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  OUT UINT16       *Command
)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{

  EFI_PCI_IO_PROTOCOL   *PciIo;

  *Command = 0;
  PciIo = &PciIoDevice->PciIo;

  return PciIo->Pci.Read (
                PciIo, 
                EfiPciIoWidthUint16, 
                PCI_COMMAND_OFFSET, 
                1, 
                Command
                );
}
  
EFI_STATUS 
PciSetCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINT16                Temp;
  EFI_PCI_IO_PROTOCOL   *PciIo;
  
  Temp = Command;
  PciIo = &PciIoDevice->PciIo;

  return PciIo->Pci.Write (
              PciIo, 
              EfiPciIoWidthUint16, 
              PCI_COMMAND_OFFSET, 
              1, 
              &Temp
              );
  
}


EFI_STATUS 
PciEnableCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINT16  OldCommand;
  EFI_PCI_IO_PROTOCOL   *PciIo;

  OldCommand = 0;
  PciIo = &PciIoDevice->PciIo;

  PciIo->Pci.Read (
          PciIo, 
          EfiPciIoWidthUint16, 
          PCI_COMMAND_OFFSET, 
          1, 
          &OldCommand
          );

  OldCommand |= Command;

  return PciIo->Pci.Write (
              PciIo, 
              EfiPciIoWidthUint16, 
              PCI_COMMAND_OFFSET, 
              1, 
              &OldCommand
              );
  
}


EFI_STATUS 
PciDisableCommandRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINT16  OldCommand;
  EFI_PCI_IO_PROTOCOL   *PciIo;

  OldCommand = 0;
  PciIo = &PciIoDevice->PciIo;

  PciIo->Pci.Read (
          PciIo, 
          EfiPciIoWidthUint16, 
          PCI_COMMAND_OFFSET, 
          1, 
          &OldCommand
          );

  OldCommand &= ~(Command);

  return PciIo->Pci.Write (
            PciIo, 
            EfiPciIoWidthUint16, 
            PCI_COMMAND_OFFSET, 
            1, 
            &OldCommand
           );
  
}



EFI_STATUS 
PciSetBridgeControlRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINT16                Temp;
  EFI_PCI_IO_PROTOCOL   *PciIo;

  Temp = Command;
  PciIo = &PciIoDevice->PciIo;

  return PciIo->Pci.Write (
                PciIo, 
                EfiPciIoWidthUint16, 
                PCI_BRIDGE_CONTROL_REGISTER_OFFSET, 
                1, 
                &Temp
               );
 
}


EFI_STATUS 
PciEnableBridgeControlRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINT16  OldCommand;
  EFI_PCI_IO_PROTOCOL   *PciIo;

  OldCommand = 0;
  PciIo = &PciIoDevice->PciIo;

  PciIo->Pci.Read (
          PciIo, 
          EfiPciIoWidthUint16, 
          PCI_BRIDGE_CONTROL_REGISTER_OFFSET, 
          1, 
          &OldCommand
          );

  OldCommand |= Command;

  return PciIo->Pci.Write (
              PciIo, 
              EfiPciIoWidthUint16, 
              PCI_BRIDGE_CONTROL_REGISTER_OFFSET, 
              1, 
              &OldCommand
             );
  
}

EFI_STATUS 
PciDisableBridgeControlRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  IN UINT16        Command
)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINT16  OldCommand;
  EFI_PCI_IO_PROTOCOL   *PciIo;

  OldCommand = 0;
  PciIo = &PciIoDevice->PciIo;

  PciIo->Pci.Read (
          PciIo, 
          EfiPciIoWidthUint16, 
          PCI_BRIDGE_CONTROL_REGISTER_OFFSET, 
          1, 
          &OldCommand
          );

  OldCommand &= ~(Command);

  return PciIo->Pci.Write (
              PciIo, 
              EfiPciIoWidthUint16, 
              PCI_BRIDGE_CONTROL_REGISTER_OFFSET, 
              1, 
              &OldCommand
              );
 
}



EFI_STATUS 
PciReadBridgeControlRegister (
  IN PCI_IO_DEVICE *PciIoDevice,
  OUT UINT16       *Command
)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{

  EFI_PCI_IO_PROTOCOL   *PciIo;

  *Command = 0;
  PciIo = &PciIoDevice->PciIo;

  return PciIo->Pci.Read (
                PciIo, 
                EfiPciIoWidthUint16, 
                PCI_BRIDGE_CONTROL_REGISTER_OFFSET, 
                1, 
                Command
                );
 
}

   




  


   




  
















  

  
