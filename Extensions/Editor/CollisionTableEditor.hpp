///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class CollisionTable;
struct CollisionFilter;
class CollisionGroup;
class CollisionTableEditor;

//-------------------------------------------------------------------CollisionTableItem

/// Stores what two groups this filter is between. This is how a user selects
/// what filter to edit in the property grid. This also displays visually
/// the current collision state of the filter.
class CollisionMatrixItem : public Composite
{
public:
  typedef CollisionMatrixItem ZilchSelf;
  CollisionMatrixItem(Composite* parent, CollisionTableEditor* tableEditor,
                      CollisionFilter* filter, CollisionFilter& searchFilter);

  /// Update the internal elements size when the transform is update.
  void UpdateTransform() override;

  /// Change to the currently selected item in the property grid.
  void OnMouseUp(MouseEvent* event);
  /// Toggle the collision state.
  void OnRightMouseUp(MouseEvent* event);
  /// Display a popup for the current item.
  void OnMouseEnter(MouseEvent* e);
  /// Displays the popup describing the current item.
  void DisplayPopup();
  /// Update the visual display of the item.
  void UpdateDisplay();

  /// The table that is editing us. Used to get the current resource and modify it.
  CollisionTableEditor* mTableEditor;
  /// The filter we represent. May be null if there is no filter.
  CollisionFilter* mFilter;
  /// The filter we are meant to represent. Used to create the filter
  /// if we need to modify it and one didn't already exist.
  CollisionFilter mSearchFilter;
  
  /// The background of the item.
  Element* mBackground;
  
  /// Text to display the current state of the filter. Might replace
  /// with a icon later, don't have a good icon to use at the moment.
  Text* mTextLabel;
  /// Icon to display the current state of the filter. This is currently set invisible,
  /// but all the logic is there so this will be easy to experiment with later.
  Element* mIcon;

  /// The padding used to offset items from the border.
  static Vec2 mPadding;
};

//-------------------------------------------------------------------CollisionTableLabel

/// A label for a row in the collision group. This needs to store what group it's
/// associated with so right click context menus can know what is being affected.
class CollisionGroupLabel : public Composite
{
public:
  typedef CollisionGroupLabel ZilchSelf;
  CollisionGroupLabel(Composite* parent, CollisionTableEditor* tableEditor, CollisionGroup* group, bool vertical = false);

  /// Update the internal elements size when the transform is update.
  void UpdateTransform() override;
  /// Used to create a context menu.
  void OnRightMouseUp(Event* e);

  void OnSetSkipDetect(Event* event);
  void OnSetSkipResolve(Event* event);
  void OnSetResolve(Event* event);
  void UpdateGroupState(uint flags);
  void OnRemoveGroup(Event* event);

  /// The display text box.
  Text* mText;
  /// Pointer back to the editor. Used to grab the actual table and modify it.
  CollisionTableEditor* mTableEditor;
  /// The group we represent. This allows us to know who we are
  /// modifying during right click context menus.
  HandleOf<CollisionGroup> mGroup;
};

//-------------------------------------------------------------------CollisionTableMatrix

/// The matrix display used to edit a collision table.
class CollisionTableMatrix : public Composite
{
public:
  typedef CollisionTableMatrix ZilchSelf;
  CollisionTableMatrix(Composite* parent, CollisionTableEditor* tableEditor);

  // public interface

  /// Sets the new table that this matrix is editing
  void SetTable(CollisionTableEditor* tableEditor);
  /// Refresh the table (the resource we're editing was modified somehow)
  void Refresh();
  /// Returns the size needed to display everything
  Vec2 GetMinSize();


  // should be private interface

  /// Creates all of the labels (on the left for now) for each group.
  /// This returns the x size taken up by the labels.
  void CreateLabels(Array<CollisionGroup*>& groups, float height, float buffer, float& xStart, float& yStart);
  /// Creates the matrix to edit all of the items in the table.
  void CreateMatrix(Array<CollisionGroup*>& groups, float xStart, float yStart, Vec2Param size, float buffer);
  /// Fills out the StringSource for what groups are addable by the user.
  void BuildGroupListing();
  /// Registers a new group when the user selects it in the combo box.
  void OnRegisterNewGroup(ObjectEvent* event);
  

  /// The editor for us. Used to get the resource that is being edited.
  CollisionTableEditor* mTableEditor;
  /// The data source for the combobox that stores what groups are available to be added.
  StringSource mAvailableGroups;
  /// Allows the user to register new groups to the table.
  ComboBox* mAddableGroupSelector;
  /// The minimum required size to display all of our elements.
  Vec2 mMinSize;
  /// The right click context menu for the labels. Stored here since there needs
  /// to be only one for all labels and this creates the labels.
  HandleOf<ContextMenu> mContextMenuHandle;
};

//-------------------------------------------------------------------CollisionTableEditor

/// The main editor for collision tables. This has two main ui pieces: the
/// collision table matrix and the property grid for editing items in the matrix.
class CollisionTableEditor : public Composite
{
public:
  typedef CollisionTableEditor ZilchSelf;
  CollisionTableEditor(Composite* parent, CollisionTable* table);

  /// Sets what filter to display in the property grid.
  void SetEditingFilter(CollisionFilter* filter);
  /// Helper to add a new filter to the current table for the given resource types.
  CollisionFilter* AddFilter(ResourceId id1, ResourceId id2);
  /// Listen for when the property grid is changed.
  void OnPropertyChanged(PropertyEvent* propEvent);

  // Helpers to refresh the table
  void RefreshAll();
  void RefreshPropertyView();
  void RefreshMatrix();

  /// Updates the scrollable area for the matrix to be the min required size.
  void UpdateScrollArea();

  /// Just a quick helper. Marks the current collision table resource as modified.
  void MarkModified();

  // Callbacks to know if collision groups were changed in any way
  void OnCollisionGroupAdded(ResourceEvent* e);
  void OnCollisionGroupModified(ResourceEvent* e);
  void OnCollisionGroupRemoved(ResourceEvent* e);

  /// Callback for if the table we're editing was deleted
  void OnCollisionTableRemoved(ResourceEvent* e);

  /// The resource being modified.
  HandleOf<CollisionTable> mTable;
  /// The matrix used to display items in the table.
  CollisionTableMatrix* mMatrix;
  /// Used to display an item in the matrix.
  PropertyView* mPropertyView;
  /// Displays the name of the current filter being edited in the property view.
  Label* mCurrentFilterLabel;
  /// The current filter that is being edited in the property view.
  CollisionFilter* mEditingFilter;
  /// The scrollable area that the matrix is put in so that the user can always
  /// see all their table when they resize this window.
  ScrollArea* mScrollArea;
};

}//namespace Zero
