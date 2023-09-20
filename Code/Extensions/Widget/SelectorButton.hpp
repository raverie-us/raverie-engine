// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class ObjectEvent;
class TextButton;

/// Sends Events::ItemSelected when the selection has changed.
class SelectorButton : public Composite
{
public:
  RaverieDeclareType(SelectorButton, TypeCopyMode::ReferenceType);

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

  /// Are the values being displayed selectable?
  void SetSelectable(bool selectable);

  /// Get the index of the Button being hovered over by the mouse
  int GetHoverItem()
  {
    return mHoverItem;
  }

  ListSource* GetDataSource();

  void CreateButton(StringParam name);

  /// Event response.
  void OnButtonPressed(ObjectEvent* e);
  void OnMouseHover(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);

  void SelectButton(TextButton* button);
  void DeSelectButton(TextButton* button);
  void DeselectAll();

  HandleOf<ToolTip> mToolTip;

  ListSource* mDataSource;
  Array<TextButton*> mButtons;
  int mSelectedItem;
  int mHoverItem;
  bool mAllowSelect;
};

} // namespace Raverie
