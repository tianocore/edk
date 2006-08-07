/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 


Module Name:

  Udp4Io.c

Abstract:

  Help functions to access UDP service, it is used by both the DHCP and MTFTP.
  
--*/

#include "Udp4Io.h"

STATIC
VOID
EFIAPI
UdpIoOnDgramSent (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  );

STATIC
VOID
EFIAPI
UdpIoOnDgramRcvd (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  );

STATIC
UDP_TX_TOKEN *
UdpIoWrapTx (
  IN UDP_IO_PORT            *UdpIo,
  IN NET_BUF                *Packet,
  IN UDP_POINTS             *EndPoint, OPTIONAL
  IN IP4_ADDR               Gateway,
  IN UDP_IO_CALLBACK        CallBack,
  IN VOID                   *Context
  )
/*++

Routine Description:

  Wrap a transmit request into a UDP_TX_TOKEN.

Arguments:

  UdpIo     - The UdpIo port to send packet to
  Packet    - The user's packet
  EndPoint  - The local and remote access point
  Gateway   - The overrided next hop
  CallBack  - The function to call when transmission completed.
  Context   - The opaque parameter to the call back

Returns:

  The wrapped transmission request or NULL if failed to allocate resources.

--*/
{
  UDP_TX_TOKEN              *Token;
  EFI_UDP4_COMPLETION_TOKEN *UdpToken;
  EFI_UDP4_TRANSMIT_DATA    *UdpTxData;
  EFI_STATUS                Status;
  UINT32                    Count;

  Token = NetAllocatePool (sizeof (UDP_TX_TOKEN) + 
                           sizeof (EFI_UDP4_FRAGMENT_DATA) * (Packet->BlockOpNum - 1));

  if (Token == NULL) {
    return NULL;
  }

  Token->Signature  = UDP_IO_TX_SIGNATURE;
  NetListInit (&Token->Link);

  Token->UdpIo      = UdpIo;
  Token->CallBack   = CallBack;
  Token->Packet     = Packet;
  Token->Context    = Context;

  UdpToken          = &(Token->UdpToken);
  UdpToken->Status  = EFI_NOT_READY;

  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_CALLBACK,
                  UdpIoOnDgramSent,
                  Token,
                  &UdpToken->Event
                  );

  if (EFI_ERROR (Status)) {
    NetFreePool (Token);
    return NULL;
  }

  UdpTxData                 = &Token->UdpTxData;
  UdpToken->Packet.TxData   = UdpTxData;

  UdpTxData->UdpSessionData = NULL;
  UdpTxData->GatewayAddress = NULL;

  if (EndPoint != NULL) {
    EFI_IP4 (Token->UdpSession.SourceAddress)      = HTONL (EndPoint->LocalAddr);
    EFI_IP4 (Token->UdpSession.DestinationAddress) = HTONL (EndPoint->RemoteAddr);
    Token->UdpSession.SourcePort                   = EndPoint->LocalPort;
    Token->UdpSession.DestinationPort              = EndPoint->RemotePort;
    UdpTxData->UdpSessionData                      = &Token->UdpSession;
  }

  if (Gateway != 0) {
    EFI_IP4 (Token->Gateway)  = HTONL (Gateway);
    UdpTxData->GatewayAddress = &Token->Gateway;
  }

  UdpTxData->DataLength = Packet->TotalSize;
  Count                 = Packet->BlockOpNum;
  NetbufBuildExt (Packet, (NET_FRAGMENT *) UdpTxData->FragmentTable, &Count);
  UdpTxData->FragmentCount = Count;

  return Token;
}

VOID
UdpIoFreeTxToken (
  IN UDP_TX_TOKEN           *Token
  )
/*++

Routine Description:

  Free a UDP_TX_TOKEN. The event is closed and memory released.

Arguments:

  Token - The UDP_TX_TOKEN to release.

Returns:

  None

--*/
{
  gBS->CloseEvent (Token->UdpToken.Event);
  NetFreePool (Token);
}

UDP_RX_TOKEN *
UdpIoCreateRxToken (
  IN UDP_IO_PORT            *UdpIo,
  IN UDP_IO_CALLBACK        CallBack,
  IN VOID                   *Context,
  IN UINT32                 HeadLen
  )
/*++

Routine Description:

  Create a UDP_RX_TOKEN to wrap the request.

Arguments:

  UdpIo     - The UdpIo to receive packets from
  CallBack  - The function to call when receive finished.
  Context   - The opaque parameter to the CallBack
  HeadLen   - The head length to reserver for the packet.

Returns:

  The Wrapped request or NULL if failed to allocate resources.

--*/
{
  UDP_RX_TOKEN              *Token;
  EFI_STATUS                Status;

  Token = NetAllocatePool (sizeof (UDP_RX_TOKEN));
  
  if (Token == NULL) {
    return NULL;
  }

  Token->Signature              = UDP_IO_RX_SIGNATURE;
  Token->UdpIo                  = UdpIo;
  Token->CallBack               = CallBack;
  Token->Context                = Context;
  Token->HeadLen                = HeadLen;

  Token->UdpToken.Status        = EFI_NOT_READY;
  Token->UdpToken.Packet.RxData = NULL;

  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_CALLBACK,
                  UdpIoOnDgramRcvd,
                  Token,
                  &Token->UdpToken.Event
                  );

  if (EFI_ERROR (Status)) {
    NetFreePool (Token);
    return NULL;
  }

  return Token;
}

VOID
UdpIoFreeRxToken (
  IN UDP_RX_TOKEN           *Token
  )
/*++

Routine Description:

  Free a receive request wrap.

Arguments:

  Token - The receive request to release.

Returns:

  None

--*/
{
  gBS->CloseEvent (Token->UdpToken.Event);
  NetFreePool (Token);
}

UDP_IO_PORT *
UdpIoCreatePort (
  IN  EFI_HANDLE            Controller,
  IN  EFI_HANDLE            Image,
  IN  UDP_IO_CONFIG         Configure,
  IN  VOID                  *Context
  )
/*++

Routine Description:

  Create a UDP IO port to access the UDP service. It will 
  create and configure a UDP child.

Arguments:

  Controller  - The controller that has the UDP service binding protocol installed.
  Image       - The image handle for the driver.
  Configure   - The function to configure the created UDP child
  Context     - The opaque parameter for the Configure funtion.

Returns:

  A point to just created UDP IO port or NULL if failed.

--*/
{
  UDP_IO_PORT               *UdpIo;
  EFI_STATUS                Status;

  ASSERT (Configure != NULL);

  UdpIo = NetAllocatePool (sizeof (UDP_IO_PORT));
  
  if (UdpIo == NULL) {
    return NULL;
  }

  UdpIo->Signature    = UDP_IO_SIGNATURE;
  NetListInit (&UdpIo->Link);
  UdpIo->RefCnt       = 1;

  UdpIo->Controller   = Controller;
  UdpIo->Image        = Image;

  NetListInit (&UdpIo->SentDatagram);
  UdpIo->RecvRequest  = NULL;
  UdpIo->UdpHandle    = NULL;

  //
  // Create a UDP child then open and configure it
  //
  Status = NetLibCreateServiceChild (
             Controller,
             Image,
             &gEfiUdp4ServiceBindingProtocolGuid,
             &UdpIo->UdpHandle
             );

  if (EFI_ERROR (Status)) {
    goto FREE_MEM;
  }

  Status = gBS->OpenProtocol (
                  UdpIo->UdpHandle,
                  &gEfiUdp4ProtocolGuid,
                  &UdpIo->Udp,
                  Image,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    goto FREE_CHILD;
  }

  if (EFI_ERROR (Configure (UdpIo, Context))) {
    goto CLOSE_PROTOCOL;
  }

  Status = UdpIo->Udp->GetModeData (UdpIo->Udp, NULL, NULL, NULL, &UdpIo->SnpMode);

  if (EFI_ERROR (Status)) {
    goto CLOSE_PROTOCOL;
  }

  return UdpIo;

CLOSE_PROTOCOL:
  gBS->CloseProtocol (UdpIo->UdpHandle, &gEfiUdp4ProtocolGuid, Image, Controller);

FREE_CHILD:
  NetLibDestroyServiceChild (
    Controller,
    Image,
    &gEfiUdp4ServiceBindingProtocolGuid,
    UdpIo->UdpHandle
    );

FREE_MEM:
  NetFreePool (UdpIo);
  return NULL;
}

STATIC
VOID
UdpIoCancelDgrams (
  IN UDP_IO_PORT            *UdpIo,
  IN EFI_STATUS             IoStatus,
  IN UDP_IO_TO_CANCEL       ToCancel,        OPTIONAL
  IN VOID                   *Context
  )
/*++

Routine Description:

  Cancel all the sent datagram that pass the selection of ToCancel. 
  If ToCancel is NULL, all the datagrams are cancelled.

Arguments:

  UdpIo     - The UDP IO port to cancel packet
  IoStatus  - The IoStatus to return to the packet owners.
  ToCancel  - The select funtion to test whether to cancel this packet or not.
  Context   - The opaque parameter to the ToCancel.

Returns:

  None

--*/
{
  NET_LIST_ENTRY            *Entry;
  NET_LIST_ENTRY            *Next;
  UDP_TX_TOKEN              *Token;

  NET_LIST_FOR_EACH_SAFE (Entry, Next, &UdpIo->SentDatagram) {
    Token = NET_LIST_USER_STRUCT (Entry, UDP_TX_TOKEN, Link);

    if ((ToCancel == NULL) || (ToCancel (Token, Context))) {
      NetListRemoveEntry (Entry);
      UdpIo->Udp->Cancel (UdpIo->Udp, &Token->UdpToken);
      Token->CallBack (Token->Packet, NULL, IoStatus, Token->Context);
      UdpIoFreeTxToken (Token);
    }
  }
}

EFI_STATUS
UdpIoFreePort (
  IN  UDP_IO_PORT           *UdpIo
  )
/*++

Routine Description:

  Free the UDP IO port and all its related resources including 
  all the transmitted packet.

Arguments:

  UdpIo - The UDP IO port to free.

Returns:

  EFI_SUCCESS - The UDP IO port is freed.

--*/
{
  UDP_RX_TOKEN  *RxToken;

  //
  // Cancel all the sent datagram and receive requests. The
  // callbacks of transmit requests are executed to allow the 
  // caller to release the resource. The callback of receive
  // request are NOT executed. This is because it is most 
  // likely that the current user of the UDP IO port is closing
  // itself. 
  //
  UdpIoCancelDgrams (UdpIo, EFI_ABORTED, NULL, NULL);

  if ((RxToken = UdpIo->RecvRequest) != NULL) {
    UdpIo->RecvRequest = NULL;
    UdpIo->Udp->Cancel (UdpIo->Udp, &RxToken->UdpToken);
    UdpIoFreeRxToken (RxToken);
  }
  
  //
  // Close then destory the UDP child
  //
  gBS->CloseProtocol (
         UdpIo->UdpHandle,
         &gEfiUdp4ProtocolGuid,
         UdpIo->Image,
         UdpIo->Controller
         );

  NetLibDestroyServiceChild (
    UdpIo->Controller,
    UdpIo->Image,
    &gEfiUdp4ServiceBindingProtocolGuid,
    UdpIo->UdpHandle
    );

  NetListRemoveEntry (&UdpIo->Link);
  NetFreePool (UdpIo);
  return EFI_SUCCESS;
}

VOID
UdpIoCleanPort (
  IN  UDP_IO_PORT           *UdpIo
  )
/*++

Routine Description:

  Clean up the UDP IO port. It will release all the transmitted
  datagrams and receive request. It will also configure NULL the
  UDP child.

Arguments:

  UdpIo - UDP IO port to clean up.

Returns:

  None

--*/
{
  UDP_RX_TOKEN              *RxToken;

  //
  // Cancel all the sent datagram and receive requests.
  //
  UdpIoCancelDgrams (UdpIo, EFI_ABORTED, NULL, NULL);

  if ((RxToken = UdpIo->RecvRequest) != NULL) {
    UdpIo->RecvRequest = NULL;
    UdpIo->Udp->Cancel (UdpIo->Udp, &RxToken->UdpToken);
    UdpIoFreeRxToken (RxToken);
  }

  UdpIo->Udp->Configure (UdpIo->Udp, NULL);
}

STATIC
VOID
EFIAPI
UdpIoOnDgramSent (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  )
/*++

Routine Description:

  The callback function when the packet is sent by UDP.
  It will remove the packet from the local list then call
  the packet owner's callback function.
  
Arguments:

  Event   - The event signalled.
  Context - The UDP TX Token.

Returns:

  None

--*/
{
  UDP_TX_TOKEN              *Token;
  EFI_TPL                   OldTpl;

  OldTpl  = NET_RAISE_TPL (NET_TPL_LOCK);

  Token   = (UDP_TX_TOKEN *) Context;
  ASSERT (Token->Signature == UDP_IO_TX_SIGNATURE);

  NetListRemoveEntry (&Token->Link);
  Token->CallBack (Token->Packet, NULL, Token->UdpToken.Status, Token->Context);

  UdpIoFreeTxToken (Token);

  NET_RESTORE_TPL (OldTpl);
}

EFI_STATUS
UdpIoSendDatagram (
  IN  UDP_IO_PORT           *UdpIo,
  IN  NET_BUF               *Packet,
  IN  UDP_POINTS            *EndPoint, OPTIONAL
  IN  IP4_ADDR              Gateway,
  IN  UDP_IO_CALLBACK       CallBack,
  IN  VOID                  *Context
  )
/*++

Routine Description:

  Send a packet through the UDP IO port.

Arguments:

  UdpIo     - The UDP IO Port to send the packet through
  Packet    - The packet to send
  EndPoint  - The local and remote access point
  Gateway   - The gateway to use
  CallBack  - The call back function to call when packet is 
              transmitted or failed.
  Context   - The opque parameter to the CallBack

Returns:

  EFI_OUT_OF_RESOURCES - Failed to allocate resource for the packet
  EFI_SUCCESS          - The packet is successfully delivered to UDP 
                         for transmission.

--*/
{
  UDP_TX_TOKEN              *Token;
  EFI_STATUS                Status;

  Token = UdpIoWrapTx (UdpIo, Packet, EndPoint, Gateway, CallBack, Context);
  
  if (Token == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = UdpIo->Udp->Transmit (UdpIo->Udp, &Token->UdpToken);

  if (EFI_ERROR (Status)) {
    UdpIoFreeTxToken (Token);
    return Status;
  }

  NetListInsertHead (&UdpIo->SentDatagram, &Token->Link);
  return EFI_SUCCESS;
}

STATIC
BOOLEAN
UdpIoCancelSingleDgram (
  IN UDP_TX_TOKEN           *Token,
  IN VOID                   *Context
  )
/*++

Routine Description:

  The selection function to cancel a single sent datagram.

Arguments:

  Token   - The UDP TX token to test againist.
  Context - The context

Returns:

  TRUE if the packet is to be cancelled, otherwise FALSE.

--*/
{
  NET_BUF                   *Packet;

  Packet = (NET_BUF *) Context;

  if (Token->Packet == Packet) {
    return TRUE;
  }

  return FALSE;
}

VOID
UdpIoCancelSentDatagram (
  IN  UDP_IO_PORT           *UdpIo,
  IN  NET_BUF               *Packet
  )
/*++

Routine Description:

  Cancel a single sent datagram.

Arguments:

  UdpIo   - The UDP IO port to cancel the packet from
  Packet  - The packet to cancel

Returns:

  None

--*/
{
  UdpIoCancelDgrams (UdpIo, EFI_ABORTED, UdpIoCancelSingleDgram, Packet);
}

STATIC
VOID
UdpIoRecycleDgram (
  IN VOID                   *Context
  )
/*++

Routine Description:

  Recycle the received UDP data. 

Arguments:

  Context - The UDP_RX_TOKEN

Returns:

  None

--*/
{
  UDP_RX_TOKEN              *Token;

  Token = (UDP_RX_TOKEN *) Context;
  gBS->SignalEvent (Token->UdpToken.Packet.RxData->RecycleSignal);
  UdpIoFreeRxToken (Token);
}

STATIC
VOID
EFIAPI
UdpIoOnDgramRcvd (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  )
/*++

Routine Description:

  The event handle for UDP receive request. It will build
  a NET_BUF from the recieved UDP data, then deliver it 
  to the receiver. 

Arguments:

  Event   - The UDP receive request event
  Context - The UDP RX token.

Returns:

  None

--*/
{
  EFI_UDP4_COMPLETION_TOKEN *UdpToken;
  EFI_UDP4_RECEIVE_DATA     *UdpRxData;
  EFI_UDP4_SESSION_DATA     *UdpSession;
  UDP_RX_TOKEN              *Token;
  UDP_POINTS                Points;
  EFI_TPL                   OldTpl;
  NET_BUF                   *Netbuf;

  OldTpl  = NET_RAISE_TPL (NET_TPL_LOCK);
  Token   = (UDP_RX_TOKEN *) Context;

  ASSERT ((Token->Signature == UDP_IO_RX_SIGNATURE) && 
          (Token == Token->UdpIo->RecvRequest));

  //
  // Clear the receive request first in case that the caller 
  // wants to restart the receive in the callback.
  //
  Token->UdpIo->RecvRequest = NULL;
  
  UdpToken  = &Token->UdpToken;
  UdpRxData = UdpToken->Packet.RxData;

  if (EFI_ERROR (UdpToken->Status) || (UdpRxData == NULL)) {
    Token->CallBack (NULL, NULL, UdpToken->Status, Token->Context);
    UdpIoFreeRxToken (Token);

    goto ON_EXIT;
  }

  //
  // Build a NET_BUF from the UDP receive data, then deliver it up.
  //
  Netbuf = NetbufFromExt (
             (NET_FRAGMENT *) UdpRxData->FragmentTable,
             UdpRxData->FragmentCount,
             0,
             (UINT32) Token->HeadLen,
             UdpIoRecycleDgram,
             Token
             );

  if (Netbuf == NULL) {
    gBS->SignalEvent (UdpRxData->RecycleSignal);
    Token->CallBack (NULL, NULL, EFI_OUT_OF_RESOURCES, Token->Context);

    UdpIoFreeRxToken (Token);
    goto ON_EXIT;
  }

  UdpSession        = &UdpRxData->UdpSession;
  Points.LocalAddr  = EFI_NTOHL (UdpSession->DestinationAddress);
  Points.LocalPort  = UdpSession->DestinationPort;
  Points.RemoteAddr = EFI_NTOHL (UdpSession->SourceAddress);
  Points.RemotePort = UdpSession->SourcePort;

  Token->CallBack (Netbuf, &Points, EFI_SUCCESS, Token->Context);

ON_EXIT:
  NET_RESTORE_TPL (OldTpl);
  return;
}

EFI_STATUS
UdpIoRecvDatagram (
  IN  UDP_IO_PORT           *UdpIo,
  IN  UDP_IO_CALLBACK       CallBack,
  IN  VOID                  *Context,
  IN  UINT32                HeadLen
  )
/*++

Routine Description:

  Issue a receive request to the UDP IO port.

Arguments:

  UdpIo     - The UDP IO port to recieve the packet from.
  CallBack  - The call back function to execute when receive finished.
  Context   - The opque context to the call back
  HeadLen   - The lenght of the application's header

Returns:

  EFI_ALREADY_STARTED  - There is already a pending receive request. Only
                         one receive request is supported.
  EFI_OUT_OF_RESOURCES - Failed to allocate some resource.
  EFI_SUCCESS          - The receive request is issued successfully.

--*/
{
  UDP_RX_TOKEN              *Token;
  EFI_STATUS                Status;

  if (UdpIo->RecvRequest != NULL) {
    return EFI_ALREADY_STARTED;
  }

  Token = UdpIoCreateRxToken (UdpIo, CallBack, Context, HeadLen);

  if (Token == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = UdpIo->Udp->Receive (UdpIo->Udp, &Token->UdpToken);

  if (EFI_ERROR (Status)) {
    UdpIoFreeRxToken (Token);
    return Status;
  }

  UdpIo->RecvRequest = Token;
  return EFI_SUCCESS;
}
