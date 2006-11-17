/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    EhciRouting.h
    
Abstract: 
    

Revision History
--*/

#ifndef _EHCI_H
#define _EHCI_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "pci22.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_CONSUMER (DriverBinding)
#include EFI_PROTOCOL_CONSUMER (PciIo)

#define EFI_D_EHCI  EFI_D_INFO

#define bit(a)      (1 << (a))

//
// ////////////////////////////////////////////////////////////////////////
//
//          Universal Host Controller Registers Definitions
//
//////////////////////////////////////////////////////////////////////////
//
// PCI Configuration Registers for USB
//
#define EHCI_PCI_CMD          0x4
#define EHCI_PCI_CLASSC       0x09
#define EHCI_PCI_MEMORY_BASE  0x10

//
// USB2.0 Memory Offset Register
//
#define EHCI_MEMORY_CAPLENGTH   0x0
#define EHCI_MEMORY_CONFIGFLAG  0x40

//
// USB Base Class Code,Sub-Class Code and Programming Interface.
//
#define PCI_CLASSC_PI_EHCI  0x20

//
// USB Class Code structure
//
#pragma pack(1)

typedef struct {
  UINT8 PI;
  UINT8 SubClassCode;
  UINT8 BaseCode;
} USB_CLASSC;

#pragma pack()

#endif
