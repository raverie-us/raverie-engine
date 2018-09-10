////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(AddWindowCancelled);
  DefineEvent(ResourceTypeSelected);
  DefineEvent(ResourceTemplateSelected);
  DefineEvent(PostAddResource);
}

//------------------------------------------------------------------------------ Post Add Resource Event
ZilchDefineType(PostAddResourceEvent, builder, type)
{
}

PostAddResourceEvent::PostAddResourceEvent(PostAddOp& postAdd, ResourceAdd* resource)
  : mPostAdd(postAdd), mResourceAdd(resource)
{
}

const String cFilesSelected = "cFilesSelected";

//**************************************************************************************************
AddResourceWindow* OpenAddWindow(BoundType* resourceType, Window** window, StringParam resourceName)
{
  Composite* parent = Z::gEditor;
  Window* addWindow = new Window(parent);
  addWindow->mClientPadding = Thickness::cZero;

  AddResourceWindow* addDialog = new AddResourceWindow(addWindow);

  if (resourceType)
    addDialog->ShowResourceTypeSearch(false);

  if(resourceType)
    addWindow->SetTitle(BuildString("Add ", resourceType->Name, " Resource"));
  else
    addWindow->SetTitle("Add a Resource");

  if (resourceType)
    addDialog->SelectResourceType(resourceType);
  // Select zilch script by default
  else
    addDialog->SelectResourceType(ZilchTypeId(ZilchScript));

  addWindow->SizeToContents();

  if (resourceType)
    addWindow->mSize.y += Pixels(20);
  else
    addWindow->mSize.y += Pixels(10);

  // Animate window in from above
  CenterToWindow(parent, addWindow, false);

  if(window)
    *window = addWindow;

  addDialog->TakeFocus();

  if(!resourceName.Empty())
    addDialog->SetResourceNameField(resourceName);

  return addDialog;
}

//------------------------------------------------------------------------------ Add Resource Window
//**************************************************************************************************
ZilchDefineType(AddResourceWindow, builder, type)
{

}

//**************************************************************************************************
void CloseAddWindow(Composite* windowElement)
{
  Event e;
  windowElement->DispatchBubble(Events::AddWindowCancelled, &e);
  CloseTabContaining(windowElement);

  Z::gEditor->GetCenterWindow()->TryTakeFocus();
}

//**************************************************************************************************
AddResourceWindow::AddResourceWindow(Composite* parent) :
  Composite(parent)
{
  mMinSize = Pixels(960, 200);
  
  // Main area and a bar at the bottom
  SetLayout(CreateStackLayout());

  Composite* mainArea = new Composite(this);
  mainArea->SetSizing(SizePolicy::Flex, 1.0f);
  mainArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));
  {
    mResourceTypeSearch = new ResourceTypeSearch(mainArea);
    mResourceTypeSearch->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(240));
    ConnectThisTo(mResourceTypeSearch, Events::ResourceTypeSelected, OnResourceTypeSelected);
    
    Widget* seperator = mainArea->CreateAttached<Element>(cWhiteSquare);
    seperator->SetColor(Vec4(0.09f, 0.09f, 0.09f, 1.0f));
    seperator->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(2));
    seperator->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);
    seperator->SetNotInLayout(false);

    mResourceTemplateSearch = new ResourceTemplateSearch(mainArea);
    mResourceTemplateSearch->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
    ConnectThisTo(mResourceTemplateSearch, Events::ResourceTemplateSelected, OnResourceTemplateSelected);
    
    seperator = mainArea->CreateAttached<Element>(cWhiteSquare);
    seperator->SetColor(Vec4(0.09f, 0.09f, 0.09f, 1.0f));
    seperator->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(2));
    seperator->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);
    seperator->SetNotInLayout(false);

    mResourceTemplateDisplay = new ResourceTemplateDisplay(mainArea, mPostAdd);
    mResourceTemplateDisplay->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(240));

    mResourceTypeSearch->mNextFocus = mResourceTemplateSearch;
    mResourceTemplateSearch->mPreviousFocus = mResourceTypeSearch;
    mResourceTemplateSearch->mNextFocus = mResourceTemplateDisplay;
    mResourceTemplateDisplay->mPreviousFocus = mResourceTemplateSearch;
  }

  // Bar at the bottom
  Widget* bottomBar = CreateAttached<Element>(cWhiteSquare);
  bottomBar->SetColor(Vec4(0.1f, 0.1f, 0.1f, 1.0f));
  bottomBar->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  bottomBar->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(19.0f));
  bottomBar->SetNotInLayout(false);

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
}

//**************************************************************************************************
void AddResourceWindow::OnKeyDown(KeyboardEvent* e)
{
  // close the add window when escape is pressed
  if (e->Key == Keys::Escape)
    CloseAddWindow(this);
}

//**************************************************************************************************
void AddResourceWindow::SetResourceNameField(StringParam resourceName)
{
  mResourceTemplateDisplay->mNameField->SetText(resourceName);
  mResourceTemplateDisplay->ValidateName(false);

  mResourceTemplateDisplay->mNameField->TakeFocus();
}

//**************************************************************************************************
void AddResourceWindow::SelectResourceType(BoundType* resourceType)
{
  mResourceTemplateDisplay->ShowResourceTemplate(nullptr);
  mResourceTypeSearch->SetSelectedResourceType(resourceType);
}

//**************************************************************************************************
void AddResourceWindow::SetLibrary(ContentLibrary* library)
{
  if (library)
  {
    StringComboBox* comboBox = mResourceTemplateDisplay->mLibrarySelect;
    int index = comboBox->GetIndexOfItem(library->Name);
    if (index != -1)
      comboBox->SetSelectedItem(index, true);
  }
}

//**************************************************************************************************
void AddResourceWindow::ShowResourceTypeSearch(bool state)
{
  mResourceTypeSearch->SetActive(state);
  if(state == false)
    mResourceTemplateDisplay->HidePreview();

  if(state)
    mMinSize = Pixels(960, 200);
  else
    mMinSize = Pixels(720, 200);
}

//**************************************************************************************************
void AddResourceWindow::AddTags(TagList& tags)
{
  TextBox* tagsBox = mResourceTemplateDisplay->mTagsBox;
  StringBuilder allTags;
  
  // Keep whatever current tags are already set
  if(!tagsBox->GetText().Empty())
  {
    allTags.Append(tagsBox->GetText());
    allTags.Append(", ");
  }

  // Add all the tags for the resource being created
  forRange(StringParam& tag, tags.All())
  {
    // Only add tags that are not resource types as they are automatically added
    // to resources upon their creation
    BoundType* type = MetaDatabase::GetInstance()->FindType(tag);
    if(type && type->IsA(ZilchTypeId(Resource)))
      continue;

    allTags.Append(tag);
    allTags.Append(", ");
  }

  tagsBox->SetText(allTags.ToString());
}

//**************************************************************************************************
void AddResourceWindow::TemplateSearchTakeFocus()
{
  mResourceTypeSearch->mSearchField->LoseFocus();
  mResourceTemplateSearch->TakeFocus();
}

//**************************************************************************************************
bool AddResourceWindow::TakeFocusOverride()
{
  if (mResourceTypeSearch->GetActive())
    mResourceTypeSearch->TakeFocus();
  else if (mResourceTemplateSearch->mTemplateCount > 1)
    mResourceTemplateSearch->TakeFocus();
  else
    mResourceTemplateDisplay->TakeFocus();

  return true;
}

//**************************************************************************************************
void AddResourceWindow::OnResourceTypeSelected(Event*)
{
  mResourceTemplateDisplay->ShowResourceTemplate(nullptr);
  BoundType* selectedType = mResourceTypeSearch->GetSelectedResourceType();
  mResourceTemplateSearch->ShowTemplates(selectedType);
}

//**************************************************************************************************
void AddResourceWindow::OnResourceTemplateSelected(Event*)
{
  Resource* resourceTemplate = mResourceTemplateSearch->GetSelectedResourceTemplate();
  mResourceTemplateDisplay->ShowResourceTemplate(resourceTemplate);
}

//---------------------------------------------------------------------- Resource Search Data Source
struct ResourceManagerPolicy : public ObjectTree<String>
{
  typedef String StoredType;
  typedef String MapType;
  enum { Mappable = true };
  enum { AllowFolders = false };

  //************************************************************************************************
  static StoredType InvalidValue()
  {
    return String();
  }

  //************************************************************************************************
  static MapType Map(StringParam name)
  {
    return name;
  }

  //************************************************************************************************
  static String UnMap(MapType name)
  {
    return name;
  }

  //************************************************************************************************
  template<typename EntryType>
  static void GetData(Any& value, EntryType* entry, StringParam name, StringParam column)
  {
    if(!column.Empty())
    {
      if(column == CommonColumns::Name)
        value = name;
    }
  }

  //************************************************************************************************
  template<typename EntryType>
  static void SetData(AnyParam newValue, EntryType* entry, StringParam object, StringParam column)
  {
  }
};

//**************************************************************************************************
class ReourceManagersDataSource : public PolicyDataSource< ResourceManagerPolicy > { };

//----------------------------------------------------------------------------- Resource Type Search
//**************************************************************************************************
ZilchDefineType(ResourceTypeSearch, builder, type)
{

}

//**************************************************************************************************
ResourceTypeSearch::ResourceTypeSearch(Composite* parent)
  : ColoredComposite(parent, Vec4(0.17f, 0.17f, 0.17f, 1.0f))
{
  SetLayout(CreateStackLayout());
  
  Composite* searchFieldArea = new Composite(this);
  searchFieldArea->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  searchFieldArea->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(24.0f));
  searchFieldArea->SetLayout(CreateFillLayout(Thickness(Pixels(12, 4, 12, 4))));
  {
    mSearchField = new TagChainTextBox(searchFieldArea);
    mSearchField->SetSizing(SizePolicy::Flex, 1.0f);
   
    ConnectThisTo(mSearchField, Events::TextChanged, OnTextEntered);
    ConnectThisTo(mSearchField, Events::TextEnter, OnEnter);
    ConnectThisTo(mSearchField, Events::KeyDown, OnKeyDownSearch);
    ConnectThisTo(mSearchField, Events::KeyRepeated, OnKeyDownSearch);
    ConnectThisTo(mSearchField, Events::KeyPreview, OnSearchKeyPreview);
  }

  ColoredComposite* importArea = new ColoredComposite(this, Vec4(1, 1, 1, 0.05f));
  importArea->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  importArea->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(24.0f));
  {
    ImportButton* import = new ImportButton(importArea);
    import->SetTranslation(Pixels(12, 5, 0));
    import->SetSize(Pixels(61, 16));
    ConnectThisTo(import, Events::LeftClick, OnImportClicked);
  }
  

  ColoredComposite* resourceListArea = new ColoredComposite(this, Vec4(1, 1, 1, 0.1f));
  resourceListArea->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  resourceListArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);
  resourceListArea->SetLayout(CreateFillLayout());
  {
    mResourceList = new ItemList(resourceListArea, Pixels(19.0f), 1);
    mResourceList->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
    mResourceList->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);
    mResourceList->SetMinSize(Pixels(340, 380));
    ConnectThisTo(mResourceList, Events::ItemSelected, OnResourceTypeSelected);
    ConnectThisTo(mResourceList, Events::KeyDown, OnKeyDown);
  }

  HashMap<String, uint> categorySortWeights;
  categorySortWeights.Insert("Code", 10);
  categorySortWeights.Insert("Graphics", 20);
  categorySortWeights.Insert("Physics", 30);
  categorySortWeights.Insert("Sound", 40);
  categorySortWeights.Insert("Networking", 50);
  categorySortWeights.Insert("Other", 60);

  ResourceSystem* resourceSystem = Z::gResources;
  forRange(ResourceManager* manager, resourceSystem->Managers.Values())
  {
    if (manager->mCanAddFile || manager->mCanCreateNew)
    {
      String resourceTypeName = manager->GetResourceType()->Name;
      String category = manager->mCategory;
      
      // Default if it's empty
      if (category.Empty())
        category = "Other";

      uint categoryWeight = categorySortWeights.FindValue(category, 100);
      mResourceList->AddGroup(category, categoryWeight);

      mResourceList->AddItem(resourceTypeName, resourceTypeName, category, manager->mAddSortWeight);

      mValidResourceTypes.PushBack(resourceTypeName);
    }
  }

  mResourceList->Sort();

  ConnectThisTo(this, cFilesSelected, OnFilesSelected);
}

//**************************************************************************************************
BoundType* ResourceTypeSearch::GetSelectedResourceType()
{
  return mSelectedType;
}

//**************************************************************************************************
void ResourceTypeSearch::SetSelectedResourceType(BoundType* resourceType)
{
  mSelectedType = resourceType;
  mResourceList->SetSelectedItem(mSelectedType->Name, false);

  Event e;
  DispatchEvent(Events::ResourceTypeSelected, &e);
}

//**************************************************************************************************
bool ResourceTypeSearch::TakeFocusOverride()
{
  if (GetActive())
    mSearchField->TakeFocus();
  return true;
}

//**************************************************************************************************
void ResourceTypeSearch::OnImportClicked(Event*)
{
  FileDialogConfig config;
  config.EventName = "cFilesSelected";
  config.CallbackObject = this;
  config.Title = "Add a content file";
  config.AddFilter("All Content (*.*)", "*.*");
  config.Flags = FileDialogFlags::MultiSelect;
  Z::gEngine->has(OsShell)->OpenFile(config);
}

//**************************************************************************************************
void ResourceTypeSearch::OnFilesSelected(OsFileSelection* e)
{
  // Don't do anything if we select nothing
  if (e->Files.Empty())
    return;

  OpenGroupImport(e->Files);
  CloseTabContaining(this);

  Z::gEditor->GetCenterWindow()->TryTakeFocus();
}

//**************************************************************************************************
void ResourceTypeSearch::OnTextEntered(Event* e)
{
  if(!mSearchField->HasFocus())
    return;
  
  String currentText = mSearchField->GetText();
  if (currentText.Empty())
    return;
  
  auto filtered = FilterStrings(mValidResourceTypes.All(), currentText);
  
  if (filtered.mPrimary)
  {
    String resourceTypeName = *filtered.mPrimary;
    mResourceList->SetSelectedItem(resourceTypeName);
    mResourceList->ScrollToItem(resourceTypeName);
  }
}

//**************************************************************************************************
void ResourceTypeSearch::OnKeyDownSearch(KeyboardEvent* e)
{
  // Forward arrow events to the resource list
  if (mSearchField->HasFocus())
    mResourceList->DispatchEvent(e->EventId, e);
  else
    OnKeyDown(e);
}

//**************************************************************************************************
void ResourceTypeSearch::OnKeyDown(KeyboardEvent* e)
{
  if (e->Key == Keys::Tab || e->Key == Keys::Enter)
  {
    mSearchField->mSearchBar->LoseFocus();
    mNextFocus->TakeFocus();
  }
}

//**************************************************************************************************
void ResourceTypeSearch::OnSearchKeyPreview(KeyboardEvent* e)
{
  if (e->Key == Keys::Escape)
  {
    // ResourceTypeSearch Parent = Main Area, Main Area Parent = Add Window
    CloseAddWindow(this);
  }
}

//**************************************************************************************************
void ResourceTypeSearch::OnEnter(Event*)
{
  mNextFocus->TakeFocus();
}

//**************************************************************************************************
void ResourceTypeSearch::OnResourceTypeSelected(Event*)
{
  String resourceTypeName = mResourceList->GetSelectedItem();
  if (BoundType* resourceType = MetaDatabase::FindType(resourceTypeName))
    SetSelectedResourceType(resourceType);
}

//------------------------------------------------------------------------- Resource Template Search
//**************************************************************************************************
ZilchDefineType(ResourceTemplateSearch, builder, type)
{

}

//**************************************************************************************************
ResourceTemplateSearch::ResourceTemplateSearch(Composite* parent)
  : ColoredComposite(parent, Vec4(0.22f, 0.22f, 0.22f, 1.0f))
  , mManager(nullptr)
  , mTemplateCount(0)
{
  SetLayout(CreateStackLayout());

  // Name bar
  Widget* topBar = CreateAttached<Element>(cWhiteSquare);
  topBar->SetVisible(false);
  topBar->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  topBar->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(24.0f));
  topBar->SetNotInLayout(false);

  mImportButton = new ImportButton(this);
  mImportButton->SetTranslation(Pixels(12, 5, 0));
  mImportButton->SetActive(false);
  mImportButton->SetNotInLayout(true);
  ConnectThisTo(mImportButton, Events::LeftClick, OnImportClicked);

  mNoTemplateBackground = CreateAttached<Element>(cWhiteSquare);
  mNoTemplateBackground->SetColor(Vec4(1, 1, 1, 0.05f));
  mNoTemplateBackground->SetVisible(false);
  mNoTemplateBackground->SetNotInLayout(true);

  mNothingSelectedText = new Text(this, "NotoSans-Regular", 24);
  mNothingSelectedText->SetText("nothing selected...");
  mNothingSelectedText->SizeToContents();
  mNothingSelectedText->SetNotInLayout(true);
  mNothingSelectedText->SetColor(Vec4(1, 1, 1, 0.25f));

  mTemplateList = new ItemList(this, Pixels(60), 2);
  mTemplateList->SetSizing(SizePolicy::Flex, Vec2(1));
  ConnectThisTo(mTemplateList, Events::ItemSelected, OnTemplateSelected);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);

  ConnectThisTo(this, cFilesSelected, OnFilesSelected);
}

//**************************************************************************************************
void ResourceTemplateSearch::ShowTemplates(BoundType* resourceType)
{
  mTemplateCount = 0;
  mNothingSelectedText->SetVisible(false);

  // Disconnect from all previous modified events
  if (mManager)
  {
    forRange(Resource* resource, mManager->ResourceIdMap.Values())
    {
      if (ResourceTemplate* resourceTemplate = resource->GetResourceTemplate())
        DisconnectAll(resourceTemplate, this);
      if (ContentItem* contentItem = resource->mContentItem)
        DisconnectAll(contentItem, this);
    }
  }
  
  mManager = Z::gResources->GetResourceManager(resourceType);

  if(mManager->mCanAddFile)
  {
    mImportButton->SetActive(true);
    mImportButton->mText->SetText(BuildString("IMPORT ", resourceType->Name.ToUpper()));
    mImportButton->SizeToContents();
  }
  else
  {
    mImportButton->SetActive(false);
  }

  mTemplateList->Clear();
  
  bool templatesFound = false;

  // Connect to modified events so we can update 
  forRange(Resource* resource, mManager->ResourceIdMap.Values())
  {
    if (ResourceTemplate* resourceTemplate = resource->GetResourceTemplate())
    {
      templatesFound = true;
      ++mTemplateCount;

      String displayName = resourceTemplate->mDisplayName;
      
      if (displayName.Empty())
        displayName = resource->Name;

      String category = resourceTemplate->mCategory;
      uint categoryWeight = resourceTemplate->mCategorySortWeight;
      if (category.Empty())
      {
        category = "Templates";
        categoryWeight = 100;
      }

      // Add the group
      mTemplateList->AddGroup(category, categoryWeight);

      uint itemWeight = resourceTemplate->mSortWeight;
      Item* item = mTemplateList->AddItem(resource->Name, displayName, category, itemWeight);

      PreviewWidget* tileWidget = ResourcePreview::CreatePreviewWidget(item, displayName, resource);

      if (resourceType->IsA(ZilchTypeId(ColorGradient)))
      {
        item->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(10, 3)));
        tileWidget->SetSizing(SizePolicy::Flex, Vec2(1, 1));
      }
      else
      {
        tileWidget->SetSizing(SizePolicy::Fixed, Pixels(48, 48));
        tileWidget->mVerticalAlignment = VerticalAlignment::Center;

        // Move the label to the right of the preview widget
        item->mLabel->MoveToFront();
      }

      ConnectThisTo(resourceTemplate, Events::PropertyModified, OnContentComponentsChanged);
    }
  
    if (ContentItem* contentItem = resource->mContentItem)
      ConnectThisTo(contentItem, Events::ComponentsModified, OnContentComponentsChanged);
  }

  mNoTemplateBackground->SetVisible(false);
  if(templatesFound == false)
  {
    mNothingSelectedText->SetVisible(true);
    mNothingSelectedText->SetText("no templates available");
    mNothingSelectedText->SizeToContents();
    mNoTemplateBackground->SetVisible(true);
  }

  mTemplateList->Sort();
  mTemplateList->SelectFirstItem();
}

//**************************************************************************************************
Resource* ResourceTemplateSearch::GetSelectedResourceTemplate()
{
  String resourceName = mTemplateList->GetSelectedItem();
  return mManager->GetResource(resourceName, ResourceNotFound::ErrorFallback);
}

//**************************************************************************************************
void ResourceTemplateSearch::UpdateTransform()
{
  CenterToWindow(this, mNothingSelectedText, false);
  mNoTemplateBackground->SetTranslation(Pixels(0,24,0));
  mNoTemplateBackground->SetSize(mSize - Pixels(0, 24));
  ColoredComposite::UpdateTransform();
}

//**************************************************************************************************
void ResourceTemplateSearch::OnImportClicked(Event*)
{
  FileDialogConfig config;
  config.EventName = "cFilesSelected";
  config.CallbackObject = this;
  config.Title = String::Format("Add a %s", mManager->GetResourceType()->Name.c_str());

  forRange(FileDialogFilter& filter, mManager->mOpenFileFilters)
    config.mSearchFilters.PushBack(filter);

  config.Flags = FileDialogFlags::MultiSelect;
  Z::gEngine->has(OsShell)->OpenFile(config);
}

//**************************************************************************************************
void ResourceTemplateSearch::OnFilesSelected(OsFileSelection* e)
{
  // Don't do anything if we select nothing
  if (e->Files.Empty())
    return;

  OpenGroupImport(e->Files);
  CloseTabContaining(this);

  Z::gEditor->GetCenterWindow()->TryTakeFocus();
}

//**************************************************************************************************
bool ResourceTemplateSearch::TakeFocusOverride()
{
  mTemplateList->TakeFocus();
  return true;
}

//**************************************************************************************************
void ResourceTemplateSearch::OnTemplateSelected(Event*)
{
  Event e;
  DispatchEvent(Events::ResourceTemplateSelected, &e);
}

//**************************************************************************************************
void ResourceTemplateSearch::OnContentComponentsChanged(Event*)
{
  ShowTemplates(mManager->GetResourceType());
}

//**************************************************************************************************
void ResourceTemplateSearch::OnKeyDown(KeyboardEvent* e)
{
  if (e->Key == Keys::Tab)
  {
    if(e->ShiftPressed)
      mPreviousFocus->TakeFocus();
    else
      mNextFocus->TakeFocus();
  }
  else if(e->Key == Keys::Enter)
  {
    mNextFocus->TakeFocus();
  }
}

//------------------------------------------------------------------------ Resource Template Display
//**************************************************************************************************
ZilchDefineType(ResourceTemplateDisplay, builder, type)
{

}

//**************************************************************************************************
ResourceTemplateDisplay::ResourceTemplateDisplay(Composite* parent, PostAddOp& postAdd)
  : ColoredComposite(parent, Vec4(0.27f, 0.27f, 0.27f, 1.0f))
  , mPostAdd(postAdd)
  , mPreviewWidget(nullptr)
{
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 0), Thickness::cZero));

  Composite* topBar = new Composite(this);
  topBar->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  topBar->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(24));
  topBar->SetLayout(CreateFillLayout());
  {
    mTemplateName = new Label(topBar);
    mTemplateName->SetColor(Vec4(1, 1, 1, 0.5f));
    mTemplateName->mHorizontalAlignment = HorizontalAlignment::Center;
    mTemplateName->mVerticalAlignment = VerticalAlignment::Bottom;
    mTemplateName->SetNotInLayout(false);
  }

  mPreviewParent = new ColoredComposite(this, Vec4(0, 0, 0, 0.5f));
  mPreviewParent->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  mPreviewParent->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(216));
  mPreviewParent->SetLayout(CreateFillLayout());
  {
    mPreviewText = new Text(mPreviewParent, "NotoSans-Regular", 24);
    mPreviewText->SetText("preview");
    mPreviewText->SizeToContents();
    mPreviewText->SetNotInLayout(true);
    mPreviewText->SetColor(Vec4(1, 1, 1, 0.25f));
  }

  mDescription = new MultiLineText(this);
  mDescription->SetSizing(SizePolicy::Flex, 1.0f);
  mDescription->SetText("Template description");
  mDescription->SetColor(Vec4(1, 1, 1, 0.5f));
  mDescription->mBackground->SetActive(false);
  mDescription->mBorder->SetActive(false);

  ColoredComposite* name = new ColoredComposite(this, Vec4(1,1,1, 0.05f));
  name->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  name->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(44.0f));
  name->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(12, 0, 12, 5)));
  {
    Label* nameLabel = new Label(name);
    nameLabel->SetText("Name");
    nameLabel->SizeToContents();

    mNameField = new TextBox(name);
    mNameField->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
    mNameField->SetEditable(true);
    mNameField->SetEnterLoseFocus(false);

    ConnectThisTo(mNameField, Events::KeyDown, OnKeyDownNameField);
    ConnectThisTo(mNameField, Events::KeyUp, OnKeyUpNameField);
  }

  new Spacer(this, SizePolicy::Fixed, Pixels(0, 2));

  ColoredComposite* tags = new ColoredComposite(this, Vec4(1, 1, 1, 0.05f));
  tags->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  tags->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(44.0f));
  tags->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(12, 0, 12, 5)));
  {
    Label* tagsLabel = new Label(tags);
    tagsLabel->SetText("Tags");
    tagsLabel->SizeToContents();

    mTagsBox = new TextBox(tags);
    mTagsBox->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
    mTagsBox->SetEditable(true);

    ConnectThisTo(mTagsBox, Events::TextTyped, OnTextTypedTag);
    ConnectThisTo(mTagsBox, Events::KeyDown, OnKeyDownTagField);
    ConnectThisTo(mTagsBox->mEditTextField, Events::FocusGained, OnTagsFocusGained);
    ConnectThisTo(mTagsBox->mEditTextField, Events::FocusLost, OnTagsFocusLost);
  }

  new Spacer(this, SizePolicy::Fixed, Pixels(0, 2));

  ColoredComposite* library = new ColoredComposite(this, Vec4(1, 1, 1, 0.05f));
  library->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  library->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(44.0f));
  library->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(12, 0, 12, 5)));
  {
    Label* libraryLabel = new Label(library);
    libraryLabel->SetText("Library");
    libraryLabel->SizeToContents();

    mLibrarySelect = new StringComboBox(library);
  }

  new Spacer(this, SizePolicy::Fixed, Pixels(0, 2));

  Composite* buttonRow = new Composite(this);
  buttonRow->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(36));
  buttonRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(6, 0), Thickness(Pixels(12, 5, 12, 0))));
  {
    // Push everything right
    new Spacer(buttonRow, SizePolicy::Flex, Vec2(1));

    TextButton* createButton = new TextButton(buttonRow);
    createButton->SetText("CREATE");
    createButton->SetSizing(SizePolicy::Fixed, Pixels(61, 24));
    ConnectThisTo(createButton, Events::ButtonPressed, OnCreate);

    Widget* cancelButtonDummy = new Widget(buttonRow);
    cancelButtonDummy->SetSizing(SizePolicy::Fixed, Pixels(64, 24));
  }

  BuildContentLibraryList();

  // Hide the library group if there was only one library
  if(!mLibrarySelect->GetActive())
    library->SetActive(false);

  mDisabledCover = CreateAttached<Element>(cWhiteSquare);
  mDisabledCover->SetColor(Vec4(0, 0, 0, 0.20f));
  mDisabledCover->SetNotInLayout(true);
  mDisabledCover->SetActive(false);

  mCancelButton = new TextButton(this);
  mCancelButton->SetText("CANCEL");
  mCancelButton->SetNotInLayout(true);
  mCancelButton->SetSizing(SizePolicy::Fixed, Pixels(64, 24));
  ConnectThisTo(mCancelButton, Events::ButtonPressed, OnCancel);
}

//**************************************************************************************************
void ResourceTemplateDisplay::ShowResourceTemplate(Resource* resource)
{
  mSelectedTemplate = resource;

  if(resource)
  {
    mDisabledCover->SetActive(false);
    ResourceTemplate* resourceTemplate = resource->GetResourceTemplate();
    mDescription->SetText(resourceTemplate->mDescription);
    mDescription->mSize.x = mSize.x;
    mDescription->SizeToContents();
    mDescription->SetVisible(true);

    if (mPreviewWidget)
      mPreviewWidget->Destroy();

    mTemplateName->SetVisible(true);
    if(!resourceTemplate->mDisplayName.Empty())
      mTemplateName->SetText(resourceTemplate->mDisplayName);
    else
      mTemplateName->SetText(resource->Name);
    mTemplateName->SizeToContents();
    mTemplateName->SetSizing(SizePolicy::Fixed, mTemplateName->GetSize());

    mPreviewWidget = ResourcePreview::CreatePreviewWidget(mPreviewParent, resource->Name, resource);
    //mPreviewWidget->SetSizing(SizePolicy::Fixed, Pixels(48, 48));

    mPreviewWidget->SetSizing(SizePolicy::Flex, Vec2(1));
    if (ZilchVirtualTypeId(resource)->IsA(ZilchTypeId(ColorGradient)))
      mPreviewWidget->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(34));
    
    mPreviewWidget->mVerticalAlignment = VerticalAlignment::Center;
    mPreviewText->SetVisible(false);

    mNameField->SetReadOnly(false);
    mTagsBox->SetReadOnly(false);
  }
  else
  {
    mDisabledCover->SetActive(true);
    mPreviewText->SetVisible(true);
    mTemplateName->SetVisible(false);
    mNameField->SetReadOnly(true);
    mTagsBox->SetReadOnly(true);
    mDescription->SetVisible(false);
    
    if (mPreviewWidget)
      mPreviewWidget->Destroy();
    mPreviewWidget = nullptr;
  }
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
void ResourceTemplateDisplay::HidePreview()
{
  mPreviewParent->SetActive(false);
}


//**************************************************************************************************
bool ResourceTemplateDisplay::TakeFocusOverride()
{
  mNameField->TakeFocus();
  return true;
}

//**************************************************************************************************
void ResourceTemplateDisplay::UpdateTransform()
{
  CenterToWindow(mPreviewParent, mPreviewText, false);

  // Update tooltip position
  if(ToolTip* toolTip = mNameToolTip)
  {
    ToolTipPlacement placement;
    placement.SetScreenRect(mNameField->GetScreenRect());
    placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left,
                          IndicatorSide::Bottom, IndicatorSide::Top);
    toolTip->SetArrowTipTranslation(placement);
  }

  mDisabledCover->SetSize(GetSize());

  Vec2 cancelPos = mSize - mCancelButton->GetSize() - Pixels(12, 7);
  mCancelButton->SetTranslation(ToVector3(cancelPos));
  ColoredComposite::UpdateTransform();
}

//**************************************************************************************************
void ResourceTemplateDisplay::BuildContentLibraryList()
{
  bool devConfig = Z::gEngine->GetConfigCog()->has(Zero::DeveloperConfig);
  forRange(ResourceLibrary* resourceLibrary, Z::gResources->LoadedResourceLibraries.Values())
  {
    ContentLibrary* contentLibrary = Z::gContentSystem->Libraries.FindValue(resourceLibrary->Name, nullptr);
    if (contentLibrary == nullptr || (contentLibrary->GetReadOnly() && !devConfig))
      continue;
    
    mLibrarySelect->AddItem(resourceLibrary->Name);
  }

  // Only show the library select if there's  more than one
  if (mLibrarySelect->GetCount() > 1)
    mLibrarySelect->SetActive(true);
  else
    mLibrarySelect->SetActive(false);

  // Select the last one
  mLibrarySelect->SetSelectedItem(mLibrarySelect->GetCount() - 1, false);
}

//**************************************************************************************************
void ResourceTemplateDisplay::CreateNameToolTip(StringParam message)
{
  mNameToolTip.SafeDestroy();

  ToolTip* toolTip = new ToolTip(mParent->mParent->mParent->mParent);
  toolTip->SetText(message);
  toolTip->SetColorScheme(ToolTipColorScheme::Red);
  toolTip->SetDestroyOnMouseExit(false);

  ToolTipPlacement placement;
  placement.SetScreenRect(mNameField->GetScreenRect());
  placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left,
                        IndicatorSide::Bottom, IndicatorSide::Top);
  toolTip->SetArrowTipTranslation(placement);

  mNameToolTip = toolTip;

  mNameField->mBackgroundColor = ToByteColor(Vec4(0.49f, 0.21f, 0.21f, 1));
  mNameField->mBorderColor = ToByteColor(Vec4(0.49f, 0.21f, 0.21f, 1));
  mNameField->mFocusBorderColor = ToByteColor(Vec4(0.625f, 0.256f, 0.256f, 1));
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
void ResourceTemplateDisplay::RemoveNameToolTip()
{
  mNameToolTip.SafeDestroy();
  mNameField->SetStyle(TextBoxStyle::Classic);
}

//**************************************************************************************************
void ResourceTemplateDisplay::CreateTagToolTip(StringParam message, ToolTipColorScheme::Enum tagColor)
{
  mTagsToolTip.SafeDestroy();

  ToolTip* toolTip = new ToolTip(mTagsBox);
  toolTip->SetText(message);
  toolTip->SetDestroyOnMouseExit(false);
  toolTip->SetColorScheme(tagColor);

  ToolTipPlacement placement;
  placement.SetScreenRect(mTagsBox->GetScreenRect());
  placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left,
                        +IndicatorSide::Bottom, IndicatorSide::Top);
  toolTip->SetArrowTipTranslation(placement);

  // if this is a warning tooltip also color the tag text box red
  if (tagColor == ToolTipColorScheme::Red)
  {
    mTagsBox->mBackgroundColor = ToByteColor(Vec4(0.49f, 0.21f, 0.21f, 1));
    mTagsBox->mBorderColor = ToByteColor(Vec4(0.49f, 0.21f, 0.21f, 1));
    mTagsBox->mFocusBorderColor = ToByteColor(Vec4(0.625f, 0.256f, 0.256f, 1));
  }
  // otherwise set the tag box to its default selected state
  else 
  {
    mTagsBox->SetStyle(TextBoxStyle::Classic);
  }

  mTagsToolTip = toolTip;
}

//**************************************************************************************************
void ResourceTemplateDisplay::RemoveTagToolTip()
{
  mTagsToolTip.SafeDestroy();
  mTagsBox->SetStyle(TextBoxStyle::Classic);
}

//**************************************************************************************************
void ResourceTemplateDisplay::OnTextTypedTag(Event*)
{
  ValidateTags();
}

//**************************************************************************************************
void ResourceTemplateDisplay::OnKeyDownNameField(KeyboardEvent* e)
{
  if (e->Key == Keys::Enter)
    OnCreate(nullptr);
  else if (e->Key == Keys::Tab && e->ShiftPressed)
  {
    mNameField->LoseFocus();
    mPreviousFocus->TakeFocus();
  }
}

//**************************************************************************************************
void ResourceTemplateDisplay::OnKeyUpNameField(KeyboardEvent* e)
{
  if (e->Key != Keys::Enter)
    ValidateName(false);
}

//**************************************************************************************************
void ResourceTemplateDisplay::OnKeyDownTagField(KeyboardEvent* e)
{
  if (e->Key == Keys::Enter)
    OnCreate(nullptr);
}

//**************************************************************************************************
bool ResourceTemplateDisplay::ValidateName(bool finalValidation)
{
  // Can't do anything if there is no selected template
  if (mSelectedTemplate.IsNull())
    return false;

  ResourceManager* manager = mSelectedTemplate->GetManager();

  // Did they even specify a name?
  String resourceName = mNameField->GetText();
  if (resourceName.Empty())
  {
    if(finalValidation)
      CreateNameToolTip("You must assign a name for the Resource");
    else
      RemoveNameToolTip();
    return false;
  }

  // Check the resource manager for custom validation
  Status status;
  manager->ValidateRawName(status, resourceName, nullptr);

  if (status.Succeeded())
    manager->ValidateNewName(status, resourceName, nullptr);

  if (status.Failed())
  {
    CreateNameToolTip(status.Message);
    return false;
  }

  if(!finalValidation && resourceName.SizeInBytes() < 2)
  {
    RemoveNameToolTip();
    return false;
  }

  // Check to make sure it doesn't contain any invalid characters and meets our resource name
  // requirements
  if (!IsValidFilename(resourceName, status))
  {
    CreateNameToolTip(status.Message);
    return false;
  }
  
  // Is there already a resource under the same name?
  Resource* resource = manager->GetResource(resourceName, ResourceNotFound::ReturnNull);
  if (resource)
  {
    CreateNameToolTip("Name already in use");
    return false;
  }

  RemoveNameToolTip();
  return true;
}

//**************************************************************************************************
bool ResourceTemplateDisplay::ValidateTags()
{
  // Can't do anything if there is no selected template
  if (mSelectedTemplate.IsNull())
    return false;

  String tagString = mTagsBox->GetText();

  // replace tag delimiters that will be detected as invalid for our check
  tagString = tagString.Replace(",", "");
  tagString = tagString.Replace(" ", "");

  // check to see if the tag string changed after being sanitized, if so it was invalid
  String santiziedTags = Cog::SanitizeName(tagString);
  if(tagString != santiziedTags)
  {
    CreateTagToolTip("Tags contain invalid symbols", ToolTipColorScheme::Red);
    return false;
  }

  // let the user know how to specify a list of tags
  CreateTagToolTip("Tags are space or comma delimited");
  return true;
}

//**************************************************************************************************
void ResourceTemplateDisplay::OnCancel(Event*)
{
  Event event;
  GetDispatcher()->Dispatch(Events::AddWindowCancelled, &event);
  CloseTabContaining(this);

  Z::gEditor->GetCenterWindow()->TryTakeFocus();
}

//**************************************************************************************************
void ResourceTemplateDisplay::OnCreate(Event*)
{
  if (ValidateName(true) == false || ValidateTags() == false)
    return;
  
  Resource* resourceTemplate = mSelectedTemplate;
  if (resourceTemplate == nullptr)
    return;

  BoundType* resourceType = ZilchVirtualTypeId(resourceTemplate);
  ResourceManager* manager = Z::gResources->GetResourceManager(resourceType);

  String newName = mNameField->GetText();

  String libraryName = mLibrarySelect->GetSelectedString();
  ContentLibrary* library = Z::gContentSystem->Libraries.FindValue(libraryName, nullptr);

  //Attempt to add the resource with given name
  ResourceAdd resourceAdd;
  resourceAdd.Library = library;
  resourceAdd.Name = newName;
  resourceAdd.Template = resourceTemplate;
  AddNewResource(manager, resourceAdd);

  if (resourceAdd.WasSuccessful())
  {
    Z::gEditor->EditResource(resourceAdd.SourceResource);

    // If a post operation was set update the property
    forRange(Handle instance, mPostAdd.mObjects)
    {
      if (instance.IsNotNull())
      {
        Property* property = mPostAdd.mProperty.GetPropertyFromRoot(instance);

        if (property && ZilchVirtualTypeId(resourceAdd.SourceResource)->IsA(property->PropertyType))
          ChangeAndQueueProperty(Z::gEditor->GetOperationQueue(), instance, mPostAdd.mProperty, resourceAdd.SourceResource);
      }
    }

    // Add all the tags set in the add window on our new resource
    String tagList = mTagsBox->GetText().Replace(",", " ");
    StringSplitRange splitTags = tagList.Split(" ");
    HashSet<String> tags;
    forRange(StringRange r, splitTags)
    {
      // At this point all tags have already been validated
      if (!r.Empty())
        tags.Insert(r);
    }
    // Add all the tags from the resource template to the newly created resource
    resourceTemplate->GetTags(tags);
    // Set all the collected tags on the newly created resource
    resourceAdd.SourceResource->mContentItem->SetTags(tags);

    // Dispatch an event that the resource has been modified on the resource itself
    // and on the resource system
    ResourceEvent e;
    e.Manager = resourceTemplate->GetManager();
    e.EventResource = resourceTemplate;
    resourceTemplate->GetManager()->DispatchEvent(Events::ResourceTagsModified, &e);
    Z::gResources->DispatchEvent(Events::ResourceTagsModified, &e);

    PostAddResourceEvent eventToSend(mPostAdd, &resourceAdd);
    DispatchBubble(Events::PostAddResource, &eventToSend);
  }

  Z::gEditor->GetCenterWindow()->TryTakeFocus();

  RemoveTagToolTip();
  CloseTabContaining(this);
}

//**************************************************************************************************
void ResourceTemplateDisplay::OnTagsFocusGained(Event*)
{
  // create the appropriate tooltip when the tags box gains focus
  ValidateTags();
}

//**************************************************************************************************
void ResourceTemplateDisplay::OnTagsFocusLost(Event*)
{
  // if the tags are invalid do not remove the tooltip just because the user deselected it
  if (ValidateTags() == true)
    RemoveTagToolTip();
}

//------------------------------------------------------------------------ Resource Template Display
//**************************************************************************************************
ZilchDefineType(ImportButton, builder, type)
{
}

//**************************************************************************************************
ImportButton::ImportButton(Composite* parent)
  : ColoredComposite(parent, Vec4::cZero)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(4, 0), Thickness::cZero));

  mText = new Text(this, "NotoSans-Regular", 10);
  mText->SetText("IMPORT");
  mText->SetColor(Vec4(1, 1, 1, 0.85f));
  mText->SizeToContents();
  mText->SetNotInLayout(false);

  mIcon = CreateAttached<Element>("ImportIcon");
  mIcon->SetColor(Vec4(1, 1, 1, 0.85f));
  mIcon->SetTranslation(Pixels(46, 1, 0));
  mIcon->SetNotInLayout(false);
  mIcon->SetSizing(SizePolicy::Fixed, mIcon->GetMinSize());

  ConnectThisTo(this, Events::MouseEnterHierarchy, OnMouseEnter);
  ConnectThisTo(this, Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(this, Events::LeftMouseUp, OnLeftMouseUp);
  ConnectThisTo(this, Events::MouseExitHierarchy, OnMouseExit);
}

//**************************************************************************************************
void ImportButton::OnMouseEnter(Event*)
{
  mText->SetColor(Vec4(1));
  mIcon->SetColor(Vec4(1));
}

//**************************************************************************************************
void ImportButton::OnLeftMouseDown(Event*)
{
  mText->SetColor(Vec4(1, 1, 1, 0.7f));
  mIcon->SetColor(Vec4(1, 1, 1, 0.7f));
}

//**************************************************************************************************
void ImportButton::OnLeftMouseUp(Event*)
{
  mText->SetColor(Vec4(1));
  mIcon->SetColor(Vec4(1));
}

//**************************************************************************************************
void ImportButton::OnMouseExit(Event*)
{
  mText->SetColor(Vec4(1, 1, 1, 0.85f));
  mIcon->SetColor(Vec4(1, 1, 1, 0.85f));
}

} // namespace Zero
