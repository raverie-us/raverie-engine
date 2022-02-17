// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(ScrolledAllTheWayOut);
DeclareEvent(ScrolledAllTheWayIn);
DeclareEvent(TileViewRightClick);
} // namespace Events

const float cTileViewNameOffset = Pixels(19);

class TileViewWidget;
class TileView;

class TileViewEvent : public Event
{
public:
  ZilchDeclareType(TileViewEvent, TypeCopyMode::ReferenceType);
  TileViewWidget* mTile;
};

class ItemPopUp : public PopUp
{
public:
  ItemPopUp(TileViewWidget* source, MouseEvent* mouseEvent);

  void UpdateTransform() override;

  Label* mName;
  Label* mType;
  Label* mExtra;
};

class TileViewWidget : public Composite
{
public:
  ZilchDeclareType(TileViewWidget, TypeCopyMode::ReferenceType);

  TileViewWidget(Composite* parent, TileView* tileView, PreviewWidget* tileWidget, DataIndex dataIndex);
  ~TileViewWidget();

  DataIndex GetDataIndex();

  // Widget Interface
  void UpdateTransform() override;

  // TileViewWidget Interface
  virtual void Refresh(){};
  virtual void OnRightUp(MouseEvent* event);
  virtual void OnDoubleClick(MouseEvent* event);
  virtual void AnimatePreview(PreviewAnimate::Enum value)
  {
    mContent->AnimatePreview(value);
  }
  virtual Handle GetEditObject()
  {
    return mContent->GetEditObject();
  }

  // Event Handlers
  virtual void OnMouseEnter(MouseEvent* event);
  virtual void OnMouseClick(MouseEvent* event);
  virtual void OnMouseExit(MouseEvent* event);
  virtual void OnMouseHover(MouseEvent* event);
  virtual void OnMouseDrag(MouseEvent* event);
  void OnValueChanged(ObjectEvent* event);
  void OnTextChanged(TextUpdatedEvent* event);

  void Edit();

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
  InPlaceTextEditor* mEditableText;
  /// The contained widget.
  PreviewWidget* mContent;

  DataIndex mIndex;
  TileView* mTileView;
  HandleOf<ItemPopUp> mTilePopUp;
};

/// TileView displays a tile widgets in a grid layout
class TileView : public Composite
{
public:
  ZilchDeclareType(TileView, TypeCopyMode::ReferenceType);

  TileView(Composite* parent);
  ~TileView();

  void SetDataSource(DataSource* source);

  /// Set the selection to use for this Tree View.
  void SetSelection(DataSelection* selection);
  DataSelection* GetSelection();

  /// Selects the first active tile.
  void SelectFirstTile();

  TileViewWidget* FindTileByIndex(DataIndex& index);

  /// Percentage from [0-1].  0 being the smallest size, 1 being the largest.
  void SetItemSizePercent(float percentage);
  float GetItemSize();
  void SetItemSize(float size);

  virtual TileViewWidget* CreateTileViewWidget(Composite* parent,
                                               StringParam name,
                                               HandleParam instance,
                                               DataIndex index,
                                               PreviewImportance::Enum minImportance = PreviewImportance::None);

  ScrollArea* GetScrollArea();

  /// Builds a range of the selected tiles and sets the min
  /// and max of the range to the given values.
  void GetSelectionRange(uint* minIndex, uint* maxIndex);

  /// Widget Interface.
  void UpdateTransform() override;

  /// Determines if the tile will refresh whenever a value is changed. Set
  /// this to true if setting a property might alter the input value such
  /// that the display needs to be reset.
  void SetRefreshOnValueChange(bool state);

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
  HashMap<u64, TileViewWidget*> mTileWidgetMap;
  float mItemSize;
  uint mTileCount;
  ScrollArea* mArea;
  bool mRefreshOnValueChange;
  Element* mBackground;
};

} // namespace Zero
