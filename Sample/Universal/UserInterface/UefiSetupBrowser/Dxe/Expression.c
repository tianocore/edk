/*++

Copyright (c) 2007 - 2008, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Expression.c

Abstract:

  Expression evaluation.

--*/

#include "Ui.h"
#include "Setup.h"

//
// Global stack used to evaluate boolean expresions
//
EFI_HII_VALUE *mOpCodeScopeStack = NULL;
EFI_HII_VALUE *mOpCodeScopeStackEnd = NULL;
EFI_HII_VALUE *mOpCodeScopeStackPointer = NULL;

EFI_HII_VALUE *mExpressionEvaluationStack = NULL;
EFI_HII_VALUE *mExpressionEvaluationStackEnd = NULL;
EFI_HII_VALUE *mExpressionEvaluationStackPointer = NULL;

//
// Unicode collation protocol interface
//
EFI_UNICODE_COLLATION2_PROTOCOL *mUnicodeCollation = NULL;

STATIC
EFI_STATUS
GrowStack (
  IN OUT EFI_HII_VALUE  **Stack,
  IN OUT EFI_HII_VALUE  **StackPtr,
  IN OUT EFI_HII_VALUE  **StackEnd
  )
/*++

Routine Description:
  Grow size of the stack

Arguments:
  Stack     - On input: old stack; On output: new stack
  StackPtr  - On input: old stack pointer; On output: new stack pointer
  StackEnd  - On input: old stack end; On output: new stack end

Returns:
  EFI_SUCCESS          - Grow stack success.
  EFI_OUT_OF_RESOURCES - No enough memory for stack space.

--*/
{
  UINTN           Size;
  EFI_HII_VALUE  *NewStack;

  Size = EXPRESSION_STACK_SIZE_INCREMENT;
  if (*StackPtr != NULL) {
    Size = Size + (*StackEnd - *Stack);
  }

  NewStack = EfiLibAllocatePool (Size * sizeof (EFI_HII_VALUE));
  if (NewStack == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (*StackPtr != NULL) {
    //
    // Copy from Old Stack to the New Stack
    //
    EfiCopyMem (
      NewStack,
      *Stack,
      (*StackEnd - *Stack) * sizeof (EFI_HII_VALUE)
      );

    //
    // Free The Old Stack
    //
    gBS->FreePool (*Stack);
  }

  //
  // Make the Stack pointer point to the old data in the new stack
  //
  *StackPtr = NewStack + (*StackPtr - *Stack);
  *Stack    = NewStack;
  *StackEnd = NewStack + Size;

  return EFI_SUCCESS;
}

EFI_STATUS
PushStack (
  IN OUT EFI_HII_VALUE       **Stack,
  IN OUT EFI_HII_VALUE       **StackPtr,
  IN OUT EFI_HII_VALUE       **StackEnd,
  IN EFI_HII_VALUE           *Data
  )
/*++

Routine Description:
  Push an element onto the Boolean Stack

Arguments:
  Stack     - On input: old stack; On output: new stack
  StackPtr  - On input: old stack pointer; On output: new stack pointer
  StackEnd  - On input: old stack end; On output: new stack end
  Data      - Data to push.

Returns:
  EFI_SUCCESS - Push stack success.

--*/
{
  EFI_STATUS  Status;

  //
  // Check for a stack overflow condition
  //
  if (*StackPtr >= *StackEnd) {
    //
    // Grow the stack
    //
    Status = GrowStack (Stack, StackPtr, StackEnd);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Push the item onto the stack
  //
  EfiCopyMem (*StackPtr, Data, sizeof (EFI_HII_VALUE));
  *StackPtr = *StackPtr + 1;

  return EFI_SUCCESS;
}

EFI_STATUS
PopStack (
  IN OUT EFI_HII_VALUE       **Stack,
  IN OUT EFI_HII_VALUE       **StackPtr,
  IN OUT EFI_HII_VALUE       **StackEnd,
  OUT EFI_HII_VALUE          *Data
  )
/*++

Routine Description:
  Pop an element from the stack.

Arguments:
  Stack     - On input: old stack; On output: new stack
  StackPtr  - On input: old stack pointer; On output: new stack pointer
  StackEnd  - On input: old stack end; On output: new stack end
  Data      - Data to pop.

Returns:
  EFI_SUCCESS       - The value was popped onto the stack.
  EFI_ACCESS_DENIED - The pop operation underflowed the stack

--*/
{
  //
  // Check for a stack underflow condition
  //
  if (*StackPtr == *Stack) {
    return EFI_ACCESS_DENIED;
  }

  //
  // Pop the item off the stack
  //
  *StackPtr = *StackPtr - 1;
  EfiCopyMem (Data, *StackPtr, sizeof (EFI_HII_VALUE));
  return EFI_SUCCESS;
}

VOID
ResetScopeStack (
  VOID
  )
/*++

Routine Description:
  Reset stack pointer to begin of the stack.

Arguments:
  None.

Returns:
  None.

--*/
{
  mOpCodeScopeStackPointer = mOpCodeScopeStack;
}

EFI_STATUS
PushScope (
  IN UINT8   Operand
  )
/*++

Routine Description:
  Push an Operand onto the Stack

Arguments:
  Operand - Operand to push.

Returns:
  EFI_SUCCESS          - The value was pushed onto the stack.
  EFI_OUT_OF_RESOURCES - There is not enough system memory to grow the stack.

--*/
{
  EFI_HII_VALUE  Data;

  Data.Type = EFI_IFR_TYPE_NUM_SIZE_8;
  Data.Value.u8 = Operand;

  return PushStack (
           &mOpCodeScopeStack,
           &mOpCodeScopeStackPointer,
           &mOpCodeScopeStackEnd,
           &Data
           );
}

EFI_STATUS
PopScope (
  OUT UINT8     *Operand
  )
/*++

Routine Description:
  Pop an Operand from the Stack

Arguments:
  Operand - Operand to pop.

Returns:
  EFI_SUCCESS          - The value was pushed onto the stack.
  EFI_OUT_OF_RESOURCES - There is not enough system memory to grow the stack.

--*/
{
  EFI_STATUS     Status;
  EFI_HII_VALUE  Data;

  Status = PopStack (
             &mOpCodeScopeStack,
             &mOpCodeScopeStackPointer,
             &mOpCodeScopeStackEnd,
             &Data
             );

  *Operand = Data.Value.u8;

  return Status;
}

VOID
ResetExpressionStack (
  VOID
  )
/*++

Routine Description:
  Reset stack pointer to begin of the stack.

Arguments:
  None.

Returns:
  None.

--*/
{
  mExpressionEvaluationStackPointer = mExpressionEvaluationStack;
}

EFI_STATUS
PushExpression (
  IN EFI_HII_VALUE  *Value
  )
/*++

Routine Description:
  Push an Expression value onto the Stack

Arguments:
  Value - Expression value to push.

Returns:
  EFI_SUCCESS          - The value was pushed onto the stack.
  EFI_OUT_OF_RESOURCES - There is not enough system memory to grow the stack.

--*/
{
  return PushStack (
           &mExpressionEvaluationStack,
           &mExpressionEvaluationStackPointer,
           &mExpressionEvaluationStackEnd,
           Value
           );
}

EFI_STATUS
PopExpression (
  OUT EFI_HII_VALUE  *Value
  )
/*++

Routine Description:
  Pop an Expression value from the stack.

Arguments:
  Value - Expression value to pop.

Returns:
  EFI_SUCCESS       - The value was popped onto the stack.
  EFI_ACCESS_DENIED - The pop operation underflowed the stack

--*/
{
  return PopStack (
           &mExpressionEvaluationStack,
           &mExpressionEvaluationStackPointer,
           &mExpressionEvaluationStackEnd,
           Value
           );
}

FORM_BROWSER_FORM *
IdToForm (
  IN FORM_BROWSER_FORMSET  *FormSet,
  IN UINT16                FormId
)
/*++

Routine Description:
  Get Form given its FormId.

Arguments:
  FormSet - The formset which contains this form.
  FormId  - Id of this form.

Returns:
  Pointer - The form.
  NULL    - Specified Form is not found in the formset.

--*/
{
  EFI_LIST_ENTRY     *Link;
  FORM_BROWSER_FORM  *Form;

  Link = GetFirstNode (&FormSet->FormListHead);
  while (!IsNull (&FormSet->FormListHead, Link)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (Link);

    if (Form->FormId == FormId) {
      return Form;
    }

    Link = GetNextNode (&FormSet->FormListHead, Link);
  }

  return NULL;
}

FORM_BROWSER_STATEMENT *
IdToQuestion2 (
  IN FORM_BROWSER_FORM  *Form,
  IN UINT16             QuestionId
  )
/*++

Routine Description:
  Search a Question in Form scope using its QuestionId.

Arguments:
  Form        - The form which contains this Question.
  QuestionId  - Id of this Question.

Returns:
  Pointer - The Question.
  NULL    - Specified Question not found in the form.

--*/
{
  EFI_LIST_ENTRY          *Link;
  FORM_BROWSER_STATEMENT  *Question;

  if (QuestionId == 0) {
    //
    // The value of zero is reserved
    //
    return NULL;
  }

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);

    if (Question->QuestionId == QuestionId) {
      return Question;
    }

    Link = GetNextNode (&Form->StatementListHead, Link);
  }

  return NULL;
}

FORM_BROWSER_STATEMENT *
IdToQuestion (
  IN FORM_BROWSER_FORMSET  *FormSet,
  IN FORM_BROWSER_FORM     *Form,
  IN UINT16                QuestionId
  )
/*++

Routine Description:
  Search a Question in Formset scope using its QuestionId.

Arguments:
  FormSet     - The formset which contains this form.
  Form        - The form which contains this Question.
  QuestionId  - Id of this Question.

Returns:
  Pointer - The Question.
  NULL    - Specified Question not found in the form.

--*/
{
  EFI_LIST_ENTRY          *Link;
  FORM_BROWSER_STATEMENT  *Question;

  //
  // Search in the form scope first
  //
  Question = IdToQuestion2 (Form, QuestionId);
  if (Question != NULL) {
    return Question;
  }

  //
  // Search in the formset scope
  //
  Link = GetFirstNode (&FormSet->FormListHead);
  while (!IsNull (&FormSet->FormListHead, Link)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (Link);

    Question = IdToQuestion2 (Form, QuestionId);
    if (Question != NULL) {
      //
      // EFI variable storage may be updated by Callback() asynchronous,
      // to keep synchronous, always reload the Question Value.
      //
      if (Question->Storage->Type == EFI_HII_VARSTORE_EFI_VARIABLE) {
        GetQuestionValue (FormSet, Form, Question, FALSE);
      }

      return Question;
    }

    Link = GetNextNode (&FormSet->FormListHead, Link);
  }

  return NULL;
}

FORM_EXPRESSION *
RuleIdToExpression (
  IN FORM_BROWSER_FORM  *Form,
  IN UINT8              RuleId
  )
/*++

Routine Description:
  Get Expression given its RuleId.

Arguments:
  Form    - The form which contains this Expression.
  RuleId  - Id of this Expression.

Returns:
  Pointer - The Expression.
  NULL    - Specified Expression not found in the form.

--*/
{
  EFI_LIST_ENTRY   *Link;
  FORM_EXPRESSION  *Expression;

  Link = GetFirstNode (&Form->ExpressionListHead);
  while (!IsNull (&Form->ExpressionListHead, Link)) {
    Expression = FORM_EXPRESSION_FROM_LINK (Link);

    if (Expression->Type == EFI_HII_EXPRESSION_RULE && Expression->RuleId == RuleId) {
      return Expression;
    }

    Link = GetNextNode (&Form->ExpressionListHead, Link);
  }

  return NULL;
}

EFI_STATUS
InitializeUnicodeCollationProtocol (
  VOID
  )
/*++

Routine Description:
  Locate the Unicode Collation Protocol interface for later use.

Arguments:
  None.

Returns:
  EFI_SUCCESS - Protocol interface initialize success.
  Other       - Protocol interface initialize failed.

--*/
{
  EFI_STATUS  Status;

  if (mUnicodeCollation != NULL) {
    return EFI_SUCCESS;
  }

  //
  // BUGBUG: Proper impelmentation is to locate all Unicode Collation Protocol
  // instances first and then select one which support English language.
  // Current implementation just pick the first instance.
  //
  Status = gBS->LocateProtocol (
                  &gEfiUnicodeCollation2ProtocolGuid,
                  NULL,
                  &mUnicodeCollation
                  );
  return Status;
}

VOID
IfrStrToUpper (
  CHAR16                   *String
  )
{
  while (*String != 0) {
    if ((*String >= 'a') && (*String <= 'z')) {
      *String = (*String) & ((UINT16) ~0x20);
    }
    String++;
  }
}

EFI_STATUS
IfrToString (
  IN FORM_BROWSER_FORMSET  *FormSet,
  IN UINT8                 Format,
  OUT  EFI_HII_VALUE       *Result
  )
/*++

Routine Description:
  Evaluate opcode EFI_IFR_TO_STRING.

Arguments:
  FormSet     - Formset which contains this opcode.
  Format      - String format in EFI_IFR_TO_STRING.
  Result      - Evaluation result for this opcode.

Returns:
  EFI_SUCCESS - Opcode evaluation success.
  Other       - Opcode evaluation failed.

--*/
{
  EFI_STATUS     Status;
  EFI_HII_VALUE  Value;
  CHAR16         *String;
  CHAR16         *PrintFormat;
  CHAR16         Buffer[CHARACTER_NUMBER_FOR_VALUE];
  UINTN          BufferSize;

  Status = PopExpression (&Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  switch (Value.Type) {
  case EFI_IFR_TYPE_NUM_SIZE_8:
  case EFI_IFR_TYPE_NUM_SIZE_16:
  case EFI_IFR_TYPE_NUM_SIZE_32:
  case EFI_IFR_TYPE_NUM_SIZE_64:
    BufferSize = CHARACTER_NUMBER_FOR_VALUE * sizeof (CHAR16);
    switch (Format) {
    case EFI_IFR_STRING_UNSIGNED_DEC:
    case EFI_IFR_STRING_SIGNED_DEC:
      PrintFormat = L"%ld";
      break;

    case EFI_IFR_STRING_LOWERCASE_HEX:
      PrintFormat = L"%lx";
      break;

    case EFI_IFR_STRING_UPPERCASE_HEX:
      PrintFormat = L"%lX";
      break;

    default:
      return EFI_UNSUPPORTED;
    }
    SPrint (Buffer, BufferSize, PrintFormat, Value.Value.u64);
    String = Buffer;
    break;

  case EFI_IFR_TYPE_STRING:
    EfiCopyMem (Result, &Value, sizeof (EFI_HII_VALUE));
    return EFI_SUCCESS;

  case EFI_IFR_TYPE_BOOLEAN:
    String = (Value.Value.b) ? L"True" : L"False";
    break;

  default:
    return EFI_UNSUPPORTED;
  }

  Result->Type = EFI_IFR_TYPE_STRING;
  Result->Value.string = NewString (String, FormSet->HiiHandle);
  return EFI_SUCCESS;
}

EFI_STATUS
IfrToUint (
  IN FORM_BROWSER_FORMSET  *FormSet,
  OUT  EFI_HII_VALUE       *Result
  )
/*++

Routine Description:
  Evaluate opcode EFI_IFR_TO_UINT.

Arguments:
  FormSet     - Formset which contains this opcode.
  Result      - Evaluation result for this opcode.

Returns:
  EFI_SUCCESS - Opcode evaluation success.
  Other       - Opcode evaluation failed.

--*/
{
  EFI_STATUS     Status;
  EFI_HII_VALUE  Value;
  CHAR16         *String;
  CHAR16         *StringPtr;
  UINTN          BufferSize;

  Status = PopExpression (&Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Value.Type >= EFI_IFR_TYPE_OTHER) {
    return EFI_UNSUPPORTED;
  }

  Status = EFI_SUCCESS;
  if (Value.Type == EFI_IFR_TYPE_STRING) {
    String = GetToken (Value.Value.string, FormSet->HiiHandle);
    if (String == NULL) {
      return EFI_NOT_FOUND;
    }

    IfrStrToUpper (String);
    StringPtr = EfiStrStr (String, L"0X");
    if (StringPtr != NULL) {
      //
      // Hex string
      //
      BufferSize = sizeof (UINT64);
      Status = HexStringToBuf ((UINT8 *) &Result->Value.u64, &BufferSize, StringPtr + 2, NULL);
    } else {
      //
      // BUGBUG: Need handle decimal string
      //
    }
    gBS->FreePool (String);
  } else {
    EfiCopyMem (Result, &Value, sizeof (EFI_HII_VALUE));
  }

  Result->Type = EFI_IFR_TYPE_NUM_SIZE_64;
  return Status;
}

EFI_STATUS
IfrCatenate (
  IN FORM_BROWSER_FORMSET  *FormSet,
  OUT  EFI_HII_VALUE       *Result
  )
/*++

Routine Description:
  Evaluate opcode EFI_IFR_CATENATE.

Arguments:
  FormSet     - Formset which contains this opcode.
  Result      - Evaluation result for this opcode.

Returns:
  EFI_SUCCESS - Opcode evaluation success.
  Other       - Opcode evaluation failed.

--*/
{
  EFI_STATUS     Status;
  EFI_HII_VALUE  Value;
  CHAR16         *String[2];
  UINTN          Index;
  CHAR16         *StringPtr;

  //
  // String[0] - The second string
  // String[1] - The first string
  //
  String[0] = NULL;
  String[1] = NULL;
  StringPtr = NULL;
  Status = EFI_SUCCESS;

  for (Index = 0; Index < 2; Index++) {
    Status = PopExpression (&Value);
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    if (Value.Type != EFI_IFR_TYPE_STRING) {
      Status = EFI_UNSUPPORTED;
      goto Done;
    }

    String[Index] = GetToken (Value.Value.string, FormSet->HiiHandle);
    if (String== NULL) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
  }

  StringPtr= EfiLibAllocatePool (EfiStrSize (String[1]) + EfiStrSize (String[0]));
  ASSERT (StringPtr != NULL);
  EfiStrCpy (StringPtr, String[1]);
  EfiStrCat (StringPtr, String[0]);

  Result->Type = EFI_IFR_TYPE_STRING;
  Result->Value.string = NewString (StringPtr, FormSet->HiiHandle);

Done:
  EfiLibSafeFreePool (String[0]);
  EfiLibSafeFreePool (String[1]);
  EfiLibSafeFreePool (StringPtr);

  return Status;
}

EFI_STATUS
IfrMatch (
  IN FORM_BROWSER_FORMSET  *FormSet,
  OUT  EFI_HII_VALUE       *Result
  )
/*++

Routine Description:
  Evaluate opcode EFI_IFR_MATCH.

Arguments:
  FormSet     - Formset which contains this opcode.
  Result      - Evaluation result for this opcode.

Returns:
  EFI_SUCCESS - Opcode evaluation success.
  Other       - Opcode evaluation failed.

--*/
{
  EFI_STATUS     Status;
  EFI_HII_VALUE  Value;
  CHAR16         *String[2];
  UINTN          Index;

  //
  // String[0] - The string to search
  // String[1] - pattern
  //
  String[0] = NULL;
  String[1] = NULL;
  Status = EFI_SUCCESS;
  for (Index = 0; Index < 2; Index++) {
    Status = PopExpression (&Value);
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    if (Value.Type != EFI_IFR_TYPE_STRING) {
      Status = EFI_UNSUPPORTED;
      goto Done;
    }

    String[Index] = GetToken (Value.Value.string, FormSet->HiiHandle);
    if (String== NULL) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
  }

  Result->Type = EFI_IFR_TYPE_BOOLEAN;
  Result->Value.b = mUnicodeCollation->MetaiMatch (mUnicodeCollation, String[0], String[1]);

Done:
  EfiLibSafeFreePool (String[0]);
  EfiLibSafeFreePool (String[1]);

  return Status;
}

EFI_STATUS
IfrFind (
  IN FORM_BROWSER_FORMSET  *FormSet,
  IN UINT8                 Format,
  OUT  EFI_HII_VALUE       *Result
  )
/*++

Routine Description:
  Evaluate opcode EFI_IFR_FIND.

Arguments:
  FormSet     - Formset which contains this opcode.
  Format      - Case sensitive or insensitive.
  Result      - Evaluation result for this opcode.

Returns:
  EFI_SUCCESS - Opcode evaluation success.
  Other       - Opcode evaluation failed.

--*/
{
  EFI_STATUS     Status;
  EFI_HII_VALUE  Value;
  CHAR16         *String[2];
  UINTN          Base;
  CHAR16         *StringPtr;
  UINTN          Index;

  if (Format > EFI_IFR_FF_CASE_INSENSITIVE) {
    return EFI_UNSUPPORTED;
  }

  Status = PopExpression (&Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (Value.Type > EFI_IFR_TYPE_NUM_SIZE_64) {
    return EFI_UNSUPPORTED;
  }
  Base = (UINTN) Value.Value.u64;

  //
  // String[0] - sub-string
  // String[1] - The string to search
  //
  String[0] = NULL;
  String[1] = NULL;
  for (Index = 0; Index < 2; Index++) {
    Status = PopExpression (&Value);
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    if (Value.Type != EFI_IFR_TYPE_STRING) {
      Status = EFI_UNSUPPORTED;
      goto Done;
    }

    String[Index] = GetToken (Value.Value.string, FormSet->HiiHandle);
    if (String== NULL) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }

    if (Format == EFI_IFR_FF_CASE_INSENSITIVE) {
      //
      // Case insensitive, convert both string to upper case
      //
      IfrStrToUpper (String[Index]);
    }
  }

  Result->Type = EFI_IFR_TYPE_NUM_SIZE_64;
  if (Base >= EfiStrLen (String[1])) {
    Result->Value.u64 = 0xFFFFFFFFFFFFFFFF;
  } else {
    StringPtr = EfiStrStr (String[1] + Base, String[0]);
    Result->Value.u64 = (StringPtr == NULL) ? 0xFFFFFFFFFFFFFFFF : (StringPtr - String[1]);
  }

Done:
  EfiLibSafeFreePool (String[0]);
  EfiLibSafeFreePool (String[1]);

  return Status;
}

EFI_STATUS
IfrMid (
  IN FORM_BROWSER_FORMSET  *FormSet,
  OUT  EFI_HII_VALUE       *Result
  )
/*++

Routine Description:
  Evaluate opcode EFI_IFR_MID.

Arguments:
  FormSet     - Formset which contains this opcode.
  Result      - Evaluation result for this opcode.

Returns:
  EFI_SUCCESS - Opcode evaluation success.
  Other       - Opcode evaluation failed.

--*/
{
  EFI_STATUS     Status;
  EFI_HII_VALUE  Value;
  CHAR16         *String;
  UINTN          Base;
  UINTN          Length;
  CHAR16         *SubString;

  Status = PopExpression (&Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (Value.Type > EFI_IFR_TYPE_NUM_SIZE_64) {
    return EFI_UNSUPPORTED;
  }
  Length = (UINTN) Value.Value.u64;

  Status = PopExpression (&Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (Value.Type > EFI_IFR_TYPE_NUM_SIZE_64) {
    return EFI_UNSUPPORTED;
  }
  Base = (UINTN) Value.Value.u64;

  Status = PopExpression (&Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (Value.Type != EFI_IFR_TYPE_STRING) {
    return EFI_UNSUPPORTED;
  }
  String = GetToken (Value.Value.string, FormSet->HiiHandle);
  if (String == NULL) {
    return EFI_NOT_FOUND;
  }

  if (Length == 0 || Base >= EfiStrLen (String)) {
    SubString = gEmptyString;
  } else {
    SubString = String + Base;
    if ((Base + Length) < EfiStrLen (String)) {
      SubString[Length] = L'\0';
    }
  }

  Result->Type = EFI_IFR_TYPE_STRING;
  Result->Value.string = NewString (SubString, FormSet->HiiHandle);

  gBS->FreePool (String);

  return Status;
}

EFI_STATUS
IfrToken (
  IN FORM_BROWSER_FORMSET  *FormSet,
  OUT  EFI_HII_VALUE       *Result
  )
/*++

Routine Description:
  Evaluate opcode EFI_IFR_TOKEN.

Arguments:
  FormSet     - Formset which contains this opcode.
  Result      - Evaluation result for this opcode.

Returns:
  EFI_SUCCESS - Opcode evaluation success.
  Other       - Opcode evaluation failed.

--*/
{
  EFI_STATUS     Status;
  EFI_HII_VALUE  Value;
  CHAR16         *String[2];
  UINTN          Count;
  CHAR16         *Delimiter;
  CHAR16         *SubString;
  CHAR16         *StringPtr;
  UINTN          Index;

  Status = PopExpression (&Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (Value.Type > EFI_IFR_TYPE_NUM_SIZE_64) {
    return EFI_UNSUPPORTED;
  }
  Count = (UINTN) Value.Value.u64;

  //
  // String[0] - Delimiter
  // String[1] - The string to search
  //
  String[0] = NULL;
  String[1] = NULL;
  for (Index = 0; Index < 2; Index++) {
    Status = PopExpression (&Value);
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    if (Value.Type != EFI_IFR_TYPE_STRING) {
      Status = EFI_UNSUPPORTED;
      goto Done;
    }

    String[Index] = GetToken (Value.Value.string, FormSet->HiiHandle);
    if (String== NULL) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
  }

  Delimiter = String[0];
  SubString = String[1];
  while (Count > 0) {
    SubString = EfiStrStr (SubString, Delimiter);
    if (SubString != NULL) {
      //
      // Skip over the delimiter
      //
      SubString = SubString + EfiStrLen (Delimiter);
    } else {
      break;
    }
    Count--;
  }

  if (SubString == NULL) {
    //
    // nth delimited sub-string not found, push an empty string
    //
    SubString = gEmptyString;
  } else {
    //
    // Put a NULL terminator for nth delimited sub-string
    //
    StringPtr = EfiStrStr (SubString, Delimiter);
    if (StringPtr != NULL) {
      *StringPtr = L'\0';
    }
  }

  Result->Type = EFI_IFR_TYPE_STRING;
  Result->Value.string = NewString (SubString, FormSet->HiiHandle);

Done:
  EfiLibSafeFreePool (String[0]);
  EfiLibSafeFreePool (String[1]);

  return Status;
}

EFI_STATUS
IfrSpan (
  IN FORM_BROWSER_FORMSET  *FormSet,
  IN UINT8                 Flags,
  OUT  EFI_HII_VALUE       *Result
  )
/*++

Routine Description:
  Evaluate opcode EFI_IFR_SPAN.

Arguments:
  FormSet     - Formset which contains this opcode.
  Flags       - FIRST_MATCHING or FIRST_NON_MATCHING.
  Result      - Evaluation result for this opcode.

Returns:
  EFI_SUCCESS - Opcode evaluation success.
  Other       - Opcode evaluation failed.

--*/
{
  EFI_STATUS     Status;
  EFI_HII_VALUE  Value;
  CHAR16         *String[2];
  CHAR16         *Charset;
  UINTN          Base;
  UINTN          Index;
  CHAR16         *StringPtr;
  BOOLEAN        Found;

  Status = PopExpression (&Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (Value.Type > EFI_IFR_TYPE_NUM_SIZE_64) {
    return EFI_UNSUPPORTED;
  }
  Base = (UINTN) Value.Value.u64;

  //
  // String[0] - Charset
  // String[1] - The string to search
  //
  String[0] = NULL;
  String[1] = NULL;
  for (Index = 0; Index < 2; Index++) {
    Status = PopExpression (&Value);
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    if (Value.Type != EFI_IFR_TYPE_STRING) {
      Status = EFI_UNSUPPORTED;
      goto Done;
    }

    String[Index] = GetToken (Value.Value.string, FormSet->HiiHandle);
    if (String== NULL) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
  }

  if (Base >= EfiStrLen (String[1])) {
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  Found = FALSE;
  StringPtr = String[1] + Base;
  Charset = String[0];
  while (*StringPtr != 0 && !Found) {
    Index = 0;
    while (Charset[Index] != 0) {
      if (*StringPtr >= Charset[Index] && *StringPtr <= Charset[Index + 1]) {
        if (Flags == EFI_IFR_FLAGS_FIRST_MATCHING) {
          Found = TRUE;
          break;
        }
      } else {
        if (Flags == EFI_IFR_FLAGS_FIRST_NON_MATCHING) {
          Found = TRUE;
          break;
        }
      }
      //
      // Skip characters pair representing low-end of a range and high-end of a range
      //
      Index += 2;
    }

    if (!Found) {
      StringPtr++;
    }
  }

  Result->Type = EFI_IFR_TYPE_NUM_SIZE_64;
  Result->Value.u64 = StringPtr - String[1];

Done:
  EfiLibSafeFreePool (String[0]);
  EfiLibSafeFreePool (String[1]);

  return Status;
}

VOID
ExtendValueToU64 (
  IN  EFI_HII_VALUE   *Value
  )
/*++

Routine Description:
  Zero extend integer/boolean/date/time to UINT64 for comparing.

Arguments:
  Value    - HII Value to be converted.

Returns:
  None.

--*/
{
  UINT64  Temp;

  Temp = 0;
  switch (Value->Type) {
  case EFI_IFR_TYPE_NUM_SIZE_8:
    Temp = Value->Value.u8;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_16:
    Temp = Value->Value.u16;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_32:
    Temp = Value->Value.u32;
    break;

  case EFI_IFR_TYPE_BOOLEAN:
    Temp = Value->Value.b;
    break;

  case EFI_IFR_TYPE_TIME:
    Temp = Value->Value.u32 & 0xffffff;
    break;

  case EFI_IFR_TYPE_DATE:
    Temp = Value->Value.u32;
    break;

  default:
    return;
  }

  Value->Value.u64 = Temp;
}

INTN
CompareHiiValue (
  IN  EFI_HII_VALUE   *Value1,
  IN  EFI_HII_VALUE   *Value2,
  IN  EFI_HII_HANDLE  HiiHandle OPTIONAL
  )
/*++

Routine Description:
  Compare two Hii value.

Arguments:
  Value1    - Expression value to compare on left-hand
  Value2    - Expression value to compare on right-hand
  HiiHandle - Only required for string compare

Returns:
  EFI_INVALID_PARAMETER  - Could not perform comparation on two values
  0                      - Two operators equeal
  < 0                    - Value1 is greater than Value2
  > 0                    - Value1 is less than Value2

--*/
{
  INTN    Result;
  INT64   Temp64;
  CHAR16  *Str1;
  CHAR16  *Str2;

  if (Value1->Type >= EFI_IFR_TYPE_OTHER || Value2->Type >= EFI_IFR_TYPE_OTHER ) {
    return EFI_INVALID_PARAMETER;
  }

  if (Value1->Type == EFI_IFR_TYPE_STRING || Value2->Type == EFI_IFR_TYPE_STRING ) {
    if (Value1->Type != Value2->Type) {
      //
      // Both Operator should be type of String
      //
      return EFI_INVALID_PARAMETER;
    }

    if (Value1->Value.string == 0 || Value2->Value.string == 0) {
      //
      // StringId 0 is reserved
      //
      return EFI_INVALID_PARAMETER;
    }

    if (Value1->Value.string == Value2->Value.string) {
      return 0;
    }

    Str1 = GetToken (Value1->Value.string, HiiHandle);
    if (Str1 == NULL) {
      //
      // String not found
      //
      return EFI_INVALID_PARAMETER;
    }

    Str2 = GetToken (Value2->Value.string, HiiHandle);
    if (Str2 == NULL) {
      gBS->FreePool (Str1);
      return EFI_INVALID_PARAMETER;
    }

    Result = EfiStrCmp (Str1, Str2);

    gBS->FreePool (Str1);
    gBS->FreePool (Str2);

    return Result;
  }

  //
  // Take remain types(integer, boolean, date/time) as integer
  //
  Temp64 = (INT64) (Value1->Value.u64 - Value2->Value.u64);
  if (Temp64 > 0) {
    Result = 1;
  } else if (Temp64 < 0) {
    Result = -1;
  } else {
    Result = 0;
  }

  return Result;
}

EFI_STATUS
EvaluateExpression (
  IN FORM_BROWSER_FORMSET  *FormSet,
  IN FORM_BROWSER_FORM     *Form,
  IN OUT FORM_EXPRESSION   *Expression
  )
/*++

Routine Description:

  Evaluate the result of a HII expression

Arguments:

  FormSet    - FormSet associated with this expression.
  Form       - Form associated with this expression.
  Expression - Expression to be evaluated.

Returns:

  EFI_SUCCESS           - The expression evaluated successfuly
  EFI_NOT_FOUND         - The Question which referenced by a QuestionId could not be found.
  EFI_OUT_OF_RESOURCES  - There is not enough system memory to grow the stack.
  EFI_ACCESS_DENIED     - The pop operation underflowed the stack
  EFI_INVALID_PARAMETER - Syntax error with the Expression

--*/
{
  EFI_STATUS              Status;
  EFI_LIST_ENTRY          *Link;
  EXPRESSION_OPCODE       *OpCode;
  FORM_BROWSER_STATEMENT  *Question;
  FORM_BROWSER_STATEMENT  *Question2;
  UINT16                  Index;
  EFI_HII_VALUE           Data1;
  EFI_HII_VALUE           Data2;
  EFI_HII_VALUE           Data3;
  FORM_EXPRESSION         *RuleExpression;
  EFI_HII_VALUE           *Value;
  INTN                    Result;
  CHAR16                  *StrPtr;

  //
  // Always reset the stack before evaluating an Expression
  //
  ResetExpressionStack ();

  Expression->Result.Type = EFI_IFR_TYPE_OTHER;

  Link = GetFirstNode (&Expression->OpCodeListHead);
  while (!IsNull (&Expression->OpCodeListHead, Link)) {
    OpCode = EXPRESSION_OPCODE_FROM_LINK (Link);

    Link = GetNextNode (&Expression->OpCodeListHead, Link);

    EfiZeroMem (&Data1, sizeof (EFI_HII_VALUE));
    EfiZeroMem (&Data2, sizeof (EFI_HII_VALUE));
    EfiZeroMem (&Data3, sizeof (EFI_HII_VALUE));

    Value = &Data3;
    Value->Type = EFI_IFR_TYPE_BOOLEAN;
    Status = EFI_SUCCESS;

    switch (OpCode->Operand) {
    //
    // Built-in functions
    //
    case EFI_IFR_EQ_ID_VAL_OP:
      Question = IdToQuestion (FormSet, Form, OpCode->QuestionId);
      if (Question == NULL) {
        return EFI_NOT_FOUND;
      }

      Result = CompareHiiValue (&Question->HiiValue, &OpCode->Value, NULL);
      if (Result == EFI_INVALID_PARAMETER) {
        return EFI_INVALID_PARAMETER;
      }
      Value->Value.b = (Result == 0) ? TRUE : FALSE;
      break;

    case EFI_IFR_EQ_ID_ID_OP:
      Question = IdToQuestion (FormSet, Form, OpCode->QuestionId);
      if (Question == NULL) {
        return EFI_NOT_FOUND;
      }

      Question2 = IdToQuestion (FormSet, Form, OpCode->QuestionId2);
      if (Question2 == NULL) {
        return EFI_NOT_FOUND;
      }

      Result = CompareHiiValue (&Question->HiiValue, &Question2->HiiValue, FormSet->HiiHandle);
      if (Result == EFI_INVALID_PARAMETER) {
        return EFI_INVALID_PARAMETER;
      }
      Value->Value.b = (Result == 0) ? TRUE : FALSE;
      break;

    case EFI_IFR_EQ_ID_LIST_OP:
      Question = IdToQuestion (FormSet, Form, OpCode->QuestionId);
      if (Question == NULL) {
        return EFI_NOT_FOUND;
      }

      Value->Value.b = FALSE;
      for (Index =0; Index < OpCode->ListLength; Index++) {
        if (Question->HiiValue.Value.u16 == OpCode->ValueList[Index]) {
          Value->Value.b = TRUE;
          break;
        }
      }
      break;

    case EFI_IFR_DUP_OP:
      Status = PopExpression (Value);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Status = PushExpression (Value);
      break;

    case EFI_IFR_QUESTION_REF1_OP:
    case EFI_IFR_THIS_OP:
      Question = IdToQuestion (FormSet, Form, OpCode->QuestionId);
      if (Question == NULL) {
        return EFI_NOT_FOUND;
      }

      Value = &Question->HiiValue;
      break;

    case EFI_IFR_QUESTION_REF3_OP:
      if (OpCode->DevicePath == 0) {
        //
        // EFI_IFR_QUESTION_REF3
        // Pop an expression from the expression stack
        //
        Status = PopExpression (Value);
        if (EFI_ERROR (Status)) {
          return Status;
        }

        //
        // Validate the expression value
        //
        if ((Value->Type > EFI_IFR_TYPE_NUM_SIZE_64) || (Value->Value.u64 > 0xffff)) {
          return EFI_NOT_FOUND;
        }

        Question = IdToQuestion (FormSet, Form, Value->Value.u16);
        if (Question == NULL) {
          return EFI_NOT_FOUND;
        }

        //
        // push the questions' value on to the expression stack
        //
        Value = &Question->HiiValue;
      } else {
        //
        // BUGBUG: push 0 for EFI_IFR_QUESTION_REF3_2 and EFI_IFR_QUESTION_REF3_3,
        // since it is impractical to evaluate the value of a Question in another
        // Hii Package list.
        //
        EfiZeroMem (Value, sizeof (EFI_HII_VALUE));
      }
      break;

    case EFI_IFR_RULE_REF_OP:
      //
      // Find expression for this rule
      //
      RuleExpression = RuleIdToExpression (Form, OpCode->RuleId);
      if (RuleExpression == NULL) {
        return EFI_NOT_FOUND;
      }

      //
      // Evaluate this rule expression
      //
      Status = EvaluateExpression (FormSet, Form, RuleExpression);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Value = &RuleExpression->Result;
      break;

    case EFI_IFR_STRING_REF1_OP:
      Value->Type = EFI_IFR_TYPE_STRING;
      Value->Value.string = OpCode->Value.Value.string;
      break;

    //
    // Constant
    //
    case EFI_IFR_TRUE_OP:
    case EFI_IFR_FALSE_OP:
    case EFI_IFR_ONE_OP:
    case EFI_IFR_ONES_OP:
    case EFI_IFR_UINT8_OP:
    case EFI_IFR_UINT16_OP:
    case EFI_IFR_UINT32_OP:
    case EFI_IFR_UINT64_OP:
    case EFI_IFR_UNDEFINED_OP:
    case EFI_IFR_VERSION_OP:
    case EFI_IFR_ZERO_OP:
      Value = &OpCode->Value;
      break;

    //
    // unary-op
    //
    case EFI_IFR_LENGTH_OP:
      Status = PopExpression (Value);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (Value->Type != EFI_IFR_TYPE_STRING) {
        return EFI_INVALID_PARAMETER;
      }

      StrPtr = GetToken (Value->Value.string, FormSet->HiiHandle);
      if (StrPtr == NULL) {
        return EFI_INVALID_PARAMETER;
      }

      Value->Type = EFI_IFR_TYPE_NUM_SIZE_64;
      Value->Value.u64 = EfiStrLen (StrPtr);
      gBS->FreePool (StrPtr);
      break;

    case EFI_IFR_NOT_OP:
      Status = PopExpression (Value);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (Value->Type != EFI_IFR_TYPE_BOOLEAN) {
        return EFI_INVALID_PARAMETER;
      }
      Value->Value.b = !Value->Value.b;
      break;

    case EFI_IFR_QUESTION_REF2_OP:
      //
      // Pop an expression from the expression stack
      //
      Status = PopExpression (Value);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      //
      // Validate the expression value
      //
      if ((Value->Type > EFI_IFR_TYPE_NUM_SIZE_64) || (Value->Value.u64 > 0xffff)) {
        return EFI_NOT_FOUND;
      }

      Question = IdToQuestion (FormSet, Form, Value->Value.u16);
      if (Question == NULL) {
        return EFI_NOT_FOUND;
      }

      Value = &Question->HiiValue;
      break;

    case EFI_IFR_STRING_REF2_OP:
      //
      // Pop an expression from the expression stack
      //
      Status = PopExpression (Value);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      //
      // Validate the expression value
      //
      if ((Value->Type > EFI_IFR_TYPE_NUM_SIZE_64) || (Value->Value.u64 > 0xffff)) {
        return EFI_NOT_FOUND;
      }

      Value->Type = EFI_IFR_TYPE_STRING;
      StrPtr = GetToken (Value->Value.u16, FormSet->HiiHandle);
      if (StrPtr == NULL) {
        //
        // If String not exit, push an empty string
        //
        Value->Value.string = NewString (gEmptyString, FormSet->HiiHandle);
      } else {
        Index = (UINT16) Value->Value.u64;
        Value->Value.string = Index;
        gBS->FreePool (StrPtr);
      }
      break;

    case EFI_IFR_TO_BOOLEAN_OP:
      //
      // Pop an expression from the expression stack
      //
      Status = PopExpression (Value);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      //
      // Convert an expression to a Boolean
      //
      if (Value->Type <= EFI_IFR_TYPE_DATE) {
        //
        // When converting from an unsigned integer, zero will be converted to
        // FALSE and any other value will be converted to TRUE.
        //
        Value->Value.b = (Value->Value.u64) ? TRUE : FALSE;

        Value->Type = EFI_IFR_TYPE_BOOLEAN;
      } else if (Value->Type == EFI_IFR_TYPE_STRING) {
        //
        // When converting from a string, if case-insensitive compare
        // with "true" is True, then push True. If a case-insensitive compare
        // with "false" is True, then push False.
        //
        StrPtr = GetToken (Value->Value.string, FormSet->HiiHandle);
        if (StrPtr == NULL) {
          return EFI_INVALID_PARAMETER;
        }

        if ((EfiStrCmp (StrPtr, L"true") == 0) || (EfiStrCmp (StrPtr, L"false") == 0)){
          Value->Value.b = TRUE;
        } else {
          Value->Value.b = FALSE;
        }
        gBS->FreePool (StrPtr);
        Value->Type = EFI_IFR_TYPE_BOOLEAN;
      }
      break;

    case EFI_IFR_TO_STRING_OP:
      Status = IfrToString (FormSet, OpCode->Format, Value);
      break;

    case EFI_IFR_TO_UINT_OP:
      Status = IfrToUint (FormSet, Value);
      break;

    case EFI_IFR_TO_LOWER_OP:
    case EFI_IFR_TO_UPPER_OP:
      Status = InitializeUnicodeCollationProtocol ();
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Status = PopExpression (Value);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (Value->Type != EFI_IFR_TYPE_STRING) {
        return EFI_UNSUPPORTED;
      }

      StrPtr = GetToken (Value->Value.string, FormSet->HiiHandle);
      if (StrPtr == NULL) {
        return EFI_NOT_FOUND;
      }

      if (OpCode->Operand == EFI_IFR_TO_LOWER_OP) {
        mUnicodeCollation->StrLwr (mUnicodeCollation, StrPtr);
      } else {
        mUnicodeCollation->StrUpr (mUnicodeCollation, StrPtr);
      }
      Value->Value.string = NewString (StrPtr, FormSet->HiiHandle);
      gBS->FreePool (StrPtr);
      break;

    case EFI_IFR_BITWISE_NOT_OP:
      //
      // Pop an expression from the expression stack
      //
      Status = PopExpression (Value);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (Value->Type > EFI_IFR_TYPE_DATE) {
        return EFI_INVALID_PARAMETER;
      }

      Value->Type = EFI_IFR_TYPE_NUM_SIZE_64;
      Value->Value.u64 = ~Value->Value.u64;
      break;

    //
    // binary-op
    //
    case EFI_IFR_ADD_OP:
    case EFI_IFR_SUBTRACT_OP:
    case EFI_IFR_MULTIPLY_OP:
    case EFI_IFR_DIVIDE_OP:
    case EFI_IFR_MODULO_OP:
    case EFI_IFR_BITWISE_AND_OP:
    case EFI_IFR_BITWISE_OR_OP:
    case EFI_IFR_SHIFT_LEFT_OP:
    case EFI_IFR_SHIFT_RIGHT_OP:
      //
      // Pop an expression from the expression stack
      //
      Status = PopExpression (&Data2);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (Data2.Type > EFI_IFR_TYPE_DATE) {
        return EFI_INVALID_PARAMETER;
      }

      //
      // Pop another expression from the expression stack
      //
      Status = PopExpression (&Data1);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (Data1.Type > EFI_IFR_TYPE_DATE) {
        return EFI_INVALID_PARAMETER;
      }

      Value->Type = EFI_IFR_TYPE_NUM_SIZE_64;

      switch (OpCode->Operand) {
        case EFI_IFR_ADD_OP:
          Value->Value.u64 = Data1.Value.u64 + Data2.Value.u64;
          break;

        case EFI_IFR_SUBTRACT_OP:
          Value->Value.u64 = Data1.Value.u64 - Data2.Value.u64;
          break;

        case EFI_IFR_MULTIPLY_OP:
          Value->Value.u64 = MultU64x32 (Data1.Value.u64, (UINTN) Data2.Value.u64);
          break;

        case EFI_IFR_DIVIDE_OP:
          Value->Value.u64 = DivU64x32 (Data1.Value.u64, (UINTN) Data2.Value.u64, NULL);
          break;

        case EFI_IFR_MODULO_OP:
          DivU64x32 (Data1.Value.u64, (UINTN) Data2.Value.u64, (UINTN *) &Value->Value.u64);
          break;

        case EFI_IFR_BITWISE_AND_OP:
          Value->Value.u64 = Data1.Value.u64 & Data2.Value.u64;
          break;

        case EFI_IFR_BITWISE_OR_OP:
          Value->Value.u64 = Data1.Value.u64 | Data2.Value.u64;
          break;

        case EFI_IFR_SHIFT_LEFT_OP:
          Value->Value.u64 = LShiftU64 (Data1.Value.u64, (UINTN) Data2.Value.u64);
          break;

        case EFI_IFR_SHIFT_RIGHT_OP:
          Value->Value.u64 = RShiftU64 (Data1.Value.u64, (UINTN) Data2.Value.u64);
          break;

        default:
          break;
      }
      break;

    case EFI_IFR_AND_OP:
    case EFI_IFR_OR_OP:
      //
      // Two Boolean operator
      //
      Status = PopExpression (&Data2);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (Data2.Type != EFI_IFR_TYPE_BOOLEAN) {
        return EFI_INVALID_PARAMETER;
      }

      //
      // Pop another expression from the expression stack
      //
      Status = PopExpression (&Data1);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (Data1.Type != EFI_IFR_TYPE_BOOLEAN) {
        return EFI_INVALID_PARAMETER;
      }

      if (OpCode->Operand == EFI_IFR_AND_OP) {
        Value->Value.b = Data1.Value.b && Data2.Value.b;
      } else {
        Value->Value.b = Data1.Value.b || Data2.Value.b;
      }
      break;

    case EFI_IFR_EQUAL_OP:
    case EFI_IFR_NOT_EQUAL_OP:
    case EFI_IFR_GREATER_EQUAL_OP:
    case EFI_IFR_GREATER_THAN_OP:
    case EFI_IFR_LESS_EQUAL_OP:
    case EFI_IFR_LESS_THAN_OP:
      //
      // Compare two integer, string, boolean or date/time
      //
      Status = PopExpression (&Data2);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (Data2.Type > EFI_IFR_TYPE_BOOLEAN && Data2.Type != EFI_IFR_TYPE_STRING) {
        return EFI_INVALID_PARAMETER;
      }

      //
      // Pop another expression from the expression stack
      //
      Status = PopExpression (&Data1);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Result = CompareHiiValue (&Data1, &Data2, FormSet->HiiHandle);
      if (Result == EFI_INVALID_PARAMETER) {
        return EFI_INVALID_PARAMETER;
      }

      switch (OpCode->Operand) {
      case EFI_IFR_EQUAL_OP:
        Value->Value.b = (Result == 0) ? TRUE : FALSE;
        break;

      case EFI_IFR_NOT_EQUAL_OP:
        Value->Value.b = (Result != 0) ? TRUE : FALSE;
        break;

      case EFI_IFR_GREATER_EQUAL_OP:
        Value->Value.b = (Result >= 0) ? TRUE : FALSE;
        break;

      case EFI_IFR_GREATER_THAN_OP:
        Value->Value.b = (Result > 0) ? TRUE : FALSE;
        break;

      case EFI_IFR_LESS_EQUAL_OP:
        Value->Value.b = (Result <= 0) ? TRUE : FALSE;
        break;

      case EFI_IFR_LESS_THAN_OP:
        Value->Value.b = (Result < 0) ? TRUE : FALSE;
        break;

      default:
        break;
      }
      break;

    case EFI_IFR_MATCH_OP:
      Status = IfrMatch (FormSet, Value);
      break;

    case EFI_IFR_CATENATE_OP:
      Status = IfrCatenate (FormSet, Value);
      break;

    //
    // ternary-op
    //
    case EFI_IFR_CONDITIONAL_OP:
      //
      // Pop third expression from the expression stack
      //
      Status = PopExpression (&Data3);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      //
      // Pop second expression from the expression stack
      //
      Status = PopExpression (&Data2);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      //
      // Pop first expression from the expression stack
      //
      Status = PopExpression (&Data1);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (Data1.Type != EFI_IFR_TYPE_BOOLEAN) {
        return EFI_INVALID_PARAMETER;
      }

      if (Data1.Value.b) {
        Value = &Data3;
      } else {
        Value = &Data2;
      }
      break;

    case EFI_IFR_FIND_OP:
      Status = IfrFind (FormSet, OpCode->Format, Value);
      break;

    case EFI_IFR_MID_OP:
      Status = IfrMid (FormSet, Value);
      break;

    case EFI_IFR_TOKEN_OP:
      Status = IfrToken (FormSet, Value);
      break;

    case EFI_IFR_SPAN_OP:
      Status = IfrSpan (FormSet, OpCode->Flags, Value);
      break;

    default:
      break;
    }
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = PushExpression (Value);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Pop the final result from expression stack
  //
  Value = &Data1;
  Status = PopExpression (Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // After evaluating an expression, there should be only one value left on the expression stack
  //
  if (PopExpression (Value) != EFI_ACCESS_DENIED) {
    return EFI_INVALID_PARAMETER;
  }

  EfiCopyMem (&Expression->Result, Value, sizeof (EFI_HII_VALUE));

  return EFI_SUCCESS;
}
