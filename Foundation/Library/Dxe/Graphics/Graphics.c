/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Graphics.c

Abstract:

  Support for Basic Graphics operations.

  BugBug: Currently *.BMP files are supported. This will be replaced
          when Tiano graphics format is supported.

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "GraphicsLib.h"


EFI_STATUS
GetGraphicsBitMapFromFV (
  IN  EFI_GUID      *FileNameGuid,
  OUT VOID          **Image,
  OUT UINTN         *ImageSize
  )
/*++

Routine Description:

  Return the graphics image file named FileNameGuid into Image and return it's
  size in ImageSize. All Firmware Volumes (FV) in the system are searched for the
  file name.

Arguments:

  FileNameGuid  - File Name of graphics file in the FV(s).

  Image         - Pointer to pointer to return graphics image.  If NULL, a 
                  buffer will be allocated.

  ImageSize     - Size of the graphics Image in bytes. Zero if no image found.


Returns: 

  EFI_SUCCESS          - Image and ImageSize are valid. 
  EFI_BUFFER_TOO_SMALL - Image not big enough. ImageSize has required size
  EFI_NOT_FOUND        - FileNameGuid not found

--*/
{
  EFI_STATUS                    Status;
  UINTN                         FvProtocolCount;
  EFI_HANDLE                    *FvHandles;
  EFI_FIRMWARE_VOLUME_PROTOCOL  *Fv;
  UINTN                         Index;
  UINT32                        AuthenticationStatus;


  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolumeProtocolGuid,
                  NULL, 
                  &FvProtocolCount,
                  &FvHandles
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  for (Index = 0; Index < FvProtocolCount; Index++) {
    Status = gBS->HandleProtocol (
                    FvHandles[Index], 
                    &gEfiFirmwareVolumeProtocolGuid,
                    (VOID **)&Fv
                    );

    //
    // Assuming Image and ImageSize are correct on input.
    //
    Status = Fv->ReadSection (
                  Fv, 
                  &gEfiDefaultBmpLogoGuid, 
                  EFI_SECTION_RAW,
                  0,
                  Image, 
                  ImageSize, 
                  &AuthenticationStatus
                  );
    if (!EFI_ERROR (Status)) {
      return EFI_SUCCESS;
    } else if (Status == EFI_BUFFER_TOO_SMALL) {
      //
      // ImageSize updated to needed size so return
      //
      return EFI_BUFFER_TOO_SMALL;
    }
  }

  return EFI_NOT_FOUND;
}


EFI_STATUS
ConvertBmpToUgaBlt (
  IN  VOID      *BmpImage,
  IN  UINTN     BmpImageSize,
  IN OUT VOID   **UgaBlt,
  IN OUT UINTN  *UgaBltSize,
  OUT UINTN     *PixelHeight,
  OUT UINTN     *PixelWidth
  )
/*++

Routine Description:

  Convert a *.BMP graphics image to a UGA blt buffer. If a NULL UgaBlt buffer
  is passed in a UgaBlt buffer will be allocated by this routine. If a UgaBlt
  buffer is passed in it will be used if it is big enough.

Arguments:

  BmpImage      - Pointer to BMP file

  BmpImageSize  - Number of bytes in BmpImage

  UgaBlt        - Buffer containing UGA version of BmpImage.

  UgaBltSize    - Size of UgaBlt in bytes.

  PixelHeight   - Height of UgaBlt/BmpImage in pixels

  PixelWidth    - Width of UgaBlt/BmpImage in pixels


Returns: 

  EFI_SUCCESS           - UgaBlt and UgaBltSize are returned. 
  EFI_UNSUPPORTED       - BmpImage is not a valid *.BMP image
  EFI_BUFFER_TOO_SMALL  - The passed in UgaBlt buffer is not big enough.
                          UgaBltSize will contain the required size.

--*/
{
  UINT8                   *Image;
  BMP_IMAGE_HEADER        *BmpHeader;
  BMP_COLOR_MAP           *BmpColorMap;
  EFI_UGA_PIXEL           *BltBuffer, *Blt;
  UINTN                   BltBufferSize;
  UINTN                   Index;
  UINTN                   Height;
  UINTN                   Width;
  UINTN                   ImageIndex;


  BmpHeader = (BMP_IMAGE_HEADER *)BmpImage;
  if (BmpHeader->CharB != 'B' || BmpHeader->CharM != 'M') {
    return EFI_UNSUPPORTED;
  }

  if (BmpHeader->CompressionType != 0) {
    return EFI_UNSUPPORTED;
  }

  //
  // Calculate Color Map offset in the image.
  //
  Image = BmpImage;
  BmpColorMap = (BMP_COLOR_MAP *)(Image + sizeof (BMP_IMAGE_HEADER));

  //
  // Calculate graphics image data address in the image
  //
  Image = ((UINT8 *)BmpImage) + BmpHeader->ImageOffset;

  BltBufferSize = BmpHeader->PixelWidth * BmpHeader->PixelHeight * sizeof (EFI_UGA_PIXEL);
  if (*UgaBlt == NULL) {
    *UgaBltSize = BltBufferSize;
    *UgaBlt = EfiLibAllocatePool (*UgaBltSize);
    if (*UgaBlt == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  } else {
    if (*UgaBltSize < BltBufferSize) {
      *UgaBltSize = BltBufferSize;
      return EFI_BUFFER_TOO_SMALL;
    }
  }

  *PixelWidth = BmpHeader->PixelWidth;
  *PixelHeight = BmpHeader->PixelHeight;


  //
  // Convert image from BMP to Blt buffer format
  //
  BltBuffer = *UgaBlt;
  for (Height = 0; Height < BmpHeader->PixelHeight; Height++) {
    Blt = &BltBuffer[(BmpHeader->PixelHeight - Height - 1) * BmpHeader->PixelWidth];
    for (Width = 0; Width < BmpHeader->PixelWidth; Width++, Image++, Blt++) {
      switch (BmpHeader->BitPerPixel) {
      case 4:
        //
        // Convert BMP Palette to 24-bit color
        //
        Index = (*Image) >> 4;
        Blt->Red   = BmpColorMap[Index].Red;
        Blt->Green = BmpColorMap[Index].Green;
        Blt->Blue  = BmpColorMap[Index].Blue;
        if (Width < (BmpHeader->PixelWidth - 1)) {
          Blt++;
          Width++;
          Index = (*Image) & 0x0f;
          Blt->Red   = BmpColorMap[Index].Red;
          Blt->Green = BmpColorMap[Index].Green;
          Blt->Blue  = BmpColorMap[Index].Blue;
        }
        break;
      case 8:
        //
        // Convert BMP Palette to 24-bit color
        //
        Blt->Red   = BmpColorMap[*Image].Red;
        Blt->Green = BmpColorMap[*Image].Green;
        Blt->Blue  = BmpColorMap[*Image].Blue;
      break;
      case 24:
        Blt->Blue = *Image++;
        Blt->Green = *Image++;
        Blt->Red = *Image;
        break;
      default:
        return EFI_UNSUPPORTED;
        break;
      };

    }

    ImageIndex = (UINTN)(Image - BmpHeader->ImageOffset);
    if ((ImageIndex % 4) != 0) {
      //
      // Bmp Image starts each row on a 32-bit boundary!
      //
      Image = Image + (4 - (ImageIndex % 4));
    }
  }

  return EFI_SUCCESS;
}


EFI_STATUS
LockKeyboards (
  IN  CHAR16    *Password
  )
/*++

Routine Description:
  Use Console Control Protocol to lock the Console In Spliter virtual handle. 
  This is the ConInHandle and ConIn handle in the EFI system table. All key
  presses will be ignored until the Password is typed in. The only way to
  disable the password is to type it in to a ConIn device.

Arguments:
  Password - Password used to lock ConIn device


Returns: 

  EFI_SUCCESS     - ConsoleControl has been flipped to graphics and logo
                          displayed.
  EFI_UNSUPPORTED - Logo not found

--*/
{
  EFI_STATUS                    Status;
  EFI_CONSOLE_CONTROL_PROTOCOL  *ConsoleControl;

  Status = gBS->LocateProtocol (&gEfiConsoleControlProtocolGuid, NULL, &ConsoleControl);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Status = ConsoleControl->LockStdIn (ConsoleControl, Password);
  return Status;
}



EFI_STATUS
EnableQuietBoot (
  IN  EFI_GUID  *LogoFile
  )
/*++

Routine Description:

  Use Console Control to turn off UGA based Simple Text Out consoles from going
  to the UGA device. Put up LogoFile on every UGA device that is a console

Arguments:

  LogoFile - File name of logo to display on the center of the screen.


Returns: 

  EFI_SUCCESS           - ConsoleControl has been flipped to graphics and logo
                          displayed.
  EFI_UNSUPPORTED       - Logo not found

--*/
{
  EFI_STATUS                    Status;
  EFI_CONSOLE_CONTROL_PROTOCOL  *ConsoleControl;
  EFI_UGA_DRAW_PROTOCOL         *UgaDraw;
  EFI_OEM_BADGING_PROTOCOL      *Badging;
  UINT32                        SizeOfX;
  UINT32                        SizeOfY;
  UINT32                        ColorDepth;
  UINT32                        RefreshRate;
  INTN                          DestX;
  INTN                          DestY;

  UINT8                         *ImageData;
  UINTN                         ImageSize;
  EFI_UGA_PIXEL                 *UgaBlt;
  UINTN                         UgaBltSize;

  UINT32                         Instance;
  EFI_BADGING_FORMAT             Format;
  EFI_BADGING_DISPLAY_ATTRIBUTE  Attribute;
  UINTN                          CoordinateX;
  UINTN                          CoordinateY;  
  UINTN                          Height;
  UINTN                          Width;

  Status = gBS->LocateProtocol (&gEfiConsoleControlProtocolGuid, NULL, &ConsoleControl);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->HandleProtocol (gST->ConsoleOutHandle, &gEfiUgaDrawProtocolGuid, &UgaDraw);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Badging = NULL;
  Status = gBS->LocateProtocol (&gEfiOEMBadgingProtocolGuid, NULL, &Badging);
  
  ConsoleControl->SetMode (ConsoleControl, EfiConsoleControlScreenGraphics);

  Status = UgaDraw->GetMode (UgaDraw, &SizeOfX, &SizeOfY, &ColorDepth, &RefreshRate);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }


  Instance = 0;
  while (1) {  
    ImageData = NULL;
    ImageSize = 0;
    
    if (Badging != NULL) {
      Status = Badging->GetImage (Badging,
                &Instance,
            &Format,
            &ImageData,
            &ImageSize,
            &Attribute,
            &CoordinateX,
            &CoordinateY
            );
      if (EFI_ERROR(Status)) {
        return Status;
      }

      //
      //Currently only support BMP format
      //
      if (Format != EfiBadgingFormatBMP) {
        gBS->FreePool (ImageData);
        continue;
      }
    } else {
      Status = GetGraphicsBitMapFromFV (LogoFile, &ImageData, &ImageSize);
      if (EFI_ERROR (Status)) {
        return EFI_UNSUPPORTED;
      }
      CoordinateX = 0;
      CoordinateY = 0;
      Attribute = EfiBadgingDisplayAttributeCenter;
    }


    UgaBlt = NULL;
    Status = ConvertBmpToUgaBlt (ImageData,
                                 ImageSize,
                                 &UgaBlt,
                                 &UgaBltSize,
                                 &Height,
                                 &Width
                                 );
    if (EFI_ERROR (Status)) {
      gBS->FreePool (ImageData);
      continue;
    }
    switch (Attribute) {
      case EfiBadgingDisplayAttributeLeftTop:
        DestX = CoordinateX;
        DestY = CoordinateY;
        break;
      case EfiBadgingDisplayAttributeCenterTop:
        DestX = (SizeOfX - Width)/2;
        DestY = CoordinateY;
        break;
      case EfiBadgingDisplayAttributeRightTop:
        DestX = (SizeOfX - Width - CoordinateX);
        DestY = CoordinateY;;
        break;
      case EfiBadgingDisplayAttributeCenterRight:
        DestX = (SizeOfX - Width - CoordinateX);
        DestY = (SizeOfY - Height)/2;
        break;
      case EfiBadgingDisplayAttributeRightBottom:
        DestX = (SizeOfX - Width - CoordinateX);
        DestY = (SizeOfY - Height - CoordinateY);
        break;
      case EfiBadgingDisplayAttributeCenterBottom:
        DestX = (SizeOfX - Width)/2;
        DestY = (SizeOfY - Height - CoordinateY);
        break;
      case EfiBadgingDisplayAttributeLeftBottom:
        DestX = CoordinateX;
        DestY = (SizeOfY - Height - CoordinateY);
        break;
      case EfiBadgingDisplayAttributeCenterLeft:
        DestX = CoordinateX;
        DestY = (SizeOfY - Height)/2;
        break;
      case EfiBadgingDisplayAttributeCenter:
        DestX = (SizeOfX - Width)/2;
        DestY = (SizeOfY - Height)/2;
        break;
      default:
        DestX = CoordinateX;
        DestY = CoordinateY;
        break;
    }
    if ((DestX >= 0) && (DestY >= 0)) { 
      Status = UgaDraw->Blt (
            UgaDraw,
            UgaBlt,               EfiUgaBltBufferToVideo,           
            0,                    0,
            (UINTN)DestX, (UINTN)DestY,
            Width,                Height,
            Width * sizeof (EFI_UGA_PIXEL)
            );
    }
    gBS->FreePool (ImageData);
    gBS->FreePool (UgaBlt);
    
    if (Badging == NULL) {
      break;
    }
  }
      
  return Status;
}



EFI_STATUS
DisableQuietBoot (
  VOID
  )
/*++

Routine Description:

  Use Console Control to turn on UGA based Simple Text Out consoles. The UGA 
  Simple Text Out screens will now be synced up with all non UGA output devices

Arguments:

  NONE

Returns: 

  EFI_SUCCESS           - UGA devices are back in text mode and synced up.
  EFI_UNSUPPORTED       - Logo not found

--*/
{
  EFI_STATUS                    Status;
  EFI_CONSOLE_CONTROL_PROTOCOL  *ConsoleControl;

  Status = gBS->LocateProtocol (&gEfiConsoleControlProtocolGuid, NULL, &ConsoleControl);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  return ConsoleControl->SetMode (ConsoleControl, EfiConsoleControlScreenText);
}














