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
#    makefile for building the StrGather utility.
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
TARGET_NAME         = StrGather
TARGET_SRC_DIR      = $(EDK_TOOLS_SOURCE)\$(TARGET_NAME)
TARGET_EXE          = $(EDK_TOOLS_OUTPUT)\StrGather.exe

#
# Build targets
#

all: $(TARGET_EXE)


LIBS      = "$(EDK_TOOLS_OUTPUT)\Common.lib"

OBJECTS   = $(EDK_TOOLS_OUTPUT)\StrGather.obj  \
            $(EDK_TOOLS_OUTPUT)\StringDB.obj   

INC_DEPS  = $(TARGET_SRC_DIR)\StrGather.h $(TARGET_SRC_DIR)\StringDB.h 

C_FLAGS = $(C_FLAGS) /W4

#
# Compile each source file
#
$(EDK_TOOLS_OUTPUT)\StrGather.obj : $(TARGET_SRC_DIR)\StrGather.c $(INC_DEPS)
  $(CC) $(C_FLAGS) $(INC_PATHS) $(TARGET_SRC_DIR)\StrGather.c /Fo$@

$(EDK_TOOLS_OUTPUT)\StringDB.obj : $(TARGET_SRC_DIR)\StringDB.c $(INC_DEPS)
  $(CC) $(C_FLAGS) $(INC_PATHS) $(TARGET_SRC_DIR)\StringDB.c /Fo$@

#
# Link the object files together
#
$(TARGET_EXE) : $(OBJECTS) $(LIBS)
  $(LINK) $(MSVS_LINK_LIBPATHS) $(L_FLAGS) $(LIBS) /out:$(TARGET_EXE) $(OBJECTS)

clean:
  
