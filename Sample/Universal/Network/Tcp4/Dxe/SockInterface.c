/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  SockInterface.c

Abstract:

--*/

#include "SockImpl.h"

STATIC
BOOLEAN
SockTokenExistedInList (
  IN NET_LIST_ENTRY *List,
  IN EFI_EVENT      Event
  )
/*++

Routine Description:

  Check whether the Event is in the List.

Arguments:

  List  - Pointer to the token list to be searched.
  Event - The event to be checked.

Returns:

  BOOLEAN - If TRUE, the specific Event exists in the List.
            If FALSE, the specific Event is not in the List.

--*/
{
  NET_LIST_ENTRY  *ListEntry;
  SOCK_TOKEN      *SockToken;

  NET_LIST_FOR_EACH (ListEntry, List) {
    SockToken = NET_LIST_USER_STRUCT (
                  ListEntry,
                  SOCK_TOKEN,
                  TokenList
                  );

    if (Event == SockToken->Token->Event) {
      return TRUE;
    }
  }

  return FALSE;
}

BOOLEAN
SockTokenExisted (
  IN SOCKET    *Sock,
  IN EFI_EVENT Event
  )
/*++

Routine Description:

  Call SockTokenExistedInList() to check whether the Event is
  in the related socket's lists.

Arguments:

  Sock  - Pointer to the instance's socket.
  Event - The event to be checked.

Returns:

  The specific Event exists in one of socket's lists or not.

--*/
{

  if (SockTokenExistedInList (&Sock->SndTokenList, Event) ||
      SockTokenExistedInList (&Sock->ProcessingSndTokenList, Event) ||
      SockTokenExistedInList (&Sock->RcvTokenList, Event) ||
      SockTokenExistedInList (&Sock->ListenTokenList, Event)
        ) {

    return TRUE;
  }

  if ((Sock->ConnectionToken != NULL) &&
      (Sock->ConnectionToken->Event == Event)) {

    return TRUE;
  }

  if ((Sock->CloseToken != NULL) && (Sock->CloseToken->Event == Event)) {
    return TRUE;
  }

  return FALSE;
}

SOCK_TOKEN *
SockBufferToken (
  IN SOCKET         *Sock,
  IN NET_LIST_ENTRY *List,
  IN VOID           *Token,
  IN UINT32         DataLen
  )
/*++

Routine Description:

  Buffer a token into the specific list of socket Sock.

Arguments:

  Sock    - Pointer to the instance's socket.
  List    - Pointer to the list to store the token.
  Token   - Pointer to the token to be buffered.
  DataLen - The data length of the buffer contained in Token.

Returns:

  Pointer to the token that wraps Token. If NULL, error condition occurred.

--*/
{
  SOCK_TOKEN  *SockToken;

  SockToken = NetAllocatePool (sizeof (SOCK_TOKEN));
  if (NULL == SockToken) {

    SOCK_DEBUG_ERROR (("SockBufferIOToken: No Memory "
      "to allocate SockToken\n"));

    return NULL;
  }

  SockToken->Sock           = Sock;
  SockToken->Token          = (SOCK_COMPLETION_TOKEN *) Token;
  SockToken->RemainDataLen  = DataLen;
  NetListInsertTail (List, &SockToken->TokenList);

  return SockToken;
}

EFI_STATUS
SockDestroyChild (
  IN   SOCKET *Sock
  )
/*++

Routine Description:

  Destory the socket Sock and its associated protocol control block.

Arguments:

  Sock  - The socket to be destroyed.

Returns:

  EFI_SUCCESS       - The socket Sock is destroyed successfully.
  EFI_ACCESS_DENIED - Failed to get the lock to access the socket.

--*/
{
  EFI_STATUS  Status;

  ASSERT (Sock && Sock->ProtoHandler);

  if (Sock->IsDestroyed) {
    return EFI_SUCCESS;
  }

  Sock->IsDestroyed = TRUE;

  Status = NET_TRYLOCK (&(Sock->Lock));
  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockDestroyChild: Get the lock to "
      "access socket failed with %r\n", Status));

    return EFI_ACCESS_DENIED;
  }

  //
  // force protocol layer to detach the PCB
  //
  Status = Sock->ProtoHandler (Sock, SOCK_DETACH, NULL);

  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockDestroyChild: Protocol detach socket"
      " failed with %r\n", Status));

    Sock->IsDestroyed = FALSE;
  } else if (SOCK_IS_CONFIGURED (Sock)) {

    SockConnFlush (Sock);
    SockSetState (Sock, SO_CLOSED);

    Sock->ConfigureState = SO_UNCONFIGURED;
  }

  NET_UNLOCK (&(Sock->Lock));

  if (EFI_ERROR (Status)) {
    return Status;
  }

  SockDestroy (Sock);
  return EFI_SUCCESS;
}

SOCKET *
SockCreateChild (
  IN SOCK_INIT_DATA *SockInitData,
  IN VOID           *ProtoData,
  IN UINT32         Len
  )
/*++

Routine Description:

  Create a socket and its associated protocol control block
  with the intial data SockInitData and protocol specific
  data ProtoData.

Arguments:

  SockInitData  - Inital data to setting the socket.
  ProtoData     - Pointer to the protocol specific data.
  Len           - Length of the protocol specific data.

Returns:

  Pointer to the newly created socket. If NULL, error condition occured.

--*/
{
  SOCKET      *Sock;
  EFI_STATUS  Status;

  ASSERT (ProtoData && (Len <= PROTO_RESERVED_LEN));

  //
  // create a new socket
  //
  Sock = SockCreate (SockInitData);
  if (NULL == Sock) {

    SOCK_DEBUG_ERROR (("SockCreateChild: No resource to "
      "create a new socket\n"));

    return NULL;
  }

  //
  // Open the 
  //

  //
  // copy the protodata into socket
  //
  NetCopyMem (Sock->ProtoReserved, ProtoData, Len);

  Status = NET_TRYLOCK (&(Sock->Lock));
  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockCreateChild: Get the lock to "
      "access socket failed with %r\n", Status));

    SockDestroy (Sock);
    return NULL;
  }
  //
  // inform the protocol layer to attach the socket
  // with a new protocol control block
  //
  Status = Sock->ProtoHandler (Sock, SOCK_ATTACH, NULL);
  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockCreateChild: Protocol failed to"
      " attach a socket with %r\n", Status));

    SockDestroy (Sock);
    Sock = NULL;
  }

  NET_UNLOCK (&(Sock->Lock));
  return Sock;
}

EFI_STATUS
SockConfigure (
  IN SOCKET *Sock,
  IN VOID   *ConfigData
  )
/*++

Routine Description:

  Configure the specific socket Sock using configuration data
  ConfigData.

Arguments:

  Sock        - Pointer to the socket to be configured.
  ConfigData  - Pointer to the configuration data.

Returns:

  EFI_SUCCESS       - The socket is configured successfully.
  EFI_ACCESS_DENIED - Failed to get the lock to access the socket
                      or the socket is already configured.

--*/
{
  EFI_STATUS  Status;

  Status = NET_TRYLOCK (&(Sock->Lock));
  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockConfigure: Get the access for "
      "socket failed with %r", Status));

    return EFI_ACCESS_DENIED;
  }

  if (SOCK_IS_CONFIGURED (Sock)) {
    Status = EFI_ACCESS_DENIED;
    goto OnExit;
  }

  ASSERT (Sock->State == SO_CLOSED);

  Status = Sock->ProtoHandler (Sock, SOCK_CONFIGURE, ConfigData);

OnExit:
  NET_UNLOCK (&(Sock->Lock));

  return Status;
}

EFI_STATUS
SockConnect (
  IN SOCKET *Sock,
  IN VOID   *Token
  )
/*++

Routine Description:

  Initiate a connection establishment process.

Arguments:

  Sock  - Pointer to the socket to initiate the initate the connection.
  Token - Pointer to the token used for the connection operation.

Returns:

  EFI_SUCCESS       - The connection is initialized successfully.
  EFI_ACCESS_DENIED - Failed to get the lock to access the socket, or
                      the socket is closed, or the socket is not configured
                      to be an active one, or the token is already in
                      one of this socket's lists.
  EFI_NO_MAPPING    - The IP address configuration operation is not finished.
  EFI_NOT_STARTED   - The socket is not configured.

--*/
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  Status = NET_TRYLOCK (&(Sock->Lock));
  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockConnect: Get the access for "
      "socket failed with %r", Status));

    return EFI_ACCESS_DENIED;
  }

  if (SOCK_IS_NO_MAPPING (Sock)) {
    Status = EFI_NO_MAPPING;
    goto OnExit;
  }

  if (SOCK_IS_UNCONFIGURED (Sock)) {

    Status = EFI_NOT_STARTED;
    goto OnExit;
  }

  if (!SOCK_IS_CLOSED (Sock) || !SOCK_IS_CONFIGURED_ACTIVE (Sock)) {

    Status = EFI_ACCESS_DENIED;
    goto OnExit;
  }

  Event = ((SOCK_COMPLETION_TOKEN *) Token)->Event;

  if (SockTokenExisted (Sock, Event)) {

    Status = EFI_ACCESS_DENIED;
    goto OnExit;
  }

  Sock->ConnectionToken = (SOCK_COMPLETION_TOKEN *) Token;
  SockSetState (Sock, SO_CONNECTING);
  Status = Sock->ProtoHandler (Sock, SOCK_CONNECT, NULL);

OnExit:
  NET_UNLOCK (&(Sock->Lock));
  return Status;
}

EFI_STATUS
SockAccept (
  IN SOCKET *Sock,
  IN VOID   *Token
  )
/*++

Routine Description:

  Issue a listen token to get an existed connected network instance
  or wait for a connection if there is none.

Arguments:

  Sock  - Pointer to the socket to accept connections.
  Token - The token to accept a connection.

Returns:

  EFI_SUCCESS       - Either a connection is accpeted or the Token is
                      buffered for further acception.
  EFI_ACCESS_DENIED - Failed to get the lock to access the socket, or
                      the socket is closed, or the socket is not configured
                      to be a passive one, or the token is already in
                      one of this socket's lists.
  EFI_NO_MAPPING    - The IP address configuration operation is not finished.
  EFI_NOT_STARTED   - The socket is not configured.
  EFI_OUT_OF_RESOURCE - Failed to buffer the Token due to memory limit.

--*/
{
  EFI_TCP4_LISTEN_TOKEN *ListenToken;
  NET_LIST_ENTRY        *ListEntry;
  EFI_STATUS            Status;
  SOCKET                *Socket;
  EFI_EVENT             Event;

  ASSERT (SOCK_STREAM == Sock->Type);

  Status = NET_TRYLOCK (&(Sock->Lock));
  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockAccept: Get the access for socket"
      " failed with %r", Status));

    return EFI_ACCESS_DENIED;
  }

  if (SOCK_IS_NO_MAPPING (Sock)) {
    Status = EFI_NO_MAPPING;
    goto Exit;
  }

  if (SOCK_IS_UNCONFIGURED (Sock)) {

    Status = EFI_NOT_STARTED;
    goto Exit;
  }

  if (!SOCK_IS_LISTENING (Sock)) {

    Status = EFI_ACCESS_DENIED;
    goto Exit;
  }

  Event = ((SOCK_COMPLETION_TOKEN *) Token)->Event;

  if (SockTokenExisted (Sock, Event)) {

    Status = EFI_ACCESS_DENIED;
    goto Exit;
  }

  ListenToken = (EFI_TCP4_LISTEN_TOKEN *) Token;

  //
  // Check if a connection has already in this Sock->ConnectionList
  //
  NET_LIST_FOR_EACH (ListEntry, &Sock->ConnectionList) {

    Socket = NET_LIST_USER_STRUCT (ListEntry, SOCKET, ConnectionList);

    if (SOCK_IS_CONNECTED (Socket)) {
      ListenToken->NewChildHandle = Socket->SockHandle;
      SIGNAL_TOKEN (&(ListenToken->CompletionToken), EFI_SUCCESS);

      NetListRemoveEntry (ListEntry);

      ASSERT (Socket->Parent);

      Socket->Parent->ConnCnt--;

      SOCK_DEBUG_WARN (("SockAccept: Accept a socket,"
        "now conncount is %d", Socket->Parent->ConnCnt)
        );
      Socket->Parent = NULL;

      goto Exit;
    }
  }

  //
  // Buffer this token for latter incoming connection request
  //
  if (NULL == SockBufferToken (Sock, &(Sock->ListenTokenList), Token, 0)) {

    Status = EFI_OUT_OF_RESOURCES;
  }

Exit:
  NET_UNLOCK (&(Sock->Lock));

  return Status;
}

EFI_STATUS
SockSend (
  IN SOCKET *Sock,
  IN VOID   *Token
  )
/*++

Routine Description:

  Issue a token with data to the socket to send out.

Arguments:

  Sock  - Pointer to the socket to process the token with data.
  Token - The token with data that needs to send out.

Returns:

  EFI_SUCCESS       - The token is processed successfully.
  EFI_ACCESS_DENIED - Failed to get the lock to access the socket, or
                      the socket is closed, or the socket is not in a
                      synchronized state , or the token is already in
                      one of this socket's lists.
  EFI_NO_MAPPING    - The IP address configuration operation is not finished.
  EFI_NOT_STARTED   - The socket is not configured.
  EFI_OUT_OF_RESOURCE - Failed to buffer the token due to memory limit.

--*/
{
  SOCK_IO_TOKEN           *SndToken;
  EFI_EVENT               Event;
  UINT32                  FreeSpace;
  EFI_TCP4_TRANSMIT_DATA  *TxData;
  EFI_STATUS              Status;
  SOCK_TOKEN              *SockToken;
  UINT32                  DataLen;

  ASSERT (SOCK_STREAM == Sock->Type);

  Status = NET_TRYLOCK (&(Sock->Lock));
  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockSend: Get the access for socket"
      " failed with %r", Status));

    return EFI_ACCESS_DENIED;
  }

  if (SOCK_IS_NO_MAPPING (Sock)) {
    Status = EFI_NO_MAPPING;
    goto Exit;
  }

  SndToken  = (SOCK_IO_TOKEN *) Token;
  TxData    = (EFI_TCP4_TRANSMIT_DATA *) SndToken->Packet.TxData;

  if (SOCK_IS_UNCONFIGURED (Sock)) {
    Status = EFI_NOT_STARTED;
    goto Exit;
  }

  if (!(SOCK_IS_CONNECTING (Sock) || SOCK_IS_CONNECTED (Sock))) {

    Status = EFI_ACCESS_DENIED;
    goto Exit;
  }

  //
  // check if a token is already in the token buffer
  //
  Event = SndToken->Token.Event;

  if (SockTokenExisted (Sock, Event)) {
    Status = EFI_ACCESS_DENIED;
    goto Exit;
  }

  DataLen = TxData->DataLength;

  //
  // process this sending token now or buffer it only?
  //
  FreeSpace = SockGetFreeSpace (Sock, SOCK_SND_BUF);

  if ((FreeSpace < Sock->SndBuffer.LowWater) || !SOCK_IS_CONNECTED (Sock)) {

    SockToken = SockBufferToken (
                  Sock,
                  &Sock->SndTokenList,
                  SndToken,
                  DataLen
                  );

    if (NULL == SockToken) {
      Status = EFI_OUT_OF_RESOURCES;
    }
  } else {

    SockToken = SockBufferToken (
                  Sock,
                  &Sock->ProcessingSndTokenList,
                  SndToken,
                  DataLen
                  );

    if (NULL == SockToken) {
      SOCK_DEBUG_ERROR (("SockSend: Failed to buffer IO token into"
        " socket processing SndToken List\n", Status));

      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }

    Status = SockProcessTcpSndData (Sock, TxData);

    if (EFI_ERROR (Status)) {
      SOCK_DEBUG_ERROR (("SockSend: Failed to process "
        "Snd Data\n", Status));

      NetListRemoveEntry (&(SockToken->TokenList));
      NetFreePool (SockToken);
    }
  }

Exit:
  NET_UNLOCK (&(Sock->Lock));
  return Status;
}

EFI_STATUS
SockRcv (
  IN SOCKET *Sock,
  IN VOID   *Token
  )
/*++

Routine Description:

  Issue a token to get data from the socket.

Arguments:

  Sock  - Pointer to the socket to get data from.
  Token - The token to store the received data from the socket.

Returns:

  EFI_SUCCESS       - The token is processed successfully.
  EFI_ACCESS_DENIED - Failed to get the lock to access the socket, or
                      the socket is closed, or the socket is not in a
                      synchronized state , or the token is already in
                      one of this socket's lists.
  EFI_NO_MAPPING    - The IP address configuration operation is not finished.
  EFI_NOT_STARTED   - The socket is not configured.
  EFI_CONNECTION_FIN  - The connection is closed and there is no more data.
  EFI_OUT_OF_RESOURCE - Failed to buffer the token due to memory limit.

--*/
{
  SOCK_IO_TOKEN *RcvToken;
  UINT32        RcvdBytes;
  EFI_STATUS    Status;
  EFI_EVENT     Event;

  ASSERT (SOCK_STREAM == Sock->Type);

  Status = NET_TRYLOCK (&(Sock->Lock));
  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockRcv: Get the access for socket"
      " failed with %r", Status));

    return EFI_ACCESS_DENIED;
  }

  if (SOCK_IS_NO_MAPPING (Sock)) {

    Status = EFI_NO_MAPPING;
    goto Exit;
  }

  if (SOCK_IS_UNCONFIGURED (Sock)) {

    Status = EFI_NOT_STARTED;
    goto Exit;
  }

  if (!(SOCK_IS_CONNECTED (Sock) || SOCK_IS_CONNECTING (Sock))) {

    Status = EFI_ACCESS_DENIED;
    goto Exit;
  }

  RcvToken = (SOCK_IO_TOKEN *) Token;

  //
  // check if a token is already in the token buffer of this socket
  //
  Event = RcvToken->Token.Event;
  if (SockTokenExisted (Sock, Event)) {
    Status = EFI_ACCESS_DENIED;
    goto Exit;
  }

  RcvToken  = (SOCK_IO_TOKEN *) Token;
  RcvdBytes = GET_RCV_DATASIZE (Sock);

  //
  // check whether an error has happened before
  //
  if (EFI_ABORTED != Sock->SockError) {

    SIGNAL_TOKEN (&(RcvToken->Token), Sock->SockError);
    Sock->SockError = EFI_ABORTED;
    goto Exit;
  }

  //
  // check whether can not receive and there is no any
  // data buffered in Sock->RcvBuffer
  //
  if (SOCK_IS_NO_MORE_DATA (Sock) && (0 == RcvdBytes)) {

    Status = EFI_CONNECTION_FIN;
    goto Exit;
  }

  if (RcvdBytes != 0) {
    Status = SockProcessRcvToken (Sock, RcvToken);

    if (EFI_ERROR (Status)) {
      goto Exit;
    }

    Status = Sock->ProtoHandler (Sock, SOCK_CONSUMED, NULL);
  } else {

    if (NULL == SockBufferToken (Sock, &Sock->RcvTokenList, RcvToken, 0)) {
      Status = EFI_OUT_OF_RESOURCES;
    }
  }

Exit:
  NET_UNLOCK (&(Sock->Lock));
  return Status;
}

EFI_STATUS
SockFlush (
  IN SOCKET *Sock
  )
/*++

Routine Description:

  Reset the socket and its associated protocol control block.

Arguments:

  Sock  - Pointer to the socket to be flushed.

Returns:

  EFI_SUCCESS       - The socket is flushed successfully.
  EFI_ACCESS_DENIED - Failed to get the lock to access the socket.

--*/
{
  EFI_STATUS  Status;

  ASSERT (SOCK_STREAM == Sock->Type);

  Status = NET_TRYLOCK (&(Sock->Lock));
  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockFlush: Get the access for socket"
      " failed with %r", Status));

    return EFI_ACCESS_DENIED;
  }

  if (!SOCK_IS_CONFIGURED (Sock)) {
    goto Exit;
  }

  Status = Sock->ProtoHandler (Sock, SOCK_FLUSH, NULL);
  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockFlush: Protocol failed handling"
      " SOCK_FLUSH with %r", Status));

    goto Exit;
  }

  SOCK_ERROR (Sock, EFI_ABORTED);
  SockConnFlush (Sock);
  SockSetState (Sock, SO_CLOSED);

  Sock->ConfigureState = SO_UNCONFIGURED;

Exit:
  NET_UNLOCK (&(Sock->Lock));
  return Status;
}

EFI_STATUS
SockClose (
  IN SOCKET  *Sock,
  IN VOID    *Token,
  IN BOOLEAN OnAbort
  )
/*++

Routine Description:

  Close or abort the socket associated connection.

Arguments:

  Sock    - Pointer to the socket of the connection to close or abort.
  Token   - The token for close operation.
  OnAbort - TRUE for aborting the connection, FALSE to close it.

Returns:

  EFI_SUCCESS       - The close or abort operation is initialized successfully.
  EFI_ACCESS_DENIED - Failed to get the lock to access the socket, or
                      the socket is closed, or the socket is not in a
                      synchronized state , or the token is already in
                      one of this socket's lists.
  EFI_NO_MAPPING    - The IP address configuration operation is not finished.
  EFI_NOT_STARTED   - The socket is not configured.

--*/
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  ASSERT (SOCK_STREAM == Sock->Type);

  Status = NET_TRYLOCK (&(Sock->Lock));
  if (EFI_ERROR (Status)) {
    SOCK_DEBUG_ERROR (("SockClose: Get the access for socket"
      " failed with %r", Status));

    return EFI_ACCESS_DENIED;
  }

  if (SOCK_IS_NO_MAPPING (Sock)) {
    Status = EFI_NO_MAPPING;
    goto Exit;
  }

  if (SOCK_IS_UNCONFIGURED (Sock)) {
    Status = EFI_NOT_STARTED;
    goto Exit;
  }

  if (SOCK_IS_DISCONNECTING (Sock)) {
    Status = EFI_ACCESS_DENIED;
    goto Exit;
  }

  Event = ((SOCK_COMPLETION_TOKEN *) Token)->Event;

  if (SockTokenExisted (Sock, Event)) {
    Status = EFI_ACCESS_DENIED;
    goto Exit;
  }

  Sock->CloseToken = Token;
  SockSetState (Sock, SO_DISCONNECTING);

  if (OnAbort) {
    Status = Sock->ProtoHandler (Sock, SOCK_ABORT, NULL);
  } else {
    Status = Sock->ProtoHandler (Sock, SOCK_CLOSE, NULL);
  }

Exit:
  NET_UNLOCK (&(Sock->Lock));
  return Status;
}

EFI_STATUS
SockGetMode (
  IN SOCKET *Sock,
  IN VOID   *Mode
  )
/*++

Routine Description:

  Get the mode data of the low layer protocol.

Arguments:

  Sock  - Pointer to the socket to get mode data from.
  Mode  - Pointer to the data to store the low layer mode
          information.

Returns:

  EFI_SUCCESS     - The mode data is got successfully.
  EFI_NOT_STARTED - The socket is not configured.

--*/
{
  return Sock->ProtoHandler (Sock, SOCK_MODE, Mode);
}

EFI_STATUS
SockGroup (
  IN SOCKET *Sock,
  IN VOID   *GroupInfo
  )
/*++

Routine Description:

  Configure the low level protocol to join a multicast group for
  this socket's connection.

Arguments:

  Sock      - Pointer to the socket of the connection to join the
              specific multicast group.
  GroupInfo - Pointer to the multicast group info.

Returns:

  EFI_SUCCESS       - The configuration is done successfully.
  EFI_ACCESS_DENIED - Failed to get the lock to access the socket.
  EFI_NOT_STARTED   - The socket is not configured.

--*/
{
  EFI_STATUS  Status;

  Status = NET_TRYLOCK (&(Sock->Lock));

  if (EFI_ERROR (Status)) {

    SOCK_DEBUG_ERROR (("SockGroup: Get the access for socket"
      " failed with %r", Status));

    return EFI_ACCESS_DENIED;
  }

  if (SOCK_IS_UNCONFIGURED (Sock)) {
    Status = EFI_NOT_STARTED;
    goto Exit;
  }

  Status = Sock->ProtoHandler (Sock, SOCK_GROUP, GroupInfo);

Exit:
  NET_UNLOCK (&(Sock->Lock));
  return Status;
}

EFI_STATUS
SockRoute (
  IN SOCKET    *Sock,
  IN VOID      *RouteInfo
  )
/*++

Routine Description:

  Add or remove route information in IP route table associated
  with this socket.

Arguments:

  Sock      - Pointer to the socket associated with the IP route
              table to operate on.
  RouteInfo - Pointer to the route information to be processed.

Returns:

  EFI_SUCCESS       - The route table is updated successfully.
  EFI_ACCESS_DENIED - Failed to get the lock to access the socket.
  EFI_NO_MAPPING    - The IP address configuration operation is 
                      not finished.
  EFI_NOT_STARTED   - The socket is not configured.

--*/
{
  EFI_STATUS  Status;

  Status = NET_TRYLOCK (&(Sock->Lock));
  if (EFI_ERROR (Status)) {
    SOCK_DEBUG_ERROR (("SockRoute: Get the access for socket"
      " failed with %r", Status));

    return EFI_ACCESS_DENIED;
  }

  if (SOCK_IS_NO_MAPPING (Sock)) {
    Status = EFI_NO_MAPPING;
    goto Exit;
  }

  if (SOCK_IS_UNCONFIGURED (Sock)) {
    Status = EFI_NOT_STARTED;
    goto Exit;
  }

  Status = Sock->ProtoHandler (Sock, SOCK_ROUTE, RouteInfo);

Exit:
  NET_UNLOCK (&(Sock->Lock));
  return Status;
}
