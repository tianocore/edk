/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  math.c

Abstract:

  64-bit Math worker functions for Intel Itanium(TM) processors.

--*/

#include "Tiano.h"
#include "Pei.h"
#include "PeiLib.h"



UINT64
LShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )
// Left shift 64bit by 32bit and get a 64bit result
{
  return Operand << Count;
}

UINT64
RShiftU64 (
  IN UINT64   Operand,
  IN UINTN    Count
  )
// Right shift 64bit by 32bit and get a 64bit result
{
  return Operand >> Count;
}


UINT64
MultU64x32 (
  IN UINT64   Multiplicand,
  IN UINTN    Multiplier
  )
// Multiple 64bit by 32bit and get a 64bit result
{
  return Multiplicand * Multiplier;
}

UINT64
DivU64x32 (
  IN UINT64   Dividend,
  IN UINTN    Divisor,
  OUT UINTN   *Remainder OPTIONAL
  )
// divide 64bit by 32bit and get a 64bit result
// N.B. only works for 31bit divisors!!
{
  if (Remainder) {
    *Remainder = Dividend % Divisor;
  }

  return Dividend / Divisor;
}
