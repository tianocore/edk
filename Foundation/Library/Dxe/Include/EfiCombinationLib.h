/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
  EfiCombinationLib.h

Abstract:

  Library functions that can be called in both PEI and DXE phase

--*/

#ifndef _EFI_COMBINATION_LIB_H_
#define _EFI_COMBINATION_LIB_H_


EFI_STATUS
EfiInitializeCommonDriverLib (
  IN EFI_HANDLE   ImageHandle,
  IN VOID         *SystemTable
  );

EFI_STATUS
EfiCommonIoRead (
  IN  UINT8       Width,
  IN  UINTN       Address,
  IN  UINTN       Count,
  IN  OUT VOID    *Buffer
  );

EFI_STATUS
EfiCommonIoWrite (
  IN  UINT8       Width,
  IN  UINTN       Address,
  IN  UINTN       Count,
  IN  OUT VOID    *Buffer
  );

EFI_STATUS
EfiCommonPciRead (
  IN  UINT8       Width,
  IN  UINT64      Address,
  IN  UINTN       Count,
  IN  OUT VOID    *Buffer
  );

EFI_STATUS
EfiCommonPciWrite (
  IN  UINT8       Width,
  IN  UINT64      Address,
  IN  UINTN       Count,
  IN  OUT VOID    *Buffer
  );

EFI_STATUS
EfiCommonStall (
  IN  UINTN      Microseconds
  );

EFI_STATUS
EfiCommonCopyMem (
  IN VOID       *Destination,
  IN VOID       *Source,
  IN UINTN      Length
  );

EFI_STATUS
EfiCommonAllocatePages (
  IN EFI_ALLOCATE_TYPE          Type,
  IN EFI_MEMORY_TYPE            MemoryType,
  IN UINTN                      Pages,
  IN OUT EFI_PHYSICAL_ADDRESS   *Memory
  );

EFI_STATUS
EfiCommonLocateInterface (
  IN EFI_GUID                  *Guid,
  IN VOID                      **Interface
  );

EFI_STATUS
EfiCommonReportStatusCode (
  IN EFI_STATUS_CODE_TYPE     CodeType,
  IN EFI_STATUS_CODE_VALUE    Value, 
  IN UINT32                   Instance,
  IN EFI_GUID                 *CallerId,
  IN EFI_STATUS_CODE_DATA     *Data OPTIONAL
  );

#endif
