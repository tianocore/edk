/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  BiosSnp16.h
    
Abstract: 

Revision History
--*/

#ifndef _BIOS_SNP_16_H
#define _BIOS_SNP_16_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "Pci22.h"
#include "Pxe.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (LegacyBiosThunk)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (EfiNetworkInterfaceIdentifier)
#include EFI_PROTOCOL_DEFINITION (SimpleNetwork)

//
// BIOS Simple Network Protocol Device Structure
//
#define EFI_SIMPLE_NETWORK_DEV_SIGNATURE    EFI_SIGNATURE_32 ('s', 'n', '1', '6')

#define INIT_PXE_STATUS                     0xabcd

#define EFI_SIMPLE_NETWORK_MAX_TX_FIFO_SIZE 64

typedef struct {
  UINT32  First;
  UINT32  Last;
  VOID * (Data[EFI_SIMPLE_NETWORK_MAX_TX_FIFO_SIZE]);
} EFI_SIMPLE_NETWORK_DEV_FIFO;

typedef struct {
  UINTN                                     Signature;
  EFI_HANDLE                                Handle;
  EFI_SIMPLE_NETWORK_PROTOCOL               SimpleNetwork;
  EFI_SIMPLE_NETWORK_MODE                   SimpleNetworkMode;
  EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL NII;
  EFI_DEVICE_PATH_PROTOCOL                  *DevicePath;
  EFI_PCI_IO_PROTOCOL                       *PciIo;
  LEGACY_BIOS_THUNK_PROTOCOL                *LegacyBiosThunk;

  //
  // Local Data for Simple Network Protocol interface goes here
  //
  BOOLEAN                                   UndiLoaded;
  EFI_EVENT                                 Event;
  UINT16                                    PxeEntrySegment;
  UINT16                                    PxeEntryOffset;
  EFI_SIMPLE_NETWORK_DEV_FIFO               TxBufferFifo;
  EFI_DEVICE_PATH_PROTOCOL                  *BaseDevicePath;
  PXE_t                                     *Pxe;                   // Pointer to !PXE structure
  PXENV_UNDI_GET_INFORMATION_t              GetInformation;         // Data from GET INFORMATION
  PXENV_UNDI_GET_NIC_TYPE_t                 GetNicType;             // Data from GET NIC TYPE
  PXENV_UNDI_GET_NDIS_INFO_t                GetNdisInfo;            // Data from GET NDIS INFO
  BOOLEAN                                   IsrValid;               // TRUE if Isr contains valid data
  PXENV_UNDI_ISR_t                          Isr;                    // Data from ISR
  PXENV_UNDI_TBD_t                          *Xmit;                  //
  VOID                                      *TxRealModeMediaHeader; // < 1 MB Size = 0x100
  VOID                                      *TxRealModeDataBuffer;  // < 1 MB Size = GetInformation.MaxTranUnit
  VOID                                      *TxDestAddr;            // < 1 MB Size = 16
  UINT8                                     InterruptStatus;        // returned/cleared by GetStatus, set in ISR
  UINTN                                     UndiLoaderTablePages;
  UINTN                                     DestinationDataSegmentPages;
  UINTN                                     DestinationStackSegmentPages;
  UINTN                                     DestinationCodeSegmentPages;
  VOID                                      *UndiLoaderTable;
  VOID                                      *DestinationDataSegment;
  VOID                                      *DestinationStackSegment;
  VOID                                      *DestinationCodeSegment;
} EFI_SIMPLE_NETWORK_DEV;

#define EFI_SIMPLE_NETWORK_DEV_FROM_THIS(a) \
  CR (a, \
      EFI_SIMPLE_NETWORK_DEV, \
      SimpleNetwork, \
      EFI_SIMPLE_NETWORK_DEV_SIGNATURE \
      )

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gBiosSnp16DriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gBiosSnp16ComponentName;

//
// Driver Binding Protocol functions
//
EFI_STATUS
BiosSnp16DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
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
BiosSnp16DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
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
BiosSnp16DriverBindingStop (
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
// Simple Network Protocol functions
//
EFI_STATUS
Undi16SimpleNetworkStart (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
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
Undi16SimpleNetworkStop (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
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
Undi16SimpleNetworkInitialize (
  IN EFI_SIMPLE_NETWORK_PROTOCOL                     *This,
  IN UINTN                                           ExtraRxBufferSize  OPTIONAL,
  IN UINTN                                           ExtraTxBufferSize  OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  ExtraRxBufferSize - GC_TODO: add argument description
  ExtraTxBufferSize - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
Undi16SimpleNetworkReset (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
  IN BOOLEAN                      ExtendedVerification
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
Undi16SimpleNetworkShutdown (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
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
Undi16SimpleNetworkReceiveFilters (
  IN EFI_SIMPLE_NETWORK_PROTOCOL                     * This,
  IN UINT32                                          Enable,
  IN UINT32                                          Disable,
  IN BOOLEAN                                         ResetMCastFilter,
  IN UINTN                                           MCastFilterCnt     OPTIONAL,
  IN EFI_MAC_ADDRESS                                 * MCastFilter OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  Enable            - GC_TODO: add argument description
  Disable           - GC_TODO: add argument description
  ResetMCastFilter  - GC_TODO: add argument description
  MCastFilterCnt    - GC_TODO: add argument description
  MCastFilter       - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
Undi16SimpleNetworkStationAddress (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  * This,
  IN BOOLEAN                      Reset,
  IN EFI_MAC_ADDRESS              * New OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description
  Reset - GC_TODO: add argument description
  New   - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
Undi16SimpleNetworkStatistics (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  * This,
  IN BOOLEAN                      Reset,
  IN OUT UINTN                    *StatisticsSize OPTIONAL,
  OUT EFI_NETWORK_STATISTICS      * StatisticsTable OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This            - GC_TODO: add argument description
  Reset           - GC_TODO: add argument description
  StatisticsSize  - GC_TODO: add argument description
  StatisticsTable - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
Undi16SimpleNetworkMCastIpToMac (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
  IN BOOLEAN                      IPv6,
  IN EFI_IP_ADDRESS               *IP,
  OUT EFI_MAC_ADDRESS             *MAC
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description
  IPv6  - GC_TODO: add argument description
  IP    - GC_TODO: add argument description
  MAC   - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
Undi16SimpleNetworkNvData (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
  IN BOOLEAN                      Write,
  IN UINTN                        Offset,
  IN UINTN                        BufferSize,
  IN OUT VOID                     *Buffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  Write       - GC_TODO: add argument description
  Offset      - GC_TODO: add argument description
  BufferSize  - GC_TODO: add argument description
  Buffer      - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
Undi16SimpleNetworkGetStatus (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  * This,
  OUT UINT32                      *InterruptStatus OPTIONAL,
  OUT VOID                        **TxBuf OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This            - GC_TODO: add argument description
  InterruptStatus - GC_TODO: add argument description
  TxBuf           - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
Undi16SimpleNetworkTransmit (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  * This,
  IN UINTN                        HeaderSize,
  IN UINTN                        BufferSize,
  IN VOID                         *Buffer,
  IN EFI_MAC_ADDRESS              * SrcAddr OPTIONAL,
  IN EFI_MAC_ADDRESS              * DestAddr OPTIONAL,
  IN UINT16                       *Protocol OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  HeaderSize  - GC_TODO: add argument description
  BufferSize  - GC_TODO: add argument description
  Buffer      - GC_TODO: add argument description
  SrcAddr     - GC_TODO: add argument description
  DestAddr    - GC_TODO: add argument description
  Protocol    - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
Undi16SimpleNetworkReceive (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  * This,
  OUT UINTN                       *HeaderSize OPTIONAL,
  IN OUT UINTN                    *BufferSize,
  OUT VOID                        *Buffer,
  OUT EFI_MAC_ADDRESS             * SrcAddr OPTIONAL,
  OUT EFI_MAC_ADDRESS             * DestAddr OPTIONAL,
  OUT UINT16                      *Protocol OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  HeaderSize  - GC_TODO: add argument description
  BufferSize  - GC_TODO: add argument description
  Buffer      - GC_TODO: add argument description
  SrcAddr     - GC_TODO: add argument description
  DestAddr    - GC_TODO: add argument description
  Protocol    - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

//
//
//
STATIC
VOID
Undi16SimpleNetworkEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Event   - GC_TODO: add argument description
  Context - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

STATIC
EFI_STATUS
Undi16SimpleNetworkLoadUndi (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

STATIC
EFI_STATUS
Undi16SimpleNetworkUnloadUndi (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
Undi16SimpleNetworkWaitForPacket (
  IN EFI_EVENT               Event,
  IN VOID                    *Context
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Event   - GC_TODO: add argument description
  Context - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
Undi16SimpleNetworkCheckForPacket (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This
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
SimpleNetworkAppendMacAddressDevicePath (
  EFI_DEVICE_PATH_PROTOCOL             **DevicePath,
  EFI_MAC_ADDRESS                      *CurrentAddress
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  DevicePath      - GC_TODO: add argument description
  CurrentAddress  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
CacheVectorAddress (
  UINT8   VectorNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  VectorNumber  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
RestoreCachedVectorAddress (
  UINT8   VectorNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  VectorNumber  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
LaunchBaseCode (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice,
  UINTN                   RomAddress
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  RomAddress          - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeStartUndi (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_START_UNDI_t               *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiStartup (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_STARTUP_t             *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiCleanup (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_CLEANUP_t             *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiInitialize (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_INITIALIZE_t          *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiResetNic (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_RESET_t               *PxeUndiTable,
  IN UINT16                               rx_filter
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description
  rx_filter           - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiShutdown (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_SHUTDOWN_t            *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiOpen (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_OPEN_t                *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiClose (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_CLOSE_t               *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiTransmit (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_TRANSMIT_t            *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiSetMcastAddr (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_SET_MCAST_ADDR_t      *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiSetStationAddr (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_SET_STATION_ADDR_t    *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiSetPacketFilter (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_SET_PACKET_FILTER_t   *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiGetInformation (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_GET_INFORMATION_t     *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiGetStatistics (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_GET_STATISTICS_t      *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiClearStatistics (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_CLEAR_STATISTICS_t    *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiInitiateDiags (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_INITIATE_DIAGS_t      *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiForceInterrupt (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_FORCE_INTERRUPT_t     *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiGetMcastAddr (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_GET_MCAST_ADDR_t      *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiGetNicType (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_GET_NIC_TYPE_t        *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiGetNdisInfo (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_GET_NDIS_INFO_t       *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiIsr (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_ISR_t                 *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiStop (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_STOP_UNDI_t                *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PxeUndiGetState (
  IN EFI_SIMPLE_NETWORK_DEV               *SimpleNetworkDevice,
  IN OUT PXENV_UNDI_GET_STATE_t           *PxeUndiTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  PxeUndiTable        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
MakePxeCall (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice,
  IN OUT VOID             *pTable,
  IN UINTN                TableSize,
  IN UINT16               CallIndex
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  pTable              - GC_TODO: add argument description
  TableSize           - GC_TODO: add argument description
  CallIndex           - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
BiosSnp16AllocatePagesBelowOneMb (
  UINTN  NumPages,
  VOID   **Buffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  NumPages  - GC_TODO: add argument description
  Buffer    - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

#endif
