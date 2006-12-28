;------------------------------------------------------------------------------
;*
;*   Copyright 2006, Intel Corporation                                                         
;*   All rights reserved. This program and the accompanying materials                          
;*   are licensed and made available under the terms and conditions of the BSD License         
;*   which accompanies this distribution.  The full text of the license may be found at        
;*   http://opensource.org/licenses/bsd-license.php                                            
;*                                                                                             
;*   THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
;*   WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
;*   
;*    bootsect.asm
;*  
;*   Abstract:
;*
;------------------------------------------------------------------------------

        .model  small
;       .dosseg
        .stack
        .486p
        .code

FAT_DIRECTORY_ENTRY_SIZE  EQU     020h
FAT_DIRECTORY_ENTRY_SHIFT EQU     5
BLOCK_SIZE                EQU     0200h
BLOCK_MASK                EQU     01ffh
BLOCK_SHIFT               EQU     9

        org 0h
Ia32Jump:
  jmp   BootSectorEntryPoint  ; JMP inst                  - 3 bytes
  nop

OemId             db  "INTEL   "    ; OemId               - 8 bytes
SectorSize        dw  0200h         ; Sector Size         - 16 bits
SectorsPerCluster db  01h           ; Sector Per Cluster  - 8 bits
ReservedSectors   dw  01h           ; Reserved Sectors    - 16 bits
NoFats            db  02h           ; Number of FATs      - 8 bits
RootEntries       dw  00e0h         ; Root Entries        - 16 bits
Sectors           dw  0b40h         ; Number of Sectors   - 16 bits
Media             db  0f0h          ; Media               - 8 bits  - ignored
SectorsPerFat     dw  0009h         ; Sectors Per FAT     - 16 bits
SectorsPerTrack   dw  0012h         ; Sectors Per Track   - 16 bits - ignored
Heads             dw  0002h         ; Heads               - 16 bits - ignored
HiddenSectors     dd  0000h         ; Hidden Sectors      - 32 bits - ignored
LargeSectors      dd  0000h         ; Large Sectors       - 32 bits 
PhysicalDrive     db  00h           ; PhysicalDriveNumber - 8 bits  - ignored
CurrentHead       db  00h           ; Current Head        - 8 bits
Signature         db  29h           ; Signature           - 8 bits  - ignored
Id                db  "    "        ; Id                  - 4 bytes
FatLabel          db  "EFI FLOPPY " ; Label               - 11 bytes
SystemId          db  "FAT12   "    ; SystemId            - 8 bytes

BootSectorEntryPoint:
        ASSUME  ds:@code
        ASSUME  ss:@code

; ****************************************************************************
; Start Print
; ****************************************************************************

    mov  ax,0b800h
    mov  es,ax
    mov  ax, 07c0h
    mov  ds, ax
    lea  si, cs:[StartString]
    mov  cx, 10
    mov  di, 160
    rep  movsw 

; ****************************************************************************
; Print over
; ****************************************************************************

  mov   ax,cs         ; ax = 0
  mov   ss,ax         ; ss = 0
  add   ax,1000h
  mov   ds,ax

  mov   sp,07c00h     ; sp = 0x7c00
  mov   bp,sp         ; bp = 0x7c00

  mov   ah,8          ; ah = 8 - Get Drive Parameters Function
  mov   dl,0          ; dl = 0 - Floppy Drive
  int   13h           ; Get Drive Parameters
  xor   ax,ax         ; ax = 0
  mov   al,dh         ; al = dh
  and   al,03fh       ; MaxHead = (al & 0x3f)
  inc   al            ; MaxHead = (al & 0x3f) + 1
  push  ax            ; 0000:7bfe = MaxHead + 1
  mov   al,cl         ; al = cl
  and   al,03fh       ; MaxSector = al & 0x3f
  push  ax            ; 0000:7bfc = MaxSector

  cmp   word ptr [bp+SectorSignature],0aa55h  ; Verify Boot Sector Signature
  jne   BadBootSector

  mov   cx,word ptr [bp+RootEntries]      ; cx = RootEntries
  shl   cx,FAT_DIRECTORY_ENTRY_SHIFT      ; cx = cx * 32 = cx * sizeof(FAT_DIRECTORY_ENTRY) = Size of Root Directory in bytes
  mov   bx,cx                             ; bx = size of the Root Directory in bytes
  and   bx,BLOCK_MASK                     ; See if it is an even number of sectors long
  jne   BadBootSector                     ; If is isn't, then the boot sector is bad.
  mov   bx,cx                             ; bx = size of the Root Directory in bytes
  shr   bx,BLOCK_SHIFT                    ; bx = size of Root Directory in sectors
  mov   di,0                              ; Store directory in es:di = 1000:0000
  mov   al,byte ptr [bp+NoFats]           ; al = NoFats
  xor   ah,ah                             ; ah = 0  ==> ax = NoFats
  mul   word ptr [bp+SectorsPerFat]       ; ax = NoFats * SectorsPerFat
  add   ax,word ptr [bp+ReservedSectors]  ; ax = NoFats * SectorsPerFat + ReservedSectors = RootLBA
  push  ds
  pop   es
  call  ReadBlocks                        ; Read entire Root Directory
  add   ax,bx                             ; ax = NoFats * SectorsPerFat + ReservedSectors + RootDirSectors = FirstClusterLBA
  mov   word ptr [bp],ax                  ; Save FirstClusterLBA for later use
FindEFILDR:
  cmp   dword ptr [di],04c494645h         ; Compare to "EFIL"
  jne   NotMatchingEFILDR
  cmp   dword ptr [di+4],020205244h       ; Compare to "DR  "
  jne   NotMatchingEFILDR
  mov   ax,02020h                         ; ax = "  "
  cmp   word ptr [di+8],ax                ; Compare to "  "
  jne   NotMatchingEFILDR
  cmp   word ptr [di+9],ax                ; Compare to "  "
  jne   NotMatchingEFILDR
  mov   al, byte ptr [di+11]
  and   al,058h
  je    FoundEFILDR
NotMatchingEFILDR:
  add   di,FAT_DIRECTORY_ENTRY_SIZE       ; Increment di
  sub   cx,FAT_DIRECTORY_ENTRY_SIZE       ; Decrement cx
  jne   FindEFILDR
  jmp   NotFoundEFILDR

FoundEFILDR:
    mov     cx, word ptr [di+26]                ; cx = FileCluster for EFILDR
    mov     ax,cs                               ; Destination = 2000:0000
    add     ax,2000h
    mov     es,ax
    xor     di,di
ReadFirstClusterOfEFILDR:
    mov     ax,cx                               ; ax = StartCluster
    sub     ax,2                                ; ax = StartCluster - 2
    xor     bh,bh                               
    mov     bl,byte ptr [bp+SectorsPerCluster]  ; bx = SectorsPerCluster
    mul     bx                                  ; ax = (StartCluster - 2) * SectorsPerCluster
    add     ax, word ptr [bp]                   ; ax = FirstClusterLBA + (StartCluster-2)*SectorsPerCluster
    xor     bh,bh
    mov     bl,byte ptr [bp+SectorsPerCluster]  ; bx = Number of Sectors in a cluster
    push    es
    call    ReadBlocks
    pop     ax
JumpIntoFirstSectorOfEFILDR:
    mov     word ptr [bp+JumpSegment],ax
JumpFarInstruction:
    db      0eah
JumpOffset:
    dw      0000h
JumpSegment:
    dw      2000h

; ****************************************************************************
; ReadBlocks - Reads a set of blocks from a block device
;
; AX    = Start LBA
; BX    = Number of Blocks to Read
; ES:DI = Buffer to store sectors read from disk
; ****************************************************************************

; cx = Blocks
; bx = NumberOfBlocks
; si = StartLBA

ReadBlocks:
    pusha
    mov     si,ax                               ; si = Start LBA
    mov     cx,bx                               ; cx = Number of blocks to read
ReadCylinderLoop:
    mov     bp,07bfch                           ; bp = 0x7bfc
    mov     ax,si                               ; ax = Start LBA
    xor     dx,dx                               ; dx = 0
    div     word ptr [bp]                       ; ax = StartLBA / MaxSector
    inc     dx                                  ; dx = (StartLBA % MaxSector) + 1

    mov     bx,word ptr [bp]                    ; bx = MaxSector
    sub     bx,dx                               ; bx = MaxSector - Sector
    inc     bx                                  ; bx = MaxSector - Sector + 1
    cmp     cx,bx                               ; Compare (Blocks) to (MaxSector - Sector + 1)
    jg      LimitTransfer
    mov     bx,cx                               ; bx = Blocks
LimitTransfer:
    push    cx
    mov     cl,dl                               ; cl = (StartLBA % MaxSector) + 1 = Sector
    xor     dx,dx                               ; dx = 0
    div     word ptr [bp+2]                     ; ax = ax / (MaxHead + 1) = Cylinder  
                                                ; dx = ax % (MaxHead + 1) = Head

    push    bx                                  ; Save number of blocks to transfer
    mov     dh,dl                               ; dh = Head
    mov     dl,0                                ; dl = Drive Number
    mov     ch,al                               ; ch = Cylinder
    mov     al,bl                               ; al = Blocks
    mov     ah,2                                ; ah = Function 2
    mov     bx,di                               ; es:bx = Buffer address
    int     013h
    jc      DiskError
    pop     bx
    pop     cx
    add     si,bx                               ; StartLBA = StartLBA + NumberOfBlocks
    sub     cx,bx                               ; Blocks = Blocks - NumberOfBlocks
    mov     ax,es
    shl     bx,(BLOCK_SHIFT-4)
    add     ax,bx
    mov     es,ax                               ; es:di = es:di + NumberOfBlocks*BLOCK_SIZE
    cmp     cx,0
    jne     ReadCylinderLoop
    popa
    ret

; ****************************************************************************
; ERROR Condition:
; ****************************************************************************

BadBootSector:
NotFoundEFILDR:
DiskError:
    mov  ax,0b800h
    mov  es,ax
    mov  ax, 07c0h
    mov  ds, ax
    lea  si, cs:[ErrorString]
    mov  cx, 10
    mov  di, 320
    rep  movsw 
Halt:
    jmp   Halt

StartString:
    db 'B', 0ch, 'o', 0ch, 'o', 0ch, 't', 0ch, 'S', 0ch, 't', 0ch, 'a', 0ch, 'r', 0ch, 't', 0ch, '!', 0ch
ErrorString:
    db 'B', 0ch, 'o', 0ch, 'o', 0ch, 't', 0ch, 'E', 0ch, 'r', 0ch, 'r', 0ch, 'o', 0ch, 'r', 0ch, '!', 0ch

; ****************************************************************************
; Sector Signature
; ****************************************************************************

  org 01feh
SectorSignature:
  dw        0aa55h      ; Boot Sector Signature

  end 
  
