/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:
 
  PxeBcDriver.h
 
Abstract:
 
--*/

#ifndef __EFI_PXEBC_DRIVER_H__
#define __EFI_PXEBC_DRIVER_H__

EFI_STATUS
PxeBcDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
 
  Routine Description:
    Test to see if this driver supports ControllerHandle. 
 
  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to test
    RemainingDevicePath - Optional parameter use to pick a specific child 
                          device to start.
 
  Returns:
    EFI_SUCCES         
    EFI_ALREADY_STARTED 
    Others             
 
--*/
// GC_NOTO:    Controller - add argument and description to function comment
;

EFI_STATUS
PxeBcDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
 
  Routine Description:
    Start this driver on ControllerHandle.
 
  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to bind driver to
    RemainingDevicePath - Optional parameter use to pick a specific child 
                          device to start.
 
  Returns:
    EFI_SUCCES          
    EFI_ALREADY_STARTED 
    EFI_OUT_OF_RESOURCES
    Others            
 
--*/
// GC_NOTO:    Controller - add argument and description to function comment
;

EFI_STATUS
PxeBcDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  UINTN                       NumberOfChildren,
  IN  EFI_HANDLE                  *ChildHandleBuffer
  )
/*++
 
  Routine Description:
    Stop this driver on ControllerHandle.
 
  Arguments:
    This              - Protocol instance pointer.
    ControllerHandle  - Handle of device to stop driver on 
    NumberOfChildren  - Number of Handles in ChildHandleBuffer. If number of 
                        children is zero stop the entire bus driver.
    ChildHandleBuffer - List of Child Handles to Stop.
 
  Returns:
    EFI_SUCCESS   
    EFI_DEVICE_ERROR
    Others
 
--*/
// GC_NOTO:    Controller - add argument and description to function comment
;

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
extern EFI_COMPONENT_NAME2_PROTOCOL gPxeBcComponentName;
#else
extern EFI_COMPONENT_NAME_PROTOCOL  gPxeBcComponentName;
#endif
extern EFI_DRIVER_BINDING_PROTOCOL  gPxeBcDriverBinding;
#endif

