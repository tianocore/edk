/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MnpMain.c

Abstract:

  Implementation of Managed Network Protocol public services.

--*/

#include "MnpImpl.h"

EFI_STATUS
EFIAPI
MnpGetModeData (
  IN  EFI_MANAGED_NETWORK_PROTOCOL     *This,
  OUT EFI_MANAGED_NETWORK_CONFIG_DATA  *MnpConfigData OPTIONAL,
  OUT EFI_SIMPLE_NETWORK_MODE          *SnpModeData OPTIONAL
  )
/*++

Routine Description:

  Get configuration data of this instance.

Arguments:

  This          - Pointer to the Managed Network Protocol.
  MnpConfigData - Pointer to strorage for MNP operational parameters.
  SnpModeData   - Pointer to strorage for SNP operational parameters.

Returns:

  EFI_SUCCESS           - The operation completed successfully.
  EFI_INVALID_PARAMETER - This is NULL.
  EFI_NOT_STARTED       - This MNP child driver instance has not been configured
                          The default values are returned in MnpConfigData if it
                          is not NULL.

--*/
{
  MNP_INSTANCE_DATA           *Instance;
  EFI_SIMPLE_NETWORK_PROTOCOL *Snp;

  if (This == NULL) {

    return EFI_INVALID_PARAMETER;
  }

  Instance = MNP_INSTANCE_DATA_FROM_THIS (This);

  if (MnpConfigData != NULL) {
    //
    // Copy the instance configuration data.
    //
    *MnpConfigData = Instance->ConfigData;
  }

  if (SnpModeData != NULL) {
    //
    // Copy the underlayer Snp mode data.
    //
    Snp           = Instance->MnpServiceData->Snp;
    *SnpModeData  = *(Snp->Mode);
  }

  if (!Instance->Configured) {

    return EFI_NOT_STARTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
MnpConfigure (
  IN EFI_MANAGED_NETWORK_PROTOCOL     *This,
  IN EFI_MANAGED_NETWORK_CONFIG_DATA  *MnpConfigData OPTIONAL
  )
/*++

Routine Description:

  Set or clear the operational parameters for the MNP child driver.

Arguments:

  This          - Pointer to the Managed Network Protocol.
  MnpConfigData - Pointer to the configuration data that will be assigned to
                  the MNP child driver instance. If NULL, the MNP child driver
                  instance is reset to startup defaults and all pending transmit
                  and receive requests are flushed.

Returns:

  EFI_SUCCESS           - The operation completed successfully.
  EFI_INVALID_PARAMETER - One or more parameter is invalid.
  EFI_OUT_OF_RESOURCES  - Required system resources (usually memory) could not be
                          allocated.
  EFI_UNSUPPORTED       - EnableReceiveTimestamps is TRUE, this implementation doesn't
                          support it.
  EFI_DEVICE_ERROR      - An unexpected network or system error occurred.
  Other                 - The MNP child driver instance has been reset to startup defaults.

--*/
{
  MNP_INSTANCE_DATA *Instance;

  if ((This == NULL) || 
    ((MnpConfigData != NULL) &&
    (MnpConfigData->ProtocolTypeFilter > 0) &&
    (MnpConfigData->ProtocolTypeFilter <= 1500))) {

    return EFI_INVALID_PARAMETER;
  }

  Instance = MNP_INSTANCE_DATA_FROM_THIS (This);

  if ((MnpConfigData == NULL) && (!Instance->Configured)) {
    //
    // If the instance is not configured and a reset is requested, just return.
    //
    return EFI_SUCCESS;
  }

  //
  // Configure the instance.
  //
  return MnpConfigureInstance (Instance, MnpConfigData);
}

EFI_STATUS
EFIAPI
MnpMcastIpToMac (
  IN  EFI_MANAGED_NETWORK_PROTOCOL  *This,
  IN  BOOLEAN                       Ipv6Flag,
  IN  EFI_IP_ADDRESS                *IpAddress,
  OUT EFI_MAC_ADDRESS               *MacAddress
  )
/*++

Routine Description:

  Translate a multicast IP address to a multicast hardware (MAC) address.
    
Arguments:

  This       - Pointer to the Managed Network Protocol.
  Ipv6Flag   - Set to TRUE if IpAddress is an IPv6 multicast address.
               Set to FALSE if IpAddress is an IPv4 multicast address.
  IpAddress  - Pointer to the multicast IP address to convert.
  MacAddress - Pointer to the resulting multicast MAC address.

Returns:
    
  EFI_SUCCESS           - The operation completed successfully.
  EFI_INVALID_PARAMETER - One or more parameter is invalid.
  EFI_NOT_STARTED       - This MNP child driver instance has not been configured.
  EFI_UNSUPPORTED       - Ipv6Flag is TRUE, this implementation doesn't supported it.
  EFI_DEVICE_ERROR      - An unexpected network or system error occurred.
  Other                 - The address could not be converted.

--*/
{
  EFI_STATUS                  Status;
  MNP_INSTANCE_DATA           *Instance;
  EFI_SIMPLE_NETWORK_PROTOCOL *Snp;
  EFI_TPL                     OldTpl;

  if ((This == NULL) || (IpAddress == NULL) || (MacAddress == NULL)) {

    return EFI_INVALID_PARAMETER;
  }

  if (Ipv6Flag) {
    //
    // Currently IPv6 isn't supported.
    //
    return EFI_UNSUPPORTED;
  }

  if (!IP4_IS_MULTICAST (EFI_NTOHL (*IpAddress))) {
    //
    // The IPv4 address passed in is not a multicast address.
    //
    return EFI_INVALID_PARAMETER;
  }

  Instance = MNP_INSTANCE_DATA_FROM_THIS (This);

  if (!Instance->Configured) {

    return EFI_NOT_STARTED;
  }

  Snp = Instance->MnpServiceData->Snp;
  ASSERT (Snp != NULL);

  if (Snp->Mode->IfType == NET_IFTYPE_ETHERNET) {
    //
    // Translate the IPv4 address into a multicast MAC address if the NIC is an
    // ethernet NIC.
    //
    MacAddress->Addr[0] = 0x01;
    MacAddress->Addr[1] = 0x00;
    MacAddress->Addr[2] = 0x5E;
    MacAddress->Addr[3] = IpAddress->v4.Addr[1] & 0x7F;
    MacAddress->Addr[4] = IpAddress->v4.Addr[2];
    MacAddress->Addr[5] = IpAddress->v4.Addr[3];

    return EFI_SUCCESS;
  } else {
    //
    // Invoke Snp to translate the multicast IP address.
    //
    OldTpl = gBS->RaiseTPL (NET_TPL_GLOBAL_LOCK);
    Status = Snp->MCastIpToMac (
                    Snp,
                    Ipv6Flag,
                    IpAddress,
                    MacAddress
                    );
    gBS->RestoreTPL (OldTpl);

    return Status;
  }
}

EFI_STATUS
EFIAPI
MnpGroups (
  IN EFI_MANAGED_NETWORK_PROTOCOL  *This,
  IN BOOLEAN                       JoinFlag,
  IN EFI_MAC_ADDRESS               *MacAddress OPTIONAL
  )
/*++

Routine Description:

  Enable or disable receie filters for multicast address.
    
Arguments:

  This       - Pointer to the Managed Network Protocol.
  JoinFlag   - Set to TRUE to join this multicast group.
               Set to FALSE to leave this multicast group.
  MacAddress - Pointer to the multicast MAC group (address) to join or leave.
    
Returns:
    
  EFI_SUCCESS           - The operation completed successfully.
  EFI_INVALID_PARAMETER - One or more parameter is invalid
  EFI_NOT_STARTED       - This MNP child driver instance has not been configured.
  EFI_ALREADY_STARTED   - The supplied multicast group is already joined.
  EFI_NOT_FOUND         - The supplied multicast group is not joined.
  EFI_DEVICE_ERROR      - An unexpected network or system error occurred.
  Other                 - The requested operation could not be completed.
                          The MNP multicast group settings are unchanged.

--*/
{
  MNP_INSTANCE_DATA       *Instance;
  EFI_SIMPLE_NETWORK_MODE *SnpMode;
  MNP_GROUP_CONTROL_BLOCK *GroupCtrlBlk;
  MNP_GROUP_ADDRESS       *GroupAddress;
  NET_LIST_ENTRY          *ListEntry;
  BOOLEAN                 AddressExist;

  if (This == NULL || (JoinFlag && (MacAddress == NULL))) {
    //
    // This is NULL, or it's a join operation but MacAddress is NULL.
    //
    return EFI_INVALID_PARAMETER;
  }

  Instance  = MNP_INSTANCE_DATA_FROM_THIS (This);
  SnpMode   = Instance->MnpServiceData->Snp->Mode;

  if ((MacAddress != NULL) && 
    !NET_MAC_IS_MULTICAST (MacAddress, &SnpMode->BroadcastAddress, SnpMode->HwAddressSize)) {
    //
    // The passed in MacAddress is not a mutlticast mac address.
    //
    return EFI_INVALID_PARAMETER;
  }

  if (!Instance->Configured) {

    return EFI_NOT_STARTED;
  }

  if (!Instance->ConfigData.EnableMulticastReceive) {
    //
    // It's invalid to do any group operation when this instance isn't configured
    // to do multicast receive.
    //
    return EFI_INVALID_PARAMETER;
  }

  AddressExist  = FALSE;
  GroupCtrlBlk  = NULL;

  if (MacAddress != NULL) {
    //
    // Search the instance's GroupCtrlBlkList to find the specific address.
    //
    NET_LIST_FOR_EACH (ListEntry, &Instance->GroupCtrlBlkList) {

      GroupCtrlBlk = NET_LIST_USER_STRUCT (
                      ListEntry,
                      MNP_GROUP_CONTROL_BLOCK,
                      CtrlBlkEntry
                      );
      GroupAddress = GroupCtrlBlk->GroupAddress;
      if (0 == NetCompareMem (
                MacAddress,
                &GroupAddress->Address,
                SnpMode->HwAddressSize
                )) {
        //
        // There is already the same multicast mac address configured.
        //
        AddressExist = TRUE;
        break;
      }
    }

    if (JoinFlag && AddressExist) {
      //
      // The multicast mac address to join already exists.
      //
      return EFI_ALREADY_STARTED;
    }

    if (!JoinFlag && !AddressExist) {
      //
      // The multicast mac address to leave doesn't exist in this instance.
      //
      return EFI_NOT_FOUND;
    }
  } else if (NetListIsEmpty (&Instance->GroupCtrlBlkList)) {
    //
    // The MacAddress is NULL and there is no configured multicast mac address,
    // just return.
    //
    return EFI_SUCCESS;
  }

  //
  // OK, it is time to take action.
  //
  return MnpGroupOp (Instance, JoinFlag, MacAddress, GroupCtrlBlk);
}

EFI_STATUS
EFIAPI
MnpTransmit (
  IN EFI_MANAGED_NETWORK_PROTOCOL          *This,
  IN EFI_MANAGED_NETWORK_COMPLETION_TOKEN  *Token
  )
/*++

Routine Description:

  Place an outgoing packet into the transmit queue.
    
Arguments:

  This  - Pointer to the Managed Network Protocol.
  Token - Pointer to a token associated with the transmit data descriptor.
    
Returns:
    
  EFI_SUCCESS             - The operation completed successfully.
  EFI_INVALID_PARAMETER   - One or more parameter is invalid
  EFI_NOT_STARTED         - This MNP child driver instance has not been configured.
  EFI_ACCESS_DENIED       - The transmit completion token is already in the
                            transmit queue.
  EFI_OUT_OF_RESOURCES    - The transmit data could not be queued due to a
                            lack of system resources (usually memory).
  EFI_DEVICE_ERROR        - An unexpected system or network error occurred. 
                            The MNP child driver instance has been reset
                            to startup defaults.
  EFI_NOT_READY           - The transmit request could not be queued because
                            the transmit queue is full.

--*/
{
  EFI_STATUS        Status;
  MNP_INSTANCE_DATA *Instance;
  MNP_SERVICE_DATA  *MnpServiceData;
  UINT8             *PktBuf;
  UINT32            PktLen;

  if ((This == NULL) || (Token == NULL)) {

    return EFI_INVALID_PARAMETER;
  }

  Instance = MNP_INSTANCE_DATA_FROM_THIS (This);

  if (!Instance->Configured) {

    return EFI_NOT_STARTED;
  }

  if (!MnpIsValidTxToken (Instance, Token)) {
    //
    // The Token is invalid.
    //
    return EFI_INVALID_PARAMETER;
  }

  MnpServiceData = Instance->MnpServiceData;
  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);

  //
  // Try to acquire the TxLock.
  //
  if (EFI_ERROR (NET_TRYLOCK (&MnpServiceData->TxLock))) {

    return EFI_ACCESS_DENIED;
  }
  //
  // Build the tx packet
  //
  MnpBuildTxPacket (MnpServiceData, Token->Packet.TxData, &PktBuf, &PktLen);

  //
  //  OK, send the packet synchronously.
  //
  Status = MnpSyncSendPacket (MnpServiceData, PktBuf, PktLen, Token);

  NET_UNLOCK (&MnpServiceData->TxLock);

  return Status;
}

EFI_STATUS
EFIAPI
MnpReceive (
  IN EFI_MANAGED_NETWORK_PROTOCOL          *This,
  IN EFI_MANAGED_NETWORK_COMPLETION_TOKEN  *Token
  )
/*++

Routine Description:

  Place an asynchronous receiving request into the receiving queue.
    
Arguments:

  This  - Pointer to the EFI_MANAGED_NETWORK_PROTOCOL instance.
  Token - Pointer to a token associated with the receive data descriptor. 
    
Returns:

  EFI_SUCCESS             - The receive completion token was cached.
  EFI_NOT_STARTED         - This MNP child driver instance has not been configured.
  EFI_INVALID_PARAMETER   - One or more parameter is invalid.
  EFI_OUT_OF_RESOURCES    - The transmit data could not be queued due to a lack of
                            system resources (usually memory).
  EFI_DEVICE_ERROR        - An unexpected system or network error occurred. The MNP
                            child driver instance has been reset to startup defaults.
  EFI_ACCESS_DENIED       - The receive completion token was already in the receive queue.
  EFI_NOT_READY           - The receive request could not be queued because the receive
                            queue is full.

--*/
{
  EFI_STATUS        Status;
  MNP_INSTANCE_DATA *Instance;

  if ((This == NULL) || (Token == NULL) || (Token->Event == NULL)) {

    return EFI_INVALID_PARAMETER;
  }

  Instance = MNP_INSTANCE_DATA_FROM_THIS (This);

  if (!Instance->Configured) {

    return EFI_NOT_STARTED;
  }
  //
  // Try to acquire the lock
  //
  if (EFI_ERROR (NET_TRYLOCK (&Instance->RxLock))) {

    return EFI_ACCESS_DENIED;
  }
  //
  // Check whether this token(event) is already in the rx token queue.
  //
  Status = NetMapIterate (&Instance->RxTokenMap, MnpTokenExist, (VOID *) Token);
  if (EFI_ERROR (Status)) {

    goto UNLOCK_EXIT;
  }

  //
  // Insert the Token into the RxTokenMap.
  //
  Status = NetMapInsertTail (&Instance->RxTokenMap, (VOID *) Token, NULL);

  if (!EFI_ERROR (Status)) {
    //
    // Try to deliver any buffered packets.
    //
    Status = MnpInstanceDeliverPacket (Instance);
  }

UNLOCK_EXIT:

  NET_UNLOCK (&Instance->RxLock);

  return Status;
}

EFI_STATUS
EFIAPI
MnpCancel (
  IN EFI_MANAGED_NETWORK_PROTOCOL          *This,
  IN EFI_MANAGED_NETWORK_COMPLETION_TOKEN  *Token OPTIONAL
  )
/*++

Routine Description:

  Abort a pending transmit or receive request.
    
Arguments:

  This    - Pointer to the EFI_MANAGED_NETWORK_PROTOCOL instance.
  Token   - Pointer to a token that has been issued by
            EFI_MANAGED_NETWORK_PROTOCOL.Transmit() or
            EFI_MANAGED_NETWORK_PROTOCOL.Receive().
            If NULL, all pending tokens are aborted.
    
Returns:
    
  EFI_SUCCESS           - The asynchronous I/O request was aborted and Token->Event
                          was signaled.
  EFI_NOT_STARTED       - This MNP child driver instance has not been configured.
  EFI_INVALID_PARAMETER - This is NULL.
  EFI_NOT_FOUND         - The asynchronous I/O request was not found in the transmit or
                          receive queue. It has either completed or was not issued by
                          Transmit() and Receive().

--*/
{
  EFI_STATUS        Status;
  MNP_INSTANCE_DATA *Instance;

  if (This == NULL) {

    return EFI_INVALID_PARAMETER;
  }

  Instance = MNP_INSTANCE_DATA_FROM_THIS (This);

  if (!Instance->Configured) {

    return EFI_NOT_STARTED;
  }

  if (EFI_ERROR (NET_TRYLOCK (&Instance->RxLock))) {
    return EFI_ACCESS_DENIED;
  }

  //
  // Iterate the RxTokenMap to cancel the specified Token.
  //
  Status = NetMapIterate (&Instance->RxTokenMap, MnpCancelTokens, (VOID *) Token);

  NET_UNLOCK (&Instance->RxLock);

  if (Token != NULL) {

    Status = (Status == EFI_ABORTED) ? EFI_SUCCESS : EFI_NOT_FOUND;
  }

  return Status;
}

EFI_STATUS
EFIAPI
MnpPoll (
  IN EFI_MANAGED_NETWORK_PROTOCOL  *This
  )
/*++

Routine Description:

  Poll the network interface to do transmit/receive work.
    
Arguments:

  This    - Pointer to the EFI_MANAGED_NETWORK_PROTOCOL instance.
    
Returns:

  EFI_SUCCESS      - Incoming or outgoing data was processed.
  EFI_NOT_STARTED  - This MNP child driver instance has not been configured.
  EFI_DEVICE_ERROR - An unexpected system or network error occurred.
                     The MNP child driver instance has been reset to startup defaults.
  EFI_NOT_READY    - No incoming or outgoing data was processed.
  EFI_TIMEOUT      - Data was dropped out of the transmit and/or receive queue.

--*/
{
  EFI_STATUS        Status;
  MNP_INSTANCE_DATA *Instance;

  if (This == NULL) {

    return EFI_INVALID_PARAMETER;
  }

  Instance = MNP_INSTANCE_DATA_FROM_THIS (This);
  ASSERT (Instance->Signature == MNP_INSTANCE_DATA_SIGNATURE);

  if (!Instance->Configured) {

    return EFI_NOT_STARTED;
  }
  //
  // Try to acquire the rx lock.
  //
  if (EFI_ERROR (NET_TRYLOCK (&Instance->RxLock))) {

    return EFI_SUCCESS;
  }
  //
  // Try to receive packets.
  //
  Status = MnpReceivePacket (Instance->MnpServiceData);

  NET_UNLOCK (&Instance->RxLock);

  return Status;
}
