/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    EfiCommonLib.h

Abstract:

  Light weight lib to support EFI drivers.

--*/

#ifndef _EFI_COMMON_LIB_H_
#define _EFI_COMMON_LIB_H_

EFI_STATUS
EfiLibGetSystemConfigurationTable (
  IN EFI_GUID *TableGuid,
  IN OUT VOID **Table
  );

//
//ASPrint and AvSPrint definitions you must include the specific library
//to get the expected behavior from the two functions
//PEI:  PeiLib
//Graphics:  Dxe\Graphics\Unicode  Dxe\Graphics\ASCII
//ASCII: Dxe\Print\ASCII
//Unicode: Dxe\Print\Unicode
//
UINTN
ASPrint (
  OUT CHAR8       *Buffer,
  IN UINTN        BufferSize,
  IN CONST CHAR8  *Format,
  ...
  );

UINTN
AvSPrint (
  OUT CHAR8       *StartOfBuffer,
  IN  UINTN       StrSize,
  IN  CONST CHAR8 *Format,
  IN  VA_LIST     Marker
  );


//
// Lib functions which can be used in both PEI and DXE pahse
//
EFI_STATUS
EfiInitializeCommonDriverLib (
  IN EFI_HANDLE   ImageHandle,
  IN VOID         *SystemTable
  );

EFI_STATUS
EfiCommonIoRead (
  IN  UINT8       Width,
  IN  UINTN       Address,
  IN  UINTN       Count,
  IN  OUT VOID    *Buffer
  );

EFI_STATUS
EfiCommonIoWrite (
  IN  UINT8       Width,
  IN  UINTN       Address,
  IN  UINTN       Count,
  IN  OUT VOID    *Buffer
  );

EFI_STATUS
EfiCommonPciRead (
  IN  UINT8       Width,
  IN  UINT64      Address,
  IN  UINTN       Count,
  IN  OUT VOID    *Buffer
  );

EFI_STATUS
EfiCommonPciWrite (
  IN  UINT8       Width,
  IN  UINT64      Address,
  IN  UINTN       Count,
  IN  OUT VOID    *Buffer
  );

BOOLEAN
EfiCompareGuid (
  IN EFI_GUID *Guid1,
  IN EFI_GUID *Guid2
  );


VOID
EfiCommonLibSetMem (
  IN VOID   *Buffer,
  IN UINTN  Size,
  IN UINT8  Value
  );

VOID
EfiCommonLibCopyMem (
  IN VOID     *Destination,
  IN VOID     *Source,
  IN UINTN    Length
  );

INTN
EfiCompareMem (
  IN VOID     *MemOne,
  IN VOID     *MemTwo,
  IN UINTN    Len
  );

VOID
EfiCommonLibZeroMem (
  IN VOID     *Buffer,
  IN UINTN    Size
  );



//
// Min Max
//
#define EFI_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define EFI_MAX(a,b) (((a) > (b)) ? (a) : (b))


//
// Align a pointer. The pointer represented by ptr is aligned to the bound.
// The resulting pointer is always equal or greater (by no more than bound-1)
// than the ptr. I.e., if the ptr is already aligned, the result will be equal to ptr.
// Valid values for bound are powers of two: 2, 4, 8, 16, 32 etc.
// The returned pointer is VOID* this assignment-compatible with all pointer types.
//
#define EFI_ALIGN(ptr,bound)  ((VOID*)(((UINTN)(ptr) + ((UINTN)(bound) - 1)) & ~((UINTN)(bound) - 1)))

//
// Alignment tests.
//
#define EFI_UINTN_ALIGN_MASK  (sizeof (UINTN) - 1)
#define EFI_UINTN_ALIGNED(ptr) (((UINTN)(ptr)) & EFI_UINTN_ALIGN_MASK)


//
// Integer division with rounding to the nearest rather than truncating.
// For example 8/3=2 but EFI_IDIV_ROUND(8,3)=3. 1/3=0 and EFI_IDIV_ROUND(1,3)=0.
// A half is rounded up e.g., EFI_IDIV_ROUND(1,2)=1 but 1/2=0.
//
#define EFI_IDIV_ROUND(r,s)   ((r)/(s) + (((2 * ((r) % (s))) < (s)) ? 0 : 1))


//
// ReportStatusCode.c init
//

VOID *
EfiConstructStatusCodeData (
  IN  UINT16                    DataSize,
  IN  EFI_GUID                  *TypeGuid,
  IN OUT  EFI_STATUS_CODE_DATA  *Data
  );


EFI_STATUS
EfiDebugVPrintWorker (
  IN  UINTN                   ErrorLevel,
  IN  CHAR8                   *Format,
  IN  VA_LIST                 Marker,
  IN  UINTN                   BufferSize,
  IN OUT VOID                 *Buffer
  );

EFI_STATUS
EfiDebugAssertWorker (
  IN CHAR8                    *FileName,
  IN INTN                     LineNumber,
  IN CHAR8                    *Description,
  IN UINTN                    BufferSize,
  IN OUT VOID                 *Buffer
  );

BOOLEAN
ReportStatusCodeExtractAssertInfo (
  IN EFI_STATUS_CODE_TYPE     CodeType,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN EFI_STATUS_CODE_DATA     *Data,
  OUT CHAR8                   **Filename,
  OUT CHAR8                   **Description,
  OUT UINT32                  *LineNumber
  );

BOOLEAN
ReportStatusCodeExtractDebugInfo (
  IN EFI_STATUS_CODE_DATA     *Data,
  OUT UINT32                  *ErrorLevel,
  OUT VA_LIST                 *Marker,
  OUT CHAR8                   **Format
  );


BOOLEAN
CodeTypeToPostCode (
  IN  EFI_STATUS_CODE_TYPE    CodeType,
  IN EFI_STATUS_CODE_VALUE    Value,
  OUT UINT8                   *PostCode
  );



//
// math.c
//

UINT64
MultU64x32 (
  IN  UINT64  Multiplicand,
  IN  UINTN   Multiplier
  );

UINT64
DivU64x32 (
  IN  UINT64  Dividend,
  IN  UINTN   Divisor,
  OUT UINTN   *Remainder  OPTIONAL
  );

UINT64
RShiftU64 (
  IN  UINT64  Operand,
  IN  UINTN   Count
  );

UINT64
LShiftU64 (
  IN  UINT64  Operand,
  IN  UINTN   Count
  );

UINT64
Power10U64 (
  IN UINT64   Operand,
  IN UINTN    Power
  );

UINT8
Log2 (
  IN UINT64   Operand
  );

UINT32  
GetPowerOfTwo (
  IN  UINT32  Input
  );

//
// Unicode String primatives
//

VOID
EfiStrCpy (
  IN CHAR16   *Destination,
  IN CHAR16   *Source
  );

UINTN
EfiStrLen (
  IN CHAR16   *String
  );

UINTN
EfiStrSize (
  IN CHAR16   *String
  );

INTN
EfiStrCmp (
  IN CHAR16   *String,
  IN CHAR16   *String2
  );

VOID
EfiStrCat (
  IN CHAR16   *Destination,
  IN CHAR16   *Source
  );

UINTN
EfiAsciiStrLen (
  IN CHAR8   *String
  );

CHAR8 *
EfiAsciiStrCpy (
  IN CHAR8    *Destination,
  IN CHAR8    *Source
  );

//
// Print primitives
//

#define LEFT_JUSTIFY    0x01
#define PREFIX_SIGN     0x02
#define PREFIX_BLANK    0x04
#define COMMA_TYPE      0x08
#define LONG_TYPE       0x10
#define PREFIX_ZERO     0x20

//
// Length of temp string buffer to store value string.
//
#define CHARACTER_NUMBER_FOR_VALUE  30

UINTN
EfiValueToHexStr (
  IN  OUT CHAR16  *Buffer,
  IN  UINT64      Value,
  IN  UINTN       Flags,
  IN  UINTN       Width
  );

UINTN
EfiValueToString (
  IN  OUT CHAR16  *Buffer,
  IN  INT64       Value,
  IN  UINTN       Flags,
  IN  UINTN       Width
  );

BOOLEAN
IsHexDigit (
  OUT UINT8      *Digit,
  IN  CHAR16      Char
  )
/*++

  Routine Description:
    Determines if a Unicode character is a hexadecimal digit.
    The test is case insensitive.

  Arguments:
    Digit - Pointer to byte that receives the value of the hex character.
    Char  - Unicode character to test.

  Returns:
    TRUE  - If the character is a hexadecimal digit.
    FALSE - Otherwise.

--*/
;

CHAR16
NibbleToHexChar (
  UINT8 Nibble
  )
/*++

  Routine Description:
    Converts the low nibble of a byte  to hex unicode character.

  Arguments:
    Nibble - lower nibble of a byte.

  Returns:
    Hex unicode character.

--*/
;

EFI_STATUS
HexStringToBuf (
  IN OUT UINT8                     *Buf,   
  IN OUT UINTN                    *Len,
  IN     CHAR16                    *Str,
  OUT    UINTN                     *ConvertedStrLen  OPTIONAL
  )
/*++

  Routine Description:
    Converts Unicode string to binary buffer.
    The conversion may be partial.
    The first character in the string that is not hex digit stops the conversion.
    At a minimum, any blob of data could be represented as a hex string.

  Arguments:
    Buf    - Pointer to buffer that receives the data.
    Len    - Length in bytes of the buffer to hold converted data.
                If routine return with EFI_SUCCESS, containing length of converted data.
                If routine return with EFI_BUFFER_TOO_SMALL, containg length of buffer desired.
    Str    - String to be converted from.
    ConvertedStrLen - Length of the Hex String consumed.

  Returns:
    EFI_SUCCESS: Routine Success.
    EFI_BUFFER_TOO_SMALL: The buffer is too small to hold converted data.
    EFI_

--*/
;

EFI_STATUS
BufToHexString (
  IN OUT CHAR16                    *Str,
  IN OUT UINTN                     *HexStringBufferLength,
  IN     UINT8                     *Buf,
  IN     UINTN                      Len
  )
/*++

  Routine Description:
    Converts binary buffer to Unicode string.
    At a minimum, any blob of data could be represented as a hex string.

  Arguments:
    Str - Pointer to the string.
    HexStringBufferLength - Length in bytes of buffer to hold the hex string. Includes tailing '\0' character.
                                        If routine return with EFI_SUCCESS, containing length of hex string buffer.
                                        If routine return with EFI_BUFFER_TOO_SMALL, containg length of hex string buffer desired.
    Buf - Buffer to be converted from.
    Len - Length in bytes of the buffer to be converted.

  Returns:
    EFI_SUCCESS: Routine success.
    EFI_BUFFER_TOO_SMALL: The hex string buffer is too small.

--*/
;



VOID
EfiStrTrim (
  IN OUT CHAR16   *str,
  IN     CHAR16   CharC
  );
  
#endif
