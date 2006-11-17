/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  LightIsaIo.h
  
Abstract:
  
  The specific part of header file for EFI_LIGHT_ISA_IO_PROTOCOL implementation;
  Refer the common part of header file for both EFI_ISA_IO_PROTCOL and 
  EFI_LIGHT_ISA_IO_PROTOCOL in CommonIsaIo.h.
  
Revision History:

--*/

#ifndef _EFI_LIGHT_ISA_IO_LOCAL_H
#define _EFI_LIGHT_ISA_IO_LOCAL_H

#include "CommonIsaIo.h"

//
// Driver Support Global Variables
//
EFI_LIGHT_ISA_IO_PROTOCOL IsaIoInterface = {
  {
    IsaIoIoRead,
    IsaIoIoWrite
  },
  IsaIoMap,
  IsaIoUnmap,
  IsaIoFlush,
  NULL,
  0,
  NULL
};

#endif
