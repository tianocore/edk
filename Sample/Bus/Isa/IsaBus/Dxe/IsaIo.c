/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  IsaIo.c
  
Abstract:

  The specific part of implementation for EFI_ISA_IO_PROTOCOL. 
  Please refer to the common part: CommonIsaIo.c for both EFI_
  ISA_IO_PROTOCOL and EFI_LIGHT_ISA_IO_PROTOCOl.

--*/

#include "IsaIo.h"

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

  Verifies access to an ISA device

Arguments:

  IsaIoDevice           - The ISA device to be verified.
  Type                  - The Access type.
  Width                 - Signifies the width of the memory operation.
  Count                 - The number of memory operations to perform. 
  Offset                - The offset in ISA memory space to start the memory operation.  
  
Returns:

  EFI_SUCCESS           - Verify success.
  EFI_INVALID_PARAMETER - One of the parameters has an invalid value.
  EFI_UNSUPPORTED       - The device ont support the access type.

--*/
{
  EFI_ISA_ACPI_RESOURCE *Item;
  EFI_STATUS            Status;

  if (Width < EfiIsaIoWidthUint8 ||
      Width >= EfiIsaIoWidthMaximum ||
      Width == EfiIsaIoWidthReserved ||
      Width == EfiIsaIoWidthFifoReserved ||
      Width == EfiIsaIoWidthFillReserved
      ) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Type != IsaAccessTypeMem) && (Type != IsaAccessTypeIo)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // If Width is EfiIsaIoWidthFifoUintX then convert to EfiIsaIoWidthUintX
  // If Width is EfiIsaIoWidthFillUintX then convert to EfiIsaIoWidthUintX
  //
  if (Width >= EfiIsaIoWidthFifoUint8 && Width <= EfiIsaIoWidthFifoReserved) {
    Count = 1;
  }

  Width &= 0x03;

  Status  = EFI_UNSUPPORTED;
  Item    = IsaIoDevice->IsaIo.ResourceList->ResourceItem;
  while (Item->Type != EfiIsaAcpiResourceEndOfList) {
    if ((Type == IsaAccessTypeMem && Item->Type == EfiIsaAcpiResourceMemory) ||
        (Type == IsaAccessTypeIo && Item->Type == EfiIsaAcpiResourceIo)
        ) {
      if (*Offset >= Item->StartRange && (*Offset + Count * (1 << Width)) - 1 <= Item->EndRange) {
        return EFI_SUCCESS;
      }

      if (*Offset >= Item->StartRange && *Offset <= Item->EndRange) {
        Status = EFI_INVALID_PARAMETER;
      }
    }

    Item++;
  }

  return Status;
}

EFI_STATUS
EFIAPI
IsaIoMemRead (
  IN     EFI_INTERFACE_DEFINITION_FOR_ISA_IO       *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH                 Width,
  IN     UINT32                                    Offset,
  IN     UINTN                                     Count,
  IN OUT VOID                                      *Buffer
  )
/*++

Routine Description:

  Performs an ISA Memory Read Cycle

Arguments:

  This                  - A pointer to the EFI_ISA_IO_PROTOCOL instance.
  Width                 - Signifies the width of the memory operation.
  Offset                - The offset in ISA memory space to start the memory operation.  
  Count                 - The number of memory operations to perform. 
  Buffer                - The destination buffer to store the results
  
Returns:

  EFI_SUCCESS           - The data was read from the device successfully.
  EFI_UNSUPPORTED       - The Offset is not valid for this device.
  EFI_INVALID_PARAMETER - Width or Count, or both, were invalid.
  EFI_OUT_OF_RESOURCES  - The request could not be completed due to a lack of resources.

--*/
{
  EFI_STATUS    Status;
  ISA_IO_DEVICE *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Verify the Isa Io Access
  //
  Status = IsaIoVerifyAccess (
             IsaIoDevice,
             IsaAccessTypeMem,
             Width,
             Count,
             &Offset
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Call PciIo->Mem.Read
  //
  Status = IsaIoDevice->PciIo->Mem.Read (
                                     IsaIoDevice->PciIo,
                                     (EFI_PCI_IO_PROTOCOL_WIDTH) Width,
                                     EFI_PCI_IO_PASS_THROUGH_BAR,
                                     Offset,
                                     Count,
                                     Buffer
                                     );

  if (EFI_ERROR (Status)) {
    ReportErrorStatusCode (EFI_IO_BUS_LPC | EFI_IOB_EC_CONTROLLER_ERROR);
  }

  return Status;
}

EFI_STATUS
EFIAPI
IsaIoMemWrite (
  IN     EFI_INTERFACE_DEFINITION_FOR_ISA_IO        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH                  Width,
  IN     UINT32                                     Offset,
  IN     UINTN                                      Count,
  IN OUT VOID                                       *Buffer
  )
/*++

Routine Description:

  Performs an ISA Memory Write Cycle

Arguments:

  This                  - A pointer to the EFI_ISA_IO_PROTOCOL instance.  
  Width                 - Signifies the width of the memory operation.
  Offset                - The offset in ISA memory space to start the memory operation.  
  Count                 - The number of memory operations to perform. 
  Buffer                - The source buffer to write data from

Returns:

  EFI_SUCCESS           - The data was written to the device sucessfully.
  EFI_UNSUPPORTED       - The Offset is not valid for this device.
  EFI_INVALID_PARAMETER - Width or Count, or both, were invalid.
  EFI_OUT_OF_RESOURCES  - The request could not be completed due to a lack of resources.

--*/
{
  EFI_STATUS    Status;
  ISA_IO_DEVICE *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Verify Isa IO Access
  //
  Status = IsaIoVerifyAccess (
             IsaIoDevice,
             IsaAccessTypeMem,
             Width,
             Count,
             &Offset
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Call PciIo->Mem.Write
  //
  Status = IsaIoDevice->PciIo->Mem.Write (
                                     IsaIoDevice->PciIo,
                                     (EFI_PCI_IO_PROTOCOL_WIDTH) Width,
                                     EFI_PCI_IO_PASS_THROUGH_BAR,
                                     Offset,
                                     Count,
                                     Buffer
                                     );

  if (EFI_ERROR (Status)) {
    ReportErrorStatusCode (EFI_IO_BUS_LPC | EFI_IOB_EC_CONTROLLER_ERROR);
  }

  return Status;
}

EFI_STATUS
EFIAPI
IsaIoCopyMem (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO        *This,
  IN EFI_ISA_IO_PROTOCOL_WIDTH                  Width,
  IN UINT32                                     DestOffset,
  IN UINT32                                     SrcOffset,
  IN UINTN                                      Count
  )
/*++

Routine Description:

  Performs an ISA I/O Copy Memory 

Arguments:

  This                  - A pointer to the EFI_ISA_IO_PROTOCOL instance.
  Width                 - Signifies the width of the memory copy operation.
  DestOffset            - The offset of the destination 
  SrcOffset             - The offset of the source
  Count                 - The number of memory copy  operations to perform

Returns:

  EFI_SUCCESS           - The data was copied sucessfully.
  EFI_UNSUPPORTED       - The DestOffset or SrcOffset is not valid for this device.
  EFI_INVALID_PARAMETER - Width or Count, or both, were invalid.
  EFI_OUT_OF_RESOURCES  - The request could not be completed due to a lack of resources.

--*/
{
  EFI_STATUS    Status;
  ISA_IO_DEVICE *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Verify Isa IO Access for destination and source
  //
  Status = IsaIoVerifyAccess (
             IsaIoDevice,
             IsaAccessTypeMem,
             Width,
             Count,
             &DestOffset
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = IsaIoVerifyAccess (
             IsaIoDevice,
             IsaAccessTypeMem,
             Width,
             Count,
             &SrcOffset
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Call PciIo->CopyMem
  //
  Status = IsaIoDevice->PciIo->CopyMem (
                                 IsaIoDevice->PciIo,
                                 (EFI_PCI_IO_PROTOCOL_WIDTH) Width,
                                 EFI_PCI_IO_PASS_THROUGH_BAR,
                                 DestOffset,
                                 EFI_PCI_IO_PASS_THROUGH_BAR,
                                 SrcOffset,
                                 Count
                                 );

  if (EFI_ERROR (Status)) {
    ReportErrorStatusCode (EFI_IO_BUS_LPC | EFI_IOB_EC_CONTROLLER_ERROR);
  }

  return Status;
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

  Maps a memory region for DMA

Arguments:

  This                  - A pointer to the EFI_ISA_IO_PROTOCOL instance.
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
                        - to access the hosts HostAddress.  
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
  BOOLEAN               Master;
  BOOLEAN               Read;
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
  // Make sure the Operation parameter is valid
  //
  if (Operation < 0 || Operation >= EfiIsaIoOperationMaximum) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // See if this is a Slave DMA Operation
  //
  Master  = TRUE;
  Read    = FALSE;
  if (Operation == EfiIsaIoOperationSlaveRead) {
    Operation = EfiIsaIoOperationBusMasterRead;
    Master    = FALSE;
    Read      = TRUE;
  }

  if (Operation == EfiIsaIoOperationSlaveWrite) {
    Operation = EfiIsaIoOperationBusMasterWrite;
    Master    = FALSE;
    Read      = FALSE;
  }

  if (!Master) {
    //
    // Make sure that ChannelNumber is a valid channel number
    // Channel 4 is used to cascade, so it is illegal.
    //
    if (ChannelNumber == 4 || ChannelNumber > 7) {
      return EFI_INVALID_PARAMETER;
    }
    //
    // This implementation only support COMPATIBLE DMA Transfers
    //
    if (!(ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_COMPATIBLE)) {
      return EFI_INVALID_PARAMETER;
    }

    if (ChannelAttributes &
       (
         EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_A |
         EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_B |
         EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SPEED_C
       )
       ) {
      return EFI_INVALID_PARAMETER;
    }

    if (ChannelNumber < 4) {
      //
      // If this is Channel 0..3, then the width must be 8 bit
      //
      if (!(ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_WIDTH_8) ||
          (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_WIDTH_16)
          ) {
        return EFI_INVALID_PARAMETER;
      }
    } else {
      //
      // If this is Channel 4..7, then the width must be 16 bit
      //
      if ((ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_WIDTH_8) ||
          (!(ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_WIDTH_16))
          ) {
        return EFI_INVALID_PARAMETER;
      }
    }
    //
    // Either Demand Mode or Single Mode must be selected, but not both
    //
    if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SINGLE_MODE) {
      if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_DEMAND_MODE) {
        return EFI_INVALID_PARAMETER;
      }
    } else {
      if (!(ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_DEMAND_MODE)) {
        return EFI_INVALID_PARAMETER;
      }
    }
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
    if (Operation == EfiIsaIoOperationBusMasterRead) {
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
  // If this is a Bus Master operation then return
  //
  if (Master) {
    return EFI_SUCCESS;
  }
  //
  // Figure out what to program into the DMA Channel Mode Register
  //
  DmaMode = (UINT8) (DMA_MODE_INCREMENT | (ChannelNumber & 0x03));
  if (Read) {
    DmaMode |= DMA_MODE_READ;
  } else {
    DmaMode |= DMA_MODE_WRITE;
  }

  if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_AUTO_INITIALIZE) {
    DmaMode |= DMA_MODE_AUTO_INITIALIZE;
  }

  if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_DEMAND_MODE) {
    DmaMode |= DMA_MODE_DEMAND;
  }

  if (ChannelAttributes & EFI_ISA_IO_SLAVE_DMA_ATTRIBUTE_SINGLE_MODE) {
    DmaMode |= DMA_MODE_SINGLE;
  }
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
  if (ChannelNumber < 4) {
    BaseAddress = (UINT32) (*DeviceAddress);
    Count       = (UINT16) (*NumberOfBytes - 1);
  } else {
    BaseAddress = (UINT32) (((UINT32) (*DeviceAddress) & 0xff0000) | (((UINT32) (*DeviceAddress) & 0xffff) >> 1));
    Count       = (UINT16) ((*NumberOfBytes - 1) >> 1);
  }
  //
  // Program the DMA Write Single Mask Register for ChannelNumber
  // Clear the DMA Byte Pointer Register
  //
  if (ChannelNumber < 4) {
    DmaMask         = DMA_SINGLE_MASK_0_3;
    DmaClear        = DMA_CLEAR_0_3;
    DmaChannelMode  = DMA_MODE_0_3;
  } else {
    DmaMask         = DMA_SINGLE_MASK_4_7;
    DmaClear        = DMA_CLEAR_4_7;
    DmaChannelMode  = DMA_MODE_4_7;
  }

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

EFI_STATUS
EFIAPI
IsaIoAllocateBuffer (
  IN  EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This,
  IN  EFI_ALLOCATE_TYPE                    Type,
  IN  EFI_MEMORY_TYPE                      MemoryType,
  IN  UINTN                                Pages,
  OUT VOID                                 **HostAddress,
  IN  UINT64                               Attributes
  )
/*++

Routine Description:

  Allocates a common buffer for DMA

Arguments:

  This                  - A pointer to the EFI_ISA_IO_PROTOCOL instance.
  Type                  - The type allocation to perform.
  MemoryType            - The type of memory to allocate.
  Pages                 - The number of pages to allocate.
  HostAddress           - A pointer to store the base address of the allocated range.
  Attributes            - The requested bit mask of attributes for the allocated range.

Returns:

  EFI_SUCCESS           - The requested memory pages were allocated.
  EFI_INVALID_PARAMETER - Type is invalid or MemoryType is invalid or HostAddress is NULL
  EFI_UNSUPPORTED       - Attributes is unsupported or the memory range specified 
                          by HostAddress, Pages, and Type is not available for common buffer use.
  EFI_OUT_OF_RESOURCES  - The memory pages could not be allocated.

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;

  if (HostAddress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Type < AllocateAnyPages || Type >= MaxAllocateType) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // The only valid memory types are EfiBootServicesData and EfiRuntimeServicesData
  //
  if (MemoryType != EfiBootServicesData && MemoryType != EfiRuntimeServicesData) {
    return EFI_INVALID_PARAMETER;
  }

  if (Attributes &~(EFI_ISA_IO_ATTRIBUTE_MEMORY_WRITE_COMBINE | EFI_ISA_IO_ATTRIBUTE_MEMORY_CACHED)) {
    return EFI_UNSUPPORTED;
  }

  PhysicalAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) (ISA_MAX_MEMORY_ADDRESS - 1);
  if (Type == AllocateAddress) {
    if ((UINTN) (*HostAddress) >= ISA_MAX_MEMORY_ADDRESS) {
      return EFI_UNSUPPORTED;
    } else {
      PhysicalAddress = (UINTN) (*HostAddress);
    }
  }

  if (Type == AllocateAnyPages) {
    Type = AllocateMaxAddress;
  }

  Status = gBS->AllocatePages (Type, MemoryType, Pages, &PhysicalAddress);
  if (EFI_ERROR (Status)) {
    ReportErrorStatusCode (EFI_IO_BUS_LPC | EFI_IOB_EC_CONTROLLER_ERROR);
    return Status;
  }

  *HostAddress = (VOID *) (UINTN) PhysicalAddress;
  return Status;
}

EFI_STATUS
EFIAPI
IsaIoFreeBuffer (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This,
  IN UINTN                                Pages,
  IN VOID                                 *HostAddress
  )
/*++

Routine Description:

  Frees a common buffer 

Arguments:

  This                  - A pointer to the EFI_ISA_IO_PROTOCOL instance.
  Pages                 - The number of pages to free.
  HostAddress           - The base address of the allocated range.

Returns:

  EFI_SUCCESS           - The requested memory pages were freed.
  EFI_INVALID_PARAMETER - The memory was not allocated with EFI_ISA_IO.AllocateBufer().

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;

  PhysicalAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) HostAddress;
  Status = gBS->FreePages (
                  PhysicalAddress,
                  Pages
                  );
  if (EFI_ERROR (Status)) {
    ReportErrorStatusCode (EFI_IO_BUS_LPC | EFI_IOB_EC_CONTROLLER_ERROR);
  }

  return Status;
}
