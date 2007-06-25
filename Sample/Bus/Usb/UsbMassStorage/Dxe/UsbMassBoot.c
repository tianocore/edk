/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  UsbMassBoot.c
    
Abstract:

  This file implement the command set of "USB Mass Storage Specification 
  for Bootability". 

Revision History

--*/

#include "UsbMassImpl.h"

STATIC
UINT32
UsbBootGetUint32 (
  IN UINT8                  *Buf
  )
/*++

Routine Description:

  Read an UINT32 from the buffer to avoid byte alignment problems, then 
  convert that to the little endia. The USB mass storage bootability spec
  use big endia
  
Arguments:

  Buf - The buffer contains the first byte of the UINT32 in big endia.

Returns:

  The UINT32 value read from the buffer in little endia.

--*/
{
  UINT32                    Value;

  EfiCopyMem (&Value, Buf, sizeof (UINT32));
  return USB_BOOT_SWAP32 (Value);
}

STATIC
VOID
UsbBootPutUint32 (
  IN UINT8                  *Buf,
  IN UINT32                 Data32
  )
/*++

Routine Description:

  Put an UINT32 in little endia to the buffer. The data is converted to
  big endia before writing.

Arguments:

  Buf     - The buffer to write data to
  Data32  - The data to write.

Returns:

  None

--*/
{
  Data32 = USB_BOOT_SWAP32 (Data32);
  EfiCopyMem (Buf, &Data32, sizeof (UINT32));
}

STATIC
VOID
UsbBootPutUint16 (
  IN UINT8                   *Buf,
  IN UINT16                  Data16
  )
/*++

Routine Description:

  Put an UINT16 in little endia to the buffer. The data is converted to
  big endia before writing.

Arguments:

  Buf     - The buffer to write data to
  Data16  - The data to write

Returns:

  None

--*/
{
  Data16 = USB_BOOT_SWAP16 (Data16);
  EfiCopyMem (Buf, &Data16, sizeof (UINT16));
}

EFI_STATUS
UsbBootRequestSense (
  IN USB_MASS_DEVICE          *UsbMass
  )
/*++

  Routine Description:
    Request sense information via sending Request Sense
    Packet Command.
  
  Arguments:
    UsbMass          - The device to be requested sense data
      
  Returns:  
    EFI_DEVICE_ERROR - Hardware error
    EFI_SUCCESS      - Success

--*/         
{ 
  USB_BOOT_REQUEST_SENSE_CMD  SenseCmd;
  USB_BOOT_REQUEST_SENSE_DATA SenseData;
  EFI_BLOCK_IO_MEDIA          *Media;
  USB_MASS_TRANSPORT          *Transport;
  EFI_STATUS                  Status;
  UINT32                      CmdResult;

  Transport = UsbMass->Transport;

  //
  // Request the sense data from the device if command failed
  //
  EfiZeroMem (&SenseCmd, sizeof (USB_BOOT_REQUEST_SENSE_CMD));
  EfiZeroMem (&SenseData, sizeof (USB_BOOT_REQUEST_SENSE_DATA));
  
  SenseCmd.OpCode   = USB_BOOT_REQUEST_SENSE_OPCODE;
  SenseCmd.Lun      = USB_BOOT_LUN (UsbMass->Lun);
  SenseCmd.AllocLen = sizeof (USB_BOOT_REQUEST_SENSE_DATA);

  Status = Transport->ExecCommand (
                        UsbMass->Context,
                        &SenseCmd,
                        sizeof (USB_BOOT_REQUEST_SENSE_CMD),
                        EfiUsbDataIn,
                        &SenseData,
                        sizeof (USB_BOOT_REQUEST_SENSE_DATA),
                        USB_BOOT_GENERAL_CMD_TIMEOUT,
                        &CmdResult
                        );
  if (EFI_ERROR (Status) || CmdResult != USB_MASS_CMD_SUCCESS) {
    DEBUG ((mUsbMscError, "UsbBootRequestSense: (%r) CmdResult=0x%x\n", Status, CmdResult));
    return EFI_DEVICE_ERROR;
  }

  //
  // Interpret the sense data and update the media status if necessary.
  //
  Media = &UsbMass->BlockIoMedia;

  switch (USB_BOOT_SENSE_KEY (SenseData.SenseKey)) {
    
  case USB_BOOT_SENSE_NO_SENSE:
  case USB_BOOT_SENSE_RECOVERED:
    //
    // Suppose hardware can handle this case, and recover later by itself
    //
    Status = EFI_NOT_READY;
    break;

  case USB_BOOT_SENSE_NOT_READY:
    switch (SenseData.ASC) {    
    case USB_BOOT_ASC_NO_MEDIA:
      Status              = EFI_NO_MEDIA;
      Media->MediaPresent = FALSE;
      break;
      
    case USB_BOOT_ASC_MEDIA_UPSIDE_DOWN:
      Status              = EFI_DEVICE_ERROR;
      Media->MediaPresent = FALSE;
      break;
      
    case USB_BOOT_ASC_NOT_READY:
      if (SenseData.ASCQ == USB_BOOT_ASCQ_IN_PROGRESS || 
          SenseData.ASCQ == USB_BOOT_ASCQ_DEVICE_BUSY) {
        //
        // Regular timeout, and need retry once more
        //
        DEBUG ((mUsbMscInfo, "UsbBootRequestSense: Not ready and need retry once more\n"));
        Status = EFI_NOT_READY; 
      }
    }
    break;

  case USB_BOOT_SENSE_ILLEGAL_REQUEST:
    Status = EFI_INVALID_PARAMETER;
    break;

  case USB_BOOT_SENSE_UNIT_ATTENTION:
    Status = EFI_DEVICE_ERROR;
    if (SenseData.ASC == USB_BOOT_ASC_MEDIA_CHANGE) {
      Status = EFI_MEDIA_CHANGED;
      UsbMass->BlockIoMedia.MediaId++;
    }
    break;

  case USB_BOOT_SNESE_DATA_PROTECT:
    Status                          = EFI_WRITE_PROTECTED;
    UsbMass->BlockIoMedia.ReadOnly  = TRUE;
    break;

  default:
    Status = EFI_DEVICE_ERROR;
    break;
  }

  DEBUG ((mUsbMscInfo, "UsbBootRequestSense: (%r) with sense key %x/%x/%x\n", 
          Status,
          USB_BOOT_SENSE_KEY (SenseData.SenseKey),
          SenseData.ASC,
          SenseData.ASCQ
          ));
  
  return Status;
}

STATIC
EFI_STATUS
UsbBootExecCmd (
  IN USB_MASS_DEVICE            *UsbMass,
  IN VOID                       *Cmd,
  IN UINT8                      CmdLen,
  IN EFI_USB_DATA_DIRECTION     DataDir,
  IN VOID                       *Data,
  IN UINT32                     DataLen,
  IN UINT32                     Timeout
  )
/*++

Routine Description:

  Execute the USB mass storage bootability commands. If execution
  failed, retrieve the error by REQUEST_SENSE then update the device's
  status, such as ReadyOnly.

Arguments:

  UsbMass - The device to issue commands to
  Cmd     - The command to execute
  CmdLen  - The length of the command
  DataDir - The direction of data transfer
  Data    - The buffer to hold the data
  DataLen - The length of expected data
  Timeout - The timeout used to transfer
    
Returns:

  EFI_SUCCESS           - The command is excuted OK
  EFI_DEVICE_ERROR      - Failed to request sense
  EFI_INVALID_PARAMETER - The command has some invalid parameters
  EFI_WRITE_PROTECTED   - The device is write protected
  EFI_MEDIA_CHANGED     - The device media has been changed

--*/
{
  USB_MASS_TRANSPORT          *Transport;
  EFI_STATUS                  Status;
  UINT32                      CmdResult;

  Transport = UsbMass->Transport;
  Status    = Transport->ExecCommand (
                           UsbMass->Context,
                           Cmd,
                           CmdLen,
                           DataDir,
                           Data,
                           DataLen,
                           Timeout,
                           &CmdResult
                           );
  //
  // ExecCommand return success and get the right CmdResult means
  // the commnad transfer is OK.
  //
  if ((CmdResult == USB_MASS_CMD_SUCCESS) && !EFI_ERROR(Status)) {
    return EFI_SUCCESS;
  }
  
  return UsbBootRequestSense (UsbMass);
}

STATIC
EFI_STATUS
UsbBootExecCmdWithRetry (
  IN USB_MASS_DEVICE          *UsbMass,
  IN VOID                     *Cmd,
  IN UINT8                    CmdLen,
  IN EFI_USB_DATA_DIRECTION   DataDir,
  IN VOID                     *Data,
  IN UINT32                   DataLen,
  IN UINT32                   Timeout 
  )
/*++

Routine Description:

  Execute the USB mass storage bootability commands. If execution
  failed, retrieve the error by REQUEST_SENSE then update the device's
  status, such as ReadyOnly.

Arguments:

  UsbMass - The device to issue commands to
  Cmd     - The command to execute
  CmdLen  - The length of the command
  DataDir - The direction of data transfer
  Data    - The buffer to hold the data
  DataLen - The length of expected data

Returns:

  EFI_SUCCESS           - The command is excuted OK
  EFI_DEVICE_ERROR      - Failed to request sense
  EFI_INVALID_PARAMETER - The command has some invalid parameters
  EFI_WRITE_PROTECTED   - The device is write protected
  EFI_MEDIA_CHANGED     - The device media has been changed

--*/
{
  EFI_STATUS                  Status;
  INT16                       Index;

  //
  // If the device isn't ready, wait some time. If the device is ready,
  // retry the command again.
  //
  Status  = EFI_SUCCESS;
    
  for (Index = 0; Index < USB_BOOT_COMMAND_RETRY; Index++) {
    //
    // Execute the command with an increasingly larger timeout value.
    //
    Status = UsbBootExecCmd (
               UsbMass, 
               Cmd, 
               CmdLen, 
               DataDir, 
               Data, 
               DataLen, 
               Timeout * (Index + 1)
               );
    if (Status == EFI_SUCCESS ||
        Status == EFI_MEDIA_CHANGED) {
      break;
    }
    //
    // Need retry once more, so reset index
    //
    if (Status == EFI_NOT_READY) {
      Index = 0;
    }
  }

  return Status;
}


EFI_STATUS
UsbBootIsUnitReady (
  IN USB_MASS_DEVICE            *UsbMass
  )
/*++

Routine Description:

  Use the TEST UNIT READY command to check whether it is ready.
  If it is ready, update the parameters.

Arguments:

  UsbMass - The device to test

Returns:

  EFI_SUCCESS - The device is ready and parameters are updated.
  Others      - Device not ready.

--*/
{
  USB_BOOT_TEST_UNIT_READY_CMD  TestCmd;
  
  EfiZeroMem (&TestCmd, sizeof (USB_BOOT_TEST_UNIT_READY_CMD));

  TestCmd.OpCode  = USB_BOOT_TEST_UNIT_READY_OPCODE;
  TestCmd.Lun     = USB_BOOT_LUN (UsbMass->Lun);
	
  return UsbBootExecCmdWithRetry (
           UsbMass, 
           &TestCmd, 
           sizeof (USB_BOOT_TEST_UNIT_READY_CMD), 
           EfiUsbNoData, 
           NULL, 
           0, 
           USB_BOOT_GENERAL_CMD_TIMEOUT
           );
}

EFI_STATUS
UsbBootInquiry (
  IN USB_MASS_DEVICE            *UsbMass
  )
/*++

Routine Description:

  Inquiry Command requests that information regrarding parameters of
  the Device be sent to the Host.

Arguments:

  UsbMass - The device to inquiry.

Returns:

  EFI_SUCCESS - The device is ready and parameters are updated.
  Others      - Device not ready.

--*/
{
  USB_BOOT_INQUIRY_CMD        InquiryCmd;
  USB_BOOT_INQUIRY_DATA       InquiryData;
  EFI_BLOCK_IO_MEDIA          *Media;
  EFI_STATUS                  Status;

  Media = &(UsbMass->BlockIoMedia);

  //
  // Use the Inquiry command to get the RemovableMedia setting.
  //
  EfiZeroMem (&InquiryCmd, sizeof (USB_BOOT_INQUIRY_CMD));
  EfiZeroMem (&InquiryData, sizeof (USB_BOOT_INQUIRY_DATA));

  InquiryCmd.OpCode   = USB_BOOT_INQUIRY_OPCODE;
  InquiryCmd.Lun      = USB_BOOT_LUN (UsbMass->Lun);
  InquiryCmd.AllocLen = sizeof (InquiryData);
		
  Status = UsbBootExecCmdWithRetry (
             UsbMass,
             &InquiryCmd,
             sizeof (USB_BOOT_INQUIRY_CMD),
             EfiUsbDataIn,
             &InquiryData,
             sizeof (USB_BOOT_INQUIRY_DATA),
             USB_BOOT_INQUIRY_CMD_TIMEOUT
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  UsbMass->Pdt          = USB_BOOT_PDT (InquiryData.Pdt);
  Media->RemovableMedia = USB_BOOT_REMOVABLE (InquiryData.Removable);
  //
  // Default value 512 Bytes, in case no media present at first time
  //
  Media->BlockSize      = 0x0200; 

  return Status;
}

EFI_STATUS
UsbBootReadCapacity (
  IN USB_MASS_DEVICE          *UsbMass
  )
/*++

Routine Description:

  Get the capacity of the USB mass storage media, including
  the presentation, block size, and last block number. This
  function is used to get the disk parameters at the start if
  it is a non-removable media or to detect the media if it is
  removable. 

Arguments:

  UsbMass - The device to retireve disk gemotric.

Returns:

  EFI_SUCCESS      - The disk gemotric is successfully retrieved.
  EFI_DEVICE_ERROR - Something is inconsistent with the disk gemotric.

--*/
{
  USB_BOOT_READ_CAPACITY_CMD  CapacityCmd;
  USB_BOOT_READ_CAPACITY_DATA CapacityData;
  EFI_BLOCK_IO_MEDIA          *Media;
  EFI_STATUS                  Status;

  Media   = &UsbMass->BlockIoMedia;
  
  //
  // Use the READ CAPACITY command to get the block length and last blockno
  //
  EfiZeroMem (&CapacityCmd, sizeof (USB_BOOT_READ_CAPACITY_CMD));
  EfiZeroMem (&CapacityData, sizeof (USB_BOOT_READ_CAPACITY_DATA));

  CapacityCmd.OpCode = USB_BOOT_READ_CAPACITY_OPCODE;
  CapacityCmd.Lun    = USB_BOOT_LUN (UsbMass->Lun);
	
  Status = UsbBootExecCmdWithRetry (
             UsbMass,
             &CapacityCmd,
             sizeof (USB_BOOT_READ_CAPACITY_CMD),
             EfiUsbDataIn,
             &CapacityData,
             sizeof (USB_BOOT_READ_CAPACITY_DATA),
             USB_BOOT_GENERAL_CMD_TIMEOUT
             );
  if (EFI_ERROR (Status)) { 
    return Status;
  }
  
  Media->MediaPresent = TRUE;
  Media->LastBlock    = UsbBootGetUint32 (CapacityData.LastLba);
  Media->BlockSize    = UsbBootGetUint32 (CapacityData.BlockLen);
  
  DEBUG ((mUsbMscInfo, "UsbBootReadCapacity Success LBA=%d BlockSize=%d\n", 
    Media->LastBlock, Media->BlockSize));
  
  return EFI_SUCCESS;
}

EFI_STATUS
UsbBootModeSense (
  IN USB_MASS_DEVICE          *UsbMass
  )
/*++

  Routine Description:
    Retrieves mode sense information via sending Mode Sense
    Packet Command.
  
  Arguments:
    UsbMass          - The USB_FLOPPY_DEV instance.
      
  Returns:  
    EFI_DEVICE_ERROR - Hardware error
    EFI_SUCCESS      - Success

--*/         
{ 
  EFI_STATUS                Status;
  USB_BOOT_MODE_SENSE_CMD   ModeSenseCmd;
  USB_BOOT_MODE_PARA_HEADER ModeParaHeader; 
  UINT8                     CommandSet;

  EfiZeroMem (&ModeSenseCmd, sizeof (USB_BOOT_MODE_SENSE_CMD));
  EfiZeroMem (&ModeParaHeader, sizeof (USB_BOOT_MODE_PARA_HEADER));

  //
  // overuse Context Pointer, the first field of Bot or Cbi is EFI_USB_INTERFACE_DESCRIPTOR
  //
  CommandSet = ((EFI_USB_INTERFACE_DESCRIPTOR *) (UsbMass->Context))->InterfaceSubClass;
  
  if (CommandSet == USB_MASS_STORE_SCSI) {
    //
    // Not UFI Command Set, no ModeSense Command
    //
    return EFI_SUCCESS;
  }

  ModeSenseCmd.OpCode         = USB_BOOT_MODE_SENSE10_OPCODE;
  ModeSenseCmd.PageCode       = 0x3f;
  ModeSenseCmd.ParaListLenLsb = (UINT8) sizeof (USB_BOOT_MODE_PARA_HEADER);

  Status = UsbBootExecCmdWithRetry (
             UsbMass,
             &ModeSenseCmd,
             sizeof (USB_BOOT_MODE_SENSE_CMD),
             EfiUsbDataIn,
             &ModeParaHeader,
             sizeof (USB_BOOT_MODE_PARA_HEADER),
             USB_BOOT_GENERAL_CMD_TIMEOUT
             );
  //
  // Did nothing with the Header here
  // But probably should
  //

  return Status;
}

EFI_STATUS
UsbBootGetParams (
  IN USB_MASS_DEVICE          *UsbMass
  )
/*++

Routine Description:

  Get the parameters for the USB mass storage media, including
  the RemovableMedia, block size, and last block number. This
  function is used both to initialize the media during the
  DriverBindingStart and to re-initialize it when the media is
  changed. Althought the RemoveableMedia is unlikely to change,
  I include it here.

Arguments:

  UsbMass - The device to retireve disk gemotric.

Returns:

  EFI_SUCCESS      - The disk gemotric is successfully retrieved.
  EFI_DEVICE_ERROR - Something is inconsistent with the disk gemotric.

--*/
{
  EFI_BLOCK_IO_MEDIA          *Media;
  EFI_STATUS                  Status;

  Status = UsbBootInquiry (UsbMass);
  if (EFI_ERROR (Status)) {
    DEBUG ((mUsbMscError, "UsbBootGetParams: UsbBootInquiry (%r)\n", Status));
    return Status;
  }

  Media = &(UsbMass->BlockIoMedia);
  //
  // Don't use the Removable bit in inquirydata to test whether the media 
  // is removable because many flash disks wrongly set this bit.
  //
  if ((UsbMass->Pdt == USB_PDT_CDROM) || (UsbMass->Pdt == USB_PDT_OPTICAL)) {
    //
    // CD-Rom or Optical device
    //
    UsbMass->OpticalStorage = TRUE;
    //
    // Default value 2048 Bytes, in case no media present at first time
    //
    Media->BlockSize        = 0x0800;
  } else {
    //
    // Non CD-Rom device need ModeSenseCmd between InquiryCmd and ReadCapacityCmd
    //
    Status = UsbBootModeSense (UsbMass);
    if (EFI_ERROR (Status)) {
      DEBUG ((mUsbMscError, "UsbBootGetParams: UsbBootModeSense (%r)\n", Status));
      return Status;
    }
  }
    
  return UsbBootReadCapacity (UsbMass);
}

EFI_STATUS
UsbBootDetectMedia (
  IN  USB_MASS_DEVICE       *UsbMass
  )
/*++

Routine Description:

  Detect whether the removable media is present and whether it has changed.
  The Non-removable media doesn't need it.

Arguments:

  UsbMass - The device to retireve disk gemotric.

Returns:

  EFI_SUCCESS      - The disk gemotric is successfully retrieved.
  EFI_DEVICE_ERROR - Something is inconsistent with the disk gemotric.

--*/
{   
  EFI_BLOCK_IO_MEDIA        OldMedia;
  EFI_BLOCK_IO_MEDIA        *Media;
  EFI_STATUS                Status;
  
  Media    = &UsbMass->BlockIoMedia;
  OldMedia = UsbMass->BlockIoMedia;
  
  //
  // First test whether the device is ready and get status
  // If media changed or ready, need read the device's capacity
  //
  Status = UsbBootIsUnitReady (UsbMass);
  if ((Status == EFI_SUCCESS && Media->MediaPresent) || 
      (Status == EFI_MEDIA_CHANGED)) {
    if ((UsbMass->Pdt != USB_PDT_CDROM) && 
        (UsbMass->Pdt != USB_PDT_OPTICAL)) {
      //
      // Non CD-Rom device need ModeSenseCmd between InquiryCmd and ReadCapacityCmd
      //
      UsbBootModeSense (UsbMass);
    }
    DEBUG ((mUsbMscInfo, "UsbBootDetectMedia: Need Read Capacity\n"));
    Status = UsbBootReadCapacity (UsbMass);
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Detect whether it is necessary to reinstall the BlockIO
  //
  if ((Media->MediaId != OldMedia.MediaId) || 
      (Media->MediaPresent != OldMedia.MediaPresent) || 
      (Media->ReadOnly != OldMedia.ReadOnly) || 
      (Media->BlockSize != OldMedia.BlockSize) || 
      (Media->LastBlock != OldMedia.LastBlock)) {
    DEBUG ((mUsbMscInfo, "UsbBootDetectMedia: Need reinstall BlockIoProtocol\n"));
    Media->MediaId++;
    gBS->ReinstallProtocolInterface (
           UsbMass->Controller,
           &gEfiBlockIoProtocolGuid,
           &UsbMass->BlockIo,
           &UsbMass->BlockIo
           );
    //
    // Check whether media present or media changed or write protected
    //
    if (Media->MediaPresent == FALSE) {
      Status = EFI_NO_MEDIA;
    } 
    if (Media->MediaId != OldMedia.MediaId) {
      Status = EFI_MEDIA_CHANGED;
    } 
    if (Media->ReadOnly != OldMedia.ReadOnly) {
      Status = EFI_WRITE_PROTECTED;
    }
  }
  
  return Status;
}

EFI_STATUS
UsbBootReadBlocks (
  IN  USB_MASS_DEVICE       *UsbMass,
  IN  UINT32                Lba,
  IN  UINTN                 TotalBlock,
  OUT UINT8                 *Buffer
  )
/*++

Routine Description:

  Read some blocks from the device.

Arguments:

  UsbMass     - The USB mass storage device to read from
  Lba         - The start block number
  TotalBlock  - Total block number to read
  Buffer      - The buffer to read to

Returns:

  EFI_SUCCESS - Data are read into the buffer
  Others      - Failed to read all the data

--*/
{
  USB_BOOT_READ10_CMD       ReadCmd;
  EFI_STATUS                Status;
  UINT16                    Count;
  UINT32                    BlockSize;
  UINT32                    ByteSize;
  UINT32                    Timeout;

  BlockSize = UsbMass->BlockIoMedia.BlockSize;
  Status    = EFI_SUCCESS;

  while (TotalBlock > 0) {
    //
    // Split the total blocks into smaller pieces to ease the pressure
    // on the device. We must split the total block because the READ10
    // command only has 16 bit transfer length (in the unit of block).
    //
    Count     = (UINT16)((TotalBlock < USB_BOOT_IO_BLOCKS) ? TotalBlock : USB_BOOT_IO_BLOCKS);
    ByteSize  = (UINT32)Count * BlockSize;

    //
    // Optical device need longer timeout than other device
    //
    if (UsbMass->OpticalStorage == TRUE) {
      Timeout = (UINT32)Count * USB_BOOT_OPTICAL_BLOCK_TIMEOUT;
    } else {
      Timeout = (UINT32)Count * USB_BOOT_GENERAL_BLOCK_TIMEOUT;
    }

    //
    // Fill in the command then execute 
    //
    EfiZeroMem (&ReadCmd, sizeof (USB_BOOT_READ10_CMD));
    
    ReadCmd.OpCode  = USB_BOOT_READ10_OPCODE;
    ReadCmd.Lun     = USB_BOOT_LUN (UsbMass->Lun);
    UsbBootPutUint32 (ReadCmd.Lba, Lba);
    UsbBootPutUint16 (ReadCmd.TransferLen, Count);
    
    Status = UsbBootExecCmdWithRetry (
               UsbMass,
               &ReadCmd,
               sizeof (USB_BOOT_READ10_CMD),
               EfiUsbDataIn,
               Buffer,
               ByteSize,
               Timeout
               );
    if (EFI_ERROR (Status)) {   
      return Status;
    }

    Lba        += Count;
    Buffer     += Count * BlockSize;
    TotalBlock -= Count;
  }

  return Status;
}

EFI_STATUS
UsbBootWriteBlocks (
  IN  USB_MASS_DEVICE         *UsbMass,
  IN  UINT32                  Lba,
  IN  UINTN                   TotalBlock,
  OUT UINT8                   *Buffer
  )
/*++

Routine Description:

  Write some blocks to the device.

Arguments:

  UsbMass     - The USB mass storage device to write to
  Lba         - The start block number
  TotalBlock  - Total block number to write
  Buffer      - The buffer to write to

Returns:

  EFI_SUCCESS - Data are written into the buffer
  Others      - Failed to write all the data


--*/
{
  USB_BOOT_WRITE10_CMD  WriteCmd;
  EFI_STATUS            Status;
  UINT16                Count;
  UINT32                BlockSize;
  UINT32                ByteSize;
  UINT32                Timeout;

  BlockSize = UsbMass->BlockIoMedia.BlockSize;
  Status    = EFI_SUCCESS;

  while (TotalBlock > 0) {
    //
    // Split the total blocks into smaller pieces to ease the pressure
    // on the device. We must split the total block because the WRITE10
    // command only has 16 bit transfer length (in the unit of block).
    //
    Count     = (UINT16)((TotalBlock < USB_BOOT_IO_BLOCKS) ? TotalBlock : USB_BOOT_IO_BLOCKS);
    ByteSize  = (UINT32)Count * BlockSize;

    //
    // Optical device need longer timeout than other device
    //
    if (UsbMass->OpticalStorage == TRUE) {
      Timeout = (UINT32)Count * USB_BOOT_OPTICAL_BLOCK_TIMEOUT;
    } else {
      Timeout = (UINT32)Count * USB_BOOT_GENERAL_BLOCK_TIMEOUT;
    }

    //
    // Fill in the write10 command block
    //
    EfiZeroMem (&WriteCmd, sizeof (USB_BOOT_WRITE10_CMD));

    WriteCmd.OpCode = USB_BOOT_WRITE10_OPCODE;
    WriteCmd.Lun    = USB_BOOT_LUN (UsbMass->Lun);
    UsbBootPutUint32 (WriteCmd.Lba, Lba);
    UsbBootPutUint16 (WriteCmd.TransferLen, Count);

    Status = UsbBootExecCmdWithRetry (
               UsbMass,
               &WriteCmd,
               sizeof (USB_BOOT_WRITE10_CMD),
               EfiUsbDataOut,
               Buffer,
               ByteSize,
               Timeout
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Lba        += Count;
    Buffer     += Count * BlockSize;
    TotalBlock -= Count;
  }

  return Status;
}

EFI_STATUS
UsbClearEndpointStall (
  IN EFI_USB_IO_PROTOCOL    *UsbIo,
  IN UINT8                  EndpointAddr
  )
/*++

Routine Description:

  Use the USB clear feature control transfer to clear the endpoint
  stall condition.

Arguments:

  UsbIo         - The USB IO protocol to use
  EndpointAddr  - The endpoint to clear stall for

Returns:

  EFI_SUCCESS - The endpoint stall condtion is clear
  Others      - Failed to clear the endpoint stall condtion

--*/
{
  EFI_USB_DEVICE_REQUEST    Request;
  EFI_STATUS                Status;
  UINT32                    CmdResult;
  UINT32                    Timeout;

  Request.RequestType = 0x02;
  Request.Request     = USB_DEV_CLEAR_FEATURE;
  Request.Value       = EfiUsbEndpointHalt;
  Request.Index       = EndpointAddr;
  Request.Length      = 0;
  Timeout             = USB_BOOT_GENERAL_CMD_TIMEOUT / USB_MASS_STALL_1_MS;

  Status = UsbIo->UsbControlTransfer (
                    UsbIo,
                    &Request,
                    EfiUsbNoData,
                    Timeout,
                    NULL,
                    0,
                    &CmdResult
                    );

  return Status;
}
