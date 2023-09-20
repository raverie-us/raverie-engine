// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
class ScrollArea;
class KeyboardEvent;

namespace Events
{
DeclareEvent(ItemSelected);
DeclareEvent(ItemDoubleClicked);
DeclareEvent(ListBoxOpened);
DeclareEvent(ListEntriesChanged);
} // namespace Events

const int cNoItemSelected = -1;

/// List box widget class.
class ListBox : public Composite
{
public:
  RaverieDeclareType(ListBox, TypeCopyMode::ReferenceType);

  ListBox(Composite* parent);
  ~ListBox();

  bool TakeFocusOverride() override;
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  void SetSelectedItem(int index, bool message);
  int GetSelectedItem()
  {
    return mSelectedItem;
  }
  Vec2 GetSizeWithItems(float width, int itemCount);

  uint GetCount()
  {
    return mDataSource->GetCount();
  }
  void HighlightItem(int index);
  void SetDataSource(ListSource* source);
  void EnableDropShadow();
  void ScrollToView();
  ListSource* GetDataSource();

  void SetSelectionColor(Vec4Param color);
  Vec4 GetSelectionColor();
  void SetBackgroundColor(Vec4Param color);
  Vec4 GetBackgroundColor();
  void SetBorderColor(Vec4Param color);
  Vec4 GetBorderColor();

  // Events
  void OnScrollUpdate(ObjectEvent* object);
  void OnMouseMove(MouseEvent* event);
  void OnMouseClick(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);
  void OnMouseEnter(MouseEvent* event);
  void OnMouseEnterItem(MouseEvent* event);
  void OnDoubleClick(MouseEvent* event);
  void DataModified(DataEvent* event);
  void DataDestroyed(DataEvent* event);
  void OnKeyDown(KeyboardEvent* event);

  int IndexFromPosition(Vec2Param localPosition);

  bool mCustomBorderColor;

  HandleOf<ToolTip> mToolTip;

  ListSource* mDataSource;
  Element* mBackground;
  Element* mBorder;
  Element* mDropShadow;
  Element* mSelection;
  Element* mHighlightBox;
  bool mMouseHover;

  typedef Pair<Element*, Text*> ItemEntry;
  Array<ItemEntry> mTextBlocks;
  HashSet<uint> mHighlightedItems;
  ScrollArea* mListArea;
  Composite* mClient;
  int mSelectedItem;
  int mHighlightItem;
};

class ComboBox : public Composite
{
public:
  RaverieDeclareType(ComboBox, TypeCopyMode::ReferenceType);

  ComboBox(Composite* parent);
  ~ComboBox();

  bool TakeFocusOverride() override;
  Vec2 GetMinSize() override;
  void UpdateTransform() override;

  void SetInvalid();
  void SetListSource(ListSource* source);
  void OpenList();
  void CloseList();
  void SetText(StringParam text);
  void SetScrollToSelected(bool scroll);

  bool IsOpen()
  {
    return mListBox.IsNotNull();
  }

  void SetSelectedItem(int index, bool message);
  int GetSelectedItem()
  {
    return mSelectedItem;
  }

  /// Is the the drop down selectable?
  void SetSelectable(bool selectable);

  void OnMouseDown(MouseEvent* event);
  void OnMouseEnter(MouseEvent* event);
  void OnItemSelected(ObjectEvent* event);
  void OnListFocusLost(FocusEvent* event);
  void OnListFocusReset(FocusEvent* event);
  void OnKeyDown(KeyboardEvent* event);

  void OnDataModified(DataEvent* event);
  void OnDataDestroyed(DataEvent* event);

  void OnFocusGained(Event*);
  void OnFocusLost(Event*);

  ByteColor mBackgroundColor;

  bool mReady;
  Text* mText;
  Element* mBackground;
  Element* mPullImage;
  Element* mBorder;
  HandleOf<ListBox> mListBox;
  ListSource* mDataSource;
  int mSelectedItem;
  bool mScrollToSelected;
  bool mAllowSelect;
};

class StringComboBox : public ComboBox
{
public:
  RaverieDeclareType(StringComboBox, TypeCopyMode::ReferenceType);

  StringComboBox(Composite* parent);
  ~StringComboBox();

  void AddItem(StringParam string);
  void RemoveItem(StringParam string);
  void RemoveItem(uint index);
  void InsertItem(uint index, StringParam string);
  void ClearItems();
  uint GetIndexOfItem(StringParam string);
  StringParam GetItem(uint index);
  uint GetCount();

  String GetSelectedString();

private:
  StringSource* mStrings;
};

} // namespace Raverie
