/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciHotPlugSupport.h
  
Abstract:

  

Revision History

--*/

#ifndef _EFI_PCI_HOT_PLUG_SUPPORT_H
#define _EFI_PCI_HOT_PLUG_SUPPORT_H


#include EFI_PROTOCOL_DEFINITION(PciHotPlugInit)

#define STALL_1_MILLI_SECOND    1000    // stall 1 ms
#define STALL_1_SECOND          1000000 // stall 1 second


typedef struct {
  EFI_EVENT Event;
  BOOLEAN   Initialized;
  VOID      *Padding;
} ROOT_HPC_DATA;

extern EFI_PCI_HOT_PLUG_INIT_PROTOCOL *gPciHotPlugInit;
extern EFI_HPC_LOCATION               *gPciRootHpcPool;
extern UINTN                          gPciRootHpcCount;
extern ROOT_HPC_DATA                  *gPciRootHpcData;

VOID 
PciHPCInitialized (
  IN EFI_EVENT    Event,
  IN VOID         *Context
);

BOOLEAN
EfiCompareDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath1,
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath2
);

EFI_STATUS
InitializeHotPlugSupport(
    VOID
);

EFI_STATUS
IsPciHotPlugBus (
  PCI_IO_DEVICE                       *PciIoDevice
);
  
BOOLEAN
IsRootPciHotPlugBus (
  IN EFI_DEVICE_PATH_PROTOCOL         *HpbDevicePath,
  OUT UINTN                           *HpIndex
);

BOOLEAN
IsRootPciHotPlugController (
  IN EFI_DEVICE_PATH_PROTOCOL         *HpcDevicePath,
  OUT UINTN                           *HpIndex
);

EFI_STATUS
CreateEventForHpc (
  IN UINTN       HpIndex,
  OUT EFI_EVENT  *Event
);

EFI_STATUS
AllRootHPCInitialized (
   IN  UINTN           TimeoutInMilliSeconds
);

EFI_STATUS
IsSHPC (
  PCI_IO_DEVICE                       *PciIoDevice
);
  
EFI_STATUS
GetResourcePaddingForHpb (
  IN PCI_IO_DEVICE *PciIoDevice
);

#endif