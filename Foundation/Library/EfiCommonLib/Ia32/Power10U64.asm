      TITLE   Power10U64.asm: calculates Operand * 10 ^ Power

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
;   Power10U64.asm
; 
; Abstract:
; 
;   Calculates Operand * 10 ^ Power
;
;------------------------------------------------------------------------------

  .686P
  .XMM
  .MODEL SMALL
  .CODE

Power10U64 PROTO C  Operand:      QWORD, Power:      DWORD
MultU64x32 PROTO C  Multiplicand: QWORD, Multiplier: DWORD
  
Power10U64 PROC C   Operand:      QWORD, Power:      DWORD

;------------------------------------------------------------------------------
; UINT64
; Power10U64 (
;   IN UINT64   Operand,
;   IN UINTN    Power
;   )
;
; Routine Description:
;
;   Raise 10 to the power of Power, and multiply the result with Operand
;
; Arguments:
;
;   Operand  - multiplicand
;   Power    - power
;
; Returns:
;
;   Operand * 10 ^ Power
;------------------------------------------------------------------------------

  push   ecx
  
  mov    ecx, Power
  jcxz   _Power10U64_Done
  
_Power10U64_Wend:
  invoke MultU64x32, Operand, 10
  mov    dword ptr Operand[0], eax
  mov    dword ptr Operand[4], edx
  loop   _Power10U64_Wend

_Power10U64_Done:
  pop    ecx
  ret

Power10U64 ENDP

END
