/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Dispatcher.c

Abstract:

  EFI PEI Core dispatch services

Revision History

--*/

#include "Tiano.h"
#include "PeiCore.h"
#include "PeiLib.h"
#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)

EFI_STATUS
TransferOldDataToNewDataRange (
  IN PEI_CORE_INSTANCE        *PrivateData,
  OUT UINTN                   *PrivateDataInMem
  );

EFI_GUID gEfiPeiCorePrivateGuid = EFI_PEI_CORE_PRIVATE_GUID;


EFI_STATUS 
PeiDispatcher (
  IN EFI_PEI_STARTUP_DESCRIPTOR  *PeiStartupDescriptor,
  IN PEI_CORE_INSTANCE           *PrivateData,
  IN PEI_CORE_DISPATCH_DATA      *DispatchData
  )

/*++

Routine Description:

  Conduct PEIM dispatch.

Arguments:

  PeiStartupDescriptor - Pointer to IN EFI_PEI_STARTUP_DESCRIPTOR
  PrivateData          - Pointer to the private data passed in from caller
  DispatchData         - Pointer to PEI_CORE_DISPATCH_DATA data.

Returns:

  EFI_SUCCESS   - Successfully dispatched PEIM.
  EFI_NOT_FOUND - The dispatch failed.
            
--*/
{
  EFI_STATUS                        Status;
  PEI_CORE_TEMP_POINTERS            TempPtr;
  UINTN                             PrivateDataInMem;
  BOOLEAN                           NextFvFound;
  EFI_FIRMWARE_VOLUME_HEADER        *NextFvAddress;
  EFI_FIRMWARE_VOLUME_HEADER        *DefaultFvAddress;

  //
  // Debug data for uninstalled Peim list
  //
  PEI_DEBUG_CODE (

    UINT32        DebugNotDispatchedBitmap;
    EFI_GUID      DebugFoundPeimList[32];
    UINT8         DebugFoundPeimPoint;

  )

  PEI_REPORT_STATUS_CODE_CODE (
    
    EFI_DEVICE_HANDLE_EXTENDED_DATA   ExtendedData;
    
    ExtendedData.DataHeader.HeaderSize = (UINT16)sizeof (EFI_STATUS_CODE_DATA);
    ExtendedData.DataHeader.Size = (UINT16)(sizeof (EFI_DEVICE_HANDLE_EXTENDED_DATA) - ExtendedData.DataHeader.HeaderSize);
  
    PeiCoreCopyMem (
      &ExtendedData.DataHeader.Type, 
      &gEfiStatusCodeSpecificDataGuid, 
      sizeof (EFI_GUID)
      ); 
  )
  //
  // save the Current FV Address so that we will not process it again if FindFv returns it later
  //
  DefaultFvAddress = DispatchData->BootFvAddress;
 
  //
  // This is the main dispatch loop.  It will search known FVs for PEIMs and
  // attempt to dispatch them.  If any PEIM gets dispatched through a single
  // pass of the dispatcher, it will start over from the Bfv again to see
  // if any new PEIMs dependencies got satisfied.  With a well ordered
  // FV where PEIMs are found in the order their dependencies are also
  // satisfied, this dipatcher should run only once.
  //  
  for (;;) {
    //
    // This is the PEIM search loop. It will scan through all PEIMs it can find
    // looking for PEIMs to dispatch, and will dipatch them if they have not
    // already been dispatched and all of their dependencies are met.
    // If no more PEIMs can be found in this pass through all known FVs,
    // then it will break out of this loop.
    //
    for (;;) {

      Status = FindNextPeim (
                 &PrivateData->PS,
                 DispatchData->CurrentFvAddress,
                 &DispatchData->CurrentPeimAddress
                 );

      //
      // If we found a PEIM, check if it is dispatched.  If so, go to the
      // next PEIM.  If not, dispatch it if its dependencies are satisfied.
      // If its dependencies are not satisfied, go to the next PEIM.
      //
      if (Status == EFI_SUCCESS) {

        PEI_DEBUG_CODE (

          //
          // Fill list of found Peims for later list of those not installed
          //
          PeiCoreCopyMem (
            &DebugFoundPeimList[DispatchData->CurrentPeim],
            &DispatchData->CurrentPeimAddress->Name,
            sizeof (EFI_GUID)
            );

        )

        if (!Dispatched (
               DispatchData->CurrentPeim,
               DispatchData->DispatchedPeimBitMap
               )) {
          if (DepexSatisfied (&PrivateData->PS, DispatchData->CurrentPeimAddress)) {
            Status = PeiLoadImage (
                       &PrivateData->PS,
                       DispatchData->CurrentPeimAddress,
                       &TempPtr.Raw
                       );
            if (Status == EFI_SUCCESS) {

              //
              // The PEIM has its dependencies satisfied, and its entry point
              // has been found, so invoke it. 
              //
              PEI_PERF_START (
                &PrivateData->PS,
                L"PEIM",
                (EFI_FFS_FILE_HEADER *)(DispatchData->CurrentPeimAddress),
                0
                );

              PEI_REPORT_STATUS_CODE_CODE (
               
                ExtendedData.Handle = (EFI_HANDLE)DispatchData->CurrentPeimAddress;
              
              )                
                                                        
              PEI_REPORT_STATUS_CODE_CODE (
                
                PeiReportStatusCode (
                  &(PrivateData->PS),
                  EFI_PROGRESS_CODE,
                  EFI_SOFTWARE_PEI_CORE | EFI_SW_PC_INIT_BEGIN,
                  0,
                  NULL,
                  (EFI_STATUS_CODE_DATA *)(&ExtendedData)
                  ); 

              )

              //
              // Is this a authentic image
              //
              Status = VerifyPeim (
                        &PrivateData->PS,
                        DispatchData->CurrentPeimAddress
                        );

              if (Status != EFI_SECURITY_VIOLATION) {

                Status =  TempPtr.PeimEntry (
                                    DispatchData->CurrentPeimAddress,
                                    &PrivateData->PS
                                    );
              }

              PEI_REPORT_STATUS_CODE_CODE (
                
                PeiReportStatusCode (
                  &(PrivateData->PS),
                  EFI_PROGRESS_CODE,
                  EFI_SOFTWARE_PEI_CORE | EFI_SW_PC_INIT_END,
                  0,
                  NULL,
                  (EFI_STATUS_CODE_DATA *)(&ExtendedData)
                  );

              )
              PEI_PERF_END (&PrivateData->PS, L"PEIM", (EFI_FFS_FILE_HEADER *)(DispatchData->CurrentPeimAddress), 0);

              //
              // Mark the PEIM as dispatched so we don't attempt to run it again
              //
              SetDispatched (
                &PrivateData->PS,
                DispatchData->CurrentPeim,
                &DispatchData->DispatchedPeimBitMap
                );                 
             
              //
              // Process the Notify list and dispatch any notifies for
              // newly installed PPIs.
              //
              ProcessNotifyList (&PrivateData->PS);

              //
              // If real system memory was discovered and installed by this
              // PEIM, switch the stacks to the new memory.  Since we are
              // at dispatch level, only the Core's private data is preserved,
              // nobody else should have any data on the stack.
              //
              if (PrivateData->SwitchStackSignal) {
                TempPtr.PeiCore = PeiCore;
                Status = TransferOldDataToNewDataRange (
                           PrivateData,
                           &PrivateDataInMem
                           );
                if (EFI_ERROR (Status)) {
                  return Status;
                }
                //
                //Subtract 0x10 from the 4th parameter indicating the new stack base,
                //in order to provide buffer protection against possible illegal stack
                //access that might corrupt the stack.
                //
                SwitchCoreStacks (
                  TempPtr.Raw,
                  (UINTN)PeiStartupDescriptor,
                  PrivateDataInMem,
                  (VOID *)((UINTN)PrivateData->StackBase + (UINTN)PrivateData->StackSize - 0x10)
                );
              }
            }
          }
        }
        DispatchData->CurrentPeim++;
        continue;

      } else {

        //
        // If we could not find another PEIM in the current FV, go try
        // the FindFv PPI to look in other FVs for more PEIMs.  If we can
        // not locate the FindFv PPI, or if the FindFv PPI can not find
        // anymore FVs, then exit the PEIM search loop.
        //
        if (DispatchData->FindFv == NULL) {
          Status = PeiLocatePpi (
                     &PrivateData->PS,
                     &gEfiFindFvPpiGuid,
                     0,
                     NULL,
                     &DispatchData->FindFv
                     );
          if (Status != EFI_SUCCESS) {
            break;
          }
        }
        NextFvFound = FALSE;
        while (!NextFvFound) {     
          Status = DispatchData->FindFv->FindFv ( 
                                           DispatchData->FindFv,
                                           &PrivateData->PS,
                                           &DispatchData->CurrentFv,
                                           &NextFvAddress
                                           );
          //
          // if there is no next fv, get out of this loop of finding FVs
          //
          if (Status != EFI_SUCCESS) {
            break;
          }
          //
          // don't process the default Fv again. (we don't know the order in which the hobs were created)
          //
          if ((NextFvAddress != DefaultFvAddress) && 
              (NextFvAddress != DispatchData->CurrentFvAddress)) {
            
            //
            // VerifyFv() is currently returns SUCCESS all the time, add code to it to 
            // actually verify the given FV
            //
            Status = VerifyFv (NextFvAddress);
            if (Status == EFI_SUCCESS) {
              NextFvFound = TRUE;
              DispatchData->CurrentFvAddress = NextFvAddress;
              DispatchData->CurrentPeimAddress = NULL;
              // 
              // current PRIM number (CurrentPeim) must continue as is, don't reset it here
              //
            }
          }         
        }
        //
        // if there is no next fv, get out of this loop of dispatching PEIMs
        //
        if (!NextFvFound) {
          break;
        }
        //
        // continue in the inner for(;;) loop with a new FV;
        //
      }
    }

    //
    // If all the PEIMs that we have found have been dispatched, then
    // there is nothing left to dispatch and we don't need to go search
    // through all PEIMs again.
    //
    if ((~(DispatchData->DispatchedPeimBitMap) & 
         ((1 << DispatchData->CurrentPeim)-1)) == 0) {
      break;
    }
      
    // 
    // Check if no more PEIMs that depex was satisfied
    //
    if (DispatchData->DispatchedPeimBitMap == DispatchData->PreviousPeimBitMap) {
      break;
    }

    //
    // Case when Depex is not satisfied and has to traverse the list again
    //
    DispatchData->CurrentPeim = 0;
    DispatchData->CurrentPeimAddress = 0;
    DispatchData->PreviousPeimBitMap = DispatchData->DispatchedPeimBitMap;

    //
    // don't go back to the loop without making sure that the CurrentFvAddress is the 
    // same as the 1st (or default) FV we started with. otherwise we will interpret the bimap wrongly and
    // mess it up, always start processing the PEIMs from the default FV just like in the first time around.
    //
    DispatchData->CurrentFv = 0;
    DispatchData->CurrentFvAddress = DefaultFvAddress;
  }

  PEI_DEBUG_CODE (

    DebugFoundPeimPoint = 0;
    //
    // Get bitmap of Peims that were not dispatched,
    // 

    DebugNotDispatchedBitmap = ((DispatchData->DispatchedPeimBitMap) ^ ((1 << DispatchData->CurrentPeim)-1));
    //
    // Scan bitmap of Peims not installed and print GUIDS
    //
    while (DebugNotDispatchedBitmap != 0) {
      if ((DebugNotDispatchedBitmap & 1) != 0) {
        PEI_DEBUG (
          (&PrivateData->PS, EFI_D_INFO, 
           "WARNING -> InstallPpi: Not Installed: %g\n", 
           &DebugFoundPeimList[DebugFoundPeimPoint])
          );
      }
      DebugFoundPeimPoint++;
      DebugNotDispatchedBitmap >>= 1;
    }

  )

   return EFI_NOT_FOUND;
}

VOID
InitializeDispatcherData (
  IN EFI_PEI_SERVICES             **PeiServices,
  IN PEI_CORE_INSTANCE            *OldCoreData,
  IN EFI_PEI_STARTUP_DESCRIPTOR   *PeiStartupDescriptor
  )
/*++

Routine Description:

  Initialize the Dispatcher's data members

Arguments:

  PeiServices          - The PEI core services table.
  OldCoreData          - Pointer to old core data (before switching stack).
                         NULL if being run in non-permament memory mode.
  PeiStartupDescriptor - Information and services provided by SEC phase.

Returns:

  None.

--*/
{
  PEI_CORE_INSTANCE *PrivateData;

  PrivateData = PEI_CORE_INSTANCE_FROM_PS_THIS (PeiServices);

  if (OldCoreData == NULL) {
    PrivateData->DispatchData.CurrentFvAddress = (EFI_FIRMWARE_VOLUME_HEADER *) PeiStartupDescriptor->BootFirmwareVolume;
    PrivateData->DispatchData.BootFvAddress = (EFI_FIRMWARE_VOLUME_HEADER *) PeiStartupDescriptor->BootFirmwareVolume;    
  } else {
    
    //
    // Current peim has been dispatched, but not count
    //
    PrivateData->DispatchData.CurrentPeim = (UINT8)(OldCoreData->DispatchData.CurrentPeim + 1);
  }

  return;
}


BOOLEAN
Dispatched (
  IN UINT8  CurrentPeim,
  IN UINT32 DispatchedPeimBitMap
  )
/*++

Routine Description:

  This routine checks to see if a particular PEIM has been dispatched during
  the PEI core dispatch.

Arguments:
  CurrentPeim          - The PEIM/FV in the bit array to check.
  DispatchedPeimBitMap - Bit array, each bit corresponds to a PEIM/FV.

Returns:
  TRUE  - PEIM already dispatched
  FALSE - Otherwise

--*/
{
  return (BOOLEAN)((DispatchedPeimBitMap & (1 << CurrentPeim)) != 0);
}

VOID 
SetDispatched ( 
  IN EFI_PEI_SERVICES   **PeiServices,
  IN UINT8              CurrentPeim,
  OUT UINT32            *DispatchedPeimBitMap
  ) 
/*++

Routine Description:

  This routine sets a PEIM as having been dispatched once its entry
  point has been invoked.

Arguments:

  PeiServices          - The PEI core services table.
  CurrentPeim          - The PEIM/FV in the bit array to check.
  DispatchedPeimBitMap - Bit array, each bit corresponds to a PEIM/FV.

Returns:
  None

--*/
{
  //
  // Check if the total number of PEIMs exceed the bitmap.
  // CurrentPeim is 0-based
  //
PEI_DEBUG_CODE (
  if (CurrentPeim > (sizeof (*DispatchedPeimBitMap) * 8 - 1)) {
    ASSERT_PEI_ERROR (PeiServices, EFI_OUT_OF_RESOURCES);
  }
)
  *DispatchedPeimBitMap |= (1 << CurrentPeim);
  return;
}

BOOLEAN
DepexSatisfied (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN VOID              *CurrentPeimAddress
  )
/*++

Routine Description:

  This routine parses the Dependency Expression, if available, and
  decides if the module can be executed.

Arguments:
  PeiServices - The PEI Service Table
  CurrentPeimAddress - Address of the PEIM Firmware File under investigation

Returns:
  TRUE  - Can be dispatched
  FALSE - Cannot be dispatched

--*/
{
  EFI_STATUS  Status;
  INT8        *DepexData;
  BOOLEAN     Runnable;

  Status = PeiFfsFindSectionData (
            PeiServices,
            EFI_SECTION_PEI_DEPEX,
            CurrentPeimAddress,
            &DepexData 
            );
  //
  // If there is no DEPEX, assume the module can be executed
  //
  if (EFI_ERROR (Status)) {
    return TRUE;
  }

  //
  // Evaluate a given DEPEX
  //
  Status = PeimDispatchReadiness (
            PeiServices, 
            DepexData, 
            &Runnable  
            );

  return Runnable;
}


EFI_STATUS
TransferOldDataToNewDataRange (
  IN PEI_CORE_INSTANCE        *PrivateData,
  OUT UINTN                   *PrivateDataInMem
  )
/*++

Routine Description:

  This routine transfers the contents of the pre-permanent memory
  PEI Core private data to a post-permanent memory data location.

Arguments:
  PrivateData       - Pointer to the current PEI Core private data pre-permanent memory
  PrivateDataInMem  - Pointer to the PrivateData once the private data has been transferred
                      to permanent memory

Returns:
  EFI_SUCCESS   - Successfully transfered
  EFI_ERROR     - Fail to transfer

--*/
{
    EFI_STATUS  Status;

    Status = PeiBuildHobGuid (
              &(PrivateData->PS),
              &gEfiPeiCorePrivateGuid,
              sizeof (PEI_CORE_INSTANCE),
              (VOID*)PrivateDataInMem
              );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    (*PrivateDataInMem) += sizeof (EFI_HOB_GUID_TYPE);
    PrivateData->PS->CopyMem (
                      (VOID*)(*PrivateDataInMem),
                      (VOID*)PrivateData,
                      sizeof (PEI_CORE_INSTANCE)
                      );

    return EFI_SUCCESS;
}

