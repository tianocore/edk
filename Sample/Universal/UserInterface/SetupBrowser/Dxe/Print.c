/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Print.c

Abstract:

  Basic Ascii AvSPrintf() function named VSPrint(). VSPrint() enables very
  simple implemenation of SPrint() and Print() to support debug. 

  You can not Print more than EFI_DRIVER_LIB_MAX_PRINT_BUFFER characters at a 
  time. This makes the implementation very simple.

  VSPrint, Print, SPrint format specification has the follwoing form

  %type

  type:
    'S','s' - argument is an Unicode string
    'c' - argument is an ascii character
    '%' - Print a %

--*/

#include "Tiano.h"
#include "efidriverlib.h"
#include "print.h"
#include "efistdarg.h"
#include EFI_PROTOCOL_DEFINITION (Hii)

STATIC
CHAR16  *
GetFlagsAndWidth (
  IN  CHAR16      *Format,
  OUT UINTN       *Flags,
  OUT UINTN       *Width,
  IN OUT  VA_LIST *Marker
  );

STATIC
UINTN
GuidToString (
  IN  EFI_GUID  *Guid,
  IN OUT CHAR16 *Buffer,
  IN  UINTN     BufferSize
  );

UINTN
ValueToString (
  IN  OUT CHAR16  *Buffer,
  IN  BOOLEAN     Flags,
  IN  INT64       Value
  );

UINTN
EFIAPI
VSPrint (
  OUT CHAR16              *StartOfBuffer,
  IN  UINTN               BufferSize,
  IN  CONST CHAR16        *FormatString,
  IN  VA_LIST             Marker
  );

UINTN
_IPrint (
  IN UINTN                            Column,
  IN UINTN                            Row,
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *Out,
  IN CHAR16                           *fmt,
  IN VA_LIST                          args
  )
//
// Display string worker for: Print, PrintAt, IPrint, IPrintAt
//
{
  CHAR16  *Buffer;
  CHAR16  *BackupBuffer;
  UINTN   Index;
  UINTN   PreviousIndex;

  //
  // For now, allocate an arbitrarily long buffer
  //
  Buffer        = EfiLibAllocateZeroPool (0x10000);
  BackupBuffer  = EfiLibAllocateZeroPool (0x10000);
  ASSERT (Buffer);
  ASSERT (BackupBuffer);

  if (Column != (UINTN) -1) {
    Out->SetCursorPosition (Out, Column, Row);
  }

  VSPrint (Buffer, 0x10000, fmt, args);

  Out->Mode->Attribute = Out->Mode->Attribute & 0x7f;

  Out->SetAttribute (Out, Out->Mode->Attribute);

  Index         = 0;
  PreviousIndex = 0;

  do {
    for (; (Buffer[Index] != NARROW_CHAR) && (Buffer[Index] != WIDE_CHAR) && (Buffer[Index] != 0); Index++) {
      BackupBuffer[Index] = Buffer[Index];
    }

    if (Buffer[Index] == 0) {
      break;
    }
    //
    // Null-terminate the temporary string
    //
    BackupBuffer[Index] = 0;

    //
    // Print this out, we are about to switch widths
    //
    Out->OutputString (Out, &BackupBuffer[PreviousIndex]);

    //
    // Preserve the current index + 1, since this is where we will start printing from next
    //
    PreviousIndex = Index + 1;

    //
    // We are at a narrow or wide character directive.  Set attributes and strip it and print it
    //
    if (Buffer[Index] == NARROW_CHAR) {
      //
      // Preserve bits 0 - 6 and zero out the rest
      //
      Out->Mode->Attribute = Out->Mode->Attribute & 0x7f;
      Out->SetAttribute (Out, Out->Mode->Attribute);
    } else {
      //
      // Must be wide, set bit 7 ON
      //
      Out->Mode->Attribute = Out->Mode->Attribute | EFI_WIDE_ATTRIBUTE;
      Out->SetAttribute (Out, Out->Mode->Attribute);
    }

    Index++;

  } while (Buffer[Index] != 0);

  //
  // We hit the end of the string - print it
  //
  Out->OutputString (Out, &BackupBuffer[PreviousIndex]);

  gBS->FreePool (Buffer);
  gBS->FreePool (BackupBuffer);
  return EFI_SUCCESS;
}

UINTN
Print (
  IN CHAR16   *fmt,
  ...
  )
/*++

Routine Description:

    Prints a formatted unicode string to the default console

Arguments:

    fmt         - Format string

Returns:

    Length of string printed to the console

--*/
{
  VA_LIST args;

  VA_START (args, fmt);
  return _IPrint ((UINTN) -1, (UINTN) -1, gST->ConOut, fmt, args);
}

UINTN
PrintString (
  CHAR16       *String
  )
/*++

Routine Description:

  Prints a unicode string to the default console,
  using L"%s" format.

Arguments:

  String      - String pointer.

Returns:

  Length of string printed to the console

--*/
{
  return Print (L"%s", String);
}

UINTN
PrintChar (
  CHAR16       Character
  )
/*++

Routine Description:

  Prints a chracter to the default console,
  using L"%c" format.

Arguments:

  Character   - Character to print.

Returns:

  Length of string printed to the console.

--*/
{
  return Print (L"%c", Character);
}

/*
UINTN
PrintToken (
  IN EFI_HII_HANDLE   Handle,
  IN UINT16           Token,
  IN CHAR16           *Language,
  ...
  )
{
  VA_LIST             args;
  UINTN               NumberOfHiiHandles;
  EFI_HANDLE          *HandleBuffer;
  EFI_HII_PROTOCOL    *Hii;

  //
  // There should only be one HII image
  //
  Status = gBS->LocateHandleBuffer (
                 ByProtocol, 
                 &gEfiHiiProtocolGuid, 
                 NULL,
                 &NumberOfHiiHandles, 
                 &HandleBuffer
                 );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Retrieve the Hii protocol interface
  //
  Status = gBS->HandleProtocol (
                 HandleBuffer[0], 
                 &gEfiHiiProtocolGuid, 
                 &Hii
                 );

  Hii->GetString (Hii, Handle, Token, FALSE, Language, 

  VA_START (args, fmt);
  return _IPrint ((UINTN) -1, (UINTN) -1, gST->ConOut, fmt, args);
}

*/
UINTN
PrintAt (
  IN UINTN     Column,
  IN UINTN     Row,
  IN CHAR16    *fmt,
  ...
  )
/*++

Routine Description:

  Prints a formatted unicode string to the default console, at 
  the supplied cursor position

Arguments:

  Column, Row - The cursor position to print the string at

  fmt         - Format string

Returns:

  Length of string printed to the console

--*/
{
  VA_LIST args;

  VA_START (args, fmt);
  return _IPrint (Column, Row, gST->ConOut, fmt, args);
}

UINTN
PrintStringAt (
  IN UINTN     Column,
  IN UINTN     Row,
  CHAR16       *String
  )
/*++

Routine Description:

  Prints a unicode string to the default console, at 
  the supplied cursor position, using L"%s" format.

Arguments:

  Column, Row - The cursor position to print the string at

  String      - String pointer.

Returns:

  Length of string printed to the console

--*/
{
  return PrintAt (Column, Row, L"%s", String);
}

UINTN
PrintCharAt (
  IN UINTN     Column,
  IN UINTN     Row,
  CHAR16       Character
  )
/*++

Routine Description:

  Prints a chracter to the default console, at 
  the supplied cursor position, using L"%c" format.

Arguments:

  Column, Row - The cursor position to print the string at

  Character   - Character to print.

Returns:

  Length of string printed to the console.

--*/
{
  return PrintAt (Column, Row, L"%c", Character);
}

UINTN
SPrint (
  OUT CHAR16        *Buffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR16  *Format,
  ...
  )
/*++

Routine Description:

  SPrint function to process format and place the results in Buffer.

Arguments:

  Buffer     - Ascii buffer to print the results of the parsing of Format into.

  BufferSize - Maximum number of characters to put into buffer. Zero means no 
               limit.

  Format - Ascii format string see file header for more details.

  ...    - Vararg list consumed by processing Format.

Returns: 

  Number of characters printed.

--*/
{
  UINTN   Return;
  VA_LIST Marker;

  VA_START (Marker, Format);
  Return = VSPrint (Buffer, BufferSize, Format, Marker);
  VA_END (Marker);

  return Return;
}

UINTN
EFIAPI
VSPrint (
  OUT CHAR16        *StartOfBuffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR16  *FormatString,
  IN  VA_LIST       Marker
  )
/*++

Routine Description:

  VSPrint function to process format and place the results in Buffer. Since a 
  VA_LIST is used this rountine allows the nesting of Vararg routines. Thus 
  this is the main print working routine

Arguments:

  StartOfBuffer - Unicode buffer to print the results of the parsing of Format into.

  BufferSize    - Maximum number of characters to put into buffer. Zero means 
                  no limit.

  FormatString  - Unicode format string see file header for more details.

  Marker        - Vararg list consumed by processing Format.

Returns: 

  Number of characters printed.

--*/
{
  CHAR16    TmpBuffer[40];
  CHAR16    *Buffer;
  CHAR16    *UnicodeStr;
  CHAR16    *Format;
  UINTN     Index;
  UINTN     Flags;
  UINTN     Width;
  UINT64    Value;
  EFI_GUID  *TmpGUID;

  //
  // Process the format string. Stop if Buffer is over run.
  //
  Buffer  = StartOfBuffer;
  Format  = (CHAR16 *) FormatString;
  for (Index = 0; (*Format != '\0') && (Index < BufferSize - 1); Format++) {
    if (*Format != '%') {
      if (*Format == '\n' && Index < BufferSize - 2) {
        //
        // If carage return add line feed
        //
        Buffer[Index++] = '\r';
      }

      Buffer[Index++] = *Format;
      continue;
    }
    //
    // Now it's time to parse what follows after %
    //
    Format = GetFlagsAndWidth (Format, &Flags, &Width, &Marker);
    switch (*Format) {
    case 'x':
      if ((Flags & LONG_TYPE) == LONG_TYPE) {
        Value = VA_ARG (Marker, UINT64);
      } else {
        Value = VA_ARG (Marker, UINTN);
      }

      if (Width > 39) {
        Width = 39;
      }

#if 0
      Index += EfiValueToHexStr (&Buffer[Index], Value, Flags, Width);
      continue;
#else
      EfiValueToHexStr (TmpBuffer, Value, Flags, Width);
      UnicodeStr = TmpBuffer;
      break;
#endif

    case 'd':
      if ((Flags & LONG_TYPE) == LONG_TYPE) {
        Value = VA_ARG (Marker, UINT64);
      } else {
        Value = (UINTN) VA_ARG (Marker, UINTN);
      }

#if 0
      Index += ValueToString (&Buffer[Index], FALSE, Value);
      continue;
#else
      ValueToString (TmpBuffer, FALSE, Value);
      UnicodeStr = TmpBuffer;
      break;
#endif

    case 's':
    case 'S':
      UnicodeStr = (CHAR16 *) VA_ARG (Marker, CHAR16 *);

      if (UnicodeStr == NULL) {
        UnicodeStr = L"<null string>";
      }

      break;

    case 'c':
      Buffer[Index++] = (CHAR16) VA_ARG (Marker, UINTN);
      continue;

    case 'g':
      TmpGUID = VA_ARG (Marker, EFI_GUID *);

      if (TmpGUID != NULL) {
        Index += GuidToString (
                  TmpGUID,
                  &Buffer[Index],
                  BufferSize - Index
                  );
      }

      continue;

    case '%':
    //
    // Fall through...
    //
    default:
      //
      // if the type is unknown print it to the screen
      //
      Buffer[Index++] = *Format;
      continue;
    }

    for (; *UnicodeStr != '\0' && Index < BufferSize - 1; UnicodeStr++) {
      Buffer[Index++] = *UnicodeStr;
    }
  }

  Buffer[Index++] = '\0';

  return &Buffer[Index] - StartOfBuffer;
}

STATIC
CHAR16 *
GetFlagsAndWidth (
  IN  CHAR16      *Format,
  OUT UINTN       *Flags,
  OUT UINTN       *Width,
  IN OUT  VA_LIST *Marker
  )
/*++

Routine Description:

  VSPrint worker function that parses flag and width information from the 
  Format string and returns the next index into the Format string that needs
  to be parsed. See file headed for details of Flag and Width.

Arguments:

  Format - Current location in the VSPrint format string.

  Flags  - Returns flags

  Width  - Returns width of element

  Marker - Vararg list that may be paritally consumed and returned.

Returns: 

  Pointer indexed into the Format string for all the information parsed
  by this routine.

--*/
{
  UINTN   Count;
  BOOLEAN Done;

  *Flags  = 0;
  *Width  = 0;
  for (Done = FALSE; !Done;) {
    Format++;

    switch (*Format) {

    case '0':
      *Flags |= PREFIX_ZERO;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      Count = 0;
      do {
        Count = (Count * 10) +*Format - '0';
        Format++;
      } while ((*Format >= '0') && (*Format <= '9'));
      Format--;
      *Width = Count;
      break;

    default:
      Done = TRUE;
    }
  }

  return Format;
}

STATIC
UINTN
GuidToString (
  IN  EFI_GUID  *Guid,
  IN  CHAR16    *Buffer,
  IN  UINTN     BufferSize
  )
/*++

Routine Description:

  VSPrint worker function that prints an EFI_GUID.

Arguments:

  Guid       - Pointer to GUID to print.

  Buffer     - Buffe to print Guid into.
  
  BufferSize - Size of Buffer.

Returns: 

  Number of characters printed.  

--*/
{
  UINTN Size;

  Size = SPrint (
          Buffer,
          BufferSize,
          L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
          (UINTN)Guid->Data1,
          (UINTN)Guid->Data2,
          (UINTN)Guid->Data3,
          (UINTN)Guid->Data4[0],
          (UINTN)Guid->Data4[1],
          (UINTN)Guid->Data4[2],
          (UINTN)Guid->Data4[3],
          (UINTN)Guid->Data4[4],
          (UINTN)Guid->Data4[5],
          (UINTN)Guid->Data4[6],
          (UINTN)Guid->Data4[7]
          );

  //
  // SPrint will null terminate the string. The -1 skips the null
  //
  return Size - 1;
}

UINTN
ValueToString (
  IN  OUT CHAR16  *Buffer,
  IN  BOOLEAN     Flags,
  IN  INT64       Value
  )
/*++

Routine Description:

  VSPrint worker function that prints a Value as a decimal number in Buffer

Arguments:

  Buffer - Location to place ascii decimal number string of Value.

  Value  - Decimal value to convert to a string in Buffer.

  Flags  - Flags to use in printing decimal string, see file header for details.

Returns: 

  Number of characters printed.  

--*/
{
  CHAR16  TempBuffer[30];
  CHAR16  *TempStr;
  CHAR16  *BufferPtr;
  UINTN   Count;
  UINTN   Remainder;

  TempStr   = TempBuffer;
  BufferPtr = Buffer;
  Count     = 0;

  if (Value < 0) {
    *(BufferPtr++)  = '-';
    Value           = -Value;
    Count++;
  }

  do {
    Value         = (INT64) DivU64x32 ((UINT64) Value, 10, &Remainder);
    *(TempStr++)  = (CHAR16) (Remainder + '0');
    Count++;
    if ((Flags & COMMA_TYPE) == COMMA_TYPE) {
      if (Count % 3 == 0) {
        *(TempStr++) = ',';
      }
    }
  } while (Value != 0);

  //
  // Reverse temp string into Buffer.
  //
  while (TempStr != TempBuffer) {
    *(BufferPtr++) = *(--TempStr);
  }

  *BufferPtr = 0;
  return Count;
}
