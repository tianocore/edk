/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    Uhci.h
    
Abstract: 
    

Revision History
--*/

#ifndef _UHCI_H
#define _UHCI_H

/*
 * Universal Host Controller Interface data structures and defines
 */
#include "Tiano.h"
#include "EfiDriverLib.h"
#include "pci22.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (PciIo)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (UsbHostController)
#include EFI_PROTOCOL_DEFINITION (ComponentName)

#define EFI_D_UHCI  EFI_D_INFO

//
// stall time
//
#define STALL_1_MILLI_SECOND      1000
#define STALL_1_SECOND            1000 * STALL_1_MILLI_SECOND

#define FORCE_GLOBAL_RESUME_TIME  20 * STALL_1_MILLI_SECOND

#define ROOT_PORT_REST_TIME       50 * STALL_1_MILLI_SECOND

#define PORT_RESET_RECOVERY_TIME  10 * STALL_1_MILLI_SECOND

//
// 50 ms
//
#define INTERRUPT_POLLING_TIME  50 * 1000 * 10

//
// UHCI IO Space Address Register Register locates at
// offset 20 ~ 23h of PCI Configuration Space (UHCI spec, Revision 1.1),
// so, its BAR Index is 4.
//
#define USB_BAR_INDEX 4

//
// One memory block uses 1 page (common buffer for QH,TD use.)
//
#define NORMAL_MEMORY_BLOCK_UNIT_IN_PAGES 1

#define EFI_PAGES_TO_SIZE(a)   ( (a) << EFI_PAGE_SHIFT)

#define bit(a)                            1 << (a)

//
// ////////////////////////////////////////////////////////////////////////
//
//          Universal Host Controller Registers Definitions
//
//////////////////////////////////////////////////////////////////////////
extern UINT16 USBBaseAddr;

/* Command register */
#define USBCMD          0       /* Command Register Offset 00-01h */
#define USBCMD_RS       bit (0) /* Run/Stop */
#define USBCMD_HCRESET  bit (1) /* Host reset */
#define USBCMD_GRESET   bit (2) /* Global reset */
#define USBCMD_EGSM     bit (3) /* Global Suspend Mode */
#define USBCMD_FGR      bit (4) /* Force Global Resume */
#define USBCMD_SWDBG    bit (5) /* SW Debug mode */
#define USBCMD_CF       bit (6) /* Config Flag (sw only) */
#define USBCMD_MAXP     bit (7) /* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#define USBSTS        2       /* Status Register Offset 02-03h */
#define USBSTS_USBINT bit (0) /* Interrupt due to IOC */
#define USBSTS_ERROR  bit (1) /* Interrupt due to error */
#define USBSTS_RD     bit (2) /* Resume Detect */
#define USBSTS_HSE    bit (3) /* Host System Error*/
#define USBSTS_HCPE   bit (4) /* Host Controller Process Error*/
#define USBSTS_HCH    bit (5) /* HC Halted */

/* Interrupt enable register */
#define USBINTR         4       /* Interrupt Enable Register 04-05h */
#define USBINTR_TIMEOUT bit (0) /* Timeout/CRC error enable */
#define USBINTR_RESUME  bit (1) /* Resume interrupt enable */
#define USBINTR_IOC     bit (2) /* Interrupt On Complete enable */
#define USBINTR_SP      bit (3) /* Short packet interrupt enable */

/* Frame Number Register Offset 06-08h */
#define USBFRNUM  6

/* Frame List Base Address Register Offset 08-0Bh */
#define USBFLBASEADD  8

/* Start of Frame Modify Register Offset 0Ch */
#define USBSOF  0x0c

/* USB port status and control registers */
#define USBPORTSC1      0x10      /*Port 1 offset 10-11h */
#define USBPORTSC2      0x12      /*Port 2 offset 12-13h */

#define USBPORTSC_CCS   bit (0)   /* Current Connect Status*/
#define USBPORTSC_CSC   bit (1)   /* Connect Status Change */
#define USBPORTSC_PED   bit (2)   /* Port Enable / Disable */
#define USBPORTSC_PEDC  bit (3)   /* Port Enable / Disable Change */
#define USBPORTSC_LSL   bit (4)   /* Line Status Low bit*/
#define USBPORTSC_LSH   bit (5)   /* Line Status High bit*/
#define USBPORTSC_RD    bit (6)   /* Resume Detect */
#define USBPORTSC_LSDA  bit (8)   /* Low Speed Device Attached */
#define USBPORTSC_PR    bit (9)   /* Port Reset */
#define USBPORTSC_SUSP  bit (12)  /* Suspend */

/* PCI Configuration Registers for USB */

//
// Class Code Register offset
//
#define CLASSC  0x09
//
// USB IO Space Base Address Register offset
//
#define USBBASE 0x20

//
// USB legacy Support
//
#define USB_EMULATION 0xc0

//
// USB Base Class Code,Sub-Class Code and Programming Interface.
//
#define PCI_CLASSC_PI_UHCI  0x00

#define SETUP_PACKET_ID     0x2D
#define INPUT_PACKET_ID     0x69
#define OUTPUT_PACKET_ID    0xE1
#define ERROR_PACKET_ID     0x55

//
// ////////////////////////////////////////////////////////////////////////
//
//          USB Transfer Mechanism Data Structures
//
//////////////////////////////////////////////////////////////////////////
#pragma pack(1)
//
// USB Class Code structure
//
typedef struct {
  UINT8 PI;
  UINT8 SubClassCode;
  UINT8 BaseCode;
} USB_CLASSC;

typedef struct {
  UINT32  QHHorizontalTerminate : 1;
  UINT32  QHHorizontalQSelect : 1;
  UINT32  QHHorizontalRsvd : 2;
  UINT32  QHHorizontalPtr : 28;
  UINT32  QHVerticalTerminate : 1;
  UINT32  QHVerticalQSelect : 1;
  UINT32  QHVerticalRsvd : 2;
  UINT32  QHVerticalPtr : 28;
} QUEUE_HEAD;

typedef struct {
  UINT32  TDLinkPtrTerminate : 1;
  UINT32  TDLinkPtrQSelect : 1;
  UINT32  TDLinkPtrDepthSelect : 1;
  UINT32  TDLinkPtrRsvd : 1;
  UINT32  TDLinkPtr : 28;
  UINT32  TDStatusActualLength : 11;
  UINT32  TDStatusRsvd : 5;
  UINT32  TDStatus : 8;
  UINT32  TDStatusIOC : 1;
  UINT32  TDStatusIOS : 1;
  UINT32  TDStatusLS : 1;
  UINT32  TDStatusErr : 2;
  UINT32  TDStatusSPD : 1;
  UINT32  TDStatusRsvd2 : 2;
  UINT32  TDTokenPID : 8;
  UINT32  TDTokenDevAddr : 7;
  UINT32  TDTokenEndPt : 4;
  UINT32  TDTokenDataToggle : 1;
  UINT32  TDTokenRsvd : 1;
  UINT32  TDTokenMaxLen : 11;
  UINT32  TDBufferPtr;
} TD;

#pragma pack()

typedef struct {
  QUEUE_HEAD  QH;
  VOID        *ptrNext;
  VOID        *ptrDown;
  VOID        *ptrNextIntQH;  // for interrupt transfer's special use
  VOID        *LoopPtr;
} QH_STRUCT;

typedef struct {
  TD      TDData;
  UINT8   *pTDBuffer;
  VOID    *ptrNextTD;
  VOID    *ptrNextQH;
  UINT16  TDBufferLength;
  UINT16  reserved;
} TD_STRUCT;

//
// ////////////////////////////////////////////////////////////////////////
//
//          Universal Host Controller Device Data Structure
//
//////////////////////////////////////////////////////////////////////////
#define USB_HC_DEV_FROM_THIS(a)   CR (a, USB_HC_DEV, UsbHc, USB_HC_DEV_SIGNATURE)

#define USB_HC_DEV_SIGNATURE      EFI_SIGNATURE_32 ('u', 'h', 'c', 'i')
#define INTERRUPT_LIST_SIGNATURE  EFI_SIGNATURE_32 ('i', 'n', 't', 's')
typedef struct {
  UINTN                           Signature;

  EFI_LIST_ENTRY                  Link;
  UINT8                           DevAddr;
  UINT8                           EndPoint;
  UINT8                           DataToggle;
  UINT8                           Reserved[5];
  TD_STRUCT                       *PtrFirstTD;
  QH_STRUCT                       *PtrQH;
  UINTN                           DataLen;
  UINTN                           PollInterval;
  VOID                            *Mapping;
  UINT8                           *DataBuffer;  // allocated host memory, not mapped memory
  EFI_ASYNC_USB_TRANSFER_CALLBACK InterruptCallBack;
  VOID                            *InterruptContext;
} INTERRUPT_LIST;

#define INTERRUPT_LIST_FROM_LINK(a) CR (a, INTERRUPT_LIST, Link, INTERRUPT_LIST_SIGNATURE)

typedef struct {
  UINT32  FrameListPtrTerminate : 1;
  UINT32  FrameListPtrQSelect : 1;
  UINT32  FrameListRsvd : 2;
  UINT32  FrameListPtr : 28;

} FRAMELIST_ENTRY;

typedef struct _MEMORY_MANAGE_HEADER {
  UINT8                         *BitArrayPtr;
  UINTN                         BitArraySizeInBytes;
  UINT8                         *MemoryBlockPtr;
  UINTN                         MemoryBlockSizeInBytes;
  VOID                          *Mapping;
  struct _MEMORY_MANAGE_HEADER  *Next;
} MEMORY_MANAGE_HEADER;

typedef struct {
  UINTN                     Signature;
  EFI_USB_HC_PROTOCOL       UsbHc;
  EFI_PCI_IO_PROTOCOL       *PciIo;

  //
  // local data
  //
  EFI_LIST_ENTRY            InterruptListHead;
  FRAMELIST_ENTRY           *FrameListEntry;
  VOID                      *FrameListMapping;
  MEMORY_MANAGE_HEADER      *MemoryHeader;
  EFI_EVENT                 InterruptTransTimer;
  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;

} USB_HC_DEV;

extern EFI_DRIVER_BINDING_PROTOCOL  gUhciDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gUhciComponentName;

EFI_STATUS
WriteUHCCommandReg (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  CmdAddrOffset,
  IN UINT16                  UsbCmd
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo         - TODO: add argument description
  CmdAddrOffset - TODO: add argument description
  UsbCmd        - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
ReadUHCCommandReg (
  IN       EFI_PCI_IO_PROTOCOL     *PciIo,
  IN       UINT32                  CmdAddrOffset,
  IN OUT   UINT16                  *Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo         - TODO: add argument description
  CmdAddrOffset - TODO: add argument description
  Data          - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
WriteUHCStatusReg (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  StatusAddrOffset,
  IN UINT16                  UsbSts
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo             - TODO: add argument description
  StatusAddrOffset  - TODO: add argument description
  UsbSts            - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
ReadUHCStatusReg (
  IN       EFI_PCI_IO_PROTOCOL     *PciIo,
  IN       UINT32                  StatusAddrOffset,
  IN OUT   UINT16                  *Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo             - TODO: add argument description
  StatusAddrOffset  - TODO: add argument description
  Data              - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
ClearStatusReg (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  StatusAddrOffset
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo             - TODO: add argument description
  StatusAddrOffset  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
ReadUHCFrameNumberReg (
  IN       EFI_PCI_IO_PROTOCOL     *PciIo,
  IN       UINT32                  FrameNumAddrOffset,
  IN OUT   UINT16                  *Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo               - TODO: add argument description
  FrameNumAddrOffset  - TODO: add argument description
  Data                - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
WriteUHCFrameListBaseReg (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  FlBaseAddrOffset,
  IN UINT32                  UsbFrameListBaseAddr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo                 - TODO: add argument description
  FlBaseAddrOffset      - TODO: add argument description
  UsbFrameListBaseAddr  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
ReadRootPortReg (
  IN       EFI_PCI_IO_PROTOCOL     *PciIo,
  IN       UINT32                  PortAddrOffset,
  IN OUT   UINT16                  *Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo           - TODO: add argument description
  PortAddrOffset  - TODO: add argument description
  Data            - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
WriteRootPortReg (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  PortAddrOffset,
  IN UINT16                  ControlBits
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo           - TODO: add argument description
  PortAddrOffset  - TODO: add argument description
  ControlBits     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
WaitForUHCHalt (
  IN EFI_PCI_IO_PROTOCOL      *PciIo,
  IN UINT32                   StatusRegAddr,
  IN UINTN                    Timeout
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo         - TODO: add argument description
  StatusRegAddr - TODO: add argument description
  Timeout       - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsHostSysErr (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  StatusRegAddr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo         - TODO: add argument description
  StatusRegAddr - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsHCProcessErr (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  StatusRegAddr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo         - TODO: add argument description
  StatusRegAddr - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsHCHalted (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  StatusRegAddr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo         - TODO: add argument description
  StatusRegAddr - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

//
// This routine programs the USB frame number register. We assume that the
// HC schedule execution is stopped.
//
EFI_STATUS
SetFrameNumberReg (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  FRNUMAddr,
  IN UINT16                  Index
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo     - TODO: add argument description
  FRNUMAddr - TODO: add argument description
  Index     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT16
GetCurrentFrameNumber (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  FRNUMAddr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo     - TODO: add argument description
  FRNUMAddr - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
SetFrameListBaseAddress (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  FLBASEADDRReg,
  IN UINT32                  Addr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo         - TODO: add argument description
  FLBASEADDRReg - TODO: add argument description
  Addr          - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT32
GetFrameListBaseAddress (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN UINT32                  FLBAddr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo   - TODO: add argument description
  FLBAddr - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
CreateFrameList (
  IN USB_HC_DEV     *HcDev,
  IN UINT32         FLBASEADDRReg
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev         - TODO: add argument description
  FLBASEADDRReg - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
FreeFrameListEntry (
  IN USB_HC_DEV     *UhcDev
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UhcDev  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
InitFrameList (
  IN USB_HC_DEV     *HcDev
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
AllocateQHStruct (
  IN  USB_HC_DEV     *HcDev,
  OUT QH_STRUCT      **ppQHStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev       - TODO: add argument description
  ppQHStruct  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
InitQH (
  IN QH_STRUCT     *ptrQH
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
CreateQH (
  IN  USB_HC_DEV     *HcDev,
  OUT QH_STRUCT      **pptrQH
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev   - TODO: add argument description
  pptrQH  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetQHHorizontalLinkPtr (
  IN QH_STRUCT     *ptrQH,
  IN VOID          *ptrNext
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH   - TODO: add argument description
  ptrNext - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID                                *
GetQHHorizontalLinkPtr (
  IN QH_STRUCT     *ptrQH
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetQHHorizontalQHorTDSelect (
  IN QH_STRUCT     *ptrQH,
  IN BOOLEAN       bQH
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH - TODO: add argument description
  bQH   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetQHHorizontalValidorInvalid (
  IN QH_STRUCT     *ptrQH,
  IN BOOLEAN       bValid
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH   - TODO: add argument description
  bValid  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetQHVerticalLinkPtr (
  IN QH_STRUCT     *ptrQH,
  IN VOID          *ptrNext
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH   - TODO: add argument description
  ptrNext - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID                                *
GetQHVerticalLinkPtr (
  IN QH_STRUCT     *ptrQH
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetQHVerticalQHorTDSelect (
  IN QH_STRUCT     *ptrQH,
  IN BOOLEAN       bQH
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH - TODO: add argument description
  bQH   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsQHHorizontalQHSelect (
  IN QH_STRUCT     *ptrQH
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetQHVerticalValidorInvalid (
  IN QH_STRUCT     *ptrQH,
  IN BOOLEAN       bValid
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH   - TODO: add argument description
  bValid  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
GetQHVerticalValidorInvalid (
  IN QH_STRUCT     *ptrQH
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
AllocateTDStruct (
  IN  USB_HC_DEV     *HcDev,
  OUT TD_STRUCT      **ppTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev       - TODO: add argument description
  ppTDStruct  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
InitTD (
  IN TD_STRUCT     *ptrTD
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTD - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
CreateTD (
  IN  USB_HC_DEV     *HcDev,
  OUT TD_STRUCT      **pptrTD
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev   - TODO: add argument description
  pptrTD  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
GenSetupStageTD (
  IN  USB_HC_DEV     *HcDev,
  IN  UINT8          DevAddr,
  IN  UINT8          Endpoint,
  IN  BOOLEAN        bSlow,
  IN  UINT8          *pDevReq,
  IN  UINT8          RequestLen,
  OUT TD_STRUCT      **ppTD
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev       - TODO: add argument description
  DevAddr     - TODO: add argument description
  Endpoint    - TODO: add argument description
  bSlow       - TODO: add argument description
  pDevReq     - TODO: add argument description
  RequestLen  - TODO: add argument description
  ppTD        - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
GenDataTD (
  IN  USB_HC_DEV     *HcDev,
  IN  UINT8          DevAddr,
  IN  UINT8          Endpoint,
  IN  UINT8          *pData,
  IN  UINT8          Len,
  IN  UINT8          PktID,
  IN  UINT8          Toggle,
  IN  BOOLEAN        bSlow,
  OUT TD_STRUCT      **ppTD
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev     - TODO: add argument description
  DevAddr   - TODO: add argument description
  Endpoint  - TODO: add argument description
  pData     - TODO: add argument description
  Len       - TODO: add argument description
  PktID     - TODO: add argument description
  Toggle    - TODO: add argument description
  bSlow     - TODO: add argument description
  ppTD      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
CreateStatusTD (
  IN  USB_HC_DEV     *HcDev,
  IN  UINT8          DevAddr,
  IN  UINT8          Endpoint,
  IN  UINT8          PktID,
  IN  BOOLEAN        bSlow,
  OUT TD_STRUCT      **ppTD
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev     - TODO: add argument description
  DevAddr   - TODO: add argument description
  Endpoint  - TODO: add argument description
  PktID     - TODO: add argument description
  bSlow     - TODO: add argument description
  ppTD      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDLinkPtrValidorInvalid (
  IN TD_STRUCT     *ptrTDStruct,
  IN BOOLEAN       bValid
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  bValid      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDLinkPtrQHorTDSelect (
  IN TD_STRUCT     *ptrTDStruct,
  IN BOOLEAN       bQH
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  bQH         - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDLinkPtrDepthorBreadth (
  IN TD_STRUCT     *ptrTDStruct,
  IN BOOLEAN       bDepth
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  bDepth      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDLinkPtr (
  IN TD_STRUCT     *ptrTDStruct,
  IN VOID          *ptrNext
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  ptrNext     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID                                *
GetTDLinkPtr (
  IN TD_STRUCT   *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
EnableorDisableTDShortPacket (
  IN TD_STRUCT   *ptrTDStruct,
  IN BOOLEAN     bEnable
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  bEnable     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDControlErrorCounter (
  IN TD_STRUCT     *ptrTDStruct,
  IN UINT8         nMaxErrors
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  nMaxErrors  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDLoworFullSpeedDevice (
  IN TD_STRUCT     *ptrTDStruct,
  IN BOOLEAN       bLowSpeedDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct     - TODO: add argument description
  bLowSpeedDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDControlIsochronousorNot (
  IN TD_STRUCT     *ptrTDStruct,
  IN BOOLEAN       bIsochronous
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct   - TODO: add argument description
  bIsochronous  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetorClearTDControlIOC (
  IN TD_STRUCT     *ptrTDStruct,
  IN BOOLEAN       bSet
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  bSet        - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDStatusActiveorInactive (
  IN TD_STRUCT     *ptrTDStruct,
  IN BOOLEAN       bActive
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  bActive     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT16
SetTDTokenMaxLength (
  IN TD_STRUCT     *ptrTDStruct,
  IN UINT16        nMaxLen
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  nMaxLen     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDTokenDataToggle1 (
  IN TD_STRUCT    *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDTokenDataToggle0 (
  IN TD_STRUCT    *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT8
GetTDTokenDataToggle (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDTokenEndPoint (
  IN TD_STRUCT     *ptrTDStruct,
  IN UINTN         nEndPoint
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  nEndPoint   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDTokenDeviceAddress (
  IN TD_STRUCT     *ptrTDStruct,
  IN UINTN         nDevAddr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  nDevAddr    - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDTokenPacketID (
  IN TD_STRUCT     *ptrTDStruct,
  IN UINT8         nPID
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description
  nPID        - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetTDDataBuffer (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsTDStatusActive (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsTDStatusStalled (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsTDStatusBufferError (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsTDStatusBabbleError (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsTDStatusNAKReceived (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsTDStatusCRCTimeOutError (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsTDStatusBitStuffError (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT16
GetTDStatusActualLength (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT16
GetTDTokenMaxLength (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT8
GetTDTokenEndPoint (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT8
GetTDTokenDeviceAddress (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT8
GetTDTokenPacketID (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT8                               *
GetTDDataBuffer (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
GetTDLinkPtrValidorInvalid (
  IN TD_STRUCT     *ptrTDStruct
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTDStruct - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINTN
CountTDsNumber (
  IN TD_STRUCT     *ptrFirstTD
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrFirstTD  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
LinkTDToQH (
  IN QH_STRUCT     *ptrQH,
  IN TD_STRUCT     *ptrTD
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrQH - TODO: add argument description
  ptrTD - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
LinkTDToTD (
  IN TD_STRUCT     *ptrPreTD,
  IN TD_STRUCT     *ptrTD
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrPreTD  - TODO: add argument description
  ptrTD     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetorClearCurFrameListTerminate (
  IN FRAMELIST_ENTRY     *pCurEntry,
  IN BOOLEAN             bSet
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  pCurEntry - TODO: add argument description
  bSet      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetCurFrameListQHorTD (
  IN FRAMELIST_ENTRY      *pCurEntry,
  IN BOOLEAN              bQH
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  pCurEntry - TODO: add argument description
  bQH       - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
GetCurFrameListTerminate (
  IN FRAMELIST_ENTRY     *pCurEntry
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  pCurEntry - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SetCurFrameListPointer (
  IN FRAMELIST_ENTRY     *pCurEntry,
  IN UINT8               *ptr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  pCurEntry - TODO: add argument description
  ptr       - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID                                *
GetCurFrameListPointer (
  IN FRAMELIST_ENTRY     *pCurEntry
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  pCurEntry - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
LinkQHToFrameList (
  IN FRAMELIST_ENTRY   *pEntry,
  IN UINT16            FrameListIndex,
  IN QH_STRUCT         *ptrQH
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  pEntry          - TODO: add argument description
  FrameListIndex  - TODO: add argument description
  ptrQH           - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
DeleteQHTDs (
  IN FRAMELIST_ENTRY *pEntry,
  IN QH_STRUCT       *ptrQH,
  IN TD_STRUCT       *ptrFirstTD,
  IN UINT16          FrameListIndex,
  IN BOOLEAN         SearchOther
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  pEntry          - TODO: add argument description
  ptrQH           - TODO: add argument description
  ptrFirstTD      - TODO: add argument description
  FrameListIndex  - TODO: add argument description
  SearchOther     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
DelLinkSingleQH (
  IN USB_HC_DEV      *HcDev,
  IN QH_STRUCT       *ptrQH,
  IN UINT16          FrameListIndex,
  IN BOOLEAN         SearchOther,
  IN BOOLEAN         Delete
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev           - TODO: add argument description
  ptrQH           - TODO: add argument description
  FrameListIndex  - TODO: add argument description
  SearchOther     - TODO: add argument description
  Delete          - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
DeleteQueuedTDs (
  IN USB_HC_DEV      *HcDev,
  IN TD_STRUCT       *ptrFirstTD
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev       - TODO: add argument description
  ptrFirstTD  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
InsertQHTDToINTList (
  IN USB_HC_DEV                        *HcDev,
  IN QH_STRUCT                         *ptrQH,
  IN TD_STRUCT                         *ptrFirstTD,
  IN UINT8                             DeviceAddress,
  IN UINT8                             EndPointAddress,
  IN UINT8                             DataToggle,
  IN UINTN                             DataLength,
  IN UINTN                             PollingInterval,
  IN VOID                              *Mapping,
  IN UINT8                             *DataBuffer,
  IN EFI_ASYNC_USB_TRANSFER_CALLBACK   CallBackFunction,
  IN VOID                              *Context
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev             - TODO: add argument description
  ptrQH             - TODO: add argument description
  ptrFirstTD        - TODO: add argument description
  DeviceAddress     - TODO: add argument description
  EndPointAddress   - TODO: add argument description
  DataToggle        - TODO: add argument description
  DataLength        - TODO: add argument description
  PollingInterval   - TODO: add argument description
  Mapping           - TODO: add argument description
  DataBuffer        - TODO: add argument description
  CallBackFunction  - TODO: add argument description
  Context           - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
DeleteAsyncINTQHTDs (
  IN  USB_HC_DEV  *HcDev,
  IN  UINT8       DeviceAddress,
  IN  UINT8       EndPointAddress,
  OUT UINT8       *DataToggle
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev           - TODO: add argument description
  DeviceAddress   - TODO: add argument description
  EndPointAddress - TODO: add argument description
  DataToggle      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
CheckTDsResults (
  IN  TD_STRUCT               *ptrTD,
  IN  UINTN                   RequiredLen,
  OUT UINT32                  *Result,
  OUT UINTN                   *ErrTDPos,
  OUT UINTN                   *ActualTransferSize
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrTD               - TODO: add argument description
  RequiredLen         - TODO: add argument description
  Result              - TODO: add argument description
  ErrTDPos            - TODO: add argument description
  ActualTransferSize  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
ExecuteAsyncINTTDs (
  IN  USB_HC_DEV      *HcDev,
  IN  INTERRUPT_LIST  *ptrList,
  OUT UINT32          *Result,
  OUT UINTN           *ErrTDPos,
  OUT UINTN           *ActualLen
  )
 /*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev     - TODO: add argument description
  ptrList   - TODO: add argument description
  Result    - TODO: add argument description
  ErrTDPos  - TODO: add argument description
  ActualLen - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
UpdateAsyncINTQHTDs (
  IN INTERRUPT_LIST  *ptrList,
  IN UINT32          Result,
  IN UINT32          ErrTDPos
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ptrList   - TODO: add argument description
  Result    - TODO: add argument description
  ErrTDPos  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
ReleaseInterruptList (
  IN USB_HC_DEV      *HcDev,
  IN EFI_LIST_ENTRY  *ListHead
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev     - TODO: add argument description
  ListHead  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
ExecuteControlTransfer (
  IN  USB_HC_DEV  *HcDev,
  IN  TD_STRUCT   *ptrTD,
  IN  UINT32      wIndex,
  OUT UINTN       *ActualLen,
  IN  UINTN       TimeOut,
  OUT UINT32      *TransferResult
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev           - TODO: add argument description
  ptrTD           - TODO: add argument description
  wIndex          - TODO: add argument description
  ActualLen       - TODO: add argument description
  TimeOut         - TODO: add argument description
  TransferResult  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
ExecBulkorSyncInterruptTransfer (
  IN  USB_HC_DEV  *HcDev,
  IN  TD_STRUCT   *ptrTD,
  IN  UINT32      wIndex,
  OUT UINTN       *ActualLen,
  OUT UINT8       *DataToggle,
  IN  UINTN       TimeOut,
  OUT UINT32      *TransferResult
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev           - TODO: add argument description
  ptrTD           - TODO: add argument description
  wIndex          - TODO: add argument description
  ActualLen       - TODO: add argument description
  DataToggle      - TODO: add argument description
  TimeOut         - TODO: add argument description
  TransferResult  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
InitializeMemoryManagement (
  IN USB_HC_DEV           *HcDev
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
CreateMemoryBlock (
  IN USB_HC_DEV            *HcDev,
  IN MEMORY_MANAGE_HEADER  **MemoryHeader,
  IN UINTN                 MemoryBlockSizeInPages
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev                   - TODO: add argument description
  MemoryHeader            - TODO: add argument description
  MemoryBlockSizeInPages  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
FreeMemoryHeader (
  IN USB_HC_DEV            *HcDev,
  IN MEMORY_MANAGE_HEADER  *MemoryHeader
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev         - TODO: add argument description
  MemoryHeader  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
UhciAllocatePool (
  IN USB_HC_DEV      *UhcDev,
  IN UINT8           **Pool,
  IN UINTN           AllocSize
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UhcDev    - TODO: add argument description
  Pool      - TODO: add argument description
  AllocSize - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
UhciFreePool (
  IN USB_HC_DEV      *HcDev,
  IN UINT8           *Pool,
  IN UINTN           AllocSize
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev     - TODO: add argument description
  Pool      - TODO: add argument description
  AllocSize - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
InsertMemoryHeaderToList (
  IN MEMORY_MANAGE_HEADER  *MemoryHeader,
  IN MEMORY_MANAGE_HEADER  *NewMemoryHeader
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  MemoryHeader    - TODO: add argument description
  NewMemoryHeader - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
AllocMemInMemoryBlock (
  IN MEMORY_MANAGE_HEADER  *MemoryHeader,
  IN VOID                  **Pool,
  IN UINTN                 NumberOfMemoryUnit
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  MemoryHeader        - TODO: add argument description
  Pool                - TODO: add argument description
  NumberOfMemoryUnit  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsMemoryBlockEmptied (
  IN MEMORY_MANAGE_HEADER  *MemoryHeaderPtr
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  MemoryHeaderPtr - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
DelinkMemoryBlock (
  IN MEMORY_MANAGE_HEADER    *FirstMemoryHeader,
  IN MEMORY_MANAGE_HEADER    *FreeMemoryHeader
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  FirstMemoryHeader - TODO: add argument description
  FreeMemoryHeader  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
DelMemoryManagement (
  IN USB_HC_DEV      *HcDev
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
EnableMaxPacketSize (
  IN USB_HC_DEV          *HcDev
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
CleanUsbTransactions (
  IN USB_HC_DEV    *HcDev
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  HcDev - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
TurnOffUSBEmulation (
  IN EFI_PCI_IO_PROTOCOL  *PciIo
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
