/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  CpuIo.h

Abstract:
  *.h file for the driver

  Note: the EFIAPI on the CpuIo functions is used to glue MASM (assembler) code
  into C code. By making the MASM functions EFIAPI it ensures that a standard
  C calling convention is assumed by the compiler, reguardless of the compiler
  flags.


--*/

#include "Tiano.h"
#include "EfiRuntimeLib.h"

#include EFI_PROTOCOL_DEFINITION (CpuIO)


//
// Volitile
//
typedef union {
  UINT8 VOLATILE  *buf;
  UINT8 VOLATILE  *ui8;
  UINT16 VOLATILE *ui16;
  UINT32 VOLATILE *ui32;
  UINT64 VOLATILE *ui64;
  UINTN VOLATILE  ui;
} PTR;

EFI_STATUS
EFIAPI
CpuIoInitialize (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ImageHandle - TODO: add argument description
  SystemTable - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
CpuIoMemRW (
  IN EFI_CPU_IO_PROTOCOL_WIDTH     Width,
  IN UINTN                         Count,
  IN BOOLEAN                       InStrideFlag,
  IN PTR                           In,
  IN BOOLEAN                       OutStrideFlag,
  OUT PTR                          Out
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Width         - TODO: add argument description
  Count         - TODO: add argument description
  InStrideFlag  - TODO: add argument description
  In            - TODO: add argument description
  OutStrideFlag - TODO: add argument description
  Out           - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
CpuMemoryServiceRead (
  IN  EFI_CPU_IO_PROTOCOL               *This,
  IN  EFI_CPU_IO_PROTOCOL_WIDTH         Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  OUT VOID                          *Buffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This    - TODO: add argument description
  Width   - TODO: add argument description
  Address - TODO: add argument description
  Count   - TODO: add argument description
  Buffer  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
CpuMemoryServiceWrite (
  IN EFI_CPU_IO_PROTOCOL                *This,
  IN  EFI_CPU_IO_PROTOCOL_WIDTH         Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  OUT VOID                          *Buffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This    - TODO: add argument description
  Width   - TODO: add argument description
  Address - TODO: add argument description
  Count   - TODO: add argument description
  Buffer  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
CpuIoServiceRead (
  IN EFI_CPU_IO_PROTOCOL                *This,
  IN  EFI_CPU_IO_PROTOCOL_WIDTH         Width,
  IN  UINT64                            UserAddress,
  IN  UINTN                             Count,
  IN  OUT VOID                          *UserBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  Width       - TODO: add argument description
  UserAddress - TODO: add argument description
  Count       - TODO: add argument description
  UserBuffer  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
CpuIoServiceWrite (
  IN EFI_CPU_IO_PROTOCOL                *This,
  IN  EFI_CPU_IO_PROTOCOL_WIDTH         Width,
  IN  UINT64                            UserAddress,
  IN  UINTN                             Count,
  IN  OUT VOID                          *UserBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  Width       - TODO: add argument description
  UserAddress - TODO: add argument description
  Count       - TODO: add argument description
  UserBuffer  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
CpuIoCheckAddressRange (
  IN  EFI_CPU_IO_PROTOCOL_WIDTH         Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  VOID                              *Buffer,
  IN  UINT64                            Limit
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Width   - TODO: add argument description
  Address - TODO: add argument description
  Count   - TODO: add argument description
  Buffer  - TODO: add argument description
  Limit   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;


UINT8
EFIAPI
CpuIoRead8 (
  IN  UINT16  Port
  )
/*++                                                                                                                               
Routine Description:                                                
  Cpu I/O read port
Arguments:                
   Port: - Port number to read                                                          
Returns:                                                            
   Return read 8 bit value                                                
--*/
;


UINT16
EFIAPI
CpuIoRead16 (
  IN  UINT16  Port
  )
/*++                                                                                                                               
Routine Description:                                                
  Cpu I/O read port
Arguments:                
   Port: - Port number to read                                                          
Returns:                                                            
   Return read 16 bit value                                                
--*/
;


UINT32
EFIAPI
CpuIoRead32 (
  IN  UINT16  Port
  )
/*++                                                                                                                               
Routine Description:                                                
  Cpu I/O read port
Arguments:                
   Port: - Port number to read                                                          
Returns:                                                            
   Return read 32 bit value                                                
--*/
;


VOID
EFIAPI
CpuIoWrite8 (
  IN  UINT16  Port,
  IN  UINT32  Data
  )
/*++                                                                                                                               
Routine Description:                                                
  Cpu I/O write 8 bit data to port
Arguments:                
   Port: - Port number to read  
   Data: - Data to write to the Port                                                        
Returns:                                                            
   None                                                
--*/
;

VOID
EFIAPI
CpuIoWrite16 (
  IN  UINT16  Port,
  IN  UINT32  Data
  )
/*++                                                                                                                               
Routine Description:                                                
  Cpu I/O write 16 bit data to port
Arguments:                
   Port: - Port number to read  
   Data: - Data to write to the Port                                                        
Returns:                                                            
   None                                                
--*/
;

VOID
EFIAPI
CpuIoWrite32 (
  IN  UINT16  Port,
  IN  UINT32  Data
  )
/*++                                                                                                                               
Routine Description:                                                
  Cpu I/O write 32 bit data to port
Arguments:                
   Port: - Port number to read  
   Data: - Data to write to the Port                                                        
Returns:                                                            
   None                                                
--*/
;
