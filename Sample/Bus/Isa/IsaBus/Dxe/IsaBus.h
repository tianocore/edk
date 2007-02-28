/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  IsaBus.h
  
Abstract:
  
  The header file for ISA bus driver
  
Revision History:

--*/

#ifndef _EFI_ISA_BUS_H
#define _EFI_ISA_BUS_H

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiCompNameSupport.h"
#include "ComponentName.h"

//
// Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)

//
// Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (IsaIo)

//
// Driver Consumed Protocols and GUIDs
//
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)

extern EFI_INTERFACE_DEFINITION_FOR_ISA_IO    IsaIoInterface;

typedef enum {
  IsaAccessTypeUnknown,
  IsaAccessTypeIo,
  IsaAccessTypeMem,
  IsaAccessTypeMaxType
} ISA_ACCESS_TYPE;

//
// 16 MB Memory Range
//
#define ISA_MAX_MEMORY_ADDRESS  0x1000000
//
// 64K I/O Range
//
#define ISA_MAX_IO_ADDRESS  0x10000

//
// ISA DMA Write Single Mask Register
//
#define DMA_SINGLE_MASK_0_3     0x0A
#define DMA_SINGLE_MASK_4_7     0xD4
#define DMA_CHANNEL_MASK_SELECT 0x04

//
// ISA DMA Clear Byte Pointer Register
//
#define DMA_CLEAR_0_3 0x0C
#define DMA_CLEAR_4_7 0xD8

//
// ISA DMA Channel Mode Register
//
#define DMA_MODE_0_3              0x0B
#define DMA_MODE_4_7              0xD6
#define DMA_MODE_READ             0x08
#define DMA_MODE_WRITE            0x04
#define DMA_MODE_AUTO_INITIALIZE  0x10
#define DMA_MODE_INCREMENT        0x00
#define DMA_MODE_DECREMENT        0x20
#define DMA_MODE_SINGLE           0x40
#define DMA_MODE_DEMAND           0x00

typedef struct {
  UINT8 Address;
  UINT8 Page;
  UINT8 Count;
} EFI_ISA_DMA_REGISTERS;

//
// ISA I/O Device Structure
//
#define ISA_IO_DEVICE_SIGNATURE EFI_SIGNATURE_32 ('i', 's', 'a', 'i')

typedef struct {
  UINT32                                    Signature;
  EFI_HANDLE                                Handle;
  EFI_INTERFACE_DEFINITION_FOR_ISA_IO       IsaIo;
  EFI_DEVICE_PATH_PROTOCOL                  *DevicePath;
  EFI_PCI_IO_PROTOCOL                       *PciIo;
} ISA_IO_DEVICE;

#define ISA_IO_DEVICE_FROM_ISA_IO_THIS(a) CR (a, ISA_IO_DEVICE, IsaIo, ISA_IO_DEVICE_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gIsaBusControllerDriver;

//
// Mapping structure for performing ISA DMA to a buffer above 16 MB
//
typedef struct {
  EFI_ISA_IO_PROTOCOL_OPERATION Operation;
  UINTN                         NumberOfBytes;
  UINTN                         NumberOfPages;
  EFI_PHYSICAL_ADDRESS          HostAddress;
  EFI_PHYSICAL_ADDRESS          MappedHostAddress;
} ISA_MAP_INFO;

//
// EFI Driver Binding Protocol Interface Functions
//

EFI_STATUS
EFIAPI
IsaBusControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     * RemainingDevicePath OPTIONAL
  )
/*++

  Routine Description:
  
    This function checks to see if a controller can be managed by the ISA Bus 
    Driver. This is done by checking to see if the controller supports the 
    EFI_PCI_IO_PROTOCOL protocol, and then looking at the PCI Configuration 
    Header to see if the device is a PCI to ISA bridge. The class code of 
    PCI to ISA bridge: Base class 06h, Sub class 01h Interface 00h 
  
  Arguments:
  
    This                 - The EFI_DRIVER_BINDING_PROTOCOL instance.
    Controller           - The handle of the device to check.
    RemainingDevicePath  - A pointer to the remaining portion of a device path.

  Returns:
  
    EFI_SUCCESS          - The device is supported by this driver.
    EFI_UNSUPPORTED      - The device is not supported by this driver.

--*/
;

EFI_STATUS
EFIAPI
IsaBusControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     * RemainingDevicePath OPTIONAL
  )
/*++

  Routine Description:
  
    This function tells the ISA Bus Driver to start managing a PCI to ISA 
    Bridge controller. 
  
  Arguments:
  
    This                  - The EFI_DRIVER_BINDING_PROTOCOL instance.
    Controller            - A handle to the device being started. 
    RemainingDevicePath   - A pointer to the remaining portion of a device path.

  Returns:
  
    EFI_SUCCESS           - The device was started.
    EFI_UNSUPPORTED       - The device is not supported.
    EFI_DEVICE_ERROR      - The device could not be started due to a device error.
    EFI_ALREADY_STARTED   - The device has already been started.
    EFI_INVALID_PARAMETER - One of the parameters has an invalid value.
    EFI_OUT_OF_RESOURCES  - The request could not be completed due to a lack of 
                            resources.
  
--*/
;

EFI_STATUS
EFIAPI
IsaBusControllerDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   * ChildHandleBuffer OPTIONAL
  )
/*++

  Routine Description:
  
    This function tells the ISA Bus Driver to stop managing a PCI to ISA 
    Bridge controller. 
     
  Arguments:
  
    This                   - The EFI_DRIVER_BINDING_PROTOCOL instance.
    Controller             - A handle to the device being stopped.
    NumberOfChindren       - The number of child device handles in ChildHandleBuffer.
    ChildHandleBuffer      - An array of child handles to be freed.

  
  Returns:
  
    EFI_SUCCESS            - The device was stopped.
    EFI_DEVICE_ERROR       - The device could not be stopped due to a device error.
    EFI_NOT_STARTED        - The device has not been started.
    EFI_INVALID_PARAMETER  - One of the parameters has an invalid value.
    EFI_OUT_OF_RESOURCES   - The request could not be completed due to a lack of 
                             resources.

--*/
;

//
// Function Prototypes
//

EFI_STATUS
IsaCreateDevice (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_PCI_IO_PROTOCOL          *PciIo,
  IN EFI_DEVICE_PATH_PROTOCOL     *ParentDevicePath,
  IN EFI_ISA_ACPI_RESOURCE_LIST   *IsaDeviceResourceList,
  OUT EFI_DEVICE_PATH_PROTOCOL    **ChildDevicePath
  )
/*++

  Routine Description:
  
    Create ISA device found by IsaPnpProtocol 

  Arguments:
  
    This                   - The EFI_DRIVER_BINDING_PROTOCOL instance.
    Controller             - The handle of ISA bus controller(PCI to ISA bridge)
    PciIo                  - The Pointer to the PCI protocol 
    ParentDevicePath       - Device path of the ISA bus controller
    IsaDeviceResourceList  - The resource list of the ISA device
    ChildDevicePath        - The pointer to the child device.

  Returns:
  
    EFI_SUCCESS            - Create the child device.
    EFI_OUT_OF_RESOURCES   - The request could not be completed due to a lack of 
                             resources.
    EFI_DEVICE_ERROR       - Can not create child device.
    
--*/
;

EFI_STATUS
InitializeIsaIoInstance (
  IN ISA_IO_DEVICE               *IsaIoDevice,
  IN EFI_ISA_ACPI_RESOURCE_LIST  *IsaDevice
  )
/*++

Routine Description:

  Initializes an ISA I/O Instance

Arguments:

  IsaIoDevice            - The iso device to be initialized.
  IsaDevice              - The resource list.
  
Returns:

  EFI_SUCCESS            - Initial success.
  
--*/
;

#endif
