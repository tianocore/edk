/*++

Copyright (c) 2004 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Language.h

Abstract:

  Language setting

Revision History

--*/

#ifndef _LANGUAGE_H
#define _LANGUAGE_H

#include "Tiano.h"
#include "EfiDriverLib.h"

#include "BdsLib.h"
#include "String.h"

#include EFI_GUID_DEFINITION (GlobalVariable)

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
;

#endif // _LANGUAGE_H_
