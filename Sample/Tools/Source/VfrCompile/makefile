#/*++
#  
#  Copyright (c) 2004 - 2006, Intel Corporation                                                         
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
#    Makefile for building the EFI VFR compiler
#    
#--*/  


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

BASE_NAME         = VfrCompile
ANTLR_H           = $(PCCTS_DIR)\h
C_FLAGS_PCCTS     = -I. -I$(ANTLR_H) /Zi /WX /Od /D _CRT_SECURE_NO_DEPRECATE $(VERSION_FLAGS)
C_FLAGS_PCCTS     = $(C_FLAGS_PCCTS)  

LINK_FLAGS_PCCTS  =
LIBS              = "$(EDK_TOOLS_OUTPUT)\Common.lib"

# 
# Define the EFI output and source directories.
#
ETO               = $(EDK_TOOLS_OUTPUT)
SRC               = $(EDK_TOOLS_SOURCE)\$(BASE_NAME)
TARGET_EXE        = $(ETO)\$(BASE_NAME).exe

#
# Add deeper lookahead with -ck 3
#
ANTLR_FLAGS       = -CC -e3 -ck 3
DLG_FLAGS         = -C2 -i -CC

#
# Define paths for include files
#
INC = -I $(SRC) 
INC = $(INC) -I $(EDK_SOURCE)\Foundation\Include\Ia32
INC = $(INC) -I $(EDK_SOURCE)\Foundation\Efi\Include
INC = $(INC) -I $(EDK_SOURCE)\Foundation\Framework\Include
INC = $(INC) -I $(EDK_SOURCE)\Foundation\Include\IndustryStandard
INC = $(INC) -I $(EDK_SOURCE)\Foundation\         \
             -I $(EDK_SOURCE)\Foundation\Core\Dxe \
             -I $(EDK_SOURCE)\Foundation\Efi      \
             -I $(EDK_SOURCE)\Foundation\Framework
INC = $(INC) -I $(EDK_TOOLS_SOURCE)\Common
INC = $(INC) -I $(EDK_SOURCE)\Foundation\Include 

DLG_FILE          = Parser.dlg
SCAN_FILE         = DLGLexer
PARSER_FILE       = EfiVfrParser

#
# Create a list of include dependencies 
#
INC_DEPS = $(INC_DEPS) $(SRC)\EfiVfr.h
INC_DEPS = $(INC_DEPS) $(EDK_SOURCE)\Foundation\Framework\Include\EfiInternalFormRepresentation.h

#
# This is the grammer file for our project
#
GRAMMER_FILE  = $(SRC)\$(BASE_NAME).g

#
# If we utilize a separate token file, define it here.
#
#TOKEN_FILE    = 

    
OBJECTS =       $(ETO)\$(BASE_NAME).obj \
                $(ETO)\$(PARSER_FILE).obj \
                $(ETO)\AParser.obj \
                $(ETO)\DLexerBase.obj \
                $(ETO)\ATokenBuffer.obj \
                $(ETO)\$(SCAN_FILE).obj  \
                $(ETO)\VfrServices.obj

#
# Per the Language Translation Using PCCTS and C++ Reference Guide, page 109,
# these are the outputs of ANTLR and DLG
#                
ANTLR_SPAWN =   $(ETO)\$(BASE_NAME).cpp \
                $(ETO)\$(PARSER_FILE).cpp \
	              $(ETO)\$(PARSER_FILE).h \
                $(ETO)\$(DLG_FILE) \
                $(ETO)\tokens.h 

DLG_SPAWN =     $(ETO)\$(SCAN_FILE).cpp \
                $(ETO)\$(SCAN_FILE).h


#
# Default target
#
all : $(TARGET_EXE)

#
# All antlr-generated files depend on the .g grammer file. Use the -o
# option to emit them to the appropriate output directory.
#
$(ANTLR_SPAWN) : $(GRAMMER_FILE) $(INC_DEPS)
	$(ANTLR) $(ANTLR_FLAGS) -o $(ETO) $(GRAMMER_FILE)

$(ETO)\$(BASE_NAME).obj : $(ETO)\$(BASE_NAME).cpp $(DLG_SPAWN) $(INC_DEPS)
	$(CC) -c $(C_FLAGS_PCCTS) /Fo$@ $(INC) $(ETO)\$(BASE_NAME).cpp

$(ETO)\$(SCAN_FILE).obj : $(ETO)\$(SCAN_FILE).cpp $(DLG_SPAWN) $(INC_DEPS)
	$(CC) -c $(C_FLAGS_PCCTS) /Fo$@ $(INC) $(ETO)\$(SCAN_FILE).cpp

$(ETO)\$(PARSER_FILE).obj : $(ETO)\$(PARSER_FILE).cpp $(ETO)\$(PARSER_FILE).h $(DLG_SPAWN) $(INC_DEPS)
  $(CC) -c $(C_FLAGS_PCCTS) /Fo$@ $(INC) $(ETO)\$(PARSER_FILE).cpp

$(DLG_SPAWN) : $(ETO)\$(DLG_FILE) $(INC_DEPS)
	$(DLG) $(DLG_FLAGS) -o $(ETO) $(ETO)\$(DLG_FILE)

$(ETO)\AParser.obj : $(ANTLR_H)/AParser.cpp
	$(CC) -c $(C_FLAGS_PCCTS) /Fo$@ $(ANTLR_H)/AParser.cpp

$(ETO)\ATokenBuffer.obj : $(ANTLR_H)/ATokenBuffer.cpp
	$(CC) -c $(C_FLAGS_PCCTS) /Fo$@ $(ANTLR_H)/ATokenBuffer.cpp

$(ETO)\DLexerBase.obj : $(ANTLR_H)/DLexerBase.cpp
	$(CC) -c $(C_FLAGS_PCCTS) /Fo$@ $(ANTLR_H)/DLexerBase.cpp

$(ETO)\VfrServices.obj : $(SRC)\VfrServices.cpp $(SRC)\VfrServices.h $(INC_DEPS)
	$(CC) -c $(C_FLAGS_PCCTS) $(INC) /Fo$@ $(SRC)\VfrServices.cpp

$(ETO)\$(BASE_NAME).exe : $(OBJECTS)
	$(LINK) $(MSVS_LINK_LIBPATHS) $(LIBS) /DEBUG /OUT:$(ETO)\$(BASE_NAME).exe $(LINK_FLAGS_PCCTS) $(OBJECTS) /PDB:$*.pdb


clean:
  
