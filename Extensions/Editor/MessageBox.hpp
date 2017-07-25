///////////////////////////////////////////////////////////////////////////////
///
/// \file MessageBox.hpp
/// Declaration of the MessageBox class.
/// 
/// Authors: Trevor Sundberg, Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
// Events
namespace Events
{
  DeclareEvent(MessageBoxResult);
}//namespace Events

// These are some pre-defined ways to make easy message boxes
const cstr MBYesNo[]         = { "Yes", "No", NULL };
const cstr MBYesNoCancel[]   = { "Yes", "No", "Cancel", NULL };
const cstr MBConfirmCancel[] = { "Confirm", "Cancel", NULL };

const float cMinMessageBoxWidth = 400.f;
const float cMinMessageBoxHeight = 250.f;

DeclareEnum2(MesageBoxYesNo, Yes, No);
DeclareEnum3(MessageResult, Yes, No, Cancel);
DeclareEnum2(MessageBoxConfirmCancel, Confirm, Cancel);

//----------------------------------------------------------- Message Box Result
// An event used in a message box result
class MessageBoxEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  // The button that was pressed (zero based index)
  size_t ButtonIndex;

  // The name of the button that was pressed
  String ButtonName;
};

// Forward declarations
class TextEditor;
class TextButton;
class TextBox;
class MultiLineText;

//------------------------------------------------------------------ Message Box
/// Displays simple message boxes in the editor
class MessageBox : public Window
{
public:

  // Declare the self type
  typedef MessageBox ZilchSelf;

  // Create a message box
  static MessageBox* Show(StringParam caption, StringParam text, const cstr buttons[]);
  static MessageBox* Show(StringParam caption, StringParam text, Array<String>& buttons);
  static MessageBox* Show(Composite* parent, StringParam caption, StringParam text, const cstr buttons[]);
  static MessageBox* Show(Composite* parent, StringParam caption, StringParam text, Array<String>& buttons);

  bool TakeFocusOverride() override;

protected:
  // Constructor
  MessageBox(Composite* parent, StringParam caption, StringParam text, const cstr buttons[]);
  MessageBox(Composite* parent, StringParam caption, StringParam text, Array<String>& buttons);

private:
  void OnDestroy() override;

  // Store an array of the buttons (so we can resolve what number)
  Array<TextButton*> mButtons;

  // The text that the user displays
  MultiLineText* mText;
  Widget* mBlock;

  // Occurs when the user presses any button
  void OnButtonPressed(ObjectEvent* event);
};

}//namespace Zero
