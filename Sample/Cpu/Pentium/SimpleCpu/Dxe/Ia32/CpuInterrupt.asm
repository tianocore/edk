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

.686p
.model  flat, C

PUBLIC SystemTimerHandler

.code
.stack
.MMX
.XMM

EXTRN TimerHandler: NEAR
EXTERN mTimerVector: DWORD

; VOID
; InstallInterruptHandler (
;     UINTN Vector,
;     VOID  (*Handler)(VOID)
;     )
InstallInterruptHandler    PROC C    \
  Vector:DWORD,          \
  Handler:DWORD

        push    edi
        pushfd                              ; save eflags
        cli                                 ; turn off interrupts
        sub     esp, 6                      ; open some space on the stack
        mov     edi, esp
        sidt    es:[edi]                    ; get fword address of IDT
        mov     edi, es:[edi+2]             ; move offset of IDT into EDI
        add     esp, 6                      ; correct stack
        mov     eax, Vector                 ; Get vector number
        shl     eax, 3                      ; multiply by 8 to get offset
        add     edi, eax                    ; add to IDT base to get entry
        mov     eax, Handler                ; load new address into IDT entry
        mov     word ptr es:[edi], ax       ; write bits 15..0 of offset
        shr     eax, 16                     ; use ax to copy 31..16 to descriptors
        mov     word ptr es:[edi+6], ax     ; write bits 31..16 of offset
        popfd                               ; restore flags (possible enabling interrupts)
        pop     edi
        ret

InstallInterruptHandler    ENDP

SystemTimerHandler PROC

; +---------------------+
; +    EFlags           +
; +---------------------+
; +    CS               +
; +---------------------+
; +    EIP              +
; +---------------------+
; +    EBP              +
; +---------------------+ <-- EBP

  cli
  push ebp
  mov  ebp, esp

  ;
  ; Align stack to make sure that EFI_FX_SAVE_STATE_IA32 of EFI_SYSTEM_CONTEXT_IA32
  ; is 16-byte aligned
  ;
  and     esp, 0fffffff0h
  sub     esp, 12

;; UINT32  Edi, Esi, Ebp, Esp, Ebx, Edx, Ecx, Eax;
  push    eax
  push    ecx
  push    edx
  push    ebx
  lea     ecx, [ebp + 4 * 4]
  push    ecx                          ; ESP
  push    dword ptr [ebp]              ; EBP
  push    esi
  push    edi

;; UINT32  Gs, Fs, Es, Ds, Cs, Ss;
  mov  eax, ss
  push eax
  movzx eax, word ptr [ebp + 2 * 4]
  push eax
  mov  eax, ds
  push eax
  mov  eax, es
  push eax
  mov  eax, fs
  push eax
  mov  eax, gs
  push eax

;; UINT32  Eip;
  push    dword ptr [ebp + 1 * 4]

;; UINT32  Gdtr[2], Idtr[2];
  sub  esp, 8
  sidt fword ptr [esp]
  sub  esp, 8
  sgdt fword ptr [esp]

;; UINT32  Ldtr, Tr;
  xor  eax, eax
  str  ax
  push eax
  sldt ax
  push eax

;; UINT32  EFlags;
  push    dword ptr [ebp + 3 * 4]

;; UINT32  Cr0, Cr1, Cr2, Cr3, Cr4;
  mov  eax, cr4
  or   eax, 208h
  mov  cr4, eax
  push eax
  mov  eax, cr3
  push eax
  mov  eax, cr2
  push eax
  xor  eax, eax
  push eax
  mov  eax, cr0
  push eax

;; UINT32  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
  mov     eax, dr7
  push    eax
;; clear Dr7 while executing debugger itself
  xor     eax, eax
  mov     dr7, eax

  mov     eax, dr6
  push    eax
;; insure all status bits in dr6 are clear...
  xor     eax, eax
  mov     dr6, eax

  mov     eax, dr3
  push    eax
  mov     eax, dr2
  push    eax
  mov     eax, dr1
  push    eax
  mov     eax, dr0
  push    eax

;; FX_SAVE_STATE_IA32 FxSaveState;
  sub esp, 512
  mov edi, esp
  db 0fh, 0aeh, 00000111y ;fxsave [edi]

;; UINT32  ExceptionData;
  push    0

;; Prepare parameter and call
  mov     edx, esp
  push    edx
  push    mTimerVector
  call    TimerHandler
  add     esp, 8

  cli
;; UINT32  ExceptionData;
  add esp, 4

;; FX_SAVE_STATE_IA32 FxSaveState;
  mov esi, esp
  db 0fh, 0aeh, 00001110y ; fxrstor [esi]
  add esp, 512

;; UINT32  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
  pop     eax
  mov     dr0, eax
  pop     eax
  mov     dr1, eax
  pop     eax
  mov     dr2, eax
  pop     eax
  mov     dr3, eax
;; skip restore of dr6.  We cleared dr6 during the context save.
  add     esp, 4
  pop     eax
  mov     dr7, eax

;; UINT32  Cr0, Cr1, Cr2, Cr3, Cr4;
  pop     eax
  mov     cr0, eax
  add     esp, 4    ; not for Cr1
  pop     eax
  mov     cr2, eax
  pop     eax
  mov     cr3, eax
  pop     eax
  mov     cr4, eax

;; UINT32  EFlags;
  pop     dword ptr [ebp + 3 * 4]

;; UINT32  Ldtr, Tr;
;; UINT32  Gdtr[2], Idtr[2];
;; Best not let anyone mess with these particular registers...
  add     esp, 24

;; UINT32  Eip;
  pop     dword ptr [ebp + 1 * 4]

;; UINT32  Gs, Fs, Es, Ds, Cs, Ss;
;; NOTE - modified segment registers could hang the debugger...  We
;;        could attempt to insulate ourselves against this possibility,
;;        but that poses risks as well.
;;
  pop     gs
  pop     fs
  pop     es
  pop     ds
  pop     dword ptr [ebp + 2 * 4]
  pop     ss

;; UINT32  Edi, Esi, Ebp, Esp, Ebx, Edx, Ecx, Eax;
  pop     edi
  pop     esi
  add     esp, 4   ; not for ebp
  add     esp, 4   ; not for esp
  pop     ebx
  pop     edx
  pop     ecx
  pop     eax

  mov     esp, ebp
  pop     ebp
  iretd

SystemTimerHandler ENDP

END
