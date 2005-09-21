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
#    makefile
#  
#  Abstract:
#
#    This is the makefile for creating the Logo FFS file. It requires its own
#    makefile because it's a special build case that does not fit in well
#    with the standard build of drivers.
#
#--*/  

#
# Define some useful macros, then include the master Efi toolchain setup
# file.
#
BIN_DIR     = $(BUILD_DIR)\$(PROCESSOR)
TOOLCHAIN   = TOOLCHAIN_$(PROCESSOR)

!INCLUDE $(BUILD_DIR)\PlatformTools.env

#
# We simply copy the BMP file from the source directory to the build directory
#
$(BIN_DIR)\$(BASE_NAME).bmp : $(SOURCE_DIR)\$(BASE_NAME).bmp
  copy $(SOURCE_DIR)\$(BASE_NAME).bmp $(BIN_DIR)\$(BASE_NAME).bmp /y

#
# Use GenFFSFile to convert it to an FFS file
#
$(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).ffs : $(BIN_DIR)\$(BASE_NAME).bmp
  $(GENSECTION) -I $(BIN_DIR)\$(BASE_NAME).bmp -O $(BIN_DIR)\$(BASE_NAME).sec -S EFI_SECTION_RAW
  $(GENFFSFILE) -B $(BIN_DIR) -P1 $(DEST_DIR)\$(BASE_NAME).pkg -V

all: $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).ffs


