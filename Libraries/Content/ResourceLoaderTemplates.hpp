// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Resource loader for engine's data file format (.data)
template <typename ResourceMananger, DataFileFormat::Enum defaultFormat>
class GenericDataLoader : public ResourceLoader
{
public:
  typedef typename ResourceMananger::ResourceType ResourceType;

  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override
  {
    // try to create a new resource from a data file
    ResourceType* resource = new ResourceType();
    resource->mContentItem = entry.mLibrarySource;
    if (LoadFromDataFile(*resource, entry.FullPath, defaultFormat, false))
    {
      if (entry.mBuilder)
        resource->FilterTag = entry.mBuilder->GetTag();

      resource->Name = entry.Name;
      resource->Initialize();
      ResourceMananger::GetInstance()->AddResource(entry, resource);
      return resource;
    }
    // if failed, we have to release the memory that was allocated
    else
    {
      delete resource;
      return nullptr;
    }
  }

  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override
  {
    ResourceType* resource = new ResourceType();
    resource->mContentItem = entry.mLibrarySource;

    LoadFromDataBlock(*resource, entry.Block, defaultFormat);
    resource->Initialize();

    ResourceMananger::GetInstance()->AddResource(entry, resource);

    return resource;
  }

  void ReloadFromFile(Resource* resourceToReload, ResourceEntry& entry) override
  {
    ResourceType* resource = (ResourceType*)resourceToReload;
    resource->Unload();
    LoadFromDataFile(*resource, entry.FullPath, defaultFormat, false);
    resource->Initialize();
  }
};

template <typename ResourceMananger>
class ObjectInheritanceFileLoader : public ResourceLoader
{
public:
  typedef typename ResourceMananger::ResourceType ResourceType;

  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override
  {
    // try to create a new resource from a data file
    ResourceType* resource = new ResourceType();
    resource->mContentItem = entry.mLibrarySource;
    // if(LoadFromDataFile(*resource, entry.FullPath, defaultFormat, false))
    if (LoadResource(resource, entry))
    {
      if (entry.mBuilder)
        resource->FilterTag = entry.mBuilder->GetTag();

      resource->Name = entry.Name;
      resource->Initialize();
      ResourceMananger::GetInstance()->AddResource(entry, resource);
      return resource;
    }
    // if failed, we have to release the memory that was allocated
    else
    {
      delete resource;
      return nullptr;
    }
  }

  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override
  {
    // ResourceType* resource = new ResourceType();
    // resource->mContentItem = entry.mLibrarySource;
    //
    // LoadFromDataBlock(*resource, entry.Block, defaultFormat);
    // resource->Initialize();
    //
    // ResourceMananger::GetInstance()->AddResource(entry, resource);
    //
    // return resource;

    return nullptr;
  }

  void ReloadFromFile(Resource* resourceToReload, ResourceEntry& entry) override
  {
    ResourceType* resource = (ResourceType*)resourceToReload;
    resource->Unload();
    LoadResource(resource, entry);
    // LoadFromDataFile(*resource, entry.FullPath, defaultFormat, false);
    resource->Initialize();
  }

  bool LoadResource(Resource* resource, ResourceEntry& entry)
  {
    ObjectLoader loader;
    Status status;
    loader.OpenFile(status, entry.FullPath);

    if (status.Failed())
      return false;

    PolymorphicNode node;
    loader.GetPolymorphic(node);

    resource->Serialize(loader);

    loader.EndPolymorphic();

    // Record all modifications
    loader.RecordModifications(resource);

    return true;
  }
};

template <typename ResourceMananger>
class TextDataFileLoader : public GenericDataLoader<ResourceMananger, DataFileFormat::Text>
{
public:
};

template <typename ResourceMananger>
class BinaryDataFileLoader : public GenericDataLoader<ResourceMananger, DataFileFormat::Binary>
{
public:
};

template <typename ResourceMananger, typename LoadPattern>
class ChunkFileLoader : public ResourceLoader
{
public:
  typedef typename ResourceMananger::ResourceType ResourceType;

  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override
  {
    ResourceType* newResource = new ResourceType();

    ChunkFileReader reader;
    reader.Open(entry.FullPath);

    LoadPattern::Load(newResource, reader);

    ResourceMananger::GetInstance()->AddResource(entry, newResource);

    reader.Close();
    return newResource;
  }

  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override
  {
    ResourceType* newResource = new ResourceType();

    ChunkBufferReader reader;
    reader.Open(entry.Block);

    LoadPattern::Load(newResource, reader);

    ResourceMananger::GetInstance()->AddResource(entry, newResource);

    return newResource;
  }

  void ReloadFromFile(Resource* resource, ResourceEntry& entry) override
  {
    ResourceType* newResource = (ResourceType*)resource;
    newResource->Unload();

    ChunkFileReader reader;
    reader.Open(entry.FullPath);

    LoadPattern::Load(newResource, reader);

    resource->SendModified();
  }
};

} // namespace Zero
