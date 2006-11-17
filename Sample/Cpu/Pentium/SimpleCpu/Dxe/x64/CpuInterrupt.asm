      TITLE   CpuInterrupt.asm: 
;------------------------------------------------------------------------------
;*
;*   Copyright 2006, Intel Corporation                                                         
;*   All rights reserved. This program and the accompanying materials                          
;*   are licensed and made available under the terms and conditions of the BSD License         
;*   which accompanies this distribution.  The full text of the license may be found at        
;*   http://opensource.org/licenses/bsd-license.php                                            
;*                                                                                             
;*   THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
;*   WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
;*   
;*    CpuInterrupt.asm
;*  
;*   Abstract:
;*
;------------------------------------------------------------------------------

.code

EXTRN TimerHandler: FAR
EXTERN mTimerVector: DWORD

; VOID
; InstallInterruptHandler (
;     UINTN Vector,           // rcx
;     void  (*Handler)(void)  // rdx
;     )
InstallInterruptHandler PROC 
        push    rbx
        pushfq                              ; save eflags
        cli                                 ; turn off interrupts
        sub     rsp, 10h                    ; open some space on the stack
        mov     rbx, rsp
        sidt    [rbx]                       ; get fword address of IDT
        mov     rbx, [rbx+2]                ; move offset of IDT into RBX
        add     rsp, 10h                    ; correct stack
        mov     rax, rcx                    ; Get vector number
        shl     rax, 4                      ; multiply by 16 to get offset
        add     rbx, rax                    ; add to IDT base to get entry
        mov     rax, rdx                    ; load new address into IDT entry
        mov     word ptr [rbx], ax          ; write bits 15..0 of offset
        shr     rax, 16                     ; use ax to copy 31..16 to descriptors
        mov     word ptr [rbx+6], ax        ; write bits 31..16 of offset
        shr     rax, 16                     ; use eax to copy 63..32 to descriptors
        mov     dword ptr [rbx+8], eax      ; write bits 63..32 of offset
        popfq                               ; restore flags (possible enabling interrupts)
        pop     rbx
        ret

InstallInterruptHandler ENDP

SystemTimerHandler PROC

; +---------------------+ <-- 16-byte aligned ensured by processor
; +    Old SS           +
; +---------------------+
; +    Old RSP          +
; +---------------------+
; +    RFlags           +
; +---------------------+
; +    CS               +
; +---------------------+
; +    RIP              +
; +---------------------+
; +    RBP              +
; +---------------------+ <-- RBP, 16-byte aligned

  cli
  push rbp
  mov  rbp, rsp

  ;
  ; Since here the stack pointer is 16-byte aligned, so
  ; EFI_FX_SAVE_STATE_X64 of EFI_SYSTEM_CONTEXT_x64
  ; is 16-byte aligned
  ;       

;; UINT64  Rdi, Rsi, Rbp, Rsp, Rbx, Rdx, Rcx, Rax;
;; UINT64  R8, R9, R10, R11, R12, R13, R14, R15;
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rax
  push rcx
  push rdx
  push rbx
  push qword ptr [rbp + 4 * 8]  ; RSP
  push qword ptr [rbp]          ; RBP
  push rsi
  push rdi

;; UINT64  Gs, Fs, Es, Ds, Cs, Ss;  insure high 16 bits of each is zero
  movzx   rax, word ptr [rbp + 5 * 8]
  push    rax                      ; for ss
  movzx   rax, word ptr [rbp + 2 * 8]
  push    rax                      ; for cs
  mov     rax, ds
  push    rax
  mov     rax, es
  push    rax
  mov     rax, fs
  push    rax
  mov     rax, gs
  push    rax

;; UINT64  Rip;
  push    qword ptr [rbp + 1 * 8]

;; UINT64  Gdtr[2], Idtr[2];
  sub     rsp, 16
  sidt    fword ptr [rsp]
  sub     rsp, 16
  sgdt    fword ptr [rsp]

;; UINT64  Ldtr, Tr;
  xor     rax, rax
  str     ax
  push    rax
  sldt    ax
  push    rax

;; UINT64  RFlags;
  push    qword ptr [rbp + 3 * 8]

;; UINT64  Cr0, Cr1, Cr2, Cr3, Cr4, Cr8;
  mov     rax, cr8
  push    rax
  mov     rax, cr4
  or      rax, 208h
  mov     cr4, rax
  push    rax
  mov     rax, cr3
  push    rax
  mov     rax, cr2
  push    rax
  xor     rax, rax
  push    rax
  mov     rax, cr0
  push    rax

;; UINT64  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
  mov     rax, dr7
  push    rax
;; clear Dr7 while executing debugger itself
  xor     rax, rax
  mov     dr7, rax

  mov     rax, dr6
  push    rax
;; insure all status bits in dr6 are clear...
  xor     rax, rax
  mov     dr6, rax

  mov     rax, dr3
  push    rax
  mov     rax, dr2
  push    rax
  mov     rax, dr1
  push    rax
  mov     rax, dr0
  push    rax

;; FX_SAVE_STATE_X64 FxSaveState;

  sub rsp, 512
  mov rdi, rsp
  db 0fh, 0aeh, 00000111y ;fxsave [rdi]

;; UINT32  ExceptionData;
  push    0

;; call into exception handler
;; Prepare parameter and call
  xor     rcx, rcx
  mov     ecx, mTimerVector
  mov     rdx, rsp
  ;
  ; Per X64 calling convention, allocate maximum parameter stack space
  ; and make sure RSP is 16-byte aligned
  ;
  sub     rsp, 4 * 8 + 8
  call    TimerHandler
  add     rsp, 4 * 8 + 8

  cli
;; UINT64  ExceptionData;
  add     rsp, 8

;; FX_SAVE_STATE_X64 FxSaveState;

  mov rsi, rsp
  db 0fh, 0aeh, 00001110y ; fxrstor [rsi]
  add rsp, 512

;; UINT64  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
  pop     rax
  mov     dr0, rax
  pop     rax
  mov     dr1, rax
  pop     rax
  mov     dr2, rax
  pop     rax
  mov     dr3, rax
;; skip restore of dr6.  We cleared dr6 during the context save.
  add     rsp, 8
  pop     rax
  mov     dr7, rax

;; UINT64  Cr0, Cr1, Cr2, Cr3, Cr4, Cr8;
  pop     rax
  mov     cr0, rax
  add     rsp, 8   ; not for Cr1
  pop     rax
  mov     cr2, rax
  pop     rax
  mov     cr3, rax
  pop     rax
  mov     cr4, rax
  pop     rax
  mov     cr8, rax

;; UINT64  RFlags;
  pop     qword ptr [rbp + 3 * 8]

;; UINT64  Ldtr, Tr;
;; UINT64  Gdtr[2], Idtr[2];
;; Best not let anyone mess with these particular registers...
  add     rsp, 48

;; UINT64  Rip;
  pop     qword ptr [rbp + 1 * 8]

;; UINT64  Gs, Fs, Es, Ds, Cs, Ss;
  pop     rax
  ; mov     gs, rax ; not for gs
  pop     rax
  ; mov     fs, rax ; not for fs
  ; (X64 will not use fs and gs, so we do not restore it)
  pop     rax
  mov     es, rax
  pop     rax
  mov     ds, rax
  pop     qword ptr [rbp + 2 * 8]  ; for cs
  pop     qword ptr [rbp + 5 * 8]  ; for ss

;; UINT64  Rdi, Rsi, Rbp, Rsp, Rbx, Rdx, Rcx, Rax;
;; UINT64  R8, R9, R10, R11, R12, R13, R14, R15;
  pop     rdi
  pop     rsi
  add     rsp, 8                  ; not for rbp
  pop     qword ptr [rbp + 4 * 8] ; for rsp
  pop     rbx
  pop     rdx
  pop     rcx
  pop     rax
  pop     r8
  pop     r9
  pop     r10
  pop     r11
  pop     r12
  pop     r13
  pop     r14
  pop     r15

  mov     rsp, rbp
  pop     rbp
  iretq

SystemTimerHandler ENDP

END
