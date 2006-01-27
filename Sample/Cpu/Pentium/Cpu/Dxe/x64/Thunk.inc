      TITLE   Thunk.inc: Assembly code for the thunk protocol

;****************************************************************************;*
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
;*   Module Name:
;*     Thunk.inc
;*  
;*   Abstract:
;*      Types, data structures, and constant values for the thunk and reverse thunk.
;*
;*      GDT_ENTRY, DESCRIPTOR64, DESCRIPTOR32, LOW_MEMORY_THUNK are structures
;*       that are also declared in Thunk.c
;*
;****************************************************************************;*


;
; Thunk Status Codes
;   (These apply only to errors with the thunk and not to the code that was
;   thunked to.)
;
THUNK_OK                        EQU 00h
THUNK_ERR_A20_UNSUP             EQU 01h
THUNK_ERR_A20_FAILED            EQU 02h


; 
; Interesting CR0 flags
; 
CR0_PE      EQU     00000001h
CR0_PG      EQU     80000000h

OPERAND_SIZE_OVERRIDE_PREFIX    EQU 066h
NOP_OPCODE                      EQU 090h

NUM_REAL_GDT_ENTRIES            EQU 8

;
; This data must match gTrapoleanGdt
;
REAL_CODE_SELECTOR        EQU   08h
REAL_DATA_SELECTOR        EQU   10h
PROTECTED_CODE_SELECTOR   EQU   18h
PROTECTED_DATA_SELECTOR   EQU   20h
SPARE_DATA_SELECTOR       EQU   28h
X64_DATA_SELECTOR         EQU   30h
X64_CODE_SELECTOR         EQU   38h

;
; Data Types
;
UINT64      TYPEDEF     QWORD
UINT32      TYPEDEF     DWORD
UINT16      TYPEDEF     WORD
UINT8       TYPEDEF     BYTE
CHAR8       TYPEDEF     BYTE

; 
;  Define what a processor GDT looks like
;   Keep in sync with Thunk.c
; 
GDT_ENTRY   STRUCT 4
LimitLow    UINT16    ?
BaseLow     UINT16    ?
BaseMid     UINT8     ?
Attribute   UINT8     ?
LimitHi     UINT8     ?
BaseHi      UINT8     ?
GDT_ENTRY   ENDS

; 
; Define what a processor descriptor looks like
; Keep in sync with Thunk.c definition
; 
DESCRIPTOR64  STRUCT 1
Limit       UINT16      ?
Base        UINT64      ?
DESCRIPTOR64  ENDS

DESCRIPTOR32  STRUCT 1
Limit       UINT16      ?
Base        UINT32      ?
DESCRIPTOR32  ENDS

;
; LOW_MEMORY_THUNK - Keep in sync with Thunc.c definition
;
LOW_MEMORY_THUNK      STRUCT 2
CodeBuffer            BYTE              4096 DUP (?)
x64GdtDesc            DESCRIPTOR64      <>
x64IdtDesc            DESCRIPTOR64      <>
x64Ss                 UINT64            ?
x64Esp                UINT64            ?
RealStack             UINT64            ?
RealModeIdtDesc       DESCRIPTOR32      <>
RealModeGdtDesc       DESCRIPTOR32      <>
RealModeGdt           GDT_ENTRY         NUM_REAL_GDT_ENTRIES DUP (<>)
PageMapLevel4         UINT64            <>
Stack                 CHAR8             8192 DUP (?)
LOW_MEMORY_THUNK          ENDS

