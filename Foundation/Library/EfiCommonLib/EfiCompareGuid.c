/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiCompareGuid.c

Abstract:

  Driver library routine to compare two GUIDs.

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"

BOOLEAN
EfiCompareGuid (
  IN EFI_GUID *Guid1,
  IN EFI_GUID *Guid2
  )
/*++

Routine Description:

  Compares two GUIDs

Arguments:

  Guid1 - guid to compare

  Guid2 - guid to compare

Returns:
  TRUE     if Guid1 == Guid2
  FALSE    if Guid1 != Guid2

--*/
{
  UINTN   Index;
  UINT32  Result;
  UINT32  *IntGuid1;
  UINT32  *IntGuid2;
  
  IntGuid1 = (UINT32 *) Guid1;
  IntGuid2 = (UINT32 *) Guid2;
  for (Index = 0, Result=0; Index < 4; Index++) {
    Result |= (IntGuid1[Index] - IntGuid2[Index]);
  }
  return (BOOLEAN)(Result == 0);
}

