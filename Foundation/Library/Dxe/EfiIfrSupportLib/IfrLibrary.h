/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  IfrLibrary.h

Abstract:


Revision History

--*/
#ifndef _IFRLIBRARY_H
#define _IFRLIBRARY_H

#include "Tiano.h"
#include "EfiDriverLib.h"

#include EFI_PROTOCOL_DEFINITION(Hii)
#include EFI_GUID_DEFINITION(GlobalVariable)

#define  DEFAULT_FORM_BUFFER_SIZE   0xFFFF
#define  DEFAULT_STRING_BUFFER_SIZE 0xFFFF

#pragma pack(1)
typedef struct {
  CHAR16                    *OptionString;    // Passed in string to generate a token for in a truly dynamic form creation
  STRING_REF                StringToken;      // This is used when creating a single op-code without generating a StringToken (have one already)
  UINT16                    Value;
  UINT8                     Flags;
  UINT16                    Key;
} IFR_OPTION;
#pragma pack()

EFI_STATUS
GetCurrentLanguage (
  OUT     CHAR16              *Lang
  );

EFI_STATUS
AddString (
  IN      VOID                *StringBuffer,
  IN      CHAR16              *Language,
  IN      CHAR16              *String,
  IN OUT  STRING_REF          *StringToken
  );

EFI_STATUS
AddOpCode (
  IN      VOID                *FormBuffer,
  IN OUT  VOID                *OpCodeData
  );

EFI_STATUS
CreateFormSet (
  IN      CHAR16              *FormSetTitle,
  IN      EFI_GUID            *Guid,
  IN      UINT8               Class,
  IN      UINT8               SubClass,
  IN OUT  VOID                **FormBuffer,
  IN OUT  VOID                **StringBuffer
  );

EFI_STATUS
CreateForm (
  IN      CHAR16              *FormTitle,
  IN      UINT16              FormId,
  IN OUT  VOID                *FormBuffer,
  IN OUT  VOID                *StringBuffer
  );

EFI_STATUS
CreateSubTitle (
  IN      CHAR16              *SubTitle,
  IN OUT  VOID                *FormBuffer,
  IN OUT  VOID                *StringBuffer
  );

EFI_STATUS
CreateText (
  IN      CHAR16              *String,
  IN      CHAR16              *String2,
  IN      CHAR16              *String3,
  IN      UINT8               Flags,
  IN      UINT16              Key,
  IN OUT  VOID                *FormBuffer,
  IN OUT  VOID                *StringBuffer
  );

EFI_STATUS
CreateGoto (
  IN      UINT16              FormId,
  IN      CHAR16              *Prompt,
  IN OUT  VOID                *FormBuffer,
  IN OUT  VOID                *StringBuffer
  );

EFI_STATUS
CreateOneOf (
  IN      UINT16              QuestionId,
  IN      UINT8               DataWidth,
  IN      CHAR16              *Prompt,
  IN      CHAR16              *Help,
  IN      IFR_OPTION          *OptionsList,
  IN      UINTN               OptionCount,
  IN OUT  VOID                *FormBuffer,
  IN OUT  VOID                *StringBuffer
  );

EFI_STATUS
CreateOrderedList (
  IN      UINT16              QuestionId,
  IN      UINT8               MaxEntries,
  IN      CHAR16              *Prompt,
  IN      CHAR16              *Help,
  IN      IFR_OPTION          *OptionsList,
  IN      UINTN               OptionCount,
  IN OUT  VOID                *FormBuffer,
  IN OUT  VOID                *StringBuffer
  );

EFI_STATUS
CreateCheckBox (
  IN      UINT16              QuestionId,
  IN      UINT8               DataWidth,
  IN      CHAR16              *Prompt,
  IN      CHAR16              *Help,
  IN      UINT8               Flags,
  IN OUT  VOID                *FormBuffer,
  IN OUT  VOID                *StringBuffer
  );

EFI_STATUS
CreateNumeric (
  IN      UINT16              QuestionId,
  IN      UINT8               DataWidth,
  IN      CHAR16              *Prompt,
  IN      CHAR16              *Help,
  IN      UINT16              Minimum,
  IN      UINT16              Maximum,
  IN      UINT16              Step,
  IN      UINT16              Default,
  IN      UINT8               Flags,
  IN      UINT16              Key,
  IN OUT  VOID                *FormBuffer,
  IN OUT  VOID                *StringBuffer
  );

EFI_STATUS
CreateString (
  IN      UINT16              QuestionId,
  IN      UINT8               DataWidth,
  IN      CHAR16              *Prompt,
  IN      CHAR16              *Help,
  IN      UINT8               MinSize,
  IN      UINT8               MaxSize,
  IN      UINT8               Flags,
  IN      UINT16              Key,
  IN OUT  VOID                *FormBuffer,
  IN OUT  VOID                *StringBuffer
  );

EFI_STATUS
ExtractDataFromHiiHandle (
  IN      EFI_HII_HANDLE      HiiHandle,
  IN OUT  UINT16              *ImageLength,
  OUT     UINT8               *DefaultImage,
  OUT     EFI_GUID            *Guid
  );

EFI_HII_HANDLE
FindHiiHandle (
  IN OUT EFI_HII_PROTOCOL   **HiiProtocol, OPTIONAL
  IN     EFI_GUID            *Guid
  );

EFI_STATUS
CreateSubTitleOpCode (
  IN      STRING_REF          StringToken,
  IN OUT  VOID                *FormBuffer
  );

EFI_STATUS
CreateTextOpCode (
  IN      STRING_REF          StringToken,
  IN      STRING_REF          StringTokenTwo,
  IN      STRING_REF          StringTokenThree,
  IN      UINT8               Flags,
  IN      UINT16              Key,
  IN OUT  VOID                *FormBuffer
  );

EFI_STATUS
CreateGotoOpCode (
  IN      UINT16              FormId,
  IN      STRING_REF          StringToken,
  IN      STRING_REF          StringTokenTwo,
  IN      UINT8               Flags,
  IN      UINT16              Key,
  IN OUT  VOID                *FormBuffer
  );

EFI_STATUS
CreateOneOfOpCode (
  IN      UINT16              QuestionId,
  IN      UINT8               DataWidth,
  IN      STRING_REF          PromptToken,
  IN      STRING_REF          HelpToken,
  IN      IFR_OPTION          *OptionsList,
  IN      UINTN               OptionCount,
  IN OUT  VOID                *FormBuffer
  );

EFI_STATUS
CreateOrderedListOpCode (
  IN      UINT16              QuestionId,
  IN      UINT8               MaxEntries,
  IN      STRING_REF          PromptToken,
  IN      STRING_REF          HelpToken,
  IN      IFR_OPTION          *OptionsList,
  IN      UINTN               OptionCount,
  IN OUT  VOID                *FormBuffer
  );

EFI_STATUS
CreateCheckBoxOpCode (
  IN      UINT16              QuestionId,
  IN      UINT8               DataWidth,
  IN      STRING_REF          PromptToken,
  IN      STRING_REF          HelpToken,
  IN      UINT8               Flags,
  IN      UINT16              Key,
  IN OUT  VOID                *FormBuffer
  );

EFI_STATUS
CreateNumericOpCode (
  IN      UINT16              QuestionId,
  IN      UINT8               DataWidth,
  IN      STRING_REF          PromptToken,
  IN      STRING_REF          HelpToken,
  IN      UINT16              Minimum,
  IN      UINT16              Maximum,
  IN      UINT16              Step,
  IN      UINT16              Default,
  IN      UINT8               Flags,
  IN      UINT16              Key,
  IN OUT  VOID                *FormBuffer
  );

EFI_STATUS
CreateStringOpCode (
  IN      UINT16              QuestionId,
  IN      UINT8               DataWidth,
  IN      STRING_REF          PromptToken,
  IN      STRING_REF          HelpToken,
  IN      UINT8               MinSize,
  IN      UINT8               MaxSize,
  IN      UINT8               Flags,
  IN      UINT16              Key,
  IN OUT  VOID                *FormBuffer
  );

EFI_STATUS
ValidateDataFromHiiHandle (
  IN      EFI_HII_HANDLE      HiiHandle,
  OUT     BOOLEAN             *Results
  );

EFI_STATUS
CreateBannerOpCode (
  IN      UINT16              Title,
  IN      UINT16              LineNumber,
  IN      UINT8               Alignment,
  IN OUT  VOID                *FormBuffer
  );

EFI_HII_PACKAGES *
PreparePackages(
  IN      UINTN               NumberOfPackages,
  IN      EFI_GUID            *GuidId,
  ...
  );
#endif