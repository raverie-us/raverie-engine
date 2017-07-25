///////////////////////////////////////////////////////////////////////////////
///
/// \file LibraryView.hpp
/// Declaration of the LibraryView composite.
/// 
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class TagChainTextBox;
class TreeView;
class TileView;
class IconButton;
class ResourceTagEditor;

class ContentLibrary;
class ResourceLibrary;
class LibraryDataSource;

class TreeEvent;
class TreeRow;
class DataEvent;
class KeyboardEvent;
class MessageBoxEvent;
class TagEvent;
class PreviewWidgetGroup;
class LibraryView;
struct SelectionChangedEvent;

//------------------------------------------------------------ Library Tile View
class LibraryTileView : public TileView
{
public:
  LibraryTileView(LibraryView* parent);

  /// TileView interface.
  TileViewWidget* CreateTileViewWidget(Composite* parent, StringParam name,
                                HandleParam instance, DataIndex index,
                                PreviewImportance::Enum minImportance) override;

  LibraryView* mLibraryView;
};

//----------------------------------------------------------------- Library View
class LibraryView : public Composite
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  LibraryView(Composite* parent);
  ~LibraryView();

  /// Widget interface.
  void UpdateTransform() override;

  /// Views the given content library.
  void View(ContentLibrary* contentLibrary, ResourceLibrary* resourceLibrary);

  /// Views the library at the 0th index (when no project is available)
  void View();

  /// Changes the display to the tree view.
  void SwitchToTreeView();

  /// Changes the display to the tile view.
  void SwitchToTileView();

  /// Sets the current search to the given tags.
  void SetSearchTags(HashSet<String>& tags);

  /// Creates a preview group of the given tag with the current search as
  /// extra tags.
  PreviewWidgetGroup* CreatePreviewGroup(Composite* parent, StringParam tag,
                                         uint max);

  void AddHiddenLibrary(StringParam libraryName);

private:
  void UpdateVisibleResources();

  void BuildContentLibraryList();
  void OnPackageBuilt(ContentSystemEvent* e);
  void OnContentLibrarySelected(Event* e);

  /// Resource event response.
  void OnResourcesModified(ResourceEvent* event);

  /// Tree event response.
  void OnTreeRightClick(TreeEvent* event);
  void OnTileViewRightClick(TileViewEvent* event);
  void OnRightClickObject(Composite* objectToAttachTo, DataIndex index);
  void OnKeyDown(KeyboardEvent* event);
  void OnMouseEnterTreeRow(TreeEvent* event);
  void OnMouseExitTreeRow(TreeEvent* event);
  void CreateResourceToolTip(Resource* resource, TreeRow* row);
  void CreateTagToolTip(StringParam tagName, TreeRow* row);

  /// Data Source event response.
  void OnDataActivated(DataEvent* event);
  void OnDataSelectionModified(ObjectEvent* event);
  void OnDataSelectionFinal(ObjectEvent* event);
  void OnEditorSelectionChanged(SelectionChangedEvent* event);
  void SelectAll();
  
  /// Context menu event response.
  void OnRemove(ObjectEvent* event);
  void OnRename(ObjectEvent* event);
  void OnEdit(ObjectEvent* event);
  void OnEditMeta(ObjectEvent* event);
  void OnEditTags(ObjectEvent* event);
  void OnMessageBox(MessageBoxEvent* event);
  void OnDuplicate(Event* event);
  /// Extra context menus for zilch fragment translation. These should eventually
  /// be moved to some external registration once it is possible.
  void OnComposeZilchMaterial(Event* event);
  void OnTranslateZilchPixelMaterial(Event* event);
  void OnTranslateZilchGeometryMaterial(Event* event);
  void OnTranslateZilchVertexMaterial(Event* event);
  void OnTranslateFragment(Event* event);

  void OnAddTagToSearch(ObjectEvent* event);

  /// Editor event response.
  void OnToggleViewButtonPressed(Event* e);
  void OnSearchDataModified(Event* e);
  void OnSearchKeyDown(KeyboardEvent* e);
  void OnSearchKeyPreview(KeyboardEvent* e);
  void OnSearchKeyRepeated(KeyboardEvent* e);
  void HandleSearchKeyLogic(KeyboardEvent* e);
  void OnMouseScroll(MouseEvent* e);
  void OnTilesScrolledAllTheWayOut(Event* e);
  void OnProjectLoaded(Event* e);

  /// Tag editor event response.
  void OnTagEditorModified(Event* e);
  void OnTagEditorClose(MouseEvent* e);
  void OnTagEditorCloseHover(MouseEvent* e);

  float GetTagEditorHeight();
  void SetTagEditorHeight(float height);

private:
  /// Tag editor functions.
  bool TagEditorIsOpen();
  void EditTags(DataSelection* dataSelection);
  void OpenTagEditor();
  void CloseTagEditor();

  /// Used to hide 
  HashSet<String> mHiddenLibraries;
  StringComboBox* mContentLibraries;
  Composite* mLibrariesRow;

  SearchData* mSearch;
  HandleOf<ToolTip> mResourcePreview;

  ContentLibrary* mContentLibrary;
  ResourceLibrary* mResourceLibrary;
  bool mIgnoreEditorSelectionChange;

  /// Represents the objects in the tree and the selection.
  LibraryDataSource* mSource;
  HashDataSelection* mDataSelection;

  /// The index of the row that was clicked on.
  DataIndex mPrimaryCommandIndex;
  /// All indices in the selection, including the primary command index.
  Array<DataIndex> mCommandIndices;

  /// Button to switch between the tree view and grid view.
  ToggleIconButton* mToggleViewButton;

  TagChainTextBox* mSearchBox;

  Composite* mActiveView;
  TreeView* mTreeView;
  LibraryTileView* mTileView;

  float mTagEditorHeight;
  Element* mTagEditorCloseButton;
  ResourceTagEditor* mTagEditor;
};

}// namespace Zero
