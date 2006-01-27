/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  BiosSnp16.c
    
Abstract:

  ConsoleOut Routines that speak VGA.

Revision History

--*/

#include "BiosSnp16.h"

//
// EFI Driver Binding Protocol Instance
//
//   This driver has a version value of 0x00000000.  This is the
//   lowest possible priority for a driver.  This is done on purpose to help
//   the developers of native UNDI drivers.  This driver can bind if no UNDI driver
//   is present, so a network connection is available.  Then, when a UNDI driver is
//   loaded this driver can be disconnected, and the UNDI driver can be connected.
//   As long as the UNDI driver has a version value greater than 0x00000000, it
//   will be connected first and will block this driver from connecting.
//
EFI_DRIVER_BINDING_PROTOCOL gBiosSnp16DriverBinding = {
  BiosSnp16DriverBindingSupported,
  BiosSnp16DriverBindingStart,
  BiosSnp16DriverBindingStop,
  0x00000000,
  NULL,
  NULL
};

//
// Private worker functions;
//
STATIC
EFI_STATUS
Undi16SimpleNetworkStartUndi (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice,
  UINT16                  AX
  );

STATIC
EFI_STATUS
Undi16SimpleNetworkStopUndi (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice
  );

STATIC
EFI_STATUS
Undi16SimpleNetworkCleanupUndi (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice
  );

STATIC
EFI_STATUS
Undi16SimpleNetworkGetInformation (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  );

STATIC
EFI_STATUS
Undi16SimpleNetworkGetNicType (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  );

STATIC
EFI_STATUS
Undi16SimpleNetworkGetNdisInfo (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  );

//
// Driver Entry Point
//
EFI_DRIVER_ENTRY_POINT (BiosSnp16DriverEntryPoint)

EFI_STATUS
BiosSnp16DriverEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++
  
  Routine Description:
    Driver Entry Point.
        
  Arguments:
    (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)
  
  Returns:
    EFI_STATUS
--*/
// GC_TODO:    ImageHandle - add argument and description to function comment
// GC_TODO:    SystemTable - add argument and description to function comment
{
  return EfiLibInstallAllDriverProtocols (
          ImageHandle,
          SystemTable,
          &gBiosSnp16DriverBinding,
          ImageHandle,
          &gBiosSnp16ComponentName,
          NULL,
          NULL
          );
}
//
// EFI Driver Binding Protocol Functions
//
EFI_STATUS
BiosSnp16DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Supported.
    
  Arguments:
    (Standard DriverBinding Protocol Supported() function)
    
  Returns:
    EFI_STATUS
  
--*/
// GC_TODO:    This - add argument and description to function comment
// GC_TODO:    Controller - add argument and description to function comment
// GC_TODO:    RemainingDevicePath - add argument and description to function comment
{
  EFI_STATUS                  Status;
  LEGACY_BIOS_THUNK_PROTOCOL  *LegacyBiosThunk;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  PCI_TYPE00                  Pci;

  //
  // See if the Legacy BIOS Protocol is available
  //
  Status = gBS->LocateProtocol (&gLegacyBiosThunkProtocolGuid, NULL, (VOID **) &LegacyBiosThunk);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
        Controller,
        &gEfiDevicePathProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  //
  // Open the IO Abstraction(s) needed to perform the supported test
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
  // See if this is a PCI Network Controller by looking at the Command register and
  // Class Code Register
  //
  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
  if (EFI_ERROR (Status)) {
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  Status = EFI_UNSUPPORTED;
  if (Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) {
    Status = EFI_SUCCESS;
  }

Done:
  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

EFI_STATUS
BiosSnp16DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Install VGA Mini Port Protocol onto VGA device handles
  
  Arguments:
    (Standard DriverBinding Protocol Start() function)
    
  Returns:
    EFI_STATUS
    
--*/
// GC_TODO:    This - add argument and description to function comment
// GC_TODO:    Controller - add argument and description to function comment
// GC_TODO:    RemainingDevicePath - add argument and description to function comment
{
  EFI_STATUS                  Status;
  LEGACY_BIOS_THUNK_PROTOCOL  *LegacyBiosThunk;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  EFI_SIMPLE_NETWORK_DEV      *SimpleNetworkDevice;
  EFI_DEV_PATH                Node;
  UINTN                       Index;
  UINTN                       j;
  UINTN                       Segment;
  UINTN                       Bus;
  UINTN                       Device;
  UINTN                       Function;

  SimpleNetworkDevice = NULL;
  PciIo               = NULL;

  //
  // See if the Legacy BIOS Protocol is available
  //
  Status = gBS->LocateProtocol (&gLegacyBiosThunkProtocolGuid, NULL, (VOID **) &LegacyBiosThunk);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Open the IO Abstraction(s) needed
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationEnable,
                    EFI_PCI_DEVICE_ENABLE,
                    NULL
                    );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Allocate memory for this SimpleNetwork device instance
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_SIMPLE_NETWORK_DEV),
                  &SimpleNetworkDevice
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  EfiZeroMem (SimpleNetworkDevice, sizeof (EFI_SIMPLE_NETWORK_DEV));

  //
  // Initialize the SimpleNetwork device instance
  //
  SimpleNetworkDevice->Signature        = EFI_SIMPLE_NETWORK_DEV_SIGNATURE;
  SimpleNetworkDevice->LegacyBiosThunk  = LegacyBiosThunk;
  SimpleNetworkDevice->BaseDevicePath   = DevicePath;
  SimpleNetworkDevice->PciIo            = PciIo;

  //
  // Initialize the NII Protocol
  //
  SimpleNetworkDevice->NII.Revision = EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL_REVISION;
  SimpleNetworkDevice->NII.Type     = EfiNetworkInterfaceUndi;

  EfiCopyMem (&SimpleNetworkDevice->NII.StringId, "UNDI", 4);

  //
  // Load 16 bit UNDI Option ROM into Memory
  //
  Status = Undi16SimpleNetworkLoadUndi (SimpleNetworkDevice);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_NET, "ERROR : Could not load UNDI.  Status = %r\n", Status));
    goto Done;
  }

  SimpleNetworkDevice->UndiLoaded = TRUE;

  //
  // Call PXENV_START_UNDI - Initilizes the UNID interface for use.
  //
  PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
  Status = Undi16SimpleNetworkStartUndi (
            SimpleNetworkDevice,
            (UINT16) ((Bus << 0x8) | (Device << 0x3) | (Function))
            );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_NET, "ERROR : Could not StartUndi.  Status = %r\n", Status));
    goto Done;
  }
  //
  // Initialize the Simple Network Protocol
  //
  DEBUG ((EFI_D_NET, "Initialize SimpleNetworkDevice instance\n"));

  SimpleNetworkDevice->SimpleNetwork.Revision       = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
  SimpleNetworkDevice->SimpleNetwork.Start          = Undi16SimpleNetworkStart;
  SimpleNetworkDevice->SimpleNetwork.Stop           = Undi16SimpleNetworkStop;
  SimpleNetworkDevice->SimpleNetwork.Initialize     = Undi16SimpleNetworkInitialize;
  SimpleNetworkDevice->SimpleNetwork.Reset          = Undi16SimpleNetworkReset;
  SimpleNetworkDevice->SimpleNetwork.Shutdown       = Undi16SimpleNetworkShutdown;
  SimpleNetworkDevice->SimpleNetwork.ReceiveFilters = Undi16SimpleNetworkReceiveFilters;
  SimpleNetworkDevice->SimpleNetwork.StationAddress = Undi16SimpleNetworkStationAddress;
  SimpleNetworkDevice->SimpleNetwork.Statistics     = Undi16SimpleNetworkStatistics;
  SimpleNetworkDevice->SimpleNetwork.MCastIpToMac   = Undi16SimpleNetworkMCastIpToMac;
  SimpleNetworkDevice->SimpleNetwork.NvData         = Undi16SimpleNetworkNvData;
  SimpleNetworkDevice->SimpleNetwork.GetStatus      = Undi16SimpleNetworkGetStatus;
  SimpleNetworkDevice->SimpleNetwork.Transmit       = Undi16SimpleNetworkTransmit;
  SimpleNetworkDevice->SimpleNetwork.Receive        = Undi16SimpleNetworkReceive;
  SimpleNetworkDevice->SimpleNetwork.Mode           = &(SimpleNetworkDevice->SimpleNetworkMode);

  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_WAIT,
                  EFI_TPL_NOTIFY,
                  Undi16SimpleNetworkWaitForPacket,
                  &SimpleNetworkDevice->SimpleNetwork,
                  &SimpleNetworkDevice->SimpleNetwork.WaitForPacket
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ERROR : Could not create event.  Status = %r\n", Status));
    goto Done;
  }
  //
  // Create an event to be signalled when ExitBootServices occurs in order
  // to clean up nicely
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_SIGNAL_EXIT_BOOT_SERVICES,
                  EFI_TPL_NOTIFY,
                  Undi16SimpleNetworkEvent,
                  NULL,
                  &SimpleNetworkDevice->Event
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ERROR : Could not create event.  Status = %r\n", Status));
    goto Done;
  }
  //
  // Initialize the SimpleNetwork Mode Information
  //
  DEBUG ((EFI_D_NET, "Initialize Mode Information\n"));

  SimpleNetworkDevice->SimpleNetworkMode.State                = EfiSimpleNetworkStopped;
  SimpleNetworkDevice->SimpleNetworkMode.MediaHeaderSize      = 14;
  SimpleNetworkDevice->SimpleNetworkMode.MacAddressChangeable = TRUE;
  SimpleNetworkDevice->SimpleNetworkMode.MultipleTxSupported  = TRUE;
  SimpleNetworkDevice->SimpleNetworkMode.ReceiveFilterMask = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST |
    EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST |
    EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST |
    EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS |
    EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;
  SimpleNetworkDevice->SimpleNetworkMode.MaxMCastFilterCount = MAXNUM_MCADDR;

  //
  // Initialize the SimpleNetwork Private Information
  //
  DEBUG ((EFI_D_NET, "Initialize Private Information\n"));

  Status = BiosSnp16AllocatePagesBelowOneMb (
            sizeof (PXENV_UNDI_TBD_t) / EFI_PAGE_SIZE + 1,
            &SimpleNetworkDevice->Xmit
            );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = BiosSnp16AllocatePagesBelowOneMb (
            1,
            &SimpleNetworkDevice->TxRealModeMediaHeader
            );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = BiosSnp16AllocatePagesBelowOneMb (
            1,
            &SimpleNetworkDevice->TxRealModeDataBuffer
            );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = BiosSnp16AllocatePagesBelowOneMb (
            1,
            &SimpleNetworkDevice->TxDestAddr
            );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  SimpleNetworkDevice->Xmit->XmitOffset               = (UINT16) (((UINTN) SimpleNetworkDevice->TxRealModeMediaHeader) & 0x000f);

  SimpleNetworkDevice->Xmit->XmitSegment              = (UINT16) (((UINTN) SimpleNetworkDevice->TxRealModeMediaHeader) >> 4);

  SimpleNetworkDevice->Xmit->DataBlkCount             = 1;

  SimpleNetworkDevice->Xmit->DataBlock[0].TDPtrType   = 1;
  SimpleNetworkDevice->Xmit->DataBlock[0].TDRsvdByte  = 0;

  SimpleNetworkDevice->Xmit->DataBlock[0].TDDataPtrOffset = (UINT16) (((UINTN) SimpleNetworkDevice->TxRealModeDataBuffer) & 0x000f);

  SimpleNetworkDevice->Xmit->DataBlock[0].TDDataPtrSegment = (UINT16) (((UINTN) SimpleNetworkDevice->TxRealModeDataBuffer) >> 4);

  SimpleNetworkDevice->TxBufferFifo.First = 0;
  SimpleNetworkDevice->TxBufferFifo.Last  = 0;

  //
  // Start() the SimpleNetwork device
  //
  DEBUG ((EFI_D_NET, "Start()\n"));

  Status = Undi16SimpleNetworkStart (&SimpleNetworkDevice->SimpleNetwork);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // GetInformation() the SimpleNetwork device
  //
  DEBUG ((EFI_D_NET, "GetInformation()\n"));

  Status = Undi16SimpleNetworkGetInformation (&SimpleNetworkDevice->SimpleNetwork);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Build the device path for the child device
  //
  EfiZeroMem (&Node, sizeof (Node));
  Node.DevPath.Type     = MESSAGING_DEVICE_PATH;
  Node.DevPath.SubType  = MSG_MAC_ADDR_DP;
  SetDevicePathNodeLength (&Node.DevPath, sizeof (MAC_ADDR_DEVICE_PATH));
  EfiCopyMem (
    &Node.MacAddr.MacAddress,
    &SimpleNetworkDevice->SimpleNetworkMode.CurrentAddress,
    sizeof (EFI_MAC_ADDRESS)
    );
  SimpleNetworkDevice->DevicePath = EfiAppendDevicePathNode (
                                      SimpleNetworkDevice->BaseDevicePath,
                                      &Node.DevPath
                                      );

  //
  // GetNicType()  the SimpleNetwork device
  //
  DEBUG ((EFI_D_NET, "GetNicType()\n"));

  Status = Undi16SimpleNetworkGetNicType (&SimpleNetworkDevice->SimpleNetwork);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // GetNdisInfo() the SimpleNetwork device
  //
  DEBUG ((EFI_D_NET, "GetNdisInfo()\n"));

  Status = Undi16SimpleNetworkGetNdisInfo (&SimpleNetworkDevice->SimpleNetwork);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Stop() the SimpleNetwork device
  //
  DEBUG ((EFI_D_NET, "Stop()\n"));

  Status = SimpleNetworkDevice->SimpleNetwork.Stop (&SimpleNetworkDevice->SimpleNetwork);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Print Mode information
  //
  DEBUG ((EFI_D_NET, "Mode->State                = %d\n", SimpleNetworkDevice->SimpleNetworkMode.State));
  DEBUG ((EFI_D_NET, "Mode->HwAddressSize        = %d\n", SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize));
  DEBUG ((EFI_D_NET, "Mode->MacAddressChangeable = %d\n", SimpleNetworkDevice->SimpleNetworkMode.MacAddressChangeable));
  DEBUG ((EFI_D_NET, "Mode->MultiplTxSupported   = %d\n", SimpleNetworkDevice->SimpleNetworkMode.MultipleTxSupported));
  DEBUG ((EFI_D_NET, "Mode->NvRamSize            = %d\n", SimpleNetworkDevice->SimpleNetworkMode.NvRamSize));
  DEBUG ((EFI_D_NET, "Mode->NvRamAccessSize      = %d\n", SimpleNetworkDevice->SimpleNetworkMode.NvRamAccessSize));
  DEBUG ((EFI_D_NET, "Mode->ReceiveFilterSetting = %d\n", SimpleNetworkDevice->SimpleNetworkMode.ReceiveFilterSetting));
  DEBUG ((EFI_D_NET, "Mode->IfType               = %d\n", SimpleNetworkDevice->SimpleNetworkMode.IfType));
  DEBUG ((EFI_D_NET, "Mode->MCastFilterCount     = %d\n", SimpleNetworkDevice->SimpleNetworkMode.MCastFilterCount));
  for (Index = 0; Index < SimpleNetworkDevice->SimpleNetworkMode.MCastFilterCount; Index++) {
    DEBUG ((EFI_D_NET, "  Filter[%02d] = ", Index));
    for (j = 0; j < 16; j++) {
      DEBUG ((EFI_D_NET, "%02x ", SimpleNetworkDevice->SimpleNetworkMode.MCastFilter[Index].Addr[j]));
    }

    DEBUG ((EFI_D_NET, "\n"));
  }

  DEBUG ((EFI_D_NET, "CurrentAddress = "));
  for (j = 0; j < 16; j++) {
    DEBUG ((EFI_D_NET, "%02x ", SimpleNetworkDevice->SimpleNetworkMode.CurrentAddress.Addr[j]));
  }

  DEBUG ((EFI_D_NET, "\n"));

  DEBUG ((EFI_D_NET, "BroadcastAddress = "));
  for (j = 0; j < 16; j++) {
    DEBUG ((EFI_D_NET, "%02x ", SimpleNetworkDevice->SimpleNetworkMode.BroadcastAddress.Addr[j]));
  }

  DEBUG ((EFI_D_NET, "\n"));

  DEBUG ((EFI_D_NET, "PermanentAddress = "));
  for (j = 0; j < 16; j++) {
    DEBUG ((EFI_D_NET, "%02x ", SimpleNetworkDevice->SimpleNetworkMode.PermanentAddress.Addr[j]));
  }

  DEBUG ((EFI_D_NET, "\n"));

  //
  // The network device was started, information collected, and stopped.
  // Install protocol interfaces for the SimpleNetwork device.
  //
  DEBUG ((EFI_D_NET, "Install Protocol Interfaces on network interface\n"));

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &SimpleNetworkDevice->Handle,
                  &gEfiSimpleNetworkProtocolGuid,
                  &SimpleNetworkDevice->SimpleNetwork,
                  &gEfiNetworkInterfaceIdentifierProtocolGuid,
                  &SimpleNetworkDevice->NII,
                  &gEfiDevicePathProtocolGuid,
                  SimpleNetworkDevice->DevicePath,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Open PCI I/O from the newly created child handle
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  SimpleNetworkDevice->Handle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );

  DEBUG ((EFI_D_INIT, "UNDI16 Driver : EFI_SUCCESS\n"));

Done:
  if (EFI_ERROR (Status)) {
    if (SimpleNetworkDevice) {

      Undi16SimpleNetworkShutdown (&SimpleNetworkDevice->SimpleNetwork);
      //
      // CLOSE + SHUTDOWN
      //
      Undi16SimpleNetworkCleanupUndi (SimpleNetworkDevice);
      //
      // CLEANUP
      //
      Undi16SimpleNetworkStopUndi (SimpleNetworkDevice);
      //
      // STOP
      //
      if (SimpleNetworkDevice->UndiLoaded) {
        Undi16SimpleNetworkUnloadUndi (SimpleNetworkDevice);
      }

      if (SimpleNetworkDevice->SimpleNetwork.WaitForPacket) {
        gBS->CloseEvent (SimpleNetworkDevice->SimpleNetwork.WaitForPacket);
      }

      if (SimpleNetworkDevice->Event) {
        gBS->CloseEvent (SimpleNetworkDevice->Event);
      }

      if (SimpleNetworkDevice->Xmit) {
        gBS->FreePages (
              (EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->Xmit,
              sizeof (PXENV_UNDI_TBD_t) / EFI_PAGE_SIZE + 1
              );
      }

      if (SimpleNetworkDevice->TxRealModeMediaHeader) {
        gBS->FreePages ((EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->TxRealModeMediaHeader, 1);
      }

      if (SimpleNetworkDevice->TxRealModeDataBuffer) {
        gBS->FreePages ((EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->TxRealModeDataBuffer, 1);
      }

      if (SimpleNetworkDevice->TxDestAddr) {
        gBS->FreePages ((EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->TxDestAddr, 1);
      }

      gBS->FreePool (SimpleNetworkDevice);
    }

    if (PciIo) {
      PciIo->Attributes (
              PciIo,
              EfiPciIoAttributeOperationDisable,
              EFI_PCI_DEVICE_ENABLE,
              NULL
              );
    }

    gBS->CloseProtocol (
          Controller,
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );

    gBS->CloseProtocol (
          Controller,
          &gEfiDevicePathProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
  }

  return Status;
}

EFI_STATUS
BiosSnp16DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++
  
  Routine Description:
    Stop.
  
  Arguments:
    (Standard DriverBinding Protocol Stop() function)
  
  Returns:
    EFI_STATUS
  
--*/
// GC_TODO:    This - add argument and description to function comment
// GC_TODO:    Controller - add argument and description to function comment
// GC_TODO:    NumberOfChildren - add argument and description to function comment
// GC_TODO:    ChildHandleBuffer - add argument and description to function comment
// GC_TODO:    EFI_DEVICE_ERROR - add return value to function comment
// GC_TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS                  Status;
  UINTN                       Index;
  BOOLEAN                     AllChildrenStopped;
  EFI_SIMPLE_NETWORK_PROTOCOL *SimpleNetwork;
  EFI_SIMPLE_NETWORK_DEV      *SimpleNetworkDevice;
  EFI_PCI_IO_PROTOCOL         *PciIo;

  //
  // Complete all outstanding transactions to Controller.
  // Don't allow any new transaction to Controller to be started.
  //
  if (NumberOfChildren == 0) {
    //
    // Close the bus driver
    //
    Status = gBS->OpenProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {
      PciIo->Attributes (
              PciIo,
              EfiPciIoAttributeOperationDisable,
              EFI_PCI_DEVICE_ENABLE,
              NULL
              );
    }

    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    This->DriverBindingHandle,
                    Controller
                    );

    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiDevicePathProtocolGuid,
                    This->DriverBindingHandle,
                    Controller
                    );
    return Status;
  }

  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {

    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index],
                    &gEfiSimpleNetworkProtocolGuid,
                    (VOID **) &SimpleNetwork,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {

      SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (SimpleNetwork);

      Status = gBS->CloseProtocol (
                      Controller,
                      &gEfiPciIoProtocolGuid,
                      This->DriverBindingHandle,
                      ChildHandleBuffer[Index]
                      );

      Status = gBS->UninstallMultipleProtocolInterfaces (
                      SimpleNetworkDevice->Handle,
                      &gEfiSimpleNetworkProtocolGuid,
                      &SimpleNetworkDevice->SimpleNetwork,
                      &gEfiNetworkInterfaceIdentifierProtocolGuid,
                      &SimpleNetworkDevice->NII,
                      &gEfiDevicePathProtocolGuid,
                      SimpleNetworkDevice->DevicePath,
                      NULL
                      );
      if (EFI_ERROR (Status)) {
        gBS->OpenProtocol (
              Controller,
              &gEfiPciIoProtocolGuid,
              (VOID **) &PciIo,
              This->DriverBindingHandle,
              ChildHandleBuffer[Index],
              EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
              );
      } else {

        Undi16SimpleNetworkShutdown (&SimpleNetworkDevice->SimpleNetwork);
        //
        // CLOSE + SHUTDOWN
        //
        Undi16SimpleNetworkCleanupUndi (SimpleNetworkDevice);
        //
        // CLEANUP
        //
        Undi16SimpleNetworkStopUndi (SimpleNetworkDevice);
        //
        // STOP
        //
        if (SimpleNetworkDevice->UndiLoaded) {
          Undi16SimpleNetworkUnloadUndi (SimpleNetworkDevice);
        }

        if (SimpleNetworkDevice->SimpleNetwork.WaitForPacket) {
          gBS->CloseEvent (SimpleNetworkDevice->SimpleNetwork.WaitForPacket);
        }

        if (SimpleNetworkDevice->Event) {
          gBS->CloseEvent (SimpleNetworkDevice->Event);
        }

        if (SimpleNetworkDevice->Xmit) {
          gBS->FreePages (
                (EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->Xmit,
                sizeof (PXENV_UNDI_TBD_t) / EFI_PAGE_SIZE + 1
                );
        }

        if (SimpleNetworkDevice->TxRealModeMediaHeader) {
          gBS->FreePages ((EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->TxRealModeMediaHeader, 1);
        }

        if (SimpleNetworkDevice->TxRealModeDataBuffer) {
          gBS->FreePages ((EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->TxRealModeDataBuffer, 1);
        }

        if (SimpleNetworkDevice->TxDestAddr) {
          gBS->FreePages ((EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->TxDestAddr, 1);
        }

        gBS->FreePool (SimpleNetworkDevice);
      }

    }

    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
//
// FIFO Support Functions
//
STATIC
BOOLEAN
SimpleNetworkTransmitFifoFull (
  EFI_SIMPLE_NETWORK_DEV_FIFO *Fifo
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Fifo  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  if (((Fifo->Last + 1) % EFI_SIMPLE_NETWORK_MAX_TX_FIFO_SIZE) == Fifo->First) {
    return TRUE;
  }

  return FALSE;
}
//
//
//
STATIC
BOOLEAN
SimpleNetworkTransmitFifoEmpty (
  EFI_SIMPLE_NETWORK_DEV_FIFO *Fifo
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Fifo  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  if (Fifo->Last == Fifo->First) {
    return TRUE;
  }

  return FALSE;
}
//
//
//
STATIC
EFI_STATUS
SimpleNetworkTransmitFifoAdd (
  EFI_SIMPLE_NETWORK_DEV_FIFO *Fifo,
  VOID                        *Data
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Fifo  - GC_TODO: add argument description
  Data  - GC_TODO: add argument description

Returns:

  EFI_OUT_OF_RESOURCES - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  if (SimpleNetworkTransmitFifoFull (Fifo)) {
    return EFI_OUT_OF_RESOURCES;
  }

  Fifo->Data[Fifo->Last]  = Data;
  Fifo->Last              = (Fifo->Last + 1) % EFI_SIMPLE_NETWORK_MAX_TX_FIFO_SIZE;
  return EFI_SUCCESS;
}
//
//
//
STATIC
EFI_STATUS
SimpleNetworkTransmitFifoRemove (
  EFI_SIMPLE_NETWORK_DEV_FIFO *Fifo,
  VOID                        **Data
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Fifo  - GC_TODO: add argument description
  Data  - GC_TODO: add argument description

Returns:

  EFI_OUT_OF_RESOURCES - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  if (SimpleNetworkTransmitFifoEmpty (Fifo)) {
    return EFI_OUT_OF_RESOURCES;
  }

  *Data       = Fifo->Data[Fifo->First];
  Fifo->First = (Fifo->First + 1) % EFI_SIMPLE_NETWORK_MAX_TX_FIFO_SIZE;
  return EFI_SUCCESS;
}
//
// Maps from EFI Receive Packet Filter Settings to UNDI 16 Receive Packet Filter Settings
//
STATIC
UINT16
Undi16GetPacketFilterSetting (
  UINTN ReceiveFilterSetting
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ReceiveFilterSetting  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  UINT16  PktFilter;

  PktFilter = 0;
  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_UNICAST) {
    PktFilter |= FLTR_DIRECTED;
  }

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) {
    PktFilter |= FLTR_DIRECTED;
  }

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST) {
    PktFilter |= FLTR_BRDCST;
  }

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS) {
    PktFilter |= FLTR_PRMSCS;
  }

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST) {
    PktFilter |= FLTR_PRMSCS;
    //
    // BugBug : Do not know if this is right????
    //
  }
  //
  // BugBug : What is FLTR_SRC_RTG?
  //
  return PktFilter;
}
//
// Map EFI Multicast filters to UNDI 16 Multicast Filters
//
VOID
Undi16GetMCastFilters (
  IN EFI_SIMPLE_NETWORK_MODE      *Mode,
  IN OUT PXENV_UNDI_MCAST_ADDR_t  *McastBuffer,
  IN UINTN                        HwAddressSize
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Mode          - GC_TODO: add argument description
  McastBuffer   - GC_TODO: add argument description
  HwAddressSize - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  UINTN Index;

  //
  // BugBug : What if Mode->MCastFilterCount > MAXNUM_MCADDR?
  //
  McastBuffer->MCastAddrCount = (UINT16) Mode->MCastFilterCount;
  for (Index = 0; Index < MAXNUM_MCADDR; Index++) {
    if (Index < McastBuffer->MCastAddrCount) {
      EfiCopyMem (&McastBuffer->MCastAddr[Index], &Mode->MCastFilter[Index], HwAddressSize);
    } else {
      EfiZeroMem (&McastBuffer->MCastAddr[Index], HwAddressSize);
    }
  }
}
//
// Load 16 bit UNDI Option ROM into memory
//
STATIC
EFI_STATUS
Undi16SimpleNetworkLoadUndi (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_NOT_FOUND - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS                Status;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  UINTN                     RomAddress;
  PCI_EXPANSION_ROM_HEADER  *PciExpansionRomHeader;
  PCI_DATA_STRUCTURE        *PciDataStructure;
  PCI_TYPE00                Pci;

  CacheVectorAddress (0x1A);

  PciIo = SimpleNetworkDevice->PciIo;

  PciIo->Pci.Read (
              PciIo,
              EfiPciIoWidthUint32,
              0,
              sizeof (Pci) / sizeof (UINT32),
              &Pci
              );

  for (RomAddress = 0xc0000; RomAddress < 0xfffff; RomAddress += 0x800) {

    PciExpansionRomHeader = (PCI_EXPANSION_ROM_HEADER *) RomAddress;

    if (PciExpansionRomHeader->Signature != PCI_EXPANSION_ROM_HEADER_SIGNATURE) {
      continue;
    }

    DEBUG ((EFI_D_INIT, "Option ROM found at %X\n", RomAddress));

    PciDataStructure = (PCI_DATA_STRUCTURE *) (RomAddress + PciExpansionRomHeader->PcirOffset);

    if (PciDataStructure->Signature != PCI_DATA_STRUCTURE_SIGNATURE) {
      continue;
    }

    DEBUG ((EFI_D_INIT, "PCI Data Structure found at %X\n", PciDataStructure));

    if (PciDataStructure->VendorId != Pci.Hdr.VendorId || PciDataStructure->DeviceId != Pci.Hdr.DeviceId) {
      continue;
    }

    DEBUG ((EFI_D_INIT, "PCI device with matchinng VendorId and DeviceId (%d,%d,%d)\n"));

    Status = LaunchBaseCode (SimpleNetworkDevice, RomAddress);

    if (!EFI_ERROR (Status)) {
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}
//
// Unload 16 bit UNDI Option ROM from memory
//
STATIC
EFI_STATUS
Undi16SimpleNetworkUnloadUndi (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  if (SimpleNetworkDevice->UndiLoaderTable) {
    EfiZeroMem (SimpleNetworkDevice->UndiLoaderTable, SimpleNetworkDevice->UndiLoaderTablePages << EFI_PAGE_SIZE);
    gBS->FreePages (
          (EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->UndiLoaderTable,
          SimpleNetworkDevice->UndiLoaderTablePages
          );
  }

  if (SimpleNetworkDevice->DestinationDataSegment) {
    EfiZeroMem (
      SimpleNetworkDevice->DestinationDataSegment,
      SimpleNetworkDevice->DestinationDataSegmentPages << EFI_PAGE_SIZE
      );
    gBS->FreePages (
          (EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->DestinationDataSegment,
          SimpleNetworkDevice->DestinationDataSegmentPages
          );
  }

  if (SimpleNetworkDevice->DestinationStackSegment) {
    EfiZeroMem (
      SimpleNetworkDevice->DestinationStackSegment,
      SimpleNetworkDevice->DestinationStackSegmentPages << EFI_PAGE_SIZE
      );
    gBS->FreePages (
          (EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->DestinationStackSegment,
          SimpleNetworkDevice->DestinationStackSegmentPages
          );
  }

  if (SimpleNetworkDevice->DestinationCodeSegment) {
    EfiZeroMem (
      SimpleNetworkDevice->DestinationCodeSegment,
      SimpleNetworkDevice->DestinationCodeSegmentPages << EFI_PAGE_SIZE
      );
    gBS->FreePages (
          (EFI_PHYSICAL_ADDRESS) (UINTN) SimpleNetworkDevice->DestinationCodeSegment,
          SimpleNetworkDevice->DestinationCodeSegmentPages
          );
  }

  return EFI_SUCCESS;
}
//
// Start the UNDI interface
//
STATIC
EFI_STATUS
Undi16SimpleNetworkStartUndi (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice,
  UINT16                  AX
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description
  AX                  - GC_TODO: add argument description

Returns:

  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS          Status;
  PXENV_START_UNDI_t  Start;

  //
  // Call 16 bit UNDI ROM to start the network interface
  //
  //
  // BugBug : What is this state supposed to be???
  //
  Start.Status  = INIT_PXE_STATUS;
  Start.ax      = AX;
  Start.bx      = 0x0000;
  Start.dx      = 0x0000;
  Start.di      = 0x0000;
  Start.es      = 0x0000;

  Status        = PxeStartUndi (SimpleNetworkDevice, &Start);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (Start.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }

  return Status;
}
//
// Stop the UNDI interface
//
STATIC
EFI_STATUS
Undi16SimpleNetworkStopUndi (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description

Returns:

  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS        Status;
  PXENV_STOP_UNDI_t Stop;

  //
  // Call 16 bit UNDI ROM to start the network interface
  //
  Stop.Status = INIT_PXE_STATUS;

  Status      = PxeUndiStop (SimpleNetworkDevice, &Stop);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (Stop.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }

  return Status;
}

STATIC
EFI_STATUS
Undi16SimpleNetworkCleanupUndi (
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  SimpleNetworkDevice - GC_TODO: add argument description

Returns:

  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS            Status;
  PXENV_UNDI_CLEANUP_t  Cleanup;

  //
  // Call 16 bit UNDI ROM to cleanup the network interface
  //
  Cleanup.Status  = INIT_PXE_STATUS;

  Status          = PxeUndiCleanup (SimpleNetworkDevice, &Cleanup);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (Cleanup.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }

  return Status;
}
//
// GetInformation()
//
STATIC
EFI_STATUS
Undi16SimpleNetworkGetInformation (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;
  UINTN                   Index;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkStarted:
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  default:
    return EFI_DEVICE_ERROR;
  }
  //
  // Call 16 bit UNDI ROM to start the network interface
  //
  EfiZeroMem (&SimpleNetworkDevice->GetInformation, sizeof (PXENV_UNDI_GET_INFORMATION_t));

  SimpleNetworkDevice->GetInformation.Status  = INIT_PXE_STATUS;

  Status = PxeUndiGetInformation (SimpleNetworkDevice, &SimpleNetworkDevice->GetInformation);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((EFI_D_NET, "  GetInformation.Status      = %d\n", SimpleNetworkDevice->GetInformation.Status));
  DEBUG ((EFI_D_NET, "  GetInformation.BaseIo      = %d\n", SimpleNetworkDevice->GetInformation.BaseIo));
  DEBUG ((EFI_D_NET, "  GetInformation.IntNumber   = %d\n", SimpleNetworkDevice->GetInformation.IntNumber));
  DEBUG ((EFI_D_NET, "  GetInformation.MaxTranUnit = %d\n", SimpleNetworkDevice->GetInformation.MaxTranUnit));
  DEBUG ((EFI_D_NET, "  GetInformation.HwType      = %d\n", SimpleNetworkDevice->GetInformation.HwType));
  DEBUG ((EFI_D_NET, "  GetInformation.HwAddrLen   = %d\n", SimpleNetworkDevice->GetInformation.HwAddrLen));
  DEBUG ((EFI_D_NET, "  GetInformation.ROMAddress  = %d\n", SimpleNetworkDevice->GetInformation.ROMAddress));
  DEBUG ((EFI_D_NET, "  GetInformation.RxBufCt     = %d\n", SimpleNetworkDevice->GetInformation.RxBufCt));
  DEBUG ((EFI_D_NET, "  GetInformation.TxBufCt     = %d\n", SimpleNetworkDevice->GetInformation.TxBufCt));

  DEBUG ((EFI_D_NET, "  GetInformation.CurNodeAddr ="));
  for (Index = 0; Index < 16; Index++) {
    DEBUG ((EFI_D_NET, "%02x ", SimpleNetworkDevice->GetInformation.CurrentNodeAddress[Index]));
  }

  DEBUG ((EFI_D_NET, "\n"));

  DEBUG ((EFI_D_NET, "  GetInformation.PermNodeAddr ="));
  for (Index = 0; Index < 16; Index++) {
    DEBUG ((EFI_D_NET, "%02x ", SimpleNetworkDevice->GetInformation.PermNodeAddress[Index]));
  }

  DEBUG ((EFI_D_NET, "\n"));

  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (SimpleNetworkDevice->GetInformation.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }
  //
  // The information has been retrieved.  Fill in Mode data.
  //
  SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize  = SimpleNetworkDevice->GetInformation.HwAddrLen;

  SimpleNetworkDevice->SimpleNetworkMode.MaxPacketSize  = SimpleNetworkDevice->GetInformation.MaxTranUnit;

  SimpleNetworkDevice->SimpleNetworkMode.IfType         = (UINT8) SimpleNetworkDevice->GetInformation.HwType;

  EfiZeroMem (
    &SimpleNetworkDevice->SimpleNetworkMode.CurrentAddress,
    sizeof SimpleNetworkDevice->SimpleNetworkMode.CurrentAddress
    );

  EfiCopyMem (
    &SimpleNetworkDevice->SimpleNetworkMode.CurrentAddress,
    &SimpleNetworkDevice->GetInformation.CurrentNodeAddress,
    SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
    );

  EfiZeroMem (
    &SimpleNetworkDevice->SimpleNetworkMode.PermanentAddress,
    sizeof SimpleNetworkDevice->SimpleNetworkMode.PermanentAddress
    );

  EfiCopyMem (
    &SimpleNetworkDevice->SimpleNetworkMode.PermanentAddress,
    &SimpleNetworkDevice->GetInformation.PermNodeAddress,
    SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
    );

  //
  // hard code broadcast address - not avail in PXE2.1
  //
  EfiZeroMem (
    &SimpleNetworkDevice->SimpleNetworkMode.BroadcastAddress,
    sizeof SimpleNetworkDevice->SimpleNetworkMode.BroadcastAddress
    );

  EfiSetMem (
    &SimpleNetworkDevice->SimpleNetworkMode.BroadcastAddress,
    SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize,
    0xff
    );

  return Status;
}

STATIC
EFI_STATUS
Undi16SimpleNetworkGetNicType (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }

  EfiZeroMem (&SimpleNetworkDevice->GetNicType, sizeof (PXENV_UNDI_GET_NIC_TYPE_t));

  SimpleNetworkDevice->GetNicType.Status  = INIT_PXE_STATUS;

  Status = PxeUndiGetNicType (SimpleNetworkDevice, &SimpleNetworkDevice->GetNicType);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((EFI_D_NET, "  GetNicType.Status      = %d\n", SimpleNetworkDevice->GetNicType.Status));
  DEBUG ((EFI_D_NET, "  GetNicType.NicType     = %d\n", SimpleNetworkDevice->GetNicType.NicType));
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (SimpleNetworkDevice->GetNicType.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }
  //
  // The information has been retrieved.  Fill in Mode data.
  //
  return Status;
}

STATIC
EFI_STATUS
Undi16SimpleNetworkGetNdisInfo (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }

  EfiZeroMem (&SimpleNetworkDevice->GetNdisInfo, sizeof (PXENV_UNDI_GET_NDIS_INFO_t));

  SimpleNetworkDevice->GetNdisInfo.Status = INIT_PXE_STATUS;

  Status = PxeUndiGetNdisInfo (SimpleNetworkDevice, &SimpleNetworkDevice->GetNdisInfo);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((EFI_D_NET, "  GetNdisInfo.Status       = %d\n", SimpleNetworkDevice->GetNdisInfo.Status));
  DEBUG ((EFI_D_NET, "  GetNdisInfo.IfaceType    = %a\n", SimpleNetworkDevice->GetNdisInfo.IfaceType));
  DEBUG ((EFI_D_NET, "  GetNdisInfo.LinkSpeed    = %d\n", SimpleNetworkDevice->GetNdisInfo.LinkSpeed));
  DEBUG ((EFI_D_NET, "  GetNdisInfo.ServiceFlags = %08x\n", SimpleNetworkDevice->GetNdisInfo.ServiceFlags));

  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (SimpleNetworkDevice->GetNdisInfo.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }
  //
  // The information has been retrieved.  Fill in Mode data.
  //
  return Status;
}
//
// Isr()
//
EFI_STATUS
Undi16SimpleNetworkIsr (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  IN UINTN                       *FrameLength,
  IN UINTN                       *FrameHeaderLength, OPTIONAL
  IN UINT8                       *Frame, OPTIONAL
  IN UINT8                       *ProtType, OPTIONAL
  IN UINT8                       *PktType OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  FrameLength       - GC_TODO: add argument description
  FrameHeaderLength - GC_TODO: add argument description
  Frame             - GC_TODO: add argument description
  ProtType          - GC_TODO: add argument description
  PktType           - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_BUFFER_TOO_SMALL - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;
  BOOLEAN                 FrameReceived;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }

  FrameReceived = FALSE;

  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkStarted:
  default:
    return EFI_DEVICE_ERROR;
  }

  DEBUG ((EFI_D_NET, "Isr() IsrValid = %d\n", SimpleNetworkDevice->IsrValid));

  if (!SimpleNetworkDevice->IsrValid) {
    //
    // Call 16 bit UNDI ROM to open the network interface
    //
    EfiZeroMem (&SimpleNetworkDevice->Isr, sizeof (PXENV_UNDI_ISR_t));
    SimpleNetworkDevice->Isr.Status   = INIT_PXE_STATUS;
    SimpleNetworkDevice->Isr.FuncFlag = PXENV_UNDI_ISR_IN_START;

    DEBUG ((EFI_D_NET, "Isr() START\n"));

    Status = PxeUndiIsr (SimpleNetworkDevice, &SimpleNetworkDevice->Isr);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // Check the status code from the 16 bit UNDI ROM
    //
    if (SimpleNetworkDevice->Isr.Status != PXENV_STATUS_SUCCESS) {
      return EFI_DEVICE_ERROR;
    }
    //
    // There have been no events on this UNDI interface, so return EFI_NOT_READY
    //
    if (SimpleNetworkDevice->Isr.FuncFlag == PXENV_UNDI_ISR_OUT_NOT_OURS) {
      return EFI_SUCCESS;
    }
    //
    // There is data to process, so call until all events processed.
    //
    EfiZeroMem (&SimpleNetworkDevice->Isr, sizeof (PXENV_UNDI_ISR_t));
    SimpleNetworkDevice->Isr.Status   = INIT_PXE_STATUS;
    SimpleNetworkDevice->Isr.FuncFlag = PXENV_UNDI_ISR_IN_PROCESS;

    DEBUG ((EFI_D_NET, "Isr() PROCESS\n"));

    Status = PxeUndiIsr (SimpleNetworkDevice, &SimpleNetworkDevice->Isr);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    SimpleNetworkDevice->IsrValid = TRUE;
  }
  //
  // Call UNDI GET_NEXT until DONE
  //
  while (SimpleNetworkDevice->Isr.FuncFlag != PXENV_UNDI_ISR_OUT_DONE) {
    //
    // Check the status code from the 16 bit UNDI ROM
    //
    if (SimpleNetworkDevice->Isr.Status != PXENV_STATUS_SUCCESS) {
      return EFI_DEVICE_ERROR;
    }
    //
    // UNDI is busy.  Caller will have to call again.
    // This should never happen with a polled mode driver.
    //
    if (SimpleNetworkDevice->Isr.FuncFlag == PXENV_UNDI_ISR_OUT_BUSY) {
      DEBUG ((EFI_D_NET, "  BUSY\n"));
      return EFI_SUCCESS;
    }
    //
    // Check for invalud UNDI FuncFlag
    //
    if (SimpleNetworkDevice->Isr.FuncFlag != PXENV_UNDI_ISR_OUT_RECEIVE &&
        SimpleNetworkDevice->Isr.FuncFlag != PXENV_UNDI_ISR_OUT_TRANSMIT
        ) {
      DEBUG ((EFI_D_NET, "  Invalid SimpleNetworkDevice->Isr.FuncFlag value %d\n", SimpleNetworkDevice->Isr.FuncFlag));
      return EFI_DEVICE_ERROR;
    }
    //
    // Check for Transmit Event
    //
    if (SimpleNetworkDevice->Isr.FuncFlag == PXENV_UNDI_ISR_OUT_TRANSMIT) {
      DEBUG ((EFI_D_NET, "  TRANSMIT\n"));
      SimpleNetworkDevice->InterruptStatus |= EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT;
    }
    //
    // Check for Receive Event
    //
    else if (SimpleNetworkDevice->Isr.FuncFlag == PXENV_UNDI_ISR_OUT_RECEIVE) {
      //
      // note - this code will hang on a receive interrupt in a GetStatus loop
      //
      DEBUG ((EFI_D_NET, "  RECEIVE\n"));
      SimpleNetworkDevice->InterruptStatus |= EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;

      DEBUG ((EFI_D_NET, "SimpleNetworkDevice->Isr.BufferLength      = %d\n", SimpleNetworkDevice->Isr.BufferLength));
      DEBUG ((EFI_D_NET, "SimpleNetworkDevice->Isr.FrameLength       = %d\n", SimpleNetworkDevice->Isr.FrameLength));
      DEBUG ((EFI_D_NET, "SimpleNetworkDevice->Isr.FrameHeaderLength = %d\n", SimpleNetworkDevice->Isr.FrameHeaderLength));
      DEBUG (
        (
        EFI_D_NET, "SimpleNetworkDevice->Isr.Frame             = %04x:%04x\n", SimpleNetworkDevice->Isr.FrameSegSel,
        SimpleNetworkDevice->Isr.FrameOffset
        )
        );
      DEBUG ((EFI_D_NET, "SimpleNetworkDevice->Isr.ProtType          = 0x%02x\n", SimpleNetworkDevice->Isr.BufferLength));
      DEBUG ((EFI_D_NET, "SimpleNetworkDevice->Isr.PktType           = 0x%02x\n", SimpleNetworkDevice->Isr.BufferLength));

      if (FrameReceived) {
        return EFI_SUCCESS;
      }

      if ((Frame == NULL) || (SimpleNetworkDevice->Isr.FrameLength > *FrameLength)) {
        DEBUG ((EFI_D_NET, "return EFI_BUFFER_TOO_SMALL   *FrameLength = %08x\n", *FrameLength));
        *FrameLength = SimpleNetworkDevice->Isr.FrameLength;
        return EFI_BUFFER_TOO_SMALL;
      }

      *FrameLength = SimpleNetworkDevice->Isr.FrameLength;
      if (FrameHeaderLength != NULL) {
        *FrameHeaderLength = SimpleNetworkDevice->Isr.FrameHeaderLength;
      }

      if (ProtType != NULL) {
        *ProtType = SimpleNetworkDevice->Isr.ProtType;
      }

      if (PktType != NULL) {
        *PktType = SimpleNetworkDevice->Isr.PktType;
      }

      EfiCopyMem (
        Frame,
        (VOID *)(UINTN)((SimpleNetworkDevice->Isr.FrameSegSel << 4) + SimpleNetworkDevice->Isr.FrameOffset),
        SimpleNetworkDevice->Isr.BufferLength
        );
      Frame = Frame + SimpleNetworkDevice->Isr.BufferLength;
      if (SimpleNetworkDevice->Isr.BufferLength == SimpleNetworkDevice->Isr.FrameLength) {
        FrameReceived = TRUE;
      }
    }
    //
    // There is data to process, so call until all events processed.
    //
    EfiZeroMem (&SimpleNetworkDevice->Isr, sizeof (PXENV_UNDI_ISR_t));
    SimpleNetworkDevice->Isr.Status   = INIT_PXE_STATUS;
    SimpleNetworkDevice->Isr.FuncFlag = PXENV_UNDI_ISR_IN_GET_NEXT;

    DEBUG ((EFI_D_NET, "Isr() GET NEXT\n"));

    Status = PxeUndiIsr (SimpleNetworkDevice, &SimpleNetworkDevice->Isr);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // Check the status code from the 16 bit UNDI ROM
    //
    //        if (SimpleNetworkDevice->Isr.Status != PXENV_STATUS_SUCCESS) {
    //            return EFI_DEVICE_ERROR;
    //        }
    //
  }

  SimpleNetworkDevice->IsrValid = FALSE;
  return EFI_SUCCESS;
}
//
// ///////////////////////////////////////////////////////////////////////////////////////
// Simple Network Protocol Interface Functions using 16 bit UNDI Option ROMs
/////////////////////////////////////////////////////////////////////////////////////////
//
// Start()
//
EFI_STATUS
Undi16SimpleNetworkStart (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_ALREADY_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;
  PXENV_UNDI_STARTUP_t    Startup;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkStopped:
    break;

  case EfiSimpleNetworkStarted:
  case EfiSimpleNetworkInitialized:
    return EFI_ALREADY_STARTED;

  default:
    return EFI_DEVICE_ERROR;
  }
  //
  // Call 16 bit UNDI ROM to start the network interface
  //
  Startup.Status  = INIT_PXE_STATUS;

  Status          = PxeUndiStartup (SimpleNetworkDevice, &Startup);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (Startup.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }
  //
  // The UNDI interface has been started, so update the State.
  //
  SimpleNetworkDevice->SimpleNetworkMode.State = EfiSimpleNetworkStarted;

  //
  //
  //
  SimpleNetworkDevice->SimpleNetworkMode.ReceiveFilterSetting = 0;
  SimpleNetworkDevice->SimpleNetworkMode.MCastFilterCount     = 0;

  return Status;
}
//
// Stop()
//
EFI_STATUS
Undi16SimpleNetworkStop (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkStarted:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkInitialized:
  default:
    return EFI_DEVICE_ERROR;
  }

  SimpleNetworkDevice->SimpleNetworkMode.State = EfiSimpleNetworkStopped;

  return Status;
}
//
// Initialize()
//
EFI_STATUS
Undi16SimpleNetworkInitialize (
  IN EFI_SIMPLE_NETWORK_PROTOCOL            *This,
  IN UINTN                                  ExtraRxBufferSize  OPTIONAL,
  IN UINTN                                  ExtraTxBufferSize  OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  ExtraRxBufferSize - GC_TODO: add argument description
  ExtraTxBufferSize - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;
  PXENV_UNDI_INITIALIZE_t Initialize;
  PXENV_UNDI_OPEN_t       Open;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkStarted:
    break;

  case EfiSimpleNetworkInitialized:
  default:
    return EFI_DEVICE_ERROR;
  }
  //
  // Call 16 bit UNDI ROM to start the network interface
  //
  Initialize.Status       = INIT_PXE_STATUS;
  Initialize.ProtocolIni  = 0;

  Status                  = PxeUndiInitialize (SimpleNetworkDevice, &Initialize);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ERROR : PxeUndiInitialize() - Status = %r\n", Status));
    DEBUG ((EFI_D_ERROR, "Initialize.Status == %xh\n", Initialize.Status));

    if (Initialize.Status == PXENV_STATUS_UNDI_MEDIATEST_FAILED) {
      Status = EFI_NO_MEDIA;
    }

    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (Initialize.Status != PXENV_STATUS_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "ERROR : PxeUndiInitialize() - Initialize.Status = %04x\n", Initialize.Status));
    return EFI_DEVICE_ERROR;
  }
  //
  // Call 16 bit UNDI ROM to open the network interface
  //
  Open.Status     = INIT_PXE_STATUS;
  Open.OpenFlag   = 0;
  Open.PktFilter  = Undi16GetPacketFilterSetting (SimpleNetworkDevice->SimpleNetworkMode.ReceiveFilterSetting);
  Undi16GetMCastFilters (
    &SimpleNetworkDevice->SimpleNetworkMode,
    &Open.McastBuffer,
    SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
    );

  Status = PxeUndiOpen (SimpleNetworkDevice, &Open);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ERROR : PxeUndiOpen() - Status = %r\n", Status));
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (Open.Status != PXENV_STATUS_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "ERROR : PxeUndiOpen() - Open.Status = %04x\n", Open.Status));
    return EFI_DEVICE_ERROR;
  }
  //
  // The UNDI interface has been initialized, so update the State.
  //
  SimpleNetworkDevice->SimpleNetworkMode.State = EfiSimpleNetworkInitialized;

  //
  // If initialize succeeds, then assume that media is present.
  //
  SimpleNetworkDevice->SimpleNetworkMode.MediaPresent = TRUE;

  //
  // Reset the recycled transmit buffer FIFO
  //
  SimpleNetworkDevice->TxBufferFifo.First = 0;
  SimpleNetworkDevice->TxBufferFifo.Last  = 0;
  SimpleNetworkDevice->IsrValid           = FALSE;

  return Status;
}
//
// Reset()
//
EFI_STATUS
Undi16SimpleNetworkReset (
  IN EFI_SIMPLE_NETWORK_PROTOCOL   *This,
  IN BOOLEAN                       ExtendedVerification
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                  - GC_TODO: add argument description
  ExtendedVerification  - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;
  PXENV_UNDI_RESET_t      Reset;
  UINT16                  rx_filter;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
  default:
    return EFI_DEVICE_ERROR;
  }

  Reset.Status  = INIT_PXE_STATUS;

  rx_filter     = Undi16GetPacketFilterSetting (SimpleNetworkDevice->SimpleNetworkMode.ReceiveFilterSetting);

  Undi16GetMCastFilters (
    &SimpleNetworkDevice->SimpleNetworkMode,
    &Reset.R_Mcast_Buf,
    SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
    );

  Status = PxeUndiResetNic (SimpleNetworkDevice, &Reset, rx_filter);

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (Reset.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Reset the recycled transmit buffer FIFO
  //
  SimpleNetworkDevice->TxBufferFifo.First = 0;
  SimpleNetworkDevice->TxBufferFifo.Last  = 0;
  SimpleNetworkDevice->IsrValid           = FALSE;

  return Status;
}
//
// Shutdown()
//
EFI_STATUS
Undi16SimpleNetworkShutdown (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;
  PXENV_UNDI_CLOSE_t      Close;
  PXENV_UNDI_SHUTDOWN_t   Shutdown;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
  default:
    return EFI_DEVICE_ERROR;
  }

  SimpleNetworkDevice->IsrValid = FALSE;

  //
  // Call 16 bit UNDI ROM to start the network interface
  //
  Close.Status  = INIT_PXE_STATUS;

  Status        = PxeUndiClose (SimpleNetworkDevice, &Close);

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (Close.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Call 16 bit UNDI ROM to open the network interface
  //
  Shutdown.Status = INIT_PXE_STATUS;

  Status          = PxeUndiShutdown (SimpleNetworkDevice, &Shutdown);

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (Shutdown.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }
  //
  // The UNDI interface has been initialized, so update the State.
  //
  SimpleNetworkDevice->SimpleNetworkMode.State = EfiSimpleNetworkStarted;

  //
  // If shutdown succeeds, then assume that media is not present.
  //
  SimpleNetworkDevice->SimpleNetworkMode.MediaPresent = FALSE;

  //
  // Reset the recycled transmit buffer FIFO
  //
  SimpleNetworkDevice->TxBufferFifo.First = 0;
  SimpleNetworkDevice->TxBufferFifo.Last  = 0;

  //
  // A short delay.  Without this an initialize immediately following
  // a shutdown will cause some versions of UNDI-16 to stop operating.
  //
  gBS->Stall (250000);

  return Status;
}
//
// ReceiveFilters()
//
EFI_STATUS
Undi16SimpleNetworkReceiveFilters (
  IN EFI_SIMPLE_NETWORK_PROTOCOL                     * This,
  IN UINT32                                          Enable,
  IN UINT32                                          Disable,
  IN BOOLEAN                                         ResetMCastFilter,
  IN UINTN                                           MCastFilterCnt     OPTIONAL,
  IN EFI_MAC_ADDRESS                                 * MCastFilter OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  Enable            - GC_TODO: add argument description
  Disable           - GC_TODO: add argument description
  ResetMCastFilter  - GC_TODO: add argument description
  MCastFilterCnt    - GC_TODO: add argument description
  MCastFilter       - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  UINTN                   i;
  UINT32                  NewFilter;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;
  PXENV_UNDI_CLOSE_t      Close;
  PXENV_UNDI_OPEN_t       Open;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
  default:
    return EFI_DEVICE_ERROR;
  }
  //
  // First deal with possible filter setting changes
  //
  if (!Enable && !Disable && !ResetMCastFilter) {
    return EFI_SUCCESS;
  }

  NewFilter = (SimpleNetworkDevice->SimpleNetworkMode.ReceiveFilterSetting | Enable) &~Disable;

  if (NewFilter & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) {
    if (!MCastFilterCnt || !MCastFilter || MCastFilterCnt > SimpleNetworkDevice->SimpleNetworkMode.MaxMCastFilterCount) {
      return EFI_INVALID_PARAMETER;
    }
  }
  //
  // Call 16 bit UNDI ROM to close the network interface
  //
  Close.Status  = INIT_PXE_STATUS;

  Status        = PxeUndiClose (SimpleNetworkDevice, &Close);

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (Close.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Call 16 bit UNDI ROM to open the network interface
  //
  //
  // Reset the recycled transmit buffer FIFO
  //
  SimpleNetworkDevice->TxBufferFifo.First = 0;
  SimpleNetworkDevice->TxBufferFifo.Last  = 0;

  //
  // Call 16 bit UNDI ROM to open the network interface
  //
  EfiZeroMem (&Open, sizeof Open);

  Open.Status     = INIT_PXE_STATUS;
  Open.PktFilter  = Undi16GetPacketFilterSetting (NewFilter);

  if (NewFilter & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) {
    //
    // Copy the MAC addresses into the UNDI open parameter structure
    //
    Open.McastBuffer.MCastAddrCount = (UINT16) MCastFilterCnt;
    for (i = 0; i < MCastFilterCnt; ++i) {
      EfiCopyMem (
        Open.McastBuffer.MCastAddr[i],
        &MCastFilter[i],
        sizeof Open.McastBuffer.MCastAddr[i]
        );
    }
  } else if (!ResetMCastFilter) {
    for (i = 0; i < SimpleNetworkDevice->SimpleNetworkMode.MCastFilterCount; ++i) {
      EfiCopyMem (
        Open.McastBuffer.MCastAddr[i],
        &SimpleNetworkDevice->SimpleNetworkMode.MCastFilter[i],
        sizeof Open.McastBuffer.MCastAddr[i]
        );
    }
  }

  Status = PxeUndiOpen (SimpleNetworkDevice, &Open);

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (Open.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }

  SimpleNetworkDevice->IsrValid = FALSE;
  SimpleNetworkDevice->SimpleNetworkMode.ReceiveFilterSetting = NewFilter;

  if (NewFilter & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) {
    SimpleNetworkDevice->SimpleNetworkMode.MCastFilterCount = (UINT32) MCastFilterCnt;
    for (i = 0; i < MCastFilterCnt; ++i) {
      SimpleNetworkDevice->SimpleNetworkMode.MCastFilter[i] = MCastFilter[i];
    }
  }
  //
  // Read back multicast addresses.
  //
  return EFI_SUCCESS;
}
//
// StationAddress()
//
EFI_STATUS
Undi16SimpleNetworkStationAddress (
  IN EFI_SIMPLE_NETWORK_PROTOCOL           * This,
  IN BOOLEAN                               Reset,
  IN EFI_MAC_ADDRESS                       * New OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description
  Reset - GC_TODO: add argument description
  New   - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS                    Status;
  EFI_SIMPLE_NETWORK_DEV        *SimpleNetworkDevice;
  PXENV_UNDI_SET_STATION_ADDR_t SetStationAddr;
  //
  // EFI_DEVICE_PATH_PROTOCOL     *OldDevicePath;
  //
  PXENV_UNDI_CLOSE_t            close;
  PXENV_UNDI_OPEN_t             open;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;

  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkStarted:
  default:
    return EFI_DEVICE_ERROR;
  }
  //
  // Call 16 bit UNDI ROM to open the network interface
  //
  SetStationAddr.Status = INIT_PXE_STATUS;

  if (Reset) {
    //
    // If we are reseting the Station Address to the permanent address, and the
    // Station Address is not programmable, then just return EFI_SUCCESS.
    //
    if (!SimpleNetworkDevice->SimpleNetworkMode.MacAddressChangeable) {
      return EFI_SUCCESS;
    }
    //
    // If the address is already the permanent address, then just return success.
    //
    if (!EfiCompareMem (
          &SimpleNetworkDevice->SimpleNetworkMode.CurrentAddress,
          &SimpleNetworkDevice->SimpleNetworkMode.PermanentAddress,
          SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
          )) {
      return EFI_SUCCESS;
    }
    //
    // Copy the adapters permanent address to the new station address
    //
    EfiCopyMem (
      &SetStationAddr.StationAddress,
      &SimpleNetworkDevice->SimpleNetworkMode.PermanentAddress,
      SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
      );
  } else {
    //
    // If we are setting the Station Address, and the
    // Station Address is not programmable, return invalid parameter.
    //
    if (!SimpleNetworkDevice->SimpleNetworkMode.MacAddressChangeable) {
      return EFI_INVALID_PARAMETER;
    }
    //
    // If the address is already the new address, then just return success.
    //
    if (!EfiCompareMem (
          &SimpleNetworkDevice->SimpleNetworkMode.CurrentAddress,
          New,
          SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
          )) {
      return EFI_SUCCESS;
    }
    //
    // Copy New to the new station address
    //
    EfiCopyMem (
      &SetStationAddr.StationAddress,
      New,
      SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
      );

  }
  //
  // Call 16 bit UNDI ROM to stop the network interface
  //
  close.Status = INIT_PXE_STATUS;

  PxeUndiClose (SimpleNetworkDevice, &close);

  //
  // Call 16-bit UNDI ROM to set the station address
  //
  SetStationAddr.Status = PXENV_STATUS_SUCCESS;

  Status                = PxeUndiSetStationAddr (SimpleNetworkDevice, &SetStationAddr);

  //
  // Call 16-bit UNDI ROM to start the network interface
  //
  open.Status     = PXENV_STATUS_SUCCESS;
  open.OpenFlag   = 0;
  open.PktFilter  = Undi16GetPacketFilterSetting (SimpleNetworkDevice->SimpleNetworkMode.ReceiveFilterSetting);
  Undi16GetMCastFilters (
    &SimpleNetworkDevice->SimpleNetworkMode,
    &open.McastBuffer,
    SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
    );

  PxeUndiOpen (SimpleNetworkDevice, &open);

  //
  // Check status from station address change
  //
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (SetStationAddr.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }

  EfiCopyMem (
    &SimpleNetworkDevice->SimpleNetworkMode.CurrentAddress,
    &SetStationAddr.StationAddress,
    SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
    );

#if 0 /* The device path is based on the permanent address not the current address. */
  //
  // The station address was changed, so update the device path with the new MAC address.
  //
  OldDevicePath                   = SimpleNetworkDevice->DevicePath;
  SimpleNetworkDevice->DevicePath = DuplicateDevicePath (SimpleNetworkDevice->BaseDevicePath);
  SimpleNetworkAppendMacAddressDevicePath (
    &SimpleNetworkDevice->DevicePath,
    &SimpleNetworkDevice->SimpleNetworkMode.CurrentAddress
    );

  Status = LibReinstallProtocolInterfaces (
            SimpleNetworkDevice->Handle,
            &DevicePathProtocol,
            OldDevicePath,
            SimpleNetworkDevice->DevicePath,
            NULL
            );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to reinstall the DevicePath protocol for the Simple Network Device\n"));
    DEBUG ((EFI_D_ERROR, "  Status = %r\n", Status));
  }

  FreePool (OldDevicePath);
#endif /* 0 */

  return Status;
}
//
// Statistics()
//
EFI_STATUS
Undi16SimpleNetworkStatistics (
  IN EFI_SIMPLE_NETWORK_PROTOCOL       * This,
  IN BOOLEAN                           Reset,
  IN OUT UINTN                         *StatisticsSize OPTIONAL,
  OUT EFI_NETWORK_STATISTICS           * StatisticsTable OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This            - GC_TODO: add argument description
  Reset           - GC_TODO: add argument description
  StatisticsSize  - GC_TODO: add argument description
  StatisticsTable - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS                    Status;
  EFI_SIMPLE_NETWORK_DEV        *SimpleNetworkDevice;
  PXENV_UNDI_CLEAR_STATISTICS_t ClearStatistics;
  PXENV_UNDI_GET_STATISTICS_t   GetStatistics;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkStarted:
  default:
    return EFI_DEVICE_ERROR;
  }

  if (StatisticsSize && *StatisticsSize && !StatisticsTable) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // If Reset is TRUE, then clear all the statistics.
  //
  if (Reset) {

    DEBUG ((EFI_D_NET, "  RESET Statistics\n"));

    //
    // Call 16 bit UNDI ROM to open the network interface
    //
    ClearStatistics.Status  = INIT_PXE_STATUS;

    Status                  = PxeUndiClearStatistics (SimpleNetworkDevice, &ClearStatistics);

    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // Check the status code from the 16 bit UNDI ROM
    //
    if (ClearStatistics.Status != PXENV_STATUS_SUCCESS) {
      return EFI_DEVICE_ERROR;
    }

    DEBUG ((EFI_D_NET, "  RESET Statistics Complete"));
  }

  if (StatisticsSize != NULL) {
    EFI_NETWORK_STATISTICS  LocalStatisticsTable;

    DEBUG ((EFI_D_NET, "  GET Statistics\n"));

    //
    // If the size if valid, then see if the table is valid
    //
    if (StatisticsTable == NULL) {
      DEBUG ((EFI_D_NET, "  StatisticsTable is NULL\n"));
      return EFI_INVALID_PARAMETER;
    }
    //
    // Call 16 bit UNDI ROM to open the network interface
    //
    GetStatistics.Status            = INIT_PXE_STATUS;
    GetStatistics.XmtGoodFrames     = 0;
    GetStatistics.RcvGoodFrames     = 0;
    GetStatistics.RcvCRCErrors      = 0;
    GetStatistics.RcvResourceErrors = 0;

    Status                          = PxeUndiGetStatistics (SimpleNetworkDevice, &GetStatistics);

    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // Check the status code from the 16 bit UNDI ROM
    //
    if (GetStatistics.Status != PXENV_STATUS_SUCCESS) {
      return EFI_DEVICE_ERROR;
    }
    //
    // Fill in the Statistics Table with the collected values.
    //
    EfiSetMem (&LocalStatisticsTable, sizeof LocalStatisticsTable, 0xff);

    LocalStatisticsTable.TxGoodFrames     = GetStatistics.XmtGoodFrames;
    LocalStatisticsTable.RxGoodFrames     = GetStatistics.RcvGoodFrames;
    LocalStatisticsTable.RxCrcErrorFrames = GetStatistics.RcvCRCErrors;
    LocalStatisticsTable.RxDroppedFrames  = GetStatistics.RcvResourceErrors;

    EfiCopyMem (StatisticsTable, &LocalStatisticsTable, *StatisticsSize);

    DEBUG (
      (EFI_D_NET,
      "  Statistics Collected : Size=%d  Buf=%08x\n",
      Reset,
      *StatisticsSize,
      StatisticsTable)
      );

    DEBUG ((EFI_D_NET, "  GET Statistics Complete"));

    if (*StatisticsSize < sizeof LocalStatisticsTable) {
      DEBUG ((EFI_D_NET, "  BUFFER TOO SMALL\n"));
      Status = EFI_BUFFER_TOO_SMALL;
    }

    *StatisticsSize = sizeof LocalStatisticsTable;

    return Status;

  }

  return EFI_SUCCESS;
}
//
// MCastIpToMac()
//
EFI_STATUS
Undi16SimpleNetworkMCastIpToMac (
  IN EFI_SIMPLE_NETWORK_PROTOCOL            *This,
  IN BOOLEAN                                IPv6,
  IN EFI_IP_ADDRESS                         *IP,
  OUT EFI_MAC_ADDRESS                       *MAC
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description
  IPv6  - GC_TODO: add argument description
  IP    - GC_TODO: add argument description
  MAC   - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_UNSUPPORTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS                  Status;
  EFI_SIMPLE_NETWORK_DEV      *SimpleNetworkDevice;
  PXENV_UNDI_GET_MCAST_ADDR_t GetMcastAddr;

  if (!This || !IP || !MAC) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
  default:
    return EFI_DEVICE_ERROR;
  }
  //
  // 16 bit UNDI Option ROMS do not support IPv6.  Check for IPv6 usage.
  //
  if (IPv6) {
    return EFI_UNSUPPORTED;
  }
  //
  // Call 16 bit UNDI ROM to open the network interface
  //
  GetMcastAddr.Status = INIT_PXE_STATUS;
  EfiCopyMem (&GetMcastAddr.InetAddr, IP, 4);

  Status = PxeUndiGetMcastAddr (SimpleNetworkDevice, &GetMcastAddr);

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  if (GetMcastAddr.Status != PXENV_STATUS_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Copy the MAC address from the returned data structure.
  //
  EfiCopyMem (
    MAC,
    &GetMcastAddr.MediaAddr,
    SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
    );

  return Status;
}
//
// NvData()
//
EFI_STATUS
Undi16SimpleNetworkNvData (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *This,
  IN BOOLEAN                      ReadWrite,
  IN UINTN                        Offset,
  IN UINTN                        BufferSize,
  IN OUT VOID                     *Buffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  ReadWrite   - GC_TODO: add argument description
  Offset      - GC_TODO: add argument description
  BufferSize  - GC_TODO: add argument description
  Buffer      - GC_TODO: add argument description

Returns:

  EFI_UNSUPPORTED - GC_TODO: Add description for return value

--*/
{
  return EFI_UNSUPPORTED;
}
//
// GetStatus()
//
EFI_STATUS
Undi16SimpleNetworkGetStatus (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  * This,
  OUT UINT32                      *InterruptStatus OPTIONAL,
  OUT VOID                        **TxBuf OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This            - GC_TODO: add argument description
  InterruptStatus - GC_TODO: add argument description
  TxBuf           - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;
  UINTN                   FrameLength;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkStarted:
  default:
    return EFI_DEVICE_ERROR;
  }

  if (!InterruptStatus && !TxBuf) {
    return EFI_INVALID_PARAMETER;
  }

  FrameLength = 0;
  Status      = Undi16SimpleNetworkIsr (This, &FrameLength, NULL, NULL, NULL, NULL);

  if (Status != EFI_BUFFER_TOO_SMALL) {
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  //
  // See if the caller wants interrupt info.
  //
  if (InterruptStatus != NULL) {
    *InterruptStatus                      = SimpleNetworkDevice->InterruptStatus;
    SimpleNetworkDevice->InterruptStatus  = 0;
  }
  //
  // See if the caller wants transmit buffer status info.
  //
  if (TxBuf) {
    *TxBuf = 0;
    SimpleNetworkTransmitFifoRemove (&(SimpleNetworkDevice->TxBufferFifo), TxBuf);
  }

  return EFI_SUCCESS;
}
//
// Transmit()
//
EFI_STATUS
Undi16SimpleNetworkTransmit (
  IN EFI_SIMPLE_NETWORK_PROTOCOL           * This,
  IN UINTN                                 HeaderSize,
  IN UINTN                                 BufferSize,
  IN VOID                                  *Buffer,
  IN EFI_MAC_ADDRESS                       * SrcAddr OPTIONAL,
  IN EFI_MAC_ADDRESS                       * DestAddr OPTIONAL,
  IN UINT16                                *Protocol OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  HeaderSize  - GC_TODO: add argument description
  BufferSize  - GC_TODO: add argument description
  Buffer      - GC_TODO: add argument description
  SrcAddr     - GC_TODO: add argument description
  DestAddr    - GC_TODO: add argument description
  Protocol    - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_BUFFER_TOO_SMALL - GC_TODO: Add description for return value
  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_NOT_READY - GC_TODO: Add description for return value
  EFI_NOT_READY - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;
  PXENV_UNDI_TRANSMIT_t   XmitInfo;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkStarted:
  default:
    return EFI_DEVICE_ERROR;
  }

  if (!Buffer) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize < SimpleNetworkDevice->SimpleNetworkMode.MediaHeaderSize) {
    return EFI_BUFFER_TOO_SMALL;
  }

  if (HeaderSize) {
    if (HeaderSize != SimpleNetworkDevice->SimpleNetworkMode.MediaHeaderSize) {
      return EFI_INVALID_PARAMETER;
    }

    if (!DestAddr || !Protocol) {
      return EFI_INVALID_PARAMETER;
    }

    if (DestAddr) {
      EfiCopyMem (
        Buffer,
        DestAddr,
        SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
        );
    }

    if (!SrcAddr) {
      SrcAddr = &SimpleNetworkDevice->SimpleNetworkMode.CurrentAddress;
    }

    EfiCopyMem (
      (UINT8 *) Buffer + SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize,
      SrcAddr,
      SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
      );

    if (Protocol) {
      *(UINT16 *) ((UINT8 *) Buffer + 2 * SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize) = (UINT16) (((*Protocol & 0xFF) << 8) | ((*Protocol >> 8) & 0xFF));
    }
  }
  //
  // See if the recycled transmit buffer FIFO is full.
  // If it is full, then we can not transmit until the caller calls GetStatus() to pull
  // off recycled transmit buffers.
  //
  if (SimpleNetworkTransmitFifoFull (&(SimpleNetworkDevice->TxBufferFifo))) {
    return EFI_NOT_READY;
  }
  //
  //  Output debug trace message.
  //
  DEBUG ((EFI_D_NET, "Undi16SimpleNetworkTransmit\n\r "));

  //
  // Initialize UNDI WRITE parameter structure.
  //
  XmitInfo.Status           = INIT_PXE_STATUS;
  XmitInfo.Protocol         = P_UNKNOWN;
  XmitInfo.XmitFlag         = XMT_DESTADDR;
  XmitInfo.DestAddrOffset   = (UINT16) ((UINTN) SimpleNetworkDevice->TxDestAddr & 0x000f);
  XmitInfo.DestAddrSegment  = (UINT16) ((UINTN) SimpleNetworkDevice->TxDestAddr >> 4);
  XmitInfo.TBDOffset        = (UINT16) ((UINTN) SimpleNetworkDevice->Xmit & 0x000f);
  XmitInfo.TBDSegment       = (UINT16) ((UINTN) SimpleNetworkDevice->Xmit >> 4);
  XmitInfo.Reserved[0]      = 0;
  XmitInfo.Reserved[1]      = 0;

  EfiCopyMem (
    SimpleNetworkDevice->TxDestAddr,
    Buffer,
    SimpleNetworkDevice->SimpleNetworkMode.HwAddressSize
    );

  EfiCopyMem (
    SimpleNetworkDevice->TxRealModeMediaHeader,
    Buffer,
    SimpleNetworkDevice->SimpleNetworkMode.MediaHeaderSize
    );

  SimpleNetworkDevice->Xmit->ImmedLength            = (UINT16) SimpleNetworkDevice->SimpleNetworkMode.MediaHeaderSize;

  SimpleNetworkDevice->Xmit->DataBlock[0].TDDataLen = (UINT16) (BufferSize - SimpleNetworkDevice->Xmit->ImmedLength);

  EfiCopyMem (
    SimpleNetworkDevice->TxRealModeDataBuffer,
    (UINT8 *) Buffer + SimpleNetworkDevice->SimpleNetworkMode.MediaHeaderSize,
    SimpleNetworkDevice->Xmit->DataBlock[0].TDDataLen
    );

  //
  // Make API call to UNDI TRANSMIT
  //
  XmitInfo.Status = 0;

  Status          = PxeUndiTransmit (SimpleNetworkDevice, &XmitInfo);

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check the status code from the 16 bit UNDI ROM
  //
  switch (XmitInfo.Status) {
  case PXENV_STATUS_OUT_OF_RESOURCES:
    return EFI_NOT_READY;

  case PXENV_STATUS_SUCCESS:
    break;

  default:
    return EFI_DEVICE_ERROR;
  }
  //
  // Add address of Buffer to the recycled transmit buffer FIFO
  //
  SimpleNetworkTransmitFifoAdd (&(SimpleNetworkDevice->TxBufferFifo), Buffer);

  return EFI_SUCCESS;
}
//
// Receive()
//
EFI_STATUS
Undi16SimpleNetworkReceive (
  IN EFI_SIMPLE_NETWORK_PROTOCOL            * This,
  OUT UINTN                                 *HeaderSize OPTIONAL,
  IN OUT UINTN                              *BufferSize,
  OUT VOID                                  *Buffer,
  OUT EFI_MAC_ADDRESS                       * SrcAddr OPTIONAL,
  OUT EFI_MAC_ADDRESS                       * DestAddr OPTIONAL,
  OUT UINT16                                *Protocol OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  HeaderSize  - GC_TODO: add argument description
  BufferSize  - GC_TODO: add argument description
  Buffer      - GC_TODO: add argument description
  SrcAddr     - GC_TODO: add argument description
  DestAddr    - GC_TODO: add argument description
  Protocol    - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_READY - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;
  UINTN                   MediaAddrSize;
  UINT8                   ProtType;

  if (!This || !BufferSize || !Buffer) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkStarted:
  default:
    return EFI_DEVICE_ERROR;
  }

  Status = Undi16SimpleNetworkIsr (
            This,
            BufferSize,
            HeaderSize,
            Buffer,
            &ProtType,
            NULL
            );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!(SimpleNetworkDevice->InterruptStatus & EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT)) {
    return EFI_NOT_READY;

  }

  SimpleNetworkDevice->InterruptStatus &= ~EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;

  MediaAddrSize = This->Mode->HwAddressSize;

  if (SrcAddr != NULL) {
    EfiCopyMem (SrcAddr, (UINT8 *) Buffer + MediaAddrSize, MediaAddrSize);
  }

  if (DestAddr != NULL) {
    EfiCopyMem (DestAddr, Buffer, MediaAddrSize);
  }

  if (Protocol != NULL) {
    EfiCopyMem (Protocol, (UINT8 *) Buffer + 2 * MediaAddrSize, sizeof (UINT16));
  }

  DEBUG ((EFI_D_NET, "Packet Received: BufferSize=%d  HeaderSize = %d\n", *BufferSize, *HeaderSize));

  return Status;
}
//
// WaitForPacket()
//
VOID
Undi16SimpleNetworkWaitForPacket (
  IN EFI_EVENT               Event,
  IN VOID                    *Context
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Event   - GC_TODO: add argument description
  Context - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  //
  // Someone is waiting on the receive packet event, if there's
  // a packet pending, signal the event
  //
  if (!EFI_ERROR (Undi16SimpleNetworkCheckForPacket (Context))) {
    gBS->SignalEvent (Event);
  }
}
//
// CheckForPacket()
//
EFI_STATUS
Undi16SimpleNetworkCheckForPacket (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value
  EFI_NOT_STARTED - GC_TODO: Add description for return value
  EFI_DEVICE_ERROR - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS              Status;
  EFI_SIMPLE_NETWORK_DEV  *SimpleNetworkDevice;
  UINTN                   FrameLength;

  if (!This) {
    return EFI_INVALID_PARAMETER;
  }

  Status              = EFI_SUCCESS;
  SimpleNetworkDevice = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (This);

  if (!SimpleNetworkDevice) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Verify that the current state of the adapter is valid for this call.
  //
  switch (SimpleNetworkDevice->SimpleNetworkMode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkStarted:
  default:
    return EFI_DEVICE_ERROR;
  }

  FrameLength = 0;
  Status = Undi16SimpleNetworkIsr (
            This,
            &FrameLength,
            NULL,
            NULL,
            NULL,
            NULL
            );

  if (Status != EFI_BUFFER_TOO_SMALL) {
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return (SimpleNetworkDevice->InterruptStatus & EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT) ? EFI_SUCCESS : EFI_NOT_READY;
}

STATIC
VOID
Undi16SimpleNetworkEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
/*++

Routine Description:

  Signal handlers for ExitBootServices event

  Clean up any Real-mode UNDI residue from the system

Arguments:

  Event type
  Context fo the event

Returns:
   Nothing

--*/
{
  //
  // NOTE:  This is not the only way to effect this cleanup.  The prescribed mechanism
  //        would be to perform an UNDI STOP command.  This strategam has been attempted
  //        but results in problems making some of the EFI core services from TPL_CALLBACK.
  //        This issue needs to be resolved, but the other alternative has been to perform
  //        the unchain logic explicitly, as done below.
  //
  RestoreCachedVectorAddress (0x1A);
}

EFI_STATUS
BiosSnp16AllocatePagesBelowOneMb (
  UINTN  NumPages,
  VOID   **Buffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  NumPages  - GC_TODO: add argument description
  Buffer    - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;

  PhysicalAddress = 0x000fffff;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiRuntimeServicesData,
                  NumPages,
                  &PhysicalAddress
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *Buffer = (VOID *) (UINTN) PhysicalAddress;
  return EFI_SUCCESS;
}
