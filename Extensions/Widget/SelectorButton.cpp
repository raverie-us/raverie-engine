///////////////////////////////////////////////////////////////////////////////
///
/// \file SelectorButton.cpp
/// Implementation of the SelectorButton control.
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace SelectorButtonUi
{
  const cstr cLocation = "EditorUi/Controls/SelectorButton";
  Tweakable(Vec4, DefaultColor,           Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, DefaultHighlightColor,  Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, DefaultClickedColor,    Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, SelectedColor,          Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, SelectedHighlightColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, SelectedClickedColor,   Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, DefaultTextColor,       Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, SelectedTextColor,      Vec4(1,1,1,1), cLocation);
}

//-------------------------------------------------------------- Selector Button
ZilchDefineType(SelectorButton, builder, type)
{
}

//******************************************************************************
SelectorButton::SelectorButton(Composite* parent)
  : Composite(parent)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight ,Pixels(2,2),
                              Thickness::cZero));
  mSelectedItem = 0;
  mHoverItem = -1;

  ConnectThisTo(this, Events::MouseHover, OnMouseHover);
}

//******************************************************************************
Vec2 SelectorButton::GetMinSize()
{
  Vec2 minSize = Vec2::cZero;
  for(uint i = 0; i < mButtons.Size(); ++i)
  {
    minSize += mButtons[i]->GetMinSize();

    // Padding
    minSize.x += Pixels(2);
  }

  return minSize;
}

//******************************************************************************
void SelectorButton::UpdateTransform()
{
  Composite::UpdateTransform();
}

//******************************************************************************
void SelectorButton::CreateButtons(const cstr* names, uint count)
{
  // Clear any old buttons
  Clear();

  for(uint i = 0; i < count; ++i)
    CreateButton(names[i]);
}

//******************************************************************************
void SelectorButton::CreateButtons(ListSource* source)
{
  // Clear any old buttons
  Clear();

  for(uint i = 0; i < source->GetCount(); ++i)
    CreateButton(source->GetStringValueAt(i));
}

//******************************************************************************
void SelectorButton::Clear()
{
  for(uint i = 0; i < mButtons.Size(); ++i)
    SafeDestroy(mButtons[i]);
  mButtons.Clear();
}

//******************************************************************************
void SelectorButton::SetSelectedItem(int index, bool sendEvent)
{
  SelectButton(mButtons[index]);
}

//******************************************************************************
int SelectorButton::GetSelectedItem()
{
  return mSelectedItem;
}

//******************************************************************************
void SelectorButton::CreateButton(StringParam name)
{
  TextButton* button = new TextButton(this);
  button->SetText(name);
  button->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  DeSelectButton(button);
  mButtons.PushBack(button);
  ConnectThisTo(button, Events::ButtonPressed, OnButtonPressed);
}

//******************************************************************************
void SelectorButton::OnButtonPressed(ObjectEvent* e)
{
  SelectButton((TextButton*)e->Source);

  ObjectEvent eventToSend(this);
  DispatchEvent(Events::ItemSelected, &eventToSend);
}

//******************************************************************************
void SelectorButton::OnMouseHover(MouseEvent* event)
{
  int buttonCount = mButtons.Size();
  for(int i = 0; i < buttonCount; ++i)
  {
    if(mButtons[i]->mMouseOver)
    {
      mHoverItem = i;
      break;
    }

  }

}

//******************************************************************************
void SelectorButton::OnMouseExit(MouseEvent* event)
{
  mHoverItem = -1;
}

//******************************************************************************
void SelectorButton::SelectButton(TextButton* button)
{
  DeselectAll();
  button->mBackgroundColor = ToByteColor(SelectorButtonUi::SelectedColor);
  button->mBackgroundHoverColor = ToByteColor(SelectorButtonUi::SelectedHighlightColor);
  button->mBackgroundClickedColor = ToByteColor(SelectorButtonUi::SelectedClickedColor);
  button->mButtonText->SetColor(SelectorButtonUi::SelectedTextColor);
  button->MarkAsNeedsUpdate();

  mSelectedItem = (int)mButtons.FindIndex(button);
}

//******************************************************************************
void SelectorButton::DeSelectButton(TextButton* button)
{
  button->mBackgroundColor = ToByteColor(SelectorButtonUi::DefaultColor);
  button->mBackgroundHoverColor = ToByteColor(SelectorButtonUi::DefaultHighlightColor);
  button->mBackgroundClickedColor = ToByteColor(SelectorButtonUi::DefaultClickedColor);
  button->mButtonText->SetColor(SelectorButtonUi::DefaultTextColor);
  button->MarkAsNeedsUpdate();
}

//******************************************************************************
void SelectorButton::DeselectAll()
{
  for(uint i = 0; i < mButtons.Size(); ++i)
    DeSelectButton(mButtons[i]);
}

}//namespace Zero
