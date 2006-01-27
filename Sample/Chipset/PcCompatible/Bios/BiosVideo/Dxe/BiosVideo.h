/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  BiosVideo.h
    
Abstract: 

Revision History
--*/

#ifndef _BIOS_UGA_H
#define _BIOS_UGA_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "VesaBiosExtensions.h"
#include "Pci22.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (LegacyBiosThunk)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (UgaDraw)
//
// Packed format support: The number of bits reserved for each of the colors and the actual
// position of RGB in the frame buffer is specified in the VBE Mode information
//
typedef struct {
  UINT8 Position; // Position of the color
  UINT8 Mask;     // The number of bits expressed as a mask
} BIOS_VIDEO_COLOR_PLACEMENT;

//
// BIOS UGA Draw Graphical Mode Data
//
typedef struct {
  UINT16                      VbeModeNumber;
  UINT16                      BytesPerScanLine;
  VOID                        *LinearFrameBuffer;
  UINT32                      HorizontalResolution;
  UINT32                      VerticalResolution;
  UINT32                      ColorDepth;
  UINT32                      RefreshRate;
  UINT32                      BitsPerPixel;
  BIOS_VIDEO_COLOR_PLACEMENT  Red;
  BIOS_VIDEO_COLOR_PLACEMENT  Green;
  BIOS_VIDEO_COLOR_PLACEMENT  Blue;
} BIOS_VIDEO_MODE_DATA;

//
// BIOS UGA Device Structure
//
#define BIOS_VIDEO_DEV_SIGNATURE  EFI_SIGNATURE_32 ('B', 'V', 'M', 'p')

typedef struct {
  UINTN                                       Signature;
  EFI_HANDLE                                  Handle;

  //
  // Consumed Protocols
  //
  EFI_PCI_IO_PROTOCOL                         *PciIo;
  LEGACY_BIOS_THUNK_PROTOCOL                  *LegacyBiosThunk;

  //
  // Produced Protocols
  //
  EFI_UGA_DRAW_PROTOCOL                       UgaDraw;

  //
  // General fields
  //
  EFI_EVENT                                   ExitBootServicesEvent;
  BOOLEAN                                     VgaCompatible;

  //
  // UGA Draw related fields
  //
  BOOLEAN                                     HardwareNeedsStarting;
  UINTN                                       CurrentMode;
  UINTN                                       MaxMode;
  BIOS_VIDEO_MODE_DATA                        *ModeData;
  UINT8                                       *LineBuffer;
  EFI_UGA_PIXEL                               *VbeFrameBuffer;
  UINT8                                       *VgaFrameBuffer;

  //
  // VESA Bios Extensions related fields
  //
  UINTN                                       NumberOfPagesBelow1MB;    // Number of 4KB pages in PagesBelow1MB
  EFI_PHYSICAL_ADDRESS                        PagesBelow1MB;            // Buffer for all VBE Information Blocks
  VESA_BIOS_EXTENSIONS_INFORMATION_BLOCK      *VbeInformationBlock;     // 0x200 bytes.  Must be allocated below 1MB
  VESA_BIOS_EXTENSIONS_MODE_INFORMATION_BLOCK *VbeModeInformationBlock; // 0x100 bytes.  Must be allocated below 1MB
  VESA_BIOS_EXTENSIONS_CRTC_INFORMATION_BLOCK *VbeCrtcInformationBlock; // 59 bytes.  Must be allocated below 1MB
  UINTN                                       VbeSaveRestorePages;      // Number of 4KB pages in VbeSaveRestoreBuffer
  EFI_PHYSICAL_ADDRESS                        VbeSaveRestoreBuffer;     // Must be allocated below 1MB
  //
  // Status code
  //
  EFI_DEVICE_PATH_PROTOCOL                    *DevicePath;
} BIOS_VIDEO_DEV;

#define BIOS_VIDEO_DEV_FROM_UGA_DRAW_THIS(a)      CR (a, BIOS_VIDEO_DEV, UgaDraw, BIOS_VIDEO_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gBiosVideoDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gBiosVideoComponentName;

//
// Driver Binding Protocol functions
//
EFI_STATUS
EFIAPI
BiosVideoDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                - TODO: add argument description
  Controller          - TODO: add argument description
  RemainingDevicePath - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
BiosVideoDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                - TODO: add argument description
  Controller          - TODO: add argument description
  RemainingDevicePath - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
BiosVideoDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This              - TODO: add argument description
  Controller        - TODO: add argument description
  NumberOfChildren  - TODO: add argument description
  ChildHandleBuffer - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

//
// Private worker functions
//
EFI_STATUS
BiosVideoCheckForVbe (
  BIOS_VIDEO_DEV  *BiosVideoPrivate
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  BiosVideoPrivate  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
BiosVideoCheckForVga (
  BIOS_VIDEO_DEV  *BiosVideoPrivate
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  BiosVideoPrivate  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

//
// BIOS UGA Draw Protocol functions
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

  TODO: add return values

--*/
;

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

  TODO: add return values

--*/
;

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

  TODO: add return values

--*/
;

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

  TODO: add return values

--*/
;


BOOLEAN
BiosVideoIsVga (
  IN  EFI_PCI_IO_PROTOCOL       *PciIo
  )
;


//
// Standard VGA Definitions
//
#define VGA_HORIZONTAL_RESOLUTION                         640
#define VGA_VERTICAL_RESOLUTION                           480
#define VGA_NUMBER_OF_BIT_PLANES                          4
#define VGA_PIXELS_PER_BYTE                               8
#define VGA_BYTES_PER_SCAN_LINE                           (VGA_HORIZONTAL_RESOLUTION / VGA_PIXELS_PER_BYTE)
#define VGA_BYTES_PER_BIT_PLANE                           (VGA_VERTICAL_RESOLUTION * VGA_BYTES_PER_SCAN_LINE)

#define VGA_GRAPHICS_CONTROLLER_ADDRESS_REGISTER          0x3ce
#define VGA_GRAPHICS_CONTROLLER_DATA_REGISTER             0x3cf

#define VGA_GRAPHICS_CONTROLLER_SET_RESET_REGISTER        0x00

#define VGA_GRAPHICS_CONTROLLER_ENABLE_SET_RESET_REGISTER 0x01

#define VGA_GRAPHICS_CONTROLLER_COLOR_COMPARE_REGISTER    0x02

#define VGA_GRAPHICS_CONTROLLER_DATA_ROTATE_REGISTER      0x03
#define VGA_GRAPHICS_CONTROLLER_FUNCTION_REPLACE          0x00
#define VGA_GRAPHICS_CONTROLLER_FUNCTION_AND              0x08
#define VGA_GRAPHICS_CONTROLLER_FUNCTION_OR               0x10
#define VGA_GRAPHICS_CONTROLLER_FUNCTION_XOR              0x18

#define VGA_GRAPHICS_CONTROLLER_READ_MAP_SELECT_REGISTER  0x04

#define VGA_GRAPHICS_CONTROLLER_MODE_REGISTER             0x05
#define VGA_GRAPHICS_CONTROLLER_READ_MODE_0               0x00
#define VGA_GRAPHICS_CONTROLLER_READ_MODE_1               0x08
#define VGA_GRAPHICS_CONTROLLER_WRITE_MODE_0              0x00
#define VGA_GRAPHICS_CONTROLLER_WRITE_MODE_1              0x01
#define VGA_GRAPHICS_CONTROLLER_WRITE_MODE_2              0x02
#define VGA_GRAPHICS_CONTROLLER_WRITE_MODE_3              0x03

#define VGA_GRAPHICS_CONTROLLER_MISCELLANEOUS_REGISTER    0x06

#define VGA_GRAPHICS_CONTROLLER_COLOR_DONT_CARE_REGISTER  0x07

#define VGA_GRAPHICS_CONTROLLER_BIT_MASK_REGISTER         0x08

#endif
