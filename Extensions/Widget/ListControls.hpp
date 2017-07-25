///////////////////////////////////////////////////////////////////////////////
///
/// \file ListControls.hpp
/// Declaration of the basic ListBox and ComboBox widget data controls.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class ScrollArea;
class KeyboardEvent;

namespace Events
{
  DeclareEvent(ItemSelected);
  DeclareEvent(ItemDoubleClicked);
  DeclareEvent(ListBoxOpened);
}

const int cNoItemSelected = -1;

//--------------------------------------------------------------------- List Box
///List box widget class.
class ListBox : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ListBox(Composite* parent);
  ~ListBox();

  bool TakeFocusOverride() override;
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  void SetSelectedItem(int index, bool message);
  int GetSelectedItem(){return mSelectedItem;}
  Vec2 GetSizeWithItems(float width, int itemCount);

  int GetHighlightItem( ) { return mHighlightItem; }

  uint GetCount(){return mDataSource->GetCount();}
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

  bool mCustomBorderColor;

private: 
  //Events
  void OnScrollUpdate(ObjectEvent* object);
  void OnMouseMove(MouseEvent* event);
  void OnMouseClick(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);
  void OnMouseEnter(MouseEvent* event);
  void OnDoubleClick(MouseEvent* event);
  void DataModified(DataEvent* event);
  void DataDestroyed(DataEvent* event);
  void OnKeyDown(KeyboardEvent* event);

  int IndexFromPosition(Vec2Param localPosition);
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

//-------------------------------------------------------------------- Combo Box
class ComboBox : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

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

  bool IsOpen(){ return mListBox.IsNotNull(); }

  void SetSelectedItem(int index, bool message);
  int GetSelectedItem( ) { return mSelectedItem; }

  void OnMouseDown(MouseEvent* event);
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

};

//------------------------------------------------------------- String Combo Box
class StringComboBox : public ComboBox
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
 
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

}//namespace Zero
