/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinkedList.h

Abstract:

  This implementation of a linked list provides data structures for the
  list itself and for list nodes.  It provides operations for initializing
  the list, modifying the list, and walking the list.  
  
--*/

// Prevent multiple includes in the same source file
#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_


typedef struct _EFI_LIST_ENTRY {
  struct _EFI_LIST_ENTRY  *ForwardLink;
  struct _EFI_LIST_ENTRY  *BackLink;
} EFI_LIST_ENTRY;

typedef EFI_LIST_ENTRY EFI_LIST;      
typedef EFI_LIST_ENTRY EFI_LIST_NODE;

#define INITIALIZE_LIST_HEAD_VARIABLE(ListHead)  {&ListHead, &ListHead}

//
//  EFI_FIELD_OFFSET - returns the byte offset to a field within a structure
//
   
#define EFI_FIELD_OFFSET(TYPE,Field) ((UINTN)(&(((TYPE *) 0)->Field)))

//
// A lock structure
//

typedef struct {
    EFI_TPL     Tpl;
    EFI_TPL     OwnerTpl;
    UINTN       Lock;
} FLOCK;

VOID
InitializeListHead (
  EFI_LIST_ENTRY       *List
  );

BOOLEAN
IsListEmpty (
  EFI_LIST_ENTRY  *List
  );

VOID
RemoveEntryList (
  EFI_LIST_ENTRY  *Entry
  );

VOID
InsertTailList (
  EFI_LIST_ENTRY  *ListHead,
  EFI_LIST_ENTRY  *Entry
  );

VOID
InsertHeadList (
  EFI_LIST_ENTRY  *ListHead,
  EFI_LIST_ENTRY  *Entry
  );

VOID
SwapListEntries (
  EFI_LIST_ENTRY  *Entry1,
  EFI_LIST_ENTRY  *Entry2
  );

EFI_LIST_ENTRY *
GetFirstNode (
  EFI_LIST_ENTRY  *List 
  );

EFI_LIST_ENTRY *
GetNextNode (
  EFI_LIST_ENTRY  *List,
  EFI_LIST_ENTRY  *Node
  );

BOOLEAN 
IsNull ( 
  EFI_LIST_ENTRY  *List,
  EFI_LIST_ENTRY  *Node 
  );

BOOLEAN 
IsNodeAtEnd ( 
  EFI_LIST_ENTRY  *List, 
  EFI_LIST_ENTRY  *Node 
  );

#endif
