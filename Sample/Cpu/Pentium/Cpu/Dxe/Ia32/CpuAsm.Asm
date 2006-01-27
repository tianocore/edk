  page    ,132
  title   CPU ARCHITECTURAL DXE PROTOCOL ASSEMBLY HOOKS
;------------------------------------------------------------------------------
;
; Copyright (c) 2005, Intel Corporation                                                         
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
;   CpuAsm.asm
; 
; Abstract:
; 
;   This is the code that supports IA32 CPU architectural protocol
;
;------------------------------------------------------------------------------
.686p
.model  flat        
.data

ExternalVectorTablePtr DD ?             ; Table of call backs
CommonInterruptEntry   DD CommonEntry   ; Address of CommonEntry
Idtr                   DW ?             ; FWORD for IDT register
Idtr1                  DD ?             ; MUST BE IMMEDIATELY AFTER Idtr

.stack
.code
.MMX
.XMM
;---------------------------------------;
; _InitializeIdt                ;
;----------------------------------------------------------------------------;
; 
; Protocol prototype
;   InitializeIdt (
;   IN EFI_CPU_INTERRUPT_HANDLER TableStart,
;   IN UINTN                     *IdtTablePtr,
;   IN UINT16                    IdtLimit
;   )
;           
; Routine Description:
; 
;  Creates an IDT table starting at IdtTablPtr. It has IdtLimit/8 entries.
;  Table is initialized to intxx where xx is from 00 to number of entries or
;  100h, whichever is smaller. After table has been initialized the LIDT
;  instruction is invoked.
; 
;  TableStart is the pointer to the callback table and is not used by 
;  InitializedIdt but by commonEntry. CommonEntry handles all interrupts,
;  does the context save and calls the callback entry, if non-NULL.
;  It is the responsibility of the callback routine to do hardware EOIs.
; 
; Arguments:
; 
;   TableStart          - Pointer to interrupt callback table
;
;   IdtTablePtr         - Pointer to IDT table
;
;   IdtLimit            - IDT Table limit = number of interrupt entries * 8
; 
; Returns: 
; 
;   Nothing
;
; 
; Input:  [ebp][0]  = Original ebp
;         [ebp][4]  = Return address
;         [ebp][8]  = TableStart
;         [ebp][0c] = *IdtTablePtr
;         [ebp][10] = IdtLimit
;          
; Output: Nothing
;           
; Destroys: Nothing
;-----------------------------------------------------------------------------;

_InitializeIdt  proc near public
  push    ebp               ; C prolog
  mov     ebp, esp

  mov     eax, [ebp+8]                  ; Get ExternalVectorTable Address
  mov     ExternalVectorTablePtr, eax

  mov     ax, [ebp+10h]                 ; Get IDT Table limit
  dec     ax
  mov     Idtr, ax

  mov     eax, [ebp+0ch]                  ; Get Start of IDT
  mov     Idtr1, eax
  
  mov     edi, OFFSET Idtr              ; Load IDT register
  lidt    FWORD PTR es:[edi]

  pop     ebp
  ret
_InitializeIdt  endp

CommonEntry:

;---------------------------------------;
; _CommonEntry                  ;
;----------------------------------------------------------------------------;
; The follow algorithm is used for the common interrupt routine.
; Entry from each interrupt with a push eax and eax=interrupt number
; 1.  Save flags and disable interrupts.
; 2.  Save processor registers and standard segment registers.
; 3.  Save FP, MMX and XMM context by FXSAVE command
;     CS, SS, ESP, CRx, DRx, GDTR, IDTR, LDTR, TR and processor context
;     are not saved on the assumption that no interrupt routine should
;     ever change these. SMM code is outside the scope of the CPU Architect
;     protocol.
; 4.  Call back routine is ExternalVectorTable[Interrupt number *4]
; 5.  If call back routine is != NULL then pass control to call back
;     routine.
; 6.  Restore FP, MMX and XMM state
; 7.  Restore segment registers
; 8.  Restore processor registers
; 9.  Restore flags
; 10. Restore eax from individual interrupt routine
; 11. Do IRET.
  pushfd
  cli
  pushad
  push ds
  push es
  push fs
  push gs
  push ebp;
  
  ; BugBug: Bad Math Count for real
  mov eax, [esp+20]                         ; Get the reutnr address
  sub eax, DWORD PTR  AsmIdtVector00Base    ; Convert to Vector Table index
  shr eax, 3                                ; Each entry is 8 bytes
  
  mov ebx, eax
  shl eax, 2;
  add eax, ExternalVectorTablePtr;
  cmp DWORD PTR [eax], 00h    ; Check for NULL
  je  nonNullValue;

  push ebp;           ; System Context pointer see debug protocol in the EFI 1.1 specification
  push ebx            ; InterruptType
  call DWORD PTR [eax];
  add esp, 8        ; Clear off parameters

nonNullValue:
  pop ebp;
  pop gs;
  pop fs;
  pop es;
  pop ds;
  popad;
  popfd;
  pop eax;
  iretd;


;------------------------------------------------------------------------------
; UINT16
; CpuCodeSegment (
;   VOID
;   );
;------------------------------------------------------------------------------
_CpuCodeSegment PROC    NEAR    PUBLIC  
    xor   eax, eax
    mov   eax, cs
    ret
_CpuCodeSegment  ENDP

;---------------------------------------;
; _InitializeSelectors                  ;
;----------------------------------------------------------------------------;
; 
; Protocol prototype
;   InitializeSelectors (
;   )
;           
; Routine Description:
; 
;  Creates an new GDT in RAM.  The problem is that our former selectors
;  were ROM based and the EFI OS Loader does not manipulate the machine state 
;  to change them (as it would for a 16-bit PC/AT startup code that had to
;  go from Real Mode to flat mode).
; 
; Arguments:
; 
; 
; Returns: 
; 
;   Nothing
;
; 
; Input:  [ebp][0]  = Original ebp
;         [ebp][4]  = Return address
;          
; Output: Nothing
;           
; Destroys: Nothing
;-----------------------------------------------------------------------------;

CODE_SELECTOR EQU 10h
DATA_SELECTOR EQU 18h

_InitializeSelectors  proc near public
  push    ebp                 ; C prolog
  mov     ebp, esp
  pushad
  mov     edi, OFFSET Gdtr    ; Load GDT register

  mov     ax,cs               ; Get the selector data from our code image          
  mov     es,ax
  lgdt    FWORD PTR es:[edi]  ; and update the GDTR

  db      067h
  db      0eah              ; Far Jump Offset:Selector to reload CS
  dd      OFFSET SelectorRld;   Offset is ensuing instruction boundary
  dw      CODE_SELECTOR     ;   Selector is our code selector, 10h
SelectorRld::
  mov     ax, DATA_SELECTOR ; Update the Base for the new selectors, too
  mov     ds, ax
  mov     es, ax
  mov     fs, ax
  mov     gs, ax
  mov     ss, ax  

  popad
  pop     ebp
  ret
_InitializeSelectors  endp
  

;-----------------------------------------------------------------------------;
; data
;-----------------------------------------------------------------------------;

        align 16

gdtr    dw GDT_END - GDT_BASE - 1   ; GDT limit
        dd OFFSET GDT_BASE          ; (GDT base gets set above)

;-----------------------------------------------------------------------------;
;   global descriptor table (GDT)
;-----------------------------------------------------------------------------;

        align 16

public GDT_BASE
GDT_BASE:
; null descriptor
NULL_SEL            equ $-GDT_BASE    ; Selector [0]
        dw 0            ; limit 15:0
        dw 0            ; base 15:0
        db 0            ; base 23:16
        db 0            ; type
        db 0            ; limit 19:16, flags
        db 0            ; base 31:24

; linear data segment descriptor
LINEAR_SEL      equ $-GDT_BASE        ; Selector [0x8]
        dw 0FFFFh       ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 092h         ; present, ring 0, data, expand-up, writable
        db 0CFh                 ; page-granular, 32-bit
        db 0

; linear code segment descriptor
LINEAR_CODE_SEL equ $-GDT_BASE        ; Selector [0x10]
        dw 0FFFFh       ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 09Ah         ; present, ring 0, data, expand-up, writable
        db 0CFh                 ; page-granular, 32-bit
        db 0

; system data segment descriptor
SYS_DATA_SEL    equ $-GDT_BASE        ; Selector [0x18]
        dw 0FFFFh       ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 092h         ; present, ring 0, data, expand-up, writable
        db 0CFh                 ; page-granular, 32-bit
        db 0

; system code segment descriptor
SYS_CODE_SEL    equ $-GDT_BASE
        dw 0FFFFh       ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 09Ah         ; present, ring 0, data, expand-up, writable
        db 0CFh                 ; page-granular, 32-bit
        db 0

; spare segment descriptor
SPARE3_SEL  equ $-GDT_BASE
        dw 0            ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 0            ; present, ring 0, data, expand-up, writable
        db 0            ; page-granular, 32-bit
        db 0

; spare segment descriptor
SPARE4_SEL  equ $-GDT_BASE
        dw 0            ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 0            ; present, ring 0, data, expand-up, writable
        db 0            ; page-granular, 32-bit
        db 0

; spare segment descriptor
SPARE5_SEL  equ $-GDT_BASE
        dw 0            ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 0            ; present, ring 0, data, expand-up, writable
        db 0            ; page-granular, 32-bit
        db 0

GDT_END:


_AsmIdtVector00 PROC NEAR PUBLIC
    ; CJW add for debug
CJW_DEBUG_LOOP:
    nop
    nop
    nop
    jmp	CJW_DEBUG_LOOP  
    ; CJW
    call  CommonInterruptEntry
_AsmIdtVector00 ENDP
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



  end

