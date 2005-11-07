/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DeviceIo.c
  
Abstract:

--*/

#include "PciHostBridge.h"
#include "DeviceIo.h"
#include "pci22.h"

typedef struct {
  EFI_DEVICE_PATH_PROTOCOL  DeviceIoRootPath;
  EFI_DEVICE_PATH_PROTOCOL  EndDevicePath;
} EFI_DEV_IO_DEFAULT_DEVICE_PATH;

static EFI_DEV_IO_DEFAULT_DEVICE_PATH mEndDevicePath = {
  END_DEVICE_PATH_TYPE,
  END_ENTIRE_DEVICE_PATH_SUBTYPE,
  END_DEVICE_PATH_LENGTH,
  0
};

//
// Prototypes
//

//
// Device I/O Protocol Interface
//

EFI_STATUS
EFIAPI
DeviceIoMemRead (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  );

EFI_STATUS
EFIAPI
DeviceIoMemWrite (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  );

EFI_STATUS
EFIAPI
DeviceIoIoRead (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  );

EFI_STATUS
EFIAPI
DeviceIoIoWrite (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  );

EFI_STATUS
EFIAPI
DeviceIoPciRead (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  );

EFI_STATUS
EFIAPI
DeviceIoPciWrite (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  );

EFI_STATUS
EFIAPI
DeviceIoPciDevicePath (
  IN EFI_DEVICE_IO_PROTOCOL        *This,
  IN UINT64                        Address,
  IN OUT EFI_DEVICE_PATH_PROTOCOL  **PciDevicePath
  );

EFI_STATUS
EFIAPI
DeviceIoMap (
  IN EFI_DEVICE_IO_PROTOCOL   *This,
  IN EFI_IO_OPERATION_TYPE    Operation,
  IN EFI_PHYSICAL_ADDRESS     *HostAddress,
  IN OUT UINTN                *NumberOfBytes,
  OUT EFI_PHYSICAL_ADDRESS    *DeviceAddress,
  OUT VOID                    **Mapping
  );

EFI_STATUS
EFIAPI
DeviceIoUnmap (
  IN EFI_DEVICE_IO_PROTOCOL   *This,
  IN VOID                     *Mapping
  );

EFI_STATUS
EFIAPI
DeviceIoAllocateBuffer (
  IN EFI_DEVICE_IO_PROTOCOL    *This,
  IN EFI_ALLOCATE_TYPE         Type,
  IN EFI_MEMORY_TYPE           MemoryType,
  IN UINTN                     Pages,
  IN OUT EFI_PHYSICAL_ADDRESS  *HostAddress
  );

EFI_STATUS
EFIAPI
DeviceIoFlush (
  IN EFI_DEVICE_IO_PROTOCOL  *This
  );

EFI_STATUS
EFIAPI
DeviceIoFreeBuffer (
  IN EFI_DEVICE_IO_PROTOCOL   *This,
  IN UINTN                    Pages,
  IN EFI_PHYSICAL_ADDRESS     HostAddress
  );

EFI_STATUS
DeviceIoConstructor (
  VOID
  )
/*++

  Routine Description:
    Initialize and install a Device IO protocol on a empty device path handle.

  Arguments:

  Returns:
    EFI_SUCCESS         - This driver is added to ControllerHandle.
    EFI_ALREADY_STARTED - This driver is already running on ControllerHandle.
    other               - This driver does not support this device.

--*/
// TODO:    EFI_NOT_FOUND - add return value to function comment
{
  EFI_STATUS                      Status;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  DEVICE_IO_PRIVATE_DATA          *Private;
  EFI_HANDLE                      Handle;
  EFI_HANDLE                      *HandleBuffer;
  UINTN                           HandleCount;

  Handle  = NULL;
  Private = NULL;

  //
  // Locate PciRootBridge IO protocol.
  //
  Status = gBS->LocateProtocol (&gEfiPciRootBridgeIoProtocolGuid, NULL, &PciRootBridgeIo);
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciRootBridgeIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status) || HandleCount == 0) {
    return EFI_NOT_FOUND;
  }

  Status = gBS->HandleProtocol (
                  HandleBuffer[0],
                  &gEfiDevicePathProtocolGuid,
                  (VOID *) (&DevicePath)
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Initialize the Device IO device instance.
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (DEVICE_IO_PRIVATE_DATA),
                  (VOID **) &Private
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  EfiZeroMem (Private, sizeof (DEVICE_IO_PRIVATE_DATA));

  Private->Signature                = DEVICE_IO_PRIVATE_DATA_SIGNATURE;
  Private->PciRootBridgeIo          = PciRootBridgeIo;
  Private->DevicePath               = DevicePath;
  Private->PrimaryBus               = 0;
  Private->SubordinateBus           = 255;

  Private->DeviceIo.Mem.Read        = DeviceIoMemRead;
  Private->DeviceIo.Mem.Write       = DeviceIoMemWrite;
  Private->DeviceIo.Io.Read         = DeviceIoIoRead;
  Private->DeviceIo.Io.Write        = DeviceIoIoWrite;
  Private->DeviceIo.Pci.Read        = DeviceIoPciRead;
  Private->DeviceIo.Pci.Write       = DeviceIoPciWrite;
  Private->DeviceIo.PciDevicePath   = DeviceIoPciDevicePath;
  Private->DeviceIo.Map             = DeviceIoMap;
  Private->DeviceIo.Unmap           = DeviceIoUnmap;
  Private->DeviceIo.AllocateBuffer  = DeviceIoAllocateBuffer;
  Private->DeviceIo.Flush           = DeviceIoFlush;
  Private->DeviceIo.FreeBuffer      = DeviceIoFreeBuffer;

  //
  // Install protocol interfaces for the Device IO device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiDeviceIoProtocolGuid,
                  &Private->DeviceIo,
                  &gEfiDevicePathProtocolGuid,
                  &mEndDevicePath,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (Private);
    return Status;
  }

  return Status;
}

EFI_STATUS
EFIAPI
DeviceIoMemRead (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  EFI_STATUS              Status;
  DEVICE_IO_PRIVATE_DATA  *Private;

  Private = DEVICE_IO_PRIVATE_DATA_FROM_THIS (This);

  if (Width > MMIO_COPY_UINT64) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width >= MMIO_COPY_UINT8) {
    Width = Width - MMIO_COPY_UINT8;
    Status = Private->PciRootBridgeIo->CopyMem (
                                        Private->PciRootBridgeIo,
                                        (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH) Width,
                                        (UINT64) Buffer,
                                        Address,
                                        Count
                                        );
  } else {
    Status = Private->PciRootBridgeIo->Mem.Read (
                                            Private->PciRootBridgeIo,
                                            (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH) Width,
                                            Address,
                                            Count,
                                            Buffer
                                            );
  }

  return Status;
}

EFI_STATUS
EFIAPI
DeviceIoMemWrite (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  EFI_STATUS              Status;
  DEVICE_IO_PRIVATE_DATA  *Private;

  Private = DEVICE_IO_PRIVATE_DATA_FROM_THIS (This);

  if (Width > MMIO_COPY_UINT64) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width >= MMIO_COPY_UINT8) {
    Width = Width - MMIO_COPY_UINT8;
    Status = Private->PciRootBridgeIo->CopyMem (
                                        Private->PciRootBridgeIo,
                                        (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH) Width,
                                        Address,
                                        (UINT64) Buffer,
                                        Count
                                        );
  } else {
    Status = Private->PciRootBridgeIo->Mem.Write (
                                            Private->PciRootBridgeIo,
                                            (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH) Width,
                                            Address,
                                            Count,
                                            Buffer
                                            );
  }

  return Status;
}

EFI_STATUS
EFIAPI
DeviceIoIoRead (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  EFI_STATUS              Status;
  DEVICE_IO_PRIVATE_DATA  *Private;

  Private = DEVICE_IO_PRIVATE_DATA_FROM_THIS (This);

  if (Width >= MMIO_COPY_UINT8) {
    return EFI_INVALID_PARAMETER;
  }

  Status = Private->PciRootBridgeIo->Io.Read (
                                          Private->PciRootBridgeIo,
                                          (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH) Width,
                                          Address,
                                          Count,
                                          Buffer
                                          );

  return Status;
}

EFI_STATUS
EFIAPI
DeviceIoIoWrite (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  EFI_STATUS              Status;
  DEVICE_IO_PRIVATE_DATA  *Private;

  Private = DEVICE_IO_PRIVATE_DATA_FROM_THIS (This);

  if (Width >= MMIO_COPY_UINT8) {
    return EFI_INVALID_PARAMETER;
  }

  Status = Private->PciRootBridgeIo->Io.Write (
                                          Private->PciRootBridgeIo,
                                          (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH) Width,
                                          Address,
                                          Count,
                                          Buffer
                                          );

  return Status;
}

EFI_STATUS
EFIAPI
DeviceIoPciRead (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  EFI_STATUS              Status;
  DEVICE_IO_PRIVATE_DATA  *Private;

  Private = DEVICE_IO_PRIVATE_DATA_FROM_THIS (This);

  if (Width < 0 || Width >= MMIO_COPY_UINT8) {
    return EFI_INVALID_PARAMETER;
  }

  Status = Private->PciRootBridgeIo->Pci.Read (
                                          Private->PciRootBridgeIo,
                                          (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH) Width,
                                          Address,
                                          Count,
                                          Buffer
                                          );

  return Status;
}

EFI_STATUS
EFIAPI
DeviceIoPciWrite (
  IN EFI_DEVICE_IO_PROTOCOL *This,
  IN EFI_IO_WIDTH           Width,
  IN UINT64                 Address,
  IN UINTN                  Count,
  IN OUT VOID               *Buffer
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Width - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    Count - add argument and description to function comment
// TODO:    Buffer - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  EFI_STATUS              Status;
  DEVICE_IO_PRIVATE_DATA  *Private;

  Private = DEVICE_IO_PRIVATE_DATA_FROM_THIS (This);

  if (Width < 0 || Width >= MMIO_COPY_UINT8) {
    return EFI_INVALID_PARAMETER;
  }

  Status = Private->PciRootBridgeIo->Pci.Write (
                                          Private->PciRootBridgeIo,
                                          (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH) Width,
                                          Address,
                                          Count,
                                          Buffer
                                          );

  return Status;
}

EFI_DEVICE_PATH_PROTOCOL *
AppendPciDevicePath (
  IN     DEVICE_IO_PRIVATE_DATA    *Private,
  IN     UINT8                     Bus,
  IN     UINT8                     Device,
  IN     UINT8                     Function,
  IN     EFI_DEVICE_PATH_PROTOCOL  *DevicePath,
  IN OUT UINT16                    *BridgePrimaryBus,
  IN OUT UINT16                    *BridgeSubordinateBus
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    Private - add argument and description to function comment
// TODO:    Bus - add argument and description to function comment
// TODO:    Device - add argument and description to function comment
// TODO:    Function - add argument and description to function comment
// TODO:    DevicePath - add argument and description to function comment
// TODO:    BridgePrimaryBus - add argument and description to function comment
// TODO:    BridgeSubordinateBus - add argument and description to function comment
{
  UINT16                    ThisBus;
  UINT8                     ThisDevice;
  UINT8                     ThisFunc;
  UINT64                    Address;
  PCI_TYPE01                PciBridge;
  PCI_TYPE01                *PciPtr;
  EFI_DEVICE_PATH_PROTOCOL  *ReturnDevicePath;
  PCI_DEVICE_PATH           PciNode;

  PciPtr = &PciBridge;
  for (ThisBus = *BridgePrimaryBus; ThisBus <= *BridgeSubordinateBus; ThisBus++) {
    for (ThisDevice = 0; ThisDevice <= PCI_MAX_DEVICE; ThisDevice++) {
      for (ThisFunc = 0; ThisFunc <= PCI_MAX_FUNC; ThisFunc++) {
        Address = EFI_PCI_ADDRESS (ThisBus, ThisDevice, ThisFunc, 0);
        EfiZeroMem (PciPtr, sizeof (PCI_TYPE01));
        Private->DeviceIo.Pci.Read (
                                &Private->DeviceIo,
                                IO_UINT32,
                                Address,
                                1,
                                &(PciPtr->Hdr.VendorId)
                                );
        if (PciPtr->Hdr.VendorId == 0xffff) {
          break;
        } else {
          Private->DeviceIo.Pci.Read (
                                  &Private->DeviceIo,
                                  IO_UINT32,
                                  Address,
                                  sizeof (PCI_TYPE01) / sizeof (UINT32),
                                  PciPtr
                                  );
          if (IS_PCI_BRIDGE (PciPtr)) {
            if (Bus >= PciPtr->Bridge.SecondaryBus && 
                Bus <= PciPtr->Bridge.SubordinateBus) {

              PciNode.Header.Type     = HARDWARE_DEVICE_PATH;
              PciNode.Header.SubType  = HW_PCI_DP;
              SetDevicePathNodeLength (&PciNode.Header, sizeof (PciNode));

              PciNode.Device        = ThisDevice;
              PciNode.Function      = ThisFunc;
              ReturnDevicePath      = EfiAppendDevicePathNode (DevicePath, &PciNode.Header);

              *BridgePrimaryBus     = PciPtr->Bridge.SecondaryBus;
              *BridgeSubordinateBus = PciPtr->Bridge.SubordinateBus;
              return ReturnDevicePath;
            }
          }

          if (ThisFunc == 0 && !(PciPtr->Hdr.HeaderType & HEADER_TYPE_MULTI_FUNCTION)) {
            //
            // Skip sub functions, this is not a multi function device
            //
            ThisFunc = 8;
          }
        }
      }
    }
  }

  EfiZeroMem (&PciNode, sizeof (PciNode));
  PciNode.Header.Type     = HARDWARE_DEVICE_PATH;
  PciNode.Header.SubType  = HW_PCI_DP;
  SetDevicePathNodeLength (&PciNode.Header, sizeof (PciNode));
  PciNode.Device        = Device;
  PciNode.Function      = Function;

  ReturnDevicePath      = EfiAppendDevicePathNode (DevicePath, &PciNode.Header);

  *BridgePrimaryBus     = 0xffff;
  *BridgeSubordinateBus = 0xffff;
  return ReturnDevicePath;
}

EFI_STATUS
EFIAPI
DeviceIoPciDevicePath (
  IN EFI_DEVICE_IO_PROTOCOL        *This,
  IN UINT64                        Address,
  IN OUT EFI_DEVICE_PATH_PROTOCOL  **PciDevicePath
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Address - add argument and description to function comment
// TODO:    PciDevicePath - add argument and description to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
// TODO:    EFI_OUT_OF_RESOURCES - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  DEVICE_IO_PRIVATE_DATA  *Private;
  UINT16                  PrimaryBus;
  UINT16                  SubordinateBus;
  UINT8                   Bus;
  UINT8                   Device;
  UINT8                   Func;

  Private = DEVICE_IO_PRIVATE_DATA_FROM_THIS (This);

  Bus     = (UINT8) (((UINT32) Address >> 24) & 0xff);
  Device  = (UINT8) (((UINT32) Address >> 16) & 0xff);
  Func    = (UINT8) (((UINT32) Address >> 8) & 0xff);

  if (Bus < Private->PrimaryBus || Bus > Private->SubordinateBus) {
    return EFI_UNSUPPORTED;
  }

  *PciDevicePath  = Private->DevicePath;
  PrimaryBus      = Private->PrimaryBus;
  SubordinateBus  = Private->SubordinateBus;
  do {
    *PciDevicePath = AppendPciDevicePath (
                      Private,
                      Bus,
                      Device,
                      Func,
                      *PciDevicePath,
                      &PrimaryBus,
                      &SubordinateBus
                      );
    if (*PciDevicePath == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  } while (PrimaryBus != 0xffff);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DeviceIoMap (
  IN EFI_DEVICE_IO_PROTOCOL   *This,
  IN EFI_IO_OPERATION_TYPE    Operation,
  IN EFI_PHYSICAL_ADDRESS     *HostAddress,
  IN OUT UINTN                *NumberOfBytes,
  OUT EFI_PHYSICAL_ADDRESS    *DeviceAddress,
  OUT VOID                    **Mapping
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Operation - add argument and description to function comment
// TODO:    HostAddress - add argument and description to function comment
// TODO:    NumberOfBytes - add argument and description to function comment
// TODO:    DeviceAddress - add argument and description to function comment
// TODO:    Mapping - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
{
  EFI_STATUS              Status;
  DEVICE_IO_PRIVATE_DATA  *Private;

  Private = DEVICE_IO_PRIVATE_DATA_FROM_THIS (This);

  if (Operation < 0 || Operation > EfiBusMasterCommonBuffer) {
    return EFI_INVALID_PARAMETER;
  }

  if (((UINTN) (*HostAddress) != (*HostAddress)) && Operation == EfiBusMasterCommonBuffer) {
    return EFI_UNSUPPORTED;
  }

  Status = Private->PciRootBridgeIo->Map (
                                      Private->PciRootBridgeIo,
                                      (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION) Operation,
                                      (VOID *) (UINTN) (*HostAddress),
                                      NumberOfBytes,
                                      DeviceAddress,
                                      Mapping
                                      );

  return Status;
}

EFI_STATUS
EFIAPI
DeviceIoUnmap (
  IN EFI_DEVICE_IO_PROTOCOL   *This,
  IN VOID                     *Mapping
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Mapping - add argument and description to function comment
{
  EFI_STATUS              Status;
  DEVICE_IO_PRIVATE_DATA  *Private;

  Private = DEVICE_IO_PRIVATE_DATA_FROM_THIS (This);

  Status = Private->PciRootBridgeIo->Unmap (
                                      Private->PciRootBridgeIo,
                                      Mapping
                                      );

  return Status;
}

EFI_STATUS
EFIAPI
DeviceIoAllocateBuffer (
  IN EFI_DEVICE_IO_PROTOCOL    *This,
  IN EFI_ALLOCATE_TYPE         Type,
  IN EFI_MEMORY_TYPE           MemoryType,
  IN UINTN                     Pages,
  IN OUT EFI_PHYSICAL_ADDRESS  *PhysicalAddress
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Type - add argument and description to function comment
// TODO:    MemoryType - add argument and description to function comment
// TODO:    Pages - add argument and description to function comment
// TODO:    PhysicalAddress - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  HostAddress;

  HostAddress = *PhysicalAddress;

  if (MemoryType != EfiBootServicesData && MemoryType != EfiRuntimeServicesData) {
    return EFI_INVALID_PARAMETER;
  }

  if (Type >= MaxAllocateType || Type < AllocateAnyPages) {
    return EFI_UNSUPPORTED;
  }

  if ((Type == AllocateAddress) &&
      (HostAddress + EFI_PAGES_TO_SIZE (Pages) - 1 > MAX_COMMON_BUFFER)) {
    return EFI_UNSUPPORTED;
  }

  if (AllocateAnyPages == Type || 
      (AllocateMaxAddress == Type && HostAddress > MAX_COMMON_BUFFER)) {
    Type        = AllocateMaxAddress;
    HostAddress = MAX_COMMON_BUFFER;
  }

  Status = gBS->AllocatePages (Type, MemoryType, Pages, &HostAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *PhysicalAddress = HostAddress;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DeviceIoFlush (
  IN EFI_DEVICE_IO_PROTOCOL  *This
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
{
  EFI_STATUS              Status;
  DEVICE_IO_PRIVATE_DATA  *Private;

  Private = DEVICE_IO_PRIVATE_DATA_FROM_THIS (This);

  Status  = Private->PciRootBridgeIo->Flush (Private->PciRootBridgeIo);

  return Status;
}

EFI_STATUS
EFIAPI
DeviceIoFreeBuffer (
  IN EFI_DEVICE_IO_PROTOCOL   *This,
  IN UINTN                    Pages,
  IN EFI_PHYSICAL_ADDRESS     HostAddress
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Pages - add argument and description to function comment
// TODO:    HostAddress - add argument and description to function comment
// TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  if (((HostAddress & EFI_PAGE_MASK) != 0) || (Pages <= 0)) {
    return EFI_INVALID_PARAMETER;
  }

  return gBS->FreePages (HostAddress, Pages);
}
