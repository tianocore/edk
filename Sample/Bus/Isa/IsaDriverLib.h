/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  IsaDriverLib.h
  
Abstract:
  
  The header file for ISA Driver Library
  
Revision History:

--*/

#ifndef _EFI_ISA_Driver_H
#define _EFI_ISA_Driver_H

#include "EfiScriptLib.h"
#include "IsaBus.h"
#include "IsaAcpi.h"
#ifdef ISA_FLOPPY_ENABLE
#include "IsaFloppy.h"
#endif
#ifdef ISA_SERIAL_ENABLE
#include "Serial.h"
#endif
#ifdef PS2_KEYBOARD_ENABLE
#include "Ps2Keyboard.h"
#endif
#ifdef PS2_MOUSE_ENABLE
#include "Ps2Mouse.h"
#endif

//
// Isa bus driver entry point
//
#define INSTALL_ISA_BUS_DRIVER_PROTOCOL(ImageHandle,               \
                                        SystemTable,               \
                                        DriverBinding,             \
                                        DriverBindingHandle)       \
        EfiLibInstallDriverBinding ((ImageHandle),                 \
                                    (SystemTable),                 \
                                    (DriverBinding),               \
                                    (DriverBindingHandle))

//
// Isa floppy driver entry point
//   
#ifdef  ISA_FLOPPY_ENABLE
#define INSTALL_ISA_FLOPPY_DRIVER_PROTOCOL(ImageHandle,            \
                                           SystemTable,            \
                                           DriverBinding,          \
                                           DriverBindingHandle)    \
        EfiLibInstallDriverBinding ((ImageHandle),                 \
                                    (SystemTable),                 \
                                    (DriverBinding),               \
                                    (DriverBindingHandle))
#else
#define INSTALL_ISA_FLOPPY_DRIVER_PROTOCOL(ImageHandle,            \
                                           SystemTable,            \
                                           DriverBinding,          \
                                           DriverBindingHandle)                  
#endif      

//
// Isa serial driver entry point
//
#ifdef  ISA_SERIAL_ENABLE      
#define INSTALL_ISA_SERIAL_DRIVER_PROTOCOL(ImageHandle,            \
                                           SystemTable,            \
                                           DriverBinding,          \
                                           DriverBindingHandle)    \
        EfiLibInstallDriverBinding (ImageHandle,                   \
                                    SystemTable,                   \
                                    DriverBinding,                 \
                                    DriverBindingHandle)      
#else
#define INSTALL_ISA_SERIAL_DRIVER_PROTOCOL(ImageHandle,            \
                                           SystemTable,            \
                                           DriverBinding,          \
                                           DriverBindingHandle)
#endif      

//
// Ps2 keyboard driver entry point
//      
#ifdef  PS2_KEYBOARD_ENABLE
#define INSTALL_PS2_KEYBOARD_DRIVER_PROTOCOL(ImageHandle,          \
                                             SystemTable,          \
                                             DriverBinding,        \
                                             DriverBindingHandle)  \
        EfiLibInstallDriverBinding ((ImageHandle),                 \
                                    (SystemTable),                 \
                                    (DriverBinding),               \
                                    (DriverBindingHandle))
#else
#define INSTALL_PS2_KEYBOARD_DRIVER_PROTOCOL(ImageHandle,          \
                                             SystemTable,          \
                                             DriverBinding,        \
                                             DriverBindingHandle)
#endif

// 
// Ps2 mouse driver entry point
//
#ifdef  PS2_MOUSE_ENABLE    
#define INSTALL_PS2_MOUSE_DRIVER_PROTOCOL(ImageHandle,             \
                                          SystemTable,             \
                                          DriverBinding,           \
                                          DriverBindingHandle)     \
        EfiLibInstallDriverBinding ((ImageHandle),                 \
                                    (SystemTable),                 \
                                    (DriverBinding),               \
                                    (DriverBindingHandle))      
#else                                       
#define INSTALL_PS2_MOUSE_DRIVER_PROTOCOL(ImageHandle,             \
                                          SystemTable,             \
                                          DriverBinding,           \
                                          DriverBindingHandle)
#endif

#endif
