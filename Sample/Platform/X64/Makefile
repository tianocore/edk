#/*++
#
#  Copyright (c)  2006 - 2007, Intel Corporation                                                         
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
#    Default build target
######################################################################################
all : uefi

######################################################################################
#    Feature flags for target efi
######################################################################################
BUILD_DIR = efi

$(BUILD_DIR)dir : Pseudotargets
    @if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
    @copy Build\*.* $(BUILD_DIR)
    @if exist $(BUILD_DIR)\Config.env del /f $(BUILD_DIR)\Config.env
    @findstr /V "UEFI_MODE PI_MODE BUILD_TARGET_DIRECTORY" Build\Config.env > $(BUILD_DIR)\Config.env
    @echo UEFI_MODE                      = NO                  >> $(BUILD_DIR)\Config.env
    @echo PI_MODE                        = NO                  >> $(BUILD_DIR)\Config.env
    @echo BUILD_TARGET_DIRECTORY         = $(BUILD_DIR)        >> $(BUILD_DIR)\Config.env

$(BUILD_DIR) : $(BUILD_DIR)dir
    @cd $(BUILD_DIR)
    @nmake all
    @cd ..

######################################################################################
#    Feature flags for target uefi
######################################################################################
BUILD_DIR = uefi

$(BUILD_DIR)dir : Pseudotargets
    @if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
    @copy Build\*.* $(BUILD_DIR)
    @if exist $(BUILD_DIR)\Config.env del /f $(BUILD_DIR)\Config.env
    @findstr /V "UEFI_MODE PI_MODE BUILD_TARGET_DIRECTORY" Build\Config.env > $(BUILD_DIR)\Config.env
    @echo UEFI_MODE                      = YES                 >> $(BUILD_DIR)\Config.env
    @echo PI_MODE                        = YES                 >> $(BUILD_DIR)\Config.env
    @echo BUILD_TARGET_DIRECTORY         = $(BUILD_DIR)        >> $(BUILD_DIR)\Config.env

$(BUILD_DIR) : $(BUILD_DIR)dir
    @cd $(BUILD_DIR)
    @nmake all
    @cd ..

Pseudotargets:

eficlean:
    @cd efi
    @nmake clean
    @cd ..

ueficlean:
    @cd uefi
    @nmake clean
    @cd ..

eficleanall:
    @cd efi
    @nmake cleanall
    @cd ..

ueficleanall:
    @cd uefi
    @nmake cleanall
    @cd ..

clean:
    - @if exist efi nmake eficlean
    - @if exist uefi nmake ueficlean

cleanall:
    - @if exist efi nmake eficleanall
    - @rd /s /q efi > NUL 2>&1
    - @if exist uefi nmake ueficleanall
    - @rd /s /q uefi > NUL 2>&1
