///////////////////////////////////////////////////////////////////////////////
///
/// \file ResourceTableEditor.hpp
/// Declaration of the SearcahableResourceTextBox, WeightedTableBar,
/// ResourceWeightedTableView ResourceTableTreeView and
/// ResourceTableEditor classes.
/// 
/// Authors: Joshua Davis
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ComboBox;
class DataEvent;
class GraphWidget;
class IconButton;
class PropertyView;
class ScrollArea;
class SearchViewEvent;
class TextBox;
class TextButton;
class TreeView;
class Window;
class MetaDropEvent;
class ContextMenu;
class ResourceTableEditor;
class ResourceTableSource;
class ResourceWeightedTableView;

//-------------------------------------------------------------------ResourceTableEntryChangeOp
class ResourceTableEntryChangeOp : public Operation
{
public:
  ResourceTableEntryChangeOp( ) { mName = "ResourceTableEntryChange"; };
  ResourceTableEntryChangeOp(ResourceTableEditor* editor, ResourceTableEntry* oldEntry, const ResourceTable::ValueType& newValue);
  ResourceTableEntryChangeOp(ResourceTableEditor* editor, ResourceTableEntry* oldEntry);
  void Undo() override;
  void Redo() override;

  ResourceTableEntry mStartEntry;
  ResourceTableEntry mEndEntry;
  uint mIndex;
  ResourceTableEditor* mEditor;
};

//-------------------------------------------------------------------ResourceTableMaxWeightChangeOp
class ResourceTableMaxWeightChangeOp : public Operation
{
public:
  ResourceTableMaxWeightChangeOp(ResourceTableEditor* editor, float newMaxWeight);

  void Undo() override;
  void Redo() override;

  float mOldMaxWeight;
  float mNewMaxWeight;

  ResourceTableEditor* mEditor;
  Array<float> mOldWeights;
};

//-------------------------------------------------------------------ResourceTableResourceTypeChangeOp
class ResourceTableResourceTypeChangeOp : public Operation
{
public:
  ResourceTableResourceTypeChangeOp(ResourceTableEditor* editor, StringParam newType);
  void Undo() override;
  void Redo() override;

  void PerformOp(StringParam resourceType);

  String mOldResourceType;
  String mNewResourceType;
  ResourceTableEditor* mEditor;
};

//-------------------------------------------------------------------ResourceTableAddRemoveRowOp
class ResourceTableAddRemoveRowOp : public Operation
{
public:
  ResourceTableAddRemoveRowOp(ResourceTableEditor* editor, ResourceTableEntry* entry, bool add);
  void Undo() override;
  void Redo() override;

  void PerformOp(bool add);

  ResourceTableEditor* mEditor;
  uint mIndex;
  ResourceTableEntry mEntry;
  bool mAdd;
};

//-------------------------------------------------------------------ResourceTableRowReorderOp
class ResourceTableRowReorderOp : public Operation
{
public:
  ResourceTableRowReorderOp(ResourceTableEditor* editor, uint oldIndex, uint newIndex);
  void Undo() override;
  void Redo() override;

  void PerformOp(uint currentIndex, uint newIndex);

  ResourceTableEditor* mEditor;
  uint mOldIndex;
  uint mNewIndex;
};

//-------------------------------------------------------------------ResourceTableBatchRowReorderOp
class ResourceTableBatchRowReorderOp : public Operation
{
public:
  ResourceTableBatchRowReorderOp(ResourceTableEditor* editor, Array<int>& oldIndices, Array<int>& newIndices);
  void Undo() override;
  void Redo() override;

  void PerformOp(Array<int>& currentIndices, Array<int>& newIndices);

  ResourceTableEditor* mEditor;
  Array<int> mOldIndices;
  Array<int> mNewIndices;
  uint mOldIndex;
  uint mNewIndex;
};

//-------------------------------------------------------------------SearchableResourceTextBox

/// A "text box" that allows searching of resource types. This wraps itself up
/// to still pretend (slightly) that it's a text box, however when edited it
/// will make a search view so the user can pick from the resources.
class SearchableResourceTextBox : public Composite
{
public:
  typedef SearchableResourceTextBox ZilchSelf;

  SearchableResourceTextBox(Composite* parent, StringParam resourceType, StringParam resourceIdName);

  void UpdateTransform();

  void OnMouseDown(MouseEvent* mouseEvent);
  void OnSearchCompleted(SearchViewEvent* e);
  Vec2 GetMinSize();

  void SetTextOffset(float offset);

  /// The resource guid/name of the selected resource type (different than
  /// what's in the display text box as that shows a user friendly name)
  String mResourceIdName;
  /// User friendly display name of the resource
  TextBox* mDisplayTextBox;
  /// The type of resource being search
  String mResourceType;
  /// Any already open search views we've created
  HandleOf<FloatingSearchView> mActiveSearch;
};

//-------------------------------------------------------------------WeightedTableBar

/// Bar composite to display the weight of an item. Also allows clicking
/// and dragging to update the current probability.
class WeightedTableBar : public Composite
{
public:
  typedef WeightedTableBar ZilchSelf;

  WeightedTableBar(Composite* parent, ResourceWeightedTableView* weightedTableView);

  void UpdateTransform() override;

  void UpdateProbability(float prob, bool queueUndo);
  void UpdateProbability(MouseEvent* mouseEvent, bool queueUndo);
  void OnLeftMouseDown(MouseEvent* mouseEvent);
  void OnRightMouseUp(MouseEvent* mouseEvent);
  void OnProbabilityChanged(Event*);
  void OnNameChanged(Event*);
  void OnValueChanged(Event*);
  void OnDuplicate(Event*);
  void OnRemove(Event*);

  ResourceWeightedTableView* mWeightedTableView;
  Element* mBackground;
  uint mIndex;
};

//-------------------------------------------------------------------WeightedTableBarDragManipulation
class WeightedTableBarDragManipulation : public MouseManipulation
{
public:
  WeightedTableBarDragManipulation(MouseEvent* e, Composite* parent, WeightedTableBar* bar, ResourceTableEditor* editor);

  void OnMouseMove(MouseEvent* e) override;
  void OnMouseUp(MouseEvent* e) override;

  ResourceTableEditor* mEditor;
  WeightedTableBar* mBar;
  float mStartProbability;
};


//-------------------------------------------------------------------ResourceWeightedTableView

/// Weighted table view for the resource table. Used primarily to set relative
/// weights and get a good idea of how probable one item is over another.
class ResourceWeightedTableView : public Composite
{
public:
  typedef ResourceWeightedTableView ZilchSelf;

  ResourceWeightedTableView(Composite* parent, ResourceTableEditor* editor);

  void Setup(ResourceTable* table);
  /// Clears all of the child widgets and rebuilds the table. Used to
  //easily "refresh" the entire view when the underlying resource has changed.
  void Rebuild();
  /// Creates the resource specific widgets (bars, labels, etc...)
  void SetTable(ResourceTable* table);
  
  /// Position and size the sub items
  void UpdateTransform() override;

  /// Core functions for creating/removing/duplicating items
  void CreateItem(StringParam name, StringParam value, float probability);
  void DuplicateItem(uint index);
  void RemoveItem(uint index);
  void UpdateProbability(WeightedTableBar* item, float prob, bool queueUndo);

  void OnMouseDown(MouseEvent* mouseEvent);
  void OnRightMouseUp(MouseEvent* mouseEvent);
  void OnMaxWeightChanged(Event*);
  void OnAddItem(Event*);

  void FadeOutContextMenu();
  void FixValuesAfterExpansion(bool queueUndo);
  float GetMaxWeight();
  void SetMaxWeight(float maxHeight, bool queueUndo);

  Vec2 GetMinSize();
  Vec2 GetMinSize(float& width);

  /// A sub-struct to organize widgets for each
  /// item in the ResourceTable together.
  struct Entry
  {
    Entry()
    {
      mBar = NULL;
      mProbability = NULL;
      mNameTag = NULL;
      mValueTag = NULL;
    }
    
    WeightedTableBar* mBar;
    TextBox* mProbability;
    TextBox* mNameTag;
    SearchableResourceTextBox* mValueTag;
  };

  ResourceTable* mTable;
  ResourceTableEditor* mEditor;

  typedef Array<Entry> EntryList;
  EntryList mEntries;
  
  /// A scroll area that all of the children widgets are contained within.
  ScrollArea* mScrollArea;
  GraphWidget* mGraph;
  /// The max weight value (height of a bar) allowed in the graph.
  TextBox* mMaxWeightTextBox;
  HandleOf<ContextMenu> mContextMenuHandle;

  // Tweakables

  /// Controls the minimum width of a bar.
  float mMinBarWidth;
  /// Controls the number of decimal places we round
  /// the probability to during dragging.
  int mRoundingPlaces;

  // Margins of the graph (hardcoded margins for now)
  Thickness mMargins;
};

//-------------------------------------------------------------------ResourceTableTreeView

/// The tree view editor for resource tables. Allows compactly
/// viewing and editing of all the items in the tree view.
class ResourceTableTreeView : public Composite
{
public:
  typedef ResourceTableTreeView ZilchSelf;

  ResourceTableTreeView(Composite* parent, ResourceTableEditor* editor);
  ~ResourceTableTreeView();

  void Rebuild();
  void SetFormatting(StringParam resourceType);

  void GetSelection(Array<ResourceTableEntry*>& entries);
  void OnAddContextMenu(MouseEvent* e);
  void OnAddRow(ObjectEvent* e);
  void OnRemoveRow(ObjectEvent* e);
  void OnMetaDrop(MetaDropEvent* e);
  void OnKeyDown(KeyboardEvent* e);

  void RemoveSelectedRows();
  void FadeOutContextMenu();

  /// The actual resource being edited.
  HandleOf<ResourceTable> mTable;
  
  TreeView* mTreeView;
  ResourceTableSource* mSource;
  ResourceTableEditor* mEditor;

  HandleOf<ContextMenu> mContextMenuHandle;
};

//-------------------------------------------------------------------ResourceTableEditor

/// The main editor for resource tables. Manages switching between
/// the tree view and the weighted table view.
class ResourceTableEditor : public Composite
{
public:
  typedef ResourceTableEditor ZilchSelf;

  ResourceTableEditor(Composite* parent, ResourceTable* table);
  ~ResourceTableEditor();

  void SetTable(ResourceTable* table);
  void CreateToolbar();
  void Rebuild();

  void OnKeyDown(KeyboardEvent* e);

  /// The swap view button was pressed, swap between tree and weight view
  void OnSwapView(Event*);

  Vec2 GetMinSize() override;

  void AddRow(MouseEvent* e);
  void OnResourceTypeSelected(Event* e);
  void OnMaxWeightChanged(Event* e);
  void OnResourceRemoved(ResourceEvent* e);
  void OnMouseDown(MouseEvent* mouseEvent);

  /// Sets the max weight on the table while properly queuing up undo operations
  void SetMaxWeightWithUndo(float maxWeight, bool queueUndo);

  /// When the resource type has changed (or resources have been removed),
  /// all the resources need to be iterated through to see if they are valid for
  /// the new type. Aka, fix bad old values to the default of the new type.
  void RemapResources();

  /// The actual resource we're editing.
  HandleOf<ResourceTable> mTable;
  ResourceTableTreeView* mTreeView;
  ResourceWeightedTableView* mWeightedView;

  /// Used to swap between tree and weight view
  TextButton* mSwapViewButton;

  /// Data source for the types of resources that can be contained.
  StringSource mResourceTypeSource;
  /// The actual composite used to select the type of resource being contained.
  ComboBox* mResourceTypeSelector;
  /// Button used to add more rows into the tree view. Adding is not natively
  /// supported for a user, so this button allows users that control.
  TextButton* mAddButton;
  /// Composite to allow the user to set the max weight value for the table.
  TextBox* mMaxWeightTextBox;

  OperationQueue mQueue;
};

}//namespace Zero
