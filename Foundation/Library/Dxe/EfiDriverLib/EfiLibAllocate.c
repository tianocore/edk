/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiLibAllocate.c

Abstract:

  Support routines for memory allocation routines for use with drivers.

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"

VOID *
EfiLibAllocatePool (
  IN  UINTN   AllocationSize
  )
{
  VOID  *Memory;

  Memory = NULL;
  gBS->AllocatePool (EfiBootServicesData, AllocationSize, &Memory);
  return Memory;
}

VOID *
EfiLibAllocateRuntimePool (
  IN  UINTN   AllocationSize
  )
{
  VOID  *Memory;

  Memory = NULL;
  gBS->AllocatePool (EfiRuntimeServicesData, AllocationSize, &Memory);
  return Memory;
}

VOID *
EfiLibAllocateZeroPool (
  IN  UINTN   AllocationSize
  )
{
  VOID  *Memory;

  Memory = EfiLibAllocatePool (AllocationSize);
  if (Memory != NULL) {
    gBS->SetMem (Memory, AllocationSize, 0);
  }
  return Memory;
}

VOID *
EfiLibAllocateRuntimeZeroPool (
  IN  UINTN   AllocationSize
  )
{
  VOID  *Memory;

  Memory = EfiLibAllocateRuntimePool (AllocationSize);
  if (Memory != NULL) {
    gBS->SetMem (Memory, AllocationSize, 0);
  }
  return Memory;
}

VOID *
EfiLibAllocateCopyPool (
  IN  UINTN   AllocationSize,
  IN  VOID    *Buffer
  )
{
  VOID  *Memory;

  Memory = NULL;
  gBS->AllocatePool (EfiBootServicesData, AllocationSize, &Memory);
  if (Memory != NULL) {
    gBS->CopyMem (Memory, Buffer, AllocationSize);
  }
  return Memory;
}


VOID *
EfiLibAllocateRuntimeCopyPool (
  IN  UINTN            AllocationSize,
  IN  VOID             *Buffer
  )
{
  VOID  *Memory;

  Memory = NULL;
  gBS->AllocatePool (EfiRuntimeServicesData, AllocationSize, &Memory);
  if (Memory != NULL) {
    gBS->CopyMem (Memory, Buffer, AllocationSize);
  }
  return Memory;
}

