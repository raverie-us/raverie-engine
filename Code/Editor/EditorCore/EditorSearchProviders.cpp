// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

static const String cResourcesTag = "Resources";

ResourceSearchProvider::ResourceSearchProvider(ResourceLibrary* library, bool showHidden, ResourceLibrary* defaultLib) :
    SearchProvider("Resource"),
    mDefaultLibrary(defaultLib),
    mResourceLibrary(library),
    mShowHidden(showHidden)
{
}

void ResourceSearchProvider::RunCommand(SearchView* searchView, SearchViewResult& element)
{
  // When selected edit the resource
  Resource* resource = (Resource*)element.Data;
  Z::gEditor->EditResource(resource);
}

void ResourceSearchProvider::Search(SearchData& search)
{
  // allResources is active if the active tags contain only "Resources"
  // so all resources and tags should be added
  bool resourcesTag = search.ActiveTags.Contains(cResourcesTag);

  // Copy tags to a local map so matched values can be removed
  // for quick filtering
  HashSet<String> localTags = search.ActiveTags;
  // If ResourcesTag is present remove from local
  localTags.Erase(cResourcesTag);
  // Also remove the currently selected library tag, if there is one.
  if (mResourceLibrary != nullptr)
    localTags.Erase(BuildString(mResourceLibrary->Name, "(Library)"));

  if (search.ActiveTags.Empty())
    search.AvailableTags.Insert(cResourcesTag);

  // For every resource manager
  ResourceSystem::ManagerMapType::valuerange r = Z::gResources->Managers.Values();
  for (; !r.Empty(); r.PopFront())
  {
    ResourceManager* resourceManager = r.Front();

    // Special resource managers that always add a tag if no tags are active
    bool globalSearchable = resourceManager->mSearchable;

    if (resourceManager->mHidden)
      continue;

    // If any meta types were specified, check if the resource type
    // of this manager is a matching type
    if (search.ActiveMeta.Size() != 0)
    {
      bool metaMatch = false;
      BoundType* resourceType = resourceManager->mResourceType;
      forRange (BoundType* meta, search.ActiveMeta.All())
      {
        if (resourceType->IsA(meta))
          metaMatch = true;
      }

      if (metaMatch == false)
        continue;
    }

    // Add all resources in manager
    if (resourcesTag || globalSearchable)
    {
      // For every resource in the manager
      forRange (Resource* resource, resourceManager->ResourceIdMap.Values())
      {
        AttemptAddResource(search, localTags, resource);
      }
    }
  }
}

void ResourceSearchProvider::AttemptAddResource(SearchData& search, HashSet<String>& localTags, Resource* resource)
{
  if (mResourceLibrary != nullptr && resource->mResourceLibrary != mResourceLibrary)
    return;

  if (resource->mContentItem == nullptr)
    return;

  bool hasDefault = mDefaultLibrary != nullptr;
  bool hasTarget = mResourceLibrary != nullptr;
  bool hasOverride = resource->mContentItem->ShowInEditor;

  // If there is no default library then showing hidden items is valid.
  bool canShow = (mShowHidden && !hasDefault);
  // Showing hidden items is also valid if a library is in focus.
  canShow |= (mShowHidden && hasTarget);
  // Showing hidden items is not valid when a default library is the only
  // library available.  Further, The only way to show a non-default library
  // item requires an explicit override.
  canShow |= (hasDefault && !hasTarget && hasOverride);
  // No library available, so fall back to the override.
  canShow |= (!hasDefault && !hasTarget && hasOverride);

  if (!canShow)
    return;

  // Get the tags for the resource
  HashSet<String> resourceTags;
  resource->GetTags(resourceTags);

  if (CheckTags(localTags, resourceTags))
  {
    resource->AddTags(search.AvailableTags);

    // Match on the name
    int priority = PartialMatch(search.SearchString.All(), resource->Name.All(), CaseInsensitiveCompare);
    if (priority != cNoMatch)
    {
      // Add a result
      SearchViewResult& result = search.Results.PushBack();
      result.Data = resource;
      result.Interface = this;
      result.Name = resource->Name;
      result.Priority = priority;

      if (FilterResult(result) == false)
        search.Results.PopBack();
    }
  }
}

Composite* ResourceSearchProvider::CreatePreview(Composite* parent, SearchViewResult& element)
{
  // Use the general resource preview
  Resource* resource = (Resource*)element.Data;
  PreviewWidget* preview =
      ResourcePreview::CreatePreviewWidget(parent, resource->Name, resource, PreviewImportance::High);

  if (preview)
  {
    preview->AnimatePreview(PreviewAnimate::Always);

    if (element.mStatus.Failed())
    {
      // Group the error text and the preview widget
      Composite* group = new Composite(parent);
      group->SetLayout(CreateStackLayout());

      // Create the error text on top
      MultiLineText* text = (MultiLineText*)CreateTextPreview(group, element.mStatus.Message);

      // Defer border-display to the parent's border
      if (text != nullptr)
        text->mBorder->SetVisible(false);

      group->AttachChildWidget(preview);

      return group;
    }

    return preview;
  }
  else if (element.mStatus.Failed())
  {
    return CreateTextPreview(parent, element.mStatus.Message);
  }

  return nullptr;
}

String ResourceSearchProvider::GetElementType(SearchViewResult& element)
{
  Resource* resource = (Resource*)element.Data;
  return resource->GetManager()->GetResourceType()->Name;
}

LibrarySearchProvider::LibrarySearchProvider(bool canReturnResources, ResourceLibrary* defaultLibrary) :
    SearchProvider("Library"),
    mCanReturnResources(canReturnResources),
    mDefaultLibrary(defaultLibrary),
    mTargetResourceProvider(nullptr, true, defaultLibrary),
    mLibraries(Z::gContentSystem->Libraries)
{
}

bool LibrarySearchProvider::OnMatch(SearchView* searchView, SearchViewResult& element)
{
  ResourceLibrary* library = (ResourceLibrary*)element.Data;
  mTargetResourceProvider.mResourceLibrary = library;

  if (library == nullptr)
  {
    mActiveLibrary.Clear();
    return false;
  }

  mActiveLibrary = GetElementNameAndSearchType(element);

  // Create a UI Tag element to show that subsequent search results will
  // be in the context of the selected library, only.
  searchView->AddTag(mActiveLibrary);

  // Do not close the SearchView.
  return false;
}

void LibrarySearchProvider::RunCommand(SearchView* searchView, SearchViewResult& element)
{
  mTargetResourceProvider.RunCommand(searchView, element);
}

void LibrarySearchProvider::Search(SearchData& search)
{
  bool libraryActive = search.ActiveTags.Contains(mActiveLibrary);

  if (!libraryActive)
  {
    mActiveLibrary.Clear();
    mTargetResourceProvider.mResourceLibrary = nullptr;
  }

  if (mCanReturnResources)
    mTargetResourceProvider.Search(search);

  // Don't include other libraries if there's already one selected.
  if (libraryActive)
    return;

  forRange (ContentLibrary* library, mLibraries.Values())
  {
    String& name = library->Name;
    if (name == "ZeroLauncherResources")
      continue;

    // Match on the name
    int priority = PartialMatch(search.SearchString.All(), name.All(), CaseInsensitiveCompare);
    if (priority != cNoMatch)
    {
      // Add a result
      SearchViewResult& result = search.Results.PushBack();
      result.Data = Z::gResources->GetResourceLibrary(library->Name);
      result.Interface = this;
      result.Name = library->Name;
      result.Priority = priority + SearchViewResultPriority::LibraryBegin;
    }
  }
}

String LibrarySearchProvider::GetElementType(SearchViewResult& element)
{
  const String type = "Library";
  return type;
}

ObjectSearchProvider::ObjectSearchProvider() : SearchProvider("Object")
{
}

void ObjectSearchProvider::RunCommand(SearchView* searchView, SearchViewResult& element)
{
  // Focus on the object when selected
  if (Cog* cog = element.ObjectHandle.Get<Cog*>())
  {
    MetaSelection* select = Z::gEditor->GetSelection();
    select->SelectOnly(cog);
    FocusOnSelectedObjects();
    select->FinalSelectionChanged();
  }
}

/// Add an object the search results
void ObjectSearchProvider::AddObject(Cog& object, SearchData& search)
{
  String name = object.GetName();
  if (!name.Empty())
  {
    // Filter the name
    int priority = PartialMatch(search.SearchString.All(), name.All(), CaseInsensitiveCompare);
    if (priority != cNoMatch)
    {
      // Add a result
      SearchViewResult& result = search.Results.PushBack();
      result.ObjectHandle = &object;
      result.Interface = this;
      result.Name = name;
      result.Priority = priority;
    }
  }
}

void ObjectSearchProvider::Search(SearchData& search)
{
  // Check for objects tag
  const String ObjectsTag = "Objects";
  if (!CheckAndAddSingleTag(search, ObjectsTag))
    return;

  // Search all object in this space
  Space* space = Z::gEditor->GetEditSpace();
  if (space)
  {
    AddObject(*space, search);

    forRange (Cog& object, space->AllObjects())
    {
      AddObject(object, search);
    }
  }
}

Composite* ObjectSearchProvider::CreatePreview(Composite* parent, SearchViewResult& element)
{
  // Commented out for the time being as creating a preview for cogs in the
  // scene when using general search moves the object and creates a new camera
  // in the scene that is visibly seen coming into and out of existence in both
  // the scene and object view - Dane Curbow

  // Use camera preview
  //     if (Cog* cog = element.ObjectHandle.Get<Cog*>())
  //       return ResourcePreview::CreatePreviewWidget(parent, cog->GetName(),
  //       cog);
  //     else
  return nullptr;
}

String ObjectSearchProvider::GetElementType(SearchViewResult& element)
{
  const String ObjectName = "Cog";
  return ObjectName;
}

ComponentSearchProvider::ComponentSearchProvider(HandleParam object, HandleOf<MetaComposition>& composition) :
    SearchProvider("Component"),
    mObject(object),
    mComposition(composition)
{
  mResultsContainExactMatch = false;
}

void ComponentSearchProvider::Search(SearchData& search)
{
  // Dereference the handle and get the object
  if (mObject.IsNull())
    return;

  mResultsContainExactMatch = false;

  // Enumerate all possible types that can be added to this composition
  Array<BoundType*> types;
  mComposition->Enumerate(types, EnumerateAction::All, mObject);

  forRange (BoundType* boundType, types.All())
  {
    if (CheckAndAddTags(search, boundType))
    {
      int priority = PartialMatch(search.SearchString.All(), boundType->Name.All(), CaseInsensitiveCompare);
      if (priority != cNoMatch)
      {
        mResultsContainExactMatch |= (priority == cExactMatch);

        // Add a match
        SearchViewResult& result = search.Results.PushBack();
        result.Data = (void*)boundType;
        result.Interface = this;
        result.Name = boundType->Name;
        result.Priority = priority;

        AddInfo addInfo;
        if (mComposition->CanAddComponent(mObject, boundType, &addInfo) == false)
          result.mStatus.SetFailed(addInfo.Reason);
      }
    }
  }
}

String ComponentSearchProvider::GetElementType(SearchViewResult& element)
{
  return String();
}

Composite* ComponentSearchProvider::CreatePreview(Composite* parent, SearchViewResult& element)
{
  // For preview attempt to look up class description from documentation system.
  BoundType* boundType = (BoundType*)element.Data;
  ClassDoc* classDoc = Z::gDocumentation->mClassMap.FindValue(boundType->Name, NULL);

  // Try to include class documentation in the preview.
  if (classDoc)
  {
    String& description = classDoc->mDescription;

    if (element.mStatus.Failed())
    {
      // Group the error text and description text
      Composite* group = new Composite(parent);
      group->SetLayout(CreateStackLayout());

      // Create the error text on top
      MultiLineText* text = (MultiLineText*)CreateTextPreview(group, element.mStatus.Message);
      // Defer border-display to the parent's border.
      if (text != nullptr)
        text->mBorder->SetVisible(false);

      if ((text = (MultiLineText*)CreateTextPreview(group, description)))
      {
        // Gray-scale with less alpha to inherit some of the parent's display
        // color.
        text->mTextField->SetColor(Vec4(1, 1, 1, 0.35f));
        // Defer border-display to the parent's border.
        text->mBorder->SetVisible(false);
      }

      return group;
    }
    else
    {
      return CreateTextPreview(parent, description);
    }
  }
  // No class documentation.  So, only create a text preview if there's a
  // valid 'failed' message to display.
  else if (element.mStatus.Failed())
  {
    return CreateTextPreview(parent, element.mStatus.Message);
  }

  return nullptr;
}

bool ComponentSearchProvider::AddToAlternatePreview(SearchData* search, Composite* searchPreviewWidget)
{
  mSearchType.Clear();

  // Cannot offer to create a ZilchComponent if there is already a component
  // with a name that matches the 'SearchString'.
  if (mResultsContainExactMatch)
    return false;

  mSearchType = search->SearchString;

  String message = BuildString("Component '", mSearchType, "' could not be found. Press Alt + Enter to create it.");
  MultiLineText* text = (MultiLineText*)CreateTextPreview(searchPreviewWidget, message);

  // Defer border-display to the parent's border.
  if (text != nullptr)
    text->mBorder->SetVisible(false);

  return true;
}

void ComponentSearchProvider::AttemptAlternateSearchCompleted()
{
  // Cannot create a ZilchComponent if there is already a component with
  // a name that matches the search or if there is no search string.
  if (mResultsContainExactMatch || mSearchType.Empty())
    return;

  bool altPressed = Keyboard::Instance->KeyIsDown(Keys::Alt);
  bool ctrlPressed = Keyboard::Instance->KeyIsDown(Keys::Control);
  bool shiftPressed = Keyboard::Instance->KeyIsDown(Keys::Shift);

  // Alt is the only modifier key allowed for the command
  if (!altPressed || ctrlPressed || shiftPressed)
    return;

  AlternateSearchCompletedEvent eventToSend;
  eventToSend.mSearchText = mSearchType;

  DispatchEvent(Events::AlternateSearchCompleted, &eventToSend);
}

SearchProvider* GetLibrarySearchProvider(bool canReturnResources, ResourceLibrary* defaultLibrary)
{
  return new LibrarySearchProvider(canReturnResources, defaultLibrary);
}

SearchProvider* GetObjectSearchProvider()
{
  return new ObjectSearchProvider();
}

SearchProvider* GetResourceSearchProvider(ResourceLibrary* resourceLibrary, bool showHidden)
{
  return new ResourceSearchProvider(resourceLibrary, showHidden);
}

SearchProvider* GetFactoryProvider(HandleParam object, HandleOf<MetaComposition>& composition)
{
  return new ComponentSearchProvider(object, composition);
}

void AddEditorProviders(SearchData& search)
{
  ResourceLibrary* library = Z::gResources->GetResourceLibrary(Z::gEditor->mProjectLibrary->Name);
  search.SearchProviders.PushBack(GetLibrarySearchProvider(true, library));

  search.SearchProviders.PushBack(GetObjectSearchProvider());

  CommandManager* commandManager = CommandManager::GetInstance();
  search.SearchProviders.PushBack(commandManager->GetCommandSearchProvider());
}

} // namespace Zero
