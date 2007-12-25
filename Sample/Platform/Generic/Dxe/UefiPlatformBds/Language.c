/*++

Copyright (c) 2004 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Language.c

Abstract:

  Language settings

Revision History

--*/

#include "Language.h"
#include "FrontPage.h"

#define NARROW_GLYPH_NUMBER 8
#define WIDE_GLYPH_NUMBER   75

//
// Default language code, currently is English
//
CHAR8 *mDefaultLangCode = LANGUAGE_CODE_ENGLISH;

EFI_GUID  mFontPackageGuid = {
  0x78941450, 0x90ab, 0x4fb1, 0xb7, 0x5f, 0x58, 0x92, 0x14, 0xe2, 0x4a, 0xc
};

typedef struct {
  //
  // This 4-bytes total array length is required by PreparePackageList()
  //
  UINT32                 Length;

  //
  // This is the Font package definition
  //
  EFI_HII_PACKAGE_HEADER Header;
  UINT16                 NumberOfNarrowGlyphs;
  UINT16                 NumberOfWideGlyphs;
  EFI_NARROW_GLYPH       NarrowArray[NARROW_GLYPH_NUMBER];
  EFI_WIDE_GLYPH         WideArray[WIDE_GLYPH_NUMBER];
} FONT_PACK_BIN;

FONT_PACK_BIN mFontBin = {
  sizeof (FONT_PACK_BIN),
  {
    sizeof (FONT_PACK_BIN) - sizeof (UINT32),
    EFI_HII_PACKAGE_SIMPLE_FONTS,
  },
  NARROW_GLYPH_NUMBER,
  0,
  {     // Narrow Glyphs
    {
      0x05d0,
      0x00,
      {
        0x00,
        0x00,
        0x00,
        0x4E,
        0x6E,
        0x62,
        0x32,
        0x32,
        0x3C,
        0x68,
        0x4C,
        0x4C,
        0x46,
        0x76,
        0x72,
        0x00,
        0x00,
        0x00,
        0x00
      }
    },
    {
      0x05d1,
      0x00,
      {
        0x00,
        0x00,
        0x00,
        0x78,
        0x7C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x7E,
        0x7E,
        0x00,
        0x00,
        0x00,
        0x00
      }
    },
    {
      0x05d2,
      0x00,
      {
        0x00,
        0x00,
        0x00,
        0x78,
        0x7C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x1C,
        0x3E,
        0x66,
        0x66,
        0x00,
        0x00,
        0x00,
        0x00
      }
    },
    {
      0x05d3,
      0x00,
      {
        0x00,
        0x00,
        0x00,
        0x7E,
        0x7E,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x00,
        0x00,
        0x00,
        0x00
      }
    },
    {
      0x05d4,
      0x00,
      {
        0x00,
        0x00,
        0x00,
        0x7C,
        0x7E,
        0x06,
        0x06,
        0x06,
        0x06,
        0x66,
        0x66,
        0x66,
        0x66,
        0x66,
        0x66,
        0x00,
        0x00,
        0x00,
        0x00
      }
    },
    {
      0x05d5,
      0x00,
      {
        0x00,
        0x00,
        0x00,
        0x3C,
        0x3C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x0C,
        0x00,
        0x00,
        0x00,
        0x00
      }
    },
    {
      0x05d6,
      0x00,
      {
        0x00,
        0x00,
        0x00,
        0x38,
        0x38,
        0x1E,
        0x1E,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00
      }
    },
    {
      0x0000,
      0x00,
      {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00
      }
    }
  }
};

VOID
ExportFonts (
  VOID
  )
/*++

Routine Description:
  Routine to export glyphs to the HII database.  This is in addition to whatever is defined in the Graphics Console driver.

Arguments:
  None

Returns:

--*/
{
  EFI_STATUS                   Status;
  EFI_HANDLE                   DriverHandle;
  EFI_HII_HANDLE               HiiHandle;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;

  //
  // Create driver handle used by HII database
  //
  Status = CreateHiiDriverHandle (&DriverHandle);
  if (EFI_ERROR (Status)) {
    return ;
  }

  PackageList = PreparePackageList (1, &mFontPackageGuid, &mFontBin);
  ASSERT (PackageList != NULL);

  gHiiDatabase->NewPackageList (gHiiDatabase, PackageList, DriverHandle, &HiiHandle);
  gBS->FreePool (PackageList);
}

VOID
InitializeLanguage (
  BOOLEAN LangCodesSettingRequired
  )
/*++

Routine Description:
  Determine the current language that will be used
  based on language related EFI Variables

Arguments:
  LangCodesSettingRequired - If required to set LangCode variable

Returns:

--*/
{
  EFI_STATUS  Status;
  UINTN       Size;
  CHAR8       Lang[RFC_3066_ENTRY_SIZE];
  CHAR8       CurrentLang[RFC_3066_ENTRY_SIZE];
  CHAR8       *LangCodes;
  CHAR8       *LangStrings;
#ifdef LANG_SUPPORT
  CHAR8       *OldLangCodes;
  CHAR8       OldLang[ISO_639_2_ENTRY_SIZE];
#endif

  ExportFonts ();

  //
  // Collect the languages from what our current Language support is based on our VFR
  //
  LangCodes = GetSupportedLanguages (gStringPackHandle);
  ASSERT (LangCodes != NULL);

  if (LangCodesSettingRequired) {
    Status = gRT->SetVariable (
                    L"PlatformLangCodes",
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    EfiAsciiStrSize (LangCodes),
                    LangCodes
                    );

#ifdef LANG_SUPPORT
    //
    // Set UEFI deprecated variable "LangCodes" for backwards compatibility
    //
    OldLangCodes = Rfc3066ToIso639 (LangCodes);
    Status = gRT->SetVariable (
                    L"LangCodes",
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    EfiAsciiStrLen (OldLangCodes),
                    OldLangCodes
                    );
    gBS->FreePool (OldLangCodes);
#endif
  }

  //
  // Find current LangCode from Lang NV Variable
  //
  Size = RFC_3066_ENTRY_SIZE;
  Status = gRT->GetVariable (
                  L"PlatformLang",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &Size,
                  CurrentLang
                  );

  if (!EFI_ERROR (Status)) {
    Status = EFI_NOT_FOUND;

    LangStrings = LangCodes;
    while (*LangStrings != 0) {
      GetNextLanguage (&LangStrings, Lang);

      if (EfiAsciiStrCmp (Lang, CurrentLang) == 0) {
        Status = EFI_SUCCESS;
        break;
      }
    }
  }

  //
  // If we cannot get language code from variable,
  // or LangCode cannot be found from language table,
  // set the first language in language table to variable.
  //
  if (EFI_ERROR (Status)) {
    LangStrings = LangCodes;
    GetNextLanguage (&LangStrings, Lang);
    Status = gRT->SetVariable (
                    L"PlatformLang",
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    EfiAsciiStrSize (mDefaultLangCode),
                    Lang
                    );
#ifdef LANG_SUPPORT
    //
    // Set UEFI deprecated variable "Lang" for backwards compatibility
    //
    Status = ConvertRfc3066LanguageToIso639Language (mDefaultLangCode, OldLang);
    if (!EFI_ERROR (Status)) {
      Status = gRT->SetVariable (
                      L"Lang",
                      &gEfiGlobalVariableGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      ISO_639_2_ENTRY_SIZE,
                      OldLang
                      );
    }
#endif
  }

  if (LangCodes != NULL) {
    gBS->FreePool (LangCodes);
  }
}
