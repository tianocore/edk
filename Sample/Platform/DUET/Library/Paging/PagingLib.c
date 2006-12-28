/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  PagingLib.h

Abstract:

Revision History:

--*/

#include "PagingLib.h"

VOID *
GetPageTableBase (
  VOID
  );

BOOLEAN
GetNullPointerProtectionState (
  UINT8 *PageTable
  );

VOID
EnableNullPointerProtection (
  UINT8 *PageTable
  );

VOID
DisableNullPointerProtection (
  UINT8 *PageTable
  );

BOOLEAN
EnableProtection (
  IN VOID
  )
{
  BOOLEAN   OldState;
  UINT8     *PageTable;

  PageTable = GetPageTableBase ();
  OldState  = GetNullPointerProtectionState (PageTable);
  EnableNullPointerProtection (PageTable);

  return OldState;
}

BOOLEAN
DisableProtection (
  IN VOID
  )
{
  BOOLEAN   OldState;
  UINT8     *PageTable;

  PageTable = GetPageTableBase ();
  OldState  = GetNullPointerProtectionState (PageTable);
  DisableNullPointerProtection (PageTable);

  return OldState;
}

