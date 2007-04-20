/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EdbSupportString.c
  
Abstract:


--*/

#include "Edb.h"

UINTN
Xtoi (
  CHAR16  *str
  )
/*++

Routine Description:

  Convert hex string to uint

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UINTN   u;
  CHAR16  c;
  UINTN   m;
  
  EFI_DEBUGGER_ASSERT (str != NULL);
  
  m = (UINTN) -1 >> 4;
  //
  // skip preceeding white space
  //
  while (*str && *str == ' ') {
    str += 1;
  }
  //
  // skip preceeding zeros
  //
  while (*str && *str == '0') {
    str += 1;
  }
  //
  // skip preceeding white space
  //
  if (*str && (*str == 'x' || *str == 'X')) {
    str += 1;
  }
  //
  // convert hex digits
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= 'a' && c <= 'f') {
      c -= 'a' - 'A';
    }

    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
      if (u > m) {
        return (UINTN) -1;
      }

      u = u << 4 | c - (c >= 'A' ? 'A' - 10 : '0');
    } else {
      break;
    }

    c = *(str++);
  }

  return u;
}

UINT64
LXtoi (
  CHAR16  *str
  )
/*++

Routine Description:

  Convert hex string to uint

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UINT64  u;
  CHAR16  c;
  UINT64  m;
  
  EFI_DEBUGGER_ASSERT (str != NULL);
  
  m = RShiftU64 ((UINT64) -1, 4);
  //
  // skip preceeding white space
  //
  while (*str && *str == ' ') {
    str += 1;
  }
  //
  // skip preceeding zeros
  //
  while (*str && *str == '0') {
    str += 1;
  }
  //
  // skip preceeding white space
  //
  if (*str && (*str == 'x' || *str == 'X')) {
    str += 1;
  }
  //
  // convert hex digits
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= 'a' && c <= 'f') {
      c -= 'a' - 'A';
    }

    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
      if (u > m) {
        return (UINT64) -1;
      }

      u = LShiftU64 (u, 4);
      u = u + (c - (c >= 'A' ? 'A' - 10 : '0'));
    } else {
      break;
    }

    c = *(str++);
  }

  return u;
}

UINTN
Atoi (
  CHAR16  *str
  )
/*++

Routine Description:

  Convert hex string to uint

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UINTN   u;
  CHAR16  c;
  UINTN   m;
  UINTN   n;
  
  EFI_DEBUGGER_ASSERT (str != NULL);
  
  m = (UINTN) -1 / 10;
  n = (UINTN) -1 % 10;
  //
  // skip preceeding white space
  //
  while (*str && *str == ' ') {
    str += 1;
  }
  //
  // convert digits
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= '0' && c <= '9') {
      if (u > m || u == m && c - '0' > (INTN) n) {
        return (UINTN) -1;
      }

      u = (u * 10) + c - '0';
    } else {
      break;
    }

    c = *(str++);
  }

  return u;
}

UINTN
AsciiXtoi (
  CHAR8  *str
  )
/*++

Routine Description:

  Convert hex string to uint

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UINTN   u;
  CHAR8   c;
  UINTN   m;
  
  EFI_DEBUGGER_ASSERT (str != NULL);
  
  m = (UINTN) -1 >> 4;
  //
  // skip preceeding white space
  //
  while (*str && *str == ' ') {
    str += 1;
  }
  //
  // skip preceeding zeros
  //
  while (*str && *str == '0') {
    str += 1;
  }
  //
  // skip preceeding white space
  //
  if (*str && (*str == 'x' || *str == 'X')) {
    str += 1;
  }
  //
  // convert hex digits
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= 'a' && c <= 'f') {
      c -= 'a' - 'A';
    }

    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
      if (u > m) {
        return (UINTN) -1;
      }

      u = u << 4 | c - (c >= 'A' ? 'A' - 10 : '0');
    } else {
      break;
    }

    c = *(str++);
  }

  return u;
}

UINTN
AsciiAtoi (
  CHAR8  *str
  )
/*++

Routine Description:

  Convert hex string to uint

Arguments:

  Str  -  The string
  
Returns:

--*/
{
  UINTN   u;
  CHAR8   c;
  UINTN   m;
  UINTN   n;
  
  EFI_DEBUGGER_ASSERT (str != NULL);
  
  m = (UINTN) -1 / 10;
  n = (UINTN) -1 % 10;
  //
  // skip preceeding white space
  //
  while (*str && *str == ' ') {
    str += 1;
  }
  //
  // convert digits
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= '0' && c <= '9') {
      if (u > m || u == m && c - '0' > (INTN) n) {
        return (UINTN) -1;
      }

      u = (u * 10) + c - '0';
    } else {
      break;
    }

    c = *(str++);
  }

  return u;
}

STATIC
CHAR16
UnicodeToUpper (
  IN      CHAR16                    Chr
  )
{
  return (Chr >= L'a' && Chr <= L'z') ? Chr - (L'a' - L'A') : Chr;
}

INTN
EfiStriCmp (
  IN CHAR16   *String,
  IN CHAR16   *String2
  )
/*++

Routine Description:
  Compare the Unicode string pointed by String to the string pointed by String2.

Arguments:
  String - String to process

  String2 - The other string to process

Returns:
  Return a positive integer if String is lexicall greater than String2; Zero if 
  the two strings are identical; and a negative interger if String is lexically 
  less than String2.

--*/
{
  while ((*String != L'\0') &&
         (UnicodeToUpper (*String) == UnicodeToUpper (*String2))) {
    String++;
    String2++;
  }

  return UnicodeToUpper (*String) - UnicodeToUpper (*String2);
}

STATIC
CHAR8
AsciiToUpper (
  IN      CHAR8                     Chr
  )
{
  return (Chr >= 'a' && Chr <= 'z') ? Chr - ('a' - 'A') : Chr;
}

INTN
EfiAsciiStriCmp (
  IN CHAR8   *String,
  IN CHAR8   *String2
  )
/*++

Routine Description:
  Compare the Ascii string pointed by String to the string pointed by String2.

Arguments:
  String - String to process

  String2 - The other string to process

Returns:
  Return a positive integer if String is lexicall greater than String2; Zero if 
  the two strings are identical; and a negative interger if String is lexically 
  less than String2.

--*/
{
  while ((*String != '\0') &&
         (AsciiToUpper (*String) == AsciiToUpper (*String2))) {
    String++;
    String2++;
  }

  return AsciiToUpper (*String) - AsciiToUpper (*String2);
}

INTN
EfiStrCmpUnicodeAndAscii (
  IN CHAR16   *String,
  IN CHAR8    *String2
  )
/*++

Routine Description:
  Compare the Unicode and Ascii string pointed by String to the string pointed by String2.

Arguments:
  String - Unicode String to process

  String2 - Ascii string to process

Returns:
  Return a positive integer if String is lexicall greater than String2; Zero if 
  the two strings are identical; and a negative interger if String is lexically 
  less than String2.

--*/
{
  while (*String) {
    if (*String != (CHAR16)*String2) {
      break;
    }

    String += 1;
    String2 += 1;
  }

  return (*String - (CHAR16)*String2);
}

INTN
EfiStriCmpUnicodeAndAscii (
  IN CHAR16   *String,
  IN CHAR8    *String2
  )
/*++

Routine Description:
  Compare the Unicode and Ascii string pointed by String to the string pointed by String2.

Arguments:
  String - Unicode String to process

  String2 - Ascii string to process

Returns:
  Return a positive integer if String is lexicall greater than String2; Zero if 
  the two strings are identical; and a negative interger if String is lexically 
  less than String2.

--*/
{
  while ((*String != L'\0') &&
         (UnicodeToUpper (*String) == (CHAR16)AsciiToUpper (*String2))) {
    String++;
    String2++;
  }

  return UnicodeToUpper (*String) - (CHAR16)AsciiToUpper (*String2);
}

INTN
AsciiStrnCmp (
  IN      CHAR8               *FirstString,
  IN      CHAR8               *SecondString,
  IN      UINTN               Length
  )
{
  if (Length == 0) {
    return 0;
  }

  while ((*FirstString != '\0') &&
         (*FirstString == *SecondString) &&
         (Length > 1)) {
    FirstString++;
    SecondString++;
    Length--;
  }
  return *FirstString - *SecondString;
}

BOOLEAN
StrEndWith (
  IN CHAR16                       *Str,
  IN CHAR16                       *SubStr
  )
/*++

Routine Description:

  Verify if the string is end with the sub string.

--*/
{
  CHAR16  *Temp;

  if ((Str == NULL) || (SubStr == NULL) || (EfiStrLen(Str) < EfiStrLen(SubStr))) {
    return FALSE;
  }

  Temp = Str + EfiStrLen(Str) - EfiStrLen(SubStr);

  //
  // Compare
  //
  if (EfiStriCmp (Temp, SubStr) == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

CHAR16 *
StrDuplicate (
  IN CHAR16   *Src
  )
// duplicate a string
{
  CHAR16      *Dest;
  UINTN       Size;

  Size = (EfiStrLen(Src) + 1) * sizeof(CHAR16);
  Dest = EfiLibAllocateZeroPool (Size);
  if (Dest) {
    EfiCopyMem (Dest, Src, Size);
  }
  return Dest;
}


CHAR16  *mLineBuffer          = NULL;
CHAR16  *mFieldBuffer         = NULL;

UINTN
StrSpn (
  IN CHAR16                       *String,
  IN CHAR16                       *CharSet
  )
/*++

Routine Description:

  Find the first substring.

--*/
{
  UINTN   Count;
  CHAR16  *Str1;
  CHAR16  *Str2;

  Count = 0;

  for (Str1 = String; *Str1 != L'\0'; Str1 ++) {
    for (Str2 = CharSet; *Str2 != L'\0'; Str2 ++) {
      if (*Str1 == *Str2) {
        break;
      }
    }

    if (*Str2 == L'\0') {
      return Count;
    }

    Count ++;
  }

  return Count;
}


CHAR16 *
StrBrk (
  IN CHAR16                       *String,
  IN CHAR16                       *CharSet
  )
/*++

Routine Description:

  Searches a string for the first occurrence of a character contained in a
  specified buffer.

--*/
{
  CHAR16  *Str1;
  CHAR16  *Str2;

  for (Str1 = String; *Str1 != L'\0'; Str1 ++) {
    for (Str2 = CharSet; *Str2 != L'\0'; Str2 ++) {
      if (*Str1 == *Str2) {
        return (CHAR16 *) Str1;
      }
    }
  }

  return NULL;
}

CHAR16 *
StrTokenLine (
  IN CHAR16                       *String OPTIONAL,
  IN CHAR16                       *CharSet
  )
/*++

Routine Description:

  Find the next token after one or more specified characters.

--*/
{
  CHAR16  *Begin;
  CHAR16  *End;

  Begin = (String == NULL) ? mLineBuffer : String;
  if (Begin == NULL) {
    return NULL;
  }

  Begin += StrSpn (Begin, CharSet);
  if (*Begin == L'\0') {
    mLineBuffer = NULL;
    return NULL;
  }

  End = StrBrk (Begin, CharSet);
  if ((End != NULL) && (*End != L'\0')) {
    *End = L'\0';
    End ++;
  }

  mLineBuffer = End;
  return Begin;
}


CHAR16 *
StrTokenField (
  IN CHAR16                       *String OPTIONAL,
  IN CHAR16                       *CharSet
  )
/*++

Routine Description:

  Find the next token after one specificed characters.

--*/
{
  CHAR16  *Begin;
  CHAR16  *End;


  Begin = (String == NULL) ? mFieldBuffer : String;
  if (Begin == NULL) {
    return NULL;
  }

  if (*Begin == L'\0') {
    mFieldBuffer = NULL;
    return NULL;
  }

  End = StrBrk (Begin, CharSet);
  if ((End != NULL) && (*End != L'\0')) {
    *End = L'\0';
    End ++;
  }

  mFieldBuffer = End;
  return Begin;
}

CHAR16 *
StrGetNewTokenLine (
  IN CHAR16                       *String,
  IN CHAR16                       *CharSet
  )
{
  return StrTokenLine (String, CharSet);
}

CHAR16 *
StrGetNextTokenLine (
  IN CHAR16                       *CharSet
  )
{
  return StrTokenLine (NULL, CharSet);
}

CHAR16 *
StrGetNewTokenField (
  IN CHAR16                       *String,
  IN CHAR16                       *CharSet
  )
{
  return StrTokenField (String, CharSet);
}

CHAR16 *
StrGetNextTokenField (
  IN CHAR16                       *CharSet
  )
{
  return StrTokenField (NULL, CharSet);
}

VOID
PatchForStrTokenAfter (
  IN CHAR16    *Buffer,
  IN CHAR16    Patch
  )
{
  CHAR16 *Str;

  if (Buffer == NULL) {
    return ;
  }

  Str = Buffer;
  while (*Str != 0) {
    Str ++;
  }
  *Str = Patch;

  while (*(Str ++)) {
    if (*Str == 0) {
      *Str = Patch;
    } else {
      break;
    }
  }

  return ;
}

VOID
PatchForStrTokenBefore (
  IN CHAR16    *Buffer,
  IN CHAR16    Patch
  )
{
  CHAR16 *Str;

  if (Buffer == NULL) {
    return ;
  }

  Str = Buffer;
  while (*(Str --)) {
    if ((*Str == 0) || (*Str == Patch)) {
      *Str = Patch;
    } else {
      break;
    }
  }

  return ;
}

CHAR8  *mAsciiLineBuffer          = NULL;
CHAR8  *mAsciiFieldBuffer         = NULL;

UINTN
AsciiStrSpn (
  IN CHAR8                       *String,
  IN CHAR8                       *CharSet
  )
/*++

Routine Description:

  Find the first substring.

--*/
{
  UINTN   Count;
  CHAR8  *Str1;
  CHAR8  *Str2;

  Count = 0;

  for (Str1 = String; *Str1 != '\0'; Str1 ++) {
    for (Str2 = CharSet; *Str2 != '\0'; Str2 ++) {
      if (*Str1 == *Str2) {
        break;
      }
    }

    if (*Str2 == '\0') {
      return Count;
    }

    Count ++;
  }

  return Count;
}


CHAR8 *
AsciiStrBrk (
  IN CHAR8                       *String,
  IN CHAR8                       *CharSet
  )
/*++

Routine Description:

  Searches a string for the first occurrence of a character contained in a
  specified buffer.

--*/
{
  CHAR8  *Str1;
  CHAR8  *Str2;

  for (Str1 = String; *Str1 != '\0'; Str1 ++) {
    for (Str2 = CharSet; *Str2 != '\0'; Str2 ++) {
      if (*Str1 == *Str2) {
        return (CHAR8 *) Str1;
      }
    }
  }

  return NULL;
}

CHAR8 *
AsciiStrTokenLine (
  IN CHAR8                       *String OPTIONAL,
  IN CHAR8                       *CharSet
  )
/*++

Routine Description:

  Find the next token after one or more specified characters.

--*/
{
  CHAR8  *Begin;
  CHAR8  *End;

  Begin = (String == NULL) ? mAsciiLineBuffer : String;
  if (Begin == NULL) {
    return NULL;
  }

  Begin += AsciiStrSpn (Begin, CharSet);
  if (*Begin == '\0') {
    mAsciiLineBuffer = NULL;
    return NULL;
  }

  End = AsciiStrBrk (Begin, CharSet);
  if ((End != NULL) && (*End != '\0')) {
    *End = '\0';
    End ++;
  }

  mAsciiLineBuffer = End;
  return Begin;
}


CHAR8 *
AsciiStrTokenField (
  IN CHAR8                       *String OPTIONAL,
  IN CHAR8                       *CharSet
  )
/*++

Routine Description:

  Find the next token after one specificed characters.

--*/
{
  CHAR8  *Begin;
  CHAR8  *End;


  Begin = (String == NULL) ? mAsciiFieldBuffer : String;
  if (Begin == NULL) {
    return NULL;
  }

  if (*Begin == L'\0') {
    mAsciiFieldBuffer = NULL;
    return NULL;
  }

  End = AsciiStrBrk (Begin, CharSet);
  if ((End != NULL) && (*End != '\0')) {
    *End = '\0';
    End ++;
  }

  mAsciiFieldBuffer = End;
  return Begin;
}

CHAR8 *
AsciiStrGetNewTokenLine (
  IN CHAR8                       *String,
  IN CHAR8                       *CharSet
  )
{
  return AsciiStrTokenLine (String, CharSet);
}

CHAR8 *
AsciiStrGetNextTokenLine (
  IN CHAR8                       *CharSet
  )
{
  return AsciiStrTokenLine (NULL, CharSet);
}

CHAR8 *
AsciiStrGetNewTokenField (
  IN CHAR8                       *String,
  IN CHAR8                       *CharSet
  )
{
  return AsciiStrTokenField (String, CharSet);
}

CHAR8 *
AsciiStrGetNextTokenField (
  IN CHAR8                       *CharSet
  )
{
  return AsciiStrTokenField (NULL, CharSet);
}

VOID
PatchForAsciiStrTokenAfter (
  IN CHAR8    *Buffer,
  IN CHAR8    Patch
  )
{
  CHAR8 *Str;

  if (Buffer == NULL) {
    return ;
  }

  Str = Buffer;
  while (*Str != 0) {
    Str ++;
  }
  *Str = Patch;

  while (*(Str ++)) {
    if (*Str == 0) {
      *Str = Patch;
    } else {
      break;
    }
  }

  return ;
}

VOID
PatchForAsciiStrTokenBefore (
  IN CHAR8    *Buffer,
  IN CHAR8    Patch
  )
{
  CHAR8 *Str;

  if (Buffer == NULL) {
    return ;
  }

  Str = Buffer;
  while (*(Str --)) {
    if ((*Str == 0) || (*Str == Patch)) {
      *Str = Patch;
    } else {
      break;
    }
  }

  return ;
}