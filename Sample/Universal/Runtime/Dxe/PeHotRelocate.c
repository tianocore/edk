/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PeHotRelocate.c

Abstract:

  Relocate runtime images.
  
--*/

#include "Runtime.h"

extern VOID                          *mMyImageBase;

VOID *
RuntimePeImageAddress (
  IN   EFI_RUNTIME_IMAGE_ENTRY        *Image,
  IN   UINTN                          Address
  )
/*++

Routine Description:

  Converts an image address to the loaded address.

Arguments:

  Image         - The relocation data of the image being loaded
  Address       - The address to be converted to the loaded address

Returns:

  NULL if the address can not be converted, otherwise, the converted address

--*/
{
  if (Address >= (UINTN) (EFI_SIZE_TO_PAGES ((UINTN)Image->ImageSize) << EFI_PAGE_SHIFT)) {
    return NULL;
  }

  return (CHAR8 *) ((UINTN) Image->ImageBase + Address);
}

VOID
RelocatePeImageForRuntime (
  IN EFI_RUNTIME_IMAGE_ENTRY     *Image
  )
/*++

Routine Description:

  Relocate runtime images.

Arguments:

  Image   - Points to the relocation data of the image.

Returns:

  None.

--*/  
{
  EFI_STATUS                            Status;
  CHAR8                                 *OldBase;
  CHAR8                                 *NewBase;
  EFI_IMAGE_DOS_HEADER                  *DosHdr;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION   Hdr;
  UINT32                                NumberOfRvaAndSizes;
  EFI_IMAGE_DATA_DIRECTORY              *DataDirectory;
  EFI_IMAGE_DATA_DIRECTORY              *RelocDir;
  EFI_IMAGE_BASE_RELOCATION             *RelocBase;
  EFI_IMAGE_BASE_RELOCATION             *RelocBaseEnd;
  UINT16                                *Reloc;
  UINT16                                *RelocEnd;
  CHAR8                                 *Fixup;
  CHAR8                                 *FixupBase;
  UINT16                                *F16;
  UINT32                                *F32;
  UINT64                                *F64;
  CHAR8                                 *FixupData;
  UINTN                                 Adjust;
  UINT16                                Magic;
  
  if (mMyImageBase == (VOID *) (UINTN) Image->ImageBase) {
    //
    // We don't want to relocate our selves, as we only run in physical mode.
    //
    return;
  }
  
  OldBase = (CHAR8 *) ((UINTN) Image->ImageBase);
  NewBase = (CHAR8 *) ((UINTN) Image->ImageBase);

  Status  = RuntimeDriverConvertPointer (0, &NewBase);
  ASSERT_EFI_ERROR (Status);

  Adjust = (UINTN) NewBase - (UINTN) OldBase;

  //
  // Find the image's relocate dir info.
  //
  DosHdr = (EFI_IMAGE_DOS_HEADER *) OldBase;
  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    //
    // Valid DOS header so get address of PE header.
    //
    Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *) (((CHAR8 *) DosHdr) + DosHdr->e_lfanew);
  } else {
    //
    // No Dos header so assume image starts with PE header.
    //
    Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *) OldBase;
  }

  if (Hdr.Pe32->Signature != EFI_IMAGE_NT_SIGNATURE) {
    //
    // Not a valid PE image so Exit.
    //
    return;
  }
  
  //
  // Get some data from the PE type dependent data.
  //
  // NOTE: We use Machine to identify PE32/PE32+, instead of Magic.
  //       It is for backward-compatibility consideration, because
  //       some system will generate PE32+ image with PE32 Magic.
  //
  if (Hdr.Pe32->FileHeader.Machine == EFI_IMAGE_MACHINE_IA32) {
    Magic = EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC;
  } else if (Hdr.Pe32->FileHeader.Machine == EFI_IMAGE_MACHINE_IA64) {
    Magic = EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC;
  } else if (Hdr.Pe32->FileHeader.Machine == EFI_IMAGE_MACHINE_X64) {
    Magic = EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC;
  } else {
    Magic = Hdr.Pe32->OptionalHeader.Magic;
  }
  
  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    //     
    // Use PE32 offset
    //
    NumberOfRvaAndSizes = Hdr.Pe32->OptionalHeader.NumberOfRvaAndSizes;
    DataDirectory = (EFI_IMAGE_DATA_DIRECTORY *)&(Hdr.Pe32->OptionalHeader.DataDirectory[0]);
  } else {
    //     
    // Use PE32+ offset
    //
    NumberOfRvaAndSizes = Hdr.Pe32Plus->OptionalHeader.NumberOfRvaAndSizes;
    DataDirectory = (EFI_IMAGE_DATA_DIRECTORY *)&(Hdr.Pe32Plus->OptionalHeader.DataDirectory[0]);
  }    

  //
  // Find the relocation block.
  //
  // Per the PE/COFF spec, you can't assume that a given data directory
  // is present in the image. You have to check the NumberOfRvaAndSizes in
  // the optional header to verify a desired directory entry is there.
  //
  if (NumberOfRvaAndSizes > EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC) {
    RelocDir      = DataDirectory + EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC;
    RelocBase     = RuntimePeImageAddress (Image, RelocDir->VirtualAddress);
    RelocBaseEnd  = RuntimePeImageAddress (Image, RelocDir->VirtualAddress + RelocDir->Size);
  } else {
    //
    // Cannot find relocations, cannot continue.
    //
    ASSERT (FALSE);
    return;
  }

  ASSERT (RelocBase != NULL && RelocBaseEnd != NULL);

  //
  // Run the whole relocation block. And re-fixup data that has not been
  // modified. The FixupData is used to see if the image has been modified
  // since it was relocated. This is so data sections that have been updated
  // by code will not be fixed up, since that would set them back to
  // defaults.
  //
  FixupData = Image->RelocationData;
  while (RelocBase < RelocBaseEnd) {

    Reloc     = (UINT16 *) ((UINT8 *) RelocBase + sizeof (EFI_IMAGE_BASE_RELOCATION));
    RelocEnd  = (UINT16 *) ((UINT8 *) RelocBase + RelocBase->SizeOfBlock);
    FixupBase = (UINT8 *) ((UINTN) Image->ImageBase) + RelocBase->VirtualAddress;

    //
    // Run this relocation record.
    //
    while (Reloc < RelocEnd) {

      Fixup = FixupBase + (*Reloc & 0xFFF);
      switch ((*Reloc) >> 12) {

      case EFI_IMAGE_REL_BASED_ABSOLUTE:
        break;

      case EFI_IMAGE_REL_BASED_HIGH:
        F16 = (UINT16 *) Fixup;
        if (*(UINT16 *) FixupData == *F16) {
          *F16  = (UINT16) (*F16 + (UINT16)(((UINT32)Adjust) >> 16));
        }

        FixupData = FixupData + sizeof (UINT16);
        break;

      case EFI_IMAGE_REL_BASED_LOW:
        F16 = (UINT16 *) Fixup;
        if (*(UINT16 *) FixupData == *F16) {
          *F16 = (UINT16) (*F16 + ((UINT16) Adjust & 0xffff));
        }

        FixupData = FixupData + sizeof (UINT16);
        break;

      case EFI_IMAGE_REL_BASED_HIGHLOW:
        F32       = (UINT32 *) Fixup;
        FixupData = ALIGN_POINTER (FixupData, sizeof (UINT32));
        if (*(UINT32 *) FixupData == *F32) {
          *F32 = *F32 + (UINT32) Adjust;
        }

        FixupData = FixupData + sizeof (UINT32);
        break;

      case EFI_IMAGE_REL_BASED_DIR64:
        //
        // For X64 and IPF.
        //
        F64       = (UINT64 *)Fixup;
        FixupData = ALIGN_POINTER (FixupData, sizeof (UINT64));
        if (*(UINT64 *) FixupData == *F64) {
          *F64 = *F64 + (UINT64)Adjust;
        }
        
        FixupData = FixupData + sizeof (UINT64);
        break;

      case EFI_IMAGE_REL_BASED_HIGHADJ:
        //
        // Not implemented, but not used in EFI 1.0.
        //
        ASSERT (FALSE);
        break;

      default:
        //
        // Only Itanium requires ConvertPeImage_Ex
        //
        Status = PeHotRelocateImageEx (Reloc, Fixup, &FixupData, Adjust);
        if (EFI_ERROR (Status)) {
          return ;
        }
      }
      //
      // Next relocation record.
      //
      Reloc += 1;
    }
    
    //
    // next reloc block.
    //
    RelocBase = (EFI_IMAGE_BASE_RELOCATION *) RelocEnd;
  }

  EfiCpuFlushCache ((EFI_PHYSICAL_ADDRESS) Image->ImageBase, (UINT64) Image->ImageSize);
}
