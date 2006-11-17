#/*++
#  
#  Copyright (c) 2006, Intel Corporation                                                         
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
#    makefile for building the GenPage utility.
#    
#--*/  

#
# Make sure environmental variable EDK_SOURCE is set
#
!IFNDEF EDK_SOURCE
!ERROR EDK_SOURCE environmental variable not set 
!ENDIF

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

TARGET_NAME=GenPage
TARGET_SOURCE_DIR = $(EDK_TOOLS_SOURCE)\$(TARGET_NAME)

TARGET_EXE = $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).exe

TARGET_EXE_SOURCE = "$(TARGET_SOURCE_DIR)\GenPage.c"
TARGET_EXE_INCLUDE = "$(TARGET_SOURCE_DIR)\VirtualMemory.h"

#
# Build targets
#

all: $(TARGET_EXE)

#
# Build EXE
#

$(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj: $(TARGET_EXE_SOURCE) $(TARGET_EXE_INCLUDE)
  $(CC) $(C_FLAGS) $(INC) $(TARGET_EXE_SOURCE) /Fo$(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj

$(TARGET_EXE): $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj $(TARGET_EXE_LIBS) $(TARGET_DLL)
  $(LINK) $(MSVS_LINK_LIBPATHS) $(L_FLAGS) $(LIBS) /out:$(TARGET_EXE) $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj $(TARGET_LIB)  $(TARGET_EXE_LIBS)

clean:
  @if exist $(TIANO_TOOLS_OUTPUT)\$(TARGET_NAME).* del $(TIANO_TOOLS_OUTPUT)\$(TARGET_NAME).* > NUL

