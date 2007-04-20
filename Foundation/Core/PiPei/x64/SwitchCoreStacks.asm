      TITLE   SwitchCoreStacks.asm: Core stack switching routine

;------------------------------------------------------------------------------
;Copyright (c) 2005 - 2007, Intel Corporation                                                         
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

text    SEGMENT

;------------------------------------------------------------------------------
; VOID
; SwitchCoreStacks (
;   VOID  *EntryPoint,  // rcx
;   UINTN Parameter1,   // rdx
;   UINTN Parameter2,   // r8
;   UINTN Parameter3,   // r9
;   VOID  *NewStack     // [rsp+40]
;   )
;
; Routine Description:
;   Routine for PEI switching stacks.
;
; Arguments:
;   EntryPoint - Entry point with new stack.
;   Parameter1 - First parameter for entry point.
;   Parameter2 - Second parameter for entry point.
;   Parameter3 - Third parameter for entry point.
;   NewStack   - Pointer to new stack.
;   
; Returns:
;   None
;
;----------------------------------------------------
SwitchCoreStacks PROC NEAR PUBLIC
    mov   r10, rcx       ; Save EntryPoint
    mov   r11, [rsp+40]  ; Save NewStack
    ; Adjust stack for
    ;   1) leave 4 registers space
    ;   2) let it 16 bytes aligned before call
    sub   r11, 20h
    and   r11w, 0fff0h    ; do not assume 16 bytes aligned

    mov   rsp, r11        ; rsp = NewStack
    mov   rcx, rdx       ; Arg1 = Parameter 1
    mov   rdx, r8        ; Arg2 = Parameter 2
    mov   r8,  r9        ; Arg3 = Parameter 3
    call  r10            ; rcx = EntryPoint
 ;
 ; no ret as we have a new stack and we jumped to the new location
 ;     
    ret
  
SwitchCoreStacks ENDP

END
