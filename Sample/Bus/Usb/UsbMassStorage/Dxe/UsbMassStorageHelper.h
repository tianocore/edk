/*++
Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    UsbMassStorageHelper.h

Abstract:

    Function prototype for USB Mass Storage Driver

Revision History
++*/

// TODO: fix comment to end with --*/
#ifndef _USB_FLPHLP_H
#define _USB_FLPHLP_H

#include "UsbMassStorage.h"

EFI_STATUS
USBFloppyIdentify (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
USBFloppyPacketCommand (
  USB_FLOPPY_DEV            *UsbFloppyDevice,
  VOID                      *Command,
  UINT8                     CommandSize,
  VOID                      *DataBuffer,
  UINT32                    BufferLength,
  EFI_USB_DATA_DIRECTION    Direction,
  UINT16                    TimeOutInMilliSeconds
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice       - TODO: add argument description
  Command               - TODO: add argument description
  CommandSize           - TODO: add argument description
  DataBuffer            - TODO: add argument description
  BufferLength          - TODO: add argument description
  Direction             - TODO: add argument description
  TimeOutInMilliSeconds - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
USBFloppyInquiry (
  IN    USB_FLOPPY_DEV    *UsbFloppyDevice,
  OUT   USB_INQUIRY_DATA  **Idata
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description
  Idata           - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
USBFloppyRead10 (
  IN    USB_FLOPPY_DEV    *UsbFloppyDevice,
  IN    VOID              *Buffer,
  IN    EFI_LBA           Lba,
  IN    UINTN             NumberOfBlocks
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description
  Buffer          - TODO: add argument description
  Lba             - TODO: add argument description
  NumberOfBlocks  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
USBFloppyReadFormatCapacity (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
UsbFloppyRequestSense (
  IN  USB_FLOPPY_DEV  *UsbFloppyDevice,
  OUT UINTN           *SenseCounts
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description
  SenseCounts     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
UsbFloppyTestUnitReady (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
USBFloppyWrite10 (
  IN    USB_FLOPPY_DEV    *UsbFloppyDevice,
  IN    VOID              *Buffer,
  IN    EFI_LBA           Lba,
  IN    UINTN             NumberOfBlocks
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description
  Buffer          - TODO: add argument description
  Lba             - TODO: add argument description
  NumberOfBlocks  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
UsbFloppyDetectMedia (
  IN  USB_FLOPPY_DEV  *UsbFloppyDevice,
  OUT BOOLEAN         *MediaChange
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description
  MediaChange     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
UsbFloppyModeSense5APage5 (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
UsbFloppyModeSense5APage1C (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
UsbFloppyModeSense5APage3F (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
UsbSCSIModeSense1APage3F (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
UsbMassStorageModeSense (
  IN  USB_FLOPPY_DEV    *UsbFloppyDevice
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  UsbFloppyDevice - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
