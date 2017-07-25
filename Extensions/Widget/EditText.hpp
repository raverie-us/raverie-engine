///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  //The value in the text has been changed and submitted by
  //enter being pressed or the control lost focus.
  DeclareEvent(TextBoxChanged);

  /// When the text has been changed at all.
  DeclareEvent(TextChanged);

  /// When focus on the edit text is lost, but the text was modified.
  DeclareEvent(TextSubmit);

  /// When the Enter key is pressed while editing text.
  DeclareEvent(TextEnter);
}

class RenderFont;

class EditText : public Widget
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  EditText(Composite* parent);
  ~EditText();

  // Basic text functions.
  void SetText(StringParam text);
  StringParam GetText(){return mDisplayText;}

  void SetTextSize(uint textSize);
  void GetTextSize();

  // Sets the offset used to scroll text.
  // A wrapper for now in case this needs to change.
  void SetTextOffset(float offset);
  
  // Can the text be edited?
  void SetEditable(bool state);
  void SetTextClipping(bool value){mClipText = value;}
  void SetClearFocus(bool value){mEnterClearFocus = value;};
  void SetEditCaretPos(int caretPos);
  int MoveEditCaret(Vec2Param newPosition);
  void MakeLetterVisible(int characterIndex);
  int CharacterPositionAt(Vec2Param screenPos);
  void SetEditSelection(int selectionStart, int selectionEnd);
  void SelectNone();
  void SelectAll();
  void Clear();
  bool IsValidSelection();
  void ReplaceSelection(StringRange text);
  void StopEdit();

  StringRange GetSelectedText();
  RenderFont* GetFont(){return mFont;}

  // Widget Interface
  Vec2 GetMinSize() override;

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

  String GetDisplayName();
  void SizeToContents() override;

  // Events
  void OnTextTyped(KeyboardTextEvent* keyboardEvent);
  void OnKeyDown(KeyboardEvent* keyboardEvent);
  void OnFocusGained(FocusEvent* focusEvent);
  void OnFocusLost(FocusEvent* focusEvent);
  void OnFocusReset(FocusEvent* focusEvent);
  void OnLeftMouseDown(MouseEvent* mouseEvent);
  void OnLeftMouseUp(MouseEvent* mouseEvent);
  void OnMouseMove(MouseEvent* mouseEvent);
  void OnMouseDrag(MouseEvent* mouseEvent);
  void OnDoubleClicked(MouseEvent* mouseEvent);
  void ChangeDefinition(BaseDefinition* def);
//private:
  // State
  bool mHasFocus;
  bool mDragging;
  float mOffset;
  int mStartDragPos;
  bool mEditEnabled;
  bool mTextModified;
  bool mMouseMovedFocus;
  bool mClipText;
  bool mPassword;
  bool mEnterClearFocus;
  int mSelectionStart;
  int mSelectionEnd;
  int mCaretPos;
  TextAlign::Enum mAlign;
  int mTextSize;
  RenderFont* mFont;
  String mDisplayText;
};

}
