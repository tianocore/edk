/*++

Copyright (c) 2007 - 2009, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  IScsiCHAP.h

Abstract:

--*/

#ifndef _ISCSI_CHAP_H_
#define _ISCSI_CHAP_H_

#define ISCSI_CHAP_AUTH_INFO_GUID \
  { \
    0x786ec0ac, 0x65ae, 0x4d1b, 0xb1, 0x37, 0xd, 0x11, 0xa, 0x48, 0x37, 0x97 \
  }

extern EFI_GUID mIScsiCHAPAuthInfoGuid;

#define ISCSI_AUTH_METHOD_CHAP    "CHAP"

#define ISCSI_KEY_CHAP_ALGORITHM  "CHAP_A"
#define ISCSI_KEY_CHAP_IDENTIFIER "CHAP_I"
#define ISCSI_KEY_CHAP_CHALLENGE  "CHAP_C"
#define ISCSI_KEY_CHAP_NAME       "CHAP_N"
#define ISCSI_KEY_CHAP_RESPONSE   "CHAP_R"

#define ISCSI_CHAP_ALGORITHM_MD5  5

#define ISCSI_CHAP_AUTH_MAX_LEN   1024
#define ISCSI_CHAP_RSP_LEN        16  // == MD5_HASHSIZE
typedef enum {
  ISCSI_CHAP_INITIAL,
  ISCSI_CHAP_STEP_ONE,
  ISCSI_CHAP_STEP_TWO,
  ISCSI_CHAP_STEP_THREE,
  ISCSI_CHAP_STEP_FOUR
} ISCSI_CHAP_STEP;

#pragma pack(1)

typedef struct _ISCSI_CHAP_AUTH_CONFIG_NVDATA {
  UINT8 CHAPType;
  CHAR8 CHAPName[ISCSI_CHAP_NAME_MAX_LEN];
  CHAR8 CHAPSecret[ISCSI_CHAP_SECRET_STORAGE];
  CHAR8 ReverseCHAPName[ISCSI_CHAP_NAME_MAX_LEN];
  CHAR8 ReverseCHAPSecret[ISCSI_CHAP_SECRET_STORAGE];
} ISCSI_CHAP_AUTH_CONFIG_NVDATA;

#pragma pack()

//
// ISCSI CHAP Authentication Data
//
typedef struct _ISCSI_CHAP_AUTH_DATA {
  ISCSI_CHAP_AUTH_CONFIG_NVDATA AuthConfig;
  UINT32                        InIdentifier;
  UINT8                         InChallenge[ISCSI_CHAP_AUTH_MAX_LEN];
  UINT32                        InChallengeLength;
  //
  // Calculated CHAP Response (CHAP_R) value
  //
  UINT8                         CHAPResponse[ISCSI_CHAP_RSP_LEN];

  //
  // Auth-data to be sent out for mutual authentication
  //
  UINT32                        OutIdentifier;
  UINT8                         OutChallenge[ISCSI_CHAP_AUTH_MAX_LEN];
  UINT32                        OutChallengeLength;
} ISCSI_CHAP_AUTH_DATA;

EFI_STATUS
IScsiCHAPOnRspReceived (
  IN ISCSI_CONNECTION  *Conn,
  IN BOOLEAN           Transit
  );

EFI_STATUS
IScsiCHAPToSendReq (
  IN ISCSI_CONNECTION  *Conn,
  IN NET_BUF           *Pdu
  );

#endif
