#/*++
#   
# Copyright 2004, Intel Corporation                                                         
# All rights reserved. This program and the accompanying materials                          
# are licensed and made available under the terms and conditions of the BSD License         
# which accompanies this distribution.  The full text of the license may be found at        
# http://opensource.org/licenses/bsd-license.php                                            
#                                                                                           
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
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
COMMON_SOURCE      = $(EDK_TOOLS_COMMON)

#
# Common information
#

INC=$(INC)

#
# Target specific information
#

TARGET_NAME=GenDepex
TARGET_LIB_NAME=DepexParser
TARGET_SOURCE_DIR = $(EDK_TOOLS_SOURCE)\$(TARGET_NAME)

TARGET_LIB = $(EDK_TOOLS_OUTPUT)\$(TARGET_LIB_NAME).lib
TARGET_EXE = $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).exe

TARGET_LIB_SOURCE = "$(TARGET_SOURCE_DIR)\DepexParser.c"
TARGET_LIB_INCLUDE = "$(TARGET_SOURCE_DIR)\DepexParser.h"

TARGET_EXE_SOURCE = "$(TARGET_SOURCE_DIR)\GenDepex.c"
TARGET_EXE_INCLUDE = "$(EDK_SOURCE)\Foundation\Include\TianoCommon.h"

#
# Build targets
#

all: $(TARGET_LIB) $(TARGET_EXE)

#
# Build EXE
#

$(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj: $(TARGET_EXE_SOURCE) $(TARGET_EXE_INCLUDE)
  $(CC) $(C_FLAGS) $(INC) $(TARGET_EXE_SOURCE) /Fo$(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj

$(TARGET_EXE): $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj $(TARGET_EXE_LIBS) $(TARGET_DLL)
  $(LINK) $(MSVS_LINK_LIBPATHS) $(L_FLAGS) $(LIBS) /out:$(TARGET_EXE) $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj $(TARGET_LIB)  $(TARGET_EXE_LIBS)

#
# Build LIB
#

$(TARGET_LIB): $(EDK_TOOLS_OUTPUT)\$(TARGET_LIB_NAME).obj 
  $(LIB) $(LIB_FLAGS) $(EDK_TOOLS_OUTPUT)\$(TARGET_LIB_NAME).obj /OUT:$(TARGET_LIB)

$(EDK_TOOLS_OUTPUT)\$(TARGET_LIB_NAME).obj:  $(TARGET_LIB_SOURCE) $(TARGET_LIB_INCLUDE)
  $(CC) $(C_FLAGS) $(INC) $(TARGET_LIB_SOURCE) /Fo$(EDK_TOOLS_OUTPUT)\$(TARGET_LIB_NAME).obj

clean:
  @if exist $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).* del $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).* > NUL
