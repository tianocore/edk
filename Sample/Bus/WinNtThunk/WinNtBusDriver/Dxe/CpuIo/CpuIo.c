/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CpuIo.c

Abstract:

  This is the code that publishes the CPU I/O Protocol.
  The intent herein is to have a single I/O service that can load
  as early as possible, extend into runtime, and be layered upon by 
  the implementations of architectural protocols and the PCI Root
  Bridge I/O Protocol.

--*/

#include "WinNtBusDriver.h"

EFI_STATUS
CpuIoCheckAddressRange (
  IN  EFI_CPU_IO_PROTOCOL_WIDTH         Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  VOID                              *Buffer,
  IN  UINT64                            Limit
  );

EFI_STATUS
CpuIoMemRW (
  IN EFI_CPU_IO_PROTOCOL_WIDTH     Width,
  IN UINTN                         Count,
  IN BOOLEAN                       InStrideFlag,
  IN PTR                           In,
  IN BOOLEAN                       OutStrideFlag,
  OUT PTR                          Out
  )
/*++

Routine Description:

  Private service to provide the memory read/write

Arguments:

  Width of the Memory Access
  Count of the number of accesses to perform

Returns:

  Status

  EFI_SUCCESS           - Successful transaction
  EFI_INVALID_PARAMETER - Unsupported width and address combination

--*/
// TODO:    InStrideFlag - add argument and description to function comment
// TODO:    In - add argument and description to function comment
// TODO:    OutStrideFlag - add argument and description to function comment
// TODO:    Out - add argument and description to function comment
{
  UINTN Stride;
  UINTN InStride;
  UINTN OutStride;
  LONG  ReturnedLength;

  Width     = Width & 0x03;
  Stride    = 1 << Width;
  InStride  = InStrideFlag ? Stride : 0;
  OutStride = OutStrideFlag ? Stride : 0;

  //
  // Loop for each iteration and move the data
  //
  switch (Width) {
  case EfiCpuIoWidthUint8:
    for (; Count > 0; Count--, In.buf += InStride, Out.buf += OutStride) {
      MEMORY_FENCE ();
      gWinNtThunk->DeviceIoControl (
                    gDeviceHandle,    // Handle to device
                    IOCTL_MEM_COPY,   // IO Control code to use
                    Out.ui8,          // Out Buffer to communicate to driver
                    Count,            // Length of buffer in bytes.
                    In.ui8,           // In Buffer to fill in by kernel driver.
                    Count,            // Length of buffer in bytes.
                    &ReturnedLength,  // Bytes placed in In buffer.
                    NULL              // NULL means wait till op. completes.
                    );
      MEMORY_FENCE ();
    }
    break;

  case EfiCpuIoWidthUint16:
    for (; Count > 0; Count--, In.buf += InStride, Out.buf += OutStride) {
      MEMORY_FENCE ();
      gWinNtThunk->DeviceIoControl (
                    gDeviceHandle,    // Handle to device
                    IOCTL_MEM_COPY,   // IO Control code to use
                    Out.ui16,         // Out Buffer to communicate to driver
                    Count * 2,        // Length of buffer in bytes.
                    In.ui16,          // In Buffer to fill in by kernel driver.
                    Count * 2,        // Length of buffer in bytes.
                    &ReturnedLength,  // Bytes placed in In buffer.
                    NULL              // NULL means wait till op. completes.
                    );
      MEMORY_FENCE ();
    }
    break;

  case EfiCpuIoWidthUint32:
    for (; Count > 0; Count--, In.buf += InStride, Out.buf += OutStride) {
      MEMORY_FENCE ();
      gWinNtThunk->DeviceIoControl (
                    gDeviceHandle,    // Handle to device
                    IOCTL_MEM_COPY,   // IO Control code to use
                    Out.ui32,         // Out Buffer to communicate to driver
                    Count * 4,        // Length of buffer in bytes.
                    In.ui32,          // In Buffer to fill in by kernel driver.
                    Count * 4,        // Length of buffer in bytes.
                    &ReturnedLength,  // Bytes placed in In buffer.
                    NULL              // NULL means wait till op. completes.
                    );
      MEMORY_FENCE ();
    }
    break;

  default:
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CpuMemoryServiceRead (
  IN  EFI_CPU_IO_PROTOCOL               *This,
  IN  EFI_CPU_IO_PROTOCOL_WIDTH         Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  OUT VOID                          *Buffer
  )
/*++

Routine Description:

  Perform the Memory Access Read service for the CPU I/O Protocol

Arguments:

  Pointer to an instance of the CPU I/O Protocol
  Width of the Memory Access
  Address of the Memory access
  Count of the number of accesses to perform
  Pointer to the buffer to read or write from memory

Returns:

  Status

  EFI_SUCCESS             - The data was read from or written to the EFI 
                            System.
  EFI_INVALID_PARAMETER   - Width is invalid for this EFI System.
  EFI_INVALID_PARAMETER   - Buffer is NULL.
  EFI_UNSUPPORTED         - The Buffer is not aligned for the given Width.
  EFI_UNSUPPORTED         - The address range specified by Address, Width, 
                            and Count is not valid for this EFI System.

--*/
// TODO:    This - add argument and description to function comment
{
  PTR         In;
  PTR         Out;
  EFI_STATUS  Status;

  if (!Buffer) {
    return EFI_INVALID_PARAMETER;
  }

  Status = CpuIoCheckAddressRange (Width, Address, Count, Buffer, IA32_MAX_MEM_ADDRESS);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  In.buf  = Buffer;
  Out.buf = (VOID *) (UINTN) Address;
  if (Width >= EfiCpuIoWidthUint8 && Width <= EfiCpuIoWidthUint64) {
    return CpuIoMemRW (Width, Count, TRUE, In, TRUE, Out);
  }

  if (Width >= EfiCpuIoWidthFifoUint8 && Width <= EfiCpuIoWidthFifoUint64) {
    return CpuIoMemRW (Width, Count, TRUE, In, FALSE, Out);
  }

  if (Width >= EfiCpuIoWidthFillUint8 && Width <= EfiCpuIoWidthFillUint64) {
    return CpuIoMemRW (Width, Count, FALSE, In, TRUE, Out);
  }

  return EFI_INVALID_PARAMETER;
}

EFI_STATUS
EFIAPI
CpuMemoryServiceWrite (
  IN EFI_CPU_IO_PROTOCOL                *This,
  IN  EFI_CPU_IO_PROTOCOL_WIDTH         Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  OUT VOID                          *Buffer
  )
/*++

Routine Description:

  Perform the Memory Access Read service for the CPU I/O Protocol

Arguments:

  Pointer to an instance of the CPU I/O Protocol
  Width of the Memory Access
  Address of the Memory access
  Count of the number of accesses to perform
  Pointer to the buffer to read or write from memory

Returns:

  Status

  EFI_SUCCESS             - The data was read from or written to the EFI System.
  EFI_INVALID_PARAMETER   - Width is invalid for this EFI System.
  EFI_INVALID_PARAMETER   - Buffer is NULL.
  EFI_UNSUPPORTED         - The Buffer is not aligned for the given Width.
  EFI_UNSUPPORTED         - The address range specified by Address, Width, and 
                            Count is not valid for this EFI System.

--*/
// TODO:    This - add argument and description to function comment
{
  EFI_STATUS  Status;
  PTR         In;
  PTR         Out;

  if (!Buffer) {
    return EFI_INVALID_PARAMETER;
  }

  Status = CpuIoCheckAddressRange (Width, Address, Count, Buffer, IA32_MAX_MEM_ADDRESS);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  In.buf  = (VOID *) (UINTN) Address;
  Out.buf = Buffer;
  if (Width >= EfiCpuIoWidthUint8 && Width <= EfiCpuIoWidthUint64) {
    return CpuIoMemRW (Width, Count, TRUE, In, TRUE, Out);
  }

  if (Width >= EfiCpuIoWidthFifoUint8 && Width <= EfiCpuIoWidthFifoUint64) {
    return CpuIoMemRW (Width, Count, FALSE, In, TRUE, Out);
  }

  if (Width >= EfiCpuIoWidthFillUint8 && Width <= EfiCpuIoWidthFillUint64) {
    return CpuIoMemRW (Width, Count, TRUE, In, FALSE, Out);
  }

  return EFI_INVALID_PARAMETER;
}

EFI_STATUS
EFIAPI
CpuIoServiceRead (
  IN  EFI_CPU_IO_PROTOCOL               *This,
  IN  EFI_CPU_IO_PROTOCOL_WIDTH         Width,
  IN  UINT64                            UserAddress,
  IN  UINTN                             Count,
  IN  OUT VOID                          *UserBuffer
  )
/*++

Routine Description:
  
  This is the service that implements the I/O read

Arguments:

  Pointer to an instance of the CPU I/O Protocol
  Width of the Memory Access
  Address of the I/O access
  Count of the number of accesses to perform
  Pointer to the buffer to read or write from I/O space

Returns:

  Status
  EFI_SUCCESS             - The data was read from or written to the EFI System.
  EFI_INVALID_PARAMETER   - Width is invalid for this EFI System.
  EFI_INVALID_PARAMETER   - Buffer is NULL.
  EFI_UNSUPPORTED         - The Buffer is not aligned for the given Width.
  EFI_UNSUPPORTED         - The address range specified by Address, Width, and 
                            Count is not valid for this EFI System.
--*/
// TODO:    This - add argument and description to function comment
// TODO:    UserAddress - add argument and description to function comment
// TODO:    UserBuffer - add argument and description to function comment
{
  UINTN       Address;
  EFI_STATUS  Status;
  UINTN       InStride;
  UINTN       OutStride;
  PTR         Buffer;
  UINT32      Result;
  LONG        ReturnedLength;

  if (!UserBuffer) {
    return EFI_INVALID_PARAMETER;
  }

  Address     = (UINTN) UserAddress;
  Buffer.buf  = (UINT8 *) UserBuffer;

  if (Width >= EfiCpuIoWidthMaximum) {
    return EFI_INVALID_PARAMETER;
  }

  Status = CpuIoCheckAddressRange (Width, Address, Count, UserBuffer, IA32_MAX_IO_ADDRESS);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  InStride  = 1 << (Width & 0x03);
  OutStride = InStride;
  if (Width >= EfiCpuIoWidthFifoUint8 && Width <= EfiCpuIoWidthFifoUint64) {
    InStride = 0;
  }

  if (Width >= EfiCpuIoWidthFillUint8 && Width <= EfiCpuIoWidthFillUint64) {
    OutStride = 0;
  }

  Width   = Width & 0x03;

  Result  = 0xFFFF;

  //
  // NT32PASSTHRU: Thunk into our I/O routine to talk to the kernel driver IOCTL
  //               I/O such as write CF8->80000000, Read CFC etc......  Bit 31    = Enable PCI Config Access
  //                                                                    Bit 24-30 = Reserved must be zero (segment?)
  //                                                                    Bit 16-23 = Bus (0-255)
  //                                                                    Bit 11-15 = Device (0-31)
  //                                                                    Bit 8-10  = Function (0-7)
  //                                                                    Bit 2-7   = Target dword (0-63) (yields 256 bytes of access)
  //                                                                    Bit 0-1   = Zero's
  //
  //               To get the PCI express config header (4K) need to access address in the following manner:
  //                                                                    Bit 63:28 = 36bit address of the 256MB base address
  //                                                                    Bit 27:20 = Target Bus (0-255)
  //                                                                    Bit 19:15 = Target Device (0-31)
  //                                                                    Bit 14:12 = Target Function (0-7)
  //                                                                    Bit 11:2  = Target Dword (0-1023) (yields 4K size)
  //                                                                    Bit 0:1   = Zero's
  //
  switch (Width) {
  case EfiCpuIoWidthUint8:
    //
    // If the NT32 Passthrough has forced a HostBridgeInit
    //
    if (gHostBridgeInit) {
      if (gReadPending) {
        //
        // We found an access to our device
        //
        EFI_BREAKPOINT ();
        gWinNtThunk->DeviceIoControl (
                      gDeviceHandle,    // Handle to device
                      IOCTL_IO_READ,    // IO Control code to use
                      &Address,         // Address to communicate to driver
                      sizeof (UINT32),  // Length of buffer in bytes.
                      &Result,          // In Buffer to fill in by kernel driver.
                      sizeof (UINT8),   // Length of buffer in bytes.
                      &ReturnedLength,  // Bytes placed in In buffer.
                      NULL              // NULL means wait till op. completes.
                      );
        gReadPending = FALSE;
      } else {
        Result = 0xFFFF;
      }
    }

    *Buffer.ui8 = (UINT8) Result;
    break;

  case EfiCpuIoWidthUint16:
    //
    // If the NT32 Passthrough has forced a HostBridgeInit
    //
    if (gHostBridgeInit) {
      if (gReadPending) {
        //
        // We found an access to our device
        //
        EFI_BREAKPOINT ();
        gWinNtThunk->DeviceIoControl (
                      gDeviceHandle,    // Handle to device
                      IOCTL_IO_READ,    // IO Control code to use
                      &Address,         // Address to communicate to driver
                      sizeof (UINT32),  // Length of buffer in bytes.
                      &Result,          // In Buffer to fill in by kernel driver.
                      sizeof (UINT16),  // Length of buffer in bytes.
                      &ReturnedLength,  // Bytes placed in In buffer.
                      NULL              // NULL means wait till op. completes.
                      );
        gReadPending = FALSE;
      } else {
        Result = 0xFFFF;
      }
    }

    *Buffer.ui16 = (UINT16) Result;
    break;

  case EfiCpuIoWidthUint32:
    //
    // If the NT32 Passthrough has forced a HostBridgeInit
    //
    if (gHostBridgeInit) {
      if (gReadPending) {
        //
        // We found an access to our device
        //
        EFI_BREAKPOINT ();
        gWinNtThunk->DeviceIoControl (
                      gDeviceHandle,    // Handle to device
                      IOCTL_IO_READ,    // IO Control code to use
                      &Address,         // Address to communicate to driver
                      sizeof (UINT32),  // Length of buffer in bytes.
                      &Result,          // In Buffer to fill in by kernel driver.
                      sizeof (UINT32),  // Length of buffer in bytes.
                      &ReturnedLength,  // Bytes placed in In buffer.
                      NULL              // NULL means wait till op. completes.
                      );
        gReadPending = FALSE;
      } else {
        Result = 0xFFFF;
      }
    }

    *Buffer.ui32 = Result;
    break;

  default:
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CpuIoServiceWrite (
  IN EFI_CPU_IO_PROTOCOL                *This,
  IN  EFI_CPU_IO_PROTOCOL_WIDTH         Width,
  IN  UINT64                            UserAddress,
  IN  UINTN                             Count,
  IN  OUT VOID                          *UserBuffer
  )
/*++

Routine Description:

  
  This is the service that implements the I/O Write

Arguments:

  Pointer to an instance of the CPU I/O Protocol
  Width of the Memory Access
  Address of the I/O access
  Count of the number of accesses to perform
  Pointer to the buffer to read or write from I/O space

Returns:

  Status

  Status
  EFI_SUCCESS             - The data was read from or written to the EFI System.
  EFI_INVALID_PARAMETER   - Width is invalid for this EFI System.
  EFI_INVALID_PARAMETER   - Buffer is NULL.
  EFI_UNSUPPORTED         - The Buffer is not aligned for the given Width.
  EFI_UNSUPPORTED         - The address range specified by Address, Width, and 
                            Count is not valid for this EFI System.

--*/
// TODO:    This - add argument and description to function comment
// TODO:    UserAddress - add argument and description to function comment
// TODO:    UserBuffer - add argument and description to function comment
{
  UINTN       Address;
  EFI_STATUS  Status;
  UINTN       InStride;
  UINTN       OutStride;
  PTR         Buffer;
  LONG        ReturnedLength;
  UINT32      Result;

  if (!UserBuffer) {
    return EFI_INVALID_PARAMETER;
  }

  Address     = (UINTN) UserAddress;
  Buffer.buf  = (UINT8 *) UserBuffer;

  if (Width >= EfiCpuIoWidthMaximum) {
    return EFI_INVALID_PARAMETER;
  }

  Status = CpuIoCheckAddressRange (Width, Address, Count, UserBuffer, IA32_MAX_IO_ADDRESS);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  InStride  = 1 << (Width & 0x03);
  OutStride = InStride;
  if (Width >= EfiCpuIoWidthFifoUint8 && Width <= EfiCpuIoWidthFifoUint64) {
    InStride = 0;
  }

  if (Width >= EfiCpuIoWidthFillUint8 && Width <= EfiCpuIoWidthFillUint64) {
    OutStride = 0;
  }

  Width         = Width & 0x03;

  gReadPending  = FALSE;
  //
  // NT32PASSTHRU: Thunk into our I/O routine to talk to the kernel driver IOCTL
  //               I/O such as write CF8->80000000, Read CFC etc......  Bit 31    = Enable PCI Config Access
  //                                                                    Bit 24-30 = Reserved must be zero (segment?)
  //                                                                    Bit 16-23 = Bus (0-255)
  //                                                                    Bit 11-15 = Device (0-31)
  //                                                                    Bit 8-10  = Function (0-7)
  //                                                                    Bit 2-7   = Target dword (0-63) (yields 256 bytes of access)
  //                                                                    Bit 0-1   = Zero's)
  //
  switch (Width) {
  case EfiCpuIoWidthUint8:
    //
    // If the NT32 Passthrough has forced a HostBridgeInit and if the request is from a device
    // on a given Bus/Device location.  Mask off the remaining functions
    //
    if (gHostBridgeInit) {
      if ((*(UINT32 *) Buffer.ui32 & 0xFFFFFF00) == *((UINT32 *) &gConfigData)) {
        Result = *Buffer.ui8;

        //
        // We found an access to our device
        //
        EFI_BREAKPOINT ();
        gWinNtThunk->DeviceIoControl (
                      gDeviceHandle,    // Handle to device
                      IOCTL_IO_WRITE,   // IO Control code to use
                      &Address,         // Address to communicate to driver
                      sizeof (UINT32),  // Length of buffer in bytes.
                      &Result,          // In Buffer to fill in by kernel driver.
                      sizeof (UINT8),   // Length of buffer in bytes.
                      &ReturnedLength,  // Bytes placed in In buffer.
                      NULL              // NULL means wait till op. completes.
                      );
        gReadPending = TRUE;
        return EFI_SUCCESS;
      }
    }
    break;

  case EfiCpuIoWidthUint16:
    //
    // If the NT32 Passthrough has forced a HostBridgeInit and if the request is from a device
    // on a given Bus/Device location.  Mask off the remaining functions
    //
    if (gHostBridgeInit) {
      if ((*(UINT32 *) Buffer.ui32 & 0xFFFFFF00) == *((UINT32 *) &gConfigData)) {
        Result = *Buffer.ui16;

        //
        // We found an access to our device
        //
        EFI_BREAKPOINT ();
        gWinNtThunk->DeviceIoControl (
                      gDeviceHandle,    // Handle to device
                      IOCTL_IO_WRITE,   // IO Control code to use
                      &Address,         // Address to communicate to driver
                      sizeof (UINT32),  // Length of buffer in bytes.
                      &Result,          // In Buffer to fill in by kernel driver.
                      sizeof (UINT16),  // Length of buffer in bytes.
                      &ReturnedLength,  // Bytes placed in In buffer.
                      NULL              // NULL means wait till op. completes.
                      );
        gReadPending = TRUE;
        return EFI_SUCCESS;
      }
    }
    break;

  case EfiCpuIoWidthUint32:
    //
    // If the NT32 Passthrough has forced a HostBridgeInit and if the request is from a device
    // on a given Bus/Device location.  Mask off the remaining functions
    //
    if (gHostBridgeInit) {
      if ((*(UINT32 *) Buffer.ui32 & 0xFFFFFF00) == *((UINT32 *) &gConfigData)) {
        Result = *Buffer.ui32;

        //
        // We found an access to our device
        //
        EFI_BREAKPOINT ();
        gWinNtThunk->DeviceIoControl (
                      gDeviceHandle,    // Handle to device
                      IOCTL_IO_WRITE,   // IO Control code to use
                      &Address,         // Address to communicate to driver
                      sizeof (UINT32),  // Length of buffer in bytes.
                      &Result,          // In Buffer to fill in by kernel driver.
                      sizeof (UINT32),  // Length of buffer in bytes.
                      &ReturnedLength,  // Bytes placed in In buffer.
                      NULL              // NULL means wait till op. completes.
                      );
        gReadPending = TRUE;
        return EFI_SUCCESS;
      }
    }
    break;

  default:
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
CpuIoVirtualAddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
/*++

Routine Description:
  Fixup Private data with new virtual addresses. All the member functions need
  to be converted to virtual mode.

Arguments:
  (Standard EFI Event - EFI_EVENT_NOTIFY)

Returns:

--*/
// TODO:    Context - add argument and description to function comment
{
}

EFI_STATUS
CpuIoCheckAddressRange (
  IN  EFI_CPU_IO_PROTOCOL_WIDTH         Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  VOID                              *Buffer,
  IN  UINT64                            Limit
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Width   - TODO: add argument description
  Address - TODO: add argument description
  Count   - TODO: add argument description
  Buffer  - TODO: add argument description
  Limit   - TODO: add argument description

Returns:

  EFI_UNSUPPORTED - TODO: Add description for return value
  EFI_UNSUPPORTED - TODO: Add description for return value
  EFI_UNSUPPORTED - TODO: Add description for return value
  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  UINTN AlignMask;

  if (Address > Limit) {
    return EFI_UNSUPPORTED;
  }

  //
  // For FiFo type, the target address won't increase during the access, so treat count as 1
  //
  if (Width >= EfiCpuIoWidthFifoUint8 && Width <= EfiCpuIoWidthFifoUint64) {
    Count = 1;
  }

  Width = Width & 0x03;
  if (Address - 1 + (1 << Width) * Count > Limit) {
    return EFI_UNSUPPORTED;
  }

  AlignMask = (1 << Width) - 1;
  if ((UINTN) Buffer & AlignMask) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CpuIoInitialize (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
/*++

Routine Description:

  Initialize the state information for the CPU I/O Protocol

Arguments:

  ImageHandle of the loaded driver
  Pointer to the System Table

Returns:

  Status

  EFI_SUCCESS           - Protocol successfully installed
  EFI_OUT_OF_RESOURCES  - cannot allocate protocol data structure

--*/
// TODO:    SystemTable - add argument and description to function comment
{
  EFI_STATUS          Status;
  UINTN               NumHandles;
  EFI_HANDLE          *HandleBuffer;
  UINTN               Index;
  EFI_CPU_IO_PROTOCOL *OldCpuIoProtocol;

  //
  // Initialize the library
  //
  DxeInitializeDriverLib (ImageHandle, SystemTable);

  mCpuIoProtocol.Mem.Read   = CpuMemoryServiceRead;
  mCpuIoProtocol.Mem.Write  = CpuMemoryServiceWrite;
  mCpuIoProtocol.Io.Read    = CpuIoServiceRead;
  mCpuIoProtocol.Io.Write   = CpuIoServiceWrite;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiCpuIoProtocolGuid,
                  NULL,
                  &NumHandles,
                  &HandleBuffer
                  );

  if (Status == EFI_SUCCESS) {
    for (Index = 0; Index < NumHandles; Index++) {
      Status = gBS->HandleProtocol (
                      HandleBuffer[Index],
                      &gEfiCpuIoProtocolGuid,
                      (VOID **) &OldCpuIoProtocol
                      );
      if (Status == EFI_SUCCESS) {
        gBS->ReinstallProtocolInterface (
              HandleBuffer[Index],
              &gEfiCpuIoProtocolGuid,
              OldCpuIoProtocol,
              &mCpuIoProtocol
              );
      }
    }
  }

  ASSERT_EFI_ERROR (Status);
  return Status;
}
