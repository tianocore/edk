#/*++
#
# Copyright (c) 2006 - 2007, Intel Corporation                                                         
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
#    X64.dsc
#
#  Abstract:
#
#    This is the build description file containing the platform 
#    build definitions.
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
PLATFORM                  = $(PROJECT_NAME)

[=============================================================================]
#
# Include other common build descriptions
#
!include "$(EDK_SOURCE)\Sample\Platform\Common.dsc"
!include "$(EDK_SOURCE)\Sample\Platform\Common$(PROCESSOR).dsc"

[=============================================================================]
[Fv.Fv.Attributes]

[Fv.Fv.options]

[Build.Fv.Fv]

[=============================================================================]
#
# These are the libraries that will be built by the master makefile
#
[=============================================================================]
[Libraries]
DEFINE EDK_PREFIX=

DEFINE PROCESSOR=IA32

!include "$(EDK_SOURCE)\Sample\Platform\EdkLib32.dsc"

#!include "$(EDK_SOURCE)\Sample\Platform\EdkIIGlueLib32.dsc"

DEFINE PROCESSOR=X64

!include "$(EDK_SOURCE)\Sample\Platform\EdkLibAll.dsc"

#!include "$(EDK_SOURCE)\Sample\Platform\EdkIIGlueLibAll.dsc"

[=============================================================================]
#
# These are platform specific libraries that must be built prior to building
# certain drivers that depend upon them.
#
[=============================================================================]
[Libraries.Platform]

DEFINE PROCESSOR=X64

Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\BsDataHubStatusCode\BsDataHubStatusCode.inf
Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\RtMemoryStatusCode\RtMemoryStatusCode.inf

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
# Select the default FV's
#
DEFINE FV=Fv

DEFINE PROCESSOR=IA32

#
# SEC Core
#

#
# PEI Core
#
Foundation\Core\Pei\PeiMain.inf 

#
# PEIM
#
Sample\Universal\DxeIpl\Pei\DxeIplX64.inf

DEFINE PROCESSOR=X64

#
# DXE Core
#
Foundation\Core\Dxe\DxeMain.inf PACKAGE=DxeMain

#
# Guided Section Extraction Protocol used to authenticate images
#
Sample\Universal\FirmwareVolume\GuidedSectionExtraction\Crc32SectionExtract\Dxe\Crc32SectionExtract.inf

#
# Components that produce the architectural protocols
#
Sample\Universal\WatchdogTimer\Dxe\WatchDogTimer.inf
Sample\Universal\Runtime\Dxe\Runtime.inf
Sample\Universal\MonotonicCounter\RuntimeDxe\MonotonicCounter.inf
Sample\Universal\Variable\RuntimeDxe\Emu\EmuVariable.inf                            
Sample\Universal\Security\SecurityStub\Dxe\SecurityStub.inf

#
# Following are the DXE drivers (alphabetical order)
#
Sample\Platform\Generic\Dxe\ConPlatform\ConPlatform.inf
Sample\Universal\Console\ConSplitter\Dxe\ConSplitter.inf
Sample\Universal\DataHub\DataHub\Dxe\DataHub.inf
Sample\Universal\DataHub\DataHubStdErr\Dxe\DataHubStdErr.inf
Sample\Universal\Disk\DiskIo\Dxe\DiskIo.inf
Sample\Universal\UserInterface\SetupBrowser\Dxe\DriverSample\DriverSample.Inf
Sample\Universal\Ebc\Dxe\Ebc.inf
Sample\Universal\Disk\UnicodeCollation\English\Dxe\English.inf
Sample\Universal\Console\GraphicsConsole\Dxe\GraphicsConsole.inf
Sample\Universal\UserInterface\HiiDataBase\Dxe\HiiDatabase.inf
Sample\Bus\Pci\IdeBus\Dxe\IdeBus.inf
Sample\Platform\Generic\Logo\Logo.inf
Sample\Universal\GenericMemoryTest\Dxe\NullMemoryTest.inf
Sample\Universal\Disk\Partition\Dxe\Partition.inf
Sample\Bus\Pci\PciBusNoEnumeration\Dxe\PciBusNoEnumeration.inf
Sample\Bus\Scsi\ScsiBus\Dxe\ScsiBus.inf
Sample\Bus\Scsi\ScsiDisk\Dxe\ScsiDisk.inf   
Sample\Universal\UserInterface\SetupBrowser\Dxe\SetupBrowser.Inf
Sample\Universal\Console\Terminal\Dxe\Terminal.inf
Sample\Bus\Pci\Uhci\Dxe\Uhci.inf   

#
# USB Support
#                                  
Sample\Bus\Usb\UsbBus\Dxe\UsbBus.inf                                                                           
Sample\Bus\Usb\UsbKb\Dxe\UsbKb.inf                                            
Sample\Bus\Usb\UsbMassStorage\Dxe\UsbMassStorage.inf                          
Sample\Bus\Usb\UsbMouse\Dxe\UsbMouse.inf   

#
# Network Boot
#                                   
#Sample\Bus\Pci\Undi\RuntimeDxe\Undi.inf                                       
Sample\Universal\Network\PxeBc\Dxe\BC.inf                                     
Sample\Universal\Network\PxeDhcp4\Dxe\PxeDhcp4.inf
Sample\Universal\Network\Snp32_64\Dxe\SNP.inf    

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

[=============================================================================]