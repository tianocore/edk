#/*++
#
# Copyright (c) 2005, Intel Corporation                                                         
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
#    EoE.dsc
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
PROCESSOR                 = $(PROCESSOR)
PLATFORM                  = $(PROJECT_NAME)
SHELL_INF                 = $(SHELL_INF)

[=============================================================================]
#
# Include other common build descriptions
#
!include "$(EDK_SOURCE)\Sample\Platform\Common.dsc"
!include "$(EDK_SOURCE)\Sample\Platform\Common$(PROCESSOR).dsc"

[=============================================================================]
#
# Commands to build a firmware volume
#
[Build.Fv.x64,Build.Fv.FvMain32]
#
# ORIGIN: [Build.Fv.*] section of the platform DSC file
#
Fv\$(FV_FILENAME).fv : $(DSC_FILENAME) $($(FV_FILENAME)_FILES) Fv\$(FV_FILENAME).inf
  @cd Fv
  $(GENFVIMAGE) -I $(FV_FILENAME).inf
  @cd ..

[=============================================================================]
#
# These control the generation of the FV files
#
[=============================================================================]
[Fv.x64.Options,Fv.FvMain32.Options]
EFI_BASE_ADDRESS        = 0x100000
EFI_FILE_NAME           = $(FV_FILENAME).fv
EFI_NUM_BLOCKS          = 0x016
EFI_BLOCK_SIZE          = 0x010000


[=============================================================================]
[Fv.x64.Attributes,Fv.FvMain32.Attributes]
EFI_READ_DISABLED_CAP   = TRUE
EFI_READ_ENABLED_CAP    = TRUE
EFI_READ_STATUS         = TRUE
EFI_WRITE_DISABLED_CAP  = TRUE
EFI_WRITE_ENABLED_CAP   = TRUE
EFI_WRITE_STATUS        = TRUE
EFI_LOCK_CAP            = TRUE
EFI_LOCK_STATUS         = FALSE
EFI_STICKY_WRITE        = TRUE
EFI_MEMORY_MAPPED       = TRUE
EFI_ERASE_POLARITY      = 1
EFI_ALIGNMENT_CAP       = TRUE
EFI_ALIGNMENT_2         = TRUE
EFI_ALIGNMENT_4         = TRUE
EFI_ALIGNMENT_8         = TRUE
EFI_ALIGNMENT_16        = TRUE
EFI_ALIGNMENT_32        = TRUE
EFI_ALIGNMENT_64        = TRUE
EFI_ALIGNMENT_128       = TRUE
EFI_ALIGNMENT_256       = TRUE
EFI_ALIGNMENT_512       = TRUE
EFI_ALIGNMENT_1K        = TRUE
EFI_ALIGNMENT_2K        = TRUE
EFI_ALIGNMENT_4K        = TRUE
EFI_ALIGNMENT_8K        = TRUE
EFI_ALIGNMENT_16K       = TRUE
EFI_ALIGNMENT_32K       = TRUE
EFI_ALIGNMENT_64K       = TRUE

[=============================================================================]
#
# These are platform specific libraries that must be built prior to building
# certain drivers that depend upon them.
#
[=============================================================================]
[Libraries.Platform]

DEFINE TOOLCHAIN=TOOLCHAIN_IA32
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



DEFINE TOOLCHAIN=TOOLCHAIN_X64
DEFINE PROCESSOR=X64

Foundation\Library\Dxe\Hob\HobLib.inf

Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\BsDataHubStatusCode\BsDataHubStatusCode.inf
Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\RtMemoryStatusCode\RtMemoryStatusCode.inf
Sample\Platform\EoE\RuntimeDxe\RtPlatformStatusCode\RtPlatformStatusCode.inf

Sample\Bus\Usb\UsbLib\Dxe\UsbDxeLib.inf
Sample\Bus\Scsi\ScsiLib\Dxe\ScsiLib.inf

Sample\Chipset\PcCompatible\Protocol\PcCompatibleProtocolLib.inf

[=============================================================================]
#
# These are the components that will be built by the master makefile
#
[=============================================================================]
[Components]

DEFINE TOOLCHAIN=TOOLCHAIN_X64
DEFINE PROCESSOR=X64
#
# The default package
#
DEFINE PACKAGE=Default

#
# Select the default FV's
#
DEFINE FV=X64

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
Sample\Universal\GenericMemoryTest\Pei\BaseMemoryTest.inf
Sample\Universal\DxeIpl\Pei\DxeIpl.inf

Sample\Platform\EoE\PpisNeededByDxeCore\PpisNeededByDxeCore.inf

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
Sample\Platform\EoE\Dxe\PlatformBds\PlatformBds.inf
Sample\Platform\Generic\RuntimeDxe\StatusCode\StatusCode.inf


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
Sample\Platform\EoE\Dxe\IdeController\Dxe\IdeController.inf
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
Sample\Universal\Network\PxeDhcp4\Dxe\Dhcp4.inf                               
Sample\Universal\Network\Snp32_64\Dxe\SNP.inf    

#
# CPU Drivers
#
Sample\Cpu\Pentium\CpuIo\RuntimeDxe\CpuIo.inf
Sample\Cpu\Pentium\Cpu\Dxe\Cpu.inf

#
# PC AT Compatabilble Drivers
#
Sample\Chipset\PcCompatible\8254Timer\Dxe\8254Timer.inf
Sample\Chipset\PcCompatible\8259\Dxe\8259.inf
Sample\Chipset\PcCompatible\Cf9Reset\RuntimeDxe\Cf9Reset.inf
Sample\Chipset\PcCompatible\Metronome\Dxe\Metronome.inf
Sample\Chipset\PcCompatible\PciRootBridgeNoEnumeration\Dxe\PciRootBridgeNoEnumeration.inf
Sample\Chipset\PcCompatible\RealTimeClock\RuntimeDxe\RealTimeClock.inf


#
# Legacy BIOS thunk drivers
#
Sample\Chipset\PcCompatible\Bios\BiosKeyboard\Dxe\BiosKeyboard.inf
Sample\Chipset\PcCompatible\Bios\BiosSnp16\Dxe\BiosSnp16.inf
Sample\Chipset\PcCompatible\Bios\BiosVideo\Dxe\BiosVideo.inf

Other\Maintained\Universal\Disk\FileSystem\EnhancedFat\Dxe\Fat.inf

Other\Maintained\Application\Shell\bin\Shell.inf

DEFINE TOOLCHAIN=TOOLCHAIN_IA32
DEFINE PROCESSOR=IA32

Sample\Platform\EoE\Go64\Go64.inf
Sample\Platform\EoE\PeiCommand\PeiCommand.inf

[=============================================================================]