/*++
Copyright (c) 2004 - 2009, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:
  Presentation.c

Abstract:

  Some presentation routines.

--*/

#include "Setup.h"
#include "Ui.h"

BOOLEAN            mHiiPackageListUpdated;
UI_MENU_SELECTION  *gCurrentSelection;

VOID
ClearLines (
  UINTN                                       LeftColumn,
  UINTN                                       RightColumn,
  UINTN                                       TopRow,
  UINTN                                       BottomRow,
  UINTN                                       TextAttribute
  )
/*++

Routine Description:
  Clear retangle with specified text attribute.

Arguments:
  LeftColumn     - Left column of retangle.
  RightColumn    - Right column of retangle.
  TopRow         - Start row of retangle.
  BottomRow      - End row of retangle.
  TextAttribute  - The character foreground and background.

Returns:
  None.

--*/
{
  CHAR16  *Buffer;
  UINTN   Row;

  //
  // For now, allocate an arbitrarily long buffer
  //
  Buffer = EfiLibAllocateZeroPool (0x10000);
  ASSERT (Buffer != NULL);

  //
  // Set foreground and background as defined
  //
  gST->ConOut->SetAttribute (gST->ConOut, TextAttribute);

  //
  // Much faster to buffer the long string instead of print it a character at a time
  //
  SetUnicodeMem (Buffer, RightColumn - LeftColumn, L' ');

  //
  // Clear the desired area with the appropriate foreground/background
  //
  for (Row = TopRow; Row <= BottomRow; Row++) {
    PrintStringAt (LeftColumn, Row, Buffer);
  }

  gST->ConOut->SetCursorPosition (gST->ConOut, LeftColumn, TopRow);

  gBS->FreePool (Buffer);
  return ;
}

VOID
NewStrCat (
  CHAR16                                      *Destination,
  CHAR16                                      *Source
  )
{
  UINTN Length;

  for (Length = 0; Destination[Length] != 0; Length++)
    ;

  //
  // We now have the length of the original string
  // We can safely assume for now that we are concatenating a narrow value to this string.
  // For instance, the string is "XYZ" and cat'ing ">"
  // If this assumption changes, we need to make this routine a bit more complex
  //
  Destination[Length] = NARROW_CHAR;
  Length++;

  EfiStrCpy (Destination + Length, Source);
}

UINTN
GetStringWidth (
  CHAR16                                      *String
  )
{
  UINTN Index;
  UINTN Count;
  UINTN IncrementValue;

  Index           = 0;
  Count           = 0;
  IncrementValue  = 1;

  do {
    //
    // Advance to the null-terminator or to the first width directive
    //
    for (;
         (String[Index] != NARROW_CHAR) && (String[Index] != WIDE_CHAR) && (String[Index] != 0);
         Index++, Count = Count + IncrementValue
        )
      ;

    //
    // We hit the null-terminator, we now have a count
    //
    if (String[Index] == 0) {
      break;
    }
    //
    // We encountered a narrow directive - strip it from the size calculation since it doesn't get printed
    // and also set the flag that determines what we increment by.(if narrow, increment by 1, if wide increment by 2)
    //
    if (String[Index] == NARROW_CHAR) {
      //
      // Skip to the next character
      //
      Index++;
      IncrementValue = 1;
    } else {
      //
      // Skip to the next character
      //
      Index++;
      IncrementValue = 2;
    }
  } while (String[Index] != 0);

  //
  // Increment by one to include the null-terminator in the size
  //
  Count++;

  return Count * sizeof (CHAR16);
}

VOID
DisplayPageFrame (
  VOID
  )
{
  UINTN                  Index;
  UINT8                  Line;
  UINT8                  Alignment;
  CHAR16                 Character;
  CHAR16                 *Buffer;
  CHAR16                 *StrFrontPageBanner;
  UINTN                  Row;
  EFI_SCREEN_DESCRIPTOR  LocalScreen;

  EfiZeroMem (&LocalScreen, sizeof (EFI_SCREEN_DESCRIPTOR));
  gST->ConOut->QueryMode (gST->ConOut, gST->ConOut->Mode->Mode, &LocalScreen.RightColumn, &LocalScreen.BottomRow);
  ClearLines (0, LocalScreen.RightColumn, 0, LocalScreen.BottomRow, KEYHELP_BACKGROUND);

  EfiCopyMem (&LocalScreen, &gScreenDimensions, sizeof (EFI_SCREEN_DESCRIPTOR));

  //
  // For now, allocate an arbitrarily long buffer
  //
  Buffer = EfiLibAllocateZeroPool (0x10000);
  ASSERT (Buffer != NULL);

  Character = BOXDRAW_HORIZONTAL;

  for (Index = 0; Index + 2 < (LocalScreen.RightColumn - LocalScreen.LeftColumn); Index++) {
    Buffer[Index] = Character;
  }

  if ((gClassOfVfr & FORMSET_CLASS_FRONT_PAGE) != 0) {
    //
    //    ClearLines(0, LocalScreen.RightColumn, 0, BANNER_HEIGHT-1, BANNER_TEXT | BANNER_BACKGROUND);
    //
    ClearLines (
      LocalScreen.LeftColumn,
      LocalScreen.RightColumn,
      LocalScreen.TopRow,
      FRONT_PAGE_HEADER_HEIGHT - 1 + LocalScreen.TopRow,
      BANNER_TEXT | BANNER_BACKGROUND
      );
    //
    //    for (Line = 0; Line < BANNER_HEIGHT; Line++) {
    //
    for (Line = (UINT8) LocalScreen.TopRow; Line < BANNER_HEIGHT + (UINT8) LocalScreen.TopRow; Line++) {
      //
      //      for (Alignment = 0; Alignment < BANNER_COLUMNS; Alignment++) {
      //
      for (Alignment = (UINT8) LocalScreen.LeftColumn;
           Alignment < BANNER_COLUMNS + (UINT8) LocalScreen.LeftColumn;
           Alignment++
          ) {
        if (gBannerData->Banner[Line - (UINT8) LocalScreen.TopRow][Alignment - (UINT8) LocalScreen.LeftColumn] != 0x0000) {
          StrFrontPageBanner = GetToken (
                                gBannerData->Banner[Line - (UINT8) LocalScreen.TopRow][Alignment - (UINT8) LocalScreen.LeftColumn],
                                gFrontPageHandle
                                );
        } else {
          continue;
        }

        switch (Alignment - LocalScreen.LeftColumn) {
        case 0:
          //
          // Handle left column
          //
          PrintStringAt (LocalScreen.LeftColumn, Line, StrFrontPageBanner);
          break;

        case 1:
          //
          // Handle center column
          //
          PrintStringAt (
            LocalScreen.LeftColumn + (LocalScreen.RightColumn - LocalScreen.LeftColumn) / 3,
            Line,
            StrFrontPageBanner
            );
          break;

        case 2:
          //
          // Handle right column
          //
          PrintStringAt (
            LocalScreen.LeftColumn + (LocalScreen.RightColumn - LocalScreen.LeftColumn) * 2 / 3,
            Line,
            StrFrontPageBanner
            );
          break;
        }

        gBS->FreePool (StrFrontPageBanner);
      }
    }
  }

  ClearLines (
    LocalScreen.LeftColumn,
    LocalScreen.RightColumn,
    LocalScreen.BottomRow - STATUS_BAR_HEIGHT - FOOTER_HEIGHT,
    LocalScreen.BottomRow - STATUS_BAR_HEIGHT - 1,
    KEYHELP_TEXT | KEYHELP_BACKGROUND
    );

  if ((gClassOfVfr & FORMSET_CLASS_FRONT_PAGE) == 0) {
    ClearLines (
      LocalScreen.LeftColumn,
      LocalScreen.RightColumn,
      LocalScreen.TopRow,
      LocalScreen.TopRow + NONE_FRONT_PAGE_HEADER_HEIGHT - 1,
      TITLE_TEXT | TITLE_BACKGROUND
      );
    //
    // Print Top border line
    // +------------------------------------------------------------------------------+
    // ?                                                                             ?
    // +------------------------------------------------------------------------------+
    //
    Character = BOXDRAW_DOWN_RIGHT;

    PrintChar (Character);
    PrintString (Buffer);

    Character = BOXDRAW_DOWN_LEFT;
    PrintChar (Character);

    Character = BOXDRAW_VERTICAL;
    for (Row = LocalScreen.TopRow + 1; Row <= LocalScreen.TopRow + NONE_FRONT_PAGE_HEADER_HEIGHT - 2; Row++) {
      PrintCharAt (LocalScreen.LeftColumn, Row, Character);
      PrintCharAt (LocalScreen.RightColumn - 1, Row, Character);
    }

    Character = BOXDRAW_UP_RIGHT;
    PrintCharAt (LocalScreen.LeftColumn, LocalScreen.TopRow + NONE_FRONT_PAGE_HEADER_HEIGHT - 1, Character);
    PrintString (Buffer);

    Character = BOXDRAW_UP_LEFT;
    PrintChar (Character);

    if ((gClassOfVfr & FORMSET_CLASS_PLATFORM_SETUP) != 0) {
      //
      // Print Bottom border line
      // +------------------------------------------------------------------------------+
      // ?                                                                             ?
      // +------------------------------------------------------------------------------+
      //
      Character = BOXDRAW_DOWN_RIGHT;
      PrintCharAt (LocalScreen.LeftColumn, LocalScreen.BottomRow - STATUS_BAR_HEIGHT - FOOTER_HEIGHT, Character);

      PrintString (Buffer);

      Character = BOXDRAW_DOWN_LEFT;
      PrintChar (Character);
      Character = BOXDRAW_VERTICAL;
      for (Row = LocalScreen.BottomRow - STATUS_BAR_HEIGHT - FOOTER_HEIGHT + 1;
           Row <= LocalScreen.BottomRow - STATUS_BAR_HEIGHT - 2;
           Row++
          ) {
        PrintCharAt (LocalScreen.LeftColumn, Row, Character);
        PrintCharAt (LocalScreen.RightColumn - 1, Row, Character);
      }

      Character = BOXDRAW_UP_RIGHT;
      PrintCharAt (LocalScreen.LeftColumn, LocalScreen.BottomRow - STATUS_BAR_HEIGHT - 1, Character);

      PrintString (Buffer);

      Character = BOXDRAW_UP_LEFT;
      PrintChar (Character);
    }
  }

  gBS->FreePool (Buffer);

}

EFI_STATUS
EvaluateFormExpressions (
  IN FORM_BROWSER_FORMSET  *FormSet,
  IN FORM_BROWSER_FORM     *Form
  )
/*++

Routine Description:
  Evaluate all expressions in a Form.

Arguments:
  FormSet    - FormSet this Form belongs to.
  Form       - The Form.

Returns:
  EFI_SUCCESS - The expression evaluated successfuly

--*/
{
  EFI_STATUS       Status;
  EFI_LIST_ENTRY   *Link;
  FORM_EXPRESSION  *Expression;

  Link = GetFirstNode (&Form->ExpressionListHead);
  while (!IsNull (&Form->ExpressionListHead, Link)) {
    Expression = FORM_EXPRESSION_FROM_LINK (Link);
    Link = GetNextNode (&Form->ExpressionListHead, Link);

    if (Expression->Type == EFI_HII_EXPRESSION_INCONSISTENT_IF ||
        Expression->Type == EFI_HII_EXPRESSION_NO_SUBMIT_IF) {
      //
      // Postpone Form validation to Question editing or Form submiting
      //
      continue;
    }

    Status = EvaluateExpression (FormSet, Form, Expression);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/*
+------------------------------------------------------------------------------+
?                                 Setup Page                                  ?
+------------------------------------------------------------------------------+

















+------------------------------------------------------------------------------+
|                            F9=Reset to Defaults          F10=Save and Exit   |
| ^"=Move Highlight          <Spacebar> Toggles Checkbox   Esc=Discard Changes |
+------------------------------------------------------------------------------+
*/
EFI_STATUS
DisplayForm (
  IN OUT UI_MENU_SELECTION           *Selection
  )
{
  CHAR16                 *StringPtr;
  UINTN                  Count;
  UINT16                 MenuItemCount;
  EFI_HII_HANDLE         Handle;
  UINT16                 FormId;
  BOOLEAN                Suppress;
  EFI_SCREEN_DESCRIPTOR  LocalScreen;
  UINT16                 Width;
  UINTN                  ArrayEntry;
  CHAR16                 *OutputString;
  EFI_LIST_ENTRY         *Link;
  FORM_BROWSER_STATEMENT *Statement;
  UINT16                 NumberOfLines;
  EFI_STATUS             Status;
  UI_MENU_OPTION         *MenuOption;

  Handle        = Selection->Handle;
  FormId        = 0;
  MenuItemCount = 0;
  ArrayEntry    = 0;
  OutputString  = NULL;

  UiInitMenu ();

  EfiCopyMem (&LocalScreen, &gScreenDimensions, sizeof (EFI_SCREEN_DESCRIPTOR));

  StringPtr = GetToken (Selection->Form->FormTitle, Handle);

  if ((gClassOfVfr & FORMSET_CLASS_FRONT_PAGE) == 0) {
    gST->ConOut->SetAttribute (gST->ConOut, TITLE_TEXT | TITLE_BACKGROUND);
    PrintStringAt (
      (LocalScreen.RightColumn + LocalScreen.LeftColumn - GetStringWidth (StringPtr) / 2) / 2,
      LocalScreen.TopRow + 1,
      StringPtr
      );
  }

  //
  // Remove Buffer allocated for StringPtr after it has been used.
  //
  gBS->FreePool (StringPtr);

  //
  // Evaluate all the Expressions in this Form
  //
  Status = EvaluateFormExpressions (Selection->FormSet, Selection->Form);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Selection->FormEditable = FALSE;
  Link = GetFirstNode (&Selection->Form->StatementListHead);
  while (!IsNull (&Selection->Form->StatementListHead, Link)) {
    Statement = FORM_BROWSER_STATEMENT_FROM_LINK (Link);

    if (Statement->SuppressExpression != NULL) {
      Suppress = Statement->SuppressExpression->Result.Value.b;
    } else {
      Suppress = FALSE;
    }

    if (Statement->DisableExpression != NULL) {
      Suppress = Suppress || Statement->DisableExpression->Result.Value.b;
    }

    if (!Suppress) {
      StringPtr = GetToken (Statement->Prompt, Handle);

      Width     = GetWidth (Statement, Handle);

      NumberOfLines = 1;
      ArrayEntry = 0;
      for (Count = 0; GetLineByWidth (StringPtr, Width, &ArrayEntry, &OutputString) != 0x0000;) {
        //
        // If there is more string to process print on the next row and increment the Skip value
        //
        if (EfiStrLen (&StringPtr[ArrayEntry])) {
          NumberOfLines++;
        }

        gBS->FreePool (OutputString);
      }

      //
      // We are NOT!! removing this StringPtr buffer via FreePool since it is being used in the menuoptions, we will do
      // it in UiFreeMenu.
      //
      MenuOption = UiAddMenuOption (StringPtr, Selection->Handle, Statement, NumberOfLines, MenuItemCount);
      MenuItemCount++;

      if (MenuOption->IsQuestion && !MenuOption->ReadOnly) {
        //
        // At least one item is not readonly, this Form is considered as editable
        //
        Selection->FormEditable = TRUE;
      }
    }

    Link = GetNextNode (&Selection->Form->StatementListHead, Link);
  }

  Status = UiDisplayMenu (Selection);

  UiFreeMenu ();

  return Status;
}

VOID
InitializeBrowserStrings (
  VOID
  )
{
  gFunctionNineString   = GetToken (STRING_TOKEN (FUNCTION_NINE_STRING), gHiiHandle);
  gFunctionTenString    = GetToken (STRING_TOKEN (FUNCTION_TEN_STRING), gHiiHandle);
  gEnterString          = GetToken (STRING_TOKEN (ENTER_STRING), gHiiHandle);
  gEnterCommitString    = GetToken (STRING_TOKEN (ENTER_COMMIT_STRING), gHiiHandle);
  gEnterEscapeString    = GetToken (STRING_TOKEN (ENTER_ESCAPE_STRING), gHiiHandle);
  gEscapeString         = GetToken (STRING_TOKEN (ESCAPE_STRING), gHiiHandle);
  gSaveFailed           = GetToken (STRING_TOKEN (SAVE_FAILED), gHiiHandle);
  gMoveHighlight        = GetToken (STRING_TOKEN (MOVE_HIGHLIGHT), gHiiHandle);
  gMakeSelection        = GetToken (STRING_TOKEN (MAKE_SELECTION), gHiiHandle);
  gDecNumericInput      = GetToken (STRING_TOKEN (DEC_NUMERIC_INPUT), gHiiHandle);
  gHexNumericInput      = GetToken (STRING_TOKEN (HEX_NUMERIC_INPUT), gHiiHandle);
  gToggleCheckBox       = GetToken (STRING_TOKEN (TOGGLE_CHECK_BOX), gHiiHandle);
  gPromptForData        = GetToken (STRING_TOKEN (PROMPT_FOR_DATA), gHiiHandle);
  gPromptForPassword    = GetToken (STRING_TOKEN (PROMPT_FOR_PASSWORD), gHiiHandle);
  gPromptForNewPassword = GetToken (STRING_TOKEN (PROMPT_FOR_NEW_PASSWORD), gHiiHandle);
  gConfirmPassword      = GetToken (STRING_TOKEN (CONFIRM_PASSWORD), gHiiHandle);
  gConfirmError         = GetToken (STRING_TOKEN (CONFIRM_ERROR), gHiiHandle);
  gPassowordInvalid     = GetToken (STRING_TOKEN (PASSWORD_INVALID), gHiiHandle);
  gPressEnter           = GetToken (STRING_TOKEN (PRESS_ENTER), gHiiHandle);
  gEmptyString          = GetToken (STRING_TOKEN (EMPTY_STRING), gHiiHandle);
  gAreYouSure           = GetToken (STRING_TOKEN (ARE_YOU_SURE), gHiiHandle);
  gYesResponse          = GetToken (STRING_TOKEN (ARE_YOU_SURE_YES), gHiiHandle);
  gNoResponse           = GetToken (STRING_TOKEN (ARE_YOU_SURE_NO), gHiiHandle);
  gMiniString           = GetToken (STRING_TOKEN (MINI_STRING), gHiiHandle);
  gPlusString           = GetToken (STRING_TOKEN (PLUS_STRING), gHiiHandle);
  gMinusString          = GetToken (STRING_TOKEN (MINUS_STRING), gHiiHandle);
  gAdjustNumber         = GetToken (STRING_TOKEN (ADJUST_NUMBER), gHiiHandle);
  gSaveChanges          = GetToken (STRING_TOKEN (SAVE_CHANGES), gHiiHandle);
  gOptionMismatch       = GetToken (STRING_TOKEN (OPTION_MISMATCH), gHiiHandle);
  return ;
}

VOID
FreeBrowserStrings (
  VOID
  )
{
  EfiLibSafeFreePool (gFunctionNineString);
  EfiLibSafeFreePool (gFunctionTenString);
  EfiLibSafeFreePool (gEnterString);
  EfiLibSafeFreePool (gEnterCommitString);
  EfiLibSafeFreePool (gEnterEscapeString);
  EfiLibSafeFreePool (gEscapeString);
  EfiLibSafeFreePool (gMoveHighlight);
  EfiLibSafeFreePool (gMakeSelection);
  EfiLibSafeFreePool (gDecNumericInput);
  EfiLibSafeFreePool (gHexNumericInput);
  EfiLibSafeFreePool (gToggleCheckBox);
  EfiLibSafeFreePool (gPromptForData);
  EfiLibSafeFreePool (gPromptForPassword);
  EfiLibSafeFreePool (gPromptForNewPassword);
  EfiLibSafeFreePool (gConfirmPassword);
  EfiLibSafeFreePool (gPassowordInvalid);
  EfiLibSafeFreePool (gConfirmError);
  EfiLibSafeFreePool (gPressEnter);
  EfiLibSafeFreePool (gEmptyString);
  EfiLibSafeFreePool (gAreYouSure);
  EfiLibSafeFreePool (gYesResponse);
  EfiLibSafeFreePool (gNoResponse);
  EfiLibSafeFreePool (gMiniString);
  EfiLibSafeFreePool (gPlusString);
  EfiLibSafeFreePool (gMinusString);
  EfiLibSafeFreePool (gAdjustNumber);
  EfiLibSafeFreePool (gSaveChanges);
  EfiLibSafeFreePool (gOptionMismatch);
  return ;
}

VOID
UpdateKeyHelp (
  IN  UI_MENU_SELECTION           *Selection,
  IN  UI_MENU_OPTION              *MenuOption,
  IN  BOOLEAN                     Selected
  )
/*++

Routine Description:
  Update key's help imformation

Arguments:
  MenuOption - The Menu option
  Selected   - Whether or not a tag be selected

Returns:
  None

--*/
{
  UINTN                  SecCol;
  UINTN                  ThdCol;
  UINTN                  LeftColumnOfHelp;
  UINTN                  RightColumnOfHelp;
  UINTN                  TopRowOfHelp;
  UINTN                  BottomRowOfHelp;
  UINTN                  StartColumnOfHelp;
  EFI_SCREEN_DESCRIPTOR  LocalScreen;
  FORM_BROWSER_STATEMENT *Statement;

  EfiCopyMem (&LocalScreen, &gScreenDimensions, sizeof (EFI_SCREEN_DESCRIPTOR));

  SecCol            = LocalScreen.LeftColumn + (LocalScreen.RightColumn - LocalScreen.LeftColumn) / 3;
  ThdCol            = LocalScreen.LeftColumn + (LocalScreen.RightColumn - LocalScreen.LeftColumn) * 2 / 3;

  StartColumnOfHelp = LocalScreen.LeftColumn + 2;
  LeftColumnOfHelp  = LocalScreen.LeftColumn + 1;
  RightColumnOfHelp = LocalScreen.RightColumn - 2;
  TopRowOfHelp      = LocalScreen.BottomRow - 4;
  BottomRowOfHelp   = LocalScreen.BottomRow - 3;

  gST->ConOut->SetAttribute (gST->ConOut, KEYHELP_TEXT | KEYHELP_BACKGROUND);

  Statement = MenuOption->ThisTag;
  switch (Statement->Operand) {
  case EFI_IFR_ORDERED_LIST_OP:
  case EFI_IFR_ONE_OF_OP:
  case EFI_IFR_NUMERIC_OP:
  case EFI_IFR_TIME_OP:
  case EFI_IFR_DATE_OP:
    ClearLines (LeftColumnOfHelp, RightColumnOfHelp, TopRowOfHelp, BottomRowOfHelp, KEYHELP_TEXT | KEYHELP_BACKGROUND);

    if (!Selected) {
      if ((gClassOfVfr & FORMSET_CLASS_PLATFORM_SETUP) != 0) {
        if (Selection->FormEditable) {
          PrintStringAt (SecCol, TopRowOfHelp, gFunctionNineString);
          PrintStringAt (ThdCol, TopRowOfHelp, gFunctionTenString);
        }
        PrintStringAt (ThdCol, BottomRowOfHelp, gEscapeString);
      }

      if ((Statement->Operand == EFI_IFR_DATE_OP) ||
          (Statement->Operand == EFI_IFR_TIME_OP)) {
        PrintAt (
          StartColumnOfHelp,
          BottomRowOfHelp,
          L"%c%c%c%c%s",
          ARROW_UP,
          ARROW_DOWN,
          ARROW_RIGHT,
          ARROW_LEFT,
          gMoveHighlight
          );
        PrintStringAt (SecCol, BottomRowOfHelp, gAdjustNumber);
      } else {
        PrintAt (StartColumnOfHelp, BottomRowOfHelp, L"%c%c%s", ARROW_UP, ARROW_DOWN, gMoveHighlight);
        if (Statement->Operand == EFI_IFR_NUMERIC_OP && Statement->Step != 0) {
          PrintStringAt (SecCol, BottomRowOfHelp, gAdjustNumber);
        } else if (!MenuOption->ReadOnly) {
          PrintStringAt (SecCol, BottomRowOfHelp, gEnterString);
        }
      }
    } else {
      PrintStringAt (SecCol, BottomRowOfHelp, gEnterCommitString);

      //
      // If it is a selected numeric with manual input, display different message
      //
      if ((Statement->Operand == EFI_IFR_NUMERIC_OP) && (Statement->Step == 0)) {
        PrintStringAt (
          SecCol,
          TopRowOfHelp,
          (Statement->Flags & EFI_IFR_DISPLAY_UINT_HEX) ? gHexNumericInput : gDecNumericInput
          );
      } else if (Statement->Operand != EFI_IFR_ORDERED_LIST_OP) {
        PrintAt (StartColumnOfHelp, BottomRowOfHelp, L"%c%c%s", ARROW_UP, ARROW_DOWN, gMoveHighlight);
      }

      if (Statement->Operand == EFI_IFR_ORDERED_LIST_OP) {
        PrintStringAt (StartColumnOfHelp, TopRowOfHelp, gPlusString);
        PrintStringAt (ThdCol, TopRowOfHelp, gMinusString);
      }

      PrintStringAt (ThdCol, BottomRowOfHelp, gEnterEscapeString);
    }
    break;

  case EFI_IFR_CHECKBOX_OP:
    ClearLines (LeftColumnOfHelp, RightColumnOfHelp, TopRowOfHelp, BottomRowOfHelp, KEYHELP_TEXT | KEYHELP_BACKGROUND);

    if ((gClassOfVfr & FORMSET_CLASS_PLATFORM_SETUP) != 0) {
      if (Selection->FormEditable) {
        PrintStringAt (SecCol, TopRowOfHelp, gFunctionNineString);
        PrintStringAt (ThdCol, TopRowOfHelp, gFunctionTenString);
      }
      PrintStringAt (ThdCol, BottomRowOfHelp, gEscapeString);
    }

    PrintAt (StartColumnOfHelp, BottomRowOfHelp, L"%c%c%s", ARROW_UP, ARROW_DOWN, gMoveHighlight);
    PrintStringAt (SecCol, BottomRowOfHelp, gToggleCheckBox);
    break;

  case EFI_IFR_REF_OP:
  case EFI_IFR_PASSWORD_OP:
  case EFI_IFR_STRING_OP:
  case EFI_IFR_TEXT_OP:
  case EFI_IFR_ACTION_OP:
  case EFI_IFR_RESET_BUTTON_OP:
    ClearLines (LeftColumnOfHelp, RightColumnOfHelp, TopRowOfHelp, BottomRowOfHelp, KEYHELP_TEXT | KEYHELP_BACKGROUND);

    if (!Selected) {
      if ((gClassOfVfr & FORMSET_CLASS_PLATFORM_SETUP) != 0) {
        if (Selection->FormEditable) {
          PrintStringAt (SecCol, TopRowOfHelp, gFunctionNineString);
          PrintStringAt (ThdCol, TopRowOfHelp, gFunctionTenString);
        }
        PrintStringAt (ThdCol, BottomRowOfHelp, gEscapeString);
      }

      PrintAt (StartColumnOfHelp, BottomRowOfHelp, L"%c%c%s", ARROW_UP, ARROW_DOWN, gMoveHighlight);
      if (Statement->Operand != EFI_IFR_TEXT_OP) {
        PrintStringAt (SecCol, BottomRowOfHelp, gEnterString);
      }
    } else {
      if (Statement->Operand != EFI_IFR_REF_OP) {
        PrintStringAt (
          (LocalScreen.RightColumn - GetStringWidth (gEnterCommitString) / 2) / 2,
          BottomRowOfHelp,
          gEnterCommitString
          );
        PrintStringAt (ThdCol, BottomRowOfHelp, gEnterEscapeString);
      }
    }
    break;

  default:
    break;
  }
}

EFI_STATUS
FormUpdateNotify (
  IN UINT8                              PackageType,
  IN CONST EFI_GUID                     *PackageGuid,
  IN CONST EFI_HII_PACKAGE_HEADER       *Package,
  IN EFI_HII_HANDLE                     Handle,
  IN EFI_HII_DATABASE_NOTIFY_TYPE       NotifyType
  )
{
  mHiiPackageListUpdated = TRUE;

  return EFI_SUCCESS;
}

EFI_STATUS
SetupBrowser (
  IN OUT UI_MENU_SELECTION    *Selection
  )
{
  EFI_STATUS                      Status;
  EFI_LIST_ENTRY                  *Link;
  EFI_BROWSER_ACTION_REQUEST      ActionRequest;
  EFI_HANDLE                      NotifyHandle;
  EFI_HII_VALUE                   *HiiValue;
  FORM_BROWSER_STATEMENT          *Statement;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;

  gMenuRefreshHead = NULL;
  gResetRequired = FALSE;

  //
  // Register notify for Form package update
  //
  Status = mHiiDatabase->RegisterPackageNotify (
                           mHiiDatabase,
                           EFI_HII_PACKAGE_FORMS,
                           NULL,
                           FormUpdateNotify,
                           EFI_HII_DATABASE_NOTIFY_REMOVE_PACK,
                           &NotifyHandle
                           );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  do {
    //
    // Initialize Selection->Form
    //
    if (Selection->FormId == 0) {
      //
      // Zero FormId indicates display the first Form in a FormSet
      //
      Link = GetFirstNode (&Selection->FormSet->FormListHead);

      Selection->Form = FORM_BROWSER_FORM_FROM_LINK (Link);
      Selection->FormId = Selection->Form->FormId;
    } else {
      Selection->Form = IdToForm (Selection->FormSet, Selection->FormId);
    }

    if (Selection->Form == NULL) {
      //
      // No Form to display
      //
      return EFI_NOT_FOUND;
    }

    //
    // Load Questions' Value for display
    //
    Status = LoadFormSetConfig (Selection->FormSet);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Displays the Header and Footer borders
    //
    DisplayPageFrame ();

    //
    // Display form
    //
    Status = DisplayForm (Selection);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Check Selected Statement (if press ESC, Selection->Statement will be NULL)
    //
    Statement = Selection->Statement;
    if (Statement != NULL) {
      if (Statement->QuestionFlags & EFI_IFR_FLAG_RESET_REQUIRED) {
        gResetRequired = TRUE;
      }

      //
      // Reset FormPackage update flag
      //
      mHiiPackageListUpdated = FALSE;

      if (Statement->QuestionFlags & EFI_IFR_FLAG_CALLBACK && Statement->Operand != EFI_IFR_PASSWORD_OP) {
        ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;

        HiiValue = &Statement->HiiValue;
        if (HiiValue->Type == EFI_IFR_TYPE_STRING) {
          //
          // Create String in HII database for Configuration Driver to retrieve
          //
          HiiValue->Value.string = NewString ((CHAR16 *) Statement->BufferValue, Selection->FormSet->HiiHandle);
        }

        ConfigAccess = Selection->FormSet->ConfigAccess;
        if (ConfigAccess == NULL) {
          return EFI_UNSUPPORTED;
        }
        Status = ConfigAccess->Callback (
                                 ConfigAccess,
                                 EFI_BROWSER_ACTION_CHANGING,
                                 Statement->QuestionId,
                                 HiiValue->Type,
                                 &HiiValue->Value,
                                 &ActionRequest
                                 );

        if (HiiValue->Type == EFI_IFR_TYPE_STRING) {
          //
          // Clean the String in HII Database
          //
          DeleteString (HiiValue->Value.string, Selection->FormSet->HiiHandle);
        }

        if (!EFI_ERROR (Status)) {
          switch (ActionRequest) {
          case EFI_BROWSER_ACTION_REQUEST_RESET:
            gResetRequired = TRUE;
            break;

          case EFI_BROWSER_ACTION_REQUEST_SUBMIT:
            SubmitForm (Selection->FormSet, Selection->Form);
            break;

          case EFI_BROWSER_ACTION_REQUEST_EXIT:
            Selection->Action = UI_ACTION_EXIT;
            gNvUpdateRequired = FALSE;
            break;

          default:
            break;
          }
        } else if (Status != EFI_UNSUPPORTED) {
          //
          // Callback return error status other than EFI_UNSUPPORTED
          //
          if (Statement->Operand == EFI_IFR_REF_OP) {
            //
            // Cross reference will not be taken
            //
            Selection->FormId = Selection->Form->FormId;
            Selection->QuestionId = 0;
          }
        }
      }

      //
      // Check whether Form Package has been updated during Callback
      //
      if (mHiiPackageListUpdated && (Selection->Action == UI_ACTION_REFRESH_FORM)) {
        //
        // Force to reparse IFR binary of target Formset
        //
        Selection->Action = UI_ACTION_REFRESH_FORMSET;
      }
    }
  } while (Selection->Action == UI_ACTION_REFRESH_FORM);

  //
  // Unregister notify for Form package update
  //
  Status = mHiiDatabase->UnregisterPackageNotify (
                           mHiiDatabase,
                           NotifyHandle
                           );
  return Status;
}
