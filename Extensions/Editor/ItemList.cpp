////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
// Used to sort the widget hierarchy
struct SortGroup
{
  bool operator()(Widget& left, Widget& right)
  {
    return (*this)(&left, &right);
  }

  bool operator()(Widget* left, Widget* right)
  {
    // Hack fix, don't really care about this code as it will be removed soon. There is a spacer
    // in the list of widgets I'm trying to sort, so we're just ignoring it here
    if (Type::DynamicCast<Spacer*>(left))
      return true;

    WeightedComposite& lhs = *(WeightedComposite*)left;
    WeightedComposite& rhs = *(WeightedComposite*)right;
    return lhs < rhs;
  }
};

struct SortItem
{
  bool operator()(Item* left, Item* right)
  {
    if (left->mGroup != right->mGroup)
      return *left->mGroup < *right->mGroup;
    return *left < *right;
  }
};

//------------------------------------------------------------------------------------ Item Selector
//**************************************************************************************************
ZilchDefineType(ItemList, builder, type)
{
}

//**************************************************************************************************
ItemList::ItemList(Composite* parent, float itemHeight, uint columns)
  : Composite(parent)
  , mItemHeight(itemHeight)
  , mColumns(columns)
  , mSelectedItem(nullptr)
{
  SetLayout(CreateFillLayout());

  mScrollArea = new ScrollArea(this, true);
  mScrollArea->DisableScrollBar(0);

  Composite* clientArea = mScrollArea->GetClientWidget();

  clientArea->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 2), Thickness::cZero));
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(this, Events::KeyRepeated, OnKeyDown);
}

//**************************************************************************************************
void ItemList::UpdateTransform()
{
  Composite* clientArea = mScrollArea->GetClientWidget();
  Vec2 clientSize = clientArea->GetSize();
  clientSize.x = mSize.x;
  clientArea->SetSize(clientSize);

  Composite::UpdateTransform();
}

//**************************************************************************************************
void ItemList::AddGroup(StringParam groupName, uint weight)
{
  if (mGroups.ContainsKey(groupName))
    return;

  ItemGroup* group = new ItemGroup(mScrollArea, groupName, weight, mItemHeight, mColumns);
  group->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  group->SetSizing(SizeAxis::Y, SizePolicy::Auto, 0.0f);

  mGroups.Insert(groupName, group);
}

//**************************************************************************************************
Item* ItemList::AddItem(StringParam name, StringParam displayName, StringParam groupName, uint weight)
{
  ItemGroup* group = mGroups.FindValue(groupName, nullptr);
  ReturnIf(group == nullptr, nullptr, "Create group first");

  Item* item = group->AddItem(name, displayName, weight);

  mItems.Insert(name, item);
  mSortedItems.PushBack(item);
  ConnectThisTo(item, Events::ItemSelected, OnItemSelected);

  mScrollArea->GetClientWidget()->SizeToContents();
  return item;
}

//**************************************************************************************************
void ItemList::Clear()
{
  // Destroy all groups and items
  forRange(ItemGroup* group, mGroups.Values())
    group->Destroy();

  mSelectedItem = nullptr;
  mGroups.Clear();
  mSortedItems.Clear();
  mItems.Clear();
}

//**************************************************************************************************
String ItemList::GetSelectedItem()
{
  if (mSelectedItem)
    return mSelectedItem->mItemName;
  return "";
}

//**************************************************************************************************
void ItemList::SetSelectedItem(StringParam itemName, bool sendEvent)
{
  if(Item* item = mItems.FindValue(itemName, nullptr))
  {
    // Don't do anything if it's already selected
    if (item == mSelectedItem)
      return;

    bool previousHasFocus = false;

    // Deselect previous item
    if (mSelectedItem)
    {
      mSelectedItem->DeSelect();
      previousHasFocus = mSelectedItem->HasFocus();
    }

    // Select the new item
    item->Select();
    mSelectedItem = item;

    if (previousHasFocus)
      mSelectedItem->SoftTakeFocus();

    // Send an event saying something was selected
    if(sendEvent)
    {
      Event e;
      DispatchEvent(Events::ItemSelected, &e);
    }
  }
}

//**************************************************************************************************
void ItemList::ClearSelection()
{
  if (mSelectedItem)
    mSelectedItem->DeSelect();
  mSelectedItem = nullptr;
}

//**************************************************************************************************
void ItemList::ScrollToItem(StringParam itemName)
{
  if (Item* item = mItems.FindValue(itemName, nullptr))
  {
    Vec3 itemPosScreen = item->GetScreenPosition();
    Vec2 itemPosLocal = ToVector2(mScrollArea->GetClientWidget()->ToLocal(itemPosScreen));

    mScrollArea->ScrollAreaToView(itemPosLocal, itemPosLocal + item->GetSize(), true);
  }
}

//**************************************************************************************************
void ItemList::SelectFirstItem()
{
  if(!mSortedItems.Empty())
    SetSelectedItem(mSortedItems[0]->mItemName);
}

//**************************************************************************************************
void ItemList::OnKeyDown(KeyboardEvent* e)
{
  if(mSelectedItem)
  {
    int startIndex = mSortedItems.FindIndex(mSelectedItem);
    int currentIndex = startIndex;
    if (e->Key == Keys::Right)
      ++currentIndex;
    else if (e->Key == Keys::Left)
      --currentIndex;
    else if (e->Key == Keys::Down)
      currentIndex += mColumns;
    else if (e->Key == Keys::Up)
      currentIndex -= mColumns;

    currentIndex = Math::Clamp(currentIndex, 0, (int)mItems.Size() - 1);

    Item* newItem = mSortedItems[(uint)currentIndex];

    if (e->Key == Keys::Down)
    {
      if (mSelectedItem->mGroup != newItem->mGroup && mSelectedItem->mColumn != newItem->mColumn)
        currentIndex = startIndex + 1;
    }
    else if (e->Key == Keys::Up)
    {
      if (mSelectedItem->mGroup != newItem->mGroup && mSelectedItem->mColumn != newItem->mColumn)
        currentIndex = startIndex - 1;
    }

    newItem = mSortedItems[(uint)currentIndex];
    SetSelectedItem(newItem->mItemName);
    ScrollToItem(newItem->mItemName);
  }
}

//**************************************************************************************************
void ItemList::Sort()
{
  Composite* groupParent = mScrollArea->GetClientWidget();
  groupParent->mChildren.Sort(SortGroup());
  mScrollArea->mBackground->MoveToBack();

  Zero::Sort(mSortedItems.All(), SortItem());

  forRange(ItemGroup* group, mGroups.Values())
    group->Sort();
}

//**************************************************************************************************
bool ItemList::TakeFocusOverride()
{
  if(mSelectedItem)
    mSelectedItem->SoftTakeFocus();
  return true;
}

//**************************************************************************************************
void ItemList::OnItemSelected(ObjectEvent* e)
{
  Item* item = (Item*)e->Source;
  SetSelectedItem(item->mItemName);
}

//------------------------------------------------------------------------------- Weighted Composite
//**************************************************************************************************
ZilchDefineType(WeightedComposite, builder, type)
{
}

//**************************************************************************************************
WeightedComposite::WeightedComposite(Composite* parent, StringParam displayName, uint weight, Vec4Param color)
  : ColoredComposite(parent, color)
  , mWeight(weight)
{
  mName = displayName;
}

//**************************************************************************************************
bool WeightedComposite::operator<(const WeightedComposite& rhs)
{
  if (mWeight == rhs.mWeight)
    return mName < rhs.mName;
  return mWeight < rhs.mWeight;
}

//--------------------------------------------------------------------------------------- Item Group
//**************************************************************************************************
ZilchDefineType(ItemGroup, builder, type)
{
}

//**************************************************************************************************
ItemGroup::ItemGroup(Composite* parent, StringParam name, uint weight, float itemHeight, uint columns)
  : WeightedComposite(parent, name, weight, Vec4(1, 1, 1, 0.05f))
  , mColumns(columns)
{
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 2), Thickness(Pixels(0, 3, 0, 6))));

  // Label across the top
  Label* label = new Label(this);
  label->SetText(name);
  label->SizeToContents();
  label->mHorizontalAlignment = HorizontalAlignment::Center;
  label->SetSizing(SizePolicy::Fixed, label->mSize);

  // Items will attach directly to this Composite
  mItemsParent = new Composite(this);
  mItemsParent->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  mItemsParent->SetSizing(SizeAxis::Y, SizePolicy::Auto, 0.0f);
  mItemsParent->SetLayout(new ItemGridLayout(itemHeight, columns));
}

//**************************************************************************************************
Item* ItemGroup::AddItem(StringParam name, StringParam displayName, uint weight)
{
  Item* item = new Item(mItemsParent, name, displayName, weight);
  item->mGroup = this;
  mItems.PushBack(item);
  return item;
}

//**************************************************************************************************
void ItemGroup::Sort()
{
  // Sort the widgets
  mItemsParent->mChildren.Sort(SortGroup());

  // Sort our array
  Zero::Sort(mItems.All(), SortGroup());

  uint column = 0;
  forRange(Item* item, mItems.All())
  {
    item->mColumn = column;
    column = (column + 1) % mColumns;
  }
}

//--------------------------------------------------------------------------------------------- Item
//**************************************************************************************************
ZilchDefineType(Item, builder, type)
{
}

//**************************************************************************************************
Item::Item(Composite* parent, StringParam itemName, StringParam displayName, uint weight)
  : WeightedComposite(parent, displayName, weight, Vec4(1, 1, 1, 0.05f))
  , mSelected(false)
  , mItemName(itemName)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(12, 0), Thickness(Pixels(12, 0, 12, 0))));

  mLabel = new Label(this);
  mLabel->SetText(displayName);
  mLabel->SizeToContents();
  mLabel->SetSizing(SizePolicy::Fixed, mLabel->GetSize());
  mLabel->mVerticalAlignment = VerticalAlignment::Center;

  mFocusHighlight = CreateAttached<Element>(cWhiteSquareBorder);
  mFocusHighlight->SetNotInLayout(true);
  mFocusHighlight->SetColor(Vec4(0.191699997, 0.475143999, 0.709999979, 0.675f));
  mFocusHighlight->SetActive(false);

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::LeftClick, OnMouseClick);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);

  ConnectThisTo(this, Events::FocusGained, OnFocusGained);
  ConnectThisTo(this, Events::FocusLost, OnFocusLost);
}

//**************************************************************************************************
void Item::OnMouseEnter(Event*)
{
  Vec4 highlightColor(0.191699997, 0.475143999, 0.709999979, 0.5f);
  mBackground->SetColor(highlightColor);
}

//**************************************************************************************************
void Item::OnMouseClick(Event*)
{
  ObjectEvent e(this);
  DispatchEvent(Events::ItemSelected, &e);
}

//**************************************************************************************************
void Item::OnMouseExit(Event*)
{
  Vec4 highlightColor(0.191699997, 0.475143999, 0.709999979, 0.375f);

  if(mSelected)
    mBackground->SetColor(highlightColor);
  else
    mBackground->SetColor(Vec4(1, 1, 1, 0.05f));
}

//**************************************************************************************************
void Item::OnFocusGained(Event*)
{
  mFocusHighlight->SetActive(true);
  ObjectEvent e(this);
  DispatchEvent(Events::ItemSelected, &e);
}

//**************************************************************************************************
void Item::OnFocusLost(Event*)
{
  mFocusHighlight->SetActive(false);
}

//**************************************************************************************************
void Item::Select()
{
  mSelected = true;
  Vec4 highlightColor(0.191699997, 0.475143999, 0.709999979, 0.375f);
  mBackground->SetColor(highlightColor);
}

//**************************************************************************************************
void Item::DeSelect()
{
  mSelected = false;
  mBackground->SetColor(Vec4(1, 1, 1, 0.05f));
}

//**************************************************************************************************
void Item::UpdateTransform()
{
  mFocusHighlight->SetSize(mSize);
  WeightedComposite::UpdateTransform();
}

//--------------------------------------------------------------------------------- Item Grid Layout
//**************************************************************************************************
ItemGridLayout::ItemGridLayout(float itemHeight, uint columns)
  : Layout(Thickness::cZero)
  , mItemHeight(itemHeight)
  , mColumns(columns)
{

}

//**************************************************************************************************
Vec2 ItemGridLayout::Measure(Composite* widget, LayoutArea data)
{
  uint childCount = 0;
  forRange(Widget& child, widget->GetChildren())
    ++childCount;
  
  uint rows = (uint)Math::Ceil((float)childCount / (float)mColumns);

  Vec2 sizeNeeded;
  sizeNeeded.x = Pixels(10); // ??
  sizeNeeded.y = float(rows) * mItemHeight;
  // Account for spacing
  sizeNeeded.y += float(rows - 1) * Pixels(2);

  return sizeNeeded;
}

//**************************************************************************************************
Vec2 ItemGridLayout::DoLayout(Composite* widget, LayoutArea data)
{
  uint childIndex = 0;

  float itemWidth = data.Size.x / (float)mColumns;

  forRange(Widget& child, widget->GetChildren())
  {
    uint x = (childIndex % mColumns);
    uint y = (childIndex / mColumns);

    Vec3 translation(Vec3::cZero);
    translation.x = (float)x * (itemWidth + Pixels(2));
    translation.y = (float)y * (mItemHeight + Pixels(2));

    Vec2 size(itemWidth, mItemHeight);

    child.SetTranslationAndSize(translation, size);
    child.UpdateTransformExternal();

    ++childIndex;
  }

  return data.Size;
}

} // namespace Zero
