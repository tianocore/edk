/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  DeviceManager.c

Abstract:

  The platform device manager reference implement

Revision History

--*/

#ifndef _DEVICE_MANAGER_H
#define _DEVICE_MANAGER_H

#include "Tiano.h"
#include "Bds.h"
#include "FrontPage.h"

#include EFI_PROTOCOL_DEFINITION (Hii)
#include EFI_PROTOCOL_DEFINITION (FormBrowser)
#include EFI_PROTOCOL_DEFINITION (FormCallback)

EFI_STATUS
EFIAPI
DeviceManagerCallbackRoutine (
  IN EFI_FORM_CALLBACK_PROTOCOL       *This,
  IN UINT16                           KeyValue,
  IN EFI_IFR_DATA_ARRAY               *DataArray,
  OUT EFI_HII_CALLBACK_PACKET         **Packet
  )
;

EFI_STATUS
InitializeDeviceManager (
  VOID
  )
;

EFI_STATUS
CallDeviceManager (
  VOID
  )
;

#endif
