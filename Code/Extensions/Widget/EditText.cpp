// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(TextBoxChanged);
DefineEvent(TextChanged);
DefineEvent(TextSubmit);
DefineEvent(TextEnter);
} // namespace Events

RaverieDefineType(EditText, builder, type)
{
}

EditText::EditText(Composite* parent) : Widget(parent)
{
  mFont = FontManager::GetDefault()->GetRenderFont(11);
  mSize = mFont->MeasureText(" ");
  mDragging = false;
  mEditEnabled = false;
  mCaretPos = 0;
  mSelectionStartPos = 0;
  mHasFocus = false;
  mClipText = false;
  mEnterClearFocus = true;
  mPassword = false;
  mAlign = TextAlign::Left;
  mOffset = 0;

  SetTakeFocusMode(FocusMode::Hard);

  SelectNone();

  ConnectThisTo(this, Events::Cut, OnCut);
  ConnectThisTo(this, Events::Copy, OnCopy);
  ConnectThisTo(this, Events::Paste, OnPaste);

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(this, Events::KeyRepeated, OnKeyDown);

  ConnectThisTo(this, Events::TextTyped, OnTextTyped);

  ConnectThisTo(this, Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(this, Events::LeftMouseUp, OnLeftMouseUp);
  ConnectThisTo(this, Events::MouseMove, OnMouseMove);
  ConnectThisTo(this, Events::DoubleClick, OnDoubleClicked);
  ConnectThisTo(this, Events::LeftMouseDrag, OnMouseDrag);

  ConnectThisTo(this, Events::FocusLost, OnFocusLost);
  ConnectThisTo(this, Events::FocusGained, OnFocusGained);
  ConnectThisTo(this, Events::FocusReset, OnFocusReset);
}

EditText::~EditText()
{
}

void EditText::SizeToContents()
{
  mSize = GetMinSize();
}

void EditText::ChangeDefinition(BaseDefinition* def)
{
  // mDef = (TextDefinition*)def;
  // mFont = mDef->mFont;
}

void EditText::RenderUpdate(
    ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, WidgetRect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  String text;
  if (mPassword)
    text = String::Repeat('*', mDisplayText.SizeInBytes());
  else
    text = mDisplayText;

  Vec4 color = mColor * colorTx.ColorMultiply;
  Vec2 textStart = Vec2(mOffset, 0);

  Vec2 textSize = mFont->MeasureText(text, (uint)text.SizeInBytes());
  bool needsClipping = textSize.x + mOffset > mSize.x || mOffset < 0.0f;
  if (needsClipping)
    clipRect = WidgetRect::PointAndSize(Vec2(mWorldTx.m30, mWorldTx.m31), Vec2(mSize.x + 1, mSize.y));

  if (mEditEnabled && mHasFocus)
  {
    Vec2 pos0, pos1;
    Vec4 boxColor = color;
    // If there is a valid selection range draw the selection background
    if (IsValidSelection())
    {
      pos0 = mFont->MeasureText(text, mSelectionLeftPos, 1.0f);
      pos0.y = 0.0f;
      pos1 = mFont->MeasureText(text, mSelectionRightPos, 1.0f);
      pos1.y = mSize.y;
      boxColor = ToFloatColor(Color::DarkOrange);
    }
    // Only draw the caret if there is no selection range
    else
    {
      pos0 = mFont->MeasureText(text, mCaretPos, 1.0f);
      pos0.y = 0.0f;
      pos1 = pos0 + Vec2(Pixels(1), mSize.y);
    }

    pos0 += textStart;
    pos1 += textStart;

    Texture* white = TextureManager::FindOrNull("White");
    ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, white);

    frameBlock.mRenderQueues->AddStreamedQuad(viewNode, Vec3(pos0, 0), Vec3(pos1, 0), Vec2(0, 0), Vec2(1, 1), boxColor);
  }

  if (text.SizeInBytes() == 0)
    return;

  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, mFont->mTexture);
  FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, color);
  AddTextRange(fontProcessor, mFont, text, textStart, mAlign, Vec2(1, 1), mSize, mClipText);
}

void EditText::SetText(StringParam text)
{
  mDisplayText = text;
  SelectNone();
  mCaretPos = 0;
}

void EditText::SetEditable(bool state)
{
  mEditEnabled = state;
  SetTakeFocusMode(FocusMode::Hard);
}

Vec2 EditText::GetMinSize()
{
  if (mDisplayText.Empty())
    return mFont->MeasureText(" ");
  else
    return mFont->MeasureText(mDisplayText.All());
}

void EditText::Clear()
{
  SetText(String());
}

void EditText::SelectAll()
{
  SetEditSelection(0, mDisplayText.ComputeRuneCount());
}

void EditText::SelectNone()
{
  mSelectionStartPos = 0;
  mSelectionLeftPos = 0;
  mSelectionRightPos = 0;
}

void EditText::MakeLetterVisible(int characterIndex)
{
  Vec2 size = mFont->MeasureText(mDisplayText, characterIndex, 1.0f);

  if (size.x > mSize.x - mOffset)
    mOffset = mSize.x - size.x;

  if (size.x + mOffset < 0)
    mOffset = -size.x;
}

int EditText::SetEditCaretPos(int caretPos)
{
  // Selection can be at the End()
  int maxSize = mDisplayText.ComputeRuneCount();
  mCaretPos = Math::Clamp(caretPos, 0, maxSize);

  MakeLetterVisible(mCaretPos);
  return mCaretPos;
}

void EditText::SetEditSelection(int selectionStart, int selectionEnd)
{
  if (selectionEnd < selectionStart)
    Math::Swap(selectionStart, selectionEnd);

  // Selection can include the End()
  int maxSize = mDisplayText.ComputeRuneCount();
  mSelectionLeftPos = Math::Clamp(selectionStart, 0, maxSize);
  mSelectionRightPos = Math::Clamp(selectionEnd, 0, maxSize);
}

bool EditText::IsValidSelection()
{
  return mSelectionRightPos - mSelectionLeftPos > 0;
}

int EditText::CharacterPositionAt(Vec2Param screenPos)
{
  Vec2 textStart = Vec2(mOffset, 0);
  Vec2 localPos = this->ToLocal(screenPos) - textStart;
  return mFont->GetPosition(mDisplayText.All(), localPos.x, 1.0f, TextRounding::Nearest);
}

int EditText::MoveEditCaret(Vec2Param screenPos)
{
  int newCaretPosition = CharacterPositionAt(screenPos);
  return SetEditCaretPos(newCaretPosition);
}

void EditText::MoveCaretNextToken()
{
  StringIterator begin = mDisplayText.Begin();
  StringIterator currentCaretPos = begin + mCaretPos;
  StringRange range = mDisplayText.SubString(currentCaretPos, mDisplayText.End());

  // Already at the end of the text
  if (range.Empty())
    return;

  // If currently at a whitespace rune trim all whitespace and
  // set the caret at the next non-whitespace rune
  if (range.IsCurrentRuneWhitespace())
  {
    range = range.TrimStart();
  }
  // Move to the end of the current token
  else
  {
    // If currently at a symbol move past it and any other consecutive tokens
    if (IsSymbol(range.Front()))
    {
      while (!range.Empty())
      {
        if (IsSymbol(range.Front()))
          range.PopFront();
        else
          break;
      }
    }
    // Move to the end of the current alphanumeric token (i.e word)
    else
    {
      while (!range.Empty())
      {
        if (IsAlphaNumeric(range.Front()))
          range.PopFront();
        else
          break;
      }
    }
  }

  // Compute the position to set the caret at
  int newPosition = range.Begin() - begin;
  // Set the new caret position
  SetEditCaretPos(newPosition);
}

void EditText::MoveCaretPrevToken()
{
  StringIterator begin = mDisplayText.Begin();
  StringIterator currentCaretPos = mDisplayText.Begin() + mCaretPos;
  StringRange range = mDisplayText.SubString(begin, currentCaretPos);

  // Already at the start of the text
  if (range.Empty())
    return;

  // If currently at a whitespace rune trim all whitespace and
  // set the caret at the end of the previous non-whitespace rune
  if (UTF8::IsWhiteSpace(range.Back()))
  {
    range = range.TrimEnd();
  }
  // Move to the start the current token
  else
  {
    // If currently at a symbol move before it and any other consecutive tokens
    if (IsSymbol(range.Back()))
    {
      while (!range.Empty())
      {
        if (IsSymbol(range.Back()))
          range.PopBack();
        else
          break;
      }
    }
    // Move to the start of the current alphanumeric token (i.e word)
    else
    {
      while (!range.Empty())
      {
        if (IsAlphaNumeric(range.Back()))
          range.PopBack();
        else
          break;
      }
    }
  }

  // Compute the position to set the caret at
  int newPosition = range.End() - begin;
  // Set the new caret position
  SetEditCaretPos(newPosition);
}

void EditText::ExtendSelection(SelectMode::Enum direction)
{
  // If no text is currently selected set the caret position as our selections
  // start
  if (mSelectionLeftPos == 0 && mSelectionRightPos == 0)
    mSelectionStartPos = mCaretPos;

  // Move the caret to the next valid token in the direction we are selecting
  switch (direction)
  {
  case SelectMode::Left:
    MoveCaretPrevToken();
    break;
  case SelectMode::Right:
    MoveCaretNextToken();
    break;
  case SelectMode::Start:
    mCaretPos = 0;
    break;
  case SelectMode::End:
    mCaretPos = mDisplayText.ComputeRuneCount();
    break;
  }

  // Select from our current selection start to the new caret position
  // Set the selection range based on whether the next caret position is
  // to the left or right of our selections starting position
  if (mCaretPos > mSelectionStartPos)
    SetEditSelection(mSelectionStartPos, mCaretPos);
  else
    SetEditSelection(mCaretPos, mSelectionStartPos);
}

void EditText::ReplaceSelection(StringRange text)
{
  mTextModified = true;

  // If their is no selection used the caret position
  if (!IsValidSelection())
  {
    mSelectionRightPos = mCaretPos;
    mSelectionLeftPos = mCaretPos;
  }

  // Replace the sub string
  StringIterator displayTextStartIt = mDisplayText.Begin();
  StringIterator selectionStartIt = displayTextStartIt + mSelectionLeftPos;
  StringIterator selectionEndIt = displayTextStartIt + mSelectionRightPos;
  mDisplayText = BuildString(mDisplayText.SubString(displayTextStartIt, selectionStartIt),
                             text,
                             mDisplayText.SubString(selectionEndIt, mDisplayText.End()));
  // Move the caret to the End() of the pasted text

  int newCaretPos = mSelectionLeftPos + text.ComputeRuneCount();
  SetEditCaretPos(newCaretPos);

  // Clear the selection
  SelectNone();
}

StringRange EditText::GetSelectedText()
{
  int selectionSizeInBytes = mSelectionRightPos - mSelectionLeftPos;
  if (mDisplayText.SizeInBytes() && selectionSizeInBytes > 0)
  {
    return StringRange(mDisplayText.Begin() + mSelectionLeftPos, mDisplayText.Begin() + mSelectionRightPos);
  }
  else
  {
    return StringRange();
  }
}

void EditText::SetTextOffset(float offset)
{
  mOffset = offset;
}

String EditText::GetDisplayName()
{
  return BuildString("EditText : '", mDisplayText, "'");
}

void EditText::OnTextTyped(KeyboardTextEvent* keyboardEvent)
{
  if (!mEditEnabled)
    return;

  Rune key = keyboardEvent->mRune;
  bool textSelected = IsValidSelection();

  // key > 255 is temporary fix for unicode and windows
  if (IsGraph(key) || key == ' ' || key > 255)
  {
    mTextModified = true;

    String textToAdd(key);

    if (textSelected)
    {
      // Replace the current selection
      ReplaceSelection(textToAdd);
    }
    else
    {
      // Set selection to where the caret is
      SetEditSelection(mCaretPos, mCaretPos);
      ReplaceSelection(textToAdd);
    }
    ObjectEvent objectEvent(this);
    DispatchBubble(Events::TextChanged, &objectEvent);
  }
}

void EditText::OnCut(ClipboardEvent* event)
{
  StringRange toCut = GetSelectedText();
  event->SetText(toCut);
  ReplaceSelection(String());
  event->mHandled = true;
  ObjectEvent objectEvent(this);
  DispatchBubble(Events::TextChanged, &objectEvent);
}

void EditText::OnCopy(ClipboardEvent* event)
{
  String toCopy = GetSelectedText();
  event->SetText(toCopy);
  event->mHandled = true;
}

void EditText::OnPaste(ClipboardEvent* event)
{
  ReplaceSelection(RangeUntilFirst(event->GetText(), IsControl));
  event->mHandled = true;
  ObjectEvent objectEvent(this);
  DispatchBubble(Events::TextChanged, &objectEvent);
}

void EditText::OnKeyDown(KeyboardEvent* keyboardEvent)
{
  if (!mEditEnabled)
    return;

  uint key = keyboardEvent->Key;
  int size = mDisplayText.ComputeRuneCount();
  bool textSelected = IsValidSelection();
  bool shiftPressed = keyboardEvent->ShiftPressed;
  bool ctrlPressed = keyboardEvent->CtrlPressed;

  // Handle all graphical keys (including space)
  if (IsGraphOrSpace(Rune(key)))
    keyboardEvent->Handled = true;

  ObjectEvent objectEvent(this);
  // Process control keys
  switch (key)
  {
  // Enter key
  case Keys::Enter:
  {
    if (!keyboardEvent->GetModifierPressed())
      DispatchBubble(Events::TextEnter, &objectEvent);

    if (mEnterClearFocus)
      StopEdit();
    keyboardEvent->Handled = true;
    break;
  }

  case Keys::Tab:
  {
    // Tab Jump
    TabJump(this->GetParent(), keyboardEvent);
    keyboardEvent->Handled = true;
    break;
  }

  case Keys::Escape:
  {
    // Cancel the edit
    mTextModified = false;
    StopEdit();
    keyboardEvent->Handled = true;
    break;
  }

  case Keys::Left:
  {
    if (shiftPressed)
    {
      // Highlight everything to the left
      if (ctrlPressed)
      {
        ExtendSelection(SelectMode::Left);
      }
      else if (textSelected)
      {
        if (mCaretPos > 0)
          --mCaretPos;

        SetEditSelection(mSelectionStartPos, mCaretPos);
        MakeLetterVisible(mCaretPos);
      }
      else if (mCaretPos > 0)
      {
        mSelectionStartPos = mCaretPos;
        --mCaretPos;

        SetEditSelection(mSelectionStartPos, mCaretPos);
        MakeLetterVisible(mCaretPos);
      }
    }
    // Move to the start of the previous token
    else if (ctrlPressed)
    {
      MoveCaretPrevToken();
      SelectNone();
    }
    // Move the caret to the start of the selection and de-select
    else if (textSelected)
    {
      SetEditCaretPos(mSelectionLeftPos);
      SelectNone();
    }
    else
    {
      mSelectionStartPos = SetEditCaretPos(mCaretPos - 1);
    }
    keyboardEvent->Handled = true;
    break;
  }

  case Keys::Right:
  {
    if (shiftPressed)
    {
      // Highlight everything to the right
      if (ctrlPressed)
      {
        ExtendSelection(SelectMode::Right);
      }
      else if (textSelected)
      {
        if (mCaretPos < size)
          ++mCaretPos;

        SetEditSelection(mSelectionStartPos, mCaretPos);
        MakeLetterVisible(mCaretPos);
      }
      else if (mCaretPos < size)
      {
        mSelectionStartPos = mCaretPos;
        ++mCaretPos;

        SetEditSelection(mSelectionStartPos, mCaretPos);
        MakeLetterVisible(mCaretPos);
      }
    }
    // Move to the start of the next token
    else if (ctrlPressed)
    {
      MoveCaretNextToken();
      SelectNone();
    }
    // Move the caret to the End() of the selection and de-select
    else if (textSelected)
    {
      SetEditCaretPos(mSelectionRightPos);
      SelectNone();
    }
    else
    {
      mSelectionStartPos = SetEditCaretPos(mCaretPos + 1);
    }

    keyboardEvent->Handled = true;
    break;
  }

  case Keys::Home:
  {
    // Select from the beginning to the caret
    if (shiftPressed)
    {
      ExtendSelection(SelectMode::Start);
      MakeLetterVisible(0);
    }
    else
    {
      // Move to the start and clear the selection
      SetEditCaretPos(0);
      SelectNone();
    }

    keyboardEvent->Handled = true;
    break;
  }

  case Keys::End:
  {
    // Select from the caret to the End()
    if (shiftPressed)
    {
      ExtendSelection(SelectMode::End);
      MakeLetterVisible(size);
    }
    else
    {
      // Move to the End() and clear the selection
      SetEditCaretPos(size);
      SelectNone();
    }

    keyboardEvent->Handled = true;
    break;
  }

  case Keys::Back:
  {
    // If text is selected, remove the selection and move the
    // caret where appropriate
    if (textSelected)
    {
      ReplaceSelection(StringRange());
    }
    else
    {
      if (mCaretPos > 0)
      {
        // When holding control delete the entire previous token
        if (ctrlPressed)
        {
          ExtendSelection(SelectMode::Left);
          ReplaceSelection(StringRange());
        }
        // Otherwise delete just the previous rune from the text
        else
        {
          SetEditSelection(mCaretPos - 1, mCaretPos);
          ReplaceSelection(StringRange());
        }
      }
    }

    DispatchBubble(Events::TextChanged, &objectEvent);
    keyboardEvent->Handled = true;
    break;
  }

  case Keys::Delete:
  {
    // If text is selected, remove the selection and move the
    // caret where appropriate
    if (textSelected)
    {
      ReplaceSelection(StringRange());
    }
    else
    {
      if (mCaretPos < size)
      {
        // When holding control delete the entire following token
        if (ctrlPressed)
        {
          ExtendSelection(SelectMode::Right);
          ReplaceSelection(StringRange());
        }
        // Otherwise delete just the following rune from the text
        else
        {
          SetEditSelection(mCaretPos, mCaretPos + 1);
          ReplaceSelection(StringRange());
        }
      }
    }

    DispatchBubble(Events::TextChanged, &objectEvent);

    keyboardEvent->Handled = true;
    break;
  }
  }

  // Process shortcuts
  if (ctrlPressed)
  {
    switch (key)
    {
    case Keys::A:
    {
      SetEditSelection(0, mDisplayText.ComputeRuneCount());
      keyboardEvent->Handled = true;
      break;
    }
    }
  }
}

void EditText::OnFocusGained(FocusEvent* focusEvent)
{
  SelectNone();
  mTextModified = false;
  mMouseMovedFocus = false;
  mHasFocus = true;
}

void EditText::OnFocusLost(FocusEvent* focusEvent)
{
  if (!mEditEnabled)
    return;

  mOffset = 0.0f;
  SelectNone();
  mHasFocus = false;
  if (mTextModified)
  {
    ObjectEvent objectEvent(this);
    DispatchBubble(Events::TextSubmit, &objectEvent);
  }
}

void EditText::StopEdit()
{
  // Cancel the edit of text
  this->GetParent()->LoseFocus();
}

void EditText::OnFocusReset(FocusEvent* focusEvent)
{
  StopEdit();
}

void EditText::OnLeftMouseDown(MouseEvent* mouseEvent)
{
  if (!mEditEnabled)
    return;

  // Processed the mouse down
  mouseEvent->Handled = true;

  if (mouseEvent->ShiftPressed)
  {
    // Shift Selection to cursor
    int endSelection = CharacterPositionAt(mouseEvent->Position);
    SetEditSelection(mCaretPos, endSelection);
  }
  else
  {
    SelectNone();
    mStartDragPos = MoveEditCaret(mouseEvent->Position);
    mSelectionStartPos = mStartDragPos;
    this->CaptureMouse();
    mDragging = true;
  }
}

void EditText::OnLeftMouseUp(MouseEvent* mouseEvent)
{
  if (!mEditEnabled)
    return;

  // Processed the mouse Up
  mouseEvent->Handled = true;

  if (!mMouseMovedFocus)
  {
    mMouseMovedFocus = true;
    SelectAll();
  }

  this->ReleaseMouseCapture();
  mDragging = false;
}

void EditText::OnDoubleClicked(MouseEvent* mouseEvent)
{
  if (!mEditEnabled)
    return;

  mouseEvent->Handled = true;
  SelectAll();
}

void EditText::OnMouseMove(MouseEvent* mouseEvent)
{
  if (mDragging)
  {
    mMouseMovedFocus = true;
    int newPos = MoveEditCaret(mouseEvent->Position);
    mSelectionStartPos = newPos;
    if (mStartDragPos < newPos)
      SetEditSelection(mStartDragPos, newPos);
    else
      SetEditSelection(newPos, mStartDragPos);
  }
}

void EditText::OnMouseDrag(MouseEvent* mouseEvent)
{
  // If this is a editable text box dragged is
  // handled
  if (mEditEnabled)
    mouseEvent->Handled = true;
}

} // namespace Raverie
