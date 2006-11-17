/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  NetLib.c

Abstract:


--*/

#include "NetLib.h"

//
// All the supported IP4 maskes in host byte order.
//
IP4_ADDR  mIp4AllMasks[IP4_MASK_NUM] = {
  0x00000000,
  0x80000000,
  0xC0000000,
  0xE0000000,
  0xF0000000,
  0xF8000000,
  0xFC000000,
  0xFE000000,

  0xFF000000,
  0xFF800000,
  0xFFC00000,
  0xFFE00000,
  0xFFF00000,
  0xFFF80000,
  0xFFFC0000,
  0xFFFE0000,

  0xFFFF0000,
  0xFFFF8000,
  0xFFFFC000,
  0xFFFFE000,
  0xFFFFF000,
  0xFFFFF800,
  0xFFFFFC00,
  0xFFFFFE00,

  0xFFFFFF00,
  0xFFFFFF80,
  0xFFFFFFC0,
  0xFFFFFFE0,
  0xFFFFFFF0,
  0xFFFFFFF8,
  0xFFFFFFFC,
  0xFFFFFFFE,
  0xFFFFFFFF,
};

INTN
NetGetMaskLength (
  IN IP4_ADDR               NetMask
  )
/*++

Routine Description:

  Return the length of the mask. If the mask is invalid,
  return the invalid length 33, which is IP4_MASK_NUM.
  NetMask is in the host byte order.

Arguments:

  NetMask  - The netmask to get the length from

Returns:

  The length of the netmask, IP4_MASK_NUM if the mask isn't 
  supported.

--*/
{
  INTN                      Index;

  for (Index = 0; Index < IP4_MASK_NUM; Index++) {
    if (NetMask == mIp4AllMasks[Index]) {
      break;
    }
  }

  return Index;
}


INTN
NetGetIpClass (
  IN IP4_ADDR               Addr
  )
/*++

Routine Description:

  Return the class of the address, such as class a, b, c.
  Addr is in host byte order.

Arguments:

  Addr  - The address to get the class from

Returns:

  IP address class, such as IP4_ADDR_CLASSA

--*/
{
  UINT8                     ByteOne;

  ByteOne = (UINT8) (Addr >> 24);

  if ((ByteOne & 0x80) == 0) {
    return IP4_ADDR_CLASSA;

  } else if ((ByteOne & 0xC0) == 0x80) {
    return IP4_ADDR_CLASSB;

  } else if ((ByteOne & 0xE0) == 0xC0) {
    return IP4_ADDR_CLASSC;

  } else if ((ByteOne & 0xF0) == 0xE0) {
    return IP4_ADDR_CLASSD;

  } else {
    return IP4_ADDR_CLASSE;

  }
}

BOOLEAN
Ip4IsUnicast (
  IN IP4_ADDR               Ip,
  IN IP4_ADDR               NetMask
  )
/*++

Routine Description:

  Check whether the IP is a valid unicast address according to
  the netmask. If NetMask is zero, use the IP address's class to 
  get the default mask.

Arguments:

  Ip      - The IP to check againist
  NetMask - The mask of the IP

Returns:

  TRUE if IP is a valid unicast address on the network, otherwise FALSE

--*/
{
  INTN                      Class;

  Class = NetGetIpClass (Ip);

  if ((Ip == 0) || (Class >= IP4_ADDR_CLASSD)) {
    return FALSE;
  }

  if (NetMask == 0) {
    NetMask = mIp4AllMasks[Class << 3];
  }

  if (((Ip &~NetMask) == ~NetMask) || ((Ip &~NetMask) == 0)) {
    return FALSE;
  }

  return TRUE;
}

UINT32
NetRandomInitSeed (
  VOID
  )
/*++

Routine Description:

  Initialize a random seed using current time.

Arguments:

  None

Returns:

  The random seed initialized with current time.

--*/
{
  EFI_TIME                  Time;
  UINT32                    Seed;

  gRT->GetTime (&Time, NULL);
  Seed = (~Time.Hour << 24 | Time.Second << 16 | Time.Minute << 8 | Time.Day);
  Seed ^= Time.Nanosecond;
  Seed ^= Time.Year << 7;

  return Seed;
}

UINT32
NetGetUint32 (
  IN UINT8                  *Buf
  )
/*++

Routine Description:

  Extract a UINT32 from a byte stream, then convert it to host 
  byte order. Use this function to avoid alignment error.

Arguments:

  Buf - The buffer to extract the UINT32.

Returns:

  The UINT32 extracted.

--*/
{
  UINT32                    Value;
  
  NetCopyMem (&Value, Buf, sizeof (UINT32));
  return NTOHL (Value);
}

VOID
NetPutUint32 (
  IN UINT8                  *Buf,
  IN UINT32                 Data
  )
/*++

Routine Description:

  Put a UINT32 to the byte stream. Convert it from host byte order
  to network byte order before putting.

Arguments:

  Buf   - The buffer to put the UINT32
  Data  - The data to put

Returns:

  None

--*/
{
  Data = HTONL (Data);
  NetCopyMem (Buf, &Data, sizeof (UINT32));
}

NET_LIST_ENTRY *
NetListRemoveHead (
  NET_LIST_ENTRY            *Head
  )
/*++

Routine Description:

  Remove the first entry on the list
  
Arguments:

  Head  - The list header

Returns:

  The entry that is removed from the list, NULL if the list is empty.

--*/
{
  NET_LIST_ENTRY            *First;

  ASSERT (Head != NULL);

  if (NetListIsEmpty (Head)) {
    return NULL;
  }

  First                         = Head->ForwardLink;
  Head->ForwardLink             = First->ForwardLink;
  First->ForwardLink->BackLink  = Head;

  DEBUG_CODE (
    First->ForwardLink  = (EFI_LIST_ENTRY *) EFI_BAD_POINTER;
    First->BackLink     = (EFI_LIST_ENTRY *) EFI_BAD_POINTER;
  )

  return First;
}

NET_LIST_ENTRY *
NetListRemoveTail (
  NET_LIST_ENTRY            *Head
  )
/*++

Routine Description:

  Remove the last entry on the list

Arguments:

  Head  - The list head

Returns:

  The entry that is removed from the list, NULL if the list is empty.

--*/
{
  NET_LIST_ENTRY            *Last;

  ASSERT (Head != NULL);

  if (NetListIsEmpty (Head)) {
    return NULL;
  }

  Last                        = Head->BackLink;
  Head->BackLink              = Last->BackLink;
  Last->BackLink->ForwardLink = Head;

  DEBUG_CODE (
    Last->ForwardLink = (EFI_LIST_ENTRY *) EFI_BAD_POINTER;
    Last->BackLink    = (EFI_LIST_ENTRY *) EFI_BAD_POINTER;
  )

  return Last;
}

VOID
NetListInsertAfter (
  IN NET_LIST_ENTRY         *PrevEntry,
  IN NET_LIST_ENTRY         *NewEntry
  )
/*++

Routine Description:

  Insert the NewEntry after the PrevEntry

Arguments:

  PrevEntry - The previous entry to insert after
  NewEntry  - The new entry to insert

Returns:

  None

--*/
{
  NewEntry->BackLink                = PrevEntry;
  NewEntry->ForwardLink             = PrevEntry->ForwardLink;
  PrevEntry->ForwardLink->BackLink  = NewEntry;
  PrevEntry->ForwardLink            = NewEntry;
}

VOID
NetListInsertBefore (
  IN NET_LIST_ENTRY *PostEntry,
  IN NET_LIST_ENTRY *NewEntry
  )
/*++

Routine Description:

  Insert the NewEntry before the PostEntry

Arguments:

  PostEntry - The entry to insert before
  NewEntry  - The new entry to insert

Returns:

  None

--*/
{
  NewEntry->ForwardLink             = PostEntry;
  NewEntry->BackLink                = PostEntry->BackLink;
  PostEntry->BackLink->ForwardLink  = NewEntry;
  PostEntry->BackLink               = NewEntry;
}

VOID
NetMapInit (
  IN NET_MAP                *Map
  )
/*++

Routine Description:

  Initialize the netmap. Netmap is a reposity to keep the <Key, Value> pairs.

Arguments:

  Map - The netmap to initialize

Returns:

  None

--*/
{
  ASSERT (Map != NULL);

  NetListInit (&Map->Used);
  NetListInit (&Map->Recycled);
  Map->Count = 0;
}

VOID
NetMapClean (
  IN NET_MAP                *Map
  )
/*++

Routine Description:

  To clean up the netmap, that is, release allocated memories.

Arguments:

  Map - The netmap to clean up.

Returns:

  None

--*/
{
  NET_MAP_ITEM              *Item;
  NET_LIST_ENTRY            *Entry;
  NET_LIST_ENTRY            *Next;

  ASSERT (Map != NULL);

  NET_LIST_FOR_EACH_SAFE (Entry, Next, &Map->Used) {
    Item = NET_LIST_USER_STRUCT (Entry, NET_MAP_ITEM, Link);

    NetListRemoveEntry (&Item->Link);
    Map->Count--;

    NetFreePool (Item);
  }

  ASSERT ((Map->Count == 0) && NetListIsEmpty (&Map->Used));

  NET_LIST_FOR_EACH_SAFE (Entry, Next, &Map->Recycled) {
    Item = NET_LIST_USER_STRUCT (Entry, NET_MAP_ITEM, Link);

    NetListRemoveEntry (&Item->Link);
    NetFreePool (Item);
  }

  ASSERT (NetListIsEmpty (&Map->Recycled));
}

BOOLEAN
NetMapIsEmpty (
  IN NET_MAP                *Map
  )
/*++

Routine Description:

  Test whether the netmap is empty

Arguments:

  Map - The net map to test

Returns:

  TRUE if the netmap is empty, otherwise FALSE.

--*/
{
  ASSERT (Map != NULL);
  return (BOOLEAN) (Map->Count == 0);
}

UINTN
NetMapGetCount (
  IN NET_MAP                *Map
  )
/*++

Routine Description:

  Return the number of the <Key, Value> pairs in the netmap.

Arguments:

  Map - The netmap to get the entry number

Returns:

  The entry number in the netmap.

--*/
{
  return Map->Count;
}

STATIC
NET_MAP_ITEM *
NetMapAllocItem (
  IN NET_MAP                *Map
  )
/*++

Routine Description:

  Allocate an item for the netmap. It will try to allocate
  a batch of items and return one.

Arguments:

  Map - The netmap to allocate item for

Returns:

  The allocated item or NULL

--*/
{
  NET_MAP_ITEM              *Item;
  NET_LIST_ENTRY            *Head;
  UINTN                     Index;

  ASSERT (Map != NULL);

  Head = &Map->Recycled;

  if (NetListIsEmpty (Head)) {
    for (Index = 0; Index < NET_MAP_INCREAMENT; Index++) {
      Item = NetAllocatePool (sizeof (NET_MAP_ITEM));

      if (Item == NULL) {
        if (Index == 0) {
          return NULL;
        }

        break;
      }

      NetListInsertHead (Head, &Item->Link);
    }
  }

  Item = NET_LIST_HEAD (Head, NET_MAP_ITEM, Link);
  NetListRemoveHead (Head);

  return Item;
}

EFI_STATUS
NetMapInsertHead (
  IN NET_MAP                *Map,
  IN VOID                   *Key,
  IN VOID                   *Value    OPTIONAL
  )
/*++

Routine Description:

  Allocate an item to save the <Key, Value> pair to the head of the netmap.

Arguments:

  Map   - The netmap to insert into
  Key   - The user's key 
  Value - The user's value for the key

Returns:

  EFI_OUT_OF_RESOURCES - Failed to allocate the memory for the item
  EFI_SUCCESS          - The item is inserted to the head

--*/
{
  NET_MAP_ITEM              *Item;

  ASSERT (Map != NULL);

  Item = NetMapAllocItem (Map);
  
  if (Item == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Item->Key   = Key;
  Item->Value = Value;
  NetListInsertHead (&Map->Used, &Item->Link);

  Map->Count++;
  return EFI_SUCCESS;
}

EFI_STATUS
NetMapInsertTail (
  IN NET_MAP                *Map,
  IN VOID                   *Key,
  IN VOID                   *Value    OPTIONAL
  )
/*++

Routine Description:

  Allocate an item to save the <Key, Value> pair to the tail of the netmap.

Arguments:

  Map   - The netmap to insert into
  Key   - The user's key 
  Value - The user's value for the key

Returns:

  EFI_OUT_OF_RESOURCES - Failed to allocate the memory for the item
  EFI_SUCCESS          - The item is inserted to the tail

--*/
{
  NET_MAP_ITEM              *Item;

  ASSERT (Map != NULL);

  Item = NetMapAllocItem (Map);
  
  if (Item == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Item->Key   = Key;
  Item->Value = Value;
  NetListInsertTail (&Map->Used, &Item->Link);

  Map->Count++;

  return EFI_SUCCESS;
}

STATIC
BOOLEAN
NetItemInMap (
  IN NET_MAP                *Map,
  IN NET_MAP_ITEM           *Item
  )
/*++

Routine Description:

  Check whther the item is in the Map

Arguments:

  Map   - The netmap to search within
  Item  - The item to search

Returns:

  TRUE if the item is in the netmap, otherwise FALSE.

--*/
{
  NET_LIST_ENTRY            *ListEntry;

  NET_LIST_FOR_EACH (ListEntry, &Map->Used) {
    if (ListEntry == &Item->Link) {
      return TRUE;
    }
  }

  return FALSE;
}

NET_MAP_ITEM *
NetMapFindKey (
  IN  NET_MAP               *Map,
  IN  VOID                  *Key
  )
/*++

Routine Description:

  Find the key in the netmap

Arguments:

  Map - The netmap to search within
  Key - The key to search

Returns:

  The point to the item contains the Key, or NULL if Key isn't in the map.

--*/
{
  NET_LIST_ENTRY          *Entry;
  NET_MAP_ITEM            *Item;

  ASSERT (Map != NULL);

  NET_LIST_FOR_EACH (Entry, &Map->Used) {
    Item = NET_LIST_USER_STRUCT (Entry, NET_MAP_ITEM, Link);
    
    if (Item->Key == Key) {
      return Item;
    }
  }

  return NULL;
}

VOID *
NetMapRemoveItem (
  IN  NET_MAP             *Map,
  IN  NET_MAP_ITEM        *Item,
  OUT VOID                **Value           OPTIONAL
  )
/*++

Routine Description:

  Remove the item from the netmap

Arguments:

  Map   - The netmap to remove the item from
  Item  - The item to remove
  Value - The variable to receive the value if not NULL

Returns:

  The key of the removed item.

--*/
{
  ASSERT ((Map != NULL) && (Item != NULL));
  ASSERT (NetItemInMap (Map, Item));

  NetListRemoveEntry (&Item->Link);
  Map->Count--;
  NetListInsertHead (&Map->Recycled, &Item->Link);

  if (Value != NULL) {
    *Value = Item->Value;
  }

  return Item->Key;
}

VOID *
NetMapRemoveHead (
  IN  NET_MAP               *Map,
  OUT VOID                  **Value         OPTIONAL
  )
/*++

Routine Description:

  Remove the first entry on the netmap

Arguments:

  Map   - The netmap to remove the head from
  Value - The variable to receive the value if not NULL
  
Returns:

  The key of the item removed

--*/
{
  NET_MAP_ITEM  *Item;

  //
  // Often, it indicates a programming error to remove
  // the first entry in an empty list
  //
  ASSERT (Map && !NetListIsEmpty (&Map->Used));

  Item = NET_LIST_HEAD (&Map->Used, NET_MAP_ITEM, Link);
  NetListRemoveEntry (&Item->Link);
  Map->Count--;
  NetListInsertHead (&Map->Recycled, &Item->Link);

  if (Value != NULL) {
    *Value = Item->Value;
  }

  return Item->Key;
}

VOID *
NetMapRemoveTail (
  IN  NET_MAP               *Map,
  OUT VOID                  **Value       OPTIONAL
  )
/*++

Routine Description:

  Remove the last entry on the netmap

Arguments:

  Map   - The netmap to remove the tail from
  Value - The variable to receive the value if not NULL
  
Returns:

  The key of the item removed


--*/
{
  NET_MAP_ITEM              *Item;

  //
  // Often, it indicates a programming error to remove
  // the last entry in an empty list
  //
  ASSERT (Map && !NetListIsEmpty (&Map->Used));

  Item = NET_LIST_TAIL (&Map->Used, NET_MAP_ITEM, Link);
  NetListRemoveEntry (&Item->Link);
  Map->Count--;
  NetListInsertHead (&Map->Recycled, &Item->Link);

  if (Value != NULL) {
    *Value = Item->Value;
  }

  return Item->Key;
}

EFI_STATUS
NetMapIterate (
  IN NET_MAP                *Map,
  IN NET_MAP_CALLBACK       CallBack,
  IN VOID                   *Arg
  )
/*++

Routine Description:

  Iterate through the netmap and call CallBack for each item. It will
  contiue the traverse if CallBack returns EFI_SUCCESS, otherwise, break
  from the loop. It returns the CallBack's last return value. This 
  function is delete safe for the current item.

Arguments:

  Map       - The Map to iterate through
  CallBack  - The callback function to call for each item.
  Arg       - The opaque parameter to the callback

Returns:

  It returns the CallBack's last return value.

--*/
{

  NET_LIST_ENTRY            *Entry;
  NET_LIST_ENTRY            *Next;
  NET_LIST_ENTRY            *Head;
  NET_MAP_ITEM              *Item;
  EFI_STATUS                Result;

  ASSERT ((Map != NULL) && (CallBack != NULL));

  Head = &Map->Used;

  if (NetListIsEmpty (Head)) {
    return EFI_SUCCESS;
  }

  NET_LIST_FOR_EACH_SAFE (Entry, Next, Head) {
    Item   = NET_LIST_USER_STRUCT (Entry, NET_MAP_ITEM, Link);
    Result = CallBack (Map, Item, Arg);

    if (EFI_ERROR (Result)) {
      return Result;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
NetLibDefaultUnload (
  IN EFI_HANDLE             ImageHandle
  )
/*++

Routine Description:

  This is the default unload handle for all the network drivers.

Arguments:

  ImageHandle - The drivers' driver image.

Returns:

  EFI_SUCCESS - The image is unloaded.
  Others      - Failed to unload the image.
  
--*/
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        *DeviceHandleBuffer;
  UINTN                             DeviceHandleCount;
  UINTN                             Index;
  EFI_DRIVER_BINDING_PROTOCOL       *DriverBinding;
  EFI_COMPONENT_NAME_PROTOCOL       *ComponentName;
  EFI_DRIVER_CONFIGURATION_PROTOCOL *DriverConfiguration;
  EFI_DRIVER_DIAGNOSTICS_PROTOCOL   *DriverDiagnostics;

  //
  // Get the list of all the handles in the handle database.
  // If there is an error getting the list, then the unload 
  // operation fails.
  //
  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &DeviceHandleCount,
                  &DeviceHandleBuffer
                  );
  
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Disconnect the driver specified by ImageHandle from all
  // the devices in the handle database.
  //
  for (Index = 0; Index < DeviceHandleCount; Index++) {
    Status = gBS->DisconnectController (
                    DeviceHandleBuffer[Index],
                    ImageHandle,
                    NULL
                    );
  }
  
  //
  // Uninstall all the protocols installed in the driver entry point
  //
  for (Index = 0; Index < DeviceHandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    DeviceHandleBuffer[Index],
                    &gEfiDriverBindingProtocolGuid,
                    &DriverBinding
                    );
    
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (DriverBinding->ImageHandle != ImageHandle) {
      continue;
    }

    gBS->UninstallProtocolInterface (
          ImageHandle,
          &gEfiDriverBindingProtocolGuid,
          DriverBinding
          );
    
    Status = gBS->HandleProtocol (
                    DeviceHandleBuffer[Index],
                    &gEfiComponentNameProtocolGuid,
                    &ComponentName
                    );
    
    if (!EFI_ERROR (Status)) {
      gBS->UninstallProtocolInterface (
            ImageHandle,
            &gEfiComponentNameProtocolGuid,
            ComponentName
            );
    }

    Status = gBS->HandleProtocol (
                    DeviceHandleBuffer[Index],
                    &gEfiDriverConfigurationProtocolGuid,
                    &DriverConfiguration
                    );
    
    if (!EFI_ERROR (Status)) {
      gBS->UninstallProtocolInterface (
            ImageHandle,
            &gEfiDriverConfigurationProtocolGuid,
            DriverConfiguration
            );
    }

    Status = gBS->HandleProtocol (
                    DeviceHandleBuffer[Index],
                    &gEfiDriverDiagnosticsProtocolGuid,
                    &DriverDiagnostics
                    );
    
    if (!EFI_ERROR (Status)) {
      gBS->UninstallProtocolInterface (
            ImageHandle,
            &gEfiDriverDiagnosticsProtocolGuid,
            DriverDiagnostics
            );
    }
  }
  
  //
  // Free the buffer containing the list of handles from the handle database
  //
  if (DeviceHandleBuffer != NULL) {
    gBS->FreePool (DeviceHandleBuffer);
  }

  return EFI_SUCCESS;
}


EFI_STATUS
NetLibCreateServiceChild (
  IN  EFI_HANDLE            Controller,
  IN  EFI_HANDLE            Image,
  IN  EFI_GUID              *ServiceBindingGuid,
  OUT EFI_HANDLE            *ChildHandle
  )
/*++

Routine Description:

  Create a child of the service that is identified by ServiceBindingGuid.

Arguments:

  Controller          - The controller which has the service installed.
  Image               - The image handle used to open service.
  ServiceBindingGuid  - The service's Guid.
  ChildHandle         - The handle to receive the create child

Returns:

  EFI_SUCCESS  - The child is successfully created.
  Others       - Failed to create the child.

--*/
{
  EFI_STATUS                    Status;
  EFI_SERVICE_BINDING_PROTOCOL  *Service;

  
  ASSERT ((ServiceBindingGuid != NULL) && (ChildHandle != NULL));
  
  //
  // Get the ServiceBinding Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  ServiceBindingGuid,
                  (VOID **) &Service,
                  Image,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Create a child
  //
  Status = Service->CreateChild (Service, ChildHandle);
  return Status;
}

EFI_STATUS
NetLibDestroyServiceChild (
  IN  EFI_HANDLE            Controller,
  IN  EFI_HANDLE            Image,
  IN  EFI_GUID              *ServiceBindingGuid,
  IN  EFI_HANDLE            ChildHandle
  )
/*++

Routine Description:

  Destory a child of the service that is identified by ServiceBindingGuid.

Arguments:

  Controller          - The controller which has the service installed.
  Image               - The image handle used to open service.
  ServiceBindingGuid  - The service's Guid.
  ChildHandle         - The child to destory

Returns:

  EFI_SUCCESS  - The child is successfully destoried.
  Others       - Failed to destory the child.

--*/
{
  EFI_STATUS                    Status;
  EFI_SERVICE_BINDING_PROTOCOL  *Service;

  ASSERT (ServiceBindingGuid != NULL);

  //
  // Get the ServiceBinding Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  ServiceBindingGuid,
                  (VOID **) &Service,
                  Image,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // destory the child
  //
  Status = Service->DestroyChild (Service, ChildHandle);
  return Status;
}

EFI_STATUS
NetLibGetMacString (
  IN           EFI_HANDLE  SnpHandle,
  IN           EFI_HANDLE  ImageHandle,
  IN OUT CONST CHAR16      **MacString
  )
/*++

Routine Description:

  Convert the mac address of the simple network protocol installed on
  SnpHandle to a unicode string. Callers are responsible for freeing the
  string storage.

Arguments:

  SnpHandle    - The handle where the simple network protocol is installed on.
  ImageHandle  - The image handle used to act as the agent handle to get the simple
                 network protocol.
  MacString    - The pointer to store the address of the string representation of 
                 the mac address.

Returns:

  EFI_OUT_OF_RESOURCES - There are not enough memory resource.
  other                - Failed to open the simple network protocol.

--*/
{
  EFI_STATUS                   Status;
  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp;
  EFI_SIMPLE_NETWORK_MODE      *Mode;
  CHAR16                       *MacAddress;
  UINTN                        Index;

  *MacString = NULL;

  //
  // Get the Simple Network protocol from the SnpHandle.
  //
  Status = gBS->OpenProtocol (
                  SnpHandle,
                  &gEfiSimpleNetworkProtocolGuid,
                  (VOID **) &Snp,
                  ImageHandle,
                  SnpHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Mode = Snp->Mode;

  //
  // It takes 2 unicode characters to represent a 1 byte binary buffer.
  // Plus one unicode character for the null-terminator.
  //
  MacAddress = NetAllocatePool ((2 * Mode->HwAddressSize + 1) * sizeof (CHAR16));
  if (MacAddress == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Convert the mac address into a unicode string.
  //
  for (Index = 0; Index < Mode->HwAddressSize; Index++) {
    MacAddress[Index * 2]     = NibbleToHexChar (Mode->CurrentAddress.Addr[Index] >> 4);
    MacAddress[Index * 2 + 1] = NibbleToHexChar (Mode->CurrentAddress.Addr[Index]);
  }

  MacAddress[Mode->HwAddressSize * 2] = L'\0';

  *MacString = MacAddress;

  return EFI_SUCCESS;
}

EFI_HANDLE
NetLibGetNicHandle (
  IN EFI_HANDLE             Controller,
  IN EFI_GUID               *ProtocolGuid
  )
/*++

Routine Description:

  Find the UNDI/SNP handle from controller and protocol GUID.
  For example, IP will open a MNP child to transmit/receive 
  packets, when MNP is stopped, IP should also be stopped. IP
  needs to find its own private data which is related the IP's 
  service binding instance that is install on UNDI/SNP handle. 
  Now, the controller is either a MNP or ARP child handle. But
  IP opens these handle BY_DRIVER, use that info, we can get the
  UNDI/SNP handle.

Arguments:

  Controller    - Then protocol handle to check
  ProtocolGuid  - The protocol that is related with the handle.

Returns:

  The UNDI/SNP handle or NULL.

--*/
{
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenBuffer;
  EFI_HANDLE                          Handle;
  EFI_STATUS                          Status;
  UINTN                               OpenCount;
  UINTN                               Index;

  Status = gBS->OpenProtocolInformation (
                  Controller,
                  ProtocolGuid,
                  &OpenBuffer,
                  &OpenCount
                  );

  if (EFI_ERROR (Status)) {
    return NULL;
  }
  
  Handle = NULL;
  
  for (Index = 0; Index < OpenCount; Index++) {
    if (OpenBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) {
      Handle = OpenBuffer[Index].ControllerHandle;
      break;
    }
  }

  gBS->FreePool (OpenBuffer);
  return Handle;
}

EFI_STATUS
NetLibInstallAllDriverProtocols (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL        *DriverBinding,
  IN EFI_HANDLE                         DriverBindingHandle,
  IN EFI_COMPONENT_NAME_PROTOCOL        *ComponentName,       OPTIONAL
  IN EFI_DRIVER_CONFIGURATION_PROTOCOL  *DriverConfiguration, OPTIONAL
  IN EFI_DRIVER_DIAGNOSTICS_PROTOCOL    *DriverDiagnostics    OPTIONAL
  )
/*++

Routine Description:

  Intialize a driver by installing the Driver Binding Protocol onto the 
  driver's DriverBindingHandle.  This is typically the same as the driver's
  ImageHandle, but it can be different if the driver produces multiple
  DriverBinding Protocols.  This function also initializes the EFI Driver
  Library that initializes the global variables gST, gBS, gRT.

Arguments:

  ImageHandle         - The image handle of the driver
  SystemTable         - The EFI System Table that was passed to the driver's
                        entry point
  DriverBinding       - A Driver Binding Protocol instance that this driver
                        is producing.
  DriverBindingHandle - The handle that DriverBinding is to be installe onto.
                        If this parameter is NULL, then a new handle is created.
  ComponentName       - A Component Name Protocol instance that this driver is
                        producing.
  DriverConfiguration - A Driver Configuration Protocol instance that this
                        driver is producing.  
  DriverDiagnostics   - A Driver Diagnostics Protocol instance that this
                        driver is producing.

Returns: 

  EFI_SUCCESS if all the protocols were installed onto DriverBindingHandle
  Otherwise, then return status from gBS->InstallProtocolInterface()

--*/
{
  return NetLibInstallAllDriverProtocolsWithUnload (
           ImageHandle,
           SystemTable,
           DriverBinding,
           DriverBindingHandle,
           ComponentName,
           DriverConfiguration,
           DriverDiagnostics,
           NetLibDefaultUnload
           );
}

EFI_STATUS
NetLibInstallAllDriverProtocolsWithUnload (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL        *DriverBinding,
  IN EFI_HANDLE                         DriverBindingHandle,
  IN EFI_COMPONENT_NAME_PROTOCOL        *ComponentName,         OPTIONAL
  IN EFI_DRIVER_CONFIGURATION_PROTOCOL  *DriverConfiguration,   OPTIONAL
  IN EFI_DRIVER_DIAGNOSTICS_PROTOCOL    *DriverDiagnostics,     OPTIONAL
  IN NET_LIB_DRIVER_UNLOAD              Unload
  )
/*++

Routine Description:

  Intialize a driver by installing the Driver Binding Protocol onto the 
  driver's DriverBindingHandle.  This is typically the same as the driver's
  ImageHandle, but it can be different if the driver produces multiple
  DriverBinding Protocols.  This function also initializes the EFI Driver
  Library that initializes the global variables gST, gBS, gRT.

Arguments:

  ImageHandle         - The image handle of the driver
  SystemTable         - The EFI System Table that was passed to the driver's
                        entry point
  DriverBinding       - A Driver Binding Protocol instance that this driver
                        is producing.
  DriverBindingHandle - The handle that DriverBinding is to be installe onto.
                        If this parameter is NULL, then a new handle is created.
  ComponentName       - A Component Name Protocol instance that this driver is
                        producing.
  DriverConfiguration - A Driver Configuration Protocol instance that this
                        driver is producing.  
  DriverDiagnostics   - A Driver Diagnostics Protocol instance that this
                        driver is producing.
  Unload    - The customized unload to install.
  
Returns: 

  EFI_SUCCESS if all the protocols were installed onto DriverBindingHandle
  Otherwise, then return status from gBS->InstallProtocolInterface()

--*/
{
  EFI_STATUS                Status;
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;

  Status = EfiLibInstallAllDriverProtocols (
             ImageHandle,
             SystemTable,
             DriverBinding,
             DriverBindingHandle,
             ComponentName,
             DriverConfiguration,
             DriverDiagnostics
             );
  
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Retrieve the Loaded Image Protocol from Image Handle
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **) &LoadedImage,
                  ImageHandle,
                  ImageHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Fill in the Unload() service of the Loaded Image Protocol
  //
  LoadedImage->Unload = (Unload == NULL) ? NetLibDefaultUnload : Unload;
  return EFI_SUCCESS;
}

