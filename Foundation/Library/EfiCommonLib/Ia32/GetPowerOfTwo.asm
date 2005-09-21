      TITLE   GetPowerOfTwo.asm: Calculates the power of two value just below input

;------------------------------------------------------------------------------
;
; Copyright (c) 2004, Intel Corporation                                                         
; All rights reserved. This program and the accompanying materials                          
; are licensed and made available under the terms and conditions of the BSD License         
; which accompanies this distribution.  The full text of the license may be found at        
; http://opensource.org/licenses/bsd-license.php                                            
;                                                                                           
; THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
; WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
; 
; Module Name:
;
;   GetPowerOfTwo.asm
; 
; Abstract:
; 
;   Calculates the largest integer that is both 
;   a power of two and less than Input
;
;------------------------------------------------------------------------------

  .686P
  .XMM
  .MODEL SMALL
  .CODE
  
GetPowerOfTwo PROTO C  Input: DWORD

GetPowerOfTwo PROC C   Input: DWORD

;------------------------------------------------------------------------------
; UINT32
; GetPowerOfTwo (
;   IN UINT32   Input
;   )
;
; Routine Description:
;
;   Calculates the largest integer that is both 
;   a power of two and less than Input
;
; Arguments:
;
;   Input  - value to calculate power of two
;
; Returns:
;
;   the largest integer that is both  a power of 
;   two and less than Input
;------------------------------------------------------------------------------
  push   edx
  xor    eax, eax

  bsr    edx, Input
  bts    eax, edx
  pop    edx
  ret
  
GetPowerOfTwo ENDP


END
