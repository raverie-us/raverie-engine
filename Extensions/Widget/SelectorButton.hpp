///////////////////////////////////////////////////////////////////////////////
///
/// \file SelectorButton.hpp
/// Declaration of the SelectorButton control.
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ObjectEvent;
class TextButton;

//-------------------------------------------------------------- Selector Button
/// Sends Events::ItemSelected when the selection has changed.
class SelectorButton : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SelectorButton(Composite* parent);

  /// Widget Interface.
  Vec2 GetMinSize() override;
  void UpdateTransform() override;

  /// Builds the buttons from he given names
  void CreateButtons(const cstr* names, uint count);
  void CreateButtons(ListSource* source);
  void Clear();

  /// Selected items
  void SetSelectedItem(int index, bool sendEvent);
  int GetSelectedItem();

  /// Get the index of the Button being hovered over by the mouse
  int GetHoverItem( ) { return mHoverItem; }

  /// Get the Button being hovered over by the mouse, if there is one.
  TextButton* GetHoverButton( ) { return (mHoverItem != -1) ? mButtons[mHoverItem] : nullptr;  }

private:
  void CreateButton(StringParam name);

  /// Event response.
  void OnButtonPressed(ObjectEvent* e);
  void OnMouseHover(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);

  void SelectButton(TextButton* button);
  void DeSelectButton(TextButton* button);
  void DeselectAll();

  Array<TextButton*> mButtons;
  int mSelectedItem;
  int mHoverItem;
};

}//namespace Zero
