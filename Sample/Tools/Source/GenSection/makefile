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
# Common information
#
INC                 = $(INC) -I $(EDK_TOOLS_SOURCE)\Common

LIBS                = $(LIBS) $(EDK_TOOLS_OUTPUT)\CustomizedCompress.lib

#
# Target specific information
#

TARGET_NAME         = GenSection
TARGET_SOURCE_DIR   = $(EDK_TOOLS_SOURCE)\$(TARGET_NAME)
TARGET_EXE          = $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).exe
TARGET_EXE_SOURCE   = "$(TARGET_SOURCE_DIR)\GenSection.c"
TARGET_EXE_LIBS     = "$(EDK_TOOLS_OUTPUT)\Common.lib"
TARGET_EXE_INCLUDE  = "$(EDK_SOURCE)\Foundation\Include\TianoCommon.h" \
                     "$(EDK_SOURCE)\Foundation\Framework\Include\EfiFirmwareFileSystem.h" \
                     "$(EDK_SOURCE)\Foundation\Framework\Include\EfiFirmwareVolumeHeader.h" \
                     "$(EDK_TOOLS_COMMON)\ParseInf.h"

#
# Build targets
#

all: $(TARGET_EXE)

#
# Build EXE
#

$(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj: $(TARGET_EXE_SOURCE) $(TARGET_EXE_INCLUDE)
  $(CC) $(C_FLAGS) $(INC) $(TARGET_EXE_SOURCE) /Fo$(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj

$(TARGET_EXE): $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj $(TARGET_EXE_LIBS) $(TARGET_DLL) $(LIBS)
  $(LINK) /DEBUG $(MSVS_LINK_LIBPATHS) $(L_FLAGS) $(LIBS) /NODEFAULTLIB:LIBCMT /out:$(TARGET_EXE) $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj $(TARGET_LIB)  $(TARGET_EXE_LIBS)

clean:
  @if exist $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).* del $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).* > NUL
