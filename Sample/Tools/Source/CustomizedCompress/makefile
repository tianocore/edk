#/*++
#   
#  Copyright 2004, Intel Corporation                                                         
#  All rights reserved. This program and the accompanying materials                          
#  are licensed and made available under the terms and conditions of the BSD License         
#  which accompanies this distribution.  The full text of the license may be found at        
#  http://opensource.org/licenses/bsd-license.php                                            
#                                                                                            
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
#  
#  Module Name:  makefile
#   
#  Abstract:
#   
#    This file is used to build the EFI utility.
#   
#--*/

#
# Do this if you want to compile from this directory
#
!IFNDEF TOOLCHAIN
TOOLCHAIN = TOOLCHAIN_MSVC
!ENDIF

!INCLUDE PlatformTools.env

#
# Define some macros we use here. Should get rid of them someday and 
# get rid of the extra level of indirection.
#
TARGET_NAME        = CustomizedCompress
TARGET_SOURCE_DIR  = $(EDK_TOOLS_SOURCE)\$(TARGET_NAME)
COMMON_SOURCE      = $(EDK_TOOLS_COMMON)

#
# Common information
#

TARGET_LIB = $(EDK_TOOLS_OUTPUT)\CustomizedCompress.lib

OBJECTS = "$(EDK_TOOLS_OUTPUT)\CustomizedCompress.obj" 

#
# Build targets
#

all: $(TARGET_LIB)

#
# Object targets
#
 
"$(EDK_TOOLS_OUTPUT)\CustomizedCompress.obj": "$(TARGET_SOURCE_DIR)\CustomizedCompress.c" $(EDK_SOURCE)\Foundation\Include\EfiCommon.h
  $(CC) $(C_FLAGS) "$(TARGET_SOURCE_DIR)\CustomizedCompress.c" /Fo"$(EDK_TOOLS_OUTPUT)\CustomizedCompress.obj"

#
# Build LIB
#

$(TARGET_LIB): $(OBJECTS)
  $(LIB) $(LIB_FLAGS) $(OBJECTS) /OUT:$(TARGET_LIB)

clean:
  @if exist $(EDK_TOOLS_OUTPUT)\CustomizedCompress.* del /q $(EDK_TOOLS_OUTPUT)\CustomizedCompress.* > NUL
  @if exist $(TARGET_LIB) del $(TARGET_LIB)
