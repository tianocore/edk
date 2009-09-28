      TITLE   SwitchCoreStacks.asm: Core stack switching routine

;------------------------------------------------------------------------------
;Copyright (c) 2005 - 2009, Intel Corporation                                                         
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
; EFIAPI
; AsmWriteMm7 (
;   IN UINT64   Value
;   );
;------------------------------------------------------------------------------
AsmWriteMm7 PROC PUBLIC
    db 48h, 0fh, 06eh, 0f9h ; movq    mm7, rcx
    ret
AsmWriteMm7 ENDP


;------------------------------------------------------------------------------
; VOID
; SwitchCoreStacks (
;   VOID  *EntryPoint,  // rcx
;   UINTN Parameter1,   // rdx
;   UINTN Parameter2,   // r8
;   VOID  *NewStack     // r9
;   )
;
; Routine Description:
;   Routine for PEI switching stacks.
;
; Arguments:
;   EntryPoint - Entry point with new stack.
;   Parameter1 - First parameter for entry point.
;   Parameter2 - Second parameter for entry point.
;   NewStack   - Pointer to new stack.
;   
; Returns:
;   None
;
;----------------------------------------------------
SwitchCoreStacks PROC  PUBLIC
    mov   r10, rcx       ; Save EntryPoint

    ; Adjust stack for
    ;   1) leave 4 registers space
    ;   2) let it 16 bytes aligned before call
    sub   r9, 20h
    and   r9w, 0fff0h    ; do not assume 16 bytes aligned

    mov   rsp, r9        ; rsp = NewStack
    mov   rcx, rdx       ; Arg1 = Parameter 1
    mov   rdx, r8        ; Arg2 = Parameter 2
    call  r10            ; rcx = EntryPoint
 ;
 ; no ret as we have a new stack and we jumped to the new location
 ;     
    ret
  
SwitchCoreStacks ENDP

END
