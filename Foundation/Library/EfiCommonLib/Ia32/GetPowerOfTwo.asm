      TITLE   GetPowerOfTwo.asm: Calculates the power of two value just below input

;------------------------------------------------------------------------------
;
; Copyright (c) 2005 Intel Corporation                                                         
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
  
_GetPowerOfTwo  PROC
;------------------------------------------------------------------------------
; UINT32
; _GetPowerOfTwo (
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
    xor     eax, eax
    mov     edx, eax
    mov     ecx, [esp + 8]
    jecxz   @F
    bsr     ecx, ecx
    bts     edx, ecx
    jmp     @Exit
@@:
    mov     ecx, [esp + 4]
    jecxz   @Exit
    bsr     ecx, ecx
    bts     eax, ecx
@Exit:
    ret
_GetPowerOfTwo  ENDP

END
