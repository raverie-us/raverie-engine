///////////////////////////////////////////////////////////////////////////////
///
/// \file TileView.hpp
/// Declaration of the TileView and supporting classes.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(ScrolledAllTheWayOut);
DeclareEvent(ScrolledAllTheWayIn);
DeclareEvent(TileViewRightClick);
}//namespace Events

const float cTileViewNameOffset = Pixels(19);

class TileViewWidget;
class TileView;

//-------------------------------------------------------------- Tile View Event
class TileViewEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  TileViewWidget* mTile;
};

//------------------------------------------------------------- Tile View Widget
class TileViewWidget : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TileViewWidget(Composite* parent, TileView* tileView,
                 PreviewWidget* tileWidget, DataIndex dataIndex);

  DataIndex GetDataIndex();

  //Widget Interface
  void UpdateTransform() override;

  //TileViewWidget Interface
  virtual void Refresh(){};
  virtual void OnDoubleClick(MouseEvent* event);
  virtual void AnimatePreview(PreviewAnimate::Enum value){mContent->AnimatePreview(value);}
  virtual Handle GetEditObject(){return mContent->GetEditObject();}

  //Event Handlers
  virtual void OnMouseEnter(MouseEvent* event);
  virtual void OnMouseClick(MouseEvent* event);
  virtual void OnMouseExit(MouseEvent* event);
  virtual void OnMouseHover(MouseEvent* event);
  virtual void OnMouseDrag(MouseEvent* event);
  virtual void OnMouseRightClick(MouseEvent* event);

  Thickness mContentMargins;
  String mItemType;
  String mName;
  Handle mObject;

protected:
  void Select(bool singleSelect);
  void MultiSelect();

  friend class TileView;
  Element* mBackground;
  Element* mTitleBar;
  Element* mHighlight;
  Label* mText;
  /// The contained widget.
  PreviewWidget* mContent;

  DataIndex mIndex;
  TileView* mTileView;
};

//-------------------------------------------------------------------- Tile View
/// TileView displays a tile widgets in a grid layout
class TileView : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TileView(Composite* parent);
  ~TileView();

  void SetDataSource(DataSource* source);

  /// Set the selection to use for this Tree View.
  void SetSelection(DataSelection* selection);
  DataSelection* GetSelection();

  /// Selects the first active tile.
  void SelectFirstTile();

  /// Percentage from [0-1].  0 being the smallest size, 1 being the largest.
  void SetItemSizePercent(float percentage);
  float GetItemSize();
  void  SetItemSize(float size);

  virtual TileViewWidget* CreateTileViewWidget(Composite* parent,
               StringParam name, HandleParam instance, DataIndex index,
               PreviewImportance::Enum minImportance = PreviewImportance::None);

  ScrollArea* GetScrollArea();

  /// Builds a range of the selected tiles and sets the min 
  /// and max of the range to the given values.
  void GetSelectionRange(uint* minIndex, uint* maxIndex);

  /// Widget Interface.
  void UpdateTransform() override;

private:
  friend class TileViewWidget;
  friend class TileSelector;
  
  void UpdateTileLayout();
  void UpdateVisibleTiles();

  /// Selects all rows in the range [min, max]
  void SelectTilesInRange(uint min, uint max);

  /// Event response.
  virtual void OnLeftClick(MouseEvent* e);
  virtual void OnLeftMouseDrag(MouseDragEvent* e);
  virtual void OnMouseScroll(MouseEvent* e);
  virtual void OnScrolled(Event* e);
  virtual void OnKeyDown(KeyboardEvent* e);

  void DragScroll(Vec2Param screenPosition);

  void ReloadData();
  void ClearTiles();
  void OnDataModified(Event*);
  
  TileLayout mLayout;
  DataSource* mDataSource;
  // The tile view does not own the selection
  DataSelection* mSelection;
  DataSelection* mDefaultSelection;
  Array<TileViewWidget*> mTileWidgets;
  float mItemSize;
  uint mTileCount;
  ScrollArea* mArea;
  Element* mBackground;
};

}//namespace Zero
