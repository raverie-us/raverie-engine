///////////////////////////////////////////////////////////////////////////////
///
/// \file LibraryView.cpp
/// Implementation of the LibraryView composite.
/// 
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------ LibraryDataSource
const String cTagIcon = "Tag";

struct LibDataEntry
{
  LibDataEntry() : mResource(nullptr) {}
  Resource* mResource;
  String mTag;
};

// Helpers for Sprite Frame
inline bool LibDataEntrySort(LibDataEntry* left, LibDataEntry* right)
{
  return left->mTag < right->mTag;
}

class LibraryDataSource : public DataSource
{
public:
  //****************************************************************************
  LibraryDataSource()
  {
    mRoot.mTag = "Root";
  }

  ~LibraryDataSource()
  {
    Clear();
  }

  LibDataEntry mRoot;
  Array<LibDataEntry*> mEntries;

  //****************************************************************************
  void AddResource(Resource* resource)
  {
    LibDataEntry* entry = new LibDataEntry();
    entry->mResource = resource;
    mEntries.PushBack(entry);
  }

  //****************************************************************************
  void AddTag(StringParam tag)
  {
    LibDataEntry* entry = new LibDataEntry();
    entry->mTag = tag;
    mEntries.PushBack(entry);
  }

  //****************************************************************************
  void SortByTagName()
  {
    Zero::Sort(mEntries.All(), LibDataEntrySort);
  }

  //****************************************************************************
  void Clear()
  {
    DeleteObjectsInContainer(mEntries);
  }

  //****************************************************************************
  DataEntry* GetRoot() override
  {
    return &mRoot;
  }

  //****************************************************************************
  DataEntry* ToEntry(DataIndex index) override
  {
    if(index == cRootIndex || (uint)index.Id >= mEntries.Size())
      return &mRoot;
    return mEntries[(uint)index.Id];
  }

  //****************************************************************************
  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    if(dataEntry == &mRoot)
      return cRootIndex;
    LibDataEntry* entry = (LibDataEntry*)dataEntry;
    uint index = mEntries.FindIndex(entry);
    return DataIndex(index);
  }

  //****************************************************************************
  DataIndex GetResourceIndex(Resource* resource)
  {
    for(uint i = 0; i < mEntries.Size(); ++i)
    {
      if(resource == mEntries[i]->mResource)
        return DataIndex(i);
    }

    return DataIndex((u64)-1);
  }

  //****************************************************************************
  Handle ToHandle(DataEntry* dataEntry) override
  {
    LibDataEntry* entry = (LibDataEntry*)dataEntry;
    return entry->mResource;
  }
  
  //****************************************************************************
  DataEntry* Parent(DataEntry* dataEntry) override
  {
    if(dataEntry == &mRoot)
      return nullptr;
    return &mRoot;
  }

  //****************************************************************************
  uint ChildCount(DataEntry* dataEntry) override
  {
    if(dataEntry == &mRoot)
      return mEntries.Size();
    return 0;
  }

  //****************************************************************************
  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    if(dataEntry == &mRoot)
      return mEntries[index];
    return nullptr;
  }
  
  //****************************************************************************
  bool IsExpandable(DataEntry* dataEntry) override
  {
    return false;
  }

  //****************************************************************************
  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    LibDataEntry* entry = (LibDataEntry*)dataEntry;

    // Resource entry
    if(entry->mResource)
    {
      Resource* resource = entry->mResource;
      BoundType* resourceType = ZilchVirtualTypeId(resource);
      if(column == CommonColumns::Name)
        variant = resource->Name;
      else if(column == CommonColumns::Type)
        variant = resourceType->Name;
      else if(column == CommonColumns::Icon)
        variant = String("ResourceIcon");
    }
    // Icon entry
    else
    {
      if(column == CommonColumns::Name)
        variant = entry->mTag;
      else if(column == CommonColumns::Type)
        variant = cTagIcon;
      else if(column == CommonColumns::Icon)
        variant = cTagIcon;
    }
  }

  //****************************************************************************
  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    LibDataEntry* entry = (LibDataEntry*)dataEntry;
    Resource* resource = entry->mResource;

    if(resource == nullptr)
      return false;

    if(!column.Empty())
    {
      if(column == CommonColumns::Name)
      {
        String newName = variant.Get<String>();
        Status status;
        if(IsValidName(newName, status))
          return RenameResource(resource, newName);
        else
          DoNotifyWarning("Invalid resource name.",status.Message);
      }
    }

    return false;
  }
};

class TagTileWidget : public TileViewWidget
{
public:
  TagTileWidget(Composite* parent, TileView* tileView,
                PreviewWidget* tileWidget, DataIndex dataIndex)
    : TileViewWidget(parent, tileView, tileWidget, dataIndex)
  {
    // Create the tag icon
    mTagIcon = CreateAttached<Element>(cTagIcon);
  }

  /// Widget Interface.
  void UpdateTransform() override
  {
    TileViewWidget::UpdateTransform();
    //mTagIcon->SetTranslation(Pixels(2, 2, 0));
    
    float tagWidth = mTagIcon->mSize.x;

    // Place the tag to the left of the text
    Vec3 tagTranslation(mText->mTranslation.x - tagWidth - Pixels(2), Pixels(2), 0);

    // The amount the tag would be negative on the left
    float overflow = tagTranslation.x - Pixels(2);
    overflow = -Math::Min(overflow, 0.0f);

    tagTranslation.x += overflow;
    mTagIcon->SetTranslation(tagTranslation);

    // Push the text to the right to account for the tag being stuck too far on the left
    mText->mTranslation.x += overflow;

    // Make sure the size isn't too large
    mText->mSize.x = Math::Min(mText->mSize.x, mSize.x - tagWidth - Pixels(2));

    // We need to update our children again to let the ellipses (...) on mText process
    Composite::UpdateTransform();
  }

  Element* mTagIcon;
};

//------------------------------------------------------------ Library Tile View
//******************************************************************************
LibraryTileView::LibraryTileView(LibraryView* parent)
  : TileView(parent), mLibraryView(parent)
{

}

//******************************************************************************
TileViewWidget* LibraryTileView::CreateTileViewWidget(Composite* parent,
                                  StringParam name, HandleParam instance,
                                  DataIndex index,
                                  PreviewImportance::Enum minImportance)
{
  PreviewWidget* previewWidget = nullptr;

  if(instance.IsNull())
    previewWidget = mLibraryView->CreatePreviewGroup(parent, name, 4);

  // If the instance isn't valid, it's a tag
  if(instance.IsNotNull() || previewWidget == nullptr)
    previewWidget = ResourcePreview::CreatePreviewWidget(parent, name, instance, minImportance);

  if(previewWidget == nullptr)
    return nullptr;

  if(instance.IsNull())
    return new TagTileWidget(parent, this, previewWidget, index);
  else
    return new TileViewWidget(parent, this, previewWidget, index);
}

//----------------------------------------------------------------- Library View
ZilchDefineType(LibraryView, builder, type)
{
}

void RegisterEditorTileViewWidgets();

//******************************************************************************
LibraryView::LibraryView(Composite* parent)
  : Composite(parent)
  , mResourceLibrary(nullptr)
{
  mSource = nullptr;
  mDataSelection = nullptr;
  mContentLibrary = nullptr;
  mIgnoreEditorSelectionChange = false;
  this->SetLayout(CreateStackLayout());

  mLibrariesRow = new Composite(this);
  mLibrariesRow->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(16));
  mLibrariesRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(5, 0), Thickness(1, 0, 2, 0)));
  {
    mContentLibraries = new StringComboBox(mLibrariesRow);

    ConnectThisTo(mContentLibraries, Events::ItemSelected, OnContentLibrarySelected);
    
    mContentLibraries->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

    BuildContentLibraryList();
  }

  Spacer* spacer = new Spacer(this);
  spacer->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(4));

  ConnectThisTo(Z::gContentSystem, Events::PackageBuilt, OnPackageBuilt);

  Composite* topRow = new Composite(this);
  topRow->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(16));
  topRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(1, 0, 2, 0)));
  {
    mSearchBox = new TagChainTextBox(topRow);
    mSearchBox->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);
    mSearchBox->mAddTagsOnEnter = false;
    ConnectThisTo(mSearchBox, Events::SearchDataModified, OnSearchDataModified);
    ConnectThisTo(mSearchBox, Events::KeyDown, OnSearchKeyDown);
    ConnectThisTo(mSearchBox, Events::KeyPreview, OnSearchKeyPreview);
    ConnectThisTo(mSearchBox, Events::KeyRepeated, OnSearchKeyRepeated);
    mSearch = &mSearchBox->mSearch;
    mSearch->ActiveTags.Insert("Resources");

    mToggleViewButton = new ToggleIconButton(topRow);
    mToggleViewButton->SetEnabledIcon("GridIcon");
    mToggleViewButton->SetDisabledIcon("TreeIcon");
    ConnectThisTo(mToggleViewButton, Events::ButtonPressed, OnToggleViewButtonPressed);
  }

  spacer = new Spacer(this);
  spacer->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(4));

  RegisterEditorTileViewWidgets();

  mTreeView = new TreeView(this);
  mTreeView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mTreeView->SetFormatNameAndType();

  ConnectThisTo(mTreeView, Events::TreeRightClick, OnTreeRightClick);
  ConnectThisTo(mTreeView, Events::KeyDown, OnKeyDown);
  ConnectThisTo(mTreeView, Events::MouseEnterRow, OnMouseEnterTreeRow);
  ConnectThisTo(mTreeView, Events::MouseExitRow, OnMouseExitTreeRow);

  mActiveView = mTreeView;

  mTileView = new LibraryTileView(this);
  mTileView->SetActive(false);
  mTileView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  ConnectThisTo(mTileView, Events::TileViewRightClick, OnTileViewRightClick);
  ConnectThisTo(mTileView, Events::ScrolledAllTheWayOut, OnTilesScrolledAllTheWayOut);

  mTagEditor = new ResourceTagEditor(this);
  mTagEditor->SetSizing(SizeAxis::Y, SizePolicy::Fixed, 20);

  ConnectThisTo(mTagEditor, Events::TagsModified, OnTagEditorModified);

  mTagEditorCloseButton = mTagEditor->CreateAttached<Element>("Minimize");
  ConnectThisTo(mTagEditorCloseButton, Events::LeftMouseDown, OnTagEditorClose);
  ConnectThisTo(mTagEditorCloseButton, Events::MouseHover, OnTagEditorCloseHover);

  SetTagEditorHeight(0);

  ConnectThisTo(Z::gResources, Events::ResourceAdded, OnResourcesModified);
  ConnectThisTo(Z::gResources, Events::ResourceRemoved, OnResourcesModified);
  ConnectThisTo(Z::gResources, Events::ResourceModified, OnResourcesModified);
  ConnectThisTo(Z::gResources, Events::ResourcesLoaded, OnResourcesModified);
  ConnectThisTo(Z::gResources, Events::ResourcesUnloaded, OnResourcesModified);

  ConnectThisTo(mTreeView, Events::MouseScroll, OnMouseScroll);

  ConnectThisTo(Z::gEditor, Events::ProjectLoaded, OnProjectLoaded);

  MetaSelection* selection = Z::gEditor->GetSelection();
  ConnectThisTo(selection, Events::SelectionFinal, OnEditorSelectionChanged);
}

LibraryView::~LibraryView()
{
  SafeDelete(mSource);
}

//******************************************************************************
void LibraryView::UpdateTransform()
{
  Composite::UpdateTransform();
}

//******************************************************************************
void LibraryView::View(ContentLibrary* contentLibrary, ResourceLibrary* resourceLibrary)
{
  if(mResourceLibrary)
    DisconnectAll(mResourceLibrary, this);

  mContentLibrary = contentLibrary;
  mResourceLibrary = resourceLibrary;

  // These can be set to NULL
  if(mContentLibrary == nullptr || mResourceLibrary == nullptr)
    return;

  ConnectThisTo(resourceLibrary, Events::ResourceAdded, OnResourcesModified);

  mSearch->ClearSearchProviders();
  // We want to show all hidden resources in this resource library. The reason hidden exists is
  // because we don't want them to show up when setting resource properties, but we do want them
  // to show up when viewing the resource library
  mSearch->SearchProviders.PushBack(GetResourceSearchProvider(mResourceLibrary, true));

  // Refresh the search with the new library
  mSearchBox->Refresh();

  // The data source represents the visible objects in the tree and tile view
  if(mSource == nullptr)
  {
    mSource = new LibraryDataSource();
    mTreeView->SetDataSource(mSource);
    mTileView->SetDataSource(mSource);
    ConnectThisTo(mSource, Events::DataActivated, OnDataActivated);
  }

  // The selected objects
  if(mDataSelection == nullptr)
  {
    mDataSelection = new HashDataSelection();
    mTreeView->SetSelection(mDataSelection);
    mTileView->SetSelection(mDataSelection);
    ConnectThisTo(mDataSelection, Events::DataSelectionModified, OnDataSelectionModified);
    ConnectThisTo(mDataSelection, Events::DataSelectionFinal, OnDataSelectionFinal);
  }

  UpdateVisibleResources();

  uint index = mContentLibraries->GetIndexOfItem(contentLibrary->Name);
  mContentLibraries->SetSelectedItem((int)index, false);
}

void LibraryView::View()
{
  if (mContentLibraries->GetCount() > 0)
  {
    String selectedLibrary = mContentLibraries->GetItem(0);
    ResourceLibrary* resourceLibrary = Z::gResources->GetResourceLibrary(selectedLibrary);
    ContentLibrary** contentLibrary = Z::gContentSystem->Libraries.FindPointer(selectedLibrary);
  
    View(*contentLibrary, resourceLibrary);
  }
}

//******************************************************************************
void LibraryView::SwitchToTreeView()
{
  if(mActiveView == mTileView)
  {
    mTileView->SetActive(false);
    mTreeView->SetActive(true);
    mTreeView->MarkAsNeedsUpdate();
    mActiveView = mTreeView;
  }
}

//******************************************************************************
void LibraryView::SwitchToTileView()
{
  if(mActiveView == mTreeView)
  {
    mTreeView->SetActive(false);
    mTileView->SetActive(true);
    mActiveView = mTileView;
  }
}

//******************************************************************************
void LibraryView::SetSearchTags(HashSet<String>& tags)
{
  mSearchBox->ClearTags();
  forRange(String tag, tags.All())
  {
    mSearchBox->AddTag(tag, true, false);
  }
  mSearchBox->Refresh();
}

//******************************************************************************
void PopulateGroup(PreviewWidgetGroup* group, SearchData* searchData,
                   uint maxResults)
{
  searchData->Search();

  uint count = searchData->Results.Size();
  count = Math::Min(count, maxResults);
  uint found = 0;
  for(uint i = 0; i < searchData->Results.Size() && found < count; ++i)
  {
    SearchViewResult& result = searchData->Results[i];

    String resultType = result.Interface->GetType(result);
    if(resultType != "Tag")
    {
      Resource* resource = (Resource*)result.Data;

      String name = resource->Name;
      PreviewWidget* widget = group->AddPreviewWidget(name, resource,
                                                      PreviewImportance::High);
      if(widget)
        ++found;
    }
  }
}


//******************************************************************************
PreviewWidgetGroup* LibraryView::CreatePreviewGroup(Composite* parent, 
                                                    StringParam tag, uint max)
{
  PreviewWidgetGroup* group = new PreviewWidgetGroup(parent, tag);

  // Create a search to search for items with the given tag
  SearchData temporarySearch;
  temporarySearch.SearchProviders.PushBack(GetResourceSearchProvider(mResourceLibrary, true));
  forRange(String currTag, mSearch->ActiveTags.All())
  {
    temporarySearch.ActiveTags.Insert(currTag);
  }
  temporarySearch.ActiveTags.Insert(tag);

  // Fill out the group 
  PopulateGroup(group, &temporarySearch, max);

  if(group->mPreviewWidgets.Size() == 0)
  {
    group->Destroy();
    return nullptr;
  }

  return group;
}

//******************************************************************************
void LibraryView::AddHiddenLibrary(StringParam libraryName)
{
  mHiddenLibraries.Insert(libraryName);
  BuildContentLibraryList();
}

//******************************************************************************
void LibraryView::UpdateVisibleResources()
{
  if(mSource == nullptr || mResourceLibrary == nullptr)
    return;

  mSource->Clear();

  // If there's only the default active tag (Resources), and they haven't
  // typed anything yet, we want to just show tags for the resources
  // they have in their project
  if(mSearch->ActiveTags.Size() == 1 && mSearch->SearchString.Empty())
  {
    HashSet<String> resourceTypes;
    forRange(HandleOf<Resource> resourceHandle, mResourceLibrary->Resources.All())
    {
      Resource* resource = resourceHandle;
      if(resource == nullptr)
      {
        Error("This should never be the case. "
              "Somehow we have a reference to a resource that doesn't exist.");
        continue;
      }

      // Add the type name
      resourceTypes.Insert(ZilchVirtualTypeId(resource)->Name);

      // Add a filter tag as well
      String filterTag = resource->FilterTag;
      if(!filterTag.Empty())
        resourceTypes.Insert(filterTag);
    }

    forRange(String& resourceType, resourceTypes.All())
    {
      mSource->AddTag(resourceType);
    }

    // Sort by resource name
    mSource->SortByTagName();
  }
  // Otherwise, we want to show the results
  else
  {
    forRange(SearchViewResult& result, mSearch->Results.All())
    {
      String resultType = result.Interface->GetType(result);
      if(resultType != "Tag")
      {
        Resource* resource = (Resource*)result.Data;
        
        // Only add it if it's the set we're editing
        //if(resource->mSet == mResourceLibrary)
        mSource->AddResource(resource);
      }
      else
      {
        mSource->AddTag(result.Name);
      }
    }
  }

  mTreeView->ClearAllRows();
  mTreeView->SetDataSource(mSource);
  mTileView->SetDataSource(mSource);
}

//******************************************************************************
void LibraryView::BuildContentLibraryList()
{
  mContentLibraries->ClearItems();
  mLibrariesRow->SetActive(false);

  forRange(ContentLibrary* library, Z::gContentSystem->Libraries.Values())
  {
    if(library == nullptr || mHiddenLibraries.Contains(library->Name))
      continue;
    
    // Only show the library if a resource library was built from it
    if(Z::gResources->GetResourceLibrary(library->Name))
      mContentLibraries->AddItem(library->Name);
  }

  // Enable dropdown selection menu for multiple libraries
  if (mContentLibraries->GetCount() > 1)
  {
    mLibrariesRow->SetActive(true);
  }
}

//******************************************************************************
void LibraryView::OnPackageBuilt(ContentSystemEvent* e)
{
  String libraryName = e->mLibrary->Name;
  if(mHiddenLibraries.Contains(libraryName))
    return;

  mContentLibraries->AddItem(libraryName);
  if(mContentLibraries->GetCount() > 1)
    mLibrariesRow->SetActive(true);

  // If nothing is selected, attempt to select the library that was initially viewed
  int selected = mContentLibraries->GetSelectedItem();
  if(selected == cNoItemSelected && mContentLibrary)
  {
    uint index = mContentLibraries->GetIndexOfItem(mContentLibrary->Name);
    mContentLibraries->SetSelectedItem((int)index, false);
  }
}

//******************************************************************************
void LibraryView::OnContentLibrarySelected(Event* e)
{
  int selectedIndex = mContentLibraries->GetSelectedItem();
  if (selectedIndex == -1)
    return;

  String selectedItem = mContentLibraries->GetItem(selectedIndex);
  ContentLibrary* contentLibrary = Z::gContentSystem->Libraries.FindValue(selectedItem, nullptr);
  ResourceLibrary* resourceLibrary = Z::gResources->LoadedResourceLibraries.FindValue(selectedItem, nullptr);
  if(contentLibrary && resourceLibrary)
    View(contentLibrary, resourceLibrary);
}

//******************************************************************************
void LibraryView::OnResourcesModified(ResourceEvent* event)
{
  // If an archetype has a RunInEditor script that creates a runtime resource
  // we don't want the preview to cause itself to be recreated
  if (event->EventResource != nullptr && event->EventResource->IsRuntime())
    return;

  mSearchBox->Refresh();

  // If there are no results (likely because resources have been removed),
  // clear the active tags and restart from the beginning
  if(mSearch->Results.Empty())
  {
    mSearchBox->ClearTags();
    mSearchBox->Refresh();
  }
  
  UpdateVisibleResources();
}

//******************************************************************************
void LibraryView::OnTreeRightClick(TreeEvent* event)
{
  DataIndex index = event->Row->mIndex;
  OnRightClickObject(event->Row, index);
}

//******************************************************************************
void LibraryView::OnTileViewRightClick(TileViewEvent* event)
{
  DataIndex index = event->mTile->GetDataIndex();
  OnRightClickObject(event->mTile, index);
}

//******************************************************************************
void LibraryView::OnRightClickObject(Composite* objectToAttachTo, DataIndex index)
{
  mCommandIndices.Clear();
  ContextMenu* menu = new ContextMenu(objectToAttachTo);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0));

  mDataSelection->GetSelected(mCommandIndices);
  mPrimaryCommandIndex = index;
  LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);

  Resource* resource = entry->mResource;
  if(resource)
  {
    ConnectMenu(menu, "Edit", OnEdit);
    ConnectMenu(menu, "Rename", OnRename);
    ConnectMenu(menu, "Edit Content Meta", OnEditMeta);
    ConnectMenu(menu, "Edit Tags", OnEditTags);
    ConnectMenu(menu, "Remove", OnRemove);

    if(resource && resource->mManager->mCanDuplicate)
    {
      ConnectMenu(menu, "Duplicate", OnDuplicate);
    }

    BoundType* resourceType = ZilchVirtualTypeId(resource);

    // Add composing and translation test functions for materials
    if(resourceType->IsA(ZilchTypeId(Material)))
    {
      ConnectMenu(menu, "ComposeZilchMaterial", OnComposeZilchMaterial);
      ConnectMenu(menu, "TranslateZilchPixelMaterial", OnTranslateZilchPixelMaterial);
      ConnectMenu(menu, "TranslateZilchGeometryMaterial", OnTranslateZilchGeometryMaterial);
      ConnectMenu(menu, "TranslateZilchVertexMaterial", OnTranslateZilchVertexMaterial);
    }
    // Add a translation tests function for fragments
    if(resourceType->IsA(ZilchTypeId(ZilchFragment)))
    {
      ConnectMenu(menu, "TranslateFragment", OnTranslateFragment);
    }
  }
  else
  {
    ConnectMenu(menu, "Add Tag To Search", OnAddTagToSearch);
    //ConnectMenu(menu, "Just This Tag", OnRename);
  }

  menu->SizeToContents();
}

//******************************************************************************
void LibraryView::OnKeyDown(KeyboardEvent* event)
{
  if(event->Handled)
    return;

  //Delete selected objects
  if(event->Key == Keys::Delete)
  {
    mCommandIndices.Clear();
    mDataSelection->GetSelected(mCommandIndices);
    if(!mCommandIndices.Empty())
    {
      mPrimaryCommandIndex = mCommandIndices.Front();
      OnRemove(nullptr);
    }
  }
  else if(event->Key == Keys::Enter)
  {
    Array<DataIndex> indices;
    mDataSelection->GetSelected(indices);

    if(indices.Size() == 1)
    {
      LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(indices.Front());
      if(entry->mResource)
      {
        Z::gEditor->EditResource(entry->mResource);
      }
      else
      {
        mSearchBox->AddTag(entry->mTag, true, false);
      }
    }
  }

  if(event->Key == Keys::F2)
  {
    DataSelection* selection = mTreeView->GetSelection();
    // check if we have something currently selected
    if(selection)
    {
      Array<DataIndex> selectedIndices;
      mDataSelection->GetSelected(selectedIndices);
      // make sure we have anything actively selected
      if(selectedIndices.Size())
      {
        // get the row of the selected item and edit its name
        TreeRow* row = mTreeView->FindRowByIndex(selectedIndices.Front());
        row->Edit(CommonColumns::Name);
      }
    }
  }

  if(event->CtrlPressed && event->Key == Keys::A)
    SelectAll();
}

//******************************************************************************
void LibraryView::OnMouseEnterTreeRow(TreeEvent* event)
{
  mResourcePreview.SafeDestroy();
  DataIndex index = event->Row->mIndex;
  LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(index);
  if(entry->mResource)
    CreateResourceToolTip(entry->mResource, event->Row);
  else
    CreateTagToolTip(entry->mTag, event->Row);
}

//******************************************************************************
void LibraryView::OnMouseExitTreeRow(TreeEvent* event)
{
  mResourcePreview.SafeDestroy();
}

//******************************************************************************
void LibraryView::CreateResourceToolTip(Resource* resource, TreeRow* row)
{
  // Create the tooltip
  ToolTip* toolTip = new ToolTip(row);
  toolTip->mContentPadding = Thickness(2,2,2,2);
  toolTip->SetColor(ToolTipColor::Gray);

  // Create the resource widget and attach it to the tooltip
  String name = resource->Name;
  PreviewWidget* tileWidget = ResourcePreview::CreatePreviewWidget(toolTip, name, resource, PreviewImportance::High);
  if(tileWidget == nullptr)
  {
    toolTip->Destroy();
    return;
  }
  tileWidget->SetSize(Pixels(200, 200));
  toolTip->SetContent(tileWidget);

  // Position the tooltip
  ToolTipPlacement placement;
  placement.SetScreenRect(row->GetScreenRect());
  placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, 
                        IndicatorSide::Bottom, IndicatorSide::Top);
  toolTip->SetArrowTipTranslation(placement);

  tileWidget->AnimatePreview(PreviewAnimate::Always);

  mResourcePreview = toolTip;
}

//******************************************************************************
void LibraryView::CreateTagToolTip(StringParam tagName, TreeRow* row)
{
  // Create the tooltip
  ToolTip* toolTip = new ToolTip(row);
  toolTip->mContentPadding = Thickness(2,2,2,2);
  toolTip->SetColor(ToolTipColor::Gray);

  PreviewWidgetGroup* group = CreatePreviewGroup(toolTip, tagName, 9);
  if(group == nullptr)
  {
    toolTip->Destroy();
    return;
  }

  group->SetSize(group->GetMinSize());
  toolTip->SetContent(group);

  // Position the tooltip
  ToolTipPlacement placement;
  placement.SetScreenRect(row->GetScreenRect());
  placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, 
                        IndicatorSide::Bottom, IndicatorSide::Top);
  toolTip->SetArrowTipTranslation(placement);

  group->AnimatePreview(PreviewAnimate::Always);

  mResourcePreview = toolTip;
}

//******************************************************************************
void LibraryView::OnDataActivated(DataEvent* event)
{
  LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(event->Index);
  if(entry->mResource)
  {
    Z::gEditor->EditResource(entry->mResource);
  }
  else
  {
    mSearchBox->AddTag(entry->mTag, true, false);
  }
}

//******************************************************************************
void LibraryView::OnDataSelectionModified(ObjectEvent* event)
{
  if(TagEditorIsOpen())
    EditTags(mDataSelection);

  // Get the selected indices
  Array<DataIndex> selectedIndices;
  mDataSelection->GetSelected(selectedIndices);

  if(mDataSelection->Size() != 0)
    mSearchBox->mSearchIndex = (uint)selectedIndices.Front().Id;

  // Add all selected resources to the editors selection
  MetaSelection* editorSelection = Z::gEditor->GetSelection();

  // We only want to clear the editors selection if we're selecting a resource
  bool cleared = false;
  forRange(DataIndex dataIndex, selectedIndices.All())
  {
    // Only add it if it's a resource
    LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(dataIndex);
    if(entry->mResource)
    {
      // Ignore all documents (scripts, shader fragments, etc...)
      if(Type::DynamicCast<DocumentResource*>(entry->mResource))
        continue;

      // Clear the editor selection first
      if(!cleared)
      {
        editorSelection->Clear();
        cleared = true;
      }
      editorSelection->Add(entry->mResource, SendsEvents::False);
    }
  }

  if(cleared)
    editorSelection->SelectionChanged();
}

//******************************************************************************
void LibraryView::OnDataSelectionFinal(ObjectEvent* event)
{
  OnDataSelectionModified(event);

  MetaSelection* editorSelection = Z::gEditor->GetSelection();

  mIgnoreEditorSelectionChange = true;
  editorSelection->FinalSelectionChanged();
  mIgnoreEditorSelectionChange = false;
}

//******************************************************************************
void LibraryView::OnEditorSelectionChanged(SelectionChangedEvent* event)
{
  if(mIgnoreEditorSelectionChange || mDataSelection == nullptr)
    return;

  mDataSelection->SelectNone();

  forRange(Resource* resource, event->Selection->AllOfType<Resource>())
  {
    DataIndex index = mSource->GetResourceIndex(resource);
    if(index.Id != (u64)-1)
      mDataSelection->Select(index);
  }
  mTreeView->MarkAsNeedsUpdate();
  mTileView->MarkAsNeedsUpdate();
}

//******************************************************************************
void LibraryView::SelectAll()
{
  mTreeView->SelectAll();
}

//******************************************************************************
void LibraryView::OnRemove(ObjectEvent* event)
{
  String message, title;

  // This isn't the resource count, because it may contain rows that are tags
  uint indexCount = mCommandIndices.Size();
  if(indexCount > 1)
  {
    // We're removing multiple resources
    title = "Remove Resources";

    String names;
    uint resourceCount = 0;
    for(uint i = 0; i < mCommandIndices.Size(); ++i)
    {
      DataIndex currIndex = mCommandIndices[i];

      LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(currIndex);
      if(Resource* resource = treeNode->mResource)
      {
        // Add this resource name to the list of names
        names = BuildString(names, resource->Name);

        // Only add a comma after if it's not the last resource
        if(i != mCommandIndices.Size() - 1)
          names = BuildString(names, ", ");

        ++resourceCount;
      }
    }

    message = String::Format("Are you sure you want to remove %i resources [%s]"
                             " from the content library?", resourceCount, names.c_str());
  }
  else
  {
    // We're removing a single resource
    title = "Remove Resource";

    LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
    if(Resource* resource = treeNode->mResource)
    {
      message = String::Format("Are you sure you want to remove the resource '%s' " 
                               " from the content library?", resource->Name.c_str());
    }
    else
    {
      // If it's not a resource, there's nothing we can do
      return;
    }
  }

  MessageBox* box = MessageBox::Show(title, message , MBConfirmCancel);
  ConnectThisTo(box, Events::MessageBoxResult, OnMessageBox);
}

//******************************************************************************
void LibraryView::OnRename(ObjectEvent* event)
{
  TreeRow* row = mTreeView->FindRowByIndex(mPrimaryCommandIndex);
  row->Edit(CommonColumns::Name);
}

//******************************************************************************
void LibraryView::OnEdit(ObjectEvent* event)
{
  LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);

  if(Resource* resource = entry->mResource)
    Z::gEditor->EditResource(resource);
}

//******************************************************************************
void LibraryView::OnEditMeta(ObjectEvent* event)
{
  LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);

  if (Resource* resource = entry->mResource)
  {
    ContentItem* contentItem = resource->mContentItem;

    MetaSelection* selection = Z::gEditor->GetSelection();
    selection->SelectOnly(contentItem);
    selection->FinalSelectionChanged();
  }
}

//******************************************************************************
void LibraryView::OnEditTags(ObjectEvent* event)
{
  EditTags(mDataSelection);
}

//******************************************************************************
void LibraryView::OnMessageBox(MessageBoxEvent* event)
{
  if(event->ButtonIndex == MessageResult::Yes)
  {
    Array<Resource*> resourcesToRemove;

    for(uint i = 0; i < mCommandIndices.Size(); ++i)
    {
      DataIndex currIndex = mCommandIndices[i];
      LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(currIndex);
      if(Resource* resource = treeNode->mResource)
        resourcesToRemove.PushBack(resource);
    }

    forRange(Resource* resource, resourcesToRemove.All())
    {
      RemoveResource(resource);
    }

    mPrimaryCommandIndex = 0;
    mCommandIndices.Clear();

    Z::gEditor->GetSelection()->FinalSelectionChanged();
  }
}

//******************************************************************************
void LibraryView::OnDuplicate(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if(Resource* resource = treeNode->mResource)
    DuplicateResource(resource);
}

//******************************************************************************
void LibraryView::OnComposeZilchMaterial(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if(Resource* resource = treeNode->mResource)
    if(Resource* resource = treeNode->mResource)
    {
      ObjectEvent toSend(resource);
      Z::gEditor->DispatchEvent("ComposeZilchMaterial", &toSend);
    }
}

//******************************************************************************
void LibraryView::OnTranslateZilchPixelMaterial(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if(Resource* resource = treeNode->mResource)
  {
    ObjectEvent toSend(resource);
    Z::gEditor->DispatchEvent("TranslateZilchPixelMaterial", &toSend);
  }
}

//******************************************************************************
void LibraryView::OnTranslateZilchGeometryMaterial(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if(Resource* resource = treeNode->mResource)
  {
    ObjectEvent toSend(resource);
    Z::gEditor->DispatchEvent("TranslateZilchGeometryMaterial", &toSend);
  }
}

//******************************************************************************
void LibraryView::OnTranslateZilchVertexMaterial(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if(Resource* resource = treeNode->mResource)
  {
    ObjectEvent toSend(resource);
    Z::gEditor->DispatchEvent("TranslateZilchVertexMaterial", &toSend);
  }
}

//******************************************************************************
void LibraryView::OnTranslateFragment(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if(Resource* resource = treeNode->mResource)
  {
    ObjectEvent toSend(resource);
    Z::gEditor->DispatchEvent("TranslateZilchFragment", &toSend);
  }
}

//******************************************************************************
void LibraryView::OnAddTagToSearch(ObjectEvent* event)
{
  Array<DataIndex> selected;
  mDataSelection->GetSelected(selected);

  Array<String> tagsToAdd;
  forRange(DataIndex& index, selected.All())
  {
    LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(index);
    if(entry->mResource == nullptr)
      tagsToAdd.PushBack(entry->mTag);
  }

  forRange(String& tag, tagsToAdd.All())
  {
    mSearchBox->AddTag(tag, true, false);
  }
}

//******************************************************************************
void LibraryView::OnToggleViewButtonPressed(Event* e)
{
  if (mToggleViewButton->GetEnabled())
    SwitchToTreeView();
  else
    SwitchToTileView();
}

//******************************************************************************
void LibraryView::OnSearchDataModified(Event* e)
{
  if(mSearchBox->mSearchBar->HasFocus())
  {
    if(mTreeView->GetActive())
      mTreeView->SelectFirstRow();
    else if(mTileView->GetActive())
      mTileView->SelectFirstTile();
  }
  else if(mDataSelection)
  {
    mDataSelection->SelectNone();
  }

  UpdateVisibleResources();
}

//******************************************************************************
void LibraryView::OnSearchKeyDown(KeyboardEvent* e)
{
  HandleSearchKeyLogic(e);
}

//******************************************************************************
void LibraryView::OnSearchKeyPreview(KeyboardEvent* e)
{
  if(e->Handled)
    return;

  if(e->Key == Keys::Enter)
  {
    Array<DataIndex> indices;
    mDataSelection->GetSelected(indices);

    if(indices.Size() == 1)
    {
      LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(indices.Front());

      if(entry->mResource)
        Z::gEditor->EditResource(entry->mResource);
      else
        mSearchBox->AddTag(entry->mTag, true, false);

      e->Handled = true;
    }
  }
}

//******************************************************************************
void LibraryView::OnSearchKeyRepeated(KeyboardEvent* e)
{
  HandleSearchKeyLogic(e);
}

//******************************************************************************
void LibraryView::HandleSearchKeyLogic(KeyboardEvent* e)
{
  if(e->Key == Keys::Down || e->Key == Keys::Up ||
     e->Key == Keys::Left || e->Key == Keys::Right)
  {
    e->Handled = false;
    if(mTreeView->GetActive())
      mTreeView->GetScrollArea()->GetClientWidget()->DispatchEvent(Events::KeyDown, e);
    else if(mTileView->GetActive())
      mTileView->GetScrollArea()->GetClientWidget()->DispatchEvent(Events::KeyDown, e);
  }
}

//******************************************************************************
void LibraryView::OnMouseScroll(MouseEvent* e)
{
  if(e->CtrlPressed)
  {
    if(e->Scroll.y > 0)
    {
      SwitchToTileView();
      mTileView->SetItemSizePercent(0);
    }
  }
}

//******************************************************************************
void LibraryView::OnTilesScrolledAllTheWayOut(Event* e)
{
  SwitchToTreeView();
}

//******************************************************************************
void LibraryView::OnProjectLoaded(Event* e)
{
  mSearchBox->ClearTags();
  mSearchBox->Refresh();
}

//******************************************************************************
void LibraryView::OnTagEditorModified(Event* e)
{
  OpenTagEditor();
}

//******************************************************************************
void LibraryView::OnTagEditorClose(MouseEvent* e)
{
  CloseTagEditor();
}

//******************************************************************************
void LibraryView::OnTagEditorCloseHover(MouseEvent* e)
{
  //new ToolTip(this, "Hide");
}

//******************************************************************************
float LibraryView::GetTagEditorHeight()
{
  return mTagEditorHeight;
}

//******************************************************************************
void LibraryView::SetTagEditorHeight(float height)
{
  if(height < Pixels(1))
  {
    mTagEditor->SetActive(false);
  }
  else
  {
    mTagEditorCloseButton->SetTranslation(Vec3(mTagEditor->mSize.x - Pixels(18), Pixels(2), 0));
    mTagEditor->SetSize(Vec2(0, height));
    mTagEditor->SetActive(true);
  }
  mTagEditorHeight = height;
  MarkAsNeedsUpdate();
}

//******************************************************************************
bool LibraryView::TagEditorIsOpen()
{
  // When it's closed the height should be 0
  return mTagEditorHeight != 0.0f;
}

//******************************************************************************
void LibraryView::EditTags(DataSelection* dataSelection)
{
  Array<DataIndex> selection;
  dataSelection->GetSelected(selection);

  Array<Resource*> resources;
  for(uint i = 0; i < selection.Size(); ++i)
  {
    DataIndex dataIndex = selection[i];
    Resource* resource = ((LibDataEntry*)mSource->ToEntry(dataIndex))->mResource;
    if(resource)
      resources.PushBack(resource);
  }

  mTagEditor->EditResources(resources);
  OpenTagEditor();
}

//******************************************************************************
void LibraryView::OpenTagEditor()
{
  float height = mTagEditor->GetDesiredHeight();

  ActionSequence* seq = new ActionSequence(this);
  seq->Add(AnimatePropertyGetSet(ZilchSelf, TagEditorHeight, Ease::Quad::Out, 
                                 this, 0.3f, height));
}

//******************************************************************************
void LibraryView::CloseTagEditor()
{
  float height = 0;

  ActionSequence* seq = new ActionSequence(this);
  seq->Add(AnimatePropertyGetSet(ZilchSelf, TagEditorHeight, Ease::Quad::Out, 
                                 this, 0.3f, height));
}

}// namespace Zero
