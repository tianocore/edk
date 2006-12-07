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

#
# Copy shell project to Sample\Application\Shell and uncomment shell library
# for building EBC shell applications
#
#$(EDK_PREFIX)Sample\Application\Shell\Library\EfiShellLib.inf

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
Sample\Bus\Pci\AtapiExtPassThru\Dxe\AtapiExtPassThru.inf

[=============================================================================]