/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  EbcTestSub.c

Abstract:

  EBC Debugger Test Driver

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"

UINTN  TestSubVariable1 = 4;

//
// Function implementations
//

EFI_STATUS
TestSubRoutine1 (
  IN UINTN        Arg1,
  IN UINTN        Arg2
  )
{
  Arg2 = Arg1 + Arg2;
  TestSubVariable1 = 5;

  return EFI_SUCCESS;
}

EFI_STATUS
TestSubRoutine2 (
  IN UINTN        Arg1,
  IN UINTN        *Arg2
  )
{
  *Arg2 = Arg1 + *Arg2;
  TestSubVariable1 = 6;

  return EFI_SUCCESS;
}
