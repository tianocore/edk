/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  PxeBcMtftp.h
 
Abstract:

  Mtftp routines for PxeBc

--*/

#ifndef __EFI_PXEBC_MTFTP_H__
#define __EFI_PXEBC_MTFTP_H__

enum {
  PXE_MTFTP_OPTION_BLKSIZE_INDEX,
  PXE_MTFTP_OPTION_TIMEOUT_INDEX,
  PXE_MTFTP_OPTION_TSIZE_INDEX,
  PXE_MTFTP_OPTION_MULTICAST_INDEX,
  PXE_MTFTP_OPTION_MAXIMUM_INDEX
};

EFI_STATUS
PxeBcTftpGetFileSize (
  IN PXEBC_PRIVATE_DATA         *Private,
  IN EFI_MTFTP4_CONFIG_DATA     *Config,
  IN UINT8                      *Filename,
  IN UINTN                      *BlockSize,
  IN OUT UINT64                 *BufferSize
  )
/*++

Routine Description:

  This function is to get size of a file by Tftp.

Arguments:

  Private     - Pointer to PxeBc private data
  Config      - Pointer to Mtftp configuration data
  Filename    - Pointer to file name
  BlockSize   - Pointer to block size
  BufferSize  - Pointer to buffer size

Returns:

  EFI_SUCCESS
  EFI_NOT_FOUND
  EFI_DEVICE_ERROR
  
--*/
;

EFI_STATUS
PxeBcTftpReadFile (
  IN PXEBC_PRIVATE_DATA         *Private,
  IN EFI_MTFTP4_CONFIG_DATA     *Config,
  IN UINT8                      *Filename,
  IN UINTN                      *BlockSize,
  IN UINT8                      *BufferPtr,
  IN OUT UINT64                 *BufferSize,
  IN BOOLEAN                    DontUseBuffer
  )
/*++

Routine Description:

  This function is to get data of a file by Tftp.

Arguments:

  Private       - Pointer to PxeBc private data
  Config        - Pointer to Mtftp configuration data
  Filename      - Pointer to file name
  BlockSize     - Pointer to block size
  BufferPtr     - Pointer to buffer
  BufferSize    - Pointer to buffer size
  DontUseBuffer - Indicate whether with a receive buffer

Returns:

  EFI_SUCCESS
  EFI_DEVICE_ERROR
  
--*/
;

EFI_STATUS
PxeBcTftpWriteFile (
  IN PXEBC_PRIVATE_DATA         *Private,
  IN EFI_MTFTP4_CONFIG_DATA     *Config,
  IN UINT8                      *Filename,
  IN BOOLEAN                    Overwrite,
  IN UINTN                      *BlockSize,
  IN UINT8                      *BufferPtr,
  IN OUT UINT64                 *BufferSize
  )
/*++

Routine Description:

  This function is put data of a file by Tftp.

Arguments:
  
  Private     - Pointer to PxeBc private data
  Config      - Pointer to Mtftp configuration data
  Filename    - Pointer to file name
  Overwrite   - Indicate whether with overwrite attribute
  BlockSize   - Pointer to block size
  BufferPtr   - Pointer to buffer
  BufferSize  - Pointer to buffer size

Returns:

  EFI_SUCCESS
  EFI_DEVICE_ERROR

--*/
;

EFI_STATUS
PxeBcTftpReadDirectory (
  IN PXEBC_PRIVATE_DATA            *Private,
  IN EFI_MTFTP4_CONFIG_DATA        *Config,
  IN UINT8                         *Filename,
  IN UINTN                         *BlockSize,
  IN UINT8                         *BufferPtr,
  IN OUT UINT64                    *BufferSize,
  IN BOOLEAN                       DontUseBuffer
  )
/*++

Routine Description:

  This function is to get data of a directory by Tftp.

Arguments:

  Private       - Pointer to PxeBc private data
  Config        - Pointer to Mtftp configuration data
  Filename      - Pointer to file name
  BlockSize     - Pointer to block size
  BufferPtr     - Pointer to buffer
  BufferSize    - Pointer to buffer size
  DontUseBuffer - Indicate whether with a receive buffer

Returns:

  EFI_SUCCES
  EFI_DEVICE_ERROR

--*/
;

#endif

