#/*++
#
# Copyright (c) 2006, Intel Corporation                                                         
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
# These are platform specific libraries that must be built prior to building
# certain drivers that depend upon them.
#
[=============================================================================]
[Libraries.Platform]

DEFINE PROCESSOR=IA32

Foundation\Guid\EdkGuidLib.inf
Foundation\Framework\Guid\EdkFrameworkGuidLib.inf
Foundation\Efi\Guid\EfiGuidLib.inf
Foundation\Library\EfiCommonLib\EfiCommonLib.inf
Foundation\Cpu\Pentium\CpuIA32Lib\CpuIA32Lib.inf
Foundation\Ppi\EdkPpiLib.inf
Foundation\Framework\Ppi\EdkFrameworkPpiLib.inf
Foundation\Library\Pei\PeiLib\PeiLib.inf
Foundation\Library\Pei\Hob\PeiHobLib.inf
Foundation\Library\CustomizedDecompress\CustomizedDecompress.inf
Foundation\Protocol\EdkProtocolLib.inf
Foundation\Framework\Protocol\EdkFrameworkProtocolLib.inf
Foundation\Efi\Protocol\EfiProtocolLib.inf
Foundation\Core\Dxe\ArchProtocol\ArchProtocolLib.inf
Foundation\Library\Dxe\EfiDriverLib\EfiDriverLib.inf
Foundation\Library\RuntimeDxe\EfiRuntimeLib\EfiRuntimeLib.inf
Foundation\Library\Dxe\Graphics\Graphics.inf
Foundation\Library\Dxe\EfiIfrSupportLib\EfiIfrSupportLib.inf
Foundation\Library\Dxe\Print\PrintLib.inf
Foundation\Library\Dxe\PrintLite\PrintLib.inf
Sample\Bus\Usb\UsbLib\Dxe\UsbDxeLib.inf
Foundation\Library\Dxe\PrintLite\PrintLib.inf
Foundation\Library\Dxe\GraphicsLite\Graphics.inf
Foundation\Library\Dxe\Hob\HobLib.inf
Foundation\Library\CompilerStub\CompilerStubLib.inf

DEFINE PROCESSOR=X64

Foundation\Library\Dxe\Hob\HobLib.inf
Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\BsDataHubStatusCode\BsDataHubStatusCode.inf
Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\RtMemoryStatusCode\RtMemoryStatusCode.inf
Sample\Bus\Usb\UsbLib\Dxe\UsbDxeLib.inf
Sample\Bus\Scsi\ScsiLib\Dxe\ScsiLib.inf
Foundation\Library\CompilerStub\CompilerStubLib.inf

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
Sample\Bus\Pci\CirrusLogic\Dxe\CirrusLogic5430.inf
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
Sample\Platform\Generic\Logo\Logo.inf                                 PACKAGE=Logo
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
Sample\Bus\Usb\UsbBot\Dxe\UsbBot.inf                                          
Sample\Bus\Usb\UsbBus\Dxe\UsbBus.inf                                          
Sample\Bus\Usb\UsbCbi\Dxe\Cbi0\UsbCbi0.inf                                    
Sample\Bus\Usb\UsbCbi\Dxe\Cbi1\UsbCbi1.inf                                    
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

Other\Maintained\Universal\Disk\FileSystem\EnhancedFat\Dxe\Fat.inf
Other\Maintained\Application\$(UEFI_PREFIX)Shell\bin\Shell.inf

[=============================================================================]