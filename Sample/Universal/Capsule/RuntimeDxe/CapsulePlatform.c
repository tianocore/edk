/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CapsulePlatform.c

Abstract:

  Capsule Platform Service 

--*/

#include "Tiano.h"

//
//Max size capsule services support are platform policy,to populate capsules we just need
//memory to maintain them across reset,it is not a problem. And to special capsules ,for
//example,update flash,it is mostly decided by the platform. Here is a sample size for 
//different type capsules.
//
#define MAX_SIZE_POPULATE              (0)
#define MAX_SIZE_NON_POPULATE          (0)




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
  //If the platform has a way to guarantee the memory integrity across a system reset, return 
  //TRUE, else FALSE. 
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
  //Here is a sample size, different platforms have different sizes.
  //
  *MaxSizePopulate    = MAX_SIZE_POPULATE;
  *MaxSizeNonPopulate = MAX_SIZE_NON_POPULATE;
  return; 
}



