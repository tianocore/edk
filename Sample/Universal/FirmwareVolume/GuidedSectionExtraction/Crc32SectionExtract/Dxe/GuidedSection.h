/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  GuidedSection.h
  
Abstract:

  Header file for GuidedSection.c
  Please refer to EFI 2.0 File Image Format specification 
  FV spec 0.3.6
  
--*/

#ifndef _GUIDED_SECTION_EXTRACTION_H
#define _GUIDED_SECTION_EXTRACTION_H

//
// Statements that include other header files
//
#include EFI_PROTOCOL_DEFINITION(GuidedSectionExtraction)

//
// Function prototype declarations
//
EFI_STATUS
GuidedSectionExtractionProtocolConstructor (
  OUT EFI_GUIDED_SECTION_EXTRACTION_PROTOCOL      **GuidedSep,
  IN  EFI_EXTRACT_GUIDED_SECTION                  ExtractSection
  );

#endif
