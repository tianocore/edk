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

EFI_SOURCE = $(EDK_SOURCE)

!IFNDEF EDK_TOOLS_PATH
!MESSAGE EDK_TOOLS_PATH environmental variable not set, default setting used
!ENDIF

######################################################################################
#    Default build target is efi
######################################################################################
all : efi

######################################################################################
#    Target efi use default feature flags
######################################################################################
EFI_DEBUG                      = YES
EFI_DEBUG_CLEAR_MEMORY         = NO
EFI_PEI_PERFORMANCE            = YES
EFI_DXE_PERFORMANCE            = YES
EFI_SYMBOLIC_DEBUG             = YES
EFI_GENERATE_INTERMEDIATE_FILE = NO
EFI_SPECIFICATION_VERSION      = 0x0001000A
TIANO_RELEASE_VERSION          = 0x00080006

efi : Pseudotargets
    @if not exist $@ mkdir $@
    @copy Build\*.* $@
    @if exist $@\Config.env del /f $@\Config.env
    @echo #                                                                  >  $@\Config.env
    @echo # This file is generated automatically.                            >> $@\Config.env
    @echo # Current build target is: $@                                      >> $@\Config.env
    @echo #                                                                  >> $@\Config.env
    @echo EFI_DEBUG                      = $(EFI_DEBUG)                      >> $@\Config.env
    @echo EFI_DEBUG_CLEAR_MEMORY         = $(EFI_DEBUG_CLEAR_MEMORY)         >> $@\Config.env
    @echo EFI_PEI_PERFORMANCE            = $(EFI_PEI_PERFORMANCE)            >> $@\Config.env
    @echo EFI_DXE_PERFORMANCE            = $(EFI_DXE_PERFORMANCE)            >> $@\Config.env
    @echo EFI_SYMBOLIC_DEBUG             = $(EFI_SYMBOLIC_DEBUG)             >> $@\Config.env
    @echo EFI_GENERATE_INTERMEDIATE_FILE = $(EFI_GENERATE_INTERMEDIATE_FILE) >> $@\Config.env
    @echo EFI_SPECIFICATION_VERSION      = $(EFI_SPECIFICATION_VERSION)      >> $@\Config.env
    @echo TIANO_RELEASE_VERSION          = $(TIANO_RELEASE_VERSION)          >> $@\Config.env
    @echo BUILD_TARGET_DIRECTORY         = $@                                >> $@\Config.env
    @cd $@
    @nmake all
    @cd ..

######################################################################################
#    Feature flags for target uefi
######################################################################################
EFI_DEBUG                      = YES
EFI_DEBUG_CLEAR_MEMORY         = NO
EFI_PEI_PERFORMANCE            = YES
EFI_DXE_PERFORMANCE            = YES
EFI_SYMBOLIC_DEBUG             = YES
EFI_GENERATE_INTERMEDIATE_FILE = NO
EFI_SPECIFICATION_VERSION      = 0x00020000
TIANO_RELEASE_VERSION          = 0x00080006

uefi : Pseudotargets
    @if not exist $@ mkdir $@
    @copy Build\*.* $@
    @if exist $@\Config.env del /f $@\Config.env
    @echo #                                                                  >  $@\Config.env
    @echo # This file is generated automatically.                            >> $@\Config.env
    @echo # Current build target is: $@                                      >> $@\Config.env
    @echo #                                                                  >> $@\Config.env
    @echo EFI_DEBUG                      = $(EFI_DEBUG)                      >> $@\Config.env
    @echo EFI_DEBUG_CLEAR_MEMORY         = $(EFI_DEBUG_CLEAR_MEMORY)         >> $@\Config.env
    @echo EFI_PEI_PERFORMANCE            = $(EFI_PEI_PERFORMANCE)            >> $@\Config.env
    @echo EFI_DXE_PERFORMANCE            = $(EFI_DXE_PERFORMANCE)            >> $@\Config.env
    @echo EFI_SYMBOLIC_DEBUG             = $(EFI_SYMBOLIC_DEBUG)             >> $@\Config.env
    @echo EFI_GENERATE_INTERMEDIATE_FILE = $(EFI_GENERATE_INTERMEDIATE_FILE) >> $@\Config.env
    @echo EFI_SPECIFICATION_VERSION      = $(EFI_SPECIFICATION_VERSION)      >> $@\Config.env
    @echo TIANO_RELEASE_VERSION          = $(TIANO_RELEASE_VERSION)          >> $@\Config.env
    @echo BUILD_TARGET_DIRECTORY         = $@                                >> $@\Config.env
    @cd $@
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

clean:
    - @if exist efi nmake eficlean
    - @rd /s /q efi > NUL 2>&1
    - @if exist uefi nmake ueficlean
    - @rd /s /q uefi > NUL 2>&1
