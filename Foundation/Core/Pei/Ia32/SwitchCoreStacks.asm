      TITLE   SwitchCoreStacks.asm: Core stack switching routine

;------------------------------------------------------------------------------
;Copyright 2004, Intel Corporation                                                         
;All rights reserved. This program and the accompanying materials                          
;are licensed and made available under the terms and conditions of the BSD License         
;which accompanies this distribution.  The full text of the license may be found at        
;http://opensource.org/licenses/bsd-license.php                                            
;                                                                                          
;THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
;WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
;
;Module Name:
;
;    SwitchCoreStacks.asm
;
;Abstract:
;
;    Core stack switching routine, invoked when real system memory is
;    discovered and installed.
;
;------------------------------------------------------------------------------

  .686P
  .XMM
  .MODEL SMALL
  .CODE
  
SwitchCoreStacks PROTO C  EntryPoint: DWORD, Parameter1: DWORD, Parameter2: DWORD, NewStack: DWORD

SwitchCoreStacks PROC C   EntryPoint: DWORD, Parameter1: DWORD, Parameter2: DWORD, NewStack: DWORD

;------------------------------------------------------------------------------
; VOID
; SwitchCoreStacks (
;   VOID  *EntryPoint,
;   UINTN Parameter1,
;   UINTN Parameter2,
;   VOID  *NewStack
;   )
;
; Routine Description:
; 
;   Routine for PEI switching stacks.
;
; Arguments:
;
;   EntryPoint - Entry point with new stack.
;   Parameter1 - First parameter for entry point.
;   Parameter2 - Second parameter for entry point.
;   NewStack   - Pointer to new stack.
;   
; Returns:
;
;   None
;
;----------------------------------------------------

  mov    ebx, Parameter1
  mov    edx, Parameter2
  mov    ecx, EntryPoint
  mov    esp, NewStack
  
  ; First push Parameter2, and then Parameter1.
  push   edx
  push   ebx
  call   ecx
  
  ret
  
SwitchCoreStacks ENDP

END
