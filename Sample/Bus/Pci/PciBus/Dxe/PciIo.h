/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciIo.h
  
Abstract:

  PCI Bus Driver

Revision History

--*/


#ifndef _EFI_PCI_IO_PROTOCOL_H
#define _EFI_PCI_IO_PROTOCOL_H


EFI_STATUS
InitializePciIoInstance (
  PCI_IO_DEVICE  *PciIoDevice
);

EFI_STATUS
PciIoVerifyBarAccess (
  PCI_IO_DEVICE          *PciIoDevice,
  UINT8                  BarIndex,
  PCI_BAR_TYPE           Type,
  IN EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN UINTN               Count,
  UINT64                 *Offset
);

EFI_STATUS
PciIoVerifyConfigAccess (
  PCI_IO_DEVICE                 *PciIoDevice,
  IN EFI_PCI_IO_PROTOCOL_WIDTH  Width,
  IN UINTN                      Count,
  IN UINT64                     *Offset
);

EFI_STATUS
EFIAPI
PciIoPollMem (
  IN  EFI_PCI_IO_PROTOCOL        *This,
  IN  EFI_PCI_IO_PROTOCOL_WIDTH  Width,
  IN  UINT8                      BarIndex,
  IN  UINT64                     Offset,
  IN  UINT64                     Mask,
  IN  UINT64                     Value,
  IN  UINT64                     Delay,
  OUT UINT64                     *Result
);

EFI_STATUS
EFIAPI
PciIoPollIo (
  IN  EFI_PCI_IO_PROTOCOL        *This,
  IN  EFI_PCI_IO_PROTOCOL_WIDTH  Width,
  IN  UINT8                      BarIndex,
  IN  UINT64                     Offset,
  IN  UINT64                     Mask,
  IN  UINT64                     Value,
  IN  UINT64                     Delay,
  OUT UINT64                     *Result
);

EFI_STATUS
EFIAPI
PciIoMemRead (
  IN     EFI_PCI_IO_PROTOCOL        *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
  IN     UINT8                      BarIndex,
  IN     UINT64                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
);

EFI_STATUS
EFIAPI
PciIoMemWrite (
  IN     EFI_PCI_IO_PROTOCOL        *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
  IN     UINT8                      BarIndex,
  IN     UINT64                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
);

EFI_STATUS
EFIAPI
PciIoIoRead (
  IN     EFI_PCI_IO_PROTOCOL        *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
  IN     UINT8                      BarIndex,
  IN     UINT64                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
);

EFI_STATUS
EFIAPI
PciIoIoWrite (
  IN     EFI_PCI_IO_PROTOCOL        *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
  IN     UINT8                      BarIndex,
  IN     UINT64                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
);

EFI_STATUS
EFIAPI
PciIoConfigRead (
  IN     EFI_PCI_IO_PROTOCOL        *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
  IN     UINT32                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
);

EFI_STATUS
EFIAPI
PciIoConfigWrite (
  IN     EFI_PCI_IO_PROTOCOL        *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH  Width,
  IN     UINT32                     Offset,
  IN     UINTN                      Count,
  IN OUT VOID                       *Buffer
);

EFI_STATUS
EFIAPI
PciIoCopyMem (
  IN EFI_PCI_IO_PROTOCOL  *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT8                        DestBarIndex,
  IN     UINT64                       DestOffset,
  IN     UINT8                        SrcBarIndex,
  IN     UINT64                       SrcOffset,
  IN     UINTN                        Count
);

EFI_STATUS
EFIAPI
PciIoMap (
  IN     EFI_PCI_IO_PROTOCOL            *This,
  IN     EFI_PCI_IO_PROTOCOL_OPERATION  Operation,
  IN     VOID                           *HostAddress,
  IN OUT UINTN                          *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS           *DeviceAddress,
  OUT    VOID                           **Mapping
);

EFI_STATUS
EFIAPI
PciIoUnmap (
  IN  EFI_PCI_IO_PROTOCOL  *This,
  IN  VOID                 *Mapping
);

EFI_STATUS
EFIAPI
PciIoAllocateBuffer (
  IN  EFI_PCI_IO_PROTOCOL   *This,
  IN  EFI_ALLOCATE_TYPE     Type,
  IN  EFI_MEMORY_TYPE       MemoryType,
  IN  UINTN                 Pages,
  OUT VOID                  **HostAddress,
  IN  UINT64                Attributes
);


EFI_STATUS
EFIAPI
PciIoFreeBuffer (
  IN  EFI_PCI_IO_PROTOCOL   *This,
  IN  UINTN                 Pages,
  IN  VOID                  *HostAddress
);

EFI_STATUS
EFIAPI
PciIoFlush (
  IN  EFI_PCI_IO_PROTOCOL  *This
);

EFI_STATUS
EFIAPI
PciIoGetLocation (
  IN  EFI_PCI_IO_PROTOCOL  *This,
  OUT UINTN                *Segment,
  OUT UINTN                *Bus,
  OUT UINTN                *Device,
  OUT UINTN                *Function
);

BOOLEAN 
CheckBarType ( 
  IN PCI_IO_DEVICE  *PciIoDevice,
  UINT8                  BarIndex,
  PCI_BAR_TYPE      BarType
);
 
EFI_STATUS
ModifyRootBridgeAttributes (  
  IN  PCI_IO_DEVICE                            *PciIoDevice,
  IN  UINT64                                   Attributes,
  IN  EFI_PCI_IO_PROTOCOL_ATTRIBUTE_OPERATION  Operation
);
  
EFI_STATUS
SupportPaletteSnoopAttributes (  
  IN  PCI_IO_DEVICE                            *PciIoDevice,  
  IN  EFI_PCI_IO_PROTOCOL_ATTRIBUTE_OPERATION  Operation
);

EFI_STATUS
EFIAPI
PciIoAttributes (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN  EFI_PCI_IO_PROTOCOL_ATTRIBUTE_OPERATION  Operation,
  IN  UINT64                                   Attributes,
  OUT UINT64                                   *Result   OPTIONAL
);

EFI_STATUS
EFIAPI
PciIoGetBarAttributes(
  IN EFI_PCI_IO_PROTOCOL    *This,
  IN  UINT8                          BarIndex,
  OUT UINT64                         *Supports,   OPTIONAL
  OUT VOID                           **Resources  OPTIONAL
);

EFI_STATUS
EFIAPI
PciIoSetBarAttributes(
  IN EFI_PCI_IO_PROTOCOL  *This,
  IN     UINT64                       Attributes,
  IN     UINT8                        BarIndex,
  IN OUT UINT64                       *Offset,
  IN OUT UINT64                       *Length
);
    
EFI_STATUS 
UpStreamBridgesAttributes(
  IN  PCI_IO_DEVICE                            *PciIoDevice,
  IN  EFI_PCI_IO_PROTOCOL_ATTRIBUTE_OPERATION  Operation,
  IN  UINT64                                   Attributes
);
  
BOOLEAN 
PciDevicesOnTheSamePath(
  IN PCI_IO_DEVICE        *PciDevice1,
  IN PCI_IO_DEVICE       *PciDevice2
);

#endif