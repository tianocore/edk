/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    ScsiLib.h

 Abstract:

   Common Libarary for SCSI

 Revision History

--*/

#ifndef _SCSI_LIB_H
#define _SCSI_LIB_H

#include "scsibus.h"

#include EFI_PROTOCOL_DEFINITION (ScsiIo)

//
// the time unit is 100ns, since the SCSI I/O defines timeout in 100ns unit.
//
#define EFI_SCSI_STALL_1_MICROSECOND  10
#define EFI_SCSI_STALL_1_MILLISECOND  10000
#define EFI_SCSI_STALL_1_SECOND       10000000

//
// this macro cannot be directly used by the gBS->Stall(),
// since the value output by this macro is in 100ns unit,
// not 1us unit (1us = 1000ns)
//
#define EfiScsiStallSeconds(a)  (a) * EFI_SCSI_STALL_1_SECOND

EFI_STATUS
SubmitTestUnitReadyCommand (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  OUT VOID                  *SenseData,
  OUT UINT8                 *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus
  )
/*++

Routine Description:

  Function tests the ready status of SCSI unit.

Arguments:

  ScsiIo               - A pointer to SCSI IO protocol.
  Timeout              - The length of timeout period.
  SenseData            - A pointer to output sense data.
  SenseDataLength      - The length of output sense data.
  HostAdapterStatus    - The status of Host Adapter.
  TargetStatus         - The status of the target.

Returns:

  EFI_SUCCESS                - The status of the unit is tested successfully.
  EFI_BAD_BUFFER_SIZE        - The SCSI Request Packet was executed, 
                             but the entire DataBuffer could not be transferred.
                             The actual number of bytes transferred is returned
                             in TransferLength.
  EFI_NOT_READY              - The SCSI Request Packet could not be sent because 
                             there are too many SCSI Command Packets already 
                             queued.
  EFI_DEVICE_ERROR           - A device error occurred while attempting to send 
                             the SCSI Request Packet.
  EFI_INVALID_PARAMETER      - The contents of CommandPacket are invalid.  
  EFI_UNSUPPORTED            - The command described by the SCSI Request Packet
                             is not supported by the SCSI initiator(i.e., SCSI 
                             Host Controller).
  EFI_TIMEOUT                - A timeout occurred while waiting for the SCSI 
                             Request Packet to execute.

--*/
;

EFI_STATUS
SubmitInquiryCommand (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  IN  VOID                  *SenseData,
  IN OUT UINT8              *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus,
  IN OUT VOID               *InquiryDataBuffer,
  IN OUT UINT32             *InquiryDataLength,
  IN  BOOLEAN               EnableVitalProductData
  )
/*++

Routine Description:

  Function to submit SCSI inquiry command.

Arguments:

  ScsiIo               - A pointer to SCSI IO protocol.
  Timeout              - The length of timeout period.
  SenseData            - A pointer to output sense data.
  SenseDataLength      - The length of output sense data.
  HostAdapterStatus    - The status of Host Adapter.
  TargetStatus         - The status of the target.
  InquiryDataBuffer    - A pointer to inquiry data buffer.
  InquiryDataLength    - The length of inquiry data buffer.
  EnableVitalProductData - Boolean to enable Vital Product Data.

Returns:

  EFI_SUCCESS                - The status of the unit is tested successfully.
  EFI_BAD_BUFFER_SIZE        - The SCSI Request Packet was executed, 
                               but the entire DataBuffer could not be transferred.
                               The actual number of bytes transferred is returned
                               in TransferLength.
  EFI_NOT_READY              - The SCSI Request Packet could not be sent because 
                               there are too many SCSI Command Packets already 
                               queued.
  EFI_DEVICE_ERROR           - A device error occurred while attempting to send 
                               the SCSI Request Packet.
  EFI_INVALID_PARAMETER      - The contents of CommandPacket are invalid.  
  EFI_UNSUPPORTED            - The command described by the SCSI Request Packet
                               is not supported by the SCSI initiator(i.e., SCSI 
                               Host Controller).
  EFI_TIMEOUT                - A timeout occurred while waiting for the SCSI 
                               Request Packet to execute.

--*/
;

EFI_STATUS
SubmitModeSense10Command (
  IN  EFI_SCSI_IO_PROTOCOL    *ScsiIo,
  IN  UINT64                  Timeout,
  IN  VOID                    *SenseData,
  IN OUT UINT8                *SenseDataLength,
  OUT UINT8                   *HostAdapterStatus,
  OUT UINT8                   *TargetStatus,
  IN  VOID                    *DataBuffer,
  IN OUT UINT32               *DataLength,
  IN  UINT8                   DBDField, OPTIONAL
  IN  UINT8                   PageControl,
  IN  UINT8                   PageCode
  )
/*++

Routine Description:

  Function to submit SCSI mode sense 10 command.

Arguments:

  ScsiIo               - A pointer to SCSI IO protocol.
  Timeout              - The length of timeout period.
  SenseData            - A pointer to output sense data.
  SenseDataLength      - The length of output sense data.
  HostAdapterStatus    - The status of Host Adapter.
  TargetStatus         - The status of the target.
  DataBuffer           - A pointer to input data buffer.
  DataLength           - The length of input data buffer.
  DBDField             - The DBD Field (Optional).
  PageControl          - Page Control.
  PageCode             - Page code.

Returns:

  EFI_SUCCESS                - The status of the unit is tested successfully.
  EFI_BAD_BUFFER_SIZE        - The SCSI Request Packet was executed, 
                               but the entire DataBuffer could not be transferred.
                               The actual number of bytes transferred is returned
                               in TransferLength.
  EFI_NOT_READY              - The SCSI Request Packet could not be sent because 
                               there are too many SCSI Command Packets already 
                               queued.
  EFI_DEVICE_ERROR           - A device error occurred while attempting to send 
                               the SCSI Request Packet.
  EFI_INVALID_PARAMETER      - The contents of CommandPacket are invalid.  
  EFI_UNSUPPORTED            - The command described by the SCSI Request Packet
                               is not supported by the SCSI initiator(i.e., SCSI 
                               Host Controller).
  EFI_TIMEOUT                - A timeout occurred while waiting for the SCSI 
                               Request Packet to execute.

--*/
;

EFI_STATUS
SubmitRequestSenseCommand (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  IN  VOID                  *SenseData,
  IN OUT UINT8              *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus
  )
/*++

Routine Description:

  Function to submit SCSI request sense command.

Arguments:

  ScsiIo               - A pointer to SCSI IO protocol.
  Timeout              - The length of timeout period.
  SenseData            - A pointer to output sense data.
  SenseDataLength      - The length of output sense data.
  HostAdapterStatus    - The status of Host Adapter.
  TargetStatus         - The status of the target.

Returns:

  EFI_SUCCESS                - The status of the unit is tested successfully.
  EFI_BAD_BUFFER_SIZE        - The SCSI Request Packet was executed, 
                               but the entire DataBuffer could not be transferred.
                               The actual number of bytes transferred is returned
                               in TransferLength.
  EFI_NOT_READY              - The SCSI Request Packet could not be sent because 
                               there are too many SCSI Command Packets already 
                               queued.
  EFI_DEVICE_ERROR           - A device error occurred while attempting to send 
                               the SCSI Request Packet.
  EFI_INVALID_PARAMETER      - The contents of CommandPacket are invalid.  
  EFI_UNSUPPORTED            - The command described by the SCSI Request Packet
                               is not supported by the SCSI initiator(i.e., SCSI 
                               Host Controller).
  EFI_TIMEOUT                - A timeout occurred while waiting for the SCSI 
                               Request Packet to execute.

--*/
;

//
// Commands for direct access command
//
EFI_STATUS
SubmitReadCapacityCommand (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  IN  VOID                  *SenseData,
  IN OUT UINT8              *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus,
  OUT VOID                  *DataBuffer,
  IN OUT UINT32             *DataLength,
  IN  BOOLEAN               PMI
  )
/*++

Routine Description:

  Function to submit read capacity command.

Arguments:

  ScsiIo               - A pointer to SCSI IO protocol.
  Timeout              - The length of timeout period.
  SenseData            - A pointer to output sense data.
  SenseDataLength      - The length of output sense data.
  HostAdapterStatus    - The status of Host Adapter.
  TargetStatus         - The status of the target.
  DataBuffer           - A pointer to a data buffer.
  DataLength           - The length of data buffer.
  PMI                  - Partial medium indicator.

Returns:

  EFI_SUCCESS                - The status of the unit is tested successfully.
  EFI_BAD_BUFFER_SIZE        - The SCSI Request Packet was executed, 
                               but the entire DataBuffer could not be transferred.
                               The actual number of bytes transferred is returned
                               in TransferLength.
  EFI_NOT_READY              - The SCSI Request Packet could not be sent because 
                               there are too many SCSI Command Packets already 
                               queued.
  EFI_DEVICE_ERROR           - A device error occurred while attempting to send 
                               the SCSI Request Packet.
  EFI_INVALID_PARAMETER      - The contents of CommandPacket are invalid.  
  EFI_UNSUPPORTED            - The command described by the SCSI Request Packet
                               is not supported by the SCSI initiator(i.e., SCSI 
                               Host Controller).
  EFI_TIMEOUT                - A timeout occurred while waiting for the SCSI 
                               Request Packet to execute.

--*/
;

EFI_STATUS
SubmitRead10Command (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  IN  VOID                  *SenseData,
  IN OUT UINT8              *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus,
  OUT VOID                  *DataBuffer,
  IN OUT UINT32             *DataLength,
  IN  UINT32                StartLba,
  IN  UINT32                SectorSize
  )
/*++

Routine Description:

  Function to submit read 10 command.

Arguments:

  ScsiIo               - A pointer to SCSI IO protocol.
  Timeout              - The length of timeout period.
  SenseData            - A pointer to output sense data.
  SenseDataLength      - The length of output sense data.
  HostAdapterStatus    - The status of Host Adapter.
  TargetStatus         - The status of the target.
  DataBuffer           - A pointer to a data buffer.
  DataLength           - The length of data buffer.
  StartLba             - The start address of LBA.
  SectorSize           - The sector size.

Returns:

  EFI_SUCCESS                - The status of the unit is tested successfully.
  EFI_BAD_BUFFER_SIZE        - The SCSI Request Packet was executed, 
                               but the entire DataBuffer could not be transferred.
                               The actual number of bytes transferred is returned
                               in TransferLength.
  EFI_NOT_READY              - The SCSI Request Packet could not be sent because 
                               there are too many SCSI Command Packets already 
                               queued.
  EFI_DEVICE_ERROR           - A device error occurred while attempting to send 
                               the SCSI Request Packet.
  EFI_INVALID_PARAMETER      - The contents of CommandPacket are invalid.  
  EFI_UNSUPPORTED            - The command described by the SCSI Request Packet
                               is not supported by the SCSI initiator(i.e., SCSI 
                               Host Controller).
  EFI_TIMEOUT                - A timeout occurred while waiting for the SCSI 
                               Request Packet to execute.


--*/
;

EFI_STATUS
SubmitWrite10Command (
  IN  EFI_SCSI_IO_PROTOCOL  *ScsiIo,
  IN  UINT64                Timeout,
  IN  VOID                  *SenseData,
  IN OUT UINT8              *SenseDataLength,
  OUT UINT8                 *HostAdapterStatus,
  OUT UINT8                 *TargetStatus,
  OUT VOID                  *DataBuffer,
  IN OUT UINT32             *DataLength,
  IN  UINT32                StartLba,
  IN  UINT32                SectorSize
  )
/*++

Routine Description:

  Function to submit SCSI write 10 command.

Arguments:

  ScsiIo               - A pointer to SCSI IO protocol.
  Timeout              - The length of timeout period.
  SenseData            - A pointer to output sense data.
  SenseDataLength      - The length of output sense data.
  HostAdapterStatus    - The status of Host Adapter.
  TargetStatus         - The status of the target.
  DataBuffer           - A pointer to a data buffer.
  DataLength           - The length of data buffer.
  StartLba             - The start address of LBA.
  SectorSize           - The sector size.

Returns:

  EFI_SUCCESS                - The status of the unit is tested successfully.
  EFI_BAD_BUFFER_SIZE        - The SCSI Request Packet was executed, 
                               but the entire DataBuffer could not be transferred.
                               The actual number of bytes transferred is returned
                               in TransferLength.
  EFI_NOT_READY              - The SCSI Request Packet could not be sent because 
                               there are too many SCSI Command Packets already 
                               queued.
  EFI_DEVICE_ERROR           - A device error occurred while attempting to send 
                               the SCSI Request Packet.
  EFI_INVALID_PARAMETER      - The contents of CommandPacket are invalid.  
  EFI_UNSUPPORTED            - The command described by the SCSI Request Packet
                               is not supported by the SCSI initiator(i.e., SCSI 
                               Host Controller).
  EFI_TIMEOUT                - A timeout occurred while waiting for the SCSI 
                               Request Packet to execute.

--*/
;

#endif
