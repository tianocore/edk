#/*++
#
# Copyright 2004, Intel Corporation                                                         
# All rights reserved. This program and the accompanying materials                          
# are licensed and made available under the terms and conditions of the BSD License         
# which accompanies this distribution.  The full text of the license may be found at        
# http://opensource.org/licenses/bsd-license.php                                            
#                                                                                           
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
# 
#  Module Name:
#
#    Common.dsc
#
#  Abstract:
#
#    This is the build description file containing the platform
#    independent build instructions.  Platform specific instructions will
#    be prepended to produce the final build DSC file.
#
#
#  Notes:
#    
#    The info in this file is broken down into sections. The start of a section
#    is designated by a "[" in the first column. So the [=====] separater ends
#    a section.
#    
#--*/

[=============================================================================]
#
# These get emitted at the top of the generated master makefile. 
#
[=============================================================================]
[Makefile.out]

#
# From the [makefile.out] section of the DSC file
#
TOOLCHAIN = 
MAKE      = nmake -nologo

!INCLUDE $(BUILD_DIR)\PlatformTools.env

all : libraries fvs fv_null_components

#
# summary target for components with FV=NULL to handle the case
# when no components exist with FV=NULL
#
fv_null_components ::
  @echo.

[=============================================================================]
#
# These get expanded and dumped out to each component makefile after the
# component INF [defines] section gets parsed.
#
[=============================================================================]
[Makefile.Common]
#
# From the [Makefile.Common] section of the description file.
#
PROCESSOR        = $(PROCESSOR)
BASE_NAME        = $(BASE_NAME)
BUILD_NUMBER     = $(BUILD_NUMBER)
VERSION_STRING   = $(VERSION_STRING)
TOOLCHAIN        = TOOLCHAIN_$(PROCESSOR)
FILE_GUID        = $(FILE_GUID)
COMPONENT_TYPE   = $(COMPONENT_TYPE)
INF_FILENAME     = $(INF_FILENAME)
PACKAGE_FILENAME = $(PACKAGE_FILENAME)
FV_DIR           = $(BUILD_DIR)\FV
PLATFORM         = $(PROJECT_NAME) 
#INC_DEPS         = $(INC_DEPS) $(BUILD_DIR)\PlatformTools.env
#INC_DEPS         = $(INC_DEPS) $(EDK_SOURCE)\Sample\CommonTools.env
#INC_DEPS         = $(INC_DEPS) $(EDK_SOURCE)\Sample\LocalTools.env

!IF "$(LANGUAGE)" != ""
LANGUAGE_FLAGS    = -lang $(LANGUAGE)
!ENDIF

!INCLUDE $(BUILD_DIR)\PlatformTools.env

!IF "$(COMPONENT_TYPE)" == "PIC_PEIM" || "$(COMPONENT_TYPE)" == "PE32_PEIM" || "$(COMPONENT_TYPE)" == "RELOCATABLE_PEIM" || "$(COMPONENT_TYPE)" == "COMBINED_PEIM_DRIVER"
DEPEX_TYPE = EFI_SECTION_PEI_DEPEX
!ELSE
DEPEX_TYPE = EFI_SECTION_DXE_DEPEX
!ENDIF


[=============================================================================]
#
# These are the commands to compile source files. One of these blocks gets 
# emitted to the component's makefile for each source file. The section
# name is encoded as [Compile.$(PROCESSOR).source_filename_extension], where
# the source filename comes from the sources section of the component INF file.
#
[=============================================================================]
[Compile.Ia32.asm]

$(DEST_DIR)\$(FILE).obj : $(SOURCE_DIR)\$(FILE).asm
  $(ASM) $(ASM_FLAGS) $**

[=============================================================================]
[Compile.Ipf.s]

$(DEST_DIR)\$(FILE).pro : $(SOURCE_DIR)\$(FILE).s $(INF_FILENAME)
 $(CC) $(C_FLAGS_PRO) $(SOURCE_DIR)\$(FILE).s > $@

$(DEST_DIR)\$(FILE).obj : $(DEST_DIR)\$(FILE).pro
 $(ASM) $(ASM_FLAGS) $(DEST_DIR)\$(FILE).pro

[=============================================================================]
[Compile.Ia32.c,Compile.Ipf.c]

#
# If it already exists, then include the dependency list file for this 
# source file. If it doesn't exist, then this is a clean build and the
# dependency file will get created below and the source file will get 
# compiled. Don't do any of this if NO_MAKEDEPS is defined.
#
!IF ("$(NO_MAKEDEPS)" == "")

!IF EXIST($(DEST_DIR)\$(FILE).dep)
!INCLUDE $(DEST_DIR)\$(FILE).dep
!ENDIF

#
# This is how to create the dependency file.
#
DEP_FILE = $(DEST_DIR)\$(FILE).dep

$(DEP_FILE) : $(SOURCE_FILE_NAME)
  $(MAKEDEPS) -ignorenotfound -f $(SOURCE_FILE_NAME) -q -target \
    $(DEST_DIR)\$(FILE).obj \
    -o $(DEP_FILE) $(INC)

!ENDIF

#
# Compile the file
#
$(DEST_DIR)\$(FILE).obj : $(SOURCE_FILE_NAME) $(INC_DEPS) $(DEP_FILE)
  $(CC) $(C_FLAGS) $(SOURCE_FILE_NAME)

[=============================================================================]
[Compile.Ebc.c]

#
# If it already exists, then include the dependency list file for this 
# source file. If it doesn't exist, then this is a clean build and the
# dependency file will get created below and the source file will get 
# compiled. Don't do any of this if NO_MAKEDEPS is defined.
#
!IF ("$(NO_MAKEDEPS)" == "")

!IF EXIST($(DEST_DIR)\$(FILE).dep)
!INCLUDE $(DEST_DIR)\$(FILE).dep
!ENDIF

#
# This is how to create the dependency file.
#
DEP_FILE = $(DEST_DIR)\$(FILE).dep

$(DEP_FILE) : $(SOURCE_FILE_NAME)
  $(MAKEDEPS) -ignorenotfound -f $(SOURCE_FILE_NAME) -q -target \
    $(DEST_DIR)\$(FILE).obj \
    -o $(DEP_FILE) $(INC)

!ENDIF

#
# Redefine the entry point function if an entry point has been defined at all.
# This is required because all EBC entry point functions must be called
# EfiMain.
#
!IF DEFINED(IMAGE_ENTRY_POINT)
!IF "$(IMAGE_ENTRY_POINT)" != "EfiMain"
EBC_C_FLAGS = $(EBC_C_FLAGS) /D $(IMAGE_ENTRY_POINT)=EfiMain
!ENDIF
!ENDIF

#
# This is how to compile the source .c file
# Use -P to get preprocessor output file (.i)
#
$(DEST_DIR)\$(FILE).obj : $(SOURCE_DIR)\$(FILE).c $(INF_FILENAME) $(DEP_FILE)
  $(EBC_CC) $(EBC_C_FLAGS) -X $(INC) -Fa$(DEST_DIR)\$(FILE).cod \
    $(SOURCE_DIR)\$(FILE).c -Fo$(DEST_DIR)\$(FILE).obj

[=============================================================================]
#
# Commands for compiling a ".apr" Apriori source file.
#
[=============================================================================]
[Compile.Ia32.Apr,Compile.Ipf.Apr,Compile.Ebc.Apr]

#
# Create the raw binary file. If you get an error on the build saying it doesn't
# know how to create the .apr file, then you're missing (or mispelled) the
# "APRIORI=" on the component lines in components section in the DSC file.
#
$(DEST_DIR)\$(BASE_NAME).bin : $(BUILD_DIR)\$(DSC_FILENAME)
  $(GENAPRIORI) -v -f $(FILE).apr -o $(DEST_DIR)\$(BASE_NAME).bin -i

$(DEST_DIR)\$(BASE_NAME).sec : $(DEST_DIR)\$(BASE_NAME).bin
  $(GENSECTION) -I $(DEST_DIR)\$(BASE_NAME).bin -O $(DEST_DIR)\$(BASE_NAME).sec -S EFI_SECTION_RAW

[=============================================================================]
[Build.Ia32.Apriori,Build.Ipf.Apriori,Build.Ebc.Apriori]

all : $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).FFS

#
# Run GenFfsFile on the package file and .raw file to create the firmware file
#
$(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).FFS : $(DEST_DIR)\$(BASE_NAME).sec
  $(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

[=============================================================================]
[Build.Ia32.Makefile,Build.Ipf.Makefile,Build.Ebc.Makefile]

#
# Copy the makefile directly from the source directory, then make it
# writable so we can copy over it later if we try to.
#
$(DEST_DIR)\makefile.new : $(SOURCE_DIR)\makefile.new
  copy $(SOURCE_DIR)\makefile.new $(DEST_DIR)\makefile.new
  attrib -r $(DEST_DIR)\makefile.new

#
# Make the all target, set some required macros.
#
call_makefile :
  $(MAKE) -f $(DEST_DIR)\makefile.new all   \
          SOURCE_DIR=$(SOURCE_DIR)          \
          BUILD_DIR=$(BUILD_DIR)            \
          FILE_GUID=$(FILE_GUID)            \
          DEST_DIR=$(DEST_DIR)              \
          FILE_GUID=$(FILE_GUID)            \
          PROCESSOR=$(PROCESSOR)            \
          TOOLCHAIN=TOOLCHAIN_$(PROCESSOR)  \
          BASE_NAME=$(BASE_NAME)            \
          PACKAGE_FILENAME=$(PACKAGE_FILENAME)

all : $(DEST_DIR)\makefile.new call_makefile

[=============================================================================]
#
# Instructions for building a component that uses a custom makefile. Encoding 
# is [build.$(PROCESSOR).$(BUILD_TYPE)].
#
# To build these components, simply call the makefile from the source 
# directory.
#
[Build.Ia32.Custom_Makefile,Build.Ipf.Custom_Makefile,Build.Ebc.Custom_Makefile]

#
# Just call the makefile from the source directory, passing in some
# useful info.
#
all : 
  $(MAKE) -f $(SOURCE_DIR)\makefile all    \
          SOURCE_DIR=$(SOURCE_DIR)         \
          BUILD_DIR=$(BUILD_DIR)           \
          DEST_DIR=$(DEST_DIR)             \
          FILE_GUID=$(FILE_GUID)           \
          PROCESSOR=$(PROCESSOR)           \
          TOOLCHAIN=TOOLCHAIN_$(PROCESSOR) \
          BASE_NAME=$(BASE_NAME)           \
          PLATFORM=$(PLATFORM)             \
          SOURCE_FV=$(SOURCE_FV)           \
          EFI_FVMAIN_COMPACT=$(EFI_FVMAIN_COMPACT) \
          PACKAGE_FILENAME=$(PACKAGE_FILENAME)

[=============================================================================]
#
# These commands are used to build libraries
#
[=============================================================================]
[Build.Ia32.LIBRARY,Build.Ipf.LIBRARY]
#
# LIB all the object files into to our target lib file. Put
# a dependency on the component's INF file in case it changes.
#
LIB_NAME = $(LIB_DIR)\$(BASE_NAME).lib

$(LIB_NAME) : $(OBJECTS) $(LIBS) $(INF_FILENAME)
  $(LIB) $(LIB_FLAGS) $(OBJECTS) $(LIBS) /OUT:$@

all: $(LIB_NAME)

[=============================================================================]
[Build.Ebc.Library]

$(LIB_DIR)\$(BASE_NAME).lib : $(OBJECTS) $(LIBS)
   $(EBC_LIB) $(EBC_LIB_FLAGS) $(OBJECTS) $(LIBS) /OUT:$(LIB_DIR)\$(BASE_NAME).lib

all : $(LIB_DIR)\$(BASE_NAME).lib

[=============================================================================]
#
# This is the Build.$(PROCESSOR).$(COMPONENT_TYPE) section that tells how to
# convert a firmware volume into an FV FFS file. Simply run it through
# GenFfsFile with the appropriate package file. SOURCE_FV must be defined
# in the component INF file Defines section.
#
[Build.Ia32.FvImageFile]

all : $(DEST_DIR)\$(FILE_GUID)-$(BASE_NAME).Fvi

#
# Run GenFfsFile on the package file and FV file to create the firmware 
# volume FFS file
#
$(DEST_DIR)\$(FILE_GUID)-$(BASE_NAME).Fvi : $(DEST_DIR)\$(SOURCE_FV)Fv.sec
  $(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

[=============================================================================]
#
# Since many of the steps are the same for the different component types, we 
# share this section for BS_DRIVER, RT_DRIVER, .... and IFDEF the parts that 
# differ.  The entire section gets dumped to the output makefile.
#
[=============================================================================]
[Build.Ia32.BS_DRIVER|RT_DRIVER|SAL_RT_DRIVER|PE32_PEIM|PEI_CORE|PIC_PEIM|RELOCATABLE_PEIM|DXE_CORE|APPLICATION|COMBINED_PEIM_DRIVER, Build.Ipf.BS_DRIVER|RT_DRIVER|SAL_RT_DRIVER|PEI_CORE|PE32_PEIM|PIC_PEIM|DXE_CORE|APPLICATION|COMBINED_PEIM_DRIVER]

!IF "$(LOCALIZE)" == "YES"

!IF "$(EFI_GENERATE_HII_EXPORT)" == "YES"
STRGATHER_FLAGS   = $(STRGATHER_FLAGS) -hpk $(DEST_DIR)\$(BASE_NAME)Strings.hpk

#
# There will be one HII pack containing all the strings. Add that file
# to the list of HII pack files we'll use to create our final HII export file.
#
HII_PACK_FILES    = $(HII_PACK_FILES) $(DEST_DIR)\$(BASE_NAME)Strings.hpk
LOCALIZE_TARGETS  = $(LOCALIZE_TARGETS) $(DEST_DIR)\$(BASE_NAME).hii
!ENDIF

$(DEST_DIR)\$(BASE_NAME).sdb : $(SDB_FILES) $(SOURCE_FILES)
  $(STRGATHER) -scan -vdbr $(STRGATHER_FLAGS) -od $(DEST_DIR)\$(BASE_NAME).sdb \
    -skipext .uni -skipext .h $(SOURCE_FILES)

$(DEST_DIR)\$(BASE_NAME)Strings.c $(DEST_DIR)\$(BASE_NAME)StrDefs.h $(DEST_DIR)\$(BASE_NAME)Strings.hpk : $(DEST_DIR)\$(BASE_NAME).sdb
  $(STRGATHER) -dump $(LANGUAGE_FLAGS) -bn $(BASE_NAME)Strings -db $(DEST_DIR)\$(BASE_NAME).sdb \
    -oc $(DEST_DIR)\$(BASE_NAME)Strings.c -hpk $(DEST_DIR)\$(BASE_NAME)Strings.hpk -oh $(DEST_DIR)\$(BASE_NAME)StrDefs.h

OBJECTS = $(OBJECTS) $(DEST_DIR)\$(BASE_NAME)Strings.obj

$(DEST_DIR)\$(BASE_NAME)Strings.obj : $(DEST_DIR)\$(BASE_NAME)Strings.c
  $(CC) $(C_FLAGS) $(DEST_DIR)\$(BASE_NAME)Strings.c

LOCALIZE_TARGETS = $(LOCALIZE_TARGETS) $(DEST_DIR)\$(BASE_NAME)StrDefs.h

!ENDIF

#
# If we have any objects associated with this component, then we're
# going to build a local library from them.
#
!IFNDEF OBJECTS
!ERROR No source files to build were defined in the INF file
!ENDIF

TARGET_LOCAL_LIB  = $(DEST_DIR)\$(BASE_NAME)Local.lib
BIN_TARGETS       = $(BIN_TARGETS) $(TARGET_LOCAL_LIB)

#
# LIB all the object files into our (local) target lib file. Put
# a dependency on the component's INF file in case it changes.
#
$(TARGET_LOCAL_LIB) : $(OBJECTS)  $(INF_FILENAME)
  $(LIB) $(LIB_FLAGS) $(OBJECTS) /OUT:$@

#
# Defines for standard intermediate files and build targets
#
TARGET_DLL      = $(BIN_DIR)\$(BASE_NAME).dll
TARGET_EFI      = $(BIN_DIR)\$(BASE_NAME).efi
TARGET_DPX      = $(DEST_DIR)\$(BASE_NAME).dpx
TARGET_UI       = $(DEST_DIR)\$(BASE_NAME).ui
TARGET_VER      = $(DEST_DIR)\$(BASE_NAME).ver
TARGET_MAP      = $(DEST_DIR)\$(BASE_NAME).map
TARGET_PDB      = $(EFI_SYMBOL_PATH)\$(BASE_NAME).pdb
TARGET_SYM      = $(BIN_DIR)\$(BASE_NAME).sym

#
# Target executable section extension depends on the component type.
# Only define "TARGET_DXE_DPX" if it's a combined peim driver.
#
!IF "$(COMPONENT_TYPE)" == "PIC_PEIM"
TARGET_PE32 = $(DEST_DIR)\$(BASE_NAME).pic
!ELSE
TARGET_PE32 = $(DEST_DIR)\$(BASE_NAME).pe32
!ENDIF

#
# Target FFS file extension depends on the component type
# Also define "TARGET_DXE_DPX" if it's a combined PEIM driver.
#
SUBSYSTEM = EFI_BOOT_SERVICE_DRIVER

!IF "$(COMPONENT_TYPE)" == "APPLICATION"
TARGET_FFS_FILE = $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).app
SUBSYSTEM       = EFI_APPLICATION
!ELSE IF "$(COMPONENT_TYPE)" == "PEI_CORE"
TARGET_FFS_FILE = $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).pei
!ELSE IF "$(COMPONENT_TYPE)" == "PE32_PEIM"
TARGET_FFS_FILE = $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).pei
!ELSE IF "$(COMPONENT_TYPE)" == "RELOCATABLE_PEIM"
TARGET_FFS_FILE = $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).pei
!ELSE IF "$(COMPONENT_TYPE)" == "PIC_PEIM"
TARGET_FFS_FILE = $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).pei
!ELSE IF "$(COMPONENT_TYPE)" == "COMBINED_PEIM_DRIVER"
TARGET_FFS_FILE = $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).pei
TARGET_DXE_DPX  = $(DEST_DIR)\$(BASE_NAME).dpxd
!ELSE
TARGET_FFS_FILE = $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).dxe
!ENDIF

#
# Build a FFS file from the sections and package
#
$(TARGET_FFS_FILE) : $(TARGET_PE32) $(TARGET_DPX) $(TARGET_UI) $(TARGET_VER) $(TARGET_DXE_DPX)
#
# Some of our components require padding to align code
#
!IF "$(PROCESSOR)" == "IPF"
!IF "$(COMPONENT_TYPE)" == "PIC_PEIM" || "$(COMPONENT_TYPE)" == "PE32_PEIM" || "$(COMPONENT_TYPE)" == "RELOCATABLE_PEIM" || "$(COMPONENT_TYPE)" == "PEI_CORE" || "$(COMPONENT_TYPE)" == "COMBINED_PEIM_DRIVER"
  copy $(BIN_DIR)\Blank.pad $(DEST_DIR)
!ENDIF
!ENDIF
  $(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

#
# Different methods to build section based on if PIC_PEIM
#
!IF "$(COMPONENT_TYPE)" == "PIC_PEIM"

$(TARGET_PE32) : $(TARGET_DLL)
  $(PE2BIN) $(TARGET_DLL) $(DEST_DIR)\$(BASE_NAME).TMP
#
# BUGBUG: Build PEIM header, needs to go away with new PEI.
#
  $(TEMPGENSECTION) -P $(SOURCE_DIR)\$(BASE_NAME).INF -I $(DEST_DIR)\$(BASE_NAME).TMP -O $(TARGET_PIC_PEI).tmp -M $(TARGET_MAP) -S EFI_SECTION_TYPE_NO_HEADER
  $(GENSECTION) -I $(TARGET_PIC_PEI).tmp -O $(TARGET_PE32) -S EFI_SECTION_PIC
  del $(DEST_DIR)\$(BASE_NAME).TMP

!ELSE

$(TARGET_PE32) : $(TARGET_EFI) $(INF_FILENAME)
  $(GENSECTION) -I $(TARGET_EFI) -O $(TARGET_PE32) -S EFI_SECTION_PE32

#
# BUGBUG: This step is obsolete when a linker is released that supports EFI.
#
$(TARGET_EFI) : $(TARGET_DLL)
  $(FWIMAGE) -t 0 $(COMPONENT_TYPE) $(TARGET_DLL) $(TARGET_EFI)

!ENDIF

#
# Link all objects and libs to create the executable
#
$(TARGET_DLL) : $(TARGET_LOCAL_LIB) $(LIBS)
  $(LINK) $(LINK_FLAGS_DLL) $(LIBS) /ENTRY:$(IMAGE_ENTRY_POINT) \
     $(TARGET_LOCAL_LIB) /OUT:$(TARGET_DLL) /MAP:$(TARGET_MAP) \
     /PDB:$(TARGET_PDB) 
  $(SETSTAMP) $(TARGET_DLL) $(BUILD_DIR)\GenStamp.txt
!IF "$(EFI_GENERATE_SYM_FILE)" == "YES"
  if exist $(TARGET_PDB) $(PE2SYM) $(TARGET_PDB) $(TARGET_SYM)
!ENDIF

!IF "$(EFI_ZERO_DEBUG_DATA)" == "YES"
  $(ZERODEBUGDATA) $(TARGET_DLL)
!ENDIF

#
# Create the user interface section
#
$(TARGET_UI) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_UI) -S EFI_SECTION_USER_INTERFACE -A "$(BASE_NAME)"

#
# Create the version section
#
!IF "$(BUILD_NUMBER)" != ""
!IF "$(VERSION_STRING)" != ""
$(TARGET_VER) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER) -A "$(VERSION_STRING)"
!ELSE
$(TARGET_VER) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER)
!ENDIF
!ELSE
$(TARGET_VER) : $(INF_FILENAME)
  echo.>$(TARGET_VER)
  type $(TARGET_VER)>$(TARGET_VER)
!ENDIF

#
# Makefile entries to create the dependency expression section.
# Use the DPX file from the source directory unless an override file
# was specified.
# If no DPX source file was specified, then create an empty file to
# be used.
#
!IF "$(DPX_SOURCE)" != ""
DPX_SOURCE_FILE = $(SOURCE_DIR)\$(DPX_SOURCE)
!ENDIF

!IF "$(DPX_SOURCE_OVERRIDE)" != ""
DPX_SOURCE_FILE = $(DPX_SOURCE_OVERRIDE)
!ENDIF

!IF "$(DPX_SOURCE_FILE)" != ""
!IF EXIST ($(DPX_SOURCE_FILE))
$(TARGET_DPX) : $(DPX_SOURCE_FILE) $(INF_FILENAME)
  $(CC) $(INC) /EP $(DPX_SOURCE_FILE) > $*.tmp1
  $(GENDEPEX) -I $*.tmp1 -O $*.tmp2
  $(GENSECTION) -I $*.tmp2 -O $@ -S $(DEPEX_TYPE)
  del $*.tmp1 > NUL
  del $*.tmp2 > NUL
!ELSE
!ERROR Dependency expression source file "$(DPX_SOURCE_FILE)" does not exist.
!ENDIF
!ELSE
$(TARGET_DPX) : $(INF_FILENAME)
  echo. > $(TARGET_DPX)
  type $(TARGET_DPX) > $(TARGET_DPX)
!ENDIF

#
# Makefile entries for DXE DPX for combined PEIM drivers.
# If a DXE_DPX_SOURCE file was specified in the INF file, use it. Otherwise 
# create an empty file and use it as a DPX file.
#
!IF "$(COMPONENT_TYPE)" == "COMBINED_PEIM_DRIVER"
!IF "$(DXE_DPX_SOURCE)" != ""
!IF EXIST ($(SOURCE_DIR)\$(DPX_SOURCE))
$(TARGET_DXE_DPX) : $(SOURCE_DIR)\$(DXE_DPX_SOURCE) $(INF_FILENAME)
  $(CC) $(INC) /EP $(SOURCE_DIR)\$(DXE_DPX_SOURCE) > $*.tmp1
  $(GENDEPEX) -I $*.tmp1 -O $*.tmp2
  $(GENSECTION) -I $*.tmp2 -O $@ -S EFI_SECTION_DXE_DEPEX
  del $*.tmp1 > NUL
  del $*.tmp2 > NUL
!ELSE
!ERROR Dependency expression source file "$(SOURCE_DIR)\$(DXE_DPX_SOURCE)" does not exist.
!ENDIF
!ELSE
$(TARGET_DXE_DPX) : $(INF_FILENAME)
  echo. > $(TARGET_DXE_DPX)
  type $(TARGET_DXE_DPX) > $(TARGET_DXE_DPX)
!ENDIF
!ENDIF

#
# Describe how to build the HII export file from all the input HII pack files.
# Use the FFS file GUID for the package GUID in the export file. Only used
# when multiple VFR share strings.
#
$(DEST_DIR)\$(BASE_NAME).hii : $(HII_PACK_FILES)
  $(HIIPACK) create -g $(FILE_GUID) -p $(HII_PACK_FILES) -o $(DEST_DIR)\$(BASE_NAME).hii

#
# If the build calls for creating an FFS file with the IFR included as
# a separate binary (not compiled into the driver), then build the binary
# section now. Note that the PACKAGE must be set correctly to actually get
# this IFR section pulled into the FFS file.
#
!IF ("$(HII_IFR_PACK_FILES)" != "")

$(DEST_DIR)\$(BASE_NAME)IfrBin.sec : $(HII_IFR_PACK_FILES)
  $(HIIPACK) create -novarpacks -p $(HII_IFR_PACK_FILES) -o $(DEST_DIR)\$(BASE_NAME)IfrBin.hii
  $(GENSECTION) -I $(DEST_DIR)\$(BASE_NAME)IfrBin.hii -O $(DEST_DIR)\$(BASE_NAME)IfrBin.sec -S EFI_SECTION_RAW

BIN_TARGETS = $(BIN_TARGETS) $(DEST_DIR)\$(BASE_NAME)IfrBin.sec

!ENDIF

all: $(LOCALIZE_TARGETS) $(BIN_TARGETS) $(TARGET_FFS_FILE)

[=============================================================================]
[Build.Ia32.TE_PEIM,Build.Ipf.TE_PEIM]
#
# Define the library file we'll build if we have any objects defined.
#
!IFDEF OBJECTS
TARGET_LOCAL_LIB  = $(DEST_DIR)\$(BASE_NAME)Local.lib
BIN_TARGETS       = $(BIN_TARGETS) $(TARGET_LOCAL_LIB)
#
# LIB all the object files into our (local) target lib file. Put
# a dependency on the component's INF file in case it changes.
#
$(TARGET_LOCAL_LIB) : $(OBJECTS)  $(INF_FILENAME)
  $(LIB) $(LIB_FLAGS) $(OBJECTS) /OUT:$@

!ELSE
!ERROR No source files to build were defined in the INF file
!ENDIF

#
# Defines for standard intermediate files and build targets
#
TARGET_DLL        = $(BIN_DIR)\$(BASE_NAME).dll
TARGET_EFI        = $(BIN_DIR)\$(BASE_NAME).efi
TARGET_DPX        = $(DEST_DIR)\$(BASE_NAME).dpx
TARGET_UI         = $(DEST_DIR)\$(BASE_NAME).ui
TARGET_VER        = $(DEST_DIR)\$(BASE_NAME).ver
TARGET_MAP        = $(DEST_DIR)\$(BASE_NAME).map
TARGET_PDB        = $(EFI_SYMBOL_PATH)\$(BASE_NAME).pdb
TARGET_SYM        = $(BIN_DIR)\$(BASE_NAME).sym
TARGET_TE         = $(BIN_DIR)\$(BASE_NAME).te
TARGET_PE32       = $(DEST_DIR)\$(BASE_NAME).pe32
TARGET_TES        = $(DEST_DIR)\$(BASE_NAME).tes
TARGET_FFS_FILE   = $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).pei

#
# Build an FFS file from the sections and package
#
$(TARGET_FFS_FILE) : $(TARGET_TES) $(TARGET_DPX) $(TARGET_UI) $(TARGET_VER)
  $(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

#
# Create our TE section from our TE file
#
$(TARGET_TES) : $(TARGET_TE) $(INF_FILENAME)
  $(GENSECTION) -I $(TARGET_TE) -O $(TARGET_TES) -S EFI_SECTION_TE

#
# Run FWImage on the DLL to set it as an EFI image type.
# BUGBUG: This step is obsolete when a linker is released that supports EFI.
#
$(TARGET_EFI) : $(TARGET_DLL)
  $(FWIMAGE) $(COMPONENT_TYPE) $(TARGET_DLL) $(TARGET_EFI)

#
# Run GenTEImage on the built .efi file to create our TE file.
#
$(TARGET_TE) : $(TARGET_EFI) 
  $(GENTEIMAGE) -o $(TARGET_TE) $(TARGET_EFI)

#
# Link all objects and libs to create the executable
#
$(TARGET_DLL) : $(TARGET_LOCAL_LIB) $(LIBS)
  $(LINK) $(LINK_FLAGS_DLL) $(LIBS) /ENTRY:$(IMAGE_ENTRY_POINT) \
     $(TARGET_LOCAL_LIB) /OUT:$(TARGET_DLL) /MAP:$(TARGET_MAP) \
     /PDB:$(TARGET_PDB) /SUBSYSTEM:EFI_BOOT_SERVICE_DRIVER
  $(SETSTAMP) $(TARGET_DLL) $(BUILD_DIR)\GenStamp.txt
!IF "$(EFI_GENERATE_SYM_FILE)" == "YES"
  if exist $(TARGET_PDB) $(PE2SYM) $(TARGET_PDB) $(TARGET_SYM)
!ENDIF

!IF "$(EFI_ZERO_DEBUG_DATA)" == "YES"
  $(ZERODEBUGDATA) $(TARGET_DLL)
!ENDIF

#
# Create the user interface section
#
$(TARGET_UI) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_UI) -S EFI_SECTION_USER_INTERFACE -A "$(BASE_NAME)"

#
# Create the version section
#
!IF "$(BUILD_NUMBER)" != ""
!IF "$(VERSION_STRING)" != ""
$(TARGET_VER) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER) -A "$(VERSION_STRING)"
!ELSE
$(TARGET_VER) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER)
!ENDIF
!ELSE
$(TARGET_VER) : $(INF_FILENAME)
  echo.>$(TARGET_VER)
  type $(TARGET_VER)>$(TARGET_VER)
!ENDIF

#
# Makefile entries to create the dependency expression section.
# Use the DPX file from the source directory unless an override file
# was specified.
# If no DPX source file was specified, then create an empty file to
# be used.
#
!IF "$(DPX_SOURCE)" != ""
DPX_SOURCE_FILE = $(SOURCE_DIR)\$(DPX_SOURCE)
!ENDIF

!IF "$(DPX_SOURCE_OVERRIDE)" != ""
DPX_SOURCE_FILE = $(DPX_SOURCE_OVERRIDE)
!ENDIF

!IF "$(DPX_SOURCE_FILE)" != ""
!IF EXIST ($(DPX_SOURCE_FILE))
$(TARGET_DPX) : $(DPX_SOURCE_FILE) $(INF_FILENAME)
  $(CC) $(INC) /EP $(DPX_SOURCE_FILE) > $*.tmp1
  $(GENDEPEX) -I $*.tmp1 -O $*.tmp2
  $(GENSECTION) -I $*.tmp2 -O $@ -S $(DEPEX_TYPE)
  del $*.tmp1 > NUL
  del $*.tmp2 > NUL
!ELSE
!ERROR Dependency expression source file "$(DPX_SOURCE_FILE)" does not exist.
!ENDIF
!ELSE
$(TARGET_DPX) : $(INF_FILENAME)
  echo. > $(TARGET_DPX)
  type $(TARGET_DPX) > $(TARGET_DPX)
!ENDIF

all: $(BIN_TARGETS) $(TARGET_FFS_FILE)

[=============================================================================]
#
# These are the commands to build EBC EFI targets
#
[=============================================================================]
[Build.Ebc.BS_DRIVER|APPLICATION]

#
# Add the EBC library to our list of libs
#
LIBS = $(LIBS) $(EBC_TOOLS_PATH)\lib\EbcLib.lib 

!IF "$(LOCALIZE)" == "YES"

!IF "$(EFI_GENERATE_HII_EXPORT)" == "YES"
STRGATHER_FLAGS   = $(STRGATHER_FLAGS) -hpk $(DEST_DIR)\$(BASE_NAME)Strings.hpk

#
# There will be one HII pack containing all the strings. Add that file
# to the list of HII pack files we'll use to create our final HII export file.
#
HII_PACK_FILES = $(HII_PACK_FILES) $(DEST_DIR)\$(BASE_NAME)Strings.hpk

LOCALIZE_TARGETS  = $(LOCALIZE_TARGETS) $(DEST_DIR)\$(BASE_NAME).hii
!ENDIF

$(DEST_DIR)\$(BASE_NAME).sdb : $(SDB_FILES) $(SOURCE_FILES)
  $(STRGATHER) -scan -vdbr $(STRGATHER_FLAGS) -od $(DEST_DIR)\$(BASE_NAME).sdb \
    -skipext .uni -skipext .h $(SOURCE_FILES)

$(DEST_DIR)\$(BASE_NAME)Strings.c $(DEST_DIR)\$(BASE_NAME)StrDefs.h $(DEST_DIR)\$(BASE_NAME)Strings.hpk : $(DEST_DIR)\$(BASE_NAME).sdb
  $(STRGATHER) -dump $(LANGUAGE_FLAGS) -bn $(BASE_NAME)Strings -db $(DEST_DIR)\$(BASE_NAME).sdb \
    -oc $(DEST_DIR)\$(BASE_NAME)Strings.c -hpk $(DEST_DIR)\$(BASE_NAME)Strings.hpk -oh $(DEST_DIR)\$(BASE_NAME)StrDefs.h

OBJECTS = $(OBJECTS) $(DEST_DIR)\$(BASE_NAME)Strings.obj

$(DEST_DIR)\$(BASE_NAME)Strings.obj : $(DEST_DIR)\$(BASE_NAME)Strings.c
  $(CC) $(C_FLAGS) $(DEST_DIR)\$(BASE_NAME)Strings.c

LOCALIZE_TARGETS = $(LOCALIZE_TARGETS) $(DEST_DIR)\$(BASE_NAME)StrDefs.h

!ENDIF

#
# If building an application, then the target is a .app, not .dxe
#
!IF "$(COMPONENT_TYPE)" == "APPLICATION"
TARGET_FFS_FILE = $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).app
SUBSYSTEM       = EFI_APPLICATION
!ELSE
TARGET_FFS_FILE = $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).dxe
SUBSYSTEM       = EFI_BOOT_SERVICE_DRIVER
!ENDIF

#
# Defines for standard intermediate files and build targets
#
TARGET_EFI  = $(BIN_DIR)\$(BASE_NAME).efi
TARGET_DPX  = $(DEST_DIR)\$(BASE_NAME).dpx
TARGET_UI   = $(DEST_DIR)\$(BASE_NAME).ui
TARGET_VER  = $(DEST_DIR)\$(BASE_NAME).ver
TARGET_MAP  = $(DEST_DIR)\$(BASE_NAME).map
TARGET_PDB  = $(EFI_SYMBOL_PATH)\$(BASE_NAME).pdb
TARGET_PE32 = $(DEST_DIR)\$(BASE_NAME).pe32
TARGET_DLL  = $(BIN_DIR)\$(BASE_NAME).dll

#
# First link all the objects and libs together to make a .dll file
#
$(TARGET_DLL) : $(OBJECTS) $(LIBS)
  $(EBC_LINK) $(EBC_LINK_FLAGS) /SUBSYSTEM:$(SUBSYSTEM) /ENTRY:EfiStart \
    $(OBJECTS) $(LIBS) /OUT:$(TARGET_DLL)
  $(SETSTAMP) $(TARGET_DLL) $(BUILD_DIR)\GenStamp.txt
!IF "$(EFI_ZERO_DEBUG_DATA)" == "YES"
  $(ZERODEBUGDATA) $(TARGET_DLL)
!ENDIF

#
# Now take the .dll file and make a .efi file
#
$(TARGET_EFI) : $(TARGET_DLL)
  $(FWIMAGE) -t 0 $(COMPONENT_TYPE) $(TARGET_DLL) $(TARGET_EFI)

#
# Now take the .efi file and make a .pe32 section
#
$(TARGET_PE32) : $(TARGET_EFI) $(INF_FILENAME)
  $(GENSECTION) -I $(TARGET_EFI) -O $(TARGET_PE32) -S EFI_SECTION_PE32

#
# Create the user interface section
#
$(TARGET_UI) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_UI) -S EFI_SECTION_USER_INTERFACE -A "$(BASE_NAME)"

#
# Create the version section
#
!IF "$(BUILD_NUMBER)" != ""
!IF "$(VERSION_STRING)" != ""
$(TARGET_VER) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER) -A "$(VERSION_STRING)"
!ELSE
$(TARGET_VER) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER)
!ENDIF
!ELSE
$(TARGET_VER) : $(INF_FILENAME)
  echo. > $(TARGET_VER)
  type $(TARGET_VER) > $(TARGET_VER)
!ENDIF

#
# Makefile entries to create the dependency expression section.
# Use the DPX file from the source directory unless an override file
# was specified.
# If no DPX source file was specified, then create an empty file to
# be used.
#
!IF "$(DPX_SOURCE)" != ""
DPX_SOURCE_FILE = $(SOURCE_DIR)\$(DPX_SOURCE)
!ENDIF

!IF "$(DPX_SOURCE_OVERRIDE)" != ""
DPX_SOURCE_FILE = $(DPX_SOURCE_OVERRIDE)
!ENDIF

!IF "$(DPX_SOURCE_FILE)" != ""
!IF EXIST ($(DPX_SOURCE_FILE))
$(TARGET_DPX) : $(DPX_SOURCE_FILE) $(INF_FILENAME)
  $(CC) $(INC) /EP $(DPX_SOURCE_FILE) > $*.tmp1
  $(GENDEPEX) -I $*.tmp1 -O $*.tmp2
  $(GENSECTION) -I $*.tmp2 -O $@ -S $(DEPEX_TYPE)
  del $*.tmp1 > NUL
  del $*.tmp2 > NUL
!ELSE
!ERROR Dependency expression source file "$(DPX_SOURCE_FILE)" does not exist.
!ENDIF
!ELSE
$(TARGET_DPX) : $(INF_FILENAME)
  echo. > $(TARGET_DPX)
  type $(TARGET_DPX) > $(TARGET_DPX)
!ENDIF

#
# Build an FFS file from the sections and package
#
$(TARGET_FFS_FILE) : $(TARGET_PE32) $(TARGET_DPX) $(TARGET_UI) $(TARGET_VER)
  $(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

all: $(LOCALIZE_TARGETS) $(TARGET_FFS_FILE)

[=============================================================================]
#
# These are the commands to build vendor-provided *.EFI files into an FV.
# To use them, create an INF file with BUILD_TYPE=BS_DRIVER_EFI.
# This section, as it now exists, only supports boot service drivers.
#
[=============================================================================]
[Build.Ia32.BS_DRIVER_EFI|RT_DRIVER_EFI|APPLICATION_EFI,Build.Ebc.BS_DRIVER_EFI|RT_DRIVER_EFI|APPLICATION_EFI]
#
# Defines for standard intermediate files and build targets. For the source
# .efi file, take the one in the source directory if it exists. If there's not
# one there, look for one in the processor-specfic subdirectory.
#
!IF EXIST ("$(SOURCE_DIR)\$(BASE_NAME).efi")
TARGET_EFI        = $(SOURCE_DIR)\$(BASE_NAME).efi
!ELSEIF EXIST ("$(SOURCE_DIR)\$(PROCESSOR)\$(BASE_NAME).efi")
TARGET_EFI        = $(SOURCE_DIR)\$(PROCESSOR)\$(BASE_NAME).efi
!ELSE
!ERROR Pre-existing $(BASE_NAME).efi file not found in $(SOURCE_DIR) nor $(SOURCE_DIR)\$(PROCESSOR)
!ENDIF

TARGET_FFS_FILE   = $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME).dxe
TARGET_DPX        = $(DEST_DIR)\$(BASE_NAME).dpx
TARGET_UI         = $(DEST_DIR)\$(BASE_NAME).ui
TARGET_VER        = $(DEST_DIR)\$(BASE_NAME).ver
TARGET_MAP        = $(DEST_DIR)\$(BASE_NAME).map
TARGET_PDB        = $(EFI_SYMBOL_PATH)\$(BASE_NAME).pdb
TARGET_PE32       = $(DEST_DIR)\$(BASE_NAME).pe32
TARGET_DLL        = $(BIN_DIR)\$(BASE_NAME).dll

#
# Take the .efi file and make a .pe32 file
#
$(TARGET_PE32) : $(TARGET_EFI) $(INF_FILENAME)
  $(GENSECTION) -I $(TARGET_EFI) -O $(TARGET_PE32) -S EFI_SECTION_PE32

#
# Create the user interface section
#
$(TARGET_UI) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_UI) -S EFI_SECTION_USER_INTERFACE -A "$(BASE_NAME)"

#
# Create the version section
#
!IF "$(BUILD_NUMBER)" != ""
!IF "$(VERSION_STRING)" != ""
$(TARGET_VER) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER) -A "$(VERSION_STRING)"
!ELSE
$(TARGET_VER) : $(INF_FILENAME)
  $(GENSECTION) -O $(TARGET_VER) -S EFI_SECTION_VERSION -V $(BUILD_NUMBER)
!ENDIF
!ELSE
$(TARGET_VER) : $(INF_FILENAME)
  echo. > $(TARGET_VER)
  type $(TARGET_VER) > $(TARGET_VER)
!ENDIF

#
# Makefile entries to create the dependency expression section.
# Use the DPX file from the source directory unless an override file
# was specified.
# If no DPX source file was specified, then create an empty file to
# be used.
#
!IF "$(DPX_SOURCE)" != ""
DPX_SOURCE_FILE = $(SOURCE_DIR)\$(DPX_SOURCE)
!ENDIF

!IF "$(DPX_SOURCE_OVERRIDE)" != ""
DPX_SOURCE_FILE = $(DPX_SOURCE_OVERRIDE)
!ENDIF

!IF "$(DPX_SOURCE_FILE)" != ""
!IF EXIST ($(DPX_SOURCE_FILE))
$(TARGET_DPX) : $(DPX_SOURCE_FILE) $(INF_FILENAME)
  $(CC) $(INC) /EP $(DPX_SOURCE_FILE) > $*.tmp1
  $(GENDEPEX) -I $*.tmp1 -O $*.tmp2
  $(GENSECTION) -I $*.tmp2 -O $@ -S $(DEPEX_TYPE)
  del $*.tmp1 > NUL
  del $*.tmp2 > NUL
!ELSE
!ERROR Dependency expression source file "$(DPX_SOURCE_FILE)" does not exist.
!ENDIF
!ELSE
$(TARGET_DPX) : $(INF_FILENAME)
  echo. > $(TARGET_DPX)
  type $(TARGET_DPX) > $(TARGET_DPX)
!ENDIF

#
# Build a FFS file from the sections and package
#
$(TARGET_FFS_FILE) : $(TARGET_PE32) $(TARGET_DPX) $(TARGET_UI) $(TARGET_VER)
  $(GENFFSFILE) -B $(DEST_DIR) -P1 $(PACKAGE_FILENAME) -V

all: $(TARGET_FFS_FILE)

[=============================================================================]
#
# These commands are used to build EFI shell applications. That's all
# we need to do for these is link the objects and libraries together
# to create a .EFI file. See the example INF file in the shell\ls
# directory for a template INF file. You may also need to build the 
# shell library below via shell\lib\shelllib.inf.
#
[=============================================================================]
[Build.Ia32.SHELLAPP,Build.Ipf.SHELLAPP]

TARGET_DLL = $(BIN_DIR)\$(BASE_NAME).dll
TARGET_EFI = $(BIN_DIR)\$(BASE_NAME).efi
TARGET_PDB = $(EFI_SYMBOL_PATH)\$(BASE_NAME).pdb
TARGET_MAP = $(BIN_DIR)\$(BASE_NAME).map
SUBSYSTEM  = EFI_APPLICATION

#
# Link all the object files and library files together to create our
# final target.
#
$(TARGET_DLL) : $(OBJECTS) $(LIBS)
  $(LINK) $(LINK_FLAGS_DLL) $(OBJECTS) $(LIBS) /ENTRY:$(IMAGE_ENTRY_POINT) \
     /OUT:$(TARGET_DLL) /MAP:$(TARGET_MAP) /PDB:$(TARGET_PDB) /SUBSYSTEM:EFI_APPLICATION

$(TARGET_EFI) : $(TARGET_DLL)
  $(FWIMAGE) -t 0 $(COMPONENT_TYPE) $(TARGET_DLL) $(TARGET_EFI)
  $(SETSTAMP) $(TARGET_EFI) $(BUILD_DIR)\GenStamp.txt
!IF "$(EFI_ZERO_DEBUG_DATA)" == "YES"
  $(ZERODEBUGDATA) $(TARGET_DLL)
!ENDIF

all: $(TARGET_EFI)

[=============================================================================]
[Compile.Ia32.Bin,Build.Ipf.Bin]
#
# We simply copy the 16 bit binary file from the source directory to the destination directory
#
$(DEST_DIR)\$(BASE_NAME).bin : $(SOURCE_DIR)\$(BASE_NAME).bin
  copy $** $@

[=============================================================================]
[Compile.Ia32.Bmp,Build.Ipf.Bmp]
#
# We simply copy the BMP file from the source directory to the destination directory and change the extension to bin.
# This is so that we can build BINARY types the same way, with the same default package, etc.
#
$(DEST_DIR)\$(BASE_NAME).bin : $(SOURCE_DIR)\$(BASE_NAME).bmp
  copy $** $@

[=============================================================================]
[Build.Ia32.BINARY,Build.Ipf.BINARY]
#
#
# Use GenFfsFile to convert it to an FFS file
#
$(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME)$(FFS_EXT) : $(DEST_DIR)\$(BASE_NAME).bin
  $(GENSECTION) -I $(DEST_DIR)\$(BASE_NAME).bin -O $(DEST_DIR)\$(BASE_NAME).sec -S EFI_SECTION_RAW
  $(GENFFSFILE) -B $(BIN_DIR) -P1 $(DEST_DIR)\$(BASE_NAME).pkg -V

all: $(BIN_DIR)\$(FILE_GUID)-$(BASE_NAME)$(FFS_EXT)

[=============================================================================]
# 
# These are commands to compile unicode .uni files.
# Emit an error message if the file's base name is the same as the
# component base name. This causes build issues.
#
[Compile.Ia32.Uni,Compile.Ipf.Uni,Compile.Ebc.Uni]

!IF "$(FILE)" == "$(BASE_NAME)"
!ERROR Component Unicode string file name cannot be the same as the component BASE_NAME.
!ENDIF

$(DEST_DIR)\$(FILE).sdb : $(SOURCE_FILE_NAME)
  $(STRGATHER) -parse -newdb -db $(DEST_DIR)\$(FILE).sdb $(INC) $(SOURCE_FILE_NAME)

SDB_FILES       = $(SDB_FILES) $(DEST_DIR)\$(FILE).sdb
STRGATHER_FLAGS = $(STRGATHER_FLAGS) -db $(DEST_DIR)\$(FILE).sdb
LOCALIZE        = YES

[=============================================================================]
[Compile.Ia32.Vfr,Compile.Ipf.Vfr,Compile.Ebc.Vfr]

HII_PACK_FILES  = $(HII_PACK_FILES) $(DEST_DIR)\$(FILE).hpk

#
# Add a dummy command for building the HII pack file. In reality, it's built 
# below, but the C_FLAGS macro reference the target as $@, so you can't specify
# the obj and hpk files as dual targets of the same command.
#
$(DEST_DIR)\$(FILE).hpk : $(DEST_DIR)\$(FILE).obj
  @echo.
  
$(DEST_DIR)\$(FILE).obj : $(SOURCE_DIR)\$(FILE).vfr $(INC_DEPS) $(DEST_DIR)\$(BASE_NAME)StrDefs.h
  $(VFRCOMPILE) $(VFRCOMPILE_FLAGS) $(INC) -ibin -od $(DEST_DIR)\$(SOURCE_RELATIVE_PATH) \
    -l $(VFR_FLAGS) $(SOURCE_DIR)\$(FILE).vfr
  $(CC) $(C_FLAGS) $(DEST_DIR)\$(FILE).c

[=============================================================================]
#
# Commands for building IFR as uncompressed binary into the FFS file. To 
# use it, set COMPILE_SELECT=.vfr=Ifr_Bin for the component in the DSC file.
#
[Compile.Ia32.Ifr_Bin,Compile.Ipf.Ifr_Bin;Compile.Ebc.Ifr_Bin]

HII_PACK_FILES  = $(HII_PACK_FILES) $(DEST_DIR)\$(FILE).hpk

#
# Add a dummy command for building the HII pack file. In reality, it's built 
# below, but the C_FLAGS macro reference the target as $@, so you can't specify
# the obj and hpk files as dual targets of the same command.
#
$(DEST_DIR)\$(FILE).hpk : $(DEST_DIR)\$(FILE).obj
  @echo.
  
$(DEST_DIR)\$(FILE).obj : $(SOURCE_DIR)\$(FILE).vfr $(INC_DEPS) $(DEST_DIR)\$(BASE_NAME)StrDefs.h
  $(VFRCOMPILE) $(VFRCOMPILE_FLAGS) $(INC) -ibin -od $(DEST_DIR)\$(SOURCE_RELATIVE_PATH) \
    -l $(VFR_FLAGS) $(SOURCE_DIR)\$(FILE).vfr
  $(CC) $(C_FLAGS) $(DEST_DIR)\$(FILE).c

#
# Add to the variable that contains the list of VFR binary files we're going
# to merge together at the end of the build. 
#
HII_IFR_PACK_FILES = $(HII_IFR_PACK_FILES) $(DEST_DIR)\$(FILE).hpk

[=============================================================================]
[Compile.Ia32.Fv,Compile.Ipf.Fv]
#
# Run GenSection on the firmware volume image.
#
$(DEST_DIR)\$(SOURCE_FV)Fv.sec : $(FV_DIR)\$(SOURCE_FV).fv
  copy $(FV_DIR)\$(SOURCE_FV).fv $(DEST_DIR)\$(SOURCE_FV).fv /y
  $(GENSECTION) -I $(DEST_DIR)\$(SOURCE_FV).fv -O $(DEST_DIR)\$(SOURCE_FV)Fv.sec -S EFI_SECTION_FIRMWARE_VOLUME_IMAGE

[=============================================================================]
#
# These are the package descriptions. They are tagged as
# [Package.$(COMPONENT_TYPE).$(PACKAGE)], where COMPONENT_TYPE is typically
# defined in the component INF file, and PACKAGE is typically specified
# in the components section in the main DSC file.
#

[=============================================================================]
[Package.APPLICATION.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_APPLICATION
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{
  Compress (Dummy) {
    Tool (
      $(OEMTOOLPATH)\GenCRC32Section
      ARGS= -i $(DEST_DIR)\$(BASE_NAME).pe32
               $(DEST_DIR)\$(BASE_NAME).ui
               $(DEST_DIR)\$(BASE_NAME).ver
            -o $(DEST_DIR)\$(BASE_NAME).crc32
      OUTPUT = $(DEST_DIR)\$(BASE_NAME).crc32
    )
  }
}

[=============================================================================]
[Package.FILE.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_FREEFORM
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{ 
  $(BASE_NAME).sec 
}
[=============================================================================]
[Package.Apriori.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_FREEFORM
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{ 
  $(DEST_DIR)\$(BASE_NAME).sec 
}

[=============================================================================]
[Package.Logo.Logo,Package.Logo.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_FREEFORM
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{
  Compress (Dummy) {
    Tool ( $(OEMTOOLPATH)\GenCRC32Section
      ARGS = -i $(BIN_DIR)\$(BASE_NAME).sec
             -o $(BIN_DIR)\$(BASE_NAME).crc32
      OUTPUT = $(BIN_DIR)\$(BASE_NAME).crc32
    )
  }
}

[=============================================================================]
[Package.RAWFILE.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_RAW
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{
  $(DEST_DIR)\$(BASE_NAME).FV
}

[=============================================================================]
[Package.Legacy16.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_FREEFORM
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{
  Compress (Dummy) {
    Tool ( $(OEMTOOLPATH)\GenCRC32Section
      ARGS = -i $(BIN_DIR)\$(BASE_NAME).sec
             -o $(BIN_DIR)\$(BASE_NAME).crc32
      OUTPUT = $(BIN_DIR)\$(BASE_NAME).crc32
    )
  }
}

[=============================================================================]
[Package.BINARY.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_FREEFORM
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{
  Compress (Dummy) {
    Tool ( $(OEMTOOLPATH)\GenCRC32Section
      ARGS = -i $(DEST_DIR)\$(BASE_NAME).sec
             -o $(DEST_DIR)\$(BASE_NAME).crc32
      OUTPUT = $(DEST_DIR)\$(BASE_NAME).crc32
    )
  }
}

[=============================================================================]
#
# Package definition for TE files
#
[Package.PE32_PEIM.TE_PEIM]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_PEIM
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{ 
  $(BASE_NAME).dpx 
  $(BASE_NAME).tes
  $(BASE_NAME).ui 
  $(BASE_NAME).ver 
}

[=============================================================================]
[Package.Config.Config]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_RAW
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{ 
  $(BASE_NAME).ini 
}

[=============================================================================]
#
# Package definition to put the IFR data in a separate section in the
# FFS file.
#
[Package.BS_DRIVER.Ifr_Bin]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_DRIVER
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{
  Compress (Dummy) {
    Tool (
      $(OEMTOOLPATH)\GenCRC32Section
      ARGS= -i $(DEST_DIR)\$(BASE_NAME).dpx
               $(DEST_DIR)\$(BASE_NAME).pe32
               $(DEST_DIR)\$(BASE_NAME).ui
               $(DEST_DIR)\$(BASE_NAME).ver
               $(DEST_DIR)\$(BASE_NAME)IfrBin.sec
            -o $(DEST_DIR)\$(BASE_NAME).crc32
      OUTPUT = $(DEST_DIR)\$(BASE_NAME).crc32
    )
  }
}
[=============================================================================]
#
# These are the libraries that will be built by the master makefile
#
[=============================================================================]
[Libraries]
DEFINE PLATFORM=$(PLATFORM)

#
# SEC libraries
#
# None required

#
# Libraries common to PEI and DXE
#
Foundation\Guid\EdkGuidLib.inf
Foundation\Framework\Guid\EdkFrameworkGuidLib.inf
Foundation\Efi\Guid\EfiGuidLib.inf
Foundation\Library\EfiCommonLib\EfiCommonLib.inf
Foundation\Cpu\Pentium\CpuIA32Lib\CpuIA32Lib.inf

#
# PEI libraries
#
Foundation\Ppi\EdkPpiLib.inf
Foundation\Framework\Ppi\EdkFrameworkPpiLib.inf
Foundation\Library\Pei\PeiLib\PeiLib.inf
Foundation\Library\Pei\Hob\PeiHobLib.inf

#
# DXE libraries
#

Foundation\Protocol\EdkProtocolLib.inf
Foundation\Framework\Protocol\EdkFrameworkProtocolLib.inf
Foundation\Efi\Protocol\EfiProtocolLib.inf
Foundation\Core\Dxe\ArchProtocol\ArchProtocolLib.inf
Foundation\Library\CustomizedDecompress\CustomizedDecompress.inf
Foundation\Library\Dxe\EfiDriverLib\EfiDriverLib.inf
Foundation\Library\RuntimeDxe\EfiRuntimeLib\EfiRuntimeLib.inf
Foundation\Library\Dxe\Graphics\Graphics.inf
Foundation\Library\Dxe\EfiIfrSupportLib\EfiIfrSupportLib.inf
Foundation\Library\Dxe\Print\PrintLib.inf
Sample\Bus\Usb\UsbLib\Dxe\UsbDxeLib.inf
Sample\Bus\Scsi\ScsiLib\Dxe\ScsiLib.inf

#
# BDS libraries
#
Sample\Platform\Generic\Dxe\GenericBds\GenericBds.inf

#
#Print/Graphics Library consume SetupBrowser Print Protocol
#
Foundation\Library\Dxe\PrintLite\PrintLib.inf
Foundation\Library\Dxe\GraphicsLite\Graphics.inf

#
# EBC libraries required by drivers
#
#
# EBC libraries required by applications
#

[=============================================================================]
[=============================================================================]
