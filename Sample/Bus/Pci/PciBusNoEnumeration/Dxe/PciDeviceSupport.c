/*++

Copyright (c) 2005 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciDeviceSupport.c
  
Abstract:

  This file provides routine to support Pci device node manipulation

Revision History

--*/

#include "Pcibus.h"
#include "PciDeviceSupport.h"

//
// This device structure is serviced as a header.
// Its Next field points to the first root bridge device node
//
EFI_LIST_ENTRY  gPciDevicePool;

EFI_STATUS
InitializePciDevicePool (
  VOID
  )
/*++

Routine Description:

  Initialize the gPciDevicePool

Arguments:

Returns:

  None

--*/
{
  InitializeListHead (&gPciDevicePool);

  return EFI_SUCCESS;
}

EFI_STATUS
InsertRootBridge (
  PCI_IO_DEVICE *RootBridge
  )
/*++

Routine Description:

  Insert a root bridge into PCI device pool

Arguments:

  RootBridge    - A pointer to the PCI_IO_DEVICE.

Returns:

  None

--*/
{
  InsertTailList (&gPciDevicePool, &(RootBridge->Link));

  return EFI_SUCCESS;
}

EFI_STATUS
InsertPciDevice (
  PCI_IO_DEVICE *Bridge,
  PCI_IO_DEVICE *PciDeviceNode
  )
/*++

Routine Description:

  This function is used to insert a PCI device node under
  a bridge

Arguments:
  Bridge        - A pointer to the PCI_IO_DEVICE.
  PciDeviceNode - A pointer to the PCI_IO_DEVICE.

Returns:

  None

--*/

{

  InsertTailList (&Bridge->ChildList, &(PciDeviceNode->Link));
  PciDeviceNode->Parent = Bridge;

  return EFI_SUCCESS;
}

EFI_STATUS
DestroyRootBridge (
  IN PCI_IO_DEVICE *RootBridge
  )
/*++

Routine Description:

  
Arguments:

  RootBridge   - A pointer to the PCI_IO_DEVICE.

Returns:

  None

--*/
{
  DestroyPciDeviceTree (RootBridge);

  FreePciDevice (RootBridge);

  return EFI_SUCCESS;
}

EFI_STATUS
FreePciDevice (
  IN PCI_IO_DEVICE *PciIoDevice
  )
/*++

Routine Description:

  Destroy a pci device node.
  Also all direct or indirect allocated resource for this node will be freed.   

Arguments:

  PciIoDevice   - A pointer to the PCI_IO_DEVICE.

Returns:

  None

--*/
// TODO:    EFI_SUCCESS - add return value to function comment
{

  //
  // Assume all children have been removed underneath this device
  //
  if (PciIoDevice->DevicePath != NULL) {
    gBS->FreePool (PciIoDevice->DevicePath);
  }

  gBS->FreePool (PciIoDevice);

  return EFI_SUCCESS;
}

EFI_STATUS
DestroyPciDeviceTree (
  IN PCI_IO_DEVICE *Bridge
  )
/*++

Routine Description:

  Destroy all the pci device node under the bridge.
  Bridge itself is not included.

Arguments:

  Bridge   - A pointer to the PCI_IO_DEVICE.

Returns:

  None

--*/
{
  EFI_LIST_ENTRY  *CurrentLink;
  PCI_IO_DEVICE   *Temp;

  while (!IsListEmpty (&Bridge->ChildList)) {

    CurrentLink = Bridge->ChildList.ForwardLink;

    //
    // Remove this node from the linked list
    //
    RemoveEntryList (CurrentLink);

    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    if (!IsListEmpty (&Temp->ChildList)) {
      DestroyPciDeviceTree (Temp);
    }
    FreePciDevice (Temp);
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
DestroyRootBridgeByHandle (
  EFI_HANDLE Controller
  )
/*++

Routine Description:

  Destroy all device nodes under the root bridge
  specified by Controller. 
  The root bridge itself is also included.

Arguments:

  Controller   - An efi handle.

Returns:

  None

--*/
{

  EFI_LIST_ENTRY  *CurrentLink;
  PCI_IO_DEVICE   *Temp;

  CurrentLink = gPciDevicePool.ForwardLink;

  while (CurrentLink && CurrentLink != &gPciDevicePool) {
    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    if (Temp->Handle == Controller) {

      RemoveEntryList (CurrentLink);

      DestroyPciDeviceTree (Temp);

      FreePciDevice (Temp);

      return EFI_SUCCESS;
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
RegisterPciDevice (
  IN  EFI_HANDLE                     Controller,
  IN  PCI_IO_DEVICE                  *PciIoDevice,
  OUT EFI_HANDLE                     *Handle OPTIONAL
  )
/*++

Routine Description:

  This function registers the PCI IO device. It creates a handle for this PCI IO device 
  (if the handle does not exist), attaches appropriate protocols onto the handle, does
  necessary initialization, and sets up parent/child relationship with its bus controller.

Arguments:

  Controller    - An EFI handle for the PCI bus controller.
  PciIoDevice   - A PCI_IO_DEVICE pointer to the PCI IO device to be registered.
  Handle        - A pointer to hold the EFI handle for the PCI IO device.

Returns:

  EFI_SUCCESS   - The PCI device is successfully registered.
  Others        - An error occurred when registering the PCI device.

--*/
{
  EFI_STATUS          Status;
  UINT8               PciExpressCapRegOffset;
  BOOLEAN             HasEfiImage;

  //
  // Install the pciio protocol, device path protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &PciIoDevice->Handle,
                  &gEfiDevicePathProtocolGuid,
                  PciIoDevice->DevicePath,
                  &gEfiPciIoProtocolGuid,
                  &PciIoDevice->PciIo,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Detect if PCI Express Device
  //
  PciExpressCapRegOffset = 0;
  Status = LocateCapabilityRegBlock (
             PciIoDevice,
             EFI_PCI_CAPABILITY_ID_PCIEXP,
             &PciExpressCapRegOffset,
             NULL
             );
  if (!EFI_ERROR (Status)) {
    PciIoDevice->IsPciExp = TRUE;
    DEBUG ((EFI_D_ERROR, "PciExp - %x (B-%x, D-%x, F-%x)\n", (UINTN) PciIoDevice->IsPciExp, (UINTN) PciIoDevice->BusNumber, (UINTN) PciIoDevice->DeviceNumber, (UINTN) PciIoDevice->FunctionNumber));
  }

  //
  // Determine if there are EFI images in the option rom
  //
  HasEfiImage = ContainEfiImage (PciIoDevice->PciIo.RomImage, PciIoDevice->PciIo.RomSize);

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  if (HasEfiImage) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &PciIoDevice->Handle,
                    &gEfiLoadFile2ProtocolGuid,
                    &PciIoDevice->LoadFile2,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      gBS->UninstallMultipleProtocolInterfaces (
             &PciIoDevice->Handle,
             &gEfiDevicePathProtocolGuid,
             PciIoDevice->DevicePath,
             &gEfiPciIoProtocolGuid,
             &PciIoDevice->PciIo,
             NULL
             );
      return Status;
    }
  }
#endif

  if (!PciIoDevice->AllOpRomProcessed) {

    PciIoDevice->AllOpRomProcessed = TRUE;
    //
    // Dispatch the EFI OpRom for the PCI device.
    // The OpRom is got from platform in the above code
    //   or loaded from device in the previous round of bus enumeration
    //
    if (HasEfiImage) {
      ProcessOpRomImage (PciIoDevice);
    }
  }


  if (PciIoDevice->BusOverride) {
    //
    // Install BusSpecificDriverOverride Protocol
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &PciIoDevice->Handle,
                    &gEfiBusSpecificDriverOverrideProtocolGuid,
                    &PciIoDevice->PciDriverOverride,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      gBS->UninstallMultipleProtocolInterfaces (
             &PciIoDevice->Handle,
             &gEfiDevicePathProtocolGuid,
             PciIoDevice->DevicePath,
             &gEfiPciIoProtocolGuid,
             &PciIoDevice->PciIo,
             NULL
             );

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
      if (HasEfiImage) {
        gBS->UninstallMultipleProtocolInterfaces (
               &PciIoDevice->Handle,
               &gEfiLoadFile2ProtocolGuid,
               &PciIoDevice->LoadFile2,
               NULL
               );
      }
#endif
      
      return Status;
    }
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciRootBridgeIoProtocolGuid,
                  (VOID **) &(PciIoDevice->PciRootBridgeIo),
                  gPciBusDriverBinding.DriverBindingHandle,
                  PciIoDevice->Handle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Handle != NULL) {
    *Handle = PciIoDevice->Handle;
  }

  //
  // Indicate the pci device is registered
  //
  PciIoDevice->Registered = TRUE;

  return EFI_SUCCESS;
}


EFI_STATUS
DeRegisterPciDevice (
  IN  EFI_HANDLE                     Controller,
  IN  EFI_HANDLE                     Handle
  )
/*++

Routine Description:

  This function is used to de-register the PCI device from the EFI,
  That includes un-installing PciIo protocol from the specified PCI 
  device handle.

Arguments:

  Controller   - An efi handle.
  Handle       - An efi handle.

Returns:

  None

--*/
{
  EFI_PCI_IO_PROTOCOL             *PciIo;
  EFI_STATUS                      Status;
  PCI_IO_DEVICE                   *PciIoDevice;
  PCI_IO_DEVICE                   *Node;
  EFI_LIST_ENTRY                  *CurrentLink;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;

  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  gPciBusDriverBinding.DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    PciIoDevice = PCI_IO_DEVICE_FROM_PCI_IO_THIS (PciIo);

    //
    // If it is already de-registered
    //
    if (!PciIoDevice->Registered) {
      return EFI_SUCCESS;
    }

    //
    // If it is PPB, first de-register its children
    //

    if (!IsListEmpty (&PciIoDevice->ChildList)) {

      CurrentLink = PciIoDevice->ChildList.ForwardLink;

      while (CurrentLink && CurrentLink != &PciIoDevice->ChildList) {
        Node    = PCI_IO_DEVICE_FROM_LINK (CurrentLink);
        Status  = DeRegisterPciDevice (Controller, Node->Handle);

        if (EFI_ERROR (Status)) {
          return Status;
        }

        CurrentLink = CurrentLink->ForwardLink;
      }
    }

    //
    // First disconnect this device
    //
//    PciIoDevice->PciIo.Attributes(&(PciIoDevice->PciIo),
//                                    EfiPciIoAttributeOperationDisable,
//                                    EFI_PCI_DEVICE_ENABLE,
//                                    NULL
//                                    );
       
    //
    // Close the child handle
    //
    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiPciRootBridgeIoProtocolGuid,
                    gPciBusDriverBinding.DriverBindingHandle,
                    Handle
                    );

    //
    // Un-install the device path protocol and pci io protocol
    //
    if (PciIoDevice->BusOverride) {
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      Handle,
                      &gEfiDevicePathProtocolGuid,
                      PciIoDevice->DevicePath,
                      &gEfiPciIoProtocolGuid,
                      &PciIoDevice->PciIo,
                      &gEfiBusSpecificDriverOverrideProtocolGuid,
                      &PciIoDevice->PciDriverOverride,
                      NULL
                      );
    } else {
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      Handle,
                      &gEfiDevicePathProtocolGuid,
                      PciIoDevice->DevicePath,
                      &gEfiPciIoProtocolGuid,
                      &PciIoDevice->PciIo,
                      NULL
                      );
    }

#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
    if (!EFI_ERROR (Status)) {
      //
      // Try to uninstall LoadFile2 protocol if exists
      //
      Status = gBS->OpenProtocol (
                      Handle,
                      &gEfiLoadFile2ProtocolGuid,
                      NULL,
                      gPciBusDriverBinding.DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                      );
      if (!EFI_ERROR (Status)) {
        Status = gBS->UninstallMultipleProtocolInterfaces (
                        Handle,
                        &gEfiLoadFile2ProtocolGuid,
                        &PciIoDevice->LoadFile2,
                        NULL
                        );
      }
      //
      // Restore Status
      //
      Status = EFI_SUCCESS;
    }
#endif

    if (EFI_ERROR (Status)) {
      gBS->OpenProtocol (
            Controller,
            &gEfiPciRootBridgeIoProtocolGuid,
            (VOID **) &PciRootBridgeIo,
            gPciBusDriverBinding.DriverBindingHandle,
            Handle,
            EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
            );
      return Status;
    }
    
    //
    // The Device Driver should disable this device after disconnect
    // so the Pci Bus driver will not touch this device any more.
    // Restore the register field to the original value
    //
    PciIoDevice->Registered = FALSE;
    PciIoDevice->Handle     = NULL;
  } else {

    //
    // Handle may be closed before
    //
    return EFI_SUCCESS;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EnableBridgeAttributes (
  IN PCI_IO_DEVICE                       *PciIoDevice
  )
{
  PCI_TYPE01                PciData;

  //
  // NOTE: We should not set EFI_PCI_DEVICE_ENABLE for a bridge
  //       directly, because some legacy BIOS will NOT assign
  //       IO or Memory resource for a bridge who has no child
  //       device. So we add check IO or Memory here.
  //

  PciIoDevice->PciIo.Pci.Read (
                           &PciIoDevice->PciIo,
                           EfiPciIoWidthUint8,
                           0,
                           sizeof (PciData),
                           &PciData
                           );

  if ((((PciData.Bridge.IoBase & 0xF) == 0) &&
        (PciData.Bridge.IoBase != 0 || PciData.Bridge.IoLimit != 0)) ||
      (((PciData.Bridge.IoBase & 0xF) == 1) &&
        ((PciData.Bridge.IoBase & 0xF0) != 0 || (PciData.Bridge.IoLimit & 0xF0) != 0 || PciData.Bridge.IoBaseUpper16 != 0 || PciData.Bridge.IoLimitUpper16 != 0))) {
    PciIoDevice->PciIo.Attributes(
                         &(PciIoDevice->PciIo),
                         EfiPciIoAttributeOperationEnable,
                         (EFI_PCI_IO_ATTRIBUTE_IO | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER),
                         NULL
                         );
  }
  if ((PciData.Bridge.MemoryBase & 0xFFF0) != 0 || (PciData.Bridge.MemoryLimit & 0xFFF0) != 0) {
    PciIoDevice->PciIo.Attributes(
                         &(PciIoDevice->PciIo),
                         EfiPciIoAttributeOperationEnable,
                         (EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER),
                         NULL
                         );
  }
  if ((((PciData.Bridge.PrefetchableMemoryBase & 0xF) == 0) &&
        (PciData.Bridge.PrefetchableMemoryBase != 0 || PciData.Bridge.PrefetchableMemoryLimit != 0)) ||
      (((PciData.Bridge.PrefetchableMemoryBase & 0xF) == 1) &&
        ((PciData.Bridge.PrefetchableMemoryBase & 0xFFF0) != 0 || (PciData.Bridge.PrefetchableMemoryLimit & 0xFFF0) != 0 || PciData.Bridge.PrefetchableBaseUpper32 != 0 || PciData.Bridge.PrefetchableLimitUpper32 != 0))) {
    PciIoDevice->PciIo.Attributes(
                         &(PciIoDevice->PciIo),
                         EfiPciIoAttributeOperationEnable,
                         (EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER),
                         NULL
                         );
  }

  return EFI_SUCCESS;
}

EFI_STATUS 
StartPciDevicesOnBridge (
  IN EFI_HANDLE                          Controller,
  IN PCI_IO_DEVICE                       *RootBridge,
  IN EFI_DEVICE_PATH_PROTOCOL            *RemainingDevicePath 
  )
/*++

Routine Description:

  Start to manage the PCI device on specified the root bridge or PCI-PCI Bridge

Arguments:

  Controller          - An efi handle.
  RootBridge          - A pointer to the PCI_IO_DEVICE.
  RemainingDevicePath - A pointer to the EFI_DEVICE_PATH_PROTOCOL.
  NumberOfChildren    - Children number.
  ChildHandleBuffer   - A pointer to the child handle buffer.

Returns:

  None

--*/
{
  PCI_IO_DEVICE             *PciIoDevice;
  EFI_DEV_PATH_PTR          Node;
  EFI_DEVICE_PATH_PROTOCOL  *CurrentDevicePath;
  EFI_STATUS                Status;
  EFI_LIST_ENTRY            *CurrentLink;

  CurrentLink = RootBridge->ChildList.ForwardLink;

  while (CurrentLink && CurrentLink != &RootBridge->ChildList) {

    PciIoDevice = PCI_IO_DEVICE_FROM_LINK (CurrentLink);
    if (RemainingDevicePath != NULL) {

      Node.DevPath = RemainingDevicePath;

      if (Node.Pci->Device != PciIoDevice->DeviceNumber || 
          Node.Pci->Function != PciIoDevice->FunctionNumber) {
        CurrentLink = CurrentLink->ForwardLink;
        continue;
      }

      //
      // Check if the device has been assigned with required resource
      //
      if (!PciIoDevice->Allocated) {
        return EFI_NOT_READY;
      }
      
      //
      // Check if the current node has been registered before
      // If it is not, register it
      //
      if (!PciIoDevice->Registered) {
        Status = RegisterPciDevice (
                   Controller,
                   PciIoDevice,
                   NULL
                   );

      }
      
      //
      // Get the next device path
      //
      CurrentDevicePath = EfiNextDevicePathNode (RemainingDevicePath);
      if (EfiIsDevicePathEnd (CurrentDevicePath)) {
        return EFI_SUCCESS;
      }
  
      //
      // If it is a PPB
      //
      if (!IsListEmpty (&PciIoDevice->ChildList)) {
        Status = StartPciDevicesOnBridge (
                   Controller,
                   PciIoDevice,
                   CurrentDevicePath
                   );
        EnableBridgeAttributes (PciIoDevice);

        return Status;
      } else {

        //
        // Currently, the PCI bus driver only support PCI-PCI bridge
        //
        return EFI_UNSUPPORTED;
      }

    } else {

      //
      // If remaining device path is NULL,
      // try to enable all the pci devices under this bridge
      //

      if (!PciIoDevice->Registered && PciIoDevice->Allocated) {
        Status = RegisterPciDevice (
                  Controller,
                  PciIoDevice,
                  NULL
                  );

      }

      if (!IsListEmpty (&PciIoDevice->ChildList)) {
        Status = StartPciDevicesOnBridge ( 
                   Controller,
                   PciIoDevice,
                   RemainingDevicePath
                   );
        EnableBridgeAttributes (PciIoDevice);
      }

      CurrentLink = CurrentLink->ForwardLink;
    }
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
StartPciDevices (
  IN EFI_HANDLE                         Controller,
  IN EFI_DEVICE_PATH_PROTOCOL           *RemainingDevicePath
  )
/*++

Routine Description:

  Start to manage the PCI device according to RemainingDevicePath
  If RemainingDevicePath == NULL, the PCI bus driver will start 
  to manage all the PCI devices it found previously

Arguments:
  Controller          - An efi handle.
  RemainingDevicePath - A pointer to the EFI_DEVICE_PATH_PROTOCOL.

Returns:

  None

--*/
{
  EFI_DEV_PATH_PTR  Node;
  PCI_IO_DEVICE     *RootBridge;
  EFI_LIST_ENTRY    *CurrentLink;

  if (RemainingDevicePath != NULL) {

    //
    // Check if the RemainingDevicePath is valid
    //
    Node.DevPath = RemainingDevicePath;
    if (Node.DevPath->Type != HARDWARE_DEVICE_PATH ||
        Node.DevPath->SubType != HW_PCI_DP         &&
        DevicePathNodeLength (Node.DevPath) != sizeof (PCI_DEVICE_PATH)
        ) {
      return EFI_UNSUPPORTED;
    }
  }

  CurrentLink = gPciDevicePool.ForwardLink;

  while (CurrentLink && CurrentLink != &gPciDevicePool) {

    RootBridge = PCI_IO_DEVICE_FROM_LINK (CurrentLink);
    //
    // Locate the right root bridge to start
    //
    if (RootBridge->Handle == Controller) {
      StartPciDevicesOnBridge (
        Controller,
        RootBridge,
        RemainingDevicePath
        );
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return EFI_SUCCESS;
}

PCI_IO_DEVICE *
CreateRootBridge (
  IN EFI_HANDLE RootBridgeHandle
  )
/*++

Routine Description:


Arguments:
  RootBridgeHandle   - An efi handle.

Returns:

  None

--*/
{

  EFI_STATUS                      Status;
  PCI_IO_DEVICE                   *Dev;
  EFI_DEVICE_PATH_PROTOCOL        *ParentDevicePath;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;

  Dev = NULL;
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (PCI_IO_DEVICE),
                  (VOID **) &Dev
                  );

  if (EFI_ERROR (Status)) {
    return NULL;
  }

  EfiZeroMem (Dev, sizeof (PCI_IO_DEVICE));
  Dev->Signature  = PCI_IO_DEVICE_SIGNATURE;
  Dev->Handle     = RootBridgeHandle;
  InitializeListHead (&Dev->ChildList);

  Status = gBS->OpenProtocol (
                  RootBridgeHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath,
                  gPciBusDriverBinding.DriverBindingHandle,
                  RootBridgeHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    gBS->FreePool (Dev);
    return NULL;
  }

  //
  // Record the root bridge parent device path
  //
  Dev->DevicePath = EfiDuplicateDevicePath (ParentDevicePath);

  //
  // Get the pci root bridge io protocol
  //
  Status = gBS->OpenProtocol (
                  RootBridgeHandle,
                  &gEfiPciRootBridgeIoProtocolGuid,
                  (VOID **) &PciRootBridgeIo,
                  gPciBusDriverBinding.DriverBindingHandle,
                  RootBridgeHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    FreePciDevice (Dev);
    return NULL;
  }

  Dev->PciRootBridgeIo = PciRootBridgeIo;

  //
  // Initialize the PCI I/O instance structure
  //
  InitializePciIoInstance (Dev);
  InitializePciDriverOverrideInstance (Dev);
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  InitializePciLoadFile2 (Dev);
#endif

  //
  // Initialize reserved resource list and
  // option rom driver list
  //
  InitializeListHead (&Dev->ReservedResourceList);
  InitializeListHead (&Dev->OptionRomDriverList);

  return Dev;
}

PCI_IO_DEVICE *
GetRootBridgeByHandle (
  EFI_HANDLE RootBridgeHandle
  )
/*++

Routine Description:


Arguments:

  RootBridgeHandle    - An efi handle.

Returns:

  None

--*/
{
  PCI_IO_DEVICE   *RootBridgeDev;
  EFI_LIST_ENTRY  *CurrentLink;

  CurrentLink = gPciDevicePool.ForwardLink;

  while (CurrentLink && CurrentLink != &gPciDevicePool) {

    RootBridgeDev = PCI_IO_DEVICE_FROM_LINK (CurrentLink);
    if (RootBridgeDev->Handle == RootBridgeHandle) {
      return RootBridgeDev;
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return NULL;
}

BOOLEAN
PciDeviceExisted (
  IN PCI_IO_DEVICE    *Bridge,
  IN PCI_IO_DEVICE    *PciIoDevice
  )
/*++

Routine Description:
  
Arguments:

  Bridge       - A pointer to the PCI_IO_DEVICE.
  PciIoDevice  - A pointer to the PCI_IO_DEVICE.

Returns:

  None

--*/
{

  PCI_IO_DEVICE   *Temp;
  EFI_LIST_ENTRY  *CurrentLink;

  CurrentLink = Bridge->ChildList.ForwardLink;

  while (CurrentLink && CurrentLink != &Bridge->ChildList) {

    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    if (Temp == PciIoDevice) {
      return TRUE;
    }

    if (!IsListEmpty (&Temp->ChildList)) {
      if (PciDeviceExisted (Temp, PciIoDevice)) {
        return TRUE;
      }
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return FALSE;
}

PCI_IO_DEVICE *
ActiveVGADeviceOnTheSameSegment (
  IN PCI_IO_DEVICE        *VgaDevice
  )
/*++

Routine Description:

Arguments:

  VgaDevice    - A pointer to the PCI_IO_DEVICE.
  
Returns:

  None

--*/
{
  EFI_LIST_ENTRY  *CurrentLink;
  PCI_IO_DEVICE   *Temp;

  CurrentLink = gPciDevicePool.ForwardLink;

  while (CurrentLink && CurrentLink != &gPciDevicePool) {

    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    if (Temp->PciRootBridgeIo->SegmentNumber == VgaDevice->PciRootBridgeIo->SegmentNumber) {

      Temp = ActiveVGADeviceOnTheRootBridge (Temp);

      if (Temp != NULL) {
        return Temp;
      }
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return NULL;
}

PCI_IO_DEVICE *
ActiveVGADeviceOnTheRootBridge (
  IN PCI_IO_DEVICE        *RootBridge
  )
/*++

Routine Description:

Arguments:

  RootBridge    - A pointer to the PCI_IO_DEVICE.

Returns:

  None

--*/
{
  EFI_LIST_ENTRY  *CurrentLink;
  PCI_IO_DEVICE   *Temp;

  CurrentLink = RootBridge->ChildList.ForwardLink;

  while (CurrentLink && CurrentLink != &RootBridge->ChildList) {

    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    if (IS_PCI_VGA(&Temp->Pci) && 
        (Temp->Attributes &
         (EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY |
          EFI_PCI_IO_ATTRIBUTE_VGA_IO     |
          EFI_PCI_IO_ATTRIBUTE_VGA_IO_16))) {
      return Temp;
    }

    if (IS_PCI_BRIDGE (&Temp->Pci)) {

      Temp = ActiveVGADeviceOnTheRootBridge (Temp);

      if (Temp != NULL) {
        return Temp;
      }
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return NULL;
}
