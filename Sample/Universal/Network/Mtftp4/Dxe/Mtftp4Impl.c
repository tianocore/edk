/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  Mtftp4Impl.c
 
Abstract:

  Interface routine for Mtftp4

--*/

#include "Mtftp4Impl.h"

STATIC
EFI_STATUS
EFIAPI
EfiMtftp4ReadFile (
  IN EFI_MTFTP4_PROTOCOL    *This,
  IN EFI_MTFTP4_TOKEN       *Token
  );

STATIC
EFI_STATUS
EFIAPI
EfiMtftp4GetModeData (
  IN EFI_MTFTP4_PROTOCOL    *This,
  OUT EFI_MTFTP4_MODE_DATA  *ModeData
  )
/*++

Routine Description:

  Get the current operation parameter for the MTFTP session

Arguments:

  This      - The MTFTP protocol instance
  ModeData  - The MTFTP mode data

Returns:

  EFI_INVALID_PARAMETER - This or ModeData is NULL
  EFI_SUCCESS           - The operation parameter is saved in ModeData

--*/
{
  MTFTP4_PROTOCOL           *Instance;

  if ((This == NULL) || (ModeData == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Instance                         = MTFTP4_PROTOCOL_FROM_THIS (This);
  ModeData->ConfigData             = Instance->Config;
  ModeData->SupportedOptionCount   = MTFTP4_SUPPORTED_OPTIONS;
  ModeData->SupportedOptoins       = mMtftp4SupportedOptions;
  ModeData->UnsupportedOptionCount = 0;
  ModeData->UnsupportedOptoins     = NULL;
  return EFI_SUCCESS;
}

VOID
Mtftp4CleanOperation (
  IN MTFTP4_PROTOCOL        *Instance,
  IN EFI_STATUS             Result
  )
/*++

Routine Description:

  Clean up the MTFTP session to get ready for new operation.

Arguments:

  Instance  - The MTFTP session to clean up
  Result    - The result to return to the caller who initiated the operation.

Returns:

  None

--*/
{
  NET_LIST_ENTRY            *Entry;
  NET_LIST_ENTRY            *Next;
  MTFTP4_BLOCK_RANGE        *Block;
  EFI_MTFTP4_TOKEN          *Token;

  //
  // Free various resources.
  //
  Token = Instance->Token;
  
  if (Token != NULL) {
    Token->Status = Result;

    if (Token->Event != NULL) {
      gBS->SignalEvent (Token->Event);
    }

    Instance->Token = NULL;
  }

  ASSERT (Instance->UnicastPort != NULL);
  UdpIoCleanPort (Instance->UnicastPort);

  if (Instance->LastPacket != NULL) {
    NetbufFree (Instance->LastPacket);
    Instance->LastPacket = NULL;
  }

  if (Instance->McastUdpPort != NULL) {
    UdpIoFreePort (Instance->McastUdpPort);
    Instance->McastUdpPort = NULL;
  }

  NET_LIST_FOR_EACH_SAFE (Entry, Next, &Instance->Blocks) {
    Block = NET_LIST_USER_STRUCT (Entry, MTFTP4_BLOCK_RANGE, Link);
    NetListRemoveEntry (Entry);
    NetFreePool (Block);
  }

  NetZeroMem (&Instance->RequestOption, sizeof (MTFTP4_OPTION));

  Instance->Operation     = 0;

  Instance->BlkSize       = MTFTP4_DEFAULT_BLKSIZE;
  Instance->LastBlock     = 0;
  Instance->ServerIp      = 0;
  Instance->ListeningPort = 0;
  Instance->ConnectedPort = 0;
  Instance->Gateway       = 0;
  Instance->PacketToLive  = 0;
  Instance->MaxRetry      = 0;
  Instance->CurRetry      = 0;
  Instance->Timeout       = 0;
  Instance->McastIp       = 0;
  Instance->McastPort     = 0;
  Instance->Master        = TRUE;
}

STATIC
EFI_STATUS
EFIAPI
EfiMtftp4Configure (
  IN EFI_MTFTP4_PROTOCOL    *This,
  IN EFI_MTFTP4_CONFIG_DATA *ConfigData
  )
/*++

Routine Description:

  Configure the MTFTP session for new operation or reset the current
  operation if ConfigData is NULL.

Arguments:

  This        - The MTFTP session to configure
  ConfigData  - The configure parameters

Returns:

  EFI_INVALID_PARAMETER - Some of the parameter is invalid.
  EFI_ACCESS_DENIED     - There is pending operation 
  EFI_SUCCESS           - The instance is configured for operation.

--*/
{
  MTFTP4_PROTOCOL           *Instance;
  EFI_TPL                   OldTpl;
  IP4_ADDR                  Ip;
  IP4_ADDR                  Netmask;
  IP4_ADDR                  Gateway;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = MTFTP4_PROTOCOL_FROM_THIS (This);

  if (ConfigData == NULL) {
    //
    // Reset the operation if ConfigData is NULL
    //
    OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

    Mtftp4CleanOperation (Instance, EFI_ABORTED);
    NetZeroMem (&Instance->Config, sizeof (EFI_MTFTP4_CONFIG_DATA));
    Instance->State = MTFTP4_STATE_UNCONFIGED;

    NET_RESTORE_TPL (OldTpl);
    
  } else {
    //
    // Configure the parameters for new operation.
    //
    Ip      = EFI_NTOHL (ConfigData->StationIp);
    Netmask = EFI_NTOHL (ConfigData->SubnetMask);
    Gateway = EFI_NTOHL (ConfigData->GatewayIp);

    if (!Ip4IsUnicast (EFI_NTOHL (ConfigData->ServerIp), 0)) {
      return EFI_INVALID_PARAMETER;
    }

    if (!ConfigData->UseDefaultSetting &&
       ((!IP4_IS_VALID_NETMASK (Netmask) || !Ip4IsUnicast (Ip, Netmask)))) {

      return EFI_INVALID_PARAMETER;
    }

    if ((Gateway != 0) && 
       (!IP4_NET_EQUAL (Gateway, Ip, Netmask) || !Ip4IsUnicast (Gateway, Netmask))) {

      return EFI_INVALID_PARAMETER;
    }

    OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

    if ((Instance->State == MTFTP4_STATE_CONFIGED) && (Instance->Operation != 0)) {
      NET_RESTORE_TPL (OldTpl);
      return EFI_ACCESS_DENIED;
    }

    Instance->Config = *ConfigData;
    Instance->State = MTFTP4_STATE_CONFIGED;

    NET_RESTORE_TPL (OldTpl);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
Mtftp4GetInfoCheckPacket (
  IN EFI_MTFTP4_PROTOCOL    *This,
  IN EFI_MTFTP4_TOKEN       *Token,
  IN UINT16                 PacketLen,
  IN EFI_MTFTP4_PACKET      *Packet
  )
/*++

Routine Description:

  Check packet for GetInfo. GetInfo is implemented with EfiMtftp4ReadFile.
  It use Mtftp4GetInfoCheckPacket to inspect the first packet from server,
  then abort the session.

Arguments:

  This      - The MTFTP4 protocol instance
  Token     - The user's token
  PacketLen - The length of the packet
  Packet    - The received packet.

Returns:

  EFI_ABORTED - Abort the ReadFile operation and return.

--*/
{
  MTFTP4_GETINFO_STATE      *State;
  EFI_STATUS                Status;
  UINT16                    OpCode;

  State   = (MTFTP4_GETINFO_STATE *) Token->Context;
  OpCode  = NTOHS (Packet->OpCode);

  //
  // Set the GetInfo's return status according to the OpCode.
  //
  switch (OpCode) {
  case EFI_MTFTP4_OPCODE_ERROR:
    State->Status = EFI_TFTP_ERROR;
    break;

  case EFI_MTFTP4_OPCODE_OACK:
    State->Status = EFI_SUCCESS;
    break;

  default:
    State->Status = EFI_PROTOCOL_ERROR;
  }
  
  //
  // Allocate buffer then copy the packet over. Use gBS->AllocatePool
  // in case NetAllocatePool will implements something tricky.
  //
  Status = gBS->AllocatePool (EfiBootServicesData, PacketLen, State->Packet);

  if (EFI_ERROR (Status)) {
    State->Status = EFI_OUT_OF_RESOURCES;
    return EFI_ABORTED;
  }

  *(State->PacketLen) = PacketLen;
  NetCopyMem (*(State->Packet), Packet, PacketLen);

  return EFI_ABORTED;
}

STATIC
EFI_STATUS
EFIAPI
EfiMtftp4GetInfo (
  IN EFI_MTFTP4_PROTOCOL      *This,
  IN EFI_MTFTP4_OVERRIDE_DATA *OverrideData,        OPTIONAL
  IN UINT8                    *Filename,
  IN UINT8                    *ModeStr,             OPTIONAL
  IN UINT8                    OptionCount,
  IN EFI_MTFTP4_OPTION        *OptionList,
  OUT UINT32                  *PacketLength,
  OUT EFI_MTFTP4_PACKET       **Packet OPTIONAL
  )
/*++

Routine Description:

  Get the information of the download from the server. It is implemented
  with EfiMtftp4ReadFile: build a token, then pass it to EfiMtftp4ReadFile. 
  In its check packet callback abort the opertions.

Arguments:

  This          - The MTFTP protocol instance
  OverrideData  - The MTFTP override data
  Filename      - The file to get information
  ModeStr       - The mode to use
  OptionCount   - The number of options to append
  OptionList    - The options to append
  PacketLength  - The variable to receive the packet length
  Packet        - The variable to receive the packet. 

Returns:

  EFI_INVALID_PARAMETER - The parameter is invaid
  EFI_SUCCESS           - The information is got
  Others                - Failed to get the information.

--*/
{
  EFI_MTFTP4_TOKEN          Token;
  MTFTP4_GETINFO_STATE      State;
  EFI_STATUS                Status;

  if ((This == NULL) || (Filename == NULL) || (PacketLength == NULL) || 
      (OptionCount && (OptionList == NULL))) {
    return EFI_INVALID_PARAMETER;
  }

  if (Packet != NULL) {
    *Packet = NULL;
  }

  *PacketLength         = 0;
  State.Packet          = Packet;
  State.PacketLen       = PacketLength;
  State.Status          = EFI_SUCCESS;

  //
  // Fill in the Token to issue an synchronous ReadFile operation
  //
  Token.Status          = EFI_SUCCESS;
  Token.Event           = NULL;
  Token.OverrideData    = OverrideData;
  Token.Filename        = Filename;
  Token.ModeStr         = ModeStr;
  Token.OptionCount     = OptionCount;
  Token.OptionList      = OptionList;
  Token.BufferSize      = 0;
  Token.Buffer          = NULL;
  Token.Context         = &State;
  Token.CheckPacket     = Mtftp4GetInfoCheckPacket;
  Token.TimeoutCallback = NULL;
  Token.PacketNeeded    = NULL;

  Status                = EfiMtftp4ReadFile (This, &Token);

  if (EFI_ABORTED == Status) {
    return State.Status;
  }

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
EfiMtftp4ParseOptions (
  IN EFI_MTFTP4_PROTOCOL    *This,
  IN UINT32                 PacketLen,
  IN EFI_MTFTP4_PACKET      *Packet,
  IN OUT UINT32             *OptionCount,
  OUT EFI_MTFTP4_OPTION     **OptionList          OPTIONAL
  )
/*++

Routine Description:

  Parse the packet into an array of options. The OptionList is allocated
  by this function, and caller should free it when used.

Arguments:

  This        - The MTFTP protocol instance
  PacketLen   - The length of the packet
  Packet      - The packet to parse
  OptionCount - The size of the OptionList array allocated.
  OptionList  - The allocated option array to save the option addresses.

Returns:

  EFI_INVALID_PARAMETER - The parameters are invalid.
  EFI_NOT_FOUND         - There is no valid option in the packet
  EFI_SUCCESS           - The packet is parsed.

--*/
{
  EFI_STATUS                Status;

  if ((This == NULL) || (PacketLen < MTFTP4_OPCODE_LEN) || 
      (Packet == NULL) || (OptionCount == NULL)) {
      
    return EFI_INVALID_PARAMETER;
  }

  Status = Mtftp4ExtractOptions (Packet, PacketLen, OptionCount, OptionList);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (*OptionCount == 0) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

STATIC
BOOLEAN
Mtftp4OverrideValid (
  IN MTFTP4_PROTOCOL          *Instance,
  IN EFI_MTFTP4_OVERRIDE_DATA *Override
  )
/*++

Routine Description:

  Check whether the override data is valid. It will first 
  validate whether the server is a valid unicast. If a gateway
  is provided in the Override, it also check that it is a 
  unicast on the connected network.

Arguments:

  Instance  - The MTFTP instance
  Override  - The override data to validate.

Returns:

  TRUE if the override data is valid, otherwise FALSE.

--*/
{
  EFI_MTFTP4_CONFIG_DATA    *Config;
  IP4_ADDR                  Ip;
  IP4_ADDR                  Netmask;
  IP4_ADDR                  Gateway;

  if (!Ip4IsUnicast (EFI_NTOHL (Override->ServerIp), 0)) {
    return FALSE;
  }

  Config  = &Instance->Config;
  Gateway = EFI_NTOHL (Override->GatewayIp);

  if (!Config->UseDefaultSetting && (Gateway != 0)) {
    Netmask = EFI_NTOHL (Config->SubnetMask);
    Ip   = EFI_NTOHL (Config->StationIp);

    if (!Ip4IsUnicast (Gateway, Netmask) || !IP4_NET_EQUAL (Gateway, Ip, Netmask)) {
      return FALSE;
    }
  }

  return TRUE;
}

BOOLEAN
Mtftp4GetMapping (
  IN MTFTP4_PROTOCOL        *Instance,
  IN UDP_IO_PORT            *UdpPort,
  IN EFI_UDP4_CONFIG_DATA   *UdpCfgData
  )
/*++

Routine Description:

  Poll the UDP to get the IP4 default address, which may be retrieved 
  by DHCP. The default time out value is 5 seconds. If IP has retrieved
  the default address, the UDP is reconfigured.

Arguments:

  Instance    - The Mtftp instance
  UdpPort     - The UDP port to poll
  UdpCfgData  - The UDP configure data to reconfigure the UDP port.

Returns:

  TRUE if the default address is retrieved and UDP is reconfigured. 
  Otherwise FALSE.

--*/
{
  MTFTP4_SERVICE            *Service;
  EFI_IP4_MODE_DATA         Ip4Mode;
  EFI_UDP4_PROTOCOL         *Udp;

  ASSERT (Instance->Config.UseDefaultSetting);

  Service = Instance->Service;
  Udp     = UdpPort->Udp;

  if (Service->TimeToGetMap == 0) {
    Service->TimeToGetMap = MTFTP4_TIME_TO_GETMAP;
  }

  while (Service->TimeToGetMap != 0) {
    Udp->Poll (Udp);

    if (!EFI_ERROR (Udp->GetModeData (Udp, NULL, &Ip4Mode, NULL, NULL)) && 
        Ip4Mode.IsConfigured) {

      Udp->Configure (Udp, NULL);
      return (BOOLEAN) (Udp->Configure (Udp, UdpCfgData) == EFI_SUCCESS);
    }
  }

  return FALSE;
}

STATIC
EFI_STATUS
Mtftp4ConfigUnicastPort (
  IN UDP_IO_PORT            *UdpIo,
  IN MTFTP4_PROTOCOL        *Instance
  )
/*++

Routine Description:

  Configure the UDP port for unicast receiving.

Arguments:

  UdpIo     - The UDP port
  Instance  - The MTFTP session

Returns:

  EFI_SUCCESS - The UDP port is successfully configured for the session
  to unicast receive.

--*/
{
  EFI_MTFTP4_CONFIG_DATA    *Config;
  EFI_UDP4_CONFIG_DATA      UdpConfig;
  EFI_STATUS                Status;

  Config = &Instance->Config;

  UdpConfig.AcceptBroadcast     = FALSE;
  UdpConfig.AcceptPromiscuous   = FALSE;
  UdpConfig.AcceptAnyPort       = FALSE;
  UdpConfig.AllowDuplicatePort  = FALSE;
  UdpConfig.TypeOfService       = 0;
  UdpConfig.TimeToLive          = 64;
  UdpConfig.DoNotFragment       = FALSE;
  UdpConfig.ReceiveTimeout      = 0;
  UdpConfig.TransmitTimeout     = 0;
  UdpConfig.UseDefaultAddress   = Config->UseDefaultSetting;
  UdpConfig.StationAddress      = Config->StationIp;
  UdpConfig.SubnetMask          = Config->SubnetMask;
  UdpConfig.StationPort         = 0;
  UdpConfig.RemotePort          = 0;
  EFI_IP4 (UdpConfig.RemoteAddress) = HTONL (Instance->ServerIp);

  Status = UdpIo->Udp->Configure (UdpIo->Udp, &UdpConfig);

  if ((Status == EFI_NO_MAPPING) && Mtftp4GetMapping (Instance, UdpIo, &UdpConfig)) {
    return EFI_SUCCESS;
  }

  return Status;
}

STATIC
EFI_STATUS
Mtftp4Start (
  IN EFI_MTFTP4_PROTOCOL    *This,
  IN EFI_MTFTP4_TOKEN       *Token,
  IN UINT16                 Operation
  )
/*++

Routine Description:

  Start the MTFTP session to do the operation, such as read file,
  write file, and read directory.

Arguments:

  This      - The MTFTP session
  Token     - The token than encapsues the user's request.
  Operation - The operation to do

Returns:

  EFI_INVALID_PARAMETER - Some of the parameters are invalid.
  EFI_NOT_STARTED       - The MTFTP session hasn't been configured.
  EFI_ALREADY_STARTED   - There is pending operation for the session.
  EFI_SUCCESS           - The operation is successfully started.

--*/
{
  MTFTP4_PROTOCOL           *Instance;
  EFI_MTFTP4_OVERRIDE_DATA  *Override;
  EFI_MTFTP4_CONFIG_DATA    *Config;
  EFI_TPL                   OldTpl;
  EFI_STATUS                Status;

  //
  // Validate the parameters
  //
  if ((This == NULL) || (Token == NULL) || (Token->Filename == NULL) ||
      ((Token->OptionCount != 0) && (Token->OptionList == NULL))) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // User must provide at least one method to collect the data for download.
  //
  if (((Operation == EFI_MTFTP4_OPCODE_RRQ) || (Operation == EFI_MTFTP4_OPCODE_DIR)) &&
      ((Token->Buffer == NULL) && (Token->CheckPacket == NULL))) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // User must provide at least one method to provide the data for upload.
  //
  if ((Operation == EFI_MTFTP4_OPCODE_WRQ) && 
     ((Token->Buffer == NULL) && (Token->PacketNeeded == NULL))) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = MTFTP4_PROTOCOL_FROM_THIS (This);

  if (Instance->State != MTFTP4_STATE_CONFIGED) {
    return EFI_NOT_STARTED;
  }

  if (Instance->Operation != 0) {
    return EFI_ACCESS_DENIED;
  }
  
  //
  // Set the Operation now to prevent the application start other
  // operations. Don't get the lock or raise TPL here because
  // Mtftp4GetMapping, which is called by UdpIoCreatePort, must be
  // run below the NET_TPL_CALLBACK
  //
  Instance->Operation = Operation;
  Override            = Token->OverrideData;

  if ((Override != NULL) && !Mtftp4OverrideValid (Instance, Override)) {
    Status = EFI_INVALID_PARAMETER;
    goto ON_ERROR;
  }

  if (Token->OptionCount != 0) {
    Status = Mtftp4ParseOption (
               Token->OptionList,
               Token->OptionCount,
               TRUE,
               &Instance->RequestOption
               );

    if (EFI_ERROR (Status)) {
      goto ON_ERROR;
    }
  }
  
  //
  // Set the operation parameters from the configuration or override data.
  //
  Config                  = &Instance->Config;
  Instance->Token         = Token;
  Instance->BlkSize       = MTFTP4_DEFAULT_BLKSIZE;
  Instance->ServerIp      = EFI_NTOHL (Config->ServerIp);
  Instance->ListeningPort = Config->InitialServerPort;
  Instance->ConnectedPort = 0;
  Instance->Gateway       = EFI_NTOHL (Config->GatewayIp);
  Instance->MaxRetry      = Config->TryCount;
  Instance->Timeout       = Config->TimeoutValue;
  Instance->Master        = TRUE;

  if (Override != NULL) {
    Instance->Gateway       = EFI_NTOHL (Override->GatewayIp);
    Instance->ServerIp      = EFI_NTOHL (Override->ServerIp);
    Instance->ListeningPort = Override->ServerPort;
    Instance->MaxRetry      = Override->TryCount;
    Instance->Timeout       = Override->TimeoutValue;
  }

  if (Instance->ListeningPort == 0) {
    Instance->ListeningPort = MTFTP4_DEFAULT_SERVER_PORT;
  }

  if (Instance->MaxRetry == 0) {
    Instance->MaxRetry = MTFTP4_DEFAULT_RETRY;
  }

  if (Instance->Timeout == 0) {
    Instance->Timeout = MTFTP4_DEFAULT_TIMEOUT;
  }
  
  //
  // Config the unicast UDP child to send initial request
  //
  Status = Mtftp4ConfigUnicastPort (Instance->UnicastPort, Instance);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }
  
  //
  // Build and send an initial requests
  //
  OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

  if (Operation == EFI_MTFTP4_OPCODE_WRQ) {
    Status = Mtftp4WrqStart (Instance, Operation);
  } else {
    Status = Mtftp4RrqStart (Instance, Operation);
  }

  NET_RESTORE_TPL (OldTpl);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }
  
  //
  // Return immediately for asynchronous operation or poll the
  // instance for synchronous operation.
  //
  Token->Status = EFI_NOT_READY;
  
  if (Token->Event != NULL) {
    return EFI_SUCCESS;
  }

  while (Token->Status == EFI_NOT_READY) {
    This->Poll (This);
  }

  return Token->Status;

ON_ERROR:
  Mtftp4CleanOperation (Instance, Status);
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
EfiMtftp4ReadFile (
  IN EFI_MTFTP4_PROTOCOL    *This,
  IN EFI_MTFTP4_TOKEN       *Token
  )
/*++

Routine Description:

  Read a file from the server.

Arguments:

  This  - The Mtftp protocol instance.
  Token - The user's request wrap token.

Returns:

  EFI_SUCCESS - The ReadFile has finished, the file has been downloaded
                if it is synchronous operation, otherwise it has been 
                initated.
  Others      - Some error happened.                

--*/
{
  return Mtftp4Start (This, Token, EFI_MTFTP4_OPCODE_RRQ);
}

STATIC
EFI_STATUS
EFIAPI
EfiMtftp4WriteFile (
  IN EFI_MTFTP4_PROTOCOL    *This,
  IN EFI_MTFTP4_TOKEN       *Token
  )
/*++

Routine Description:

  Upload a file to the server.

Arguments:

  This  - The MTFTP protocol session
  Token - The user's request wrap token.

Returns:

  EFI_SUCCESS - The WriteFile has finished, the file has been uploaded
                if it is synchronous operation, otherwise it has been 
                initated.
  Others      - Some error happened.                

--*/
{
  return Mtftp4Start (This, Token, EFI_MTFTP4_OPCODE_WRQ);
}

STATIC
EFI_STATUS
EFIAPI
EfiMtftp4ReadDirectory (
  IN EFI_MTFTP4_PROTOCOL        *This,
  IN EFI_MTFTP4_TOKEN           *Token
  )
/*++

Routine Description:

  Read a directory from the server. The only difference
  between ReadFile and ReadDirectory is the opcode used.

Arguments:

  This  - The MTFTP protocol session
  Token - The user's request wrap token.

Returns:

  EFI_SUCCESS - The ReadDirectory has finished, the directory has been 
                downloaded as a file if it is synchronous operation, 
                otherwise it has been initated.
  Others      - Some error happened.                

--*/
{
  return Mtftp4Start (This, Token, EFI_MTFTP4_OPCODE_DIR);
}

STATIC
EFI_STATUS
EFIAPI
EfiMtftp4Poll (
  IN EFI_MTFTP4_PROTOCOL    *This
  )
/*++

Routine Description:

  Poll the network stack to accelerate the packet process.

Arguments:

  This  - The MTFTP protocol instance.

Returns:

  EFI_INVALID_PARAMETER - This is NULL.
  EFI_NOT_STARTED       - The MTFTP session hasn't been configured.
  EFI_DEVICE_ERROR      - The MTFTP session has been destoried.

--*/
{
  MTFTP4_PROTOCOL           *Instance;
  EFI_UDP4_PROTOCOL         *Udp;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = MTFTP4_PROTOCOL_FROM_THIS (This);

  if (Instance->State == MTFTP4_STATE_UNCONFIGED) {
    return EFI_NOT_STARTED;
  } else if (Instance->State == MTFTP4_STATE_DESTORY) {
    return EFI_DEVICE_ERROR;
  }

  Udp = Instance->UnicastPort->Udp;
  return Udp->Poll (Udp);
}

EFI_MTFTP4_PROTOCOL gMtftp4ProtocolTemplate = {
  EfiMtftp4GetModeData,
  EfiMtftp4Configure,
  EfiMtftp4GetInfo,
  EfiMtftp4ParseOptions,
  EfiMtftp4ReadFile,
  EfiMtftp4WriteFile,
  EfiMtftp4ReadDirectory,
  EfiMtftp4Poll
};
