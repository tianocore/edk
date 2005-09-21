/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    ConsoleLib.c
    
Abstract: 
    
    It provides library API implementation for console's wide glyph support
    The width of unicode is defined by standard glyph. The unicode supported
    here conforms to the Unicode Standard Version 2.0

Revision History
--*/

#include "ConsoleLib.h"

typedef struct {
  CHAR16  WChar;
  UINT32  Width;
} UNICODE_WIDTH_ENTRY;

UNICODE_WIDTH_ENTRY mUnicodeWidthTable[] = {
  //
  // General script area
  //
  {0x1FFF,  1},
  /*
   * Merge the blocks and replace them with the above entry as they fall to 
   * the same category and they are all narrow glyph. This will reduce search
   * time and table size. The merge will omit the reserved code.
   *
   * Remove the above item if below is un-commented.
   *
  {0x007F,  1},       // C0 controls and basic Latin. 0x0000-0x007F
  {0x00FF,  1},       // C1 controls and Latin-1 support. 0x0080-0x00FF
  {0x017F,  1},       // Latin extended-A. 0x0100-0x017F
  {0x024F,  1},       // Latin extended-B. 0x0180-0x024F
  {0x02AF,  1},       // IPA extensions. 0x0250-0x02AF
  {0x02FF,  1},       // Spacing modifier letters. 0x02B0-0x02FF
  {0x036F,  1},       // Combining diacritical marks. 0x0300-0x036F
  {0x03FF,  1},       // Greek. 0x0370-0x03FF
  {0x04FF,  1},       // Cyrillic. 0x0400-0x04FF
  {0x052F,  0},       // Unassigned. As Armenian in ver3.0. 0x0500-0x052F
  {0x058F,  1},       // Armenian. 0x0530-0x058F
  {0x05FF,  1},       // Hebrew. 0x0590-0x05FF
  {0x06FF,  1},       // Arabic. 0x0600-0x06FF
  {0x08FF,  0},       // Unassigned. 0x0700-0x08FF
  {0x097F,  1},       // Devanagari. 0x0900-0x097F
  {0x09FF,  1},       // Bengali. 0x0980-0x09FF
  {0x0A7F,  1},       // Gurmukhi. 0x0A00-0x0A7F
  {0x0AFF,  1},       // Gujarati. 0x0A80-0x0AFF
  {0x0B7F,  1},       // Oriya. 0x0B00-0x0B7F
  {0x0BFF,  1},       // Tamil. (See page 7-92). 0x0B80-0x0BFF
  {0x0C7F,  1},       // Telugu. 0x0C00-0x0C7F
  {0x0CFF,  1},       // Kannada. (See page 7-100). 0x0C80-0x0CFF
  {0x0D7F,  1},       // Malayalam (See page 7-104). 0x0D00-0x0D7F
  {0x0DFF,  0},       // Unassigned. 0x0D80-0x0DFF
  {0x0E7F,  1},       // Thai. 0x0E00-0x0E7F
  {0x0EFF,  1},       // Lao. 0x0E80-0x0EFF
  {0x0FBF,  1},       // Tibetan. 0x0F00-0x0FBF
  {0x109F,  0},       // Unassigned. 0x0FC0-0x109F
  {0x10FF,  1},       // Georgian. 0x10A0-0x10FF
  {0x11FF,  1},       // Hangul Jamo. 0x1100-0x11FF
  {0x1DFF,  0},       // Unassigned. 0x1200-0x1DFF
  {0x1EFF,  1},       // Latin extended additional. 0x1E00-0x1EFF
  {0x1FFF,  1},       // Greek extended. 0x1F00-0x1FFF
  *
  */

  //
  // Symbol area
  //
  {0x2FFF,  1},
  /*
   * Merge the blocks and replace them with the above entry as they fall to 
   * the same category and they are all narrow glyph. This will reduce search
   * time and table size. The merge will omit the reserved code.
   *
   * Remove the above item if below is un-commented.
   *
  {0x206F,  1},       // General punctuation. (See page7-154). 0x200-0x206F
  {0x209F,  1},       // Superscripts and subscripts. 0x2070-0x209F
  {0x20CF,  1},       // Currency symbols. 0x20A0-0x20CF
  {0x20FF,  1},       // Combining diacritical marks for symbols. 0x20D0-0x20FF
  {0x214F,  1},       // Letterlike sympbols. 0x2100-0x214F
  {0x218F,  1},       // Number forms. 0x2150-0x218F
  {0x21FF,  1},       // Arrows. 0x2190-0x21FF
  {0x22FF,  1},       // Mathematical operators. 0x2200-0x22FF
  {0x23FF,  1},       // Miscellaneous technical. 0x2300-0x23FF
  {0x243F,  1},       // Control pictures. 0x2400-0x243F
  {0x245F,  1},       // Optical character recognition. 0x2440-0x245F
  {0x24FF,  1},       // Enclosed alphanumerics. 0x2460-0x24FF
  {0x257F,  1},       // Box drawing. 0x2500-0x257F
  {0x259F,  1},       // Block elements. 0x2580-0x259F
  {0x25FF,  1},       // Geometric shapes. 0x25A0-0x25FF
  {0x26FF,  1},       // Miscellaneous symbols. 0x2600-0x26FF
  {0x27BF,  1},       // Dingbats. 0x2700-0x27BF
  {0x2FFF,  0},       // Reserved. 0x27C0-0x2FFF
  *
  */

  //
  // CJK phonetics and symbol area
  //
  {0x33FF,  2},
  /*
   * Merge the blocks and replace them with the above entry as they fall to 
   * the same category and they are all wide glyph. This will reduce search
   * time and table size. The merge will omit the reserved code.
   *
   * Remove the above item if below is un-commented.
   *
  {0x303F,  2},       // CJK symbols and punctuation. 0x3000-0x303F
  {0x309F,  2},       // Hiragana. 0x3040-0x309F
  {0x30FF,  2},       // Katakana. 0x30A0-0x30FF
  {0x312F,  2},       // Bopomofo. 0x3100-0x312F
  {0x318F,  2},       // Hangul compatibility jamo. 0x3130-0x318F
  {0x319F,  2},       // Kanbun. 0x3190-0x319F
  {0x31FF,  0},       // Reserved. As Bopomofo extended in ver3.0. 0x31A0-0x31FF
  {0x32FF,  2},       // Enclosed CJK letters and months. 0x3200-0x32FF
  {0x33FF,  2},       // CJK compatibility. 0x3300-0x33FF
  *
  */

  //
  // CJK ideograph area
  //
  {0x9FFF,  2},
  /*
   * Merge the blocks and replace them with the above entry as they fall to 
   * the same category and they are all wide glyph. This will reduce search
   * time and table size. The merge will omit the reserved code.
   *
   * Remove the above item if below is un-commented.
   *
  {0x4DFF,  0},       // Reserved. 0x3400-0x4DBF as CJK unified ideographs 
                      // extension A in ver3.0. 0x3400-0x4DFF
  {0x9FFF,  2},       // CJK unified ideographs. 0x4E00-0x9FFF
  *
  */

  //
  // Reserved
  //
  {0xABFF,  0},       // Reserved. 0xA000-0xA490 as Yi syllables. 0xA490-0xA4D0
  // as Yi radicals in ver3.0. 0xA000-0xABFF
  //
  // Hangul syllables
  //
  {0xD7FF,  2},
  /*
   * Merge the blocks and replace them with the above entry as they fall to 
   * the same category and they are all wide glyph. This will reduce search
   * time and table size. The merge will omit the reserved code.
   *
   * Remove the above item if below is un-commented.
   *
  {0xD7A3,  2},       // Hangul syllables. 0xAC00-0xD7A3
  {0xD7FF,  0},       // Reserved. 0xD7A3-0xD7FF
  *
  */

  //
  // Surrogates area
  //
  {0xDFFF,  0},       // Surrogates, not used now. 0xD800-0xDFFF

  //
  // Private use area
  //
  {0xF8FF,  0},       // Private use area. 0xE000-0xF8FF

  //
  // Compatibility area and specials
  //
  {0xFAFF,  2},       // CJK compatibility ideographs. 0xF900-0xFAFF
  {0xFB4F,  1},       // Alphabetic presentation forms. 0xFB00-0xFB4F
  {0xFDFF,  1},       // Arabic presentation forms-A. 0xFB50-0xFDFF
  {0xFE1F,  0},       // Reserved. As variation selectors in ver3.0. 0xFE00-0xFE1F
  {0xFE2F,  1},       // Combining half marks. 0xFE20-0xFE2F
  {0xFE4F,  2},       // CJK compatibility forms. 0xFE30-0xFE4F
  {0xFE6F,  1},       // Small Form Variants. 0xFE50-0xFE6F
  {0xFEFF,  1},       // Arabic presentation forms-B. 0xFE70-0xFEFF
  {0xFFEF,  1},       // Half width and full width forms. 0xFF00-0xFFEF
  {0xFFFF,  0},       // Speicials. 0xFFF0-0xFFFF
};

EFI_STATUS
GetGlyphWidth (
  IN  CHAR16                              UnicodeChar,
  OUT UINT32                              *GlyphWidth
  )
/*++
  Routine Description:
  
    Library function to get the width of a unicode character.
  
  Arguments:
  
    UnicodeChar - The unicode character to be inquired

    GlyphWidth  - The returning width of the character

  Returns:
  
    EFI_SUCCESS
        The width of the character is found and successfully returned.
    
    EFI_UNSUPPORTED
        The character is not found.       
                
--*/
{
  UINTN               Index;
  UINTN               Low;
  UINTN               High;
  UNICODE_WIDTH_ENTRY *Item;

  Item  = NULL;
  Low   = 0;
  High  = (sizeof (mUnicodeWidthTable)) / (sizeof (UNICODE_WIDTH_ENTRY)) - 1;
  while (Low <= High) {
    Index = (Low + High) >> 1;
    Item  = &(mUnicodeWidthTable[Index]);
    if (Index == 0) {
      if (UnicodeChar <= Item->WChar) {
        break;
      }

      return EFI_UNSUPPORTED;
    }

    if (UnicodeChar > Item->WChar) {
      Low = Index + 1;
    } else if (UnicodeChar <= mUnicodeWidthTable[Index - 1].WChar) {
      High = Index - 1;
    } else {
      //
      // Index - 1 < UnicodeChar <= Index. Found
      //
      break;
    }
  }

  if (Low <= High) {
    *GlyphWidth = Item->Width;
    if (*GlyphWidth != 0) {
      return EFI_SUCCESS;
    }
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS
UnicodeStrDisplayLen (
  IN  CHAR16                              *UnicodeStr,
  OUT UINT32                              *DisplayLength
  )
/*++
  Routine Description:
  
    Library function to get the length of a unicode string displayed on screen
  
  Arguments:
  
    UnicodeStr - The unicode character to be inquired

    DisplayLength - The returning width of the character

  Returns:
  
    EFI_SUCCESS
        The length of the string is successfully returned.

    EFI_UNSUPPORTED
        Some of the characters is not supported by our glyph.

--*/
{
  UINT32      Length;
  UINT32      Width;
  EFI_STATUS  Status;

  //
  // bugbug. Shall the control characters such as "BackSpace",
  // "LineFeed" etc be considered?
  //
  Length = 0;
  while (*UnicodeStr) {
    Status = GetGlyphWidth (*UnicodeStr, &Width);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Length += Width;
    UnicodeStr++;
  }

  *DisplayLength = Length;

  return EFI_SUCCESS;
}
