/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DeviceIo.h
  
Abstract:
  Private Data definition for Device IO driver

--*/

#ifndef _DEVICE_IO_H
#define _DEVICE_IO_H

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (PciRootBridgeIo)
#include EFI_PROTOCOL_DEFINITION (DevicePath)

#define MAX_COMMON_BUFFER 0xffffffff

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DeviceIo)

#define DEVICE_IO_PRIVATE_DATA_SIGNATURE  EFI_SIGNATURE_32 ('d', 'e', 'v', 'I')

typedef struct {
  UINTN                           Signature;
  EFI_DEVICE_IO_PROTOCOL          DeviceIo;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  UINT16                          PrimaryBus;
  UINT16                          SubordinateBus;
} DEVICE_IO_PRIVATE_DATA;

#define DEVICE_IO_PRIVATE_DATA_FROM_THIS(a) CR (a, DEVICE_IO_PRIVATE_DATA, DeviceIo, DEVICE_IO_PRIVATE_DATA_SIGNATURE)

EFI_STATUS
DeviceIoConstructor (
  VOID
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  None

Returns:

  TODO: add return values

--*/
;

#endif
