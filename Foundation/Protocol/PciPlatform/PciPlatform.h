/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  PciPlatform.h

Abstract:
  This file declares PlatfromOpRom protocols.

--*/

#ifndef _PCI_PLATFORM_H_
#define _PCI_PLATFORM_H_


#include "Tiano.h"
#include "TianoTypes.h"

#include EFI_PROTOCOL_DEFINITION (PciHostBridgeResourceAllocation)
#include EFI_PROTOCOL_DEFINITION (PciRootBridgeIo)

//
// Protocol for GUID.
//

#define EFI_PCI_PLATFORM_PROTOCOL_GUID \
{ 0x7d75280, 0x27d4, 0x4d69, 0x90, 0xd0, 0x56, 0x43, 0xe2, 0x38, 0xb3, 0x41}


EFI_FORWARD_DECLARATION (EFI_PCI_PLATFORM_PROTOCOL);

typedef    UINT32   EFI_PCI_PLATFORM_POLICY;

 
#define     EFI_RESERVE_NONE_IO_ALIAS        0x0000
#define     EFI_RESERVE_ISA_IO_ALIAS         0x0001
#define     EFI_RESERVE_ISA_IO_NO_ALIAS      0x0002
#define    EFI_RESERVE_VGA_IO_ALIAS         0x0004
#define    EFI_RESERVE_VGA_IO_NO_ALIAS      0x0008


typedef enum {
  ChipsetEntry,
  ChipsetExit,
  MaximumChipsetPhase
} EFI_PCI_CHIPSET_EXECUTION_PHASE;


typedef
EFI_STATUS
(EFIAPI * EFI_PCI_PLATFORM_PHASE_NOTIFY) (
  IN EFI_PCI_PLATFORM_PROTOCOL              *This,
  IN  EFI_HANDLE                                     HostBridge,
  IN  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE  Phase,
  IN  EFI_PCI_CHIPSET_EXECUTION_PHASE                 ChipsetPhase  
);



typedef
EFI_STATUS
(EFIAPI * EFI_PCI_PLATFORM_PREPROCESS_CONTROLLER) (
  IN EFI_PCI_PLATFORM_PROTOCOL              *This,
  IN  EFI_HANDLE                                     HostBridge,
  IN  EFI_HANDLE                                      RootBridge,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS    PciAddress,
  IN  EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE   Phase,
  IN  EFI_PCI_CHIPSET_EXECUTION_PHASE                  ChipsetPhase
);

typedef
EFI_STATUS
(EFIAPI * EFI_PCI_PLATFORM_GET_PLATFORM_POLICY) (
  IN EFI_PCI_PLATFORM_PROTOCOL            *This,
  OUT EFI_PCI_PLATFORM_POLICY                       *PciPolicy
);


typedef
EFI_STATUS
(EFIAPI *EFI_PCI_PLATFORM_GET_PCI_ROM) (        
  IN EFI_PCI_PLATFORM_PROTOCOL    *This,
  IN EFI_HANDLE                           PciHandle,
  OUT  VOID                                  **RomImage,
  OUT  UINTN                              *RomSize              
  );

typedef struct _EFI_PCI_PLATFORM_PROTOCOL {
  EFI_PCI_PLATFORM_PHASE_NOTIFY          PhaseNotify;
  EFI_PCI_PLATFORM_PREPROCESS_CONTROLLER PlatformPrepController;
  EFI_PCI_PLATFORM_GET_PLATFORM_POLICY   GetPlatformPolicy;
  EFI_PCI_PLATFORM_GET_PCI_ROM           GetPciRom;
} EFI_PCI_PLATFORM_PROTOCOL;

extern EFI_GUID   gEfiPciPlatformProtocolGuid;



#endif
