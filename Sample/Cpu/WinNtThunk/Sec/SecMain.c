/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SecMain.c

Abstract:

  WinNt emulator of SEC phase. 
  We build a file to emulate memory. 
  We build an SEC handoff state on the stack. 
  We put SEC handoff into the memory. 
  We memory map the Boot Firmware Volume into memory.

--*/

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
#include EFI_GUID_DEFINITION (FirmwareFileSystem)
#include EFI_GUID_DEFINITION (PeiFlushInstructionCache)
#include EFI_GUID_DEFINITION (PeiPeCoffLoader)
#include EFI_GUID_DEFINITION (PeiTransferControl)
#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)
#include "ImageRead.h"
#include "Pei.h"
#include EFI_PPI_DEFINITION (NtPeiLoadFile)
#include EFI_PPI_DEFINITION (NtAutoscan)
#include EFI_PPI_DEFINITION (NtThunk)
#include EFI_PPI_DEFINITION (NtFwh)
#include EFI_PPI_DEFINITION (NtLoadAsDll)
#include EFI_PPI_DEFINITION (StatusCode)
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)
#include "CustomizedDecompress.h"

#define EFI_MEMORY_SIZE_STR       "EFI_MEMORY_SIZE"
#define EFI_FIRMWARE_VOLUMES_STR  "EFI_FIRMWARE_VOLUMES"
#define EFI_BOOT_MODE_STR         "EFI_BOOT_MODE"
#define MAX_FILE_NAME_LENGTH      280

//
// Ugly globals
//


EFI_PEI_PE_COFF_LOADER_PROTOCOL          *gPeiEfiPeiPeCoffLoader;
EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL *gPeiEfiPeiFlushInstructionCache;

EFI_PHYSICAL_ADDRESS                     gTopOfMemory;
EFI_PEI_HOB_POINTERS                         gLargestMemoryHob;
EFI_PHYSICAL_ADDRESS                     gEfiEmulatorBootFirmwareDevice;
UINT64                                   gEfiEmulatorBootFirmwareDeviceSize;


typedef struct  {
  EFI_PEI_PPI_DESCRIPTOR  PeiLoadFileServiceThunk;
  EFI_PEI_PPI_DESCRIPTOR  PeiAutoscanServiceThunk;
  EFI_PEI_PPI_DESCRIPTOR  PeiWinNtThunkThunk;
  EFI_PEI_PPI_DESCRIPTOR  PeiWinNtLoadAsDll;
  EFI_PEI_PPI_DESCRIPTOR  PeiStatusCode;
  EFI_PEI_PPI_DESCRIPTOR  PeiFirmwareVolumeThunk;
} PEI_WIN_NT_CALLBACK;

//
// Declare the main structure
//
PEI_WIN_NT_CALLBACK                       gPeiWinNtCallbackTable;

EFI_GUID                                 gPeiFwhGuid      = PEI_NT_FWH_PRIVATE_GUID;
EFI_PEI_PPI_DESCRIPTOR                       gPeiFwh;
PEI_NT_FWH_CALLBACK_PROTOCOL             gPeiFwhInfo;

EFI_GUID                                 gPeiNtLoadAsDllGuid = EFI_NT_LOAD_AS_DLL_PPI_GUID; 
EFI_PEI_PPI_DESCRIPTOR                       gPeiNtDllLoad;
EFI_NT_LOAD_AS_DLL_PPI                   gPeiNtLoadAsDll;                                      


EFI_GUID                                 gPeiWinNtGuid    = PEI_WIN_NT_THUNK_PRIVATE_GUID;
EFI_PEI_PPI_DESCRIPTOR                       gPeiThunk;
PEI_NT_WIN_NT_THUNK_CALLBACK_PROTOCOL     gPeiWinNtThunk;

EFI_GUID                                 gPeiSecLoadFileGuid = PEI_LOAD_FILE_PRIVATE_GUID; 
EFI_PEI_PPI_DESCRIPTOR                       gPeiLoadFile;
PEI_NT_CALLBACK_PROTOCOL                 gPeiNtLoadFile;                                      

EFI_GUID                                 gPeiSecAutoScanGuid = PEI_AUTOSCAN_PRIVATE_GUID;
EFI_PEI_PPI_DESCRIPTOR                       gPeiAutoScan;
PEI_NT_AUTOSCAN_CALLBACK_PROTOCOL        gPeiNtAutoScanService;

EFI_GUID                                 gPeiStatusCodeGuid = PEI_STATUS_CODE_PPI_GUID;
EFI_PEI_PPI_DESCRIPTOR                       gPeiStatusCode;
PEI_STATUS_CODE_PPI                      gPeiStatusCodePpi;



//
// Hob GUID for Pei Status Code callback
//
EFI_GUID  gEfiStatusCodeArchProtocolGuid = EFI_STATUS_CODE_ARCH_PROTOCOL_GUID;

//
// Pattern the stack to study the call depth nesting
//
#define DEBUG_PATTERN        0xDEADBEEF

EFI_STATUS
PrivateStackPeiLoadFile(
  VOID                  *Pe32Data,
  EFI_PHYSICAL_ADDRESS  *ImageAddress,
  UINT64                *ImageSize,
  EFI_PHYSICAL_ADDRESS  *EntryPoint
 );

EFI_STATUS
PrivateStackPeiAutoScan (
  IN OUT UINT64                *MemorySize,
  IN OUT EFI_PHYSICAL_ADDRESS  *MemoryBase
  );

EFI_STATUS
PrivateStackWinNtThunkAddress (
  IN OUT UINT64                *InterfaceSize,
  IN OUT EFI_PHYSICAL_ADDRESS  *InterfaceBase
  );

EFI_STATUS
PrivateStackWinNtFwhAddress  ( 
  IN OUT UINT64                *FwhSize,
  IN OUT EFI_PHYSICAL_ADDRESS  *FwhBase
  );

EFI_STATUS
EFIAPI
PeiReportStatusCode (
  IN EFI_PEI_SERVICES **PeiServices,
  IN EFI_STATUS_CODE_TYPE     CodeType,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 *CallerId,
  IN EFI_STATUS_CODE_DATA     *Data OPTIONAL
  );

VOID
PeiSwitchStacks (
  IN VOID                   *EntryPoint,
  IN EFI_PEI_STARTUP_DESCRIPTOR *PeiStartup,
  IN VOID                   *NewStack,
  IN VOID                   *NewBsp
  );

VOID
CopyMem (
  IN VOID   *Destination,
  IN VOID   *Source,
  IN UINTN  Length
  );

INTN
main (
  IN  INTN  Argc,
  IN  CHAR8 **Argv,
  IN  CHAR8 **Envp
  );


EFI_STATUS
WinNtOpenFile (
  CHAR16                *FileName,
  UINT32                MapSize,
  DWORD                 CreationDispostion,
  EFI_PHYSICAL_ADDRESS  *BaseAddress,
  UINT64                *Length
  );

EFI_STATUS
InstallWinNtThunkTable (
  IN  EFI_WIN_NT_THUNK_PROTOCOL  **WinNtThunkPointer
  );

EFI_STATUS
PeCoffLoaderWinNtLoadAsDll (
  IN CHAR8   *PdbFileName,
  IN VOID     **ImageEntryPoint,
  OUT VOID  **ModHandle
  );

EFI_STATUS
PeCoffLoaderWinNtFreeLibrary (
  VOID      *ModHandle
  );

VOID
PeiLoadFromCore (
  UINTN HobBase,
  UINTN BootFirmwareVolumeBase
  );

VOID
SwitchStacks (
  VOID  *EntryPoint,
  UINTN Parameter,
  VOID  *NewStack,
  VOID  *NewBsp
  );

EFI_STATUS
PeiFindFile(
  EFI_PEI_HOB_POINTERS       *Hob,
  UINT8                  Type,
  UINT16                 SectionType,
  EFI_PHYSICAL_ADDRESS   *TopOfMemory,
  EFI_PEI_HOB_POINTERS       LargestMemoryHob,
  EFI_GUID               **FileName,
  VOID                   **Pe32Data
  );

EFI_STATUS
PeiLoadFile(
  EFI_PEI_PE_COFF_LOADER_PROTOCOL          *PeiEfiPeiPeCoffLoader,
  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL *PeiEfiPeiFlushInstructionCache,
  VOID                  *Pe32Data,
  EFI_PHYSICAL_ADDRESS  *TopOfMemory,
  EFI_PEI_HOB_POINTERS      LargestMemoryHob,
  EFI_PHYSICAL_ADDRESS  *ImageAddress,
  UINT64                *ImageSize,
  EFI_PHYSICAL_ADDRESS  *EntryPoint
  );

STATIC
UINT8
GetFileState(
  IN UINT8                ErasePolarity,
  IN EFI_FFS_FILE_HEADER  *FfsHeader
  )
/*
  return the highest bit set of the state field
*/
{
  EFI_FFS_FILE_STATE           FileState;
  EFI_FFS_FILE_STATE           HighestBit;

  FileState = FfsHeader->State;

  if (ErasePolarity != 0) {
    FileState = (EFI_FFS_FILE_STATE)~FileState;
  }

  HighestBit = 0x80;
  while (HighestBit != 0 && (HighestBit & FileState) == 0) {
    HighestBit >>= 1;
  }

  return HighestBit;
}  

BOOLEAN
IsBufferErased (
  IN UINT8    ErasePolarity,
  IN UINT8    *Buffer,
  IN UINT32   BufferSize
  )
/*
  Tell if a block of buffer is erased
*/
{
  UINTN Count;
  UINT8 EraseByte;

  if(ErasePolarity == 1) {
    EraseByte = 0xFF;
  }
  else {
    EraseByte = 0;
  }

  for (Count = 0; Count < BufferSize; Count++) {
    if (Buffer[Count] != EraseByte) {
      return FALSE;
    }
  }

  return TRUE;
}

STATIC
UINT32
GetOccupiedSize (
  IN UINT32  ActualSize,
  IN UINT32  Alignment
  )
{
  UINT32  OccupiedSize;
  
  OccupiedSize = ActualSize;
  while ((OccupiedSize & (Alignment - 1)) != 0) {
    OccupiedSize++;
  }
  
  return OccupiedSize;
}

//
// Interface and GUID for the WinNt Thunk Protocol
//
EFI_WIN_NT_THUNK_PROTOCOL  *gWinNt;
EFI_WIN_NT_THUNK_PROTOCOL  mWinNtThunkTable;
EFI_GUID                  gEfiWinNtThunkProtocolGuid = EFI_WIN_NT_THUNK_PROTOCOL_GUID;


//
// Interface and GUID for the Instruction Cache Flushing APIs shared between PEI and DXE
//
EFI_GUID  mPeiEfiPeiFlushInstructionCacheGuid = EFI_PEI_FLUSH_INSTRUCTION_CACHE_GUID;

//
// Interface and GUID for the PE/COFF Loader APIs shared between PEI and DXE
//
EFI_GUID  mPeiEfiPeiPeCoffLoaderGuid = EFI_PEI_PE_COFF_LOADER_GUID;

//
// Interface and GUID for the setjump()/longjump() APIs shared between PEI and DXE
//
EFI_GUID  mPeiEfiPeiTransferControlGuid = EFI_PEI_TRANSFER_CONTROL_GUID;

//
// GUID for the Firmware Volume type that PEI supports
//
EFI_GUID  mPeiFwFileSysTypeGuid = EFI_FIRMWARE_FILE_SYSTEM_GUID;

INTN
main (
  IN  INTN  Argc,
  IN  CHAR8 **Argv,
  IN  CHAR8 **Envp
  )
/*++

Routine Description:

  Main entry point to SEC for WinNt

Arguments:

  Argc - Number of command line arguments

  Argv - Array of command line argument strings

  Envp - Array of environmemt variable strings

Returns:

  0 - Normal exit
  1 - Abnormal exit

--*/
{
  EFI_STATUS                            Status;
  EFI_PHYSICAL_ADDRESS                  EfiEmulatorMemory;
  UINT64                                EfiEmulatorMemorySize;
  EFI_PHYSICAL_ADDRESS                  EfiEmulatorBootFirmwareDevice;
  UINT64                                EfiEmulatorBootFirmwareDeviceSize;
  VOID                                  *HobStart;
  EFI_BOOT_MODE                         BootMode;
  EFI_PEI_HOB_POINTERS                      Hob;
  UINTN                                 Index;
  CHAR8                                 *MemorySizeStr;
  CHAR8                                 *FirmwareVolumesStr;
  CHAR8                                 *BootModeStr;
  CHAR16                                FileName[100];
  UINTN                                 CoreFileIndex;
  BOOLEAN                               Done;

  EfiEmulatorBootFirmwareDevice = 0;  // make the compiler happy

  printf ("SEC Main NT Emulation Environment 0.5");

  //
  // Parse Envp for EFI_MEMORY_SIZE and EFI_FIRMWARE_VOLUMES
  //
  MemorySizeStr      = "";
  FirmwareVolumesStr = "";
  BootModeStr        = "";


  for (Index = 0; Envp[Index] != NULL; Index ++) {

    if (strncmp (Envp[Index], EFI_MEMORY_SIZE_STR, sizeof (EFI_MEMORY_SIZE_STR)-1) == 0) {
      MemorySizeStr = &Envp[Index][sizeof (EFI_MEMORY_SIZE_STR)];
    }

    if (strncmp (Envp[Index], EFI_FIRMWARE_VOLUMES_STR, sizeof (EFI_FIRMWARE_VOLUMES_STR)-1) == 0) {
      FirmwareVolumesStr = &Envp[Index][sizeof (EFI_FIRMWARE_VOLUMES_STR)];
    }

    if (strncmp (Envp[Index], EFI_BOOT_MODE_STR, sizeof (EFI_BOOT_MODE_STR)-1) == 0) {
      BootModeStr = &Envp[Index][sizeof (EFI_BOOT_MODE_STR)];
    }

  }

  //
  // Build Protocol Instance for all the NT services used by PEI and DXE
  //
  Status = InstallWinNtThunkTable (&gWinNt);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Set the process to the highest priority in the idle class
  //
  gWinNt->SetPriorityClass  (gWinNt->GetCurrentProcess (), IDLE_PRIORITY_CLASS);
  gWinNt->SetThreadPriority (gWinNt->GetCurrentThread (),  THREAD_PRIORITY_HIGHEST);


  //
  // Setup Boot Mode. If BootModeStr == "" then BootMode = 0 (BOOT_WITH_FULL_CONFIGURATION)
  //
  BootMode = atoi (BootModeStr);
  printf ("  BootMode 0x%02x\n\n", BootMode);

  //
  // Build HOBs for all emulated memory regions
  // Only use these HOB's for this transient SEC preparation
  //
  HobStart = NULL;
  Hob.Raw = HobStart;
  CoreFileIndex = 0;
  Done = FALSE;
  while (!Done) {

    EfiEmulatorMemorySize = atoi(MemorySizeStr) * 0x100000;
    for (Index = 0; MemorySizeStr[Index] != '!' && MemorySizeStr[Index] != 0; Index++);
    if (MemorySizeStr[Index] == 0) {
      Done = TRUE;
    } else {
      MemorySizeStr = MemorySizeStr + Index + 1;
    }

    swprintf (FileName, L"CoreFile%d",CoreFileIndex);

    Status = WinNtOpenFile (
               FileName,
               (UINT32)EfiEmulatorMemorySize,
               OPEN_ALWAYS,
               &EfiEmulatorMemory,
               &EfiEmulatorMemorySize
               );

    if (EFI_ERROR (Status)) {
      printf ("ERROR : Can not open Memory file %S (%r).  Exiting.\n", FileName, Status);
      exit (1);
    }

    if (CoreFileIndex == 0) {
      //
      // Build Hobs at the begining of memory
      //
      HobStart = (VOID *)((UINTN)EfiEmulatorMemory);
      BuildHobHandoffInfoTable (
              HobStart,
              1,
              BootMode,
              EfiEmulatorMemory + EfiEmulatorMemorySize,
              EfiEmulatorMemory,
              EfiEmulatorMemory + EfiEmulatorMemorySize,
              EfiEmulatorMemory
              );
    }

    //
    // Make a Hob entry for all the emulated memory.
    //
    BuildHobResourceDescriptor (
            HobStart,
            EFI_RESOURCE_SYSTEM_MEMORY,
            EFI_RESOURCE_ATTRIBUTE_PRESENT,
            EfiEmulatorMemory,
            EfiEmulatorMemorySize
            );

    CoreFileIndex++;
  }

  //
  // Build HOBs for all Firmware Volumes
  //
  Done = FALSE;
  while (!Done) {

    for (Index = 0; FirmwareVolumesStr[Index] != '!' && FirmwareVolumesStr[Index] != 0; Index++) {
      FileName[Index] = FirmwareVolumesStr[Index];
    }
    FileName[Index] = 0;

    if (FirmwareVolumesStr[Index] == 0) {
      Done = TRUE;
    } else {
      FirmwareVolumesStr = FirmwareVolumesStr + Index + 1;
    }

    //
    // Setup Boot Firmware Device Emulation
    //
    if (FileName[0] != 0) {
      Status = WinNtOpenFile (
                 FileName,
                 0,
                 OPEN_EXISTING,
                 &EfiEmulatorBootFirmwareDevice,
                 &EfiEmulatorBootFirmwareDeviceSize
                 );

      if (EFI_ERROR (Status)) {
        printf ("ERROR : Can not open Firmware Volume file %S (%r).  Exiting.\n", FileName, Status);
        exit (1);
      }

      EfiEmulatorBootFirmwareDevice += EFI_WINNT_FIRMWARE_OFFSET;
      EfiEmulatorBootFirmwareDeviceSize = EFI_WINNT_FIRMWARE_LENGTH;
      //
      // Make a Hob entry for the Boot Firmware Device
      //
      BuildHobResourceDescriptor (
              HobStart,
              EFI_RESOURCE_FIRMWARE_DEVICE,
              EFI_RESOURCE_ATTRIBUTE_PRESENT,
              EfiEmulatorBootFirmwareDevice,
              EfiEmulatorBootFirmwareDeviceSize
              );

      //
      // Make a Hob entry for the Boot Firmware Volume
      //
      BuildHobFvDescriptor (
              HobStart,
              EfiEmulatorBootFirmwareDevice,
              EfiEmulatorBootFirmwareDeviceSize
              );

      gEfiEmulatorBootFirmwareDevice = EfiEmulatorBootFirmwareDevice;
      gEfiEmulatorBootFirmwareDeviceSize = EfiEmulatorBootFirmwareDeviceSize;
    }
  }



//  //
//  // Add Hob for Pei Status Code callback
//  //
//  Interface = (VOID *)(UINTN)PeiReportStatusCode;
//  BuildHobGuidType (
//          HobStart,
//          &gEfiStatusCodeArchProtocolGuid,
//          &Interface,
//          sizeof (VOID *)
//          );
//
//
//  //
//  // Add HOB for the WinNT Thunk Protocol
//  //
//  Interface = (VOID *)&mWinNtThunkTable;
//  BuildHobGuidType (
//          HobStart,
//          &gEfiWinNtThunkProtocolGuid,
//          &Interface,
//          sizeof (VOID *)
//          );

  //
  // Hand off to standard DXE IPL PEIM
  //
  PeiLoadFromCore ((UINTN)HobStart, (UINTN) EfiEmulatorBootFirmwareDevice);

  //
  // If we get here, then the DXE Core returned.  This is an error
  //
  printf ("ERROR : PEI Core returned\n");
  exit (1);
}

EFI_STATUS
WinNtOpenFile (
  CHAR16                *FileName,
  UINT32                MapSize,
  DWORD                 CreationDisposition,
  EFI_PHYSICAL_ADDRESS  *BaseAddress,
  UINT64                *Length
  )
/*++

Routine Description:

  Opens and memory maps a file using WinNt services

Arguments:

  FileName            - The name of the file to open and map

  MapSize             - The amount of the file to map in bytes

  CreationDisposition - The flags to pass to CreateFile().  Use to create new files for
                        memory emulation, and exiting files for firmware volume emulation

  BaseAddress         - The base address of the mapped file in the user address space

  Length              - The size of the mapped region in bytes

Returns:

  EFI_SUCCESS      - The file was opened and mapped.

  EFI_NOT_FOUND    - FileName was not found in the current directory

  EFI_DEVICE_ERROR - An error occured attempting to map the opened file

--*/
{
  HANDLE            NtFileHandle;
  HANDLE            NtMapHandle;
  VOID              *VirtualAddress;
  UINTN             FileSize;

  //
  // Setup Boot Firmware Device Emulation
  //
  NtFileHandle = gWinNt->CreateFile (
                           FileName,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           CreationDisposition,
                           0,
                           NULL
                           );

  if (NtFileHandle == INVALID_HANDLE_VALUE) {
    return EFI_NOT_FOUND;
  }

  NtMapHandle = gWinNt->CreateFileMapping (
                          NtFileHandle,
                          NULL,
                          PAGE_READWRITE,
                          0,
                          MapSize,
                          NULL
                          );

  if (NtMapHandle == NULL) {
    return EFI_DEVICE_ERROR;
  }

  VirtualAddress = gWinNt->MapViewOfFileEx (
                             NtMapHandle,
                             FILE_MAP_ALL_ACCESS,
                             0,
                             0,
                             MapSize,
                             NULL
                             );

  if (VirtualAddress == NULL) {
    return EFI_DEVICE_ERROR;
  }

  FileSize = gWinNt->SetFilePointer (
                       NtFileHandle,
                       0,
                       NULL,
                       FILE_END
                       );

  if (FileSize == -1) {
    return EFI_DEVICE_ERROR;
  }

  *BaseAddress = (EFI_PHYSICAL_ADDRESS)VirtualAddress;
  *Length      = (UINT64)FileSize;

  return EFI_SUCCESS;
}

EFI_STATUS
InstallWinNtThunkTable (
  IN  EFI_WIN_NT_THUNK_PROTOCOL  **WinNtThunkPointer
  )
/*++

Routine Description:

  Builds the WinNt Thunk Protocol Interface

Arguments:

  WinNtThunkPointer - Pointer to the WinNt Thunk Protocol Interface

Returns:

  EFI_SUCCESS      - The file was opened and mapped.

--*/
{
  //
  // Initialize table header
  //
  mWinNtThunkTable.Signature = EFI_WIN_NT_THUNK_PROTOCOL_SIGNATURE;

  //
  // Win32 Process APIs
  //
  mWinNtThunkTable.GetCurrentProcess                = GetCurrentProcess;
  mWinNtThunkTable.GetCurrentThread                 = GetCurrentThread;
  mWinNtThunkTable.GetCurrentThreadId               = GetCurrentThreadId;
  mWinNtThunkTable.GetProcAddress                   = GetProcAddress;
  mWinNtThunkTable.GetTickCount                     = GetTickCount;
  mWinNtThunkTable.LoadLibraryEx                    = LoadLibraryEx;
  mWinNtThunkTable.FreeLibrary                       = FreeLibrary;

  mWinNtThunkTable.SuspendThread                    = SuspendThread;
  mWinNtThunkTable.CreateThread                     = CreateThread;
  mWinNtThunkTable.TerminateThread                  = TerminateThread;
  mWinNtThunkTable.SendMessage                      = SendMessage;
  mWinNtThunkTable.ExitThread                       = ExitThread;
  mWinNtThunkTable.ResumeThread                     = ResumeThread;
  mWinNtThunkTable.DuplicateHandle                  = DuplicateHandle;

  mWinNtThunkTable.SetPriorityClass                 = SetPriorityClass;
  mWinNtThunkTable.SetThreadPriority                = SetThreadPriority;
  mWinNtThunkTable.Sleep                            = Sleep;

  mWinNtThunkTable.InitializeCriticalSection        = InitializeCriticalSection;
  mWinNtThunkTable.EnterCriticalSection             = EnterCriticalSection;
  mWinNtThunkTable.LeaveCriticalSection             = LeaveCriticalSection;
  mWinNtThunkTable.DeleteCriticalSection            = DeleteCriticalSection;
  mWinNtThunkTable.TlsAlloc                         = TlsAlloc;
  mWinNtThunkTable.TlsFree                          = TlsFree;
  mWinNtThunkTable.TlsSetValue                      = TlsSetValue;
  mWinNtThunkTable.TlsGetValue                      = TlsGetValue;
  mWinNtThunkTable.CreateSemaphore                  = CreateSemaphore;
  mWinNtThunkTable.WaitForSingleObject              = WaitForSingleObject;
  mWinNtThunkTable.ReleaseSemaphore                 = ReleaseSemaphore;


  //
  // Win32 Console APIs
  //
  mWinNtThunkTable.CreateConsoleScreenBuffer        = CreateConsoleScreenBuffer;
  mWinNtThunkTable.FillConsoleOutputAttribute       = FillConsoleOutputAttribute;
  mWinNtThunkTable.FillConsoleOutputCharacter       = FillConsoleOutputCharacter;
  mWinNtThunkTable.GetConsoleCursorInfo             = GetConsoleCursorInfo;
  mWinNtThunkTable.GetNumberOfConsoleInputEvents    = GetNumberOfConsoleInputEvents;
  mWinNtThunkTable.PeekConsoleInput                 = PeekConsoleInput;
  mWinNtThunkTable.ScrollConsoleScreenBuffer        = ScrollConsoleScreenBuffer;
  mWinNtThunkTable.ReadConsoleInput                 = ReadConsoleInput;

  mWinNtThunkTable.SetConsoleActiveScreenBuffer     = SetConsoleActiveScreenBuffer;
  mWinNtThunkTable.SetConsoleCursorInfo             = SetConsoleCursorInfo;
  mWinNtThunkTable.SetConsoleCursorPosition         = SetConsoleCursorPosition;
  mWinNtThunkTable.SetConsoleScreenBufferSize       = SetConsoleScreenBufferSize;
  mWinNtThunkTable.SetConsoleTitleW                 = SetConsoleTitleW;
  mWinNtThunkTable.WriteConsoleInput                = WriteConsoleInput;
  mWinNtThunkTable.WriteConsoleOutput               = WriteConsoleOutput;

  //
  // Win32 File APIs
  //
  mWinNtThunkTable.CreateFile                       = CreateFile;
  mWinNtThunkTable.DeviceIoControl                  = DeviceIoControl;
  mWinNtThunkTable.CreateDirectory                  = CreateDirectory;
  mWinNtThunkTable.RemoveDirectory                  = RemoveDirectory;
  mWinNtThunkTable.GetFileAttributes                = GetFileAttributes;
  mWinNtThunkTable.SetFileAttributes                = SetFileAttributes;
  mWinNtThunkTable.CreateFileMapping                = CreateFileMapping;
  mWinNtThunkTable.CloseHandle                      = CloseHandle;
  mWinNtThunkTable.DeleteFile                       = DeleteFile;
  mWinNtThunkTable.FindFirstFile                    = FindFirstFile;
  mWinNtThunkTable.FindNextFile                     = FindNextFile;
  mWinNtThunkTable.FindClose                        = FindClose;
  mWinNtThunkTable.FlushFileBuffers                 = FlushFileBuffers;
  mWinNtThunkTable.GetEnvironmentVariable           = GetEnvironmentVariable;
  mWinNtThunkTable.GetLastError                     = GetLastError;
  mWinNtThunkTable.SetErrorMode                     = SetErrorMode;
  mWinNtThunkTable.GetStdHandle                     = GetStdHandle;
  mWinNtThunkTable.MapViewOfFileEx                  = MapViewOfFileEx;
  mWinNtThunkTable.ReadFile                         = ReadFile;
  mWinNtThunkTable.SetEndOfFile                     = SetEndOfFile;
  mWinNtThunkTable.SetFilePointer                   = SetFilePointer;
  mWinNtThunkTable.WriteFile                        = WriteFile;
  mWinNtThunkTable.GetFileInformationByHandle       = GetFileInformationByHandle;
  mWinNtThunkTable.GetDiskFreeSpace                 = GetDiskFreeSpace;
  mWinNtThunkTable.GetDiskFreeSpaceEx               = GetDiskFreeSpaceEx;
  mWinNtThunkTable.MoveFile                         = MoveFile;
  mWinNtThunkTable.SetFileTime                      = SetFileTime;
  mWinNtThunkTable.SystemTimeToFileTime             = SystemTimeToFileTime;

  //
  // Win32 Time APIs
  //
  mWinNtThunkTable.FileTimeToLocalFileTime          = FileTimeToLocalFileTime;
  mWinNtThunkTable.FileTimeToSystemTime             = FileTimeToSystemTime;
  mWinNtThunkTable.GetSystemTime                    = GetSystemTime;
  mWinNtThunkTable.SetSystemTime                    = SetSystemTime;
  mWinNtThunkTable.GetLocalTime                     = GetLocalTime;
  mWinNtThunkTable.SetLocalTime                     = SetLocalTime;
  mWinNtThunkTable.GetTimeZoneInformation           = GetTimeZoneInformation;
  mWinNtThunkTable.SetTimeZoneInformation           = SetTimeZoneInformation;
  mWinNtThunkTable.timeSetEvent                     = timeSetEvent;
  mWinNtThunkTable.timeKillEvent                    = timeKillEvent;

  //
  // Win32 Serial APIs
  //
  mWinNtThunkTable.ClearCommError                   = ClearCommError;
  mWinNtThunkTable.EscapeCommFunction               = EscapeCommFunction;
  mWinNtThunkTable.GetCommModemStatus               = GetCommModemStatus;
  mWinNtThunkTable.GetCommState                     = GetCommState;
  mWinNtThunkTable.SetCommState                     = SetCommState;
  mWinNtThunkTable.PurgeComm                        = PurgeComm;
  mWinNtThunkTable.SetCommTimeouts                  = SetCommTimeouts;

  mWinNtThunkTable.ExitProcess                      = ExitProcess;

  mWinNtThunkTable.SPrintf                          = swprintf;
  
  mWinNtThunkTable.GetDesktopWindow                 = GetDesktopWindow;
  mWinNtThunkTable.GetForegroundWindow              = GetForegroundWindow;
  mWinNtThunkTable.CreateWindowEx                   = CreateWindowEx;
  mWinNtThunkTable.ShowWindow                       = ShowWindow;
  mWinNtThunkTable.UpdateWindow                     = UpdateWindow;
  mWinNtThunkTable.InvalidateRect                   = InvalidateRect;
  mWinNtThunkTable.DestroyWindow                    = DestroyWindow;
  mWinNtThunkTable.GetWindowDC                      = GetWindowDC;
  mWinNtThunkTable.InvalidateRect                   = InvalidateRect;
  mWinNtThunkTable.GetClientRect                    = GetClientRect;
  mWinNtThunkTable.AdjustWindowRect                 = AdjustWindowRect;
  mWinNtThunkTable.SetDIBitsToDevice                = SetDIBitsToDevice;
  mWinNtThunkTable.BitBlt                           = BitBlt;
  mWinNtThunkTable.GetDC                            = GetDC;
  mWinNtThunkTable.ReleaseDC                        = ReleaseDC;
  mWinNtThunkTable.RegisterClassEx                  = RegisterClassEx;
  mWinNtThunkTable.UnregisterClass                  = UnregisterClass;

  mWinNtThunkTable.BeginPaint                       = BeginPaint;
  mWinNtThunkTable.EndPaint                         = EndPaint;
  mWinNtThunkTable.PostQuitMessage                  = PostQuitMessage;
  mWinNtThunkTable.DefWindowProc                    = DefWindowProc;
  mWinNtThunkTable.LoadIcon                         = LoadIcon;
  mWinNtThunkTable.LoadCursor                       = LoadCursor;
  mWinNtThunkTable.GetStockObject                   = GetStockObject;
  mWinNtThunkTable.SetViewportOrgEx                 = SetViewportOrgEx;
  mWinNtThunkTable.SetWindowOrgEx                   = SetWindowOrgEx;
  mWinNtThunkTable.MoveWindow                       = MoveWindow;
  mWinNtThunkTable.GetWindowRect                    = GetWindowRect;

  mWinNtThunkTable.GetMessage                       = GetMessage;
  mWinNtThunkTable.TranslateMessage                 = TranslateMessage;
  mWinNtThunkTable.DispatchMessage                  = DispatchMessage;

  mWinNtThunkTable.GetProcessHeap                   = GetProcessHeap;
  mWinNtThunkTable.HeapAlloc                        = HeapAlloc;
  mWinNtThunkTable.HeapFree                         = HeapFree;
 

  //
  // Set up the global
  //
  *WinNtThunkPointer = &mWinNtThunkTable;

  return EFI_SUCCESS;
}

#define BYTES_PER_RECORD  512


EFI_STATUS
EFIAPI
PeiReportStatusCode (
  IN EFI_PEI_SERVICES   **PeiServices,
  IN EFI_STATUS_CODE_TYPE       CodeType,
  IN EFI_STATUS_CODE_VALUE      Value,
  IN UINT32                     Instance,
  IN EFI_GUID                   *CallerId,
  IN EFI_STATUS_CODE_DATA       *Data OPTIONAL
  )
/*++

Routine Description:

  This routine produces the EFI 2.0 BootService ReportStatusCode. It's passed 
  up to the DxeCore via a GUIDed HOB. The DxeCore uses this function until it's
  StatusCode AP is loaded. The StatusCode AP may use this code.

  This code currently uses the NT clib printf. This does not work the same way 
  as the EFI Print (), as %t, %g, %s as Unicode are not supported.

Arguments:
  (see EFI_REPORT_STATUS_CODE)

Returns:

  EFI_SUCCESS - Always return success

--*/
{
  CHAR8             *Format;
  EFI_DEBUG_INFO    *DebugInfo;
  VA_LIST           Marker;
  CHAR8             PrintBuffer[BYTES_PER_RECORD*2];
  CHAR8             *Filename;
  CHAR8             *Description;
  UINT32            LineNumber;

  if ((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_DEBUG_CODE) {
    //
    // Data format
    //
    //  EFI_STATUS_CODE_DATA
    //  EFI_DEBUG_INFO
    //
    // The first 12 * UINT64 bytes of the string are really an 
    // arguement stack to support varargs on the Format string.
    //
    DebugInfo = (EFI_DEBUG_INFO *)(Data + 1);
    Marker = (VA_LIST) (DebugInfo + 1);
    Format = (CHAR8 *)(((UINT64 *)Marker) + 12);

    AvSPrint (PrintBuffer, BYTES_PER_RECORD, Format, Marker);
    printf (PrintBuffer);
  }

  if (((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_ERROR_CODE) && 
      ((CodeType & EFI_STATUS_CODE_SEVERITY_MASK) == EFI_ERROR_UNCONTAINED)) {
    //
    // Assume if we have an uncontained unrecoverable error that the data hub
    // may not work. So we will print out data here. If we had an IPMI controller,
    // or error log we could wack the hardware here.
    //
    if (ReportStatusCodeExtractAssertInfo (CodeType, Value, Data, &Filename, &Description, &LineNumber)) {
      printf ("ASSERT %s(%d): %s\n", Filename, LineNumber, Description);
      EFI_BREAKPOINT ();
    } 
  }

  return EFI_SUCCESS;
}




EFI_STATUS
PeCoffLoaderWinNtLoadAsDll (
  IN CHAR8   *PdbFileName,
  IN VOID     **ImageEntryPoint,
  OUT VOID  **ModHandle
  )
/*++

Routine Description:

  Loads the .DLL file is present when a PE/COFF file is loaded.  This provides source level
  debugging for drivers that have cooresponding .DLL files on the local system.

Arguments:

  PdbFileName     - The name of the .PDB file.  This was found from the PE/COFF
                    file's debug directory entry.

  ImageEntryPoint - A pointer to the DLL entry point of the .DLL file was loaded.

Returns:

  EFI_SUCCESS     - The .DLL file was loaded, and the DLL entry point is returned in ImageEntryPoint

  EFI_NOT_FOUND   - The .DLL file could not be found

  EFI_UNSUPPORTED - The .DLL file was loaded, but the entry point to the .DLL file could not
                    determined.

--*/
{
  CHAR16      DllFileName[MAX_FILE_NAME_LENGTH];
  HMODULE     Library;
  UINTN       Index;

  *ImageEntryPoint = NULL;
  *ModHandle = NULL;

  //
  // Convert filename from ASCII to Unicode
  //
  for (Index = 0;Index < MAX_FILE_NAME_LENGTH && PdbFileName[Index] != 0; Index++) {
    DllFileName[Index] = PdbFileName[Index];
  }
  //BugBug - could set beyond MAX_FILE_NAME_LENGTH!
  DllFileName[Index] = 0;

  //
  // Check that we have a valid filename
  //
  if (Index < 5 ||
      Index >= MAX_FILE_NAME_LENGTH ||
      DllFileName[Index - 4] != '.') {
    return EFI_NOT_FOUND;
  }

  //
  // Replace .PDB with .DLL on the filename
  //
  DllFileName[Index - 3] = 'D';
  DllFileName[Index - 2] = 'L';
  DllFileName[Index - 1] = 'L';

  //
  // Load the .DLL file into the user process's address space
  //
  Library = gWinNt->LoadLibraryEx (
                      DllFileName,
                      NULL,
                      DONT_RESOLVE_DLL_REFERENCES
                      );

  if (Library == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // InitializeDriver is the entry point we put in all our EFI DLL's. The
  // DONT_RESOLVE_DLL_REFERENCES argument to LoadLIbraryEx() supresses the normal
  // DLL entry point of DllMain, and prevents other modules that are referenced
  // in side the DllFileName from being loaded.
  //
  *ImageEntryPoint = (VOID *)(UINTN)gWinNt->GetProcAddress (
                                              Library,
                                              "InitializeDriver"
                                              );

  if (*ImageEntryPoint == NULL) {
    return EFI_UNSUPPORTED;
  }

  *ModHandle = Library;

  return EFI_SUCCESS;
}

EFI_STATUS 
PeCoffLoaderWinNtFreeLibrary (
  VOID      *ModHandle
  )
{
  BOOL    Bool;

  Bool = FreeLibrary ((HANDLE)ModHandle); 
  
  return EFI_SUCCESS;
}

VOID
PeiLoadFromCore (
  UINTN HobBase,
  UINTN BootFirmwareVolumeBase
  )
/*++

Routine Description:

  This is the service to load the PEI Core from the Firmware Volume

Arguments:


Returns:

  0 - Normal exit
  1 - Abnormal exit

--*/
{
  EFI_STATUS                                Status;
  VOID                                      *HobStart;
  EFI_PEI_HOB_POINTERS                          Hob;
  EFI_PEI_HOB_POINTERS                          FirstHob;
  EFI_PEI_HOB_POINTERS                          LargestMemoryHob;
  EFI_PEI_HOB_POINTERS                          StackHob;
  EFI_PEI_HOB_POINTERS                          BspStoreHob;
//  EFI_PEI_HOB_POINTERS                          PeiCoreHob;
  EFI_PEI_HOB_POINTERS                          SearchHob;
  EFI_PHYSICAL_ADDRESS                      TopOfMemory;
  VOID                                      *TopOfStack;
//  VOID                                      *BspStore;
  EFI_GUID                                  *PeiCoreFileName;
  VOID                                      *Pe32Data;
  VOID                                      *Interface;
  EFI_PHYSICAL_ADDRESS                      PeiCoreAddress;
  UINT64                                    PeiCoreSize;
  EFI_PHYSICAL_ADDRESS                      PeiCoreEntryPoint;
  BOOLEAN                                   LargestHobValid;
  BOOLEAN                                   StackHobValid;
  BOOLEAN                                   BspStoreHobValid;
  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  *PeiEfiPeiFlushInstructionCache;
  EFI_PEI_PE_COFF_LOADER_PROTOCOL           *PeiEfiPeiPeCoffLoader;
  EFI_PEI_TRANSFER_CONTROL_PROTOCOL         *PeiEfiPeiTransferControl;
//UINT32                                    *pt;
//UINTN                                     Index;
  EFI_PEI_STARTUP_DESCRIPTOR                    PeiStartup;

  //
  // Install the PEI Protocols that are shared between PEI and DXE
  //
  PeiEfiPeiPeCoffLoader = NULL;
  InstallEfiPeiFlushInstructionCache (&PeiEfiPeiFlushInstructionCache);
  InstallEfiPeiTransferControl       (&PeiEfiPeiTransferControl);

  //
  // Build HOBs for all memory regions
  //
  HobStart             = (VOID *) HobBase;
  FirstHob.Raw         = HobStart;
  Hob.Raw              = HobStart;
  StackHob.Raw         = HobStart;
  BspStoreHob.Raw      = HobStart;
  LargestMemoryHob.Raw = HobStart;

  LargestHobValid   = FALSE;
  StackHobValid     = FALSE;
  BspStoreHobValid  = FALSE;
  while (Hob.Header->HobType != EFI_HOB_TYPE_END_OF_HOB_LIST) {
    if (Hob.Header->HobType == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR &&
        Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY &&
        (Hob.ResourceDescriptor->ResourceAttribute & EFI_RESOURCE_ATTRIBUTE_PRESENT)) {
      if (!LargestHobValid) {
        LargestMemoryHob.Raw = Hob.Raw;
        LargestHobValid      = TRUE;
      } else if (Hob.ResourceDescriptor->ResourceLength > LargestMemoryHob.ResourceDescriptor->ResourceLength) {
        LargestMemoryHob.Raw = Hob.Raw;
      }
    }
    if (Hob.Header->HobType == EFI_HOB_TYPE_GUID_EXTENSION &&
        CompareGuid ( &Hob.Guid->Name, &mPeiEfiPeiPeCoffLoaderGuid)) {
      CopyMem(&PeiEfiPeiPeCoffLoader, Hob.Raw + sizeof(EFI_HOB_GUID_TYPE), sizeof (VOID *));
    }
    Hob.Raw += Hob.Header->HobLength;
  }

  if (PeiEfiPeiPeCoffLoader == NULL) {
      InstallEfiPeiPeCoffLoader          (NULL, &PeiEfiPeiPeCoffLoader, (EFI_PEI_PPI_DESCRIPTOR*)&gPeiNtLoadAsDll);
  }

  if (!LargestHobValid) {
    return;
  }

  //
  // Compute Top Of Memory for Stack and PEI Core Allocations
  //
  TopOfMemory = (LargestMemoryHob.ResourceDescriptor->PhysicalStart + LargestMemoryHob.ResourceDescriptor->ResourceLength) & (~15);

  //
  // Allocate 128KB for the Stack
  //
  TopOfStack = (VOID *)(UINTN)(TopOfMemory - 0x10);
  TopOfStack = (VOID *)(UINTN)(TopOfMemory - sizeof (EFI_PEI_STARTUP_DESCRIPTOR));
  TopOfMemory = TopOfMemory - EFI_STACK_SIZE;


  gPeiNtLoadFile.PeiLoadFileService = PrivateStackPeiLoadFile; 
  gPeiLoadFile.Guid   = &gPeiSecLoadFileGuid;
  gPeiLoadFile.Flags  = (EFI_PEI_PPI_DESCRIPTOR_PPI);
  gPeiLoadFile.Ppi    = &gPeiNtLoadFile;

  gPeiNtAutoScanService.NtAutoScan = PrivateStackPeiAutoScan;
  gPeiAutoScan.Guid   =  &gPeiSecAutoScanGuid;
  gPeiAutoScan.Flags  = (EFI_PEI_PPI_DESCRIPTOR_PPI);
  gPeiAutoScan.Ppi    = &gPeiNtAutoScanService;

  gPeiWinNtThunk.NtThunk  = PrivateStackWinNtThunkAddress;
  gPeiThunk.Guid      = &gPeiWinNtGuid;
  gPeiThunk.Flags     = (EFI_PEI_PPI_DESCRIPTOR_PPI);
  gPeiThunk.Ppi       = &gPeiWinNtThunk;

  gPeiNtLoadAsDll.Entry     = PeCoffLoaderWinNtLoadAsDll;
  gPeiNtLoadAsDll.FreeLibrary = PeCoffLoaderWinNtFreeLibrary;
  gPeiNtDllLoad.Guid        = &gPeiNtLoadAsDllGuid;
  gPeiNtDllLoad.Flags       = (EFI_PEI_PPI_DESCRIPTOR_PPI);
  gPeiNtDllLoad.Ppi         = &gPeiNtLoadAsDll;

  gPeiStatusCodePpi.ReportStatusCode   = PeiReportStatusCode; 
  gPeiStatusCode.Guid         = &gPeiStatusCodeGuid;
  gPeiStatusCode.Flags        = (EFI_PEI_PPI_DESCRIPTOR_PPI);
  gPeiStatusCode.Ppi          = &gPeiStatusCodePpi;


  gPeiFwhInfo.NtFwh    = PrivateStackWinNtFwhAddress; 
  gPeiFwh.Guid         = &gPeiFwhGuid;
  gPeiFwh.Flags        = (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
  gPeiFwh.Ppi          = &gPeiFwhInfo;

  //
  // Build the thunk table w/ our PPI descriptor
  //
  CopyMem(&(gPeiWinNtCallbackTable.PeiLoadFileServiceThunk), &gPeiLoadFile,  sizeof(EFI_PEI_PPI_DESCRIPTOR));
  CopyMem(&(gPeiWinNtCallbackTable.PeiAutoscanServiceThunk), &gPeiAutoScan,  sizeof(EFI_PEI_PPI_DESCRIPTOR));
  CopyMem(&(gPeiWinNtCallbackTable.PeiWinNtThunkThunk),      &gPeiThunk,     sizeof(EFI_PEI_PPI_DESCRIPTOR));
  CopyMem(&(gPeiWinNtCallbackTable.PeiWinNtLoadAsDll),       &gPeiNtDllLoad, sizeof(EFI_PEI_PPI_DESCRIPTOR));
  CopyMem(&(gPeiWinNtCallbackTable.PeiStatusCode),           &gPeiStatusCode,sizeof(EFI_PEI_PPI_DESCRIPTOR));
  CopyMem(&(gPeiWinNtCallbackTable.PeiFirmwareVolumeThunk),  &gPeiFwh,       sizeof(EFI_PEI_PPI_DESCRIPTOR));

  //
  // Bind this information into the SEC hand-off state
  // 
  PeiStartup.DispatchTable = (EFI_PEI_PPI_DESCRIPTOR*) &gPeiWinNtCallbackTable; 

  PeiStartup.SizeOfCacheAsRam     = EFI_STACK_SIZE;
  PeiStartup.BootFirmwareVolume   = BootFirmwareVolumeBase;
  CopyMem (TopOfStack, &PeiStartup, sizeof(EFI_PEI_STARTUP_DESCRIPTOR));


  //
  // Put a recognizable pattern on the stack for debug
  // 
  if (!StackHobValid) {
    BuildHobStack (HobStart, TopOfMemory, EFI_STACK_SIZE);
  } else {
    StackHob.MemoryAllocationStack->AllocDescriptor.MemoryBaseAddress = TopOfMemory;
    StackHob.MemoryAllocationStack->AllocDescriptor.MemoryLength      = EFI_STACK_SIZE;
  }

  //
  // Add HOB for the Flush Instruction Cache Protocol
  //
  Interface = (VOID *)PeiEfiPeiFlushInstructionCache;
  BuildHobGuidType (
          HobStart,
          &mPeiEfiPeiFlushInstructionCacheGuid,
          &Interface,
          sizeof(VOID *)
          );

  //
  // Add HOB for the PE/COFF Loader Protocol
  //
  Interface = (VOID *)PeiEfiPeiPeCoffLoader;
  BuildHobGuidType (
          HobStart,
          &mPeiEfiPeiPeCoffLoaderGuid,
          &Interface,
          sizeof(VOID *)
          );

  //
  // Add HOB for the Transfer Control Protocol
  //
  Interface = (VOID *)PeiEfiPeiTransferControl;
  BuildHobGuidType (
          HobStart,
          &mPeiEfiPeiTransferControlGuid,
          &Interface,
          sizeof(VOID *)
          );

  //
  // Find the PEI Core in a Firmware Volume
  //
  PeiCoreFileName = NULL;
  SearchHob.Raw = FirstHob.Raw;
  Status = PeiFindFile (&SearchHob,
                         EFI_FV_FILETYPE_PEI_CORE,                     
                         EFI_SECTION_PE32,
                         &TopOfMemory,
                         LargestMemoryHob,
                         &PeiCoreFileName,
                         &Pe32Data);

  if (EFI_ERROR(Status)) {
    return;
  }

  //
  // Load the PEI Core from a Firmware Volume
  //
  Status = PeiLoadFile (
             PeiEfiPeiPeCoffLoader,
             PeiEfiPeiFlushInstructionCache,
             Pe32Data,
             &TopOfMemory,
             LargestMemoryHob,
             &PeiCoreAddress,
             &PeiCoreSize,
             &PeiCoreEntryPoint
             );

  if (EFI_ERROR(Status)) {
    return;
  }

  //
  // Transfer control to the SEC Core
  // The handoff state is the list of data from the PEI SEC chapter
  // HOB's are ignorable for purposes of PEI debug.  The only thing of interest
  // is the stack.
  //

  //
  // Capture State before jumping off edge of the world
  //
  gTopOfMemory            = TopOfMemory;   
  gLargestMemoryHob       = LargestMemoryHob;
  gPeiEfiPeiPeCoffLoader  = PeiEfiPeiPeCoffLoader;
  gPeiEfiPeiFlushInstructionCache = PeiEfiPeiFlushInstructionCache;

  PeiSwitchStacks ((VOID *)(UINTN)PeiCoreEntryPoint,
                   (VOID *)(UINTN)TopOfStack,
                   (VOID *)(UINTN)TopOfStack, 
                   (VOID *)(UINTN)NULL 
                   );

  //
  // If we get here, then the DXE Core returned.  This is an error
  //
  return;
}

//
// This function is re-written to find Driver in the Firmware Volume
//
EFI_STATUS
PeiFindFile(
  EFI_PEI_HOB_POINTERS       *Hob,
  UINT8                  Type,
  UINT16                 SectionType,
  EFI_PHYSICAL_ADDRESS   *TopOfMemory,
  EFI_PEI_HOB_POINTERS       LargestMemoryHob,
  EFI_GUID               **FileName,
  VOID                   **Pe32Data
  )
/*++

Routine Description:

  Finds a PE/COFF of a specific Type and SectionType in the Firmware Volumes
  described in the HOB list. Able to search in a compression set in a FFS file.
  But only one level of compression is supported, that is, not able to search
  in a compression set that is within another compression set.

Arguments:

  Hob         - The HOB list to search for Firmware Volumes

  Type        - The Type of file to retrieve

  SectionType - The type of section to retrieve from a file

  FileName    - The name of the file found in the Firmware Volume

  Pe32Data    - Pointer to the beginning of the PE/COFF file found in the Firmware Volume

Returns:

  EFI_SUCCESS   - The file was found, and the name is returned in FileName, and a pointer to
                  the PE/COFF image is returned in Pe32Data

  EFI_NOT_FOUND - The file was not found in the Firmware Volumes present in the HOB List

--*/
{
  EFI_FIRMWARE_VOLUME_HEADER  *FwVolHeader;
  EFI_FFS_FILE_HEADER         *FfsFileHeader;
  VOID                        *FileData;
  UINTN                       FileSize;
  UINTN                       OccupiedSize;
  EFI_COMMON_SECTION_HEADER   *Section;
  UINT32                      SectionLength;
  UINT32                      OccupiedSectionLength;
  EFI_DECOMPRESS_DECOMPRESS   Decompress;
  EFI_DECOMPRESS_GET_INFO     GetInfo;
  EFI_DECOMPRESS_PROTOCOL     *DecompressProtocol;
  EFI_STATUS                  Status;
  EFI_PHYSICAL_ADDRESS        OldTopOfMemory;
  UINT8                       *DstBuffer;
  UINT8                       *ScratchBuffer;
  UINT32                      DstBufferSize;
  UINT32                      ScratchBufferSize;
  EFI_COMMON_SECTION_HEADER   *CmpSection;
  UINT32                      CmpSectionLength;
  UINT32                      OccupiedCmpSectionLength;
  VOID                        *CmpFileData;
  UINTN                       CmpFileSize;
  UINT8                       FileState;
  UINT8                       ErasePolarity;
  UINTN                       ParsedLength;
  UINT32                      TestLength;
  UINT8                       *ptr;
  EFI_COMPRESSION_SECTION     *CompressionSection;
  
  Decompress = NULL;
  GetInfo = NULL;
  DecompressProtocol = NULL;
  Status = EFI_SUCCESS;
  DstBuffer = NULL;
  ScratchBuffer = NULL;
  DstBufferSize = 0;
  ScratchBufferSize = 0;

  //
  // Remember the top of memory
  //
  OldTopOfMemory = (*TopOfMemory);
  
  while (Hob->Header->HobType != EFI_HOB_TYPE_END_OF_HOB_LIST) {
    if (Hob->Header->HobType == EFI_HOB_TYPE_FV) {
      FwVolHeader = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)Hob->FirmwareVolume->BaseAddress;
      if (FwVolHeader->Signature == EFI_FVH_SIGNATURE &&
          CompareGuid (&FwVolHeader->FileSystemGuid, &mPeiFwFileSysTypeGuid)) {
        if ((FwVolHeader->Attributes & EFI_FVB_ERASE_POLARITY) != 0) {
          ErasePolarity = 1;
        } else {
          ErasePolarity = 0;
        }
        ParsedLength = FwVolHeader->HeaderLength;
        ptr = (UINT8 *)FwVolHeader + FwVolHeader->HeaderLength;
        while (ParsedLength < FwVolHeader->FvLength) {
          TestLength = (UINTN)FwVolHeader->FvLength - ParsedLength;
          if (TestLength > sizeof(EFI_FFS_FILE_HEADER)) {
            TestLength = sizeof(EFI_FFS_FILE_HEADER);
          }
          if (IsBufferErased(ErasePolarity, ptr, TestLength)) {
            ParsedLength += TestLength;
            ptr += TestLength;
            continue;
          }
          
          //
          // Maybe we found a possible Fv File
          //
          FfsFileHeader = (EFI_FFS_FILE_HEADER *)ptr;
          FileState = GetFileState(ErasePolarity, FfsFileHeader);
          if (FileState == EFI_FILE_HEADER_CONSTRUCTION) {
            ParsedLength += TestLength;
            ptr += TestLength;
            continue;
          }
          if (FileState == EFI_FILE_HEADER_INVALID) {
            ParsedLength += TestLength;
            ptr += TestLength;
            continue;
          }
          
          //
          // Here we may need to caculate header & file checksum
          //
          FileSize = *(UINT32 *)FfsFileHeader->Size & 0x00FFFFFF;
          OccupiedSize = GetOccupiedSize (FileSize, 8);
          if (FileState == EFI_FILE_DELETED) {
            ParsedLength += OccupiedSize;
            ptr += OccupiedSize;
            continue;
          }

          //
          // Bug here: There are maybe EFI_FILE_MARKED_FOR_UPDATED files
          //
          if (FfsFileHeader->Type != Type) {
            ParsedLength += OccupiedSize;
            ptr += OccupiedSize;
            continue;
          }

          //
          // We found file wanted
          //                
          *FileName = &FfsFileHeader->Name;
          Section = (EFI_COMMON_SECTION_HEADER *)(FfsFileHeader + 1);
          FileData = (VOID *)(FfsFileHeader + 1);
          *Pe32Data = NULL;
            
          do {
            SectionLength = *(UINT32 *)(Section->Size) & 0x00ffffff;
            OccupiedSectionLength = GetOccupiedSize (SectionLength, 4);
              
            if (Section->Type == SectionType) {
            //
            // This is what we want
            //
              *Pe32Data = (VOID *)(Section +1);
              return EFI_SUCCESS;
            } else if (Section->Type == EFI_SECTION_COMPRESSION) {
              //
              // This is a compression set, expand it
              //                            
              (*TopOfMemory) = OldTopOfMemory;
              DstBufferSize = ScratchBufferSize = 0;
              DstBuffer = (UINT8*)(UINTN)(*TopOfMemory);
              ScratchBuffer = (UINT8*)(UINTN)(*TopOfMemory);
              
              CompressionSection = (EFI_COMPRESSION_SECTION*)Section; 
              if (CompressionSection->CompressionType == EFI_STANDARD_COMPRESSION) {
                Status = GetInfo (
                           DecompressProtocol,
                           (UINT8 *)((EFI_COMPRESSION_SECTION*)Section + 1),
                           SectionLength - sizeof (EFI_COMPRESSION_SECTION),
                           &DstBufferSize,
                           &ScratchBufferSize
                           );
              } else {
              	Status = CustomizedGetInfo (
                           DecompressProtocol,
                           (UINT8 *)((EFI_COMPRESSION_SECTION*)Section + 1),
                           SectionLength - sizeof (EFI_COMPRESSION_SECTION),
                           &DstBufferSize,
                           &ScratchBufferSize
                           );   
              }            	
              
              if (EFI_SUCCESS == Status) {
                //
                // Allocate scratch buffer
                //
                if ((*TopOfMemory - LargestMemoryHob.ResourceDescriptor->PhysicalStart) 
                    < ScratchBufferSize) {
                  (*TopOfMemory) = OldTopOfMemory;
                  return EFI_OUT_OF_RESOURCES;
                }                  
                *TopOfMemory = (*TopOfMemory - ScratchBufferSize) & (~15);
                ScratchBuffer = (UINT8*)(UINTN)(*TopOfMemory);
                //
                // Allocate destination buffer
                //
                if ((*TopOfMemory - LargestMemoryHob.ResourceDescriptor->PhysicalStart) 
                    < DstBufferSize) {
                  (*TopOfMemory) = OldTopOfMemory;
                  return EFI_OUT_OF_RESOURCES;
                }                  
                *TopOfMemory = (*TopOfMemory - DstBufferSize) & (~15);
                DstBuffer = (UINT8*)(UINTN)(*TopOfMemory);
                  
                //
                // Call decompress function
                //
                if (CompressionSection->CompressionType == EFI_STANDARD_COMPRESSION) {
                  Status = Decompress (
                             DecompressProtocol,
                             (CHAR8 *)((EFI_COMPRESSION_SECTION*)Section + 1),
                             SectionLength - sizeof (EFI_COMPRESSION_SECTION),
                             DstBuffer,
                             DstBufferSize,
                             ScratchBuffer,
                             ScratchBufferSize
                             );
                 } else {
                   Status = CustomizedDecompress (
                              DecompressProtocol,
                              (CHAR8 *)((EFI_COMPRESSION_SECTION*)Section + 1),
                              SectionLength - sizeof (EFI_COMPRESSION_SECTION),
                              DstBuffer,
                              DstBufferSize,
                              ScratchBuffer,
                              ScratchBufferSize
                              );
                 }
              }   
              if (EFI_ERROR (Status)) {
                //
                // Decompress failed
                //
                (*TopOfMemory) = OldTopOfMemory;
                return EFI_NOT_FOUND;
              }
                
              //
              // Decompress successfully.
              // Loop the decompressed data searching for expected section.
              //
              CmpSection = (EFI_COMMON_SECTION_HEADER *)DstBuffer;
              CmpFileData = (VOID *)DstBuffer;
              CmpFileSize = DstBufferSize;
              do {
                CmpSectionLength = *(UINT32 *)(CmpSection->Size) & 0x00ffffff;
                if (CmpSection->Type == SectionType) {
                   
                  //
                  // This is what we want
                  //
                  *Pe32Data = (VOID *)(CmpSection +1);
                  return EFI_SUCCESS;
                }
                OccupiedCmpSectionLength = GetOccupiedSize (CmpSectionLength, 4);
                CmpSection = (EFI_COMMON_SECTION_HEADER *)((UINT8 *)CmpSection + OccupiedCmpSectionLength);
              } while (CmpSection->Type != 0 && 
                  (UINTN)((UINT8 *)CmpSection - (UINT8 *)CmpFileData) < CmpFileSize);
            }

            Section = (EFI_COMMON_SECTION_HEADER *)((UINT8 *)Section + OccupiedSectionLength);
              
          } while (Section->Type != 0 && (UINTN)((UINT8 *)Section - (UINT8 *)FileData) < FileSize);
          
        ParsedLength += OccupiedSize;
        ptr += OccupiedSize;
        continue;
        }
      }
    }
    Hob->Raw += Hob->Header->HobLength;
  }
  return EFI_NOT_FOUND;
}

EFI_STATUS
PrivateStackPeiAutoScan (
  IN OUT UINT64                *MemorySize,
  IN OUT EFI_PHYSICAL_ADDRESS  *MemoryBase
  )
{
  EFI_STATUS  Status;

  Status = WinNtOpenFile (
             L"SystemMemory",
             (UINT32)*MemorySize,
             OPEN_ALWAYS,
             MemoryBase,
             MemorySize
             );

  return Status;
}


EFI_STATUS
PrivateStackPeiLoadFile(
  VOID                  *Pe32Data,
  EFI_PHYSICAL_ADDRESS  *ImageAddress,
  UINT64                *ImageSize,
  EFI_PHYSICAL_ADDRESS  *EntryPoint
 )
{
  EFI_STATUS  Status;
//  _asm {
//    mov ecx, PeiLoadFile
//    
//    mov eax, gOldStack
//    mov gStoreStack, esp
//    mov esp, eax

//    push  EntryPoint
//    push  ImageSize
//    push  ImageAddress
//    push  gLargestMemoryHob
//    push  gTopOfMemory
//    push  Pe32Data
//    push  gPeiEfiPeiFlushInstructionCache
//    push  gPeiEfiPeiPeCoffLoader
//    call ecx
//    mov esp, gStoreStack
//  }

  Status = 
  PeiLoadFile(
    gPeiEfiPeiPeCoffLoader,
    gPeiEfiPeiFlushInstructionCache,
    Pe32Data,
    &gTopOfMemory,
    gLargestMemoryHob,
    ImageAddress,
    ImageSize,
    EntryPoint
  );
  return Status;
}


EFI_STATUS
PrivateStackWinNtThunkAddress (
  IN OUT UINT64                *InterfaceSize,
  IN OUT EFI_PHYSICAL_ADDRESS  *InterfaceBase
  )
{
  *InterfaceSize = sizeof (EFI_WIN_NT_THUNK_PROTOCOL);
  *InterfaceBase = (UINT32)&mWinNtThunkTable;

  return EFI_SUCCESS;
}

EFI_STATUS
PrivateStackWinNtFwhAddress  ( 
  IN OUT UINT64                *FwhSize,
  IN OUT EFI_PHYSICAL_ADDRESS  *FwhBase
  )

{
  *FwhBase = gEfiEmulatorBootFirmwareDevice;
  *FwhSize = gEfiEmulatorBootFirmwareDeviceSize;
  return EFI_SUCCESS;
}

EFI_STATUS
PeiLoadFile(
  EFI_PEI_PE_COFF_LOADER_PROTOCOL          *PeiEfiPeiPeCoffLoader,
  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL *PeiEfiPeiFlushInstructionCache,
  VOID                  *Pe32Data,
  EFI_PHYSICAL_ADDRESS  *TopOfMemory,
  EFI_PEI_HOB_POINTERS      LargestMemoryHob,
  EFI_PHYSICAL_ADDRESS  *ImageAddress,
  UINT64                *ImageSize,
  EFI_PHYSICAL_ADDRESS  *EntryPoint
  )
/*++

Routine Description:

  Loads and relocates a PE/COFF image into memory.

Arguments:

  Pe32Data         - The base address of the PE/COFF file that is to be loaded and relocated

  TopOfMemory      - The top free memory address.  Memory can be allocated below this point

  LargestMemoryHob - The HOB with the largest amount of free memory

  ImageAddress     - The base address of the relocated PE/COFF image

  ImageSize        - The size of the relocated PE/COFF image

  EntryPoint       - The entry point of the relocated PE/COFF image

Returns:

  EFI_SUCCESS   - The file was loaded and relocated

  EFI_OUT_OF_RESOURCES - There was not enough memory to load and relocate the PE/COFF file

--*/
{
  EFI_STATUS                            Status;
  EFI_PEI_PE_COFF_LOADER_IMAGE_CONTEXT  ImageContext;

  ZeroMem (&ImageContext, sizeof(ImageContext));
  ImageContext.Handle    = Pe32Data;

  ImageContext.ImageRead = (EFI_PEI_PE_COFF_LOADER_READ_FILE)PeiImageRead;

  Status = PeiEfiPeiPeCoffLoader->GetImageInfo(PeiEfiPeiPeCoffLoader, &ImageContext);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if ((*TopOfMemory - LargestMemoryHob.ResourceDescriptor->PhysicalStart) < ImageContext.ImageSize) {
    return EFI_OUT_OF_RESOURCES;
  }

  *TopOfMemory = (*TopOfMemory - ImageContext.ImageSize) & (~(ImageContext.SectionAlignment-1));
  ImageContext.ImageAddress = *TopOfMemory;

  Status = PeiEfiPeiPeCoffLoader->LoadImage(PeiEfiPeiPeCoffLoader, &ImageContext);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = PeiEfiPeiPeCoffLoader->RelocateImage (PeiEfiPeiPeCoffLoader, &ImageContext);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = PeiEfiPeiFlushInstructionCache->Flush (
             PeiEfiPeiFlushInstructionCache,
             ImageContext.ImageAddress,
             ImageContext.ImageSize
             );

  if (EFI_ERROR(Status)) {
    return Status;
  }

  *ImageAddress = ImageContext.ImageAddress;
  *ImageSize    = ImageContext.ImageSize;
  *EntryPoint   = ImageContext.EntryPoint;

  return EFI_SUCCESS;
}


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
  CHAR8   *Destination8;
  CHAR8   *Source8;
  UINTN   Length;

  Destination8 = Buffer;
  Source8 = (CHAR8 *)((UINTN)FileHandle + FileOffset);
  Length = *ReadSize;
  while (Length--) {
    *(Destination8++) = *(Source8++);
  }
  return EFI_SUCCESS;
}
