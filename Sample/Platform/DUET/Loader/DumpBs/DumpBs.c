/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  DumpBs.c

Abstract:

Revision History:

--*/

#include "Tiano.h"
#include "EfiCommonLib.h"
#include "EfiShellLib.h"

#define BOOT_HEADER_SIZE  0x200
#define BOOT_HEADER_MASK  0x1ff
#define BOOT_HEADER_SHIFT 9

#define MBR_START_OFFSET        0x1BE
#define MBR_END_OFFSET          0x1FE
#define MBR_ENTRY_LEN           0x10
#define MBR_OS_TYPE_OFFSET      4
#define MBR_STARTINGLBA_OFFSET  8
#define GPT_PARTITIONENTRY_LBA  72
#define GPT_PARTITIONENTRY_SIZE 84
#define GPT_ENTRT_STARTING_LBA  32
#define FAT32_BPB_START_OFFSET  11
#define FAT32_BPB_END_OFFSET    65
#define FAT_BPB_START_OFFSET    11
#define FAT_BPB_END_OFFSET      37

#define BOOT_SECTOR_LBA_OFFSET 0x1FA
#define GPT_PARTITION_INDEX    0x1B7

EFI_HANDLE   mImageHandle;

EFI_STATUS
GetBlkIOFromName (
  IN  CHAR16                  *BlockIdName,
  OUT EFI_BLOCK_IO_PROTOCOL   **BlkIo
  );

EFI_STATUS
GetFileHandleFromName (
  IN  CHAR16                        *FileName,
  OUT EFI_FILE                      **FileHandle
  );

VOID
PrintHelp (
  VOID
  );

EFI_STATUS
PatchBootSectorLbaOffset (
  IN UINT64       BootSectorLbaOffset,
  IN CHAR16       *FileName
  );

EFI_STATUS
PatchGptPartitionIndex (
  IN UINT8        GptPartitionIndex,
  IN CHAR16       *FileName
  );

UINT64
ShowBootSectorLbaOffset (
  IN CHAR16       *BlkIoName,
  IN UINTN        PartitionIndex
  );

EFI_STATUS
WriteMbr (
  IN CHAR16       *BlkIoName,
  IN CHAR16       *FileName
  );

EFI_STATUS
WriteBootSector (
  IN CHAR16       *BlkIoName,
  IN CHAR16       *FileName,
  IN UINT64       StartingLba
  );

EFI_STATUS
ReadBootSector (
  IN CHAR16       *BlkIoName,
  IN CHAR16       *FileName,
  IN UINT64       StartingLba
  );

EFI_STATUS
ReadBootSectorAndPatch (
  IN CHAR16       *BlkIoName,
  IN CHAR16       *FileName,
  IN UINT64       StartingLba
  );

EFI_STATUS
AutomaticalPatch (
  IN UINTN                     Argc,
  IN CHAR16                    **Argv
  );

EFI_APPLICATION_ENTRY_POINT(DumpBootSector)

EFI_STATUS
EFIAPI
DumpBootSector (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Dump Data from block IO devices.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error

--*/
{
  EFI_STATUS                Status;
  CHAR16                    **Argv;
  UINTN                     Argc;

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  mImageHandle  = ImageHandle;
  Argc          = SI->Argc;
  Argv          = SI->Argv;
  Status        = EFI_SUCCESS;
    
  if ((Argc == 2) &&
      (Argv[1][0] == '-' || Argv[1][0] == '/') && (Argv[1][1] == '?')) {
    PrintHelp ();
    return EFI_SUCCESS;
  }
  
  if (Argv[1][0] != '-') {
    Print (L"dumpbs: arguments error\n");
    return EFI_INVALID_PARAMETER;
  }

  if (Argv[1][1] != 'a') {
    if (Argc != 4) {
      Print (L"dumpbs: arguments error\n");
      return EFI_INVALID_PARAMETER;
    }
  } else {
    if ((Argc != 6) && (Argc != 7)) {
      Print (L"dumpbs: arguments error\n");
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Parse parameters
  //
  if (Argv[1][1] == 'g') {
    WriteMbr (Argv[2], Argv[3]);
  } else if (Argv[1][1] == 'w') {
    WriteBootSector (Argv[2], Argv[3], 0);
  } else if (Argv[1][1] == 'r') {
    ReadBootSector (Argv[2], Argv[3], 0);
  } else if (Argv[1][1] == 'p') {
    ReadBootSectorAndPatch (Argv[2], Argv[3], 0);
  } else if (Argv[1][1] == 'v') {
    ShowBootSectorLbaOffset (Argv[2], Atoi(Argv[3]));
  } else if (Argv[1][1] == 'l') {
    PatchBootSectorLbaOffset ((UINT64)Xtoi(Argv[2]), Argv[3]);
  } else if (Argv[1][1] == 'i') {
    PatchGptPartitionIndex ((UINT8)Atoi(Argv[2]), Argv[3]);
  } else if (Argv[1][1] == 'a') {
    AutomaticalPatch (Argc, Argv);
  } else {
    Print (L"dumpbs: arguments error\n");
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

//
// Internal Function
//

VOID
PrintHelp (
  VOID
  )
{
  Print (
    L"Read or write the boot sector of the specified block device.\n"
    L"\n"
    L"dumpbs -r|-p|-w|-g BlockDeviceName SectorFileName\n"
    L"dumpbs -l BootSectorLBAOffset SectorFileName\n"
    L"dumpbs -i PartitionIndex SectorFileName\n"
    L"dumpbs -v BlockDeviceName PartitionIndex\n"
    L"dumpbs -a BlockDeviceName PartitionIndex EfildrFileName SectorFileName [GptFileName]\n"
    L"  -a                  - Automatically prepare all environment\n"
    L"  -r                  - Read the boot sector to file\n"
    L"  -p                  - Read the boot sector and patch to file\n"
    L"  -w                  - Write FAT boot sector from a file\n"
    L"  -g                  - Write GPT from a file\n"
    L"  -l                  - Patch BootSectorLBAOffset for sector file\n"
    L"  -i                  - Patch GPT Partition for sector file\n"
    L"  -v                  - View the BootSectorLBAOffset\n"
    L"  BlockDeviceName     - Block device name\n"
    L"  SectorFileName      - Boot sector file name\n"
    L"  BootSectorLBAOffset - Heximal-based BootSector LBA Offset\n"
    L"  PartitionIndex      - Decimal-based Partition Index (from 0)\n"
    L"For example:\n"
    L"  dumpbs -a 0 blk3 Efildr20 bs32.com gpt.com\n"
    );
}

VOID
PatchBlock (
  IN UINT8   *DestBuffer,
  IN UINT8   *SourceBuffer,
  IN BOOLEAN IsMbr
  )
{
  if (IsMbr) {
    //
    // Handle MBR
    //
    CopyMem (
      DestBuffer + MBR_START_OFFSET,
      SourceBuffer + MBR_START_OFFSET,
      MBR_END_OFFSET - MBR_START_OFFSET
      );
  } else {
    if (*(UINT64 *)(DestBuffer + 82) == (0x2020203233544146l)) {
      //
      // Handle FAT32 BPB
      //
      CopyMem (
        DestBuffer + FAT32_BPB_START_OFFSET,
        SourceBuffer + FAT32_BPB_START_OFFSET,
        FAT32_BPB_END_OFFSET - FAT32_BPB_START_OFFSET
      );
    } else {
      //
      // Handle FAT BPB
      //
      CopyMem (
        DestBuffer + FAT_BPB_START_OFFSET,
        SourceBuffer + FAT_BPB_START_OFFSET,
        FAT_BPB_END_OFFSET - FAT_BPB_START_OFFSET
        );
    }
  }
  
  return ;
}

EFI_STATUS
PatchBootSectorLbaOffset (
  IN UINT64       BootSectorLbaOffset,
  IN CHAR16       *FileName
  )
{
  EFI_FILE                  *FileHandle;
  VOID                      *Buffer;
  UINTN                     BufSize;
  EFI_STATUS                Status;

  Print (L"dumpbs: BootSectorLBAOffset - 0x%x\n", BootSectorLbaOffset);

  //
  // Get the File from sector file name
  //
  Status = GetFileHandleFromName (FileName, &FileHandle);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: GetFileHandleFromName fail - %s\n", FileName);
    return Status;
  }

  //
  // Init
  //
  BufSize = BOOT_HEADER_SIZE;
  Buffer = AllocatePool (BOOT_HEADER_SIZE);
  ASSERT (Buffer != NULL);

  //
  // Set file pointer to end start position
  //
  Status = FileHandle->SetPosition (FileHandle, 0);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Set %hs pos error - %r\n", FileName, Status);
    goto Done;
  }

  //
  // Read data from file
  //
  Status = FileHandle->Read (FileHandle, &BufSize, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: - Read error - %r\n", Status);
    goto Done;
  }

  *(UINT32 *)((UINT8 *)Buffer + BOOT_SECTOR_LBA_OFFSET) = (UINT32)BootSectorLbaOffset;

  //
  // Set file pointer to end start position
  //
  Status = FileHandle->SetPosition (FileHandle, 0);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Set %hs pos error - %r\n", FileName, Status);
    goto Done;
  }

  //
  // Write the boot sector to file
  //
  Status = FileHandle->Write (FileHandle, &BufSize, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: - Write error - %r\n", Status);
    goto Done;
  }

  Print(L"dumpbs: Patch BootSectorLBAOffset successfully!\n");

Done:
  FreePool(Buffer);
  FileHandle->Close(FileHandle);

  return Status;
}

EFI_STATUS
PatchGptPartitionIndex (
  IN UINT8        GptPartitionIndex,
  IN CHAR16       *FileName
  )
{
  EFI_FILE                  *FileHandle;
  VOID                      *Buffer;
  UINTN                     BufSize;
  EFI_STATUS                Status;

  Print (L"dumpbs: GptPartitionIndex - 0x%x\n", GptPartitionIndex);

  //
  // Get the File from sector file name
  //
  Status = GetFileHandleFromName (FileName, &FileHandle);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: GetFileHandleFromName fail - %s\n", FileName);
    return Status;
  }

  //
  // Init
  //
  BufSize = BOOT_HEADER_SIZE;
  Buffer = AllocatePool (BOOT_HEADER_SIZE);
  ASSERT (Buffer != NULL);

  //
  // Set file pointer to end start position
  //
  Status = FileHandle->SetPosition (FileHandle, 0);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Set %hs pos error - %r\n", FileName, Status);
    goto Done;
  }

  //
  // Read data from file
  //
  Status = FileHandle->Read (FileHandle, &BufSize, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: - Read error - %r\n", Status);
    goto Done;
  }

  *((UINT8 *)Buffer + GPT_PARTITION_INDEX) = (UINT8)GptPartitionIndex;

  //
  // Set file pointer to end start position
  //
  Status = FileHandle->SetPosition (FileHandle, 0);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Set %hs pos error - %r\n", FileName, Status);
    goto Done;
  }

  //
  // Write the boot sector to file
  //
  Status = FileHandle->Write (FileHandle, &BufSize, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: - Write error - %r\n", Status);
    goto Done;
  }

  Print(L"dumpbs: Patch GptParitionIndex successfully!\n");

Done:
  FreePool(Buffer);
  FileHandle->Close(FileHandle);

  return Status;
}

UINT64
ShowBootSectorLbaOffset (
  IN CHAR16       *BlkIoName,
  IN UINTN        PartitionIndex
  )
{
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;
  VOID                      *Buffer;
  UINTN                     BufSize;
  EFI_STATUS                Status;
  UINT64                    LBAOffset;
  UINT64                    PartitionEntryLBA;
  UINT64                    TargetEntryLBA;
  UINT32                    TargetEntryOffset;

  Status = GetBlkIOFromName (BlkIoName, &BlkIo);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: GetBlkIOFromName fail - %s\n", BlkIoName);
    return 0;
  }

  LBAOffset = 0;

  //
  // Init
  //
  BufSize = BOOT_HEADER_SIZE;
  Buffer = AllocatePool (BOOT_HEADER_SIZE);
  ASSERT (Buffer != NULL);

  //
  // Read the block device's boot sector
  //
  Status = BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, 0, BOOT_HEADER_SIZE, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Read boot sector error - %r\n", Status);
    goto Done;
  }

  if (*((UINT8 *)Buffer + MBR_START_OFFSET + MBR_OS_TYPE_OFFSET) != 0xEE) {
    //
    // MBR
    //
    Print(L"dumpbs: MBR Parition\n");

    if (PartitionIndex >= 4) {
      Print (L"dumpbs: ParitionIndex error - %d\n", PartitionIndex);
    } else {
      LBAOffset = (UINT64)*(UINT32 *)((UINT8 *)Buffer + 
                              MBR_START_OFFSET + 
                              MBR_ENTRY_LEN * PartitionIndex +
                              MBR_STARTINGLBA_OFFSET
                              );
    }
  } else {
    //
    // GPT
    //

    Print(L"dumpbs: GPT Parition\n");

    //
    // Read the GPT Header
    //
    Status = BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, 1, BOOT_HEADER_SIZE, Buffer);
    if (EFI_ERROR(Status)) {
      Print(L"dumpbs: Read boot sector error - %r\n", Status);
      goto Done;
    }
    PartitionEntryLBA = *(UINT64 *)((UINT8 *)Buffer + GPT_PARTITIONENTRY_LBA);
    TargetEntryLBA    = (UINT64)(*(UINT32 *)((UINT8 *)Buffer + GPT_PARTITIONENTRY_SIZE) * 
                                 PartitionIndex);
    TargetEntryOffset = ((UINT32)TargetEntryLBA & BOOT_HEADER_MASK);
    TargetEntryLBA    = PartitionEntryLBA + RShiftU64 (TargetEntryLBA, BOOT_HEADER_SHIFT);

    //
    // Read the GPT Entry
    //
    Status = BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, TargetEntryLBA, BOOT_HEADER_SIZE, Buffer);
    if (EFI_ERROR(Status)) {
      Print(L"dumpbs: Read boot sector error - %r\n", Status);
      goto Done;
    }
    LBAOffset = *(UINT64 *)((UINT8 *)Buffer + TargetEntryOffset + GPT_ENTRT_STARTING_LBA);
  }

  Print(L"dumpbs: BootSectorLBAOffset is 0x%lX\n", LBAOffset);

Done:
  FreePool(Buffer);
  return LBAOffset;
}

EFI_STATUS
WriteMbr (
  IN CHAR16       *BlkIoName,
  IN CHAR16       *FileName
  )
{
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;
  EFI_FILE                  *FileHandle;
  VOID                      *Buffer;
  VOID                      *TempBuffer;
  UINTN                     BufSize;
  EFI_STATUS                Status;

  Status = GetBlkIOFromName (BlkIoName, &BlkIo);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: GetBlkIOFromName fail - %s\n", BlkIoName);
    return Status;
  }

  //
  // Get the File from sector file name
  //
  Status = GetFileHandleFromName (FileName, &FileHandle);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: GetFileHandleFromName fail - %s\n", FileName);
    return Status;
  }

  //
  // Init
  //
  BufSize = BOOT_HEADER_SIZE;
  Buffer = AllocatePool (BOOT_HEADER_SIZE);
  ASSERT (Buffer != NULL);
  TempBuffer = AllocatePool (BOOT_HEADER_SIZE);
  ASSERT (TempBuffer != NULL);

  //
  // Write the boot sector to block device from a file
  //

  //
  // Set file pointer to end start position
  //
  Status = FileHandle->SetPosition (FileHandle, 0);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Set %hs pos error - %r\n", FileName, Status);
    goto Done;
  }

  //
  // Read data from file
  //
  Status = FileHandle->Read (FileHandle, &BufSize, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: - Read error - %r\n", Status);
    goto Done;
  }

  //
  // Whether Boot sector file size is 512
  //
  if (BufSize != BOOT_HEADER_SIZE) {
    Print(L"dumpbs: Error, Boot sector file is smaller than %d bytes\n", BOOT_HEADER_SIZE);
    Status = EFI_ABORTED;
    goto Done;
  }

  //
  // Read block device's boot sector to TempBuffer
  //      
  Status = BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, 0, BOOT_HEADER_SIZE, TempBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Read boot sector error - %r\n", Status);
    goto Done;
  }

  //
  // Update target buffer
  //
  PatchBlock (Buffer, TempBuffer, TRUE);

  //
  // Finnally write the boot sector
  //
  Status = BlkIo->WriteBlocks (BlkIo, BlkIo->Media->MediaId, 0, BOOT_HEADER_SIZE, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Write boot sector error - %r\n", Status);
    goto Done;
  }

  Print(L"dumpbs: Write boot sector finished\n");

Done:
  FreePool(TempBuffer);
  FreePool(Buffer);
  FileHandle->Close(FileHandle);

  return Status;
}

EFI_STATUS
WriteBootSector (
  IN CHAR16       *BlkIoName,
  IN CHAR16       *FileName,
  IN UINT64       StartingLba
  )
{
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;
  EFI_FILE                  *FileHandle;
  VOID                      *Buffer;
  VOID                      *TempBuffer;
  UINTN                     BufSize;
  EFI_STATUS                Status;

  Status = GetBlkIOFromName (BlkIoName, &BlkIo);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: GetBlkIOFromName fail - %s\n", BlkIoName);
    return Status;
  }

  //
  // Get the File from sector file name
  //
  Status = GetFileHandleFromName (FileName, &FileHandle);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: GetFileHandleFromName fail - %s\n", FileName);
    return Status;
  }

  //
  // Init
  //
  BufSize = BOOT_HEADER_SIZE;
  Buffer = AllocatePool (BOOT_HEADER_SIZE);
  ASSERT (Buffer != NULL);
  TempBuffer = AllocatePool (BOOT_HEADER_SIZE);
  ASSERT (TempBuffer != NULL);

  //
  // Write the boot sector to block device from a file
  //

  //
  // Set file pointer to end start position
  //
  Status = FileHandle->SetPosition (FileHandle, 0);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Set %hs pos error - %r\n", FileName, Status);
    goto Done;
  }

  //
  // Read data from file
  //
  Status = FileHandle->Read (FileHandle, &BufSize, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: - Read error - %r\n", Status);
    goto Done;
  }

  //
  // Whether Boot sector file size is 512
  //
  if (BufSize != BOOT_HEADER_SIZE) {
    Print(L"dumpbs: Error, Boot sector file is smaller than %d bytes\n", BOOT_HEADER_SIZE);
    Status = EFI_ABORTED;
    goto Done;
  }

  //
  // Read block device's boot sector to TempBuffer
  //      
  Status = BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, StartingLba, BOOT_HEADER_SIZE, TempBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Read boot sector error - %r\n", Status);
    goto Done;
  }

  //
  // Update target buffer
  //
  PatchBlock (Buffer, TempBuffer, FALSE);

  //
  // Finnally write the boot sector
  //
  Status = BlkIo->WriteBlocks (BlkIo, BlkIo->Media->MediaId, StartingLba, BOOT_HEADER_SIZE, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Write boot sector error - %r\n", Status);
    goto Done;
  }

  Print(L"dumpbs: Write boot sector finished\n");

Done:
  FreePool(TempBuffer);
  FreePool(Buffer);
  FileHandle->Close(FileHandle);

  return Status;
}

EFI_STATUS
ReadBootSector (
  IN CHAR16       *BlkIoName,
  IN CHAR16       *FileName,
  IN UINT64       StartingLba
  )
{
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;
  EFI_FILE                  *FileHandle;
  VOID                      *Buffer;
  UINTN                     BufSize;
  EFI_STATUS                Status;

  Status = GetBlkIOFromName (BlkIoName, &BlkIo);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: GetBlkIOFromName fail - %s\n", BlkIoName);
    return Status;
  }

  //
  // Get the File from sector file name
  //
  Status = GetFileHandleFromName (FileName, &FileHandle);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: GetFileHandleFromName fail - %s\n", FileName);
    return Status;
  }

  //
  // Init
  //
  BufSize = BOOT_HEADER_SIZE;
  Buffer = AllocatePool (BOOT_HEADER_SIZE);
  ASSERT (Buffer != NULL);

  //
  // Dump the boot sector to file from a block device
  //

  //
  // Read the block device's boot sector
  //
  Status = BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, StartingLba, BOOT_HEADER_SIZE, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Read boot sector error - %r\n", Status);
    goto Done;
  }

  //
  // Set file pointer to end start position
  //
  Status = FileHandle->SetPosition (FileHandle, 0);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Set %hs pos error - %r\n", FileName, Status);
    goto Done;
  }

  //
  // Write the boot sector to file
  //
  Status = FileHandle->Write (FileHandle, &BufSize, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: - Write error - %r\n", Status);
    goto Done;
  }

  //
  // Boot sector file is smaller than 512 bytes
  //
  if (BufSize != BOOT_HEADER_SIZE) {
    Print(L"dumpbs: Boot sector file is smaller than %d bytes\n", BOOT_HEADER_SIZE);
    Status = EFI_ABORTED;
    goto Done;
  }

  Print(L"dumpbs: Dump boot sector successfully\n");

Done:
  FreePool(Buffer);
  FileHandle->Close(FileHandle);

  return Status;
}

EFI_STATUS
ReadBootSectorAndPatch (
  IN CHAR16       *BlkIoName,
  IN CHAR16       *FileName,
  IN UINT64       StartingLba
  )
{
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;
  EFI_FILE                  *FileHandle;
  VOID                      *Buffer;
  VOID                      *TempBuffer;
  UINTN                     BufSize;
  EFI_STATUS                Status;

  Status = GetBlkIOFromName (BlkIoName, &BlkIo);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: GetBlkIOFromName fail - %s\n", BlkIoName);
    return Status;
  }

  //
  // Get the File from sector file name
  //
  Status = GetFileHandleFromName (FileName, &FileHandle);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: GetFileHandleFromName fail - %s\n", FileName);
    return Status;
  }

  //
  // Init
  //
  BufSize = BOOT_HEADER_SIZE;
  Buffer = AllocatePool (BOOT_HEADER_SIZE);
  ASSERT (Buffer != NULL);
  TempBuffer = AllocatePool (BOOT_HEADER_SIZE);
  ASSERT (TempBuffer != NULL);

  //
  // Dump the boot sector to file from a block device
  //

  //
  // Read the block device's boot sector
  //
  Status = BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, StartingLba, BOOT_HEADER_SIZE, Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Read boot sector error - %r\n", Status);
    goto Done;
  }

  //
  // Set file pointer to end start position
  //
  Status = FileHandle->SetPosition (FileHandle, 0);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Set %hs pos error - %r\n", FileName, Status);
    goto Done;
  }

  //
  // Read data from file
  //    
  Status = FileHandle->Read (FileHandle, &BufSize, TempBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: - Read error - %r\n", Status);
    goto Done;
  }
      
  //
  // Whether Boot sector file size is 512
  //
  if (BufSize < BOOT_HEADER_SIZE) {
    Print(L"dumpbs: Error, Boot sector file is smaller than %d bytes\n", BOOT_HEADER_SIZE);
    Status = EFI_ABORTED;
    goto Done;
  }
      
  //
  // Update target buffer
  //
  PatchBlock (TempBuffer, Buffer, FALSE);

  //
  // Set file pointer to end start position
  //
  Status = FileHandle->SetPosition (FileHandle, 0);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: Set %hs pos error - %r\n", FileName, Status);
    goto Done;
  }

  //
  // Write the boot sector to file
  //
  Status = FileHandle->Write (FileHandle, &BufSize, TempBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"dumpbs: - Write error - %r\n", Status);
    goto Done;
  }

  //
  // Boot sector file is smaller than 512 bytes
  //
  if (BufSize != BOOT_HEADER_SIZE) {
    Print(L"dumpbs: Boot sector file is smaller than %d bytes\n", BOOT_HEADER_SIZE);
    Status = EFI_ABORTED;
    goto Done;
  }

  Print(L"dumpbs: Dump boot sector successfully\n");

Done:
  FreePool(TempBuffer);
  FreePool(Buffer);
  FileHandle->Close(FileHandle);

  return Status;
}

EFI_STATUS
AutomaticalPatch (
  IN UINTN                     Argc,
  IN CHAR16                    **Argv
  )
{
  UINT64       BootSectorLba;
  EFI_STATUS   Status;

  //
  // Get BootSectorLba
  //
  BootSectorLba = ShowBootSectorLbaOffset (Argv[2], Atoi(Argv[3]));
  if (BootSectorLba == 0) {
    Print (L"dumpbs: BootSectorLba invalid for parititon %d\n", Argv[3]);
    return EFI_INVALID_PARAMETER;
  }

  //
  // Prepare Efildr file
  //
  Status = ReadBootSectorAndPatch (Argv[2], Argv[4], BootSectorLba);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  Status = PatchBootSectorLbaOffset (BootSectorLba, Argv[4]);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Prepare BootSector file
  //
  Status = ReadBootSectorAndPatch (Argv[2], Argv[5], BootSectorLba);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  Status = PatchBootSectorLbaOffset (BootSectorLba, Argv[5]);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  Status = WriteBootSector (Argv[2], Argv[5], BootSectorLba);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Write GPT MBR (optionally)
  //
  if (Argc == 7) {
    Status = WriteMbr (Argv[2], Argv[6]);
  }

  return Status;
}

//
// Internal Library
//

EFI_STATUS
GetBlkIOFromName (
  IN  CHAR16                        *BlockIdName,
  OUT EFI_BLOCK_IO_PROTOCOL         **BlkIo
  )
/*++

Routine Description:

Arguments:
    
  BlockIdName           Pointer to the block device name
  BlkIo                 Returned block IO

Returns:

  EFI_SUCCESS             Get BlkIo successful
  EFI_INVALID_PARAMETER   Invalid BlockIdName

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_STATUS                Status;
    
  DevicePath  = NULL;
  
  //
  // Get the device path from block device id
  //
  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)ShellGetMap (BlockIdName);
  if (DevicePath == NULL) {
    Print(L"dumpbs: Invalid block id \"%hs\"\n", BlockIdName);
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Get the block io from device path
  //
  Status = LibDevicePathToInterface (&gEfiBlockIoProtocolGuid, DevicePath, (VOID **)BlkIo);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: Device is not a BlockIo device - %r\n", Status);
    return Status;
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
GetFileHandleFromName (
  IN  CHAR16                        *FileName,
  OUT EFI_FILE                      **FileHandle
  )
/*++

Routine Description:

Arguments:
    
  FileName              Pointer to the file name
  FileHandle            Returned file handle

Returns:

  EFI_SUCCESS             Get FileHandle successful
  EFI_INVALID_PARAMETER   Invalid FileName

--*/
{
  EFI_LOADED_IMAGE_PROTOCOL             *Image;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  EFI_HANDLE                            DeviceHandle;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *SimpleFileSystem;
  static EFI_FILE                       *mRoot = NULL;
  EFI_STATUS                            Status;
 
  if (mRoot == NULL) {
    Status = BS->HandleProtocol (
                   mImageHandle,
                   &gEfiLoadedImageProtocolGuid,
                   &Image
                   );
    if (EFI_ERROR(Status)) {
      Print (L"Error: HandleProtocol LoadedImage ! - %r\n", Status);
      return EFI_INVALID_PARAMETER;
    }
    Status = BS->HandleProtocol (
                   Image->DeviceHandle,
                   &gEfiDevicePathProtocolGuid,
                   &DevicePath
                   );
    if (EFI_ERROR(Status)) {
      Print (L"Error: HandleProtocol DevicePath ! - %r\n", Status);
      return EFI_INVALID_PARAMETER;
    }
    Status = BS->LocateDevicePath ( 
                   &gEfiSimpleFileSystemProtocolGuid,
                   &DevicePath,
                   &DeviceHandle
                   );
    if (EFI_ERROR (Status)) {
      Print (L"Error: LocateDevicePath SimpleFileSystem ! - %r\n", Status);
      return EFI_INVALID_PARAMETER;
    }

    Status = BS->HandleProtocol (
                   DeviceHandle, 
                   &gEfiSimpleFileSystemProtocolGuid,
                   (VOID*)&SimpleFileSystem
                   );
    if (EFI_ERROR (Status)) {
      Print (L"Error: HandleProtocol SimpleFileSystem ! - %r\n", Status);
      return EFI_INVALID_PARAMETER;
    }
    Status = SimpleFileSystem->OpenVolume (
                                 SimpleFileSystem,
                                 &mRoot
                                 );
    if (EFI_ERROR (Status)) {
      Print (L"Error: SimpleFileSystem->OpenVolume() ! - %r\n", Status);
      return EFI_INVALID_PARAMETER;
    }
  }
  
  Status = mRoot->Open (
                   mRoot,
                   FileHandle,
                   FileName,
                   EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                   0
                   );
  if (EFI_ERROR (Status)) {
    Print (L"Error: mRoot->Open() ! - %r\n", Status);
    return EFI_INVALID_PARAMETER;
  }
  
  return EFI_SUCCESS;
}

