/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MnpConfig.c

Abstract:

  Implementation of Managed Network Protocol private services.

--*/

#include "MnpImpl.h"

EFI_SERVICE_BINDING_PROTOCOL    mMnpServiceBindingProtocol = {
  MnpServiceBindingCreateChild,
  MnpServiceBindingDestroyChild
};

EFI_MANAGED_NETWORK_PROTOCOL    mMnpProtocolTemplate = {
  MnpGetModeData,
  MnpConfigure,
  MnpMcastIpToMac,
  MnpGroups,
  MnpTransmit,
  MnpReceive,
  MnpCancel,
  MnpPoll
};

EFI_MANAGED_NETWORK_CONFIG_DATA mMnpDefaultConfigData = {
  10000,
  10000,
  0,
  FALSE,
  FALSE,
  FALSE,
  FALSE,
  FALSE,
  FALSE,
  FALSE
};

STATIC
EFI_STATUS
MnpAddFreeNbuf (
  IN MNP_SERVICE_DATA  *MnpServiceData,
  IN UINTN             Count
  );

STATIC
EFI_STATUS
MnpStartSnp (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  );

STATIC
EFI_STATUS
MnpStopSnp (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  );

STATIC
EFI_STATUS
MnpStart (
  IN MNP_SERVICE_DATA  *MnpServiceData,
  IN BOOLEAN           IsConfigUpdate,
  IN BOOLEAN           EnableSystemPoll
  );

STATIC
EFI_STATUS
MnpStop (
  IN MNP_SERVICE_DATA  *MnpServiceData
  );

STATIC
EFI_STATUS
MnpConfigReceiveFilters (
  IN MNP_SERVICE_DATA  *MnpServiceData
  );

STATIC
EFI_STATUS
MnpGroupOpAddCtrlBlk (
  IN MNP_INSTANCE_DATA        *Instance,
  IN MNP_GROUP_CONTROL_BLOCK  *CtrlBlk,
  IN MNP_GROUP_ADDRESS        *GroupAddress OPTIONAL,
  IN EFI_MAC_ADDRESS          *MacAddress,
  IN UINT32                   HwAddressSize
  );

STATIC
BOOLEAN
MnpGroupOpDelCtrlBlk (
  IN MNP_INSTANCE_DATA        *Instance,
  IN MNP_GROUP_CONTROL_BLOCK  *CtrlBlk
  );

STATIC
EFI_STATUS
MnpAddFreeNbuf (
  IN MNP_SERVICE_DATA  *MnpServiceData,
  IN UINTN             Count
  )
/*++

Routine Description:

  Add some NET_BUF into MnpServiceData->FreeNbufQue. The buffer length of
  the NET_BUF is specified by MnpServiceData->BufferLength.

Arguments:

  MnpServiceData - Pointer to the MNP_SERVICE_DATA.
  Count          - Number of NET_BUFFERs to add.

Returns:

  EFI_SUCCESS          - The specified amount of NET_BUFs are allocated and 
                         added into MnpServiceData->FreeNbufQue.
  EFI_OUT_OF_RESOURCES - Failed to allocate a NET_BUF structure.

--*/
{
  EFI_STATUS  Status;
  UINTN       Index;
  NET_BUF     *Nbuf;

  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);
  ASSERT ((Count > 0) && (MnpServiceData->BufferLength > 0));

  Status = EFI_SUCCESS;

  for (Index = 0; Index < Count; Index++) {

    Nbuf = NetbufAlloc (MnpServiceData->BufferLength);
    if (Nbuf == NULL) {

      MNP_DEBUG_ERROR (("MnpAddFreeNbuf: NetBufAlloc failed.\n"));
      Status = EFI_OUT_OF_RESOURCES;
      break;
    }

    NetbufQueAppend (&MnpServiceData->FreeNbufQue, Nbuf);
  }

  MnpServiceData->NbufCnt += Index;

  return Status;
}

NET_BUF *
MnpAllocNbuf (
  IN MNP_SERVICE_DATA  *MnpServiceData
  )
/*++

Routine Description:

  Allocate a free NET_BUF from MnpServiceData->FreeNbufQue. If there is none
  in the queue, first try to allocate some and add them into the queue, then
  fetch the NET_BUF from the updated FreeNbufQue.

Arguments:

  MnpServiceData - Pointer to the MNP_SERVICE_DATA.

Returns:

  Pointer to the allocated free NET_BUF structure, if NULL the operation is failed.

--*/
{
  EFI_STATUS    Status;
  NET_BUF_QUEUE *FreeNbufQue;
  NET_BUF       *Nbuf;

  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);

  FreeNbufQue = &MnpServiceData->FreeNbufQue;

  //
  // Check whether there are available buffers, or else try to add some.
  //
  if (FreeNbufQue->BufNum == 0) {

    if ((MnpServiceData->NbufCnt + MNP_NET_BUFFER_INCREASEMENT) > MNP_MAX_NET_BUFFER_NUM) {

      MNP_DEBUG_ERROR (
        ("MnpAllocNbuf: The maximum NET_BUF size is reached ""for MNP driver instance %p.\n",
        MnpServiceData)
        );
      return NULL;
    }

    Status = MnpAddFreeNbuf (MnpServiceData, MNP_NET_BUFFER_INCREASEMENT);
    if (EFI_ERROR (Status)) {

      MNP_DEBUG_ERROR (
        ("MnpAllocNbuf: Failed to add NET_BUFs into the ""FreeNbufQue, %r.\n",
        Status)
        );
      //
      // Don't return NULL, perhaps MnpAddFreeNbuf does add some NET_BUFs but
      // the amount is less than MNP_NET_BUFFER_INCREASEMENT.
      //
    }
  }

  Nbuf = NetbufQueRemove (FreeNbufQue);

  //
  // Increase the RefCnt.
  //
  if (Nbuf != NULL) {
    NET_GET_REF (Nbuf);
  }

  return Nbuf;
}

VOID
MnpFreeNbuf (
  IN MNP_SERVICE_DATA  *MnpServiceData,
  IN NET_BUF           *Nbuf
  )
/*++

Routine Description:

  Try to reclaim the Nbuf into the buffer pool.

Arguments:

  MnpServiceData - Pointer to the mnp service context data.
  Nbuf           - Pointer to the NET_BUF to free.

Returns:

  None.

--*/
{
  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);
  ASSERT (Nbuf->RefCnt > 1);

  NET_PUT_REF (Nbuf);

  if (Nbuf->RefCnt == 1) {
    //
    // Trim all buffer contained in the Nbuf, then append it to the NbufQue.
    //
    NetbufTrim (Nbuf, Nbuf->TotalSize, NET_BUF_TAIL);
    NetbufQueAppend (&MnpServiceData->FreeNbufQue, Nbuf);
  }
}

EFI_STATUS
MnpInitializeServiceData (
  IN MNP_SERVICE_DATA  *MnpServiceData,
  IN EFI_HANDLE        ImageHandle,
  IN EFI_HANDLE        ControllerHandle
  )
/*++

Routine Description:

  Initialize the mnp service context data.

Arguments:

  MnpServiceData - Pointer to the mnp service context data.
  Snp            - Pointer to the simple network protocol.

Returns:

  EFI_SUCCESS - The mnp service context is initialized.
  Other       - Some error occurs.

--*/
{
  EFI_STATUS                   Status;
  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp;
  EFI_SIMPLE_NETWORK_MODE      *SnpMode;

  MnpServiceData->Signature = MNP_SERVICE_DATA_SIGNATURE;

  MnpServiceData->ControllerHandle = ControllerHandle;

  //
  // Copy the ServiceBinding structure.
  //
  MnpServiceData->ServiceBinding = mMnpServiceBindingProtocol;

  //
  // Open the Simple Network protocol.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiSimpleNetworkProtocolGuid,
                  (VOID **) &Snp,
                  ImageHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Get MTU from Snp.
  //
  SnpMode             = Snp->Mode;
  MnpServiceData->Snp = Snp;
  MnpServiceData->Mtu = SnpMode->MaxPacketSize;

  //
  // Initialize the lists.
  //
  NetListInit (&MnpServiceData->GroupAddressList);
  NetListInit (&MnpServiceData->ChildrenList);

  //
  // Initialize the locks.
  //
  NET_GLOBAL_LOCK_INIT (&MnpServiceData->ChildrenListLock);
  NET_GLOBAL_LOCK_INIT (&MnpServiceData->GroupAddressLock);
  NET_GLOBAL_LOCK_INIT (&MnpServiceData->TxLock);

  //
  // Get the buffer length used to allocate NET_BUF to hold data received
  // from SNP. Do this before fill the FreeNetBufQue.
  //
  MnpServiceData->BufferLength = MnpServiceData->Mtu + SnpMode->MediaHeaderSize + NET_ETHER_FCS_SIZE;

  //
  // Initialize the FreeNetBufQue and pre-allocate some NET_BUFs.
  //
  NetbufQueInit (&MnpServiceData->FreeNbufQue);
  Status = MnpAddFreeNbuf (MnpServiceData, MNP_INIT_NET_BUFFER_NUM);
  if (EFI_ERROR (Status)) {

    MNP_DEBUG_ERROR (("MnpInitializeServiceData: MnpAddFreeNbuf failed, %r.\n", Status));
    goto ERROR;
  }
  //
  // Get one NET_BUF from the FreeNbufQue for rx cache.
  //
  MnpServiceData->RxNbufCache = MnpAllocNbuf (MnpServiceData);
  NetbufAllocSpace (
    MnpServiceData->RxNbufCache,
    MnpServiceData->BufferLength,
    NET_BUF_TAIL
    );

  //
  // Allocate buffer pool for tx.
  //
  MnpServiceData->TxBuf = NetAllocatePool (MnpServiceData->Mtu + SnpMode->MediaHeaderSize);
  if (MnpServiceData->TxBuf == NULL) {

    MNP_DEBUG_ERROR (("MnpInitializeServiceData: NetAllocatePool failed.\n"));
    Status = EFI_OUT_OF_RESOURCES;

    goto ERROR;
  }

  //
  // Create the system poll timer.
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL | EFI_EVENT_TIMER,
                  NET_TPL_SYSTEM_POLL,
                  MnpSystemPoll,
                  MnpServiceData,
                  &MnpServiceData->PollTimer
                  );
  if (EFI_ERROR (Status)) {

    MNP_DEBUG_ERROR (("MnpInitializeServiceData: CreateEvent for poll timer failed.\n"));
    goto ERROR;
  }

  //
  // Create the timer for packet timeout check.
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL | EFI_EVENT_TIMER,
                  NET_TPL_EVENT,
                  MnpCheckPacketTimeout,
                  MnpServiceData,
                  &MnpServiceData->TimeoutCheckTimer
                  );
  if (EFI_ERROR (Status)) {

    MNP_DEBUG_ERROR (("MnpInitializeServiceData: CreateEvent for packet ""timeout check failed.\n"));
    goto ERROR;
  }

  //
  // Create the timer for tx timeout check.
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_TIMER,
                  NET_TPL_SLOW_TIMER,
                  NULL,
                  NULL,
                  &MnpServiceData->TxTimeoutEvent
                  );
  if (EFI_ERROR (Status)) {

    MNP_DEBUG_ERROR (("MnpInitializeServiceData: CreateEvent for tx ""timeout event failed.\n"));
  }

ERROR:

  if (EFI_ERROR (Status)) {
    //
    // Free the dynamic allocated resources if necessary.
    //
    if (MnpServiceData->TimeoutCheckTimer != NULL) {

      gBS->CloseEvent (MnpServiceData->TimeoutCheckTimer);
    }

    if (MnpServiceData->PollTimer != NULL) {

      gBS->CloseEvent (MnpServiceData->PollTimer);
    }

    if (MnpServiceData->TxBuf != NULL) {

      NetFreePool (MnpServiceData->TxBuf);
    }

    if (MnpServiceData->RxNbufCache != NULL) {

      MnpFreeNbuf (MnpServiceData, MnpServiceData->RxNbufCache);
    }

    if (MnpServiceData->FreeNbufQue.BufNum != 0) {

      NetbufQueFlush (&MnpServiceData->FreeNbufQue);
    }
  }

  return Status;
}

VOID
MnpFlushServiceData (
  MNP_SERVICE_DATA  *MnpServiceData
  )
/*++

Routine Description:

  Flush the mnp service context data.

Arguments:

  MnpServiceData - Pointer to the mnp service context data.

Returns:

  None.

--*/
{
  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);

  //
  // The GroupAddressList must be empty.
  //
  ASSERT (NetListIsEmpty (&MnpServiceData->GroupAddressList));

  //
  // Close the event.
  //
  gBS->CloseEvent (&MnpServiceData->TxTimeoutEvent);
  gBS->CloseEvent (&MnpServiceData->TimeoutCheckTimer);
  gBS->CloseEvent (&MnpServiceData->PollTimer);

  //
  // Free the tx buffer.
  //
  NetFreePool (MnpServiceData->TxBuf);

  //
  // Free the RxNbufCache.
  //
  MnpFreeNbuf (MnpServiceData, MnpServiceData->RxNbufCache);

  //
  // Flush the FreeNbufQue.
  //
  MnpServiceData->NbufCnt -= MnpServiceData->FreeNbufQue.BufNum;
  NetbufQueFlush (&MnpServiceData->FreeNbufQue);

  DEBUG_CODE (

    if (MnpServiceData->NbufCnt != 0) {

    MNP_DEBUG_WARN (("MnpFlushServiceData: Memory leak, ""MnpServiceData->NbufCnt != 0.\n"));
  }
  );
}

VOID
MnpInitializeInstanceData (
  IN MNP_SERVICE_DATA   *MnpServiceData,
  IN MNP_INSTANCE_DATA  *Instance
  )
/*++

Routine Description:

  Initialize the mnp instance context data.

Arguments:

  MnpServiceData - Pointer to the mnp service context data.
  Instance       - Pointer to the mnp instance context data to initialize.

Returns:

  None.

--*/
{
  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);
  ASSERT (Instance != NULL);

  //
  // Set the signature.
  //
  Instance->Signature = MNP_INSTANCE_DATA_SIGNATURE;

  //
  // Copy the MNP Protocol interfaces from the template.
  //
  Instance->ManagedNetwork = mMnpProtocolTemplate;

  //
  // Copy the default config data.
  //
  Instance->ConfigData = mMnpDefaultConfigData;

  //
  // Initialize the locks.
  //
  NET_GLOBAL_LOCK_INIT (&Instance->RxLock);

  //
  // Initialize the lists.
  //
  NetListInit (&Instance->GroupCtrlBlkList);
  NetListInit (&Instance->RcvdPacketQueue);
  NetListInit (&Instance->RxDeliveredPacketQueue);

  //
  // Initialize the RxToken Map.
  //
  NetMapInit (&Instance->RxTokenMap);

  //
  // Save the MnpServiceData info.
  //
  Instance->MnpServiceData = MnpServiceData;
}

EFI_STATUS
MnpTokenExist (
  IN NET_MAP       *Map,
  IN NET_MAP_ITEM  *Item,
  IN VOID          *Arg
  )
/*++

Routine Description:

  Check whether the token specified by Arg maches the token in Item.

Arguments:

  Map   - Pointer to the NET_MAP.
  Item  - Pointer to the NET_MAP_ITEM
  Arg   - Pointer to the Arg, it's a pointer to the token to check.

Returns:

  EFI_SUCCESS       - The token specified by Arg is different from the token in Item.
  EFI_ACCESS_DENIED - The token specified by Arg is the same as that in Item.

--*/
{
  EFI_MANAGED_NETWORK_COMPLETION_TOKEN  *Token;
  EFI_MANAGED_NETWORK_COMPLETION_TOKEN  *TokenInItem;

  Token       = (EFI_MANAGED_NETWORK_COMPLETION_TOKEN *) Arg;
  TokenInItem = (EFI_MANAGED_NETWORK_COMPLETION_TOKEN *) Item->Key;

  if ((Token == TokenInItem) || (Token->Event == TokenInItem->Event)) {
    //
    // The token is the same either the two tokens equals or the Events in
    // the two tokens are the same.
    //
    return EFI_ACCESS_DENIED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
MnpCancelTokens (
  IN NET_MAP       *Map,
  IN NET_MAP_ITEM  *Item,
  IN VOID          *Arg
  )
/*++

Routine Description:

  Cancel the token specified by Arg if it matches the token in Item.

Arguments:

  Map   - Pointer to the NET_MAP.
  Item  - Pointer to the NET_MAP_ITEM
  Arg   - Pointer to the Arg, it's a pointer to the token to cancel.

Returns:

  EFI_SUCCESS - The Arg is NULL, and the token in Item is cancelled, or
                the Arg isn't NULL, and the token in Item is different from
                the Arg.
  EFI_ABORTED - The Arg isn't NULL, the token in Item mathces the Arg, and the
                token is cancelled.

--*/
{
  EFI_MANAGED_NETWORK_COMPLETION_TOKEN  *TokenToCancel;

  if ((Arg != NULL) && (Item->Key != Arg)) {
    //
    // The token in Item is not the token specified by Arg.
    //
    return EFI_SUCCESS;
  }

  TokenToCancel         = (EFI_MANAGED_NETWORK_COMPLETION_TOKEN *) Item->Key;

  //
  // Cancel this token with status set to EFI_ABORTED.
  //
  TokenToCancel->Status = EFI_ABORTED;
  gBS->SignalEvent (TokenToCancel->Event);

  //
  // Remove the item from the map.
  //
  NetMapRemoveItem (Map, Item, NULL);

  if (Arg != NULL) {
    //
    // Only abort the token specified by Arg if Arg isn't NULL.
    //
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MnpStartSnp (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  )
/*++

Routine Description:

  Start and initialize the simple network.

Arguments:

  Snp - Pointer to the simple network protocol.

Returns:

  EFI_SUCCESS - The simple network protocol is started.
  Other       - Some error occurs.

--*/
{
  EFI_STATUS  Status;

  ASSERT (Snp != NULL);

  //
  // Start the simple network.
  //
  Status = Snp->Start (Snp);

  if (!EFI_ERROR (Status)) {
    //
    // Initialize the simple network.
    //
    Status  = Snp->Initialize (Snp, 0, 0);
  }

  return Status;
}

STATIC
EFI_STATUS
MnpStopSnp (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  )
/*++

Routine Description:

  Stop the simple network.

Arguments:

  Snp - Pointer to the simple network protocol.

Returns:

  EFI_SUCCESS - The simple network is stopped.
  Other       - Some error occurs.
  
--*/
{
  EFI_STATUS  Status;

  ASSERT (Snp != NULL);

  //
  // Shut down the simple network.
  //
  Status = Snp->Shutdown (Snp);

  if (!EFI_ERROR (Status)) {
    //
    // Stop the simple network.
    //
    Status = Snp->Stop (Snp);
  }

  return Status;
}

STATIC
EFI_STATUS
MnpStart (
  IN MNP_SERVICE_DATA  *MnpServiceData,
  IN BOOLEAN           IsConfigUpdate,
  IN BOOLEAN           EnableSystemPoll
  )
/*++

Routine Description:

  Start the managed network, this function is called when one instance is configured
  or reconfigured.

Arguments:

  MnpServiceData   - Pointer to the mnp service context data.
  IsConfigUpdate   - The instance is reconfigured or it's the first time 
                     the instanced is configured.
  EnableSystemPoll - Enable the system polling or not.

Returns:

  EFI_SUCCESS - The managed network is started and some configuration is updated.
  Other       - Some error occurs.

--*/
{
  EFI_STATUS      Status;
  EFI_TIMER_DELAY TimerOpType;

  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);

  Status = EFI_SUCCESS;

  if (!IsConfigUpdate) {
    //
    // If it's not a configuration update, increase the configured children number.
    //
    MnpServiceData->ConfiguredChildrenNumber++;

    if (MnpServiceData->ConfiguredChildrenNumber == 1) {
      //
      // It's the first configured child, start the simple network.
      //
      Status = MnpStartSnp (MnpServiceData->Snp);
      if (EFI_ERROR (Status)) {

        MNP_DEBUG_ERROR (("MnpStart: MnpStartSnp failed, %r.\n", Status));
        goto ErrorExit;
      }

      //
      // Start the timeout timer.
      //
      Status = gBS->SetTimer (
                      MnpServiceData->TimeoutCheckTimer,
                      TimerPeriodic,
                      MNP_TIMEOUT_CHECK_INTERVAL
                      );
      if (EFI_ERROR (Status)) {

        MNP_DEBUG_ERROR (
          ("MnpStart, gBS->SetTimer for TimeoutCheckTimer ""%r.\n",
          Status)
          );
        goto ErrorExit;
      }
    }
  }

  if (MnpServiceData->EnableSystemPoll ^ EnableSystemPoll) {
    //
    // The EnableSystemPoll differs with the current state, disable or enable
    // the system poll.
    //
    TimerOpType = EnableSystemPoll ? TimerPeriodic : TimerCancel;

    Status      = gBS->SetTimer (MnpServiceData->PollTimer, TimerOpType, MNP_SYS_POLL_INTERVAL);
    if (EFI_ERROR (Status)) {

      MNP_DEBUG_ERROR (("MnpStart: gBS->SetTimer for PollTimer failed, %r.\n", Status));
      goto ErrorExit;
    }

    MnpServiceData->EnableSystemPoll = EnableSystemPoll;
  }

  //
  // Change the receive filters if need.
  //
  Status = MnpConfigReceiveFilters (MnpServiceData);

ErrorExit:

  return Status;
}

STATIC
EFI_STATUS
MnpStop (
  IN MNP_SERVICE_DATA  *MnpServiceData
  )
/*++

Routine Description:

  Stop the managed network.

Arguments:

  MnpServiceData - Pointer to the mnp service context data.

Returns:

  EFI_SUCCESS - The managed network is stopped.
  Other       - Some error occurs.

--*/
{
  EFI_STATUS  Status;

  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);
  ASSERT (MnpServiceData->ConfiguredChildrenNumber > 0);

  //
  // Configure the receive filters.
  //
  MnpConfigReceiveFilters (MnpServiceData);

  //
  // Decrease the children number.
  //
  MnpServiceData->ConfiguredChildrenNumber--;

  if (MnpServiceData->ConfiguredChildrenNumber > 0) {
    //
    // If there are other configured chilren, return and keep the timers and
    // simple network unchanged.
    //
    return EFI_SUCCESS;
  }

  //
  // No configured children now.  
  //

  if (MnpServiceData->EnableSystemPoll) {
    //
    //  The system poll in on, cancel the poll timer.
    //
    Status  = gBS->SetTimer (MnpServiceData->PollTimer, TimerCancel, 0);
    MnpServiceData->EnableSystemPoll = FALSE;
  }

  //
  // Cancel the timeout timer.
  //
  Status  = gBS->SetTimer (MnpServiceData->TimeoutCheckTimer, TimerCancel, 0);

  //
  // Stop the simple network.
  //
  Status  = MnpStopSnp (MnpServiceData->Snp);

  return Status;
}

VOID
MnpFlushRcvdDataQueue (
  IN MNP_INSTANCE_DATA  *Instance
  )
/*++

Routine Description:

  Flush the instance's received data.

Arguments:

  Instance - Pointer to the mnp instance context data.

Returns:

  None.

--*/
{
  MNP_RXDATA_WRAP *RxDataWrap;

  NET_CHECK_SIGNATURE (Instance, MNP_INSTANCE_DATA_SIGNATURE);

  if (EFI_ERROR (NET_TRYLOCK (&Instance->RxLock))) {
    return ;
  }

  while (!NetListIsEmpty (&Instance->RcvdPacketQueue)) {
    //
    // Remove all the Wraps.
    //
    RxDataWrap = NET_LIST_HEAD (&Instance->RcvdPacketQueue, MNP_RXDATA_WRAP, WrapEntry);

    //
    // Recycle the RxDataWrap.
    //
    MnpRecycleRxData (NULL, (VOID *) RxDataWrap);
    Instance->RcvdPacketQueueSize--;
  }

  ASSERT (Instance->RcvdPacketQueueSize == 0);

  NET_UNLOCK (&Instance->RxLock);
}

EFI_STATUS
MnpConfigureInstance (
  IN MNP_INSTANCE_DATA                *Instance,
  IN EFI_MANAGED_NETWORK_CONFIG_DATA  *ConfigData OPTIONAL
  )
/*++

Routine Description:

  Configure the Instance using ConfigData.

Arguments:

  Instance   - Pointer to the mnp instance context data.
  ConfigData - Pointer to the configuration data used to configure the isntance.

Returns:

  EFI_SUCCESS     - The Instance is configured.
  EFI_UNSUPPORTED - EnableReceiveTimestamps is on and the implementation doesn't
                      support it.
  Other           - Some error occurs.

--*/
{
  EFI_STATUS                      Status;
  MNP_SERVICE_DATA                *MnpServiceData;
  EFI_MANAGED_NETWORK_CONFIG_DATA *OldConfigData;
  EFI_MANAGED_NETWORK_CONFIG_DATA *NewConfigData;
  BOOLEAN                         IsConfigUpdate;

  NET_CHECK_SIGNATURE (Instance, MNP_INSTANCE_DATA_SIGNATURE);

  if ((ConfigData != NULL) && ConfigData->EnableReceiveTimestamps) {
    //
    // Don't support timestamp.
    //
    return EFI_UNSUPPORTED;
  }

  Status          = EFI_SUCCESS;

  MnpServiceData  = Instance->MnpServiceData;
  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);

  IsConfigUpdate  = (BOOLEAN) ((Instance->Configured) && (ConfigData != NULL));

  OldConfigData   = &Instance->ConfigData;
  NewConfigData   = ConfigData;
  if (NewConfigData == NULL) {
    //
    // Restore back the default config data if a reset of this instance
    // is required.
    //
    NewConfigData = &mMnpDefaultConfigData;
  }

  //
  // Reset the instance's receive filter.
  //
  Instance->ReceiveFilter = 0;

  //
  // Clear the receive counters according to the old ConfigData.
  //
  if (OldConfigData->EnableUnicastReceive) {
    MnpServiceData->UnicastCount--;
  }

  if (OldConfigData->EnableMulticastReceive) {
    MnpServiceData->MulticastCount--;
  }

  if (OldConfigData->EnableBroadcastReceive) {
    MnpServiceData->BroadcastCount--;
  }

  if (OldConfigData->EnablePromiscuousReceive) {
    MnpServiceData->PromiscuousCount--;
  }

  //
  // Set the receive filter counters and the receive filter of the
  // instance according to the new ConfigData.
  //
  if (NewConfigData->EnableUnicastReceive) {
    MnpServiceData->UnicastCount++;
    Instance->ReceiveFilter |= MNP_RECEIVE_UNICAST;
  }

  if (NewConfigData->EnableMulticastReceive) {
    MnpServiceData->MulticastCount++;
  }

  if (NewConfigData->EnableBroadcastReceive) {
    MnpServiceData->BroadcastCount++;
    Instance->ReceiveFilter |= MNP_RECEIVE_BROADCAST;
  }

  if (NewConfigData->EnablePromiscuousReceive) {
    MnpServiceData->PromiscuousCount++;
  }

  if (EFI_ERROR (NET_TRYLOCK (&Instance->RxLock))) {

    return EFI_ACCESS_DENIED;
  }

  if (OldConfigData->FlushQueuesOnReset) {

    MnpFlushRcvdDataQueue (Instance);
  }

  if (ConfigData == NULL) {

    NetMapIterate (&Instance->RxTokenMap, MnpCancelTokens, NULL);
  }

  NET_UNLOCK (&Instance->RxLock);

  if (!NewConfigData->EnableMulticastReceive) {

    if (EFI_ERROR (NET_TRYLOCK (&MnpServiceData->GroupAddressLock))) {

      return EFI_ACCESS_DENIED;
    }

    MnpGroupOp (Instance, FALSE, NULL, NULL);

    NET_UNLOCK (&MnpServiceData->GroupAddressLock);
  }

  //
  // Save the new configuration data.
  //
  *OldConfigData        = *NewConfigData;

  Instance->Configured  = (BOOLEAN) (ConfigData != NULL);

  if (Instance->Configured) {
    //
    // The instance is configured, start the Mnp.
    //
    Status = MnpStart (
              MnpServiceData,
              IsConfigUpdate,
              !NewConfigData->DisableBackgroundPolling
              );
  } else {
    //
    // The instance is changed to the unconfigured state, stop the Mnp.
    //
    Status = MnpStop (MnpServiceData);
  }

  return Status;
}

STATIC
EFI_STATUS
MnpConfigReceiveFilters (
  IN MNP_SERVICE_DATA  *MnpServiceData
  )
/*++

Routine Description:

  Configure the Snp receive filters according to the instances' receive filter
  settings.

Arguments:

  MnpServiceData - Pointer to the mnp service context data.

Returns:

  EFI_SUCCESS          - The receive filters is configured.
  EFI_OUT_OF_RESOURCES - The receive filters can't be configured due to lack of
                         memory resource.

--*/
{
  EFI_STATUS                  Status;
  EFI_SIMPLE_NETWORK_PROTOCOL *Snp;
  EFI_MAC_ADDRESS             *MCastFilter;
  UINT32                      MCastFilterCnt;
  UINT32                      EnableFilterBits;
  UINT32                      DisableFilterBits;
  BOOLEAN                     ResetMCastFilters;
  NET_LIST_ENTRY              *Entry;
  UINT32                      Index;
  MNP_GROUP_ADDRESS           *GroupAddress;

  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);

  Snp = MnpServiceData->Snp;

  //
  // Initialize the enable filter and disable filter.
  //
  EnableFilterBits  = 0;
  DisableFilterBits = Snp->Mode->ReceiveFilterMask;

  if (MnpServiceData->UnicastCount != 0) {
    //
    // Enable unicast if any instance wants to receive unicast.
    //
    EnableFilterBits |= EFI_SIMPLE_NETWORK_RECEIVE_UNICAST;
  }

  if (MnpServiceData->BroadcastCount != 0) {
    //
    // Enable broadcast if any instance wants to receive broadcast.
    //
    EnableFilterBits |= EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST;
  }

  MCastFilter       = NULL;
  MCastFilterCnt    = 0;
  ResetMCastFilters = TRUE;

  if ((MnpServiceData->MulticastCount != 0) && (MnpServiceData->GroupAddressCount != 0)) {
    //
    // There are instances configured to receive multicast and already some group
    // addresses are joined.
    //

    ResetMCastFilters = FALSE;

    if (MnpServiceData->GroupAddressCount <= Snp->Mode->MaxMCastFilterCount) {
      //
      // The joind group address is less than simple network's maximum count.
      // Just configure the snp to do the multicast filtering.
      //

      EnableFilterBits |= EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST;

      //
      // Allocate pool for the mulicast addresses.
      //
      MCastFilterCnt  = MnpServiceData->GroupAddressCount;
      MCastFilter     = NetAllocatePool (sizeof (EFI_MAC_ADDRESS) * MCastFilterCnt);
      if (MCastFilter == NULL) {

        MNP_DEBUG_ERROR (("MnpConfigReceiveFilters: Failed to allocate memory"" resource for MCastFilter.\n"));
        return EFI_OUT_OF_RESOURCES;
      }

      //
      // Fill the multicast HW address buffer.
      //
      Index = 0;
      NET_LIST_FOR_EACH (Entry, &MnpServiceData->GroupAddressList) {

        GroupAddress            = NET_LIST_USER_STRUCT (Entry, MNP_GROUP_ADDRESS, AddrEntry);
        *(MCastFilter + Index)  = GroupAddress->Address;
        Index++;

        ASSERT (Index <= MCastFilterCnt);
      }
    } else {
      //
      // The maximum multicast is reached, set the filter to be promiscuous
      // multicast.
      //

      if (Snp->Mode->ReceiveFilterMask & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST) {
        EnableFilterBits |= EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;
      } else {
        //
        // Either MULTICAST or PROMISCUOUS_MULTICAST is not supported by Snp,
        // set the NIC to be promiscuous although this will tremendously degrade
        // the performance.
        //
        EnableFilterBits |= EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS;
      }
    }
  }

  if (MnpServiceData->PromiscuousCount != 0) {
    //
    // Enable promiscuous if any instance wants to receive promiscuous.
    //
    EnableFilterBits |= EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS;
  }

  //
  // Set the disable filter.
  //
  DisableFilterBits ^= EnableFilterBits;

  //
  // Configure the receive filters of SNP.
  //
  Status = Snp->ReceiveFilters (
                  Snp,
                  EnableFilterBits,
                  DisableFilterBits,
                  ResetMCastFilters,
                  MCastFilterCnt,
                  MCastFilter
                  );
  DEBUG_CODE (
    if (EFI_ERROR (Status)) {

    MNP_DEBUG_ERROR (
      ("MnpConfigReceiveFilters: Snp->ReceiveFilters ""failed, %r.\n",
      Status)
      );
  }
  );

  if (MCastFilter != NULL) {
    //
    // Free the buffer used to hold the group addresses.
    //
    NetFreePool (MCastFilter);
  }

  return Status;
}

STATIC
EFI_STATUS
MnpGroupOpAddCtrlBlk (
  IN MNP_INSTANCE_DATA        *Instance,
  IN MNP_GROUP_CONTROL_BLOCK  *CtrlBlk,
  IN MNP_GROUP_ADDRESS        *GroupAddress OPTIONAL,
  IN EFI_MAC_ADDRESS          *MacAddress,
  IN UINT32                   HwAddressSize
  )
/*++

Routine Description:

  Add a group address control block which controls the MacAddress for
  this instance.

Arguments:

  Instance      - Pointer to the mnp instance context data.
  CtrlBlk       - Pointer to the group address control block.
  GroupAddress  - Pointer to the group adress.
  MacAddress    - Pointer to the mac address.
  HwAddressSize - The hardware address size.

Returns:

  EFI_SUCCESS         - The group address control block is added.
  EFI_OUT_OF_RESOURCE - Failed due to lack of memory resources.

--*/
{
  MNP_SERVICE_DATA  *MnpServiceData;

  NET_CHECK_SIGNATURE (Instance, MNP_INSTANCE_DATA_SIGNATURE);

  MnpServiceData = Instance->MnpServiceData;
  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);

  if (GroupAddress == NULL) {

    ASSERT (MacAddress != NULL);

    //
    // Allocate a new GroupAddress to be added into MNP's GroupAddressList.
    //
    GroupAddress = NetAllocatePool (sizeof (MNP_GROUP_ADDRESS));
    if (GroupAddress == NULL) {

      MNP_DEBUG_ERROR (("MnpGroupOpFormCtrlBlk: Failed to allocate memory ""resource.\n"));

      return EFI_OUT_OF_RESOURCES;
    }

    GroupAddress->Address = *MacAddress;
    GroupAddress->RefCnt  = 0;
    NetListInsertTail (
      &MnpServiceData->GroupAddressList,
      &GroupAddress->AddrEntry
      );
    MnpServiceData->GroupAddressCount++;
  }

  //
  // Increase the RefCnt.
  //
  GroupAddress->RefCnt++;

  //
  // Add the CtrlBlk into the instance's GroupCtrlBlkList.
  //
  CtrlBlk->GroupAddress = GroupAddress;
  NetListInsertTail (&Instance->GroupCtrlBlkList, &CtrlBlk->CtrlBlkEntry);

  return EFI_SUCCESS;
}

STATIC
BOOLEAN
MnpGroupOpDelCtrlBlk (
  IN MNP_INSTANCE_DATA        *Instance,
  IN MNP_GROUP_CONTROL_BLOCK  *CtrlBlk
  )
/*++

Routine Description:

  Delete a group control block from the instance. If the controlled group address's
  reference count reaches zero, the group address is removed too.

Arguments:

  Instance - Pointer to the instance context data.
  CtrlBlk  - Pointer to the group control block to delete.

Returns:

  The group address controlled by the control block is no longer used or not.

--*/
{
  MNP_SERVICE_DATA  *MnpServiceData;
  MNP_GROUP_ADDRESS *GroupAddress;

  NET_CHECK_SIGNATURE (Instance, MNP_INSTANCE_DATA_SIGNATURE);

  MnpServiceData = Instance->MnpServiceData;
  NET_CHECK_SIGNATURE (MnpServiceData, MNP_SERVICE_DATA_SIGNATURE);

  //
  // Remove and free the CtrlBlk.
  //
  GroupAddress = CtrlBlk->GroupAddress;
  NetListRemoveEntry (&CtrlBlk->CtrlBlkEntry);
  NetFreePool (CtrlBlk);

  ASSERT (GroupAddress->RefCnt > 0);

  //
  // Count down the RefCnt.
  //
  GroupAddress->RefCnt--;

  if (GroupAddress->RefCnt == 0) {
    //
    // Free this GroupAddress entry if no instance uses it.
    //
    MnpServiceData->GroupAddressCount--;
    NetListRemoveEntry (&GroupAddress->AddrEntry);
    NetFreePool (GroupAddress);

    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
MnpGroupOp (
  IN MNP_INSTANCE_DATA        *Instance,
  IN BOOLEAN                  JoinFlag,
  IN EFI_MAC_ADDRESS          *MacAddress OPTIONAL,
  IN MNP_GROUP_CONTROL_BLOCK  *CtrlBlk OPTIONAL
  )
/*++

Routine Description:

  Do the group operations for this instance.

Arguments:

  Instance   - Pointer to the instance context data.
  JoinFlag   - Set to TRUE to join a group. Set to TRUE to leave a group/groups.
  MacAddress - Pointer to the group address to join or leave.
  CtrlBlk    - Pointer to the group control block if JoinFlag if FALSE.

Returns:

  EFI_SUCCESS - The group operation finished.
  Other       - Some error occurs.

--*/
{
  MNP_SERVICE_DATA        *MnpServiceData;
  NET_LIST_ENTRY          *Entry;
  NET_LIST_ENTRY          *NextEntry;
  MNP_GROUP_ADDRESS       *GroupAddress;
  EFI_SIMPLE_NETWORK_MODE *SnpMode;
  MNP_GROUP_CONTROL_BLOCK *NewCtrlBlk;
  EFI_STATUS              Status;
  BOOLEAN                 AddressExist;
  BOOLEAN                 NeedUpdate;

  NET_CHECK_SIGNATURE (Instance, MNP_INSTANCE_DATA_SIGNATURE);

  MnpServiceData  = Instance->MnpServiceData;
  SnpMode         = MnpServiceData->Snp->Mode;

  if (JoinFlag) {
    //
    // A new gropu address is to be added.
    //

    GroupAddress  = NULL;
    AddressExist  = FALSE;

    //
    // Allocate memory for the control block.
    //
    NewCtrlBlk    = NetAllocatePool (sizeof (MNP_GROUP_CONTROL_BLOCK));
    if (NewCtrlBlk == NULL) {

      MNP_DEBUG_ERROR (("MnpGroupOp: Failed to allocate memory resource.\n"));
      return EFI_OUT_OF_RESOURCES;
    }

    NET_LIST_FOR_EACH (Entry, &MnpServiceData->GroupAddressList) {
      //
      // Check whether the MacAddress is already joined by other instances.
      //
      GroupAddress = NET_LIST_USER_STRUCT (Entry, MNP_GROUP_ADDRESS, AddrEntry);
      if (0 == NetCompareMem (
                MacAddress,
                &GroupAddress->Address,
                SnpMode->HwAddressSize
                )) {

        AddressExist = TRUE;
        break;
      }
    }

    if (!AddressExist) {
      GroupAddress = NULL;
    }

    //
    // Add the GroupAddress for this instance.
    //
    Status = MnpGroupOpAddCtrlBlk (
              Instance,
              NewCtrlBlk,
              GroupAddress,
              MacAddress,
              SnpMode->HwAddressSize
              );
    if (EFI_ERROR (Status)) {

      return Status;
    }

    NeedUpdate = TRUE;
  } else {

    if (MacAddress != NULL) {

      ASSERT (CtrlBlk != NULL);

      //
      // Leave the specific multicast mac address.
      //
      NeedUpdate = MnpGroupOpDelCtrlBlk (Instance, CtrlBlk);
    } else {
      //
      // Leave all multicast mac addresses.
      //
      NeedUpdate = FALSE;

      NET_LIST_FOR_EACH_SAFE (Entry, NextEntry, &Instance->GroupCtrlBlkList) {

        NewCtrlBlk = NET_LIST_USER_STRUCT (
                      Entry,
                      MNP_GROUP_CONTROL_BLOCK,
                      CtrlBlkEntry
                      );
        //
        // Update is required if the group address left is no longer used
        // by other instances.
        //
        NeedUpdate = MnpGroupOpDelCtrlBlk (Instance, NewCtrlBlk);
      }
    }
  }

  Status = EFI_SUCCESS;

  if (NeedUpdate) {
    //
    // Reconfigure the receive filters if necessary.
    //
    Status = MnpConfigReceiveFilters (MnpServiceData);
  }

  return Status;
}
