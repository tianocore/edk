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
#    DUET.dsc
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
#
# Commands to build a firmware volume
#
[Build.Fv.EfiMain,Build.Fv.EfiExtended]
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
[Fv.EfiMain.Options]
EFI_BASE_ADDRESS        = 0x100000
EFI_FILE_NAME           = $(FV_FILENAME).fv
EFI_NUM_BLOCKS          = 0xC
EFI_BLOCK_SIZE          = 0x8000

[=============================================================================]
[Fv.EfiExtended.Options]
EFI_BASE_ADDRESS        = 0x100000
EFI_FILE_NAME           = $(FV_FILENAME).fv
EFI_NUM_BLOCKS          = 0x4
EFI_BLOCK_SIZE          = 0x8000

[=============================================================================]
[Fv.EfiMain.Attributes,Fv.EfiExtended.Attributes]
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
# These are the libraries that will be built by the master makefile
#
[=============================================================================]
[Libraries]
DEFINE EDK_PREFIX=

!include "$(EDK_SOURCE)\Sample\Platform\EdkLibAll.dsc"

[=============================================================================]
#
# These are platform specific libraries that must be built prior to building
# certain drivers that depend upon them.
#
[=============================================================================]
[Libraries.Platform]
Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\BsDataHubStatusCode\BsDataHubStatusCode.inf
Sample\Platform\Generic\RuntimeDxe\StatusCode\Lib\RtMemoryStatusCode\RtMemoryStatusCode.inf
Sample\Platform\DUET\RuntimeDxe\RtPlatformStatusCode\RtPlatformStatusCode.inf
#Sample\Platform\DUET\Library\Paging\PagingLib.inf

#Sample\Bus\Scsi\ScsiLib\Dxe\ScsiLib.inf
#Sample\Universal\Network\Library\NetLib.inf

Foundation\Library\Thunk16\Thunk16Lib.inf

#
# Shell Library
#
#Sample\Application\Shell\Library\EfiShellLib.inf

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
#DEFINE PACKAGE=DefaultCompact

#
# Select the default FV's
#
DEFINE FV=EfiMain

#
# Loader
#
Sample\Platform\DUET\Loader\EfiLdr\$(PROCESSOR)\EfiLdr.inf FV=NULL
Sample\Platform\DUET\Loader\DxeIpl\DxeIpl.inf FV=NULL

#
# SEC Core
#

#
# PEI Core
#

#
# PEIM
#

#
# DXE Core
#
Foundation\Core\Dxe\DxeMain.inf PACKAGE=DxeMain FV=NULL

#
# Guided Section Extraction Protocol used to authenticate images
#
Sample\Universal\FirmwareVolume\GuidedSectionExtraction\Crc32SectionExtract\Dxe\Crc32SectionExtract.inf

#
# Components that produce the architectural protocols
#
Sample\Universal\WatchdogTimer\Dxe\WatchDogTimer.inf
Sample\Universal\Runtime\Dxe\Runtime.inf
Sample\Universal\MonotonicCounter\RuntimeDxe\FS\MonotonicCounter.inf
Sample\Universal\Variable\RuntimeDxe\FS\FSVariable.inf
#Sample\Universal\Variable\RuntimeDxe\Emu\EmuVariable.inf
Sample\Universal\Security\SecurityStub\Dxe\SecurityStub.inf
Sample\Platform\DUET\Dxe\PlatformBds\PlatformBds.inf
Sample\Platform\Generic\RuntimeDxe\StatusCode\StatusCode.inf
$(CAPSULE_INF)

#
# BDS/UI Support
#
Sample\Platform\Generic\Dxe\ConPlatform\ConPlatform.inf
Sample\Universal\UserInterface\HiiDataBase\Dxe\HiiDatabase.inf
#Sample\Platform\Generic\Logo\Logo.inf                                 PACKAGE=Logo
Sample\Universal\UserInterface\SetupBrowser\Dxe\SetupBrowser.inf

#
# DataHub
#
Sample\Universal\DataHub\DataHub\Dxe\DataHub.inf
Sample\Platform\DUET\Dxe\DataHubGen\DataHubGen.inf

#
# DevPath
#
$(DEVPATH_INF)

#
# CPU Support
#
Sample\Cpu\Pentium\CpuIo\RuntimeDxe\CpuIo.inf
Sample\Cpu\Pentium\SimpleCpu\Dxe\Cpu.inf

#
# PC AT Compatabilble Drivers
#
Sample\Chipset\PcCompatible\8254Timer\Dxe\8254Timer.inf
Sample\Chipset\AdvancedConfiguration\SmartTimer\Dxe\SmartTimer.inf FV=NULL
Sample\Chipset\PcCompatible\8259\Dxe\8259.inf
Sample\Chipset\PcCompatible\Port92Reset\RuntimeDxe\Reset.inf FV=NULL
Sample\Chipset\PcCompatible\KbcReset\RuntimeDxe\Reset.inf
Sample\Chipset\AdvancedConfiguration\AcpiReset\RuntimeDxe\Reset.inf FV=NULL
Sample\Chipset\PcCompatible\Metronome\Dxe\Metronome.inf
Sample\Chipset\AdvancedConfiguration\AcpiMetronome\Dxe\AcpiMetronome.inf FV=NULL
Sample\Chipset\PcCompatible\RealTimeClock\RuntimeDxe\RealTimeClock.inf

#
# PCI Support
#
Sample\Chipset\PcCompatible\PciRootBridgeNoEnumeration\Dxe\PciRootBridgeNoEnumeration.inf
Sample\Bus\Pci\PciBusNoEnumeration\Dxe\PciBusNoEnumeration.inf

#
# LegacyBiosThunk interface
#
Sample\Csm\LegacyBiosThunk\Dxe\LegacyBiosThunk.inf

#
# Video Support (User can choose either VgaMiniPort or thunk BiosVideo, or his/her native Video driver)
#
Sample\Csm\BiosThunk\Video\Dxe\$(UEFI_PREFIX)BiosVideo.inf    FV=NULL
Sample\Bus\Pci\VgaMiniPort\Dxe\VgaMiniPort.inf
#Sample\Bus\Pci\CirrusLogic\Dxe\CirrusLogic5430.inf

#
# Console Support
#
Sample\Universal\Console\ConSplitter\Dxe\ConSplitter.inf
Sample\Universal\Console\GraphicsConsole\Dxe\GraphicsConsole.inf
Sample\Universal\Console\VgaClass\Dxe\VgaClass.inf
Sample\Universal\Console\Terminal\Dxe\Terminal.inf

#
# IDE Support
#
Sample\Bus\Pci\IdeBus\Dxe\IdeBus.inf
Sample\Bus\Pci\IdeController\Dxe\IdeController.inf

#
# USB Support
#
Sample\Bus\Pci\Uhci\Dxe\Uhci.inf
Sample\Bus\Usb\UsbBus\Dxe\UsbBus.inf
Sample\Bus\Usb\UsbBot\Dxe\UsbBot.inf
Sample\Bus\Usb\UsbCbi\Dxe\Cbi0\UsbCbi0.inf
Sample\Bus\Usb\UsbCbi\Dxe\Cbi1\UsbCbi1.inf
Sample\Bus\Usb\UsbKb\Dxe\UsbKb.inf
Sample\Bus\Usb\UsbMassStorage\Dxe\UsbMassStorage.inf
#
# For EHCI, we route it to classic host controller.
#
Sample\Bus\Pci\Ehci\Dxe\Ehci.inf
#Sample\Bus\Pci\EhciRouting\Dxe\EhciRouting.inf

#
# ISA Support
#
Sample\Chipset\PcCompatible\IsaAcpi\Dxe\IsaAcpi.inf
Sample\Bus\Isa\IsaBus\Dxe\IsaBus.inf
Sample\Bus\Isa\IsaFloppy\Dxe\IsaFloppy.inf
Sample\Bus\Isa\IsaSerial\Dxe\IsaSerial.inf
Sample\Bus\Isa\Ps2Keyboard\Dxe\Ps2Keyboard.inf

#
# File System
#
Sample\Universal\Disk\DiskIo\Dxe\DiskIo.inf
Sample\Universal\Disk\UnicodeCollation\English\Dxe\English.inf
Sample\Universal\Disk\Partition\Dxe\Partition.inf
Other\Maintained\Universal\Disk\FileSystem\EnhancedFat\Dxe\Fat.inf

#
# EBC
#
Sample\Universal\Ebc\Dxe\Ebc.inf

#
# Shell
#
#Other\Maintained\Application\$(UEFI_PREFIX)Shell\bin\Shell.inf  FV=NULL

#
# Select the extended FV's
#
DEFINE FV=EfiExtended

#
# Dump Debug Info in DataHub to StdErr Console
#
Sample\Universal\DataHub\DataHubStdErr\Dxe\DataHubStdErr.inf

#
# Drivers necessary to use the debugger
#
#Sample\Universal\Debugger\Debugport\Dxe\DebugPort.inf
#Sample\Cpu\DebugSupport\Dxe\DebugSupport.inf

#
# ISA Support
#
Sample\Bus\Isa\Ps2Mouse\Dxe\Ps2Mouse.inf

#
# USB Support
#
Sample\Bus\Usb\UsbMouse\Dxe\UsbMouse.inf

#
# Network Support
#                                   
Sample\Bus\Pci\Undi\RuntimeDxe\Undi.inf
Sample\Universal\Network\Snp32_64\Dxe\SNP.inf
Sample\Universal\Network\PxeBc\Dxe\BC.inf
Sample\Universal\Network\PxeDhcp4\Dxe\PxeDhcp4.inf

#
# UEFI Network Support
#
#Sample\Universal\Network\Mnp\Dxe\Mnp.inf                         FV=NULL
#Sample\Universal\Network\Arp\Dxe\Arp.inf                         FV=NULL
#Sample\Universal\Network\Ip4\Dxe\Ip4.inf                         FV=NULL
#Sample\Universal\Network\Ip4Config\Dxe\Ip4Config.inf             FV=NULL
#Sample\Universal\Network\Udp4\Dxe\Udp4.inf                       FV=NULL
#Sample\Universal\Network\Tcp4\Dxe\Tcp4.inf                       FV=NULL
#Sample\Universal\Network\Dhcp4\Dxe\Dhcp4.inf                     FV=NULL
#Sample\Universal\Network\Mtftp4\Dxe\Mtftp4.inf                   FV=NULL

#
# SCSI Support
#
#Sample\Bus\Pci\AtapiExtPassThru\Dxe\AtapiExtPassThru.inf
#Sample\Bus\Pci\AtapiPassThru\Dxe\AtapiPassThru.inf
#Sample\Bus\Scsi\ScsiBus\Dxe\ScsiBus.inf
#Sample\Bus\Scsi\ScsiDisk\Dxe\ScsiDisk.inf

#
# Support Application
#
#Sample\Platform\DUET\Loader\DumpBs\DumpBs.inf   FV=NULL

[=============================================================================]
