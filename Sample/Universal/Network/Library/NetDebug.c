/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  NetDebug.c

Abstract:

  Network debug facility. The debug information is wrapped in 
  SYSLOG packets, then sent over SNP. This debug facility can't 
  be used by SNP. Apply caution when used in MNP and non-network 
  module because SNP is most likely not "thread safe". We assume 
  that the SNP supports the EHTERNET.

--*/

#include "NetDebug.h"
#include "EfiDriverLib.h"
#include "EfiPrintLib.h"
#include "NetHeader.h"
#include EFI_PROTOCOL_CONSUMER (SimpleNetwork)

//
// Any error level digitally larger than mNetDebugLevelMax 
// will be silently discarded.
//
UINTN  mNetDebugLevelMax = NETDEBUG_LEVEL_ERROR;
UINT32 mSyslogPacketSeq  = 0xDEADBEEF;

// 
// You can change mSyslogDstMac mSyslogDstIp and mSyslogSrcIp 
// here to direct the syslog packets to the syslog deamon. The 
// default is broadcast to both the ethernet and IP. 
//
UINT8  mSyslogDstMac [NET_ETHER_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
UINT32 mSyslogDstIp                       = 0xffffffff;
UINT32 mSyslogSrcIp                       = 0;

UINT8 *
MonthName[] = {
  "Jan",
  "Feb",
  "Mar",
  "Apr",
  "May",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Oct",
  "Nov",
  "Dec"
};

EFI_SIMPLE_NETWORK_PROTOCOL *
SyslogLocateSnp (
  VOID
  )
/*++

Routine Description:

  Locate the handles that support SNP, then open one of them 
  to send the syslog packets. The caller isn't required to close
  the SNP after use because the SNP is opened by HandleProtocol.

Arguments:

  None

Returns:

  The point to SNP if one is properly openned. Otherwise NULL

--*/
{
  EFI_SIMPLE_NETWORK_PROTOCOL *Snp;
  EFI_STATUS                  Status;
  EFI_HANDLE                  *Handles;
  UINTN                       HandleCount;
  UINTN                       Index;

  //
  // Locate the handles which has SNP installed.
  //
  Handles = NULL;
  Status  = gBS->LocateHandleBuffer (
                   ByProtocol,
                   &gEfiSimpleNetworkProtocolGuid,
                   NULL,
                   &HandleCount,
                   &Handles
                   );

  if (EFI_ERROR (Status) || (HandleCount == 0)) {
    return NULL;
  }
  
  //
  // Try to open one of the ethernet SNP protocol to send packet
  //
  Snp = NULL;
  
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    Handles[Index],
                    &gEfiSimpleNetworkProtocolGuid,
                    (VOID **) &Snp
                    );

    if ((Status == EFI_SUCCESS) && (Snp != NULL) && 
        (Snp->Mode->IfType == NET_IFTYPE_ETHERNET) &&
        (Snp->Mode->MaxPacketSize >= NET_SYSLOG_PACKET_LEN)) {
        
      break;
    }

    Snp = NULL;
  }

  gBS->FreePool (Handles);
  return Snp;
}

EFI_STATUS
SyslogSendPacket (
  IN UINT8                    *Packet,
  IN UINT32                   Length
  )
/*++

Routine Description:

  Transmit a syslog packet synchronously through SNP. The Packet
  already has the ethernet header prepended. This function should 
  fill in the source MAC because it will try to locate a SNP each
  time it is called to avoid the problem if SNP is unloaded.
  This code snip is copied from MNP. 

Arguments:

  Packet  - The Syslog packet 
  Length  - The length of the packet

Returns:

  EFI_DEVICE_ERROR - Failed to locate a usable SNP protocol
  EFI_TIMEOUT      - Timeout happened to send the packet.
  EFI_SUCCESS      - Packet is sent.
  
--*/
{
  EFI_SIMPLE_NETWORK_PROTOCOL *Snp;
  ETHER_HEAD                  *Ether;
  EFI_STATUS                  Status;
  EFI_EVENT                   TimeoutEvent;
  UINT8                       *TxBuf;

  Snp = SyslogLocateSnp ();

  if (Snp == NULL) {
    return EFI_DEVICE_ERROR;
  }

  Ether = (ETHER_HEAD *) Packet;
  EfiCopyMem (Ether->SrcMac, Snp->Mode->CurrentAddress.Addr, NET_ETHER_ADDR_LEN);

  //
  // Start the timeout event.
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_TIMER,
                  EFI_TPL_NOTIFY,
                  NULL,
                  NULL,
                  &TimeoutEvent
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->SetTimer (TimeoutEvent, TimerRelative, NET_SYSLOG_TX_TIMEOUT);

  if (EFI_ERROR (Status)) {
    goto ON_EXIT;
  }

  for (;;) {
    //
    // Transmit the packet through SNP.
    //
    Status = Snp->Transmit (Snp, 0, Length, Packet, NULL, NULL, NULL);

    if ((Status != EFI_SUCCESS) && (Status != EFI_NOT_READY)) {
      Status = EFI_DEVICE_ERROR;
      break;
    }
    
    //
    // If Status is EFI_SUCCESS, the packet is put in the transmit queue.
    // if Status is EFI_NOT_READY, the transmit engine of the network
    // interface is busy. Both need to sync SNP.
    //
    TxBuf = NULL;

    do {
      //
      // Get the recycled transmit buffer status.
      //
      Snp->GetStatus (Snp, NULL, &TxBuf);

      if (!EFI_ERROR (gBS->CheckEvent (TimeoutEvent))) {
        Status = EFI_TIMEOUT;
        break;
      }

    } while (TxBuf == NULL);

    if ((Status == EFI_SUCCESS) || (Status == EFI_TIMEOUT)) {
      break;
    }
    
    //
    // Status is EFI_NOT_READY. Restart the timer event and
    // call Snp->Transmit again.
    //
    gBS->SetTimer (TimeoutEvent, TimerRelative, NET_SYSLOG_TX_TIMEOUT);
  }

  gBS->SetTimer (TimeoutEvent, TimerCancel, 0);

ON_EXIT:
  gBS->CloseEvent (TimeoutEvent);
  return Status;
}

UINT16
SyslogChecksum (
  IN UINT8                  *Bulk,
  IN UINT32                 Len
  )
/*++

Routine Description:

  Compute checksum for a bulk of data. This code is copied from the 
  Netbuffer library.

Arguments:

  Bulk  - Pointer to the data.
  Len   - Length of the data, in bytes.

Returns:

  UINT16 - The computed checksum.

--*/
{
  register UINT32           Sum;

  Sum = 0;

  while (Len > 1) {
    Sum += *(UINT16 *) Bulk;
    Bulk += 2;
    Len -= 2;
  }
  
  //
  // Add left-over byte, if any
  //
  if (Len > 0) {
    Sum += *(UINT8 *) Bulk;
  }
  
  //
  // Fold 32-bit sum to 16 bits
  //
  while (Sum >> 16) {
    Sum = (Sum & 0xffff) + (Sum >> 16);
  }

  return (UINT16) ~Sum;
}

UINT32
SyslogBuildPacket (
  UINT8                     *Buf,
  UINT32                    BufLen,
  UINT32                    Level,
  UINT8                     *Module,
  UINT8                     *File,
  UINT32                    Line,
  UINT8                     *Message
  )
/*++

Routine Description:

  Build a syslog packet, including the Ethernet/Ip/Udp headers 
  and user's message. 

Arguments:

  Buf     - The buffer to put the packet data
  BufLen  - The lenght of the Buf
  Level   - Syslog servity level
  Module  - The module that generates the log
  File    - The file that contains the current log
  Line    - The line of code in the File that contains the current log
  Message - The log message

Returns:

  The length of the syslog packet built.

--*/
{
  ETHER_HEAD                *Ether;
  IP4_HEAD                  *Ip4;
  EFI_UDP4_HEADER           *Udp4;
  EFI_TIME                  Time;
  UINT32                    Pri;
  UINT32                    Len;

  //
  // Fill in the Ethernet header. Leave alone the source MAC. 
  // SyslogSendPacket will fill in the address for us.
  //
  Ether = (ETHER_HEAD *) Buf;
  EfiCopyMem (Ether->DstMac, mSyslogDstMac, NET_ETHER_ADDR_LEN);
  EfiZeroMem (Ether->SrcMac, NET_ETHER_ADDR_LEN);

  Ether->EtherType = HTONS (0x0800);    // IP protocol

  Buf             += sizeof (ETHER_HEAD);
  BufLen          -= sizeof (ETHER_HEAD);

  //
  // Fill in the IP header
  //
  Ip4              = (IP4_HEAD *) Buf;
  Ip4->HeadLen     = 5;
  Ip4->Ver         = 4;
  Ip4->Tos         = 0;
  Ip4->TotalLen    = 0;
  Ip4->Id          = (UINT16) mSyslogPacketSeq;
  Ip4->Fragment    = 0;
  Ip4->Ttl         = 16;
  Ip4->Protocol    = 0x11;
  Ip4->Checksum    = 0;
  Ip4->Src         = mSyslogSrcIp;
  Ip4->Dst         = mSyslogDstIp;

  Buf             += sizeof (IP4_HEAD);
  BufLen          -= sizeof (IP4_HEAD);

  //
  // Fill in the UDP header, Udp checksum is optional. Leave it zero.
  //
  Udp4             = (EFI_UDP4_HEADER*) Buf;
  Udp4->SrcPort    = HTONS (514);
  Udp4->DstPort    = HTONS (514);
  Udp4->Length     = 0;
  Udp4->Checksum   = 0;

  Buf             += sizeof (EFI_UDP4_HEADER);
  BufLen          -= sizeof (EFI_UDP4_HEADER);

  //
  // Build the syslog message body with <PRI> Timestamp  machine module Message
  //
  Pri = ((NET_SYSLOG_FACILITY & 31) << 3) | (Level & 7);
  gRT->GetTime (&Time, NULL);

  //
  // Use %a to format the ASCII strings, %s to format UNICODE strings
  //
  Len  = 0;
  Len += (UINT32) ASPrint (
                    Buf,
                    BufLen,
                    "<%d> %a %d %d:%d:%d ",
                    Pri,
                    MonthName [Time.Month-1], 
                    Time.Day,
                    Time.Hour,
                    Time.Minute,
                    Time.Second
                    );
  Len--;

  Len += (UINT32) ASPrint (
                    Buf + Len, 
                    BufLen - Len, 
                    "Tiano %a: %a (Line: %d File: %a)", 
                    Module,
                    Message,
                    Line,
                    File
                    );
  Len--;

  //
  // OK, patch the IP length/checksum and UDP length fields.
  //
  Len           += sizeof (EFI_UDP4_HEADER);
  Udp4->Length   = HTONS ((UINT16) Len);

  Len           += sizeof (IP4_HEAD);
  Ip4->TotalLen  = HTONS ((UINT16) Len);
  Ip4->Checksum  = SyslogChecksum ((UINT8 *) Ip4, sizeof (IP4_HEAD));

  return Len + sizeof (ETHER_HEAD);
}

UINT8 *
NetDebugASPrint (
  UINT8                     *Format,
  ...
  )
/*++

Routine Description:

  Allocate a buffer, then format the message to it. This is a 
  help function for the NET_DEBUG_XXX macros. The PrintArg of 
  these macros treats the variable length print parameters as a 
  single parameter, and pass it to the NetDebugASPrint. For
  example, NET_DEBUG_TRACE ("Tcp", ("State transit to %a\n", Name))
  if extracted to:   
  
         NetDebugOutput (
           NETDEBUG_LEVEL_TRACE, 
           "Tcp", 
           __FILE__,
           __LINE__,
           NetDebugASPrint ("State transit to %a\n", Name) 
         )
           
  This is exactly what we want.

Arguments:

  Format  - The ASCII format string.
  ...     - The variable length parameter whose format is determined 
            by the Format string.

Returns:

  The buffer containing the formatted message, or NULL if failed to 
  allocate memory. 

--*/
{
  VA_LIST                   Marker;
  UINT8                     *Buf;

  Buf = EfiLibAllocatePool (NET_DEBUG_MSG_LEN);

  if (Buf == NULL) {
    return NULL;
  }

  VA_START (Marker, Format);
  AvSPrint (Buf, NET_DEBUG_MSG_LEN, Format, Marker);
  VA_END (Marker);

  return Buf;
}

EFI_STATUS
NetDebugOutput (
  UINT32                    Level, 
  UINT8                     *Module,
  UINT8                     *File,
  UINT32                    Line,
  UINT8                     *Message
  )
/*++

Routine Description:

  Output a debug message to syslog. This function will locate a 
  instance of SNP then send the message through it. Because it 
  isn't open the SNP BY_DRIVER, apply caution when using it.

Arguments:

  Level   - The servity level of the message.
  Module  - The Moudle that generates the log.
  File    - The file that contains the log.
  Line    - The exact line that contains the log.
  Message - The user message to log.

Returns:

  EFI_OUT_OF_RESOURCES - Failed to allocate memory for the packet
  EFI_SUCCESS          - The log is discard because that it is more verbose 
                         than the mNetDebugLevelMax. Or, it has been sent out.
--*/
{
  UINT8                     *Packet;
  UINT32                    Len;
  EFI_STATUS                Status;

  //
  // Check whether the message should be sent out
  //
  if (Message == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (Level > mNetDebugLevelMax) {
    Status = EFI_SUCCESS;
    goto ON_EXIT;
  }
  
  //
  // Allocate a maxium of 1024 bytes, the caller should ensure
  // that the message plus the ethernet/ip/udp header is shorter
  // than this
  //
  Packet = EfiLibAllocatePool (NET_SYSLOG_PACKET_LEN);

  if (Packet == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ON_EXIT;
  }
  
  //
  // Build the message: Ethernet header + IP header + Udp Header + user data
  //
  Len = SyslogBuildPacket (
          Packet,
          NET_SYSLOG_PACKET_LEN,
          Level,
          Module,
          File,
          Line,
          Message
          );

  mSyslogPacketSeq++;
  Status = SyslogSendPacket (Packet, Len);
  gBS->FreePool (Packet);

ON_EXIT:
  gBS->FreePool (Message);
  return Status;
}
