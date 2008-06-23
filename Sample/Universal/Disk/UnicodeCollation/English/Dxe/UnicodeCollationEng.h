/*++

Copyright (c) 2004 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
  UnicodeCollationEng.h

Abstract:

  Head file for Unicode Collation Protocol (English)

Revision History

--*/

#ifndef _UNICODE_COLLATION_ENG_H
#define _UNICODE_COLLATION_ENG_H

#include "Tiano.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocol Prototypes
//
// None.
//
// Driver Produced Protocol Prototypes
//
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
#include EFI_PROTOCOL_DEFINITION (UnicodeCollation2)
#else
#include EFI_PROTOCOL_DEFINITION (UnicodeCollation)
#endif

//
// Globals
//
extern CHAR8  *mEngUpperMap;
extern CHAR8  *mEngLowerMap;
extern CHAR8  *mEngInfoMap;
extern CHAR8  mOtherChars[];

//
// Defines
//
#define CHAR_FAT_VALID  0x01

#define ToUpper(a)      (CHAR16) (a <= 0xFF ? mEngUpperMap[a] : a)
#define ToLower(a)      (CHAR16) (a <= 0xFF ? mEngLowerMap[a] : a)

//
// Prototypes
//
INTN
EFIAPI
EngStriColl (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN EFI_UNICODE_COLLATION2_PROTOCOL          *This,
#else
  IN EFI_UNICODE_COLLATION_PROTOCOL           *This,
#endif
  IN CHAR16                                   *s1,
  IN CHAR16                                   *s2
  )
;

BOOLEAN
EFIAPI
EngMetaiMatch (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN EFI_UNICODE_COLLATION2_PROTOCOL         *This,
#else
  IN EFI_UNICODE_COLLATION_PROTOCOL          *This,
#endif
  IN CHAR16                                  *String,
  IN CHAR16                                  *Pattern
  )
;

VOID
EFIAPI
EngStrLwr (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN EFI_UNICODE_COLLATION2_PROTOCOL         *This,
#else
  IN EFI_UNICODE_COLLATION_PROTOCOL          *This,
#endif
  IN OUT CHAR16                              *Str
  )
;

VOID
EFIAPI
EngStrUpr (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN EFI_UNICODE_COLLATION2_PROTOCOL         *This,
#else
  IN EFI_UNICODE_COLLATION_PROTOCOL          *This,
#endif
  IN OUT CHAR16                              *Str
  )
;

VOID
EFIAPI
EngFatToStr (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN EFI_UNICODE_COLLATION2_PROTOCOL         *This,
#else
  IN EFI_UNICODE_COLLATION_PROTOCOL          *This,
#endif
  IN UINTN                                   FatSize,
  IN CHAR8                                   *Fat,
  OUT CHAR16                                 *String
  )
;

BOOLEAN
EFIAPI
EngStrToFat (
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  IN EFI_UNICODE_COLLATION2_PROTOCOL         *This,
#else
  IN EFI_UNICODE_COLLATION_PROTOCOL          *This,
#endif
  IN CHAR16                                  *String,
  IN UINTN                                   FatSize,
  OUT CHAR8                                  *Fat
  )
;

//
// Globals
//
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
EFI_UNICODE_COLLATION2_PROTOCOL UnicodeEng = {
#else
EFI_UNICODE_COLLATION_PROTOCOL  UnicodeEng = {
#endif
  EngStriColl,
  EngMetaiMatch,
  EngStrLwr,
  EngStrUpr,
  EngFatToStr,
  EngStrToFat,
  LANGUAGE_CODE_ENGLISH
};

EFI_STATUS
EFIAPI
InitializeUnicodeCollationEng (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
;

#endif
