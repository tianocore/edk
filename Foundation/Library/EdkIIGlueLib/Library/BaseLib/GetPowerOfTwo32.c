/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.  

Module Name:

  GetPowerOfTwo32.c
  
Abstract: 

  Math worker functions.

--*/

#include "BaseLibInternals.h"

/**
  Returns the value of the highest bit set in a 32-bit value. Equivalent to
  1 << HighBitSet32(x).

  This function computes the value of the highest bit set in the 32-bit value
  specified by Operand. If Operand is zero, then zero is returned.

  @param  Operand The 32-bit operand to evaluate.

  @return 1 << HighBitSet32(Operand)
  @retval 0 Operand is zero.

**/
UINT32
EFIAPI
GetPowerOfTwo32 (
  IN      UINT32                    Operand
  )
{
  if (Operand == 0) {
    return 0;
  }

  return 1ul << HighBitSet32 (Operand);
}
