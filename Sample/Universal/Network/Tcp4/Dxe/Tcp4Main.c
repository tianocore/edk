/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  Tcp4Main.c

Abstract:

  Implementation of TCP4 protocol services.

--*/

#include "Tcp4Main.h"

STATIC
EFI_STATUS
Tcp4ChkDataBuf (
  IN UINT32                 DataLen,
  IN UINT32                 FragmentCount,
  IN EFI_TCP4_FRAGMENT_DATA *FragmentTable
  )
/*++

Routine Description:

  Check the integrity of the data buffer.

Arguments:

  DataLen       - The total length of the data buffer.
  FragmentCount - The fragment count of the fragment table.
  FragmentTable - Pointer to the fragment table of the data buffer.

Returns:

  EFI_SUCCESS           - The integrity check is passed.
  EFI_INVALID_PARAMETER - The integrity check is failed.

--*/
{
  UINT32 Index;

  UINT32 Len;

  for (Index = 0, Len = 0; Index < FragmentCount; Index++) {
    Len = Len + FragmentTable[Index].FragmentLength;
  }

  if (DataLen != Len) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Tcp4GetModeData (
  IN  EFI_TCP4_PROTOCOL                  * This,
  OUT EFI_TCP4_CONNECTION_STATE          * Tcp4State OPTIONAL,
  OUT EFI_TCP4_CONFIG_DATA               * Tcp4ConfigData OPTIONAL,
  OUT EFI_IP4_MODE_DATA                  * Ip4ModeData OPTIONAL,
  OUT EFI_MANAGED_NETWORK_CONFIG_DATA    * MnpConfigData OPTIONAL,
  OUT EFI_SIMPLE_NETWORK_MODE            * SnpModeData OPTIONAL
  )
/*++

Routine Description:

  Get the current operational status.

Arguments:

  This            - Pointer to the EFI_TCP4_PROTOCOL instance.
  Tcp4State       - Pointer to the buffer to receive the current TCP state.
  Tcp4ConfigData  - Pointer to the buffer to receive the current TCP
                    configuration.
  Ip4ModeData     - Pointer to the buffer to receive the current IPv4
                    configuration.
  MnpConfigData   - Pointer to the buffer to receive the current MNP
                    configuration data indirectly used by the TCPv4 Instance.
  SnpModeData     - Pointer to the buffer to receive the current SNP
                    configuration data indirectly used by the TCPv4 Instance.

Returns:

  EFI_SUCCESS           - The mode data was read.
  EFI_NOT_STARTED       - No configuration data is available because this
                          instance hasn't been started.
  EFI_INVALID_PARAMETER - This is NULL.

--*/
{
  TCP4_MODE_DATA  TcpMode;
  SOCKET          *Sock;

  if (NULL == This) {
    return EFI_INVALID_PARAMETER;
  }

  Sock                    = SOCK_FROM_THIS (This);

  TcpMode.Tcp4State       = Tcp4State;
  TcpMode.Tcp4ConfigData  = Tcp4ConfigData;
  TcpMode.Ip4ModeData     = Ip4ModeData;
  TcpMode.MnpConfigData   = MnpConfigData;
  TcpMode.SnpModeData     = SnpModeData;

  return SockGetMode (Sock, &TcpMode);
}

EFI_STATUS
EFIAPI
Tcp4Configure (
  IN EFI_TCP4_PROTOCOL        * This,
  IN EFI_TCP4_CONFIG_DATA     * TcpConfigData OPTIONAL
  )
/*++

Routine Description:

  Initialize or brutally reset the operational parameters for 
  this EFI TCPv4 instance.

Arguments:

  This          - Pointer to the EFI_TCP4_PROTOCOL instance.
  TcpConfigData - Pointer to the configure data to configure the instance.

Returns:

  EFI_SUCCESS           - The operational settings are set, changed, or 
                          reset successfully.
  EFI_NO_MAPPING        - When using a default address, configuration
                          (through DHCP, BOOTP, RARP, etc.) is not finished.
  EFI_INVALID_PARAMETER - One or more parameters are invalid.
  EFI_ACCESS_DENIED     - Configuring TCP instance when it is already
                          configured.
  EFI_DEVICE_ERROR      - An unexpected network or system error occurred.
  EFI_UNSUPPORTED       - One or more of the control options are not
                          supported in the implementation.
  EFI_OUT_OF_RESOURCES  - Could not allocate enough system resources.

--*/
{
  EFI_TCP4_OPTION  *Option;
  SOCKET           *Sock;
  EFI_STATUS       Status;

  if (NULL == This) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Tcp protocol related parameter check will be conducted here
  //
  if (NULL != TcpConfigData) {
    if ((EFI_IP4 (TcpConfigData->AccessPoint.RemoteAddress) != 0) &&
      !Ip4IsUnicast (EFI_NTOHL (TcpConfigData->AccessPoint.RemoteAddress), 0)) {
      return EFI_INVALID_PARAMETER;
    }

    if (!TcpConfigData->AccessPoint.UseDefaultAddress) {
      if (!Ip4IsUnicast (EFI_NTOHL (TcpConfigData->AccessPoint.StationAddress), 0) ||
          !IP4_IS_VALID_NETMASK (EFI_NTOHL (TcpConfigData->AccessPoint.SubnetMask))
            ) {
        return EFI_INVALID_PARAMETER;
      }
    }

    if (TcpConfigData->AccessPoint.ActiveFlag &&
        (0 == TcpConfigData->AccessPoint.RemotePort || 
        (EFI_IP4 (TcpConfigData->AccessPoint.RemoteAddress) == 0))
        ) {
      return EFI_INVALID_PARAMETER;
    }

    Option = TcpConfigData->ControlOption;
    if ((NULL != Option) &&
        (Option->EnableSelectiveAck || Option->EnablePathMtuDiscovery)) {
      return EFI_UNSUPPORTED;
    }
  }

  Sock = SOCK_FROM_THIS (This);

  if (NULL == TcpConfigData) {
    return SockFlush (Sock);
  }

  Status = SockConfigure (Sock, TcpConfigData);

  if (EFI_NO_MAPPING == Status) {
    Sock->ConfigureState = SO_NO_MAPPING;
  }

  return Status;
}

EFI_STATUS
EFIAPI
Tcp4Routes (
  IN EFI_TCP4_PROTOCOL           *This,
  IN BOOLEAN                     DeleteRoute,
  IN EFI_IPv4_ADDRESS            *SubnetAddress,
  IN EFI_IPv4_ADDRESS            *SubnetMask,
  IN EFI_IPv4_ADDRESS            *GatewayAddress
  )
/*++

Routine Description:

  Add or delete routing entries.

Arguments:

  This            - Pointer to the EFI_TCP4_PROTOCOL instance.
  DeleteRoute     - If TRUE, delete the specified route from routing table;
                    if FALSE, add the specified route to routing table.
  SubnetAddress   - The destination network.
  SubnetMask      - The subnet mask for the destination network.
  GatewayAddress  - The gateway address for this route.

Returns:

  EFI_SUCCESS           - The operation completed successfully.
  EFI_NOT_STARTED       - The EFI_TCP4_PROTOCOL instance has not been
                          configured.
  EFI_NO_MAPPING        - When using a default address, configuration
                          (through DHCP, BOOTP, RARP, etc.) is not finished.
  EFI_INVALID_PARAMETER - One or more parameters are invalid.
  EFI_OUT_OF_RESOURCES  - Could not allocate enough resources to add the
                          entry to the routing table.
  EFI_NOT_FOUND         - This route is not in the routing table.
  EFI_ACCESS_DENIED     - This route is already in the routing table.
  EFI_UNSUPPORTED       - The TCP driver does not support this operation.

--*/
{
  SOCKET          *Sock;
  TCP4_ROUTE_INFO RouteInfo;

  if (NULL == This) {
    return EFI_INVALID_PARAMETER;
  }

  Sock                      = SOCK_FROM_THIS (This);

  RouteInfo.DeleteRoute     = DeleteRoute;
  RouteInfo.SubnetAddress   = SubnetAddress;
  RouteInfo.SubnetMask      = SubnetMask;
  RouteInfo.GatewayAddress  = GatewayAddress;

  return SockRoute (Sock, &RouteInfo);
}

EFI_STATUS
EFIAPI
Tcp4Connect (
  IN EFI_TCP4_PROTOCOL           *This,
  IN EFI_TCP4_CONNECTION_TOKEN   *ConnectionToken
  )
/*++

Routine Description:

  Initiate a nonblocking TCP connection request for an active TCP instance.

Arguments:

  This            - Pointer to the EFI_TCP4_PROTOCOL instance
  ConnectionToken - Pointer to the connection token to return when the TCP
                    three way handshake finishes.

Returns:

  EFI_SUCCESS           - The connection request is successfully initiated.
  EFI_NOT_STARTED       - This EFI_TCP4_PROTOCOL instance hasn't been
                          configured.
  EFI_ACCESS_DENIED     - The instance is not configured as an active one
                          or it is not in Tcp4StateClosed state.
  EFI_INVALID_PARAMETER - One or more parameters are invalid.
  EFI_OUT_OF_RESOURCES  - The driver can't allocate enough resource to
                          initiate the active open.
  EFI_DEVICE_ERROR      - An unexpected system or network error occurred.

--*/
{
  SOCKET  *Sock;

  if (NULL == This ||
      NULL == ConnectionToken ||
      NULL == ConnectionToken->CompletionToken.Event) {
    return EFI_INVALID_PARAMETER;
  }

  Sock = SOCK_FROM_THIS (This);

  return SockConnect (Sock, ConnectionToken);
}

EFI_STATUS
EFIAPI
Tcp4Accept (
  IN EFI_TCP4_PROTOCOL             *This,
  IN EFI_TCP4_LISTEN_TOKEN         *ListenToken
  )
/*++

Routine Description:

  Listen on the passive instance to accept an incoming connection request.

Arguments:

  This        - Pointer to the EFI_TCP4_PROTOCOL instance 
  ListenToken - Pointer to the listen token to return when operation
                finishes.

Returns:

  EFI_SUCCESS           - The listen token has been queued successfully.
  EFI_NOT_STARTED       - The EFI_TCP4_PROTOCOL instance hasn't been
                          configured.
  EFI_ACCESS_DENIED     - The instatnce is not a passive one or it is not
                          in Tcp4StateListen state or a same listen token
                          has already existed in the listen token queue of
                          this TCP instance.
  EFI_INVALID_PARAMETER - One or more parameters are invalid.
  EFI_OUT_OF_RESOURCES  - Could not allocate enough resources to finish
                          the operation.
  EFI_DEVICE_ERROR      - An unexpected system or network error occurred.

--*/
{
  SOCKET  *Sock;

  if (NULL == This ||
      NULL == ListenToken ||
      NULL == ListenToken->CompletionToken.Event) {
    return EFI_INVALID_PARAMETER;
  }

  Sock = SOCK_FROM_THIS (This);

  return SockAccept (Sock, ListenToken);
}

EFI_STATUS
EFIAPI
Tcp4Transmit (
  IN EFI_TCP4_PROTOCOL            *This,
  IN EFI_TCP4_IO_TOKEN            *Token
  )
/*++

Routine Description:

  Queues outgoing data into the transmit queue

Arguments:

  This  - Pointer to the EFI_TCP4_PROTOCOL instance
  Token - Pointer to the completion token to queue to the transmit queue

Returns:

  EFI_SUCCESS           - The data has been queued for transmission
  EFI_NOT_STARTED       - The EFI_TCP4_PROTOCOL instance hasn't been
                          configured.
  EFI_NO_MAPPING        - When using a default address, configuration
                          (DHCP, BOOTP, RARP, etc.) is not finished yet.
  EFI_INVALID_PARAMETER - One or more parameters are invalid
  EFI_ACCESS_DENIED     - One or more of the following conditions is TRUE:
                          * A transmit completion token with the same
                            Token-> CompletionToken.Event was already in
                            the transmission queue.
                          * The current instance is in Tcp4StateClosed state
                          * The current instance is a passive one and it is
                            in Tcp4StateListen state.
                          * User has called Close() to disconnect this
                            connection.
  EFI_NOT_READY         - The completion token could not be queued because
                          the transmit queue is full.
  EFI_OUT_OF_RESOURCES  - Could not queue the transmit data because of
                          resource shortage.
  EFI_NETWORK_UNREACHABLE - There is no route to the destination network or 
                            address.

--*/
{
  SOCKET      *Sock;
  EFI_STATUS  Status;

  if (NULL == This ||
      NULL == Token ||
      NULL == Token->CompletionToken.Event ||
      NULL == Token->Packet.TxData ||
      0 == Token->Packet.TxData->FragmentCount ||
      0 == Token->Packet.TxData->DataLength
      ) {
    return EFI_INVALID_PARAMETER;
  }

  Status = Tcp4ChkDataBuf (
            Token->Packet.TxData->DataLength,
            Token->Packet.TxData->FragmentCount,
            Token->Packet.TxData->FragmentTable
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Sock = SOCK_FROM_THIS (This);

  return SockSend (Sock, Token);

}

EFI_STATUS
EFIAPI
Tcp4Receive (
  IN EFI_TCP4_PROTOCOL           *This,
  IN EFI_TCP4_IO_TOKEN           *Token
  )
/*++

Routine Description:

  Place an asynchronous receive request into the receiving queue.

Arguments:

  This  - Pointer to the EFI_TCP4_PROTOCOL instance.
  Token - Pointer to a token that is associated with the receive
          data descriptor.

Returns:

  EFI_SUCCESS           - The receive completion token was cached
  EFI_NOT_STARTED       - The EFI_TCP4_PROTOCOL instance hasn't been
                          configured.
  EFI_NO_MAPPING        - When using a default address, configuration
                          (DHCP, BOOTP, RARP, etc.) is not finished yet.
  EFI_INVALID_PARAMETER - One or more parameters are invalid.
  EFI_OUT_OF_RESOURCES  - The receive completion token could not be queued
                          due to a lack of system resources.
  EFI_DEVICE_ERROR      - An unexpected system or network error occurred.
  EFI_ACCESS_DENIED     - One or more of the following conditions is TRUE:
                          * A receive completion token with the same
                            Token->CompletionToken.Event was already in
                            the receive queue.
                          * The current instance is in Tcp4StateClosed state.
                          * The current instance is a passive one and it
                            is in Tcp4StateListen state.
                          * User has called Close() to disconnect this
                            connection.
  EFI_CONNECTION_FIN    - The communication peer has closed the connection
                          and there is no any buffered data in the receive
                          buffer of this instance.
  EFI_NOT_READY         - The receive request could not be queued because
                          the receive queue is full.

--*/
{
  SOCKET      *Sock;
  EFI_STATUS  Status;

  if (NULL == This ||
      NULL == Token ||
      NULL == Token->CompletionToken.Event ||
      NULL == Token->Packet.RxData ||
      0 == Token->Packet.RxData->FragmentCount ||
      0 == Token->Packet.RxData->DataLength
      ) {
    return EFI_INVALID_PARAMETER;
  }

  Status = Tcp4ChkDataBuf (
            Token->Packet.RxData->DataLength,
            Token->Packet.RxData->FragmentCount,
            Token->Packet.RxData->FragmentTable
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Sock = SOCK_FROM_THIS (This);

  return SockRcv (Sock, Token);

}

EFI_STATUS
EFIAPI
Tcp4Close (
  IN EFI_TCP4_PROTOCOL           *This,
  IN EFI_TCP4_CLOSE_TOKEN        *CloseToken
  )
/*++

Routine Description:

  Disconnecting a TCP connection gracefully or reset a TCP connection.

Arguments:

  This        - Pointer to the EFI_TCP4_PROTOCOL instance
  CloseToken  - Pointer to the close token to return when operation finishes.

Returns:

  EFI_SUCCESS           - The operation completed successfully
  EFI_NOT_STARTED       - The EFI_TCP4_PROTOCOL instance hasn't been
                          configured.
  EFI_ACCESS_DENIED     - One or more of the following are TRUE:
                          * Configure() has been called with TcpConfigData
                            set to NULL and this function has not returned.
                          * Previous Close() call on this instance has not
                            finished.
  EFI_INVALID_PARAMETER - One ore more parameters are invalid
  EFI_OUT_OF_RESOURCES  - Could not allocate enough resource to finish the
                          operation
  EFI_DEVICE_ERROR      - Any unexpected and not belonged to above category
                          error.

--*/
{
  SOCKET  *Sock;

  if (NULL == This ||
      NULL == CloseToken ||
      NULL == CloseToken->CompletionToken.Event) {
    return EFI_INVALID_PARAMETER;
  }

  Sock = SOCK_FROM_THIS (This);

  return SockClose (Sock, CloseToken, CloseToken->AbortOnClose);
}

EFI_STATUS
EFIAPI
Tcp4Cancel (
  IN EFI_TCP4_PROTOCOL           * This,
  IN EFI_TCP4_COMPLETION_TOKEN   * Token OPTIONAL
  )
/*++

Routine Description:

  Abort an asynchronous connection, listen, transmission or receive request.

Arguments:

  This  - Pointer to the EFI_TCP4_PROTOCOL instance.
  Token - Pointer to a token that has been issued by Connect(), Accept(),
          Transmit() or Receive(). If NULL, all pending tokens issued by
          above four functions will be aborted.

Returns:

  EFI_UNSUPPORTED - The operation is not supported in current implementation.

--*/
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
Tcp4Poll (
  IN EFI_TCP4_PROTOCOL        *This
  )
/*++

Routine Description:

  Poll to receive incoming data and transmit outgoing segments.

Arguments:

  This  - Pointer to the EFI_TCP4_PROTOCOL instance.

Returns:

  EFI_SUCCESS           - Incoming or outgoing data was processed.
  EFI_INVALID_PARAMETER - This is NULL.
  EFI_DEVICE_ERROR      - An unexpected system or network error occurred.
  EFI_NOT_READY         - No incoming or outgoing data was processed.
  EFI_TIMEOUT           - Data was dropped out of the transmission or
                          receive queue. Consider increasing the polling rate.

--*/
{
  SOCKET      *Sock;
  EFI_STATUS  Status;

  if (NULL == This) {
    return EFI_INVALID_PARAMETER;
  }

  Sock    = SOCK_FROM_THIS (This);

  Status  = Sock->ProtoHandler (Sock, SOCK_POLL, NULL);

  return Status;
}
