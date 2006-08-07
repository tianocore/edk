/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  SockImpl.c

Abstract:

--*/

#include "SockImpl.h"

STATIC
UINT32
SockTcpDataToRcv (
  IN  SOCK_BUFFER   *SockBuffer,
  OUT BOOLEAN       *IsOOB,
  IN  UINT32        BufLen
  );

STATIC
VOID
SockProcessSndToken (
  IN SOCKET *Sock
  );

VOID
SockFreeFoo (
  IN EFI_EVENT Event
  )
{
  return ;
}

STATIC
UINT32
SockTcpDataToRcv (
  IN  SOCK_BUFFER    *SockBuffer,
  OUT BOOLEAN        *IsUrg,
  IN  UINT32         BufLen
  )
/*++

Routine Description:

   Get the length of the data that can be retrieved from the socket
   receive buffer.

Arguments:

  SockBuffer  - Pointer to the socket receive buffer. 
  IsUrg       - Pointer to a BOOLEAN variable. If TRUE the data is OOB.
  BufLen      - The maximum length of the data buffer to store the
                received data in socket layer.

Returns:

  The length of the data can be retreived.

--*/
{
  NET_BUF       *RcvBufEntry;
  UINT32        DataLen;
  TCP_RSV_DATA  *TcpRsvData;
  BOOLEAN       Urg;

  ASSERT (SockBuffer && IsUrg && (BufLen > 0));

  RcvBufEntry = SockBufFirst (SockBuffer);
  ASSERT (RcvBufEntry);

  TcpRsvData  = (TCP_RSV_DATA *) RcvBufEntry->ProtoData;

  *IsUrg      = ((TcpRsvData->UrgLen > 0) ? TRUE : FALSE);

  if (*IsUrg && TcpRsvData->UrgLen < RcvBufEntry->TotalSize) {

    DataLen = NET_MIN (TcpRsvData->UrgLen, BufLen);

    if (DataLen < TcpRsvData->UrgLen) {
      TcpRsvData->UrgLen = TcpRsvData->UrgLen - DataLen;
    } else {
      TcpRsvData->UrgLen = 0;
    }

    return DataLen;

  }

  DataLen     = RcvBufEntry->TotalSize;

  RcvBufEntry = SockBufNext (SockBuffer, RcvBufEntry);

  while ((BufLen > DataLen) && (RcvBufEntry != NULL)) {

    TcpRsvData  = (TCP_RSV_DATA *) RcvBufEntry->ProtoData;

    Urg         = ((TcpRsvData->UrgLen > 0) ? TRUE : FALSE);

    if (*IsUrg != Urg) {
      break;
    }

    if (*IsUrg && TcpRsvData->UrgLen < RcvBufEntry->TotalSize) {

      if (TcpRsvData->UrgLen + DataLen < BufLen) {
        TcpRsvData->UrgLen = 0;
      } else {
        TcpRsvData->UrgLen = TcpRsvData->UrgLen - (BufLen - DataLen);
      }

      return NET_MIN (TcpRsvData->UrgLen + DataLen, BufLen);

    }

    DataLen += RcvBufEntry->TotalSize;

    RcvBufEntry = SockBufNext (SockBuffer, RcvBufEntry);
  }

  DataLen = NET_MIN (BufLen, DataLen);
  return DataLen;
}

VOID
SockSetTcpRxData (
  IN SOCKET     *Sock,
  IN VOID       *TcpRxData,
  IN UINT32     RcvdBytes,
  IN BOOLEAN    IsOOB
  )
/*++

Routine Description:

  Copy data from socket buffer to application provided receive buffer.

Arguments:

  Sock      - Pointer to the socket.
  TcpRxData - Pointer to the application provided receive buffer.
  RcvdBytes - The maximum length of the data can be copied.
  IsOOB     - If TURE the data is OOB, else the data is normal.

Returns:

  None.

--*/
{
  UINT32                  Index;
  UINT32                  CopyBytes;
  UINT32                  OffSet;
  EFI_TCP4_RECEIVE_DATA   *RxData;
  EFI_TCP4_FRAGMENT_DATA  *Fragment;

  RxData  = (EFI_TCP4_RECEIVE_DATA *) TcpRxData;

  OffSet  = 0;

  ASSERT (RxData->DataLength >= RcvdBytes);

  RxData->DataLength  = RcvdBytes;
  RxData->UrgentFlag  = IsOOB;

  for (Index = 0; (Index < RxData->FragmentCount) && (RcvdBytes > 0); Index++) {

    Fragment  = &RxData->FragmentTable[Index];
    CopyBytes = NET_MIN (Fragment->FragmentLength, RcvdBytes);

    NetbufQueCopy (
      Sock->RcvBuffer.DataQueue,
      OffSet,
      CopyBytes,
      Fragment->FragmentBuffer
      );

    Fragment->FragmentLength = CopyBytes;
    RcvdBytes -= CopyBytes;
    OffSet += CopyBytes;
  }
}

UINT32
SockProcessRcvToken (
  IN SOCKET        *Sock,
  IN SOCK_IO_TOKEN *RcvToken
  )
/*++

Routine Description:

  Get received data from the socket layer to the receive token.

Arguments:

  Sock      - Pointer to the socket.
  RcvToken  - Pointer to the application provided receive token.

Returns:

  The length of data received in this token.

--*/
{
  UINT32                 TokenRcvdBytes;
  EFI_TCP4_RECEIVE_DATA  *RxData;
  BOOLEAN                IsUrg;

  ASSERT (Sock);

  ASSERT (SOCK_STREAM == Sock->Type);

  RxData = RcvToken->Packet.RxData;

  TokenRcvdBytes = SockTcpDataToRcv (
                      &Sock->RcvBuffer,
                      &IsUrg,
                      RxData->DataLength
                      );

  //
  // Copy data from RcvBuffer of socket to user
  // provided RxData and set the fields in TCP RxData
  //
  SockSetTcpRxData (Sock, RxData, TokenRcvdBytes, IsUrg);

  SOCK_TRIM_RCV_BUFF (Sock, TokenRcvdBytes);
  SIGNAL_TOKEN (&(RcvToken->Token), EFI_SUCCESS);

  return TokenRcvdBytes;
}

EFI_STATUS
SockProcessTcpSndData (
  IN SOCKET   *Sock,
  IN VOID     *TcpTxData
  )
/*++

Routine Description:

  Process the TCP send data, buffer the tcp txdata and append
  the buffer to socket send buffer,then try to send it.

Arguments:

  Sock      - Pointer to the socket.
  TcpTxData - Pointer to the tcp txdata.

Returns:

  EFI_SUCCESS          - The operation is completed successfully.
  EFI_OUT_OF_RESOURCES - Failed due to resource limit.

--*/
{
  NET_BUF                 *SndData;
  EFI_STATUS              Status;
  EFI_TCP4_TRANSMIT_DATA  *TxData;

  TxData = (EFI_TCP4_TRANSMIT_DATA *) TcpTxData;

  //
  // transform this TxData into a NET_BUFFER
  // and insert it into Sock->SndBuffer
  //
  SndData = NetbufFromExt (
              (NET_FRAGMENT *) TxData->FragmentTable,
              TxData->FragmentCount,
              0,
              0,
              SockFreeFoo,
              NULL
              );

  if (NULL == SndData) {
    SOCK_DEBUG_ERROR (("SockKProcessSndData: Failed to"
      " call NetBufferFromExt\n"));

    return EFI_OUT_OF_RESOURCES;
  }

  NetbufQueAppend (Sock->SndBuffer.DataQueue, SndData);

  //
  // notify the low layer protocol to handle this send token
  //
  if (TxData->Urgent) {
    Status = Sock->ProtoHandler (Sock, SOCK_SNDURG, NULL);

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (TxData->Push) {
    Status = Sock->ProtoHandler (Sock, SOCK_SNDPUSH, NULL);

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // low layer protocol should really handle the sending
  // process when catching SOCK_SND request
  //
  Status = Sock->ProtoHandler (Sock, SOCK_SND, NULL);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

STATIC
VOID
SockFlushPendingToken (
  IN SOCKET         *Sock,
  IN NET_LIST_ENTRY *PendingTokenList
  )
/*++

Routine Description:

  Flush the tokens in the specific token list.

Arguments:

  Sock              - Pointer to the socket.
  PendingTokenList  - Pointer to the token list to be flushed.

Returns:

  None.

--*/
{
  SOCK_TOKEN            *SockToken;
  SOCK_COMPLETION_TOKEN *Token;

  ASSERT (Sock && PendingTokenList);

  while (!NetListIsEmpty (PendingTokenList)) {
    SockToken = NET_LIST_HEAD (
                  PendingTokenList,
                  SOCK_TOKEN,
                  TokenList
                  );

    Token = SockToken->Token;
    SIGNAL_TOKEN (Token, Sock->SockError);

    NetListRemoveEntry (&(SockToken->TokenList));
    NetFreePool (SockToken);
  }
}

STATIC
VOID
SockWakeConnToken (
  IN SOCKET *Sock
  )
/*++

Routine Description:

  Wake up the connection token while the connection is
  successfully established, then try to process any
  pending send token.

Arguments:

  Sock  - Pointer to the socket.

Returns:

  None.

--*/
{
  ASSERT (Sock->ConnectionToken != NULL);

  SIGNAL_TOKEN (Sock->ConnectionToken, EFI_SUCCESS);
  Sock->ConnectionToken = NULL;

  //
  // check to see if some pending send token existed?
  //
  SockProcessSndToken (Sock);
  return ;
}

STATIC
VOID
SockWakeListenToken (
  IN SOCKET *Sock
  )
/*++

Routine Description:

  Wake up the listen token while the connection is
  established successfully.

Arguments:

  Sock  - Pointer to the socket.

Returns:

  None.

--*/
{
  SOCKET                *Parent;
  SOCK_TOKEN            *SockToken;
  EFI_TCP4_LISTEN_TOKEN *ListenToken;

  Parent = Sock->Parent;

  ASSERT (Parent && SOCK_IS_LISTENING (Parent) && SOCK_IS_CONNECTED (Sock));

  if (!NetListIsEmpty (&Parent->ListenTokenList)) {
    SockToken = NET_LIST_HEAD (
                  &Parent->ListenTokenList,
                  SOCK_TOKEN,
                  TokenList
                  );

    ListenToken = (EFI_TCP4_LISTEN_TOKEN *) SockToken->Token;
    ListenToken->NewChildHandle = Sock->SockHandle;

    SIGNAL_TOKEN (&(ListenToken->CompletionToken), EFI_SUCCESS);

    NetListRemoveEntry (&SockToken->TokenList);
    NetFreePool (SockToken);

    NetListRemoveEntry (&Sock->ConnectionList);

    Parent->ConnCnt--;
    SOCK_DEBUG_WARN (("SockWakeListenToken: accept a socket,"
      "now conncnt is %d", Parent->ConnCnt));

    Sock->Parent = NULL;
  }
}

STATIC
VOID
SockWakeRcvToken (
  IN SOCKET *Sock
  )
/*++

Routine Description:

  Wake up the receive token while some data is received.

Arguments:

  Sock  - Pointer to the socket.

Returns:

  None.

--*/
{
  UINT32        RcvdBytes;
  UINT32        TokenRcvdBytes;
  SOCK_TOKEN    *SockToken;
  SOCK_IO_TOKEN *RcvToken;

  ASSERT (Sock->RcvBuffer.DataQueue);

  RcvdBytes = (Sock->RcvBuffer.DataQueue)->BufSize;

  ASSERT (RcvdBytes > 0);

  while (RcvdBytes > 0 && !NetListIsEmpty (&Sock->RcvTokenList)) {

    SockToken = NET_LIST_HEAD (
                  &Sock->RcvTokenList,
                  SOCK_TOKEN,
                  TokenList
                  );

    RcvToken        = (SOCK_IO_TOKEN *) SockToken->Token;
    TokenRcvdBytes  = SockProcessRcvToken (Sock, RcvToken);

    if (0 == TokenRcvdBytes) {
      return ;
    }

    NetListRemoveEntry (&(SockToken->TokenList));
    NetFreePool (SockToken);
    RcvdBytes -= TokenRcvdBytes;
  }
}

STATIC
VOID
SockProcessSndToken (
  IN SOCKET *Sock
  )
/*++

Routine Description:

  Process the send token.

Arguments:

  Sock  - Pointer to the socket.

Returns:

  None.

--*/
{
  UINT32                  FreeSpace;
  SOCK_TOKEN              *SockToken;
  UINT32                  DataLen;
  SOCK_IO_TOKEN           *SndToken;
  EFI_TCP4_TRANSMIT_DATA  *TxData;
  EFI_STATUS              Status;

  ASSERT (Sock && (SOCK_STREAM == Sock->Type));

  FreeSpace = SockGetFreeSpace (Sock, SOCK_SND_BUF);

  //
  // to determine if process a send token using
  // socket layer flow control policy
  //
  while ((FreeSpace >= Sock->SndBuffer.LowWater) &&
         !NetListIsEmpty (&Sock->SndTokenList)) {

    SockToken = NET_LIST_HEAD (
                  &(Sock->SndTokenList),
                  SOCK_TOKEN,
                  TokenList
                  );

    //
    // process this token
    //
    NetListRemoveEntry (&(SockToken->TokenList));
    NetListInsertTail (
      &(Sock->ProcessingSndTokenList),
      &(SockToken->TokenList)
      );

    //
    // Proceess it in the light of  SockType
    //
    SndToken  = (SOCK_IO_TOKEN *) SockToken->Token;
    TxData    = SndToken->Packet.TxData;

    DataLen = TxData->DataLength;
    Status  = SockProcessTcpSndData (Sock, TxData);

    if (EFI_ERROR (Status)) {
      goto OnError;
    }

    if (DataLen >= FreeSpace) {
      FreeSpace = 0;

    } else {
      FreeSpace -= DataLen;

    }
  }

  return ;

OnError:

  NetListRemoveEntry (&SockToken->TokenList);
  SIGNAL_TOKEN (SockToken->Token, Status);
  NetFreePool (SockToken);
}

SOCKET *
SockCreate (
  IN SOCK_INIT_DATA *SockInitData
  )
/*++

Routine Description:

  Create a socket with initial data SockInitData.

Arguments:

  SockInitData  - Pointer to the initial data of the socket.

Returns:

  Pointer to the newly created socket.

--*/
{
  SOCKET      *Sock;
  SOCKET      *Parent;
  EFI_STATUS  Status;

  ASSERT (SockInitData && SockInitData->ProtoHandler);
  ASSERT (SockInitData->Type == SOCK_STREAM);

  Parent = SockInitData->Parent;

  if (Parent && (Parent->ConnCnt == Parent->BackLog)) {
    SOCK_DEBUG_ERROR (
      ("SockCreate: Socket parent has "
      "reached its connection limit with %d ConnCnt and %d BackLog\n",
      Parent->ConnCnt,
      Parent->BackLog)
      );

    return NULL;
  }

  Sock = NetAllocateZeroPool (sizeof (SOCKET));
  if (NULL == Sock) {

    SOCK_DEBUG_ERROR (("SockCreate: No resource to create a new socket\n"));
    return NULL;
  }

  NetListInit (&Sock->ConnectionList);
  NetListInit (&Sock->ListenTokenList);
  NetListInit (&Sock->RcvTokenList);
  NetListInit (&Sock->SndTokenList);
  NetListInit (&Sock->ProcessingSndTokenList);

  NET_LOCK_INIT (&(Sock->Lock));

  Sock->SndBuffer.DataQueue = NetbufQueAlloc ();
  if (NULL == Sock->SndBuffer.DataQueue) {
    SOCK_DEBUG_ERROR (("SockCreate: No resource to allocate"
      " SndBuffer for new socket\n"));

    goto OnError;
  }

  Sock->RcvBuffer.DataQueue = NetbufQueAlloc ();
  if (NULL == Sock->RcvBuffer.DataQueue) {
    SOCK_DEBUG_ERROR (("SockCreate: No resource to allocate "
      "RcvBuffer for new socket\n"));

    goto OnError;
  }

  Sock->Signature           = SOCK_SIGNATURE;

  Sock->Parent              = Parent;
  Sock->BackLog             = SockInitData->BackLog;
  Sock->ProtoHandler        = SockInitData->ProtoHandler;
  Sock->SndBuffer.HighWater = SockInitData->SndBufferSize;
  Sock->RcvBuffer.HighWater = SockInitData->RcvBufferSize;
  Sock->Type                = SockInitData->Type;
  Sock->DriverBinding       = SockInitData->DriverBinding;
  Sock->State               = SockInitData->State;

  Sock->SockError           = EFI_ABORTED;
  Sock->SndBuffer.LowWater  = SOCK_BUFF_LOW_WATER;
  Sock->RcvBuffer.LowWater  = SOCK_BUFF_LOW_WATER;

  //
  // Install protocol on Sock->SockHandle
  //
  NetCopyMem (
    &(Sock->NetProtocol.TcpProtocol),
    SockInitData->Protocol,
    sizeof (EFI_TCP4_PROTOCOL)
    );

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Sock->SockHandle,
                  &gEfiTcp4ProtocolGuid,
                  &(Sock->NetProtocol.TcpProtocol),
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    SOCK_DEBUG_ERROR (("SockCreate: Install TCP protocol in "
      "socket failed with %r\n", Status));

    goto OnError;
  }

  if (Parent != NULL) {
    ASSERT (Parent->BackLog > 0);
    ASSERT (SOCK_IS_LISTENING (Parent));

    //
    // need to add it into Parent->ConnectionList
    // if the Parent->ConnCnt < Parent->BackLog
    //
    Parent->ConnCnt++;

    SOCK_DEBUG_WARN (("SockCreate: Create a new socket and"
      "add to parent, now conncnt is %d\n", Parent->ConnCnt));

    NetListInsertTail (&Parent->ConnectionList, &Sock->ConnectionList);
  }

  return Sock;

OnError:
  if (NULL != Sock) {

    if (NULL != Sock->SndBuffer.DataQueue) {
      NetbufQueFree (Sock->SndBuffer.DataQueue);
    }

    if (NULL != Sock->RcvBuffer.DataQueue) {
      NetbufQueFree (Sock->RcvBuffer.DataQueue);
    }

    NetFreePool (Sock);
  }

  return NULL;
}

VOID
SockDestroy (
  IN SOCKET *Sock
  )
/*++

Routine Description:

  Destroy a socket.

Arguments:

  Sock  - Pointer to the socket.

Returns:

  None.

--*/
{
  VOID        *SockProtocol;
  EFI_GUID    *ProtocolGuid;
  EFI_STATUS  Status;

  ASSERT (SOCK_STREAM == Sock->Type);

  //
  // Flush the completion token buffered
  // by sock and rcv, snd buffer
  //
  if (!SOCK_IS_UNCONFIGURED (Sock)) {

    SockConnFlush (Sock);
    SockSetState (Sock, SO_CLOSED);
    Sock->ConfigureState = SO_UNCONFIGURED;

  }
  //
  // Destory the RcvBuffer Queue and SendBuffer Queue
  //
  NetbufQueFree (Sock->RcvBuffer.DataQueue);
  NetbufQueFree (Sock->SndBuffer.DataQueue);

  //
  // Remove it from parent connection list if needed
  //
  if (Sock->Parent) {

    NetListRemoveEntry (&(Sock->ConnectionList));
    (Sock->Parent->ConnCnt)--;

    SOCK_DEBUG_WARN (("SockDestory: Delete a unaccepted socket from parent"
      "now conncnt is %d\n", Sock->Parent->ConnCnt));

    Sock->Parent = NULL;
  }

  //
  // Set the protocol guid and driver binding handle
  // in the light of Sock->SockType
  //
  ProtocolGuid = &gEfiTcp4ProtocolGuid;

  //
  // Retrieve the protocol installed on this sock
  //
  Status = gBS->OpenProtocol (
                  Sock->SockHandle,
                  ProtocolGuid,
                  &SockProtocol,
                  Sock->DriverBinding,
                  Sock->SockHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockDestroy: Open protocol installed "
      "on socket failed with %r\n", Status));

    goto FreeSock;
  }

  //
  // Uninstall the protocol installed on this sock
  // in the light of Sock->SockType
  //
  gBS->UninstallMultipleProtocolInterfaces (
        Sock->SockHandle,
        ProtocolGuid,
        SockProtocol,
        NULL
        );

FreeSock:
  NetFreePool (Sock);
  return ;
}

VOID
SockConnFlush (
  IN SOCKET *Sock
  )
/*++

Routine Description:

  Flush the socket.

Arguments:

  Sock  - Pointer to the socket.

Returns:

  None.

--*/
{
  SOCKET  *Child;

  ASSERT (Sock);

  //
  // Clear the flag in this socket
  //
  Sock->Flag = 0;

  //
  // Flush the SndBuffer and RcvBuffer of Sock
  //
  NetbufQueFlush (Sock->SndBuffer.DataQueue);
  NetbufQueFlush (Sock->RcvBuffer.DataQueue);

  //
  // Signal the pending token
  //
  if (Sock->ConnectionToken != NULL) {
    SIGNAL_TOKEN (Sock->ConnectionToken, Sock->SockError);
    Sock->ConnectionToken = NULL;
  }

  if (Sock->CloseToken != NULL) {
    SIGNAL_TOKEN (Sock->CloseToken, Sock->SockError);
    Sock->CloseToken = NULL;
  }

  SockFlushPendingToken (Sock, &(Sock->ListenTokenList));
  SockFlushPendingToken (Sock, &(Sock->RcvTokenList));
  SockFlushPendingToken (Sock, &(Sock->SndTokenList));
  SockFlushPendingToken (Sock, &(Sock->ProcessingSndTokenList));

  //
  // Destroy the pending connection, if it is a listening socket
  //
  if (SOCK_IS_LISTENING (Sock)) {
    while (!NetListIsEmpty (&Sock->ConnectionList)) {
      Child = NET_LIST_HEAD (
                &Sock->ConnectionList,
                SOCKET,
                ConnectionList
                );

      SockDestroyChild (Child);
    }

    Sock->ConnCnt = 0;
  }

  return ;
}

VOID
SockSetState (
  IN SOCKET     *Sock,
  IN SOCK_STATE State
  )
/*++

Routine Description:

  Set the state of the socket.

Arguments:

  Sock  - Pointer to the socket.
  State - The new state to be set.

Returns:

  None.

--*/
{
  Sock->State = State;
}

SOCKET *
SockClone (
  IN SOCKET *Sock
  )
/*++

Routine Description:

  Clone a new socket including its associated protocol control block.

Arguments:

  Sock  - Pointer to the socket to be cloned.

Returns:

  SOCKET * - Pointer to the newly cloned socket.
             If NULL, error condition occurred.

--*/
{
  SOCKET          *ClonedSock;
  SOCK_INIT_DATA  InitData;

  InitData.BackLog        = Sock->BackLog;
  InitData.Parent         = Sock;
  InitData.State          = Sock->State;
  InitData.ProtoHandler   = Sock->ProtoHandler;
  InitData.Type           = Sock->Type;
  InitData.RcvBufferSize  = Sock->RcvBuffer.HighWater;
  InitData.SndBufferSize  = Sock->SndBuffer.HighWater;
  InitData.DriverBinding  = Sock->DriverBinding;
  InitData.Protocol       = &(Sock->NetProtocol);

  ClonedSock              = SockCreate (&InitData);

  if (NULL == ClonedSock) {
    SOCK_DEBUG_ERROR (("SockClone: no resource to create a cloned sock\n"));
    return NULL;
  }

  NetCopyMem (
    ClonedSock->ProtoReserved,
    Sock->ProtoReserved,
    PROTO_RESERVED_LEN
    );

  SockSetState (ClonedSock, SO_CONNECTING);
  ClonedSock->ConfigureState = Sock->ConfigureState;

  return ClonedSock;
}

VOID
SockConnEstablished (
  IN SOCKET *Sock
  )
/*++

Routine Description:

  Called by the low layer protocol to indicate the socket
  a connection is established. This function just changes
  the socket's state to SO_CONNECTED and signals the token
  used for connection establishment.

Arguments:

  Sock  - Pointer to the socket associated with the established
          connection.

Returns:

  None.

--*/
{

  ASSERT (SO_CONNECTING == Sock->State);

  SockSetState (Sock, SO_CONNECTED);

  if (NULL == Sock->Parent) {
    SockWakeConnToken (Sock);
  } else {
    SockWakeListenToken (Sock);
  }

  return ;
}

VOID
SockConnClosed (
  IN SOCKET *Sock
  )
/*++

Routine Description:

  Called by the low layer protocol to indicate the connection
  is closed. This function flushes the socket, sets the state 
  to SO_CLOSED and signals the close token.

Arguments:

  Sock  - Pointer to the socket associated with the closed
          connection.

Returns:

  None.

--*/
{
  if (Sock->CloseToken) {
    SIGNAL_TOKEN (Sock->CloseToken, EFI_SUCCESS);
    Sock->CloseToken = NULL;
  }

  SockConnFlush (Sock);
  SockSetState (Sock, SO_CLOSED);

  if (Sock->Parent != NULL) {
    SockDestroyChild (Sock);
  }

}

VOID
SockDataSent (
  IN SOCKET     *Sock,
  IN UINT32     Count
  )
/*++

Routine Description:

  Called by low layer protocol to indicate that some
  data is sent or processed. This function trims the
  sent data in the socket send buffer, signals the
  data token if proper

Arguments:

  Sock  - Pointer to the socket.
  Count - The length of the data processed or sent, in bytes.

Returns:

  None.

--*/
{
  SOCK_TOKEN            *SockToken;
  SOCK_COMPLETION_TOKEN *SndToken;

  ASSERT (!NetListIsEmpty (&Sock->ProcessingSndTokenList));
  ASSERT (Count <= (Sock->SndBuffer.DataQueue)->BufSize);

  NetbufQueTrim (Sock->SndBuffer.DataQueue, Count);

  //
  // To check if we can signal some snd token in this socket
  //
  while (Count > 0) {
    SockToken = NET_LIST_HEAD (
                  &(Sock->ProcessingSndTokenList),
                  SOCK_TOKEN,
                  TokenList
                  );

    SndToken = SockToken->Token;

    if (SockToken->RemainDataLen <= Count) {

      NetListRemoveEntry (&(SockToken->TokenList));
      SIGNAL_TOKEN (SndToken, EFI_SUCCESS);
      Count -= SockToken->RemainDataLen;
      NetFreePool (SockToken);
    } else {

      SockToken->RemainDataLen -= Count;
      Count = 0;
    }
  }

  //
  // to judge if we can process some send token in
  // Sock->SndTokenList, if so process those send token
  //
  SockProcessSndToken (Sock);
  return ;
}

UINT32
SockGetDataToSend (
  IN SOCKET      *Sock,
  IN UINT32      Offset,
  IN UINT32      Len,
  IN UINT8       *Dest
  )
/*++

Routine Description:

  Called by the low layer protocol to copy some data in socket send
  buffer starting from the specific offset to a buffer provided by
  the caller.

Arguments:

  Sock    - Pointer to the socket.
  Offset  - The start point of the data to be copied.
  Len     - The length of the data to be copied.
  Dest    - Pointer to the destination to copy the data.

Returns:

  The data size copied.

--*/
{
  ASSERT (Sock && SOCK_STREAM == Sock->Type);

  return NetbufQueCopy (
          Sock->SndBuffer.DataQueue,
          Offset,
          Len,
          Dest
          );
}

VOID
SockDataRcvd (
  IN SOCKET    *Sock,
  IN NET_BUF   *NetBuffer,
  IN UINT32    UrgLen
  )
/*++

Routine Description:

  Called by the low layer protocol to deliver received data
  to socket layer. This function will append the data to the
  socket receive buffer, set ther urgent data length and then
  check if any receive token can be signaled.

Arguments:

  Sock      - Pointer to the socket.
  NetBuffer - Pointer to the buffer that contains the received data.
  UrgLen    - The length of the urgent data in the received data.

Returns:

  None.

--*/
{
  ASSERT (Sock && Sock->RcvBuffer.DataQueue &&
    UrgLen <= NetBuffer->TotalSize);

  NET_GET_REF (NetBuffer);

  ((TCP_RSV_DATA *) (NetBuffer->ProtoData))->UrgLen = UrgLen;

  NetbufQueAppend (Sock->RcvBuffer.DataQueue, NetBuffer);

  SockWakeRcvToken (Sock);
  return ;
}

UINT32
SockGetFreeSpace (
  IN SOCKET  *Sock,
  IN UINT32  Which
  )
/*++

Routine Description:

  Get the length of the free space of the specific socket buffer.

Arguments:

  Sock  - Pointer to the socket.
  Which - Flag to indicate which socket buffer to check, either
          send buffer or receive buffer.

Returns:

  The length of the free space, in bytes.

--*/
{
  UINT32      BufferCC;
  SOCK_BUFFER *SockBuffer;

  ASSERT (Sock && ((SOCK_SND_BUF == Which) || (SOCK_RCV_BUF == Which)));

  if (SOCK_SND_BUF == Which) {
    SockBuffer = &(Sock->SndBuffer);
  } else {
    SockBuffer = &(Sock->RcvBuffer);
  }

  BufferCC = (SockBuffer->DataQueue)->BufSize;

  if (BufferCC >= SockBuffer->HighWater) {

    return 0;
  }

  return SockBuffer->HighWater - BufferCC;
}

VOID
SockRcvdErr (
  IN SOCKET       *Sock,
  IN EFI_STATUS   Error
  )
/*++

Routine Description:

  Signal the receive token with the specific error or
  set socket error code after error is received.

Arguments:

  Sock  - Pointer to the socket.
  Error - The error code received.

Returns:

  None.

--*/
{
  SOCK_TOKEN  *SockToken;

  if (!NetListIsEmpty (&Sock->RcvTokenList)) {

    SockToken = NET_LIST_HEAD (
                  &Sock->RcvTokenList,
                  SOCK_TOKEN,
                  TokenList
                  );

    NetListRemoveEntry (&SockToken->TokenList);

    SIGNAL_TOKEN (SockToken->Token, Error);

    NetFreePool (SockToken);
  } else {

    SOCK_ERROR (Sock, Error);
  }
}

VOID
SockNoMoreData (
  IN SOCKET *Sock
  )
/*++

Routine Description:

  Called by the low layer protocol to indicate that there
  will be no more data from the communication peer. This
  function set the socket's state to SO_NO_MORE_DATA and
  signal all queued IO tokens with the error status
  EFI_CONNECTION_FIN.

Arguments:

  Sock  - Pointer to the socket.

Returns:

  None.

--*/
{
  EFI_STATUS  Err;

  SOCK_NO_MORE_DATA (Sock);

  if (!NetListIsEmpty (&Sock->RcvTokenList)) {

    ASSERT (0 == GET_RCV_DATASIZE (Sock));

    Err = Sock->SockError;

    SOCK_ERROR (Sock, EFI_CONNECTION_FIN);

    SockFlushPendingToken (Sock, &Sock->RcvTokenList);

    SOCK_ERROR (Sock, Err);

  }

}

NET_BUF *
SockBufFirst (
  IN SOCK_BUFFER *Sockbuf
  )
/*++

Routine Description:

  Get the first buffer block in the specific socket buffer.

Arguments:

  Sockbuf - Pointer to the socket buffer.

Returns:

  Pointer to the first buffer in the queue. NULL if the queue is empty.

--*/
{
  NET_LIST_ENTRY  *NetbufList;

  NetbufList = &(Sockbuf->DataQueue->BufList);

  if (NetListIsEmpty (NetbufList)) {
    return NULL;
  }

  return NET_LIST_HEAD (NetbufList, NET_BUF, List);
}

NET_BUF *
SockBufNext (
  IN SOCK_BUFFER *Sockbuf,
  IN NET_BUF     *SockEntry
  )
/*++

Routine Description:

  Get the next buffer block in the specific socket buffer.

Arguments:

  Sockbuf   - Pointer to the socket buffer.
  SockEntry - Pointer to the buffer block prior to the
              required one.

Returns:

  Pointer to the buffer block next to SockEntry. NULL if SockEntry is the tail or head entry.

--*/
{
  NET_LIST_ENTRY  *NetbufList;

  NetbufList = &(Sockbuf->DataQueue->BufList);

  if ((SockEntry->List.ForwardLink == NetbufList) ||
      (SockEntry->List.BackLink == &SockEntry->List) ||
      (SockEntry->List.ForwardLink == &SockEntry->List)
      ) {

    return NULL;
  }

  return NET_LIST_USER_STRUCT (SockEntry->List.ForwardLink, NET_BUF, List);
}
