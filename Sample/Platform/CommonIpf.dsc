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
#   CommonIpf.dsc
#
#  Abstract:
#
#    This is the build description file containing the processor architecture
#    dependent build instructions.
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
# These are the package descriptions. They are tagged as
# [Package.$(COMPONENT_TYPE).$(PACKAGE)], where COMPONENT_TYPE is typically
# defined in the component INF file, and PACKAGE is typically specified
# in the [components] section below.
#

[=============================================================================]
[Package.SECURITY_CORE.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_SECURITY_CORE
FFS_ATTRIB_CHECKSUM         = TRUE
FFS_ALIGNMENT               = 0x1

IMAGE_SCRIPT =
{          \
  Blank.pad \
  $(BASE_NAME).pe32 \
  $(BASE_NAME).ui \
  $(BASE_NAME).ver \
}

[=============================================================================]
[Package.PEI_CORE.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_PEI_CORE
FFS_ATTRIB_CHECKSUM         = TRUE
FFS_ALIGNMENT               = 0x1

IMAGE_SCRIPT =
{          \
  Blank.pad \
  $(BASE_NAME).pe32 \
  $(BASE_NAME).ui \
  $(BASE_NAME).ver \
}

[=============================================================================]
[Package.PE32_PEIM.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_PEIM
FFS_ATTRIB_CHECKSUM         = TRUE
FFS_ALIGNMENT               = 0x1

IMAGE_SCRIPT =
{ \
  Blank.pad \
  $(BASE_NAME).pe32 \
  $(BASE_NAME).dpx \
  $(BASE_NAME).ui \
  $(BASE_NAME).ver \
}

[=============================================================================]
[Package.PE32_PEIM.CompressPEIM]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_PEIM
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{ 
  $(BASE_NAME).dpx 
  Compress (dummy) {
    $(BASE_NAME).pe32
    $(BASE_NAME).ui 
    $(BASE_NAME).ver
  }
}

[=============================================================================]
[Package.COMBINED_PEIM_DRIVER.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER
FFS_ATTRIB_CHECKSUM         = TRUE
FFS_ALIGNMENT               = 0x1

IMAGE_SCRIPT =
{ \
  Blank.pad \
  $(BASE_NAME).pe32 \
  $(BASE_NAME).dpx \
  $(BASE_NAME).dpxd \
  $(BASE_NAME).ui \
  $(BASE_NAME).ver \
}

[=============================================================================]
[Package.PIC_PEIM.Default]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_PEIM
FFS_ATTRIB_CHECKSUM         = TRUE
FFS_ALIGNMENT               = 0x1

IMAGE_SCRIPT =
{ \
  Blank.pad \
  $(BASE_NAME).dpx \
  $(BASE_NAME).pic \
  $(BASE_NAME).ui \
  $(BASE_NAME).ver \
}


[=============================================================================]
[Package.DxeIplPad.Default]
PACKAGE.INF 
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_RAW
FFS_ATTRIB_CHECKSUM         = TRUE
FFS_ALIGNMENT               = 0x7

IMAGE_SCRIPT =
{ \
  $(BASE_NAME).pad \
}


[=============================================================================]
[Package.BS_DRIVER.DxeMain]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_DXE_CORE
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{
  Compress (Dummy) {
    $(BASE_NAME).pe32
    $(BASE_NAME).ui
    $(BASE_NAME).ver
  }
}


[=============================================================================]
[Package.BS_DRIVER.Default,Package.RT_DRIVER.Default,Package.SAL_RT_DRIVER.Default,Package.BS_DRIVER_EFI.Default]
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
            -o $(DEST_DIR)\$(BASE_NAME).crc32
      OUTPUT = $(DEST_DIR)\$(BASE_NAME).crc32
    )
  }
}


[=============================================================================]
[=============================================================================]
