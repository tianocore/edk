/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    EfiScriptLib.h

Abstract:

 
--*/

#ifndef _EFI_SCRIPT_LIB_H_
#define _EFI_SCRIPT_LIB_H_

#include "Tiano.h"
#include "EfiCommonLib.h"
#include "EfiBootScript.h"
#include EFI_PROTOCOL_DEFINITION (BootScriptSave)


EFI_STATUS
BootScriptSaveInitialize (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
BootScriptSaveIoWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  VOID                              *Buffer
  );

EFI_STATUS
BootScriptSaveIoReadWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  VOID                              *Data,
  IN  VOID                              *DataMask
  );

EFI_STATUS
BootScriptSaveMemWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  VOID                              *Buffer
  );

EFI_STATUS
BootScriptSaveMemReadWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  VOID                              *Data,
  IN  VOID                              *DataMask
  );

EFI_STATUS
BootScriptSavePciCfgWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  VOID                              *Buffer
  );

EFI_STATUS
BootScriptSavePciCfgReadWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  VOID                              *Data,
  IN  VOID                              *DataMask
  );

EFI_STATUS
BootScriptSaveSmbusExecute (
  IN  UINT16                            TableName,
  IN  EFI_SMBUS_DEVICE_ADDRESS          SlaveAddress,
  IN  EFI_SMBUS_DEVICE_COMMAND          Command,
  IN  EFI_SMBUS_OPERATION               Operation,
  IN  BOOLEAN                           PecCheck,
  IN  UINTN                             *Length,
  IN  VOID                              *Buffer
  );

EFI_STATUS
BootScriptSaveStall (
  IN  UINT16                            TableName,
  IN  UINTN                             Duration
  );

EFI_STATUS
BootScriptSaveDispatch (
  IN  UINT16                            TableName,
  IN  EFI_PHYSICAL_ADDRESS              EntryPoint
  );
  
#ifdef EFI_S3_RESUME
  
#define INITIALIZE_SCRIPT(ImageHandle, SystemTable) \
          BootScriptSaveInitialize(ImageHandle, SystemTable)

#define SCRIPT_IO_WRITE(TableName, Width, Address, Count, Buffer) \
          BootScriptSaveIoWrite(TableName, Width, Address, Count, Buffer)

#define SCRIPT_IO_READ_WRITE(TableName, Width, Address, Data, DataMask) \
          BootScriptSaveIoReadWrite(TableName, Width, Address, Data, DataMask)

#define SCRIPT_MEM_WRITE(TableName, Width, Address, Count, Buffer) \
          BootScriptSaveMemWrite(TableName, Width, Address, Count, Buffer)

#define SCRIPT_MEM_READ_WRITE(TableName, Width, Address, Data, DataMask) \
          BootScriptSaveMemReadWrite(TableName, Width, Address, Data, DataMask)

#define SCRIPT_PCI_CFG_WRITE(TableName, Width, Address, Count, Buffer) \
          BootScriptSavePciCfgWrite(TableName, Width, Address, Count, Buffer)

#define SCRIPT_PCI_CFG_READ_WRITE(TableName, Width, Address, Data, DataMask) \
          BootScriptSavePciCfgReadWrite(TableName, Width, Address, Data, DataMask)

#define SCRIPT_SMBUS_EXECUTE(TableName, SlaveAddress, Command, Operation, PecCheck, Length, Buffer) \
          BootScriptSaveSmbusExecute(TableName, SlaveAddress, Command, Operation, PecCheck, Length, Buffer)

#define SCRIPT_STALL(TableName, Duration) \
          BootScriptSaveStall(TableName, Duration)

#define SCRIPT_DISPATCH(TableName, EntryPoint) \
          BootScriptSaveDispatch(TableName, EntryPoint)
#else

#define INITIALIZE_SCRIPT(ImageHandle, SystemTable)          

#define SCRIPT_IO_WRITE(TableName, Width, Address, Count, Buffer) 

#define SCRIPT_IO_READ_WRITE(TableName, Width, Address, Data, DataMask) 
          
#define SCRIPT_MEM_WRITE(TableName, Width, Address, Count, Buffer) 

#define SCRIPT_MEM_READ_WRITE(TableName, Width, Address, Data, DataMask) 

#define SCRIPT_PCI_CFG_WRITE(TableName, Width, Address, Count, Buffer) 

#define SCRIPT_PCI_CFG_READ_WRITE(TableName, Width, Address, Data, DataMask) 

#define SCRIPT_SMBUS_EXECUTE(TableName, SlaveAddress, Command, Operation, PecCheck, Length, Buffer) 

#define SCRIPT_STALL(TableName, Duration)           

#define SCRIPT_DISPATCH(TableName, EntryPoint) 

#endif

#endif
