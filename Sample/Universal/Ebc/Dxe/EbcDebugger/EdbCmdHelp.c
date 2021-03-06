/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EdbCmdHelp.c
  
Abstract:


--*/

#include "Edb.h"

EFI_DEBUG_STATUS
DebuggerHelp (
  IN     CHAR16                    *CommandArg,
  IN     EFI_DEBUGGER_PRIVATE_DATA *DebuggerPrivate,
  IN     EFI_EXCEPTION_TYPE        ExceptionType,
  IN OUT EFI_SYSTEM_CONTEXT        SystemContext
  )
/*++

Routine Description:

  DebuggerCommand - Help
  
Arguments:

  CommandArg      - The argument for this command
  DebuggerPrivate - EBC Debugger private data structure
  InterruptType   - Interrupt type.
  SystemContext   - EBC system context.

Returns:

  EFI_DEBUG_CONTINUE - formal return value
  
--*/
{
  UINTN Index;

  //
  // if no argument, print all the command title
  //
  if (CommandArg == NULL) {
    for (Index = 0; DebuggerPrivate->DebuggerCommandSet[Index].CommandName != NULL; Index++) {
      EDBPrint (DebuggerPrivate->DebuggerCommandSet[Index].ClassName);
      if (EfiStrCmp (DebuggerPrivate->DebuggerCommandSet[Index].CommandTitle, L"") != 0) {
        EDBPrint (L"  ");
        EDBPrint (DebuggerPrivate->DebuggerCommandSet[Index].CommandTitle);
      }
    }
    return EFI_DEBUG_CONTINUE;
  }

  //
  // If there is argument, the argument should be command name.
  // Find the command and print the detail information.
  //
  for (Index = 0; DebuggerPrivate->DebuggerCommandSet[Index].CommandName != NULL; Index++) {
    if (EfiStriCmp (CommandArg, DebuggerPrivate->DebuggerCommandSet[Index].CommandName) == 0) {
      EDBPrint (DebuggerPrivate->DebuggerCommandSet[Index].CommandHelp);
      EDBPrint (DebuggerPrivate->DebuggerCommandSet[Index].CommandSyntax);
      return EFI_DEBUG_CONTINUE;
    }
  }

  //
  // Command not found.
  //
  EDBPrint (L"No help info for this command\n");

  //
  // Done
  //
  return EFI_DEBUG_CONTINUE;
}
