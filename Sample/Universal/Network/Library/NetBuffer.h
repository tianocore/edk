/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  NetBuffer.h

Abstract:


--*/

#ifndef _NET_BUFFER_H_
#define _NET_BUFFER_H_

#include "NetLib.h"

enum {
  //
  //Various signatures
  //
  NET_BUF_SIGNATURE    = EFI_SIGNATURE_32 ('n', 'b', 'u', 'f'),
  NET_VECTOR_SIGNATURE = EFI_SIGNATURE_32 ('n', 'v', 'e', 'c'),
  NET_QUE_SIGNATURE    = EFI_SIGNATURE_32 ('n', 'b', 'q', 'u'),

  
  NET_PROTO_DATA       = 64,   // Opaque buffer for protocols
  NET_BUF_HEAD         = 1,    // Trim or allocate space from head
  NET_BUF_TAIL         = 0,    // Trim or allocate space from tail
  NET_VECTOR_OWN_FIRST = 0x01, // We allocated the 1st block in the vector
};

#define NET_CHECK_SIGNATURE(PData, SIGNATURE) \
  ASSERT (((PData) != NULL) && ((PData)->Signature == (SIGNATURE)))

#define NET_SWAP_SHORT(Value) \
  ((((Value) & 0xff) << 8) | (((Value) >> 8) & 0xff))

//
// Single memory block in the vector. 
//
typedef struct {
  UINT32              Len;        // The block's length
  UINT8               *Bulk;      // The block's Data
} NET_BLOCK;

typedef VOID (*NET_VECTOR_EXT_FREE) (VOID *Arg);

//
//NET_VECTOR contains several blocks to hold all packet's
//fragments and other house-keeping stuff for sharing. It 
//doesn't specify the where actual packet fragment begins.
//
typedef struct {
  UINT32              Signature;
  INTN                RefCnt;  // Reference count to share NET_VECTOR.
  NET_VECTOR_EXT_FREE Free;    // external function to free NET_VECTOR
  VOID                *Arg;    // opeque argument to Free
  UINT32              Flag;    // Flags, NET_VECTOR_OWN_FIRST
  UINT32              Len;     // Total length of the assocated BLOCKs

  UINT32              BlockNum;
  NET_BLOCK           Block[1];
} NET_VECTOR;

//
//NET_BLOCK_OP operate on the NET_BLOCK, It specifies 
//where the actual fragment begins and where it ends
//
typedef struct {
  UINT8               *BlockHead;   // Block's head, or the smallest valid Head
  UINT8               *BlockTail;   // Block's tail. BlockTail-BlockHead=block length
  UINT8               *Head;        // 1st byte of the data in the block
  UINT8               *Tail;        // Tail of the data in the block, Tail-Head=Size
  UINT32              Size;         // The size of the data
} NET_BLOCK_OP;


//
//NET_BUF is the buffer manage structure used by the 
//network stack. Every network packet may be fragmented,
//and contains multiple fragments. The Vector points to
//memory blocks used by the each fragment, and BlockOp 
//specifies where each fragment begins and ends.
//
//It also contains a opaque area for protocol to store
//per-packet informations. Protocol must be caution not
//to overwrite the members after that.
//
typedef struct {
  UINT32              Signature;
  INTN                RefCnt;     
  NET_LIST_ENTRY      List;       // The List this NET_BUF is on

  IP4_HEAD            *Ip;        // Network layer header, for fast access
  TCP_HEAD            *Tcp;       // Transport layer header, for fast access
  UINT8               ProtoData [NET_PROTO_DATA]; //Protocol specific data
                                    
  NET_VECTOR          *Vector;    // The vector containing the packet
  
  UINT32              BlockOpNum; // Total number of BlockOp in the buffer
  UINT32              TotalSize;  // Total size of the actual packet
  NET_BLOCK_OP        BlockOp[1]; // Specify the position of actual packet
} NET_BUF;


//
//A queue of NET_BUFs, It is just a thin extension of
//NET_BUF functions.
//
typedef struct {
  UINT32              Signature;
  INTN                RefCnt;
  NET_LIST_ENTRY      List;       // The List this buffer queue is on
  
  NET_LIST_ENTRY      BufList;    // list of queued buffers
  UINT32              BufSize;    // total length of DATA in the buffers
  UINT32              BufNum;     // total number of buffers on the chain
} NET_BUF_QUEUE;

//
// Pseudo header for TCP and UDP checksum
//
#pragma pack(1)
typedef struct {
  IP4_ADDR            SrcIp;
  IP4_ADDR            DstIp;
  UINT8               Reserved;
  UINT8               Protocol;
  UINT16              Len;
} NET_PSEUDO_HDR;
#pragma pack()

//
// The fragment entry table used in network interfaces. This is
// the same as NET_BLOCK now. Use two different to distinguish
// the two in case that NET_BLOCK be enhanced later.
//
typedef struct {
  UINT32              Len;        
  UINT8               *Bulk;      
} NET_FRAGMENT;

#define NET_GET_REF(PData)      ((PData)->RefCnt++)
#define NET_PUT_REF(PData)      ((PData)->RefCnt--)
#define NETBUF_FROM_PROTODATA(Info) _CR((Info), NET_BUF, ProtoData)

#define NET_BUF_SHARED(Buf) \
  (((Buf)->RefCnt > 1) || ((Buf)->Vector->RefCnt > 1))

#define NET_VECTOR_SIZE(BlockNum) \
  (sizeof (NET_VECTOR) + ((BlockNum) - 1) * sizeof (NET_BLOCK))

#define NET_BUF_SIZE(BlockOpNum)  \
  (sizeof (NET_BUF) + ((BlockOpNum) - 1) * sizeof (NET_BLOCK_OP))

#define NET_HEADSPACE(BlockOp)  \
  (UINTN)((BlockOp)->Head - (BlockOp)->BlockHead)

#define NET_TAILSPACE(BlockOp)  \
  (UINTN)((BlockOp)->BlockTail - (BlockOp)->Tail)

NET_BUF  *
NetbufAlloc (
  IN UINT32                 Len
  );

VOID
NetbufFree (
  IN NET_BUF                *Nbuf
  );


UINT8  *
NetbufGetByte (
  IN  NET_BUF               *Nbuf,
  IN  UINT32                Offset, 
  OUT UINT32                *Index      OPTIONAL
  );

NET_BUF  *
NetbufClone (
  IN NET_BUF                *Nbuf
  );

NET_BUF  *
NetbufDuplicate (
  IN NET_BUF                *Nbuf,
  IN NET_BUF                *Duplicate    OPTIONAL,
  IN UINT32                 HeadSpace
  );

NET_BUF  *
NetbufGetFragment (
  IN NET_BUF                *Nbuf,
  IN UINT32                 Offset,
  IN UINT32                 Len,
  IN UINT32                 HeadSpace
  );

VOID
NetbufReserve (
  IN NET_BUF                *Nbuf,
  IN UINT32                 Len
  );

UINT8  *
NetbufAllocSpace (
  IN NET_BUF                *Nbuf,
  IN UINT32                 Len,
  IN BOOLEAN                FromHead
  );

UINT32
NetbufTrim (
  IN NET_BUF                *Nbuf,
  IN UINT32                 Len,
  IN BOOLEAN                FromHead
  );

UINT32
NetbufCopy (
  IN NET_BUF                *Nbuf,
  IN UINT32                 Offset,
  IN UINT32                 Len,
  IN UINT8                  *Dest
  );

NET_BUF  *
NetbufFromExt (
  IN NET_FRAGMENT           *ExtFragment,
  IN UINT32                 ExtNum,
  IN UINT32                 HeadSpace,
  IN UINT32                 HeadLen,
  IN NET_VECTOR_EXT_FREE    ExtFree,
  IN VOID                   *Arg          OPTIONAL
  );

EFI_STATUS
NetbufBuildExt (
  IN NET_BUF                *Nbuf,             
  IN NET_FRAGMENT           *ExtFragment,
  IN UINT32                 *ExtNum
  );

NET_BUF  *
NetbufFromBufList (
  IN NET_LIST_ENTRY         *BufList,
  IN UINT32                 HeadSpace,
  IN UINT32                 HeaderLen,
  IN NET_VECTOR_EXT_FREE    ExtFree,
  IN VOID                   *Arg                OPTIONAL
  );

VOID
NetbufFreeList (
  IN NET_LIST_ENTRY         *Head
  );

VOID
NetbufQueInit (
  IN NET_BUF_QUEUE          *NbufQue
  );

NET_BUF_QUEUE  *
NetbufQueAlloc (
  VOID
  );

VOID
NetbufQueFree (
  IN NET_BUF_QUEUE          *NbufQue
  );

NET_BUF  *
NetbufQueRemove (
  IN NET_BUF_QUEUE          *NbufQue
  );

VOID
NetbufQueAppend (
  IN NET_BUF_QUEUE          *NbufQue,
  IN NET_BUF                *Nbuf
  );

UINT32 
NetbufQueCopy (
  IN NET_BUF_QUEUE          *NbufQue,
  IN UINT32                 Offset,
  IN UINT32                 Len,
  IN UINT8                  *Dest
  );

UINT32 
NetbufQueTrim (
  IN NET_BUF_QUEUE          *NbufQue,
  IN UINT32                 Len
  );

VOID
NetbufQueFlush (
  IN NET_BUF_QUEUE          *NbufQue
  );

UINT16
NetblockChecksum (
  IN UINT8                  *Bulk,
  IN UINT32                 Len
  );

UINT16
NetAddChecksum (
  IN UINT16                 Checksum1,
  IN UINT16                 Checksum2
  );

UINT16
NetbufChecksum (
  IN NET_BUF                *Nbuf
  );

UINT16
NetPseudoHeadChecksum (
  IN IP4_ADDR               Src,
  IN IP4_ADDR               Dst,
  IN UINT8                  Proto,
  IN UINT16                 Len
  );
#endif
