/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    Acpi.c
    
Abstract:

  GUIDs used for ACPI entries in the EFI 1.0 system table

--*/
#include "EfiSpec.h"
#include EFI_GUID_DEFINITION(Acpi)


EFI_GUID gEfiAcpiTableGuid     = EFI_ACPI_TABLE_GUID;

EFI_GUID_STRING(&gEfiAcpiTableGuid, "ACPI Table", "ACPI 1.0 Table GUID in EFI System Table");

EFI_GUID gEfiAcpi20TableGuid   = EFI_ACPI_20_TABLE_GUID;

EFI_GUID_STRING(&gEfiAcpi20TableGuid, "ACPI 2.0 Table", "ACPI 2.0 Table GUID in EFI System Table");
