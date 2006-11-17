/*++

Copyright 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  genbootsector.c
  
Abstract:

Revision History

--*/

#include <windows.h>
#include <stdio.h>
#include <string.h>

#define REG_INVALID_PARAMETER 2
#define REG_NOT_EXIST         1
#define REG_SUCCESS           0

HRESULT QueryReg(char *strName, HKEY *phKey)
{
  LRESULT  hr;

  if (phKey == NULL) {
    return REG_INVALID_PARAMETER;
  }
  hr = RegOpenKey(HKEY_LOCAL_MACHINE, strName, phKey);
  if (hr == ERROR_SUCCESS) {
    return REG_SUCCESS;
  } else {
    return REG_NOT_EXIST;
  }
}

HRESULT QueryRegValue(HKEY hKey, char *strName, ULONG dwType, VOID *lpcbData, DWORD cbBin)
{
  DWORD  cbData;
  DWORD  dwTypeLocal;

  dwTypeLocal = dwType;
  switch (dwType) {
  case REG_EXPAND_SZ:
  case REG_SZ:
  case REG_LINK:
  case REG_MULTI_SZ:
  case REG_RESOURCE_LIST:
    cbData = lstrlen ((char *)(lpcbData));
    break;
  case REG_DWORD_BIG_ENDIAN:
  case REG_DWORD_LITTLE_ENDIAN:
    cbData = 4;
    break;
  case REG_BINARY:
    cbData = cbBin;
    break;
  default:
    return REG_INVALID_PARAMETER;
  }
  if (RegQueryValueEx(hKey, strName, NULL, &dwTypeLocal, (LPBYTE)(lpcbData), &cbData) == ERROR_SUCCESS)
  {
    return REG_SUCCESS;
  } else {
    return REG_NOT_EXIST;
  }
}

#define MAX_DRIVE 26
DWORD DiskNumber = 0;
char  DiskName[MAX_DRIVE][MAX_PATH];

typedef enum {
  DriveTypeUnknown,
  DriveTypeFloppy,
  DriveTypeIde,
  DriveTypeIdeMbr,
  DriveTypeUsb,
  DriveTypeUsbMbr,
  DriveTypeCdrom,
  DriveTypeMax
} DRIVE_TYPE;

#define BOOT_SECTOR_LBA_OFFSET 0x1FA

void
InitDrive (
  void
  )
{
  HKEY  pKey;
  DWORD Index;
  char  RegName[MAX_PATH];

  if (QueryReg ("SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum", &pKey) != 0) {
    printf ("QueryReg error\n");
    return ;
  }

  if (QueryRegValue (pKey, "Count", REG_DWORD_LITTLE_ENDIAN, &DiskNumber,sizeof (DWORD)) != 0) {
    printf ("QueryRegValue error\n");
    RegCloseKey(pKey);
    return ;
  }

  for (Index = 0; Index < DiskNumber; Index++) {
    wsprintf (RegName, "%u", Index);
    memset (DiskName[Index], ' ', MAX_PATH - 1);
    DiskName[Index][MAX_PATH - 1] = 0;
    if (QueryRegValue (pKey, RegName, REG_SZ, &DiskName[Index], MAX_PATH) != 0) {
      printf ("QueryRegValue error index %d\n", Index);
      continue;
    }
  }

  RegCloseKey(pKey);
}

void
ListDrive (
  void
  )
{
  DWORD Index;

  InitDrive ();
  printf ("Physical Disk:\n");
  for (Index = 0; Index < DiskNumber; Index++) {
    //
    // Reset Disk Name
    //
    printf ("%d - %s\n", Index, DiskName[Index]);
  }
}

char
CheckDevice (
  DWORD Index,
  char  *DeviceName
  )
{
  char  DriveName[MAX_PATH];

  memset (DriveName, 0, MAX_PATH);
  strcpy (DriveName, DiskName[Index]);
  DriveName[strlen(DeviceName)] = 0;
  if (strcmp (DriveName, DeviceName) == 0) {
    return 1;
  }
  return 0;
}

DRIVE_TYPE
GetDiskType (
  DWORD Index
  )
{
  if (CheckDevice (Index, "USB") == 1) {
    return DriveTypeUsb;
  }
  if (CheckDevice (Index, "IDE") == 1) {
    return DriveTypeIde;
  }
  return DriveTypeUnknown;
}

DWORD
GetFirstUsb (
  void
  )
{
  DWORD Index;

  InitDrive ();
  for (Index = 0; Index < DiskNumber; Index++) {
    if (CheckDevice (Index, "USB") == 1) {
      printf ("Select: %d - %s\n", Index, DiskName[Index]);
      return Index;
    }
  }

  return (DWORD)-1;
}

DWORD
GetFirstIde (
  void
  )
{
  DWORD Index;

  InitDrive ();
  for (Index = 0; Index < DiskNumber; Index++) {
    if (CheckDevice (Index, "IDE") == 1) {
      printf ("Select: %d - %s\n", Index, DiskName[Index]);
      return Index;
    }
  }

  return (DWORD)-1;
}

int WriteToFile (BYTE *BootSector, char *FileName)
{
  FILE *FileHandle;
  int  result;

  FileHandle = fopen (FileName, "wb");
  if (FileHandle == NULL) {
    printf ("error open file: %s\n", FileName);
    return 0;
  }

  result = fwrite (BootSector, 1, 512, FileHandle);
  if (result != 512) {
    printf ("error write file: %s\n", FileName);
    result = 0;
  }

  fclose (FileHandle);
  return result;
}

int ReadFromFile (BYTE *BootSector, char *FileName)
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

DWORD
GetBootSectorOffset (
  HANDLE     hFile,
  char       IsWrite,
  DRIVE_TYPE DriveType
  )
{
  BYTE    DiskPartition[512];
  DWORD   dwbytes;
  DWORD   Offset;
  char    IsMbr;
  DWORD   Index;

  IsMbr = FALSE;
  Offset = 0;
  SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
  if (ReadFile (hFile, DiskPartition, 512, &dwbytes, NULL)) {
    if ((DiskPartition[510] == 0x55) && (DiskPartition[511] == 0xAA)) {
      if (((DiskPartition[0] != 0xEB) || (DiskPartition[2] != 0x90)) &&
          (DiskPartition[0] != 0xE9)) {
        if (((DiskPartition[0x1BE] == 0x0) || (DiskPartition[0x1BE] == 0x80)) &&
            ((DiskPartition[0x1CE] == 0x0) || (DiskPartition[0x1CE] == 0x80)) &&
            ((DiskPartition[0x1DE] == 0x0) || (DiskPartition[0x1DE] == 0x80)) &&
            ((DiskPartition[0x1EE] == 0x0) || (DiskPartition[0x1EE] == 0x80))) {
          //
          // Check Signature, Jmp, and Boot Indicator.
          // if all pass, we assume MBR found.
          //
          IsMbr = TRUE;
        }
      }
    }
  }

  if (IsMbr) {
    //
    // Skip MBR
    //
    printf ("Skip MBR!\n");
    for (Index = 0; Index < 4; Index++) {
      //
      // Found Boot Indicator.
      //
      if (DiskPartition[0x1BE + (Index * 0x10)] == 0x80) {
        Offset = *(DWORD *)&DiskPartition[0x1BE + (Index * 0x10) + 8];
        break;
      }
    }
    //
    // If no boot indicator, we select 1st one.
    // And patch it to MBR.
    //
    if (Index >= 4) {
      Offset = *(DWORD *)&DiskPartition[0x1BE + 8];
      if (IsWrite && (DriveType == DriveTypeUsb)) {
        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
        DiskPartition[0x1BE] = 0x80;
        WriteFile (hFile, DiskPartition, 512, &dwbytes, NULL);
      }
    }
  }

  return Offset;
}

void
ProcessBootSector (
  char *DiskDriverName,
  char *FileName,
  char IsWrite,
  DRIVE_TYPE DriveType
  )
{
  BYTE    DiskPartition[512];
  HANDLE  hFile;
  DWORD   dwbytes;
  DWORD   Offset;

  hFile = CreateFile (DiskDriverName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != INVALID_HANDLE_VALUE) {
    Offset = 0;
    if ((DriveType == DriveTypeIde) || (DriveType == DriveTypeUsb)) {
      //
      // Skip potential MBR
      //
      Offset = GetBootSectorOffset (hFile, IsWrite, DriveType);
      SetFilePointer(hFile, Offset * 512, NULL, FILE_BEGIN);
    }
    if (IsWrite) {
      if (ReadFromFile (DiskPartition, FileName)) {
        if (WriteFile (hFile, DiskPartition, 512, &dwbytes, NULL)) {
          printf ("Write BootSector successfully!\n");
        }
      }
    } else {
      if (ReadFile (hFile, DiskPartition, 512, &dwbytes, NULL)) {
        if (DriveType == DriveTypeIde) {
          //
          // Patch LBAOffsetForBootSector
          //
          *(DWORD *)&DiskPartition [BOOT_SECTOR_LBA_OFFSET] = Offset;
        }
        if (WriteToFile (DiskPartition, FileName)) {
          printf ("Read BootSector successfully!\n");
        }
      }
    } 
    CloseHandle (hFile);
  }
}

void
ProcessDiskBootSector (
  DWORD Index,
  char *FileName,
  char IsWrite
  )
{
  char    DiskDriverName[MAX_PATH];
  DRIVE_TYPE DriveType;

  InitDrive ();

  if (Index >= DiskNumber) {
    printf ("This is no STORAGE disk!\n");
    return ;
  }

  DriveType = GetDiskType (Index);

  if (IsWrite && (DriveType != DriveTypeUsb)) {
    printf ("Update STORAGE is not allowed!\n");
    return ;
  }
  
  wsprintf(DiskDriverName, "\\\\.\\PHYSICALDRIVE%u",Index);

  //
  // Open Physical Disk
  //
  ProcessBootSector (DiskDriverName, FileName, IsWrite, DriveType);

  return ;
}

void
ProcessDiskMbr (
  DWORD Index,
  char *FileName,
  char IsWrite
  )
{
  char    DiskDriverName[MAX_PATH];
  DRIVE_TYPE DriveType;

  InitDrive ();

  if (Index >= DiskNumber) {
    printf ("This is no STORAGE disk!\n");
    return ;
  }

  DriveType = GetDiskType (Index);
  if (DriveType == DriveTypeIde) {
    DriveType = DriveTypeIdeMbr;
  } else if (DriveType == DriveTypeUsb) {
    DriveType = DriveTypeUsbMbr;
  } else {
    printf ("Update MBR is not suported!\n");
    return ;
  }

  if (IsWrite) {
    printf ("Update STORAGE is not allowed!\n");
    return ;
  }
  
  wsprintf(DiskDriverName, "\\\\.\\PHYSICALDRIVE%u",Index);

  //
  // Open Physical Disk
  //
  ProcessBootSector (DiskDriverName, FileName, IsWrite, DriveType);

  return ;
}

void
ProcessFloppyBootSector (
  DWORD Index,
  char *FileName,
  char IsWrite
  )
{
  char    DiskDriverName[MAX_PATH];

  if (Index != 0) {
    printf ("This is no Floppy disk!\n");
    return ;
  }

  wsprintf(DiskDriverName, "\\\\.\\A:");
  printf ("Select: %d - %s\n", Index, DiskDriverName);

  //
  // Open Floppy
  //
  ProcessBootSector (DiskDriverName, FileName, IsWrite, DriveTypeFloppy);

  return ;
}

void
PrintUsage (
  void
  )
{
  printf (
    "Usage:\n"
    "genbootsector -l\n"
    "genbootsector [-m] -r/-w <device number>/-u/-f/-h <file name>\n"
    "  -l: list device\n"
    "  -r: read file to device boot sector\n"
    "  -w: write file to device boot sector\n"
    "  -u: auto select 1st USB drive\n"
    "  -f: auto select 1st Floppy drive\n"
    "  -h: auto select 1st IDE drive\n"
    "  -m: process MBR instead of boot sector\n"
    );
}
 
int
main (
  int argc,
  char *argv[]
  )
{
  DWORD Drive;
  char  IsWrite;

  if (argc < 2 || argc > 5) {
    PrintUsage ();
    return -1;
  }

  if ((strcmp (argv[1], "-l") == 0) && (argc == 2)) {
    ListDrive ();
  } else if (((strcmp (argv[1], "-w") == 0) || (strcmp (argv[1], "-r") == 0)) && (argc == 4)) {
    if (strcmp (argv[1], "-w") == 0) {
      IsWrite = 1;
    } else {
      IsWrite = 0;
    }
    if (strcmp (argv[2], "-u") == 0) {
      Drive = GetFirstUsb ();
      ProcessDiskBootSector (Drive, argv[3], IsWrite);
    } else if (strcmp (argv[2], "-h") == 0) {
      Drive = GetFirstIde ();
      ProcessDiskBootSector (Drive, argv[3], IsWrite);
    } else if (strcmp (argv[2], "-f") == 0) {
      Drive = 0; // hardcode for a:
      ProcessFloppyBootSector (Drive, argv[3], IsWrite);
    } else {
      Drive = atoi (argv[2]);
      ProcessDiskBootSector (Drive, argv[3], IsWrite);
    }
  } else if ((strcmp (argv[1], "-m") == 0) && (argc == 5)) {
    if ((strcmp (argv[2], "-w") == 0) || (strcmp (argv[2], "-r") == 0)) {
      if (strcmp (argv[2], "-w") == 0) {
        IsWrite = 1;
      } else {
        IsWrite = 0;
      }
      if (strcmp (argv[3], "-h") == 0) {
        Drive = GetFirstIde ();
        ProcessDiskMbr (Drive, argv[4], IsWrite);
      } else if (strcmp (argv[3], "-u") == 0) {
        Drive = GetFirstUsb ();
        ProcessDiskMbr (Drive, argv[4], IsWrite);
      } else {
        Drive = atoi (argv[3]);
        ProcessDiskMbr (Drive, argv[4], IsWrite);
      }
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

