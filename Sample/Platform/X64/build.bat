@rem #/*++
@rem #  
@rem #  Copyright (c) 2007, Intel Corporation                                                         
@rem #  All rights reserved. This program and the accompanying materials                          
@rem #  are licensed and made available under the terms and conditions of the BSD License         
@rem #  which accompanies this distribution.  The full text of the license may be found at        
@rem #  http://opensource.org/licenses/bsd-license.php                                            
@rem #                                                                                            
@rem #  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
@rem #  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
@rem #
@rem #  Module Name:
@rem #  
@rem #    build.bat
@rem #  
@rem #  Abstract:
@rem #    
@rem #    This script wraps Makefile to provide its targets for user. 
@rem #    
@rem #--*/

@echo off
setlocal
set function=
set target=
set mode=

:parse
if /I "%1"=="/c" (
  set function=clean
  shift
  goto parse
)
if /I "%1"=="/?" (
  set function=help
  shift
  goto parse
)
if /I "%1"=="/h" (
  set function=help
  shift
  goto parse
)
if /I "%1"=="all" (
  set mode=all
  shift
  goto parse
)
if not "%1"=="" (
  set target=%1
  shift
  goto parse
) else (
  if "%function%"=="help" (
    goto usage
  ) else (
    set target=%target%%function%%mode%
    goto run
  )
)

:run
if "%target%" == "" set target=all
set OLDTIME=%time%
nmake -f Makefile %target%
set NEWTIME=%time%
echo.
echo Start Time %OLDTIME%
echo End   Time %NEWTIME%
goto end

:usage
echo This script directly invokes the target in wrapper Makefile.
echo.
echo build [/c] [/h] [targetname]
echo.
echo build [targetname]
echo       invoke one target in wrapper Makefile.
echo       if no targetname, target "all" will be invoked.
echo.
echo build /c [all] [targetname]
echo       clean one target in wrapper Makefile.
echo       if option all is used, cleanall for one target.
echo       if no targetname, target "clean" or "cleanall" will be invoked.
echo.
echo build /h
echo build /?
echo       display help information. 
echo.
goto end

:end
@echo on
