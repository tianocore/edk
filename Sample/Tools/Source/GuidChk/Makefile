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
#  Module Name:
#  
#    Makefile
#  
#  Abstract:
#  
#    makefile for building the GUID check utility.
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
# Target specific information
#

TARGET_NAME         = GuidChk
TARGET_SRC_DIR      = $(EDK_TOOLS_SOURCE)\$(TARGET_NAME)
TARGET_EXE          = $(EDK_TOOLS_OUTPUT)\GuidChk.exe

#LIBS                = $(LIBS) "$(EDK_TOOLS_OUTPUT)\Common.lib"

#
# Build targets
#
all: $(TARGET_EXE)

INC_DEPS  = $(TARGET_SRC_DIR)\FileSearch.h    $(INC_DEPS)
#INC_DEPS  = $(TARGET_SRC_DIR)\CommonUtils.h   $(INC_DEPS)
#INC_DEPS  = $(TARGET_SRC_DIR)\UtilsMsgs.h     $(INC_DEPS)

OBJECTS   = $(EDK_TOOLS_OUTPUT)\GuidChk.obj  \
            $(EDK_TOOLS_OUTPUT)\FileSearch.obj \
            $(EDK_TOOLS_OUTPUT)\GuidList.obj \
            $(EDK_TOOLS_OUTPUT)\UtilsMsgs.obj
#
# Compile each source file
#
$(EDK_TOOLS_OUTPUT)\GuidChk.obj : $(TARGET_SRC_DIR)\GuidChk.c $(INC_DEPS)
  $(CC) $(C_FLAGS) $(INC) $(TARGET_SRC_DIR)\GuidChk.c /Fo$@

$(EDK_TOOLS_OUTPUT)\FileSearch.obj : $(TARGET_SRC_DIR)\FileSearch.c $(INC_DEPS)
  $(CC) $(C_FLAGS) $(INC) $(TARGET_SRC_DIR)\FileSearch.c /Fo$@

$(EDK_TOOLS_OUTPUT)\GuidList.obj : $(TARGET_SRC_DIR)\GuidList.c $(INC_DEPS)
  $(CC) $(C_FLAGS) $(INC) $(TARGET_SRC_DIR)\GuidList.c /Fo$@

$(EDK_TOOLS_OUTPUT)\UtilsMsgs.obj : $(TARGET_SRC_DIR)\UtilsMsgs.c $(INC_DEPS)
  $(CC) $(C_FLAGS) $(INC) $(TARGET_SRC_DIR)\UtilsMsgs.c /Fo$@

#
# Link the object files together, then run it to generate the GUID-to-name list file
#
$(TARGET_EXE) : $(OBJECTS)
  $(LINK) $(MSVS_LINK_LIBPATHS) $(L_FLAGS) $(LIBS) /out:$(TARGET_EXE) $(OBJECTS)
  $(TARGET_EXE) -b $(EDK_TOOLS_OUTPUT)\GuidDatabase.txt

clean:
  @if exist $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).* del $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).* > NUL
  @if exist $(EDK_TOOLS_OUTPUT)\FileSearch.* del $(EDK_TOOLS_OUTPUT)\FileSearch.* > NUL
