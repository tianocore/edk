/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  fwimage.c

Abstract:

  Converts a pe32/pe32+ image to an FW image type

--*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "TianoCommon.h"
#include "EfiImage.h"
#include "EfiUtilityMsgs.c"

#define UTILITY_NAME  "FwImage"

typedef union {
  IMAGE_NT_HEADERS32 PeHeader32;
  IMAGE_NT_HEADERS64 PeHeader64;
} PE_HEADER;

VOID
Usage (
  VOID
  )
{
  printf ("Usage: " UTILITY_NAME "  {-t time-date} {-e} [APPLICATION|BS_DRIVER|RT_DRIVER|SAL_RT_DRIVER|COMBINED_PEIM_DRIVER|SECURITY_CORE|PEI_CORE|PE32_PEIM|RELOCATABLE_PEIM] peimage [outimage]\n");
  printf ("  -t: Add Time Stamp for output image\n");
  printf ("  -e: Not clear ExceptionTable for output image\n");
}

static
STATUS
FCopyFile (
  FILE    *in,
  FILE    *out
  )
{
  ULONG filesize;
  ULONG offset;
  ULONG length;
  UCHAR Buffer[8 * 1024];

  fseek (in, 0, SEEK_END);
  filesize = ftell (in);

  fseek (in, 0, SEEK_SET);
  fseek (out, 0, SEEK_SET);

  offset = 0;
  while (offset < filesize) {
    length = sizeof (Buffer);
    if (filesize - offset < length) {
      length = filesize - offset;
    }

    fread (Buffer, length, 1, in);
    fwrite (Buffer, length, 1, out);
    offset += length;
  }

  if ((ULONG) ftell (out) != filesize) {
    Error (NULL, 0, 0, "write error", NULL);
    return STATUS_ERROR;
  }

  return STATUS_SUCCESS;
}

VOID
ZeroExceptionTable (
  IN FILE              *fpIn,
  IN FILE              *fpOut,
  IN IMAGE_DOS_HEADER  *DosHdr,
  IN PE_HEADER         *PeHdr
  )
{
  UINT32 PdataSize;
  UINT32 PdataOffset;
  UINT32 SectionOffset;
  UINT16 SectionNumber;
  UINT32 SectionNameSize;
  EFI_IMAGE_SECTION_HEADER Section;

  PdataSize   = 0;
  PdataOffset = 0;
  SectionOffset = 0;

  //
  // Search .pdata section
  //
  if (PeHdr->PeHeader32.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    if ((PeHdr->PeHeader32.OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_EXCEPTION) &&
        (PeHdr->PeHeader32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress != 0) &&
        (PeHdr->PeHeader32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size != 0)) {

      PdataOffset = PeHdr->PeHeader32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;
      PdataSize = PeHdr->PeHeader32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size;

      PeHdr->PeHeader32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress = 0;
      PeHdr->PeHeader32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size = 0;

      SectionOffset = sizeof(PeHdr->PeHeader32);
    }
  } else {
    if ((PeHdr->PeHeader64.OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_EXCEPTION) &&
        (PeHdr->PeHeader64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress != 0) &&
        (PeHdr->PeHeader64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size != 0)) {

      PdataOffset = PeHdr->PeHeader64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;
      PdataSize = PeHdr->PeHeader64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size;

      PeHdr->PeHeader64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress = 0;
      PeHdr->PeHeader64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size = 0;

      SectionOffset = sizeof(PeHdr->PeHeader64);
    }
  }

  //
  // RVA == File offset
  //
  if ((PdataSize != 0) && (PdataOffset != 0)) {
    fseek (fpOut, PdataOffset, SEEK_SET);

    while (PdataSize > 0) {
      fputc (0, fpOut);
      PdataSize--;
    }

    //
    // Zero .pdata Section Header Name
    //
    SectionNumber = PeHdr->PeHeader32.FileHeader.NumberOfSections;
    SectionNameSize = sizeof(Section.Name);
    while (SectionNumber > 0) {
      fseek (fpIn, DosHdr->e_lfanew + SectionOffset, SEEK_SET);
      fread (&Section, sizeof (Section), 1, fpIn);
      if (strcmp (Section.Name, ".pdata") == 0) {
        fseek (fpOut, DosHdr->e_lfanew + SectionOffset, SEEK_SET);
        while (SectionNameSize > 0) {
          fputc (0, fpOut);
          SectionNameSize--;
        }
        break;
      }
      SectionNumber--;
      SectionOffset += sizeof(Section);
    }
  }
  
  return ;
}

int
main (
  int  argc,
  char *argv[]
  )
/*++

Routine Description:

  Main function.

Arguments:

  argc - Number of command line parameters.
  argv - Array of pointers to command line parameter strings.

Returns:

  STATUS_SUCCESS - Utility exits successfully.
  STATUS_ERROR   - Some error occurred during execution.

--*/
{
  ULONG             Type;
  PUCHAR            Ext;
  PUCHAR            p;
  PUCHAR            pe;
  PUCHAR            OutImageName;
  UCHAR             outname[500];
  FILE              *fpIn;
  FILE              *fpOut;
  IMAGE_DOS_HEADER  DosHdr;
  PE_HEADER         PeHdr;
  time_t            TimeStamp;
  struct tm         TimeStruct;
  IMAGE_DOS_HEADER  BackupDosHdr;
  ULONG             Index;
  BOOLEAN           TimeStampPresent;
  BOOLEAN           NeedClearExceptionTable;

  SetUtilityName (UTILITY_NAME);
  //
  // Assign to fix compile warning
  //
  OutImageName      = NULL;
  Type              = 0;
  Ext               = 0;
  TimeStamp         = 0;
  TimeStampPresent  = FALSE;
  NeedClearExceptionTable = TRUE;

  //
  // Look for -t time-date option first. If the time is "0", then
  // skip it.
  //
  if ((argc > 2) && !strcmp (argv[1], "-t")) {
    TimeStampPresent = TRUE;
    if (strcmp (argv[2], "0") != 0) {
      //
      // Convert the string to a value
      //
      memset ((char *) &TimeStruct, 0, sizeof (TimeStruct));
      if (sscanf(
          argv[2], "%d/%d/%d,%d:%d:%d",
          &TimeStruct.tm_mon,   /* months since January - [0,11] */
          &TimeStruct.tm_mday,  /* day of the month - [1,31] */
          &TimeStruct.tm_year,  /* years since 1900 */
          &TimeStruct.tm_hour,  /* hours since midnight - [0,23] */
          &TimeStruct.tm_min,   /* minutes after the hour - [0,59] */
          &TimeStruct.tm_sec    /* seconds after the minute - [0,59] */
            ) != 6) {
        Error (NULL, 0, 0, argv[2], "failed to convert to mm/dd/yyyy,hh:mm:ss format");
        return STATUS_ERROR;
      }
      //
      // Now fixup some of the fields
      //
      TimeStruct.tm_mon--;
      TimeStruct.tm_year -= 1900;
      //
      // Sanity-check values?
      // Convert
      //
      TimeStamp = mktime (&TimeStruct);
      if (TimeStamp == (time_t) - 1) {
        Error (NULL, 0, 0, argv[2], "failed to convert time");
        return STATUS_ERROR;
      }
    }
    //
    // Skip over the args
    //
    argc -= 2;
    argv += 2;
  }

  //
  // Look for -e option.
  //
  if ((argc > 1) && !strcmp (argv[1], "-e")) {
    NeedClearExceptionTable = FALSE;
    //
    // Skip over the args
    //
    argc -= 1;
    argv += 1;
  }

  //
  // Check for enough args
  //
  if (argc < 3) {
    Usage ();
    return STATUS_ERROR;
  }

  if (argc == 4) {
    OutImageName = argv[3];
  }
  //
  // Get new image type
  //
  p = argv[1];
  if (*p == '/' || *p == '\\') {
    p += 1;
  }

  if (_stricmp (p, "app") == 0 || _stricmp (p, "APPLICATION") == 0) {
    Type  = EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION;
    Ext   = ".efi";

  } else if (_stricmp (p, "bsdrv") == 0 || _stricmp (p, "BS_DRIVER") == 0) {
    Type  = EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER;
    Ext   = ".efi";

  } else if (_stricmp (p, "rtdrv") == 0 || _stricmp (p, "RT_DRIVER") == 0) {
    Type  = EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER;
    Ext   = ".efi";

  } else if (_stricmp (p, "rtdrv") == 0 || _stricmp (p, "SAL_RT_DRIVER") == 0) {
    Type  = EFI_IMAGE_SUBSYSTEM_SAL_RUNTIME_DRIVER;
    Ext   = ".efi";
  } else if (_stricmp (p, "SECURITY_CORE") == 0) {
    Type  = EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER;
    Ext   = ".sec";
  } else if (_stricmp (p, "peim") == 0 ||
           _stricmp (p, "PEI_CORE") == 0 ||
           _stricmp (p, "PE32_PEIM") == 0 ||
           _stricmp (p, "RELOCATABLE_PEIM") == 0 ||
           _stricmp (p, "combined_peim_driver") == 0
          ) {
    Type  = EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER;
    Ext   = ".pei";
  } else {
    Usage ();
    return STATUS_ERROR;
  }
  //
  // open source file
  //
  fpIn = fopen (argv[2], "rb");
  if (!fpIn) {
    Error (NULL, 0, 0, argv[2], "failed to open input file for reading");
    return STATUS_ERROR;
  }
  //
  // Read the dos & pe hdrs of the image
  //
  fseek (fpIn, 0, SEEK_SET);
  fread (&DosHdr, sizeof (DosHdr), 1, fpIn);
  if (DosHdr.e_magic != IMAGE_DOS_SIGNATURE) {
    Error (NULL, 0, 0, argv[2], "DOS header signature not found in source image");
    fclose (fpIn);
    return STATUS_ERROR;
  }

  fseek (fpIn, DosHdr.e_lfanew, SEEK_SET);
  fread (&PeHdr, sizeof (PeHdr), 1, fpIn);
  if (PeHdr.PeHeader32.Signature != IMAGE_NT_SIGNATURE) {
    Error (NULL, 0, 0, argv[2], "PE header signature not found in source image");
    fclose (fpIn);
    return STATUS_ERROR;
  }
  //
  // open output file
  //
  strcpy (outname, argv[2]);
  pe = NULL;
  for (p = outname; *p; p++) {
    if (*p == '.') {
      pe = p;
    }
  }

  if (!pe) {
    pe = p;
  }

  strcpy (pe, Ext);

  if (!OutImageName) {
    OutImageName = outname;
  }

  fpOut = fopen (OutImageName, "w+b");
  if (!fpOut) {
    Error (NULL, 0, 0, OutImageName, "could not open output file for writing");
    fclose (fpIn);
    return STATUS_ERROR;
  }
  //
  // Copy the file
  //
  if (FCopyFile (fpIn, fpOut) != STATUS_SUCCESS) {
    fclose (fpIn);
    fclose (fpOut);
    return STATUS_ERROR;
  }
  //
  // Zero all unused fields of the DOS header
  //
  memcpy (&BackupDosHdr, &DosHdr, sizeof (DosHdr));
  memset (&DosHdr, 0, sizeof (DosHdr));
  DosHdr.e_magic  = BackupDosHdr.e_magic;
  DosHdr.e_lfanew = BackupDosHdr.e_lfanew;
  fseek (fpOut, 0, SEEK_SET);
  fwrite (&DosHdr, sizeof (DosHdr), 1, fpOut);

  fseek (fpOut, sizeof (DosHdr), SEEK_SET);
  for (Index = sizeof (DosHdr); Index < (ULONG) DosHdr.e_lfanew; Index++) {
    fwrite (&DosHdr.e_cp, 1, 1, fpOut);
  }
  
  //
  // Modify some fields in the PE header
  //

  //
  // TimeDateStamp's offset is fixed for PE32/32+
  //
  if (TimeStampPresent) {
    PeHdr.PeHeader32.FileHeader.TimeDateStamp = (UINT32) TimeStamp;
  }

  //
  // PE32/32+ has different optional header layout
  // Determine format is PE32 or PE32+ before modification
  //
  if (PeHdr.PeHeader32.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    //
    // PE32 image
    //

    //
    // Set appropriate subsystem
    //
    PeHdr.PeHeader32.OptionalHeader.Subsystem = (USHORT) Type;

    //
    // Clear useless fields
    //
    PeHdr.PeHeader32.OptionalHeader.SizeOfStackReserve = 0;
    PeHdr.PeHeader32.OptionalHeader.SizeOfStackCommit  = 0;
    PeHdr.PeHeader32.OptionalHeader.SizeOfHeapReserve  = 0;
    PeHdr.PeHeader32.OptionalHeader.SizeOfHeapCommit   = 0;
  } else if (PeHdr.PeHeader32.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
    //
    // PE32+ image
    //

    //
    // Set appropriate subsystem
    //
    PeHdr.PeHeader64.OptionalHeader.Subsystem = (USHORT) Type;

    //
    // Clear useless fields
    //
    PeHdr.PeHeader64.OptionalHeader.SizeOfStackReserve = 0;
    PeHdr.PeHeader64.OptionalHeader.SizeOfStackCommit  = 0;
    PeHdr.PeHeader64.OptionalHeader.SizeOfHeapReserve  = 0;
    PeHdr.PeHeader64.OptionalHeader.SizeOfHeapCommit   = 0;
  } else {
    Error (NULL, 0, 0, argv[2], "Unsupported PE image");
    fclose (fpIn);
    fclose (fpOut);
    return STATUS_ERROR;
  }
  
  //
  // Zero PDATA section for smaller binary size after compression
  //
  if (NeedClearExceptionTable) {
    ZeroExceptionTable (fpIn, fpOut, &DosHdr, &PeHdr);
  }

  fseek (fpOut, DosHdr.e_lfanew, SEEK_SET);
  if (PeHdr.PeHeader32.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    fwrite (&PeHdr, sizeof (PeHdr.PeHeader32), 1, fpOut);
  } else {
    fwrite (&PeHdr, sizeof (PeHdr.PeHeader64), 1, fpOut);
  }

  //
  // Done
  //
  fclose (fpIn);
  fclose (fpOut);
  //
  // printf ("Created %s\n", OutImageName);
  //
  return STATUS_SUCCESS;
}
