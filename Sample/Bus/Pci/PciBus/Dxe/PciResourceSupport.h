/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciResourceSupport.h
  
Abstract:

  

Revision History

--*/

#ifndef _EFI_PCI_RESOURCE_SUPPORT_H
#define _EFI_PCI_RESOURCE_SUPPORT_H

#define RESERVED_RESOURCE_SIGNATURE   EFI_SIGNATURE_32 ('r','s','v','d')

typedef struct{
  UINT64                    Base;
  UINT64                    Length;
  PCI_BAR_TYPE              ResType;
} PCI_RESERVED_RESOURCE_NODE;

typedef struct{
  UINT32                            Signature;
  EFI_LIST_ENTRY                    Link;
  PCI_RESERVED_RESOURCE_NODE        Node;
} PCI_RESERVED_RESOURCE_LIST;

#define RESOURCED_LIST_FROM_NODE(a) \
  CR (a, PCI_RESERVED_RESOURCE_LIST, Node, RESERVED_RESOURCE_SIGNATURE)

#define RESOURCED_LIST_FROM_LINK(a) \
  CR (a, PCI_RESERVED_RESOURCE_LIST, Link, RESERVED_RESOURCE_SIGNATURE)



typedef enum {
  PciResUsageTypical = 0,
  PciResUsagePadding,
  PciResUsageOptionRomProcessing
} PCI_RESOURCE_USAGE;



#define PCI_RESOURCE_SIGNATURE   EFI_SIGNATURE_32 ('p','c','r','c')

typedef struct {
  UINT32                    Signature;
  EFI_LIST_ENTRY            Link;
  EFI_LIST_ENTRY            ChildList;
  PCI_IO_DEVICE             *PciDev;
  UINT64                    Alignment;
  UINT64                    Offset;
  UINT8                     Bar;
  PCI_BAR_TYPE              ResType;
  UINT64                    Length;
  BOOLEAN                   Reserved;
  PCI_RESOURCE_USAGE        ResourceUsage;
} PCI_RESOURCE_NODE;

#define RESOURCE_NODE_FROM_LINK(a) \
  CR (a, PCI_RESOURCE_NODE, Link, PCI_RESOURCE_SIGNATURE)


EFI_STATUS
SkipVGAAperture (
   OUT UINT64   *Start,
   IN  UINT64   Length
);

EFI_STATUS
SkipIsaAliasAperture (
   OUT UINT64   *Start,
   IN  UINT64   Length
);

EFI_STATUS 
InsertResourceNode (
  PCI_RESOURCE_NODE *Bridge,
  PCI_RESOURCE_NODE *ResNode
);
  
EFI_STATUS 
MergeResourceTree (
  PCI_RESOURCE_NODE *Dst,
  PCI_RESOURCE_NODE *Res,
  BOOLEAN           TypeMerge
);

EFI_STATUS 
CalculateApertureIo16 (
  IN PCI_RESOURCE_NODE *Bridge 
);

EFI_STATUS 
CalculateResourceAperture (
  IN PCI_RESOURCE_NODE *Bridge 
);

EFI_STATUS 
GetResourceFromDevice(
  PCI_IO_DEVICE     *PciDev,
  PCI_RESOURCE_NODE *IoNode,
  PCI_RESOURCE_NODE *Mem32Node,
  PCI_RESOURCE_NODE *PMem32Node,
  PCI_RESOURCE_NODE *Mem64Node,
  PCI_RESOURCE_NODE *PMem64Node
);

PCI_RESOURCE_NODE*
CreateResourceNode (
   IN PCI_IO_DEVICE         *PciDev ,
   IN UINT64                Length,
   IN UINT64                Alignment,
   IN UINT8                 Bar,
   IN PCI_BAR_TYPE          ResType,
   IN PCI_RESOURCE_USAGE    ResUsage
);

EFI_STATUS CreateResourceMap(
  IN PCI_IO_DEVICE     *Bridge,
  IN PCI_RESOURCE_NODE *IoNode,
  IN PCI_RESOURCE_NODE *Mem32Node,
  IN PCI_RESOURCE_NODE *PMem32Node,
  IN PCI_RESOURCE_NODE *Mem64Node,
  IN PCI_RESOURCE_NODE *PMem64Node
);

EFI_STATUS
ResourcePaddingPolicy (
  PCI_IO_DEVICE     *PciDev,
  PCI_RESOURCE_NODE *IoNode,
  PCI_RESOURCE_NODE *Mem32Node,
  PCI_RESOURCE_NODE *PMem32Node,
  PCI_RESOURCE_NODE *Mem64Node,
  PCI_RESOURCE_NODE *PMem64Node
);

EFI_STATUS 
DegradeResource (
  IN PCI_IO_DEVICE *Bridge,
  IN PCI_RESOURCE_NODE *Mem32Node,
  IN PCI_RESOURCE_NODE *PMem32Node,
  IN PCI_RESOURCE_NODE *Mem64Node,
  IN PCI_RESOURCE_NODE *PMem64Node
);

BOOLEAN 
BridgeSupportResourceDecode (
  IN PCI_IO_DEVICE *Bridge,
  IN UINT32        Decode
);

EFI_STATUS 
ProgramResource (
  IN UINT64            Base,
  IN PCI_RESOURCE_NODE *Bridge
);

EFI_STATUS 
ProgramBar (
  IN UINT64            Base,
  IN PCI_RESOURCE_NODE *Node
);

EFI_STATUS 
ProgramPpbApperture (
  IN UINT64            Base,
  IN PCI_RESOURCE_NODE *Node 
);

EFI_STATUS
ProgrameUpstreamBridgeForRom (
  IN PCI_IO_DEVICE   *PciDevice,  
  IN UINT32          OptionRomBase,
  IN BOOLEAN         Enable
);

BOOLEAN
ResourceRequestExisted (
  IN PCI_RESOURCE_NODE *Bridge
);

EFI_STATUS 
InitializeResourcePool (
  PCI_RESOURCE_NODE   *ResourcePool,
  PCI_BAR_TYPE        ResourceType
);

EFI_STATUS 
GetResourceMap(
  PCI_IO_DEVICE     *PciDev,
  PCI_RESOURCE_NODE **IoBridge,
  PCI_RESOURCE_NODE **Mem32Bridge,
  PCI_RESOURCE_NODE **PMem32Bridge,
  PCI_RESOURCE_NODE **Mem64Bridge,
  PCI_RESOURCE_NODE **PMem64Bridge,
  PCI_RESOURCE_NODE  *IoPool,
  PCI_RESOURCE_NODE  *Mem32Pool,
  PCI_RESOURCE_NODE  *PMem32Pool,
  PCI_RESOURCE_NODE  *Mem64Pool,
  PCI_RESOURCE_NODE  *PMem64Pool
);

EFI_STATUS 
DestroyResourceTree (
   IN PCI_RESOURCE_NODE *Bridge
);

EFI_STATUS
RecordReservedResource (
  IN UINT64         Base,
  IN UINT64         Length,
  IN PCI_BAR_TYPE   ResType,
  IN PCI_IO_DEVICE  *Bridge
);
  
EFI_STATUS 
ResourcePaddingForCardBusBridge(
  PCI_IO_DEVICE     *PciDev,
  PCI_RESOURCE_NODE *IoNode,
  PCI_RESOURCE_NODE *Mem32Node,
  PCI_RESOURCE_NODE *PMem32Node,
  PCI_RESOURCE_NODE *Mem64Node,
  PCI_RESOURCE_NODE *PMem64Node
);
  
EFI_STATUS 
ProgramP2C (
  IN UINT64 Base,
  IN PCI_RESOURCE_NODE *Node
);
  
EFI_STATUS
ApplyResourcePadding (
  PCI_IO_DEVICE     *PciDev,
  PCI_RESOURCE_NODE *IoNode,
  PCI_RESOURCE_NODE *Mem32Node,
  PCI_RESOURCE_NODE *PMem32Node,
  PCI_RESOURCE_NODE *Mem64Node,
  PCI_RESOURCE_NODE *PMem64Node
);

VOID GetResourcePaddingPpb(
  IN  PCI_IO_DEVICE                  *PciIoDevice
);

#endif
