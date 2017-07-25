///////////////////////////////////////////////////////////////////////////////
///
/// \file EditText.cpp
/// Implementation of the display object text class.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(TextBoxChanged);
  DefineEvent(TextChanged);
  DefineEvent(TextSubmit);
  DefineEvent(TextEnter);
}

ZilchDefineType(EditText, builder, type)
{
}

EditText::EditText(Composite* parent)
  : Widget(parent)
{
  mFont = FontManager::GetDefault()->GetRenderFont(11);
  mSize = mFont->MeasureText(" ");
  mDragging = false;
  mEditEnabled = false;
  mCaretPos = 0;
  mHasFocus = false;
  mClipText = false;
  mEnterClearFocus = true;
  mPassword = false;
  mAlign = TextAlign::Left;
  mOffset = 0;

  SetTakeFocusMode(FocusMode::Hard);

  SelectNone();

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
  //mDef = (TextDefinition*)def; 
  //mFont = mDef->mFont;
}

void EditText::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
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
    clipRect = Rect::PointAndSize(Vec2(mWorldTx.m30, mWorldTx.m31), Vec2(mSize.x + 1, mSize.y));

  if (mEditEnabled && mHasFocus)
  {
    Vec2 pos0, pos1;
    Vec4 boxColor = color;
    // If there is a valid selection range draw the selection background
    if (IsValidSelection())
    {
      pos0 = mFont->MeasureText(text, mSelectionStart, 1.0f);
      pos0.y = 0.0f;
      pos1 = mFont->MeasureText(text, mSelectionEnd, 1.0f);
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
  if(mDisplayText.Empty())
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
  mSelectionStart = 0;
  mSelectionEnd = 0;
}

void EditText::MakeLetterVisible(int characterIndex)
{
  Vec2 size = mFont->MeasureText(mDisplayText, characterIndex, 1.0f);

  if(size.x > mSize.x - mOffset)
    mOffset =  mSize.x - size.x;

  if(size.x + mOffset < 0)
    mOffset = -size.x ;
}

void EditText::SetEditCaretPos(int caretPos)
{
  // Selection can be at the End()
  int maxSize = mDisplayText.ComputeRuneCount();
  mCaretPos = Math::Clamp(caretPos, 0 , maxSize);

  MakeLetterVisible(mCaretPos);
}

void EditText::SetEditSelection(int selectionStart, int selectionEnd)
{
  if(selectionEnd < selectionStart)
    Math::Swap(selectionStart, selectionEnd);

  // Selection can include the End()
  int maxSize = mDisplayText.ComputeRuneCount();
  mSelectionStart = Math::Clamp(selectionStart, 0, maxSize);
  mSelectionEnd = Math::Clamp(selectionEnd, 0, maxSize);
}

bool EditText::IsValidSelection()
{
  return mSelectionEnd - mSelectionStart > 0;
}

int EditText::CharacterPositionAt(Vec2Param screenPos)
{
  Vec2 textStart = Vec2(mOffset, 0);
  Vec2 localPos = this->ToLocal(screenPos) - textStart;
  return mFont->GetPosition(mDisplayText.All(), 
                                  localPos.x,
                                  1.0f, TextRounding::Nearest);
}

int EditText::MoveEditCaret(Vec2Param screenPos)
{
  int newCaretPosition = CharacterPositionAt(screenPos);
  SetEditCaretPos(newCaretPosition);
  return mCaretPos;
}

void EditText::ReplaceSelection(StringRange text)
{
  mTextModified = true;

  // If their is no selection used the caret position
  if(!IsValidSelection())
  {
    mSelectionStart = mCaretPos;
    mSelectionEnd = mCaretPos;
  }

  // Replace the sub string 
  StringIterator displayTextStartIt = mDisplayText.Begin();
  StringIterator selectionStartIt = displayTextStartIt + mSelectionStart;
  StringIterator selectionEndIt = displayTextStartIt + mSelectionEnd;
  mDisplayText = BuildString(mDisplayText.SubString(displayTextStartIt, selectionStartIt), text, mDisplayText.SubString(selectionEndIt, mDisplayText.End()));
  // Move the caret to the End() of the pasted text
  
  int newCaretPos = mSelectionStart + text.ComputeRuneCount();
  SetEditCaretPos(newCaretPos);

  // Clear the selection
  SelectNone();
}

StringRange EditText::GetSelectedText()
{
  int selectionSizeInBytes = mSelectionEnd - mSelectionStart;
  if(mDisplayText.SizeInBytes() && selectionSizeInBytes > 0)
  {
    return StringRange(mDisplayText.Begin() + mSelectionStart, mDisplayText.Begin() + mSelectionEnd);
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
  if(!mEditEnabled)
    return;

  Rune key = keyboardEvent->mRune;
  bool textSelected = IsValidSelection();

  // key > 255 is temporary fix for unicode and windows
  if( IsGraph(key) || key == ' ' || key > 255)
  {
    mTextModified = true;

    String textToAdd(key);

    if(textSelected)
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

void EditText::OnKeyDown(KeyboardEvent* keyboardEvent)
{
  if(!mEditEnabled)
    return;

  uint key = keyboardEvent->Key;
  int size = mDisplayText.ComputeRuneCount();
  bool textSelected = IsValidSelection();
  bool shiftPressed = keyboardEvent->ShiftPressed;
  bool ctrlPressed = keyboardEvent->CtrlPressed;

  // Handle all graphical keys (including space)
  if(IsGraphOrSpace(Rune(key)))
    keyboardEvent->Handled = true;

  ObjectEvent objectEvent(this);
  // Process control keys
  switch(key)
  {

    case Keys::Enter:
    {
      // Enter key
      DispatchBubble(Events::TextEnter, &objectEvent);
      if(mEnterClearFocus)
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
      if(shiftPressed)
      {
        //Highlight everything to the left
        if(ctrlPressed)
          SetEditSelection(0, mCaretPos);
        else if(textSelected)
        {
          //If the selection is to the left of the caret, grow the selection 
          //left
          if(mSelectionStart < mCaretPos && mSelectionStart > 0)
            --mSelectionStart;
          //If it is to the right, shrink it from the right
          else if(mSelectionEnd > mCaretPos)
            --mSelectionEnd;

          MakeLetterVisible(mSelectionStart);

        }
        else if(mCaretPos > 0)
          SetEditSelection(mCaretPos - 1, mCaretPos);
      }
      //For now, jump all the way to the left.  Should jump to the next token.
      else if(ctrlPressed)
      {
        SetEditCaretPos(0);
        SelectNone();
      }
      //Move the caret to the start of the selection and de-select
      else if(textSelected)
      {
        SetEditCaretPos(mSelectionStart);
        SelectNone();
      }
      else
        SetEditCaretPos(mCaretPos - 1);

      keyboardEvent->Handled = true;
      break;
    }

    case Keys::Right:
    {
      if(shiftPressed)
      {
        //Highlight everything to the right
        if(ctrlPressed)
          SetEditSelection(mCaretPos, size);

        else if(textSelected)
        {
          //If the selection is to the right of the caret, grow the selection 
          //right
          if(mSelectionEnd > mCaretPos && mSelectionEnd < size)
            ++mSelectionEnd;
          //If it is to the left, shrink it from the left
          else if(mSelectionStart < mCaretPos)
            ++mSelectionStart;

          MakeLetterVisible(mSelectionEnd);
        }
        else if(mCaretPos < size)
          SetEditSelection(mCaretPos, mCaretPos + 1);
      }
      //For now, jump all the way to the right.  Should jump to the next token.
      else if(ctrlPressed)
      {
        SetEditCaretPos(size);
        SelectNone();
      }
      //Move the caret to the End() of the selection and de-select
      else if(textSelected)
      {
        SetEditCaretPos(mSelectionEnd);
        SelectNone();
      }
      else
        SetEditCaretPos(mCaretPos + 1);

      keyboardEvent->Handled = true;
      break;
    }

    case Keys::Home:
    {
      //Select from the beginning to the caret
      if(shiftPressed)
        SetEditSelection(0, mCaretPos);
      else
      {
        //Move to the start and clear the selection
        SetEditCaretPos(0);
        SelectNone();
      }

      keyboardEvent->Handled = true;
      break;
    }

    case Keys::End:
    {
      //Select from the caret to the End()
      if(shiftPressed)
        SetEditSelection(mCaretPos, size);
      else
      {
        //Move to the End() and clear the selection
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
      if(textSelected)
      {
        ReplaceSelection(StringRange());
      }
      else
      {
        if(mCaretPos > 0)
        {
          SetEditSelection(mCaretPos - 1, mCaretPos);
          ReplaceSelection(StringRange());
        }
      }

      DispatchBubble(Events::TextChanged, &objectEvent);
      keyboardEvent->Handled = true;
      break;
    }

    case Keys::Delete:
    {
      //If text is selected, remove the selection and move the
      //caret where appropriate
      if(textSelected)
      {
        ReplaceSelection(StringRange());
      }
      else
      {
        if(mCaretPos < size)
        {
          SetEditSelection(mCaretPos, mCaretPos + 1);
          ReplaceSelection(StringRange());
        }
      }

      DispatchBubble(Events::TextChanged, &objectEvent);

      keyboardEvent->Handled = true;
      break;
    }
  }

  // Process shortcuts
  if(ctrlPressed)
  {
    switch(key)
    {
      case Keys::A:
      {
        SetEditSelection(0, mDisplayText.ComputeRuneCount());
        keyboardEvent->Handled = true;
        break;
      }

      case Keys::V:
      {
        String toPaste = Z::gEngine->has(OsShell)->GetClipboardText();
        ReplaceSelection( RangeUntilFirst(toPaste, IsControl) );
        keyboardEvent->Handled = true;
        DispatchBubble(Events::TextChanged, &objectEvent);
        break;
      }

      case Keys::C:
      {
        StringRange toCopy = GetSelectedText();
        Z::gEngine->has(OsShell)->SetClipboardText(toCopy);
        keyboardEvent->Handled = true;
        break;
      }

      case Keys::X:
      {
        StringRange toCut = GetSelectedText();
        Z::gEngine->has(OsShell)->SetClipboardText(toCut);
        ReplaceSelection(String());
        keyboardEvent->Handled = true;
        DispatchBubble(Events::TextChanged, &objectEvent);
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
  if(!mEditEnabled)
    return;

  mOffset = 0.0f;
  SelectNone();
  mHasFocus = false;
  if(mTextModified)
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
  if(!mEditEnabled)
    return;

  // Processed the mouse down
  mouseEvent->Handled = true;

  if(mouseEvent->ShiftPressed)
  {
    // Shift Selection to cursor
    int endSelection = CharacterPositionAt(mouseEvent->Position);
    SetEditSelection(mCaretPos, endSelection);
  }
  else
  {
    SelectNone();
    mStartDragPos = MoveEditCaret(mouseEvent->Position);
    this->CaptureMouse();
    mDragging = true;
  }
}

void EditText::OnLeftMouseUp(MouseEvent* mouseEvent)
{
  if(!mEditEnabled)
    return;

  // Processed the mouse Up
  mouseEvent->Handled = true;

  if(!mMouseMovedFocus)
  {
    mMouseMovedFocus = true;
    SelectAll();
  }

  this->ReleaseMouseCapture();
  mDragging = false;
}

void EditText::OnDoubleClicked(MouseEvent* mouseEvent)
{
  if(!mEditEnabled)
    return;

  mouseEvent->Handled = true;
  SelectAll();
}

void EditText::OnMouseMove(MouseEvent* mouseEvent)
{
  if(mDragging)
  {
    mMouseMovedFocus = true;
    int newPos = MoveEditCaret(mouseEvent->Position);
    if(mStartDragPos < newPos)
      SetEditSelection(mStartDragPos, newPos);
    else
      SetEditSelection(newPos, mStartDragPos);
  }
}

void EditText::OnMouseDrag(MouseEvent* mouseEvent)
{
  // If this is a editable text box dragged is
  // handled
  if(mEditEnabled)
    mouseEvent->Handled = true;
}

}
