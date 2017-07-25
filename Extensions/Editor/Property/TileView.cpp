///////////////////////////////////////////////////////////////////////////////
///
/// \file TileView.cpp
/// Implementation of the TileView and supporting classes.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(TileViewWidget, builder, type)
{

}

ZilchDefineType(TileView, builder, type)
{

}

ZilchDefineType(TileViewEvent, builder, type)
{
}

namespace TileViewUi
{
const cstr cLocation = "EditorUi/TileView";
Tweakable(Vec4,  BorderColor,          Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  SelectedBorderColor,  Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  MouseOverBorderColor, Vec4(1,1,1,1), cLocation);
Tweakable(float, StartingTileSize,     100,           cLocation);
Tweakable(float, MinTileSize,          60,            cLocation);
Tweakable(float, MaxTileSize,          200,           cLocation);
Tweakable(float, TilePadding,          10,            cLocation);
Tweakable(float, MaxDragScrollSpeed,   Pixels(4),    cLocation);
Tweakable(float, DragScrollSize,       Pixels(30),    cLocation);
}

namespace Events
{
  DefineEvent(ScrolledAllTheWayOut);
  DefineEvent(ScrolledAllTheWayIn);
  DefineEvent(TileViewRightClick);
}//namespace Events

//---------------------------------------------------------------- Tile Selector
class TileSelector : public MouseManipulation
{
public:
  TileView* mTileView;
  Widget* mClientArea;

  /// Where the drag was started
  Vec2 mDragStart;

  /// Displays the drag selection
  Element* mSelectBox;

  //****************************************************************************
  TileSelector(MouseDragEvent* dragEvent, TileView* tree)
    : MouseManipulation(dragEvent->GetMouse(), tree), mTileView(tree)
  {
    mClientArea = tree->mArea->GetClientWidget();

    mDragStart = mClientArea->ToLocal(dragEvent->StartPosition);

    // Create a selection box on the scroll area
    mSelectBox = mClientArea->CreateAttached<Element>(cDragBox);
    mSelectBox->SetVisible(false);
  }

  //****************************************************************************
  void OnMouseUp(MouseEvent* event) override
  {
    mSelectBox->Destroy();
    mTileView->GetSelection()->SelectFinal();
    this->Destroy();
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* event) override
  {
    mSelectBox->SetVisible(true);
  }

  //****************************************************************************
  void OnMouseUpdate(MouseEvent* event) override
  {
    Vec2 local = mClientArea->ToLocal(event->Position);
    Vec2 min = Math::Min(local, mDragStart);
    Vec2 max = Math::Max(local, mDragStart);
    min.x = Math::Max(min.x, 0.0f);
    min.y = Math::Max(min.y, 0.0f);
    // Clamp to either the window size, or the client area size
    // Whichever is larger
    max.x = Math::Min(max.x, Math::Max(mClientArea->mSize.x, mTileView->mSize.x));
    max.y = Math::Min(max.y, Math::Max(mClientArea->mSize.y, mTileView->mSize.y));

    mSelectBox->SetSize(max - min);
    mSelectBox->SetTranslation(Vec3(min.x, min.y, 0));

    Array<int> overlappingTiles;
    Rect rect = Rect::MinAndMax(min, max);
    mTileView->mLayout.GetOverlappingTiles(rect, overlappingTiles);

    DataSelection* selection = mTileView->GetSelection();
    selection->SelectNone();
    for(uint i = 0; i < overlappingTiles.Size(); ++i)
      selection->Select(DataIndex((u64)overlappingTiles[i]));

    selection->SelectionModified();

    // Scroll if we're at the top or bottom of the tree view
    mTileView->DragScroll(event->Position);

    // Tiles will get created as we scroll, so keep pushing it to the front
    mSelectBox->MoveToFront();
  }

  //****************************************************************************
  void OnKeyDown(KeyboardEvent* event) override
  {
    if(event->Key == Keys::Escape)
    {
      this->Destroy();
      mSelectBox->Destroy();
    }
  }
};

//------------------------------------------------------------------ Item Pop Up
class ItemPopUp : public PopUp
{
public:
  Label* mName;
  Label* mType;
  Label* mExtra;

  //****************************************************************************
  ItemPopUp(TileViewWidget* source, MouseEvent* mouseEvent)
    : PopUp(source, PopUpCloseMode::MouseOutTarget)
  {
    SetBelowMouse(mouseEvent->GetMouse(), Pixels(10, 10));

    mName = new Label(this, cText);
    mType = new Label(this, cText);
    mExtra = new Label(this, cText);

    mName->SetText(BuildString("Name: ", source->mName));
    mName->SizeToContents();
    mName->SetInteractive(false);

    mType->SetText(BuildString("Type: ", source->mItemType));
    mType->SizeToContents();
    mType->SetInteractive(false);

    FadeIn();
  }

  //****************************************************************************
  void UpdateTransform()
  {
    mType->SetTranslation(Pixels(4, 4, 0));
    mName->SetTranslation(Pixels(4, 20, 0));
    mExtra->SetTranslation(Pixels(4, 36, 0));
    PopUp::UpdateTransform();
  }
};

//------------------------------------------------------------------ Item Widget
//******************************************************************************
TileViewWidget::TileViewWidget(Composite* parent, TileView* tileView, 
                               PreviewWidget* tileWidget, DataIndex dataIndex)
  : Composite(parent), mTileView(tileView)
{
  mDefSet = parent->GetDefinitionSet()->GetDefinitionSet("ItemGrid");
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mTitleBar = CreateAttached<Element>(cWhiteSquare);
  mHighlight = CreateAttached<Element>(cWhiteSquare);

  mBackground->SetColor(Vec4(0.416f, 0.416f, 0.416f, 1));
  mTitleBar->SetColor(Vec4(0.35f, 0.35f, 0.35f, 1));
  mHighlight->SetColor(Vec4(TileViewUi::MouseOverBorderColor) * Vec4(1,1,1,0.6));

  mText = new Label(this, cText);

  mObject = tileWidget->mObject;
  mName = tileWidget->mName;
  mIndex = dataIndex;

  mContentMargins = Thickness(1, 1, 1, 1);

  //
  if(tileWidget->mObject.IsNotNull())
    mItemType = tileWidget->mObject.GetBoundOrIndirectType()->ToString();

  //SetClipping(true);
  mText->SetTextClipping(true);
  mText->SetText(mName);

  AttachChildWidget(tileWidget);
  mContent = tileWidget;

  ConnectThisTo(this, Events::MouseEnterHierarchy, OnMouseEnter);
  ConnectThisTo(this, Events::LeftClick, OnMouseClick);
  ConnectThisTo(this, Events::MouseExitHierarchy, OnMouseExit);
  ConnectThisTo(this, Events::MouseHover, OnMouseHover);
  ConnectThisTo(this, Events::LeftMouseDrag, OnMouseDrag);
  ConnectThisTo(this, Events::DoubleClick, OnDoubleClick);
  ConnectThisTo(this, Events::RightClick, OnMouseRightClick);
}

//******************************************************************************
DataIndex TileViewWidget::GetDataIndex()
{
  return mIndex;
}

//******************************************************************************
void TileViewWidget::UpdateTransform()
{
  //mText->MoveToFront();

  float titleBarHeight = cTileViewNameOffset;
  if (mText->GetText().Empty())
    titleBarHeight = 0.0f;

  Vec2 backgroundSize = mSize;
  backgroundSize.y -= titleBarHeight;

  Vec2 titleBarSize = Vec2(mSize.x, titleBarHeight);

  mHighlight->SetActive(false);
  if(IsMouseOver())
  {
    mHighlight->SetColor((Vec4)TileViewUi::MouseOverBorderColor * Vec4(1, 1, 1, 0.6f));
    mHighlight->SetActive(true);
  }
  else if(mTileView->mSelection->IsSelected(mIndex))
  {
    mHighlight->SetColor((Vec4)TileViewUi::SelectedBorderColor * Vec4(1, 1, 1, 0.6f));
    mHighlight->SetActive(true);
  }

  mBackground->SetSize(mSize);
  mTitleBar->SetSize(titleBarSize);
  mHighlight->SetSize(mSize);

  // Center content
  Rect contentRect = Rect::PointAndSize(Vec2(0, titleBarHeight), backgroundSize);
  contentRect.RemoveThickness(mContentMargins);
  PlaceWithRect(contentRect, mContent);

  // Center text to the title bar
  mText->SizeToContents();
  Rect textRect = Rect::PointAndSize(Vec2::cZero, titleBarSize);
  PlaceCenterToRect(textRect, mText);

  mText->mTranslation.x = Math::Max(mText->mTranslation.x, 0.0f);
  mText->mSize.x = Math::Min(mText->mSize.x, mSize.x);

  Composite::UpdateTransform();
}

//******************************************************************************
void TileViewWidget::OnMouseHover(MouseEvent* event)
{
  ItemPopUp* popUp = new ItemPopUp(this, event);
  popUp->SetBelowMouse(event->GetMouse(), Pixels(2,2));
}

//******************************************************************************
void TileViewWidget::OnMouseEnter(MouseEvent* event)
{
  UpdateTransform();
}

//******************************************************************************
void TileViewWidget::OnMouseClick(MouseEvent* event)
{
  if(event->Handled)
    return;

  if(event->ShiftPressed)
    MultiSelect();
  else if(event->CtrlPressed)
    Select(false);
  else 
    Select(true);
}

//******************************************************************************
void TileViewWidget::OnMouseExit(MouseEvent* event)
{
  UpdateTransform();
}

//******************************************************************************
void TileViewWidget::OnDoubleClick(MouseEvent* event)
{
  if(!event->Handled && event->Button == MouseButtons::Left)
  {
    DataEvent e;
    e.Index = mIndex;
    mTileView->mDataSource->DispatchEvent(Events::DataActivated, &e);
  }
}

//******************************************************************************
void TileViewWidget::OnMouseDrag(MouseEvent* event)
{
  new MetaDrag(event->GetMouse(), this->GetRootWidget(), mObject);
}

//******************************************************************************
void TileViewWidget::OnMouseRightClick(MouseEvent* event)
{
  TileViewEvent eventToSend;
  eventToSend.mTile = this;
  mTileView->GetDispatcher()->Dispatch(Events::TileViewRightClick, &eventToSend);
}

//******************************************************************************
void TileViewWidget::Select(bool singleSelect)
{
  DataSelection* selection = mTileView->mSelection;
  bool wasSelected = selection->IsSelected(mIndex);

  // Always clear when it's a single select
  if(singleSelect)
    selection->SelectNone();

  // Select this row
  selection->Select(mIndex);

  // Only send the event if this was not already selected
  if(!wasSelected)
  {
    //Send out selected event
    DataEvent dataEvent;
    dataEvent.Index = mIndex;
    mTileView->mDataSource->DispatchEvent(Events::DataSelected, &dataEvent);
  }
  else
  {
    if(!singleSelect)
      selection->Deselect(mIndex);
  }

  MarkAsNeedsUpdate();
  selection->SelectFinal();
}

//******************************************************************************
void TileViewWidget::MultiSelect()
{
  DataSelection* selection = mTileView->mSelection;
  if (selection->mSupportsMultiSelect == false)
    return;

  uint rowIndex = (uint)mIndex.Id;

  // If the selection size is 0, select from the root to this row
  if(selection->Size() == 0)
  {
    mTileView->SelectTilesInRange(0, rowIndex);
  }
  else
  {
    // If there are items selected, build a range of the selected indices
    uint minIndex, maxIndex;
    mTileView->GetSelectionRange(&minIndex, &maxIndex);

    // We want to take into account this row as well
    minIndex = Math::Min(minIndex, rowIndex);
    maxIndex = Math::Max(maxIndex, rowIndex);

    // Select all in the range
    mTileView->SelectTilesInRange(minIndex, maxIndex);
  }

  selection->SelectFinal();
}

//-------------------------------------------------------------------- Item Grid
//******************************************************************************
TileView::TileView(Composite* parent)
  : Composite(parent)
{
  mDefSet = parent->GetDefinitionSet()->GetDefinitionSet("ItemGrid");
  mArea = new ScrollArea(this);
  mArea->SetClientSize(Pixels(10, 10));
  mArea->DisableScrollBar(0);
  mTileCount = 0;
  mDataSource = NULL;
  mSelection = mDefaultSelection = new HashDataSelection();
  mItemSize = TileViewUi::StartingTileSize;

  ConnectThisTo(mArea->GetBackground(), Events::LeftClick, OnLeftClick);
  ConnectThisTo(mArea->GetBackground(), Events::LeftMouseDrag, OnLeftMouseDrag);
  ConnectThisTo(this, Events::MouseScroll, OnMouseScroll);
  ConnectThisTo(mArea, Events::ScrollUpdated, OnScrolled);
  ConnectThisTo(mArea->GetClientWidget(), Events::KeyDown, OnKeyDown);
  ConnectThisTo(mArea->GetClientWidget(), Events::KeyRepeated, OnKeyDown);
}

//******************************************************************************
TileView::~TileView()
{
  SafeDelete(mDefaultSelection);
}

//******************************************************************************
void TileView::SetDataSource(DataSource* source)
{
  if(mDataSource)
    DisconnectAll(mDataSource, this);
  mDataSource = source;
  ConnectThisTo(mDataSource, Events::DataModified, OnDataModified);
  ReloadData();
}

//******************************************************************************
void TileView::SetSelection(DataSelection* selection)
{
  mSelection = selection;
}

//******************************************************************************
DataSelection* TileView::GetSelection()
{
  return mSelection;
}

//******************************************************************************
void TileView::SelectFirstTile()
{
  if(mTileWidgets.Size() == 0)
    return;

  if(TileViewWidget* tile = mTileWidgets.Front())
  {
    mSelection->SelectNone();
    mSelection->Select(tile->mIndex);
    tile->MarkAsNeedsUpdate();
    mSelection->SelectFinal();
  }
}

//******************************************************************************
void TileView::SetItemSizePercent(float percentage)
{
  mItemSize = TileViewUi::MinTileSize + (TileViewUi::MaxTileSize - TileViewUi::MinTileSize) * percentage;
}

//******************************************************************************
float TileView::GetItemSize()
{
  return mItemSize;
}

//******************************************************************************
void TileView::SetItemSize(float size)
{
  mItemSize = size;
}

//******************************************************************************
ScrollArea* TileView::GetScrollArea()
{
  return mArea;
}

//******************************************************************************
void TileView::GetSelectionRange(uint* minIndex, uint* maxIndex)
{
  *minIndex = (uint)-1;
  *maxIndex = 0;

  // Get all selected
  Array<DataIndex> selected;
  mSelection->GetSelected(selected);

  // Walk through the selected and min / max
  for(uint i = 0; i < selected.Size(); ++i)
  {
    uint currIndex = (uint)selected[i].Id;

    // Min / max
    *minIndex = Math::Min(currIndex, *minIndex);
    *maxIndex = Math::Max(currIndex, *maxIndex);
  }
}

//******************************************************************************
TileViewWidget* TileView::CreateTileViewWidget(Composite* parent, StringParam name,
                                               HandleParam instance, DataIndex index,
                                               PreviewImportance::Enum minImportance)
{
  PreviewWidget* tileWidget = ResourcePreview::CreatePreviewWidget(parent, name,
                                                      instance, minImportance);
  if(tileWidget == NULL)
    return NULL;

  return new TileViewWidget(parent, this, tileWidget, index);
}

//******************************************************************************
void TileView::UpdateTransform()
{
  mArea->SetSize(mSize);

  if(GetTransformUpdateState() == TransformUpdateState::ChildUpdate || mDataSource == NULL)
  {
    Composite::UpdateTransform();
    return;
  }

  UpdateTileLayout();
  UpdateVisibleTiles();

  Composite::UpdateTransform();
}

//******************************************************************************
void TileView::UpdateTileLayout()
{
  DataEntry* root = mDataSource->GetRoot();
  uint childCount = mDataSource->ChildCount(root);

  float ScrollAreaSize = mArea->GetScrollBarSize();
  float NamedPadding = Pixels(12);
  float TilePadding = TileViewUi::TilePadding;

  Vec2 sizeForTiles = mSize;

  // Remove scroll area size
  sizeForTiles.x -= ScrollAreaSize;

  // Add Pixels to y size for name area so the image is squared
  Vec2 itemSize = Vec2(mItemSize, mItemSize) + Vec2(0, NamedPadding);

  // Setup tiled layout
  mLayout = TileLayout(itemSize, sizeForTiles, childCount, TilePadding);
}

//******************************************************************************
void TileView::UpdateVisibleTiles()
{
  DataEntry* root = mDataSource->GetRoot();
  uint childCount = mDataSource->ChildCount(root);

  // Update client area with number of tiles computed
  // Do not expand on y because tiles are stacked vertically
  Vec2 clientArea = mLayout.GetSizeNeeded();
  mArea->SetClientSize(clientArea);
  Vec2 clientVisibleArea = mArea->GetClientVisibleSize();

  // Compute the first visible area
  Vec2 clientOffset = mArea->GetClientOffset();
  float topY = -clientOffset.y;
  float bottomY = -clientOffset.y + clientVisibleArea.y;

  IntVec2 visibleIndex = mLayout.GetFirstAndLastVisible(topY, bottomY);
  Array<LayoutResult> resultsArray;

  mTileWidgets.Resize(childCount, NULL);

  DataEntry* prev = NULL;
  for(uint i = 0; i < childCount; ++i)
  {
    DataEntry* entry = mDataSource->GetChild(root, i, prev);
    DataIndex index = mDataSource->ToIndex(entry);

    TileViewWidget* itemWidget = mTileWidgets[i];

    // Is this tile visible?
    bool tileVisible = int(i) >= visibleIndex.x && int(i) <= visibleIndex.y;

    if(tileVisible)
    {
      Handle object = mDataSource->ToHandle(entry);

      if(itemWidget == NULL)
      {
        Any nameVar;
        mDataSource->GetData(entry, nameVar, CommonColumns::Name);
        itemWidget = CreateTileViewWidget(mArea, nameVar.Get<String>(), object, 
                                          index, PreviewImportance::None);
        itemWidget->AnimatePreview( PreviewAnimate::MouseOver );
        mTileWidgets[i] = itemWidget;
      }

      LayoutResult result = mLayout.ComputeTileLayout(i);
      result.PlacedWidget = itemWidget;
      resultsArray.PushBack(result);
    }
    else
    {
      // Tile is not visible destroy it
      if(itemWidget)
        itemWidget->Destroy();

      mTileWidgets[i] = NULL;
    }

    prev = entry;
  }

  AnimateLayout(resultsArray, false);
}

//******************************************************************************
void TileView::SelectTilesInRange(uint min, uint max)
{
  if(mDataSource == NULL)
    return;

  // Clear the selection
  mSelection->SelectNone();

  // Select all in the given range
  for(uint i = min; i <= max && i < mTileWidgets.Size(); ++i)
  {
    if(TileViewWidget* tile = mTileWidgets[i])
      mSelection->Select(tile->mIndex, false);
  }

  mSelection->SelectionModified();
  MarkAsNeedsUpdate();
}

//******************************************************************************
void TileView::OnLeftClick(MouseEvent* e)
{
  mSelection->SelectNone();
  mSelection->SelectFinal();
  MarkAsNeedsUpdate();
}

//******************************************************************************
void TileView::OnLeftMouseDrag(MouseDragEvent* e)
{
  if(mSelection && mSelection->mSupportsMultiSelect)
    new TileSelector(e, this);
}

//******************************************************************************
void TileView::OnMouseScroll(MouseEvent* event)
{
  if(event->CtrlPressed)
  {
    float newSize = mItemSize + event->Scroll.y * 2;

    if(newSize <= TileViewUi::MinTileSize)
    {
      mItemSize = TileViewUi::MinTileSize;
      Event event;
      GetDispatcher()->Dispatch(Events::ScrolledAllTheWayOut, &event);
    }
    else if(newSize >= TileViewUi::MaxTileSize)
    {
      mItemSize = TileViewUi::MaxTileSize;
      Event event;
      GetDispatcher()->Dispatch(Events::ScrolledAllTheWayIn, &event);
    }
    else
    {
      mItemSize = newSize;
    }

    this->MarkAsNeedsUpdate();
  }
}

//******************************************************************************
void TileView::OnScrolled(Event* event)
{
  this->MarkAsNeedsUpdate();
}

//******************************************************************************
void TileView::OnKeyDown(KeyboardEvent* e)
{
  if(e->Handled)
    return;

  // Select all
  if(e->Key == Keys::A && e->CtrlPressed)
  {
    DataEntry* root = mDataSource->GetRoot();
    uint childCount = mDataSource->ChildCount(root);
    mSelection->SelectNone();
    DataEntry* prev = NULL;
    for(uint i = 0; i < childCount; ++i)
    {
      DataEntry* entry = mDataSource->GetChild(root, i, prev);
      DataIndex index = mDataSource->ToIndex(entry);
      mSelection->Select(index);
      prev = entry;
    }
    mSelection->SelectFinal();
    MarkAsNeedsUpdate();
    return;
  }
  else if(e->Key == Keys::Enter)
  {
    Array<DataIndex> selected;
    mSelection->GetSelected(selected);

    forRange(DataIndex index, selected.All())
    {
      DataEvent e;
      e.Index = index;
      mDataSource->DispatchEvent(Events::DataActivated, &e);
    }

    return;
  }

  uint minIndex, maxIndex;
  GetSelectionRange(&minIndex, &maxIndex);

  if(e->Key == Keys::Up || e->Key == Keys::Left)
  {
    IntVec2 direction(0, -1);
    if(e->Key == Keys::Left)
      direction = IntVec2(-1, 0);

    // Move upwards
    int newMin = mLayout.GetTileInDirection((int)minIndex, direction);

    if(newMin >= 0)
    {
      // Scroll to make the new visible selection
      LayoutResult result = mLayout.ComputeTileLayout(newMin);
      Vec2 scrollPos(0, result.Translation.y);
      mArea->ScrollAreaToView(scrollPos, scrollPos);

      // Tiles may not exist yet, so rebuild visible tiles
      UpdateVisibleTiles();

      // Now shift the selection
      if(e->ShiftPressed)
        SelectTilesInRange(newMin, maxIndex);
      else
        SelectTilesInRange(newMin, newMin);

      mSelection->SelectFinal();
    }
  }
  else if(e->Key == Keys::Down || e->Key == Keys::Right)
  {
    IntVec2 direction(0, 1);
    if(e->Key == Keys::Right)
      direction = IntVec2(1, 0);

    // Move down
    int newMax = mLayout.GetTileInDirection((int)maxIndex, direction);

    if(newMax < (int)mTileWidgets.Size())
    {
      // Scroll to make the new visible selection
      LayoutResult result = mLayout.ComputeTileLayout(newMax);
      Vec2 scrollPos(0, result.Translation.y + result.Size.y);
      mArea->ScrollAreaToView(scrollPos, scrollPos);

      // Tiles may not exist yet, so rebuild visible tiles
      UpdateVisibleTiles();

      // Now shift the selection
      if(e->ShiftPressed)
        SelectTilesInRange(minIndex, newMax);
      else
        SelectTilesInRange(newMax, newMax);

      mSelection->SelectFinal();
    }
  }

  MarkAsNeedsUpdate();
}

//******************************************************************************
void TileView::DragScroll(Vec2Param screenPosition)
{
  Vec2 local = mArea->ToLocal(screenPosition);

  float scrollSpeed = 0;
  float maxSpeed = TileViewUi::MaxDragScrollSpeed;
  if(mArea->IsScrollBarVisible(1))
  {
    if(local.y < TileViewUi::DragScrollSize)
    {
      float percentage = 1.0f - (local.y / TileViewUi::DragScrollSize);
      scrollSpeed = -percentage * maxSpeed;
    }
    else if(local.y > (mSize.y - TileViewUi::DragScrollSize))
    {
      float percentage = 1.0f - (mSize.y - local.y) / TileViewUi::DragScrollSize;
      scrollSpeed = percentage * maxSpeed;
    }
  }

  scrollSpeed = Math::Clamp(scrollSpeed, -maxSpeed, maxSpeed);
  mArea->ScrollPixels(Vec2(0, -scrollSpeed));
}

//******************************************************************************
void TileView::ReloadData()
{
  ClearTiles();

  if(mDataSource == NULL)
    return;

  DataEntry* root = mDataSource->GetRoot();
  mTileCount = mDataSource->ChildCount(root);
}

//******************************************************************************
void TileView::ClearTiles()
{
  forRange(TileViewWidget* widget, mTileWidgets.All())
  {
    if(widget)
      widget->Destroy();
  }

  mTileWidgets.Clear();

  MarkAsNeedsUpdate();
}

//******************************************************************************
void TileView::OnDataModified(Event*)
{
  ReloadData();
}

}//namespace Zero
