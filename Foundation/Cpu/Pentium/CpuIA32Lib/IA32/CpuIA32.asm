TITLE   CpuIA32.asm: Assembly code for the IA-32 resources

;*****************************************************************************
;*
;*   Copyright (c) 2004, Intel Corporation                                                         
;*   All rights reserved. This program and the accompanying materials                          
;*   are licensed and made available under the terms and conditions of the BSD License         
;*   which accompanies this distribution.  The full text of the license may be found at        
;*   http://opensource.org/licenses/bsd-license.php                                            
;*                                                                                             
;*   THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
;*   WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
;*   
;*   Module Name:
;*
;*    CpuIA32.asm
;*  
;*   Abstract:
;*  
;*****************************************************************************

  
  .686
  .MODEL FLAT,C
  .CODE
  
  INCLUDE IA32Type.inc

  

;------------------------------------------------------------------------------
;  VOID
;  EfiHalt (
;    VOID
;    )
;------------------------------------------------------------------------------
EfiHalt PROC    PUBLIC
    hlt
    ret
EfiHalt ENDP


;------------------------------------------------------------------------------
;  VOID
;  EfiWbinvd (
;    VOID
;    )
;------------------------------------------------------------------------------
EfiWbinvd PROC    PUBLIC
    wbinvd
    ret
EfiWbinvd ENDP


;------------------------------------------------------------------------------
;  VOID
;  EfiInvd (
;    VOID
;    )
;------------------------------------------------------------------------------
EfiInvd PROC    PUBLIC
    invd
    ret
EfiInvd ENDP

;------------------------------------------------------------------------------
;  VOID
;  EfiCpuid (
;    IN   UINT32              RegisterInEax,    
;    OUT  EFI_CPUID_REGISTER  *Reg              
;    )
;------------------------------------------------------------------------------
EfiCpuid PROC    PUBLIC RegisterInEax:UINT32, Reg:PTR EFI_CPUID_REGISTER
     pushad
     
     mov    eax, RegisterInEax
     cpuid
     mov    edi, DWORD PTR Reg 
     ASSUME  edi: PTR EFI_CPUID_REGISTER
     mov    DWORD PTR [edi].RegEax, eax   ; Reg->RegEax
     mov    DWORD PTR [edi].RegEbx, ebx   ; Reg->RegEbx
     mov    DWORD PTR [edi].RegEcx, ecx   ; Reg->RegEcx
     mov    DWORD PTR [edi].RegEdx, edx   ; Reg->RegEdx

     popad
     ret
EfiCpuid  ENDP

;------------------------------------------------------------------------------
;  UINT64
;  EfiReadMsr (
;    IN   UINT32  Index,
;    )
;------------------------------------------------------------------------------
EfiReadMsr PROC    PUBLIC Index:UINT32
    push   ecx
    mov    ecx, Index
    rdmsr
    pop    ecx
    ret
EfiReadMsr  ENDP

;------------------------------------------------------------------------------
;  VOID
;  EfiWriteMsr (
;    IN   UINT32  Index,
;    IN   UINT64  Value
;    )
;------------------------------------------------------------------------------
EfiWriteMsr PROC    PUBLIC Index:UINT32, Value:UINT64
    pushad
    mov    ecx, Index
    mov    eax, DWORD PTR Value[0]
    mov    edx, DWORD PTR Value[4]
    wrmsr            
    popad  
    ret
EfiWriteMsr  ENDP


;------------------------------------------------------------------------------
; UINT64
; EfiReadTsc (
;   VOID
;   );
;------------------------------------------------------------------------------
EfiReadTsc PROC    PUBLIC
    rdtsc
    ret
EfiReadTsc  ENDP

;------------------------------------------------------------------------------
; VOID
; EfiDisableCache (
;   VOID
;   );
;------------------------------------------------------------------------------
EfiDisableCache PROC    PUBLIC
    push  eax
    wbinvd
    mov   eax, cr0
    or    eax, 060000000h     
    mov   cr0, eax
    pop   eax
    ret
EfiDisableCache ENDP

;------------------------------------------------------------------------------
; VOID
; EfiEnableCache (
;   VOID
;   );
;------------------------------------------------------------------------------
EfiEnableCache PROC    PUBLIC
    push  eax
    invd
    mov   eax, cr0
    and   eax, 09fffffffh         
    mov   cr0, eax
    pop   eax
    ret
EfiEnableCache ENDP

;------------------------------------------------------------------------------
; UINT32
; EfiGetEflags (
;   VOID
;   );
;------------------------------------------------------------------------------
EfiGetEflags PROC    PUBLIC
    pushfd
    pop  eax
    ret
EfiGetEflags  ENDP


END
