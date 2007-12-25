#/*++
#
#  Copyright (c) 2007, Intel Corporation                                                         
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
#    This file is used to build the EFI build tools.
#
#--*/

#
# Define our toolchain before we include the master settings file
#
TOOLCHAIN = TOOLCHAIN_MSVC

!INCLUDE $(BUILD_DIR)\PlatformTools.env

#
# Define all the makefiles we want to call
#

MAKEFILES = $(DUET_TOOLS_SOURCE)\BootsectImage\Makefile      \
            $(DUET_TOOLS_SOURCE)\GenBootsector\Makefile      \
            $(DUET_TOOLS_SOURCE)\GenPage\Makefile            \
            $(DUET_TOOLS_SOURCE)\EfildrImage\Makefile

#
# Define default all target which calls all our makefiles. The special
# bang (!) tells nmake to do the command for each out-of-date dependent.
#
# Create the BIN directory, which will only exist if you pull the source tree
# from version control.
#
all : $(MAKEFILES)
  -if not exist $(EDK_TOOLS_OUTPUT) mkdir $(EDK_TOOLS_OUTPUT)
  !$(MAKE) -f $? TOOLCHAIN=$(TOOLCHAIN) BUILD_DIR=$(BUILD_DIR) all
  
#
# Call all the tools makefiles with a clean target.
#
clean : $(MAKEFILES)
  !$(MAKE) -f $? TOOLCHAIN=$(TOOLCHAIN) clean
