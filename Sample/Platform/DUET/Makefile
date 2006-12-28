#/*++
#
#  Copyright (c)  2006, Intel Corporation                                                         
#  All rights reserved. This program and the accompanying materials                          
#  are licensed and made available under the terms and conditions of the BSD License         
#  which accompanies this distribution.  The full text of the license may be found at        
#  http://opensource.org/licenses/bsd-license.php                                            
#                                                                                            
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
#  
#  Module Name:
#  
#    Makefile
#  
#  Abstract:
#   
#    This makefile is used to build different targets.
#   
#--*/

#
# Everything depends on EDK_SOURCE & EDK_TOOLS_PATH. Make sure it's defined
#
!IFNDEF EDK_SOURCE
!ERROR EDK_SOURCE environmental variable not set 
!ENDIF

!IFNDEF EDK_TOOLS_PATH
!MESSAGE EDK_TOOLS_PATH environmental variable not set, default setting used
!ENDIF

######################################################################################
#    Default build target is efi32
######################################################################################
all : efi32

######################################################################################
#    Feature flags for target efi32
######################################################################################
efi32 : Pseudotargets
    @if not exist $@ ( mkdir $@ & xcopy Build $@ /E ) else ( copy Build\*.* $@ )
    @if exist $@\Config.env del /f $@\Config.env
    @findstr /V "UEFI_MODE BUILD_TARGET_DIRECTORY EFI_BUILD_TARGET_X64" Build\Config.env > $@\Config.env
    @echo UEFI_MODE                      = NO                  >> $@\Config.env
    @echo EFI_BUILD_TARGET_X64           = NO                  >> $@\Config.env
    @echo BUILD_TARGET_DIRECTORY         = $@                  >> $@\Config.env
    @cd $@
    @nmake all
    @cd ..

######################################################################################
#    Feature flags for target efi64
######################################################################################
efi64 : Pseudotargets
    @if not exist $@ ( mkdir $@ & xcopy Build $@ /E ) else ( copy Build\*.* $@ )
    @if exist $@\Config.env del /f $@\Config.env
    @findstr /V "UEFI_MODE BUILD_TARGET_DIRECTORY EFI_BUILD_TARGET_X64" Build\Config.env > $@\Config.env
    @echo UEFI_MODE                      = NO                  >> $@\Config.env
    @echo EFI_BUILD_TARGET_X64           = YES                 >> $@\Config.env
    @echo BUILD_TARGET_DIRECTORY         = $@                  >> $@\Config.env
    @cd $@
    @nmake all
    @cd ..

######################################################################################
#    Feature flags for target uefi32
######################################################################################
uefi32 : Pseudotargets
    @if not exist $@ ( mkdir $@ & xcopy Build $@ /E ) else ( copy Build\*.* $@ )
    @if exist $@\Config.env del /f $@\Config.env
    @findstr /V "UEFI_MODE BUILD_TARGET_DIRECTORY EFI_BUILD_TARGET_X64" Build\Config.env > $@\Config.env
    @echo UEFI_MODE                      = YES                 >> $@\Config.env
    @echo EFI_BUILD_TARGET_X64           = NO                  >> $@\Config.env
    @echo BUILD_TARGET_DIRECTORY         = $@                  >> $@\Config.env
    @cd $@
    @nmake all
    @cd ..

######################################################################################
#    Feature flags for target uefi64
######################################################################################
uefi64 : Pseudotargets
    @if not exist $@ ( mkdir $@ & xcopy Build $@ /E ) else ( copy Build\*.* $@ )
    @if exist $@\Config.env del /f $@\Config.env
    @findstr /V "UEFI_MODE BUILD_TARGET_DIRECTORY EFI_BUILD_TARGET_X64" Build\Config.env > $@\Config.env
    @echo UEFI_MODE                      = YES                 >> $@\Config.env
    @echo EFI_BUILD_TARGET_X64           = YES                 >> $@\Config.env
    @echo BUILD_TARGET_DIRECTORY         = $@                  >> $@\Config.env
    @cd $@
    @nmake all
    @cd ..

Pseudotargets:

efi32clean:
    @cd efi32
    @nmake clean
    @cd ..

efi64clean:
    @cd efi64
    @nmake clean
    @cd ..

uefi32clean:
    @cd uefi32
    @nmake clean
    @cd ..

uefi64clean:
    @cd uefi64
    @nmake clean
    @cd ..

clean:
    - @if exist efi32 nmake efi32clean
    - @rd /s /q efi32 > NUL 2>&1
    - @if exist efi64 nmake efi64clean
    - @rd /s /q efi64 > NUL 2>&1
    - @if exist uefi32 nmake uefi32clean
    - @rd /s /q uefi32 > NUL 2>&1
    - @if exist uefi64 nmake uefi64clean
    - @rd /s /q uefi64 > NUL 2>&1
