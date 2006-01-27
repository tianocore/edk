      TITLE   Thunk.inc: Assembly code for the thunk protocol

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
;   Thunk.inc
; 
; Abstract:
; 
;   This is the code that supports IA32-optimized CopyMem service
;
;------------------------------------------------------------------------------


;
; Thunk Status Codes
;   (These apply only to errors with the thunk and not to the code that was
;   thunked to.)
;
THUNK_OK                        EQU 00h
THUNK_ERR_A20_UNSUP             EQU 01h
THUNK_ERR_A20_FAILED            EQU 02h

;INCLUDE masm_legacybios.inc

; 
; Interesting CR0 flags
; 
CR0_PE      EQU     00000001h
CR0_PG      EQU     80000000h

OPERAND_SIZE_OVERRIDE_PREFIX    EQU 066h
NOP_OPCODE                      EQU 090h

NUM_REAL_GDT_ENTRIES            EQU 5

;
; This data must match gTrapoleanGdt
;
REAL_CODE_SELECTOR        EQU   08h
REAL_DATA_SELECTOR        EQU   10h
PROTECTED_CODE_SELECTOR   EQU   18h
PROTECTED_DATA_SELECTOR   EQU   20h

;
; Data Types
;
UINT64      TYPEDEF     QWORD
UINT32      TYPEDEF     DWORD
UINT16      TYPEDEF     WORD
UINT8       TYPEDEF     BYTE
CHAR8       TYPEDEF     SBYTE

; 
;  Define what a processor GDT looks like
; 
GDT_ENTRY   STRUCT 4
LimitLow    UINT16    ?
BaseLow     UINT16    ?
BaseMid     UINT8     ?
Attribute   UINT8     ?
BaseHi      UINT8     ?
GDT_ENTRY   ENDS

; 
; Define what a processor descriptor looks like
; 
DESCRIPTOR      STRUCT 1
Limit       UINT16      ?
Base        UINT32      ?
DESCRIPTOR      ENDS

;
; LOW_MEMORY_THUNK
;
;   BugBug: The last 2 elements in this structure in the .h file are not
;   accessed here and could not easily be converted using h2inc.  Therfore,
;   they have been removed and replaced with an approximate placeholder
;   named "Reserved".  The size of reserved is not important.
;
LOW_MEMORY_THUNK                STRUCT 2
CodeBuffer            CHAR8       4096 DUP (?)
GdtDesc               DESCRIPTOR  <>
IdtDesc               DESCRIPTOR  <>
FlatSs                UINT32      ?
FlatEsp               UINT32      ?
RealStack             UINT32      ?
RealModeIdtDesc       DESCRIPTOR  <>
RealModeGdtDesc       DESCRIPTOR  <>
RealModeGdt           GDT_ENTRY   NUM_REAL_GDT_ENTRIES DUP (<>)
Stack                 CHAR8       8192 DUP (?)
LOW_MEMORY_THUNK          ENDS

