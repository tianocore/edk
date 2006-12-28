/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  PeLoader.h

Abstract:

Revision History:

--*/

#ifndef _EFILDR_PELOADER_H_
#define _EFILDR_PELOADER_H_

#include "EfiImage.h"

EFI_STATUS
EfiLdrGetPeImageInfo (
  IN VOID                     *FHand,
  OUT UINT64                  *ImageBase,
  OUT UINT32                  *ImageSize
  );

EFI_STATUS
EfiLdrPeCoffLoadPeImage (
  IN VOID                     *FHand,
  IN EFILDR_LOADED_IMAGE      *Image,
  IN UINTN                    *NumberOfMemoryMapEntries,
  IN EFI_MEMORY_DESCRIPTOR    *EfiMemoryDescriptor
  );

STATIC
EFI_STATUS
EfiLdrPeCoffLoadPeRelocate (
  IN EFILDR_LOADED_IMAGE      *Image,
  IN EFI_IMAGE_DATA_DIRECTORY *RelocDir,
  IN UINTN                     Adjust,
  IN UINTN                    *NumberOfMemoryMapEntries,
  IN EFI_MEMORY_DESCRIPTOR    *EfiMemoryDescriptor
  );

STATIC
EFI_STATUS
EfiLdrPeCoffImageRead (
    IN VOID                 *FHand,
    IN UINTN                Offset,
    IN OUT UINTN            ReadSize,
    OUT VOID                *Buffer
    );

STATIC
VOID *
EfiLdrPeCoffImageAddress (
  IN EFILDR_LOADED_IMAGE     *Image,
  IN UINTN                   Address
  );

EFI_STATUS
EfiLdrPeCoffSetImageType (
  IN OUT EFILDR_LOADED_IMAGE      *Image,
  IN UINTN                        ImageType
  );

EFI_STATUS
EfiLdrPeCoffCheckImageMachineType (
  IN UINT16           MachineType
  );

#endif
