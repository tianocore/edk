      TITLE   SecSwitchStacks.asm: Stack switching routine for Winnt emulation.

;------------------------------------------------------------------------------
; Copyright (c) 2007, Intel Corporation                                                         
; All rights reserved. This program and the accompanying materials                          
; are licensed and made available under the terms and conditions of the BSD License         
; which accompanies this distribution.  The full text of the license may be found at        
; http://opensource.org/licenses/bsd-license.php                                            
;                                                                                           
; THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
; WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
;
;
;Module Name:
;
;    SecSwitchStacks.asm
;
;Abstract:
;
;    Stack switching routine for Winnt emulation.
;
;------------------------------------------------------------------------------

  .686P
  .XMM
  .MODEL SMALL
  .CODE
  
SecSwitchStacks PROTO C  EntryPoint: DWORD, SecCoreData: DWORD, PpList: DWORD, NewStack: DWORD, NewBsp: DWORD

SecSwitchStacks PROC C   EntryPoint: DWORD, SecCoreData: DWORD, PpList: DWORD, NewStack: DWORD, NewBsp: DWORD

;------------------------------------------------------------------------------
;VOID
;SecSwitchStacks (
;  IN VOID                       *EntryPoint,
;  IN EFI_SEC_PEI_HAND_OFF       *SecCoreData,
;  IN EFI_PEI_PPI_DESCRIPTOR     *PpList,
;  IN VOID                       *NewStack,
;  IN VOID                       *NewBsp
;  )
;
; Routine Description:
; 
;   Stack switching routine for Winnt emulation.
;
; Arguments:
;
;   EntryPoint - Entry point with new stack.
;   PeiStartup - PEI startup descriptor for new entry point.
;   NewStack   - Pointer to new stack.
;   NewBsp     - Pointer to new BSP.
;  
; Returns:
;
;   None
;
;----------------------------------------------------

  mov    ecx, EntryPoint

  mov    eax, NewStack
  mov    esp, eax

  push   PpList
  push   SecCoreData
  call   ecx
  
  ret
  
SecSwitchStacks ENDP

END
