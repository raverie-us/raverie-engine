///////////////////////////////////////////////////////////////////////////////
///
/// \file MessageBox.cpp
/// Implementation of the MessageBox class.
/// 
/// Authors: Trevor Sundberg, Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
namespace Zero
{

namespace Events
{
  DefineEvent(MessageBoxResult);
}//namespace Events

ZilchDefineType(MessageBoxEvent, builder, type)
{
}

// Create a message box
MessageBox* MessageBox::Show(StringParam caption, StringParam text, const cstr buttons[])
{
  //// Create a new message box
  Composite* composite = Z::gEditor->GetRootWidget();

  MessageBox* dialog = new MessageBox(composite, caption, text, buttons);

  // Return the created message box
  return dialog;
}

// Create a message box
MessageBox* MessageBox::Show(StringParam caption, StringParam text, Array<String>& buttons)
{
  //// Create a new message box
  Composite* composite = Z::gEditor->GetRootWidget();

  MessageBox* dialog = new MessageBox(composite, caption, text, buttons);

  // Return the created message box
  return dialog;
}

MessageBox* MessageBox::Show(Composite* parent, StringParam caption, StringParam text, const cstr buttons[])
{
  MessageBox* dialog = new MessageBox(parent, caption, text, buttons);

  // Return the created message box
  return dialog;
}

Zero::MessageBox* MessageBox::Show(Composite* parent, StringParam caption, StringParam text, Array<String>& buttons)
{
  MessageBox* dialog = new MessageBox(parent, caption, text, buttons);

  // Return the created message box
  return dialog;
}

// Constructor
MessageBox::MessageBox(Composite* parent, StringParam caption, StringParam text,
                       const cstr buttons[]) 
  : Window(parent)
{
  //Create a large transparent block to darken and block input on the screen.
  mBlock = CreateBlackOut(parent);
  mBlock->MoveToFront();

  this->MoveToFront();

  // Set the caption
  SetTitle(caption);

  // Set us to be in dock layout mode
  this->SetLayout(CreateStackLayout());

  mText = new MultiLineText(this);
  mText->SetText(text);
  mText->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  Composite* buttonRow = new Composite(this);
  buttonRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(4,4), Thickness::All(4)));

  const cstr* current = buttons;
  while(*current != NULL)
  {
    // Create a new button and setup the text
    TextButton* button = new TextButton(buttonRow);

    // Set the text of the button
    button->SetText(*current);
    button->SetName(*current);

    // Connect to the button
    ConnectThisTo(button, Events::ButtonPressed, OnButtonPressed);

    // Add the button to a list
    mButtons.PushBack(button);

    ++current;
  }

  // Hide the exit button
  mCloseButton->SetActive(false);

  // Size to the contents of the message box taking our minimums into account vs the content
  Vec2 size = GetMinSize();
  size.x = size.x < cMinMessageBoxWidth ? cMinMessageBoxWidth : size.x;
  size.y = size.y < cMinMessageBoxHeight ? cMinMessageBoxHeight : size.y;
  SetSize(size);
  
  // Center the message box on the window
  CenterToWindow(parent, this, false);

  this->TakeFocus();
}

MessageBox::MessageBox(Composite* parent, StringParam caption, StringParam text, Array<String>& buttons)
  : Window(parent)
{
  //Create a large transparent block to darken and block input on the screen.
  mBlock = CreateBlackOut(parent);
  mBlock->MoveToFront();

  this->MoveToFront();

  // Set the caption
  SetTitle(caption);

  // Set us to be in dock layout mode
  this->SetLayout(CreateStackLayout());

  mText = new MultiLineText(this);
  mText->SetText(text);
  mText->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  Composite* buttonRow = new Composite(this);
  buttonRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(4, 4), Thickness::All(4)));

  forRange (StringParam current, buttons.All())
  {
    // Create a new button and setup the text
    TextButton* button = new TextButton(buttonRow);

    // Set the text of the button
    button->SetText(current);
    button->SetName(current);

    // Connect to the button
    ConnectThisTo(button, Events::ButtonPressed, OnButtonPressed);

    // Add the button to a list
    mButtons.PushBack(button);
  }

  // Hide the exit button
  mCloseButton->SetActive(false);

  // Size to the contents of the message box taking our minimums into account vs the content
  Vec2 size = GetMinSize();
  size.x = size.x < cMinMessageBoxWidth ? cMinMessageBoxWidth : size.x;
  size.y = size.y < cMinMessageBoxHeight ? cMinMessageBoxHeight : size.y;
  SetSize(size);

  // Center the message box on the window
  CenterToWindow(parent, this, false);
  
  this->TakeFocus();
}

bool MessageBox::TakeFocusOverride()
{
  if(mButtons.Size() > 0)
    mButtons[0]->TakeFocus();
  return true;
}

// Occurs when the user presses any button
void MessageBox::OnButtonPressed(ObjectEvent* event)
{
  // Fill out the message box result
  MessageBoxEvent result;
  result.ButtonIndex = size_t(-1);
  result.ButtonName = String();

  // Loop through all the buttons we already
  for(size_t i = 0; i < mButtons.Size(); ++i)
  {
    // Get the current button
    TextButton* button = mButtons[i];

    // Check if that is the button that pressed
    if (button == event->Source)
    {
      // Set the button's index and name
      result.ButtonIndex = i;
      result.ButtonName = button->GetName();
      break;
    }
  }

  // Invoke an event that tells any listener that a button was pressed
  GetDispatcher()->Dispatch(Events::MessageBoxResult, &result);

  // Remove the message box
  this->Destroy();
}

void MessageBox::OnDestroy()
{
  mBlock->Destroy();
  Composite::OnDestroy();
}

}//namespace Zero
