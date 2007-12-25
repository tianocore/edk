/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

    IsaAcpi.h

Abstract:

    Definition of intefaces between ISA Bus and ISA Controller module.
    The Controller module might be Lpc driver or else which allocates 
    ACPI resource for ISA Bus.

Revision History

--*/

#ifndef _ISA_ACPI_H
#define _ISA_ACPI_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "pci22.h"
#include "EfiScriptLib.h"


//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (IsaAcpi)

//
// Prototypes for the ISA ACPI protocol interface
//
VOID
EFIAPI
IsaControllerInit (
  IN  EFI_PCI_IO_PROTOCOL   *This
  )
/*++

Routine Description:

  Initialization Interface provided by Isa Controller Module

Arguments:

  This - Pci Io Protocol Instance  
  
Returns:
  
  None

--*/  
;

EFI_STATUS
IsaDeviceEnumerate (
  OUT    EFI_ISA_ACPI_DEVICE_ID      **Device
  )
/*++

Routine Description:

  Enumerate the ISA devices on the ISA bus

Arguments:

  Device        - The existing Isa device on Isa bus.

Returns:
  
  EFI_NOT_FOUND - Couldn't find any Acpi device on Isa bus.
  EFI_SUCCESS   - Find Isa Acpi device on the Isa bus.

--*/

;


EFI_STATUS
IsaGetAcpiResource (
  IN     EFI_ISA_ACPI_DEVICE_ID      *Device,
  OUT    EFI_ISA_ACPI_RESOURCE_LIST  **ResourceList
  )
/*++

Routine Description:

  Get Acpi Resource of the specific ISA device
  It is hardcode now and future will get from ACPI table

Arguments:

  Device        - Acpi Device ID.
  ResourceList  - Acpi Resource List on the Isa bus.
  
Returns:

  EFI_NOT_FOUND - Couldn't find any Acpi device on Isa bus.
  EFI_SUCCESS   - Find Isa Acpi device on the Isa bus.

--*/
;

EFI_STATUS
LpcInterfaceInit (
  IN    EFI_PCI_IO_PROTOCOL        *This
  )
/*++

Routine Description:

  Lpc Interface Init
  
Arguments:

  This - Pci Io Protocol Instance  
  
Returns:
  
  EFI_STATUS

--*/
;

EFI_STATUS  
CheckAcpiNodeStatus (
  IN  EFI_DEVICE_PATH_PROTOCOL  *IsaBridgeDevicePath
  )
/*++

Routine Description:

  Check Acpi Node to see if supported by Lpc Driver

Arguments:

  IsaBridgeDevicePath  - Bridge Device Path

Returns:

  EFI_SUCCESS          - The Acpi Node is supported
  EFI_UNSUPPORTED      - The Acpi Node is not supported

--*/  
;

#endif

