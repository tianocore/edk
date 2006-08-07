/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  NetDebug.h

Abstract:

  Definition for network debug facility

--*/

#ifndef _NET_DEBUG_H_
#define _NET_DEBUG_H_

#include "Tiano.h"

//
// The debug level definition. This value is also used as the 
// syslog's servity level. Don't change it. 
//
enum {
  NETDEBUG_LEVEL_TRACE   = 5,
  NETDEBUG_LEVEL_WARNING = 4,
  NETDEBUG_LEVEL_ERROR   = 3,
};

#ifdef EFI_NETWORK_STACK_DEBUG

//
// The debug output expects the ASCII format string, Use %a to print ASCII 
// string, and %s to print UNICODE string. PrintArg must be enclosed in (). 
// For example: NET_DEBUG_TRACE ("Tcp", ("State transit to %a\n", Name));
//
#define NET_DEBUG_TRACE(Module, PrintArg) \
  NetDebugOutput ( \
    NETDEBUG_LEVEL_TRACE, \
    Module, \
    __FILE__, \
    __LINE__, \
    NetDebugASPrint PrintArg \
    )

#define NET_DEBUG_WARNING(Module, PrintArg) \
  NetDebugOutput ( \
    NETDEBUG_LEVEL_WARNING, \
    Module, \
    __FILE__, \
    __LINE__, \
    NetDebugASPrint PrintArg \
    )

#define NET_DEBUG_ERROR(Module, PrintArg) \
  NetDebugOutput ( \
    NETDEBUG_LEVEL_ERROR, \
    Module, \
    __FILE__, \
    __LINE__, \
    NetDebugASPrint PrintArg \
    )

#else
#define NET_DEBUG_TRACE(Module, PrintString)
#define NET_DEBUG_WARNING(Module, PrintString)
#define NET_DEBUG_ERROR(Module, PrintString)
#endif

UINT8 *
NetDebugASPrint (
  UINT8                     *Format,
  ...
  );

EFI_STATUS
NetDebugOutput (
  UINT32                    Level, 
  UINT8                     *Module,
  UINT8                     *File,
  UINT32                    Line,
  UINT8                     *Message
  );

//
// Network debug message is sent out as syslog. 
//
enum {
  NET_SYSLOG_FACILITY       = 16,             // Syslog local facility local use
  NET_SYSLOG_PACKET_LEN     = 512,
  NET_DEBUG_MSG_LEN         = 470,            // 512 - (ether+ip+udp head length)
  NET_SYSLOG_TX_TIMEOUT     = 500 *1000 *10,  // 500ms
};
#endif
