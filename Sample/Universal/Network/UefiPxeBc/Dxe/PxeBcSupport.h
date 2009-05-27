/*++

Copyright (c) 2007 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  PxeBcSupport.h
 
Abstract:

  Support routines for PxeBc

--*/

#ifndef __EFI_PXEBC_SUPPORT_H__
#define __EFI_PXEBC_SUPPORT_H__

EFI_STATUS
GetSmbiosSystemGuidAndSerialNumber (
  IN  EFI_GUID  *SystemGuid,
  OUT CHAR8     **SystemSerialNumber
  );

VOID
PxeBcCommonNotify (
  IN EFI_EVENT           Event,
  IN VOID                *Context
  )
/*++

Routine Description:

  GC_NOTO: Add function description

Arguments:

  Event   - GC_NOTO: add argument description
  Context - GC_NOTO: add argument description

Returns:

  GC_NOTO: add return values

--*/
;

EFI_STATUS
PxeBcConfigureUdpWriteInstance (
  IN EFI_UDP4_PROTOCOL  *Udp4,
  IN EFI_IPv4_ADDRESS   *StationIp,
  IN EFI_IPv4_ADDRESS   *SubnetMask,
  IN EFI_IPv4_ADDRESS   *Gateway,
  IN OUT UINT16         *SrcPort
  );

BOOLEAN
PxeBcCheckIpByFilter (
  EFI_PXE_BASE_CODE_MODE    *PxeBcMode,
  EFI_UDP4_SESSION_DATA     *Session
  );

VOID
CvtNum (
  IN UINTN  Number,
  IN UINT8  *Buffer,
  IN INTN   Length
  );

UINTN
UtoA10 (
  UINTN Number,
  UINT8 *BufferPtr
  )
/*++

Routine Description:

  GC_NOTO: Add function description

Arguments:

  Number    - GC_NOTO: add argument description
  BufferPtr - GC_NOTO: add argument description

Returns:

  GC_NOTO: add return values

--*/
;

UINT64
AtoU64 (
  UINT8 *BufferPtr
  )
/*++

Routine Description:

  GC_NOTO: Add function description

Arguments:

  BufferPtr - GC_NOTO: add argument description

Returns:

  GC_NOTO: add return values

--*/
;

#endif

