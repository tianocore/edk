/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  Ip4Option.c

Abstract:

  IP4 option support functions

--*/

#include "Ip4Impl.h"

BOOLEAN
Ip4OptionIsValid (
  IN UINT8                  *Option,
  IN UINT32                 OptionLen,
  IN BOOLEAN                Rcvd
  )
/*++

Routine Description:

  Validate the IP4 option format for both the packets we received 
  and will transmit. It will compute the ICMP error message fields
  if the option is mal-formated. But this information isn't used.

Arguments:

  Option  - The first byte of the option
  OptionLen  - The length of the whole option
  Rcvd    - The option is from the packet we received if TRUE,
            otherwise the option we wants to transmit.

Returns:

  TRUE:  The option is properly formated
  FALSE: The option is mal-formated

--*/
{
  UINT32                    Cur;
  UINT32                    Len;
  UINT32                    Point;
  UINT8                     IcmpType;
  UINT8                     IcmpCode;
  UINT32                    IcmpPoint;

  IcmpType  = ICMP_PARAMETER_PROBLEM;
  IcmpCode  = 0;
  IcmpPoint = 0;

  Cur       = 0;

  while (Cur < OptionLen) {
    switch (Option[Cur]) {
    case IP4_OPTION_NOP:
      Cur++;
      break;

    case IP4_OPTION_EOP:
      Cur = OptionLen;
      break;

    case IP4_OPTION_LSRR:
    case IP4_OPTION_SSRR:
    case IP4_OPTION_RR:
      Len   = Option[Cur + 1];
      Point = Option[Cur + 2];

      //
      // SRR/RR options are formated as |Type|Len|Point|Ip1|Ip2|...
      //
      if ((OptionLen - Cur < Len) || (Len < 3) || ((Len - 3) % 4 != 0)) {
        IcmpPoint = Cur + 1;
        return FALSE;
      }

      if ((Point > Len + 1) || (Point % 4 != 0)) {
        IcmpPoint = Cur + 2;
        return FALSE;
      }
      
      //
      // The Point must point pass the last entry if the packet is received
      // by us. It must point to 4 if the packet is to be sent by us for
      // source route option.
      //
      if ((Option[Cur] != IP4_OPTION_RR) && 
          ((Rcvd && (Point != Len + 1)) || (!Rcvd && (Point != 4)))) {
          
        IcmpType  = ICMP_DEST_UNREACHABLE;
        IcmpCode  = ICMP_SOURCEROUTE_FAILED;
        return FALSE;
      }

      Cur += Len;
      break;

    default:
      Len = Option[Cur + 1];

      if ((OptionLen - Cur < Len) || (Len < 2)) {
        IcmpPoint = Cur + 1;
        return FALSE;
      }

      Cur = Cur + Len;
      break;
    }

  }

  return TRUE;
}

EFI_STATUS
Ip4CopyOption (
  IN UINT8                  *Option,
  IN UINT32                 OptionLen,
  IN BOOLEAN                FirstFragment,
  IN UINT8                  *Buf,           OPTIONAL
  IN OUT UINT32             *BufLen
  )
/*++

Routine Description:

  Copy the option from the original option to buffer. It 
  handles the details such as:
    1. whether copy the single IP4 option to the first/non-first
       fragments.
    2. Pad the options copied over to aligened to 4 bytes.

Arguments:

  Option        - The original option to copy from
  OptionLen     - The length of the original option
  FirstFragment - Whether it is the first fragment
  Buf           - The buffer to copy options to
  BufLen        - The length of the buffer

Returns:

  EFI_SUCCESS          - The options are copied over
  EFI_BUFFER_TOO_SMALL - The buffer caller provided is too small.

--*/
{
  UINT8                     OptBuf[40];
  UINT32                    Cur;
  UINT32                    Next;
  UINT8                     Type;
  UINT32                    Len;

  ASSERT ((BufLen != NULL) && (OptionLen <= 40));

  Cur   = 0;
  Next  = 0;

  while (Cur < OptionLen) {
    Type  = Option[Cur];
    Len   = Option[Cur + 1];

    if (Type == IP4_OPTION_NOP) {
      //
      // Keep the padding, in case that the sender wants to align
      // the option, say, to 4 bytes
      //
      OptBuf[Next] = IP4_OPTION_NOP;
      Next++;
      Cur++;

    } else if (Type == IP4_OPTION_EOP) {
      //
      // Don't append the EOP to avoid including only a EOP option
      //
      break;

    } else {
      //
      // don't copy options that is only valid for the first fragment
      //
      if (FirstFragment || (Type & IP4_OPTION_COPY_MASK)) {
        NetCopyMem (OptBuf + Next, Option + Cur, Len);
        Next += Len;
      }

      Cur += Len;
    }
  }
  
  //
  // Don't append an EOP only option.
  //
  if (Next == 0) {
    *BufLen = 0;
    return EFI_SUCCESS;
  }

  //
  // Append an EOP if the end of option doesn't coincide with the 
  // end of the IP header, that is, isn't aligned to 4 bytes..
  //
  if ((Next % 4) != 0) {
    OptBuf[Next] = IP4_OPTION_EOP;
    Next++;
  }
  
  //
  // Head length is in the unit of 4 bytes. Now, Len is the 
  // acutal option length to appear in the IP header.
  //
  Len = ((Next + 3) &~0x03);

  //
  // If the buffer is too small, set the BufLen then return 
  //
  if ((Buf == NULL) || (*BufLen < Len)) {
    *BufLen = Len;
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Copy the option to the Buf, zero the buffer first to pad
  // the options with NOP to align to 4 bytes.
  //
  NetZeroMem (Buf, Len);
  NetCopyMem (Buf, OptBuf, Next);
  *BufLen = Len;
  return EFI_SUCCESS;
}
