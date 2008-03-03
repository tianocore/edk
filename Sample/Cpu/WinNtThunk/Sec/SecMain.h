/*++

Copyright (c) 2004 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             


Module Name:
  SecMain.h

Abstract:
  Include file for Windows API based SEC

--*/

// TODO: add protective #ifndef
#include "stdio.h"
#include "Efi2WinNT.h"
#include "PeiHob.h"
#include "PeiLib.h"
#include "peihoblib.h"
#include "EfiImageFormat.h"
#include "EfiFirmwareVolumeHeader.h"
#include "EfiFirmwareFileSystem.h"
#include "FlashLayout.h"
#include "EfiStatusCode.h"
#include "EfiCommonLib.h"
#include "Pei.h"

#include EFI_PPI_DEFINITION (NtPeiLoadFile)
#include EFI_PPI_DEFINITION (NtAutoscan)
#include EFI_PPI_DEFINITION (NtThunk)
#include EFI_PPI_DEFINITION (NtFwh)
#include EFI_PPI_DEFINITION (NtLoadAsDll)
#include EFI_PPI_DEFINITION (StatusCode)
#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)
#include EFI_GUID_DEFINITION (PeiFlushInstructionCache)
#include EFI_GUID_DEFINITION (PeiPeCoffLoader)
#include EFI_GUID_DEFINITION (PeiTransferControl)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)
#include EFI_GUID_DEFINITION (FirmwareFileSystem)
#include EFI_GUID_DEFINITION (FirmwareFileSystem2)


#define EFI_MEMORY_SIZE_STR       "EFI_MEMORY_SIZE"
#define EFI_FIRMWARE_VOLUMES_STR  "EFI_FIRMWARE_VOLUMES"
#define EFI_BOOT_MODE_STR         "EFI_BOOT_MODE"

typedef struct {
  EFI_PHYSICAL_ADDRESS  Address;
  UINT64                Size;
} NT_FD_INFO;

typedef struct {
  EFI_PHYSICAL_ADDRESS  Memory;
  UINT64                Size;
} NT_SYSTEM_MEMORY;

#define MAX_PDB_NAME_TO_MOD_HANDLE_ARRAY_SIZE 0x100

typedef struct {
  CHAR8   *PdbPointer;
  VOID    *ModHandle;
} PDB_NAME_TO_MOD_HANDLE;

EFI_STATUS
EFIAPI
SecWinNtPeiLoadFile (
  VOID                  *Pe32Data,  // TODO: add IN/OUT modifier to Pe32Data
  EFI_PHYSICAL_ADDRESS  *ImageAddress,  // TODO: add IN/OUT modifier to ImageAddress
  UINT64                *ImageSize,  // TODO: add IN/OUT modifier to ImageSize
  EFI_PHYSICAL_ADDRESS  *EntryPoint  // TODO: add IN/OUT modifier to EntryPoint
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Pe32Data      - TODO: add argument description
  ImageAddress  - TODO: add argument description
  ImageSize     - TODO: add argument description
  EntryPoint    - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecWinNtPeiAutoScan (
  IN  UINTN                 Index,
  OUT EFI_PHYSICAL_ADDRESS  *MemoryBase,
  OUT UINT64                *MemorySize
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Index       - TODO: add argument description
  MemoryBase  - TODO: add argument description
  MemorySize  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecWinNtWinNtThunkAddress (
  IN OUT UINT64                *InterfaceSize,
  IN OUT EFI_PHYSICAL_ADDRESS  *InterfaceBase
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  InterfaceSize - TODO: add argument description
  InterfaceBase - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecWinNtWinNtFwhAddress (
  IN OUT UINT64                *FwhSize,
  IN OUT EFI_PHYSICAL_ADDRESS  *FwhBase
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  FwhSize - TODO: add argument description
  FwhBase - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecPeiReportStatusCode (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN EFI_STATUS_CODE_TYPE     CodeType,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 * CallerId,
  IN EFI_STATUS_CODE_DATA     * Data OPTIONAL
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PeiServices - TODO: add argument description
  CodeType    - TODO: add argument description
  Value       - TODO: add argument description
  Instance    - TODO: add argument description
  CallerId    - TODO: add argument description
  Data        - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#if  (PI_SPECIFICATION_VERSION  < 0x00010000)

VOID
SecSwitchStacks (
  IN VOID                       *EntryPoint,
  IN EFI_PEI_STARTUP_DESCRIPTOR *PeiStartup,
  IN VOID                       *NewStack,
  IN VOID                       *NewBsp
  )
;
#else

VOID
SecSwitchStacks (
  IN VOID                       *EntryPoint,
  IN EFI_SEC_PEI_HAND_OFF       *SecCoreData,
  IN EFI_PEI_PPI_DESCRIPTOR     *PpList,
  IN VOID                       *NewStack,
  IN VOID                       *NewBsp
  )
;

#endif

VOID
CopyMem (
  IN VOID   *Destination,
  IN VOID   *Source,
  IN UINTN  Length
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Destination - TODO: add argument description
  Source      - TODO: add argument description
  Length      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

INTN
EFIAPI
main (
  IN  INTN  Argc,
  IN  CHAR8 **Argv,
  IN  CHAR8 **Envp
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Argc  - TODO: add argument description
  Argv  - TODO: add argument description
  Envp  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
WinNtOpenFile (
  CHAR16                *FileName,
  UINT32                MapSize,
  DWORD                 CreationDispostion,
  EFI_PHYSICAL_ADDRESS  *BaseAddress,
  UINT64                *Length
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  FileName            - TODO: add argument description
  MapSize             - TODO: add argument description
  CreationDispostion  - TODO: add argument description
  BaseAddress         - TODO: add argument description
  Length              - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SecLoadFromCore (
  IN  UINTN   LargestRegion,
  IN  UINTN   LargestRegionSize,
  IN  UINTN   BootFirmwareVolumeBase,
  IN  VOID    *PeiCoreFile
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  LargestRegion           - TODO: add argument description
  LargestRegionSize       - TODO: add argument description
  BootFirmwareVolumeBase  - TODO: add argument description
  PeiCoreFile             - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
SwitchStacks (
  VOID  *EntryPoint,
  UINTN Parameter,
  VOID  *NewStack,
  VOID  *NewBsp
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  EntryPoint  - TODO: add argument description
  Parameter   - TODO: add argument description
  NewStack    - TODO: add argument description
  NewBsp      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
SecLoadFile (
  IN  VOID                    *Pe32Data,
  IN  EFI_PHYSICAL_ADDRESS    *ImageAddress,
  IN  UINT64                  *ImageSize,
  IN  EFI_PHYSICAL_ADDRESS    *EntryPoint
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Pe32Data      - TODO: add argument description
  ImageAddress  - TODO: add argument description
  ImageSize     - TODO: add argument description
  EntryPoint    - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
SecFfsFindPeiCore (
  IN  EFI_FIRMWARE_VOLUME_HEADER  *FwVolHeader,
  OUT VOID                        **Pe32Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  FwVolHeader - TODO: add argument description
  Pe32Data    - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
SecFfsFindNextFile (
  IN EFI_FV_FILETYPE             SearchType,
  IN EFI_FIRMWARE_VOLUME_HEADER  *FwVolHeader,
  IN OUT EFI_FFS_FILE_HEADER     **FileHeader
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  SearchType  - TODO: add argument description
  FwVolHeader - TODO: add argument description
  FileHeader  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
SecFfsFindSectionData (
  IN EFI_SECTION_TYPE      SectionType,
  IN EFI_FFS_FILE_HEADER   *FfsFileHeader,
  IN OUT VOID              **SectionData
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  SectionType   - TODO: add argument description
  FfsFileHeader - TODO: add argument description
  SectionData   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecWinNtPeCoffLoaderLoadAsDll (
  IN CHAR8    *PdbFileName,
  IN VOID     **ImageEntryPoint,
  OUT VOID    **ModHandle
  )
/*++

Routine Description:
  Loads the .DLL file is present when a PE/COFF file is loaded.  This provides source level
  debugging for drivers that have cooresponding .DLL files on the local system.

Arguments:
  PdbFileName     - The name of the .PDB file.  This was found from the PE/COFF
                    file's debug directory entry.
  ImageEntryPoint - A pointer to the DLL entry point of the .DLL file was loaded.
  ModHandle       - A pointer to the DLL Module of the .DLL file was loaded.

Returns:
  EFI_SUCCESS         - The .DLL file was loaded, and the DLL entry point is returned in
                        ImageEntryPoint
  EFI_NOT_FOUND       - The .DLL file could not be found
  EFI_UNSUPPORTED     - The .DLL file was loaded, but the entry point to the .DLL file
                        could not determined.
  EFI_ALREADY_STARTED - The .DLL file was already loaded

--*/
;

EFI_STATUS
EFIAPI
SecWinNtPeCoffLoaderFreeLibrary (
  IN VOID     *ModHandle
  )
/*++

Routine Description:
  Free resources allocated by SecWinNtPeCoffLoaderLoadAsDll

Arguments:
  MohHandle   - Handle of the resources to free to undo the work.

Returns:
  EFI_SUCCESS - This resource is freed successfully.

--*/
;

EFI_STATUS
EFIAPI
SecWinNtFdAddress (
  IN     UINTN                 Index,
  IN OUT EFI_PHYSICAL_ADDRESS  *FdBase,
  IN OUT UINT64                *FdSize
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Index   - TODO: add argument description
  FdBase  - TODO: add argument description
  FdSize  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
GetImageReadFunction (
  IN EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT  *ImageContext,
  IN EFI_PHYSICAL_ADDRESS                  *TopOfMemory
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ImageContext  - TODO: add argument description
  TopOfMemory   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SecImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINTN   *ReadSize,
  OUT    VOID    *Buffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  FileHandle  - TODO: add argument description
  FileOffset  - TODO: add argument description
  ReadSize    - TODO: add argument description
  Buffer      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
InstallEfiPeiTransferControl (
  IN OUT EFI_PEI_TRANSFER_CONTROL_PROTOCOL  **This
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
InstallEfiPeiFlushInstructionCache (
  IN OUT EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  **This
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

CHAR16                            *
AsciiToUnicode (
  IN  CHAR8   *Ascii,
  IN  UINTN   *StrLen OPTIONAL
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Ascii   - TODO: add argument description
  StrLen  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINTN
CountSeperatorsInString (
  IN  CHAR8   *String,
  IN  CHAR8   Seperator
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  String    - TODO: add argument description
  Seperator - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
AddModHandle (
  IN  CHAR8         *PdbPointer,
  IN  VOID          *ModHandle
  )
/*++

Routine Description:
  Store the ModHandle in an array indexed by the Pdb File name.
  The ModHandle is needed to unload the image. 

Arguments:
  PdbPointer - Input data returned from PE Laoder Library. Used to find the 
               .PDB file name of the PE Image.
  ModHandle  - Returned from LoadLibraryEx() and stored for call to 
                 FreeLibrary().

Returns:
  EFI_SUCCESS - ModHandle was stored. 

--*/
;

VOID *
RemoveModeHandle (
  IN  CHAR8         *ModHandle
  )
/*++

Routine Description:
  Return the ModHandle and delete the entry in the array.

Arguments:
  ModHandle  - Returned from LoadLibraryEx() and stored for call to 
                 FreeLibrary().

Returns:
  ModHandle - ModHandle assoicated with Image ModHandle is returned
  NULL      - No ModHandle associated with Image ModHandle

--*/
;

VOID *
GetModHandle (
  IN  CHAR8         *PdbPointer
  )
/*++

Routine Description:
  Search the ModHandle in an array indexed by the PDB File name.

Arguments:
  PdbPointer - Input data returned from PE Laoder Library. Used to find the 
               .PDB file name of the PE Image.

Returns:
  ModHandle - ModHandle assoicated with Image PdbPointer is returned
  NULL      - No ModHandle associated with Image PdbPointer

--*/
;

extern EFI_WIN_NT_THUNK_PROTOCOL  *gWinNt;
