/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  pxe_bc_tcp.c

Abstract:

--*/

#include "bc.h"

//
// //////////////////////////////////////////////////////////////////////
//
//  Tcp Write Routine - called by base code - e.g. TFTP - already locked
//
EFI_STATUS
TcpWrite (
  IN PXE_BASECODE_DEVICE            *Private,
  IN UINT16                         OpFlags,
  IN UINT16                         *UrgentPointer,
  IN UINT32                         *SequenceNumber,
  IN UINT32                         *AckNumber,
  IN UINT16                         *HlenResCode,
  IN UINT16                         *Window,
  IN EFI_IP_ADDRESS                 *DestIpPtr,
  IN EFI_PXE_BASE_CODE_TCP_PORT     *DestPortPtr,
  IN EFI_IP_ADDRESS                 *GatewayIpPtr, OPTIONAL
  IN EFI_IP_ADDRESS                 *SrcIpPtr,
  IN OUT EFI_PXE_BASE_CODE_TCP_PORT *SrcPortPtr,
  IN UINTN                          *HeaderSizePtr,
  IN VOID                           *HeaderPtr,
  IN UINTN                          *BufferSizePtr,
  IN VOID                           *BufferPtr
  )
/*++
Routine description:
  Write buffer to TCP sesion.

Parameters:
  Private := Pointer to PxeBc interface
  OpFlags := 
  UrgentPointer := 
  SequenceNumber := 
  AckNumber := 
  HlenResCode := 
  Window := 
  DestIpPtr := Destination IP address
  DestPortPtr := Destination TCP port
  GatewayIpPtr := Gateway IP address or NULL
  SrcIpPtr := Source IP address
  SrcPortPtr := Source TCP port
  HeaderSizePtr := Size of packet header
  HeaderPtr := Pointer to header buffer
  BufferSizePtr := Size of packet data
  BufferPtr := Pointer to data buffer

Returns:
  EFI_SUCCESS := 
  EFI_INVALID_PARAMETER := 
  EFI_BAD_BUFFER_SIZE := 
  other := 
--*/
{
  EFI_PXE_BASE_CODE_TCP_PORT  DefaultSrcPort;
  UINTN                       TotalLength;
  UINT8                       CodeBits;

  DefaultSrcPort = 23;

  //
  // check parameters
  //
  if (BufferSizePtr == NULL ||
      BufferPtr == NULL ||
      DestIpPtr == NULL ||
      DestPortPtr == NULL ||
      HeaderPtr == NULL ||
      (OpFlags &~(EFI_PXE_BASE_CODE_UDP_OPFLAGS_MAY_FRAGMENT | EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_PORT))
      ) {
    DEBUG (
      (EFI_D_WARN,
      "\nTcpWrite()  Exit #1  %xh (%r)",
      EFI_INVALID_PARAMETER,
      EFI_INVALID_PARAMETER)
      );

    return EFI_INVALID_PARAMETER;
  }
  //
  // Derive header size.
  //
  TotalLength = *BufferSizePtr + sizeof (TCPV4_HEADER);

  if (TotalLength > 0x0000ffff) {
    DEBUG (
      (EFI_D_WARN,
      "\nTcpWrite()  Exit #2  %xh (%r)",
      EFI_BAD_BUFFER_SIZE,
      EFI_BAD_BUFFER_SIZE)
      );

    return EFI_BAD_BUFFER_SIZE;
  }

  if (SrcIpPtr == NULL) {
    SrcIpPtr = &Private->EfiBc.Mode->StationIp;
  }

  if (SrcPortPtr == NULL) {
    SrcPortPtr = &DefaultSrcPort;
    OpFlags |= EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_PORT;
  }

  if (OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_PORT) {
    *SrcPortPtr = Private->RandomPort;

    if (++Private->RandomPort == 0) {
      Private->RandomPort = PXE_RND_PORT_LOW;
    }
  }

#define IpTxBuffer  ((IPV4_BUFFER *) Private->TransmitBufferPtr)
  //
  // build pseudo header and tcp header in transmit buffer
  //
#define Tcpv4Base ((TCPV4_HEADERS *) (IpTxBuffer->u.Data - sizeof (TCPV4_PSEUDO_HEADER)))

  Tcpv4Base->Tcpv4PseudoHeader.SrcAddr.L    = SrcIpPtr->Addr[0];
  Tcpv4Base->Tcpv4PseudoHeader.DestAddr.L   = DestIpPtr->Addr[0];
  Tcpv4Base->Tcpv4PseudoHeader.Zero         = 0;
  Tcpv4Base->Tcpv4PseudoHeader.Protocol     = PROT_TCP;
  Tcpv4Base->Tcpv4PseudoHeader.TotalLength  = HTONS (TotalLength);
  Tcpv4Base->Tcpv4Header.SrcPort            = HTONS (*SrcPortPtr);
  Tcpv4Base->Tcpv4Header.DestPort           = HTONS (*DestPortPtr);
  Tcpv4Base->Tcpv4Header.SeqNumber          = HTONL (*SequenceNumber);
  Tcpv4Base->Tcpv4Header.AckNumber          = HTONL (*AckNumber);
  Tcpv4Base->Tcpv4Header.HlenResCode        = HTONS (*HlenResCode);
  Tcpv4Base->Tcpv4Header.Window             = HTONS (*Window);
  Tcpv4Base->Tcpv4Header.UrgentPointer      = HTONS (*UrgentPointer);
  Tcpv4Base->Tcpv4Header.Checksum           = 0;

  Tcpv4Base->Tcpv4Header.Checksum = IpChecksum2 (
                                      (UINT16 *) Tcpv4Base,
                                      sizeof (TCPV4_HEADER) + sizeof (TCPV4_PSEUDO_HEADER),
                                      (UINT16 *) BufferPtr,
                                      (UINT16) *BufferSizePtr
                                      );

  if (Tcpv4Base->Tcpv4Header.Checksum == 0) {
    //
    // transmit zero checksum as ones complement
    //
    Tcpv4Base->Tcpv4Header.Checksum = 0xffff;
  }

  DEBUG (
    (EFI_D_NET,
    "\nTcpWrite()  DestIP is:  %d.%d.%d.%d SrcIP is:  %d.%d.%d.%d\n",
    Tcpv4Base->Tcpv4PseudoHeader.DestAddr.B[0],
    Tcpv4Base->Tcpv4PseudoHeader.DestAddr.B[1],
    Tcpv4Base->Tcpv4PseudoHeader.DestAddr.B[2],
    Tcpv4Base->Tcpv4PseudoHeader.DestAddr.B[3],
    Tcpv4Base->Tcpv4PseudoHeader.SrcAddr.B[0],
    Tcpv4Base->Tcpv4PseudoHeader.SrcAddr.B[1],
    Tcpv4Base->Tcpv4PseudoHeader.SrcAddr.B[2],
    Tcpv4Base->Tcpv4PseudoHeader.SrcAddr.B[3])
    );

  DEBUG (
    (EFI_D_NET,
    "\nSrcPort=%d, DstPort=%d, SeqNum=%x, AckNum=%x\n",
    NTOHS (Tcpv4Base->Tcpv4Header.SrcPort),
    NTOHS (Tcpv4Base->Tcpv4Header.DestPort),
    NTOHL (Tcpv4Base->Tcpv4Header.SeqNumber),
    NTOHL (Tcpv4Base->Tcpv4Header.AckNumber))
    );

  CodeBits = (UINT8) (NTOHS (Tcpv4Base->Tcpv4Header.HlenResCode) & 0x3f);

  return Ip4Send (
          Private,
          OpFlags,
          PROT_TCP,
          Tcpv4Base->Tcpv4PseudoHeader.SrcAddr.L,
          Tcpv4Base->Tcpv4PseudoHeader.DestAddr.L,
          (GatewayIpPtr) ? GatewayIpPtr->Addr[0] : 0,
          sizeof (TCPV4_HEADER),
          BufferPtr,
          *BufferSizePtr
          );
}
//
// //////////////////////////////////////////////////////////
//
//  BC Udp Write Routine
//
EFI_STATUS
EFIAPI
BcTcpWrite (
  IN EFI_PXE_BASE_CODE_PROTOCOL       *This,
  IN UINT16                           OpFlags,
  IN UINT16                           *UrgentPointer,
  IN UINT32                           *SequenceNumber,
  IN UINT32                           *AckNumber,
  IN UINT16                           *HlenResCode,
  IN UINT16                           *Window,
  IN EFI_IP_ADDRESS                   *DestIpPtr,
  IN EFI_PXE_BASE_CODE_TCP_PORT       *DestPortPtr,
  IN EFI_IP_ADDRESS                   *GatewayIpPtr, OPTIONAL
  IN EFI_IP_ADDRESS                   *SrcIpPtr, OPTIONAL
  IN OUT EFI_PXE_BASE_CODE_TCP_PORT   *SrcPortPtr, OPTIONAL
  IN UINTN                            *HeaderSizePtr, OPTIONAL
  IN VOID                             *HeaderPtr, OPTIONAL
  IN UINTN                            *BufferSizePtr,
  IN VOID                             *BufferPtr
  )
/*++
Routine description:
  TCP write API entry point.

Parameters:
  This := Pointer to PxeBc interface
  OpFlags := 
  UrgentPointer := 
  SequencePointer := 
  AckNumber := 
  HlenResCode := 
  Window := 
  DestIpPtr := 
  DestPortPtr := 
  GatewayIpPtr := 
  SrcIpPtr := 
  SrcPortPtr := 
  HeaderSizePtr := 
  HeaderPtr := 
  BufferSizePtr := 
  BufferPtr := 

Returns:
  EFI_SUCCESS := 
  other := 
--*/
{
  EFI_STATUS          StatCode;
  PXE_BASECODE_DEVICE *Private;

  //
  // Lock the instance data and make sure started
  //
  StatCode = EFI_SUCCESS;

  if (This == NULL) {
    DEBUG ((EFI_D_ERROR, "BC *This pointer == NULL"));
    return EFI_INVALID_PARAMETER;
  }

  Private = CR (This, PXE_BASECODE_DEVICE, EfiBc, PXE_BASECODE_DEVICE_SIGNATURE);

  if (Private == NULL) {
    DEBUG ((EFI_D_ERROR, "PXE_BASECODE_DEVICE poiner == NULL"));
    return EFI_INVALID_PARAMETER;
  }

  EfiAcquireLock (&Private->Lock);

  if (This->Mode == NULL || !This->Mode->Started) {
    DEBUG ((EFI_D_ERROR, "BC was not started."));
    EfiReleaseLock (&Private->Lock);
    return EFI_NOT_STARTED;
  }

  Private->Function = EFI_PXE_BASE_CODE_FUNCTION_TCP_WRITE;

  //
  // Issue BC command
  //
  StatCode = TcpWrite (
              Private,
              OpFlags,
              UrgentPointer,
              SequenceNumber,
              AckNumber,
              HlenResCode,
              Window,
              DestIpPtr,
              DestPortPtr,
              GatewayIpPtr,
              SrcIpPtr,
              SrcPortPtr,
              HeaderSizePtr,
              HeaderPtr,
              BufferSizePtr,
              BufferPtr
              );

  //
  // Unlock the instance data
  //
  EfiReleaseLock (&Private->Lock);
  return StatCode;
}
//
// /////////////////////////////////////////////////////////////////////
//
//  Udp Read Routine - called by base code - e.g. TFTP - already locked
//
EFI_STATUS
TcpRead (
  IN PXE_BASECODE_DEVICE            *Private,
  IN UINT16                         OpFlags,
  IN OUT EFI_IP_ADDRESS             *DestIpPtr, OPTIONAL
  IN OUT EFI_PXE_BASE_CODE_TCP_PORT *DestPortPtr, OPTIONAL
  IN OUT EFI_IP_ADDRESS             *SrcIpPtr, OPTIONAL
  IN OUT EFI_PXE_BASE_CODE_TCP_PORT *SrcPortPtr, OPTIONAL
  IN UINTN                          *HeaderSizePtr, OPTIONAL
  IN VOID                           *HeaderPtr,
  IN OUT UINTN                      *BufferSizePtr,
  IN VOID                           *BufferPtr
  )
/*++
Routine description:
  TCP read packet.

Parameters:
  Private := Pointer to PxeBc interface
  OpFlags := 
  DestIpPtr := 
  DestPortPtr := 
  SrcIpPtr := 
  SrcPortPtr := 
  HeaderSizePtr := 
  HeaderPtr := 
  BufferSizePtr := 
  BufferPtr := 

Returns:
  EFI_SUCCESS := 
  EFI_DEVICE_ERROR := 
  EFI_INVALID_PARAMETER := 
  other := 
--*/
{
  EFI_IP_ADDRESS  TmpSrcIp;
  EFI_IP_ADDRESS  TmpDestIp;
  EFI_STATUS      StatCode;
  UINTN           BufferSize;
  UINTN           HeaderSize;

  //
  // combination structure of pseudo header/tcp header
  //
#pragma pack (1)
  struct {
    IPV4_HEADER         Ipv4Hdr;
    TCPV4_PSEUDO_HEADER Tcpv4Phdr;
    TCPV4_HEADER        Tcpv4Hdr;
    UINT8               ProtHdr[64];
  } Hdrs;
#pragma pack ()

  HeaderSize = (HeaderSizePtr != NULL) ? *HeaderSizePtr : 0;
  //
  // Yes, I now require a Header Allocated
  //
  if (HeaderPtr == 0) {
    return EFI_DEVICE_ERROR;
  }

  HeaderSize = sizeof Hdrs.Ipv4Hdr + sizeof Hdrs.Tcpv4Hdr;
  EfiZeroMem (Hdrs.ProtHdr, 64);

  Hdrs.ProtHdr[0] = 'M';
  Hdrs.ProtHdr[1] = 'A';
  Hdrs.ProtHdr[2] = 'R';
  Hdrs.ProtHdr[3] = 'M';
  Hdrs.ProtHdr[4] = 'A';
  Hdrs.ProtHdr[5] = 'R';

  DEBUG ((EFI_D_NET, "\nTcpRead()  BufferSize = %xh", *BufferSizePtr));

  //
  // read [with filtering]
  // check parameters
  //
  if (BufferSizePtr == NULL ||
      BufferPtr == NULL ||
      (HeaderSize != 0 && HeaderPtr == NULL) ||
      (OpFlags &~UDP_FILTER_MASK)
      //
      // if filtering on a particular IP/Port, need it
      //
      ||
      (!(OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_IP) && SrcIpPtr == NULL) ||
      (!(OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_PORT) && SrcPortPtr == NULL) ||
      (!(OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_DEST_PORT) && DestPortPtr == NULL)
      ) {
    DEBUG ((EFI_D_INFO, "\nTcpRead()  Exit #1  Invalid Parameter"));
    return EFI_INVALID_PARAMETER;
  }

  BufferSize = *BufferSizePtr;
  //
  // in case we loop
  //
  // we need source and dest IPs for pseudo header
  //
  if (SrcIpPtr == NULL) {
    SrcIpPtr = &TmpSrcIp;
  }

  if (DestIpPtr == NULL) {
    DestIpPtr = &TmpDestIp;
    TmpDestIp = Private->EfiBc.Mode->StationIp;
  }

  for (;;) {
    *BufferSizePtr = BufferSize;

    DEBUG ((EFI_D_NET, "\nSize of Hdrs.Tcpv4Hdr = %d", sizeof Hdrs.Tcpv4Hdr));

    //
    // Let's receive the IP and TCP header at the Hdrs.Ipv4Hdr location
    // and the data for the TCP will be passed back in the BufferPtr.
    //
    StatCode = IpReceive (
                Private,
                OpFlags,
                SrcIpPtr,
                DestIpPtr,
                PROT_TCP,
                HeaderPtr,
                HeaderSize,
                BufferPtr,
                BufferSizePtr,
                0
                );

    EfiCopyMem (&Hdrs.Ipv4Hdr, HeaderPtr, HeaderSize);

    DEBUG (
      (EFI_D_NET,
      "\nTcpRead()  BufferSize = %xh  Ipv+Tcp = %d",
      *BufferSizePtr,
      sizeof Hdrs.Ipv4Hdr + sizeof Hdrs.Tcpv4Hdr)
      );

    DEBUG (
      (EFI_D_NET,
      "\nTcpRead()  Destination IP address is:  %d.%d.%d.%d\n",
      Hdrs.Ipv4Hdr.DestAddr.B[0],
      Hdrs.Ipv4Hdr.DestAddr.B[1],
      Hdrs.Ipv4Hdr.DestAddr.B[2],
      Hdrs.Ipv4Hdr.DestAddr.B[3])
      );

    DEBUG (
      (EFI_D_NET,
      "\nTcpRead()  Source IP address is:  %d.%d.%d.%d\n",
      Hdrs.Ipv4Hdr.SrcAddr.B[0],
      Hdrs.Ipv4Hdr.SrcAddr.B[1],
      Hdrs.Ipv4Hdr.SrcAddr.B[2],
      Hdrs.Ipv4Hdr.SrcAddr.B[3])
      );

    if (StatCode == EFI_SUCCESS || StatCode == EFI_BUFFER_TOO_SMALL) {
      UINT16  SPort;
      UINT16  DPort;

      SPort = NTOHS (Hdrs.Tcpv4Hdr.SrcPort);
      DPort = NTOHS (Hdrs.Tcpv4Hdr.DestPort);

      //
      // do filtering
      //
      if (!(OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_SRC_PORT) && *SrcPortPtr != SPort) {
        continue;
      }

      if (!(OpFlags & EFI_PXE_BASE_CODE_UDP_OPFLAGS_ANY_DEST_PORT) && *DestPortPtr != DPort) {
        continue;
      }
      //
      // check checksum
      //
      if (StatCode == EFI_SUCCESS && Hdrs.Tcpv4Hdr.Checksum) {
        Hdrs.Tcpv4Phdr.SrcAddr.L    = SrcIpPtr->Addr[0];
        Hdrs.Tcpv4Phdr.DestAddr.L   = DestIpPtr->Addr[0];
        Hdrs.Tcpv4Phdr.Zero         = 0;
        Hdrs.Tcpv4Phdr.Protocol     = PROT_TCP;
        Hdrs.Tcpv4Phdr.TotalLength  = (UINT16) (NTOHS (Hdrs.Ipv4Hdr.TotalLength) - sizeof Hdrs.Ipv4Hdr);
        Hdrs.Tcpv4Phdr.TotalLength  = HTONS (Hdrs.Tcpv4Phdr.TotalLength);

        if (Hdrs.Tcpv4Hdr.Checksum == 0xffff) {
          Hdrs.Tcpv4Hdr.Checksum = 0;
        }
        //
        // The HeaderPtr has the IP header in it, let's skip it and start the
        // checksum at the TCP pseudo header.
        //
        if (IpChecksum2 (
              (UINT16 *) HeaderPtr + sizeof Hdrs.Ipv4Hdr,
              sizeof Hdrs.Tcpv4Hdr + sizeof Hdrs.Tcpv4Phdr,
              (UINT16 *) BufferPtr,
              *BufferSizePtr
              )) {
          DEBUG (
            (EFI_D_NET,
            "\nTcpRead()  Hdrs.Ipv4hdr == %xh",
            &Hdrs.Ipv4Hdr)
            );
          DEBUG (
            (EFI_D_NET,
            "\nTcpRead()  Hdrs.Tcpv4hdr == %xh",
            &Hdrs.Tcpv4Hdr)
            );
          DEBUG ((EFI_D_NET, "\nTcpRead()  Header size == %d", HeaderSize));
          DEBUG ((EFI_D_NET, "\nTcpRead()  BufferPtr == %xh", BufferPtr));
          DEBUG ((EFI_D_NET, "\nTcpRead()  Buffer size == %d", *BufferSizePtr));
          DEBUG ((EFI_D_NET, "\nTcpRead()  Exit #2  Device Error"));

          //
          // Invalid checksum for a zero lenght buffer is okay.
          //
          if (*BufferSizePtr > 0) {
            return EFI_DEVICE_ERROR;
          }
        }
      }

      DEBUG ((EFI_D_NET, "\nTcpRead()  PASSED!!!!!!!!"));

      //
      // all passed
      //
      if (SrcPortPtr != NULL) {
        *SrcPortPtr = SPort;
      }

      if (DestPortPtr != NULL) {
        *DestPortPtr = DPort;
      }
    }

    if ((StatCode != EFI_SUCCESS) && (StatCode != EFI_TIMEOUT)) {
      DEBUG (
        (EFI_D_INFO,
        "\nTcpRead()  Exit #3  %Xh %r",
        StatCode,
        StatCode)
        );
    }

    return StatCode;
  }
}
//
// //////////////////////////////////////////////////////////
//
//  BC Udp Read Routine
//
EFI_STATUS
EFIAPI
BcTcpRead (
  IN EFI_PXE_BASE_CODE_PROTOCOL     *This,
  IN UINT16                         OpFlags,
  IN OUT EFI_IP_ADDRESS             *DestIp, OPTIONAL
  IN OUT EFI_PXE_BASE_CODE_TCP_PORT *DestPort, OPTIONAL
  IN OUT EFI_IP_ADDRESS             *SrcIp, OPTIONAL
  IN OUT EFI_PXE_BASE_CODE_TCP_PORT *SrcPort, OPTIONAL
  IN UINTN                          *HeaderSize, OPTIONAL
  IN VOID                           *HeaderPtr, OPTIONAL
  IN OUT UINTN                      *BufferSize,
  IN VOID                           *BufferPtr
  )
/*++
Routine description:
  TCP read API entry point.

Parameters:
  This := Pointer to PxeBc interface
  OpFlags := 
  DestIp := 
  DestPort := 
  SrcIp := 
  SrcPort := 
  HeaderSize := 
  HeaderPtr := 
  BufferSize := 
  BufferPtr := 

Returns:
  EFI_SUCCESS := 
  other := 
--*/
{
  EFI_STATUS          StatCode;
  PXE_BASECODE_DEVICE *Private;

  //
  // Lock the instance data and make sure started
  //
  StatCode = EFI_SUCCESS;

  if (This == NULL) {
    DEBUG ((EFI_D_ERROR, "BC *This pointer == NULL"));
    return EFI_INVALID_PARAMETER;
  }

  Private = CR (This, PXE_BASECODE_DEVICE, EfiBc, PXE_BASECODE_DEVICE_SIGNATURE);

  if (Private == NULL) {
    DEBUG ((EFI_D_ERROR, "PXE_BASECODE_DEVICE poiner == NULL"));
    return EFI_INVALID_PARAMETER;
  }

  EfiAcquireLock (&Private->Lock);

  if (This->Mode == NULL || !This->Mode->Started) {
    DEBUG ((EFI_D_ERROR, "BC was not started."));
    EfiReleaseLock (&Private->Lock);
    return EFI_NOT_STARTED;
  }

  Private->Function = EFI_PXE_BASE_CODE_FUNCTION_TCP_READ;

  //
  // Issue BC command
  //
  StatCode = TcpRead (
              Private,
              OpFlags,
              DestIp,
              DestPort,
              SrcIp,
              SrcPort,
              HeaderSize,
              HeaderPtr,
              BufferSize,
              BufferPtr
              );

  //
  // Unlock the instance data and return
  //
  EfiReleaseLock (&Private->Lock);
  return StatCode;
}

/* eof - pxe_bc_tcp.c */
