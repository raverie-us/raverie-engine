///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------SimpleResourceFactory
// A simple resource factory that wraps a lot of functionality of MetaComposition by expecting the 
// resource to have some basic functions. These functions are:
// uint GetSize()
// HandleOf<BlockType> GetBlockAt(uint index)
// HandleOf<BlockType> GetById(BoundType* type)
// void Add(const HandleOf<BlockType>& block)
// void Remove(const HandleOf<BlockType>& block)
template <typename ResourceType, typename BlockType>
class SimpleResourceFactory : public MetaComposition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  SimpleResourceFactory() : MetaComposition(ZilchTypeId(BlockType)), mDeleteMemory(false) {};

  SimpleResourceFactory(bool deleteMemory) : MetaComposition(ZilchTypeId(BlockType))
  {
    mDeleteMemory = deleteMemory;
  }
  
  // MetaComposition Interface.

  // Functions for accessing/removing/adding "components" (blocks) dynamically
  uint GetComponentCount(HandleParam owner) override
  {
    //just return the number of blocks we have
    ResourceType* resource = owner.Get<ResourceType*>(GetOptions::AssertOnNull);
    return resource->GetSize();
  }

  Handle GetComponent(HandleParam instance, BoundType* componentType) override
  {
    //there's no easy way to turn the id to a block without a linear search,
    //luckily there aren't many block types so it doesn't matter now
    ResourceType* resource = instance.Get<ResourceType*>(GetOptions::AssertOnNull);
    BlockType* block = resource->GetById(componentType);
    if(block != nullptr)
      return block;
    return Handle();
  }

  Handle GetComponentAt(HandleParam owner, uint index) override
  {
    //convert the instance to a filter and grab the block at the given index
    ResourceType* resource = owner.Get<ResourceType*>(GetOptions::AssertOnNull);
    BlockType* block = resource->GetBlockAt(index);
    if(block != nullptr)
      return block;
    return Handle();
  }

  Handle MakeObject(BoundType* typeToCreate) override
  {
    ReturnIf(typeToCreate->IsA(mComponentType) == false, Handle(), "Incorrect component type");

    if(typeToCreate)
      return AllocateBlock(typeToCreate, true);
    return Handle();
  }

  BoundType* MakeProxy(StringParam typeName, ProxyReason::Enum reason) override
  {
    return ProxyObject<BlockType>::CreateProxyMetaFromFile(typeName, reason);
  }

  void AddComponent(HandleParam owner, HandleParam component, int index = -1,
                    bool ignoreDependencies = false) override
  {
    // Add the block to the filter
    ResourceType* resource = owner.Get<ResourceType*>(GetOptions::AssertOnNull);
    resource->Add(component, index);
  }

  void RemoveComponent(HandleParam owner, HandleParam subObject, bool ignoreDependencies = false) override
  {
    ResourceType* resource = owner.Get<ResourceType*>(GetOptions::AssertOnNull);

    bool success = resource->Remove(subObject);
    ErrorIf(success == false, "Failed to remove block");

    // If this composition is in charge of managing the block's memory then it should delete it.
    // The main exception for this are reference counted types, but ThreadSafeHandle/Pointer managers should probably delete the memory.
    if(mDeleteMemory)
    {
      BlockType* block = subObject.Get<BlockType*>();
      delete block;
    }
  }

  // If the given block requests to be setup via default serialization then run that
  void SetupBlock(Handle& handle, BoundType* blockMeta)
  {
    BlockType* block = handle.Get<BlockType*>();
    // This should probably be done with SFINAE at some point
    CogComponentMeta* metaComponent = blockMeta->HasInherited<CogComponentMeta>();
    if(metaComponent != nullptr)
    {
      SetupMode::Enum constructionMode = metaComponent->mSetupMode;
      if(constructionMode == SetupMode::DefaultSerialization)
      {
        DefaultSerializer defaultSerializer;
        block->Serialize(defaultSerializer);
      }
    }
  }

  Handle AllocateBlock(BoundType* blockMeta, bool runSetup)
  {
    Handle handle = ZilchAllocate(BlockType, blockMeta);

    // Run default serialization if necessary
    if(runSetup)
      SetupBlock(handle, blockMeta);

    return handle;
  }

  void SerializeArray(Serializer& stream, Array<BlockType*>& dataArray)
  {
    // Polymorphic serialization for block types
    if(stream.GetMode() == SerializerMode::Saving)
    {
      // If we're saving, grab each block and serialize it
      Array<BlockType*>::range range = dataArray.All();
      for(; !range.Empty(); range.PopFront())
      {
        BlockType* block = range.Front();
        stream.SerializePolymorphic(*block);
      }
    }
    else
    {
      // If we're loading things are a little more complicated
      PolymorphicNode node;
      while(stream.GetPolymorphic(node))
      {
        // For every node, create a block from it's typename and id
        BoundType* type = MetaDatabase::FindType(node.TypeName);
        if(type == nullptr)
        {
          Error("Type not found");
          continue;
        }

        BlockType* block = AllocateBlock(type, false).Get<BlockType*>();
        if(block)
        {
          // Serialize the block and then add it to our list
          block->Serialize(stream);
          dataArray.PushBack(block);
        }
        stream.EndPolymorphic();
      }
    }
  }

  bool mDeleteMemory;
};

}//namespace Zero
