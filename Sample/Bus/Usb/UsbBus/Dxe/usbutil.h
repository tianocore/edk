/*++
 
Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    usbutil.h
  
  Abstract:
 
    Helper functions for USB
 
  Revision History
 
  
--*/

#ifndef _USB_UTIL_H
#define _USB_UTIL_H

//
// Following APIs are used to query Port Status
//
BOOLEAN
IsPortConnect (
  IN UINT16  PortStatus
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PortStatus  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsPortEnable (
  IN UINT16  PortStatus
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PortStatus  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsPortInReset (
  IN UINT16  PortStatus
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PortStatus  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsPortPowerApplied (
  IN UINT16  PortStatus
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PortStatus  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsPortLowSpeedDeviceAttached (
  IN UINT16  PortStatus
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PortStatus  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsPortSuspend (
  IN UINT16  PortStatus
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PortStatus  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

//
// Following APIs are used to query Port Change Status
//
BOOLEAN
IsPortConnectChange (
  IN UINT16  PortChangeStatus
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PortChangeStatus  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsPortEnableDisableChange (
  IN UINT16  PortChangeStatus
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PortChangeStatus  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsPortResetChange (
  IN UINT16  PortChangeStatus
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PortChangeStatus  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsPortSuspendChange (
  IN UINT16  PortChangeStatus
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  PortChangeStatus  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
