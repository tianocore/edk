#/*++
#
# Copyright (c) 2004 - 2006, Intel Corporation                                                         
# All rights reserved. This program and the accompanying materials                          
# are licensed and made available under the terms and conditions of the BSD License         
# which accompanies this distribution.  The full text of the license may be found at        
# http://opensource.org/licenses/bsd-license.php                                            
#                                                                                           
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
# 
#--*/
#
# EDK Common Library
#

#
# Libraries common to PEI and DXE
#
$(EDK_PREFIX)Foundation\Efi\Guid\EfiGuidLib.inf
$(EDK_PREFIX)Foundation\Framework\Guid\EdkFrameworkGuidLib.inf
$(EDK_PREFIX)Foundation\Guid\EdkGuidLib.inf
$(EDK_PREFIX)Foundation\Library\EfiCommonLib\EfiCommonLib.inf
$(EDK_PREFIX)Foundation\Cpu\Pentium\CpuIA32Lib\CpuIA32Lib.inf
$(EDK_PREFIX)Foundation\Cpu\Itanium\CpuIA64Lib\CpuIA64Lib.inf
$(EDK_PREFIX)Foundation\Library\CustomizedDecompress\CustomizedDecompress.inf
$(EDK_PREFIX)Foundation\Library\CompilerStub\CompilerStubLib.inf
$(EDK_PREFIX)Foundation\Library\Dxe\Hob\HobLib.inf

#
# PEI libraries
#
$(EDK_PREFIX)Foundation\Framework\Ppi\EdkFrameworkPpiLib.inf
$(EDK_PREFIX)Foundation\Ppi\EdkPpiLib.inf
$(EDK_PREFIX)Foundation\Library\Pei\PeiLib\PeiLib.inf
$(EDK_PREFIX)Foundation\Library\Pei\Hob\PeiHobLib.inf

#
# DXE libraries
#
$(EDK_PREFIX)Foundation\Core\Dxe\ArchProtocol\ArchProtocolLib.inf
$(EDK_PREFIX)Foundation\Efi\Protocol\EfiProtocolLib.inf
$(EDK_PREFIX)Foundation\Framework\Protocol\EdkFrameworkProtocolLib.inf
$(EDK_PREFIX)Foundation\Protocol\EdkProtocolLib.inf
$(EDK_PREFIX)Foundation\Library\Dxe\EfiDriverLib\EfiDriverLib.inf
$(EDK_PREFIX)Foundation\Library\RuntimeDxe\EfiRuntimeLib\EfiRuntimeLib.inf
$(EDK_PREFIX)Foundation\Library\Dxe\Graphics\Graphics.inf
$(EDK_PREFIX)Foundation\Library\Dxe\EfiIfrSupportLib\EfiIfrSupportLib.inf
$(EDK_PREFIX)Foundation\Library\Dxe\Print\PrintLib.inf

#
# Print/Graphics Library consume SetupBrowser Print Protocol
#
$(EDK_PREFIX)Foundation\Library\Dxe\PrintLite\PrintLib.inf
$(EDK_PREFIX)Foundation\Library\Dxe\GraphicsLite\Graphics.inf

#
# Module Libraries
#
$(EDK_PREFIX)Sample\Platform\Generic\Dxe\GenericBds\GenericBds.inf
$(EDK_PREFIX)Sample\Bus\Usb\UsbLib\Dxe\UsbDxeLib.inf

#
# EBC libraries required by drivers
#
#$(EDK_PREFIX)Foundation\Guid\EdkGuidLib.inf                                    PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Framework\Guid\EdkFrameworkGuidLib.inf                 PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Efi\Guid\EfiGuidLib.inf                                PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Library\EfiCommonLib\EfiCommonLib.inf                  PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Protocol\EdkProtocolLib.inf                            PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Framework\Protocol\EdkFrameworkProtocolLib.inf         PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Efi\Protocol\EfiProtocolLib.inf                        PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Core\Dxe\ArchProtocol\ArchProtocolLib.inf              PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Library\Dxe\EfiDriverLib\EfiDriverLib.inf              PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Library\Dxe\Graphics\Graphics.inf                      PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Library\Dxe\EfiIfrSupportLib\EfiIfrSupportLib.inf      PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Library\Dxe\Print\PrintLib.inf                         PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Library\Dxe\PrintLite\PrintLib.inf                     PROCESSOR=EBC
#$(EDK_PREFIX)Foundation\Library\Dxe\GraphicsLite\Graphics.inf                  PROCESSOR=EBC
