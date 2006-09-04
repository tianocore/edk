/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Runtime.h

Abstract:

  Runtime Architectural Protocol as defined in the DXE CIS

  This code is used to produce the EFI runtime architectural protocol.

--*/

#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include "Tiano.h"
#include "EfiRuntimeLib.h"
#include "EfiImage.h"
#include "LinkedList.h"

//
// Status code caller id guid definition
//
#include EFI_GUID_DEFINITION (StatusCodeCallerId)

//
//
// Consumed protocols
//
#include EFI_PROTOCOL_CONSUMER (LoadedImage)
#if (EFI_SPECIFICATION_VERSION < 0x00020000)
#include EFI_PROTOCOL_DEFINITION (UgaIo)
#endif

//
// Produced protocols
//
#include EFI_ARCH_PROTOCOL_PRODUCER (Runtime)

//
// Function Prototypes
//
VOID
RelocatePeImageForRuntime (
  IN EFI_RUNTIME_IMAGE_ENTRY  *Image
  )
/*++

Routine Description:

  Relocate runtime images.

Arguments:

  Image   - Points to the relocation data of the image.

Returns:

  None.

--*/  
;

EFI_STATUS
PeHotRelocateImageEx (
  IN UINT16      *Reloc,
  IN OUT CHAR8   *Fixup,
  IN OUT CHAR8   **FixupData,
  IN UINT64      Adjust
  )
/*++

Routine Description:

  Performs Processor specific relocation fixup

Arguments:

  Reloc      - Pointer to the relocation record
  Fixup      - Pointer to the address to fix up
  FixupData  - Pointer to a buffer to log the fixups
  Adjust     - The offset to adjust the fixup

Returns:

  EFI_SUCCESS     - The relocation item fixed up
  EFI_UNSUPPORTED - The relocation item is of unsupported relocation type
  
--*/
;

EFI_STATUS
EFIAPI
RuntimeDriverCalculateCrc32 (
  IN  VOID    *Data,
  IN  UINTN   DataSize,
  OUT UINT32  *CrcOut
  )
/*++

Routine Description:

  Calculate CRC32 for target data

Arguments:

  Data     - The target data.
  DataSize - The target data size.
  CrcOut   - The CRC32 for target data.

Returns:

  EFI_SUCCESS           - The CRC32 for target data is calculated successfully.
  EFI_INVALID_PARAMETER - Some parameter is not valid, so the CRC32 is not 
                          calculated.

--*/
;

EFI_STATUS
EFIAPI
RuntimeDriverConvertPointer (
  IN     UINTN  DebugDisposition,
  IN OUT VOID   **ConvertAddress
  )
/*++

Routine Description:

  Determines the new virtual address that is to be used on subsequent memory accesses.

Arguments:
  
  DebugDisposition    - Supplies type information for the pointer being converted.
  ConvertAddress      - A pointer to a pointer that is to be fixed to be the value needed
                        for the new virtual address mappings being applied.

Returns:

  EFI_SUCCESS             - The pointer pointed to by Address was modified.
  EFI_NOT_FOUND           - The pointer pointed to by Address was not found to be part
                            of the current memory map. This is normally fatal.
  EFI_INVALID_PARAMETER   - One of the parameters has an invalid value.

--*/
;

VOID
RuntimeDriverInitializeCrc32Table (
  VOID
  )
/*++

Routine Description:

  Initialize CRC32 table.

Arguments:

  None.

Returns:

  None.

--*/
;

EFI_STATUS
EFIAPI
RuntimeDriverInitialize (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
/*++

Routine Description:

  Install Runtime AP. This code includes the EfiRuntimeLib, but it only 
  functions at RT in physical mode. 

Arguments:
  
  ImageHandle   - Image handle of this driver.
  SystemTable   - Pointer to the EFI System Table.

Returns:

  EFI_SUCEESS - Runtime Driver Architectural Protocol installed.

--*/
;

#endif
