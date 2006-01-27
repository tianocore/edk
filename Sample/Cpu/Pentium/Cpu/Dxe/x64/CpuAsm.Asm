  page    ,132
  title   CPU ARCHITECTURAL DXE PROTOCOL ASSEMBLY HOOKS
;------------------------------------------------------------------------------
;*
;*   Copyright (c) 2005, Intel Corporation                                                         
;*   All rights reserved. This program and the accompanying materials                          
;*   are licensed and made available under the terms and conditions of the BSD License         
;*   which accompanies this distribution.  The full text of the license may be found at        
;*   http://opensource.org/licenses/bsd-license.php                                            
;*                                                                                             
;*   THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
;*   WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
;*   
;*   Module Name:
;*     CpuAsm.asm
;* 
;* Abstract:
;* 
;*   This is the code that supports IA32 CPU architectural protocol
;*  
;------------------------------------------------------------------------------
;
;
;------------------------------------------------------------------------------


text  SEGMENT

ExternalVectorTablePtr QWORD 0 ; point to the external interrupt vector table

InitializeExternalVectorTablePtr PROC NEAR PUBLIC
	mov	ExternalVectorTablePtr, rcx
	ret
InitializeExternalVectorTablePtr ENDP	
;
;
;
;------------------------------------------------------------------------------
;  Generic IDT Vector Handlers for the Host. They are all the same so they
;  will compress really well.
;
;  By knowing the return address for Vector 00 you can can calculate the
;  vector number by looking at the call CommonInterruptEntry return address.
;  (return address - AsmIdtVector00Base)/8 == IDT index
;
;------------------------------------------------------------------------------

AsmIdtVector00 PROC NEAR PUBLIC
    call  CommonInterruptEntry
AsmIdtVector00 ENDP
AsmIdtVector00Base PROC NEAR PUBLIC
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop	
    call  CommonInterruptEntry
	nop
	nop
	nop	
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop	
    call  CommonInterruptEntry
	nop
	nop
	nop   
    call  CommonInterruptEntry
	nop
	nop
	nop	
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
    call  CommonInterruptEntry
	nop
	nop
	nop
AsmIdtVector00Base ENDP


;---------------------------------------;
; CommonInterruptEntry                  ;
;---------------------------------------;
; The follow algorithm is used for the common interrupt routine.
; Save EFI_SYSTEM_CONTEXT_x64 on the stack per AP definition
;
;
CommonInterruptEntry PROC NEAR PUBLIC  
  cli
  push rax
  push rcx
  push rdx
  push rbx
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
  push rsp
  push rbp
  push rsi
  push rdi
  xor  rax, rax   ; Not the correct rip
  push rax        ;

  xor  eax, eax
  
  push rax    ;Idtr[1]
  push rax    ;Idtr[0]
  push rax    ;Gdtr[1]
  push rax    ;Gdtr[0]
  push rax    ;Tr
  push rax    ;Ldtr
  push rax    ;rflags place holder
  mov  rax, cr4
  push rax
  mov  rax, cr3
  push rax
  mov  rax, cr2
  push rax
  mov  rax, cr2 ; place holder for cr1
  push rax
  mov  rax, cr0
  push rax
  mov  rax, dr7
  push rax
  mov  rax, dr6
  push rax
  mov  rax, dr3
  push rax
  mov  rax, dr2
  push rax
  mov  rax, dr1
  push rax
  mov  rax, dr0
  push rax

  ;
  ; rcx == arg1 == vector number. Rax is offset in IDT
  ;
	mov   rax,  [rsp+280]					            ; Get the return address for the vector
	                                          ; It was pushed on by the common code in the IDT
	mov   rcx,  AsmIdtVector00Base                                          
	sub   rax,  rcx  ; Compute the vector # by subtracking the return address for the 1st 
	mov   rcx,  rax                           ; rax == Byte Index into IDT
	shr   rcx,  3                             ; Assume each vector handler is 8-bytes
	
	;
	; rdx == ar2 == Pointer to register state save context. Not implemented correctly yet!!!
	;
	mov   rdx, rsp
	; add   rdx, 300
  add   rax, QWORD PTR ExternalVectorTablePtr   ; add in base
  cmp   QWORD PTR [rax], 00h                  ; Check for NULL

  je    nonNullValue;
  call  QWORD PTR [rax]                        ;

nonNullValue:
  ;cli
  pop rax
  mov dr0, rax
  pop rax
  mov dr1, rax
  pop rax
  mov dr2, rax
  pop rax
  mov dr3, rax
  pop rax
  mov dr6, rax
  pop rax
  mov dr7, rax
  pop rax
  mov cr0, rax
  pop rax
  mov cr2, rax  ;; place holder for cr1
  pop rax
  mov cr2, rax
  pop rax
  mov cr3, rax
  pop rax
  mov cr4, rax
  pop rax       ;rflags - get rid of the flags
  pop rax       ;Ldtr - Pop of non implemented state
  pop rax       ;TR
  pop rax       ;Gdtr[0]
  pop rax       ;Tdtr[1]
  pop rax       ;Idtr[0]
  pop rax       ;Idtr[1]
  pop rax       ; We don't need ip so throw it away
  pop rdi
  pop rsi
  pop rbp
  pop rax       ; Don't need SP
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rbx
  pop rdx
  pop rcx
  pop rax

  add rsp, 8    ; cunk out stack frame from CommonHandler
  iretq

CommonInterruptEntry ENDP


LongMode PROC NEAR PUBLIC

in_long_mode::
    ;
    ; Debug Stop
    ;
    jmp   in_long_mode

    ;
    ; We're in long mode, so marshall the arguments to call the
    ; passed in function pointers
    ; Recall
    ;         [ebp][10h] = HobStart
    ;         [ebp][18h] = Stack
    ;         [ebp][20h] = PpisNeededByDxeIplEntryPoint <--- Call this first (for each call, pass HOB pointer)
    ;         [ebp][28h] = DxeCoreEntryPoint            <--- Call this second
    ;
    mov rbx, [rbp+18h]        ; Setup the stack
    mov rsp, rbx              ; On a new stack now

    mov rcx, [rbp+10h]        ; Pass Hob Start in RCX
    mov rax, [rbp+20h]        ; Get the function pointer for 
                              ; PpisNeededByDxeIplEntryPoint into EAX
    call fword ptr [rax]      ; Make the call into PpisNeededByDxeIplEntryPoint

    mov ecx, [rbp+10h]        ; Pass Hob Start in RCX
    mov eax, [rbp+28h]        ; Get the function pointer for 
                              ; DxeCoreEntryPoint into EAX
    call fword ptr [rax]      ; Make the call into Dxe Core

    call CommonInterruptEntry

    mov rdi, CommonInterruptEntry 

    lidt  fword ptr [rdi]

    lgdt  fword ptr [rdi]

    call near ptr [rax]      ; Make the call into PpisNeededByDxeIplEntryPoint

    call rax

    ;zimmer

    ;
    ; Should never get here.
    ;
no_long_mode:
    jmp   no_long_mode
    ;
    ; WE SHOULD NEVER GET HERE!!!!!!!!!!!!!
    ;
LongMode endp


text  ENDS
END

