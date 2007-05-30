/*++

Copyright (c) 2006 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CapsulePlatform.c

Abstract:

  Capsule Platform Runtime Service 

--*/

#include "Tiano.h"

#define MAX_SIZE_POPULATE              (0)
#define MAX_SIZE_NON_POPULATE          (0)


EFI_STATUS
EFIAPI
CheckCapsuleGuid (
  IN EFI_CAPSULE_HEADER     **CapsuleHeaderArray,
  IN UINTN                  CapsuleCount
  )
/*++

Routine Description:

  This routine check the capsule validity. Platform has knowledge 
  of certain CapsuleGuid.

Arguments:

  CapsuleHeaderArray             A array of pointers to capsule headers passed in
  CapsuleCount                   The number of capsule
  
Returns:

--*/  
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI    
LaunchCapsule (
  IN EFI_CAPSULE_HEADER      **CapsuleHeaderArray,
  IN UINTN                   CapsuleCount,
  OUT BOOLEAN                *NeedReset
  )
/*++

Routine Description:

  This routine check whether a system reset need, and meanwhile launch 
  capsules who has no reset flag in its header immediately.

Arguments:

  CapsuleHeaderArray             A array of pointers to capsule headers passed in
  CapsuleCount                   The number of capsule
  NeedReset                      Indicates whether a system reset needs.
  
Returns:

--*/    
{
  return EFI_UNSUPPORTED;
}


BOOLEAN
EFIAPI
SupportUpdateCapsuleRest (
  VOID
  )
/*++

Routine Description:

  This function returns if the platform supports update capsule across a system reset.
  
Arguments:

Returns:

--*/
{
  //
  // If the platform has a way to guarantee the memory integrity across a system reset,
  // return TRUE, else FALSE. 
  //
  return FALSE;
}



VOID
EFIAPI
SupportCapsuleSize (
  IN OUT UINT32 *MaxSizePopulate,
  IN OUT UINT32 *MaxSizeNonPopulate
  )
/*++

Routine Description:

  This code returns the max size capsule the platform supports.
  
Arguments:

Returns:


--*/

{
  //
  // Generally, max size varies from different platform. Strictly, certain platform
  // has different tolerance between capsule persisted across a system and capsule
  // who needs launching immediately. These two type capsule demands for different
  // memory size. Here is an example size.
  //
  *MaxSizePopulate    = MAX_SIZE_POPULATE;
  *MaxSizeNonPopulate = MAX_SIZE_NON_POPULATE;
  return; 
}
