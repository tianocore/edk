/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  BdsPlatform.c

Abstract:

  This file include all platform action which can be customized
  by IBV/OEM.

--*/

#include "BdsPlatform.h"
#include "EfiPrintLib.h"
#include "String.h"
#include "Language.h"
#include "FrontPage.h"
#include EFI_GUID_DEFINITION (SmBios)
#include EFI_GUID_DEFINITION (Acpi)
#include EFI_GUID_DEFINITION (Mps)
#include EFI_GUID_DEFINITION (PciExpressBaseAddress)

CHAR16  mFirmwareVendor[] = L"TianoCore.org";
extern BOOLEAN  gConnectAllHappened;

//
// BDS Platform Functions
//

VOID
GetSystemTablesFromHob (
  VOID
  )
/*++

Routine Description:
  Find GUID'ed HOBs that contain EFI_PHYSICAL_ADDRESS of ACPI, SMBIOS, MPs tables

Arguments:
  None

Returns:
  None.

--*/
{
  EFI_STATUS                  Status;
  EFI_HOB_HANDOFF_INFO_TABLE  *HobList;
  UINTN                       Size;
  EFI_PHYSICAL_ADDRESS       *Table;

  //
  // Get Hob List
  //
  
  Status = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, (VOID *) &HobList);
  if (EFI_ERROR (Status)) {
    return;
  }

  //
  // If there is an ACPI table in the HOB add it to the EFI System table
  //
  Status = GetNextGuidHob (&HobList, &gEfiAcpi20TableGuid, &Table, &Size);
  if (!EFI_ERROR (Status)) {
    if (*Table != 0) {
      gBS->InstallConfigurationTable (&gEfiAcpi20TableGuid, (VOID *)(UINTN)*Table);
    }
  }
  Status = GetNextGuidHob (&HobList, &gEfiAcpiTableGuid, &Table, &Size);
  if (!EFI_ERROR (Status)) {
    if (*Table != 0) {
      gBS->InstallConfigurationTable (&gEfiAcpiTableGuid, (VOID *)(UINTN)*Table);
    }
  }

  //
  // If there is a SMBIOS table in the HOB add it to the EFI System table
  //
  Status = GetNextGuidHob (&HobList, &gEfiSmbiosTableGuid, &Table, &Size);
  if (!EFI_ERROR (Status)) {
    if (*Table != 0) {
      gBS->InstallConfigurationTable (&gEfiSmbiosTableGuid, (VOID *)(UINTN)*Table);
    }
  }

  //
  // If there is a MPS table in the HOB add it to the EFI System table
  //
  Status = GetNextGuidHob (&HobList, &gEfiMpsTableGuid, &Table, &Size);
  if (!EFI_ERROR (Status)) {
    if (*Table != 0) {
      gBS->InstallConfigurationTable (&gEfiMpsTableGuid, (VOID *)(UINTN)*Table);
    }
  }
}


#define EFI_LDR_MEMORY_DESCRIPTOR_GUID \
  { 0x7701d7e5, 0x7d1d, 0x4432, 0xa4, 0x68, 0x67, 0x3d, 0xab, 0x8a, 0xde, 0x60 }

EFI_GUID gEfiLdrMemoryDescriptorGuid = EFI_LDR_MEMORY_DESCRIPTOR_GUID;

#pragma pack(1)

typedef struct {
  EFI_HOB_GUID_TYPE             Hob;
  UINTN                         MemDescCount;
  EFI_MEMORY_DESCRIPTOR         *MemDesc;
} MEMORY_DESC_HOB;

#pragma pack()

VOID
UpdateMemoryMap (
  VOID
  )
{
  EFI_STATUS                  Status;
  EFI_HOB_HANDOFF_INFO_TABLE  *HobList;
  UINTN                       Size;
  VOID                        *Table;
  MEMORY_DESC_HOB             MemoryDescHob;
  UINTN                       Index;
  EFI_PHYSICAL_ADDRESS        Memory;

  //
  // Get Hob List
  //
  Status = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, (VOID *) &HobList);
  if (EFI_ERROR (Status)) {
    return;
  }

  Status = GetNextGuidHob (&HobList, &gEfiLdrMemoryDescriptorGuid, &Table, &Size);
  if (EFI_ERROR (Status)) {
    return;
  }
  MemoryDescHob.MemDescCount = *(UINTN *)Table;
  MemoryDescHob.MemDesc      = *(EFI_MEMORY_DESCRIPTOR **)((UINTN)Table + sizeof(UINTN));

  //
  // Add ACPINVS, ACPIReclaim, and Reserved memory to MemoryMap
  //
  for (Index = 0; Index < MemoryDescHob.MemDescCount; Index++) {
    if (MemoryDescHob.MemDesc[Index].PhysicalStart < 0x100000) {
      continue;
    }
    if (MemoryDescHob.MemDesc[Index].PhysicalStart >= 0x100000000) {
      continue;
    }
    if ((MemoryDescHob.MemDesc[Index].Type == EfiReservedMemoryType) ||
        (MemoryDescHob.MemDesc[Index].Type == EfiRuntimeServicesData) ||
        (MemoryDescHob.MemDesc[Index].Type == EfiRuntimeServicesCode) ||
        (MemoryDescHob.MemDesc[Index].Type == EfiACPIReclaimMemory) ||
        (MemoryDescHob.MemDesc[Index].Type == EfiACPIMemoryNVS)) {
      DEBUG ((EFI_D_ERROR, "PhysicalStart - 0x%x, ", MemoryDescHob.MemDesc[Index].PhysicalStart));
      DEBUG ((EFI_D_ERROR, "PageNumber    - 0x%x, ", MemoryDescHob.MemDesc[Index].NumberOfPages));
      DEBUG ((EFI_D_ERROR, "Type          - 0x%x\n", MemoryDescHob.MemDesc[Index].Type));
      if ((MemoryDescHob.MemDesc[Index].Type == EfiRuntimeServicesData) ||
          (MemoryDescHob.MemDesc[Index].Type == EfiRuntimeServicesCode)) {
        //
        // Skip RuntimeSevicesData and RuntimeServicesCode, they are BFV
        //
        continue;
      }
      Status = gDS->AddMemorySpace (
                      EfiGcdMemoryTypeSystemMemory,
                      MemoryDescHob.MemDesc[Index].PhysicalStart,
                      LShiftU64 (MemoryDescHob.MemDesc[Index].NumberOfPages, EFI_PAGE_SHIFT),
                      MemoryDescHob.MemDesc[Index].Attribute
                      );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "AddMemorySpace fail!\n"));
        if ((MemoryDescHob.MemDesc[Index].Type == EfiACPIReclaimMemory) ||
            (MemoryDescHob.MemDesc[Index].Type == EfiACPIMemoryNVS)) {
          //
          // For EfiACPIReclaimMemory and EfiACPIMemoryNVS, it must success.
          // For EfiReservedMemoryType, there maybe overlap. So skip check here.
          //
//          ASSERT_EFI_ERROR (Status);
        }
        continue;
      }

      Memory = MemoryDescHob.MemDesc[Index].PhysicalStart;
      Status = gBS->AllocatePages (
                      AllocateAddress,
                      MemoryDescHob.MemDesc[Index].Type,
                      (UINTN)MemoryDescHob.MemDesc[Index].NumberOfPages,
                      &Memory
                      );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "AllocatePages fail!\n"));
        //
        // For the page added, it must be allocated.
        //
//        ASSERT_EFI_ERROR (Status);
        continue;
      }
    }
  }
  
}

VOID
PlatformBdsInit (
  IN EFI_BDS_ARCH_PROTOCOL_INSTANCE  *PrivateData
  )
/*++

Routine Description:

  Platform Bds init. Incude the platform firmware vendor, revision
  and so crc check.

Arguments:

  PrivateData  - The EFI_BDS_ARCH_PROTOCOL_INSTANCE instance

Returns:

  None.

--*/
{
  //
  // set firmwarevendor, here can be IBV/OEM customize
  //
  gST->FirmwareVendor = EfiLibAllocateRuntimeCopyPool (
                          sizeof (mFirmwareVendor),
                          &mFirmwareVendor
                          );
  ASSERT (gST->FirmwareVendor != NULL);

  gST->FirmwareRevision = EFI_FIRMWARE_REVISION;

  //
  // Fixup Tasble CRC after we updated Firmware Vendor and Revision
  //
  gBS->CalculateCrc32 ((VOID *) gST, sizeof (EFI_SYSTEM_TABLE), &gST->Hdr.CRC32);

  //
  // Initialize the platform specific string and language
  //
  InitializeStringSupport ();
  InitializeLanguage (TRUE);
  InitializeFrontPage (FALSE);

  GetSystemTablesFromHob ();

  UpdateMemoryMap ();
}

UINT64
GetPciExpressBaseAddressForRootBridge (
  IN UINTN    HostBridgeNumber,
  IN UINTN    RootBridgeNumber
  )
/*++

Routine Description:
  This routine is to get PciExpress Base Address for this RootBridge

Arguments:
  HostBridgeNumber - The number of HostBridge
  RootBridgeNumber - The number of RootBridge
    
Returns:
  UINT64 - PciExpressBaseAddress for this HostBridge and RootBridge

--*/
{
  EFI_PCI_EXPRESS_BASE_ADDRESS_INFORMATION *PciExpressBaseAddressInfo;
  UINTN                                    BufferSize;
  UINT32                                   Index;
  UINT32                                   Number;
  VOID                                     *HobList;
  EFI_STATUS                               Status;

  //
  // Get Hob List from configuration table
  //
  Status = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);
  if (EFI_ERROR (Status)) {
    return 0;
  }

  //
  // Get PciExpressAddressInfo Hob
  //
  PciExpressBaseAddressInfo = NULL;
  Status = GetNextGuidHob (&HobList, &gEfiPciExpressBaseAddressGuid, &PciExpressBaseAddressInfo, &BufferSize);
  if (EFI_ERROR (Status)) {
    return 0;
  }

  //
  // Search the PciExpress Base Address in the Hob for current RootBridge
  //
  Number = (UINT32)(BufferSize / sizeof(EFI_PCI_EXPRESS_BASE_ADDRESS_INFORMATION));
  for (Index = 0; Index < Number; Index++) {
    if ((PciExpressBaseAddressInfo[Index].HostBridgeNumber == HostBridgeNumber) &&
        (PciExpressBaseAddressInfo[Index].RootBridgeNumber == RootBridgeNumber)) {
      return PciExpressBaseAddressInfo[Index].PciExpressBaseAddress;
    }
  }

  //
  // Do not find the PciExpress Base Address in the Hob
  //
  return 0;
}

VOID
PatchPciRootBridgeDevicePath (
  IN UINTN    HostBridgeNumber,
  IN UINTN    RootBridgeNumber,
  IN PLATFORM_ROOT_BRIDGE_DEVICE_PATH  *RootBridge
  )
{
  UINT64  PciExpressBase;

  PciExpressBase = GetPciExpressBaseAddressForRootBridge (HostBridgeNumber, RootBridgeNumber);

  if (PciExpressBase != 0) {
    RootBridge->PciRootBridge.HID = EISA_PNP_ID(0x0A08);
  }
}

EFI_STATUS
ConnectRootBridge (
  VOID
  )
/*++

Routine Description:

  Connect RootBridge

Arguments:

  None.
 
Returns:

  EFI_SUCCESS             - Connect RootBridge successfully.
  EFI_STATUS              - Connect RootBridge fail.

--*/
{
  EFI_STATUS                Status;
  EFI_HANDLE                RootHandle;

  //
  // Patch Pci Root Bridge Device Path
  //
  PatchPciRootBridgeDevicePath (0, 0, &gPlatformRootBridge0);

  //
  // Make all the PCI_IO protocols on PCI Seg 0 show up
  //
  BdsLibConnectDevicePath (gPlatformRootBridges[0]);

  Status = gBS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid, 
                  &gPlatformRootBridges[0], 
                  &RootHandle
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->ConnectController (RootHandle, NULL, NULL, FALSE);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PrepareLpcBridgeDevicePath (
  IN EFI_HANDLE                DeviceHandle
  )
/*++

Routine Description:

  Add IsaKeyboard to ConIn,
  add IsaSerial to ConOut, ConIn, ErrOut.
  LPC Bridge: 06 01 00

Arguments:

  DeviceHandle            - Handle of PCIIO protocol.
 
Returns:

  EFI_SUCCESS             - LPC bridge is added to ConOut, ConIn, and ErrOut.
  EFI_STATUS              - No LPC bridge is added.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;

  DevicePath = NULL;
  Status = gBS->HandleProtocol (
                  DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  &DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  TempDevicePath = DevicePath;

  //
  // Register Keyboard
  //
  DevicePath = EfiAppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gPnpPs2KeyboardDeviceNode);

  BdsLibUpdateConsoleVariable (L"ConIn", DevicePath, NULL);

  //
  // Register COM1
  //
  DevicePath = TempDevicePath;
  gPnp16550ComPortDeviceNode.UID = 0;

  DevicePath = EfiAppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gPnp16550ComPortDeviceNode);
  DevicePath = EfiAppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode);
  DevicePath = EfiAppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode);

  BdsLibUpdateConsoleVariable (L"ConOut", DevicePath, NULL);
  BdsLibUpdateConsoleVariable (L"ConIn", DevicePath, NULL);
  BdsLibUpdateConsoleVariable (L"ErrOut", DevicePath, NULL);

  //
  // Register COM2
  //
  DevicePath = TempDevicePath;
  gPnp16550ComPortDeviceNode.UID = 1;

  DevicePath = EfiAppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gPnp16550ComPortDeviceNode);
  DevicePath = EfiAppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode);
  DevicePath = EfiAppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode);

  BdsLibUpdateConsoleVariable (L"ConOut", DevicePath, NULL);
  BdsLibUpdateConsoleVariable (L"ConIn", DevicePath, NULL);
  BdsLibUpdateConsoleVariable (L"ErrOut", DevicePath, NULL);

  return EFI_SUCCESS;
}

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
EFI_STATUS
GetGopDevicePath (
   IN  EFI_DEVICE_PATH_PROTOCOL *PciDevicePath,
   OUT EFI_DEVICE_PATH_PROTOCOL **GopDevicePath
   )
{
  UINTN                           Index;
  EFI_STATUS                      Status;
  EFI_HANDLE                      PciDeviceHandle;
  EFI_DEVICE_PATH_PROTOCOL        *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        *TempPciDevicePath;
  UINTN                           GopHandleCount;
  EFI_HANDLE                      *GopHandleBuffer;

  if (PciDevicePath == NULL || GopDevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Initialize the GopDevicePath to be PciDevicePath
  //
  *GopDevicePath    = PciDevicePath;
  TempPciDevicePath = PciDevicePath;

  Status = gBS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &TempPciDevicePath,
                  &PciDeviceHandle
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Try to connect this handle, so that GOP dirver could start on this 
  // device and create child handles with GraphicsOutput Protocol installed
  // on them, then we get device paths of these child handles and select 
  // them as possible console device.
  //
  gBS->ConnectController (PciDeviceHandle, NULL, NULL, FALSE);

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &GopHandleCount,
                  &GopHandleBuffer
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Add all the child handles as possible Console Device
    //
    for (Index = 0; Index < GopHandleCount; Index++) {
      Status = gBS->HandleProtocol (GopHandleBuffer[Index], &gEfiDevicePathProtocolGuid, &TempDevicePath);
      if (EFI_ERROR (Status)) {
        continue;
      }
      if (EfiCompareMem (
            PciDevicePath,
            TempDevicePath,
            EfiDevicePathSize (PciDevicePath) - END_DEVICE_PATH_LENGTH
            ) == 0) {
        //
        // In current implementation, we only enable one of the child handles
        // as console device, i.e. sotre one of the child handle's device
        // path to variable "ConOut"
        // In futhure, we could select all child handles to be console device
        //       

        *GopDevicePath = TempDevicePath;

        //
        // Delete the PCI device's path that added by GetPlugInPciVgaDevicePath()
        // Add the integrity GOP device path.
        //
        BdsLibUpdateConsoleVariable (L"ConOutDev", NULL, PciDevicePath);
        BdsLibUpdateConsoleVariable (L"ConOutDev", TempDevicePath, NULL);
      }
    }
    gBS->FreePool (GopHandleBuffer);
  }

  return EFI_SUCCESS;
}
#endif

EFI_STATUS
PreparePciVgaDevicePath (
  IN EFI_HANDLE                DeviceHandle
  )
/*++

Routine Description:

  Add PCI VGA to ConOut.
  PCI VGA: 03 00 00

Arguments:

  DeviceHandle            - Handle of PCIIO protocol.
 
Returns:

  EFI_SUCCESS             - PCI VGA is added to ConOut.
  EFI_STATUS              - No PCI VGA device is added.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  EFI_DEVICE_PATH_PROTOCOL  *GopDevicePath;
#endif  

  DevicePath = NULL;
  Status = gBS->HandleProtocol (
                  DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  &DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  GetGopDevicePath (DevicePath, &GopDevicePath);
  DevicePath = GopDevicePath;
#endif

  BdsLibUpdateConsoleVariable (L"ConOut", DevicePath, NULL);
  
  return EFI_SUCCESS;
}

EFI_STATUS
PreparePciSerialDevicePath (
  IN EFI_HANDLE                DeviceHandle
  )
/*++

Routine Description:

  Add PCI Serial to ConOut, ConIn, ErrOut.
  PCI Serial: 07 00 02

Arguments:

  DeviceHandle            - Handle of PCIIO protocol.
 
Returns:

  EFI_SUCCESS             - PCI Serial is added to ConOut, ConIn, and ErrOut.
  EFI_STATUS              - No PCI Serial device is added.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  DevicePath = NULL;
  Status = gBS->HandleProtocol (
                  DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  &DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DevicePath = EfiAppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode);
  DevicePath = EfiAppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode);

  BdsLibUpdateConsoleVariable (L"ConOut", DevicePath, NULL);
  BdsLibUpdateConsoleVariable (L"ConIn", DevicePath, NULL);
  BdsLibUpdateConsoleVariable (L"ErrOut", DevicePath, NULL);
  
  return EFI_SUCCESS;
}

EFI_STATUS
DetectAndPreparePlatformPciDevicePath (
  VOID
  )
/*++

Routine Description:

  Do platform specific PCI Device check and add them to ConOut, ConIn, ErrOut

Arguments:

  None.
 
Returns:

  EFI_SUCCESS             - PCI Device check and Console variable update successfully.
  EFI_STATUS              - PCI Device check or Console variable update fail.

--*/
{
  EFI_STATUS                Status;
  UINTN                     HandleCount;
  EFI_HANDLE                *HandleBuffer;
  UINTN                     Index;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  PCI_TYPE00                Pci;

  //
  // Start to check all the PciIo to find all possible device
  //
  HandleCount = 0;
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiPciIoProtocolGuid, &PciIo);
    if (EFI_ERROR (Status)) {
      continue;
    }

    //
    // Check for all PCI device
    //
    Status = PciIo->Pci.Read (
                      PciIo,
                      EfiPciIoWidthUint32,
                      0,
                      sizeof (Pci) / sizeof (UINT32),
                      &Pci
                      );
    if (EFI_ERROR (Status)) {
      continue;
    }

    //
    // Here we decide whether it is LPC Bridge
    //
    if ((IS_PCI_LPC (&Pci)) ||
        ((IS_PCI_ISA_PDECODE (&Pci)) && (Pci.Hdr.VendorId == 0x8086) && (Pci.Hdr.DeviceId == 0x7110))) {
      //
      // Add IsaKeyboard to ConIn,
      // add IsaSerial to ConOut, ConIn, ErrOut
      //
      PrepareLpcBridgeDevicePath (HandleBuffer[Index]);
      continue;
    }

    //
    // Here we decide which VGA device to enable in PCI bus 
    //
    if (IS_PCI_VGA (&Pci)) {
      //
      // Add them to ConOut.
      //
      PreparePciVgaDevicePath (HandleBuffer[Index]);
      continue;
    }

    //
    // Here we decide which Serial device to enable in PCI bus 
    //
    if (IS_PCI_16550SERIAL (&Pci)) {
      //
      // Add them to ConOut, ConIn, ErrOut.
      //
      PreparePciSerialDevicePath (HandleBuffer[Index]);
      continue;
    }

  }
  
  gBS->FreePool (HandleBuffer);
  
  return EFI_SUCCESS;
}

EFI_STATUS
PlatformBdsConnectConsole (
  IN BDS_CONSOLE_CONNECT_ENTRY   *PlatformConsole
  )
/*++

Routine Description:

  Connect the predefined platform default console device. Always try to find
  and enable the vga device if have.

Arguments:

  PlatformConsole         - Predfined platform default console device array.
 
Returns:

  EFI_SUCCESS             - Success connect at least one ConIn and ConOut 
                            device, there must have one ConOut device is 
                            active vga device.
  
  EFI_STATUS              - Return the status of 
                            BdsLibConnectAllDefaultConsoles ()

--*/
{
  EFI_STATUS  Status;
  UINTN       Index;

  Index   = 0;
  Status  = EFI_SUCCESS;

  //
  // Connect RootBridge
  //
  ConnectRootBridge ();

  //
  // Do platform specific PCI Device check and add them to ConOut, ConIn, ErrOut
  //
  DetectAndPreparePlatformPciDevicePath ();

  //
  // Have chance to connect the platform default console,
  // the platform default console is the minimue device group
  // the platform should support
  //
  while (PlatformConsole[Index].DevicePath != NULL) {
    //
    // Update the console variable with the connect type
    //
    if ((PlatformConsole[Index].ConnectType & CONSOLE_IN) == CONSOLE_IN) {
      BdsLibUpdateConsoleVariable (L"ConIn", PlatformConsole[Index].DevicePath, NULL);
    }

    if ((PlatformConsole[Index].ConnectType & CONSOLE_OUT) == CONSOLE_OUT) {
      BdsLibUpdateConsoleVariable (L"ConOut", PlatformConsole[Index].DevicePath, NULL);
    }

    if ((PlatformConsole[Index].ConnectType & STD_ERROR) == STD_ERROR) {
      BdsLibUpdateConsoleVariable (L"ErrOut", PlatformConsole[Index].DevicePath, NULL);
    }

    Index++;
  }

  //
  // Connect the all the default console with current cosole variable
  //
  Status = BdsLibConnectAllDefaultConsoles ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

VOID
PlatformBdsConnectSequence (
  VOID
  )
/*++

Routine Description:

  Connect with predeined platform connect sequence, 
  the OEM/IBV can customize with their own connect sequence.
  
Arguments:

  None.
 
Returns:

  None.
  
--*/
{
  UINTN Index;

  Index = 0;

  //
  // Here we can get the customized platform connect sequence
  // Notes: we can connect with new variable which record the
  // last time boots connect device path sequence
  //
  while (gPlatformConnectSequence[Index] != NULL) {
    //
    // Build the platform boot option
    //
    BdsLibConnectDevicePath (gPlatformConnectSequence[Index]);
    Index++;
  }

}

VOID
PlatformBdsGetDriverOption (
  IN OUT EFI_LIST_ENTRY              *BdsDriverLists
  )
/*++

Routine Description:

  Load the predefined driver option, OEM/IBV can customize this
  to load their own drivers
  
Arguments:

  BdsDriverLists  - The header of the driver option link list.
 
Returns:

  None.
  
--*/
{
  UINTN Index;

  Index = 0;

  //
  // Here we can get the customized platform driver option
  //
  while (gPlatformDriverOption[Index] != NULL) {
    //
    // Build the platform boot option
    //
    BdsLibRegisterNewOption (BdsDriverLists, gPlatformDriverOption[Index], NULL, L"DriverOrder");
    Index++;
  }

}

VOID
PlatformBdsDiagnostics (
  IN EXTENDMEM_COVERAGE_LEVEL    MemoryTestLevel,
  IN BOOLEAN                     QuietBoot
  )
/*++

Routine Description:

  Perform the platform diagnostic, such like test memory. OEM/IBV also
  can customize this fuction to support specific platform diagnostic.
  
Arguments:

  MemoryTestLevel  - The memory test intensive level
  
  QuietBoot        - Indicate if need to enable the quiet boot
 
Returns:

  None.
  
--*/
{
  EFI_STATUS  Status;

  //
  // Here we can decide if we need to show
  // the diagnostics screen
  // Notes: this quiet boot code should be remove
  // from the graphic lib
  //
  if (QuietBoot) {
    EnableQuietBoot (&gEfiUgaSplashProtocolGuid);
    //
    // Perform system diagnostic
    //
    Status = BdsMemoryTest (MemoryTestLevel);
    if (EFI_ERROR (Status)) {
      DisableQuietBoot ();
    }

    return ;
  }
  //
  // Perform system diagnostic
  //
  Status = BdsMemoryTest (MemoryTestLevel);
}

VOID
PlatformBdsPolicyBehavior (
  IN EFI_BDS_ARCH_PROTOCOL_INSTANCE  *PrivateData,
  IN OUT EFI_LIST_ENTRY              *DriverOptionList,
  IN OUT EFI_LIST_ENTRY              *BootOptionList
  )
/*++

Routine Description:

  The function will excute with as the platform policy, current policy
  is driven by boot mode. IBV/OEM can customize this code for their specific
  policy action.
  
Arguments:

  PrivateData      - The EFI_BDS_ARCH_PROTOCOL_INSTANCE instance
  
  DriverOptionList - The header of the driver option link list
  
  BootOptionList   - The header of the boot option link list
 
Returns:

  None.
  
--*/
{
  EFI_STATUS  Status;
  UINT16      Timeout;

  //
  // Init the time out value
  //
  Timeout = BdsLibGetTimeout ();

  //
  // Load the driver option as the driver option list
  //
  PlatformBdsGetDriverOption (DriverOptionList);

  //
  // Get current Boot Mode
  //
  Status = BdsLibGetBootMode (&PrivateData->BootMode);
  DEBUG ((EFI_D_ERROR, "Boot Mode:%x\n", PrivateData->BootMode));

  //
  // Go the different platform policy with different boot mode
  // Notes: this part code can be change with the table policy
  //
  switch (PrivateData->BootMode) {

  case BOOT_ASSUMING_NO_CONFIGURATION_CHANGES:
  case BOOT_WITH_MINIMAL_CONFIGURATION:
    //
    // In no-configuration boot mode, we can connect the
    // console directly.
    //
    BdsLibConnectAllDefaultConsoles ();
    PlatformBdsDiagnostics (IGNORE, TRUE);

    //
    // Perform some platform specific connect sequence
    //
    PlatformBdsConnectSequence ();

    //
    // Notes: current time out = 0 can not enter the
    // front page
    //
    PlatformBdsEnterFrontPage (Timeout, FALSE);

    //
    // Check the boot option with the boot option list
    //
    BdsLibBuildOptionFromVar (BootOptionList, L"BootOrder");
    break;

  case BOOT_ON_FLASH_UPDATE:
    //
    // Boot with the specific configuration
    //
    PlatformBdsConnectConsole (gPlatformConsole);
    PlatformBdsDiagnostics (EXTENSIVE, FALSE);
    BdsLibConnectAll ();
    ProcessCapsules (BOOT_ON_FLASH_UPDATE);
    break;

  case BOOT_IN_RECOVERY_MODE:
    //
    // In recovery mode, just connect platform console
    // and show up the front page
    //
    PlatformBdsConnectConsole (gPlatformConsole);
    PlatformBdsDiagnostics (EXTENSIVE, FALSE);

    //
    // In recovery boot mode, we still enter to the
    // frong page now
    //
    PlatformBdsEnterFrontPage (Timeout, FALSE);
    break;

  case BOOT_WITH_FULL_CONFIGURATION:
  case BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS:
  case BOOT_WITH_DEFAULT_SETTINGS:
  default:
    //
    // Connect platform console
    //
    Status = PlatformBdsConnectConsole (gPlatformConsole);
    if (EFI_ERROR (Status)) {
      //
      // Here OEM/IBV can customize with defined action
      //
      PlatformBdsNoConsoleAction ();
    }

    PlatformBdsDiagnostics (IGNORE, TRUE);

    //
    // Perform some platform specific connect sequence
    //
    PlatformBdsConnectSequence ();

    //
    // Give one chance to enter the setup if we
    // have the time out
    //
    PlatformBdsEnterFrontPage (Timeout, FALSE);

    //
    // Here we have enough time to do the enumeration of boot device
    //
    if (!gConnectAllHappened) {
      BdsLibConnectAllDriversToAllControllers ();
      gConnectAllHappened = TRUE;
    }
    BdsLibEnumerateAllBootOption (BootOptionList);
    break;
  }

  return ;

}

VOID
PlatformBdsBootSuccess (
  IN  BDS_COMMON_OPTION *Option
  )
/*++

Routine Description:
  
  Hook point after a boot attempt succeeds. We don't expect a boot option to
  return, so the EFI 1.0 specification defines that you will default to an
  interactive mode and stop processing the BootOrder list in this case. This
  is alos a platform implementation and can be customized by IBV/OEM.

Arguments:

  Option - Pointer to Boot Option that succeeded to boot.

Returns:
  
  None.

--*/
{
  CHAR16  *TmpStr;

  //
  // If Boot returned with EFI_SUCCESS and there is not in the boot device
  // select loop then we need to pop up a UI and wait for user input.
  //
  TmpStr = GetStringById (STRING_TOKEN (STR_BOOT_SUCCEEDED));
  if (TmpStr != NULL) {
    BdsLibOutputStrings (gST->ConOut, TmpStr, Option->Description, L"\n\r", NULL);
    gBS->FreePool (TmpStr);
  }
}

VOID
PlatformBdsBootFail (
  IN  BDS_COMMON_OPTION  *Option,
  IN  EFI_STATUS         Status,
  IN  CHAR16             *ExitData,
  IN  UINTN              ExitDataSize
  )
/*++

Routine Description:
  
  Hook point after a boot attempt fails.

Arguments:
  
  Option - Pointer to Boot Option that failed to boot.

  Status - Status returned from failed boot.

  ExitData - Exit data returned from failed boot.

  ExitDataSize - Exit data size returned from failed boot.

Returns:
  
  None.

--*/
{
  CHAR16  *TmpStr;

  //
  // If Boot returned with failed status then we need to pop up a UI and wait
  // for user input.
  //
  TmpStr = GetStringById (STRING_TOKEN (STR_BOOT_FAILED));
  if (TmpStr != NULL) {
    BdsLibOutputStrings (gST->ConOut, TmpStr, Option->Description, L"\n\r", NULL);
    gBS->FreePool (TmpStr);
  }

}

EFI_STATUS
PlatformBdsNoConsoleAction (
  VOID
  )
/*++

Routine Description:
  
  This function is remained for IBV/OEM to do some platform action,
  if there no console device can be connected.

Arguments:
  
  None.
  
Returns:
  
  EFI_SUCCESS      - Direct return success now.

--*/
{
  return EFI_SUCCESS;
}
