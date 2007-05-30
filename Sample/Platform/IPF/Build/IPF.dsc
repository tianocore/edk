#/*++
#
# Copyright (c) 2004 - 2007, Intel Corporation                                                         
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
PLATFORM=$(PROJECT_NAME)


[=============================================================================]
#
# Include other common build descriptions
#
!include "$(EDK_SOURCE)\Sample\Platform\Common.dsc"
!include "$(EDK_SOURCE)\Sample\Platform\Common$(PROCESSOR).dsc"



[=============================================================================]
[Fv.Fv.Attributes]

[Fv.Fv.options]

[build.fv.Fv]

[=============================================================================]
#
# These are the libraries that will be built by the master makefile
#
[=============================================================================]
[Libraries]
DEFINE EDK_PREFIX=

!include "$(EDK_SOURCE)\Sample\Platform\EdkLibAll.dsc"

#
# EdkII Glue Library
#
#!include "$(EDK_SOURCE)\Sample\Platform\EdkIIGlueLibAll.dsc"

[=============================================================================]
#
# These are platform specific libraries that must be built prior to building
# certain drivers that depend upon them.
#
[=============================================================================]
[Libraries.Platform]
Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\RtMemoryStatusCode\RtMemoryStatusCode.inf
Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\BsDataHubStatusCode\BsDataHubStatusCode.inf
Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\BsSerialStatusCode\BsSerialStatusCode.inf

Sample\Bus\Scsi\ScsiLib\Dxe\ScsiLib.inf
Sample\Universal\Network\Library\NetLib.inf

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
DEFINE FV=Fv

#
# SEC Core
#
# None required


#
# PEI core
#

#
# PEIM
#
Sample\Universal\DxeIpl\Pei\DxeIpl.inf                                               

#
# Remainder of files go to south firmware volume
#


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
Sample\Universal\FirmwareVolume\FaultTolerantWriteLite\Dxe\FtwLite.inf
Sample\Universal\Variable\RuntimeDxe\Variable.inf                              
Sample\Universal\Variable\RuntimeDxe\Emu\EmuVariable.inf                       
Sample\Universal\Runtime\Dxe\Runtime.inf
Sample\Universal\MonotonicCounter\RuntimeDxe\MonotonicCounter.inf
Sample\Universal\Console\GraphicsConsole\Dxe\GraphicsConsole.inf
Sample\Platform\Generic\Logo\Logo.inf
Sample\Bus\Pci\IdeBus\Dxe\IdeBus.inf
#
# Usb support
#                                
Sample\Bus\Usb\UsbKb\Dxe\UsbKb.inf                                     
Sample\Bus\Usb\UsbMassStorage\Dxe\UsbMassStorage.inf
Sample\Bus\Usb\UsbMouse\Dxe\UsbMouse.inf             
Sample\Bus\Pci\Uhci\Dxe\Uhci.inf
Sample\Bus\Usb\UsbBus\Dxe\UsbBus.inf

#
# SCSI modules
#
Sample\Bus\Pci\AtapiExtPassThru\Dxe\AtapiExtPassThru.inf
Sample\Bus\Pci\AtapiPassThru\Dxe\AtapiPassThru.inf
Sample\Bus\Scsi\ScsiBus\Dxe\ScsiBus.inf
Sample\Bus\Scsi\ScsiDisk\Dxe\ScsiDisk.inf


#
# The following components are used for network boot
#
Sample\Bus\Pci\Undi\RuntimeDxe\Undi.inf                       
Sample\Universal\Network\PxeDhcp4\Dxe\PxeDhcp4.inf
Sample\Universal\Network\Snp32_64\Dxe\Snp.inf                 
Sample\Universal\Network\PxeBc\Dxe\Bc.inf                      

#
# UEFI network drivers.
#
Sample\Universal\Network\Mnp\Dxe\Mnp.inf                               FV=NULL
Sample\Universal\Network\Arp\Dxe\Arp.inf                               FV=NULL
Sample\Universal\Network\Ip4\Dxe\Ip4.inf                               FV=NULL
Sample\Universal\Network\Ip4Config\Dxe\Ip4Config.inf                   FV=NULL
Sample\Universal\Network\Udp4\Dxe\Udp4.inf                             FV=NULL
Sample\Universal\Network\Tcp4\Dxe\Tcp4.inf                             FV=NULL
Sample\Universal\Network\Dhcp4\Dxe\Dhcp4.inf                           FV=NULL
Sample\Universal\Network\Mtftp4\Dxe\Mtftp4.inf                         FV=NULL

#
# Binaries
#
Other\Maintained\Universal\Disk\FileSystem\EnhancedFat\Dxe\Fat.inf
Other\Maintained\Application\$(UEFI_PREFIX)Shell\bin\Shell.inf

#
# NvStorage FV primarily contains private data of FTW and runtime updatable
# data such as variable and event log. It does not contain any FFS file
#
#DEFINE NONFFS_FV = NvStorage
[=============================================================================]
