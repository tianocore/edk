/*++
 
Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciRootBridgeIo.c
    
Abstract:

  PCI Root Bridge Io Protocol producer

--*/

#include "PciRootBridge.h"
#include "Pci22.h"

typedef struct {
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR SpaceDesp[TypeMax];
  EFI_ACPI_END_TAG_DESCRIPTOR       EndDesp;
} RESOURCE_CONFIGURATION;

RESOURCE_CONFIGURATION  mConfiguration = {
  {{0x8A, 0x2B, 1, 0, 0, 0, 0, 0, 0, 0},
  {0x8A, 0x2B, 0, 0, 0, 32, 0, 0, 0, 0}, 
  {0x8A, 0x2B, 0, 0, 6, 32, 0, 0, 0, 0},
  {0x8A, 0x2B, 0, 0, 0, 64, 0, 0, 0, 0},
  {0x8A, 0x2B, 0, 0, 6, 64, 0, 0, 0, 0},
  {0x8A, 0x2B, 2, 0, 0, 0, 0, 0, 0, 0}},
  {0x79, 0}
};

//
// Protocol Member Function Prototypes
//

EFI_STATUS
EFIAPI
RootBridgeIoPollMem (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  );

EFI_STATUS
EFIAPI
RootBridgeIoPollIo (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  );

EFI_STATUS
EFIAPI
RootBridgeIoMemRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
EFIAPI
RootBridgeIoMemWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
EFIAPI
RootBridgeIoIoRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 UserAddress,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *UserBuffer
  );

EFI_STATUS
EFIAPI
RootBridgeIoIoWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 UserAddress,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *UserBuffer
  );

EFI_STATUS
EFIAPI
RootBridgeIoCopyMem (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL          *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH    Width,
  IN     UINT64                                   DestAddress,
  IN     UINT64                                   SrcAddress,
  IN     UINTN                                    Count
  );

EFI_STATUS
EFIAPI
RootBridgeIoPciRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
EFIAPI
RootBridgeIoPciWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
EFIAPI
RootBridgeIoMap (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL            *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION  Operation,
  IN     VOID                                       *HostAddress,
  IN OUT UINTN                                      *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS                       *DeviceAddress,
  OUT    VOID                                       **Mapping
  );

EFI_STATUS
EFIAPI
RootBridgeIoUnmap (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  VOID                             *Mapping
  );

EFI_STATUS
EFIAPI
RootBridgeIoAllocateBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  EFI_ALLOCATE_TYPE                Type,
  IN  EFI_MEMORY_TYPE                  MemoryType,
  IN  UINTN                            Pages,
  OUT VOID                             **HostAddress,
  IN  UINT64                           Attributes
  );

EFI_STATUS
EFIAPI
RootBridgeIoFreeBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  UINTN                            Pages,
  OUT VOID                             *HostAddress
  );

EFI_STATUS
EFIAPI
RootBridgeIoFlush (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This
  );

EFI_STATUS
EFIAPI
RootBridgeIoGetAttributes (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT UINT64                           *Supported,
  OUT UINT64                           *Attributes
  );

EFI_STATUS
EFIAPI
RootBridgeIoSetAttributes (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN     UINT64                           Attributes,
  IN OUT UINT64                           *ResourceBase,
  IN OUT UINT64                           *ResourceLength
  );

EFI_STATUS
EFIAPI
RootBridgeIoConfiguration (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT    VOID                             **Resources
  );

//
// Sub Function Prototypes
//

typedef union {
  UINT8 VOLATILE  *buf;
  UINT8 VOLATILE  *ui8;
  UINT16 VOLATILE *ui16;
  UINT32 VOLATILE *ui32;
  UINT64 VOLATILE *ui64;
  UINTN VOLATILE  ui;
} PTR;

STATIC
EFI_STATUS
RootBridgeIoPciRW (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     BOOLEAN                                Write,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 UserAddress,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *UserBuffer
  );

//
// Pci Root Bridge Io Module Variables
//
EFI_METRONOME_ARCH_PROTOCOL *mMetronome;
EFI_CPU_IO_PROTOCOL         *mCpuIo;

EFI_STATUS
RootBridgeConstructor (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL    *Protocol,
  IN EFI_HANDLE                         HostBridgeHandle,
  IN UINT64                             Attri,
  IN PCI_ROOT_BRIDGE_RESOURCE_APPETURE  ResAppeture
  )
/*++

Routine Description:

    Construct the Pci Root Bridge Io protocol

Arguments:

    Protocol - protocol to initialize
    
Returns:

    None

--*/
// TODO:    HostBridgeHandle - add argument and description to function comment
// TODO:    Attri - add argument and description to function comment
// TODO:    ResAppeture - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS                Status;
  PCI_ROOT_BRIDGE_INSTANCE  *PrivateData;
  PCI_RESOURCE_TYPE         Index;

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS (Protocol);

  //
  // The host to pci bridge, the host memory and io addresses are
  // direct mapped to pci addresses, so no need translate, set bases to 0.
  //
  PrivateData->MemBase  = ResAppeture.MemBase;
  PrivateData->IoBase   = ResAppeture.IoBase;

  //
  // The host bridge only supports 32bit addressing for memory
  // and standard IA32 16bit io
  //
  PrivateData->MemLimit = ResAppeture.MemLimit;
  PrivateData->IoLimit  = ResAppeture.IoLimit;

  //
  // Bus Appeture for this Root Bridge (Possible Range)
  //
  PrivateData->BusBase  = ResAppeture.BusBase;
  PrivateData->BusLimit = ResAppeture.BusLimit;

  //
  // Specific for ea815 chipset
  //
  for (Index = TypeIo; Index < TypeMax; Index++) {
    PrivateData->ResAllocNode[Index].Type   = Index;
    PrivateData->ResAllocNode[Index].Base   = 0;
    PrivateData->ResAllocNode[Index].Length = 0;
    PrivateData->ResAllocNode[Index].Status = ResNone;
  }

  EfiInitializeLock (&PrivateData->PciLock, EFI_TPL_HIGH_LEVEL);
  PrivateData->PciAddress       = 0xCF8;
  PrivateData->PciData          = 0xCFC;

  PrivateData->RootBridgeAttrib = Attri;

  PrivateData->Attributes       = 0;
  PrivateData->Supports = EFI_PCI_ATTRIBUTE_IDE_PRIMARY_IO   |
                          EFI_PCI_ATTRIBUTE_IDE_SECONDARY_IO |
                          EFI_PCI_ATTRIBUTE_ISA_IO           |
                          EFI_PCI_ATTRIBUTE_VGA_PALETTE_IO   |
                          EFI_PCI_ATTRIBUTE_VGA_MEMORY       |
                          EFI_PCI_ATTRIBUTE_VGA_IO;

  Protocol->ParentHandle    = HostBridgeHandle;

  Protocol->PollMem         = RootBridgeIoPollMem;
  Protocol->PollIo          = RootBridgeIoPollIo;

  Protocol->Mem.Read        = RootBridgeIoMemRead;
  Protocol->Mem.Write       = RootBridgeIoMemWrite;

  Protocol->Io.Read         = RootBridgeIoIoRead;
  Protocol->Io.Write        = RootBridgeIoIoWrite;

  Protocol->CopyMem         = RootBridgeIoCopyMem;

  Protocol->Pci.Read        = RootBridgeIoPciRead;
  Protocol->Pci.Write       = RootBridgeIoPciWrite;

  Protocol->Map             = RootBridgeIoMap;
  Protocol->Unmap           = RootBridgeIoUnmap;

  Protocol->AllocateBuffer  = RootBridgeIoAllocateBuffer;
  Protocol->FreeBuffer      = RootBridgeIoFreeBuffer;

  Protocol->Flush           = RootBridgeIoFlush;

  Protocol->GetAttributes   = RootBridgeIoGetAttributes;
  Protocol->SetAttributes   = RootBridgeIoSetAttributes;

  Protocol->Configuration   = RootBridgeIoConfiguration;

  Protocol->SegmentNumber   = 0;

  Status                    = gBS->LocateProtocol (&gEfiMetronomeArchProtocolGuid, NULL, &mMetronome);
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (&gEfiCpuIoProtocolGuid, NULL, &mCpuIo);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RootBridgeIoPollMem (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  )
/*++

Routine Description:
  Memory Poll
  
Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Mask - add argument and description to function comment
// TODO:    Value - add argument and description to function comment
// TODO:    Delay - add argument and description to function comment
// TODO:    Result - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
// TODO:    EFI_TIMEOUT - add return value to function comment
{
  EFI_STATUS  Status;
  UINT64      NumberOfTicks;
  UINTN       Remainder;

  if (Result == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width < 0 || Width > EfiPciWidthUint64) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // No matter what, always do a single poll.
  //
  Status = This->Mem.Read (This, Width, Address, 1, Result);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((*Result & Mask) == Value) {
    return EFI_SUCCESS;
  }

  if (Delay == 0) {
    return EFI_SUCCESS;

  } else {

    //
    // Determine the proper # of metronome ticks to wait for polling the
    // location.  The nuber of ticks is Roundup (Delay / mMetronome->TickPeriod)+1
    // The "+1" to account for the possibility of the first tick being short
    // because we started in the middle of a tick.
    //
    NumberOfTicks = DivU64x32 (Delay, mMetronome->TickPeriod, &Remainder);
    if (Remainder != 0) {
      NumberOfTicks += 1;
    }

    NumberOfTicks += 1;

    while (NumberOfTicks) {

      mMetronome->WaitForTick (mMetronome, 1);

      Status = This->Mem.Read (This, Width, Address, 1, Result);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      if ((*Result & Mask) == Value) {
        return EFI_SUCCESS;
      }

      NumberOfTicks -= 1;
    }
  }

  return EFI_TIMEOUT;
}

EFI_STATUS
EFIAPI
RootBridgeIoPollIo (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  )
/*++

Routine Description:
  Io Poll
  
Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Mask - add argument and description to function comment
// TODO:    Value - add argument and description to function comment
// TODO:    Delay - add argument and description to function comment
// TODO:    Result - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
// TODO:    EFI_TIMEOUT - add return value to function comment
{
  EFI_STATUS  Status;
  UINT64      NumberOfTicks;
  UINTN       Remainder;

  //
  // No matter what, always do a single poll.
  //

  if (Result == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width < 0 || Width > EfiPciWidthUint64) {
    return EFI_INVALID_PARAMETER;
  }

  Status = This->Io.Read (This, Width, Address, 1, Result);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((*Result & Mask) == Value) {
    return EFI_SUCCESS;
  }

  if (Delay == 0) {
    return EFI_SUCCESS;

  } else {

    //
    // Determine the proper # of metronome ticks to wait for polling the
    // location.  The number of ticks is Roundup (Delay / mMetronome->TickPeriod)+1
    // The "+1" to account for the possibility of the first tick being short
    // because we started in the middle of a tick.
    //
    NumberOfTicks = DivU64x32 (Delay, (UINT32) mMetronome->TickPeriod, &Remainder);
    if (Remainder != 0) {
      NumberOfTicks += 1;
    }

    NumberOfTicks += 1;

    while (NumberOfTicks) {

      mMetronome->WaitForTick (mMetronome, 1);

      Status = This->Io.Read (This, Width, Address, 1, Result);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      if ((*Result & Mask) == Value) {
        return EFI_SUCCESS;
      }

      NumberOfTicks -= 1;
    }
  }

  return EFI_TIMEOUT;
}

EFI_STATUS
EFIAPI
RootBridgeIoMemRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  )
/*++

Routine Description:
  Memory read
  
Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  PCI_ROOT_BRIDGE_INSTANCE              *PrivateData;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH OldWidth;
  UINTN                                 OldCount;
  UINTN                                 AlignMask;
  VOID                                  *Buf;
  VOID                                  *ptr;
  EFI_STATUS                            Status;

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width < 0 || Width >= EfiPciWidthMaximum) {
    return EFI_INVALID_PARAMETER;
  }

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS (This);

  //
  // Check memory access limit
  //
  if (Address < PrivateData->MemBase) {
    return EFI_INVALID_PARAMETER;
  }

  OldWidth  = Width;
  OldCount  = Count;
  if (Width >= EfiPciWidthFifoUint8 && Width <= EfiPciWidthFifoUint64) {
    Count = 1;
  }

  Width &= 0x03;

  if (Address + (1 << Width) * Count - 1 > PrivateData->MemLimit) {
    return EFI_INVALID_PARAMETER;
  }

  AlignMask = (1 << Width) - 1;
  if ((UINTN) Buffer & AlignMask) {
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    (1 << Width) * (OldCount + 1),
                    &Buf
                    );
    ptr = (VOID *) (((UINTN) (((UINTN) Buf) &~(AlignMask))) + (1 << Width));
    Status = mCpuIo->Mem.Read (
                          mCpuIo,
                          (EFI_CPU_IO_PROTOCOL_WIDTH) OldWidth,
                          Address,
                          OldCount,
                          ptr
                          );
    EfiCopyMem (Buffer, ptr, (1 << Width) * OldCount);
    gBS->FreePool (Buf);
  } else {
    Status = mCpuIo->Mem.Read (
                          mCpuIo,
                          (EFI_CPU_IO_PROTOCOL_WIDTH) OldWidth,
                          Address,
                          OldCount,
                          Buffer
                          );
  }

  return Status;
}

EFI_STATUS
EFIAPI
RootBridgeIoMemWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  )
/*++

Routine Description:
  Memory write
  
Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  PCI_ROOT_BRIDGE_INSTANCE              *PrivateData;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH OldWidth;
  UINTN                                 OldCount;
  UINTN                                 AlignMask;
  VOID                                  *Buf;
  VOID                                  *ptr;
  EFI_STATUS                            Status;

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width < 0 || Width >= EfiPciWidthMaximum) {
    return EFI_INVALID_PARAMETER;
  }

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS (This);

  //
  // Check memory access limit
  //
  if (Address < PrivateData->MemBase) {
    return EFI_INVALID_PARAMETER;
  }

  OldWidth  = Width;
  OldCount  = Count;
  if (Width >= EfiPciWidthFifoUint8 && Width <= EfiPciWidthFifoUint64) {
    Count = 1;
  }

  Width &= 0x03;

  if (Address + (1 << Width) * Count - 1 > PrivateData->MemLimit) {
    return EFI_INVALID_PARAMETER;
  }

  AlignMask = (1 << Width) - 1;
  if ((UINTN) Buffer & AlignMask) {
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    (1 << Width) * (OldCount + 1),
                    &Buf
                    );
    ptr = (VOID *) (((UINTN) (((UINTN) Buf) &~(AlignMask))) + (1 << Width));
    EfiCopyMem (ptr, Buffer, (1 << Width) * OldCount);
    Status = mCpuIo->Mem.Write (
                          mCpuIo,
                          (EFI_CPU_IO_PROTOCOL_WIDTH) OldWidth,
                          Address,
                          OldCount,
                          ptr
                          );
    gBS->FreePool (Buf);
  } else {
    Status = mCpuIo->Mem.Write (
                          mCpuIo,
                          (EFI_CPU_IO_PROTOCOL_WIDTH) OldWidth,
                          Address,
                          OldCount,
                          Buffer
                          );
  }

  return Status;
}

EFI_STATUS
EFIAPI
RootBridgeIoIoRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  )
/*++

Routine Description:
  Io read
  
Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{

  UINTN                                 AlignMask;
  PCI_ROOT_BRIDGE_INSTANCE              *PrivateData;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH OldWidth;
  UINTN                                 OldCount;
  UINTN                                 Counter;
  UINTN                                 InStride;
  UINTN                                 OutStride;
  CHAR8                                 Buf[16];
  CHAR8                                 *Ptr;
  CHAR8                                 *TempBuf;
  EFI_STATUS                            Status;

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width < 0 || Width >= EfiPciWidthMaximum) {
    return EFI_INVALID_PARAMETER;
  }

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS (This);

  AlignMask = (1 << (Width & 0x03)) - 1;

  //
  // check Io access limit
  //
  if (Address < PrivateData->IoBase) {
    return EFI_INVALID_PARAMETER;
  }

  OldWidth  = Width;
  OldCount  = Count;
  if (Width >= EfiPciWidthFifoUint8 && Width <= EfiPciWidthFifoUint64) {
    Count = 1;
  }

  Width &= 0x03;

  if (Address + (1 << Width) * Count - 1 >= PrivateData->IoLimit) {
    return EFI_INVALID_PARAMETER;
  }

  if (Address & AlignMask) {
    return EFI_INVALID_PARAMETER;
  }

  Counter = OldCount;
  TempBuf = (CHAR8 *) Buffer;
  Status  = EFI_SUCCESS;
  if ((UINTN) Buffer & AlignMask) {
    Ptr = (CHAR8 *) (((UINTN) (((UINTN) Buf) &~(AlignMask))) + (1 << Width));
    while (Counter > 0) {
      InStride  = (OldCount - Counter) << Width;
      OutStride = (OldCount - Counter) << Width;
      if (Width >= EfiCpuIoWidthFifoUint8 && Width <= EfiCpuIoWidthFifoUint64) {
        InStride = 0;
      }

      if (Width >= EfiCpuIoWidthFillUint8 && Width <= EfiCpuIoWidthFillUint64) {
        OutStride = 0;
      }

      Status = mCpuIo->Io.Read (
                            mCpuIo,
                            (EFI_CPU_IO_PROTOCOL_WIDTH) OldWidth,
                            Address + InStride,
                            1,
                            (VOID *) Ptr
                            );
      if (EFI_ERROR (Status)) {
        return Status;
      }

      EfiCopyMem (TempBuf + OutStride, Ptr, (1 << Width));
      Counter--;
    };

  } else {
    Status = mCpuIo->Io.Read (
                          mCpuIo,
                          (EFI_CPU_IO_PROTOCOL_WIDTH) OldWidth,
                          Address,
                          OldCount,
                          Buffer
                          );
  }

  return Status;

}

EFI_STATUS
EFIAPI
RootBridgeIoIoWrite (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL         *This,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH   Width,
  IN UINT64                                  Address,
  IN UINTN                                   Count,
  IN OUT VOID                                *Buffer
  )
/*++

Routine Description:
  Io write
  
Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  UINTN                                 AlignMask;
  PCI_ROOT_BRIDGE_INSTANCE              *PrivateData;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH OldWidth;
  UINTN                                 OldCount;
  UINTN                                 Counter;
  UINTN                                 InStride;
  UINTN                                 OutStride;
  CHAR8                                 Buf[16];
  CHAR8                                 *Ptr;
  CHAR8                                 *TempBuf;
  EFI_STATUS                            Status;

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width < 0 || Width >= EfiPciWidthMaximum) {
    return EFI_INVALID_PARAMETER;
  }

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS (This);

  AlignMask   = (1 << (Width & 0x03)) - 1;

  //
  // Check Io access limit
  //
  if (Address < PrivateData->IoBase) {
    return EFI_INVALID_PARAMETER;
  }

  OldWidth  = Width;
  OldCount  = Count;
  if (Width >= EfiPciWidthFifoUint8 && Width <= EfiPciWidthFifoUint64) {
    Count = 1;
  }

  Width &= 0x03;

  if (Address + (1 << Width) * Count - 1 >= PrivateData->IoLimit) {
    return EFI_INVALID_PARAMETER;
  }

  if (Address & AlignMask) {
    return EFI_INVALID_PARAMETER;
  }

  Counter = OldCount;
  TempBuf = (CHAR8 *) Buffer;
  Status  = EFI_SUCCESS;
  if ((UINTN) Buffer & AlignMask) {
    Ptr = (CHAR8 *) (((UINTN) (((UINTN) Buf) &~(AlignMask))) + (1 << Width));
    while (Counter > 0) {
      InStride  = (OldCount - Counter) << Width;
      OutStride = (OldCount - Counter) << Width;
      if (Width >= EfiCpuIoWidthFifoUint8 && Width <= EfiCpuIoWidthFifoUint64) {
        InStride = 0;
      }

      if (Width >= EfiCpuIoWidthFillUint8 && Width <= EfiCpuIoWidthFillUint64) {
        OutStride = 0;
      }

      EfiCopyMem (Ptr, TempBuf + OutStride, (1 << Width));
      Status = mCpuIo->Io.Write (
                            mCpuIo,
                            (EFI_CPU_IO_PROTOCOL_WIDTH) OldWidth,
                            Address + InStride,
                            1,
                            (VOID *) Ptr
                            );
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Counter--;
    }

  } else {
    Status = mCpuIo->Io.Write (
                          mCpuIo,
                          (EFI_CPU_IO_PROTOCOL_WIDTH) OldWidth,
                          Address,
                          OldCount,
                          Buffer
                          );
  }

  return Status;

}

EFI_STATUS
EFIAPI
RootBridgeIoCopyMem (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL          *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH    Width,
  IN     UINT64                                   DestAddress,
  IN     UINT64                                   SrcAddress,
  IN     UINTN                                    Count
  )
/*++

Routine Description:
  Memory copy
  
Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    DestAddress - add argument and description to function comment
// TODO:    SrcAddress - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS  Status;
  BOOLEAN     Direction;
  UINTN       Stride;
  UINTN       Index;
  UINT8       Buffer[16];
  UINT8       *Result;

  if (Width < 0 || Width > EfiPciWidthUint64) {
    return EFI_INVALID_PARAMETER;
  }

  if (DestAddress == SrcAddress) {
    return EFI_SUCCESS;
  }

  //
  // Align result buffer
  //
  Result    = (UINT8 *) ((UINTN) (Buffer + 8) & (UINTN) (-8));

  Stride    = 1 << Width;

  Direction = TRUE;
  if ((DestAddress > SrcAddress) && (DestAddress < (SrcAddress + Count * Stride))) {
    Direction   = FALSE;
    SrcAddress  = SrcAddress + (Count - 1) * Stride;
    DestAddress = DestAddress + (Count - 1) * Stride;
  }

  for (Index = 0; Index < Count; Index++) {
    Status = RootBridgeIoMemRead (
              This,
              Width,
              SrcAddress,
              1,
              Result
              );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = RootBridgeIoMemWrite (
              This,
              Width,
              DestAddress,
              1,
              Result
              );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (Direction) {
      SrcAddress += Stride;
      DestAddress += Stride;
    } else {
      SrcAddress -= Stride;
      DestAddress -= Stride;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RootBridgeIoPciRead (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN UINT64                                 Address,
  IN UINTN                                  Count,
  IN OUT VOID                               *Buffer
  )
/*++

Routine Description:
  Pci read
  
Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width < 0 || Width >= EfiPciWidthMaximum) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Read Pci configuration space
  //
  return RootBridgeIoPciRW (This, FALSE, Width, Address, Count, Buffer);
}

EFI_STATUS
EFIAPI
RootBridgeIoPciWrite (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN UINT64                                 Address,
  IN UINTN                                  Count,
  IN OUT VOID                               *Buffer
  )
/*++

Routine Description:
  Pci write
  
Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width < 0 || Width >= EfiPciWidthMaximum) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Write Pci configuration space
  //
  return RootBridgeIoPciRW (This, TRUE, Width, Address, Count, Buffer);
}

EFI_STATUS
EFIAPI
RootBridgeIoMap (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL            *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION  Operation,
  IN     VOID                                       *HostAddress,
  IN OUT UINTN                                      *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS                       *DeviceAddress,
  OUT    VOID                                       **Mapping
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This          - TODO: add argument description
  Operation     - TODO: add argument description
  HostAddress   - TODO: add argument description
  NumberOfBytes - TODO: add argument description
  DeviceAddress - TODO: add argument description
  Mapping       - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_UNSUPPORTED - TODO: Add description for return value
  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;
  MAP_INFO              *MapInfo;

  if (HostAddress == NULL || NumberOfBytes == NULL || DeviceAddress == NULL || Mapping == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Initialize the return values to their defaults
  //
  *Mapping = NULL;

  //
  // Make sure that Operation is valid
  //
  if (Operation < 0 || Operation >= EfiPciOperationMaximum) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Most PCAT like chipsets can not handle performing DMA above 4GB.
  // If any part of the DMA transfer being mapped is above 4GB, then
  // map the DMA transfer to a buffer below 4GB.
  //
  PhysicalAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) HostAddress;
  if ((PhysicalAddress +*NumberOfBytes) > 0x100000000) {

    //
    // Common Buffer operations can not be remapped.  If the common buffer
    // if above 4GB, then it is not possible to generate a mapping, so return
    // an error.
    //
    if (Operation == EfiPciOperationBusMasterCommonBuffer || Operation == EfiPciOperationBusMasterCommonBuffer64) {
      return EFI_UNSUPPORTED;
    }

    //
    // Allocate a MAP_INFO structure to remember the mapping when Unmap() is
    // called later.
    //
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    sizeof (MAP_INFO),
                    &MapInfo
                    );
    if (EFI_ERROR (Status)) {
      *NumberOfBytes = 0;
      return Status;
    }

    //
    // Return a pointer to the MAP_INFO structure in Mapping
    //
    *Mapping = MapInfo;

    //
    // Initialize the MAP_INFO structure
    //
    MapInfo->Operation          = Operation;
    MapInfo->NumberOfBytes      = *NumberOfBytes;
    MapInfo->NumberOfPages      = EFI_SIZE_TO_PAGES (*NumberOfBytes);
    MapInfo->HostAddress        = PhysicalAddress;
    MapInfo->MappedHostAddress  = 0x00000000ffffffff;

    //
    // Allocate a buffer below 4GB to map the transfer to.
    //
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiBootServicesData,
                    MapInfo->NumberOfPages,
                    &MapInfo->MappedHostAddress
                    );
    if (EFI_ERROR (Status)) {
      gBS->FreePool (MapInfo);
      *NumberOfBytes = 0;
      return Status;
    }

    //
    // If this is a read operation from the Bus Master's point of view,
    // then copy the contents of the real buffer into the mapped buffer
    // so the Bus Master can read the contents of the real buffer.
    //
    if (Operation == EfiPciOperationBusMasterRead || Operation == EfiPciOperationBusMasterRead64) {
      EfiCopyMem (
        (VOID *) (UINTN) MapInfo->MappedHostAddress,
        (VOID *) (UINTN) MapInfo->HostAddress,
        MapInfo->NumberOfBytes
        );
    }

    //
    // The DeviceAddress is the address of the maped buffer below 4GB
    //
    *DeviceAddress = MapInfo->MappedHostAddress;
  } else {
    //
    // The transfer is below 4GB, so the DeviceAddress is simply the HostAddress
    //
    *DeviceAddress = PhysicalAddress;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RootBridgeIoUnmap (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN VOID                             *Mapping
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This    - TODO: add argument description
  Mapping - TODO: add argument description

Returns:

  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  MAP_INFO  *MapInfo;

  //
  // See if the Map() operation associated with this Unmap() required a mapping buffer.
  // If a mapping buffer was not required, then this function simply returns EFI_SUCCESS.
  //
  if (Mapping != NULL) {
    //
    // Get the MAP_INFO structure from Mapping
    //
    MapInfo = (MAP_INFO *) Mapping;

    //
    // If this is a write operation from the Bus Master's point of view,
    // then copy the contents of the mapped buffer into the real buffer
    // so the processor can read the contents of the real buffer.
    //
    if (MapInfo->Operation == EfiPciOperationBusMasterWrite || MapInfo->Operation == EfiPciOperationBusMasterWrite64) {
      EfiCopyMem (
        (VOID *) (UINTN) MapInfo->HostAddress,
        (VOID *) (UINTN) MapInfo->MappedHostAddress,
        MapInfo->NumberOfBytes
        );
    }

    //
    // Free the mapped buffer and the MAP_INFO structure.
    //
    gBS->FreePages (MapInfo->MappedHostAddress, MapInfo->NumberOfPages);
    gBS->FreePool (Mapping);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RootBridgeIoAllocateBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  EFI_ALLOCATE_TYPE                Type,
  IN  EFI_MEMORY_TYPE                  MemoryType,
  IN  UINTN                            Pages,
  OUT VOID                             **HostAddress,
  IN  UINT64                           Attributes
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  Type        - TODO: add argument description
  MemoryType  - TODO: add argument description
  Pages       - TODO: add argument description
  HostAddress - TODO: add argument description
  Attributes  - TODO: add argument description

Returns:

  EFI_UNSUPPORTED - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_UNSUPPORTED - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;

  //
  // Validate Attributes
  //
  if (Attributes & EFI_PCI_ATTRIBUTE_INVALID_FOR_ALLOCATE_BUFFER) {
    return EFI_UNSUPPORTED;
  }

  //
  // Check for invalid inputs
  //
  if (HostAddress == NULL) {
    return EFI_INVALID_PARAMETER;
  }
 
  //
  // The only valid memory types are EfiBootServicesData and EfiRuntimeServicesData
  //
  if (MemoryType != EfiBootServicesData && MemoryType != EfiRuntimeServicesData) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Limit allocations to memory below 4GB
  //
  PhysicalAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) (0xffffffff);

  Status          = gBS->AllocatePages (AllocateMaxAddress, MemoryType, Pages, &PhysicalAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *HostAddress = (VOID *) (UINTN) PhysicalAddress;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RootBridgeIoFreeBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  UINTN                            Pages,
  OUT VOID                             *HostAddress
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  Pages       - TODO: add argument description
  HostAddress - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  return gBS->FreePages ((EFI_PHYSICAL_ADDRESS) (UINTN) HostAddress, Pages);
}

EFI_STATUS
EFIAPI
RootBridgeIoFlush (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL           *This
  )
/*++

Routine Description:

Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  //
  // not supported yet
  //
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RootBridgeIoGetAttributes (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT UINT64                           *Supported,
  OUT UINT64                           *Attributes
  )
/*++

Routine Description:

Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Supported - add argument and description to function comment
// TODO:    Attributes - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  PCI_ROOT_BRIDGE_INSTANCE  *PrivateData;

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS (This);

  if (Attributes == NULL && Supported == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Set the return value for Supported and Attributes
  //
  if (Supported) {
    *Supported = PrivateData->Supports;
  }

  if (Attributes) {
    *Attributes = PrivateData->Attributes;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RootBridgeIoSetAttributes (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN     UINT64                           Attributes,
  IN OUT UINT64                           *ResourceBase,
  IN OUT UINT64                           *ResourceLength
  )
/*++

Routine Description:

Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Attributes - add argument and description to function comment
// TODO:    ResourceBase - add argument and description to function comment
// TODO:    ResourceLength - add argument and description to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  PCI_ROOT_BRIDGE_INSTANCE  *PrivateData;
  UINT64                    Address;
  UINT16                    Data;

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS (This);

  if (Attributes) {
    if ((Attributes & (~(PrivateData->Supports))) != 0) {
      return EFI_UNSUPPORTED;
    }
  }

  if (Attributes == PrivateData->Attributes) {
    return EFI_SUCCESS;
  }
  
  //
  // Set IDE primary
  //
  if (Attributes & EFI_PCI_ATTRIBUTE_IDE_PRIMARY_IO) {

    Address = EFI_PCI_ADDRESS (0x0, 0x1f, 0x1, 0x40);

    This->Pci.Read (This, EfiPciWidthUint16, Address, 1, &Data);
    Data |= 0x8000;
    This->Pci.Write (This, EfiPciWidthUint16, Address, 1, &Data);
  }
  
  //
  // Set IDE secondary
  //
  if (Attributes & EFI_PCI_ATTRIBUTE_IDE_SECONDARY_IO) {

    Address = EFI_PCI_ADDRESS (0x0, 0x1f, 0x1, 0x42);

    This->Pci.Read (This, EfiPciWidthUint16, Address, 1, &Data);
    Data |= 0x8000;
    This->Pci.Write (This, EfiPciWidthUint16, Address, 1, &Data);
  }
  
  //
  // It is just a trick for some attribute can only be enabled or disabled
  // otherwise it can impact on other devices
  //
  PrivateData->Attributes = Attributes;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RootBridgeIoConfiguration (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT    VOID                             **Resources
  )
/*++

Routine Description:

Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Resources - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  PCI_ROOT_BRIDGE_INSTANCE  *PrivateData;
  UINTN                     Index;

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS (This);

  for (Index = 0; Index < TypeMax; Index++) {
    if (PrivateData->ResAllocNode[Index].Status == ResAllocated) {
      mConfiguration.SpaceDesp[Index].AddrRangeMin = PrivateData->ResAllocNode[Index].Base;
      mConfiguration.SpaceDesp[Index].AddrRangeMax =
      PrivateData->ResAllocNode[Index].Base + PrivateData->ResAllocNode[Index].Length - 1;
      mConfiguration.SpaceDesp[Index].AddrLen = PrivateData->ResAllocNode[Index].Length;
    }
  }

  *Resources = &mConfiguration;
  return EFI_SUCCESS;
}

//
// Internal function
//

STATIC
EFI_STATUS
RootBridgeIoPciRW (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN BOOLEAN                                Write,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN UINT64                                 UserAddress,
  IN UINTN                                  Count,
  IN OUT VOID                               *UserBuffer
  )
/*++

Routine Description:

Arguments:
    
Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Write - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    UserAddress - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    UserBuffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  PCI_CONFIG_ACCESS_CF8     Pci;
  PCI_CONFIG_ACCESS_CF8     PciAligned;
  UINT32                    InStride;
  UINT32                    OutStride;
  UINTN                     PciData;
  UINTN                     PciDataStride;
  PCI_ROOT_BRIDGE_INSTANCE  *PrivateData;

  if ((Width & 0x03) >= EfiPciWidthUint64) {
    return EFI_INVALID_PARAMETER;
  }

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS (This);

  InStride    = 1 << (Width & 0x03);
  OutStride   = InStride;
  if (Width >= EfiCpuIoWidthFifoUint8 && Width <= EfiCpuIoWidthFifoUint64) {
    InStride = 0;
  }

  if (Width >= EfiCpuIoWidthFillUint8 && Width <= EfiCpuIoWidthFillUint64) {
    OutStride = 0;
  }

  if (((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS *) &UserAddress)->ExtendedRegister != 0) {
    //
    // For Pci bus driver has made parameter check before calling this function,
    // there is no need to check the parameters again here
    //
    return EFI_UNSUPPORTED;
  } else {
    Pci.Reg = ((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS *) &UserAddress)->Register;
  }

  Pci.Func      = ((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS *) &UserAddress)->Function;
  Pci.Dev       = ((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS *) &UserAddress)->Device;
  Pci.Bus       = ((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS *) &UserAddress)->Bus;
  Pci.Reserved  = 0;
  Pci.Enable    = 1;

  //
  // PCI Configure access are all 32-bit aligned, but by accessing the
  //  CONFIG_DATA_REGISTER (0xcfc) with different widths more cycle types
  //  are possible on PCI.
  //
  // To read a byte of PCI configuration space you load 0xcf8 and
  //  read 0xcfc, 0xcfd, 0xcfe, 0xcff
  //
  PciDataStride = ((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS *) &UserAddress)->Register & 0x03;

  while (Count) {
    PciAligned = Pci;
    PciAligned.Reg &= 0xfc;
    PciData = PrivateData->PciData + PciDataStride;
    EfiAcquireLock (&PrivateData->PciLock);
    This->Io.Write (
              This,
              EfiPciWidthUint32,
              PrivateData->PciAddress,
              1,
              &PciAligned
              );
    if (Write) {
      This->Io.Write (This, Width, PciData, 1, UserBuffer);
    } else {
      This->Io.Read (This, Width, PciData, 1, UserBuffer);
    }

    EfiReleaseLock (&PrivateData->PciLock);
    UserBuffer    = ((UINT8 *) UserBuffer) + OutStride;
    PciDataStride = (PciDataStride + InStride) % 4;
    Pci.Reg += InStride;
    Count -= 1;
  }

  return EFI_SUCCESS;
}
