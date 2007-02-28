/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  UefiCirrusLogic5430GraphicsOutput.c

Abstract:

  This file produces the graphics abstration of Graphics Output Protocol. It is called by 
  UefiCirrusLogic5430.c file which deals with the UEFI 2.0 driver model. 
  This file just does graphics.

--*/

#include "UefiCirrusLogic5430.h"

//
// Video Mode structure
//
typedef struct {
  UINT32  Width;
  UINT32  Height;
  UINT32  ColorDepth;
  UINT32  RefreshRate;
  UINT8   *CrtcSettings;
  UINT16  *SeqSettings;
  UINT8   MiscSetting;
} CIRRUS_LOGIC_5430_VIDEO_MODES;

//
// Generic Attribute Controller Register Settings
//
static UINT8                          AttributeController[21] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
  0x41, 0x00, 0x0F, 0x00, 0x00
};

//
// Generic Graphics Controller Register Settings
//
static UINT8 GraphicsController[9] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF
};

//
// 640 x 480 x 256 color @ 60 Hertz
//
static UINT8                          Crtc_640_480_256_60[28] = {
  0x5d, 0x4f, 0x50, 0x82, 0x53, 0x9f, 0x00, 0x3e,
  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xe1, 0x83, 0xdf, 0x50, 0x00, 0xe7, 0x04, 0xe3,
  0xff, 0x00, 0x00, 0x22
};

static UINT16                         Seq_640_480_256_60[15] = {
  0x0100, 0x0101, 0x0f02, 0x0003, 0x0e04, 0x1107, 0x0008, 0x4a0b, 
  0x5b0c, 0x450d, 0x7e0e, 0x2b1b, 0x2f1c, 0x301d, 0x331e
};

//
// 800 x 600 x 256 color @ 60 Hertz
//
static UINT8                          Crtc_800_600_256_60[28] = {
  0x7F, 0x63, 0x64, 0x80, 0x6B, 0x1B, 0x72, 0xF0, 
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x58, 0x8C, 0x57, 0x64, 0x00, 0x5F, 0x91, 0xE3,
  0xFF, 0x00, 0x00, 0x22
};

static UINT16                         Seq_800_600_256_60[15] = {
  0x0100, 0x0101, 0x0f02, 0x0003, 0x0e04, 0x1107, 0x0008, 0x4a0b, 
  0x5b0c, 0x450d, 0x510e, 0x2b1b, 0x2f1c, 0x301d, 0x3a1e
};

//
// 1024 x 768 x 256 color @ 60 Hertz
//
static UINT8                          Crtc_1024_768_256_60[28] = {
  0xA3, 0x7F, 0x80, 0x86, 0x85, 0x96, 0x24, 0xFD, 
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x02, 0x88, 0xFF, 0x80, 0x00, 0x00, 0x24, 0xE3,
  0xFF, 0x4A, 0x00, 0x22
};

static UINT16                         Seq_1024_768_256_60[15] = {
  0x0100, 0x0101, 0x0f02, 0x0003, 0x0e04, 0x1107, 0x0008, 0x4a0b, 
  0x5b0c, 0x450d, 0x760e, 0x2b1b, 0x2f1c, 0x301d, 0x341e
};

//
// Table of supported video modes
//
static CIRRUS_LOGIC_5430_VIDEO_MODES  CirrusLogic5430VideoModes[] = {
  {  640, 480, 8, 60, Crtc_640_480_256_60,  Seq_640_480_256_60,  0xe3 },
  {  800, 600, 8, 60, Crtc_800_600_256_60,  Seq_800_600_256_60,  0xef }, 
  { 1024, 768, 8, 60, Crtc_1024_768_256_60, Seq_1024_768_256_60, 0xef } 
};

//
// Local Function Prototypes
//
VOID
InitializeGraphicsMode (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  CIRRUS_LOGIC_5430_VIDEO_MODES   *ModeData
  );

VOID
SetPaletteColor (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  UINTN                           Index,
  UINT8                           Red,
  UINT8                           Green,
  UINT8                           Blue
  );

VOID
SetDefaultPalette (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private
  );

STATIC
VOID
ClearScreen (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private
  );

VOID
DrawLogo (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private
  );

VOID
outb (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  UINTN                           Address,
  UINT8                           Data
  );

VOID
outw (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  UINTN                           Address,
  UINT16                          Data
  );

UINT8
inb (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  UINTN                           Address
  );

UINT16
inw (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  UINTN                           Address
  );

//
// Graphics Output Protocol Member Functions
//
EFI_STATUS
EFIAPI
CirrusLogic5430GraphicsOutputQueryMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  UINT32                                ModeNumber,
  OUT UINTN                                 *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
  )
/*++

Routine Description:

  Graphics Output protocol interface to query video mode

  Arguments:
    This                  - Protocol instance pointer.
    ModeNumber            - The mode number to return information on.
    Info                  - Caller allocated buffer that returns information about ModeNumber.
    SizeOfInfo            - A pointer to the size, in bytes, of the Info buffer.

  Returns:
    EFI_SUCCESS           - Mode information returned.
    EFI_BUFFER_TOO_SMALL  - The Info buffer was too small.
    EFI_DEVICE_ERROR      - A hardware error occurred trying to retrieve the video mode.
    EFI_NOT_STARTED       - Video display is not initialized. Call SetMode ()
    EFI_INVALID_PARAMETER - One of the input args was NULL.

--*/
{
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private;
  EFI_STATUS       Status;

  Private = CIRRUS_LOGIC_5430_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (This);

  if (Private->HardwareNeedsStarting) {
    return EFI_NOT_STARTED;
  }

  if (Info == NULL || SizeOfInfo == NULL || ModeNumber >= This->Mode->MaxMode) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
                  Info
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

  (*Info)->Version              = 0;
  (*Info)->HorizontalResolution = Private->ModeData[ModeNumber].HorizontalResolution;
  (*Info)->VerticalResolution   = Private->ModeData[ModeNumber].VerticalResolution;
  (*Info)->PixelFormat          = Private->ModeData[ModeNumber].PixelFormat;
  (*Info)->PixelInformation     = Private->ModeData[ModeNumber].PixelInformation;
  (*Info)->PixelsPerScanLine    = Private->ModeData[ModeNumber].PixelsPerScanLine;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CirrusLogic5430GraphicsOutputSetMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL * This,
  IN  UINT32                       ModeNumber
  )
/*++

Routine Description:

  Graphics Output protocol interface to set video mode

  Arguments:
    This             - Protocol instance pointer.
    ModeNumber       - The mode number to be set.

  Returns:
    EFI_SUCCESS      - Graphics mode was changed.
    EFI_DEVICE_ERROR - The device had an error and could not complete the request.
    EFI_UNSUPPORTED  - ModeNumber is not supported by this device.

--*/
{
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private;
  CIRRUS_LOGIC_5430_GRAPHICS_OUTPUT_MODE_DATA    *ModeData;

  Private = CIRRUS_LOGIC_5430_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (This);

  if (ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  if (ModeNumber == This->Mode->Mode) {
    return EFI_SUCCESS;
  }

  ModeData = &Private->ModeData[ModeNumber];

  if (Private->LineBuffer) {
    gBS->FreePool (Private->LineBuffer);
  }

  Private->LineBuffer = NULL;
  Private->LineBuffer = EfiLibAllocatePool (ModeData->HorizontalResolution);
  if (Private->LineBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InitializeGraphicsMode (Private, &CirrusLogic5430VideoModes[ModeNumber]);

  This->Mode->Mode                       = ModeNumber;
  This->Mode->Info->Version              = 0;
  This->Mode->Info->HorizontalResolution = ModeData->HorizontalResolution;
  This->Mode->Info->VerticalResolution   = ModeData->VerticalResolution;
  This->Mode->Info->PixelFormat          = ModeData->PixelFormat;
  This->Mode->Info->PixelInformation     = ModeData->PixelInformation;
  This->Mode->Info->PixelsPerScanLine    = ModeData->PixelsPerScanLine;
  This->Mode->SizeOfInfo                 = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

  This->Mode->FrameBufferBase = ModeData->FrameBufferBase;
  This->Mode->FrameBufferSize = ModeData->FrameBufferSize;

  Private->CurrentMode = ModeNumber;

  Private->HardwareNeedsStarting  = FALSE;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CirrusLogic5430GraphicsOutputBlt (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *BltBuffer, OPTIONAL
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION     BltOperation,
  IN  UINTN                                 SourceX,
  IN  UINTN                                 SourceY,
  IN  UINTN                                 DestinationX,
  IN  UINTN                                 DestinationY,
  IN  UINTN                                 Width,
  IN  UINTN                                 Height,
  IN  UINTN                                 Delta
  )
/*++

Routine Description:

  Graphics Output protocol instance to block transfer for CirrusLogic device

Arguments:

  This          - Pointer to Graphics Output protocol instance
  BltBuffer     - The data to transfer to screen
  BltOperation  - The operation to perform
  SourceX       - The X coordinate of the source for BltOperation
  SourceY       - The Y coordinate of the source for BltOperation
  DestinationX  - The X coordinate of the destination for BltOperation
  DestinationY  - The Y coordinate of the destination for BltOperation
  Width         - The width of a rectangle in the blt rectangle in pixels
  Height        - The height of a rectangle in the blt rectangle in pixels
  Delta         - Not used for EfiBltVideoFill and EfiBltVideoToVideo operation.
                  If a Delta of 0 is used, the entire BltBuffer will be operated on.
                  If a subrectangle of the BltBuffer is used, then Delta represents
                  the number of bytes in a row of the BltBuffer.

Returns:

  EFI_INVALID_PARAMETER - Invalid parameter passed in
  EFI_SUCCESS - Blt operation success

--*/
{
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private;
  EFI_TPL                         OriginalTPL;
  UINTN                           DstY;
  UINTN                           SrcY;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *Blt;
  UINTN                           X;
  UINT8                           Pixel;
  UINT32                          WidePixel;
  UINTN                           ScreenWidth;
  UINTN                           Offset;
  UINTN                           SourceOffset;
  UINT32                          CurrentMode;

  Private = CIRRUS_LOGIC_5430_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (This);

  if ((BltOperation < 0) || (BltOperation >= EfiGraphicsOutputBltOperationMax)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 0 || Height == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If Delta is zero, then the entire BltBuffer is being used, so Delta
  // is the number of bytes in each row of BltBuffer.  Since BltBuffer is Width pixels size,
  // the number of bytes in each row can be computed.
  //
  if (Delta == 0) {
    Delta = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  }

  //
  // We need to fill the Virtual Screen buffer with the blt data.
  // The virtual screen is upside down, as the first row is the bootom row of
  // the image.
  //

  CurrentMode = This->Mode->Mode;
  //
  // Make sure the SourceX, SourceY, DestinationX, DestinationY, Width, and Height parameters
  // are valid for the operation and the current screen geometry.
  //
  if (BltOperation == EfiBltVideoToBltBuffer) {
    //
    // Video to BltBuffer: Source is Video, destination is BltBuffer
    //
    if (SourceY + Height > Private->ModeData[CurrentMode].VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (SourceX + Width > Private->ModeData[CurrentMode].HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // BltBuffer to Video: Source is BltBuffer, destination is Video
    //
    if (DestinationY + Height > Private->ModeData[CurrentMode].VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (DestinationX + Width > Private->ModeData[CurrentMode].HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  }
  //
  // We have to raise to TPL Notify, so we make an atomic write the frame buffer.
  // We would not want a timer based event (Cursor, ...) to come in while we are
  // doing this operation.
  //
  OriginalTPL = gBS->RaiseTPL (EFI_TPL_NOTIFY);

  switch (BltOperation) {
  case EfiBltVideoToBltBuffer:
    //
    // Video to BltBuffer: Source is Video, destination is BltBuffer
    //
    for (SrcY = SourceY, DstY = DestinationY; DstY < (Height + DestinationY); SrcY++, DstY++) {

      Offset = (SrcY * Private->ModeData[CurrentMode].HorizontalResolution) + SourceX;
      if (((Offset & 0x03) == 0) && ((Width & 0x03) == 0)) {
        Private->PciIo->Mem.Read (
                              Private->PciIo,
                              EfiPciIoWidthUint32,
                              0,
                              Offset,
                              Width >> 2,
                              Private->LineBuffer
                              );
      } else {
        Private->PciIo->Mem.Read (
                              Private->PciIo,
                              EfiPciIoWidthUint8,
                              0,
                              Offset,
                              Width,
                              Private->LineBuffer
                              );
      }

      for (X = 0; X < Width; X++) {
        Blt         = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *) ((UINT8 *) BltBuffer + (DstY * Delta) + (DestinationX + X) * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

        Blt->Red    = (UINT8) (Private->LineBuffer[X] & 0xe0);
        Blt->Green  = (UINT8) ((Private->LineBuffer[X] & 0x1c) << 3);
        Blt->Blue   = (UINT8) ((Private->LineBuffer[X] & 0x03) << 6);
      }
    }
    break;

  case EfiBltVideoToVideo:
    //
    // Perform hardware acceleration for Video to Video operations
    //
    ScreenWidth   = Private->ModeData[CurrentMode].HorizontalResolution;
    SourceOffset  = (SourceY * Private->ModeData[CurrentMode].HorizontalResolution) + (SourceX);
    Offset        = (DestinationY * Private->ModeData[CurrentMode].HorizontalResolution) + (DestinationX);

    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0000);
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0010);
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0012);
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0014);

    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0001);
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0011);
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0013);
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0015);

    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) (((Width << 8) & 0xff00) | 0x20));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) ((Width & 0xff00) | 0x21));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) (((Height << 8) & 0xff00) | 0x22));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) ((Height & 0xff00) | 0x23));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) (((ScreenWidth << 8) & 0xff00) | 0x24));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) ((ScreenWidth & 0xff00) | 0x25));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) (((ScreenWidth << 8) & 0xff00) | 0x26));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) ((ScreenWidth & 0xff00) | 0x27));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) ((((Offset) << 8) & 0xff00) | 0x28));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) ((((Offset) >> 0) & 0xff00) | 0x29));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) ((((Offset) >> 8) & 0xff00) | 0x2a));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) ((((SourceOffset) << 8) & 0xff00) | 0x2c));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) ((((SourceOffset) >> 0) & 0xff00) | 0x2d));
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) ((((SourceOffset) >> 8) & 0xff00) | 0x2e));
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x002f);
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0030);
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0d32);
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0033);
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0034);
    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0035);

    outw (Private, GRAPH_ADDRESS_REGISTER, 0x0231);

    outb (Private, GRAPH_ADDRESS_REGISTER, 0x31);
    while ((inb (Private, GRAPH_DATA_REGISTER) & 0x01) == 0x01)
      ;
    break;

  case EfiBltVideoFill:
    Blt       = BltBuffer;
    Pixel     = (UINT8) ((Blt->Red & 0xe0) | ((Blt->Green >> 3) & 0x1c) | ((Blt->Blue >> 6) & 0x03));
    WidePixel = (Pixel << 8) | Pixel;
    WidePixel = (WidePixel << 16) | WidePixel;

    if (DestinationX == 0 && Width == Private->ModeData[CurrentMode].HorizontalResolution) {
      Offset = DestinationY * Private->ModeData[CurrentMode].HorizontalResolution;
      if (((Offset & 0x03) == 0) && (((Width * Height) & 0x03) == 0)) {
        Private->PciIo->Mem.Write (
                              Private->PciIo,
                              EfiPciIoWidthFillUint32,
                              0,
                              Offset,
                              (Width * Height) >> 2,
                              &WidePixel
                              );
      } else {
        Private->PciIo->Mem.Write (
                              Private->PciIo,
                              EfiPciIoWidthFillUint8,
                              0,
                              Offset,
                              Width * Height,
                              &Pixel
                              );
      }
    } else {
      for (SrcY = SourceY, DstY = DestinationY; SrcY < (Height + SourceY); SrcY++, DstY++) {
        Offset = (DstY * Private->ModeData[CurrentMode].HorizontalResolution) + DestinationX;
        if (((Offset & 0x03) == 0) && ((Width & 0x03) == 0)) {
          Private->PciIo->Mem.Write (
                                Private->PciIo,
                                EfiPciIoWidthFillUint32,
                                0,
                                Offset,
                                Width >> 2,
                                &WidePixel
                                );
        } else {
          Private->PciIo->Mem.Write (
                                Private->PciIo,
                                EfiPciIoWidthFillUint8,
                                0,
                                Offset,
                                Width,
                                &Pixel
                                );
        }
      }
    }
    break;

  case EfiBltBufferToVideo:
    for (SrcY = SourceY, DstY = DestinationY; SrcY < (Height + SourceY); SrcY++, DstY++) {

      for (X = 0; X < Width; X++) {
        Blt                     = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *) ((UINT8 *) BltBuffer + (SrcY * Delta) + (SourceX + X) * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
        Private->LineBuffer[X]  = (UINT8) ((Blt->Red & 0xe0) | ((Blt->Green >> 3) & 0x1c) | ((Blt->Blue >> 6) & 0x03));
      }

      Offset = (DstY * Private->ModeData[CurrentMode].HorizontalResolution) + DestinationX;

      if (((Offset & 0x03) == 0) && ((Width & 0x03) == 0)) {
        Private->PciIo->Mem.Write (
                              Private->PciIo,
                              EfiPciIoWidthUint32,
                              0,
                              Offset,
                              Width >> 2,
                              Private->LineBuffer
                              );
      } else {
        Private->PciIo->Mem.Write (
                              Private->PciIo,
                              EfiPciIoWidthUint8,
                              0,
                              Offset,
                              Width,
                              Private->LineBuffer
                              );
      }
    }
    break;
  }

  gBS->RestoreTPL (OriginalTPL);

  return EFI_SUCCESS;
}

//
// Construction and Destruction functions
//

EFI_STATUS
CirrusLogic5430GraphicsOutputConstructor (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    Private - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS                   Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;
  UINTN                        Index;
  PCI_TYPE00                   Pci;

  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Status = Private->PciIo->Pci.Read (Private->PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Fill in Private->GraphicsOutput protocol
  //
  GraphicsOutput            = &Private->GraphicsOutput;

  GraphicsOutput->QueryMode = CirrusLogic5430GraphicsOutputQueryMode;
  GraphicsOutput->SetMode   = CirrusLogic5430GraphicsOutputSetMode;
  GraphicsOutput->Blt       = CirrusLogic5430GraphicsOutputBlt;

  //
  // setup EDID information
  // Leave it empty now.
  //
  Private->EdidDiscovered.Edid       = NULL;
  Private->EdidDiscovered.SizeOfEdid = 0;
  Private->EdidActive.Edid           = NULL;
  Private->EdidActive.SizeOfEdid     = 0;

  //
  // Initialize the private data
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE),
                  (VOID **) &Private->GraphicsOutput.Mode
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
                  (VOID **) &Private->GraphicsOutput.Mode->Info
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (Private->GraphicsOutput.Mode);
    Private->GraphicsOutput.Mode = NULL;
    return Status;
  }

  Private->GraphicsOutput.Mode->MaxMode = CIRRUS_LOGIC_5430_GRAPHICS_OUTPUT_MODE_COUNT;
  Private->GraphicsOutput.Mode->Mode = GRAPHICS_OUTPUT_INVALIDE_MODE_NUMBER;
  for (Index = 0; Index < Private->GraphicsOutput.Mode->MaxMode; Index++) {
    Private->ModeData[Index].HorizontalResolution          = CirrusLogic5430VideoModes[Index].Width;
    Private->ModeData[Index].VerticalResolution            = CirrusLogic5430VideoModes[Index].Height;
    Private->ModeData[Index].ColorDepth                    = CirrusLogic5430VideoModes[Index].ColorDepth;
    Private->ModeData[Index].RefreshRate                   = CirrusLogic5430VideoModes[Index].RefreshRate;
    Private->ModeData[Index].PixelFormat                   = PixelBitMask;
    Private->ModeData[Index].PixelInformation.RedMask      = CIRRUS_LOGIC_5430_RED_MASK;
    Private->ModeData[Index].PixelInformation.GreenMask    = CIRRUS_LOGIC_5430_GREEN_MASK;
    Private->ModeData[Index].PixelInformation.BlueMask     = CIRRUS_LOGIC_5430_BLUE_MASK;
    Private->ModeData[Index].PixelInformation.ReservedMask = CIRRUS_LOGIC_5430_RESERVED_MASK;
    Private->ModeData[Index].PixelsPerScanLine             = Private->ModeData[Index].HorizontalResolution;
    Private->ModeData[Index].FrameBufferBase               = (EFI_PHYSICAL_ADDRESS) (UINTN) (Pci.Device.Bar[0] & ~0xF);
    Private->ModeData[Index].FrameBufferSize               = CIRRUS_LOGIC_5430_FRAMEBUFFER_LENGTH;
  }

  Private->HardwareNeedsStarting  = TRUE;
  Private->LineBuffer             = NULL;

  //
  // Initialize the hardware
  //
  GraphicsOutput->SetMode (GraphicsOutput, 0);
  DrawLogo (Private);

  return EFI_SUCCESS;
}

EFI_STATUS
CirrusLogic5430GraphicsOutputDestructor (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    Private - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  if (Private->GraphicsOutput.Mode != NULL) {
    if (Private->GraphicsOutput.Mode->Info != NULL) {
      gBS->FreePool (Private->GraphicsOutput.Mode->Info);
    }
    gBS->FreePool (Private->GraphicsOutput.Mode);
  }

  return EFI_SUCCESS;
}

VOID
outb (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  UINTN                           Address,
  UINT8                           Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description
  Address - TODO: add argument description
  Data    - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  Private->PciIo->Io.Write (
                      Private->PciIo,
                      EfiPciIoWidthUint8,
                      EFI_PCI_IO_PASS_THROUGH_BAR,
                      Address,
                      1,
                      &Data
                      );
}

VOID
outw (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  UINTN                           Address,
  UINT16                          Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description
  Address - TODO: add argument description
  Data    - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  Private->PciIo->Io.Write (
                      Private->PciIo,
                      EfiPciIoWidthUint16,
                      EFI_PCI_IO_PASS_THROUGH_BAR,
                      Address,
                      1,
                      &Data
                      );
}

UINT8
inb (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  UINTN                           Address
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description
  Address - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UINT8 Data;

  Private->PciIo->Io.Read (
                      Private->PciIo,
                      EfiPciIoWidthUint8,
                      EFI_PCI_IO_PASS_THROUGH_BAR,
                      Address,
                      1,
                      &Data
                      );
  return Data;
}

UINT16
inw (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  UINTN                           Address
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description
  Address - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UINT16  Data;

  Private->PciIo->Io.Read (
                      Private->PciIo,
                      EfiPciIoWidthUint16,
                      EFI_PCI_IO_PASS_THROUGH_BAR,
                      Address,
                      1,
                      &Data
                      );
  return Data;
}

VOID
SetPaletteColor (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  UINTN                           Index,
  UINT8                           Red,
  UINT8                           Green,
  UINT8                           Blue
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description
  Index   - TODO: add argument description
  Red     - TODO: add argument description
  Green   - TODO: add argument description
  Blue    - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  outb (Private, PALETTE_INDEX_REGISTER, (UINT8) Index);
  outb (Private, PALETTE_DATA_REGISTER, (UINT8) (Red >> 2));
  outb (Private, PALETTE_DATA_REGISTER, (UINT8) (Green >> 2));
  outb (Private, PALETTE_DATA_REGISTER, (UINT8) (Blue >> 2));
}

VOID
SetDefaultPalette (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UINTN Index;
  UINTN RedIndex;
  UINTN GreenIndex;
  UINTN BlueIndex;

  Index = 0;
  for (RedIndex = 0; RedIndex < 8; RedIndex++) {
    for (GreenIndex = 0; GreenIndex < 8; GreenIndex++) {
      for (BlueIndex = 0; BlueIndex < 4; BlueIndex++) {
        SetPaletteColor (Private, Index, (UINT8) (RedIndex << 5), (UINT8) (GreenIndex << 5), (UINT8) (BlueIndex << 6));
        Index++;
      }
    }
  }
}

STATIC
VOID
ClearScreen (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UINT32  Color;

  Color = 0;
  Private->PciIo->Mem.Write (
                        Private->PciIo,
                        EfiPciIoWidthFillUint32,
                        0,
                        0,
                        0x100000 >> 2,
                        &Color
                        );
}

VOID
DrawLogo (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UINTN Offset;
  UINTN X;
  UINTN Y;
  UINTN ScreenWidth;
  UINTN ScreenHeight;
  UINT8 Color;

  ScreenWidth   = Private->ModeData[Private->GraphicsOutput.Mode->Mode].HorizontalResolution;
  ScreenHeight  = Private->ModeData[Private->GraphicsOutput.Mode->Mode].VerticalResolution;

  Offset        = 0;
  for (Y = 0; Y < ScreenHeight; Y++) {
    for (X = 0; X < ScreenWidth; X++) {
      Color                   = (UINT8) (256 * (X + Y) / (ScreenWidth + ScreenHeight));
      Private->LineBuffer[X]  = Color;
    }

    Private->PciIo->Mem.Write (
                          Private->PciIo,
                          EfiPciIoWidthUint32,
                          0,
                          Offset + (Y * ScreenWidth),
                          ScreenWidth >> 2,
                          Private->LineBuffer
                          );
  }
}

VOID
InitializeGraphicsMode (
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private,
  CIRRUS_LOGIC_5430_VIDEO_MODES   *ModeData
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private   - TODO: add argument description
  ModeData  - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UINT8 Byte;
  UINTN Index;

  outw (Private, SEQ_ADDRESS_REGISTER, 0x1206);
  outw (Private, SEQ_ADDRESS_REGISTER, 0x0012);

  for (Index = 0; Index < 15; Index++) {
    outw (Private, SEQ_ADDRESS_REGISTER, ModeData->SeqSettings[Index]);
  }

  outb (Private, SEQ_ADDRESS_REGISTER, 0x0f);
  Byte = (UINT8) ((inb (Private, SEQ_DATA_REGISTER) & 0xc7) ^ 0x30);
  outb (Private, SEQ_DATA_REGISTER, Byte);

  outb (Private, MISC_OUTPUT_REGISTER, ModeData->MiscSetting);
  outw (Private, GRAPH_ADDRESS_REGISTER, 0x0506);
  outw (Private, SEQ_ADDRESS_REGISTER, 0x0300);
  outw (Private, CRTC_ADDRESS_REGISTER, 0x2011);

  for (Index = 0; Index < 28; Index++) {
    outw (Private, CRTC_ADDRESS_REGISTER, (UINT16) ((ModeData->CrtcSettings[Index] << 8) | Index));
  }

  for (Index = 0; Index < 9; Index++) {
    outw (Private, GRAPH_ADDRESS_REGISTER, (UINT16) ((GraphicsController[Index] << 8) | Index));
  }

  inb (Private, INPUT_STATUS_1_REGISTER);

  for (Index = 0; Index < 21; Index++) {
    outb (Private, ATT_ADDRESS_REGISTER, (UINT8) Index);
    outb (Private, ATT_ADDRESS_REGISTER, AttributeController[Index]);
  }

  outb (Private, ATT_ADDRESS_REGISTER, 0x20);

  outw (Private, GRAPH_ADDRESS_REGISTER, 0x0009);
  outw (Private, GRAPH_ADDRESS_REGISTER, 0x000a);
  outw (Private, GRAPH_ADDRESS_REGISTER, 0x000b);
  outb (Private, DAC_PIXEL_MASK_REGISTER, 0xff);

  SetDefaultPalette (Private);
  ClearScreen (Private);
}
