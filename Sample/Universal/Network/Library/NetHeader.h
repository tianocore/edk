/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  NetHeader.h

Abstract:

  Definition of various network header, maybe used by various network drivers.
  Functions declared in this file are included in NetLib. If only use the 
  defintions and macros, it isn't necessary to link the NetLib in.

--*/

#ifndef _NET_HEADER_H_
#define _NET_HEADER_H_

#define EFI_NET_LITTLE_ENDIAN

typedef UINT32          IP4_ADDR;
typedef UINT32          TCP_SEQNO;
typedef UINT16          TCP_PORTNO;

enum {
  NET_ETHER_ADDR_LEN    = 6,
  NET_IFTYPE_ETHERNET   = 0x01,
    
  EFI_IP_PROTO_UDP      = 0x11,
  EFI_IP_PROTO_TCP      = 0x06,
  EFI_IP_PROTO_ICMP     = 0x01,

  //
  // The address classfication
  //
  IP4_ADDR_CLASSA       = 1,
  IP4_ADDR_CLASSB,
  IP4_ADDR_CLASSC,
  IP4_ADDR_CLASSD,
  IP4_ADDR_CLASSE,

  IP4_MASK_NUM          = 33,
};

#pragma pack(1)

//
// Ethernet head definition
//
typedef struct {
  UINT8                 DstMac [NET_ETHER_ADDR_LEN];
  UINT8                 SrcMac [NET_ETHER_ADDR_LEN];
  UINT16                EtherType;
} ETHER_HEAD;


//
// The EFI_IP4_HEADER is hard to use because the source and
// destination address are defined as EFI_IPv4_ADDRESS, which
// is a structure. Two structures can't be compared or masked
// directly. This is why there is an internal representation.
//
typedef struct {
#ifdef EFI_NET_LITTLE_ENDIAN
  UINT8                 HeadLen : 4;
  UINT8                 Ver     : 4;
#else
  UINT8                 Ver     : 4;
  UINT8                 HeadLen : 4;
#endif
  UINT8                 Tos;
  UINT16                TotalLen;
  UINT16                Id;
  UINT16                Fragment;
  UINT8                 Ttl;
  UINT8                 Protocol;
  UINT16                Checksum;
  IP4_ADDR              Src;
  IP4_ADDR              Dst;
} IP4_HEAD;


//
// ICMP head definition. ICMP message is categoried as either an error
// message or query message. Two message types have their own head format.
//
typedef struct {
  UINT8                 Type;
  UINT8                 Code;
  UINT16                Checksum;
} IP4_ICMP_HEAD;

typedef struct {
  IP4_ICMP_HEAD         Head;
  UINT32                Fourth; // 4th filed of the head, it depends on Type.
  IP4_HEAD              IpHead;
} IP4_ICMP_ERROR_HEAD;

typedef struct {
  IP4_ICMP_HEAD         Head;
  UINT16                Id;
  UINT16                Seq;
} IP4_ICMP_QUERY_HEAD;


//
// UDP header definition
//
typedef struct {
  UINT16                SrcPort;
  UINT16                DstPort;
  UINT16                Length;
  UINT16                Checksum;
} EFI_UDP4_HEADER;


//
// TCP header definition
//
typedef struct {
  TCP_PORTNO            SrcPort;
  TCP_PORTNO            DstPort;
  TCP_SEQNO             Seq;
  TCP_SEQNO             Ack;
#ifdef EFI_NET_LITTLE_ENDIAN
  UINT8                 Res     : 4;
  UINT8                 HeadLen : 4;
#else
  UINT8                 HeadLen : 4; 
  UINT8                 Res     : 4;
#endif
  UINT8                 Flag;
  UINT16                Wnd;
  UINT16                Checksum;
  UINT16                Urg;
} TCP_HEAD;

#pragma pack()

#define NET_MAC_EQUAL(pMac1, pMac2, Len)     \
    (NetCompareMem ((pMac1), (pMac2), Len) == 0)
    
#define NET_MAC_IS_MULTICAST(Mac, BMac, Len) \
    (((*((UINT8 *) Mac) & 0x01) == 0x01) && (!NET_MAC_EQUAL (Mac, BMac, Len)))

#ifdef EFI_NET_LITTLE_ENDIAN
#define NTOHL(x) (UINT32)((((UINT32) (x) & 0xff)     << 24) | \
                          (((UINT32) (x) & 0xff00)   << 8)  | \
                          (((UINT32) (x) & 0xff0000) >> 8)  | \
                          (((UINT32) (x) & 0xff000000) >> 24))

#define HTONL(x)  NTOHL(x)

#define NTOHS(x)  (UINT16)((((UINT16) (x) & 0xff) << 8) | \
                           (((UINT16) (x) & 0xff00) >> 8))

#define HTONS(x)  NTOHS(x)
#else
#define NTOHL(x)  (UINT32)(x)
#define HTONL(x)  (UINT32)(x)
#define NTOHS(x)  (UINT16)(x)
#define HTONS(x)  (UINT16)(x)
#endif

//
// Test the IP's attribute, All the IPs are in host byte order.
//
#define IP4_IS_MULTICAST(Ip)              (((Ip) & 0xF0000000) == 0xE0000000)
#define IP4_IS_LOCAL_BROADCAST(Ip)        ((Ip) == 0xFFFFFFFF)
#define IP4_NET_EQUAL(Ip1, Ip2, NetMask)  (((Ip1) & (NetMask)) == ((Ip2) & (NetMask)))
#define IP4_IS_VALID_NETMASK(Ip)          (NetGetMaskLength (Ip) != IP4_MASK_NUM)

//
// Convert the EFI_IP4_ADDRESS to plain UINT32 IP4 address.
//
#define EFI_IP4(EfiIpAddr)                (*(IP4_ADDR *) ((EfiIpAddr).Addr))
#define EFI_NTOHL(EfiIp)                  (NTOHL (EFI_IP4 ((EfiIp))))
#define EFI_IP_EQUAL(Ip1, Ip2)            (EFI_IP4 (Ip1) == EFI_IP4 (Ip2))

INTN
NetGetMaskLength (
  IN IP4_ADDR               Mask
  );

INTN
NetGetIpClass (
  IN IP4_ADDR               Addr
  );

BOOLEAN
Ip4IsUnicast (
  IN IP4_ADDR               Ip,
  IN IP4_ADDR               NetMask
  );

extern IP4_ADDR mIp4AllMasks [IP4_MASK_NUM];
#endif
