/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:  

  EbcDebuggerHook.h
  
Abstract:
  
--*/

#ifndef _EFI_EBC_DEBUGGER_HOOK_H_
#define _EFI_EBC_DEBUGGER_HOOK_H_

#ifdef   EFI_EBC_DEBUGGER_ENABLED
#define  EFI_EBC_DEBUGGER_CODE(a)   a
#else
#define  EFI_EBC_DEBUGGER_CODE(a)
#endif

//
// Hook in EbcInit.c
//
VOID
EbcDebuggerHookInit (
  IN EFI_HANDLE                  Handle,
  IN EFI_DEBUG_SUPPORT_PROTOCOL  *EbcDebugProtocol
  );

VOID
EbcDebuggerHookUnload (
  VOID
  );

VOID
EbcDebuggerHookEbcUnloadImage (
  IN EFI_HANDLE                  Handle
  );

//
// Hook in EbcSupport.c
//
VOID
EbcDebuggerHookExecuteEbcImageEntryPoint (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookEbcInterpret (
  IN VM_CONTEXT *VmPtr
  );

//
// Hook in EbcExecute.c
//
VOID
EbcDebuggerHookExecuteStart (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookExecuteEnd (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookCALLStart (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookCALLEnd (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookCALLEXStart (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookCALLEXEnd (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookRETStart (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookRETEnd (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookJMPStart (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookJMPEnd (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookJMP8Start (
  IN VM_CONTEXT *VmPtr
  );

VOID
EbcDebuggerHookJMP8End (
  IN VM_CONTEXT *VmPtr
  );

#endif

