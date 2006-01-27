      TITLE   ThunkAsm.asm: Assembly code for the thunk protocol

;------------------------------------------------------------------------------
;*
;* Copyright (c) 2005, Intel Corporation                                                         
;* All rights reserved. This program and the accompanying materials                          
;* are licensed and made available under the terms and conditions of the BSD License         
;* which accompanies this distribution.  The full text of the license may be found at        
;* http://opensource.org/licenses/bsd-license.php                                            
;*                                                                                           
;* THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
;* WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
;* 
;* Module Name:
;*
;*   CpuAsm.asm
;* 
;* Abstract:
;*  
;*     This is the self-modifying code that is used for thunking from 32-bit
;*     protected mode (with flat memory model) to 16-bit real mode.  This code
;*     sets up code in a region of memory.  That region of memory is then copied
;*     to low memory by LegacyBiosInt86() and LegacyBiosFarCall86() and then
;*     executed.  That code modifies itself again, does the thunk to 16-bit mode,
;*     executes the INT or far call, and returns to 32-bit mode.
;*
;*   BugBug:
;*
;*      Several times in this code, we access values at 20-bit addresses by
;*      chopping the 5th character off and using it for the segment.  If the
;*      first 16 bits are low (stack) or high, there is a chance that the
;*      accesses will wrap.  This should be checked for and fixed!
;*      Since we know the base of IntThunk and it is 4k-aligned, we can
;*      delete the base from any 20-bit addresses and use the remainder as
;*      the offset.  As long as IntThunk is smaller 64k, the address should
;*      never wrap beyond the segment.
;*
;****************************************************************************;*


include Thunk.inc


.686P
.MODEL FLAT, C
.CODE


;----------------------------------------------------------------------------
; Procedure:    RealModeTemplate: Prepares thunk and reverse thunk code for later use.
;
; Input:        IntThunk - Structure containing values used to patch code
;
; Output:       CodeStart - Start address of the code to be copied for the thunk.
;                              It is the same as "ThunkStart"!
;               CodeEnd - End address of the code to be copied for the thunk.
;                            The memory between CodeStart and CodeEnd includes the
;                            code for both Thunk and Reverse Thunk.
;
; Prototype:    VOID
;               RealModeTemplate (  
;                 OUT UINT32          *CodeStart,
;                 OUT UINT32          *CodeEnd,
;                 IN LOW_MEMORY_THUNK *IntThunk
;                 );
;
; Saves:
;
; Modified:     EAX, ECX, EDX   (EBP, ESI, EDI, EDS, and SS are not modified.)
;               The prepared code modifies the following, though all are not
;               modified during a call to this procedure:
;                   EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP, CS, DS, ES, FS, GS, SS, EFLAGS
;               The following registers are restored before exiting the Thunk
;               "function":
;                   EBX, EDX, ESI, EDI, EBP, CS, DS, EFLAGS
;               The following registers are restored before exiting the Reverse
;               Thunk "function":
; bugbug - list may change when interface is defined!
;                   EBX, EDX, ESI, EDI, EBP, CS, DS, EFLAGS
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
;               This procedure should be called from 32-bit protected mode.
;               The prepared code includes 32-bit protected, 16-bit protected 
;               and 16-bit real mode code.  The thunk code is should be executed
;               from 32-bit protected mode, and the reverse thunk code should
;               executed from 16-bit real mode.
;
;               The code that is executed when RealModeTemplate is called will
;               be referred to as "one-time code".
;               The code that is to be copied and used in subsequent thunks
;               and reverse thunks will be referred to as the "prepared code".
;
;               The legacy BIOS is responsible for enabling the A20 gate when
;               a "Gate A20 Support, Enable A20 Gate" system service call
;               (INT 15 with AX = 240h) is made.  This code makes the INT 15
;               call and assumes it succeeds.
;
;               Both thunk and reverse thunk assume that the flat CS is the same
;               as when the one-time code was executed.
;
;               Various IntThunk fields must be saved at different times.
;               The caller of RealModeTemplate must save:
;                   RealCodeSelector
;               The caller of the prepared thunk code is responsible for saving:
;                   GdtDesc
;                   IdtDesc
;                   FlatSs
;                 And must prepare:
;                   RealStack
;                   RealModeGdt
;               The following may be set at any time before a thunk (during init
;               or before each thunk):
;                   RealDataSelector
;                   RealModeGdtDesc
;                   RealModeIdtDesc
;               The following may be set at any time before a reverse thunk
;               (during init or before eachb reverse thunk):
;                   RevFlatDataSelector
;                   RevFlatStack
;
;               It is not permitted to nest more than one thunk or reverse
;               thunk.  A reverse thunk may only be executed after a thunk.
;
;               The prepared thunk code returns a status value in the EAX
;               register.  This status value only relates to errors with the
;               thunk.  The caller and target code must have their own mechanism
;               for reporting and handling errors through the registers and/or
;               stack.  The caller is responsible for reading the EAX register
;               and taking actions based on the value.  (NOTE: EDI contains the
;               status code during the thunk code.  It is not copied to EAX until
;               just before the prepared thunk code returns to the caller.
;
;----------------------------------------------------------------------------

RealModeTemplate PROC  C  \
   RealSegment:  UINT32,  \
   CodeStart:PTR UINT32,  \
   CodeEnd:PTR   UINT32,  \
   IntThunk:PTR  LOW_MEMORY_THUNK

;
; Set the output parameters to the address of the beginning of the
; prepared code (= beginning of thunk code), end of the the prepared
; of code, and the beginning of the reverse thunk code.
;
    mov     eax, CodeStart
    mov     ecx, offset ThunkStart
    mov     dword ptr [eax], ecx        ; *CodeStart = &ThunkStart

    mov     eax, CodeEnd
    mov     ecx, offset ThunkEnd
    mov     dword ptr [eax], ecx        ; *CodeEnd = &ThunkEnd


;
; Patch in the address of IntThunk
;   The address must be patched because assembly instructions that reference
;   IntThunk will be converted to an offset relative to the current stack,
;   which is not the current stack when the prepared code is run.
;
    mov     ecx, IntThunk
    mov     dword ptr [IntThunkAddr2], ecx  ; 4 bytes at IntThunkAddr2 =
                                            ; .. current address of IntThunk
    mov     dword ptr [IntThunkAddr3], ecx  ; 4 bytes at IntThunkAddr3 =
                                            ; .. current address of IntThunk

;
; Patch in passed in real mode segment information.
;  Assume all real mode segments point to ThunkStart
;
    mov     ecx, RealSegment
    mov     word ptr [PatchRealModeCS], cx    
    mov     word ptr [PatchRealModeSS], cx       

;  BugBug: Don't check *LowCodeStart like the original C code did.
;          Why did it check anyway?  If there was a failure, it would
;          run the actual thunk code, which it shouldn't.

;
; MASM turns this ret into a leave-ret combination.  Therfore,
; EBP better be the same as when this function was entered.
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;        End Initialization Code        ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;     Begin "Prepared" Thunk Code       ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; 
; Real mode int/far call thunk code (32-bit protected to 16-bit real mode)
;
;       EDX determines whether to do an INT or far call
;           (do not write EDX until after the patching code below)
;           EDX is set by the calling function (i.e. LegacyBiosInt86, LegacyBiosFarCall86)
;           if (0 == EDX)
;               INT
;           else
;               EDX is the call address
;               far call
;       CL = BiosInt# to invoke (do not write ECX/CX until it is used)
; 
; UINTN
; CallRealModeThunk (
;   IN UINTN            CallAddress   
;   IN UINT8            BiosInt,      
;   IN LOW_MEMORY_THUNK *IntThunk     
;   )

; 
; Save protected mode registers
; 
ThunkStart:
_CallRealModeThunk:

    mov     ecx, [esp + 4]    ; ecx == BiosInt
    mov     edx, [esp + 8]    ; edx == CallAddress
    mov     esi, [esp + 12]   ; esi == IntThunk
    
    pushfd                          ; Save flags (note that this is before the cli)
    cli

;
; Save C regs
;
    push    edi
    push    esi
    push    ebp
    push    edx
    push    ebx

;
; Set the internal status code register to None
;   EDI will contain the status code for the duration of the prepared code.
;   It must not be set in this section of the code for any reason except to
;   specify that an error has occurred during the thunk.
;
    mov     edi, THUNK_OK         ; clear all 32 bits!

; 
; Preserve the CR0, CR3, and CR4 register by pushing it onto the stack
; This is necessary when physical memory is greater than 4GB
;
    mov     eax, cr0
    push    eax
    mov     eax, cr4
    push    eax
    mov     eax, cr3
    push    eax   

;
; Save flat 32-bit registers
;   Only need to save DS and ESP.
;   The caller of RealModeTemplate is responsible for saving the flat SS to FlatSs.
;   CS was patched in the one-time code.
;   ES, FS, and GS are the same as DS.
;   GDTR and IDTR were saved to IntThunk by the caller.
;

; 
; Save flat 32-bit stack location (ESP)
;   (NOTE: The caller of RealModeTemplate is responsible for saving the flat SS to FlatSs.)
; 
    mov     (LOW_MEMORY_THUNK PTR [esi]).FlatEsp, esp

; 
; Patch in BiosInt or far call insturction into low memory thunk
; 
    mov     eax, esi
    add     eax, offset LOW_MEMORY_THUNK.CodeBuffer   ; EAX = &(IntThunk->Code)
    add     eax, offset PatchRealIntOrCall
    sub     eax, offset ThunkStart            ; EAX = IntThunk->Code + offset of PatchRealIntOrCall
                                        ; .. (rel. to ThunkStart)
                                        ; Note that addr (IntThunk->code) ==
                                        ; .. where the code beginning at ThunkStart is
                                        ; .. now located

    cmp     edx,0                       ; check if 0 == EDX
    jne     PatchFarCall                ; if (0 != EDX) jump to PatchFarCall
                                        ; else continue

;
; Patch Bios INT instruction at PatchRealIntOrCall - 1
;   (The 3 NOPs are to write over part of the far call patch that may be left
;    .. from a previous execution of this code.)
    mov     byte ptr [eax-1], 0cdh      ; INT inst
                                        ;   1 byte at (PatchRealIntOrCall - 1) = 'INT imm8' opcode
    mov     byte ptr [eax], cl          ; INT #
                                        ;   1 byte at PatchRealIntOrCall = interrupt number
                                        ;       (passed by the caller in CL)
    mov     byte ptr [eax+1], NOP_OPCODE    ; NOP \
    mov     byte ptr [eax+2], NOP_OPCODE    ; NOP  | 3 bytes at (PatchRealIntOrCall + 1) = 'NOP' opcode
    mov     byte ptr [eax+3], NOP_OPCODE    ; NOP /
    jmp     ThunkPatchComplete          ; Jump over the far call patching code.

;
; Patch far call instruction at PatchRealIntOrCall - 1
;
PatchFarCall:
    mov     byte ptr [eax-1], 09Ah      ; CALL inst
                                        ;   1 byte at (PatchRealIntOrCall - 1) = 'call far' opcode
                                        ;   .. (absolute, address given in operand)
    mov     dword ptr [eax], edx        ; SEG:OFF
                                        ;   4 bytes at PatchRealIntOrCall = SEG:OFF
                                        ;       (passed by the caller in EDX)
ThunkPatchComplete:
;
; It is now safe to use ECX/CX and EDX/DX.
;


; 
; Set IDTR for real mode IVT
; 
    lidt    (LOW_MEMORY_THUNK PTR [esi]).RealModeIdtDesc

;
; Get the real mode data selector and stack BEFORE loading the GDT!
;
    mov     ebx, (LOW_MEMORY_THUNK PTR [esi]).RealStack

;
; If using an in-target probe, you must not break or step between here and the
; instruction after the jump to 16-bit protected mode.
;

;
; Set GDTR for real mode segments
;
    lgdt    (LOW_MEMORY_THUNK PTR [esi]).RealModeGdtDesc

; 
; Load real-mode-style selectors and stack pointer
; 
    mov     ax, REAL_DATA_SELECTOR
    mov     gs, ax
    mov     fs, ax
    mov     es, ax
    mov     ds, ax
    mov     ss, ax
    mov     esp, ebx

;
; Go to 16-bit protected mode using a jump.
;   The offset and selector below were patched by the one-time code.
;
; jmp ptr16:32
;
    DB   0EAh                           ; 'jmp far' opcode (absolute, address given in operand)
    DD      $+7                         ; offset    (ProtectedModeOff)
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
    mov     eax, cr0
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    and     eax, not CR0_PE
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    and     eax, not CR0_PG  
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    mov     cr0, eax                    ; Clear PE in CR0.
                                        ; Clear PG in CR0.

;
; Go to real mode using a jump.
;   The offset and segment below were patched by the one-time code.
;
; jmp ptr16:16
;
    DB      0EAh                        ; 'jmp far' opcode (absolute, address given in operand)
GoReal:  
    DW      0005h                        ; offset $+5 (FixSSInstruction)
PatchRealModeCS:
    DW      0000h                       ; segment  (realmode cs)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;          (16-bit) real mode           ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; 
; We are now in real mode
;

;
; Fix SS
; 
FixSSInstruction:
    DB      0B8h                        ; 'mov ax, imm16' opcode
PatchRealModeSS:  
    DW      0000h                       ; (realmode ss)

    DB      08Eh                        ; mov ss, ax
    DB      0D0h

    sti
;
; Load regs with caller's request
;   NOTE1: that this pop sequence matches the
;          EFI_IA32_REGISTER_SET structure.
;   NOTE2: the following instructions are really executed
;          as 16-bit instructions as we are in real mode
;
    pop     eax
    pop     ebx
    pop     ecx
    pop     edx
    pop     esi
    pop     edi
    jmp $
    popfd
    pop     es
    pop     ebp         ; throw away cs
    pop     ebp         ; throw away ss
    pop     ds
    pop     ebp

;
; issue INT or a FAR CALL
;   The correct opcode and arguments were patched by the "prepared code" above.
;
; int #
;   OR
; call ptr16:16
;
;BugBug - Why is it initially 0CDh?  As a sanity check to cause an INT0 if this was not patched?
    DB   0CDh           ; 'INT' or 'far call' op code
PatchRealIntOrCall:
    DB   00             ; INT # or OFFSET
    DB   NOP_OPCODE     ; NOP or OFFSET
    DB   NOP_OPCODE     ; NOP or SEGMENT
    DB   NOP_OPCODE     ; NOP or SEGMENT

;
; We have executed the INT or far call and returned here.
;

; 
; Save the result registers.
;   See the above NOTEs.
; 

    push    ebp
    mov     ebp, eax    ; save eax

    push    ds
    push    eax         ; throw away ss
    push    eax         ; throw away cs
    push    es
    pushfd
    push    edi
    push    esi
    push    edx
    push    ecx
    push    ebx
    push    ebp         ; ax results are in bp


    xor     edi, edi
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

                                        ; .. (is really 'mov BX, SP' in 16-bit real mode)
;
; Rebalance the stack
;   Note: Although the values are popped here, they are still used by the LGDT
;   instruction below.
;
    pop     eax                         ; (is really 'pop AX' in 16-bit real mode)
    pop     eax                         ; (is really 'pop AX' in 16-bit real mode)
    pop     eax                         ; (is really 'pop AX' in 16-bit real mode)

;
; esi = IntThunk
;   This is used to restore several values below.
;   The address of IntThunk was patched by the one-time code.
;
;   mov si, imm16
    DB      0BEh                        ; 'mov si,imm16' opcode (0xB8 + 6)
IntThunkAddr3:
    DW      0000h                       ; address of IntThunk (imm16)

    lea     ebx, (LOW_MEMORY_THUNK PTR [esi]).RealModeGdtDesc

;
; Do the actual LGDT.
;   The GDT limit and base are on the stack, pointed to by BX.
;
; lgdt    fword ptr [bx]
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
    mov     eax, cr0
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    or      eax, CR0_PE
    DB      OPERAND_SIZE_OVERRIDE_PREFIX
    mov     cr0, eax                    ; Set PE in CR0.

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
    DD      $+8                         ; offset    (Backto32BitProtectedMode)
    DW      PROTECTED_CODE_SELECTOR     ; selector  (flat CS)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;         32-bit protected mode         ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Backto32BitProtectedMode:
;
; Restore data selector
;   AX was set to the value of the flat DS before the jump
;   DS, ES, FS, and GS are the same because we are using the flat memory model again.
;
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

;
; esi = IntThunk
;   This is used to restore several values below.
;   The address of IntThunk was patched by the one-time code.
;
    DB      0BEh                        ; 'mov esi,imm32' opcode (0xB8 + 6)
IntThunkAddr2:
    DD      00000000h                   ; address of IntThunk (imm32)

;
; Restore 32-bit IDT
;
    lidt    (LOW_MEMORY_THUNK PTR [esi]).IdtDesc

;
; Restore 32-bit stack
;
    mov     ecx, (LOW_MEMORY_THUNK PTR [esi]).FlatSs
    mov     ss, cx
    mov     esp, (LOW_MEMORY_THUNK PTR [esi]).FlatEsp

    
;
; Restore the CR0, CR3, and CR4 register by poping it from stack
; This also restores the PG bit in CR0 if it was set coming into real mode
;
    pop   eax
    mov   cr3, eax
    pop   eax
    mov   cr4, eax
    pop   eax
    mov   cr0, eax
 

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
    mov eax, edi

;
; Restore C regs
;
    pop     ebx
    pop     edx
    pop     ebp
    pop     esi
    pop     edi
    popfd

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


ThunkEnd:
    nop                                 ; Used to mark the end of the "prepared" code

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;     End of "Prepared" Thunk Code      ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


RealModeTemplate ENDP



GetRegisters PROC C  IntThunk: DWORD

;----------------------------------------------------------------------
;VOID
;GetRegisters(
;  LOW_MEMORY_THUNK          *IntThunk
;  )
;
; Routine Description:
; 
;   Worker function for LegacyBiosGetFlatDescs, retrieving content of
;   specific registers.
;
; Arguments:
;
;   IntThunk - Pointer to IntThunk of Legacy BIOS context.
;  
; Returns:
;
;   None
;
;----------------------------------------------------------------------
  
  push    eax
  push    ecx  

  mov     ecx,  IntThunk
  sgdt    (LOW_MEMORY_THUNK PTR [ecx]).GdtDesc
  sidt    (LOW_MEMORY_THUNK PTR [ecx]).IdtDesc
  mov     ax, ss
  movzx   eax, ax
  mov     (LOW_MEMORY_THUNK PTR [ecx]).FlatSs, eax

  pop     ecx
  pop     eax
  
  ret

GetRegisters ENDP


CpuSetPower2  PROC C Input: DWORD
;------------------------------------------------------------------------------
; UINTN
; CpuSetPower2 (
;   IN  UINTN   Input
;   );
;------------------------------------------------------------------------------
    bsr   edx, Input
    bts   eax, edx
    ret
CpuSetPower2  ENDP


END
