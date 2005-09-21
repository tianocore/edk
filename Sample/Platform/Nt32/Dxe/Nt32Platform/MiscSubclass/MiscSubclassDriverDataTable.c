/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MiscSubclassDriverDataTable.c
  
Abstract: 

  This driver parses the mMiscSubclassDataTable structure and reports
  any generated data to the DataHub.

--*/

#include "MiscSubclassDriver.h"

//
// External definitions referenced by Data Table entries.
//
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_CHASSIS_MANUFACTURER,
  MiscChassisManufacturer
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_BIOS_VENDOR,
  MiscBiosVendor
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_SYSTEM_MANUFACTURER,
  MiscSystemManufacturer
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_BASE_BOARD_MANUFACTURER,
  MiscBaseBoardManufacturer
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR,
  MiscPortInternalConnectorDesignator
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR,
  MiscPortKeyboard
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR,
  MiscPortMouse
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR,
  MiscPortCom1
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR,
  MiscPortCom2
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_SYSTEM_SLOT_DESIGNATION,
  MiscSystemSlotDesignation
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_ONBOARD_DEVICE,
  MiscOnboardDevice
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_OEM_STRING,
  OemString
  );
MISC_SUBCLASS_TABLE_EXTERNS (
  EFI_MISC_SYSTEM_OPTION_STRING,
  SystemOptionString
  );

//
// Data Table.
//
EFI_MISC_SUBCLASS_DATA_TABLE  mMiscSubclassDataTable[] = {
  MISC_SUBCLASS_TABLE_ENTRY_DATA_AND_FUNCTION(EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR, MiscPortKeyboard, MiscPortInternalConnectorDesignator),
  MISC_SUBCLASS_TABLE_ENTRY_DATA_AND_FUNCTION(EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR, MiscPortMouse, MiscPortInternalConnectorDesignator),
  MISC_SUBCLASS_TABLE_ENTRY_DATA_AND_FUNCTION(EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR, MiscPortCom1, MiscPortInternalConnectorDesignator),
  MISC_SUBCLASS_TABLE_ENTRY_DATA_AND_FUNCTION(EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR, MiscPortCom2, MiscPortInternalConnectorDesignator),
  MISC_SUBCLASS_TABLE_ENTRY_DATA_ONLY(EFI_MISC_BIOS_VENDOR, MiscBiosVendor),
  MISC_SUBCLASS_TABLE_ENTRY_DATA_ONLY(EFI_MISC_SYSTEM_MANUFACTURER, MiscSystemManufacturer),
  MISC_SUBCLASS_TABLE_ENTRY_DATA_ONLY(EFI_MISC_BASE_BOARD_MANUFACTURER, MiscBaseBoardManufacturer),
  MISC_SUBCLASS_TABLE_ENTRY_DATA_ONLY(EFI_MISC_CHASSIS_MANUFACTURER, MiscChassisManufacturer),
  MISC_SUBCLASS_TABLE_ENTRY_DATA_ONLY(EFI_MISC_SYSTEM_SLOT_DESIGNATION, MiscSystemSlotDesignation),
  MISC_SUBCLASS_TABLE_ENTRY_DATA_ONLY(EFI_MISC_OEM_STRING, OemString),
  MISC_SUBCLASS_TABLE_ENTRY_DATA_ONLY(EFI_MISC_SYSTEM_OPTION_STRING, SystemOptionString),
};

//
// Number of Data Table entries.
//
UINTN mMiscSubclassDataTableEntries = (sizeof mMiscSubclassDataTable) / sizeof (EFI_MISC_SUBCLASS_DATA_TABLE);

/* eof - MiscSubclassDriverDataTable.c */
