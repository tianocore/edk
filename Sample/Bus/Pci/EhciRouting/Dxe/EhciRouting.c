/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    EhciRouting.c

Abstract: 
    Ehci Port Routing to implementation dependent classic
    host controller.

Revision History
--*/

#include "EhciRouting.h"

//
// Prototypes
// Driver model protocol interface
//
EFI_STATUS
EFIAPI
EhciRoutingDriverEntryPoint (
  IN EFI_HANDLE          ImageHandle,
  IN EFI_SYSTEM_TABLE    *SystemTable
  );

EFI_STATUS
EFIAPI
EhciRoutingDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
EhciRoutingDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
EhciRoutingDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

EFI_STATUS
EhciRoutingToClassHostController (
  IN  EFI_PCI_IO_PROTOCOL            *PciIo
  );

EFI_STATUS
TurnOffUSBEmulation (
  IN  EFI_PCI_IO_PROTOCOL            *PciIo
  );

//
// Ehci Driver Global Variables
//
EFI_DRIVER_BINDING_PROTOCOL gEhciRoutingDriverBinding = {
  EhciRoutingDriverBindingSupported,
  EhciRoutingDriverBindingStart,
  EhciRoutingDriverBindingStop,
  0xa,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT (EhciRoutingDriverEntryPoint)

EFI_STATUS
EFIAPI
EhciRoutingDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  
  Entry point for EFI drivers.

Arguments:
  
  ImageHandle - EFI_HANDLE
  SystemTable - EFI_SYSTEM_TABLE
    
Returns:
  
  EFI_SUCCESS         Success
  EFI_DEVICE_ERROR    Fail
    
--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle,
           SystemTable,
           &gEhciRoutingDriverBinding,
           ImageHandle,
           NULL,
           NULL,
           NULL
           );
}

EFI_STATUS
EFIAPI
EhciRoutingDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

  Routine Description:
  
    Test to see if this driver supports ControllerHandle.

  Arguments:
  
    This                - Protocol instance pointer.
    Controlle           - Handle of device to test
    RemainingDevicePath - Not used

  Returns:
  
    EFI_SUCCESS       This driver supports this device.
    EFI_UNSUPPORTED   This driver does not support this device.

--*/
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  USB_CLASSC          UsbClassCReg;

  //
  // Test whether there is PCI IO Protocol attached on the controller handle.
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        EHCI_PCI_CLASSC,
                        sizeof (USB_CLASSC) / sizeof (UINT8),
                        &UsbClassCReg
                        );
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
          Controller,
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_UNSUPPORTED;
  }
  //
  // Test whether the controller belongs to Ehci type
  //
  if ((UsbClassCReg.BaseCode != PCI_CLASS_SERIAL) ||
      (UsbClassCReg.SubClassCode != PCI_CLASS_SERIAL_USB) ||
      (UsbClassCReg.PI != PCI_CLASSC_PI_EHCI)
      ) {
    gBS->CloseProtocol (
          Controller,
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );

    return EFI_UNSUPPORTED;
  }

  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
EhciRoutingDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

  Routine Description:
  
    Starting the Usb EHCI Driver

  Arguments:
  
    This                - Protocol instance pointer.
    Controller          - Handle of device to test
    RemainingDevicePath - Not used

  Returns:
  
    EFI_SUCCESS           supports this device.
    EFI_UNSUPPORTED       do not support this device.
    EFI_DEVICE_ERROR      cannot be started due to device Error
    EFI_OUT_OF_RESOURCES  cannot allocate resources

--*/
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EhciRoutingToClassHostController (PciIo);
  ASSERT_EFI_ERROR (Status);

  Status = TurnOffUSBEmulation (PciIo);
  ASSERT_EFI_ERROR (Status);

  return Status;
}

EFI_STATUS
EFIAPI
EhciRoutingDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
/*++

  Routine Description:
  
    Stop this driver on ControllerHandle. Support stoping any child handles
    created by this driver.

  Arguments:
  
    This              - Protocol instance pointer.
    Controller        - Handle of device to stop driver on
    NumberOfChildren  - Number of Children in the ChildHandleBuffer
    ChildHandleBuffer - List of handles for the children we need to stop.

  Returns:
  
    EFI_SUCCESS         Success
    EFI_DEVICE_ERROR    Fail
--*/
{
  return gBS->CloseProtocol (
                Controller,
                &gEfiPciIoProtocolGuid,
                This->DriverBindingHandle,
                Controller
                );
}

EFI_STATUS
EhciRoutingToClassHostController (
  IN  EFI_PCI_IO_PROTOCOL    *PciIo
  )
/*++
  
  Routine Description:
    Initialize the Ehci to clear the ConfigFlag bit when possible
  
  Arguments:
    PciIo  The pointer to the EFI_PCI_IO_PROTOCOL
  
  Returns:
    EFI_STATUS
--*/
{
  UINT32      Data;
  UINT8       CapLength;
  EFI_STATUS  Status;

  //
  // Enable Pci master, memory space access
  //
  Status = PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationEnable,
                    EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER,
                    NULL
                    );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = PciIo->Mem.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        0,
                        EHCI_MEMORY_CAPLENGTH,
                        1,
                        &CapLength
                        );

  if (EFI_ERROR (Status)) {
    Status = PciIo->Attributes (
                      PciIo,
                      EfiPciIoAttributeOperationDisable,
                      EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER,
                      NULL
                      );

    return Status;
  }

  DEBUG ((EFI_D_ERROR, "Ehci CapLength=0x%x\n", CapLength));

  //
  // Read CONFIGFLAG;
  //
  Status = PciIo->Mem.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        CapLength + EHCI_MEMORY_CONFIGFLAG,
                        1,
                        &Data
                        );

  if (EFI_ERROR (Status)) {
    Status = PciIo->Attributes (
                      PciIo,
                      EfiPciIoAttributeOperationDisable,
                      EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER,
                      NULL
                      );

    return Status;
  }

  DEBUG ((EFI_D_ERROR, "Ehci CONFIGFLAG=0x%x\n", (UINTN)Data));

  if ((Data & 0x1) != 0) {
    DEBUG ((EFI_D_ERROR, "Ehci CONFIGFLAG has not been cleared.Clear it now\n"));

    //
    // Clear CONFIGFLAG;
    //
    Data &= 0xfffffffe;

    Status = PciIo->Mem.Write (
                          PciIo,
                          EfiPciIoWidthUint32,
                          0,
                          CapLength + EHCI_MEMORY_CONFIGFLAG,
                          1,
                          &Data
                          );
  }
  //
  // Disable the device
  //
  Status = PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationDisable,
                    EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER,
                    NULL
                    );

  return Status;

}

EFI_STATUS
TurnOffUSBEmulation (
  IN  EFI_PCI_IO_PROTOCOL    *PciIo
  )
/*++
  
  Routine Description:
    Disable USB Emulation
  
  Arguments:
    PciIo  The pointer to the EFI_PCI_IO_PROTOCOL
  
  Returns:
    EFI_STATUS
--*/
{
  UINT32      Data;
  UINT32      HCCPARAMS;
  UINT32      EECP;
  EFI_STATUS  Status;

  //
  // Enable Pci master, memory space access
  //
  Status = PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationEnable,
                    EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER,
                    NULL
                    );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = PciIo->Mem.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        EHCI_MEMORY_HCCPARAMS,
                        1,
                        &HCCPARAMS
                        );

  if (EFI_ERROR (Status)) {
    Status = PciIo->Attributes (
                      PciIo,
                      EfiPciIoAttributeOperationDisable,
                      EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER,
                      NULL
                      );
    return Status;
  }
  EECP = (HCCPARAMS >> 8) & 0xFF;

  DEBUG ((EFI_D_ERROR, "Ehci EECP=0x%x\n", (UINTN)EECP));

  if (EECP < 0x40) {
    Status = PciIo->Attributes (
                      PciIo,
                      EfiPciIoAttributeOperationDisable,
                      EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER,
                      NULL
                      );
    return Status;
  }

  //
  // Clear USB_EMU;
  //
  Data = 0;
  Status = PciIo->Pci.Write (
                         PciIo,
                         EfiPciIoWidthUint32,
                         EECP + EHCI_PCI_USB_EMU,
                         1,
                         &Data
                         );

  //
  // Disable the device
  //
  Status = PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationDisable,
                    EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER,
                    NULL
                    );

  return Status;
}
