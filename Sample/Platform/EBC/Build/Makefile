#/*++
#
#  Copyright (c) 2006 - 2008, Intel Corporation                                                         
#  All rights reserved. This program and the accompanying materials                          
#  are licensed and made available under the terms and conditions of the BSD License         
#  which accompanies this distribution.  The full text of the license may be found at        
#  http://opensource.org/licenses/bsd-license.php                                            
#                                                                                            
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
#  
#  Module Name:  
#    Makefile
#  
#  Abstract:
#    Makefile for this build tip
#   
#--*/

#
# Everything depends on EDK_SOURCE being set. 
#
!IFNDEF EDK_SOURCE
!ERROR EDK_SOURCE environmental variable not set 
!ENDIF

!IFNDEF EDK_TOOLS_PATH
!MESSAGE EDK_TOOLS_PATH environmental variable not set, default setting used
!ENDIF

#
# Set the processor type
#
PROCESSOR = IA32

#
# Define a toolchain so we can pick up the basic defines from
# the EFI tool setup file.
#
TOOLCHAIN = TOOLCHAIN_MSVC

#
# Include the platform-specific settings file
#
!INCLUDE PlatformTools.env

#
# Default target to create the makefiles and then build the drivers
#
all : build_tools builds

#
# Make all the build tools by calling the makefile
#
build_tools :
  -if not exist $(EDK_TOOLS_OUTPUT) mkdir $(EDK_TOOLS_OUTPUT)
  -if exist $(EDK_TOOLS_BIN)\*.* copy $(EDK_TOOLS_BIN)\*.* $(EDK_TOOLS_OUTPUT)
  $(MAKE) -f $(EDK_TOOLS_SOURCE)\makefile PROCESSOR=$(PROCESSOR) BUILD_DIR=$(BUILD_DIR)
  $(GUIDCHK) -b $(EDK_TOOLS_OUTPUT)\GuidDatabase.txt

PROCESSOR = EBC

COMMON_PROCESSDSC_OPTIONS =                        \
    -d EFI_SOURCE=$(EDK_SOURCE)                    \
    -d EDK_SOURCE=$(EDK_SOURCE)                    \
    -d PROCESSOR=$(PROCESSOR)                      \
    -d PROJECT_NAME=$(PROJECT_NAME)                \
    -d PROJECT_FAMILY=$(PROJECT_FAMILY)            \
    -d UEFI_PREFIX=$(UEFI_PREFIX)                  \
    -d PI_PREFIX=$(PI_PREFIX)                      \
    -d COMPRESS_METHOD=$(COMPRESS_METHOD)          \
    -d OEMTOOLPATH=$(EDK_TOOLS_OUTPUT)             \
    -g $(EDK_TOOLS_OUTPUT)\GuidDatabase.txt

#
# Run ProcessDsc on our main description files to generate the
# makefiles and package files. Specify -v option for more verbose output.
# Specify "-xref filename" to generate a cross-reference file of 
# basename-to-fileguid that can be used by VolInfo.
#
makefiles : 
  $(PROCESSDSC) $(COMMON_PROCESSDSC_OPTIONS) $(PROJECT_NAME).dsc

#
# Multi-thread build using ProcessDsc utility.
#
builds :
  $(PROCESSDSC) $(COMMON_PROCESSDSC_OPTIONS) -n $(BUILD_THREAD_NUMBER) \
                -t libraries -t components $(PROJECT_NAME).dsc

#
# Define a pseudo-target to start building with the libraries or with the 
# components.
#
libraries :
  $(MAKE) -f makefile.out libraries 

components :
  $(MAKE) -f makefile.out components


#
# Create a target to clean up all build directories
#
clean : cleantools cleanbuilds

#
# Clean up all build directories except for tools.
#
cleanbuilds :
  - @if exist *.bak del *.bak > NUL
  - @if exist ia32 rd /s/q ia32 > NUL
  - @if exist ipf rd /s /q ipf > NUL
  - @if exist ebc rd /s /q ebc > NUL
  - @if exist x64 rd /s /q x64 > NUL
  - @if exist fv rd /s /q fv > NUL
  - @if exist log rd /s /q log > NUL
  - @if exist makefile.out del makefile.out > NUL
  - @if exist module.* del module.* > NUL
  - @echo All modules cleaned

#
# Clean up only the tools
#
cleantools :
  - @if exist vc??.* del vc??.* > NUL
  - @if exist Tools rd /s /q Tools > NUL
  - @echo All tools cleaned

#
# Rebuild target, clean and build all
#
rebuild : clean all

#
# cleanall target to clean all build directories
# and binary files in $(EFI_PLATFORM_BIN).
#
cleanall : cleantools cleanbuilds
  - @if exist $(EFI_PLATFORM_BIN) rd /s /q $(EFI_PLATFORM_BIN) > NUL
  - @if exist $(EFI_BINARY) rd $(EFI_BINARY) > NUL 2>&1
  - @echo All targets cleaned
