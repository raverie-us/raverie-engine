///////////////////////////////////////////////////////////////////////////////
///
/// \file ObjectLoader.cpp
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"


namespace Zero
{

DataNode* FindSimilarInterfaces(DataNode* parent, BoundType* typeToAdd);
void AddDependencies(DataNode* parent, BoundType* type, DataNode* addLocation);

//------------------------------------------------------------------------------------ Object Loader
//**************************************************************************************************
void ObjectLoader::RecordModifications(Object* rootObject)
{
  if(mRoot->IsPatched())
  {
    CachedModifications modifications;
    CacheModifications(&modifications);
    modifications.ApplyModificationsToObject(rootObject);
  }
}

//**************************************************************************************************
void ObjectLoader::CacheModifications(CachedModifications* modifications)
{
  modifications->Cache(mRoot);
}

//**************************************************************************************************
PatchResolveMethod::Enum ObjectLoader::ResolveInheritedData(StringRange inheritId, DataNode*& result)
{
  // Currently, the only objects that have data inheritance are resources,
  // so for now this just supports resources
  Resource* resource = Z::gResources->GetResourceByName(inheritId);
  if(!resource)
    return PatchResolveMethod::Error;

  // Get the data tree from the resource
  result = resource->GetDataTree();
  if(result == nullptr)
    return PatchResolveMethod::Error;

  return PatchResolveMethod::PatchNode;
}

//**************************************************************************************************
DependencyAction::Enum ObjectLoader::ResolveDependencies(DataNode* parent, DataNode* newChild,
                                                         DataNode** toReplace, Status& status)
{
  // We need to get the meta type of the parent to check dependencies
  String parentTypeName = parent->mTypeName.All();
  String childTypeName = newChild->mTypeName.All();
  BoundType* parentType = MetaDatabase::GetInstance()->FindType(parentTypeName);
  BoundType* childType = MetaDatabase::GetInstance()->FindType(childTypeName);

  // The parent should never be null, but just in case
  if(parentType == nullptr)
  {
    String message = String::Format("Failed to check dependencies. Type of %s "
      "is not registered.", parentTypeName.c_str());
    status.SetFailed(message);
    return DependencyAction::Discard;
  }

  // The parent should have a Composition. We don't need it to check
  // dependencies, but just to be safe we should check
  MetaComposition* metaComposition = parentType->has(MetaComposition);
  if(metaComposition == nullptr)
  {
    String message = String::Format("Failed to check dependencies. Type of %s "
      "is not a composition.", parentTypeName.c_str());
    status.SetFailed(message);
    return DependencyAction::Discard;
  }

  // The child could not exist, but we still want to add it to save data.
  // This can happen if a script Component was removed or doesn't compile
  if(childType == nullptr)
    return DependencyAction::Add;

  // If we're adding a BoxCollider, but the base has a SphereCollider,
  // we want to replace the SphereCollider with the BoxCollider
  *toReplace = FindSimilarInterfaces(parent, childType);

  // Add all dependencies if they were removed from the base class
  AddDependencies(parent, childType, *toReplace);

  return DependencyAction::Add;
}

//**************************************************************************************************
DataNode* FindSimilarInterfaces(DataNode* parent, BoundType* typeToAdd)
{
  MetaDatabase* metaDataBase = MetaDatabase::GetInstance();

  // Check all interfaces on all base classes
  forRange(CogComponentMeta* addMetaComponent, typeToAdd->HasAll<CogComponentMeta>())
  {
    // Walk all nodes and see if there's a shared interface
    forRange(DataNode& childNode, parent->mChildren.All())
    {
      BoundType* childType = metaDataBase->FindType(childNode.mTypeName.All());

      // Meta could be null if scripts don't compile
      if (childType == nullptr)
        continue;

      forRange(CogComponentMeta* childMetaComponent, childType->HasAll<CogComponentMeta>())
      {
        forRange(BoundType* addInterfaceType, addMetaComponent->mInterfaces)
        {
          // If they're shared, we want to remove the base type
          if (childMetaComponent->mInterfaces.Contains(addInterfaceType))
          {
            // We don't have to continue searching because there can only be one
            // Component on an object of a similar interface
            return &childNode;
          }
        }
      }
    }
  }

  return nullptr;
}

//**************************************************************************************************
// We could be dependent on an interface, so find an addable type
// For example, we may be dependent on Collider, but Collider cannot
// be added to objects. So we need to find say BoxCollider
BoundType* FindAddableType(BoundType* typeToFind)
{
  CogComponentMeta* metaComponent = typeToFind->Has<CogComponentMeta>();
  ReturnIf(metaComponent == nullptr, nullptr, "Type did not have a CogComponentMeta attached to it");

  bool canBeCreated = !typeToFind->Constructors.Empty();
  bool isInterface = !metaComponent->mInterfaceDerivedTypes.Empty();

  ReturnIf(!canBeCreated && !isInterface, nullptr, "Cannot add type because it "
    "cannot be created and nothing binds it as an interface.");

  // If it can be created, use it instead of randomly picking an inherited type
  if(canBeCreated)
    return typeToFind;

  // There's no "best" inherited type to choose from, so grab the first
  return metaComponent->mInterfaceDerivedTypes.Front();
}

//**************************************************************************************************
void AddDependencies(DataNode* parent, BoundType* type, DataNode* addLocation)
{
  // Extra dependencies can be defined on more derived types, so we have to check all
  forRange(CogComponentMeta* metaComponent, type->HasAll<CogComponentMeta>())
  {
    forRange(BoundType* dependency, metaComponent->mDependencies.All())
    {
      // These types were special cased with Cogs and we want to ignore them
      // until it's refactored to not use dependencies.
      if (dependency == ZilchTypeId(Cog) ||
        dependency == ZilchTypeId(Space) ||
        dependency == ZilchTypeId(GameSession))
      {
        continue;
      }

      String dependencyTypeName = dependency->Name;
      bool foundDependency = false;

      // Check all the child nodes to see if the dependency exists
      forRange(DataNode& childNode, parent->mChildren.All())
      {
        // Allocation here, we should maybe consider not using fixed strings
        String childTypeName = childNode.mTypeName.All();

        // Look up the type to check if it's the type of the dependency
        BoundType* childNodeType = MetaDatabase::GetInstance()->FindType(childTypeName);

        if (childNodeType)
        {
          if (childNodeType->IsA(dependency))
          {
            foundDependency = true;
            break;
          }
        }
        // If we couldn't find the meta, we can fall back to just checking the
        // type name. This could be the case for script errors when loading
        else if (childTypeName == dependencyTypeName)
        {
          foundDependency = true;
          break;
        }
      }

      // If it doesn't have our dependency, we need to add it
      if (!foundDependency)
      {
        // We could be dependent on an interface, so find an addable type
        // For example, we may be dependent on Collider, but Collider cannot
        // be added to objects. So we need to find say BoxCollider
        BoundType* typeToAdd = FindAddableType(dependency);

        // Add all dependencies of this type
        AddDependencies(parent, typeToAdd, addLocation);

        // Add a node for this type
        DataNode* dependencyNode = new DataNode(DataNodeType::Object, parent);
        dependencyNode->mTypeName = typeToAdd->Name;
        dependencyNode->mPatchState = PatchState::None;

        // If we're being added at a specific location in our parent (likely
        // because we're replacing a different component of the same interface),
        // we want to add the dependencies before the position we're being
        // placed at
        if (addLocation)
          parent->MoveChild(dependencyNode, addLocation);
      }
    }
  }
}

//----------------------------------------------------------------------------- Cached Modifications
//**************************************************************************************************
CachedModifications::CachedModifications() :
  mRootObjectNode(nullptr)
{

}

//**************************************************************************************************
CachedModifications::~CachedModifications()
{
  SafeDelete(mRootObjectNode);
}

//**************************************************************************************************
void CachedModifications::Cache(DataNode* root)
{
  SafeDelete(mRootObjectNode);

  if(root->IsPatched())
  {
    mRootObjectNode = new ObjectNode();
    PropertyPath path;
    ExtractInternal(root, mRootObjectNode, path);
  }
}

//**************************************************************************************************
void CachedModifications::Cache(Object* object)
{
  SafeDelete(mRootObjectNode);

  mRootObjectNode = ExtractInternal(object);
}

//**************************************************************************************************
void CachedModifications::Combine(CachedModifications& modifications)
{
  // Nothing to do if there is nothing to combine
  if(modifications.mRootObjectNode == nullptr)
    return;

  // If we have no modifications, just clone the other tree
  if(mRootObjectNode == nullptr)
  {
    mRootObjectNode = modifications.mRootObjectNode->Clone();
  }

  // Otherwise combine them
  mRootObjectNode->Combine(modifications.mRootObjectNode);
}

//**************************************************************************************************
void CachedModifications::ApplyModificationsToObject(Object* object, bool combine)
{
  ApplyModificationsToObjectInternal(object, mRootObjectNode, combine);
}

//**************************************************************************************************
void CachedModifications::ApplyModificationsToChildObject(Object* rootObject, Object* childObject,
                                                          bool combine)
{
  // Search for the node that contains the child object and apply modifications from there
  if(ObjectNode* childNode = FindChildNode(rootObject, childObject))
    ApplyModificationsToObjectInternal(childObject, childNode, combine);
}

//**************************************************************************************************
void CachedModifications::StoreOverlappingModifications(Object* object, ObjectNode* cachedNode)
{
  if(cachedNode == nullptr)
    return;
  mRootObjectNode = StoreOverlappingModificationsInternal(object, cachedNode);
}

//**************************************************************************************************
CachedModifications::ObjectNode* CachedModifications::FindChildNode(Object* rootObject,
                                                                    Object* childObject)
{
  if(mRootObjectNode == nullptr)
    return nullptr;

  // This function was initially written recursively, however it was not easy to follow. It was
  // re-written to be more legible and now uses alloca, which may not really be worse than the
  // overhead of recursive function calls
  uint pathSize = 0;

  // Count how many objects are between the child and root (child included)
  Object* currObject = childObject;
  while(currObject != rootObject)
  {
    ++pathSize;
    BoundType* currType = ZilchVirtualTypeId(currObject);
    MetaOwner* metaOwner = currType->HasInherited<MetaOwner>();
    currObject = metaOwner->GetOwner(currObject).Get<Object*>();
  }

  // Allocate the path on the stack
  Object** objectPath = (Object**)alloca(pathSize * sizeof(Object*));

  // Walk again to fill out the stack
  currObject = childObject;
  for(uint i = 0; i < pathSize; ++i)
  {
    objectPath[pathSize - i - 1] = currObject;

    BoundType* currType = ZilchVirtualTypeId(currObject);
    MetaOwner* metaOwner = currType->HasInherited<MetaOwner>();
    currObject = metaOwner->GetOwner(currObject).Get<Object*>();
  }

  // Walk the path and resolve nodes
  ObjectNode* currNode = mRootObjectNode;
  for(uint i = 0; i < pathSize; ++i)
  {
    currNode = currNode->FindChild(objectPath[i]);

    // Cannot continue if we didn't find the node
    if(currNode == nullptr)
      return nullptr;
  }

  return currNode;
}

//**************************************************************************************************
void CachedModifications::Clear()
{
  SafeDelete(mRootObjectNode);
}

//**************************************************************************************************
bool CachedModifications::Empty()
{
  return (mRootObjectNode == nullptr);
}

//**************************************************************************************************
Object* GetComponentFromChildId(Object* instance, const ObjectState::ChildId& childId)
{
  MetaComposition* composition = ZilchVirtualTypeId(instance)->Has<MetaComposition>();
  if(composition == nullptr)
    return nullptr;

  Object* childObject = nullptr;

  // First look up by the unique id
  if(childObject == nullptr)
    childObject = composition->GetComponentUniqueId(instance, (u64)childId.mId).Get<Object*>();

  // If that failed, look up by type name
  if(childObject == nullptr)
  {
    if(BoundType* childType = MetaDatabase::GetInstance()->FindType(childId.mTypeName))
      childObject = composition->GetComponent(instance, childType).Get<Object*>();
  }

  return childObject;
}

//**************************************************************************************************
void CachedModifications::ApplyModificationsToObjectInternal(Object* object, ObjectNode* objectNode,
                                                             bool combine)
{
  if(objectNode == nullptr)
    return;

  LocalModifications* modifications = LocalModifications::GetInstance();

  // Copy over the object state
  if(objectNode->mState)
  {
    if(combine)
      modifications->CombineObjectState(object, objectNode->mState);
    else
      modifications->RestoreObjectState(object, objectNode->mState->Clone());
  }

  // We can only walk children if there is a meta composition
  BoundType* objectType = ZilchVirtualTypeId(object);
  if(MetaComposition* composition = objectType->Has<MetaComposition>())
  {
    // Walk each child state and try to find the respective object
    forRange(ObjectNode::ChildMap::value_type entry, objectNode->mChildren.All())
    {
      ObjectState::ChildId& childId = entry.first;
      ObjectNode* childNode = entry.second;

      Object* childObject = GetComponentFromChildId(object, childId);

      // Recurse if we found a valid object
      if(childObject)
        ApplyModificationsToObjectInternal(childObject, childNode, combine);
    }
  }
}

//**************************************************************************************************
CachedModifications::ObjectNode* CachedModifications::StoreOverlappingModificationsInternal(
                                                                   Object* object, ObjectNode* node)
{
  LocalModifications* modifications = LocalModifications::GetInstance();

  ObjectNode* overlapNode = nullptr;

#define GetOverlapNode() (overlapNode == nullptr ? overlapNode = new ObjectNode() : overlapNode)

  // Check if both objects have the same modifications
  ObjectState* cachedState = node->GetObjectState();
  ObjectState* objectState = modifications->GetObjectState(object);
  if(cachedState && objectState)
  {
    // Modified properties
    forRange(PropertyPath& cachedProperty, cachedState->GetModifiedProperties())
    {
      if(objectState->IsPropertyModified(cachedProperty))
        GetOverlapNode()->GetObjectState()->SetPropertyModified(cachedProperty, true);
    }

    // Locally added children
    forRange(ObjectState::ChildId addedChild, cachedState->GetAddedChildren())
    {
      if(objectState->IsChildLocallyAdded(addedChild))
        GetOverlapNode()->GetObjectState()->ChildAdded(addedChild);
    }

    // Locally removed children
    forRange(ObjectState::ChildId removedChild, cachedState->GetRemovedChildren())
    {
      if(objectState->IsChildLocallyRemoved(removedChild))
        GetOverlapNode()->GetObjectState()->ChildRemoved(removedChild);
    }

    // Child order override
    if(cachedState->IsChildOrderModified())
      GetOverlapNode()->GetObjectState()->SetChildOrderModified(true);
  }

  // We can only walk children if there is a meta composition
  BoundType* objectType = ZilchVirtualTypeId(object);
  if(MetaComposition* composition = objectType->Has<MetaComposition>())
  {
    // Walk each child state and try to find the respective object
    forRange(ObjectNode::ChildMap::value_type entry, node->mChildren.All())
    {
      ObjectState::ChildId& childId = entry.first;
      ObjectNode* childNode = entry.second;

      Object* childObject = GetComponentFromChildId(object, childId);

      // Recurse if we found a valid object
      if(childObject)
      {
        ObjectNode* childOverlapNode = StoreOverlappingModificationsInternal(childObject, childNode);
        if(childOverlapNode)
          GetOverlapNode()->mChildren.Insert(childId, childOverlapNode);
      }
    }
  }

  return overlapNode;
}

//**************************************************************************************************
ObjectState::ChildId GetDataNodeKey(DataNode* dataNode)
{
  return ObjectState::ChildId(dataNode->mTypeName, dataNode->mUniqueNodeId);
}

//**************************************************************************************************
void CachedModifications::ExtractInternal(DataNode* dataNode, ObjectNode* objectNode,
                                          PropertyPath& path)
{
  // Copy over child order override flag
  if(dataNode->mFlags.IsSet(DataNodeFlags::ChildOrderOverride))
    objectNode->GetObjectState()->SetChildOrderModified(true);

  // Record any child removals (this include components and hierarchy children)
  forRange(DataNode::RemovedNode removed, dataNode->mRemovedChildren.All())
  {
    ObjectState::ChildId childId(removed.mTypeName, removed.mUniqueNodeId);

    ObjectState* objectState = objectNode->GetObjectState();
    objectState->ChildRemoved(childId);
  }

  DataNode::DataNodeList::range children = dataNode->GetChildren();

  if(dataNode->IsProperty() && children.Empty())
  {
    path.AddPropertyToPath(dataNode->mPropertyName);

    // Mark the property as modified
    objectNode->GetObjectState()->SetPropertyModified(path, true);

    path.PopEntry();
  }

  // Walk all children
  forRange(DataNode& childDataNode, children)
  {
    if(!childDataNode.IsPatched())
      continue;

    if(childDataNode.IsProperty())
    {
      // Add the current property to the path
      path.AddPropertyToPath(childDataNode.mPropertyName);

      // If it has properties as children, it's likely a property object, so recurse down
      if(childDataNode.HasChildProperties())
      {
        forRange(DataNode& subNode, childDataNode.GetChildren())
        {
          if(subNode.IsPatched())
            ExtractInternal(&subNode, objectNode, path);
        }
      }
      else
      {
        // Mark the property as modified
        objectNode->GetObjectState()->SetPropertyModified(path, true);
      }

      path.PopEntry();
    }
    else
    {
      // Is locally added?
      if(childDataNode.IsLocallyAdded())
      {
        ObjectState* objectState = objectNode->GetObjectState();

        ObjectState::ChildId childId(childDataNode.mTypeName, childDataNode.mUniqueNodeId);
        objectState->ChildAdded(childId);
      }

      // If the child was patched, recurse down
      if(childDataNode.IsPatched())
      {
        // Create the new object node for our child
        ObjectNode* childObjectNode = new ObjectNode();
        ObjectState::ChildId childKey = GetDataNodeKey(&childDataNode);
        objectNode->mChildren[childKey] = childObjectNode;

        // We've entered a new object, so start a new path
        PropertyPath newPath;
        ExtractInternal(&childDataNode, childObjectNode, newPath);
      }
    }
  }
}

//**************************************************************************************************
CachedModifications::ObjectNode* CachedModifications::ExtractInternal(Object* object)
{
  BoundType* objectType = ZilchVirtualTypeId(object);

  ObjectNode* currNode = nullptr;

  // Recurse depth first and build the tree as we go back up
  if(MetaComposition* composition = objectType->Has<MetaComposition>())
  {
    forRange(Handle child, composition->AllComponents(object))
    {
      if(ObjectNode* childNode = ExtractInternal(child.Get<Object*>()))
      {
        // Create the node if it doesn't exist
        if(currNode == nullptr)
          currNode = new ObjectNode();

        ObjectState::ChildId childId(child);
        currNode->mChildren[childId] = childNode;
      }
    }
  }

  LocalModifications* modifications = LocalModifications::GetInstance();
  if(ObjectState* state = modifications->GetObjectState(object))
  {
    // Create the node if it doesn't exist
    if(currNode == nullptr)
      currNode = new ObjectNode();

    // Copy over the object's state
    currNode->mState = new ObjectState(*state);
  }

  return currNode;
}

//-------------------------------------------------------------------------------------- Object Node
//**************************************************************************************************
CachedModifications::ObjectNode::ObjectNode()
{
  //mState = new ObjectState();
  mState = nullptr;
}

//**************************************************************************************************
CachedModifications::ObjectNode::~ObjectNode()
{
  SafeDelete(mState);
  forRange(ObjectNode* child, mChildren.Values())
    delete child;
}

//**************************************************************************************************
ObjectState* CachedModifications::ObjectNode::GetObjectState()
{
  if(mState == nullptr)
    mState = new ObjectState();
  return mState;
}

//**************************************************************************************************
CachedModifications::ObjectNode* CachedModifications::ObjectNode::FindChild(Object* child)
{
  return mChildren.FindValue(ObjectState::ChildId(child), nullptr);
}

//**************************************************************************************************
CachedModifications::ObjectNode* CachedModifications::ObjectNode::Clone()
{
  ObjectNode* clone = new ObjectNode();

  // Clone modifications
  if(mState)
    clone->mState = mState->Clone();

  // Clone children
  forRange(ChildMap::value_type& entry, mChildren.All())
  {
    ObjectState::ChildId childId = entry.first;
    ObjectNode* child = entry.second;

    clone->mChildren.Insert(childId, child->Clone());
  }

  return clone;
}

//**************************************************************************************************
void CachedModifications::ObjectNode::Combine(ObjectNode* toCombine)
{
  // Combine object states
  if(toCombine->mState != nullptr)
  {
    if(mState == nullptr)
      mState = toCombine->mState->Clone();
    else
      mState->Combine(toCombine->mState);
  }

  // Combine children
  forRange(ChildMap::value_type& entry, toCombine->mChildren.All())
  {
    ObjectState::ChildId childId = entry.first;
    ObjectNode* toCombineChild = entry.second;

    // If we have the same child, combine it, otherwise clone the other child
    if(ObjectNode* ourChild = mChildren.FindValue(childId, nullptr))
      ourChild->Combine(toCombineChild);
    else
      mChildren.Insert(childId, toCombineChild->Clone());
  }
}

}//namespace Zero
