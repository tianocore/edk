/*++

Copyright (c) 2007 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    HiiDatabase.h
    
Abstract:

    Private structures definitions in HiiDatabase.    

Revision History

--*/

#ifndef __HII_DATABASE_PRIVATE_H__
#define __HII_DATABASE_PRIVATE_H__


#include "Tiano.h"
#include "EfiDriverLib.h"
#include "UefiIfrLibrary.h"

#include EFI_PROTOCOL_DEFINITION (HiiFont)
#include EFI_PROTOCOL_DEFINITION (HiiImage)
#include EFI_PROTOCOL_DEFINITION (HiiString)
#include EFI_PROTOCOL_DEFINITION (HiiDatabase)
#include EFI_PROTOCOL_DEFINITION (HiiConfigRouting)
#include EFI_PROTOCOL_DEFINITION (HiiConfigAccess)
#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)
#include EFI_PROTOCOL_DEFINITION (ConsoleControl)

#define HII_DATABASE_NOTIFY_GUID \
  { \
    0xc1c76, 0xd79e, 0x42fe, 0x86, 0xb, 0x8b, 0xe8, 0x7b, 0x3e, 0x7a, 0x78 \
  }

#define MAX_STRING_LENGTH                  1024
#define MAX_FONT_NAME_LEN                  256
#define NARROW_BASELINE                    15
#define WIDE_BASELINE                      14
#define SYS_FONT_INFO_MASK                 0x37
#define REPLACE_UNKNOWN_GLYPH              0xFFFD
#define PROPORTIONAL_GLYPH                 0x80
#define NARROW_GLYPH                       0x40

#define BITMAP_LEN_1_BIT(Width, Height)  (((Width) + 7) / 8 * (Height))
#define BITMAP_LEN_4_BIT(Width, Height)  (((Width) + 1) / 2 * (Height))
#define BITMAP_LEN_8_BIT(Width, Height)  ((Width) * (Height))
#define BITMAP_LEN_24_BIT(Width, Height) ((Width) * (Height) * 3)

//
// Storage types
//
#define EFI_HII_VARSTORE_BUFFER            0
#define EFI_HII_VARSTORE_NAME_VALUE        1
#define EFI_HII_VARSTORE_EFI_VARIABLE      2

#define HII_FORMSET_STORAGE_SIGNATURE           EFI_SIGNATURE_32 ('H', 'S', 'T', 'G')
typedef struct {
  UINTN               Signature;
  EFI_LIST_ENTRY      Entry;

  EFI_HII_HANDLE      HiiHandle;
  EFI_HANDLE          DriverHandle;

  UINT8               Type;   // EFI_HII_VARSTORE_BUFFER, EFI_HII_VARSTORE_NAME_VALUE, EFI_HII_VARSTORE_EFI_VARIABLE
  EFI_GUID            Guid;
  CHAR16              *Name;
  UINT16              Size;
} HII_FORMSET_STORAGE;

#define HII_FORMSET_STORAGE_FROM_LINK(a)  CR (a, HII_FORMSET_STORAGE, Link, HII_FORMSET_STORAGE_SIGNATURE)


//
// String Package definitions
//
#define HII_STRING_PACKAGE_SIGNATURE    EFI_SIGNATURE_32 ('h','i','s','p')
typedef struct _HII_STRING_PACKAGE_INSTANCE {
  UINTN                                 Signature;  
  EFI_HII_STRING_PACKAGE_HDR            *StringPkgHdr;
  UINT8                                 *StringBlock;
  EFI_LIST_ENTRY                        StringEntry;
  EFI_LIST_ENTRY                        FontInfoList;  // local font info list
  UINT8                                 FontId;
} HII_STRING_PACKAGE_INSTANCE;

//
// Form Package definitions
//
#define HII_IFR_PACKAGE_SIGNATURE       EFI_SIGNATURE_32 ('h','f','r','p')
typedef struct _HII_IFR_PACKAGE_INSTANCE {
  UINTN                                 Signature;
  EFI_HII_PACKAGE_HEADER                FormPkgHdr;
  UINT8                                 *IfrData;
  EFI_LIST_ENTRY                        IfrEntry;
} HII_IFR_PACKAGE_INSTANCE;

//
// Simple Font Package definitions
//
#define HII_S_FONT_PACKAGE_SIGNATURE    EFI_SIGNATURE_32 ('h','s','f','p')
typedef struct _HII_SIMPLE_FONT_PACKAGE_INSTANCE {
  UINTN                                 Signature;
  EFI_HII_SIMPLE_FONT_PACKAGE_HDR       *SimpleFontPkgHdr;
  EFI_LIST_ENTRY                        SimpleFontEntry;
} HII_SIMPLE_FONT_PACKAGE_INSTANCE;

//
// Font Package definitions
//
#define HII_FONT_PACKAGE_SIGNATURE      EFI_SIGNATURE_32 ('h','i','f','p')
typedef struct _HII_FONT_PACKAGE_INSTANCE {
  UINTN                                 Signature;  
  EFI_HII_FONT_PACKAGE_HDR              *FontPkgHdr;
  UINT8                                 *GlyphBlock;
  EFI_LIST_ENTRY                        FontEntry;
  EFI_LIST_ENTRY                        GlyphInfoList;
} HII_FONT_PACKAGE_INSTANCE;

#define HII_GLYPH_INFO_SIGNATURE        EFI_SIGNATURE_32 ('h','g','i','s')
typedef struct _HII_GLYPH_INFO {
  UINTN                                 Signature;
  EFI_LIST_ENTRY                        Entry;
  CHAR16                                CharId;
  EFI_HII_GLYPH_INFO                    Cell;
} HII_GLYPH_INFO;

#define HII_FONT_INFO_SIGNATURE         EFI_SIGNATURE_32 ('h','l','f','i')
typedef struct _HII_FONT_INFO {
  UINTN                                 Signature;
  EFI_LIST_ENTRY                        Entry;
  EFI_LIST_ENTRY                        *GlobalEntry;
  UINT8                                 FontId;  
} HII_FONT_INFO;

#define HII_GLOBAL_FONT_INFO_SIGNATURE  EFI_SIGNATURE_32 ('h','g','f','i')
typedef struct _HII_GLOBAL_FONT_INFO {
  UINTN                                 Signature;
  EFI_LIST_ENTRY                        Entry;
  HII_FONT_PACKAGE_INSTANCE             *FontPackage;
  UINTN                                 FontInfoSize;
  EFI_FONT_INFO                         *FontInfo;  
} HII_GLOBAL_FONT_INFO;

//
// Image Package definitions
//

#define HII_PIXEL_MASK                  0x80

typedef struct _HII_IMAGE_PACKAGE_INSTANCE {
  EFI_HII_IMAGE_PACKAGE_HDR             ImagePkgHdr;
  UINT32                                ImageBlockSize;
  UINT32                                PaletteInfoSize;
  UINT8                                 *ImageBlock;
  UINT8                                 *PaletteBlock;
} HII_IMAGE_PACKAGE_INSTANCE;

//
// Keyboard Layout Pacakge definitions
//
#define HII_KB_LAYOUT_PACKAGE_SIGNATURE EFI_SIGNATURE_32 ('h','k','l','p')
typedef struct _HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE {
  UINTN                                 Signature;
  UINT8                                 *KeyboardPkg;
  EFI_LIST_ENTRY                        KeyboardEntry;
} HII_KEYBOARD_LAYOUT_PACKAGE_INSTANCE;

//
// Guid Package definitions
//
#define HII_GUID_PACKAGE_SIGNATURE      EFI_SIGNATURE_32 ('h','i','g','p')
typedef struct _HII_GUID_PACKAGE_INSTANCE {
  UINTN                                 Signature;
  UINT8                                 *GuidPkg;
  EFI_LIST_ENTRY                        GuidEntry;
} HII_GUID_PACKAGE_INSTANCE;

//
// A package list can contain only one or less than one device path package.
// This rule also applies to image package since ImageId can not be duplicate.
//
typedef struct _HII_DATABASE_PACKAGE_LIST_INSTANCE {
  EFI_HII_PACKAGE_LIST_HEADER           PackageListHdr;
  EFI_LIST_ENTRY                        GuidPkgHdr;
  EFI_LIST_ENTRY                        FormPkgHdr;
  EFI_LIST_ENTRY                        KeyboardLayoutHdr;
  EFI_LIST_ENTRY                        StringPkgHdr;
  EFI_LIST_ENTRY                        FontPkgHdr;
  HII_IMAGE_PACKAGE_INSTANCE            *ImagePkg;
  EFI_LIST_ENTRY                        SimpleFontPkgHdr;
  UINT8                                 *DevicePathPkg;
} HII_DATABASE_PACKAGE_LIST_INSTANCE;

#define HII_HANDLE_SIGNATURE            EFI_SIGNATURE_32 ('h','i','h','l')

typedef struct {
  UINTN               Signature;
  EFI_LIST_ENTRY      Handle;
  UINTN               Key;            
} HII_HANDLE;

#define HII_DATABASE_RECORD_SIGNATURE   EFI_SIGNATURE_32 ('h','i','d','r')

typedef struct _HII_DATABASE_RECORD {
  UINTN                                 Signature;
  HII_DATABASE_PACKAGE_LIST_INSTANCE    *PackageList;
  EFI_HANDLE                            DriverHandle;
  EFI_HII_HANDLE                        Handle;
  EFI_LIST_ENTRY                        DatabaseEntry;
} HII_DATABASE_RECORD;

#define HII_DATABASE_NOTIFY_SIGNATURE   EFI_SIGNATURE_32 ('h','i','d','n')

typedef struct _HII_DATABASE_NOTIFY {
  UINTN                                 Signature;  
  EFI_HANDLE                            NotifyHandle;
  UINT8                                 PackageType;
  EFI_GUID                              *PackageGuid;
  EFI_HII_DATABASE_NOTIFY               PackageNotifyFn;
  EFI_HII_DATABASE_NOTIFY_TYPE          NotifyType;
  EFI_LIST_ENTRY                        DatabaseNotifyEntry;
} HII_DATABASE_NOTIFY;

#define HII_DATABASE_PRIVATE_DATA_SIGNATURE EFI_SIGNATURE_32 ('H', 'i', 'D', 'p')

typedef struct _HII_DATABASE_PRIVATE_DATA {
  UINTN                                 Signature;
  EFI_LIST_ENTRY                        DatabaseList;
  EFI_LIST_ENTRY                        DatabaseNotifyList;
  EFI_HII_FONT_PROTOCOL                 HiiFont;
#ifndef DISABLE_UNUSED_HII_PROTOCOLS  
  EFI_HII_IMAGE_PROTOCOL                HiiImage;
#endif
  EFI_HII_STRING_PROTOCOL               HiiString;
  EFI_HII_DATABASE_PROTOCOL             HiiDatabase;
  EFI_HII_CONFIG_ROUTING_PROTOCOL       ConfigRouting;
  EFI_LIST_ENTRY                        HiiHandleList;
  UINTN                                 HiiHandleCount; 
  EFI_LIST_ENTRY                        FontInfoList;  // global font info list
  UINTN                                 Attribute;     // default system color  
  EFI_GUID                              CurrentLayoutGuid;
  EFI_HII_KEYBOARD_LAYOUT               *CurrentLayout;
} HII_DATABASE_PRIVATE_DATA;

#define HII_FONT_DATABASE_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      HII_DATABASE_PRIVATE_DATA, \
      HiiFont, \
      HII_DATABASE_PRIVATE_DATA_SIGNATURE \
      )

#define HII_IMAGE_DATABASE_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      HII_DATABASE_PRIVATE_DATA, \
      HiiImage, \
      HII_DATABASE_PRIVATE_DATA_SIGNATURE \
      )

#define HII_STRING_DATABASE_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      HII_DATABASE_PRIVATE_DATA, \
      HiiString, \
      HII_DATABASE_PRIVATE_DATA_SIGNATURE \
      )
      
#define HII_DATABASE_DATABASE_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      HII_DATABASE_PRIVATE_DATA, \
      HiiDatabase, \
      HII_DATABASE_PRIVATE_DATA_SIGNATURE \
      )

#define CONFIG_ROUTING_DATABASE_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      HII_DATABASE_PRIVATE_DATA, \
      ConfigRouting, \
      HII_DATABASE_PRIVATE_DATA_SIGNATURE \
      )

//
// Internal function prototypes.
//
BOOLEAN
IsHiiHandleValid (
  EFI_HII_HANDLE Handle
  )
/*++

  Routine Description:
    This function checks whether a handle is a valid EFI_HII_HANDLE
    
  Arguments:          
    Handle        - Pointer to a EFI_HII_HANDLE
    
  Returns:
    TRUE          - Valid
    FALSE         - Invalid
    
--*/  
;

BOOLEAN
IsFontInfoExisted (
  IN  HII_DATABASE_PRIVATE_DATA *Private,
  IN  EFI_FONT_INFO             *FontInfo,
  IN  EFI_FONT_INFO_MASK        *FontInfoMask,   OPTIONAL
  IN  EFI_FONT_HANDLE           FontHandle,      OPTIONAL
  OUT HII_GLOBAL_FONT_INFO      **GlobalFontInfo OPTIONAL
  )
/*++

  Routine Description:
    This function checks whether EFI_FONT_INFO exists in current database. If
    FontInfoMask is specified, check what options can be used to make a match. 
    Note that the masks relate to where the system default should be supplied
    are ignored by this function.
    
  Arguments:          
    Private        - Hii database private structure.        
    FontInfo       - Points to EFI_FONT_INFO structure.
    FontInfoMask   - If not NULL, describes what options can be used to make a
                     match between the font requested and the font available.
                     The caller must guarantee this mask is valid.
    FontHandle     - On entry, Points to the font handle returned by a previous 
                     call to GetFontInfo() or NULL to start with the first font.
    GlobalFontInfo - If not NULL, output the corresponding globa font info.
    
  Returns:
    TRUE           - Existed
    FALSE          - Not existed
    
--*/
;

EFI_STATUS
InvokeRegisteredFunction (
  IN HII_DATABASE_PRIVATE_DATA    *Private, 
  IN EFI_HII_DATABASE_NOTIFY_TYPE NotifyType,
  IN VOID                         *PackageInstance,
  IN UINT8                        PackageType,
  IN EFI_HII_HANDLE               Handle
  )
/*++

  Routine Description:
    This function invokes the matching registered function.
    
  Arguments:          
    Private           - HII Database driver private structure.
    NotifyType        - The type of change concerning the database.
    PackageInstance   - Points to the package referred to by the notification.
    PackageType       - Package type
    Handle            - The handle of the package list which contains the specified package.
    
  Returns:
    EFI_SUCCESS            - Already checked all registered function and invoked 
                             if matched.
    EFI_INVALID_PARAMETER  - Any input parameter is not valid.
     
--*/
;

EFI_STATUS
GetSystemFont (
  IN  HII_DATABASE_PRIVATE_DATA      *Private,
  OUT EFI_FONT_DISPLAY_INFO          **FontInfo,
  OUT UINTN                          *FontInfoSize OPTIONAL
  )
/*++

  Routine Description:
    Retrieve system default font and color.   
    
  Arguments:          
    Private                - HII database driver private data.
    FontInfo               - Points to system default font output-related information.
                             It's caller's responsibility to free this buffer.
    FontInfoSize           - If not NULL, output the size of buffer FontInfo.
                             
    
  Returns:
    EFI_SUCCESS            - Cell information is added to the GlyphInfoList.
    EFI_OUT_OF_RESOURCES   - The system is out of resources to accomplish the task.
    EFI_INVALID_PARAMETER  - Any input parameter is invalid.
    
--*/       
;

EFI_STATUS
FindStringBlock (
  IN HII_DATABASE_PRIVATE_DATA        *Private,
  IN  HII_STRING_PACKAGE_INSTANCE     *StringPackage,
  IN  EFI_STRING_ID                   StringId,
  OUT UINT8                           *BlockType, OPTIONAL
  OUT UINT8                           **StringBlockAddr, OPTIONAL
  OUT UINTN                           *StringTextOffset, OPTIONAL
  OUT EFI_STRING_ID                   *LastStringId OPTIONAL
  )
/*++

  Routine Description:
    Parse all string blocks to find a String block specified by StringId.
    
    If StringId = (EFI_STRING_ID) (-1), find out all EFI_HII_SIBT_FONT blocks 
    within this string package and backup its information.

    If StringId = 0, output the string id of last string block (EFI_HII_SIBT_END).
    
    
  Arguments:  
    Private                - Hii database private structure.      
    StringPackage          - Hii string package instance.
    StringId               - The string¡¯s id, which is unique within PackageList.
    BlockType              - Output the block type of found string block.
    StringBlockAddr        - Output the block address of found string block.
    StringTextOffset       - Offset, relative to the found block address, of the 
                             string text information.
    LastStringId           - Output the last string id when StringId = 0.

  Returns:
    EFI_SUCCESS            - The string text and font is retrieved successfully.
    EFI_NOT_FOUND          - The specified text or font info can not be found out.
    EFI_OUT_OF_RESOURCES   - The system is out of resources to accomplish the task.
    
--*/          
;

EFI_STATUS
FindGlyphBlock (
  IN  HII_FONT_PACKAGE_INSTANCE      *FontPackage,
  IN  CHAR16                         CharValue,
  OUT UINT8                          **GlyphBuffer, OPTIONAL
  OUT EFI_HII_GLYPH_INFO             *Cell, OPTIONAL
  OUT UINTN                          *GlyphBufferLen OPTIONAL
  )
/*++

  Routine Description:
    Parse all glyph blocks to find a glyph block specified by CharValue.
    
    If CharValue = (CHAR16) (-1), collect all default character cell information  
    within this font package and backup its information.
    
  Arguments:          
    FontPackage            - Hii string package instance.
    CharValue              - Unicode character value, which identifies a glyph block.
    GlyphBuffer            - Output the corresponding bitmap data of the found block.
                             It is the caller's responsiblity to free this buffer.
    Cell                   - Output cell information of the encoded bitmap.
    GlyphBufferLen         - If not NULL, output the length of GlyphBuffer.
    
  Returns:
    EFI_SUCCESS            - The bitmap data is retrieved successfully.
    EFI_NOT_FOUND          - The specified CharValue does not exist in current database.
    EFI_OUT_OF_RESOURCES   - The system is out of resources to accomplish the task.    
    
--*/      
;

//
// EFI_HII_FONT_PROTOCOL protocol interfaces
//

EFI_STATUS
EFIAPI
HiiStringToImage (
  IN  CONST EFI_HII_FONT_PROTOCOL    *This,
  IN  EFI_HII_OUT_FLAGS              Flags,
  IN  CONST EFI_STRING               String,
  IN  CONST EFI_FONT_DISPLAY_INFO    *StringInfo       OPTIONAL,
  IN  OUT EFI_IMAGE_OUTPUT           **Blt,
  IN  UINTN                          BltX,
  IN  UINTN                          BltY,
  OUT EFI_HII_ROW_INFO               **RowInfoArray    OPTIONAL,
  OUT UINTN                          *RowInfoArraySize OPTIONAL,
  OUT UINTN                          *ColumnInfoArray  OPTIONAL
  )
/*++

  Routine Description:
    Renders a string to a bitmap or to the display.

  Arguments:          
    This              - A pointer to the EFI_HII_FONT_PROTOCOL instance.
    Flags             - Describes how the string is to be drawn.                 
    String            - Points to the null-terminated string to be displayed.
    StringInfo        - Points to the string output information, including the color and font. 
                        If NULL, then the string will be output in the default system font and color.
    Blt               - If this points to a non-NULL on entry, this points to the image, which is Width pixels  
                        wide and Height pixels high. The string will be drawn onto this image and               
                        EFI_HII_OUT_FLAG_CLIP is implied. If this points to a NULL on entry, then a             
                        buffer will be allocated to hold the generated image and the pointer updated on exit. It
                        is the caller¡¯s responsibility to free this buffer.                                    
    BltX,BLTY         - Specifies the offset from the left and top edge of the image of the first character cell in
                        the image.                                                                                     
    RowInfoArray      - If this is non-NULL on entry, then on exit, this will point to an allocated buffer   
                        containing row information and RowInfoArraySize will be updated to contain the       
                        number of elements. This array describes the characters which were at least partially
                        drawn and the heights of the rows. It is the caller¡¯s responsibility to free this buffer.
    RowInfoArraySize  - If this is non-NULL on entry, then on exit it contains the number of elements in
                        RowInfoArray.                                                                   
    ColumnInfoArray   - If this is non-NULL, then on return it will be filled with the horizontal offset for each 
                        character in the string on the row where it is displayed. Non-printing characters will    
                        have the offset ~0. The caller is responsible to allocate a buffer large enough so that   
                        there is one entry for each character in the string, not including the null-terminator. It
                        is possible when character display is normalized that some character cells overlap.           
                     
  Returns:
    EFI_SUCCESS           - The string was successfully rendered.                           
    EFI_OUT_OF_RESOURCES  - Unable to allocate an output buffer for RowInfoArray or Blt.
    EFI_INVALID_PARAMETER - The String or Blt was NULL.
    EFI_INVALID_PARAMETER - Flags were invalid combination.
        
--*/
;

EFI_STATUS
EFIAPI 
HiiStringIdToImage (
  IN  CONST EFI_HII_FONT_PROTOCOL    *This,
  IN  EFI_HII_OUT_FLAGS              Flags,  
  IN  EFI_HII_HANDLE                 PackageList,
  IN  EFI_STRING_ID                  StringId,
  IN  CONST CHAR8*                   Language,
  IN  CONST EFI_FONT_DISPLAY_INFO    *StringInfo       OPTIONAL,
  IN  OUT EFI_IMAGE_OUTPUT           **Blt,
  IN  UINTN                          BltX,
  IN  UINTN                          BltY,
  OUT EFI_HII_ROW_INFO               **RowInfoArray    OPTIONAL,
  OUT UINTN                          *RowInfoArraySize OPTIONAL,
  OUT UINTN                          *ColumnInfoArray  OPTIONAL
  )
/*++

  Routine Description:
    Render a string to a bitmap or the screen containing the contents of the specified string.

  Arguments:          
    This              - A pointer to the EFI_HII_FONT_PROTOCOL instance.
    Flags             - Describes how the string is to be drawn.                 
    PackageList       - The package list in the HII database to search for the specified string.         
    StringId          - The string¡¯s id, which is unique within PackageList.                            
    Language          - Points to the language for the retrieved string. If NULL, then the current system
                        language is used.                                                                
    StringInfo        - Points to the string output information, including the color and font. 
                        If NULL, then the string will be output in the default system font and color.
    Blt               - If this points to a non-NULL on entry, this points to the image, which is Width pixels  
                        wide and Height pixels high. The string will be drawn onto this image and               
                        EFI_HII_OUT_FLAG_CLIP is implied. If this points to a NULL on entry, then a             
                        buffer will be allocated to hold the generated image and the pointer updated on exit. It
                        is the caller¡¯s responsibility to free this buffer.                                    
    BltX,BLTY         - Specifies the offset from the left and top edge of the image of the first character cell in
                        the image.                                                                                     
    RowInfoArray      - If this is non-NULL on entry, then on exit, this will point to an allocated buffer   
                        containing row information and RowInfoArraySize will be updated to contain the       
                        number of elements. This array describes the characters which were at least partially
                        drawn and the heights of the rows. It is the caller¡¯s responsibility to free this buffer.
    RowInfoArraySize  - If this is non-NULL on entry, then on exit it contains the number of elements in
                        RowInfoArray.                                                                   
    ColumnInfoArray   - If this is non-NULL, then on return it will be filled with the horizontal offset for each 
                        character in the string on the row where it is displayed. Non-printing characters will    
                        have the offset ~0. The caller is responsible to allocate a buffer large enough so that   
                        there is one entry for each character in the string, not including the null-terminator. It
                        is possible when character display is normalized that some character cells overlap.           
                     
  Returns:
    EFI_SUCCESS           - The string was successfully rendered.
    EFI_OUT_OF_RESOURCES  - Unable to allocate an output buffer for RowInfoArray or Blt.
    EFI_INVALID_PARAMETER - The Blt or PackageList was NULL.
    EFI_INVALID_PARAMETER - Flags were invalid combination.
    EFI_NOT_FOUND         - The specified PackageList is not in the Database or the stringid is not 
                            in the specified PackageList. 
        
--*/
;

EFI_STATUS
EFIAPI
HiiGetGlyph (
  IN  CONST EFI_HII_FONT_PROTOCOL    *This,
  IN  CHAR16                         Char,
  IN  CONST EFI_FONT_DISPLAY_INFO    *StringInfo,
  OUT EFI_IMAGE_OUTPUT               **Blt,
  OUT UINTN                          *Baseline OPTIONAL
  )
/*++

  Routine Description:
    Convert the glyph for a single character into a bitmap.

  Arguments:          
    This              - A pointer to the EFI_HII_FONT_PROTOCOL instance.
    Char              - Character to retrieve.
    StringInfo        - Points to the string font and color information or NULL if the string should use the
                        default system font and color.                                                      
    Blt               - Thus must point to a NULL on entry. A buffer will be allocated to hold the output and
                        the pointer updated on exit. It is the caller¡¯s responsibility to free this buffer. 
    Baseline          - Number of pixels from the bottom of the bitmap to the baseline.

  Returns:
    EFI_SUCCESS            - Glyph bitmap created.
    EFI_OUT_OF_RESOURCES   - Unable to allocate the output buffer Blt.
    EFI_WARN_UNKNOWN_GLYPH - The glyph was unknown and was
                             replaced with the glyph for Unicode
                             character 0xFFFD.
    EFI_INVALID_PARAMETER  - Blt is NULL or *Blt is not NULL.
            
--*/    
;

EFI_STATUS
EFIAPI
HiiGetFontInfo (
  IN  CONST EFI_HII_FONT_PROTOCOL    *This,
  IN  OUT   EFI_FONT_HANDLE          *FontHandle,
  IN  CONST EFI_FONT_DISPLAY_INFO    *StringInfoIn, OPTIONAL
  OUT       EFI_FONT_DISPLAY_INFO    **StringInfoOut,
  IN  CONST EFI_STRING               String OPTIONAL
  )
/*++

  Routine Description:
    This function iterates through fonts which match the specified font, using 
    the specified criteria. If String is non-NULL, then all of the characters in 
    the string must exist in order for a candidate font to be returned.
    
  Arguments:          
    This              - A pointer to the EFI_HII_FONT_PROTOCOL instance.
    FontHandle        - On entry, points to the font handle returned by a previous 
                        call to GetFontInfo() or points to NULL to start with the 
                        first font. On return, points to the returned font handle or
                        points to NULL if there are no more matching fonts.
    StringInfoIn      - Upon entry, points to the font to return information about.
                        If NULL, then the information about the system default 
                        font will be returned.
    StringInfoOut     - Upon return, contains the matching font¡¯s information. 
                        If NULL, then no information is returned.
                        It's caller's responsibility to free this buffer.
    String            - Points to the string which will be tested to determine 
                        if all characters are available. If NULL, then any font 
                        is acceptable.

  Returns:
    EFI_SUCCESS            - Matching font returned successfully.
    EFI_NOT_FOUND          - No matching font was found.
    EFI_INVALID_PARAMETER  - StringInfoIn->FontInfoMask is an invalid combination.
    EFI_OUT_OF_RESOURCES   - There were insufficient resources to complete the request.
            
--*/
;

//
// EFI_HII_IMAGE_PROTOCOL interfaces
//

EFI_STATUS
EFIAPI 
HiiNewImage (
  IN  CONST EFI_HII_IMAGE_PROTOCOL   *This,
  IN  EFI_HII_HANDLE                 PackageList,
  OUT EFI_IMAGE_ID                   *ImageId,
  IN  CONST EFI_IMAGE_INPUT          *Image
  )
/*++

  Routine Description:
    This function adds the image Image to the group of images owned by PackageList, and returns
    a new image identifier (ImageId).                                                          
    
  Arguments:          
    This              - A pointer to the EFI_HII_IMAGE_PROTOCOL instance.
    PackageList       - Handle of the package list where this image will be added.    
    ImageId           - On return, contains the new image id, which is unique within PackageList.
    Image             - Points to the image.
    
  Returns:
    EFI_SUCCESS            - The new image was added successfully.
    EFI_NOT_FOUND          - The specified PackageList could not be found in database.
    EFI_OUT_OF_RESOURCES   - Could not add the image due to lack of resources.
    EFI_INVALID_PARAMETER  - Image is NULL or ImageId is NULL.  
    
--*/    
;

EFI_STATUS
EFIAPI
HiiGetImage (
  IN  CONST EFI_HII_IMAGE_PROTOCOL   *This,
  IN  EFI_HII_HANDLE                 PackageList,
  IN  EFI_IMAGE_ID                   ImageId,
  OUT EFI_IMAGE_INPUT                *Image
  )
/*++

  Routine Description:
    This function retrieves the image specified by ImageId which is associated with
    the specified PackageList and copies it into the buffer specified by Image.
    
  Arguments:          
    This              - A pointer to the EFI_HII_IMAGE_PROTOCOL instance.
    PackageList       - Handle of the package list where this image will be searched.    
    ImageId           - The image¡¯s id,, which is unique within PackageList.
    Image             - Points to the image.
                        
  Returns:
    EFI_SUCCESS            - The new image was returned successfully.
    EFI_NOT_FOUND          - The image specified by ImageId is not available.
                             The specified PackageList is not in the database.    
    EFI_INVALID_PARAMETER  - The Image or ImageSize was NULL.
    EFI_OUT_OF_RESOURCES   - The bitmap could not be retrieved because there was not
                             enough memory.
    
--*/
;

EFI_STATUS
EFIAPI
HiiSetImage (
  IN CONST EFI_HII_IMAGE_PROTOCOL    *This,
  IN EFI_HII_HANDLE                  PackageList,
  IN EFI_IMAGE_ID                    ImageId,
  IN CONST EFI_IMAGE_INPUT           *Image
  )
/*++

  Routine Description:
    This function updates the image specified by ImageId in the specified PackageListHandle to
    the image specified by Image.
    
  Arguments:          
    This              - A pointer to the EFI_HII_IMAGE_PROTOCOL instance.
    PackageList       - The package list containing the images.
    ImageId           - The image¡¯s id,, which is unique within PackageList.
    Image             - Points to the image.
                        
  Returns:
    EFI_SUCCESS            - The new image was updated successfully.
    EFI_NOT_FOUND          - The image specified by ImageId is not in the database.
                             The specified PackageList is not in the database.    
    EFI_INVALID_PARAMETER  - The Image was NULL.
    
--*/  
;

EFI_STATUS
EFIAPI
HiiDrawImage (
  IN CONST EFI_HII_IMAGE_PROTOCOL    *This,
  IN EFI_HII_DRAW_FLAGS              Flags,
  IN CONST EFI_IMAGE_INPUT           *Image,
  IN OUT EFI_IMAGE_OUTPUT            **Blt,
  IN UINTN                           BltX,
  IN UINTN                           BltY
  )
/*++

  Routine Description:
    This function renders an image to a bitmap or the screen using the specified
    color and options. It draws the image on an existing bitmap, allocates a new
    bitmap or uses the screen. The images can be clipped.       
    
  Arguments:          
    This              - A pointer to the EFI_HII_IMAGE_PROTOCOL instance.
    Flags             - Describes how the image is to be drawn.    
    Image             - Points to the image to be displayed.
    Blt               - If this points to a non-NULL on entry, this points to the
                        image, which is Width pixels wide and Height pixels high. 
                        The image will be drawn onto this image and 
                        EFI_HII_DRAW_FLAG_CLIP is implied. If this points to a 
                        NULL on entry, then a buffer will be allocated to hold 
                        the generated image and the pointer updated on exit. It
                        is the caller¡¯s responsibility to free this buffer.
    BltX, BltY        - Specifies the offset from the left and top edge of the 
                        output image of the first pixel in the image.
                        
  Returns:
    EFI_SUCCESS            - The image was successfully drawn.
    EFI_OUT_OF_RESOURCES   - Unable to allocate an output buffer for Blt.
    EFI_INVALID_PARAMETER  - The Image or Blt was NULL.
    EFI_INVALID_PARAMETER  - Any combination of Flags is invalid.
    
--*/ 
;

EFI_STATUS
EFIAPI
HiiDrawImageId (
  IN CONST EFI_HII_IMAGE_PROTOCOL    *This,
  IN EFI_HII_DRAW_FLAGS              Flags,
  IN EFI_HII_HANDLE                  PackageList,
  IN EFI_IMAGE_ID                    ImageId,
  IN OUT EFI_IMAGE_OUTPUT            **Blt,
  IN UINTN                           BltX,
  IN UINTN                           BltY
  )

/*++

  Routine Description:
    This function renders an image to a bitmap or the screen using the specified
    color and options. It draws the image on an existing bitmap, allocates a new
    bitmap or uses the screen. The images can be clipped.       
    
  Arguments:          
    This              - A pointer to the EFI_HII_IMAGE_PROTOCOL instance.
    Flags             - Describes how the image is to be drawn.
    PackageList       - The package list in the HII database to search for the 
                        specified image.
    ImageId           - The image's id, which is unique within PackageList.
    Blt               - If this points to a non-NULL on entry, this points to the
                        image, which is Width pixels wide and Height pixels high.
                        The image will be drawn onto this image and                
                        EFI_HII_DRAW_FLAG_CLIP is implied. If this points to a 
                        NULL on entry, then a buffer will be allocated to hold 
                        the generated image and the pointer updated on exit. It
                        is the caller¡¯s responsibility to free this buffer.
    BltX, BltY        - Specifies the offset from the left and top edge of the 
                        output image of the first pixel in the image.
                        
  Returns:
    EFI_SUCCESS            - The image was successfully drawn.
    EFI_OUT_OF_RESOURCES   - Unable to allocate an output buffer for Blt.
    EFI_INVALID_PARAMETER  - The Blt was NULL.
    EFI_NOT_FOUND          - The image specified by ImageId is not in the database. 
                             The specified PackageList is not in the database.
    
--*/
;

//
// EFI_HII_STRING_PROTOCOL
//

EFI_STATUS
EFIAPI
HiiNewString (
  IN  CONST EFI_HII_STRING_PROTOCOL   *This,
  IN  EFI_HII_HANDLE                  PackageList,
  OUT EFI_STRING_ID                   *StringId,
  IN  CONST CHAR8                     *Language,
  IN  CONST CHAR16                    *LanguageName, OPTIONAL
  IN  CONST EFI_STRING                String,
  IN  CONST EFI_FONT_INFO             *StringFontInfo OPTIONAL
  )
/*++

  Routine Description:
    This function adds the string String to the group of strings owned by PackageList, with the
    specified font information StringFontInfo and returns a new string id.
    
  Arguments:          
    This              - A pointer to the EFI_HII_STRING_PROTOCOL instance.
    PackageList       - Handle of the package list where this string will be added.                        
    StringId          - On return, contains the new strings id, which is unique within PackageList.    
    Language          - Points to the language for the new string.
    LanguageName      - Points to the printable language name to associate with the passed in 
                        Language field.If LanguageName is not NULL and the string package header's LanguageName 
                        associated with a given Language is not zero, the LanguageName being passed 
                        in will be ignored.
    String            - Points to the new null-terminated string.                                                                                     
    StringFontInfo    - Points to the new string¡¯s font information or NULL if the string should have the
                        default system font, size and style.

  Returns:
    EFI_SUCCESS            - The new string was added successfully.
    EFI_NOT_FOUND          - The specified PackageList could not be found in database.
    EFI_OUT_OF_RESOURCES   - Could not add the string due to lack of resources.
    EFI_INVALID_PARAMETER  - String is NULL or StringId is NULL or Language is NULL.
    
--*/    
;

EFI_STATUS
EFIAPI
HiiGetString (
  IN  CONST EFI_HII_STRING_PROTOCOL   *This,
  IN  CONST CHAR8                     *Language,
  IN  EFI_HII_HANDLE                  PackageList,
  IN  EFI_STRING_ID                   StringId,
  OUT EFI_STRING                      String,
  IN  OUT UINTN                       *StringSize,
  OUT EFI_FONT_INFO                   **StringFontInfo OPTIONAL
  )
/*++

  Routine Description:
    This function retrieves the string specified by StringId which is associated 
    with the specified PackageList in the language Language and copies it into 
    the buffer specified by String.
    
  Arguments:          
    This              - A pointer to the EFI_HII_STRING_PROTOCOL instance.
    Language          - Points to the language for the retrieved string.
    PackageList       - The package list in the HII database to search for the 
                        specified string.    
    StringId          - The string's id, which is unique within PackageList.    
    String            - Points to the new null-terminated string.            
    StringSize        - On entry, points to the size of the buffer pointed to by 
                        String, in bytes. On return,
                        points to the length of the string, in bytes.                                                                                             
    StringFontInfo    - If not NULL, points to the string¡¯s font information. 
                        It's caller's responsibility to free this buffer.

  Returns:
    EFI_SUCCESS            - The string was returned successfully.
    EFI_NOT_FOUND          - The string specified by StringId is not available.
                             The specified PackageList is not in the database.
    EFI_INVALID_LANGUAGE   - The string specified by StringId is available but
                             not in the specified language.
    EFI_BUFFER_TOO_SMALL   - The buffer specified by StringSize is too small to 
                             hold the string.                                                      
    EFI_INVALID_PARAMETER  - The String or Language or StringSize was NULL.
    EFI_OUT_OF_RESOURCES   - There were insufficient resources to complete the 
                             request.
    
--*/ 
;

EFI_STATUS
EFIAPI
HiiSetString (
  IN CONST EFI_HII_STRING_PROTOCOL    *This,
  IN EFI_HII_HANDLE                   PackageList,
  IN EFI_STRING_ID                    StringId,
  IN CONST CHAR8                      *Language,
  IN CONST EFI_STRING                 String,
  IN CONST EFI_FONT_INFO              *StringFontInfo OPTIONAL
  )
/*++

  Routine Description:
    This function updates the string specified by StringId in the specified PackageList to the text   
    specified by String and, optionally, the font information specified by StringFontInfo.         
    
  Arguments:          
    This              - A pointer to the EFI_HII_STRING_PROTOCOL instance.
    PackageList       - The package list containing the strings.
    StringId          - The string¡¯s id, which is unique within PackageList.    
    Language          - Points to the language for the updated string.
    String            - Points to the new null-terminated string.                   
    StringFontInfo    - Points to the string¡¯s font information or NULL if the string font information is not
                        changed.  

  Returns:
    EFI_SUCCESS            - The string was updated successfully.
    EFI_NOT_FOUND          - The string specified by StringId is not in the database.    
    EFI_INVALID_PARAMETER  - The String or Language was NULL.
    EFI_OUT_OF_RESOURCES   - The system is out of resources to accomplish the task.
    
--*/ 
;

EFI_STATUS
EFIAPI
HiiGetLanguages (
  IN CONST EFI_HII_STRING_PROTOCOL    *This,
  IN EFI_HII_HANDLE                   PackageList,
  IN OUT CHAR8                        *Languages,
  IN OUT UINTN                        *LanguagesSize
  )
/*++

  Routine Description:
    This function returns the list of supported languages, in the format specified
    in Appendix M of UEFI 2.1 spec.
    
  Arguments:          
    This              - A pointer to the EFI_HII_STRING_PROTOCOL instance.
    PackageList       - The package list to examine.
    Languages         - Points to the buffer to hold the returned string.
    LanguagesSize     - On entry, points to the size of the buffer pointed to by 
                        Languages, in bytes. On 
                        return, points to the length of Languages, in bytes.
                        
  Returns:
    EFI_SUCCESS            - The languages were returned successfully.    
    EFI_INVALID_PARAMETER  - The Languages or LanguagesSize was NULL.
    EFI_BUFFER_TOO_SMALL   - The LanguagesSize is too small to hold the list of 
                             supported languages. LanguageSize is updated to
                             contain the required size.
    EFI_NOT_FOUND          - Could not find string package in specified packagelist.
    
--*/
;

EFI_STATUS
EFIAPI 
HiiGetSecondaryLanguages (
  IN CONST EFI_HII_STRING_PROTOCOL   *This,
  IN EFI_HII_HANDLE                  PackageList,
  IN CONST CHAR8                     *FirstLanguage,
  IN OUT CHAR8                       *SecondLanguages,
  IN OUT UINTN                       *SecondLanguagesSize
  )
/*++

  Routine Description:
    Each string package has associated with it a single primary language and zero
    or more secondary languages. This routine returns the secondary languages
    associated with a package list.
    
  Arguments:          
    This                   - A pointer to the EFI_HII_STRING_PROTOCOL instance.
    PackageList            - The package list to examine.
    FirstLanguage          - Points to the primary language.
    SecondaryLanguages     - Points to the buffer to hold the returned list of 
                             secondary languages for the specified FirstLanguage.
                             If there are no secondary languages, the function 
                             returns successfully, but this is set to NULL.
    SecondaryLanguageSize  - On entry, points to the size of the buffer pointed to 
                             by SecondLanguages, in bytes. On return, points to
                             the length of SecondLanguages in bytes.
                        
  Returns:
    EFI_SUCCESS            - Secondary languages were correctly returned.
    EFI_INVALID_PARAMETER  - FirstLanguage or SecondLanguages or SecondLanguagesSize was NULL. 
    EFI_BUFFER_TOO_SMALL   - The buffer specified by SecondLanguagesSize is   
                             too small to hold the returned information.      
                             SecondLanguageSize is updated to hold the size of
                             the buffer required.
    EFI_INVALID_LANGUAGE   - The language specified by FirstLanguage is not
                             present in the specified package list.
    EFI_NOT_FOUND          - The specified PackageList is not in the Database.                                

--*/
;

//
// EFI_HII_DATABASE_PROTOCOL protocol interfaces
//

EFI_STATUS
EFIAPI 
HiiNewPackageList (
  IN CONST EFI_HII_DATABASE_PROTOCOL    *This,
  IN CONST EFI_HII_PACKAGE_LIST_HEADER  *PackageList,
  IN CONST EFI_HANDLE                   DriverHandle,
  OUT EFI_HII_HANDLE                    *Handle
  )
/*++

  Routine Description:
    This function adds the packages in the package list to the database and returns a handle. If there is a
    EFI_DEVICE_PATH_PROTOCOL associated with the DriverHandle, then this function will                     
    create a package of type EFI_PACKAGE_TYPE_DEVICE_PATH and add it to the package list.                      
    
  Arguments:          
    This              - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
    PackageList       - A pointer to an EFI_HII_PACKAGE_LIST_HEADER structure.
    DriverHandle      - Associate the package list with this EFI handle.    
    Handle            - A pointer to the EFI_HII_HANDLE instance.
    
  Returns:
    EFI_SUCCESS            - The package list associated with the Handle
                             was added to the HII database.             
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary resources for the
                             new database contents.                        
    EFI_INVALID_PARAMETER  - PackageList is NULL or Handle is NULL.
     
--*/  
;

EFI_STATUS
EFIAPI 
HiiRemovePackageList (
  IN CONST EFI_HII_DATABASE_PROTOCOL    *This,
  IN EFI_HII_HANDLE                     Handle
  )
/*++

  Routine Description:
    This function removes the package list that is associated with a handle Handle 
    from the HII database. Before removing the package, any registered functions 
    with the notification type REMOVE_PACK and the same package type will be called.
    
  Arguments:          
    This              - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
    Handle            - The handle that was registered to the data that is requested 
                        for removal.
    
  Returns:
    EFI_SUCCESS            - The data associated with the Handle was removed from 
                             the HII database.
    EFI_NOT_FOUND          - The specified Handle is not in database.
     
--*/
;

EFI_STATUS
EFIAPI
HiiUpdatePackageList (
  IN CONST EFI_HII_DATABASE_PROTOCOL    *This,
  IN EFI_HII_HANDLE                     Handle,
  IN CONST EFI_HII_PACKAGE_LIST_HEADER  *PackageList
  )
/*++

  Routine Description:
    This function updates the existing package list (which has the specified Handle) 
    in the HII databases, using the new package list specified by PackageList.
    
  Arguments:          
    This              - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
    Handle            - The handle that was registered to the data that is 
                        requested to be updated.
    PackageList       - A pointer to an EFI_HII_PACKAGE_LIST_HEADER package.
    
  Returns:
    EFI_SUCCESS            - The HII database was successfully updated.
    EFI_OUT_OF_RESOURCES   - Unable to allocate enough memory for the updated database.
    EFI_INVALID_PARAMETER  - PackageList was NULL.
    EFI_NOT_FOUND          - The specified Handle is not in database.
     
--*/
;

EFI_STATUS
EFIAPI
HiiListPackageLists (
  IN  CONST EFI_HII_DATABASE_PROTOCOL   *This,
  IN  UINT8                             PackageType,
  IN  CONST EFI_GUID                    *PackageGuid,
  IN  OUT UINTN                         *HandleBufferLength,
  OUT EFI_HII_HANDLE                    *Handle
  )
/*++

  Routine Description:
    This function returns a list of the package handles of the specified type 
    that are currently active in the database. The pseudo-type 
    EFI_HII_PACKAGE_TYPE_ALL will cause all package handles to be listed.
    
  Arguments:          
    This               - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.    
    PackageType        - Specifies the package type of the packages to list or
                         EFI_HII_PACKAGE_TYPE_ALL for all packages to be listed.
    PackageGuid        - If PackageType is EFI_HII_PACKAGE_TYPE_GUID, then this 
                         is the pointer to the GUID which must match the Guid
                         field of EFI_HII_GUID_PACKAGE_GUID_HDR. Otherwise, 
                         it must be NULL.                
    HandleBufferLength - On input, a pointer to the length of the handle buffer. 
                         On output, the length of the handle buffer that is
                         required for the handles found.
    Handle             - An array of EFI_HII_HANDLE instances returned.
        
  Returns:
    EFI_SUCCESS            - The matching handles are outputed successfully.
                             HandleBufferLength is updated with the actual length.
    EFI_BUFFER_TO_SMALL    - The HandleBufferLength parameter indicates that
                             Handle is too small to support the number of handles.
                             HandleBufferLength is updated with a value that will 
                             enable the data to fit.
    EFI_NOT_FOUND          - No matching handle could not be found in database.
    EFI_INVALID_PARAMETER  - Handle or HandleBufferLength was NULL.
    EFI_INVALID_PARAMETER  - PackageType is not a EFI_HII_PACKAGE_TYPE_GUID but
                             PackageGuid is not NULL, PackageType is a EFI_HII_
                             PACKAGE_TYPE_GUID but PackageGuid is NULL.
     
--*/
;

EFI_STATUS
EFIAPI
HiiExportPackageLists (
  IN  CONST EFI_HII_DATABASE_PROTOCOL   *This,
  IN  EFI_HII_HANDLE                    Handle,
  IN  OUT UINTN                         *BufferSize,
  OUT EFI_HII_PACKAGE_LIST_HEADER       *Buffer
  )
/*++

  Routine Description:
    This function will export one or all package lists in the database to a buffer. 
    For each package list exported, this function will call functions registered 
    with EXPORT_PACK and then copy the package list to the buffer.    
    
  Arguments:          
    This               - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
    Handle             - An EFI_HII_HANDLE that corresponds to the desired package
                         list in the HII database to export or NULL to indicate 
                         all package lists should be exported.
    BufferSize         - On input, a pointer to the length of the buffer. 
                         On output, the length of the buffer that is required for
                         the exported data.
    Buffer             - A pointer to a buffer that will contain the results of 
                         the export function.
                         
  Returns:
    EFI_SUCCESS            - Package exported.
    EFI_BUFFER_TO_SMALL    - The HandleBufferLength parameter indicates that Handle
                             is too small to support the number of handles.     
                             HandleBufferLength is updated with a value that will
                             enable the data to fit.
    EFI_NOT_FOUND          - The specifiecd Handle could not be found in the current
                             database.
    EFI_INVALID_PARAMETER  - Handle or Buffer or BufferSize was NULL.
     
--*/ 
;

EFI_STATUS
EFIAPI
HiiRegisterPackageNotify (
  IN  CONST EFI_HII_DATABASE_PROTOCOL   *This,
  IN  UINT8                             PackageType,
  IN  CONST EFI_GUID                    *PackageGuid,
  IN  CONST EFI_HII_DATABASE_NOTIFY     PackageNotifyFn,
  IN  EFI_HII_DATABASE_NOTIFY_TYPE      NotifyType,
  OUT EFI_HANDLE                        *NotifyHandle
  )
/*++

  Routine Description:
    This function registers a function which will be called when specified actions related to packages of
    the specified type occur in the HII database. By registering a function, other HII-related drivers are
    notified when specific package types are added, removed or updated in the HII database.
    Each driver or application which registers a notification should use
    EFI_HII_DATABASE_PROTOCOL.UnregisterPackageNotify() before exiting. 
    
  Arguments:          
    This               - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.    
    PackageType        - Specifies the package type of the packages to list or
                         EFI_HII_PACKAGE_TYPE_ALL for all packages to be listed.    
    PackageGuid        - If PackageType is EFI_HII_PACKAGE_TYPE_GUID, then this is the pointer to
                         the GUID which must match the Guid field of                             
                         EFI_HII_GUID_PACKAGE_GUID_HDR. Otherwise, it must be NULL.          
    PackageNotifyFn    - Points to the function to be called when the event specified by                           
                         NotificationType occurs.
    NotifyType         - Describes the types of notification which this function will be receiving.                     
    NotifyHandle       - Points to the unique handle assigned to the registered notification. Can be used in
                         EFI_HII_DATABASE_PROTOCOL.UnregisterPackageNotify() to stop notifications.                                                                     
                         
  Returns:
    EFI_SUCCESS            - Notification registered successfully.    
    EFI_OUT_OF_RESOURCES   - Unable to allocate necessary data structures    
    EFI_INVALID_PARAMETER  - NotifyHandle is NULL.
    EFI_INVALID_PARAMETER  - PackageGuid is not NULL when PackageType is not
                             EFI_HII_PACKAGE_TYPE_GUID.                     
    EFI_INVALID_PARAMETER  - PackageGuid is NULL when PackageType is EFI_HII_PACKAGE_TYPE_GUID.
     
--*/  
;

EFI_STATUS
EFIAPI 
HiiUnregisterPackageNotify (
  IN CONST EFI_HII_DATABASE_PROTOCOL    *This,
  IN EFI_HANDLE                         NotificationHandle
  )
/*++

  Routine Description:
    Removes the specified HII database package-related notification.
    
  Arguments:          
    This               - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.        
    NotifyHandle       - The handle of the notification function being unregistered.                         
                         
  Returns:
    EFI_SUCCESS            - Notification is unregistered successfully.    
    EFI_NOT_FOUND          - The incoming notification handle does not exist 
                             in current hii database.
     
--*/  
;  

EFI_STATUS
EFIAPI 
HiiFindKeyboardLayouts (
  IN  EFI_HII_DATABASE_PROTOCOL         *This,
  IN  OUT UINT16                        *KeyGuidBufferLength,
  OUT EFI_GUID                          *KeyGuidBuffer
  )
/*++

  Routine Description:
    This routine retrieves an array of GUID values for each keyboard layout that
    was previously registered in the system.
    
  Arguments:          
    This                - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
    KeyGuidBufferLength - On input, a pointer to the length of the keyboard GUID 
                          buffer. On output, the length of the handle buffer 
                          that is required for the handles found.
    KeyGuidBuffer       - An array of keyboard layout GUID instances returned.
    
  Returns:
    EFI_SUCCESS            - KeyGuidBuffer was updated successfully.
    EFI_BUFFER_TOO_SMALL   - The KeyGuidBufferLength parameter indicates   
                             that KeyGuidBuffer is too small to support the
                             number of GUIDs. KeyGuidBufferLength is       
                             updated with a value that will enable the data to fit.
    EFI_INVALID_PARAMETER  - The KeyGuidBuffer or KeyGuidBufferLength was NULL.
    EFI_NOT_FOUND          - There was no keyboard layout.

--*/ 
;

EFI_STATUS
EFIAPI 
HiiGetKeyboardLayout (
  IN  EFI_HII_DATABASE_PROTOCOL         *This,
  IN  EFI_GUID                          *KeyGuid,
  IN OUT UINT16                         *KeyboardLayoutLength,
  OUT EFI_HII_KEYBOARD_LAYOUT           *KeyboardLayout
  )
/*++

  Routine Description:
    This routine retrieves the requested keyboard layout. The layout is a physical description of the keys
    on a keyboard and the character(s) that are associated with a particular set of key strokes.          
    
  Arguments:          
    This                 - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.            
    KeyGuid              - A pointer to the unique ID associated with a given keyboard layout. If KeyGuid is
                           NULL then the current layout will be retrieved.             
    KeyboardLayoutLength - On input, a pointer to the length of the KeyboardLayout buffer. 
                           On output, the length of the data placed into KeyboardLayout.                         
    KeyboardLayout       - A pointer to a buffer containing the retrieved keyboard layout.                                           
    
  Returns:
    EFI_SUCCESS            - The keyboard layout was retrieved successfully.
    EFI_NOT_FOUND          - The requested keyboard layout was not found.
    EFI_INVALID_PARAMETER  - The KeyboardLayout or KeyboardLayoutLength was NULL.
     
--*/    
;

EFI_STATUS
EFIAPI 
HiiSetKeyboardLayout (
  IN EFI_HII_DATABASE_PROTOCOL          *This,
  IN EFI_GUID                           *KeyGuid
  )
/*++

  Routine Description:
    This routine sets the default keyboard layout to the one referenced by KeyGuid. When this routine  
    is called, an event will be signaled of the EFI_HII_SET_KEYBOARD_LAYOUT_EVENT_GUID                 
    group type. This is so that agents which are sensitive to the current keyboard layout being changed
    can be notified of this change.                                                                    
    
  Arguments:          
    This                - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.            
    KeyGuid             - A pointer to the unique ID associated with a given keyboard layout.                                       
    
  Returns:
    EFI_SUCCESS            - The current keyboard layout was successfully set.
    EFI_NOT_FOUND          - The referenced keyboard layout was not found, so action was taken.                                                     
    EFI_INVALID_PARAMETER  - The KeyGuid was NULL.
     
--*/    
;

EFI_STATUS
EFIAPI
HiiGetPackageListHandle (
  IN  EFI_HII_DATABASE_PROTOCOL         *This,
  IN  EFI_HII_HANDLE                    PackageListHandle,
  OUT EFI_HANDLE                        *DriverHandle
  )
/*++

  Routine Description:
    Return the EFI handle associated with a package list.
    
  Arguments:          
    This                - A pointer to the EFI_HII_DATABASE_PROTOCOL instance.            
    PackageListHandle   - An EFI_HII_HANDLE that corresponds to the desired package list in the
                          HIIdatabase.                                                         
    DriverHandle        - On return, contains the EFI_HANDLE which was registered with the package list in
                          NewPackageList().                                                               
                          
  Returns:
    EFI_SUCCESS            - The DriverHandle was returned successfully.    
    EFI_INVALID_PARAMETER  - The PackageListHandle was not valid or DriverHandle was NULL.
     
--*/    
;

//
// EFI_HII_CONFIG_ROUTING_PROTOCOL interfaces
//

EFI_STATUS
EFIAPI 
HiiConfigRoutingExtractConfig (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL  *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
/*++

  Routine Description:
    This function allows a caller to extract the current configuration 
    for one or more named elements from one or more drivers.
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.
    Request                - A null-terminated Unicode string in <MultiConfigRequest> format.
    Progress               - On return, points to a character in the Request string.
                             Points to the string's null terminator if request was
                             successful. Points to the most recent & before the first
                             failing name / value pair (or the beginning of the string
                             if the failure is in the first name / value pair) if the
                             request was not successful.
    Results                - Null-terminated Unicode string in <MultiConfigAltResp>
                             format which has all values filled in for the names
                             in the Request string. String to be allocated by the
                             called function.                                                                                                         
                        
  Returns:              
    EFI_SUCCESS            - The Results string is filled with the values
                             corresponding to all requested names.       
    EFI_OUT_OF_RESOURCES   - Not enough memory to store the parts of the results
                             that must be stored awaiting possible future       
                             protocols.                                                                      
    EFI_NOT_FOUND          - Routing data doesn¡¯t match any known driver.        
                             Progress set to the ¡°G¡± in ¡°GUID¡± of the routing 
                             header that doesn¡¯t match. Note: There is no        
                             requirement that all routing data be validated before
                             any configuration extraction.                            
    EFI_INVALID_PARAMETER  - For example, passing in a NULL for the Request   
                             parameter would result in this type of error. The
                             Progress parameter is set to NULL.               
                             
    EFI_INVALID_PARAMETER  - Illegal syntax. Progress set to most recent & before
                             the error or the beginning of the string.               
        
    EFI_INVALID_PARAMETER  - Unknown name. Progress points to the & before
                             the name in question.                        
                             
--*/    
;
  
EFI_STATUS 
EFIAPI 
HiiConfigRoutingExportConfig (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL  *This,
  OUT EFI_STRING                             *Results
  )
/*++

  Routine Description:
    This function allows the caller to request the current configuration for the 
    entirety of the current HII database and returns the data in a null-terminated Unicode string.        
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.    
    Results                - Null-terminated Unicode string in <MultiConfigAltResp>
                             format which has all values filled in for the names
                             in the Request string. String to be allocated by the 
                             called function. De-allocation is up to the caller.                                                        
                        
  Returns:              
    EFI_SUCCESS            - The Results string is filled with the values
                             corresponding to all requested names.       
    EFI_OUT_OF_RESOURCES   - Not enough memory to store the parts of the results
                             that must be stored awaiting possible future       
                             protocols.                                                                      
    EFI_INVALID_PARAMETER  - For example, passing in a NULL for the Results
                             parameter would result in this type of error.    
                             
--*/    
;  

EFI_STATUS
EFIAPI
HiiConfigRoutingRouteConfig (
  IN  EFI_HII_CONFIG_ROUTING_PROTOCOL        *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
/*++

  Routine Description:
    This function processes the results of processing forms and routes it to the 
    appropriate handlers or storage.
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.
    Configuration          - A null-terminated Unicode string in <MulltiConfigResp> format.
    Progress               - A pointer to a string filled in with the offset of
                             the most recent & before the first failing name / value
                             pair (or the beginning of the string if the failure is
                             in the first name / value pair) or the terminating
                             NULL if all was successful.
                        
  Returns:              
    EFI_SUCCESS            - The results have been distributed or are awaiting
                             distribution.                                    
    EFI_OUT_OF_RESOURCES   - Not enough memory to store the parts of the results
                             that must be stored awaiting possible future       
                             protocols.                                                                      
    EFI_INVALID_PARAMETER  - Passing in a NULL for the Configuration
                             parameter would result in this type of error.    
    EFI_NOT_FOUND          - Target for the specified routing data was not found.        
                             
--*/    
;  
  

EFI_STATUS 
EFIAPI
HiiBlockToConfig (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL  *This,
  IN  CONST EFI_STRING                       ConfigRequest,
  IN  CONST UINT8                            *Block,
  IN  CONST UINTN                            BlockSize,
  OUT EFI_STRING                             *Config,
  OUT EFI_STRING                             *Progress
  )
/*++

  Routine Description:
    This helper function is to be called by drivers to map configuration data stored
    in byte array (¡°block¡±) formats such as UEFI Variables into current configuration strings.
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.    
    ConfigRequest          - A null-terminated Unicode string in <ConfigRequest> format.
    Block                  - Array of bytes defining the block's configuration.
    BlockSize              - Length in bytes of Block.
    Config                 - Filled-in configuration string. String allocated by 
                             the function. Returned only if call is successful.                                                                               
    Progress               - A pointer to a string filled in with the offset of 
                             the most recent & before the first failing name/value
                             pair (or the beginning of the string if the failure
                             is in the first name / value pair) or the terminating
                             NULL if all was successful.
                        
  Returns:              
    EFI_SUCCESS            - The request succeeded. Progress points to the null
                             terminator at the end of the ConfigRequest        
                             string.                                                                        
    EFI_OUT_OF_RESOURCES   - Not enough memory to allocate Config.    
                             Progress points to the first character of
                             ConfigRequest.                           
    EFI_INVALID_PARAMETER  - Passing in a NULL for the ConfigRequest or      
                             Block parameter would result in this type of    
                             error. Progress points to the first character of
                             ConfigRequest.                                   
    EFI_NOT_FOUND          - Target for the specified routing data was not found.
                             Progress points to the ¡°G¡± in ¡°GUID¡± of the     
                             errant routing data.                                
    EFI_DEVICE_ERROR       - Block not large enough. Progress undefined.
    EFI_INVALID_PARAMETER  - Encountered non <BlockName> formatted string.         
                             Block is left updated and Progress points at the ¡®&¡¯
                             preceding the first non-<BlockName>.                  
                                                          
--*/      
;

EFI_STATUS 
EFIAPI
HiiConfigToBlock (
  IN     CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *This,
  IN     CONST EFI_STRING                      ConfigResp,
  IN OUT UINT8                                 *Block,
  IN OUT UINTN                                 *BlockSize,
  OUT    EFI_STRING                            *Progress
  )
/*++

  Routine Description:
    This helper function is to be called by drivers to map configuration strings 
    to configurations stored in byte array (¡°block¡±) formats such as UEFI Variables.
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.    
    ConfigResp             - A null-terminated Unicode string in <ConfigResp> format.
    Block                  - A possibly null array of bytes representing the current 
                             block. Only bytes referenced in the ConfigResp string 
                             in the block are modified. If this parameter is null
                             or if the *BlockSize parameter is (on input) shorter than
                             required by the Configuration string, only the BlockSize 
                             parameter is updated and an appropriate status (see below) 
                             is returned.            
                             
    BlockSize              - The length of the Block in units of UINT8. 
                             On input, this is the size of the Block.
                             On output, if successful, contains the index of the 
                             last modified byte in the Block.
                             
    Progress               - On return, points to an element of the ConfigResp 
                             string filled in with the offset of the most recent
                             '&' before the first failing name / value pair (or 
                             the beginning of the string if the failure is in the 
                             first name / value pair) or the terminating NULL if
                             all was successful.
  Returns:              
    EFI_SUCCESS            - The request succeeded. Progress points to the null
                             terminator at the end of the ConfigResp
                             string.                                           
    EFI_OUT_OF_RESOURCES   - Not enough memory to allocate Config.    
                             Progress points to the first character of
                             ConfigResp.                           
    EFI_INVALID_PARAMETER  - Passing in a NULL for the ConfigResp or         
                             Block parameter would result in this type of error.
                             Progress points to the first character of          
                             ConfigResp.                                                                  
    EFI_NOT_FOUND          - Target for the specified routing data was not found.
                             Progress points to the ¡°G¡± in ¡°GUID¡± of the     
                             errant routing data.                                
    EFI_INVALID_PARAMETER  - Encountered non <BlockName> formatted name /    
                             value pair. Block is left updated and           
                             Progress points at the ¡®&¡¯ preceding the first
                             non-<BlockName>.                                
                             
--*/                         
;

EFI_STATUS 
EFIAPI
HiiGetAltCfg (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL    *This, 
  IN  CONST EFI_STRING                         Configuration, 
  IN  CONST EFI_GUID                           *Guid, 
  IN  CONST EFI_STRING                         Name, 
  IN  CONST EFI_DEVICE_PATH_PROTOCOL           *DevicePath,  
  IN  CONST UINT16                             *AltCfgId,
  OUT EFI_STRING                               *AltCfgResp 
  )
/*++

  Routine Description:
    This helper function is to be called by drivers to extract portions of 
    a larger configuration string.
    
  Arguments:          
    This                   - A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.    
    Configuration          - A null-terminated Unicode string in <MultiConfigAltResp> format.
    Guid                   - A pointer to the GUID value to search for in the 
                             routing portion of the ConfigResp string when retrieving 
                             the requested data. If Guid is NULL, then all GUID 
                             values will be searched for.
    Name                   - A pointer to the NAME value to search for in the 
                             routing portion of the ConfigResp string when retrieving 
                             the requested data. If Name is NULL, then all Name 
                             values will be searched for.                         
    DevicePath             - A pointer to the PATH value to search for in the 
                             routing portion of the ConfigResp string when retrieving 
                             the requested data. If DevicePath is NULL, then all 
                             DevicePath values will be searched for.             
    AltCfgId               - A pointer to the ALTCFG value to search for in the 
                             routing portion of the ConfigResp string when retrieving 
                             the requested data.  If this parameter is NULL, 
                             then the current setting will be retrieved.
    AltCfgResp             - A pointer to a buffer which will be allocated by the 
                             function which contains the retrieved string as requested.  
                             This buffer is only allocated if the call was successful. 
    
  Returns:              
    EFI_SUCCESS            - The request succeeded. The requested data was extracted 
                             and placed in the newly allocated AltCfgResp buffer.
    EFI_OUT_OF_RESOURCES   - Not enough memory to allocate AltCfgResp.    
    EFI_INVALID_PARAMETER  - Any parameter is invalid.
    EFI_NOT_FOUND          - Target for the specified routing data was not found.
                             
--*/        
;


//
// Global variables
//
extern EFI_EVENT gHiiKeyboardLayoutChanged;

#endif
