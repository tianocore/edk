/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.  


Module Name:

  EdkIIGluePcdDebugLib.h
  
Abstract: 

  PCD values for library customization

--*/

#ifndef __EDKII_GLUE_PCD_DEBUG_LIB_H__
#define __EDKII_GLUE_PCD_DEBUG_LIB_H__

//
// Following Pcd values are hard coded at compile time.
// Override these through compiler option "/D" in PlatformTools.env if needed
//


//
// Debug Pcds
//
#ifndef __EDKII_GLUE_PCD_PcdDebugPrintErrorLevel__
#define __EDKII_GLUE_PCD_PcdDebugPrintErrorLevel__             EFI_D_ERROR
#endif


#ifndef __EDKII_GLUE_PCD_PcdDebugPropertyMask__
#define __EDKII_GLUE_PCD_PcdDebugPropertyMask__  (  DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED        \
                                                 | DEBUG_PROPERTY_DEBUG_PRINT_ENABLED       \
                                                 | DEBUG_PROPERTY_DEBUG_CODE_ENABLED        \
                                                 | DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED      \
                                                 | DEBUG_PROPERTY_ASSERT_BREAKPOINT_ENABLED \
                                                 | DEBUG_PROPERTY_ASSERT_DEADLOOP_ENABLED   \
                                               )

#endif


#ifndef __EDKII_GLUE_PCD_PcdDebugClearMemoryValue__
#define __EDKII_GLUE_PCD_PcdDebugClearMemoryValue__            0xAF
#endif

#include "Pcd/EdkIIGluePcd.h"
#endif
