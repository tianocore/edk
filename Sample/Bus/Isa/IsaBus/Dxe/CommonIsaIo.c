/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  CommonIsaIo.c
  
Abstract:

  The common part of implementation for EFI_ISA_IO_PROTOCOL and
  EFI_LIGHT_ISA_IO_PROTOCOL.

--*/

#include "CommonIsaIo.h"

EFI_STATUS
ReportErrorStatusCode (
  EFI_STATUS_CODE_VALUE Code
  )
/*++

Routine Description:

  report a error Status code of PCI bus driver controller

Arguments:

  Code         - The error status code.
  
Returns:

  EFI_SUCCESS  - Success to report status code.
  

--*/
{
  return EfiLibReportStatusCode (
           EFI_ERROR_CODE | EFI_ERROR_MINOR,
           Code,
           0,
           &gEfiCallerIdGuid,
           NULL
           );

}

//
// Driver Support Functions
//

EFI_STATUS
InitializeIsaIoInstance (
  IN ISA_IO_DEVICE               *IsaIoDevice,
  IN EFI_ISA_ACPI_RESOURCE_LIST  *IsaDeviceResourceList
  )
/*++

Routine Description:

  Initializes an ISA I/O Instance

Arguments:

  IsaIoDevice            - The iso device to be initialized.
  IsaDeviceResourceList  - The resource list.
  
Returns:

  EFI_SUCCESS            - Initial success.
  
--*/
{
  //
  // Initializes an ISA I/O Instance
  //
  EfiCopyMem (
    &IsaIoDevice->IsaIo,
    &IsaIoInterface,
    sizeof (EFI_INTERFACE_DEFINITION_FOR_ISA_IO)
    );

  IsaIoDevice->IsaIo.ResourceList = IsaDeviceResourceList;
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
IsaIoIoRead (
  IN     EFI_INTERFACE_DEFINITION_FOR_ISA_IO        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH                  Width,
  IN     UINT32                                     Offset,
  IN     UINTN                                      Count,
  IN OUT VOID                                       *Buffer
  )
/*++

Routine Description:

  Performs an ISA I/O Read Cycle

Arguments:

  This                  - A pointer to the EFI_ISA_IO_PROTOCOL or EFI_LIGHT_ISA_IO_PROTOCOL instance.
  Width                 - Signifies the width of the I/O operation.
  Offset                - The offset in ISA I/O space to start the I/O operation.  
  Count                 - The number of I/O operations to perform. 
  Buffer                - The destination buffer to store the results

Returns:

  EFI_SUCCESS           - The data was read from the device sucessfully.
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
             IsaAccessTypeIo,
             Width,
             Count,
             &Offset
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Call PciIo->Io.Read
  //
  Status = IsaIoDevice->PciIo->Io.Read (
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
IsaIoIoWrite (
  IN     EFI_INTERFACE_DEFINITION_FOR_ISA_IO        *This,
  IN     EFI_ISA_IO_PROTOCOL_WIDTH                  Width,
  IN     UINT32                                     Offset,
  IN     UINTN                                      Count,
  IN OUT VOID                                       *Buffer
  )
/*++

Routine Description:

  Performs an ISA I/O Write Cycle

Arguments:

  This                  - A pointer to the EFI_ISA_IO_PROTOCOL or EFI_LIGHT_ISA_IO_PROTOCOL instance.
  Width                 - Signifies the width of the I/O operation.
  Offset                - The offset in ISA I/O space to start the I/O operation.  
  Count                 - The number of I/O operations to perform. 
  Buffer                - The source buffer to write data from

Returns:

  EFI_SUCCESS           - The data was writen to the device sucessfully.
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
             IsaAccessTypeIo,
             Width,
             Count,
             &Offset
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Call PciIo->Io.Write
  //
  Status = IsaIoDevice->PciIo->Io.Write (
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
WritePort (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This,
  IN UINT32                               Offset,
  IN UINT8                                Value
  )
/*++

Routine Description:

  Writes an 8 bit I/O Port

Arguments:

  This                  - A pointer to the EFI_ISA_IO_PROTOCOL or EFI_LIGHT_ISA_IO_PROTOCOL instance.
  Offset                - The offset in ISA IO space to start the IO operation.  
  Value                 - The data to write port.
  
Returns:

  EFI_SUCCESS           - Success.
  EFI_INVALID_PARAMETER - Parameter is invalid.
  EFI_UNSUPPORTED       - The address range specified by Offset is not valid.
  EFI_OUT_OF_RESOURCES  - The request could not be completed due to a lack of resources.
  
--*/
{
  EFI_STATUS    Status;
  ISA_IO_DEVICE *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Call PciIo->Io.Write
  //
  Status = IsaIoDevice->PciIo->Io.Write (
                                    IsaIoDevice->PciIo,
                                    EfiPciIoWidthUint8,
                                    EFI_PCI_IO_PASS_THROUGH_BAR,
                                    Offset,
                                    1,
                                    &Value
                                    );
  if (EFI_ERROR (Status)) {
    ReportErrorStatusCode (EFI_IO_BUS_LPC | EFI_IOB_EC_CONTROLLER_ERROR);
    return Status;
  }

  gBS->Stall (50);

  return EFI_SUCCESS;
}

EFI_STATUS
WriteDmaPort (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This,
  IN UINT32                               AddrOffset,
  IN UINT32                               PageOffset,
  IN UINT32                               CountOffset,
  IN UINT32                               BaseAddress,
  IN UINT16                               Count
  )
/*++

Routine Description:

  Writes I/O operation base address and count number to a 8 bit I/O Port.

Arguments:

  This                  - A pointer to the EFI_ISA_IO_PROTOCOL or EFI_LIGHT_ISA_IO_PROTOCOL instance.
  AddrOffset            - The address' offset.
  PageOffset            - The page's offest.
  CountOffset           - The count's offset.
  BaseAddress           - The base address.
  Count                 - The number of I/O operations to perform. 
  
Returns:

  EFI_SUCCESS           - Success.
  EFI_INVALID_PARAMETER - Parameter is invalid.
  EFI_UNSUPPORTED       - The address range specified by these Offsets and Count is not valid.
  EFI_OUT_OF_RESOURCES  - The request could not be completed due to a lack of resources.

--*/
{
  EFI_STATUS  Status;

  Status = WritePort (This, AddrOffset, (UINT8) (BaseAddress & 0xff));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = WritePort (This, AddrOffset, (UINT8) ((BaseAddress >> 8) & 0xff));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = WritePort (This, PageOffset, (UINT8) ((BaseAddress >> 16) & 0xff));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = WritePort (This, CountOffset, (UINT8) (Count & 0xff));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = WritePort (This, CountOffset, (UINT8) ((Count >> 8) & 0xff));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
IsaIoUnmap (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This,
  IN VOID                                 *Mapping
  )
/*++

Routine Description:

  Unmaps a memory region for DMA

Arguments:

  This             - A pointer to the EFI_ISA_IO_PROTOCOL or EFI_LIGHT_ISA_IO_PROTOCOL instance.
  Mapping          - The mapping value returned from EFI_ISA_IO.Map().

Returns:

  EFI_SUCCESS      - The range was unmapped.
  EFI_DEVICE_ERROR - The data was not committed to the target system memory.

--*/
{
  ISA_MAP_INFO  *IsaMapInfo;

  //
  // See if the Map() operation associated with this Unmap() required a mapping
  // buffer.If a mapping buffer was not required, then this function simply
  // returns EFI_SUCCESS.
  //
  if (Mapping != NULL) {
    //
    // Get the MAP_INFO structure from Mapping
    //
    IsaMapInfo = (ISA_MAP_INFO *) Mapping;

    //
    // If this is a write operation from the Agent's point of view,
    // then copy the contents of the mapped buffer into the real buffer
    // so the processor can read the contents of the real buffer.
    //
    if (IsaMapInfo->Operation == EFI_ISA_IO_OPERATION_TOKEN) {
      EfiCopyMem (
        (VOID *) (UINTN) IsaMapInfo->HostAddress,
        (VOID *) (UINTN) IsaMapInfo->MappedHostAddress,
        IsaMapInfo->NumberOfBytes
        );
    }
    //
    // Free the mapped buffer and the MAP_INFO structure.
    //
    gBS->FreePages (IsaMapInfo->MappedHostAddress, IsaMapInfo->NumberOfPages);
    gBS->FreePool (IsaMapInfo);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
IsaIoFlush (
  IN EFI_INTERFACE_DEFINITION_FOR_ISA_IO  *This
  )
/*++

Routine Description:

  Flushes a DMA buffer

Arguments:

  This             - A pointer to the EFI_ISA_IO_PROTOCOL or EFI_LIGHT_ISA_IO_PROTOCOL instance.

Returns:

  EFI_SUCCESS      - The buffers were flushed.
  EFI_DEVICE_ERROR - The buffers were not flushed due to a hardware error.

--*/
{
  EFI_STATUS    Status;
  ISA_IO_DEVICE *IsaIoDevice;

  IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (This);

  //
  // Call PciIo->Flush
  //
  Status = IsaIoDevice->PciIo->Flush (IsaIoDevice->PciIo);

  if (EFI_ERROR (Status)) {
    ReportErrorStatusCode (EFI_IO_BUS_LPC | EFI_IOB_EC_CONTROLLER_ERROR);
  }

  return Status;
}
