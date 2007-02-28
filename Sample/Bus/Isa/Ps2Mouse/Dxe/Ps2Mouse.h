/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  Ps2Mouse.h

Abstract:

  PS/2 Mouse driver header file

Revision History

--*/

#ifndef _PS2MOUSE_H
#define _PS2MOUSE_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiStatusCode.h"

//
// Driver consumed protocol prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (IsaIo)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)

//
// Driver produced protocol prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (SimplePointer)

//
// PS/2 mouse sample rate
//
typedef enum {
  SSR_10,
  SSR_20,
  SSR_40,
  SSR_60,
  SSR_80,
  SSR_100,
  SSR_200,
  MAX_SR
} MOUSE_SR;

//
// PS/2 mouse resolution
//
typedef enum {
  CMR1,
  CMR2,
  CMR4,
  CMR8,
  MAX_CMR
} MOUSE_RE;

//
// PS/2 mouse scaling
//
typedef enum {
  SF1,
  SF2
} MOUSE_SF;

//
// Driver Private Data
//
#define PS2_MOUSE_DEV_SIGNATURE EFI_SIGNATURE_32 ('p', 's', '2', 'm')

typedef struct {
  UINTN                               Signature;

  EFI_HANDLE                          Handle;
  EFI_SIMPLE_POINTER_PROTOCOL         SimplePointerProtocol;
  EFI_SIMPLE_POINTER_STATE            State;
  EFI_SIMPLE_POINTER_MODE             Mode;
  BOOLEAN                             StateChanged;

  //
  // PS2 Mouse device specific information
  //
  MOUSE_SR                            SampleRate;
  MOUSE_RE                            Resolution;
  MOUSE_SF                            Scaling;
  UINT8                               DataPackageSize;

  EFI_INTERFACE_DEFINITION_FOR_ISA_IO *IsaIo;

  EFI_EVENT                           TimerEvent;
  
  EFI_UNICODE_STRING_TABLE            *ControllerNameTable;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
} PS2_MOUSE_DEV;

#define PS2_MOUSE_DEV_FROM_THIS(a)  CR (a, PS2_MOUSE_DEV, SimplePointerProtocol, PS2_MOUSE_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gPS2MouseDriver;
extern EFI_COMPONENT_NAME_PROTOCOL  gPs2MouseComponentName;

//
// Driver entry point
//
EFI_STATUS
EFIAPI
PS2MouseDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ImageHandle - GC_TODO: add argument description
  SystemTable - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;
//
// Function prototypes
//
EFI_STATUS
EFIAPI
PS2MouseDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
PS2MouseDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
PS2MouseDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN EFI_HANDLE                    Controller,
  IN UINTN                         NumberOfChildren,
  IN EFI_HANDLE                    *ChildHandleBuffer
  );

EFI_STATUS
EFIAPI
MouseReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN BOOLEAN                        ExtendedVerification
  );

EFI_STATUS
EFIAPI
MouseGetState (
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN OUT EFI_SIMPLE_POINTER_STATE   *State
  );

VOID
EFIAPI
MouseWaitForInput (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );

VOID
EFIAPI
PollMouse (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  );

EFI_STATUS
In8042Data (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *IsaIo,
  IN OUT UINT8                            *Data
  );

BOOLEAN
CheckMouseConnect (
  IN  PS2_MOUSE_DEV     *MouseDev
  );

#endif
