/*++

Copyright 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    bootsectimage.c
    
Abstract:

Revision History

--*/


#include <windows.h>
#include <stdio.h>
#include "fat.h"
#include "mbr.h"

#define BOOT_SECTOR_LBA_OFFSET 0x1FA

int WriteToFile (void *BootSector, char *FileName)
{
  FILE *FileHandle;
  int  result;

  FileHandle = fopen (FileName, "r+b");
  if (FileHandle == NULL) {
    printf ("error open file: %s\n", FileName);
    return 0;
  }
  fseek (FileHandle, 0, SEEK_SET);

  result = fwrite (BootSector, 1, 512, FileHandle);
  if (result != 512) {
    printf ("error write file: %s\n", FileName);
    result = 0;
  }

  fclose (FileHandle);
  return result;
}

int ReadFromFile (void *BootSector, char *FileName)
{
  FILE *FileHandle;
  int  result;

  FileHandle = fopen (FileName, "rb");
  if (FileHandle == NULL) {
    printf ("error open file: %s\n", FileName);
    return 0;
  }

  result = fread (BootSector, 1, 512, FileHandle);
  if (result != 512) {
    printf ("error read file: %s\n", FileName);
    result = 0;
  }

  fclose (FileHandle);
  return result;
}

char *
FatTypeToString (
  IN FAT_TYPE        FatType
  )
{
  switch (FatType) {
  case FatTypeFat12:
    return "FAT12";
  case FatTypeFat16:
    return "FAT16";
  case FatTypeFat32:
    return "FAT32";
  default:
    break;
  }
  return "FAT Unknown";
}

FAT_TYPE
GetFatType (
  IN FAT_BPB_STRUCT  *FatBpb
  )
{
  FAT_TYPE FatType;
  UINTN    RootDirSectors;
  UINTN    FATSz;
  UINTN    TotSec;
  UINTN    DataSec;
  UINTN    CountOfClusters;
  CHAR8    FilSysType[9];

  FatType = FatTypeUnknown;

  //
  // Simple check
  //
  if (FatBpb->Fat12_16.Signature != FAT_BS_SIGNATURE) {
    printf ("ERROR: FAT: Signature Invalid - %04x, expected - %04x\n",
        FatBpb->Fat12_16.Signature, FAT_BS_SIGNATURE);
    return FatTypeUnknown;
  }

  //
  // Check according to FAT spec
  //
  if ((FatBpb->Fat12_16.BS_jmpBoot[0] != FAT_BS_JMP1) &&
      (FatBpb->Fat12_16.BS_jmpBoot[0] != FAT_BS_JMP2)) {
    printf ("ERROR: FAT: BS_jmpBoot - %02x, expected - %02x or %02x\n",
        FatBpb->Fat12_16.BS_jmpBoot[0], FAT_BS_JMP1, FAT_BS_JMP2);
    return FatTypeUnknown;
  }

  if ((FatBpb->Fat12_16.BPB_BytsPerSec != 512) &&
      (FatBpb->Fat12_16.BPB_BytsPerSec != 1024) &&
      (FatBpb->Fat12_16.BPB_BytsPerSec != 2048) &&
      (FatBpb->Fat12_16.BPB_BytsPerSec != 4096)) {
    printf ("ERROR: FAT: BPB_BytsPerSec - %04x, expected - %04x, %04x, %04x, or %04x\n",
        FatBpb->Fat12_16.BPB_BytsPerSec, 512, 1024, 2048, 4096);
    return FatTypeUnknown;
  }
  if (FatBpb->Fat12_16.BPB_BytsPerSec != 512) {
    printf ("WARNING: FAT: BPB_BytsPerSec - %04x, expected - %04x\n",
        FatBpb->Fat12_16.BPB_BytsPerSec, 512);
  }
  if ((FatBpb->Fat12_16.BPB_SecPerClus != 1) &&
      (FatBpb->Fat12_16.BPB_SecPerClus != 2) &&
      (FatBpb->Fat12_16.BPB_SecPerClus != 4) &&
      (FatBpb->Fat12_16.BPB_SecPerClus != 8) &&
      (FatBpb->Fat12_16.BPB_SecPerClus != 16) &&
      (FatBpb->Fat12_16.BPB_SecPerClus != 32) &&
      (FatBpb->Fat12_16.BPB_SecPerClus != 64) &&
      (FatBpb->Fat12_16.BPB_SecPerClus != 128)) {
    printf ("ERROR: FAT: BPB_SecPerClus - %02x, expected - %02x, %02x, %02x, %02x, %02x, %02x, %02x, or %02x\n",
        FatBpb->Fat12_16.BPB_BytsPerSec, 1, 2, 4, 8, 16, 32, 64, 128);
    return FatTypeUnknown;
  }
  if (FatBpb->Fat12_16.BPB_BytsPerSec * FatBpb->Fat12_16.BPB_SecPerClus > 32 * 1024) {
    printf ("ERROR: FAT: BPB_BytsPerSec * BPB_SecPerClus - %08x, expected <= %08x\n",
        FatBpb->Fat12_16.BPB_BytsPerSec * FatBpb->Fat12_16.BPB_SecPerClus, 32 * 1024);
    return FatTypeUnknown;
  }
  if (FatBpb->Fat12_16.BPB_RsvdSecCnt == 0) {
    printf ("ERROR: FAT: BPB_RsvdSecCnt - %04x, expected - Non-Zero\n",
        FatBpb->Fat12_16.BPB_RsvdSecCnt);
    return FatTypeUnknown;
  }
  if (FatBpb->Fat12_16.BPB_NumFATs != 2) {
    printf ("WARNING: FAT: BPB_NumFATs - %02x, expected - %02x\n",
        FatBpb->Fat12_16.BPB_NumFATs, 2);
  }
  if ((FatBpb->Fat12_16.BPB_Media != 0xF0) &&
      (FatBpb->Fat12_16.BPB_Media != 0xF8) &&
      (FatBpb->Fat12_16.BPB_Media != 0xF9) &&
      (FatBpb->Fat12_16.BPB_Media != 0xFA) &&
      (FatBpb->Fat12_16.BPB_Media != 0xFB) &&
      (FatBpb->Fat12_16.BPB_Media != 0xFC) &&
      (FatBpb->Fat12_16.BPB_Media != 0xFD) &&
      (FatBpb->Fat12_16.BPB_Media != 0xFE) &&
      (FatBpb->Fat12_16.BPB_Media != 0xFF)) {
    printf ("ERROR: FAT: BPB_Media - %02x, expected - %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, or %02x\n",
        FatBpb->Fat12_16.BPB_Media, 0xF0, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF);
    return FatTypeUnknown;
  }

  //
  // Algo in FAT spec
  //
  RootDirSectors = ((FatBpb->Fat12_16.BPB_RootEntCnt * sizeof(FAT_DIRECTORY_ENTRY)) +
                    (FatBpb->Fat12_16.BPB_BytsPerSec - 1)) /
                   FatBpb->Fat12_16.BPB_BytsPerSec;

  if (FatBpb->Fat12_16.BPB_FATSz16 != 0) {
    FATSz = FatBpb->Fat12_16.BPB_FATSz16;
  } else {
    FATSz = FatBpb->Fat32.BPB_FATSz32;
  }
  if (FATSz == 0) {
    printf ("ERROR: FAT: BPB_FATSz16, BPB_FATSz32 - 0, expected - Non-Zero\n");
    return FatTypeUnknown;
  }

  if (FatBpb->Fat12_16.BPB_TotSec16 != 0) {
    TotSec = FatBpb->Fat12_16.BPB_TotSec16;
  } else {
    TotSec = FatBpb->Fat12_16.BPB_TotSec32;
  }
  if (TotSec == 0) {
    printf ("ERROR: FAT: BPB_TotSec16, BPB_TotSec32 - 0, expected - Non-Zero\n");
    return FatTypeUnknown;
  }

  DataSec = TotSec - (
                      FatBpb->Fat12_16.BPB_RsvdSecCnt +
                      FatBpb->Fat12_16.BPB_NumFATs * FATSz +
                      RootDirSectors
                     );

  CountOfClusters = DataSec / FatBpb->Fat12_16.BPB_SecPerClus;

  if (CountOfClusters < FAT_MAX_FAT12_CLUSTER) {
    FatType = FatTypeFat12;
  } else if (CountOfClusters < FAT_MAX_FAT16_CLUSTER) {
    FatType = FatTypeFat16;
  } else {
    FatType = FatTypeFat32;
  }
  printf ("Fat Type: %s\n", FatTypeToString (FatType));

  //
  // Check according to FAT spec
  //
  if (((FatType == FatTypeFat12) || (FatType == FatTypeFat16)) &&
       (FatBpb->Fat12_16.BPB_RsvdSecCnt != 1)) {
    printf ("WARNING: FAT12_16: BPB_RsvdSecCnt - %04x, expected - %04x\n",
        FatBpb->Fat12_16.BPB_RsvdSecCnt, 1);
  }
  if ((FatType == FatTypeFat32) &&
       (FatBpb->Fat12_16.BPB_RsvdSecCnt != 32)) {
    printf ("WARNING: FAT32: BPB_RsvdSecCnt - %04x, expected - %04x\n",
        FatBpb->Fat12_16.BPB_RsvdSecCnt, 32);
  }
  if ((FatType == FatTypeFat16) &&
      (FatBpb->Fat12_16.BPB_RootEntCnt != 512)) {
    printf ("WARNING: FAT16: BPB_RootEntCnt - %04x, expected - %04x\n",
        FatBpb->Fat12_16.BPB_RootEntCnt, 512);
  }
  if ((FatType == FatTypeFat32) &&
      (FatBpb->Fat12_16.BPB_RootEntCnt != 0)) {
    printf ("ERROR: FAT32: BPB_RootEntCnt - %04x, expected - %04x\n",
        FatBpb->Fat12_16.BPB_RootEntCnt, 0);
    return FatTypeUnknown;
  }
  if ((FatType == FatTypeFat32) &&
      (FatBpb->Fat12_16.BPB_TotSec16 != 0)) {
    printf ("ERROR: FAT32: BPB_TotSec16 - %04x, expected - %04x\n",
        FatBpb->Fat12_16.BPB_TotSec16, 0);
    return FatTypeUnknown;
  }
  if ((FatType == FatTypeFat32) &&
      (FatBpb->Fat12_16.BPB_FATSz16 != 0)) {
    printf ("ERROR: FAT32: BPB_FATSz16 - %04x, expected - %04x\n",
        FatBpb->Fat12_16.BPB_FATSz16, 0);
    return FatTypeUnknown;
  }
  if ((FatType == FatTypeFat32) &&
      (FatBpb->Fat12_16.BPB_TotSec32 == 0)) {
    printf ("ERROR: FAT32: BPB_TotSec32 - %04x, expected - Non-Zero\n",
        FatBpb->Fat12_16.BPB_TotSec32);
    return FatTypeUnknown;
  }
  if ((FatType == FatTypeFat32) &&
      (FatBpb->Fat32.BPB_FATSz32 == 0)) {
    printf ("ERROR: FAT32: BPB_FATSz32 - %08x, expected - Non-Zero\n",
        FatBpb->Fat32.BPB_FATSz32);
    return FatTypeUnknown;
  }
  if ((FatType == FatTypeFat32) &&
      (FatBpb->Fat32.BPB_FSVer != 0)) {
    printf ("WARNING: FAT32: BPB_FSVer - %08x, expected - %04x\n",
        FatBpb->Fat32.BPB_FSVer, 0);
  }
  if ((FatType == FatTypeFat32) &&
      (FatBpb->Fat32.BPB_RootClus != 2)) {
    printf ("WARNING: FAT32: BPB_RootClus - %08x, expected - %04x\n",
        FatBpb->Fat32.BPB_RootClus, 2);
  }
  if ((FatType == FatTypeFat32) &&
      (FatBpb->Fat32.BPB_FSInfo != 1)) {
    printf ("WARNING: FAT32: BPB_FSInfo - %08x, expected - %04x\n",
        FatBpb->Fat32.BPB_FSInfo, 1);
  }
  if ((FatType == FatTypeFat32) &&
      (FatBpb->Fat32.BPB_BkBootSec != 6)) {
    printf ("WARNING: FAT32: BPB_BkBootSec - %08x, expected - %04x\n",
        FatBpb->Fat32.BPB_BkBootSec, 6);
  }
  if ((FatType == FatTypeFat32) &&
      ((*(UINT32 *)FatBpb->Fat32.BPB_Reserved != 0) ||
       (*((UINT32 *)FatBpb->Fat32.BPB_Reserved + 1) != 0) ||
       (*((UINT32 *)FatBpb->Fat32.BPB_Reserved + 2) != 0))) {
    printf ("ERROR: FAT32: BPB_Reserved - %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x, expected - 0\n",
        FatBpb->Fat32.BPB_Reserved[0],
        FatBpb->Fat32.BPB_Reserved[1],
        FatBpb->Fat32.BPB_Reserved[2],
        FatBpb->Fat32.BPB_Reserved[3],
        FatBpb->Fat32.BPB_Reserved[4],
        FatBpb->Fat32.BPB_Reserved[5],
        FatBpb->Fat32.BPB_Reserved[6],
        FatBpb->Fat32.BPB_Reserved[7],
        FatBpb->Fat32.BPB_Reserved[8],
        FatBpb->Fat32.BPB_Reserved[9],
        FatBpb->Fat32.BPB_Reserved[10],
        FatBpb->Fat32.BPB_Reserved[11]);
    return FatTypeUnknown;
  }
  if (((FatType == FatTypeFat12) || (FatType == FatTypeFat16)) &&
       (FatBpb->Fat12_16.BS_Reserved1 != 0)) {
    printf ("ERROR: FAT12_16: BS_Reserved1 - %02x, expected - 0\n",
        FatBpb->Fat12_16.BS_Reserved1);
    return FatTypeUnknown;
  }
  if ((FatType == FatTypeFat32) &&
      (FatBpb->Fat32.BS_Reserved1 != 0)) {
    printf ("ERROR: FAT32: BS_Reserved1 - %02x, expected - 0\n",
        FatBpb->Fat32.BS_Reserved1);
    return FatTypeUnknown;
  }
  if (((FatType == FatTypeFat12) || (FatType == FatTypeFat16)) &&
       (FatBpb->Fat12_16.BS_BootSig != FAT_BS_BOOTSIG)) {
    printf ("ERROR: FAT12_16: BS_BootSig - %02x, expected - %02x\n",
        FatBpb->Fat12_16.BS_BootSig, FAT_BS_BOOTSIG);
    return FatTypeUnknown;
  }
  if ((FatType == FatTypeFat32) &&
      (FatBpb->Fat32.BS_BootSig != FAT_BS_BOOTSIG)) {
    printf ("ERROR: FAT32: BS_BootSig - %02x, expected - %02x\n",
        FatBpb->Fat32.BS_BootSig, FAT_BS_BOOTSIG);
    return FatTypeUnknown;
  }
  
  if ((FatType == FatTypeFat12) || (FatType == FatTypeFat16)) {
    memcpy (FilSysType, FatBpb->Fat12_16.BS_FilSysType, 8);
    FilSysType[8] = 0;
    if ((FatType == FatTypeFat12) && 
        (strcmp (FilSysType, FAT12_FILSYSTYPE) != 0) &&
        (strcmp (FilSysType, FAT_FILSYSTYPE) != 0)) {
      printf ("WARNING: FAT12: BS_FilSysType - %s, expected - %s, or %s\n",
          FilSysType, FAT12_FILSYSTYPE, FAT_FILSYSTYPE);
    }
    if ((FatType == FatTypeFat16) && 
        (strcmp (FilSysType, FAT16_FILSYSTYPE) != 0) &&
        (strcmp (FilSysType, FAT_FILSYSTYPE) != 0)) {
      printf ("WARNING: FAT16: BS_FilSysType - %s, expected - %s, or %s\n",
          FilSysType, FAT16_FILSYSTYPE, FAT_FILSYSTYPE);
    }
  }
  if (FatType == FatTypeFat32) {
    memcpy (FilSysType, FatBpb->Fat32.BS_FilSysType, 8);
    FilSysType[8] = 0;
    if (strcmp (FilSysType, FAT32_FILSYSTYPE) != 0) {
      printf ("WARNING: FAT32: BS_FilSysType - %s, expected - %s\n",
          FilSysType, FAT32_FILSYSTYPE);
    }
  }

  //
  // pass all check, get FAT type
  //
  return FatType;
}

void
PatchFatBpbInfo (
  IN FAT_BPB_STRUCT  *DestFatBpb,
  IN FAT_BPB_STRUCT  *SourceFatBpb,
  IN FAT_TYPE        OrigFatType,
  IN FAT_TYPE        FatType
  )
{
  FAT_BPB_STRUCT  BackupFatBpb;
  CHAR8           VolLab[11];
  CHAR8           FilSysType[8];

  if (FatType <= FatTypeUnknown || FatType >= FatTypeMax) {
    printf ("ERROR: Unknown Fat Type!\n");
    return;
  }

  memcpy (&BackupFatBpb, DestFatBpb, sizeof(BackupFatBpb));

  printf ("Patching %s BPB:\n", FatTypeToString (FatType));
  if (FatType != FatTypeFat32) {
    memcpy (
      &DestFatBpb->Fat12_16.BPB_BytsPerSec,
      &SourceFatBpb->Fat12_16.BPB_BytsPerSec,
      ((UINTN)&DestFatBpb->Fat12_16.Reserved - (UINTN)&DestFatBpb->Fat12_16.BPB_BytsPerSec)
      );
  } else {
    memcpy (
      &DestFatBpb->Fat32.BPB_BytsPerSec,
      &SourceFatBpb->Fat32.BPB_BytsPerSec,
      ((UINTN)&DestFatBpb->Fat32.Reserved - (UINTN)&DestFatBpb->Fat32.BPB_BytsPerSec)
      );
  }

//  if ((OrigFatType == FatTypeFat12) || (OrigFatType == FatTypeFat16)) {
//    memcpy (VolLab, BackupFatBpb.Fat12_16.BS_VolLab, sizeof(VolLab));
//    memcpy (FilSysType, BackupFatBpb.Fat12_16.BS_FilSysType, sizeof(FilSysType));
//  } else if (OrigFatType == FatTypeFat32) {
//    memcpy (VolLab, BackupFatBpb.Fat32.BS_VolLab, sizeof(VolLab));
//    memcpy (FilSysType, BackupFatBpb.Fat32.BS_FilSysType, sizeof(FilSysType));
//  } else {
    if (FatType == FatTypeFat32) {
      memcpy (VolLab, "EFI FAT32  ", sizeof(VolLab));
      memcpy (FilSysType, FAT32_FILSYSTYPE, sizeof(FilSysType));
    } else if (FatType == FatTypeFat16) {
      memcpy (VolLab, "EFI FAT16  ", sizeof(VolLab));
      memcpy (FilSysType, FAT16_FILSYSTYPE, sizeof(FilSysType));
    } else {
      memcpy (VolLab, "EFI FAT12  ", sizeof(VolLab));
      memcpy (FilSysType, FAT12_FILSYSTYPE, sizeof(FilSysType));
    }
//  }
  if (FatType != FatTypeFat32) {
    memcpy (DestFatBpb->Fat12_16.BS_VolLab, VolLab, sizeof(VolLab));
    memcpy (DestFatBpb->Fat12_16.BS_FilSysType, FilSysType, sizeof(FilSysType));
  } else {
    memcpy (DestFatBpb->Fat32.BS_VolLab, VolLab, sizeof(VolLab));
    memcpy (DestFatBpb->Fat32.BS_FilSysType, FilSysType, sizeof(FilSysType));
  }
  
  DestFatBpb->Fat12_16.Signature = FAT_BS_SIGNATURE;

  //
  // Patch LBAOffsetForBootSector
  //
//  if ((FatType == FatTypeFat16) || (FatType == FatTypeFat32)) {
//    memcpy ((BYTE *)DestFatBpb + BOOT_SECTOR_LBA_OFFSET, (BYTE *)SourceFatBpb + BOOT_SECTOR_LBA_OFFSET, sizeof(DWORD));
//  }

  return ;
}

void
PrintFatBpbInfo (
  IN FAT_BPB_STRUCT  *FatBpb,
  IN FAT_TYPE        FatType
  )
{
  if (FatType <= FatTypeUnknown || FatType >= FatTypeMax) {
    printf ("ERROR: Unknown Fat Type!\n");
    return;
  }

  printf ("\nBoot Sector %s:\n", FatTypeToString (FatType));
  printf ("\n");
  printf ("  Offset Title                        Data\n");
  printf ("==================================================================\n");
  printf ("  0      JMP instruction              %02x %02x %02x\n",
                                                 FatBpb->Fat12_16.BS_jmpBoot[0],
                                                 FatBpb->Fat12_16.BS_jmpBoot[1],
                                                 FatBpb->Fat12_16.BS_jmpBoot[2]);
  printf ("  3      OEM                          %c%c%c%c%c%c%c%c\n",
                                                 FatBpb->Fat12_16.BS_OEMName[0],
                                                 FatBpb->Fat12_16.BS_OEMName[1],
                                                 FatBpb->Fat12_16.BS_OEMName[2],
                                                 FatBpb->Fat12_16.BS_OEMName[3],
                                                 FatBpb->Fat12_16.BS_OEMName[4],
                                                 FatBpb->Fat12_16.BS_OEMName[5],
                                                 FatBpb->Fat12_16.BS_OEMName[6],
                                                 FatBpb->Fat12_16.BS_OEMName[7]);
  printf ("\n");
  printf ("BIOS Parameter Block\n");
  printf ("  B      Bytes per sector             %04x\n", FatBpb->Fat12_16.BPB_BytsPerSec);
  printf ("  D      Sectors per cluster          %02x\n", FatBpb->Fat12_16.BPB_SecPerClus);
  printf ("  E      Reserved sectors             %04x\n", FatBpb->Fat12_16.BPB_RsvdSecCnt);
  printf ("  10     Number of FATs               %02x\n", FatBpb->Fat12_16.BPB_NumFATs);
  printf ("  11     Root entries                 %04x\n", FatBpb->Fat12_16.BPB_RootEntCnt);
  printf ("  13     Sectors (under 32MB)         %04x\n", FatBpb->Fat12_16.BPB_TotSec16);
  printf ("  15     Media descriptor             %02x\n", FatBpb->Fat12_16.BPB_Media);
  printf ("  16     Sectors per FAT (small vol.) %04x\n", FatBpb->Fat12_16.BPB_FATSz16);
  printf ("  18     Sectors per track            %04x\n", FatBpb->Fat12_16.BPB_SecPerTrk);
  printf ("  1A     Heads                        %04x\n", FatBpb->Fat12_16.BPB_NumHeads);
  printf ("  1C     Hidden sectors               %08x\n", FatBpb->Fat12_16.BPB_HiddSec);
  printf ("  20     Sectors (over 32MB)          %08x\n", FatBpb->Fat12_16.BPB_TotSec32);
  printf ("\n");
  if (FatType != FatTypeFat32) {
    printf ("  24     BIOS drive                   %02x\n", FatBpb->Fat12_16.BS_DrvNum);
    printf ("  25     (Unused)                     %02x\n", FatBpb->Fat12_16.BS_Reserved1);
    printf ("  26     Ext. boot signature          %02x\n", FatBpb->Fat12_16.BS_BootSig);
    printf ("  27     Volume serial number         %08x\n", FatBpb->Fat12_16.BS_VolID);
    printf ("  2B     Volume lable                 %c%c%c%c%c%c%c%c%c%c%c\n",
                                                   FatBpb->Fat12_16.BS_VolLab[0],
                                                   FatBpb->Fat12_16.BS_VolLab[1],
                                                   FatBpb->Fat12_16.BS_VolLab[2],
                                                   FatBpb->Fat12_16.BS_VolLab[3],
                                                   FatBpb->Fat12_16.BS_VolLab[4],
                                                   FatBpb->Fat12_16.BS_VolLab[5],
                                                   FatBpb->Fat12_16.BS_VolLab[6],
                                                   FatBpb->Fat12_16.BS_VolLab[7],
                                                   FatBpb->Fat12_16.BS_VolLab[8],
                                                   FatBpb->Fat12_16.BS_VolLab[9],
                                                   FatBpb->Fat12_16.BS_VolLab[10]);
    printf ("  36     File system                  %c%c%c%c%c%c%c%c\n",
                                                   FatBpb->Fat12_16.BS_FilSysType[0],
                                                   FatBpb->Fat12_16.BS_FilSysType[1],
                                                   FatBpb->Fat12_16.BS_FilSysType[2],
                                                   FatBpb->Fat12_16.BS_FilSysType[3],
                                                   FatBpb->Fat12_16.BS_FilSysType[4],
                                                   FatBpb->Fat12_16.BS_FilSysType[5],
                                                   FatBpb->Fat12_16.BS_FilSysType[6],
                                                   FatBpb->Fat12_16.BS_FilSysType[7]);
    printf ("\n");
  } else {
    printf ("FAT32 Section\n");
    printf ("  24     Sectors per FAT (large vol.) %08x\n", FatBpb->Fat32.BPB_FATSz32);
    printf ("  28     Flags                        %04x\n", FatBpb->Fat32.BPB_ExtFlags);
    printf ("  2A     Version                      %04x\n", FatBpb->Fat32.BPB_FSVer);
    printf ("  2C     Root dir 1st cluster         %08x\n", FatBpb->Fat32.BPB_RootClus);
    printf ("  30     FSInfo sector                %04x\n", FatBpb->Fat32.BPB_FSInfo);
    printf ("  32     Backup boot sector           %04x\n", FatBpb->Fat32.BPB_BkBootSec);
    printf ("  34     (Reserved)                   %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                                                   FatBpb->Fat32.BPB_Reserved[0],
                                                   FatBpb->Fat32.BPB_Reserved[1],
                                                   FatBpb->Fat32.BPB_Reserved[2],
                                                   FatBpb->Fat32.BPB_Reserved[3],
                                                   FatBpb->Fat32.BPB_Reserved[4],
                                                   FatBpb->Fat32.BPB_Reserved[5],
                                                   FatBpb->Fat32.BPB_Reserved[6],
                                                   FatBpb->Fat32.BPB_Reserved[7],
                                                   FatBpb->Fat32.BPB_Reserved[8],
                                                   FatBpb->Fat32.BPB_Reserved[9],
                                                   FatBpb->Fat32.BPB_Reserved[10],
                                                   FatBpb->Fat32.BPB_Reserved[11]);
    printf ("\n");
    printf ("  40     BIOS drive                   %02x\n", FatBpb->Fat32.BS_DrvNum);
    printf ("  41     (Unused)                     %02x\n", FatBpb->Fat32.BS_Reserved1);
    printf ("  42     Ext. boot signature          %02x\n", FatBpb->Fat32.BS_BootSig);
    printf ("  43     Volume serial number         %08x\n", FatBpb->Fat32.BS_VolID);
    printf ("  47     Volume lable                 %c%c%c%c%c%c%c%c%c%c%c\n",
                                                   FatBpb->Fat32.BS_VolLab[0],
                                                   FatBpb->Fat32.BS_VolLab[1],
                                                   FatBpb->Fat32.BS_VolLab[2],
                                                   FatBpb->Fat32.BS_VolLab[3],
                                                   FatBpb->Fat32.BS_VolLab[4],
                                                   FatBpb->Fat32.BS_VolLab[5],
                                                   FatBpb->Fat32.BS_VolLab[6],
                                                   FatBpb->Fat32.BS_VolLab[7],
                                                   FatBpb->Fat32.BS_VolLab[8],
                                                   FatBpb->Fat32.BS_VolLab[9],
                                                   FatBpb->Fat32.BS_VolLab[10]);
    printf ("  52     File system                  %c%c%c%c%c%c%c%c\n",
                                                   FatBpb->Fat32.BS_FilSysType[0],
                                                   FatBpb->Fat32.BS_FilSysType[1],
                                                   FatBpb->Fat32.BS_FilSysType[2],
                                                   FatBpb->Fat32.BS_FilSysType[3],
                                                   FatBpb->Fat32.BS_FilSysType[4],
                                                   FatBpb->Fat32.BS_FilSysType[5],
                                                   FatBpb->Fat32.BS_FilSysType[6],
                                                   FatBpb->Fat32.BS_FilSysType[7]);
    printf ("\n");
  }
  printf ("  1FE    Signature                    %04x\n", FatBpb->Fat12_16.Signature);
  printf ("\n");

  return ;
}

void
ParseBootSector (
  char *FileName
  )
{
  FAT_BPB_STRUCT  BootSector;
  FAT_TYPE        FatType;
  
  if (ReadFromFile ((void *)&BootSector, FileName) == 0) {
    return ;
  }
  
  FatType = GetFatType (&BootSector);
  PrintFatBpbInfo (&BootSector, FatType);
  
  return ;
}

void
PatchBootSector (
  char *DestFileName,
  char *SourceFileName,
  char ForcePatch
  )
{
  FAT_BPB_STRUCT  DestBootSector;
  FAT_BPB_STRUCT  SourceBootSector;
  FAT_TYPE        DestFatType;
  FAT_TYPE        SourceFatType;
  
  if (ReadFromFile ((void *)&DestBootSector, DestFileName) == 0) {
    return ;
  }
  if (ReadFromFile ((void *)&SourceBootSector, SourceFileName) == 0) {
    return ;
  }
  
  DestFatType = GetFatType (&DestBootSector);
  SourceFatType = GetFatType (&SourceBootSector);
  if (DestFatType != SourceFatType) {
    if (ForcePatch) {
      printf ("WARNING: FAT type mismatch: Dest - %s, Source - %s\n", FatTypeToString(DestFatType), FatTypeToString(SourceFatType));
    } else {
      printf ("ERROR: FAT type mismatch: Dest - %s, Source - %s\n", FatTypeToString(DestFatType), FatTypeToString(SourceFatType));
      return ;
    }
  }
  PatchFatBpbInfo (&DestBootSector, &SourceBootSector, DestFatType, SourceFatType);

  if (WriteToFile ((void *)&DestBootSector, DestFileName)) {
    printf ("BootSector is patched successfully!\n");
  }

  return ;
}

void
PrintMbrInfo (
  IN MASTER_BOOT_RECORD *Mbr
  )
{
  printf ("\nMaster Boot Record:\n");
  printf ("\n");
  printf ("  Offset Title                        Value\n");
  printf ("==================================================================\n");
  printf ("  0      Master bootstrap loader code (not list)\n");
  printf ("  1B8    Windows disk signature       %08x\n", Mbr->UniqueMbrSignature);
  printf ("\n");
  printf ("Partition Table Entry #1\n");
  printf ("  1BE    80 = active partition        %02x\n", Mbr->PartitionRecord[0].BootIndicator);
  printf ("  1BF    Start head                   %02x\n", Mbr->PartitionRecord[0].StartHead);
  printf ("  1C0    Start sector                 %02x\n", Mbr->PartitionRecord[0].StartSector);
  printf ("  1C1    Start cylinder               %02x\n", Mbr->PartitionRecord[0].StartTrack);
  printf ("  1C2    Partition type indicator     %02x\n", Mbr->PartitionRecord[0].OSType);
  printf ("  1C3    End head                     %02x\n", Mbr->PartitionRecord[0].EndHead);
  printf ("  1C4    End sector                   %02x\n", Mbr->PartitionRecord[0].EndSector);
  printf ("  1C5    End cylinder                 %02x\n", Mbr->PartitionRecord[0].EndTrack);
  printf ("  1C6    Sectors preceding partition  %08x\n", Mbr->PartitionRecord[0].StartingLBA);
  printf ("  1CA    Sectors in partition         %08x\n", Mbr->PartitionRecord[0].SizeInLBA);
  printf ("\n");
  printf ("Partition Table Entry #2\n");
  printf ("  1CE    80 = active partition        %02x\n", Mbr->PartitionRecord[1].BootIndicator);
  printf ("  1CF    Start head                   %02x\n", Mbr->PartitionRecord[1].StartHead);
  printf ("  1D0    Start sector                 %02x\n", Mbr->PartitionRecord[1].StartSector);
  printf ("  1D1    Start cylinder               %02x\n", Mbr->PartitionRecord[1].StartTrack);
  printf ("  1D2    Partition type indicator     %02x\n", Mbr->PartitionRecord[1].OSType);
  printf ("  1D3    End head                     %02x\n", Mbr->PartitionRecord[1].EndHead);
  printf ("  1D4    End sector                   %02x\n", Mbr->PartitionRecord[1].EndSector);
  printf ("  1D5    End cylinder                 %02x\n", Mbr->PartitionRecord[1].EndTrack);
  printf ("  1D6    Sectors preceding partition  %08x\n", Mbr->PartitionRecord[1].StartingLBA);
  printf ("  1DA    Sectors in partition         %08x\n", Mbr->PartitionRecord[1].SizeInLBA);
  printf ("\n");
  printf ("Partition Table Entry #3\n");
  printf ("  1DE    80 = active partition        %02x\n", Mbr->PartitionRecord[2].BootIndicator);
  printf ("  1DF    Start head                   %02x\n", Mbr->PartitionRecord[2].StartHead);
  printf ("  1E0    Start sector                 %02x\n", Mbr->PartitionRecord[2].StartSector);
  printf ("  1E1    Start cylinder               %02x\n", Mbr->PartitionRecord[2].StartTrack);
  printf ("  1E2    Partition type indicator     %02x\n", Mbr->PartitionRecord[2].OSType);
  printf ("  1E3    End head                     %02x\n", Mbr->PartitionRecord[2].EndHead);
  printf ("  1E4    End sector                   %02x\n", Mbr->PartitionRecord[2].EndSector);
  printf ("  1E5    End cylinder                 %02x\n", Mbr->PartitionRecord[2].EndTrack);
  printf ("  1E6    Sectors preceding partition  %08x\n", Mbr->PartitionRecord[2].StartingLBA);
  printf ("  1EA    Sectors in partition         %08x\n", Mbr->PartitionRecord[2].SizeInLBA);
  printf ("\n");
  printf ("Partition Table Entry #4\n");
  printf ("  1EE    80 = active partition        %02x\n", Mbr->PartitionRecord[3].BootIndicator);
  printf ("  1EF    Start head                   %02x\n", Mbr->PartitionRecord[3].StartHead);
  printf ("  1F0    Start sector                 %02x\n", Mbr->PartitionRecord[3].StartSector);
  printf ("  1F1    Start cylinder               %02x\n", Mbr->PartitionRecord[3].StartTrack);
  printf ("  1F2    Partition type indicator     %02x\n", Mbr->PartitionRecord[3].OSType);
  printf ("  1F3    End head                     %02x\n", Mbr->PartitionRecord[3].EndHead);
  printf ("  1F4    End sector                   %02x\n", Mbr->PartitionRecord[3].EndSector);
  printf ("  1F5    End cylinder                 %02x\n", Mbr->PartitionRecord[3].EndTrack);
  printf ("  1F6    Sectors preceding partition  %08x\n", Mbr->PartitionRecord[3].StartingLBA);
  printf ("  1FA    Sectors in partition         %08x\n", Mbr->PartitionRecord[3].SizeInLBA);
  printf ("\n");
  printf ("  1FE    Signature                    %04x\n", Mbr->Signature);
  printf ("\n");

  return ;
}

void
ParseMbr (
  char *FileName
  )
{
  MASTER_BOOT_RECORD  Mbr;
  
  if (ReadFromFile ((void *)&Mbr, FileName) == 0) {
    return ;
  }
  
  PrintMbrInfo (&Mbr);
  
  return ;
}

void
PatchMbrInfo (
  IN MASTER_BOOT_RECORD  *DestMbr,
  IN MASTER_BOOT_RECORD  *SourceMbr
  )
{
  if (SourceMbr->Signature != MBR_SIGNATURE) {
    printf ("ERROR: Invalid MBR!\n");
    return;
  }

  printf ("Patching MBR:\n");
  memcpy (
    &DestMbr->PartitionRecord[0],
    &SourceMbr->PartitionRecord[0],
    sizeof(DestMbr->PartitionRecord)
    );

  DestMbr->Signature = MBR_SIGNATURE;

  return ;
}

void
PatchMbr (
  char *DestFileName,
  char *SourceFileName
  )
{
  MASTER_BOOT_RECORD  DestMbr;
  MASTER_BOOT_RECORD  SourceMbr;
  
  if (ReadFromFile ((void *)&DestMbr, DestFileName) == 0) {
    return ;
  }
  if (ReadFromFile ((void *)&SourceMbr, SourceFileName) == 0) {
    return ;
  }
  
  PatchMbrInfo (&DestMbr, &SourceMbr);

  if (WriteToFile ((void *)&DestMbr, DestFileName)) {
    printf ("MBR is patched successfully!\n");
  }

  return ;
}

void
PrintUsage (
  void
  )
{
  printf (
    "Usage:\n"
    "bootsectimage [-m] -p <boot sector image>\n"
    "bootsectimage [-m] -g <source boot sector image> <dest boot sector image> [-f]\n"
    "  -p: parse <boot sector image>, and print it\n"
    "  -g: get info from <source boot sector image>, and patch to <dest boot sector image>\n"
    "  -f: force patch even FAT type mismatch\n"
    "  -m: process MBR instead of boot sector\n"
    );
}

int
main (
  int argc,
  char *argv[]
  )
{
  char ForcePatch;
  
  if (argc < 3 || argc > 5) {
    PrintUsage ();
    return -1;
  }

  if ((strcmp (argv[1], "-p") == 0) && (argc == 3)) {
    ParseBootSector (argv[2]);
  } else if ((strcmp (argv[1], "-g") == 0) && ((argc == 4) || (argc == 5))) {
    ForcePatch = 0;
    if (argc == 5) {
      if (strcmp (argv[4], "-f") == 0) {
        ForcePatch = 1;
      }
    }
    PatchBootSector (argv[3], argv[2], ForcePatch);
  } else if ((strcmp (argv[1], "-m") == 0) && ((argc == 4) || (argc == 5))) {
    if ((strcmp (argv[2], "-p") == 0) && (argc == 4)) {
      ParseMbr (argv[3]);
    } else if ((strcmp (argv[2], "-g") == 0) && (argc == 5)) {
      PatchMbr (argv[4], argv[3]);
    } else {
      PrintUsage ();
      return -1;
    }
  } else {
    PrintUsage ();
    return -1;
  }

  return 0;
}

