/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ImageRead.c

Abstract:

--*/

#include "TianoCommon.h"
#include "EfiCommonLib.h"
#include "Pei.h"
#include "PeiLib.h"
#include EFI_GUID_DEFINITION (PeiPeCoffLoader)

EFI_STATUS
PeiImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINTN   *ReadSize,
  OUT    VOID    *Buffer
  )
/*++

Routine Description:

  Support routine for the PE/COFF Loader that reads a buffer from a PE/COFF file

Arguments:

  FileHandle - The handle to the PE/COFF file

  FileOffset - The offset, in bytes, into the file to read

  ReadSize   - The number of bytes to read from the file starting at FileOffset

  Buffer     - A pointer to the buffer to read the data into.

Returns:

  EFI_SUCCESS - ReadSize bytes of data were read into Buffer from the PE/COFF file starting at FileOffset

--*/
{
  CHAR8 *Destination8;
  CHAR8 *Source8;
  UINTN Length;

  Destination8  = Buffer;
  Source8       = (CHAR8 *) ((UINTN) FileHandle + FileOffset);
  Length        = *ReadSize;
  while (Length--) {
    *(Destination8++) = *(Source8++);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetImageReadFunction (
  IN      EFI_PEI_SERVICES                      **PeiServices,
  IN      EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
/*++

Routine Description:

  Support routine to return the Image Read

Arguments:

  PeiServices   - PEI Services Table

  ImageContext  - The context of the image being loaded

Returns:

  EFI_SUCCESS - If Image function location is found

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  MemoryBuffer;

  Status = (*PeiServices)->AllocatePages (
                            PeiServices,
                            EfiBootServicesData,
                            0x400 / EFI_PAGE_SIZE + 1,
                            &MemoryBuffer
                            );
  ASSERT_PEI_ERROR (PeiServices, Status);

  (*PeiServices)->CopyMem (
                    (VOID *) (UINTN) MemoryBuffer,
                    (VOID *) (UINTN) PeiImageRead,
                    0x400
                    );

  ImageContext->ImageRead = (EFI_PEI_PE_COFF_LOADER_READ_FILE) (UINTN) MemoryBuffer;

  return Status;
}
