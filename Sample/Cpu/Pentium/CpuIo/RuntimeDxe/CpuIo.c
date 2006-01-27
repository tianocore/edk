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

#include "CpuIo.h"

#define IA32_MAX_IO_ADDRESS   0xFFFF
#define IA32_MAX_MEM_ADDRESS  0xFFFFFFFF


EFI_HANDLE          mHandle = NULL;
EFI_CPU_IO_PROTOCOL mCpuIo = {
  {
    CpuMemoryServiceRead,
    CpuMemoryServiceWrite
  },
  {
    CpuIoServiceRead,
    CpuIoServiceWrite
  }
};

STATIC
EFI_STATUS
EFIAPI
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
  Width         - Width of the Memory Access
  Count         - Count of the number of accesses to perform
  InStrideFlag  - Increment In for every iterration of Count if TRUE
  In            - Pointer to input buffer
  OutStrideFlag - Increment Out for every iterration of Count if TRUE
  Out           - Pointer to output buffer

Returns:
  EFI_SUCCESS           - Successful transaction
  EFI_INVALID_PARAMETER - Unsupported width and address combination

--*/
{
  UINTN Stride;
  UINTN InStride;
  UINTN OutStride;

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
      *In.ui8 = *Out.ui8;
      MEMORY_FENCE ();
    }
    break;

  case EfiCpuIoWidthUint16:
    for (; Count > 0; Count--, In.buf += InStride, Out.buf += OutStride) {
      MEMORY_FENCE ();
      *In.ui16 = *Out.ui16;
      MEMORY_FENCE ();
    }
    break;

  case EfiCpuIoWidthUint32:
    for (; Count > 0; Count--, In.buf += InStride, Out.buf += OutStride) {
      MEMORY_FENCE ();
      *In.ui32 = *Out.ui32;
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
  IN EFI_CPU_IO_PROTOCOL           *This,
  IN EFI_CPU_IO_PROTOCOL_WIDTH     Width,
  IN UINT64                        Address,
  IN UINTN                         Count,
  IN OUT VOID                      *Buffer
  )
/*++

Routine Description:
  Perform the Memory Access Read service for the CPU I/O Protocol

Arguments:
  This    - Pointer to an instance of the CPU I/O Protocol
  Width   - Width of the Memory Access
  Address - Address of the Memory access
  Count   - Count of the number of accesses to perform
  Buffer  - Pointer to the buffer to read or write from memory

Returns:
  EFI_SUCCESS             - The data was read from or written to the EFI 
                            System.
  EFI_INVALID_PARAMETER   - Width is invalid for this EFI System.
  EFI_INVALID_PARAMETER   - Buffer is NULL.
  EFI_UNSUPPORTED         - The Buffer is not aligned for the given Width.
  EFI_UNSUPPORTED         - The address range specified by Address, Width, 
                            and Count is not valid for this EFI System.

--*/
{
  PTR         In;
  PTR         Out;
  EFI_STATUS  Status;

  if (Buffer == NULL) {
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
  IN       EFI_CPU_IO_PROTOCOL           *This,
  IN       EFI_CPU_IO_PROTOCOL_WIDTH     Width,
  IN       UINT64                        Address,
  IN       UINTN                         Count,
  IN OUT   VOID                          *Buffer
  )
/*++

Routine Description:
  Perform the Memory Access Read service for the CPU I/O Protocol

Arguments:
  This    - Pointer to an instance of the CPU I/O Protocol
  Width   - Width of the Memory Access
  Address - Address of the Memory access
  Count   - Count of the number of accesses to perform
  Buffer  - Pointer to the buffer to read or write from memory

Returns:
  EFI_SUCCESS             - The data was read from or written to the EFI System.
  EFI_INVALID_PARAMETER   - Width is invalid for this EFI System.
  EFI_INVALID_PARAMETER   - Buffer is NULL.
  EFI_UNSUPPORTED         - The Buffer is not aligned for the given Width.
  EFI_UNSUPPORTED         - The address range specified by Address, Width, and 
                            Count is not valid for this EFI System.

--*/
// TODO:    This - add argument and description to function comment
{
  PTR         In;
  PTR         Out;
  EFI_STATUS  Status;

  if (Buffer == NULL) {
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
  IN       EFI_CPU_IO_PROTOCOL           *This,
  IN       EFI_CPU_IO_PROTOCOL_WIDTH     Width,
  IN       UINT64                        UserAddress,
  IN       UINTN                         Count,
  IN OUT   VOID                          *UserBuffer
  )
/*++

Routine Description:
  This is the service that implements the I/O read

Arguments:
  This        - Pointer to an instance of the CPU I/O Protocol
  Width       - Width of the Memory Access
  UserAddress - Address of the I/O access
  Count       - Count of the number of accesses to perform
  UserBuffer  - Pointer to the buffer to read or write from I/O space

Returns:

  Status
  EFI_SUCCESS             - The data was read from or written to the EFI System.
  EFI_INVALID_PARAMETER   - Width is invalid for this EFI System.
  EFI_INVALID_PARAMETER   - Buffer is NULL.
  EFI_UNSUPPORTED         - The Buffer is not aligned for the given Width.
  EFI_UNSUPPORTED         - The address range specified by Address, Width, and 
                            Count is not valid for this EFI System.
--*/
{
  UINTN       InStride;
  UINTN       OutStride;
  UINTN       Address;
  UINT32      Result;
  PTR         Buffer;
  EFI_STATUS  Status;

  if (UserBuffer == NULL) {
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

  Width = Width & 0x03;

  //
  // Loop for each iteration and move the data
  //
  switch (Width) {
  case EfiCpuIoWidthUint8:
    for (; Count > 0; Count--, Buffer.buf += OutStride, Address += InStride) {
      Result      = CpuIoRead8 ((UINT16) Address);
      *Buffer.ui8 = (UINT8) Result;
    }
    break;

  case EfiCpuIoWidthUint16:
    for (; Count > 0; Count--, Buffer.buf += OutStride, Address += InStride) {
      Result        = CpuIoRead16 ((UINT16) Address);
      *Buffer.ui16  = (UINT16) Result;
    }
    break;

  case EfiCpuIoWidthUint32:
    for (; Count > 0; Count--, Buffer.buf += OutStride, Address += InStride) {
      Result        = CpuIoRead32 ((UINT16) Address);
      *Buffer.ui32  = Result;
    }
    break;

  default:
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CpuIoServiceWrite (
  IN       EFI_CPU_IO_PROTOCOL           *This,
  IN       EFI_CPU_IO_PROTOCOL_WIDTH     Width,
  IN       UINT64                        UserAddress,
  IN       UINTN                         Count,
  IN OUT   VOID                          *UserBuffer
  )
/*++

Routine Description:
  This is the service that implements the I/O Write

Arguments:
  This        - Pointer to an instance of the CPU I/O Protocol
  Width       - Width of the Memory Access
  UserAddress - Address of the I/O access
  Count       - Count of the number of accesses to perform
  UserBuffer  - Pointer to the buffer to read or write from I/O space

Returns:
  EFI_SUCCESS             - The data was read from or written to the EFI System.
  EFI_INVALID_PARAMETER   - Width is invalid for this EFI System.
  EFI_INVALID_PARAMETER   - Buffer is NULL.
  EFI_UNSUPPORTED         - The Buffer is not aligned for the given Width.
  EFI_UNSUPPORTED         - The address range specified by Address, Width, and 
                            Count is not valid for this EFI System.

--*/
{
  UINTN       InStride;
  UINTN       OutStride;
  UINTN       Address;
  UINT32      Result;
  PTR         Buffer;
  EFI_STATUS  Status;

  if (UserBuffer == NULL) {
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

  Width = Width & 0x03;

  //
  // Loop for each iteration and move the data
  //
  switch (Width) {
  case EfiCpuIoWidthUint8:
    for (; Count > 0; Count--, Buffer.buf += OutStride, Address += InStride) {
      Result = *(UINT32 *) Buffer.ui8;
      CpuIoWrite8 ((UINT16) Address, Result);
    }
    break;

  case EfiCpuIoWidthUint16:
    for (; Count > 0; Count--, Buffer.buf += OutStride, Address += InStride) {
      Result = *(UINT32 *) Buffer.ui16;
      CpuIoWrite16 ((UINT16) Address, Result);
    }
    break;

  case EfiCpuIoWidthUint32:
    for (; Count > 0; Count--, Buffer.buf += OutStride, Address += InStride) {
      Result = *Buffer.ui32;
      CpuIoWrite32 ((UINT16) Address, Result);
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
  IN EFI_EVENT     Event,
  IN VOID          *Context
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
  EfiConvertPointer (EFI_INTERNAL_POINTER, &mCpuIo.Mem.Read);
  EfiConvertPointer (EFI_INTERNAL_POINTER, &mCpuIo.Mem.Write);
  EfiConvertPointer (EFI_INTERNAL_POINTER, &mCpuIo.Io.Read);
  EfiConvertPointer (EFI_INTERNAL_POINTER, &mCpuIo.Io.Write);
}


EFI_STATUS
CpuIoCheckAddressRange (
  IN EFI_CPU_IO_PROTOCOL_WIDTH     Width,
  IN UINT64                        Address,
  IN UINTN                         Count,
  IN VOID                          *Buffer,
  IN UINT64                        Limit
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


EFI_DRIVER_ENTRY_POINT (CpuIoInitialize)

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
  EFI_STATUS                Status;

  EfiInitializeRuntimeDriverLib (ImageHandle, SystemTable, CpuIoVirtualAddressChangeEvent);

  Status = gBS->InstallProtocolInterface (
                  &mHandle,
                  &gEfiCpuIoProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mCpuIo
                  );
  ASSERT_EFI_ERROR (Status);
  return Status;
}

