/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  BiosVideo.c
    
Abstract:

  ConsoleOut Routines that speak VGA.

Revision History

--*/

#include "BiosVideo.h"

//
// EFI Driver Binding Protocol Instance
//
EFI_DRIVER_BINDING_PROTOCOL gBiosVideoDriverBinding = {
  BiosVideoDriverBindingSupported,
  BiosVideoDriverBindingStart,
  BiosVideoDriverBindingStop,
  0x00000024,
  NULL,
  NULL
};

//
// Global lookup tables for VGA graphics modes
//
UINT8                       mVgaLeftMaskTable[]   = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };

UINT8                       mVgaRightMaskTable[]  = { 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

UINT8                       mVgaBitMaskTable[]    = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

EFI_UGA_PIXEL               mVgaColorToUgaColor[] = {
  {
    0x00,
    0x00,
    0x00,
    0x00
  },
  {
    0x98,
    0x00,
    0x00,
    0x00
  },
  {
    0x00,
    0x98,
    0x00,
    0x00
  },
  {
    0x98,
    0x98,
    0x00,
    0x00
  },
  {
    0x00,
    0x00,
    0x98,
    0x00
  },
  {
    0x98,
    0x00,
    0x98,
    0x00
  },
  {
    0x00,
    0x98,
    0x98,
    0x00
  },
  {
    0x98,
    0x98,
    0x98,
    0x00
  },
  {
    0x10,
    0x10,
    0x10,
    0x00
  },
  {
    0xff,
    0x10,
    0x10,
    0x00
  },
  {
    0x10,
    0xff,
    0x10,
    0x00
  },
  {
    0xff,
    0xff,
    0x10,
    0x00
  },
  {
    0x10,
    0x10,
    0xff,
    0x00
  },
  {
    0xf0,
    0x10,
    0xff,
    0x00
  },
  {
    0x10,
    0xff,
    0xff,
    0x00
  },
  {
    0xff,
    0xff,
    0xff,
    0x00
  }
};

//
// Driver Entry Point
//
EFI_DRIVER_ENTRY_POINT (BiosVideoDriverEntryPoint)

EFI_STATUS
EFIAPI
BiosVideoDriverEntryPoint (
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
// TODO:    ImageHandle - add argument and description to function comment
// TODO:    SystemTable - add argument and description to function comment
{
  EFI_STATUS  Status;

  Status = EfiLibInstallAllDriverProtocols (
            ImageHandle,
            SystemTable,
            &gBiosVideoDriverBinding,
            ImageHandle,
            &gBiosVideoComponentName,
            NULL,
            NULL
            );

  return Status;
}

VOID
BiosVideoExitBootServices (
  EFI_EVENT  Event,
  VOID       *Context
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Event   - TODO: add argument description
  Context - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  BIOS_VIDEO_DEV        *Private;
  EFI_IA32_REGISTER_SET Regs;

  //
  // Get our context
  //
  Private = (BIOS_VIDEO_DEV *) Context;

  //
  // Set the 80x25 Text VGA Mode
  //
  Regs.H.AH = 0x00;
  Regs.H.AL = 0x83;
  Private->LegacyBiosThunk->Int86 (Private->LegacyBiosThunk, 0x10, &Regs);

  Regs.H.AH = 0x11;
  Regs.H.AL = 0x14;
  Regs.H.BL = 0;
  Private->LegacyBiosThunk->Int86 (Private->LegacyBiosThunk, 0x10, &Regs);
}

EFI_STATUS
EFIAPI
BiosVideoDriverBindingSupported (
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
// TODO:    This - add argument and description to function comment
// TODO:    Controller - add argument and description to function comment
// TODO:    RemainingDevicePath - add argument and description to function comment
{
  EFI_STATUS                  Status;
  LEGACY_BIOS_THUNK_PROTOCOL  *LegacyBiosThunk;
  EFI_PCI_IO_PROTOCOL         *PciIo;

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
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!BiosVideoIsVga (PciIo)) {
    Status = EFI_UNSUPPORTED;
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
BiosVideoDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Install UGA Draw Protocol onto VGA device handles
  
  Arguments:
    (Standard DriverBinding Protocol Start() function)
    
  Returns:
    EFI_STATUS
    
--*/
// TODO:    This - add argument and description to function comment
// TODO:    Controller - add argument and description to function comment
// TODO:    RemainingDevicePath - add argument and description to function comment
{
  EFI_STATUS      Status;
  BIOS_VIDEO_DEV  *Private;

  //
  // Initialize local variables
  //
  Private = NULL;

  //
  // Allocate the private device structure
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (BIOS_VIDEO_DEV),
                  &Private
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  EfiZeroMem (Private, sizeof (BIOS_VIDEO_DEV));

  //
  // See if the Legacy BIOS Protocol is available
  //
  Status = gBS->LocateProtocol (&gLegacyBiosThunkProtocolGuid, NULL, (VOID **) &Private->LegacyBiosThunk);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Prepare for status code
  //
  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  &Private->DevicePath
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Open the IO Abstraction(s) needed
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &(Private->PciIo),
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if (!BiosVideoIsVga (Private->PciIo)) {
    Status = EFI_UNSUPPORTED;
    goto Done;
  }
  
  Private->VgaCompatible = TRUE; 
  //
  // Initialize the private device structure
  //
  Private->Signature = BIOS_VIDEO_DEV_SIGNATURE;
  Private->Handle    = Controller;

  Status = gBS->CreateEvent (
                  EFI_EVENT_SIGNAL_EXIT_BOOT_SERVICES,
                  EFI_TPL_NOTIFY,
                  BiosVideoExitBootServices,
                  Private,
                  &Private->ExitBootServicesEvent
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Fill in UGA Draw specific mode structures
  //
  Private->HardwareNeedsStarting = TRUE;
  Private->CurrentMode           = 0;
  Private->MaxMode               = 0;
  Private->ModeData              = NULL;
  Private->LineBuffer            = NULL;
  Private->VgaFrameBuffer        = NULL;
  Private->VbeFrameBuffer        = NULL;

  //
  // Check for VESA BIOS Extensions for modes that are compatible with UGA Draw
  //
  Status = BiosVideoCheckForVbe (Private);
  if (EFI_ERROR (Status)) {
    //
    // The VESA BIOS Extensions are not compatible with UGA Draw, so check for support
    // for the standard 640x480 16 color VGA mode
    //
    if (Private->VgaCompatible) {
      Status = BiosVideoCheckForVga (Private);
    }
    if (EFI_ERROR (Status)) {
      goto Done;
    }
  }

  //
  // Install UGA Draw Protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiUgaDrawProtocolGuid,
                  &Private->UgaDraw,
                  NULL
                  );

Done:
  if (EFI_ERROR (Status)) {
    if (Private != NULL) {
      //
      // Free mode data
      //
      if (Private->ModeData != NULL) {
        gBS->FreePool (Private->ModeData);
      }
      //
      // Free memory allocated below 1MB
      //
      if (Private->PagesBelow1MB != 0) {
        gBS->FreePages (Private->PagesBelow1MB, Private->NumberOfPagesBelow1MB);
      }

      if (Private->PciIo != NULL) {
        //
        // Release PCI I/O and UGA Draw Protocols on the controller handle.
        //
        gBS->CloseProtocol (
              Controller,
              &gEfiPciIoProtocolGuid,
              This->DriverBindingHandle,
              Controller
              );
      }
      //
      // Close the ExitBootServices event
      //
      if (Private->ExitBootServicesEvent != NULL) {
        gBS->CloseEvent (Private->ExitBootServicesEvent);
      }
      //
      // Free private data structure
      //
      gBS->FreePool (Private);
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
BiosVideoDriverBindingStop (
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
// TODO:    This - add argument and description to function comment
// TODO:    Controller - add argument and description to function comment
// TODO:    NumberOfChildren - add argument and description to function comment
// TODO:    ChildHandleBuffer - add argument and description to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS                  Status;
  EFI_UGA_DRAW_PROTOCOL       *Uga;
  BIOS_VIDEO_DEV              *Private;
  EFI_IA32_REGISTER_SET       Regs;

  Private = NULL;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUgaDrawProtocolGuid,
                  (VOID **) &Uga,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    Private = BIOS_VIDEO_DEV_FROM_UGA_DRAW_THIS (Uga);
  }

  if (Private == NULL) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Controller,
                  &gEfiUgaDrawProtocolGuid,
                  &Private->UgaDraw,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Set the 80x25 Text VGA Mode
  //
  Regs.H.AH = 0x00;
  Regs.H.AL = 0x03;
  Private->LegacyBiosThunk->Int86 (Private->LegacyBiosThunk, 0x10, &Regs);

  Regs.H.AH = 0x11;
  Regs.H.AL = 0x14;
  Regs.H.BL = 0;
  Private->LegacyBiosThunk->Int86 (Private->LegacyBiosThunk, 0x10, &Regs);

  //
  // Free VGA Frame Buffer
  //
  if (Private->VgaFrameBuffer != NULL) {
    gBS->FreePool (Private->VgaFrameBuffer);
  }

  //
  // Free VBE Frame Buffer
  //
  if (Private->VbeFrameBuffer != NULL) {
    gBS->FreePool (Private->VbeFrameBuffer);
  }

  //
  // Free line buffer
  //
  if (Private->LineBuffer != NULL) {
    gBS->FreePool (Private->LineBuffer);
  }

  //
  // Free mode data
  //
  if (Private->ModeData != NULL) {
    gBS->FreePool (Private->ModeData);
  }

  //
  // Free memory allocated below 1MB
  //
  if (Private->PagesBelow1MB != 0) {
    gBS->FreePages (Private->PagesBelow1MB, Private->NumberOfPagesBelow1MB);
  }

  if (Private->VbeSaveRestorePages != 0) {
    gBS->FreePages (Private->VbeSaveRestoreBuffer, Private->VbeSaveRestorePages);
  }

  //
  // Release PCI I/O and UGA Draw Protocols on the controller handle.
  //
  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  //
  // Close the ExitBootServices event
  //
  gBS->CloseEvent (Private->ExitBootServicesEvent);

  //
  // Free private data structure
  //
  gBS->FreePool (Private);

  return EFI_SUCCESS;
}

#define PCI_DEVICE_ENABLED  (EFI_PCI_COMMAND_IO_SPACE | EFI_PCI_COMMAND_MEMORY_SPACE)


BOOLEAN
BiosVideoIsVga (
  IN  EFI_PCI_IO_PROTOCOL       *PciIo
  )
{
  EFI_STATUS    Status;
  BOOLEAN       VgaCompatible;
  PCI_TYPE00    Pci;

  VgaCompatible = FALSE;

  //
  // Read the PCI Configuration Header
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        sizeof (Pci) / sizeof (UINT32),
                        &Pci
                        );
  if (EFI_ERROR (Status)) {
    return VgaCompatible;
  }

  //
  // See if this is a VGA compatible controller or not
  //
  if ((Pci.Hdr.Command & PCI_DEVICE_ENABLED) == PCI_DEVICE_ENABLED) {
    if (Pci.Hdr.ClassCode[2] == PCI_CLASS_OLD && Pci.Hdr.ClassCode[1] == PCI_CLASS_OLD_VGA) {
      //
      // Base Class 0x00 Sub-Class 0x01 - Backward compatible VGA device
      //
      VgaCompatible = TRUE;
    }

    if (Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY && Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA && Pci.Hdr.ClassCode[0] == 0x00) {
      //
      // Base Class 3 Sub-Class 0 Programming interface 0 - VGA compatible Display controller
      //
      VgaCompatible = TRUE;
    }
  }

  return VgaCompatible;
}


EFI_STATUS
BiosVideoCheckForVbe (
  BIOS_VIDEO_DEV  *Private  // TODO: add IN/OUT modifier to Private
  )
/*++
  
  Routine Description:

    BiosVideoCheckForVbe
  
  Arguments:
  
  Returns:
  
--*/
// TODO:    Private - add argument and description to function comment
{
  EFI_STATUS            Status;
  EFI_IA32_REGISTER_SET Regs;
  UINT16                *ModeNumberPtr;
  BOOLEAN               ModeFound;
  BIOS_VIDEO_MODE_DATA  *ModeBuffer;
  UINTN                 Index;

  //
  // Allocate buffer under 1MB for VBE data structures
  //
  Private->NumberOfPagesBelow1MB = EFI_SIZE_TO_PAGES (
                                              sizeof (VESA_BIOS_EXTENSIONS_INFORMATION_BLOCK) + sizeof (VESA_BIOS_EXTENSIONS_MODE_INFORMATION_BLOCK) +
                                              sizeof (VESA_BIOS_EXTENSIONS_CRTC_INFORMATION_BLOCK)
                                              );

  Private->PagesBelow1MB = 0x00100000 - 1;

  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiBootServicesData,
                  Private->NumberOfPagesBelow1MB,
                  &Private->PagesBelow1MB
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Fill in the UGA Draw Protocol
  //
  Private->UgaDraw.GetMode = BiosVideoUgaDrawGetMode;
  Private->UgaDraw.SetMode = BiosVideoUgaDrawSetMode;
  Private->UgaDraw.Blt     = BiosVideoUgaDrawVbeBlt;

  //
  // Fill in the VBE related data structures
  //
  Private->VbeInformationBlock = (VESA_BIOS_EXTENSIONS_INFORMATION_BLOCK *) (UINTN) (Private->PagesBelow1MB);
  Private->VbeModeInformationBlock = (VESA_BIOS_EXTENSIONS_MODE_INFORMATION_BLOCK *) (Private->VbeInformationBlock + 1);
  Private->VbeCrtcInformationBlock = (VESA_BIOS_EXTENSIONS_CRTC_INFORMATION_BLOCK *) (Private->VbeModeInformationBlock + 1);
  Private->VbeSaveRestorePages   = 0;
  Private->VbeSaveRestoreBuffer  = 0;

  //
  // Test to see if the Video Adapter is compliant with VBE 3.0
  //
  gBS->SetMem (&Regs, sizeof (Regs), 0);
  Regs.X.AX = VESA_BIOS_EXTENSIONS_RETURN_CONTROLLER_INFORMATION;
  gBS->SetMem (Private->VbeInformationBlock, sizeof (VESA_BIOS_EXTENSIONS_INFORMATION_BLOCK), 0);
  Private->VbeInformationBlock->VESASignature  = VESA_BIOS_EXTENSIONS_VBE2_SIGNATURE;
  Regs.X.ES = EFI_SEGMENT (Private->VbeInformationBlock);
  Regs.X.DI = EFI_OFFSET (Private->VbeInformationBlock);
  Private->LegacyBiosThunk->Int86 (Private->LegacyBiosThunk, 0x10, &Regs);

  Status = EFI_DEVICE_ERROR;

  //
  // See if the VESA call succeeded
  //
  if (Regs.X.AX != VESA_BIOS_EXTENSIONS_STATUS_SUCCESS) {
    return Status;
  }

  //
  // Check for 'VESA' signature
  //
  if (Private->VbeInformationBlock->VESASignature != VESA_BIOS_EXTENSIONS_VESA_SIGNATURE) {
    return Status;
  }

  //
  // Check to see if this is VBE 2.0 or higher
  //
  if (Private->VbeInformationBlock->VESAVersion < VESA_BIOS_EXTENSIONS_VERSION_2_0) {
    return Status;
  }

  //
  // Walk through the mode list to see if there is at least one mode the is compatible with the UGA_DRAW protocol
  //
  ModeNumberPtr = (UINT16 *)
    (
      (((UINTN) Private->VbeInformationBlock->VideoModePtr & 0xffff0000) >> 12) |
        ((UINTN) Private->VbeInformationBlock->VideoModePtr & 0x0000ffff)
    );
  for (; *ModeNumberPtr != VESA_BIOS_EXTENSIONS_END_OF_MODE_LIST; ModeNumberPtr++) {
    //
    // Make sure this is a mode number defined by the VESA VBE specification.  If it isn'tm then skip this mode number.
    //
    if ((*ModeNumberPtr & VESA_BIOS_EXTENSIONS_MODE_NUMBER_VESA) == 0) {
      continue;
    }

    //
    // Get the information about the mode
    //
    gBS->SetMem (&Regs, sizeof (Regs), 0);
    Regs.X.AX = VESA_BIOS_EXTENSIONS_RETURN_MODE_INFORMATION;
    Regs.X.CX = *ModeNumberPtr;
    gBS->SetMem (Private->VbeModeInformationBlock, sizeof (VESA_BIOS_EXTENSIONS_MODE_INFORMATION_BLOCK), 0);
    Regs.X.ES = EFI_SEGMENT (Private->VbeModeInformationBlock);
    Regs.X.DI = EFI_OFFSET (Private->VbeModeInformationBlock);

    Private->LegacyBiosThunk->Int86 (Private->LegacyBiosThunk, 0x10, &Regs);

    //
    // See if the call succeeded.  If it didn't, then try the next mode.
    //
    if (Regs.X.AX != VESA_BIOS_EXTENSIONS_STATUS_SUCCESS) {
      continue;
    }

    //
    // See if the mode supports color.  If it doesn't then try the next mode.
    //
    if ((Private->VbeModeInformationBlock->ModeAttributes & VESA_BIOS_EXTENSIONS_MODE_ATTRIBUTE_COLOR) == 0) {
      continue;
    }

    //
    // See if the mode supports graphics.  If it doesn't then try the next mode.
    //
    if ((Private->VbeModeInformationBlock->ModeAttributes & VESA_BIOS_EXTENSIONS_MODE_ATTRIBUTE_GRAPHICS) == 0) {
      continue;
    }

    //
    // See if the mode supports a linear frame buffer.  If it doesn't then try the next mode.
    //
    if ((
          Private->VbeModeInformationBlock->ModeAttributes &
          VESA_BIOS_EXTENSIONS_MODE_ATTRIBUTE_LINEAR_FRAME_BUFFER
    ) == 0
        ) {
      continue;
    }

    //
    // See if the mode supports 32 bit color.  If it doesn't then try the next mode.
    // 32 bit mode can be implemented by 24 Bits Per Pixels. Also make sure the
    // number of bits per pixel is a multiple of 8 or more than 32 bits per pixel
    //
    if (Private->VbeModeInformationBlock->BitsPerPixel < 24) {
      continue;
    }

    if (Private->VbeModeInformationBlock->BitsPerPixel > 32) {
      continue;
    }

    if ((Private->VbeModeInformationBlock->BitsPerPixel % 8) != 0) {
      continue;
    }
    //
    // See if the physical base pointer for the linear mode is valid.  If it isn't then try the next mode.
    //
    if (Private->VbeModeInformationBlock->PhysBasePtr == 0) {
      continue;
    }
    //
    // See if the resolution is 1024x768, 800x600, or 640x480
    //
    ModeFound = FALSE;
    if (Private->VbeModeInformationBlock->XResolution == 1024 &&
        Private->VbeModeInformationBlock->YResolution == 768
        ) {
      ModeFound = TRUE;
    }

    if (Private->VbeModeInformationBlock->XResolution == 800 &&
        Private->VbeModeInformationBlock->YResolution == 600
        ) {
      ModeFound = TRUE;
    }

    if (Private->VbeModeInformationBlock->XResolution == 640 &&
        Private->VbeModeInformationBlock->YResolution == 480
        ) {
      ModeFound = TRUE;
    }

    if (!ModeFound) {
      continue;
    }
    //
    // Add mode to the list of available modes
    //
    Private->MaxMode++;
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    Private->MaxMode * sizeof (BIOS_VIDEO_MODE_DATA),
                    (VOID **) &ModeBuffer
                    );
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    if (Private->MaxMode > 1) {
      gBS->CopyMem (
            ModeBuffer,
            Private->ModeData,
            (Private->MaxMode - 1) * sizeof (BIOS_VIDEO_MODE_DATA)
            );
    }

    if (Private->ModeData != NULL) {
      gBS->FreePool (Private->ModeData);
    }

    ModeBuffer[Private->MaxMode - 1].VbeModeNumber = *ModeNumberPtr;
    if (Private->VbeInformationBlock->VESAVersion >= VESA_BIOS_EXTENSIONS_VERSION_3_0) {
      ModeBuffer[Private->MaxMode - 1].BytesPerScanLine = Private->VbeModeInformationBlock->LinBytesPerScanLine;
      ModeBuffer[Private->MaxMode - 1].Red.Position = Private->VbeModeInformationBlock->LinRedFieldPosition;
      ModeBuffer[Private->MaxMode - 1].Red.Mask = (UINT8) ((1 << Private->VbeModeInformationBlock->LinRedMaskSize) - 1);
      ModeBuffer[Private->MaxMode - 1].Blue.Position = Private->VbeModeInformationBlock->LinBlueFieldPosition;
      ModeBuffer[Private->MaxMode - 1].Blue.Mask = (UINT8) ((1 << Private->VbeModeInformationBlock->LinBlueMaskSize) - 1);
      ModeBuffer[Private->MaxMode - 1].Green.Position = Private->VbeModeInformationBlock->LinGreenFieldPosition;
      ModeBuffer[Private->MaxMode - 1].Green.Mask = (UINT8) ((1 << Private->VbeModeInformationBlock->LinGreenMaskSize) - 1);

    } else {
      ModeBuffer[Private->MaxMode - 1].BytesPerScanLine = Private->VbeModeInformationBlock->BytesPerScanLine;
      ModeBuffer[Private->MaxMode - 1].Red.Position = Private->VbeModeInformationBlock->RedFieldPosition;
      ModeBuffer[Private->MaxMode - 1].Red.Mask = (UINT8) ((1 << Private->VbeModeInformationBlock->RedMaskSize) - 1);
      ModeBuffer[Private->MaxMode - 1].Blue.Position = Private->VbeModeInformationBlock->BlueFieldPosition;
      ModeBuffer[Private->MaxMode - 1].Blue.Mask = (UINT8) ((1 << Private->VbeModeInformationBlock->BlueMaskSize) - 1);
      ModeBuffer[Private->MaxMode - 1].Green.Position = Private->VbeModeInformationBlock->GreenFieldPosition;
      ModeBuffer[Private->MaxMode - 1].Green.Mask = (UINT8) ((1 << Private->VbeModeInformationBlock->GreenMaskSize) - 1);

    }

    ModeBuffer[Private->MaxMode - 1].LinearFrameBuffer = (VOID *)(UINTN)Private->VbeModeInformationBlock->PhysBasePtr;
    ModeBuffer[Private->MaxMode - 1].HorizontalResolution = Private->VbeModeInformationBlock->XResolution;
    ModeBuffer[Private->MaxMode - 1].VerticalResolution = Private->VbeModeInformationBlock->YResolution;

    if (Private->VbeModeInformationBlock->BitsPerPixel >= 24) {
      ModeBuffer[Private->MaxMode - 1].ColorDepth = 32;
    } else {
      ModeBuffer[Private->MaxMode - 1].ColorDepth = Private->VbeModeInformationBlock->BitsPerPixel;
    }

    ModeBuffer[Private->MaxMode - 1].BitsPerPixel  = Private->VbeModeInformationBlock->BitsPerPixel;

    ModeBuffer[Private->MaxMode - 1].RefreshRate   = 60;

    Private->ModeData = ModeBuffer;
  }
  //
  // Check to see if we found any modes that are compatible with UGA DRAW
  //
  if (Private->MaxMode == 0) {
    Status = EFI_DEVICE_ERROR;
    goto Done;
  }
  //
  // Find the best mode to initialize
  //
  Status = BiosVideoUgaDrawSetMode (&Private->UgaDraw, 800, 600, 32, 60);
  if (EFI_ERROR (Status)) {
    Status = BiosVideoUgaDrawSetMode (&Private->UgaDraw, 1024, 768, 32, 60);
    if (EFI_ERROR (Status)) {
      Status = BiosVideoUgaDrawSetMode (&Private->UgaDraw, 640, 480, 32, 60);
      for (Index = 0; EFI_ERROR (Status) && Index < Private->MaxMode; Index++) {
        Status = BiosVideoUgaDrawSetMode (
                  &Private->UgaDraw,
                  Private->ModeData[Index].HorizontalResolution,
                  Private->ModeData[Index].VerticalResolution,
                  Private->ModeData[Index].ColorDepth,
                  Private->ModeData[Index].RefreshRate
                  );
      }
    }
  }

Done:
  //
  // If there was an error, then free the mode structure
  //
  if (EFI_ERROR (Status)) {
    if (Private->ModeData != NULL) {
      gBS->FreePool (Private->ModeData);
      Private->ModeData  = NULL;
      Private->MaxMode   = 0;
    }
  }

  return Status;
}

EFI_STATUS
BiosVideoCheckForVga (
  BIOS_VIDEO_DEV  *Private  // TODO: add IN/OUT modifier to Private
  )
/*++
  
  Routine Description:

    BiosVideoCheckForVbe
  
  Arguments:
  
  Returns:
  
--*/
// TODO:    Private - add argument and description to function comment
{
  EFI_STATUS            Status;
  BIOS_VIDEO_MODE_DATA  *ModeBuffer;

  //
  // Fill in the UGA Draw Protocol
  //
  Private->UgaDraw.GetMode = BiosVideoUgaDrawGetMode;
  Private->UgaDraw.SetMode = BiosVideoUgaDrawSetMode;
  Private->UgaDraw.Blt     = BiosVideoUgaDrawVgaBlt;

  //
  // Add mode to the list of available modes
  //
  Private->MaxMode++;
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  Private->MaxMode * sizeof (BIOS_VIDEO_MODE_DATA),
                  (VOID **) &ModeBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Private->MaxMode > 1) {
    gBS->CopyMem (
          ModeBuffer,
          Private->ModeData,
          (Private->MaxMode - 1) * sizeof (BIOS_VIDEO_MODE_DATA)
          );
  }

  if (Private->ModeData != NULL) {
    gBS->FreePool (Private->ModeData);
  }

  ModeBuffer[Private->MaxMode - 1].VbeModeNumber         = 0x0012;
  ModeBuffer[Private->MaxMode - 1].BytesPerScanLine      = 640;
  ModeBuffer[Private->MaxMode - 1].LinearFrameBuffer     = (VOID *)(UINTN)(0xa0000);
  ModeBuffer[Private->MaxMode - 1].HorizontalResolution  = 640;
  ModeBuffer[Private->MaxMode - 1].VerticalResolution    = 480;
  ModeBuffer[Private->MaxMode - 1].ColorDepth            = 32;
  ModeBuffer[Private->MaxMode - 1].RefreshRate           = 60;

  Private->ModeData = ModeBuffer;

  //
  // Test to see if the Video Adapter support the 640x480 16 color mode
  //
  Status = BiosVideoUgaDrawSetMode (&Private->UgaDraw, 640, 480, 32, 60);

  //
  // If there was an error, then free the mode structure
  //
  if (EFI_ERROR (Status)) {
    Private->MaxMode = 0;
    if (Private->ModeData != NULL) {
      gBS->FreePool (Private->ModeData);
    }
  }

  return Status;
}

//
// UGA Protocol Member Functions for VESA BIOS Extensions
//
EFI_STATUS
EFIAPI
BiosVideoUgaDrawGetMode (
  IN  EFI_UGA_DRAW_PROTOCOL  *This,
  OUT UINT32                 *HorizontalResolution,
  OUT UINT32                 *VerticalResolution,
  OUT UINT32                 *ColorDepth,
  OUT UINT32                 *RefreshRate
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                  - TODO: add argument description
  HorizontalResolution  - TODO: add argument description
  VerticalResolution    - TODO: add argument description
  ColorDepth            - TODO: add argument description
  RefreshRate           - TODO: add argument description

Returns:

  EFI_DEVICE_ERROR - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  BIOS_VIDEO_DEV  *Private;

  Private = BIOS_VIDEO_DEV_FROM_UGA_DRAW_THIS (This);

  if (Private->HardwareNeedsStarting) {
    return EFI_DEVICE_ERROR;
  }

  if (HorizontalResolution == NULL || VerticalResolution == NULL || ColorDepth == NULL || RefreshRate == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *HorizontalResolution = Private->ModeData[Private->CurrentMode].HorizontalResolution;
  *VerticalResolution   = Private->ModeData[Private->CurrentMode].VerticalResolution;
  *ColorDepth           = Private->ModeData[Private->CurrentMode].ColorDepth;
  *RefreshRate          = Private->ModeData[Private->CurrentMode].RefreshRate;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BiosVideoUgaDrawSetMode (
  IN  EFI_UGA_DRAW_PROTOCOL  *This,
  IN  UINT32                 HorizontalResolution,
  IN  UINT32                 VerticalResolution,
  IN  UINT32                 ColorDepth,
  IN  UINT32                 RefreshRate
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                  - TODO: add argument description
  HorizontalResolution  - TODO: add argument description
  VerticalResolution    - TODO: add argument description
  ColorDepth            - TODO: add argument description
  RefreshRate           - TODO: add argument description

Returns:

  EFI_DEVICE_ERROR - TODO: Add description for return value
  EFI_SUCCESS - TODO: Add description for return value
  EFI_UNSUPPORTED - TODO: Add description for return value

--*/
{
  EFI_STATUS            Status;
  BIOS_VIDEO_DEV        *Private;
  UINTN                 Index;
  EFI_IA32_REGISTER_SET Regs;

  Private = BIOS_VIDEO_DEV_FROM_UGA_DRAW_THIS (This);

  for (Index = 0; Index < Private->MaxMode; Index++) {

    if (HorizontalResolution != Private->ModeData[Index].HorizontalResolution) {
      continue;
    }

    if (VerticalResolution != Private->ModeData[Index].VerticalResolution) {
      continue;
    }

    if (ColorDepth != Private->ModeData[Index].ColorDepth) {
      continue;
    }

    if (RefreshRate != Private->ModeData[Index].RefreshRate) {
      continue;
    }

    if (Private->LineBuffer) {
      gBS->FreePool (Private->LineBuffer);
    }

    if (Private->VgaFrameBuffer) {
      gBS->FreePool (Private->VgaFrameBuffer);
    }

    if (Private->VbeFrameBuffer) {
      gBS->FreePool (Private->VbeFrameBuffer);
    }

    Private->LineBuffer = NULL;
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    Private->ModeData[Index].BytesPerScanLine,
                    &Private->LineBuffer
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // Clear all registers
    //
    gBS->SetMem (&Regs, sizeof (Regs), 0);

    if (Private->ModeData[Index].VbeModeNumber < 0x100) {
      //
      // Allocate a working buffer for BLT operations to the VGA frame buffer
      //
      Private->VgaFrameBuffer = NULL;
      Status = gBS->AllocatePool (
                      EfiBootServicesData,
                      4 * 480 * 80,
                      &Private->VgaFrameBuffer
                      );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      //
      // Set VGA Mode
      //
      Regs.X.AX = Private->ModeData[Index].VbeModeNumber;
      Private->LegacyBiosThunk->Int86 (Private->LegacyBiosThunk, 0x10, &Regs);

    } else {
      //
      // Allocate a working buffer for BLT operations to the VBE frame buffer
      //
      Private->VbeFrameBuffer = NULL;
      Status = gBS->AllocatePool (
                      EfiBootServicesData,
                      Private->ModeData[Index].BytesPerScanLine * Private->ModeData[Index].VerticalResolution,
                      &Private->VbeFrameBuffer
                      );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      //
      // Set VBE mode
      //
      Regs.X.AX = VESA_BIOS_EXTENSIONS_SET_MODE;
      Regs.X.BX = (UINT16) (Private->ModeData[Index].VbeModeNumber | VESA_BIOS_EXTENSIONS_MODE_NUMBER_LINEAR_FRAME_BUFFER);
      gBS->SetMem (Private->VbeCrtcInformationBlock, sizeof (VESA_BIOS_EXTENSIONS_CRTC_INFORMATION_BLOCK), 0);
      Regs.X.ES = EFI_SEGMENT (Private->VbeCrtcInformationBlock);
      Regs.X.DI = EFI_OFFSET (Private->VbeCrtcInformationBlock);
      Private->LegacyBiosThunk->Int86 (Private->LegacyBiosThunk, 0x10, &Regs);

      //
      // Check to see if the call succeeded
      //
      if (Regs.X.AX != VESA_BIOS_EXTENSIONS_STATUS_SUCCESS) {
        return EFI_DEVICE_ERROR;
      }
      //
      // Initialize the state of the VbeFrameBuffer
      //
      Status = Private->PciIo->Mem.Read (
                                    Private->PciIo,
                                    EfiPciIoWidthUint32,
                                    EFI_PCI_IO_PASS_THROUGH_BAR,
                                    (UINT64) (UINTN) Private->ModeData[Index].LinearFrameBuffer,
                                    (Private->ModeData[Index].BytesPerScanLine * Private->ModeData[Index].VerticalResolution) >> 2,
                                    Private->VbeFrameBuffer
                                    );
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }

    Private->CurrentMode           = Index;

    Private->HardwareNeedsStarting = FALSE;

    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}
//
// BUGBUG : Add Blt for 16 bit color, 15 bit color, and 8 bit color modes
//
EFI_STATUS
EFIAPI
BiosVideoUgaDrawVbeBlt (
  IN  EFI_UGA_DRAW_PROTOCOL  *This,
  IN  EFI_UGA_PIXEL          *BltBuffer, OPTIONAL
  IN  EFI_UGA_BLT_OPERATION  BltOperation,
  IN  UINTN                  SourceX,
  IN  UINTN                  SourceY,
  IN  UINTN                  DestinationX,
  IN  UINTN                  DestinationY,
  IN  UINTN                  Width,
  IN  UINTN                  Height,
  IN  UINTN                  Delta
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This          - TODO: add argument description
  BltBuffer     - TODO: add argument description
  BltOperation  - TODO: add argument description
  SourceX       - TODO: add argument description
  SourceY       - TODO: add argument description
  DestinationX  - TODO: add argument description
  DestinationY  - TODO: add argument description
  Width         - TODO: add argument description
  Height        - TODO: add argument description
  Delta         - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  BIOS_VIDEO_DEV        *Private;
  BIOS_VIDEO_MODE_DATA  *Mode;
  EFI_PCI_IO_PROTOCOL   *PciIo;
  EFI_TPL               OriginalTPL;
  UINTN                 DstY;
  UINTN                 SrcY;
  UINTN                 DstX;
  EFI_UGA_PIXEL         *Blt;
  VOID                  *MemAddress;
  EFI_UGA_PIXEL         *VbeFrameBuffer;
  UINTN                 BytesPerScanLine;
  UINTN                 Index;
  UINT8                 *VbeBuffer;
  UINT8                 *VbeBuffer1;
  UINT8                 *BltUint8;
  UINT32                VbePixelWidth;
  UINT32                Pixel;

  Private  = BIOS_VIDEO_DEV_FROM_UGA_DRAW_THIS (This);
  Mode              = &Private->ModeData[Private->CurrentMode];
  PciIo             = Private->PciIo;

  VbeFrameBuffer    = Private->VbeFrameBuffer;
  MemAddress        = Mode->LinearFrameBuffer;
  BytesPerScanLine  = Mode->BytesPerScanLine;
  VbePixelWidth     = Mode->BitsPerPixel / 8;
  BltUint8          = (UINT8 *) BltBuffer;

  if ((BltOperation < 0) || (BltOperation >= EfiUgaBltMax)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 0 || Height == 0) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // We need to fill the Virtual Screen buffer with the blt data.
  // The virtual screen is upside down, as the first row is the bootom row of
  // the image.
  //
  if (BltOperation == EfiUgaVideoToBltBuffer) {
    //
    // Video to BltBuffer: Source is Video, destination is BltBuffer
    //
    if (SourceY + Height > Mode->VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (SourceX + Width > Mode->HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // BltBuffer to Video: Source is BltBuffer, destination is Video
    //
    if (DestinationY + Height > Mode->VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (DestinationX + Width > Mode->HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  }
  //
  // If Delta is zero, then the entire BltBuffer is being used, so Delta
  // is the number of bytes in each row of BltBuffer.  Since BltBuffer is Width pixels size,
  // the number of bytes in each row can be computed.
  //
  if (Delta == 0) {
    Delta = Width * sizeof (EFI_UGA_PIXEL);
  }
  //
  // We have to raise to TPL Notify, so we make an atomic write the frame buffer.
  // We would not want a timer based event (Cursor, ...) to come in while we are
  // doing this operation.
  //
  OriginalTPL = gBS->RaiseTPL (EFI_TPL_NOTIFY);

  switch (BltOperation) {
  case EfiUgaVideoToBltBuffer:
    for (SrcY = SourceY, DstY = DestinationY; DstY < (Height + DestinationY); SrcY++, DstY++) {
      Blt = (EFI_UGA_PIXEL *) (BltUint8 + DstY * Delta + DestinationX * sizeof (EFI_UGA_PIXEL));
      //
      // Shuffle the packed bytes in the hardware buffer to match EFI_UGA_PIXEL
      //
      VbeBuffer = ((UINT8 *) VbeFrameBuffer + (SrcY * BytesPerScanLine + SourceX * VbePixelWidth));
      for (DstX = DestinationX; DstX < (Width + DestinationX); DstX++) {
        Pixel         = *(UINT32 *) (VbeBuffer);
        Blt->Red      = (UINT8) ((Pixel >> Mode->Red.Position) & Mode->Red.Mask);
        Blt->Blue     = (UINT8) ((Pixel >> Mode->Blue.Position) & Mode->Blue.Mask);
        Blt->Green    = (UINT8) ((Pixel >> Mode->Green.Position) & Mode->Green.Mask);
        Blt->Reserved = 0;
        Blt++;
        VbeBuffer += VbePixelWidth;
      }

    }
    break;

  case EfiUgaVideoToVideo:
    for (Index = 0; Index < Height; Index++) {
      if (DestinationY <= SourceY) {
        SrcY  = SourceY + Index;
        DstY  = DestinationY + Index;
      } else {
        SrcY  = SourceY + Height - Index - 1;
        DstY  = DestinationY + Height - Index - 1;
      }

      VbeBuffer   = ((UINT8 *) VbeFrameBuffer + DstY * BytesPerScanLine + DestinationX * VbePixelWidth);
      VbeBuffer1  = ((UINT8 *) VbeFrameBuffer + SrcY * BytesPerScanLine + SourceX * VbePixelWidth);

      gBS->CopyMem (
            VbeBuffer,
            VbeBuffer1,
            Width * VbePixelWidth
            );

      if (VbePixelWidth == 4) {
        PciIo->Mem.Write (
                    PciIo,
                    EfiPciIoWidthUint32,
                    EFI_PCI_IO_PASS_THROUGH_BAR,
                    (UINT64) ((UINTN) MemAddress + (DstY * BytesPerScanLine) + DestinationX * VbePixelWidth),
                    Width,
                    VbeBuffer
                    );
      } else {
        PciIo->Mem.Write (
                    PciIo,
                    EfiPciIoWidthUint8,
                    EFI_PCI_IO_PASS_THROUGH_BAR,
                    (UINT64) ((UINTN) MemAddress + (DstY * BytesPerScanLine) + DestinationX * VbePixelWidth),
                    Width * VbePixelWidth,
                    VbeBuffer
                    );
      }
    }
    break;

  case EfiUgaVideoFill:
    VbeBuffer = (UINT8 *) ((UINTN) VbeFrameBuffer + (DestinationY * BytesPerScanLine) + DestinationX * VbePixelWidth);
    Blt       = (EFI_UGA_PIXEL *) BltUint8;
    //
    // Shuffle the RGB fields in EFI_UGA_PIXEL to match the hardware buffer
    //
    Pixel = ((Blt->Red & Mode->Red.Mask) << Mode->Red.Position) |
      (
        (Blt->Green & Mode->Green.Mask) <<
        Mode->Green.Position
      ) |
          ((Blt->Blue & Mode->Blue.Mask) << Mode->Blue.Position);

    for (Index = 0; Index < Width; Index++) {
      gBS->CopyMem (
            VbeBuffer,
            &Pixel,
            VbePixelWidth
            );
      VbeBuffer += VbePixelWidth;
    }

    VbeBuffer = (UINT8 *) ((UINTN) VbeFrameBuffer + (DestinationY * BytesPerScanLine) + DestinationX * VbePixelWidth);
    for (DstY = DestinationY + 1; DstY < (Height + DestinationY); DstY++) {
      gBS->CopyMem (
            (VOID *) ((UINTN) VbeFrameBuffer + (DstY * BytesPerScanLine) + DestinationX * VbePixelWidth),
            VbeBuffer,
            Width * VbePixelWidth
            );
    }

    for (DstY = DestinationY; DstY < (Height + DestinationY); DstY++) {
      PciIo->Mem.Write (
                  PciIo,
                  EfiPciIoWidthUint8,
                  EFI_PCI_IO_PASS_THROUGH_BAR,
                  (UINT64) ((UINTN) MemAddress + (DstY * BytesPerScanLine) + DestinationX * VbePixelWidth),
                  Width * VbePixelWidth,
                  VbeBuffer
                  );
    }
    break;

  case EfiUgaBltBufferToVideo:
    for (SrcY = SourceY, DstY = DestinationY; SrcY < (Height + SourceY); SrcY++, DstY++) {
      Blt       = (EFI_UGA_PIXEL *) (BltUint8 + (SrcY * Delta) + (SourceX) * sizeof (EFI_UGA_PIXEL));
      VbeBuffer = ((UINT8 *) VbeFrameBuffer + (DstY * BytesPerScanLine + DestinationX * VbePixelWidth));
      for (DstX = DestinationX; DstX < (Width + DestinationX); DstX++) {
        //
        // Shuffle the RGB fields in EFI_UGA_PIXEL to match the hardware buffer
        //
        Pixel = ((Blt->Red & Mode->Red.Mask) << Mode->Red.Position) |
          ((Blt->Green & Mode->Green.Mask) << Mode->Green.Position) |
            ((Blt->Blue & Mode->Blue.Mask) << Mode->Blue.Position);
        gBS->CopyMem (
              VbeBuffer,
              &Pixel,
              VbePixelWidth
              );
        Blt++;
        VbeBuffer += VbePixelWidth;
      }

      VbeBuffer = ((UINT8 *) VbeFrameBuffer + (DstY * BytesPerScanLine + DestinationX * VbePixelWidth));
      PciIo->Mem.Write (
                  PciIo,
                  EfiPciIoWidthUint8,
                  EFI_PCI_IO_PASS_THROUGH_BAR,
                  (UINT64) ((UINTN) MemAddress + (DstY * BytesPerScanLine) + DestinationX * VbePixelWidth),
                  Width * VbePixelWidth,
                  VbeBuffer
                  );
    }
    break;
  }

  gBS->RestoreTPL (OriginalTPL);

  return EFI_SUCCESS;
}

STATIC
VOID
WriteGraphicsController (
  IN  EFI_PCI_IO_PROTOCOL  *PciIo,
  IN  UINTN                Address,
  IN  UINTN                Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo   - TODO: add argument description
  Address - TODO: add argument description
  Data    - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  Address = Address | (Data << 8);
  PciIo->Io.Write (
              PciIo,
              EfiPciIoWidthUint16,
              EFI_PCI_IO_PASS_THROUGH_BAR,
              VGA_GRAPHICS_CONTROLLER_ADDRESS_REGISTER,
              1,
              &Address
              );
}

VOID
VgaReadBitPlanes (
  EFI_PCI_IO_PROTOCOL  *PciIo,
  UINT8                *HardwareBuffer,
  UINT8                *MemoryBuffer,
  UINTN                WidthInBytes,
  UINTN                Height
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIo           - TODO: add argument description
  HardwareBuffer  - TODO: add argument description
  MemoryBuffer    - TODO: add argument description
  WidthInBytes    - TODO: add argument description
  Height          - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UINTN BitPlane;
  UINTN Rows;
  UINTN FrameBufferOffset;
  UINT8 *Source;
  UINT8 *Destination;

  //
  // Program the Mode Register Write mode 0, Read mode 0
  //
  WriteGraphicsController (
    PciIo,
    VGA_GRAPHICS_CONTROLLER_MODE_REGISTER,
    VGA_GRAPHICS_CONTROLLER_READ_MODE_0 | VGA_GRAPHICS_CONTROLLER_WRITE_MODE_0
    );

  for (BitPlane = 0, FrameBufferOffset = 0;
       BitPlane < VGA_NUMBER_OF_BIT_PLANES;
       BitPlane++, FrameBufferOffset += VGA_BYTES_PER_BIT_PLANE
      ) {
    //
    // Program the Read Map Select Register to select the correct bit plane
    //
    WriteGraphicsController (
      PciIo,
      VGA_GRAPHICS_CONTROLLER_READ_MAP_SELECT_REGISTER,
      BitPlane
      );

    Source      = HardwareBuffer;
    Destination = MemoryBuffer + FrameBufferOffset;

    for (Rows = 0; Rows < Height; Rows++, Source += VGA_BYTES_PER_SCAN_LINE, Destination += VGA_BYTES_PER_SCAN_LINE) {
      PciIo->Mem.Read (
                  PciIo,
                  EfiPciIoWidthUint8,
                  EFI_PCI_IO_PASS_THROUGH_BAR,
                  (UINT64) Source,
                  WidthInBytes,
                  (VOID *) Destination
                  );
    }
  }
}

VOID
VgaConvertToUgaColor (
  UINT8          *MemoryBuffer,
  UINTN          X,
  UINTN          Y,
  EFI_UGA_PIXEL  *BltBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  MemoryBuffer  - TODO: add argument description
  X             - TODO: add argument description
  Y             - TODO: add argument description
  BltBuffer     - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UINTN Mask;
  UINTN Bit;
  UINTN Color;

  MemoryBuffer += ((Y << 6) + (Y << 4) + (X >> 3));
  Mask = mVgaBitMaskTable[X & 0x07];
  for (Bit = 0x01, Color = 0; Bit < 0x10; Bit <<= 1, MemoryBuffer += VGA_BYTES_PER_BIT_PLANE) {
    if (*MemoryBuffer & Mask) {
      Color |= Bit;
    }
  }

  *BltBuffer = mVgaColorToUgaColor[Color];
}

UINT8
VgaConvertColor (
  IN  EFI_UGA_PIXEL          *BltBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  BltBuffer - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UINT8 Color;

  Color = (UINT8) ((BltBuffer->Blue >> 7) | ((BltBuffer->Green >> 6) & 0x02) | ((BltBuffer->Red >> 5) & 0x04));
  if ((BltBuffer->Red + BltBuffer->Green + BltBuffer->Blue) > 0x180) {
    Color |= 0x08;
  }

  return Color;
}

EFI_STATUS
EFIAPI
BiosVideoUgaDrawVgaBlt (
  IN  EFI_UGA_DRAW_PROTOCOL  *This,
  IN  EFI_UGA_PIXEL          *BltBuffer, OPTIONAL
  IN  EFI_UGA_BLT_OPERATION  BltOperation,
  IN  UINTN                  SourceX,
  IN  UINTN                  SourceY,
  IN  UINTN                  DestinationX,
  IN  UINTN                  DestinationY,
  IN  UINTN                  Width,
  IN  UINTN                  Height,
  IN  UINTN                  Delta
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This          - TODO: add argument description
  BltBuffer     - TODO: add argument description
  BltOperation  - TODO: add argument description
  SourceX       - TODO: add argument description
  SourceY       - TODO: add argument description
  DestinationX  - TODO: add argument description
  DestinationY  - TODO: add argument description
  Width         - TODO: add argument description
  Height        - TODO: add argument description
  Delta         - TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_INVALID_PARAMETER - TODO: Add description for return value
  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  BIOS_VIDEO_DEV      *Private;
  EFI_TPL             OriginalTPL;
  UINT8               *MemAddress;
  UINTN               BytesPerScanLine;
  UINTN               BytesPerBitPlane;
  UINTN               Bit;
  UINTN               Index;
  UINTN               Index1;
  UINTN               StartAddress;
  UINTN               Bytes;
  UINTN               Offset;
  UINT8               LeftMask;
  UINT8               RightMask;
  UINTN               Address;
  UINT8               *Address1;
  UINT8               *SourceAddress;
  UINT8               *DestinationAddress;
  EFI_PCI_IO_PROTOCOL *PciIo;
  UINT8               Data;
  UINT8               PixelColor;
  UINT8               *VgaFrameBuffer;
  UINTN               SourceOffset;
  UINTN               SourceWidth;
  UINTN               Rows;
  UINTN               Columns;
  UINTN               X;
  UINTN               Y;

  Private  = BIOS_VIDEO_DEV_FROM_UGA_DRAW_THIS (This);

  PciIo             = Private->PciIo;
  MemAddress        = Private->ModeData[Private->CurrentMode].LinearFrameBuffer;
  BytesPerScanLine  = Private->ModeData[Private->CurrentMode].BytesPerScanLine >> 3;
  BytesPerBitPlane  = BytesPerScanLine * Private->ModeData[Private->CurrentMode].VerticalResolution;
  VgaFrameBuffer    = Private->VgaFrameBuffer;

  if (BltOperation >= EfiUgaBltMax) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 0 || Height == 0) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // We need to fill the Virtual Screen buffer with the blt data.
  // The virtual screen is upside down, as the first row is the bootom row of
  // the image.
  //
  if (BltOperation == EfiUgaVideoToBltBuffer) {
    //
    // Video to BltBuffer: Source is Video, destination is BltBuffer
    //
    if (SourceY + Height > Private->ModeData[Private->CurrentMode].VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (SourceX + Width > Private->ModeData[Private->CurrentMode].HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // BltBuffer to Video: Source is BltBuffer, destination is Video
    //
    if (DestinationY + Height > Private->ModeData[Private->CurrentMode].VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (DestinationX + Width > Private->ModeData[Private->CurrentMode].HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  }
  //
  // If Delta is zero, then the entire BltBuffer is being used, so Delta
  // is the number of bytes in each row of BltBuffer.  Since BltBuffer is Width pixels size,
  // the number of bytes in each row can be computed.
  //
  if (Delta == 0) {
    Delta = Width * sizeof (EFI_UGA_PIXEL);
  }
  //
  // We have to raise to TPL Notify, so we make an atomic write the frame buffer.
  // We would not want a timer based event (Cursor, ...) to come in while we are
  // doing this operation.
  //
  OriginalTPL = gBS->RaiseTPL (EFI_TPL_NOTIFY);

  //
  // Compute some values we need for VGA
  //
  switch (BltOperation) {
  case EfiUgaVideoToBltBuffer:

    SourceOffset  = (SourceY << 6) + (SourceY << 4) + (SourceX >> 3);
    SourceWidth   = ((SourceX + Width - 1) >> 3) - (SourceX >> 3) + 1;

    //
    // Read all the pixels in the 4 bit planes into a memory buffer that looks like the VGA buffer
    //
    VgaReadBitPlanes (
      PciIo,
      MemAddress + SourceOffset,
      VgaFrameBuffer + SourceOffset,
      SourceWidth,
      Height
      );

    //
    // Convert VGA Bit Planes to a UGA 32-bit color value
    //
    BltBuffer += (DestinationY * (Delta >> 2) + DestinationX);
    for (Rows = 0, Y = SourceY; Rows < Height; Rows++, Y++, BltBuffer += (Delta >> 2)) {
      for (Columns = 0, X = SourceX; Columns < Width; Columns++, X++, BltBuffer++) {
        VgaConvertToUgaColor (VgaFrameBuffer, X, Y, BltBuffer);
      }

      BltBuffer -= Width;
    }

    break;

  case EfiUgaVideoToVideo:
    //
    // Check for an aligned Video to Video operation
    //
    if ((SourceX & 0x07) == 0x00 && (DestinationX & 0x07) == 0x00 && (Width & 0x07) == 0x00) {
      //
      // Program the Mode Register Write mode 1, Read mode 0
      //
      WriteGraphicsController (
        Private->PciIo,
        VGA_GRAPHICS_CONTROLLER_MODE_REGISTER,
        VGA_GRAPHICS_CONTROLLER_READ_MODE_0 | VGA_GRAPHICS_CONTROLLER_WRITE_MODE_1
        );

      SourceAddress       = (UINT8 *) (MemAddress + (SourceY << 6) + (SourceY << 4) + (SourceX >> 3));
      DestinationAddress  = (UINT8 *) (MemAddress + (DestinationY << 6) + (DestinationY << 4) + (DestinationX >> 3));
      Bytes               = Width >> 3;
      for (Index = 0, Offset = 0; Index < Height; Index++, Offset += BytesPerScanLine) {
        PciIo->CopyMem (
                PciIo,
                EfiPciIoWidthUint8,
                EFI_PCI_IO_PASS_THROUGH_BAR,
                (UINT64) (DestinationAddress + Offset),
                EFI_PCI_IO_PASS_THROUGH_BAR,
                (UINT64) (SourceAddress + Offset),
                Bytes
                );
      }
    } else {
      SourceOffset  = (SourceY << 6) + (SourceY << 4) + (SourceX >> 3);
      SourceWidth   = ((SourceX + Width - 1) >> 3) - (SourceX >> 3) + 1;

      //
      // Read all the pixels in the 4 bit planes into a memory buffer that looks like the VGA buffer
      //
      VgaReadBitPlanes (
        PciIo,
        MemAddress + SourceOffset,
        VgaFrameBuffer + SourceOffset,
        SourceWidth,
        Height
        );
    }

    break;

  case EfiUgaVideoFill:
    StartAddress  = (UINTN) (MemAddress + (DestinationY << 6) + (DestinationY << 4) + (DestinationX >> 3));
    Bytes         = ((DestinationX + Width - 1) >> 3) - (DestinationX >> 3);
    LeftMask      = mVgaLeftMaskTable[DestinationX & 0x07];
    RightMask     = mVgaRightMaskTable[(DestinationX + Width - 1) & 0x07];
    if (Bytes == 0) {
      LeftMask &= RightMask;
      RightMask = 0;
    }

    if (LeftMask == 0xff) {
      StartAddress--;
      Bytes++;
      LeftMask = 0;
    }

    if (RightMask == 0xff) {
      Bytes++;
      RightMask = 0;
    }

    PixelColor = VgaConvertColor (BltBuffer);

    //
    // Program the Mode Register Write mode 2, Read mode 0
    //
    WriteGraphicsController (
      Private->PciIo,
      VGA_GRAPHICS_CONTROLLER_MODE_REGISTER,
      VGA_GRAPHICS_CONTROLLER_READ_MODE_0 | VGA_GRAPHICS_CONTROLLER_WRITE_MODE_2
      );

    //
    // Program the Data Rotate/Function Select Register to replace
    //
    WriteGraphicsController (
      Private->PciIo,
      VGA_GRAPHICS_CONTROLLER_DATA_ROTATE_REGISTER,
      VGA_GRAPHICS_CONTROLLER_FUNCTION_REPLACE
      );

    if (LeftMask != 0) {
      //
      // Program the BitMask register with the Left column mask
      //
      WriteGraphicsController (
        Private->PciIo,
        VGA_GRAPHICS_CONTROLLER_BIT_MASK_REGISTER,
        LeftMask
        );

      for (Index = 0, Address = StartAddress; Index < Height; Index++, Address += BytesPerScanLine) {
        //
        // Read data from the bit planes into the latches
        //
        PciIo->Mem.Read (
                    PciIo,
                    EfiPciIoWidthUint8,
                    EFI_PCI_IO_PASS_THROUGH_BAR,
                    (UINT64) Address,
                    1,
                    &Data
                    );
        //
        // Write the lower 4 bits of PixelColor to the bit planes in the pixels enabled by BitMask
        //
        PciIo->Mem.Write (
                    PciIo,
                    EfiPciIoWidthUint8,
                    EFI_PCI_IO_PASS_THROUGH_BAR,
                    (UINT64) Address,
                    1,
                    &PixelColor
                    );
      }
    }

    if (Bytes > 1) {
      //
      // Program the BitMask register with the middle column mask of 0xff
      //
      WriteGraphicsController (
        Private->PciIo,
        VGA_GRAPHICS_CONTROLLER_BIT_MASK_REGISTER,
        0xff
        );

      for (Index = 0, Address = StartAddress + 1; Index < Height; Index++, Address += BytesPerScanLine) {
        PciIo->Mem.Write (
                    PciIo,
                    EfiPciIoWidthFillUint8,
                    EFI_PCI_IO_PASS_THROUGH_BAR,
                    (UINT64) Address,
                    Bytes - 1,
                    &PixelColor
                    );
      }
    }

    if (RightMask != 0) {
      //
      // Program the BitMask register with the Right column mask
      //
      WriteGraphicsController (
        Private->PciIo,
        VGA_GRAPHICS_CONTROLLER_BIT_MASK_REGISTER,
        RightMask
        );

      for (Index = 0, Address = StartAddress + Bytes; Index < Height; Index++, Address += BytesPerScanLine) {
        //
        // Read data from the bit planes into the latches
        //
        PciIo->Mem.Read (
                    PciIo,
                    EfiPciIoWidthUint8,
                    EFI_PCI_IO_PASS_THROUGH_BAR,
                    (UINT64) Address,
                    1,
                    &Data
                    );
        //
        // Write the lower 4 bits of PixelColor to the bit planes in the pixels enabled by BitMask
        //
        PciIo->Mem.Write (
                    PciIo,
                    EfiPciIoWidthUint8,
                    EFI_PCI_IO_PASS_THROUGH_BAR,
                    (UINT64) Address,
                    1,
                    &PixelColor
                    );
      }
    }
    break;

  case EfiUgaBltBufferToVideo:
    StartAddress  = (UINTN) (MemAddress + (DestinationY << 6) + (DestinationY << 4) + (DestinationX >> 3));
    LeftMask      = mVgaBitMaskTable[DestinationX & 0x07];

    //
    // Program the Mode Register Write mode 2, Read mode 0
    //
    WriteGraphicsController (
      Private->PciIo,
      VGA_GRAPHICS_CONTROLLER_MODE_REGISTER,
      VGA_GRAPHICS_CONTROLLER_READ_MODE_0 | VGA_GRAPHICS_CONTROLLER_WRITE_MODE_2
      );

    //
    // Program the Data Rotate/Function Select Register to replace
    //
    WriteGraphicsController (
      Private->PciIo,
      VGA_GRAPHICS_CONTROLLER_DATA_ROTATE_REGISTER,
      VGA_GRAPHICS_CONTROLLER_FUNCTION_REPLACE
      );

    for (Index = 0, Address = StartAddress; Index < Height; Index++, Address += BytesPerScanLine) {
      for (Index1 = 0; Index1 < Width; Index1++) {
        Private->LineBuffer[Index1] = VgaConvertColor (&BltBuffer[(SourceY + Index) * (Delta >> 2) + SourceX + Index1]);
      }

      for (Bit = 0; Bit < 8; Bit++) {
        //
        // Program the BitMask register with the Left column mask
        //
        WriteGraphicsController (
          Private->PciIo,
          VGA_GRAPHICS_CONTROLLER_BIT_MASK_REGISTER,
          LeftMask
          );

        for (Index1 = Bit, Address1 = (UINT8 *) Address; Index1 < Width; Index1 += 8, Address1++) {
          //
          // Read data from the bit planes into the latches
          //
          PciIo->Mem.Read (
                      PciIo,
                      EfiPciIoWidthUint8,
                      EFI_PCI_IO_PASS_THROUGH_BAR,
                      (UINT64) Address1,
                      1,
                      &Data
                      );

          PciIo->Mem.Write (
                      PciIo,
                      EfiPciIoWidthUint8,
                      EFI_PCI_IO_PASS_THROUGH_BAR,
                      (UINT64) Address1,
                      1,
                      &Private->LineBuffer[Index1]
                      );
        }

        LeftMask = (UINT8) (LeftMask >> 1);
        if (LeftMask == 0) {
          LeftMask = 0x80;
        }
      }
    }

    break;
  }

  gBS->RestoreTPL (OriginalTPL);

  return EFI_SUCCESS;
}
