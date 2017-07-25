///////////////////////////////////////////////////////////////////////////////
///
/// \file ListControls.cpp
/// Implementation of the basic ListBox and ComboBox widget data controls.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(ItemSelected);
  DefineEvent(ItemDoubleClicked);
  DefineEvent(ListBoxOpened);
}

const float cTextCellHeight = Pixels(15);
const Thickness ComboBoxPadding = Thickness(2,2,2,2);
const Thickness ListBoxPadding = Thickness(1,1,1,1);

namespace ListControlsUi
{
const cstr cLocation = "EditorUi/ListControls";
Tweakable(Vec4, BackgroundColor,    Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, BorderColor,        Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, FocusBorderColor,   Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ListPrimaryColor,   Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ListSecondaryColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ListBackgroundColor, Vec4(1,1,1,1), cLocation);
}

const String cSelectionBox = "SelectionBox";
const String cSelectText = "SelectedText";
const String cHighlightedText = "TextHighlight";

//--------------------------------------------------------------------- List Box
ZilchDefineType(ListBox, builder, type)
{

}

ListBox::ListBox(Composite* parent)
  : Composite(parent)
{
  static const String className = "ListBox";
  mDefSet = mDefSet->GetDefinitionSet(className);

  mCustomBorderColor = false;
  mSelectedItem = cNoItemSelected;
  mHighlightItem = cNoItemSelected;
  mDataSource = nullptr;

  mDropShadow = CreateAttached<Element>(cDropShadow);
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mBackground = CreateAttached<Element>(cWhiteSquare);

  // To Prevent area from closing mBackground will take
  // hard focus
  mBackground->SetTakeFocusMode(FocusMode::Hard);

  mDropShadow->SetActive(false);
  mDropShadow->SetInteractive(false);

  mListArea = new ScrollArea(this);
  // Use list box background instead of scroll area background
  mListArea->GetBackground()->SetInteractive(false);

  mListArea->DisableScrollBar(SizeAxis::X);

  Composite* client = mListArea->GetClientWidget();
  mClient = client;

  ConnectThisTo(this, Events::ScrollUpdated, OnScrollUpdate);
  ConnectThisTo(client, Events::MouseMove, OnMouseMove);
  ConnectThisTo(client, Events::LeftClick, OnMouseClick);
  ConnectThisTo(client, Events::MouseExitHierarchy, OnMouseExit);
  ConnectThisTo(client, Events::MouseEnterHierarchy, OnMouseEnter);
  ConnectThisTo(client, Events::DoubleClick, OnDoubleClick);
  ConnectThisTo(this, Events::MouseScroll, OnMouseMove);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);

  mSelection = mClient->CreateAttached<Element>(cSelectionBox);
  mSelection->SetVisible(false);
  mSelection->SetInteractive(false);

  mHighlightBox = mClient->CreateAttached<Element>(cSelectionHighlight);
  mHighlightBox->SetVisible(false);
  mHighlightBox->SetInteractive(false);
  mMouseHover = false;
}

ListBox::~ListBox()
{

}

bool ListBox::TakeFocusOverride()
{
  this->HardTakeFocus();
  return true;
}

void ListBox::SetSelectedItem(int index, bool sendMessage)
{
  mSelectedItem = index;
  if(sendMessage)
  {
    ObjectEvent e(this);
    DispatchBubble(Events::ItemSelected, &e);
  }
  this->MarkAsNeedsUpdate();
}

Vec2 ListBox::GetSizeWithItems(float width, int itemCount)
{
  Vec2 lineSize = Vec2(width, cTextCellHeight);
  lineSize.y *= itemCount;
  return ExpandSizeByThickness(ListBoxPadding, lineSize);
}

void ListBox::HighlightItem(int index)
{
  mHighlightedItems.Insert(index);
}

void ListBox::SetDataSource(ListSource* source)
{
  mDataSource = source;
  ConnectThisTo(mDataSource, Events::DataModified, DataModified);
  ConnectThisTo(mDataSource, Events::DataDestroyed, DataDestroyed);
  this->MarkAsNeedsUpdate();
}

Vec2 ListBox::GetMinSize()
{
  // Return area for two items
  Vec2 basicSize = Vec2(40.0f, 2.0f * cTextCellHeight);
  return Math::Max(ExpandSizeByThickness(ListBoxPadding, basicSize), mMinSize);
}

void ListBox::UpdateTransform()
{
  Vec2 clientOffset = mListArea->GetClientOffset();
  Vec2 clientVisibleSize = mListArea->GetClientVisibleSize();

  Thickness borderThickness = ListBoxPadding;
  mBackground->SetColor(ListControlsUi::ListBackgroundColor);
  mBorder->SetSize(mSize);

  if(!mCustomBorderColor)
  {
    if(HasFocus())
      mBorder->SetColor(ListControlsUi::FocusBorderColor);
    else
      mBorder->SetColor(ListControlsUi::BorderColor);
  }

  float boxWidth = mSize.x;
  
  mSelection->SetSize(Vec2(boxWidth, cTextCellHeight));

  if(mListArea->IsScrollBarVisible(SizeAxis::Y))
    boxWidth -= mListArea->GetScrollBarSize();

  Rect listArea = RemoveThicknessRect(borderThickness, mSize);

  Vec2 subSize = listArea.GetSize();

  uint dataCount = 0;
  if(mDataSource)
    dataCount = mDataSource->GetCount();

  if(mSelectedItem > int(dataCount))
  {
    mSelection->SetVisible(false);
    mSelectedItem = cNoItemSelected;
  }

  if(mHighlightItem > int(dataCount))
  {
    mHighlightBox->SetVisible(false);
    mHighlightItem = cNoItemSelected;
  }

  //Compute the number of elements that can be 
  //seen at the controls current size
  uint possibleVisibleCount = 0;
  if(subSize.y > 0)
  {
    possibleVisibleCount = uint(subSize.y / cTextCellHeight) + 2;
  }

  //Add enough text blocks to display the visible elements
  while(mTextBlocks.Size() < possibleVisibleCount)
  {
    ItemEntry entry;
    entry.first = mClient->CreateAttached<Element>(cWhiteSquare);
    entry.first->MoveToBack();

    entry.second = new Text(mClient, cText);
    mTextBlocks.PushBack(entry);
  }

  //Remove extra text elements if necessary
  while(possibleVisibleCount < mTextBlocks.Size())
  {
    mTextBlocks.Back().first->Destroy();
    mTextBlocks.Back().second->Destroy();
    mTextBlocks.PopBack();
  }

  //Set the client size
  float listAreaSize = cTextCellHeight * float(dataCount);

  const float listAreaMinSize = Pixels(20);
  if(listAreaSize < listAreaMinSize)
    listAreaSize = listAreaMinSize;

  Vec2 clientSize = Vec2(boxWidth, listAreaSize);

  mListArea->SetClientSize(clientSize);

  float cellsOff = (-clientOffset.y) / cTextCellHeight;
  cellsOff = floor(cellsOff);

  for(uint i = 0; i < mTextBlocks.Size(); ++i)
  {
    Element* background = mTextBlocks[i].first;
    Text* text = mTextBlocks[i].second;

    Vec3 translation = Vec3(borderThickness.Left, 
                            cTextCellHeight * i + 
                            cellsOff * cTextCellHeight, 0);

    text->SetTranslation(translation);
    text->SetSize(Vec2(boxWidth, cTextCellHeight));

    background->SetTranslation(translation);
    background->SetSize(Vec2(boxWidth, cTextCellHeight));

    if(i % 2)
      background->SetColor(ListControlsUi::ListPrimaryColor);
    else
      background->SetColor(ListControlsUi::ListSecondaryColor);
  }

  if(mDataSource)
  {
    uint blockStart = uint(cellsOff);
    uint itemCount = mDataSource->GetCount();
    uint itemsToDisplay = mTextBlocks.Size();

    for(uint i = 0; i < itemsToDisplay; ++i)
    {
      Vec3 blockTrans = Vec3(0, cTextCellHeight * float(blockStart), 0);
      Vec2 blockSize = Vec2(boxWidth, cTextCellHeight);

      if(blockStart == mSelectedItem)
      {
        mSelection->SetVisible(true);
        mSelection->SetTranslation(blockTrans);
        mSelection->SetSize(blockSize);
      }

      if(blockStart == mHighlightItem && mMouseHover && 
         mHighlightItem < (int)dataCount)
      {
        mHighlightBox->SetVisible(true);
        mHighlightBox->SetTranslation(blockTrans);
        mHighlightBox->SetSize(blockSize);
      }

      TextDefinition* regular = (TextDefinition*)mDefSet->GetDefinition(cText);
      TextDefinition* selected = (TextDefinition*)mDefSet->GetDefinition(cSelectText);
      TextDefinition* highlighted = (TextDefinition*)mDefSet->GetDefinition(cHighlightedText);
      
      Text& text = *mTextBlocks[i].second;
      if(blockStart < itemCount)
      {
        text.SetActive(true);
        text.SetText(mDataSource->GetStringValueAt(blockStart));

        if(blockStart == mSelectedItem)
          text.ChangeDefinition(selected);
        else if(mHighlightedItems.Contains(i))
          text.ChangeDefinition(highlighted);
        else
          text.ChangeDefinition(regular);
      }
      else
      {
        text.SetActive(false);
      }

      ++blockStart;
    }
  }

  mDropShadow->SetSize(mSize);
  mDropShadow->SetTranslation(Pixels(6, 6, 0));

  PlaceWithRect(listArea, mListArea);
  PlaceWithRect(listArea, mBackground);

  Composite::UpdateTransform();
}

void ListBox::EnableDropShadow()
{
  mDropShadow->SetActive(true);
}

void ListBox::ScrollToView()
{
  if(mDataSource == nullptr)
    return;

  float numberOfItems = float(mDataSource->GetCount());

  Vec2 minSelect = Vec2(0, mSelectedItem * cTextCellHeight);
  Vec2 maxSelect =  minSelect + Vec2(0, cTextCellHeight);

  mListArea->ScrollAreaToView(minSelect, maxSelect);

  this->MarkAsNeedsUpdate();
  
  // Force update for visual issue
  this->UpdateTransformExternal();
}

ListSource* ListBox::GetDataSource()
{
  return mDataSource;
}

void ListBox::SetSelectionColor(Vec4Param color)
{
  mSelection->SetColor(color);
}

Vec4 ListBox::GetSelectionColor()
{
  return mSelection->GetColor();
}

void ListBox::SetBackgroundColor(Vec4Param color)
{
  mBackground->SetColor(color);
}

Vec4 ListBox::GetBackgroundColor()
{
  return mBackground->GetColor();
}

void ListBox::SetBorderColor(Vec4Param color)
{
  mCustomBorderColor = true;
  mBorder->SetColor(color);
}

Vec4 ListBox::GetBorderColor()
{
  return mBorder->GetColor();
}

void ListBox::OnScrollUpdate(ObjectEvent* object)
{
  this->MarkAsNeedsUpdate();
}

void ListBox::OnMouseMove(MouseEvent* event)
{
  if(mDataSource == nullptr)
    return;

  Vec2 localPosition = mClient->ToLocal(event->Position);
  int index = IndexFromPosition(localPosition);
  
  if(index < int(mDataSource->GetCount()))
    mHighlightItem = index;

  this->MarkAsNeedsUpdate();
}

void ListBox::OnMouseClick(MouseEvent* event)
{
  if(mDataSource == nullptr)
    return;
  
  Vec2 localPosition = mClient->ToLocal(event->Position);
  int index = IndexFromPosition(localPosition);
  if(index < int(mDataSource->GetCount()))
  {
    int dataIndex = IndexFromPosition(localPosition);
    SetSelectedItem(dataIndex, true);
  }
  this->MarkAsNeedsUpdate();
}

void ListBox::OnMouseExit(MouseEvent* event)
{
  mMouseHover = false;
  mHighlightBox->SetVisible(false);
}

void ListBox::OnMouseEnter(MouseEvent* event)
{ 
  mMouseHover = true;
}

void ListBox::OnDoubleClick(MouseEvent* event)
{
  Vec2 localPosition = mClient->ToLocal(event->Position);
  int index = IndexFromPosition(localPosition);

  if(mDataSource && index < int(mDataSource->GetCount()))
  {
    mDataSource->Selected(DataIndex(index));

    ObjectEvent e(this);
    DispatchBubble(Events::ItemDoubleClicked, &e);
  }
}

void ListBox::OnKeyDown(KeyboardEvent* event)
{

  switch(event->Key)
  {
  case Keys::Tab:
    TabJump(this, event);
    break;
  case Keys::Left:
  case Keys::Up:
    mSelectedItem -= 1;
    if(mSelectedItem < 0)
      mSelectedItem = mDataSource->GetCount() - 1;
    break;
  case Keys::Right:
  case Keys::Down:
    mSelectedItem += 1;
    mSelectedItem %= mDataSource->GetCount();
    break;
  case Keys::Enter:
    SetSelectedItem(mSelectedItem, true);
    break;
  case Keys::Escape:
    this->LoseFocus();
    break;
  }

  this->MarkAsNeedsUpdate();
}

void ListBox::DataModified(DataEvent* event)
{
  mSelection->SetVisible(false);
  mSelectedItem = cNoItemSelected;

  mHighlightBox->SetVisible(false);
  mHighlightItem = cNoItemSelected;

  this->MarkAsNeedsUpdate();
}

void ListBox::DataDestroyed(DataEvent* event)
{
  mDataSource = nullptr;
  this->Destroy();
}

int ListBox::IndexFromPosition(Vec2Param localPosition)
{
  float index = localPosition.y / cTextCellHeight;
  return int(index);
}

//-------------------------------------------------------------------- Combo Box
const String DownArrow = "DownArrowGrey";

ZilchDefineType(ComboBox, builder, type)
{

}

ComboBox::ComboBox(Composite* parent)
  :Composite(parent)
{
  static const String className = "ComboBox";
  mDataSource = nullptr;
  mDefSet = parent->GetDefinitionSet()->GetDefinitionSet(className);
  mSelectedItem = cNoItemSelected;
  SetClipping(true);

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackgroundColor = ToByteColor(ListControlsUi::BackgroundColor);
  mText = new Text(this, cText);
  mPullImage = CreateAttached<Element>(DownArrow);
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);

  Thickness borderThickness = ComboBoxPadding;
  mSize = ExpandSizeByThickness(borderThickness, mText->GetMinSize());
  mText->SetTranslation(Vec3(borderThickness.TopLeft()));

  ConnectThisTo(this, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(this, Events::FocusGained, OnFocusGained);
  ConnectThisTo(this, Events::FocusLost, OnFocusLost);

  mListBox = nullptr;
  mReady = true;
  mScrollToSelected = false;
}

ComboBox::~ComboBox()
{
  CloseList();
}

void ComboBox::SetInvalid()
{
  mText->SetText("-");
}

void ComboBox::SetListSource(ListSource* source)
{
  mDataSource = source;
}

void ComboBox::OpenList()
{
  ListBox* listBox = new ListBox(GetRootWidget()->GetPopUp());
  mListBox = listBox;

  ConnectThisTo(listBox, Events::FocusLostHierarchy, OnListFocusLost);
  ConnectThisTo(listBox, Events::FocusReset, OnListFocusReset);
  ConnectThisTo(listBox, Events::ItemSelected, OnItemSelected);

  listBox->TakeFocus();

  Vec2 sizeOfBox = Vec2(mSize.x - ListBoxPadding.Size().x, cTextCellHeight);
  if(mDataSource)
    sizeOfBox.y = float(mDataSource->GetCount()) * cTextCellHeight;;
  sizeOfBox = ExpandSizeByThickness(ListBoxPadding, sizeOfBox);

  const float cMaxListSize = cTextCellHeight * 12;
  if(sizeOfBox.y > cMaxListSize)
    sizeOfBox.y = cMaxListSize;

  Vec3 screenPosition = this->GetScreenPosition();
  Vec2 spaceSize = this->GetRootWidget()->GetSize();

  // If at the bottom of the scree flip the box the
  // other direction to prevent clipping
  if(screenPosition.y + sizeOfBox.y > spaceSize.y)
    screenPosition.y = screenPosition.y - sizeOfBox.y - mSize.y;

  listBox->SetTranslation(screenPosition + Vec3(0, mSize.y, 0));
  listBox->SetSize(sizeOfBox);
  listBox->SetDataSource(mDataSource);
  listBox->SetSelectedItem(mSelectedItem, false);
  listBox->EnableDropShadow();
  listBox->SetNotInLayout(true);

  if (mScrollToSelected)
  {
    listBox->UpdateTransformExternal();
    listBox->ScrollToView();
  }

  Event e;
  DispatchBubble(Events::ListBoxOpened, &e);

  this->mParent->MarkAsNeedsUpdate();
}

void ComboBox::CloseList()
{
  ListBox* listBox = mListBox;
  mListBox = nullptr;

  if(listBox)
  {
    listBox->Destroy();
    mReady = false;
  }

  this->MarkAsNeedsUpdate();
}

void ComboBox::OnMouseDown(MouseEvent* event)
{
  if(!mReady)
    return;
  if(mDataSource == nullptr)
    return;
  if(mListBox.IsNull())
    OpenList();
  else
    mListBox.SafeDestroy();
}

void ComboBox::OnItemSelected(ObjectEvent* event)
{
  ListBox* listBox = mListBox;
  if(listBox)
    SetSelectedItem(listBox->GetSelectedItem(), true);
  HardTakeFocus();
}

void ComboBox::OnListFocusLost(FocusEvent* event)
{
  if(ListBox* listBox = mListBox)
  {
    bool childOfList = !listBox->IsAncestorOf(event->ReceivedFocus);
    if(childOfList)
      CloseList();
  }
}

void ComboBox::OnListFocusReset(FocusEvent* event)
{
  CloseList();
}

void ComboBox::OnKeyDown(KeyboardEvent* event)
{
  TabJump(this, event);

  if(event->Handled)
    return;

  switch(event->Key)
  {
    case Keys::Left:
    case Keys::Right:
    case Keys::Up:
    case Keys::Down:
    case Keys::Enter:
      OpenList();
  }
}

void ComboBox::OnFocusGained(Event*)
{
  mBorder->SetColor(ListControlsUi::FocusBorderColor);
}

void ComboBox::OnFocusLost(Event*)
{
  mBorder->SetColor(ListControlsUi::BorderColor);
}

bool ComboBox::TakeFocusOverride()
{
  HardTakeFocus();
  return true;
}

Vec2 ComboBox::GetMinSize()
{
  Vec2 size = mText->GetMinSize();
  size.x += mPullImage->GetMinSize().x;
  return ExpandSizeByThickness(ComboBoxPadding, size);
}

void ComboBox::SetSelectedItem(int index, bool message)
{
  mSelectedItem = index;

  if(mSelectedItem != -1 && uint(index) < mDataSource->GetCount())
    mText->SetText(mDataSource->GetStringValueAt(DataIndex(mSelectedItem)));
  else
    mText->SetText(String());

  CloseList();

  if(message)
  {
    ObjectEvent e(this);
    DispatchBubble(Events::ItemSelected, &e);
  }
}

void ComboBox::SetText(StringParam text)
{
  mText->SetText(text);
}

void ComboBox::SetScrollToSelected(bool scroll)
{
  mScrollToSelected = scroll;
}

void ComboBox::UpdateTransform()
{
  mReady = true;
  mBackground->SetSize(mSize);
  mBorder->SetSize(mSize);

  if(HasFocus())
    mBorder->SetColor(ListControlsUi::FocusBorderColor);
  else
    mBorder->SetColor(ListControlsUi::BorderColor);

  Thickness borderThickness = ComboBoxPadding;
  Vec2 iconSize = mPullImage->GetSize();
  Vec2 newSize = mSize;

  mBackground->SetColor(ToFloatColor(mBackgroundColor));

  Rect textRect = RemoveThicknessRect(borderThickness, mSize);
  textRect.X += 4.0f;
  textRect.Y -= 1.0f;
  PlaceWithRect(textRect, mText);

  real rightBorder = 6.0f;
  mPullImage->SetTranslation(Vec3(newSize.x - iconSize.x - rightBorder,
                                  (newSize.y - iconSize.y) / 2.0f, 0.0f));
  Composite::UpdateTransform();
}

//------------------------------------------------------------- String Combo Box
ZilchDefineType(StringComboBox, builder, type)
{
}

//******************************************************************************
StringComboBox::StringComboBox(Composite* parent)
  : ComboBox(parent)
{
  mStrings = new StringSource();
  SetListSource(mStrings);
}

//******************************************************************************
StringComboBox::~StringComboBox()
{
  if (mStrings) delete mStrings;
}

//******************************************************************************
void StringComboBox::AddItem(StringParam string)
{
  mStrings->Strings.PushBack(string);
}

//******************************************************************************
void StringComboBox::RemoveItem(StringParam string)
{
  mStrings->Strings.EraseValueError(string);
}

//******************************************************************************
void StringComboBox::RemoveItem(uint index)
{
  mStrings->Strings.EraseAt(index);
}

//******************************************************************************
void StringComboBox::InsertItem(uint index, StringParam string)
{
  mStrings->Strings.InsertAt(index, string);
}

//******************************************************************************
void StringComboBox::ClearItems()
{
  mStrings->Strings.Clear();
}

//******************************************************************************
uint StringComboBox::GetIndexOfItem(StringParam string)
{
  return mStrings->Strings.FindIndex(string);
}

//******************************************************************************
StringParam StringComboBox::GetItem(uint index)
{
  return mStrings->Strings[index];
}

//******************************************************************************
uint StringComboBox::GetCount()
{
  return mStrings->GetCount();
}

//******************************************************************************
String StringComboBox::GetSelectedString()
{
  uint selectedIndex = GetSelectedItem();
  return GetItem(selectedIndex);
}

}//namespace Zero
