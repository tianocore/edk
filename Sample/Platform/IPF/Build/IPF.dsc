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
#    IPF.dsc
#
#  Abstract:
#
#    This is the build description file containing the platform 
#    specific build definitions.
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
# This section gets processed first by the utility. Define any
# macros that you may use elsewhere in this description file. This is the
# mechanism by which you can pass parameters and defines to the makefiles
# generated for each component. You can define it here, and then make an
# assignment in the [makefile.common] section. For example, if here you
# define MY_VAR = my_var_value, then you can add MY_VAR = $(MY_VAR) in
# the [makefile.common] section and it becomes MY_VAR = my_var_value in
# the output makefiles for each component.
#
[Defines]
PROCESSOR=$(PROCESSOR)
PLATFORM=$(PROJECT_NAME)


[=============================================================================]
#
# Include other common build descriptions
#
!include "$(EDK_SOURCE)\Sample\Platform\Common.dsc"
!include "$(EDK_SOURCE)\Sample\Platform\Common$(PROCESSOR).dsc"




[=============================================================================]
#
# These are the package descriptions. They are tagged as
# [Package.$(COMPONENT_TYPE).$(PACKAGE)], where COMPONENT_TYPE is typically
# defined in the component INF file, and PACKAGE is typically specified
# in the [components] section below.
#


[=============================================================================]
[Package.FILE.AcpiTable]
PACKAGE.INF
\[.]
BASE_NAME                   = $(BASE_NAME)
FFS_FILEGUID                = $(FILE_GUID)
FFS_FILETYPE                = EFI_FV_FILETYPE_FREEFORM
FFS_ATTRIB_CHECKSUM         = TRUE

IMAGE_SCRIPT =
{
  Compress (Dummy) {
    Tool (
      $(OEMTOOLPATH)\GenCRC32Section
      ARGS= -i $(DEST_DIR)\Fadt.sec
               $(DEST_DIR)\Facs.sec
               $(DEST_DIR)\Dsdt.sec
               $(DEST_DIR)\Dsdt2.sec               
               $(DEST_DIR)\Madt.sec               
               $(DEST_DIR)\Spcr.sec
               $(DEST_DIR)\$(BASE_NAME).ui
            -o $(DEST_DIR)\$(BASE_NAME).crc32
      OUTPUT = $(DEST_DIR)\$(BASE_NAME).crc32
    )
  }
}

[=============================================================================]
[Package.BS_DRIVER.OpRomDriver]
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
      ARGS= -i $(DEST_DIR)\$(BASE_NAME).pe32
            -o $(DEST_DIR)\$(BASE_NAME).crc32
      OUTPUT = $(DEST_DIR)\$(BASE_NAME).crc32
    )
  }
}

[=============================================================================]
[Package.RT_DRIVER.FpswaDriver]
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
      ARGS= -i $(DEST_DIR)\$(BASE_NAME).pe32
            -o $(DEST_DIR)\$(BASE_NAME).crc32
      OUTPUT = $(DEST_DIR)\$(BASE_NAME).crc32
    )
  }
}

[=============================================================================]
[Package.RT_DRIVER.GigUndiDriver]
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
      ARGS= -i $(DEST_DIR)\$(BASE_NAME).pe32
            -o $(DEST_DIR)\$(BASE_NAME).crc32
      OUTPUT = $(DEST_DIR)\$(BASE_NAME).crc32
    )
  }
}

[=============================================================================]
#
# These get emitted to the output makefile for each FV being built.
#

[=============================================================================]
[Build.Fv.FvNorth1]
#
# Recovery FV containing bootstrap files, PEI, and recovery components
#

[=============================================================================]
[Build.Fv.FvNorth2]
#
# Recovery FV containing bootstrap files, PEI, and recovery components
#

[=============================================================================]
[Build.Fv.FvSouth1,Build.Fv.FvRecoveryFloppy]
#
# Main FV containing DXE components
#

[=============================================================================]
[Build.Fv.FvSouth2]
#
# Main FV containing DXE components
#

[=============================================================================]
[Build.Fv.NvStorage]
#
# FV containing runtime updatable data
#

[=============================================================================]
[Build.Fv.McaStorage]
#
# FV containing runtime updatable data
#

[=============================================================================]
#
# These control the generation of the FV files
#

[=============================================================================]
[Fv.FvNorth1.Options]

[=============================================================================]
[Fv.FvNorth2.Options]

[=============================================================================]
[Fv.FvSouth1.Options]

[=============================================================================]
[Fv.FvSouth2.Options]

[=============================================================================]
[Fv.FvRecoveryFloppy.Options]

[=============================================================================]
[Fv.NvStorage.Options]

[=============================================================================]
[Fv.McaStorage.Options]

[=============================================================================]
[Fv.NvStorage.Components]

[=============================================================================]
[Fv.McaStorage.Components]

[=============================================================================]
[Fv.FvNorth1.Attributes,Fv.FvNorth2.Attributes,Fv.FvSouth1.Attributes,Fv.FvSouth2.Attributes,Fv.NvStorage.Attributes,Fv.McaStorage.Attributes,Fv.FvRecoveryFloppy.Attributes]


[=============================================================================]
#
# These are platform specific libraries that must be built prior to building
# certain drivers that depend upon them.
#
[=============================================================================]
[Libraries.Platform]
Foundation\Library\pei\PeiLib\PeiLib.inf
Foundation\Library\Dxe\Hob\HobLib.inf

Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\RtMemoryStatusCode\RtMemoryStatusCode.inf
Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\BsDataHubStatusCode\BsDataHubStatusCode.inf
Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\BsSerialStatusCode\BsSerialStatusCode.inf



[=============================================================================]
#
# These are the components that will be built by the master makefile
#
[=============================================================================]
[Components]

#
# The default package
#
DEFINE PACKAGE=Default

#
# By default components in this section belong in the recovery FV.
# This FV primarily contains PEI code.
#
DEFINE FV=FvNorth1

#
# SEC Core
#
# None required


#
# PEI core
#


DEFINE FV=FvNorth2

#
# PEIM
#
Sample\Universal\DxeIpl\Pei\DxeIpl.inf                                                 FV=FvNorth2

#
# Remainder of files go to south firmware volume
#

#
# Set default FV to both
#
DEFINE FV=FvSouth1,FvRecoveryFloppy

#
# DXE Core
#
Foundation\Core\Dxe\DxeMain.inf                                                         PACKAGE=DxeMain

#
# APRIORI list, this is a list of drivers run without dependencies
#

#
# Guided Section Extraction Protocol used to authenticate images
#
Sample\Universal\FirmwareVolume\GuidedSectionExtraction\Crc32SectionExtract\Dxe\Crc32SectionExtract.inf

#
# Components common to hardware and simulation
#
Sample\Universal\DataHub\DataHub\Dxe\DataHub.inf
Sample\Universal\Security\SecurityStub\Dxe\SecurityStub.inf

#
# Set default FV to both
#
DEFINE FV=FvSouth2,FvRecoveryFloppy

Sample\Universal\DataHub\DataHubStdErr\Dxe\DataHubStdErr.inf
Sample\Universal\UserInterface\SetupBrowser\Dxe\setupbrowser.inf
Sample\Universal\Disk\Partition\Dxe\Partition.inf
Sample\Universal\Disk\DiskIo\Dxe\DiskIo.inf
Sample\Universal\UserInterface\HiiDataBase\Dxe\HiiDatabase.inf
Sample\Universal\Disk\UnicodeCollation\English\Dxe\English.inf
Sample\Bus\Pci\PciBus\Dxe\PciBus.inf
Sample\Universal\Console\Terminal\Dxe\terminal.inf
Sample\Platform\Generic\Dxe\ConPlatform\ConPlatform.inf
Sample\Universal\Console\ConSplitter\Dxe\ConSplitter.inf
Sample\Universal\WatchdogTimer\Dxe\WatchDogTimer.inf
Sample\Universal\Variable\RuntimeDxe\Variable.inf                               FV=FvSouth2
Sample\Universal\Variable\RuntimeDxe\Emu\EmuVariable.inf                        FV=FvRecoveryFloppy
Sample\Universal\Runtime\Dxe\Runtime.inf
Sample\Universal\MonotonicCounter\RuntimeDxe\MonotonicCounter.inf
Other\Maintained\Application\Shell\Bin\Shell.inf
Sample\Universal\Console\GraphicsConsole\Dxe\GraphicsConsole.inf
Sample\Platform\Generic\Logo\Logo.inf                                           PACKAGE=Logo
Sample\Bus\Pci\IdeBus\Dxe\IdeBus.inf
#
# Usb support
#
Sample\Bus\Usb\UsbBot\Dxe\UsbBot.inf
Sample\Bus\Usb\UsbCbi\Dxe\Cbi0\UsbCbi0.inf
Sample\Bus\Usb\UsbCbi\Dxe\Cbi1\UsbCbi1.inf                                  
Sample\Bus\Usb\UsbKb\Dxe\UsbKb.inf                                     
Sample\Bus\Usb\UsbMassStorage\Dxe\UsbMassStorage.inf
Sample\Bus\Usb\UsbMouse\Dxe\UsbMouse.inf             
Sample\Bus\Pci\Uhci\Dxe\Uhci.inf
Sample\Bus\Usb\UsbBus\Dxe\UsbBus.inf

#
# The following components are used for network boot
#
Sample\Bus\Pci\Undi\RuntimeDxe\Undi.inf                        FV=FvSouth2
Sample\Universal\Network\PxeDhcp4\Dxe\Dhcp4.inf                FV=FvSouth2
Sample\Universal\Network\Snp32_64\Dxe\Snp.inf                  FV=FvSouth2
Sample\Universal\Network\PxeBc\Dxe\Bc.inf                      FV=FvSouth2    

#
# NvStorage FV primarily contains private data of FTW and runtime updatable
# data such as variable and event log. It does not contain any FFS file
#
DEFINE NONFFS_FV = NvStorage
[=============================================================================]
