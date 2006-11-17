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
#    makefile
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
# BUGBUG: Override standard flags, cannot be built without warnings.
#

C_FLAGS=/nologo /W4 /GX /Zi /Od /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /c 

#
# Common information
#

INC=$(INC)

#
# Target specific information
#

TARGET_NAME=EfiCompress
TARGET_SOURCE_DIR = $(EDK_TOOLS_SOURCE)\$(TARGET_NAME)

TARGET_EXE = $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).exe

TARGET_EXE_SOURCE = "$(TARGET_SOURCE_DIR)\EfiCompressMain.c"
TARGET_EXE_INCLUDE = "$(COMMON_SOURCE)\Compress.h"
TARGET_EXE_LIBS = "$(EDK_TOOLS_OUTPUT)\Common.lib"

#
# Build targets
#

all: $(TARGET_EXE)

#
# Build EXE
#

$(EDK_TOOLS_OUTPUT)\EfiCompressMain.obj: $(TARGET_EXE_SOURCE) $(TARGET_EXE_INCLUDE)
  $(CC) $(C_FLAGS) $(INC) $(TARGET_EXE_SOURCE) /Fo$(EDK_TOOLS_OUTPUT)\EfiCompressMain.obj

$(TARGET_EXE): $(EDK_TOOLS_OUTPUT)\EfiCompressMain.obj $(TARGET_EXE_LIBS) $(TARGET_DLL)
  $(LINK) $(MSVS_LINK_LIBPATHS) $(L_FLAGS) $(LIBS) /out:$(TARGET_EXE) $(EDK_TOOLS_OUTPUT)\EfiCompressMain.obj $(TARGET_LIB)  $(TARGET_EXE_LIBS)

clean:
  @if exist $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME)Main.* del /q $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME)Main.* > NUL
