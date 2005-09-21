      TITLE   DivU64x32.asm: 64-bit division function for IA-32

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
;   DivU64x32.asm
; 
; Abstract:
; 
;   64-bit division function for IA-32
;
;------------------------------------------------------------------------------

  .686P
  .XMM
  .MODEL SMALL
  .CODE

DivU64x32 PROTO C  Dividend: QWORD, Divisor: DWORD, Remainder: DWORD  

DivU64x32 PROC C   Dividend: QWORD, Divisor: DWORD, Remainder: DWORD  

;------------------------------------------------------------------------------
; UINT64
; DivU64x32 (
;   IN UINT64   Dividend,
;   IN UINTN    Divisor,
;   OUT UINTN   *Remainder OPTIONAL
;   )
;
; Routine Description:
;
;   This routine allows a 64 bit value to be divided with a 32 bit value returns 
;   64bit result and the Remainder.
;
; Arguments:
;
;   Dividend  - dividend
;   Divisor   - divisor
;   Remainder - buffer for remainder
;  
; Returns:
;
;   Dividend  / Divisor
;   Remainder = Dividend mod Divisor
;  
; N.B. only works for 31bit divisors!!
;------------------------------------------------------------------------------
    
  push   ecx
  ;
  ; let edx contain the intermediate result of remainder
  ;
  xor    edx, edx
  mov    ecx, 64
  
_DivU64x32_Wend:
  shl    dword ptr Dividend[0], 1
  rcl    dword ptr Dividend[4], 1    
  rcl    edx, 1            

  ;
  ; If intermediate result of remainder is larger than
  ; or equal to divisor, then set the lowest bit of dividend,
  ; and subtract divisor from intermediate remainder
  ;
  cmp    edx, Divisor                
  jb     _DivU64x32_Cont
  bts    dword ptr Dividend[0], 0            
  sub    edx, Divisor
                   
_DivU64x32_Cont:
  loop   _DivU64x32_Wend

  cmp    Remainder, 0
  je     _DivU64x32_Done
  mov    eax, Remainder
  mov    dword ptr [eax], edx

_DivU64x32_Done:
  mov    eax, dword ptr Dividend[0]
  mov    edx, dword ptr Dividend[4]
  pop    ecx
  ret
  
DivU64x32 ENDP


END
