/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ProcDep.h

Abstract:

  IPF specific Runtime Lib code. IPF has a SAL API that does not
  exit on IA-32. Thus

--*/

#ifndef _PROC_DEP_H_
#define _PROC_DEP_H_

#include EFI_PROTOCOL_DEFINITION (ExtendedSalGuid)
#include EFI_PROTOCOL_DEFINITION (ExtendedSalBootService)
#include "SalApi.h"


EFI_STATUS
RegisterEsalFunction (
  IN  UINT64                                    FunctionId,
  IN  EFI_GUID                                  *ClassGuid,
  IN  SAL_INTERNAL_EXTENDED_SAL_PROC            Function,
  IN  VOID                                      *ModuleGlobal
  );

EFI_STATUS
RegisterEsalClass (
  IN  EFI_GUID                                  *ClassGuid,
  IN  VOID                                      *ModuleGlobal,
  ...
  );




SAL_RETURN_REGS
EfiCallEsalService (
  IN  EFI_GUID                                      *ClassGuid,
  IN  UINT64                                        FunctionId,
  IN  UINT64                                        Arg2,
  IN  UINT64                                        Arg3,
  IN  UINT64                                        Arg4,
  IN  UINT64                                        Arg5,
  IN  UINT64                                        Arg6,
  IN  UINT64                                        Arg7,
  IN  UINT64                                        Arg8
  );



#endif
