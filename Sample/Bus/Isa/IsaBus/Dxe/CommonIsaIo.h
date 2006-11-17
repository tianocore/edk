/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  CommonIsaIo.h
  
Abstract:
  
  The common part of header files for the EFI_ISA_IO_PROTOCOL and 
  EFI_LIGHT_ISA_IO protocol implementation.
  
Revision History:

--*/

#ifndef _EFI_ISA_IO_COMMON_H
#define _EFI_ISA_IO_COMMON_H

#include "IsaBus.h"

//
// ISA I/O Support Function Prototypes
//

EFI_STATUS
IsaIoVerifyAccess (
  IN     ISA_IO_DEVICE              *IsaIoDevice,
  IN     ISA_ACCESS_TYPE            Type,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINTN                      Count,
  IN OUT UINT32                     *Offset
  );
  
EFI_STATUS
EFIAPI
IsaIoIoRead (
  IN     EFI_INTERFACE_DEFINITION_FOR_ISA_IO        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH                  Width,
  IN     UINT32                                     Offset,
  IN     UINTN                                      Count,
  IN OUT VOID                                       *Buffer
  );

EFI_STATUS
EFIAPI
IsaIoIoWrite (
  IN     EFI_INTERFACE_DEFINITION_FOR_ISA_IO        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH                  Width,
  IN     UINT32                                     Offset,
  IN     UINTN                                      Count,
  IN OUT VOID                                       *Buffer
  );

EFI_STATUS
EFIAPI
IsaIoMap (
  IN     EFI_INTERFACE_DEFINITION_FOR_ISA_IO               *This,
  IN     EFI_ISA_IO_PROTOCOL_OPERATION                     Operation,
  IN     UINT8                                             ChannelNumber      OPTIONAL,
  IN     UINT32                                            ChannelAttributes,
  IN     VOID                                              *HostAddress,
  IN OUT UINTN                                             *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS                              *DeviceAddress,
  OUT    VOID                                              **Mapping
  );

EFI_STATUS
EFIAPI
IsaIoUnmap (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This,
  IN VOID                                 *Mapping
  );

EFI_STATUS
EFIAPI
IsaIoFlush (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This
  );

EFI_STATUS
ReportErrorStatusCode (
  EFI_STATUS_CODE_VALUE code
  );

EFI_STATUS
WriteDmaPort (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This,
  IN UINT32                               AddrOffset,
  IN UINT32                               PageOffset,
  IN UINT32                               CountOffset,
  IN UINT32                               BaseAddress,
  IN UINT16                               Count
  );

EFI_STATUS
WritePort (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This,
  IN UINT32                               Offset,
  IN UINT8                                Value
  );    

//
// Driver Support Global Variables
//

static EFI_ISA_DMA_REGISTERS  DmaRegisters[8] = {
  {
    0x00,
    0x87,
    0x01
  },
  {
    0x02,
    0x83,
    0x03
  },
  {
    0x04,
    0x81,
    0x05
  },
  {
    0x06,
    0x82,
    0x07
  },
  {
    0x00,
    0x00,
    0x00
  },  // Channel 4 is invalid
  {
    0xC4,
    0x8B,
    0xC6
  },
  {
    0xC8,
    0x89,
    0xCA
  },
  {
    0xCC,
    0x8A,
    0xCE
  },
};

#endif
