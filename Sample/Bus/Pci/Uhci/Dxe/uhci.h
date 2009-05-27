/*++

Copyright (c) 2004 - 2009, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Uhci.h
    
Abstract: 

  The definition for UHCI driver model and HC protocol routines.

Revision History

--*/

#ifndef _UHCI_H
#define _UHCI_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "pci22.h"

#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (UsbHostController)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (ComponentName2)

EFI_FORWARD_DECLARATION (USB_HC_DEV);

#include "UsbHcMem.h"
#include "UhciQueue.h"
#include "UhciReg.h"
#include "UhciSched.h"
#include "UhciDebug.h"

enum {
  UHC_1_MICROSECOND             = 1,
  UHC_1_MILLISECOND             = 1000 * UHC_1_MICROSECOND,
  UHC_1_SECOND                  = 1000 * UHC_1_MILLISECOND,

  //
  // UHCI register operation timeout, set by experience
  //
  UHC_GENERIC_TIMEOUT           = UHC_1_SECOND,
  
  //
  // Wait for force global resume(FGR) complete, refers to
  // specification[UHCI11-2.1.1]
  // 
  UHC_FORCE_GLOBAL_RESUME_STALL = 20 * UHC_1_MILLISECOND,

  //
  // Wait for roothub port reset and recovery, reset stall
  // is set by experience, and recovery stall refers to 
  // specification[UHCI11-2.1.1]
  //
  UHC_ROOT_PORT_RESET_STALL     = 50 * UHC_1_MILLISECOND,
  UHC_ROOT_PORT_RECOVERY_STALL  = 10 * UHC_1_MILLISECOND,

  //
  // Sync and Async transfer polling interval, set by experience, 
  // and the unit of Async is 100us.
  //
  UHC_SYNC_POLL_INTERVAL        = 50 * UHC_1_MICROSECOND,
  UHC_ASYNC_POLL_INTERVAL       = 50 * 10000UL,
  
  //
  // UHC raises TPL to TPL_NOTIFY to serialize all its operations
  // to protect shared data structures.
  //
  UHCI_TPL                      = EFI_TPL_NOTIFY,

  USB_HC_DEV_SIGNATURE          = EFI_SIGNATURE_32 ('u', 'h', 'c', 'i'),
};

#pragma pack(1)
typedef struct {
  UINT8               PI;
  UINT8               SubClassCode;
  UINT8               BaseCode;
} USB_CLASSC;
#pragma pack()

#define UHC_FROM_USB_HC_PROTO(This)   CR(This, USB_HC_DEV, UsbHc, USB_HC_DEV_SIGNATURE)
#define UHC_FROM_USB2_HC_PROTO(This)  CR(This, USB_HC_DEV, Usb2Hc, USB_HC_DEV_SIGNATURE)

//
// USB_HC_DEV support the UHCI hardware controller. It schedules
// the asynchronous interrupt transfer with the same method as
// EHCI: a reversed tree structure. For synchronous interrupt,
// control and bulk transfer, it uses three static queue head to
// schedule them. SyncIntQh is for interrupt transfer. LsCtrlQh is
// for LOW speed control transfer, and FsCtrlBulkQh is for FULL
// speed control or bulk transfer. This is because FULL speed contrl
// or bulk transfer can reclaim the unused bandwidth. Some USB
// device requires this bandwidth reclamation capability.
//
typedef struct _USB_HC_DEV {
  UINT32                    Signature;
  EFI_USB_HC_PROTOCOL       UsbHc;
  EFI_USB2_HC_PROTOCOL      Usb2Hc;
  EFI_PCI_IO_PROTOCOL       *PciIo;

  //
  // Schedule data structures
  //
  UINT32                    *FrameBase;
  UHCI_QH_SW                *SyncIntQh;
  UHCI_QH_SW                *CtrlQh;
  UHCI_QH_SW                *BulkQh;

  //
  // Structures to maintain asynchronus interrupt transfers.
  // When asynchronous interrutp transfer is unlinked from 
  // the frame list, the hardware may still hold a pointer 
  // to it. To synchronize with hardware, its resoureces are
  // released in two steps using Recycle and RecycleWait.
  // Check the asynchronous interrupt management routines.
  //
  EFI_LIST_ENTRY            AsyncIntList;
  EFI_EVENT                 AsyncIntMonitor;
  UHCI_ASYNC_REQUEST        *Recycle;
  UHCI_ASYNC_REQUEST        *RecycleWait;
  
  //
  // ExitBootServicesEvent is used to stop the EHC DMA operation 
  // after exit boot service.
  //
  EFI_EVENT                 ExitBootServiceEvent;

  UINTN                     RootPorts;
  USBHC_MEM_POOL            *MemPool;
  EFI_UNICODE_STRING_TABLE  *CtrlNameTable;
  VOID                      *FrameMapping;
} USB_HC_DEV;

extern EFI_DRIVER_BINDING_PROTOCOL  gUhciDriverBinding;
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
extern EFI_COMPONENT_NAME2_PROTOCOL gUhciComponentName;
#else
extern EFI_COMPONENT_NAME_PROTOCOL  gUhciComponentName;
#endif

#endif
