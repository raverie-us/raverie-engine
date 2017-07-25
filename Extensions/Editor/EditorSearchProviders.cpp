///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorSearchProviders.cpp
/// Support for searching
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const String ResourcesTag = "Resources";

/// Search provider for resources
class ResourceSearchProvider : public SearchProvider
{
public:
  ResourceLibrary* mResourceLibrary;
  Handle mObject;
  bool mShowHidden;
  ResourceSearchProvider(ResourceLibrary* library, bool showHidden = false) :
    mResourceLibrary(library), 
    mShowHidden(showHidden)
  {
  }

  void RunCommand(SearchView* searchView, SearchViewResult& element) override
  {
    // When selected edit the resource
    Resource* resource = (Resource*)element.Data;
    Z::gEditor->EditResource(resource);
  }

  void Search(SearchData& search) override
  {
    // allResources is active if the active tags contain only "Resources"
    // so all resources and tags should be added
    bool resourcesTag = search.ActiveTags.Contains(ResourcesTag);

    // Copy tags to a local map so matched values can be removed
    // for quick filtering
    HashSet<String> localTags = search.ActiveTags;
    // If ResourcesTag is present remove from local
    localTags.Erase(ResourcesTag);

    if(search.ActiveTags.Empty())
      search.AvailableTags.Insert(ResourcesTag);

    // For every resource manager
    ResourceSystem::ManagerMapType::valuerange r = Z::gResources->Managers.Values();
    for(; !r.Empty(); r.PopFront())
    {
      ResourceManager* resourceManager = r.Front();

      // Special resource managers that always add a tag if no tags are active
      bool globalSearchable = resourceManager->mSearchable;

      if(resourceManager->mHidden)
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
      if(resourcesTag || globalSearchable)
      {
        // For every resource in the manager
        forRange(Resource* resource, resourceManager->ResourceIdMap.Values())
        {
          AttemptAddResource(search, localTags, resource);
        }
      }
    }
  }

  void AttemptAddResource(SearchData& search, HashSet<String>& localTags, Resource* resource)
  {
    if(mResourceLibrary != NULL && resource->mResourceLibrary != mResourceLibrary)
      return;

    if(resource->mContentItem == NULL)
      return;

    // Don't show hidden resources unless otherwise specified
    if(!mShowHidden && !resource->mContentItem->ShowInEditor)
      return;

    // Get the tags for the resource
    HashSet<String> resourceTags;
    resource->GetTags(resourceTags);

    if(CheckTags(localTags, resourceTags))
    {
      resource->AddTags(search.AvailableTags);

      // Match on the name
      int priority = PartialMatch(search.SearchString.All(), resource->Name.All(), CaseInsensitiveCompare);
      if(priority != cNoMatch)
      {
        // Add a result
        SearchViewResult& result = search.Results.PushBack();
        result.Data = resource;
        result.Interface = this;
        result.Name = resource->Name;
        result.Priority = priority;
      }
    }
  }

  Composite* CreatePreview(Composite* parent, SearchViewResult& element) override
  {
    // Use the general resource preview
    Resource* resource = (Resource*)element.Data;
    PreviewWidget* preview = ResourcePreview::CreatePreviewWidget(parent, resource->Name, resource);
    if (preview)
      preview->AnimatePreview(PreviewAnimate::Always);
    return preview;
  }

  String GetType(SearchViewResult& element) override
  {
    Resource* resource = (Resource*)element.Data;
    return resource->GetManager()->GetResourceType()->Name;
  }
};

// Search Provider for Objects in the Editor space
class ObjectSearchProvider : public SearchProvider
{
public:
  void RunCommand(SearchView* searchView, SearchViewResult& element) override
  {
    // Focus on the object when selected
    if(Cog* cog = element.ObjectHandle.Get<Cog*>())
    {
      MetaSelection* select = Z::gEditor->GetSelection();
      select->SelectOnly(cog);
      FocusOnSelectedObjects();
    }
  }

  /// Add an object the search results
  void AddObject(Cog& object, SearchData& search)
  {
    String name = object.GetName();
    if(!name.Empty())
    {
      // Filter the name
      int priority = PartialMatch(search.SearchString.All(), name.All(), CaseInsensitiveCompare);
      if(priority != cNoMatch)
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

  void Search(SearchData& search) override
  {
    // Check for objects tag
    const String ObjectsTag = "Objects";
    if(!CheckAndAddSingleTag(search, ObjectsTag))
      return;

    // Search all object in this space
    Space* space = Z::gEditor->GetEditSpace();
    if(space)
    {
      AddObject(*space, search);

      forRange(Cog& object, space->AllObjects())
      {
        AddObject(object, search);
      }
    }

  }

  Composite* CreatePreview(Composite* parent, SearchViewResult& element) override
  {
    // Commented out for the time being as creating a preview for cogs in the scene
    // when using general search moves the object and creates a new camera in the scene
    // that is visibly seen coming into and out of existence in both the scene and object view - Dane Curbow

    // Use camera preview
//     if (Cog* cog = element.ObjectHandle.Get<Cog*>())
//       return ResourcePreview::CreatePreviewWidget(parent, cog->GetName(), cog);
//     else
      return nullptr;
  }

  String GetType(SearchViewResult& element) override
  {
    const String ObjectName = "Cog";
    return ObjectName;
  }
};

/// Search Provider for Components to add to compositions
/// using MetaComposition on MetaType
class ComponentSearchProvider : public SearchProvider
{
public:
  // Object to check for components to add.
  HandleOf<MetaComposition> mComposition;
  Handle mObject;

  void Search(SearchData& search) override
  {
    //Deference the handle and get the object
    if(mObject.IsNull())
      return;

    // Enumerate all possible types that can be added to this composition
    Array<BoundType*> types;
    mComposition->Enumerate(types, EnumerateAction::All, mObject);

    forRange(BoundType* boundType, types.All())
    {
      // Match valid tags
      if (CheckAndAddTags(search, boundType))
      {
        int priority = PartialMatch(search.SearchString.All(), boundType->Name.All(), CaseInsensitiveCompare);
        if (priority != cNoMatch)
        {
          //Add a match
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

  String GetType(SearchViewResult& element) override
  {
    return String();
  }

  Composite* CreatePreview(Composite* parent, SearchViewResult& element) override
  {
    //For preview attempt to look up class description from documentation system;
    BoundType* boundType = (BoundType*)element.Data;
    ClassDoc* classDoc = Z::gDocumentation->mClassMap.FindValue(boundType->Name, NULL);

    // Only bother showing the preview if the class documentation exists
    if(classDoc)
    {
      String &description = classDoc->mDescription;

      if(element.mStatus.Failed())
      {
        // Group the error text and description text
        Composite* group = new Composite(parent);
        group->SetLayout(CreateStackLayout());
        
        // Create the error text on top
        CreateTextPreview(group, element.mStatus.Message);

        // Create a grayed out description text below
        MultiLineText* text = (MultiLineText*)CreateTextPreview(group, description);
        text->mTextField->SetColor(Vec4(1,1,1,0.35f));

        return group;
      }
      else
      {
        return CreateTextPreview(parent, description);
      }
    }
    // if we had no class documentation
    else
    {
      // only create a text preview if we have a 'failed' message to display
      if(element.mStatus.Failed())
        return CreateTextPreview(parent, element.mStatus.Message);
      return NULL;
    }
  }
};

SearchProvider* GetObjectSearchProvider()
{
  return new ObjectSearchProvider();
}

SearchProvider* GetResourceSearchProvider(ResourceLibrary* resourceLibrary, bool showHidden)
{
  return new ResourceSearchProvider(resourceLibrary, showHidden);
}

SearchProvider* GetResourceSearchProvider(HandleParam object)
{
  ResourceSearchProvider* provider = new ResourceSearchProvider(NULL);
  provider->mObject = object;
  return provider;
}

SearchProvider* GetFactoryProvider(HandleParam object, HandleOf<MetaComposition>& composition)
{
  ComponentSearchProvider* provider = new ComponentSearchProvider();
  provider->mObject = object;
  provider->mComposition = composition;
  return provider;
}

void AddEditorProviders(SearchData& search)
{
  search.SearchProviders.PushBack(GetResourceSearchProvider(NULL));
  search.SearchProviders.PushBack(GetObjectSearchProvider());
}

}//namespace Zero
