/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  BdsPlatform.h

Abstract:

  Head file for BDS Platform specific code

--*/

#ifndef _BDS_PLATFORM_H
#define _BDS_PLATFORM_H

#include "Tiano.h"
#include "Bds.h"
#include "BdsLib.h"
#include "GraphicsLib.h"
#include "Pci22.h"

#include EFI_PROTOCOL_DEFINITION (UgaSplash)
#include EFI_PROTOCOL_DEFINITION (WinNtThunk)
#include EFI_PROTOCOL_DEFINITION (WinNtIo)

extern BDS_CONSOLE_CONNECT_ENTRY  gPlatformConsole[];
extern EFI_DEVICE_PATH_PROTOCOL   *gPlatformConnectSequence[];
extern EFI_DEVICE_PATH_PROTOCOL   *gPlatformDriverOption[];

#define gPciRootBridge \
  { \
    ACPI_DEVICE_PATH, ACPI_DP, (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)), (UINT8) \
      ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8), EISA_PNP_ID (0x0A03), 0 \
  }

#define gEndEntire \
  { \
    END_DEVICE_PATH_TYPE,\
    END_ENTIRE_DEVICE_PATH_SUBTYPE,\
    END_DEVICE_PATH_LENGTH,\
    0\
  }

//
// Platform Root Bridge
//
typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ROOT_BRIDGE_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           IsaBridge;
  ACPI_HID_DEVICE_PATH      Keyboard;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_KEYBOARD_DEVICE_PATH;

typedef struct {
  UART_DEVICE_PATH                Uart;
  VENDOR_DEVICE_PATH              TerminalType;
  EFI_DEVICE_PATH_PROTOCOL        End;
} ROOT_BRIDGE_SERIAL_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           IsaBridge;
  ACPI_HID_DEVICE_PATH      IsaSerial;
  UART_DEVICE_PATH          Uart;
  VENDOR_DEVICE_PATH        TerminalType;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ISA_SERIAL_DEVICE_PATH;

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           OnboardVga;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ONBOARD_VGA_DEVICE_PATH;


//
// Platform BDS Functions
//
VOID
PlatformBdsInit (
  IN EFI_BDS_ARCH_PROTOCOL_INSTANCE  *PrivateData
  )
;

VOID
PlatformBdsPolicyBehavior (
  IN EFI_BDS_ARCH_PROTOCOL_INSTANCE  *PrivateData,
  IN EFI_LIST_ENTRY                  *DriverOptionList,
  IN EFI_LIST_ENTRY                  *BootOptionList
  )
;

VOID
PlatformBdsGetDriverOption (
  IN EFI_LIST_ENTRY               *BdsDriverLists
  )
;

EFI_STATUS
BdsMemoryTest (
  EXTENDMEM_COVERAGE_LEVEL Level
  )
;

EFI_STATUS
PlatformBdsShowProgress (
  EFI_UGA_PIXEL TitleForeground,
  EFI_UGA_PIXEL TitleBackground,
  CHAR16        *Title,
  EFI_UGA_PIXEL ProgressColor,
  UINTN         Progress,
  UINTN         PreviousValue
  )
;

VOID
PlatformBdsConnectSequence (
  VOID
  )
;

VOID
PlatformBdsBootFail (
  IN  BDS_COMMON_OPTION  *Option,
  IN  EFI_STATUS         Status,
  IN  CHAR16             *ExitData,
  IN  UINTN              ExitDataSize
  )
;

VOID
PlatformBdsBootSuccess (
  IN  BDS_COMMON_OPTION *Option
  )
;

EFI_STATUS
ProcessCapsules (
  EFI_BOOT_MODE BootMode
  )
;

EFI_STATUS
PlatformBdsConnectConsole (
  IN BDS_CONSOLE_CONNECT_ENTRY   *PlatformConsole
  )
;

EFI_STATUS
PlatformBdsNoConsoleAction (
  VOID
  )
;

#endif // _BDS_PLATFORM_H
