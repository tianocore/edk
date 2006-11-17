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

#ifndef _VGA_CLASS_H
#define _VGA_CLASS_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "Pci22.h"
#include "EfiStatusCode.h"
#include "EfiCompNameSupport.h"
#include "ComponentName.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (VgaMiniPort)
#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)

//
// VGA specific registers
//
#define CRTC_CURSOR_START         0xA
#define CRTC_CURSOR_END           0xB

#define CRTC_CURSOR_LOCATION_HIGH 0xE
#define CRTC_CURSOR_LOCATION_LOW  0xF

#define EFI_MAX_ATTRIBUTE         0x7f

//
// VGA Class Device Structure
//
#define VGA_CLASS_DEV_SIGNATURE EFI_SIGNATURE_32 ('V', 'G', 'A', 'C')

typedef struct {
  UINTN                         Signature;
  EFI_HANDLE                    Handle;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  SimpleTextOut;
  EFI_SIMPLE_TEXT_OUTPUT_MODE   SimpleTextOutputMode;
  EFI_VGA_MINI_PORT_PROTOCOL    *VgaMiniPort;
  EFI_PCI_IO_PROTOCOL           *PciIo;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
} VGA_CLASS_DEV;

#define VGA_CLASS_DEV_FROM_THIS(a)  CR (a, VGA_CLASS_DEV, SimpleTextOut, VGA_CLASS_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gVgaClassDriverBinding;

//
// Driver Binding Protocol functions
//
EFI_STATUS
EFIAPI
VgaClassDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN EFI_HANDLE                    Controller,
  IN EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                - GC_TODO: add argument description
  Controller          - GC_TODO: add argument description
  RemainingDevicePath - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
VgaClassDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN EFI_HANDLE                    Controller,
  IN EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                - GC_TODO: add argument description
  Controller          - GC_TODO: add argument description
  RemainingDevicePath - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
VgaClassDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  Controller        - GC_TODO: add argument description
  NumberOfChildren  - GC_TODO: add argument description
  ChildHandleBuffer - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

//
// Simple Text Output Protocol functions
//
EFI_STATUS
EFIAPI
VgaClassReset (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN  BOOLEAN                      ExtendedVerification
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                  - GC_TODO: add argument description
  ExtendedVerification  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
VgaClassOutputString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN  CHAR16                       *WString
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This    - GC_TODO: add argument description
  WString - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
VgaClassTestString (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN  CHAR16                       *WString
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This    - GC_TODO: add argument description
  WString - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
VgaClassClearScreen (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL *This
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
VgaClassSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN  UINTN                        Attribute
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This      - GC_TODO: add argument description
  Attribute - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
VgaClassSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN  UINTN                        Column,
  IN  UINTN                        Row
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This    - GC_TODO: add argument description
  Column  - GC_TODO: add argument description
  Row     - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
VgaClassEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN  BOOLEAN                      Visible
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This    - GC_TODO: add argument description
  Visible - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
VgaClassQueryMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN  UINTN                        ModeNumber,
  OUT UINTN                        *Columns,
  OUT UINTN                        *Rows
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  ModeNumber  - GC_TODO: add argument description
  Columns     - GC_TODO: add argument description
  Rows        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
VgaClassSetMode (
  IN  EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN  UINTN                        ModeNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  ModeNumber  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

#endif
