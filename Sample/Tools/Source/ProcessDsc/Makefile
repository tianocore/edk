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
#  Module Name:
#  
#    Makefile
#  
#  Abstract:
#  
#    makefile for building the ProcessDsc utility.
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

TARGET_NAME         = ProcessDsc
TARGET_SRC_DIR      = $(EDK_TOOLS_SOURCE)\$(TARGET_NAME)
TARGET_EXE          = $(EDK_TOOLS_OUTPUT)\ProcessDsc.exe

#
# Build targets
#

all: $(TARGET_EXE)

INC_DEPS  = $(TARGET_SRC_DIR)\DSCFile.h    $(INC_DEPS)
INC_DEPS  = $(TARGET_SRC_DIR)\FWVolume.h   $(INC_DEPS)
INC_DEPS  = $(TARGET_SRC_DIR)\Exceptions.h $(INC_DEPS)
INC_DEPS  = $(TARGET_SRC_DIR)\Common.h     $(INC_DEPS)

LIBS      = $(LIBS) "$(EDK_TOOLS_OUTPUT)\Common.lib"

OBJECTS   = $(EDK_TOOLS_OUTPUT)\DSCFile.obj  \
            $(EDK_TOOLS_OUTPUT)\FWVolume.obj   \
            $(EDK_TOOLS_OUTPUT)\ProcessDsc.obj \
            $(EDK_TOOLS_OUTPUT)\Exceptions.obj

#
# Compile each source file
#

$(EDK_TOOLS_OUTPUT)\DSCFile.obj : $(TARGET_SRC_DIR)\DSCFile.c $(INC_DEPS)
  $(CC) $(C_FLAGS) $(TARGET_SRC_DIR)\DSCFile.c /Fo$@

$(EDK_TOOLS_OUTPUT)\FWVolume.obj : $(TARGET_SRC_DIR)\FWVolume.c $(INC_DEPS)
  $(CC) $(C_FLAGS) $(TARGET_SRC_DIR)\FWVolume.c /Fo$@

$(EDK_TOOLS_OUTPUT)\ProcessDsc.obj : $(TARGET_SRC_DIR)\ProcessDsc.c $(INC_DEPS)
  $(CC) $(C_FLAGS) $(TARGET_SRC_DIR)\ProcessDsc.c /Fo$@

$(EDK_TOOLS_OUTPUT)\Exceptions.obj : $(TARGET_SRC_DIR)\Exceptions.c $(INC_DEPS)
  $(CC) $(C_FLAGS) $(TARGET_SRC_DIR)\Exceptions.c /Fo$@


#
# Link the object files together
#

$(TARGET_EXE): $(OBJECTS)
  $(LINK) $(MSVS_LINK_LIBPATHS) $(L_FLAGS) $(LIBS) /out:$(TARGET_EXE) $(OBJECTS)

clean:
  @if exist $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).* del $(EDK_TOOLS_OUTPUT)\$(TARGET_NAME).* > NUL
  @if exist $(EDK_TOOLS_OUTPUT)\DscFile.* del $(EDK_TOOLS_OUTPUT)\DscFile.* > NUL
  @if exist $(EDK_TOOLS_OUTPUT)\Exceptions* del $(EDK_TOOLS_OUTPUT)\Exceptions.* > NUL
  @if exist $(EDK_TOOLS_OUTPUT)\FwVolume.* del $(EDK_TOOLS_OUTPUT)\FwVolume.* > NUL
