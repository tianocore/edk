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
efi : Pseudotargets
    @if not exist $@ ( mkdir $@ & xcopy Build $@ /E ) else ( copy Build\*.* $@ )
    @if exist $@\Config.env del /f $@\Config.env
    @type Config.env                                           >  $@\Config.env
    @echo UEFI_MODE                      = NO                  >> $@\Config.env
    @echo BUILD_TARGET_DIRECTORY         = $@                  >> $@\Config.env
    @cd $@
    @nmake all
    @cd ..

######################################################################################
#    Feature flags for target uefi
######################################################################################
uefi : Pseudotargets
    @if not exist $@ ( mkdir $@ & xcopy Build $@ /E ) else ( copy Build\*.* $@ )
    @if exist $@\Config.env del /f $@\Config.env
    @type Config.env                                           >  $@\Config.env
    @echo UEFI_MODE                      = YES                 >> $@\Config.env
    @echo BUILD_TARGET_DIRECTORY         = $@                  >> $@\Config.env
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
