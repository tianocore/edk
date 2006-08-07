/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DevicePathDriver.c

Abstract:

  Device Path Driver to produce DevPathUtilities Protocol, DevPathFromText Protocol
  and DevPathToText Protocol.

--*/

#include "DevicePathDriver.h"

DEVICE_PATH_DRIVER_PRIVATE_DATA mPrivateData;

EFI_GUID mEfiDevicePathMessagingUartFlowControlGuid = DEVICE_PATH_MESSAGING_UART_FLOW_CONTROL;

EFI_GUID mEfiDevicePathMessagingSASGuid = DEVICE_PATH_MESSAGING_SAS;

EFI_DRIVER_ENTRY_POINT (DevicePathDriverEntryPoint)

EFI_STATUS
EFIAPI
DevicePathDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

  Routine Description:
    Entry point for EFI drivers.

  Arguments:
   ImageHandle - EFI_HANDLE
   SystemTable - EFI_SYSTEM_TABLE

  Returns:
    EFI_SUCCESS
    others

--*/
{
  EFI_STATUS  Status;

  EfiInitializeDriverLib (ImageHandle, SystemTable);

  mPrivateData.Signature = DEVICE_PATH_DRIVER_SIGNATURE;

  mPrivateData.DevicePathUtilities.GetDevicePathSize         = GetDevicePathSize;
  mPrivateData.DevicePathUtilities.DuplicateDevicePath       = DuplicateDevicePath;
  mPrivateData.DevicePathUtilities.AppendDevicePath          = AppendDevicePath;
  mPrivateData.DevicePathUtilities.AppendDeviceNode          = AppendDeviceNode;
  mPrivateData.DevicePathUtilities.AppendDevicePathInstance  = AppendDevicePathInstance;
  mPrivateData.DevicePathUtilities.GetNextDevicePathInstance = GetNextDevicePathInstance;
  mPrivateData.DevicePathUtilities.IsDevicePathMultiInstance = IsDevicePathMultiInstance;
  mPrivateData.DevicePathUtilities.CreateDeviceNode          = CreateDeviceNode;

  mPrivateData.DevicePathToText.ConvertDeviceNodeToText = ConvertDeviceNodeToText;
  mPrivateData.DevicePathToText.ConvertDevicePathToText = ConvertDevicePathToText;

  mPrivateData.DevicePathFromText.ConvertTextToDeviceNode = ConvertTextToDeviceNode;
  mPrivateData.DevicePathFromText.ConvertTextToDevicePath = ConvertTextToDevicePath;

  mPrivateData.Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mPrivateData.Handle,
                  &gEfiDevicePathUtilitiesProtocolGuid,
                  &mPrivateData.DevicePathUtilities,
                  &gEfiDevicePathToTextProtocolGuid,
                  &mPrivateData.DevicePathToText,
                  &gEfiDevicePathFromTextProtocolGuid,
                  &mPrivateData.DevicePathFromText,
                  NULL
                  );

  return Status;
}
