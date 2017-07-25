////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Item;
class ItemGroup;

//------------------------------------------------------------------------------------ Item Selector
class ItemList : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ItemList(Composite* parent, float itemHeight, uint columns);

  void UpdateTransform() override;

  /// Does nothing if the group already exists.
  void AddGroup(StringParam groupName, uint weight = 0);
  Item* AddItem(StringParam name, StringParam displayName, StringParam groupName, uint weight = 0);
  void Clear();
  String GetSelectedItem();
  void SetSelectedItem(StringParam itemName, bool sendEvent = true);
  void ClearSelection();
  void ScrollToItem(StringParam itemName);
  void SelectFirstItem();

  void Sort();

private:
  bool TakeFocusOverride() override;
  void OnKeyDown(KeyboardEvent* e);
  void OnItemSelected(ObjectEvent* e);

  float mItemHeight;
  uint mColumns;

  HashMap<String, ItemGroup*> mGroups;
  HashMap<String, Item*> mItems;
  Array<Item*> mSortedItems;
  ScrollArea* mScrollArea;
  Item* mSelectedItem;
};

//------------------------------------------------------------------------------- Weighted Composite
class WeightedComposite : public ColoredComposite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  WeightedComposite(Composite* parent, StringParam displayName, uint weight, Vec4Param color);
  bool operator<(const WeightedComposite& rhs);
  uint mWeight;
};

//--------------------------------------------------------------------------------------- Item Group
class ItemGroup : public WeightedComposite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ItemGroup(Composite* parent, StringParam name, uint weight, float itemHeight, uint columns);

  Item* AddItem(StringParam name, StringParam displayName, uint weight);
  void Sort();

  Array<Item*> mItems;
  /// All items are attached to this.
  Composite* mItemsParent;
  uint mColumns;
};

//--------------------------------------------------------------------------------------------- Item
class Item : public WeightedComposite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  Item(Composite* parent, StringParam itemName, StringParam displayName, uint weight);

  void OnMouseEnter(Event*);
  void OnMouseClick(Event*);
  void OnMouseExit(Event*);
  
  void OnFocusGained(Event*);
  void OnFocusLost(Event*);

  void Select();
  void DeSelect();

  void UpdateTransform() override;

  uint mColumn;
  String mItemName;
  Label* mLabel;
  bool mSelected;
  ItemGroup* mGroup;
  Element* mFocusHighlight;
};

//--------------------------------------------------------------------------------- Item Grid Layout
class ItemGridLayout : public Layout
{
public:
  float mItemHeight;
  uint mColumns;

  ItemGridLayout(float itemHeight, uint columns);
  Vec2 Measure(Composite* widget, LayoutArea data) override;
  Vec2 DoLayout(Composite* widget, LayoutArea data) override;
};

} // namespace Zero
