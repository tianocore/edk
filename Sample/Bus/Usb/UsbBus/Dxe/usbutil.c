/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    usbutil.c

  Abstract:

    Helper functions for USB

  Revision History

--*/

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "usb.h"
#include "usbutil.h"

//
// Following APIs are used to query Port Status
//
BOOLEAN
IsPortConnect (
  IN UINT16  PortStatus
  )
/*++

  Routine Description:
    Tell if there is a device connected to that port according to
    the Port Status.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    PortStatus - add argument and description to function comment
{
  //
  // return the bit 0 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_CONNECTION) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortEnable (
  IN UINT16  PortStatus
  )
/*++

  Routine Description:
    Tell if Port is enabled.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    PortStatus - add argument and description to function comment
{
  //
  // return the bit 1 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_ENABLE) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortInReset (
  IN UINT16  PortStatus
  )
/*++

  Routine Description:
    Tell if the port is being reset.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    PortStatus - add argument and description to function comment
{
  //
  // return the bit 4 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_RESET) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortPowerApplied (
  IN UINT16  PortStatus
  )
/*++

  Routine Description:
    Tell if there is power applied to that port.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    PortStatus - add argument and description to function comment
{
  //
  // return the bit 8 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_POWER) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortLowSpeedDeviceAttached (
  IN UINT16  PortStatus
  )
/*++

  Routine Description:
    Tell if the connected device is a low device.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    PortStatus - add argument and description to function comment
{
  //
  // return the bit 9 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_LOW_SPEED) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortSuspend (
  IN UINT16  PortStatus
  )
/*++

  Routine Description:
    Tell if the port is suspend.

  Parameters:
    PortStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    PortStatus - add argument and description to function comment
{
  //
  // return the bit 2 value of PortStatus
  //
  if ((PortStatus & USB_PORT_STAT_SUSPEND) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}
//
// Following APIs are used to query Port Change Status
//
BOOLEAN
IsPortConnectChange (
  IN UINT16  PortChangeStatus
  )
/*++

  Routine Description:
    Tell if there is a Connect Change status in that port.

  Parameters:
    PortChangeStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    PortChangeStatus - add argument and description to function comment
{
  //
  // return the bit 0 value of PortChangeStatus
  //
  if ((PortChangeStatus & USB_PORT_STAT_C_CONNECTION) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortEnableDisableChange (
  IN UINT16  PortChangeStatus
  )
/*++

  Routine Description:
    Tell if there is a Enable/Disable change in that port.

  Parameters:
    PortChangeStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    PortChangeStatus - add argument and description to function comment
{
  //
  // return the bit 1 value of PortChangeStatus
  //
  if ((PortChangeStatus & USB_PORT_STAT_C_ENABLE) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortResetChange (
  IN UINT16  PortChangeStatus
  )
/*++

  Routine Description:
    Tell if there is a Port Reset Change status in that port.

  Parameters:
    PortChangeStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    PortChangeStatus - add argument and description to function comment
{
  //
  // return the bit 4 value of PortChangeStatus
  //
  if ((PortChangeStatus & USB_PORT_STAT_C_RESET) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsPortSuspendChange (
  IN UINT16  PortChangeStatus
  )
/*++

  Routine Description:
    Tell if there is a Suspend Change Status in that port.

  Parameters:
    PortChangeStatus  -   The status value of that port.

  Return Value:
    TRUE
    FALSE

--*/
// TODO: function comment is missing 'Arguments:'
// TODO: function comment is missing 'Returns:'
// TODO:    PortChangeStatus - add argument and description to function comment
{
  //
  // return the bit 2 value of PortChangeStatus
  //
  if ((PortChangeStatus & USB_PORT_STAT_C_SUSPEND) != 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}
