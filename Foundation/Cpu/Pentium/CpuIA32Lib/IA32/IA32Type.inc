;
; Copyright (c) 2004, Intel Corporation                                                         
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
;   IA32Type.inc
;

UINT8    TYPEDEF    BYTE
UINT16   TYPEDEF    WORD
UINT32   TYPEDEF    DWORD
UINT64   TYPEDEF    QWORD
UINTN    TYPEDEF    UINT32
BOOLEAN  TYPEDEF    UINT8
EFI_STATUS TYPEDEF  UINTN

EFI_PHYSICAL_ADDRESS   TYPEDEF    UINT64


@EFI_CPUID_REGISTER  STRUCT 4t
RegEax    DWORD ?
RegEbx    DWORD ?
RegEcx    DWORD ?
RegEdx    DWORD ?
@EFI_CPUID_REGISTER  ENDS

EFI_CPUID_REGISTER    TYPEDEF    @EFI_CPUID_REGISTER
