/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  GraphicsLib.h

Abstract:

 
--*/

#ifndef _EFI_GRAPHICS_LIB_H_
#define _EFI_GRAPHICS_LIB_H_

#include EFI_PROTOCOL_DEFINITION (ConsoleControl)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolume)
#include EFI_PROTOCOL_DEFINITION (UgaDraw)
#include EFI_PROTOCOL_DEFINITION (EfiOEMBadging)

#include EFI_GUID_DEFINITION (Bmp)


EFI_STATUS
GetGraphicsBitMapFromFV (
  IN  EFI_GUID      *FileNameGuid,
  OUT VOID          **Image,
  OUT UINTN         *ImageSize
  );

EFI_STATUS
ConvertBmpToUgaBlt (
  IN  VOID      *BmpImage,
  IN  UINTN     BmpImageSize,
  IN OUT VOID   **UgaBlt,
  IN OUT UINTN  *UgaBltSize,
  OUT UINTN     *PixelHeight,
  OUT UINTN     *PixelWidth
  );

EFI_STATUS
EnableQuietBoot (
  IN  EFI_GUID  *LogoFile
  );

EFI_STATUS
DisableQuietBoot (
  VOID
  );

EFI_STATUS
LockKeyboards (
  IN  CHAR16    *Password
  );


#endif
