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
#    makefile for building the MakeDeps utility.
#    
#--*/  

#
# Make sure environmental variable EDK_SOURCE is set
#
!IFNDEF EDK_SOURCE
!ERROR EDK_SOURCE environmental variable not set 
!ENDIF

!INCLUDE PlatformTools.env

#
# Target specific information
#
TARGET_SRC_DIR  = $(EDK_TOOLS_SOURCE)\MakeDeps
TARGET_EXE      = $(EDK_TOOLS_OUTPUT)\MakeDeps.exe

OBJECTS         = $(EDK_TOOLS_OUTPUT)\MakeDeps.obj  
LIBS            = $(LIBS) $(EDK_TOOLS_OUTPUT)\Common.lib

#
# Build targets
#

all: $(TARGET_EXE)

#
# Compile each tool source file
#
$(EDK_TOOLS_OUTPUT)\MakeDeps.obj : $(TARGET_SRC_DIR)\MakeDeps.c $(INC_DEPS)
  $(CC) $(C_FLAGS) $(TARGET_SRC_DIR)\MakeDeps.c /Fo$@

#
# Link the objects and libs to create the executable
#
$(TARGET_EXE) : $(OBJECTS) $(LIBS)
  $(LINK) $(MSVS_LINK_LIBPATHS) $(L_FLAGS) $(LIBS) $(OBJECTS) /out:$(TARGET_EXE)

