/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  BiosKeyboard.h
    
Abstract: 

Revision History
--*/

#ifndef _BIOS_KEYBOARD_H
#define _BIOS_KEYBOARD_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "Pci22.h"
#include "EfiCompNameSupport.h"
#include "ComponentName.h"
//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (IsaIo)
#include EFI_PROTOCOL_DEFINITION (LegacyBiosThunk)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)

//
// BISO Keyboard Defines
//
#define CHAR_SCANCODE                   0xe0
#define CHAR_ESC                        0x1b

#define KEYBOARD_8042_DATA_REGISTER     0x60
#define KEYBOARD_8042_STATUS_REGISTER   0x64
#define KEYBOARD_8042_COMMAND_REGISTER  0x64

#define KEYBOARD_TIMEOUT                65536   // 0.07s
#define KEYBOARD_WAITFORVALUE_TIMEOUT   1000000 // 1s
#define KEYBOARD_BAT_TIMEOUT            4000000 // 4s
//  KEYBOARD COMMAND BYTE -- read by writing command KBC_CMDREG_VIA64_CMDBYTE_R to 64H, then read from 60H
//                           write by wrting command KBC_CMDREG_VIA64_CMDBYTE_W to 64H, then write to  60H
//  7: Reserved
//  6: PC/XT translation mode convert
//  5: Disable Auxiliary device interface
//  4: Disable keyboard interface
//  3: Reserved
//  2: System Flag: selftest successful
//  1: Enable Auxiliary device interrupt
//  0: Enable Keyboard interrupt )
//
#define KB_CMMBYTE_KSCAN2UNI_COV  (0x1 << 6)
#define KB_CMMBYTE_DISABLE_AUX    (0x1 << 5)
#define KB_CMMBYTE_DISABLE_KB     (0x1 << 4)
#define KB_CMMBYTE_SLFTEST_SUCC   (0x1 << 2)
#define KB_CMMBYTE_ENABLE_AUXINT  (0x1 << 1)
#define KB_CMMBYTE_ENABLE_KBINT   (0x1 << 0)

//
//  KEYBOARD CONTROLLER STATUS REGISTER - read from 64h
//  7: Parity error
//  6: General time out
//  5: Output buffer holds data for AUX
//  4: Keyboard is not locked
//  3: Command written via 64h  / Data written via 60h
//  2: KBC self-test successful / Power-on reset
//  1: Input buffer holds CPU data / empty
//  0: Output buffer holds keyboard data / empty
//
#define KBC_STSREG_VIA64_PARE (0x1 << 7)
#define KBC_STSREG_VIA64_TIM  (0x1 << 6)
#define KBC_STSREG_VIA64_AUXB (0x1 << 5)
#define KBC_STSREG_VIA64_KEYL (0x1 << 4)
#define KBC_STSREG_VIA64_C_D  (0x1 << 3)
#define KBC_STSREG_VIA64_SYSF (0x1 << 2)
#define KBC_STSREG_VIA64_INPB (0x1 << 1)
#define KBC_STSREG_VIA64_OUTB (0x1 << 0)

//
//  COMMANDs of KEYBOARD CONTROLLER COMMAND REGISTER - write to 64h
//
#define KBC_CMDREG_VIA64_CMDBYTE_R    0x20
#define KBC_CMDREG_VIA64_CMDBYTE_W    0x60
#define KBC_CMDREG_VIA64_AUX_DISABLE  0xA7
#define KBC_CMDREG_VIA64_AUX_ENABLE   0xA8
#define KBC_CMDREG_VIA64_KBC_SLFTEST  0xAA
#define KBC_CMDREG_VIA64_KB_CKECK     0xAB
#define KBC_CMDREG_VIA64_KB_DISABLE   0xAD
#define KBC_CMDREG_VIA64_KB_ENABLE    0xAE
#define KBC_CMDREG_VIA64_INTP_LOW_R   0xC0
#define KBC_CMDREG_VIA64_INTP_HIGH_R  0xC2
#define KBC_CMDREG_VIA64_OUTP_R       0xD0
#define KBC_CMDREG_VIA64_OUTP_W       0xD1
#define KBC_CMDREG_VIA64_OUTB_KB_W    0xD2
#define KBC_CMDREG_VIA64_OUTB_AUX_W   0xD3
#define KBC_CMDREG_VIA64_AUX_W        0xD4

//
//  echos of KEYBOARD CONTROLLER COMMAND - read from 60h
//
#define KBC_CMDECHO_KBCSLFTEST_OK 0x55
#define KBC_CMDECHO_KBCHECK_OK    0x00
#define KBC_CMDECHO_ACK           0xFA
#define KBC_CMDECHO_BATTEST_OK    0xAA
#define KBC_CMDECHO_BATTEST_FAILE 0xFC

//
// OUTPUT PORT COMMANDs - write port by writing KBC_CMDREG_VIA64_OUTP_W via 64H, then write the command to 60H
// drive data and clock of KB to high for at least 500us for BAT needs
//
#define KBC_OUTPORT_DCHIGH_BAT  0xC0
//
// scan code set type
//
#define KBC_INPBUF_VIA60_SCODESET1  0x01
#define KBC_INPBUF_VIA60_SCODESET2  0x02
#define KBC_INPBUF_VIA60_SCODESET3  0x03

//
//  COMMANDs written to INPUT BUFFER - write to 60h
//
#define KBC_INPBUF_VIA60_KBECHO   0xEE
#define KBC_INPBUF_VIA60_KBSCODE  0xF0
#define KBC_INPBUF_VIA60_KBTYPE   0xF2
#define KBC_INPBUF_VIA60_KBDELAY  0xF3
#define KBC_INPBUF_VIA60_KBEN     0xF4
#define KBC_INPBUF_VIA60_KBSTDDIS 0xF5
#define KBC_INPBUF_VIA60_KBSTDEN  0xF6
#define KBC_INPBUF_VIA60_KBRESEND 0xFE
#define KBC_INPBUF_VIA60_KBRESET  0xFF

//
// BIOS Keyboard Device Structure
//
#define BIOS_KEYBOARD_DEV_SIGNATURE EFI_SIGNATURE_32 ('B', 'K', 'B', 'D')

typedef struct {
  UINTN                         Signature;
  EFI_HANDLE                    Handle;
  LEGACY_BIOS_THUNK_PROTOCOL    *LegacyBiosThunk;
  EFI_ISA_IO_PROTOCOL           *IsaIo;
  EFI_SIMPLE_TEXT_IN_PROTOCOL   SimpleTextIn;
  UINT16                        DataRegisterAddress;
  UINT16                        StatusRegisterAddress;
  UINT16                        CommandRegisterAddress;
  BOOLEAN                       ExtendedKeyboard;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
} BIOS_KEYBOARD_DEV;

#define BIOS_KEYBOARD_DEV_FROM_THIS(a)  CR (a, BIOS_KEYBOARD_DEV, SimpleTextIn, BIOS_KEYBOARD_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gBiosKeyboardDriverBinding;

//
// Driver Binding Protocol functions
//
EFI_STATUS
BiosKeyboardDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                - TODO: add argument description
  Controller          - TODO: add argument description
  RemainingDevicePath - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
BiosKeyboardDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                - TODO: add argument description
  Controller          - TODO: add argument description
  RemainingDevicePath - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
BiosKeyboardDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This              - TODO: add argument description
  Controller        - TODO: add argument description
  NumberOfChildren  - TODO: add argument description
  ChildHandleBuffer - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

//
// Simple Text Input Protocol functions
//
EFI_STATUS
EFIAPI
BiosKeyboardReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  IN  BOOLEAN                      ExtendedVerification
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                  - TODO: add argument description
  ExtendedVerification  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
BiosKeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                *Key
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description
  Key   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
KeyboardWaitForValue (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Value,
  IN UINTN              WaitForValueTimeOut
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  BiosKeyboardPrivate - TODO: add argument description
  Value               - TODO: add argument description
  WaitForValueTimeOut - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
KeyboardWrite (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  BiosKeyboardPrivate - TODO: add argument description
  Data                - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
