      TITLE   Log2.asm: 64-bit integer logarithm function for IA-32

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
;   Log2.asm
; 
; Abstract:
; 
;   64-bit integer logarithm function for IA-32
;
;------------------------------------------------------------------------------

  .686P
  .XMM
  .MODEL SMALL
  .CODE

Log2 PROTO C  Operand: QWORD

Log2 PROC C   Operand: QWORD

;------------------------------------------------------------------------------
; UINT8
; Log2 (
;   IN UINT64   Operand
;   )
;
; Routine Description:
; 
;   Calculates and floors logarithms based on 2
;
; Arguments:
;
;   Operand - value to calculate logarithm
;  
; Returns:
;
;   The largest integer that is less than or equal
;   to the logarithm of Operand based on 2 
;------------------------------------------------------------------------------

  push   ecx
  
  mov    ecx, 64
  
  cmp    dword ptr Operand[0], 0
  jne    _Log2_Wend 
  cmp    dword ptr Operand[4], 0
  jne    _Log2_Wend 
  mov    cl, 0FFH
  jmp    _Log2_Done
  
_Log2_Wend:
  dec    ecx
  cmp    ecx, 32
  jae    _Log2_Higher
  bt     dword ptr Operand[0], ecx
  jmp    _Log2_Bit
  
_Log2_Higher:
  mov    eax, ecx
  sub    eax, 32
  bt     dword ptr Operand[4], eax
  
_Log2_Bit:
  jc     _Log2_Done
  jmp    _Log2_Wend
      
_Log2_Done:
  mov    al, cl
  pop    ecx
  ret
    
Log2 ENDP

END
