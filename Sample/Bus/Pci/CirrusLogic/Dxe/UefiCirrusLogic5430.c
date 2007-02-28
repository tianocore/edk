/*++

Copyright (c) 2006 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  UefiCirrusLogic5430.c
    
Abstract:

  Cirrus Logic 5430 Controller Driver.
  This driver is a sample implementation of the Graphics Output Protocol for the
  Cirrus Logic 5430 family of PCI video controllers.  This driver is only
  usable in the UEFI pre-boot environment.  This sample is intended to show
  how the Graphics Output Protocol is able to function.

Revision History:

--*/

//
// Cirrus Logic 5430 Controller Driver
//

#include "UefiCirrusLogic5430.h"

EFI_DRIVER_BINDING_PROTOCOL gCirrusLogic5430DriverBinding = {
  CirrusLogic5430ControllerDriverSupported,
  CirrusLogic5430ControllerDriverStart,
  CirrusLogic5430ControllerDriverStop,
  0x10,
  NULL,
  NULL
};

ACPI_ADR_DEVICE_PATH            mGopAcpiDevicePath = {
  {
    ACPI_DEVICE_PATH,
    ACPI_ADR_DP,
    {
      sizeof(ACPI_ADR_DEVICE_PATH),
      0
    }
  },
  ACPI_DISPLAY_ADR (1, 0, 0, 1, 0, ACPI_ADR_DISPLAY_TYPE_VGA, 0, 0)
};

EFI_STATUS
ControllerChildHandleUninstall (
  EFI_DRIVER_BINDING_PROTOCOL    *This,
  EFI_HANDLE                     Controller,
  EFI_HANDLE                     Handle
  );

//
// Cirrus Logic 5430 Driver Entry point
//

EFI_DRIVER_ENTRY_POINT (CirrusLogic5430GraphicsOutputDriverEntryPoint)

EFI_STATUS
EFIAPI
CirrusLogic5430GraphicsOutputDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

Arguments:

Returns:

    None

--*/
// TODO:    ImageHandle - add argument and description to function comment
// TODO:    SystemTable - add argument and description to function comment
{
  return EfiLibInstallAllDriverProtocols (
          ImageHandle,
          SystemTable,
          &gCirrusLogic5430DriverBinding,
          ImageHandle,
          &gCirrusLogic5430ComponentName,
          NULL,
          NULL
          );
}

EFI_STATUS
EFIAPI
CirrusLogic5430ControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

    None

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Controller - add argument and description to function comment
// TODO:    RemainingDevicePath - add argument and description to function comment
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;

  //
  // Open the PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        sizeof (Pci) / sizeof (UINT32),
                        &Pci
                        );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = EFI_UNSUPPORTED;
  //
  // See if the I/O enable is on.  Most systems only allow one VGA device to be turned on
  // at a time, so see if this is one that is turned on.
  //
  //  if (((Pci.Hdr.Command & 0x01) == 0x01)) {
  //
  // See if this is a Cirrus Logic PCI controller
  //
  if (Pci.Hdr.VendorId == CIRRUS_LOGIC_VENDOR_ID) {
    //
    // See if this is a 5430 or a 5446 PCI controller
    //
    if (Pci.Hdr.DeviceId == CIRRUS_LOGIC_5430_DEVICE_ID) {
      Status = EFI_SUCCESS;
    }

    if (Pci.Hdr.DeviceId == CIRRUS_LOGIC_5430_ALTERNATE_DEVICE_ID) {
      Status = EFI_SUCCESS;
    }

    if (Pci.Hdr.DeviceId == CIRRUS_LOGIC_5446_DEVICE_ID) {
      Status = EFI_SUCCESS;
    }
  }

Done:
  //
  // Close the PCI I/O Protocol
  //
  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

EFI_STATUS
EFIAPI
CirrusLogic5430ControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

    None

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Controller - add argument and description to function comment
// TODO:    RemainingDevicePath - add argument and description to function comment
{
  EFI_STATUS                      Status;
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private;
  EFI_PCI_IO_PROTOCOL             *PciIo;
  EFI_DEVICE_PATH_PROTOCOL        *ParentDevicePath;

  //
  // Prepare parent device path
  //
  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Allocate Private context data for GOP inteface.
  //
  Private = NULL;
  Private = EfiLibAllocateZeroPool (sizeof (CIRRUS_LOGIC_5430_PRIVATE_DATA));
  if (Private == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }

  //
  // Set up context record
  //
  Private->Signature  = CIRRUS_LOGIC_5430_PRIVATE_DATA_SIGNATURE;

  //
  // Open PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  Private->PciIo = PciIo;

  Status = Private->PciIo->Attributes (
                            Private->PciIo,
                            EfiPciIoAttributeOperationEnable,
                            EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO,
                            NULL
                            );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // Start the Graphics Output Protocol software stack.
  //
  Status = CirrusLogic5430GraphicsOutputConstructor (Private);
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  if (RemainingDevicePath == NULL) {
    //
    // If ACPI_ADR is not specified, use default
    //
    Private->DevicePath = EfiAppendDevicePathNode (ParentDevicePath, (EFI_DEVICE_PATH_PROTOCOL *) &mGopAcpiDevicePath);
  } else {
    Private->DevicePath = EfiAppendDevicePathNode (ParentDevicePath, RemainingDevicePath);
  }

  //
  // Publish the Graphics Output Protocol interface to the world
  //
  Private->Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,
                  &gEfiDevicePathProtocolGuid,
                  Private->DevicePath,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput,
                  &gEfiEdidDiscoveredProtocolGuid,
                  &Private->EdidDiscovered,
                  &gEfiEdidActiveProtocolGuid,
                  &Private->EdidActive,
                  NULL
                  );
  if (!EFI_ERROR(Status)) {
    //
    // Open the Parent Handle for the child
    //
    Status = gBS->OpenProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &Private->PciIo,
                    This->DriverBindingHandle,
                    Private->Handle,
                    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                    );
  }

Error:
  if (EFI_ERROR (Status)) {
    if (Private) {
      if (Private->PciIo) {
        Private->PciIo->Attributes (
                          Private->PciIo,
                          EfiPciIoAttributeOperationDisable,
                          EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO,
                          NULL
                          );
      }
    }

    //
    // Close the PCI I/O Protocol
    //
    gBS->CloseProtocol (
          Controller,
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    if (Private) {
      gBS->FreePool (Private);
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
CirrusLogic5430ControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN UINTN                          NumberOfChildren,
  IN EFI_HANDLE                     *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:

Returns:

    None

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Controller - add argument and description to function comment
// TODO:    NumberOfChildren - add argument and description to function comment
// TODO:    ChildHandleBuffer - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS  Status;
  BOOLEAN     AllChildrenStopped;
  UINTN       Index;

  if (NumberOfChildren == 0) {
    //
    // Close PCI I/O protocol on the controller handle
    //
    gBS->CloseProtocol (
          Controller,
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );

    return EFI_SUCCESS;
  }

  AllChildrenStopped = TRUE;
  for (Index = 0; Index < NumberOfChildren; Index ++) {

    Status = ControllerChildHandleUninstall (This, Controller, ChildHandleBuffer[Index]);
    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ControllerChildHandleUninstall (
  EFI_DRIVER_BINDING_PROTOCOL    *This,
  EFI_HANDLE                     Controller,
  EFI_HANDLE                     Handle
  )
/*++

Routine Description:

  Deregister an video child handle and free resources

Arguments:

  This            - Protocol instance pointer.
  Controller      - Video controller handle
  Handle          - Video child handle

Returns:

  EFI_STATUS

--*/
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput;
  EFI_STATUS                      Status;
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private;

  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **) &GraphicsOutput,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    //
    // If the Graphics Output Protocol interface does not exist the driver is not started
    //
    return Status;
  }

  //
  // Get our private context information
  //
  Private = CIRRUS_LOGIC_5430_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (GraphicsOutput);

  //
  // Close the PCI I/O Protocol
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Handle
         );

  //
  // Remove the Graphics Output Protocol interface from the system
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Private->Handle,
                  &gEfiDevicePathProtocolGuid,
                  Private->DevicePath,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput,
                  &gEfiEdidDiscoveredProtocolGuid,
                  &Private->EdidDiscovered,
                  &gEfiEdidActiveProtocolGuid,
                  &Private->EdidActive,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Shutdown the hardware
  //
  CirrusLogic5430GraphicsOutputDestructor (Private);

  Private->PciIo->Attributes (
                    Private->PciIo,
                    EfiPciIoAttributeOperationDisable,
                    EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO,
                    NULL
                    );

  //
  // Free our instance data
  //
  gBS->FreePool (Private);

  return EFI_SUCCESS;
}
