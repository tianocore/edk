/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Crc32SectionExtract.h
  
Abstract:

  Header file for Crc32SectionExtract.c
  Please refer to EFI 2.0 File Image Format specification 
  FV spec 0.3.6

--*/

#ifndef _CRC32_GUIDED_SECTION_EXTRACTION_H
#define _CRC32_GUIDED_SECTION_EXTRACTION_H

//
// Statements that include other header files
//
#include EFI_PROTOCOL_DEFINITION(GuidedSectionExtraction)

typedef struct {
  EFI_GUID_DEFINED_SECTION       GuidedSectionHeader;
  UINT32                         CRC32Checksum;
} CRC32_SECTION_HEADER;

//
// Function prototype declarations
//
STATIC
EFI_STATUS
Crc32ExtractSection (
  IN  EFI_GUIDED_SECTION_EXTRACTION_PROTOCOL  *This,
  IN  VOID                                    *InputSection,
  OUT VOID                                    **OutputBuffer,
  OUT UINTN                                   *OutputSize,
  OUT UINT32                                  *AuthenticationStatus
  );

#endif
