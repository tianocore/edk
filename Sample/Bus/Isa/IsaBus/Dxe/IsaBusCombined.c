/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  IsaBusCombined.c
  
Abstract:

  Discovers all the ISA Controllers and their resources by using the ISA PnP 
  Protocol, produces an instance of the ISA I/O Protocol for every ISA 
  Controller found, loads and initializes all ISA Device Drivers, matches ISA
  Device Drivers with their respective ISA Controllers in a deterministic 
  manner, and informs a ISA Device Driver when it is to start managing an ISA
  Controller. 

Revision History:

--*/

#include "..\..\IsaDriverLib.h"

//
// ISA Bus Driver Global Variables
//

EFI_DRIVER_BINDING_PROTOCOL gIsaBusControllerDriver = {
  IsaBusControllerDriverSupported,
  IsaBusControllerDriverStart,
  IsaBusControllerDriverStop,
  0xa,
  NULL,
  NULL
};

EFI_STATUS
EFIAPI
IsaBusControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     * RemainingDevicePath OPTIONAL
  )
/*++

  Routine Description:
  
    This function checks to see if a controller can be managed by the ISA Bus 
    Driver. This is done by checking to see if the controller supports the 
    EFI_PCI_IO_PROTOCOL protocol, and then looking at the PCI Configuration 
    Header to see if the device is a PCI to ISA bridge. The class code of 
    PCI to ISA bridge: Base class 06h, Sub class 01h Interface 00h 
  
  Arguments:
  
    This                 - The EFI_DRIVER_BINDING_PROTOCOL instance.
    Controller           - The handle of the device to check.
    RemainingDevicePath  - A pointer to the remaining portion of a device path.

  Returns:
  
    EFI_SUCCESS          - The device is supported by this driver.
    EFI_UNSUPPORTED      - The device is not supported by this driver.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *IsaBridgeDevicePath;
  PCI_TYPE00                Pci;
  EFI_PCI_IO_PROTOCOL       *PciIo;


  //
  // If RemainingDevicePath is not NULL, it should verify that the first device
  // path node in RemainingDevicePath is an ACPI Device path node
  //
  if (RemainingDevicePath != NULL) {
    if (RemainingDevicePath->Type != ACPI_DEVICE_PATH) {
      return EFI_UNSUPPORTED;
    } else if (RemainingDevicePath->SubType == ACPI_DP) {
      if (DevicePathNodeLength (RemainingDevicePath) != sizeof (ACPI_HID_DEVICE_PATH)) {
        return EFI_UNSUPPORTED;
      }
    } else if (RemainingDevicePath->SubType == ACPI_EXTENDED_DP) {
      if (DevicePathNodeLength (RemainingDevicePath) != sizeof (ACPI_EXTENDED_HID_DEVICE_PATH)) {
        return EFI_UNSUPPORTED;
      }
    } else {
      return EFI_UNSUPPORTED;
    }
  }
  //
  // Get the ISA bridge's Device Path and and test it
  // to see if it is supported by Isa bus.
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &IsaBridgeDevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    return Status;
  }

  Status = CheckAcpiNodeStatus (IsaBridgeDevicePath);
  gBS->CloseProtocol (
         Controller,
         &gEfiDevicePathProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );


  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Get PciIo protocol instance
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

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        sizeof (Pci) / sizeof (UINT32),
                        &Pci
                        );

  if (!EFI_ERROR (Status)) {
    Status = EFI_UNSUPPORTED;
    if ((Pci.Hdr.Command & 0x03) == 0x03) {
      if (Pci.Hdr.ClassCode[2] == PCI_CLASS_BRIDGE) {
        //
        // See if this is a standard PCI to ISA Bridge from the Base Code
        // and Class Code
        //
        if (Pci.Hdr.ClassCode[1] == PCI_CLASS_ISA) {
          Status = EFI_SUCCESS;
        }
        //
        // See if this is an Intel PCI to ISA bridge in Positive Decode Mode
        //
        if (Pci.Hdr.ClassCode[1] == PCI_CLASS_ISA_POSITIVE_DECODE &&
            Pci.Hdr.VendorId == 0x8086 &&
            Pci.Hdr.DeviceId == 0x7110
            ) {
          Status = EFI_SUCCESS;
        }
      }
    }
  }

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
IsaBusControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     * RemainingDevicePath OPTIONAL
  )
/*++

  Routine Description:
  
    This function tells the ISA Bus Driver to start managing a PCI to ISA 
    Bridge controller. 
  
  Arguments:
  
    This                  - The EFI_DRIVER_BINDING_PROTOCOL instance.
    Controller            - A handle to the device being started. 
    RemainingDevicePath   - A pointer to the remaining portion of a device path.

  Returns:
  
    EFI_SUCCESS           - The device was started.
    EFI_UNSUPPORTED       - The device is not supported.
    EFI_DEVICE_ERROR      - The device could not be started due to a device error.
    EFI_ALREADY_STARTED   - The device has already been started.
    EFI_INVALID_PARAMETER - One of the parameters has an invalid value.
    EFI_OUT_OF_RESOURCES  - The request could not be completed due to a lack of 
                            resources.
  
--*/
{
  EFI_STATUS                            Status;
  EFI_DEVICE_PATH_PROTOCOL              *ParentDevicePath;
  EFI_PCI_IO_PROTOCOL                   *PciIo;
  EFI_DEVICE_PATH_PROTOCOL              *ChildDevicePath;
  EFI_ISA_ACPI_DEVICE_ID                *IsaDevice;
  EFI_ISA_ACPI_RESOURCE_LIST            *ResourceList;

  //
  // Open Device Path Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    return Status;
  }
  //
  // Open Pci IO Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    //
    // Close opened protocol
    //
    gBS->CloseProtocol (
           Controller,
           &gEfiDevicePathProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );
    return Status;
  }
  
  //
  // Report Status Code here since we will initialize the host controller
  //
  ReportStatusCodeWithDevicePath (
    EFI_PROGRESS_CODE,
    (EFI_IO_BUS_LPC | EFI_IOB_PC_INIT),
    0,
    &gEfiCallerIdGuid,
    ParentDevicePath
    );

  //
  // ISA controller driver initialization
  //
  IsaControllerInit (PciIo);

  //
  // firstly init ISA interface
  //
  LpcInterfaceInit (PciIo);

  //
  // Report Status Code here since we will enable the host controller
  //
  ReportStatusCodeWithDevicePath (
    EFI_PROGRESS_CODE,
    (EFI_IO_BUS_LPC | EFI_IOB_PC_ENABLE),
    0,
    &gEfiCallerIdGuid,
    ParentDevicePath
    );

  //
  // Create each ISA device handle in this ISA bus
  //
  IsaDevice = NULL;
  do {
    Status = IsaDeviceEnumerate (&IsaDevice);
    if (EFI_ERROR (Status)) {
      break;
    }
    //
    // Get current resource of this ISA device
    //
    ResourceList  = NULL;
    Status = IsaGetAcpiResource (IsaDevice, &ResourceList);
    if (EFI_ERROR (Status)) {
      continue;
    }
    //
    // Create handle for this ISA device
    //
    Status = IsaCreateDevice (
               This,
               Controller,
               PciIo,
               ParentDevicePath,
               ResourceList,
               &ChildDevicePath
               );

    if (EFI_ERROR (Status)) {
      continue;
    }

  } while (TRUE);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
IsaBusControllerDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   * ChildHandleBuffer OPTIONAL
  )
/*++

  Routine Description:
  
    This function tells the ISA Bus Driver to stop managing a PCI to ISA 
    Bridge controller. 
     
  Arguments:
  
    This                   -  The EFI_DRIVER_BINDING_PROTOCOL instance.
    Controller             -  A handle to the device being stopped.
    NumberOfChindren       -  The number of child device handles in ChildHandleBuffer.
    ChildHandleBuffer      -  An array of child handles to be freed.

  
  Returns:
  
    EFI_SUCCESS            -  The device was stopped.
    EFI_DEVICE_ERROR       -  The device could not be stopped due to a device error.
    EFI_NOT_STARTED        -  The device has not been started.
    EFI_INVALID_PARAMETER  -  One of the parameters has an invalid value.
    EFI_OUT_OF_RESOURCES   -  The request could not be completed due to a lack of 
                              resources.

--*/
{
  EFI_STATUS                          Status;
  UINTN                               Index;
  BOOLEAN                             AllChildrenStopped;
  ISA_IO_DEVICE                       *IsaIoDevice;
  EFI_INTERFACE_DEFINITION_FOR_ISA_IO *IsaIo;


  if (NumberOfChildren == 0) {
    //
    // Close the bus driver
    //
    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    This->DriverBindingHandle,
                    Controller
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiDevicePathProtocolGuid,
                    This->DriverBindingHandle,
                    Controller
                    );

    return Status;

  }
  //
  // Complete all outstanding transactions to Controller.
  // Don't allow any new transaction to Controller to be started.
  //
  //
  // Stop all the children
  // Find all the ISA devices that were discovered on this PCI to ISA Bridge
  // with the Start() function.
  //
  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {

    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index],
                    EFI_ISA_IO_PROTOCOL_VERSION,
                    (VOID **) &IsaIo,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {

      IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS (IsaIo);

      Status = gBS->UninstallMultipleProtocolInterfaces (
                      ChildHandleBuffer[Index],
                      &gEfiDevicePathProtocolGuid,
                      IsaIoDevice->DevicePath,
                      EFI_ISA_IO_PROTOCOL_VERSION,
                      &IsaIoDevice->IsaIo,
                      NULL
                      );

      if (!EFI_ERROR (Status)) {
        //
        // Close the child handle
        //
        Status = gBS->CloseProtocol (
                        Controller,
                        &gEfiPciIoProtocolGuid,
                        This->DriverBindingHandle,
                        ChildHandleBuffer[Index]
                        );

        gBS->FreePool (IsaIoDevice->DevicePath);
        gBS->FreePool (IsaIoDevice);
      }
    }

    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Report Status Code here
  //
  EfiLibReportStatusCode (
    EFI_PROGRESS_CODE,
    (EFI_IO_BUS_LPC | EFI_IOB_PC_DISABLE),
    0,
    &gEfiCallerIdGuid,
    NULL
    );

  return EFI_SUCCESS;
}
//
// Internal Function
//
EFI_STATUS
IsaCreateDevice (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_PCI_IO_PROTOCOL          *PciIo,
  IN EFI_DEVICE_PATH_PROTOCOL     *ParentDevicePath,
  IN EFI_ISA_ACPI_RESOURCE_LIST   *IsaDeviceResourceList,
  OUT EFI_DEVICE_PATH_PROTOCOL    **ChildDevicePath
  )
/*++

  Routine Description:
  
    Create ISA device found by IsaPnpProtocol 

  Arguments:
  
    This                   - The EFI_DRIVER_BINDING_PROTOCOL instance.
    Controller             - The handle of ISA bus controller(PCI to ISA bridge)
    PciIo                  - The Pointer to the PCI protocol 
    ParentDevicePath       - Device path of the ISA bus controller
    IsaDeviceResourceList  - The resource list of the ISA device
    ChildDevicePath        - The pointer to the child device.
  
  Returns:
  
    EFI_SUCCESS            - Create the child device.
    EFI_OUT_OF_RESOURCES   - The request could not be completed due to a lack of 
                             resources.
    EFI_DEVICE_ERROR       - Can not create child device.
    
--*/
{
  EFI_STATUS    Status;
  ISA_IO_DEVICE *IsaIoDevice;
  EFI_DEV_PATH  Node;

  //
  // Initialize the ISA_IO_DEVICE structure
  //
  IsaIoDevice = EfiLibAllocateZeroPool (sizeof (ISA_IO_DEVICE));
  if (IsaIoDevice == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  IsaIoDevice->Signature  = ISA_IO_DEVICE_SIGNATURE;
  IsaIoDevice->Handle     = NULL;
  IsaIoDevice->PciIo      = PciIo;

  //
  // Initialize the ISA I/O instance structure
  //
  Status = InitializeIsaIoInstance (IsaIoDevice, IsaDeviceResourceList);
  if (EFI_ERROR (Status)) {
    gBS->FreePool (IsaIoDevice);
    return Status;
  }
  //
  // Build the child device path
  //
  Node.DevPath.Type     = ACPI_DEVICE_PATH;
  Node.DevPath.SubType  = ACPI_DP;
  SetDevicePathNodeLength (&Node.DevPath, sizeof (ACPI_HID_DEVICE_PATH));
  Node.Acpi.HID = IsaDeviceResourceList->Device.HID;
  Node.Acpi.UID = IsaDeviceResourceList->Device.UID;

  IsaIoDevice->DevicePath = EfiAppendDevicePathNode (
                              ParentDevicePath,
                              &Node.DevPath
                              );

  if (IsaIoDevice->DevicePath == NULL) {
    Status = EFI_DEVICE_ERROR;
    goto Done;
  }

  *ChildDevicePath = IsaIoDevice->DevicePath;

  //
  // Create a child handle and attach the DevicePath,
  // PCI I/O, and Controller State
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &IsaIoDevice->Handle,
                  &gEfiDevicePathProtocolGuid,
                  IsaIoDevice->DevicePath,
                  EFI_ISA_IO_PROTOCOL_VERSION,
                  &IsaIoDevice->IsaIo,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  IsaIoDevice->Handle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    gBS->UninstallMultipleProtocolInterfaces (
           IsaIoDevice->Handle,
           &gEfiDevicePathProtocolGuid,
           IsaIoDevice->DevicePath,
           EFI_ISA_IO_PROTOCOL_VERSION,
           &IsaIoDevice->IsaIo,
           NULL
           );
  }

Done:

  if (EFI_ERROR (Status)) {
    if (IsaIoDevice->DevicePath != NULL) {
      gBS->FreePool (IsaIoDevice->DevicePath);
    }

    gBS->FreePool (IsaIoDevice);
  }

  return Status;
}
