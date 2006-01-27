/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  PciDriverOverride.h
  
Abstract:

  

Revision History

--*/

#ifndef _EFI_PCI_DRIVER_OVERRRIDE_H
#define _EFI_PCI_DRIVER_OVERRRIDE_H

#include EFI_PROTOCOL_DEFINITION(BusSpecificDriverOverride)


#define DRIVER_OVERRIDE_SIGNATURE   EFI_SIGNATURE_32 ('d','r','o','v')

typedef struct _PCI_DRIVER_OVERRIDE_LIST {
  UINT32                         Signature;
  EFI_LIST_ENTRY                 Link ;
  EFI_HANDLE                     DriverImageHandle;
} PCI_DRIVER_OVERRIDE_LIST;


#define DRIVER_OVERRIDE_FROM_LINK(a) \
  CR (a, PCI_DRIVER_OVERRIDE_LIST, Link, DRIVER_OVERRIDE_SIGNATURE)


EFI_STATUS
InitializePciDriverOverrideInstance (
  PCI_IO_DEVICE  *PciIoDevice
);

EFI_STATUS
AddDriver(
  IN PCI_IO_DEVICE     *PciIoDevice,
  IN EFI_HANDLE        DriverImageHandle
);


#endif
