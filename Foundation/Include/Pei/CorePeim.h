/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CorePeim.h
    
Abstract:

  This includes the architecture-independent PEI information.
  The header and Import/Export definitions are variable size
  across IA32 and IPF ISA's so they will be found in the 
  respective architecture subdirectory in PeiBind.h, respectively.

--*/

#ifndef _COREPEIM_H
#define _COREPEIM_H

#include "PeiHob.h"

//
// Make the PHIT base 0 for this implementation
//
#define PHIT_PHYSICAL_ADDRESS_BASE  0x00000000

//
// Peim Flags
//
#define EFI_NOT_RECOVERY            0x0000000000000000
#define EFI_RECOVERY                0x0000000000000001

#define EXPORT_FLAG_DAISY_CHAIN     0x0001
#define EXPORT_FLAG_DC_FIRST        0x0002
#define EXPORT_FLAG_DC_LAST         0x0004
#define EXPORT_FLAG_DC_IGNORE       0x0008
#define EXPORT_FLAG_DC_HEAD         0x0010
#define EXPORT_FLAG_LOOPBACK        0x0020
#define EXPORT_FLAG_LAST_ENTRY      0x8000
#define EXPORT_FLAG_LAST_ENTRY_POS  0xf

#define IMPORT_FLAG_DYNAMIC         0x0001
#define IMPORT_FLAG_DAISY_CHAIN     0x0002
#define IMPORT_FLAG_STUB            0x0004
#define IMPORT_FLAG_INTERNAL        0x0008
#define IMPORT_FLAG_LAST_ENTRY      0x8000
#define IMPORT_FLAG_LAST_ENTRY_POS  0xf

#define BIT0                        0x0000000000000001
#define BIT1                        0x0000000000000002

typedef UINT16  PPI_FLAGS;

typedef struct {
  UINT16  SpecMinor;
  UINT16  SpecMajor;
  UINT16  PeiCisMinorVer;
  UINT16  PeiCisMajorVer;
} EFI_PEIM_VERSION;

#endif
