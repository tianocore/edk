/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    ConsoleLib.h
    
Abstract: 
    
    Interface definition for ConsoleLib.

Revision History
--*/

#ifndef _EFI_CONSOLE_LIBRARY_H
#define _EFI_CONSOLE_LIBRARY_H

//
// Statements that include other header files
//
#include "Tiano.h"

//
// Console Library API
//
EFI_STATUS
GetGlyphWidth (
  IN  CHAR16                              UnicodeChar,
  OUT UINT32                              *GlyphWidth
  )
;

EFI_STATUS
UnicodeStrDisplayLen (
  IN  CHAR16                              *UnicodeStr,
  OUT UINT32                              *DisplayLength
  )
;

#endif
