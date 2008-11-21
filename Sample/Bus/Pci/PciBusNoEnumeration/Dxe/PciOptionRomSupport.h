/*++

Copyright (c) 2005 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciOptionRomSupport.h
  
Abstract:

  PCI Bus Driver

Revision History

--*/

#ifndef _EFI_PCI_OP_ROM_SUPPORT_H
#define _EFI_PCI_OP_ROM_SUPPORT_H

VOID
InitializePciLoadFile2 (
  PCI_IO_DEVICE       *PciIoDevice
  )
/*++

Routine Description:

  Initialize LoadFile2 instance for PciIoDevice

Arguments:

  PciIoDevice - PCI IO Device

Returns:

  None.

--*/
;

EFI_STATUS
EFIAPI
LoadFile2 (
#if (EFI_SPECIFICATION_VERSION >= 0x0002000A)
  IN EFI_LOAD_FILE2_PROTOCOL  *This,
#else
  IN VOID                     *This,
#endif
  IN EFI_DEVICE_PATH_PROTOCOL *FilePath,
  IN BOOLEAN                  BootPolicy,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer      OPTIONAL
  );

BOOLEAN
ContainEfiImage (
  IN VOID            *RomImage,
  IN UINT64          RomSize
  )
/*++

Routine Description:
  Check if the RomImage contains EFI Images.

--*/
;

EFI_STATUS
GetOpRomInfo (
  IN PCI_IO_DEVICE    *PciIoDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciIoDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
LoadOpRomImage (
  IN PCI_IO_DEVICE   *PciDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciDevice - TODO: add argument description
  RomBase   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
ProcessOpRomImage (
  PCI_IO_DEVICE   *PciDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PciDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
