/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiPrintLib.h

Abstract:

  Light weight lib to support EFI drivers.

--*/

#ifndef _EFI_PRINT_LIB_H_
#define _EFI_PRINT_LIB_H_

#include EFI_PROTOCOL_DEFINITION(UgaDraw)
#include EFI_PROTOCOL_DEFINITION(Print)

UINTN
ErrorPrint (
  IN CONST CHAR16 *ErrorString,
  IN CONST CHAR8  *Format,
  ...
  );

VOID
ErrorDumpHex (
  IN UINTN        Indent,
  IN UINTN        Offset,
  IN UINTN        DataSize,
  IN VOID         *UserData
  );

UINTN
Print (
  IN CONST CHAR16  *Format,
  ...
  );

UINTN
PrintXY (
  IN UINTN                            X,
  IN UINTN                            Y,
  IN EFI_UGA_PIXEL                    *Foreground, OPTIONAL
  IN EFI_UGA_PIXEL                    *Background, OPTIONAL
  IN CHAR16                           *Fmt,
  ...
  );

UINTN
Aprint (
  IN CONST CHAR8  *Format,
  ...
  );

UINTN
UPrint (
  IN CONST CHAR16  *Format,
  ...
  );

UINTN
VSPrint (
  OUT CHAR16        *StartOfBuffer,
  IN  UINTN         StrLen,
  IN  CONST CHAR16  *Format,
  IN  VA_LIST       Marker
  );

UINTN
SPrint (
  OUT CHAR16      *Buffer,
  IN UINTN        BufferSize,
  IN CONST CHAR16 *Format,
  ...
  );


//
// BoxDraw support
//
BOOLEAN
IsValidEfiCntlChar (
  IN  CHAR16  CharC
  );

BOOLEAN
IsValidAscii (
  IN  CHAR16  Ascii
  );

BOOLEAN
LibIsValidTextGraphics (
  IN  CHAR16  Graphic,
  OUT CHAR8   *PcAnsi,    OPTIONAL
  OUT CHAR8   *Ascii      OPTIONAL
  );


#endif
