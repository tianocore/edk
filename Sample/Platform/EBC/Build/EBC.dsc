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
#    EBC.dsc
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
!include "$(EDK_SOURCE)\Sample\Platform\CommonIa32.dsc"

[=============================================================================]
#
# These are the libraries that will be built by the master makefile
#
[=============================================================================]
[Libraries]
DEFINE EDK_PREFIX=

$(EDK_PREFIX)Foundation\Guid\EdkGuidLib.inf
$(EDK_PREFIX)Foundation\Framework\Guid\EdkFrameworkGuidLib.inf
$(EDK_PREFIX)Foundation\Efi\Guid\EfiGuidLib.inf
$(EDK_PREFIX)Foundation\Library\EfiCommonLib\EfiCommonLib.inf
$(EDK_PREFIX)Foundation\Protocol\EdkProtocolLib.inf
$(EDK_PREFIX)Foundation\Framework\Protocol\EdkFrameworkProtocolLib.inf
$(EDK_PREFIX)Foundation\Efi\Protocol\EfiProtocolLib.inf
$(EDK_PREFIX)Foundation\Core\Dxe\ArchProtocol\ArchProtocolLib.inf
$(EDK_PREFIX)Foundation\Library\Dxe\EfiDriverLib\EfiDriverLib.inf
$(EDK_PREFIX)Foundation\Library\Dxe\Graphics\Graphics.inf
$(EDK_PREFIX)Foundation\Library\Dxe\EfiIfrSupportLib\EfiIfrSupportLib.inf
$(EDK_PREFIX)Foundation\Library\Dxe\Print\PrintLib.inf
$(EDK_PREFIX)Foundation\Library\Dxe\PrintLite\PrintLib.inf
$(EDK_PREFIX)Foundation\Library\Dxe\GraphicsLite\Graphics.inf
$(EDK_PREFIX)Foundation\Library\RuntimeDxe\EfiRuntimeLib\EfiRuntimeLib.inf

$(EDK_PREFIX)Sample\Bus\Usb\UsbLib\Dxe\UsbDxeLib.inf
$(EDK_PREFIX)Sample\Bus\Scsi\ScsiLib\Dxe\ScsiLib.inf
$(EDK_PREFIX)Sample\Universal\Network\Library\NetLib.inf

#
# EdkII Glue Library
#
#!include "$(EDK_SOURCE)\Sample\Platform\EdkIIGlueLibAll.dsc"
#

#
# Copy shell project to Other\Maintained\Application\Shell and uncomment the 
# following shell library inf file for building EBC shell applications
#
#$(EDK_PREFIX)Other\Maintained\Application\Shell\Library\EfiShellLib.inf

[=============================================================================]
#
# These are platform specific libraries that must be built prior to building
# certain drivers that depend upon them.
#
[=============================================================================]
[Libraries.Platform]

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
DEFINE FV=NULL

#
# Add your drivers and applications
#
Sample\Bus\Pci\CirrusLogic\Dxe\$(UEFI_PREFIX)CirrusLogic5430.inf
Sample\Bus\Pci\VgaMiniPort\Dxe\VgaMiniPort.inf
Sample\Universal\Console\Consplitter\Dxe\ConSplitter.inf
Sample\Universal\Console\Graphicsconsole\Dxe\GraphicsConsole.inf
Sample\Universal\Console\Vgaclass\Dxe\VgaClass.inf

Sample\Bus\Isa\IsaBus\Dxe\IsaBus.inf
Sample\Bus\Isa\IsaFloppy\Dxe\IsaFloppy.inf
Sample\Bus\Isa\IsaSerial\Dxe\IsaSerial.inf
Sample\Bus\Isa\Ps2Keyboard\Dxe\Ps2keyboard.inf
Sample\Bus\Isa\Ps2Mouse\Dxe\Ps2Mouse.inf
Sample\Universal\Console\Terminal\Dxe\Terminal.inf

Sample\Bus\Pci\AtapiExtPassThru\Dxe\AtapiExtPassThru.inf
Sample\Bus\Pci\AtapiPassThru\Dxe\AtapiPassThru.inf
Sample\Bus\Scsi\ScsiBus\Dxe\ScsiBus.inf
Sample\Bus\Scsi\ScsiDisk\Dxe\ScsiDisk.inf

Sample\Bus\Pci\IdeBus\Dxe\idebus.inf
Sample\Universal\Disk\DiskIo\Dxe\DiskIo.inf
Sample\Universal\Disk\Partition\Dxe\Partition.inf

Other\Maintained\Universal\Disk\FileSystem\EnhancedFat\Dxe\Fat.inf
#Other\Maintained\Universal\Disk\FileSystem\Fat\Dxe\Fat.inf

Sample\Bus\Pci\Ehci\Dxe\Ehci.inf
Sample\Bus\Pci\EhciRouting\Dxe\EhciRouting.inf
Sample\Bus\Pci\Uhci\Dxe\Uhci.inf
Sample\Bus\Usb\UsbBus\Dxe\UsbBus.inf
Sample\Bus\Usb\UsbMassStorage\Dxe\UsbMassStorage.inf
Sample\Bus\Usb\UsbBot\Dxe\UsbBot.inf
Sample\Bus\Usb\UsbCbi\Dxe\Cbi0\UsbCbi0.inf
Sample\Bus\Usb\UsbCbi\Dxe\Cbi1\UsbCbi1.inf
Sample\Bus\Usb\UsbKb\Dxe\UsbKb.inf
Sample\Bus\Usb\UsbMouse\Dxe\UsbMouse.inf

Sample\Bus\Pci\Undi\RuntimeDxe\Undi.inf
Sample\Universal\Network\Snp32_64\Dxe\SNP.inf
Sample\Universal\Network\PxeBc\Dxe\BC.inf
Sample\Universal\Network\PxeDhcp4\Dxe\PxeDhcp4.inf

Sample\Universal\Network\Mnp\Dxe\Mnp.inf
Sample\Universal\Network\Arp\Dxe\Arp.inf
Sample\Universal\Network\Ip4\Dxe\Ip4.inf
Sample\Universal\Network\Ip4Config\Dxe\Ip4Config.inf
Sample\Universal\Network\Udp4\Dxe\Udp4.inf
Sample\Universal\Network\Tcp4\Dxe\Tcp4.inf
Sample\Universal\Network\Dhcp4\Dxe\Dhcp4.inf
Sample\Universal\Network\Mtftp4\Dxe\Mtftp4.inf

[=============================================================================]
