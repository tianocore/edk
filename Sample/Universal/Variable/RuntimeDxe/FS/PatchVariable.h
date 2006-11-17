/*++

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

    PatchVariable.h

Abstract:

    Various patch function's declaration puts here.

Revision History

--*/

#ifndef _PATCH_VARIABLE_H_
#define _PATCH_VARIABLE_H_

VOID
LoadSyncVariable (
  VOID
  );

VOID
StoreSyncVariable (
  VOID
  );

VOID
PatchVariable (
  VOID
  );

#endif
