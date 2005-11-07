@echo off
REM #/*++
REM #  
rem #  Copyright (c) 2004, Intel Corporation                                                         
rem #  All rights reserved. This program and the accompanying materials                          
rem #  are licensed and made available under the terms and conditions of the BSD License         
rem #  which accompanies this distribution.  The full text of the license may be found at        
rem #  http://opensource.org/licenses/bsd-license.php                                            
rem #                                                                                            
rem #  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
rem #  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
rem #  
REM #  Module Name:
REM #  
REM #    system.cmd
REM #  
REM #  Abstract:
REM #  
REM #    Script file to setup environment for running the NT emulation.
REM #    
REM #--*/  
REM #
REM # This file is used to set envirnoment variables for the Win NT build.
REM # These variables are used to define the (virtual) hardware configuration
REM # of the NT environment
REM #
REM # A ! can be used to seperate multiple instances in a variable. Each 
REM # instance represents a seperate hardware device. 
REM #
REM # EFI_WIN_NT_SERIAL_PORT    - maps physical serial ports
REM # EFI_WIN_NT_UGA            - Builds UGA Window
REM # EFI_WIN_NT_FILE_SYSTEM    - map a local directory to a file system
REM # EFI_FIRMWARE_VOLUMES      - File name of FDs, ! supported
REM # EFI_MEMORY_SIZE           - size of memory in megabytes, ! supported

REM # These variables only support a single instance
REM #
REM # EFI_WIN_NT_CONSOLE        - make a logical comand line window (only one!)
REM # EFI_BOOT_MODE             - decimal representation for the boot mode
REM # EFI_CPU_MODEL             - Customize CPU model
REM # EFI_CPU_SPEED             - Customize CPU speed in MHz

REM #
REM # EFI_WIN_NT_PHYSICAL_DISKS - maps to drives on your system
REM # EFI_WIN_NT_VIRTUAL_DISKS  - maps to an device emulated by a file
REM #
REM #  <F>ixed       - Fixed disk like a hard drive.
REM #  <R>emovable   - Removable media like a floppy or CD-ROM.
REM #  Read <O>nly   - Write protected device.
REM #  Read <W>rite  - Read write device.
REM #  <block count> - Decimal number of blocks a device supports.
REM #  <block size>  - Decimal number of bytes per block.
REM #
REM #  NT envirnonment variable contents. '<' and '>' are not part of the variable, 
REM #  they are just used to make this help more readable. There should be no 
REM #  spaces between the ';'. Extra spaces will break the variable. A '!' is  
REM #  used to seperate multiple devices in a variable.
REM #
REM #  EFI_WIN_NT_VIRTUAL_DISKS = 
REM #    <F | R><O | W>;<block count>;<block size>[!...]
REM #
REM #  EFI_WIN_NT_PHYSICAL_DISKS =
REM #    <drive letter>:<F | R><O | W>;<block count>;<block size>[!...]
REM #
REM #  Virtual Disks: These devices use a file to emulate a hard disk or removable
REM #                 media device. 
REM #                 
REM #    Thus a 20 MB emulated hard drive would look like:
REM #    EFI_WIN_NT_VIRTUAL_DISKS=FW;40960;512
REM #
REM #    A 1.44MB emulated floppy with a block size of 1024 would look like:
REM #    EFI_WIN_NT_VIRTUAL_DISKS=RW;1440;1024
REM #
REM #  Physical Disks: These devices use NT to open a real device in your system
REM #	 Please be very careful you can do a lot of damage to your system with
REM #    this feature. It's a good idea to start out with a CD as it's read only.
REM #
REM #    Thus a 120 MB floppy would look like:
REM #    EFI_WIN_NT_PHYSICAL_DISKS=B:RW;245760;512
REM #
REM #    Thus a standard CD-ROM would look like:
REM #    EFI_WIN_NT_PHYSICAL_DISKS=Z:RO;307200;2048


REM #
REM # Map a: as a 1.44MB Floppy and a 128 MB USB Disk
REM #  drive letters may be different on your machine
REM #
echo on
set EFI_WIN_NT_PHYSICAL_DISKS=a:RW;2880;512!E:RW;204440;2048
@echo off

REM #
REM # Map a file as a 20 MB hard disk. 
REM #
echo on
set EFI_WIN_NT_VIRTUAL_DISKS=FW;40960;512
@echo off

REM #
REM # Map COM1 and COM2
REM #
echo on
set EFI_WIN_NT_SERIAL_PORT=COM1!COM2
@echo off

REM #
REM # Make Win NT Windows. Only one supported today. Mostly obsolete with
REM #  UGA consoles.
REM #
REM # set EFI_WIN_NT_CONSOLE="Bus Driver Console Window"
echo on
set EFI_WIN_NT_UGA="UGA Window 1!UGA Window 2"
@echo off

REM #
REM # Set FD information
REM #
set EFI_FIRMWARE_VOLUMES=..\Fv\FvRecovery.fd
@echo off

REM #
REM # These directories will show up as file systems with no Block IO
REM #
echo on
set EFI_WIN_NT_FILE_SYSTEM=.!%EDK_SOURCE%\Other\Maintained\Application\Shell\bin\ia32\Apps
@echo off

REM #
REM # The first region will be tested by PEI and handed to DXE
REM #  as tested memory. Regions are seperated by ! and other
REM #  regions are passed into DXE as untested memory
REM #
echo on
set EFI_MEMORY_SIZE=64!64

set EFI_BOOT_MODE=1
set EFI_WIN_NT_CPU_MODEL=Intel(R) Processor Model
set EFI_WIN_NT_CPU_SPEED=3000