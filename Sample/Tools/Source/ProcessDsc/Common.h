/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Common.h

Abstract:

  Common include file for the ProcessDsc utility.

--*/

#ifndef _COMMON_H_
#define _COMMON_H_

typedef char INT8;
typedef unsigned int UINT32;

#include "EfiUtilityMsgs.h"

#define MAX_LINE_LEN  512

#ifndef MAX_PATH
#define MAX_PATH  512
#endif
//
// Defines for how to expand symbols
//
#define EXPANDMODE_NO_UNDEFS    0x01
#define EXPANDMODE_NO_DESTDIR   0x02
#define EXPANDMODE_NO_SOURCEDIR 0x04
#define EXPANDMODE_RECURSIVE    0x08

//
// Defines for adding symbols
//
#define SYM_OVERWRITE 0x01      // overwrite existing assignments
#define SYM_GLOBAL    0x02      // global symbol (persistent)
#define SYM_LOCAL     0x04      // symbols at component level
#define SYM_FILE      0x08      // symbols at file level
#define SYM_FILEPATH  0x10      // symbol is a file path
#define SYM_FILENAME  0x20      // symbol is a file name
#define FV_DIR        "FV_DIR"  // symbol for base dir where FV files are
#define DSC_FILENAME  "DSC_FILENAME"

INT8  *
GetSymbolValue (
  INT8 *SymbolName
  )
;

int
AddSymbol (
  INT8  *Name,
  INT8  *Value,
  int   Mode
  )
;

int
ExpandMacros (
  INT8  *SourceLine,
  INT8  *DestLine,
  int   LineLen,
  int   ExpandMode
  )
;

void
FatalError (
  INT8 *Fmt,
  ...
  )
;

void
Message (
  UINT32  PrintMask,
  INT8    *Fmt,
  ...
  )
;

int
MakeFilePath (
  INT8 *FileName
  )
;

#endif // ifndef _COMMON_H_
