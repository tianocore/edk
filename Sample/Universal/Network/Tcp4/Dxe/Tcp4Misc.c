/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  Tcp4Misc.c

Abstract:

  Misc support routines for tcp.

--*/


#include "Tcp4Main.h"

NET_LIST_ENTRY  mTcpRunQue = {
  &mTcpRunQue,
  &mTcpRunQue
};

NET_LIST_ENTRY  mTcpListenQue = {
  &mTcpListenQue,
  &mTcpListenQue
};

TCP_SEQNO       mTcpGlobalIss = 0x4d7e980b;

STATIC CHAR16   *mTcpStateName[] = {
  L"TCP_CLOSED",
  L"TCP_LISTEN",
  L"TCP_SYN_SENT",
  L"TCP_SYN_RCVD",
  L"TCP_ESTABLISHED",
  L"TCP_FIN_WAIT_1",
  L"TCP_FIN_WAIT_2",
  L"TCP_CLOSING",
  L"TCP_TIME_WAIT",
  L"TCP_CLOSE_WAIT",
  L"TCP_LAST_ACK"
};

VOID
TcpInitTcbLocal (
  IN TCP_CB *Tcb
  )
/*++

Routine Description:

  Initialize the Tcb local related members.

Arguments:

  Tcb - Pointer to the TCP_CB of this TCP instance.

Returns:

  None

--*/
{
  //
  // Compute the checksum of the fixed parts of pseudo header
  //
  Tcb->HeadSum = NetPseudoHeadChecksum (
                  Tcb->LocalEnd.Ip,
                  Tcb->RemoteEnd.Ip,
                  0x06,
                  0
                  );

  Tcb->Iss    = TcpGetIss ();
  Tcb->SndUna = Tcb->Iss;
  Tcb->SndNxt = Tcb->Iss;

  Tcb->SndWl2 = Tcb->Iss;
  Tcb->SndWnd = 536;

  Tcb->RcvWnd = GET_RCV_BUFFSIZE (Tcb->Sk);

  //
  // Fisrt window size is never scaled
  //
  Tcb->RcvWndScale = 0;
}

VOID
TcpInitTcbPeer (
  IN TCP_CB     *Tcb,
  IN TCP_SEG    *Seg,
  IN TCP_OPTION *Opt
  )
/*++

Routine Description:

  Initialize the peer related members.

Arguments:

  Tcb - Pointer to the TCP_CB of this TCP instance.
  Seg - Pointer to the segment that contains the peer's intial info.
  Opt - Pointer to the options announced by the peer.

Returns:

  None

--*/
{
  UINT16  RcvMss;

  ASSERT (Tcb && Seg && Opt);
  ASSERT (TCP_FLG_ON (Seg->Flag, TCP_FLG_SYN));

  Tcb->SndWnd     = Seg->Wnd;
  Tcb->SndWndMax  = Tcb->SndWnd;
  Tcb->SndWl1     = Seg->Seq;

  if (TCP_FLG_ON (Seg->Flag, TCP_FLG_ACK)) {
    Tcb->SndWl2 = Seg->Ack;
  } else {
    Tcb->SndWl2 = Tcb->Iss + 1;
  }

  if (TCP_FLG_ON (Opt->Flag, TCP_OPTION_RCVD_MSS)) {
    Tcb->SndMss = NET_MAX (64, Opt->Mss);

    RcvMss = TcpGetRcvMss (Tcb->Sk);
    if (Tcb->SndMss > RcvMss) {
      Tcb->SndMss = RcvMss;
    }

  } else {
    //
    // One end doesn't support MSS option, use default.
    //
    Tcb->RcvMss = 536;
  }

  Tcb->CWnd   = Tcb->SndMss;

  Tcb->Irs    = Seg->Seq;
  Tcb->RcvNxt = Tcb->Irs + 1;

  Tcb->RcvWl2 = Tcb->RcvNxt;

  if (TCP_FLG_ON (Opt->Flag, TCP_OPTION_RCVD_WS) &&
      !TCP_FLG_ON (Tcb->CtrlFlag, TCP_CTRL_NO_WS)) {

    Tcb->SndWndScale  = Opt->WndScale;

    Tcb->RcvWndScale  = TcpComputeScale (Tcb);
    TCP_SET_FLG (Tcb->CtrlFlag, TCP_CTRL_RCVD_WS);

  } else {
    //
    // One end doesn't support window scale option. use zero.
    //
    Tcb->RcvWndScale = 0;
  }

  if (TCP_FLG_ON (Opt->Flag, TCP_OPTION_RCVD_TS) &&
      !TCP_FLG_ON (Tcb->CtrlFlag, TCP_CTRL_NO_TS)) {

    TCP_SET_FLG (Tcb->CtrlFlag, TCP_CTRL_SND_TS);
    TCP_SET_FLG (Tcb->CtrlFlag, TCP_CTRL_RCVD_TS);

    //
    // Compute the effective SndMss per RFC1122
    // section 4.2.2.6. If timestamp option is
    // enabled, it will always occupy 12 bytes.
    //
    Tcb->SndMss -= TCP_OPTION_TS_ALIGNED_LEN;
  }
}

STATIC
TCP_CB *
TcpLocateListenTcb (
  IN TCP_PEER *Local,
  IN TCP_PEER *Remote
  )
/*++

Routine Description:

  Locate a listen TCB that matchs the Local and Remote.

Arguments:

  Local   - Pointer to the local (IP, Port).
  Remote  - Pointer to the remote (IP, Port).

Returns:

  Pointer to the TCP_CB with the least number of wildcard, if NULL no match is found.

--*/
{
  NET_LIST_ENTRY  *Entry;
  TCP_CB          *Node;
  TCP_CB          *Match;
  INTN            Last;
  INTN            Cur;

  Last  = 4;
  Match = NULL;

  NET_LIST_FOR_EACH (Entry, &mTcpListenQue) {
    Node = NET_LIST_USER_STRUCT (Entry, TCP_CB, List);

    if ((Local->Port != Node->LocalEnd.Port) ||
        !TCP_PEER_MATCH (Remote, &Node->RemoteEnd) ||
        !TCP_PEER_MATCH (Local, &Node->LocalEnd)
          ) {

      continue;
    }

    //
    // Compute the number of wildcard
    //
    Cur = 0;
    if (Node->RemoteEnd.Ip == 0) {
      Cur++;
    }

    if (Node->RemoteEnd.Port == 0) {
      Cur++;
    }

    if (Node->LocalEnd.Ip == 0) {
      Cur++;
    }

    if (Cur < Last) {
      if (Cur == 0) {
        return Node;
      }

      Last  = Cur;
      Match = Node;
    }
  }

  return Match;
}

BOOLEAN
TcpFindTcbByPeer (
  IN EFI_IPv4_ADDRESS  *Addr,
  IN TCP_PORTNO        Port
  )
/*++

Routine Description:

  Try to find one Tcb whose <Ip, Port> equals to <IN Addr, IN Port>.

Arguments:

  Addr  - Pointer to the IP address needs to match.
  Port  - The port number needs to match.

Returns:

  The Tcb which matches the <Addr Port> paire exists or not. 

--*/
{
  TCP_PORTNO      LocalPort;
  NET_LIST_ENTRY  *Entry;
  TCP_CB          *Tcb;

  ASSERT ((Addr != NULL) && (Port != 0));

  LocalPort = HTONS (Port);

  NET_LIST_FOR_EACH (Entry, &mTcpListenQue) {
    Tcb = NET_LIST_USER_STRUCT (Entry, TCP_CB, List);

    if ((EFI_IP4 (*Addr) == Tcb->LocalEnd.Ip) &&
      (LocalPort == Tcb->LocalEnd.Port)) {

      return TRUE;
    }
  }

  NET_LIST_FOR_EACH (Entry, &mTcpRunQue) {
    Tcb = NET_LIST_USER_STRUCT (Entry, TCP_CB, List);

    if (((EFI_IP4 (*Addr) == Tcb->LocalEnd.Ip)) &&
      (LocalPort == Tcb->LocalEnd.Port)) {

      return TRUE;
    }
  }

  return FALSE;
}

TCP_CB *
TcpLocateTcb (
  IN TCP_PORTNO  LocalPort,
  IN UINT32      LocalIp,
  IN TCP_PORTNO  RemotePort,
  IN UINT32      RemoteIp,
  IN BOOLEAN     Syn
  )
/*++

Routine Description:

  Locate the TCP_CB related to the socket pair.

Arguments:

  LocalPort   - The local port number.
  LocalIp     - The local IP address.
  RemotePort  - The remote port number.
  RemoteIp    - The remote IP address.
  Syn         - Whether to search the listen sockets,
                if TRUE, the listen sockets are searched.

Returns:

  Pointer to the related TCP_CB, if NULL no match is found.

--*/
{
  TCP_PEER        Local;
  TCP_PEER        Remote;
  NET_LIST_ENTRY  *Entry;
  TCP_CB          *Tcb;

  Local.Port  = LocalPort;
  Local.Ip    = LocalIp;

  Remote.Port = RemotePort;
  Remote.Ip   = RemoteIp;

  //
  // First check for exact match. 
  //
  NET_LIST_FOR_EACH (Entry, &mTcpRunQue) {
    Tcb = NET_LIST_USER_STRUCT (Entry, TCP_CB, List);

    if (TCP_PEER_EQUAL (&Remote, &Tcb->RemoteEnd) &&
        TCP_PEER_EQUAL (&Local, &Tcb->LocalEnd)) {

      NetListRemoveEntry (&Tcb->List);
      NetListInsertHead (&mTcpRunQue, &Tcb->List);

      return Tcb;
    }
  }

  //
  // Only check listen queue when SYN flag is on
  //
  if (Syn) {
    return TcpLocateListenTcb (&Local, &Remote);
  }

  return NULL;
}

INTN
TcpInsertTcb (
  IN TCP_CB *Tcb
  )
/*++

Routine Description:

  Insert a Tcb into the proper queue.

Arguments:

  Tcb - Pointer to the TCP_CB to be inserted.

Returns:

  0   - The Tcb is inserted successfully.
  -1  - Error condition occurred.

--*/
{
  NET_LIST_ENTRY   *Entry;
  NET_LIST_ENTRY   *Head;
  TCP_CB           *Node;
  TCP4_PROTO_DATA  *TcpProto;

  ASSERT (
    Tcb &&
    (
    (Tcb->State == TCP_LISTEN) ||
    (Tcb->State == TCP_SYN_SENT) ||
    (Tcb->State == TCP_SYN_RCVD) ||
    (Tcb->State == TCP_CLOSED)
    )
    );

  if (Tcb->LocalEnd.Port == 0) {
    return -1;
  }

  Head = &mTcpRunQue;

  if (Tcb->State == TCP_LISTEN) {
    Head = &mTcpListenQue;
  }

  //
  // Check that Tcb isn't already on the list.
  //
  NET_LIST_FOR_EACH (Entry, Head) {
    Node = NET_LIST_USER_STRUCT (Entry, TCP_CB, List);

    if (TCP_PEER_EQUAL (&Tcb->LocalEnd, &Node->LocalEnd) &&
        TCP_PEER_EQUAL (&Tcb->RemoteEnd, &Node->RemoteEnd)) {

      return -1;
    }
  }

  NetListInsertHead (Head, &Tcb->List);

  TcpProto = (TCP4_PROTO_DATA *) Tcb->Sk->ProtoReserved;
  TcpSetVariableData (TcpProto->TcpService);

  return 0;
}

TCP_CB *
TcpCloneTcb (
  IN TCP_CB *Tcb
  )
/*++

Routine Description:

  Clone a TCP_CB from Tcb.

Arguments:

  Tcb - Pointer to the TCP_CB to be cloned.

Returns:

  Pointer to the new cloned TCP_CB, if NULL error condition occurred.

--*/
{
  TCP_CB               *Clone;

  Clone = NetAllocatePool (sizeof (TCP_CB));

  if (Clone == NULL) {
    return NULL;

  }

  NetCopyMem (Clone, Tcb, sizeof (TCP_CB));

  //
  // Increate the reference count of the shared IpInfo.
  //
  NET_GET_REF (Tcb->IpInfo);

  NetListInit (&Clone->List);
  NetListInit (&Clone->SndQue);
  NetListInit (&Clone->RcvQue);

  Clone->Sk = SockClone (Tcb->Sk);
  if (Clone->Sk == NULL) {
    TCP4_DEBUG_ERROR (("TcpCloneTcb: failed to clone a sock\n"));
    NetFreePool (Clone);
    return NULL;
  }

  ((TCP4_PROTO_DATA *) (Clone->Sk->ProtoReserved))->TcpPcb = Clone;

  return Clone;
}

TCP_SEQNO
TcpGetIss (
  VOID
  )
/*++

Routine Description:

  Compute an ISS to be used by a new connection.

Arguments:

  None

Returns:

  The result ISS.

--*/
{
  mTcpGlobalIss += 2048;
  return mTcpGlobalIss;
}

UINT16
TcpGetRcvMss (
  IN SOCKET  *Sock
  )
/*++

Routine Description:

  Get the local mss.

Arguments:

  None

Returns:

  The mss size.

--*/
{
  EFI_SIMPLE_NETWORK_MODE SnpMode;
  TCP4_PROTO_DATA         *TcpProto;
  EFI_IP4_PROTOCOL        *Ip;

  ASSERT (Sock);

  TcpProto = (TCP4_PROTO_DATA *) Sock->ProtoReserved;
  Ip       = TcpProto->TcpService->IpIo->Ip;
  ASSERT (Ip);

  Ip->GetModeData (Ip, NULL, NULL, &SnpMode);

  return (UINT16) (SnpMode.MaxPacketSize - 40);
}

VOID
TcpSetState (
  IN TCP_CB *Tcb,
  IN UINT8  State
  )
/*++

Routine Description:

  Set the Tcb's state.

Arguments:

  Tcb   - Pointer to the TCP_CB of this TCP instance.
  State - The state to be set.

Returns:

  None

--*/
{
  TCP4_DEBUG_TRACE (
    ("Tcb (%x) state %s --> %s\n",
    Tcb,
    mTcpStateName[Tcb->State],
    mTcpStateName[State])
    );

  Tcb->State = State;

  switch (State) {
  case TCP_ESTABLISHED:

    SockConnEstablished (Tcb->Sk);
    break;

  case TCP_CLOSED:

    SockConnClosed (Tcb->Sk);

    break;
  }
}

UINT16
TcpChecksum (
  IN NET_BUF *Nbuf,
  IN UINT16  HeadSum
  )
/*++

Routine Description:

  Compute the TCP segment's checksum.

Arguments:

  Nbuf    - Pointer to the buffer that contains the TCP segment.
  HeadSum - The checksum value of the fixed part of pseudo header.

Returns:

  The checksum value.

--*/
{
  UINT16  Checksum;

  Checksum  = NetbufChecksum (Nbuf);
  Checksum  = NetAddChecksum (Checksum, HeadSum);

  Checksum = NetAddChecksum (
              Checksum,
              HTONS ((UINT16) Nbuf->TotalSize)
              );

  return ~Checksum;
}

TCP_SEG *
TcpFormatNetbuf (
  IN TCP_CB  *Tcb,
  IN NET_BUF *Nbuf
  )
/*++

Routine Description:

  Translate the information from the head of the received TCP
  segment Nbuf contains and fill it into a TCP_SEG structure.

Arguments:

  Tcb   - Pointer to the TCP_CB of this TCP instance.
  Nbuf  - Pointer to the buffer contains the TCP segment.

Returns:

  Pointer to the TCP_SEG that contains the translated TCP head information.

--*/
{
  TCP_SEG   *Seg;
  TCP_HEAD  *Head;

  Seg       = TCPSEG_NETBUF (Nbuf);
  Head      = (TCP_HEAD *) NetbufGetByte (Nbuf, 0, NULL);
  Nbuf->Tcp = Head;

  Seg->Seq  = NTOHL (Head->Seq);
  Seg->Ack  = NTOHL (Head->Ack);
  Seg->End  = Seg->Seq + (Nbuf->TotalSize - (Head->HeadLen << 2));

  Seg->Urg  = NTOHS (Head->Urg);
  Seg->Wnd  = (NTOHS (Head->Wnd) << Tcb->SndWndScale);
  Seg->Flag = Head->Flag;

  //
  // SYN and FIN flag occupy one sequence space each.
  //
  if (TCP_FLG_ON (Seg->Flag, TCP_FLG_SYN)) {
    //
    // RFC requires that initial window not be scaled
    //
    Seg->Wnd = NTOHS (Head->Wnd);
    Seg->End++;
  }

  if (TCP_FLG_ON (Seg->Flag, TCP_FLG_FIN)) {
    Seg->End++;
  }

  return Seg;
}

VOID
TcpResetConnection (
  IN TCP_CB *Tcb
  )
/*++

Routine Description:

  Reset the connection related with Tcb.

Arguments:

  Tcb - Pointer to the TCP_CB of the connection to be reset.

Returns:

  None

--*/
{
  NET_BUF   *Nbuf;
  TCP_HEAD  *Nhead;

  Nbuf = NetbufAlloc (TCP_MAX_HEAD);

  if (Nbuf == NULL) {
    return ;
  }

  Nhead = (TCP_HEAD *) NetbufAllocSpace (
                        Nbuf,
                        sizeof (TCP_HEAD),
                        NET_BUF_TAIL
                        );

  ASSERT (Nhead != NULL);

  Nbuf->Tcp       = Nhead;

  Nhead->Flag     = TCP_FLG_RST;
  Nhead->Seq      = HTONL (Tcb->SndNxt);
  Nhead->Ack      = HTONL (Tcb->RcvNxt);
  Nhead->SrcPort  = Tcb->LocalEnd.Port;
  Nhead->DstPort  = Tcb->RemoteEnd.Port;
  Nhead->HeadLen  = (sizeof (TCP_HEAD) >> 2);
  Nhead->Res      = 0;
  Nhead->Wnd      = HTONS (0xFFFF);
  Nhead->Checksum = 0;
  Nhead->Urg      = 0;
  Nhead->Checksum = TcpChecksum (Nbuf, Tcb->HeadSum);

  TcpSendIpPacket (Tcb, Nbuf, Tcb->LocalEnd.Ip, Tcb->RemoteEnd.Ip);

  NetbufFree (Nbuf);
}

VOID
TcpOnAppConnect (
  IN TCP_CB  *Tcb
  )
/*++

Routine Description:

  Initialize an active connection, 

Arguments:

  Tcb - Pointer to the TCP_CB that wants to initiate
        a connection.

Returns:

  None

--*/
{
  TcpInitTcbLocal (Tcb);
  TcpSetState (Tcb, TCP_SYN_SENT);

  TcpSetTimer (Tcb, TCP_TIMER_CONNECT, Tcb->ConnectTimeout);
  TcpToSendData (Tcb, 1);
}

VOID
TcpOnAppClose (
  IN TCP_CB *Tcb
  )
/*++

Routine Description:

  Initiate the connection close procedure, called when
  applications want to close the connection.

Arguments:

  Tcb - Pointer to the TCP_CB of this TCP instance.

Returns:

  None.

--*/
{
  ASSERT (Tcb);

  if (!NetListIsEmpty (&Tcb->RcvQue) || GET_RCV_DATASIZE (Tcb->Sk)) {

    TCP4_DEBUG_WARN (("TcpOnAppClose: connection reset "
      "because data is lost for TCB %x\n", Tcb));

    TcpResetConnection (Tcb);
    TcpClose (Tcb);
    return;
  }

  switch (Tcb->State) {
  case TCP_CLOSED:
  case TCP_LISTEN:
  case TCP_SYN_SENT:
    TcpSetState (Tcb, TCP_CLOSED);
    break;

  case TCP_SYN_RCVD:
  case TCP_ESTABLISHED:
    TcpSetState (Tcb, TCP_FIN_WAIT_1);
    break;

  case TCP_CLOSE_WAIT:
    TcpSetState (Tcb, TCP_LAST_ACK);
    break;
  }

  TcpToSendData (Tcb, 1);
}

INTN
TcpOnAppSend (
  IN TCP_CB *Tcb
  )
/*++

Routine Description:

  Check whether the application's newly delivered data
  can be sent out.

Arguments:

  Tcb - Pointer to the TCP_CB of this TCP instance.

Returns:

  0   - Whether the data is sent out or is buffered for
        further sending.
  -1  - The Tcb is not in a state that data is permitted
        to be sent out.

--*/
{

  switch (Tcb->State) {
  case TCP_CLOSED:
    return -1;
    break;

  case TCP_LISTEN:
    return -1;
    break;

  case TCP_SYN_SENT:
  case TCP_SYN_RCVD:
    return 0;
    break;

  case TCP_ESTABLISHED:
  case TCP_CLOSE_WAIT:
    TcpToSendData (Tcb, 0);
    return 0;
    break;

  case TCP_FIN_WAIT_1:
  case TCP_FIN_WAIT_2:
  case TCP_CLOSING:
  case TCP_LAST_ACK:
  case TCP_TIME_WAIT:
    return -1;
    break;
  }

  return 0;
}

INTN
TcpOnAppConsume (
  IN TCP_CB *Tcb
  )
/*++

Routine Description:

  Application has consumed some data, check whether
  to send a window updata ack or a delayed ack.

Arguments:

  Tcb - Pointer to the TCP_CB of this TCP instance.

Returns:
  
--*/
{

  switch (Tcb->State) {
  case TCP_CLOSED:
    return -1;
    break;

  case TCP_LISTEN:
    return -1;
    break;

  case TCP_SYN_SENT:
  case TCP_SYN_RCVD:
    return 0;
    break;

  case TCP_ESTABLISHED:
    if (TcpRcvWinNow (Tcb) > TcpRcvWinOld (Tcb)) {

      if (TcpRcvWinOld (Tcb) < Tcb->RcvMss) {

        TCP4_DEBUG_TRACE (("TcpOnAppConsume: send a window"
          " update for a window closed Tcb(%x)\n", Tcb));

        TcpSendAck (Tcb);
      } else if (Tcb->DelayedAck == 0) {

        TCP4_DEBUG_TRACE (("TcpOnAppConsume: scheduled a delayed"
          " ACK to update window for Tcb(%x)\n", Tcb));

        Tcb->DelayedAck = 1;
      }
    }

    break;

  case TCP_CLOSE_WAIT:
    return 0;
    break;

  case TCP_FIN_WAIT_1:
  case TCP_FIN_WAIT_2:
  case TCP_CLOSING:
  case TCP_LAST_ACK:
  case TCP_TIME_WAIT:
    return -1;
    break;
  }

  return -1;
}

VOID
TcpOnAppAbort (
  IN TCP_CB *Tcb
  )
/*++

Routine Description:

  Abort the connection by sending a reset segment, called
  when the application wants to abort the connection.

Arguments:

  Tcb - Pointer to the TCP_CB of the TCP instance.

Returns:

  None.

--*/
{
  TCP4_DEBUG_WARN (("TcpOnAppAbort: connection reset "
    "issued by application for TCB %x\n", Tcb));

  switch (Tcb->State) {
  case TCP_SYN_RCVD:
  case TCP_ESTABLISHED:
  case TCP_FIN_WAIT_1:
  case TCP_FIN_WAIT_2:
  case TCP_CLOSE_WAIT:
    TcpResetConnection (Tcb);
    break;
  }

  TcpSetState (Tcb, TCP_CLOSED);
}

EFI_STATUS
TcpSetVariableData (
  IN TCP4_SERVICE_DATA  *Tcp4Service
  )
/*++

Routine Description:

  Set the Tdp4 variable data.

Arguments:

  Tcp4Service - Tcp4 service data.

Returns:

  EFI_OUT_OF_RESOURCES - There are not enough resources to set the variable.
  other                - Set variable failed.

--*/
{
  UINT32                  NumConfiguredInstance;
  NET_LIST_ENTRY          *Entry;
  TCP_CB                  *TcpPcb;
  TCP4_PROTO_DATA         *TcpProto;
  UINTN                   VariableDataSize;
  EFI_TCP4_VARIABLE_DATA  *Tcp4VariableData;
  EFI_TCP4_SERVICE_POINT  *Tcp4ServicePoint;
  CHAR16                  *NewMacString;
  EFI_STATUS              Status;

  NumConfiguredInstance = 0;

  //
  // Go through the running queue to count the instances.
  //
  NET_LIST_FOR_EACH (Entry, &mTcpRunQue) {
    TcpPcb = NET_LIST_USER_STRUCT (Entry, TCP_CB, List);

    TcpProto = (TCP4_PROTO_DATA *) TcpPcb->Sk->ProtoReserved;

    if (TcpProto->TcpService == Tcp4Service) {
      //
      // This tcp instance belongs to the Tcp4Service.
      //
      NumConfiguredInstance++;
    }
  }

  //
  // Go through the listening queue to count the instances.
  //
  NET_LIST_FOR_EACH (Entry, &mTcpListenQue) {
    TcpPcb = NET_LIST_USER_STRUCT (Entry, TCP_CB, List);

    TcpProto = (TCP4_PROTO_DATA *) TcpPcb->Sk->ProtoReserved;

    if (TcpProto->TcpService == Tcp4Service) {
      //
      // This tcp instance belongs to the Tcp4Service.
      //
      NumConfiguredInstance++;
    }
  }

  //
  // Calculate the size of the Tcp4VariableData. As there may be no Tcp4 child,
  // we should add extra buffer for the service points only if the number of configured
  // children is more than 1.
  //
  VariableDataSize = sizeof (EFI_TCP4_VARIABLE_DATA);

  if (NumConfiguredInstance > 1) {
    VariableDataSize += sizeof (EFI_TCP4_SERVICE_POINT) * (NumConfiguredInstance - 1);
  }
  
  Tcp4VariableData = NetAllocatePool (VariableDataSize);
  if (Tcp4VariableData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Tcp4VariableData->DriverHandle = Tcp4Service->DriverBindingHandle;
  Tcp4VariableData->ServiceCount = NumConfiguredInstance;

  Tcp4ServicePoint = &Tcp4VariableData->Services[0];

  //
  // Go through the running queue to fill the service points.
  //
  NET_LIST_FOR_EACH (Entry, &mTcpRunQue) {
    TcpPcb = NET_LIST_USER_STRUCT (Entry, TCP_CB, List);

    TcpProto = (TCP4_PROTO_DATA *) TcpPcb->Sk->ProtoReserved;

    if (TcpProto->TcpService == Tcp4Service) {
      //
      // This tcp instance belongs to the Tcp4Service.
      //
      Tcp4ServicePoint->InstanceHandle          = TcpPcb->Sk->SockHandle;
      EFI_IP4 (Tcp4ServicePoint->LocalAddress)  = TcpPcb->LocalEnd.Ip;
      Tcp4ServicePoint->LocalPort               = NTOHS (TcpPcb->LocalEnd.Port);
      EFI_IP4 (Tcp4ServicePoint->RemoteAddress) = TcpPcb->RemoteEnd.Ip;
      Tcp4ServicePoint->RemotePort              = NTOHS (TcpPcb->RemoteEnd.Port);

      Tcp4ServicePoint++;
    }
  }

  //
  // Go through the listening queue to fill the service points.
  //
  NET_LIST_FOR_EACH (Entry, &mTcpListenQue) {
    TcpPcb = NET_LIST_USER_STRUCT (Entry, TCP_CB, List);

    TcpProto = (TCP4_PROTO_DATA *) TcpPcb->Sk->ProtoReserved;

    if (TcpProto->TcpService == Tcp4Service) {
      //
      // This tcp instance belongs to the Tcp4Service.
      //
      Tcp4ServicePoint->InstanceHandle          = TcpPcb->Sk->SockHandle;
      EFI_IP4 (Tcp4ServicePoint->LocalAddress)  = TcpPcb->LocalEnd.Ip;
      Tcp4ServicePoint->LocalPort               = NTOHS (TcpPcb->LocalEnd.Port);
      EFI_IP4 (Tcp4ServicePoint->RemoteAddress) = TcpPcb->RemoteEnd.Ip;
      Tcp4ServicePoint->RemotePort              = NTOHS (TcpPcb->RemoteEnd.Port);

      Tcp4ServicePoint++;
    }
  }

  //
  // Get the mac string.
  //
  Status = NetLibGetMacString (
             Tcp4Service->ControllerHandle,
             Tcp4Service->DriverBindingHandle,
             &NewMacString
             );
  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  if (Tcp4Service->MacString != NULL) {
    //
    // The variable is set already, we're going to update it.
    //
    if (EfiStrCmp (Tcp4Service->MacString, NewMacString) != 0) {
      //
      // The mac address is changed, delete the previous variable first.
      //
      gRT->SetVariable (
             Tcp4Service->MacString,
             &gEfiTcp4ServiceBindingProtocolGuid,
             EFI_VARIABLE_BOOTSERVICE_ACCESS,
             0,
             NULL
             );
    }

    NetFreePool (Tcp4Service->MacString);
  }

  Tcp4Service->MacString = NewMacString;

  Status = gRT->SetVariable (
                  Tcp4Service->MacString,
                  &gEfiTcp4ServiceBindingProtocolGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  VariableDataSize,
                  (VOID *) Tcp4VariableData
                  );

ON_ERROR:

  NetFreePool (Tcp4VariableData);

  return Status;
}

VOID
TcpClearVariableData (
  IN TCP4_SERVICE_DATA  *Tcp4Service
  )
/*++

Routine Description:

  Clear the variable and free the resource.

Arguments:

  Tcp4Service - Tcp4 service data.

Returns:

  None.

--*/
{
  ASSERT (Tcp4Service->MacString != NULL);

  gRT->SetVariable (
         Tcp4Service->MacString,
         &gEfiTcp4ServiceBindingProtocolGuid,
         EFI_VARIABLE_BOOTSERVICE_ACCESS,
         0,
         NULL
         );

  NetFreePool (Tcp4Service->MacString);
  Tcp4Service->MacString = NULL;
}

