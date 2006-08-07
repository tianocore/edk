/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  IcmpIo.h

Abstract:

--*/

#ifndef _ICMP_IO_H_
#define _ICMP_IO_H_

#include "Tiano.h"

//
// type and code define for ICMP protocol error got
// from IP
//
#define ICMP_TYPE_UNREACH              3
#define ICMP_TYPE_TIMXCEED             11
#define ICMP_TYPE_PARAMPROB            12
#define ICMP_TYPE_SOURCEQUENCH         4

#define ICMP_CODE_UNREACH_NET          0
#define ICMP_CODE_UNREACH_HOST         1
#define ICMP_CODE_UNREACH_PROTOCOL     2
#define ICMP_CODE_UNREACH_PORT         3
#define ICMP_CODE_UNREACH_NEEDFRAG     4
#define ICMP_CODE_UNREACH_SRCFAIL      5
#define ICMP_CODE_UNREACH_NET_UNKNOWN  6
#define ICMP_CODE_UNREACH_HOST_UNKNOWN 7
#define ICMP_CODE_UNREACH_ISOLATED     8
#define ICMP_CODE_UNREACH_NET_PROHIB   9
#define ICMP_CODE_UNREACH_HOST_PROHIB  10
#define ICMP_CODE_UNREACH_TOSNET       11
#define ICMP_CODE_UNREACH_TOSHOST      12

//
// this error will be delivered to the
// listening transportation layer protocol
// consuming IpIO
//
typedef enum {
  ICMP_ERR_UNREACH_NET      = 0,
  ICMP_ERR_UNREACH_HOST,
  ICMP_ERR_UNREACH_PROTOCOL,
  ICMP_ERR_UNREACH_PORT,
  ICMP_ERR_MSGSIZE,
  ICMP_ERR_UNREACH_SRCFAIL,
  ICMP_ERR_TIMXCEED_INTRANS,
  ICMP_ERR_TIMXCEED_REASS,
  ICMP_ERR_QUENCH,
  ICMP_ERR_PARAMPROB
} ICMP_ERROR;

typedef struct _ICMP_ERROR_INFO {
  EFI_STATUS  Error;
  BOOLEAN     IsHard;
  BOOLEAN     Notify;
} ICMP_ERROR_INFO;

#endif
