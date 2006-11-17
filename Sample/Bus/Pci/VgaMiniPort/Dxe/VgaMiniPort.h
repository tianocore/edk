/*++

Copyright (c) 2006 Intel Corporation. All rights reserved
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             


Module Name:

    VgaClass.h
    
Abstract: 
    

Revision History
--*/

#ifndef _VGA_MINIPORT_H
#define _VGA_MINIPORT_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "Pci22.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)

// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (VgaMiniPort)

//
// PCI VGA MiniPort Device Structure
//
#define PCI_VGA_MINI_PORT_DEV_SIGNATURE   EFI_SIGNATURE_32('P','V','M','P')

typedef struct {
  UINTN                         Signature;
  EFI_HANDLE                    Handle;
  EFI_VGA_MINI_PORT_PROTOCOL    VgaMiniPort;
  EFI_PCI_IO_PROTOCOL           *PciIo;
} PCI_VGA_MINI_PORT_DEV;

#define PCI_VGA_MINI_PORT_DEV_FROM_THIS(a) CR(a, PCI_VGA_MINI_PORT_DEV, VgaMiniPort, PCI_VGA_MINI_PORT_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gPciVgaMiniPortDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gPciVgaMiniPortComponentName;

//
// Driver Binding Protocol functions
//
EFI_STATUS
PciVgaMiniPortDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
PciVgaMiniPortDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
PciVgaMiniPortDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );
  
//
// VGA Mini Port Protocol functions
//
EFI_STATUS 
EFIAPI
PciVgaMiniPortSetMode (
  IN  EFI_VGA_MINI_PORT_PROTOCOL  *This,
  IN  UINTN                       ModeNumber
  );

#endif
