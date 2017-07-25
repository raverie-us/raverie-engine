///////////////////////////////////////////////////////////////////////////////
///
/// \file TreeView.hpp
/// Declaration of Tree View
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ScrollArea;
class TreeView;
class TextBox;
class MetaDropEvent;
class KeyboardEvent;
class ValueEditor;
class TreeView;
class TreeRow;
class Spacer;
class UpdateEvent;
class Label;
class ObjectPollEvent;
class MouseDragEvent;
class TextUpdatedEvent;

DeclareEnum3(ColumnType, 
             Fixed,     // The size will never change (cannot be resized)
             Flex,      // Percentage of the owners width
             Sizeable); // The size will remain unchanged unless 
                        // the user resizes it by dragging the header

/// Format for each Column in each Row
struct ColumnFormat
{
  ColumnFormat()
  {
    ColumnType = ColumnType::Flex;
    Index = 0;
    FlexSize = 1.0;
    MinWidth = Pixels(16.0f);
    Editable = false;
    StartX = 0.0f;
    FixedSize = Pixels(1, 1);
    CurrSize = Pixels(1, 1);
    Flags = 0;
  }

  /// Column Index
  int Index;

  // The first column that is not Fixed size will absorb the hierarchy
  ColumnType::Type ColumnType;

  /// Fixed size of the Column.
  Vec2 FixedSize;

  /// Various flags that are different for each control (for instance, does a text box accept double click for edit).
  u32 Flags;

  /// A relative weight of size (relative to other flex columns).
  /// If columnnA's flex size is 3 and columnB's flex size is 1,
  /// columnA will take up 75% (3 / (3 + 1)) and 
  /// columnB will take up 25% (1 / (3 + 1)).
  float FlexSize;

  /// When resizing columns, it will never go below this size (in pixels).
  float MinWidth;

  /// Name of the column.
  String Name;

  /// The name of the header may be different than the internal name.
  /// If this is left empty, the column will not be given a header.
  String HeaderName;
  String HeaderIcon;

  /// Data Base name of column used for 
  /// getting data.
  String Column;

  /// Is the column editable for this view?
  bool Editable;

  /// The value editor used (looked up in the ValueEditorFactory).
  /// If empty, the default editor is used.
  String CustomEditor;
  /// Data specific to the custom editor (for example, the type of resource
  /// to search for in the custom resource editor).
  Any CustomEditorData;

/// Runtime
  /// The position that this column starts at based on the size of the tree.  
  /// This does NOT include the offsets from indented rows.  That will
  /// be applied when each specific row updates its Transform.
  float StartX;

  /// Similar to 'StartX', but size instead of translation.
  Vec2 CurrSize;
};

DeclareBitField4(FormatFlags, ShowRoot, ShowHeaders, ShowSeparators, ColumnsResizable);

/// Formatting for Tree View
struct TreeFormatting
{
  BitField<FormatFlags::Enum> Flags;
  Array<ColumnFormat> Columns;
};

//Internal Node for InList
class TreeBase : public Composite
{
public:
  TreeBase(Composite* parent)
  :Composite(parent)
  {}
  Link<TreeBase> link;
};

namespace Events
{
  //Tree Element Right Clicked
  DeclareEvent(TreeRightClick);
  //Key pressed in tree
  DeclareEvent(TreeKeyPress);
  DeclareEvent(MouseEnterRow);
  DeclareEvent(MouseExitRow);
}

/// Tree Event Contains what row was affected.
class TreeEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  TreeRow* Row;
};

//--------------------------------------------------------------------- Tree Row
DeclareEnum3(HighlightType, None, Selected, Preview);

//Row in the TreeView.
class TreeRow : public TreeBase
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef InListBaseLink<TreeRow, TreeBase> TreeRowList;

  TreeRow(TreeView* grid, TreeRow* rowparent, DataEntry* entry);
  ~TreeRow();

  /// Compositions Interface
  void OnDestroy() override;

  /// Widget interface
  void UpdateTransform() override;
  void UpdateColumnsTransform(Vec2Param size);

  bool IsRoot();

  /// Operations
  /// Refresh Data on this Row
  void RefreshData();
  /// Rebuild Node and Children
  void Refresh();
  /// Expand this Row
  void Expand();
  /// Collapse this Row
  void Collapse();

  /// Edit Column By Index
  void Edit(int column);
  /// Edit Column By Name
  void Edit(StringParam column);
  /// Select this row
  void Select(bool singleSelect=false);
  /// Multi select from the current selection to this row
  void MultiSelect();

  void Highlight(HighlightType::Type type, InsertMode::Type mode = InsertMode::On);

  /// Destroy Row and Children
  void RecursiveDestroy();
  /// Remove Row from DataSource.
  void Remove();

  void UpdateBgColor(uint index);

  void Fill(Array<TreeRow*>&, uint depth);
//events
  void OnKeyPress(KeyboardEvent* event);
  void OnMouseDownExpander(MouseEvent* event);
  void OnMouseClick(MouseEvent* event);
  void OnMouseDragBackground(MouseDragEvent* event);
  void OnMouseDragRow(MouseEvent* event);
  void OnDoubleClick(MouseEvent* event);
  void OnMouseHold(MouseEvent* event);
  void OnRightUp(MouseEvent* event);
  void OnKeyUp(KeyboardEvent* event);
  void OnAddGroup(MouseEvent* event);
  void OnValueChanged(ObjectEvent* event);
  void OnTextChanged(TextUpdatedEvent* event);
  void GetInsertMode(Status& status, DataEntry* movingEntry, 
                     Vec2Param screenPos, InsertMode::Type& mode);
  InsertMode::Enum GetInsertPosition(DataEntry* entry, Vec2Param screenPos);
  void OnMetaDrop(MetaDropEvent* event);
  void OnObjectPoll(ObjectPollEvent* event);
  void OnMouseEnter(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);
  
  //Data
  void DestroyChildren();
  void RebuildChildren();
  DataIndex mIndex;
  TreeView* mTree;
  TreeRow* mParentRow;
  uint mDepth;
  // The index of visible rows (updated when new rows are created or deleted).
  uint mVisibleRowIndex;
  bool mExpanded;
  bool mActive;
  Element* mExpandIcon;
  Element* mSelection;
  Array<ValueEditor*> mEditorColumns;
  TreeRowList mChildren;
  Element* mSeparator;
  Element* mGraphicBackground;
  bool mValid;
  HandleOf<ToolTip> mToolTip;

  /// We place a background object so that we can drag other items
  /// onto the entire row.  Mouse Events will be bubbled up from this object
  Spacer* mBackground;
};

//---------------------------------------------------------------- Column Header
class ColumnHeader : public Composite
{
public:
  typedef ColumnHeader ZilchSelf;
  ColumnHeader(TreeView* tree, ColumnFormat* format);

  void SetText(StringParam name);
  void SetIcon(StringParam iconName);

  void UpdateTransform();

  /// Event response
  void OnMouseEnter(MouseEvent* e);
  void OnLeftClick(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);

  bool mSortFlip;
  Element* mBackground;
  Element* mIcon;
  Label* mLabel;
  ColumnFormat* mFormat;
  TreeView* mTree;
};

//--------------------------------------------------------------- Column Resizer
/// Placed in between headers
class ColumnResizer : public Composite
{
public:
  typedef ColumnResizer ZilchSelf;
  ColumnResizer(Composite* parent, ColumnFormat* left, ColumnFormat* right);

  void UpdateTransform() override;

  void ResizeHeaders(float pos);

  void OnMouseEnter(MouseEvent* event);
  void OnMouseDown(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);

  Spacer* mSpacer;
  ColumnFormat* mLeftFormat;
  ColumnFormat* mRightFormat;
};

//-------------------------------------------------------------------- Tree View
//Tree View display a Tree of Data. Each Row can have multiple columns. Used
//to display resources, file systems, object trees etc.
class TreeView : public Composite
{
public:
  typedef TreeView ZilchSelf;

  TreeView(Composite* parent);
  ~TreeView();

  void UpdateTransform() override;
  bool TakeFocusOverride() override;

  //Set the Data Source for this Tree View
  void SetDataSource(DataSource* source);
  DataSource* GetDataSource();

  //Set the selection to use for this Tree View
  void SetSelection(DataSelection* selection);
  DataSelection* GetSelection(){return mSelection;}

  // Allows modifications to which rows are expanded.
  DataSelection* GetExpanded(){return mExpanded;}

  //Full refresh of data tree.
  void Refresh();

  TreeRow* FindRowByIndex(DataIndex& index);
  uint FindRowIndex(TreeRow* row);
  void MoveToView(TreeRow* row);

  //Return the first row who's named column has a given value. (Linear)
  TreeRow* FindRowByColumnValue(StringParam column, StringParam value);

  void SelectFirstRow();

  // Ui Events
  void OnScrollUpdate(Object* object);
  void OnKeyDown(KeyboardEvent* event);
  void OnKeyRepeated(KeyboardEvent* event);
  void HandleKeyLogic(KeyboardEvent* event);
  void OnKeyUp(KeyboardEvent* event);
  void OnMouseExit(MouseEvent* event);
  void OnMouseExitHierarchy(MouseEvent* event);
  void OnMouseEnter(MouseEvent* event);
  void OnMouseUpdate(MouseEvent* event);
  void OnLeftClickBg(MouseEvent* event);
  void OnMouseDragBg(MouseDragEvent* event);
  void OnRightClickBg(MouseEvent* event);
  void OnMetaDropBg(MetaDropEvent* event);

  // Data Source Events
  void OnDataErased(DataEvent* event);
  void OnDataAdded(DataEvent* event);
  void OnDataModified(DataEvent* event);
  void OnDataReplaced(DataReplaceEvent* e);

  // Default formats
  void SetFormatName();
  void SetFormatNameAndType();
  void SetFormat(TreeFormatting& format);

  /// Sets the height of all rows.
  void SetRowHeight(float height);

  /// Expands all parent rows and scrolls to the row at the given index.
  void ShowRow(DataIndex& index);

  /// Determines if the tree will refresh whenever a value is changed. Set
  /// this to true if setting a property might alter the input value such
  /// that the display needs to be reset.
  void SetRefreshOnValueChange(bool state);

  void ClearAllRows();

  /// Used to forward keyboard events.
  ScrollArea* GetScrollArea();

//private:
  friend class RowSelector;
  friend class TreeRow;
  
  void DoFocus();

  ColumnFormat& GetColumn(StringParam name);

  /// Builds a range of the selected rows and sets the min 
  /// and max of the range to the given values.
  void GetSelectionRange(uint* minIndex, uint* maxIndex);

  /// Selects all rows in the range [min, max]
  void SelectRowsInRange(uint min, uint max);
  /// Selects all objects in the scene
  void SelectAll();

  /// Updates the transform of each column.
  void UpdateColumnTransforms();
  /// Creates / Updates the headers.
  void UpdateHeaders();
  /// Creates / Updates the separators.
  void UpdateSeparators();
  void UpdateColumnSeparators();
  float GetHeaderRowHeight();

  void OnMetaDropUpdate(MetaDropEvent* e);
  void DragScroll(Vec2Param screenPosition);

  /// Headers for each column
  Array<ColumnHeader*> mHeaders;
  HashMap<ColumnHeader*, ColumnResizer*> mHeaderResizers;

  /// Separators for each column
  Array<Element*> mColumnSeparators;

  TreeFormatting mFormatting;
  DataSource* mDataSource;
  DataSelection* mSelection;
  DataSelection* mExpanded;

  /// The index that the mouse is currently over while dragging objects.
  DataIndex mMouseOver;

  /// Whether or not the mouse is positioned before, on, or after the index.
  InsertMode::Type mMouseOverMode;

  /// The root node of the tree.
  TreeRow* mRoot;

  ScrollArea* mArea;
  bool mRefreshOnValueChange;

  /// The height of each row in the tree.
  float mRowHeight;

  /// Visible rows
  Array<TreeRow*> mRows;
  HashMap<u64, TreeRow*> mRowMap;
};

namespace TreeViewValidUi
{
DeclareTweakable(Vec4, PrimaryColor);
DeclareTweakable(Vec4, SecondaryColor);
}//namespace TreeViewUi

}//namespace Zero
