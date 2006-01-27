/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  BiosKeyboard.c
    
Abstract:

  ConsoleOut Routines that speak VGA.

Revision History

--*/

#include "BiosKeyboard.h"

//
// EFI Driver Binding Protocol Instance
//
//   This driver has a version value of 0x00000000.  This is the
//   lowest possible priority for a driver.  This is done on purpose to
//   prevent this driver from binding unless there are no native drivers
//   available.
//
EFI_DRIVER_BINDING_PROTOCOL gBiosKeyboardDriverBinding = {
  BiosKeyboardDriverBindingSupported,
  BiosKeyboardDriverBindingStart,
  BiosKeyboardDriverBindingStop,
  0x00000000,
  NULL,
  NULL
};

//
// Private worker functions
//
VOID
EFIAPI
BiosKeyboardWaitForKey (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  );

EFI_STATUS
BiosKeyboardCheckForKey (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This
  );

UINT16
ConvertToEFIScanCode (
  IN  CHAR16  KeyChar,
  IN  UINT16  ScanCode
  );

//
// Driver Entry Point
//
EFI_DRIVER_ENTRY_POINT (BiosKeyboardDriverEntryPoint)

EFI_STATUS
BiosKeyboardDriverEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++
  
  Routine Description:
    BiosKeyboard Driver Entry Point.
        
  Arguments:              
    
    ImageHandle   -  Handle for this drivers loaded image protocol.
    SystemTable   -  EFI system table.
  
  Returns:
    EFI_STATUS    -  Successfully installed the BiosKeyboard protocol.
    other         -  Errors occurred.
    
--*/
{
  return INSTALL_ALL_DRIVER_PROTOCOLS (
          ImageHandle,
          SystemTable,
          &gBiosKeyboardDriverBinding,
          ImageHandle,
          &gBiosKeyboardComponentName,
          NULL,
          NULL
          );
}

//
// EFI Driver Binding Protocol Functions
//
EFI_STATUS
BiosKeyboardDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Supported.
    
  Arguments:
    This                - Protocol instance pointer.
    Controller          - Handle of device to bind driver to.
    RemainingDevicePath - Not used.

  Returns:
    EFI_STATUS          -  Successfully processed the BiosKeyboard protocol.
    other               -  Errors occurred.
    
--*/
{
  EFI_STATUS                  Status;
  LEGACY_BIOS_THUNK_PROTOCOL  *LegacyBiosThunk;
  EFI_ISA_IO_PROTOCOL         *IsaIo;

  //
  // See if the Legacy BIOS Protocol is available
  //
  Status = gBS->LocateProtocol (
                  &gLegacyBiosThunkProtocolGuid,
                  NULL,
                  (VOID **)&LegacyBiosThunk
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiIsaIoProtocolGuid,
                  (VOID **) &IsaIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Use the ISA I/O Protocol to see if Controller is the Keyboard controller
  //
  if (IsaIo->ResourceList->Device.HID != EISA_PNP_ID (0x303) || IsaIo->ResourceList->Device.UID != 0) {
    Status = EFI_UNSUPPORTED;
  }

  gBS->CloseProtocol (
        Controller,
        &gEfiIsaIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

EFI_STATUS
BiosKeyboardDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Start protocol interfaces for the keyboard device handles.

  Arguments:
    This                - Protocol instance pointer.
    Controller          - Handle of device to bind driver to.
    RemainingDevicePath - Not used.

  Returns:
    EFI_SUCCESS         - This driver is added to DeviceHandle.
    other               - Errors occurred.

--*/
{
  EFI_STATUS                  Status;
  LEGACY_BIOS_THUNK_PROTOCOL  *LegacyBiosThunk;
  EFI_ISA_IO_PROTOCOL         *IsaIo;
  BIOS_KEYBOARD_DEV           *BiosKeyboardPrivate;
  EFI_IA32_REGISTER_SET       Regs;
  BOOLEAN                     CarryFlag;
  EFI_STATUS_CODE_VALUE       StatusCode;

  BiosKeyboardPrivate = NULL;
  StatusCode          = 0;

  //
  // See if the Legacy BIOS Protocol is available
  //
  Status = gBS->LocateProtocol (
                  &gLegacyBiosThunkProtocolGuid,
                  NULL,
                  (VOID **)&LegacyBiosThunk
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open the IO Abstraction(s) needed
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiIsaIoProtocolGuid,
                  (VOID **)&IsaIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Allocate the private device structure
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (BIOS_KEYBOARD_DEV),
                  &BiosKeyboardPrivate
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  EfiZeroMem (BiosKeyboardPrivate, sizeof (BIOS_KEYBOARD_DEV));

  //
  // Initialize the private device structure
  //
  BiosKeyboardPrivate->Signature                  = BIOS_KEYBOARD_DEV_SIGNATURE;
  BiosKeyboardPrivate->Handle                     = Controller;
  BiosKeyboardPrivate->LegacyBiosThunk            = LegacyBiosThunk;
  BiosKeyboardPrivate->IsaIo                      = IsaIo;

  BiosKeyboardPrivate->SimpleTextIn.Reset         = BiosKeyboardReset;
  BiosKeyboardPrivate->SimpleTextIn.ReadKeyStroke = BiosKeyboardReadKeyStroke;

  BiosKeyboardPrivate->DataRegisterAddress        = KEYBOARD_8042_DATA_REGISTER;
  BiosKeyboardPrivate->StatusRegisterAddress      = KEYBOARD_8042_STATUS_REGISTER;
  BiosKeyboardPrivate->CommandRegisterAddress     = KEYBOARD_8042_COMMAND_REGISTER;
  BiosKeyboardPrivate->ExtendedKeyboard           = TRUE;

  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  &BiosKeyboardPrivate->DevicePath
                  );

  //
  // Setup the WaitForKey event
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_WAIT,
                  EFI_TPL_NOTIFY,
                  BiosKeyboardWaitForKey,
                  &(BiosKeyboardPrivate->SimpleTextIn),
                  &((BiosKeyboardPrivate->SimpleTextIn).WaitForKey)
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Report a Progress Code for an attempt to detect the precense of the keyboard device in the system
  //
  ReportStatusCodeWithDevicePath (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_PRESENCE_DETECT,
    0,
    &gEfiCallerIdGuid,
    BiosKeyboardPrivate->DevicePath
    );

  //
  // Reset the keyboard device
  //
  Status = BiosKeyboardPrivate->SimpleTextIn.Reset (
                                              &BiosKeyboardPrivate->SimpleTextIn,
                                              FALSE
                                              );

  if (EFI_ERROR (Status)) {
    StatusCode = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_NOT_DETECTED;
    goto Done;
  }

  //
  // Get Configuration
  //
  Regs.H.AH = 0xc0;
  CarryFlag = BiosKeyboardPrivate->LegacyBiosThunk->Int86 (
                                                      BiosKeyboardPrivate->LegacyBiosThunk,
                                                      0x15,
                                                      &Regs
                                                      );

  if (!CarryFlag) {
    //
    // Check bit 6 of Feature Byte 2.
    // If it is set, then Int 16 Func 09 is supported
    //
    if (*(UINT8 *)(UINTN)((Regs.X.ES << 4) + Regs.X.BX + 0x06) & 0x40) {
      //
      // Get Keyboard Functionality
      //
      Regs.H.AH = 0x09;
      CarryFlag = BiosKeyboardPrivate->LegacyBiosThunk->Int86 (
                                                    BiosKeyboardPrivate->LegacyBiosThunk,
                                                    0x16,
                                                    &Regs
                                                    );

      if (!CarryFlag) {
        //
        // Check bit 5 of AH.
        // If it is set, then INT 16 Finc 10-12 are supported.
        //
        if (Regs.H.AL & 0x40) {
          //
          // Set the flag to use INT 16 Func 10-12
          //
          BiosKeyboardPrivate->ExtendedKeyboard = TRUE;
        }
      }
    }
  }

  //
  // Install protocol interfaces for the keyboard device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  &BiosKeyboardPrivate->SimpleTextIn,
                  NULL
                  );

Done:
  if (StatusCode != 0) {
    //
    // Report an Error Code for failing to start the keyboard device
    //
    ReportStatusCodeWithDevicePath (
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      StatusCode,
      0,
      &gEfiCallerIdGuid,
      BiosKeyboardPrivate->DevicePath
      );
  }

  if (EFI_ERROR (Status)) {

    gBS->CloseEvent ((BiosKeyboardPrivate->SimpleTextIn).WaitForKey);

    gBS->FreePool (BiosKeyboardPrivate);

    gBS->CloseProtocol (
          Controller,
          &gEfiIsaIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
  }

  return Status;
}

EFI_STATUS
BiosKeyboardDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++
  
  Routine Description:
    Stop protocol interfaces for the keyboard device handles.

  Arguments:
     This                - Protocol instance pointer.
     Controller          - Handle of device to test.
     NumberOfChildren    - Not used.
     ChildHandleBuffer   - Not used.

  Returns:
     EFI_SUCCESS         - This driver is added to DeviceHandle.
     other               - Errors occurred.

--*/
{
  EFI_STATUS                  Status;
  EFI_SIMPLE_TEXT_IN_PROTOCOL *SimpleTextIn;
  BIOS_KEYBOARD_DEV           *BiosKeyboardPrivate;

  //
  // Disable Keyboard
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  (VOID **) &SimpleTextIn,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  BiosKeyboardPrivate = BIOS_KEYBOARD_DEV_FROM_THIS (SimpleTextIn);

  //
  // Uninstall the Simple TextIn Protocol
  //
  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  &BiosKeyboardPrivate->SimpleTextIn
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Release the IsaIo protocol on the controller handle
  //
  gBS->CloseProtocol (
        Controller,
        &gEfiIsaIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  //
  // Free other resources
  //
  gBS->CloseEvent ((BiosKeyboardPrivate->SimpleTextIn).WaitForKey);
  gBS->FreePool (BiosKeyboardPrivate);

  return EFI_SUCCESS;
}

STATIC
UINT8
KeyReadDataRegister (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate
  )
/*++
  
  Routine Description:
     Read data byte from output buffer of Keyboard Controller without delay and waiting for buffer-empty state.

  Arguments:
     BiosKeyboardPrivate - Keyboard instance pointer.
     
  Returns:
     UINT8               - The data byte read from output buffer of Keyboard Controller from data port which often is port 60H.

--*/
{
  UINT8 Data;

  //
  // Use IsaIo protocol to perform IO operations
  //
  BiosKeyboardPrivate->IsaIo->Io.Read (
                                  BiosKeyboardPrivate->IsaIo,
                                  EfiIsaIoWidthUint8,
                                  BiosKeyboardPrivate->DataRegisterAddress,
                                  1,
                                  &Data
                                  );

  return Data;
}

STATIC
UINT8
KeyReadStatusRegister (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate
  )
/*++
  
  Routine Description:
     Read status byte from status register of Keyboard Controller without delay and waiting for buffer-empty state.

  Arguments:
     BiosKeyboardPrivate - Keyboard instance pointer.
     
  Returns:
     UINT8               - The status byte read from status register of Keyboard Controller from command port which often is port 64H.

--*/
{
  UINT8 Data;

  //
  // Use IsaIo protocol to perform IO operations
  //
  BiosKeyboardPrivate->IsaIo->Io.Read (
                                  BiosKeyboardPrivate->IsaIo,
                                  EfiIsaIoWidthUint8,
                                  BiosKeyboardPrivate->StatusRegisterAddress,
                                  1,
                                  &Data
                                  );

  return Data;
}

STATIC
VOID
KeyWriteCommandRegister (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Data
  )
/*++
  
  Routine Description:
     Write command byte to control register of Keyboard Controller without delay and waiting for buffer-empty state.

  Arguments:
     BiosKeyboardPrivate - Keyboard instance pointer.
     Data                - Data byte to write.
     
  Returns:
     None.

--*/
{
  //
  // Use IsaIo protocol to perform IO operations
  //
  BiosKeyboardPrivate->IsaIo->Io.Write (
                                  BiosKeyboardPrivate->IsaIo,
                                  EfiIsaIoWidthUint8,
                                  BiosKeyboardPrivate->CommandRegisterAddress,
                                  1,
                                  &Data
                                  );
}

STATIC
VOID
KeyWriteDataRegister (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Data
  )
/*++
  
  Routine Description:
     Write data byte to input buffer or input/output ports of Keyboard Controller without delay and waiting for buffer-empty state.

  Arguments:
     BiosKeyboardPrivate - Keyboard instance pointer.
     Data                - Data byte to write.
     
  Returns:
     None.

--*/
{
  //
  // Use IsaIo protocol to perform IO operations
  //
  BiosKeyboardPrivate->IsaIo->Io.Write (
                                  BiosKeyboardPrivate->IsaIo,
                                  EfiIsaIoWidthUint8,
                                  BiosKeyboardPrivate->DataRegisterAddress,
                                  1,
                                  &Data
                                  );
}

EFI_STATUS
KeyboardRead (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  OUT UINT8             *Data
  )
/*++
  
  Routine Description:
     Read data byte from output buffer of Keyboard Controller with delay and waiting for buffer-empty state.

  Arguments:
     BiosKeyboardPrivate - Keyboard instance pointer.
     Data                - The pointer for data that being read out.
     
  Returns:
     EFI_SUCCESS         - The data byte read out successfully.
     EFI_TIMEOUT         - Timeout occurred during reading out data byte.     

--*/
{
  UINT32  TimeOut;
  UINT32  RegFilled;

  TimeOut   = 0;
  RegFilled = 0;

  //
  // wait till output buffer full then perform the read
  //
  for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += 30) {
    if (KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_OUTB) {
      RegFilled = 1;
      *Data     = KeyReadDataRegister (BiosKeyboardPrivate);
      break;
    }

    gBS->Stall (30);
  }

  if (!RegFilled) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
KeyboardWrite (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Data
  )
/*++
  
  Routine Description:
     Write data byte to input buffer or input/output ports of Keyboard Controller with delay and waiting for buffer-empty state.

  Arguments:
     BiosKeyboardPrivate - Keyboard instance pointer.
     Data                - Data byte to write.
     
  Returns:
     EFI_SUCCESS         - The data byte is written successfully.
     EFI_TIMEOUT         - Timeout occurred during writing.     

--*/
{
  UINT32  TimeOut;
  UINT32  RegEmptied;

  TimeOut     = 0;
  RegEmptied  = 0;

  //
  // wait for input buffer empty
  //
  for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += 30) {
    if (!(KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_INPB)) {
      RegEmptied = 1;
      break;
    }

    gBS->Stall (30);
  }

  if (!RegEmptied) {
    return EFI_TIMEOUT;
  }
  //
  // Write it
  //
  KeyWriteDataRegister (BiosKeyboardPrivate, Data);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
KeyboardCommand (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Data
  )
/*++
  
  Routine Description:
     Write command byte to control register of Keyboard Controller with delay and waiting for buffer-empty state.

  Arguments:
     BiosKeyboardPrivate - Keyboard instance pointer.
     Data                - Command byte to write.
     
  Returns:
     EFI_SUCCESS         - The command byte is written successfully.
     EFI_TIMEOUT         - Timeout occurred during writing.     

--*/
{
  UINT32  TimeOut;
  UINT32  RegEmptied;

  TimeOut     = 0;
  RegEmptied  = 0;

  //
  // Wait For Input Buffer Empty
  //
  for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += 30) {
    if (!(KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_INPB)) {
      RegEmptied = 1;
      break;
    }

    gBS->Stall (30);
  }

  if (!RegEmptied) {
    return EFI_TIMEOUT;
  }
  //
  // issue the command
  //
  KeyWriteCommandRegister (BiosKeyboardPrivate, Data);

  //
  // Wait For Input Buffer Empty again
  //
  RegEmptied = 0;
  for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += 30) {
    if (!(KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_INPB)) {
      RegEmptied = 1;
      break;
    }

    gBS->Stall (30);
  }

  if (!RegEmptied) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
KeyboardWaitForValue (
  IN BIOS_KEYBOARD_DEV  *BiosKeyboardPrivate,
  IN UINT8              Value,
  IN UINTN              WaitForValueTimeOut
  )
/*++

Routine Description:

  wait for a specific value to be presented in
  Data register of Keyboard Controller by keyboard and then read it,
  used in keyboard commands ack

Arguments:

  BiosKeyboardPrivate - Keyboard instance pointer.
  Value               - The value to be waited for
  WaitForValueTimeOut - The limit of microseconds for timeout
  
Returns:

  EFI_SUCCESS         - The command byte is written successfully.
  EFI_TIMEOUT         - Timeout occurred during writing.     
  
--*/
{
  UINT32  i;
  UINT8   Data;
  UINT32  TimeOut;
  UINT32  SumTimeOut;
  UINT32  GotIt;

  GotIt       = 0;
  TimeOut     = 0;
  SumTimeOut  = 0;
  i           = 0;

  //
  // Make sure the initial value of 'Data' is different from 'Value'
  //
  Data = 0;
  if (Data == Value) {
    Data = 1;
  }
  //
  // Read from 8042 (multiple times if needed)
  // until the expected value appears
  // use SumTimeOut to control the iteration
  //
  while (1) {
    //
    // Perform a read
    //
    for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += 30) {
      if (KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_OUTB) {
        Data = KeyReadDataRegister (BiosKeyboardPrivate);
        break;
      }

      gBS->Stall (30);
    }

    SumTimeOut += TimeOut;

    if (Data == Value) {
      GotIt = 1;
      break;
    }

    if (SumTimeOut >= WaitForValueTimeOut) {
      break;
    }
  }
  //
  // Check results
  //
  if (GotIt) {
    return EFI_SUCCESS;
  } else {
    return EFI_TIMEOUT;
  }

}
//
// EFI Simple Text In Protocol Functions
//
EFI_STATUS
EFIAPI
BiosKeyboardReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  IN  BOOLEAN                      ExtendedVerification
  )
/*++

Routine Description:

  Reset the Keyboard and do BAT test for it, if (ExtendedVerification == TRUE) then do some extra keyboard validations.

Arguments:

  This                 - Pointer of simple text Protocol.
  ExtendedVerification - Whether perform the extra validation of keyboard. True: perform; FALSE: skip.
  
Returns:

 EFI_SUCCESS         - The command byte is written successfully.
 EFI_DEVICE_ERROR    - Errors occurred during reseting keyboard.       
  
--*/
{
  BIOS_KEYBOARD_DEV *BiosKeyboardPrivate;
  EFI_STATUS        Status;
  EFI_STATUS        ExitCode;
  EFI_TPL           OldTpl;
  UINT8             CommandByte;
  BOOLEAN           MouseEnable;

  MouseEnable         = FALSE;
  ExitCode            = 0;
  BiosKeyboardPrivate = BIOS_KEYBOARD_DEV_FROM_THIS (This);

  //
  // 1
  // Report reset progress code
  //
  ReportStatusCodeWithDevicePath (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_RESET,
    0,
    &gEfiCallerIdGuid,
    BiosKeyboardPrivate->DevicePath
    );

  //
  // Report a Progress Code for clearing the keyboard buffer
  //
  ReportStatusCodeWithDevicePath (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_CLEAR_BUFFER,
    0,
    &gEfiCallerIdGuid,
    BiosKeyboardPrivate->DevicePath
    );

  //
  // 2
  // Raise TPL to avoid mouse operation impact
  //
  OldTpl = gBS->RaiseTPL (EFI_TPL_NOTIFY);

  //
  // 3
  // check for KBC itself firstly for setted-up already or not by reading SYSF (bit2) of status register via 64H
  // if not skip step 4&5 and jump to step 6 to selftest KBC and report this
  // else   go step 4
  //
  if ((KeyReadStatusRegister (BiosKeyboardPrivate) & KBC_STSREG_VIA64_SYSF)) {
    //
    // 4
    // CheckMouseStatus to decide enable it later or not
    //
    //
    // Read the command byte of KBC
    //
    Status = KeyboardCommand (
              BiosKeyboardPrivate,
              KBC_CMDREG_VIA64_CMDBYTE_R
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0x401;
      goto Exit;
    }

    Status = KeyboardRead (
              BiosKeyboardPrivate,
              &CommandByte
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0x402;
      goto Exit;
    }
    //
    // Check mouse enabled or not before
    //
    if (CommandByte & KB_CMMBYTE_DISABLE_AUX) {
      MouseEnable = FALSE;
    } else {
      MouseEnable = TRUE;
    }
    //
    // 5
    // disable mouse (via KBC) and set command byte
    //
    Status = KeyboardCommand (
              BiosKeyboardPrivate,
              KBC_CMDREG_VIA64_AUX_DISABLE
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0x501;
      goto Exit;
    }

    Status = KeyboardCommand (
              BiosKeyboardPrivate,
              KBC_CMDREG_VIA64_CMDBYTE_W
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0x502;
      goto Exit;
    }

    Status = KeyboardWrite (
              BiosKeyboardPrivate,
              (
              CommandByte |
              KB_CMMBYTE_KSCAN2UNI_COV & (~KB_CMMBYTE_ENABLE_AUXINT) |
              KB_CMMBYTE_ENABLE_KBINT |
              KB_CMMBYTE_DISABLE_AUX & (~KB_CMMBYTE_DISABLE_KB)
              )
              );

  } else {
    //
    // 6
    // KBC Self Test
    //
    //
    // Report a Progress Code for performing a self test on the keyboard controller
    //
    ReportStatusCodeWithDevicePath (
      EFI_PROGRESS_CODE,
      EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_SELF_TEST,
      0,
      &gEfiCallerIdGuid,
      BiosKeyboardPrivate->DevicePath
      );

    Status = KeyboardCommand (
              BiosKeyboardPrivate,
              KBC_CMDREG_VIA64_KBC_SLFTEST
              );
    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0x601;
      goto Exit;
    }

    Status = KeyboardWaitForValue (
              BiosKeyboardPrivate,
              KBC_CMDECHO_KBCSLFTEST_OK,
              KEYBOARD_WAITFORVALUE_TIMEOUT
              );
    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0x602;
      goto Exit;
    }
  }
  //
  // 7
  // Exhaust output buffer data
  //
  do {
    Status = KeyboardRead (
              BiosKeyboardPrivate,
              &CommandByte
              );
  } while (!EFI_ERROR (Status));

  //
  // For reseting keyboard is not mandatory before booting OS and sometimes keyboard responses very slow,
  // so we only do the real reseting for keyboard when user asks, and normally during booting an OS, it's skipped.
  //
  if (ExtendedVerification) {
    //
    // 8
    // Send keyboard reset command then read ACK
    //
    Status = KeyboardWrite (
              BiosKeyboardPrivate,
              KBC_INPBUF_VIA60_KBRESET
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0x801;
      goto Exit;
    }

    Status = KeyboardWaitForValue (
              BiosKeyboardPrivate,
              KBC_CMDECHO_ACK,
              KEYBOARD_WAITFORVALUE_TIMEOUT
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0x802;
      goto Exit;
    }
    //
    // 9
    // Wait for keyboard return test OK.
    //
    Status = KeyboardWaitForValue (
              BiosKeyboardPrivate,
              KBC_CMDECHO_BATTEST_OK,
              KEYBOARD_WAITFORVALUE_TIMEOUT
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0x901;
      goto Exit;
    }
    //
    // 10
    // set keyboard scan code set = 02 (standard configuration)
    //
    Status = KeyboardWrite (
              BiosKeyboardPrivate,
              KBC_INPBUF_VIA60_KBSCODE
              );
    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0xA01;
      goto Exit;
    }

    Status = KeyboardWaitForValue (
              BiosKeyboardPrivate,
              KBC_CMDECHO_ACK,
              KEYBOARD_WAITFORVALUE_TIMEOUT
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0xA02;
      goto Exit;
    }

    Status = KeyboardWrite (
              BiosKeyboardPrivate,
              KBC_INPBUF_VIA60_SCODESET2
              );
    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0xA03;
      goto Exit;
    }

    Status = KeyboardWaitForValue (
              BiosKeyboardPrivate,
              KBC_CMDECHO_ACK,
              KEYBOARD_WAITFORVALUE_TIMEOUT
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0xA04;
      goto Exit;
    }
    //
    // 11
    // enable keyboard itself (not via KBC) by writing CMD F4 via 60H
    //
    Status = KeyboardWrite (
              BiosKeyboardPrivate,
              KBC_INPBUF_VIA60_KBEN
              );
    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0xB01;
      goto Exit;
    }

    Status = KeyboardWaitForValue (
              BiosKeyboardPrivate,
              KBC_CMDECHO_ACK,
              KEYBOARD_WAITFORVALUE_TIMEOUT
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0xB02;
      goto Exit;
    }
    //
    // 12
    // Additional validation, do it as follow:
    // 1). check for status register of PARE && TIM via 64H
    // 2). perform KB checking by writing ABh via 64H
    //
    if (KeyReadStatusRegister (BiosKeyboardPrivate) & (KBC_STSREG_VIA64_PARE | KBC_STSREG_VIA64_TIM)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0xC01;
      goto Exit;
    }

    Status = KeyboardCommand (
              BiosKeyboardPrivate,
              KBC_CMDREG_VIA64_KB_CKECK
              );
    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0xC02;
      goto Exit;
    }

    Status = KeyboardWaitForValue (
              BiosKeyboardPrivate,
              KBC_CMDECHO_KBCHECK_OK,
              KEYBOARD_WAITFORVALUE_TIMEOUT
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0xC03;
      goto Exit;
    }
  }
  //
  // 13
  // Done for validating keyboard. Enable keyboard (via KBC)
  // and recover the command byte to proper value
  //
  Status = KeyboardCommand (
            BiosKeyboardPrivate,
            KBC_CMDREG_VIA64_KB_ENABLE
            );

  if (EFI_ERROR (Status)) {
    Status    = EFI_DEVICE_ERROR;
    ExitCode  = 0xD01;
    goto Exit;
  }

  Status = KeyboardCommand (
            BiosKeyboardPrivate,
            KBC_CMDREG_VIA64_CMDBYTE_W
            );

  if (EFI_ERROR (Status)) {
    Status    = EFI_DEVICE_ERROR;
    ExitCode  = 0xD02;
    goto Exit;
  }

  Status = KeyboardWrite (
            BiosKeyboardPrivate,
            (
            CommandByte | KB_CMMBYTE_KSCAN2UNI_COV | KB_CMMBYTE_ENABLE_AUXINT | KB_CMMBYTE_ENABLE_KBINT |
            (~KB_CMMBYTE_DISABLE_AUX) & (~KB_CMMBYTE_DISABLE_KB)
            )
            );

  //
  // 14
  // conditionally enable mouse (via KBC)
  //
  if (MouseEnable) {
    Status = KeyboardCommand (
              BiosKeyboardPrivate,
              KBC_CMDREG_VIA64_AUX_ENABLE
              );

    if (EFI_ERROR (Status)) {
      Status    = EFI_DEVICE_ERROR;
      ExitCode  = 0xE01;

    }
  }

Exit:
  //
  // 15
  // resume priority of task level
  //
  gBS->RestoreTPL (OldTpl);

  return Status;

}

EFI_STATUS
EFIAPI
BiosKeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                *Key
  )
/*++

Routine Description:

  Read out the scan code of the key that has just been stroked.

Arguments:

  This                 - Pointer of simple text Protocol.
  Key                  - Pointer for store the key that read out.
  
Returns:

 EFI_SUCCESS         - The key is read out successfully.
 other               - The key reading failed.
  
--*/
{
  BIOS_KEYBOARD_DEV     *BiosKeyboardPrivate;
  UINT16                ScanCode;
  UINT16                KeyChar;
  EFI_IA32_REGISTER_SET Regs;
  EFI_STATUS            Status;

  BiosKeyboardPrivate = BIOS_KEYBOARD_DEV_FROM_THIS (This);

  //
  // If there's no key, just return
  //
  Status = BiosKeyboardCheckForKey (This);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Read the key
  //
  if (BiosKeyboardPrivate->ExtendedKeyboard) {
    Regs.H.AH = 0x10;
  } else {
    Regs.H.AH = 0x00;
  }

  BiosKeyboardPrivate->LegacyBiosThunk->Int86 (
                                    BiosKeyboardPrivate->LegacyBiosThunk,
                                    0x16,
                                    &Regs
                                    );

  ScanCode  = Regs.H.AH;
  KeyChar   = (CHAR16) Regs.H.AL;

  if (KeyChar == CHAR_NULL || KeyChar == CHAR_SCANCODE || KeyChar == CHAR_ESC) {
    Key->ScanCode     = ConvertToEFIScanCode (KeyChar, ScanCode);
    Key->UnicodeChar  = CHAR_NULL;
  } else {
    Key->ScanCode     = 0;
    Key->UnicodeChar  = KeyChar;
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
BiosKeyboardWaitForKey (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
/*++

Routine Description:

  Waiting on the keyboard event, if there's any key pressed by the user, signal the event

Arguments:

  Event                 - The event that be siganlled when any key has been stroked.
  Context               - Pointer of the protocol EFI_SIMPLE_TEXT_IN_PROTOCOL.
  
Returns:

  None.
  
--*/
{
  if (!EFI_ERROR (BiosKeyboardCheckForKey (Context))) {
    gBS->SignalEvent (Event);
  }
}

EFI_STATUS
EFIAPI
BiosKeyboardCheckForKey (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This
  )
/*++

Routine Description:

  Use legacy INT16 to get the key stroke status.

Arguments:

  This               - Pointer of the protocol EFI_SIMPLE_TEXT_IN_PROTOCOL.
  
Returns:

  EFI_SUCCESS         - A key is being pressed now.
  other               - No key is now pressed.
  
--*/
{
  BIOS_KEYBOARD_DEV     *BiosKeyboardPrivate;
  EFI_IA32_REGISTER_SET Regs;
  BOOLEAN               CarryFlag;

  BiosKeyboardPrivate = BIOS_KEYBOARD_DEV_FROM_THIS (This);

  if (BiosKeyboardPrivate->ExtendedKeyboard) {
    Regs.H.AH = 0x11;
  } else {
    Regs.H.AH = 0x01;
  }

  CarryFlag = BiosKeyboardPrivate->LegacyBiosThunk->Int86 (
                                                BiosKeyboardPrivate->LegacyBiosThunk,
                                                0x16,
                                                &Regs
                                                );

  return (Regs.X.Flags.ZF) ? EFI_NOT_READY : EFI_SUCCESS;
}
//
// Private worker functions
//
#define TABLE_END 0x0

struct {
  UINT16  ScanCode;
  UINT16  EfiScanCode;
}
mConvertTable[] = {
  {
    0x47,
    SCAN_HOME
  },
  {
    0x48,
    SCAN_UP
  },
  {
    0x49,
    SCAN_PAGE_UP
  },
  {
    0x4b,
    SCAN_LEFT
  },
  {
    0x4d,
    SCAN_RIGHT
  },
  {
    0x4f,
    SCAN_END
  },
  {
    0x50,
    SCAN_DOWN
  },
  {
    0x51,
    SCAN_PAGE_DOWN
  },
  {
    0x52,
    SCAN_INSERT
  },
  {
    0x53,
    SCAN_DELETE
  },
  //
  // Function Keys are only valid if KeyChar == 0x00
  //  This function does not require KeyChar to be 0x00
  //
  {
    0x3b,
    SCAN_F1
  },
  {
    0x3c,
    SCAN_F2
  },
  {
    0x3d,
    SCAN_F3
  },
  {
    0x3e,
    SCAN_F4
  },
  {
    0x3f,
    SCAN_F5
  },
  {
    0x40,
    SCAN_F6
  },
  {
    0x41,
    SCAN_F7
  },
  {
    0x42,
    SCAN_F8
  },
  {
    0x43,
    SCAN_F9
  },
  {
    0x44,
    SCAN_F10
  },
  {
    0x85,
    SCAN_F11
  },
  {
    0x86,
    SCAN_F12
  },

  {
    TABLE_END,
    SCAN_NULL
  },
};

UINT16
ConvertToEFIScanCode (
  IN  CHAR16  KeyChar,
  IN  UINT16  ScanCode
  )
/*++

Routine Description:

  Convert unicode combined with scan code of key to the counterpart of EFIScancode of it.

Arguments:

  KeyChar             - Unicode of key.
  ScanCode            - Scan code of key.
  
Returns:

  SCAN_NULL           - No corresponding value in the EFI convert table is found for the key.
  other               - The value of EFI Scancode for the key.
  
--*/
{
  UINT16  EfiScanCode;
  UINT16  Index;

  if (KeyChar == CHAR_ESC) {
    EfiScanCode = SCAN_ESC;
  } else if (KeyChar == 0x00 || KeyChar == 0xe0) {
    //
    // Movement & Function Keys
    //
    for (Index = 0; mConvertTable[Index].ScanCode != TABLE_END; Index += 1) {
      if (ScanCode == mConvertTable[Index].ScanCode) {
        return mConvertTable[Index].EfiScanCode;
      }
    }
    //
    // Reach Table end, return default value
    //
    return SCAN_NULL;
  } else {
    return SCAN_NULL;
  }

  return EfiScanCode;
}
