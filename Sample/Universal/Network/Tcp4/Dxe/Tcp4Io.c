/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  Tcp4Io.c

Abstract:

  I/O interfaces between TCP and IpIo.

--*/

#include "Tcp4Main.h"

VOID
Tcp4RxCallback (
  IN EFI_STATUS                       Status,
  IN ICMP_ERROR                       IcmpErr,
  IN EFI_NET_SESSION_DATA             *NetSession,
  IN NET_BUF                          *Pkt,
  IN VOID                             *Context    OPTIONAL
  )
/*++

Routine Description:

  Packet receive callback function provided to IP_IO, used to call
  the proper function to handle the packet received by IP.

Arguments:

  Status      - Status of the received packet.
  IcmpErr     - ICMP error number.
  NetSession  - Pointer to the net session of this packet.
  Pkt         - Pointer to the recieved packet.
  Context     - Pointer to the context configured in IpIoOpen(),
                not used now.

Returns:

  None

--*/
{
  if (EFI_SUCCESS == Status) {
    TcpInput (Pkt, NetSession->Source, NetSession->Dest);
  } else {
    TcpIcmpInput (Pkt, IcmpErr, NetSession->Source, NetSession->Dest);
  }
}

INTN
TcpSendIpPacket (
  IN TCP_CB    *Tcb,
  IN NET_BUF   *Nbuf,
  IN UINT32    Src,
  IN UINT32    Dest
  )
/*++

Routine Description:

  Send the segment to IP via IpIo function.

Arguments:

  Tcb   - Pointer to the TCP_CB of this TCP instance.
  Nbuf  - Pointer to the TCP segment to be sent.
  Src   - Source address of the TCP segment.
  Dest  - Destination address of the TCP segment.

Returns:

  0     - The segment was sent out successfully.
  -1    - The segment was failed to send.

--*/
{
  EFI_STATUS       Status;
  IP_IO            *IpIo;
  IP_IO_OVERRIDE   Override;
  SOCKET           *Sock;
  VOID             *IpSender;
  TCP4_PROTO_DATA  *TcpProto;

  if (NULL == Tcb) {

    IpIo     = NULL;
    IpSender = IpIoFindSender (&IpIo, Src);

    if (IpSender == NULL) {
      TCP4_DEBUG_WARN (("TcpSendIpPacket: No appropriate IpSender.\n"));
      return -1;
    }
  } else {
  
    Sock     = Tcb->Sk;
    TcpProto = (TCP4_PROTO_DATA *) Sock->ProtoReserved;
    IpIo     = TcpProto->TcpService->IpIo;
    IpSender = Tcb->IpInfo;
  }

  Override.TypeOfService            = 0;
  Override.TimeToLive               = 255;
  Override.DoNotFragment            = FALSE;
  Override.Protocol                 = EFI_IP_PROTO_TCP;
  EFI_IP4 (Override.GatewayAddress) = 0;
  EFI_IP4 (Override.SourceAddress)  = Src;

  Status = IpIoSend (IpIo, Nbuf, IpSender, NULL, NULL, Dest, &Override);

  if (EFI_ERROR (Status)) {
    TCP4_DEBUG_ERROR (("TcpSendIpPacket: return %r error\n", Status));
    return -1;
  }

  return 0;
}
