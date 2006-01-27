      TITLE   ThunkAsm.asm: Assembly code for the thunk protocol

;------------------------------------------------------------------------------
;
; Copyright 2004, Intel Corporation
; All rights reserved. This program and the accompanying materials
; are licensed and made available under the terms and conditions of the BSD License
; which accompanies this distribution.  The full text of the license may be found at
; http://opensource.org/licenses/bsd-license.php
;
; THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
; WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
;
; Module Name:
;   ThunkAsm.asm
;
;   Abstract:
;
;     This is the self-modifying code that is used for thunking from 64-bit
;     to 16-bit real mode. The code betwen ThunkStart and ThunkEnd gets copied
;     to memory < 1 MB that was allocated by the caller of RealModeTemplate().
;     LegacyBiosInt86() and LegacyBiosFarCall86() both call CallRealModeThunk()
;     to execute the 16-bit code. This code pokes (self modifies) it's self so
;     it can do a 16-bit INT or 16-bit FAR call.
;
;     WARNING: WARNING: WARNING: WARNING: WARNING: WARNING: WARNING: WARNING:
;
;      This code is tied tightly with Thunk.c there are C structs and info in
;       Thunk.inc that match tigthly so be very carefull if you change things.
;
;      This code is very dangerous to change. You must live in fear of the
;       crack! The crack exists in the processor as it changes mode. The
;       the changing of modes is not instantanous as descriptors may be cached.
;       Thus adding code when the processor is making a transition from real to
;       16-bit protected, to protected, to Long compatability mode, to Long
;       64-bit mode. All of these transitions expose a crack.
;
;
;     WARNING: WARNING: WARNING: WARNING: WARNING: WARNING: WARNING: WARNING:
;
;     The general Idea of the thunk code is to save current x64 state, swap
;      to the new GDT and walk our way down from Long 64-bit, to Long
;      compatability mode, to protected mode, to 16-bit protected mode to
;      real mode so we can do the 16-bit INT or FAR call and return.
;      We then restor the x64 state and we are done.
;
;------------------------------------------------------------------------------


include Thunk.inc

text SEGMENT

;----------------------------------------------------------------------------
; Procedure:    InterruptRedirectionTemplate: Redirects interrupts 0x68-0x6F
;
; Input:        None
;
; Output:       None
;
; Prototype:    VOID
;               InterruptRedirectionTemplate (
;                                VOID
;                                );
;
; Saves:        None
;
; Modified:     None
;
; Description:  Contains the code that is copied into low memory (below 640K).
;               This code reflects interrupts 0x68-0x6f to interrupts 0x08-0x0f.
;               This template must be copied into low memory, and the IDT entries
;               0x68-0x6F must be point to the low memory copy of this code.  Each
;               entry is 4 bytes long, so IDT entries 0x68-0x6F can be easily
;               computed.
;
;----------------------------------------------------------------------------

InterruptRedirectionTemplate PROC NEAR PUBLIC
  int     08h
  DB      0cfh          ; IRET
  nop
  int     09h
  DB      0cfh          ; IRET
  nop
  int     0ah
  DB      0cfh          ; IRET
  nop
  int     0bh
  DB      0cfh          ; IRET
  nop
  int     0ch
  DB      0cfh          ; IRET
  nop
  int     0dh
  DB      0cfh          ; IRET
  nop
  int     0eh
  DB      0cfh          ; IRET
  nop
  int     0fh
  DB      0cfh          ; IRET
  nop
InterruptRedirectionTemplate ENDP

;----------------------------------------------------------------------------
; Procedure:    RealModeTemplate: Prepares thunk and reverse thunk code for later use.
;
;
; Prototype:    UINTN
;               RealModeTemplate (
;                 OUT UINTN                RealSegment,
;                 OUT UINTN                *CodeEnd,
;                 IN OUT LOW_MEMORY_THUNK  *IntThunk,
;                 IN  UINT32               TrapoleanCr4
;                 );
;
; Arguments:
;   RealSegment  - Real mode segment (Seg:Offset) of buffer < 1MB
;   CodeEnd      - End of thunk code
;   IntThunk     - Structure containing values used to patch code
;   TrapoleanCr4 - Temp Cr4 value that maps memory < 1MB and
;
; Return Value:
;   Start of thunk code
;
; Description:  This procedure is called during initialization to prepare the
;               code that is later used for thunking and reverse thunking.
;               It modifies itself so that offesets, etc. are correct.  Part
;               of the code is actually only modified at this time and not
;               actually executed.  This code is copied to low memory by the
;               caller of this method and is later executed by methods such as
;               LegacyBiosInt86() and LegacyBiosFarCall86().
;               That section of code modifies itself again, does the thunk or
;               reverse thunk, executes the INT or far call, and returns.
;
;               The code that is to be copied and used in subsequent thunks
;               and reverse thunks will be referred to as the "prepared code".
;
;               The legacy BIOS is responsible for enabling the A20 gate when
;               a "Gate A20 Support, Enable A20 Gate" system service call
;               (INT 15 with AX = 240h) is made.  This code makes the INT 15
;               call and assumes it succeeds.
;;
;----------------------------------------------------------------------------
; RCX - UINT64            RealSegment
; RDX - UINT64            *CodeEnd
; R8  - LOW_MEMORY_THUNK  *IntThunk
; R9  - UINT32            TrapoleanCr4
;
; RAX - CodeStart
;
RealModeTemplate PROC NEAR  PUBLIC
		push     r9
		push     rsi
    push     rcx
;
; Patch in the address of gIntThunk
;   The address must be patched because assembly instructions that reference
;   IntThunk will be converted to an offset relative to the current stack,
;   which is not the current stack when the prepared code is run.
;
    mov     rax, r8
    mov     qword ptr [IntThunkAddr2], rax  ; 8 bytes at IntThunkAddr2 =
                                            ; .. current address of IntThunk
    mov     rax, offset LOW_MEMORY_THUNK.RealModeGdtDesc
    mov     word ptr [PatchOffsetRealModeGdtDesc], ax   ; 2 bytes at IntThunkAddr2 =
                                            ; .. current address of IntThunk
                                            ; O.K. to truncate as < 1MB address
;
;   patch the address of CompatibilityModeVector
;
    mov     rax, offset LOW_MEMORY_THUNK.CodeBuffer  ; RAX = &(IntThunk->CodeBuffer)
    add     rax, r8                      ; rax:  the start address of code buffer

    mov     rcx, offset ThunkStart

    mov     r9,  offset CompatibilityMode
    sub     r9,  rcx
    mov     rsi, rax
    add     rsi, r9
    mov     dword  ptr[CompatibilityModeVector], esi

    mov     r9,  offset Backto32BitProtectedMode
    sub     r9,  rcx
    mov     rsi, rax
    add     rsi, r9
    mov     dword  ptr[OffsetBackto32BitProtectedMode], esi

    mov     r9,  offset InLongMode
    sub     r9,  rcx
    mov     rsi, rax
    add     rsi, r9
    mov     dword  ptr[OffsetInLongMode], esi
;
;   patch the address of ProtectedModeOff
;
     mov     rax,  offset ProtectedModeOff
     sub     rax,  rcx
     mov     dword ptr[Go16BitProtectedMode], eax

     mov     rax,  offset FixSSInstruction
     sub     rax,  rcx
     mov     word ptr[PatchRealModeOffset], ax

     pop     rcx
     pop     rsi
     pop     r9

;
; Poke the TrampoleanCr3 value into the code.
;
    mov     rax, r9
    mov     dword ptr[TrampoleanCr3], eax

;
; Patch in passed in real mode segment information.
;  Assume all real mode segments point to ThunkStart
;
    mov     word ptr [PatchRealModeCS], cx
    mov     word ptr [PatchRealModeSS], cx

;
; patch in the current ds so we can use it to return to x64 mode
;
    xor     rax, rax
    mov     ax, ds
    mov     qword ptr [PatchX64Ds1], rax
;    mov     word ptr [PatchX64Ds2], ax

    mov     ax, cs
    mov     word ptr [PatchLongModeCS], ax
;
; Return the offset to the End of the thunk code.
;  Offset of the start of the code (ThunkStart) is returned in RAX
;
    mov     rax, offset ThunkEnd
    mov     qword ptr [rdx], rax        ; *CodeEnd = &ThunkEnd
;
; Return CodeStart
;
    mov     rax, offset ThunkStart
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;        End Constructor Code           ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;     Begin "Prepared" Thunk Code       ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
; Real mode int/far call thunk code (Long Mode to 16-bit real mode)
;
;       RCX determines whether to do an INT or far call
;           (do not write RCX until after the patching code below)
;           if (0 == RCX)
;               INT
;           else
;               RCX is the call address
;               far call
;       DL = BiosInt# to invoke (do not write RDX/DX until it is used)
;
; UINTN
; CallRealModeThunk (
;   IN UINTN            CallAddress   // rcx
;   IN UINT8            BiosInt,      // rdx
;   IN LOW_MEMORY_THUNK *IntThunk     // r8
;   )
;
     align 16
ThunkStart:
CallRealModeThunk:
    pushf                         ; Save flags (note that this is before the cli)
    cli

;
; Save C regs
;
    push    rdi
    push    rsi
    push    rbp
    push    rbx
    push    r12
    push    r13
    push    r14
    push    r15
;
; BugBug: ABI states we need to save XMM6-XMM15
;         Only XMM6-XMM7 are availible in all modes.
;         XMM8-XMM15 are only availible in 64-bit mode
;

;
; Save flat x64 stack location (RSP)
;   (NOTE: The caller of CallRealThunkCode is responsible for saving the flat SS to FlatSs.)
;
    mov     (LOW_MEMORY_THUNK PTR [r8]).x64Esp, rsp

;
; Patch in BiosInt or far call insturction into low memory thunk
;
    mov     rax, r8
    add     rax, offset LOW_MEMORY_THUNK.CodeBuffer  ; RAX = &(IntThunk->CodeBuffer)
    mov     rdi, offset RealModePatch
    add     rax, rdi
    mov     rdi, offset ThunkStart
    sub     rax, rdi

    ; RAX now points to the RealModePatch:
    ; RAX = IntThunk->CodeBuffer + (offset (RealModePatch) - offset (ThunkStart))
    ;

    cmp     rcx,0                       ; check if 0 == rcx
    jne     PatchFarCall                ; if (0 != rcx) jump to PatchFarCall

;
; Patch Bios INT instruction at RealModePatch - 1
;   (The 3 NOPs are to write over part of the far call patch that may be left
;    .. from a previous execution of this code.)
    mov     byte ptr [rax-1], 0cdh      ; INT inst
                                        ;   1 byte at (RealModePatch - 1) = 'INT imm8' opcode
    mov     byte ptr [rax], dl          ; INT #
                                        ;   1 byte at RealModePatch = interrupt number
                                        ;       (passed by the caller in CL)
    mov     byte ptr [rax+1], NOP_OPCODE    ; NOP \
    mov     byte ptr [rax+2], NOP_OPCODE    ; NOP  | 3 bytes at (RealModePatch + 1) = 'NOP' opcode
    mov     byte ptr [rax+3], NOP_OPCODE    ; NOP /
    jmp     ThunkPatchComplete          ; Jump over the far call patching code.

;
; Patch far call instruction at RealModePatch - 1
;
PatchFarCall:
    mov     byte ptr [rax-1], 09Ah      ; CALL inst
                                        ;   1 byte at (RealModePatch - 1) = 'call far' opcode
                                        ;   .. (absolute, address given in operand)
    mov     dword ptr [rax], ecx         ; SEG:OFF
                                        ;   4 bytes at RealModePatch = SEG:OFF
                                        ;       (passed by the caller in EDX)
ThunkPatchComplete:
;
; It is now safe to use ECX/CX and EDX/DX.
;
			; get the relocated base address
			mov    rax, r8
			add    rax, offset LOW_MEMORY_THUNK.CodeBuffer  ; RAX = &(IntThunk->CodeBuffer)
;
; get the Compatibility Mode jump Vector's base address to fill the jump target address
;
			mov    rcx, offset CompatibilityModeVector
      mov    rdi, offset ThunkStart
			sub    rcx, rdi
			add    rcx, rax

;
;    save r8 to rsi for future use in protected mode
;
      mov    rsi, r8

;
; jmp into cmpatibility mode
;
      jmp    FWORD PTR[rcx]

CompatibilityMode:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;     now in 64-bit compatibility mode           ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; disable paging
;

      mov	   rax, cr0
      btr    eax, 31    ; set PG=0
      mov    cr0, rax
;
;  set EFER.LME = 0 to leave long mode
;
     mov     ecx, 0c0000080h ; EFER MSR number.
     rdmsr                   ; Read EFER.
     btr     eax, 8          ; Set LME=0.
     wrmsr                   ; Write EFER.

     jmp     Legacy32bitProtectedMode
Legacy32bitProtectedMode:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;     now in 32-bit legacy mode                  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Set IDTR for real mode IVT
     mov     ecx, offset LOW_MEMORY_THUNK.RealModeIdtDesc
     add     ecx, esi
;
;     lidt    FWORD PTR [ecx]
;
     DB      0fh
     DB      01h
     DB      19h

;
; Get the real mode data selector and stack BEFORE loading the GDT!
;
    mov     eax, REAL_DATA_SELECTOR
    mov     ecx, offset LOW_MEMORY_THUNK.RealStack
    add     ecx, esi

    DB      8bh   ; mov     ebx, DWORD PTR [ecx]
    DB      19h
;
; If using an in-target probe, you must not break or step between here and the
; instruction after the jump to 16-bit protected mode.
;

;
; Set GDTR for real mode segments
;
    mov    ecx, offset LOW_MEMORY_THUNK.RealModeGdtDesc
    add    ecx, esi
    ;
    ; lgdt    FWORD PTR [ecx]
    ;
    DB  0fh
    DB  01h
    DB  11h

;
; Load real-mode-style selectors and stack pointer
;
    mov     gs, ax
    mov     fs, ax
    mov     es, ax
    mov     ds, ax
    mov     ss, ax
    mov     esp,ebx

;;
;;
    mov rax, cr4
    btr eax, 5
    mov cr4, rax
;
; Go to 16-bit protected mode using a jump.
;   The offset and selector below were patched by the one-time code.
;
; jmp ptr16:32
;
    DB   0EAh                           ; 'jmp far' opcode (absolute, address given in operand)
Go16BitProtectedMode:
    DD      ?                           ; offset    (ProtectedModeOff)
    DW      REAL_CODE_SELECTOR          ; selector  (16bit CS)
ProtectedModeOff:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;         16-bit protected mode         ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
; We are now in 16-bit protected mode, on the low memory
;

;
; NOTE: Now that we are in 16-bit mode, we must use the operand-size override
;       prefix when we want to use 32-bit opperands.
;

;
; Turn protected mode off
;
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    mov     rax, cr0
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    and     eax, not CR0_PE
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    and     eax, not CR0_PG
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    mov     cr0, rax                    ; Clear PE in CR0.
                                        ; Clear PG in CR0.

;
; Go to real mode using a jump.
;   The offset and segment below were patched by the one-time code.
;
; jmp ptr16:16
;
    DB      0EAh                        ; 'jmp far' opcode (absolute, address given in operand)
PatchRealModeOffset:
    DW      0005h                       ; offset $+5   (FixSSInstruction)
PatchRealModeCS:
    DW      0000h                       ; segment  (realmode cs)
FixSSInstruction:


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;          (16-bit) real mode           ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
; We are now in real mode
;

;
; Fix SS and DS
;
    DB      0B8h                        ; 'mov ax, imm16' opcode
PatchRealModeSS:
    DW      0000h                       ; (realmode ss)

    DB      08Eh                        ; mov ss, ax
    DB      0D0h

    DB      08Eh
    DB      0D8h                        ; mov ds, ax

    sti
;
; Load regs with caller's request
;   NOTE1: that this pop sequence matches the
;          EFI_IA32_REGISTER_SET structure.
;   NOTE2: the following instructions are really executed
;          as 16-bit instructions as we are in real mode
;   NOTE3: Data is put on real mode stack by C code EfiCopyMem to Stack16
;
;
    pop     rax
    pop     rbx
    pop     rcx
    pop     rdx
    pop     rsi
    pop     rdi
    ;;;popf
	  DB      09Dh

;;;    pop     es
    DB      07h
    pop     rbp         ; throw away cs
    pop     rbp         ; throw away ss
    DB      1fh
    pop     rbp

;
; issue INT or a FAR CALL
;   The correct opcode and arguments were patched by the "prepared code" above.
;
; int #
;   OR
; call ptr16:16
;
    DB   0CDh           ; 'INT' or 'far call' op code - Patched location
RealModePatch:
    DB   00             ; INT # or OFFSET - Patched
    DB   NOP_OPCODE     ;NOP or OFFSET  - Patched
    DB   NOP_OPCODE     ; NOP or SEGMENT - Patched
    DB   NOP_OPCODE     ; NOP or SEGMENT - Patched

;
; We have executed the INT or far call and returned here.
;

;
; Save the result registers.
;   See the above NOTEs.
;
    push    rbp
    mov     bp, ax    ; save eax

;;;    push    ds
    DB      1eh

    push    rax         ; throw away ss
    push    rax         ; throw away cs

;;;    push    es
    DB      06h

    DB      09Ch
    push    rdi
    push    rsi
    push    rdx
    push    rcx
    push    rbx
    push    rbp         ; ax results are in bp


;
; The A20 gate may have been changed in the real mode code.
; It needs to be correctly set before continuing in protected mode.
; The legacy BIOS is responsible for restoring the A20 state.
;

;
; Make an INT 15 call to the legacy BIOS to enable the A20 Gate if necessary.
;   2401h in AX is the argument
;   Function 24h (in AH) indicates "Gate A20 Support".
;   01h in AL is a sub-argument that indicates "Enable A20 Gate"
;
;  Use rdi as a flag to return status
;
    xor     di, di

;
;  mov     ax,2401h
;
    DB      0b8h                        ; 'mov ax,imm16' opcode
    DW      02401h                      ; Gate A20 Support & Enable A20 Gate argument (imm16)

;
; make the System Service call
;
    int     15h

;
; Check if the INT 15 succeeded.
;   According to the Bios Reference Manual, if CF == 1, the function is
;   not supported.  If CF is 0, the function is supported, so check if it
;   succeeded.  If AH == 0, it succeeded.  If the Int15 Function 24 call is
;   not supported or fails, we set the internal status register and continue,
;   assuming that A20 is in the correct state.  The caller should check the
;   external status register.
    jnc     Int15F24Supported           ; If supported (CF == 0), jump.

    DB     OPERAND_SIZE_OVERRIDE_PREFIX    ; override size to write 32-bit status code.
    mov    edi, THUNK_ERR_A20_UNSUP    ; set status
    jmp     Int15Finished               ; continue.  Assume A20 is correct.


Int15F24Supported:
    cmp     ah, 0
    je      Int15Finished               ; If successful (AH == 0), jump.
    DB      OPERAND_SIZE_OVERRIDE_PREFIX    ; override size to write 32-bit status code.
    mov     edi, THUNK_ERR_A20_FAILED   ; set status and fall through. Assume
                                        ; .. A20 is correct.

Int15Finished:
    cli

;
; BugBug: I replaced the stack code
;

;
; esi = IntThunk
;   This is used to restore several values below.
;   The address of IntThunk was patched by the one-time code.
;

   ; mov     bx, offset LOW_MEMORY_THUNK.RealModeGdtDesc
    DB      0bbh
PatchOffsetRealModeGdtDesc:
    DW      0000h


;
; set DS = SS since ds may be changed by the far call
;

    ;push    ss                          ;\ Set the Data Segment to be the same as the Stack Segment
    DB      016h
    ; pop     ds                         ;/      (DS = SS)
    DB      01Fh
;
;
; Do the actual LGDT.
;   The GDT limit and base are on the stack, pointed to by BX.
;
; lgdt    fword ptr ds:[bx]
;
    DB      OPERAND_SIZE_OVERRIDE_PREFIX    ; Override prefix specifies 32-bit base
                                            ; .. address rather than 24-bit.
    DB      0Fh                         ; \ LGDT opcode (0f 01)
    DB      01h                         ; /
    DB      17h                         ; ModR/M byte - specifies BX register
                                        ;   NOTE: It will appear to be a
                                        ;   .. different register if you use
                                        ;   .. a 32-bit disassembler.

;
; If using an in-target probe, you must not break or step between here and the
; instruction after the jump to 32-bit flat mode.
;

;
; Turn protected mode on
;
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    mov     rax, cr0
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    or      eax, CR0_PE
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    mov     cr0, rax                    ; Set PE in CR0.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;         16-bit protected mode         ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
; Return to 32-bit flat mode
;
;
; Put flat DS in AX
;   The stored value is used after the jump below.
;   The value of the flat DS was patched by the one-time code.
;   The patched DS is the value of DS at that time).
;
    DB      0B8h                        ; 'mov ax,imm16' opcode
    DW      PROTECTED_DATA_SELECTOR     ; flat DS (imm16)

;
; Return to 32-bit CS
;   The offset and selector below were patched by the one-time code.
;   The patched CS is the value of CS at that time.
;

    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    DB      0EAh                        ; jmp far 16:32
OffsetBackto32BitProtectedMode:
    DD      00000008h                   ; offset $+8   (Backto32BitProtectedMode)
    DW      PROTECTED_CODE_SELECTOR     ; selector  (flat CS)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;         32-bit protected mode         ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Backto32BitProtectedMode:
;

;
; Go Long Mode HERE!!!!!
;


    ;
    ; Enable the 64-bit page-translation-table entries by
    ; setting CR4.PAE=1 (this is _required_ before activating
    ; long mode). Paging is not enabled until after long mode
    ; is enabled.
    ;
    mov rax, cr4
    bts eax, 5
    mov cr4, rax

    ;
    ; This is the Trapolean Page Tables that are guarenteed
    ;  under 4GB. They actually only map 0 - 2 MB.
    ;
;;; mov eax, imm32
    DB      0B8h                          ; 'mov eax,imm32' opcode
TrampoleanCr3:
    DD  00000000                          ; imm32

    mov cr3, rax

    ;
    ; Enable long mode (set EFER.LME=1).
    ;
    mov   ecx, 0c0000080h ; EFER MSR number.
    rdmsr                 ; Read EFER.
    bts   eax, 8          ; Set LME=1.
    wrmsr                 ; Write EFER.

    ;
    ; Enable paging to activate long mode (set CR0.PG=1)
    ;
    mov   rax, cr0        ; Read CR0.
    bts   eax, 31         ; Set PG=1.
    mov   cr0, rax        ; Write CR0.
    jmp   GoToLongMode
GoToLongMode:

    ;
    ; This is the next instruction after enabling paging.  Jump to long mode
    ;
    db      067h
    db      0eah                ; Far Jump $+9:Selector to reload CS
OffsetInLongMode:
    dd      00000009            ;   $+9 Offset is ensuing instruction boundary
PatchLongModeCS:
    dw      0000h               ;   Selector is our code selector, 10h
InLongMode:
    ; mov ax, imm16
    DB      048h
    DB      0B8h
PatchX64Ds1:
    DQ      0000h

    mov     es, ax
    mov     ss, ax
    mov     ds, ax


;
; BugBug: Restore Long Mode DS
;
;    DB      0B8h                        ; 'mov ax,imm16' opcode
;PatchX64Ds2:
;    DW      0000                        ; flat DS (imm16)
;    mov     es, ax
;    mov     ss, ax
;    mov     ds, ax


;
; esi = IntThunk
;   This is used to restore several values below.
;   The address of IntThunk was patched by the one-time code.
;
    DB      048h
    DB      0BEh                        ; 'mov rsi,imm64' opcode (0xB8 + 6)
IntThunkAddr2:
    DQ      0000000000000000h                   ; address of IntThunk (imm64)

;
; Restore 32-bit IDT
;
;!Fix Me!;    lidt    (LOW_MEMORY_THUNK PTR [rsi]).IdtDesc
;    DB      0fh
;    DB      01h
;    DB      /3

;
; Restore 64-bit stack
;
    mov     rcx, (LOW_MEMORY_THUNK PTR [rsi]).x64Ss
    mov     ss, cx
    mov     rsp, (LOW_MEMORY_THUNK PTR [rsi]).x64Esp

;
; Switch back to the original page tables
;
    mov     rax, (LOW_MEMORY_THUNK PTR [rsi]).PageMapLevel4
    mov     cr3, rax

;
; restore the callers GDT
;
    mov     rax, offset LOW_MEMORY_THUNK.x64GdtDesc
    add     rax, rsi
    lgdt    FWORD PTR [rax]

    mov     rax,  offset LOW_MEMORY_THUNK.x64IdtDesc
    add     rax, rsi
    lidt    FWORD PTR [rax]

;
; Mark the LDTR as invalid
;
    xor     eax, eax                    ; short/quick way to set EAX = 0
    lldt    ax                          ; (source operand == 0) => LDTR is invalid

;
; Set the external status register.
;   Since we stored status internally in EDI, we must copy that value to our
;   external status register, EAX.  We must do this before restoring the caller's
;   EDI.
;
    mov rax, rdi

;
; Restore C regs
;
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    pop     rbp
    pop     rsi
    pop     rdi
    popf

;
; return
;   The ret opcode is DB'd below because MASM replaces the "ret" instruction
;   with "leave" and "ret".  This is because we used PROTO C for
;   RealModeTemplate().  This if fine when we call RealModeTemplate(), but when
;   we call the prepared code in low memory, call is used, not enter, so the
;   leave instruction pops too many items off the stack.
;
    DB      0C3h                        ; ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;     End of "Prepared" Thunk Code      ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    align    16
CompatibilityModeVector:
    DD		?
    DW    20h

;
; Long mode far call reverse thunk code (16-bit real mode to 64-bit Long mode)
;
; Since currently nobody will use it, this function just return directly


ReverseThunkCode:
		nop
    DB      0C3h                        ; ret


ThunkEnd:
    nop                                 ; Used to mark the end of the "prepared" code

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;     End of "Prepared" Thunk Code      ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

RealModeTemplate ENDP



;----------------------------------------------------------------------
;VOID
;GetRegisters (
;  IN LOW_MEMORY_THUNK  *IntThunk  // rcx
;  )
;
; Routine Description:
;   Save x64 processor state so it can be restored later
;
; Arguments:
;   IntThunk - Pointer to IntThunk of Legacy BIOS context.
;
; Returns:
;   None
;
;----------------------------------------------------------------------
GetRegisters PROC NEAR PUBLIC

  sgdt    (LOW_MEMORY_THUNK PTR [rcx]).x64GdtDesc
  sidt    (LOW_MEMORY_THUNK PTR [rcx]).x64IdtDesc

  xor     rax, rax                                  ; zero RAX
  mov     ax, ss                                    ; read in the stack segment
  mov     (LOW_MEMORY_THUNK PTR [rcx]).x64Ss, rax  ; save in data structure
  mov     rax, cr3
  mov     (LOW_MEMORY_THUNK PTR [rcx]).PageMapLevel4, rax
  ret

GetRegisters ENDP


END
