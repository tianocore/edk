/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  LightIsaIo.c
  
Abstract:

  The specific part of implementation for EFI_LIGHT_ISA_IO_PROTOCOL. 
  This specific part describes the difference between the definition
  and implementation of EFI_ISA_IO_PROTOCOL and EFI_LIGHT_ISA_IO_PROTOCOL:
  IsaIoVerifyAccess is not supported actually, and IsaIoMap only supports
  Slave Read/Write to save code size.  Please refer to the common part: 
  CommonIsaIo.c for both EFI_ISA_IO_PROTOCOL and EFI_LIGHT_ISA_IO_PROTOCOL.

--*/

#include "LightIsaIo.h"

EFI_STATUS
IsaIoVerifyAccess (
  IN     ISA_IO_DEVICE              *IsaIoDevice,
  IN     ISA_ACCESS_TYPE            Type,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH  Width,
  IN     UINTN                      Count,
  IN OUT UINT32                     *Offset
  )
/*++

Routine Description:

  EFI_LIGHT_ISA_IO_PROTOCOL doesn't verfiy access for I/O operation actually.
  Keep this interface to let IsaIoIoRead/Write be common for both EFI_ISA_IO_PROTOCOL
  and EFI_LIGHT_ISA_IO_PROTOCOL.

Arguments:

  IsaIoDevice           - The ISA device to be verified.
  Type                  - The Access type.
  Width                 - Signifies the width of the memory operation.
  Count                 - The number of memory operations to perform. 
  Offset                - The offset in ISA memory space to start the memory operation.  
  
Returns:

  EFI_SUCCESS           - Always success.

--*/
{
  //
  // needn't verify access in EFI_LIGHT_ISA_IO protocol
  //
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
IsaIoMap (
  IN     EFI_INTERFACE_DEFINITION_FOR_ISA_IO                  *This,
  IN     EFI_ISA_IO_PROTOCOL_OPERATION                        Operation,
  IN     UINT8                                                ChannelNumber         OPTIONAL,
  IN     UINT32                                               ChannelAttributes,
  IN     VOID                                                 *HostAddress,
  IN OUT UINTN                                                *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS                                 *DeviceAddress,
  OUT    VOID                                                 **Mapping
  )
/*++

Routine Description:

  Maps a memory region for DMA, note that EFI_LIGHT_ISA_IO_PROTOCOL 
  only supports slave read/write operation to save code size.

Arguments:

  This                  - A pointer to the EFI_LIGHT_ISA_IO_PROTOCOL instance.
  Operation             - Indicates the type of DMA (slave or bus master), and if 
                          the DMA operation is going to read or write to system memory. 
  ChannelNumber         - The slave channel number to use for this DMA operation. 
                          If Operation and ChannelAttributes shows that this device 
                          performs bus mastering DMA, then this field is ignored.  
                          The legal range for this field is 0..7.  
  ChannelAttributes     - The attributes of the DMA channel to use for this DMA operation
  HostAddress           - The system memory address to map to the device.  
  NumberOfBytes         - On input the number of bytes to map.  On output the number 
                          of bytes that were mapped.
  DeviceAddress         - The resulting map address for the bus master device to use 
                          to access the hosts HostAddress.  
  Mapping               - A resulting value to pass to EFI_ISA_IO.Unmap().

Returns:

  EFI_SUCCESS           - The range was mapped for the returned NumberOfBytes.
  EFI_INVALID_PARAMETER - The Operation or HostAddress is undefined.
  EFI_UNSUPPORTED       - The HostAddress can not be mapped as a common buffer.
  EFI_DEVICE_ERROR      - The system hardware could not map the requested address.
  EFI_OUT_OF_RESOURCES  - The memory pages could not be allocated.

--*/
{
  EFI_STATUS            Status;
  ISA_IO_DEVICE         *IsaIoDevice;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;
  ISA_MAP_INFO          *IsaMapInfo;
  UINT8                 DmaMode;
  UINTN                 MaxNumberOfBytes;
  UINT32                BaseAddress;
  UINT16                Count;

  UINT8                 DmaMask;
  UINT8                 DmaClear;
  UINT8                 DmaChannelMode;
  
  if ((NULL == This) ||
      (NULL == HostAddress) ||
      (NULL == NumberOfBytes) ||
      (NULL == DeviceAddress) ||
      (NULL == Mapping)
      ) {
    return EFI_INVALID_PARAMETER;
  }

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Initialize the return values to their defaults
  //
  *Mapping = NULL;

  //
  // Make sure the Operation parameter is valid.
  // Light IsaIo only supports two operations.
  //
  if (!(Operation == EfiIsaIoOperationSlaveRead || 
        Operation == EfiIsaIoOperationSlaveWrite)) {
    return EFI_INVALID_PARAMETER;
  }

  if (ChannelNumber >= 4) {
    //
    // The Light IsaIo doesn't support channelNumber larger than 4.
    //
    return EFI_INVALID_PARAMETER;
  }

  //
  // Map the HostAddress to a DeviceAddress.
  //
  PhysicalAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) HostAddress;
  if ((PhysicalAddress +*NumberOfBytes) > ISA_MAX_MEMORY_ADDRESS) {
    //
    // Common Buffer operations can not be remapped.  If the common buffer
    // is above 16MB, then it is not possible to generate a mapping, so return
    // an error.
    //
    if (Operation == EfiIsaIoOperationBusMasterCommonBuffer) {
      return EFI_UNSUPPORTED;
    }
    //
    // Allocate an ISA_MAP_INFO structure to remember the mapping when Unmap()
    // is called later.
    //
    IsaMapInfo = EfiLibAllocatePool (sizeof (ISA_MAP_INFO));
    if (IsaMapInfo == NULL) {
      *NumberOfBytes = 0;
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Return a pointer to the MAP_INFO structure in Mapping
    //
    *Mapping = IsaMapInfo;

    //
    // Initialize the MAP_INFO structure
    //
    IsaMapInfo->Operation         = Operation;
    IsaMapInfo->NumberOfBytes     = *NumberOfBytes;
    IsaMapInfo->NumberOfPages     = EFI_SIZE_TO_PAGES (*NumberOfBytes);
    IsaMapInfo->HostAddress       = PhysicalAddress;
    IsaMapInfo->MappedHostAddress = ISA_MAX_MEMORY_ADDRESS - 1;

    //
    // Allocate a buffer below 4GB to map the transfer to.
    //
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiBootServicesData,
                    IsaMapInfo->NumberOfPages,
                    &IsaMapInfo->MappedHostAddress
                    );
    if (EFI_ERROR (Status)) {
      gBS->FreePool (IsaMapInfo);
      *NumberOfBytes  = 0;
      *Mapping        = NULL;
      return Status;
    }
    //
    // If this is a read operation from the DMA agents's point of view,
    // then copy the contents of the real buffer into the mapped buffer
    // so the DMA agent can read the contents of the real buffer.
    //
    if (Operation == EfiIsaIoOperationSlaveRead) {
      EfiCopyMem (
        (VOID *) (UINTN) IsaMapInfo->MappedHostAddress,
        (VOID *) (UINTN) IsaMapInfo->HostAddress,
        IsaMapInfo->NumberOfBytes
        );
    }
    //
    // The DeviceAddress is the address of the maped buffer below 16 MB
    //
    *DeviceAddress = IsaMapInfo->MappedHostAddress;
  } else {
    //
    // The transfer is below 16 MB, so the DeviceAddress is simply the
    // HostAddress
    //
    *DeviceAddress = PhysicalAddress;
  }
  
  //
  // Figure out what to program into the DMA Channel Mode Register
  //
  DmaMode = (UINT8) (DMA_MODE_INCREMENT | (ChannelNumber & 0x03));
  if (Operation == EfiIsaIoOperationSlaveRead) {
    DmaMode |= DMA_MODE_READ;
  } else {
    DmaMode |= DMA_MODE_WRITE;
  }
  //
  // We only support EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SINGLE_MODE in simplified IsaIo
  //
  DmaMode |= DMA_MODE_SINGLE;

  //
  // A Slave DMA transfer can not cross a 64K boundary.
  // Compute *NumberOfBytes based on this restriction.
  //
  MaxNumberOfBytes = 0x10000 - ((UINT32) (*DeviceAddress) & 0xffff);
  if (*NumberOfBytes > MaxNumberOfBytes) {
    *NumberOfBytes = MaxNumberOfBytes;
  }
  //
  // Compute the values to program into the BaseAddress and Count registers
  // of the Slave DMA controller
  //
  BaseAddress = (UINT32) (*DeviceAddress);
  Count       = (UINT16) (*NumberOfBytes - 1);
  //
  // Program the DMA Write Single Mask Register for ChannelNumber
  // Clear the DMA Byte Pointer Register
  //
  DmaMask         = DMA_SINGLE_MASK_0_3;
  DmaClear        = DMA_CLEAR_0_3;
  DmaChannelMode  = DMA_MODE_0_3;

  Status = WritePort (
             This,
             DmaMask,
             (UINT8) (DMA_CHANNEL_MASK_SELECT | (ChannelNumber & 0x03))
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = WritePort (
             This,
             DmaClear,
             (UINT8) (DMA_CHANNEL_MASK_SELECT | (ChannelNumber & 0x03))
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = WritePort (This, DmaChannelMode, DmaMode);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = WriteDmaPort (
             This,
             DmaRegisters[ChannelNumber].Address,
             DmaRegisters[ChannelNumber].Page,
             DmaRegisters[ChannelNumber].Count,
             BaseAddress,
             Count
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = WritePort (
             This,
             DmaMask,
             (UINT8) (ChannelNumber & 0x03)
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}
