#/*++
#   
#  Copyright (c) 2004, Intel Corporation                                                         
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

INC=$(INC) \
    -I "$(EDK_TOOLS_COMMON)"



#
# Target specific information
#

TARGET_NAME=GenFvImage

TARGET_SOURCE_DIR   = $(EDK_TOOLS_SOURCE)\$(TARGET_NAME)
TARGET_LIB          = $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).lib
TARGET_EXE          = $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).exe
TARGET_EXE_SOURCE   = "$(TARGET_SOURCE_DIR)\GenFvImageExe.c"

TARGET_EXE_INCLUDE  = "$(TARGET_SOURCE_DIR)\GenFvImageExe.h" \
                         "$(TARGET_SOURCE_DIR)\GenFvImageLib.h" \
                         "$(EDK_TOOLS_COMMON)\ParseInf.h" \
                         "$(EDK_SOURCE)\Foundation\Include\TianoCommon.h"

TARGET_EXE_LIBS     = "$(EDK_TOOLS_OUTPUT)\Common.lib" 
TARGET_LIB_SOURCE   = "$(TARGET_SOURCE_DIR)\GenFvImageLib.c"

TARGET_LIB_INCLUDE  = "$(TARGET_SOURCE_DIR)\GenFvImageLib.h" \
                         "$(TARGET_SOURCE_DIR)\GenFvImageLibInternal.h" \
                         "$(EDK_TOOLS_COMMON)\ParseInf.h" \
                         "$(EDK_SOURCE)\Foundation\Include\TianoCommon.h" \
                         "$(EDK_SOURCE)\Foundation\Framework\Include\EfiFirmwareVolumeHeader.h" \
                         "$(EDK_SOURCE)\Foundation\Framework\Include\EfiFirmwareFileSystem.h" \
                         "$(EDK_SOURCE)\Foundation\Framework\Guid\FirmwareFileSystem\FirmwareFileSystem.h"

TARGET_LIB_LIBS = "$(EDK_TOOLS_OUTPUT)\Common.lib"
#
# Build targets
#

all: $(TARGET_LIB) $(TARGET_EXE)

#
# Build EXE
#

$(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj: $(TARGET_EXE_SOURCE) $(TARGET_EXE_INCLUDE)
  $(CC) $(C_FLAGS) $(INC) $(TARGET_EXE_SOURCE) /Fo$(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj

$(TARGET_EXE): $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj $(TARGET_EXE_LIBS) $(TARGET_LIB)
  $(LINK) $(MSVS_LINK_LIBPATHS) $(L_FLAGS) $(LIBS) /out:$(TARGET_EXE) $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).obj $(TARGET_LIB)  $(TARGET_EXE_LIBS)

#
# Build LIB
#

$(TARGET_LIB): $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME)Lib.obj $(TARGET_LIB_LIBS)
  $(LIB) $(LIB_FLAGS) $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME)Lib.obj $(MSVS_LINK_LIBPATHS) $(TARGET_LIB_LIBS) RPCRT4.lib /OUT:$(TARGET_LIB)

$(EDK_TOOLS_OUTPUT)\$(TARGET_NAME)Lib.obj:  $(TARGET_LIB_SOURCE) $(TARGET_LIB_INCLUDE)
  $(CC) $(C_FLAGS) $(INC) $(TARGET_LIB_SOURCE) /Fo$(EDK_TOOLS_OUTPUT)\$(TARGET_NAME)Lib.obj

clean:
  @if exist $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME)Lib.* del $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME)Lib.* > NUL
  @if exist $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).* del $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).* > NUL
