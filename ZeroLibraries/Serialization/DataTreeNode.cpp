///////////////////////////////////////////////////////////////////////////////
///
/// \file DataTreeNode.cpp
/// Implementation of the data tree used for serialization.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

DataNode* FindMatchingChildNode(DataNode* parent, DataNode* nodeToMatch, uint childIndex);

//-------------------------------------------------------------------- Data Node
Memory::Pool* DataNode::sPool = new Memory::Pool("DataTree", Memory::GetRoot(), sizeof(DataNode), 5000);

//******************************************************************************
void* DataNode::operator new(size_t size)
{
  return sPool->Allocate(size);
}

//******************************************************************************
void DataNode::operator delete(void* pMem, size_t size)
{
  return sPool->Deallocate(pMem, size);
}

//******************************************************************************
DataNode::DataNode(DataNodeType::Enum nodeType, DataNode* parent) : 
  mNumberOfChildren(0),
  mUniqueNodeId(PolymorphicNode::cInvalidUniqueNodeId)
{
  mNodeType = nodeType;
  mPatchState = PatchState::None;
  mParent = nullptr;

  if(parent)
    AttachTo(parent);
}

//******************************************************************************
DataNode::~DataNode()
{
  DeleteObjectsIn(mChildren);
}

//******************************************************************************
DataNode* DataNode::GetParent()
{
  return mParent;
}

//******************************************************************************
void DataNode::AttachTo(DataNode* newParent)
{
  // Remove the object from its previous parent
  if(mParent)
    Detach();

  // Attach to the new parent
  newParent->mChildren.PushBack(this);
  ++newParent->mNumberOfChildren;
  mParent = newParent;
}

//******************************************************************************
void DataNode::Detach()
{
  //ErrorIf(child->mParent != this, "Child is not part of this data set.");
  if(mParent)
  {
    mParent->mChildren.Erase(this);
    --mParent->mNumberOfChildren;
    mParent = nullptr;
  }
}

//******************************************************************************
void DataNode::ReplaceChild(DataNode* oldChild, DataNode* newChild)
{
  // Remove the new child from the old parent
  if(newChild->mParent)
    newChild->Detach();

  // Insert the node
  newChild->mParent = this;
  mChildren.InsertBefore(oldChild, newChild);

  // Remove the old node
  oldChild->mParent = nullptr;
  mChildren.Erase(oldChild);
}

//******************************************************************************
void DataNode::MoveChild(DataNode* child, DataNode* location)
{
  mChildren.Erase(child);
  mChildren.InsertBefore(location, child);
}

//******************************************************************************
void DataNode::Destroy()
{
  if(mParent)
    Detach();
  delete this;
}

//******************************************************************************
DataNode* DataNode::NextSibling()
{
  if(mParent)
  {
    DataNode* next = this->link.Next;
    if(next == mParent->mChildren.End())
      return nullptr;
    else
      return next;
  }

  return nullptr;
}

//******************************************************************************
uint DataNode::GetNumberOfChildren()
{
  return mNumberOfChildren;
}

//******************************************************************************
DataNode* DataNode::GetFirstChild()
{
  if(!mChildren.Empty())
    return &mChildren.Front();
  else
    return NULL;
}

//******************************************************************************
DataNode* DataNode::FindChildWithName(StringRange name)
{
  if(!name.Empty())
  {
    if(name.Front() == 'm')
      name.PopFront();
  }

  forRange(DataNode& node, mChildren.All())
  {
    if(name == node.mPropertyName)
      return &node;
  }
  return NULL;
}

//******************************************************************************
DataNode* DataNode::ChildAtIndex(uint index)
{
  uint i = 0;
  DataNodeList::range r = mChildren.All();
  for(; i < index; ++i)
    r.PopFront();
  return &r.Front();
}

//******************************************************************************
DataNode::DataNodeList::range DataNode::GetChildren()
{
  return mChildren.All();
}

//******************************************************************************
DataNode* DataNode::FindChildWithTypeName(StringRange typeName)
{
  bool foundDuplicate;
  return FindChildWithTypeName(typeName, "", foundDuplicate);
}

//******************************************************************************
DataNode* DataNode::FindChildWithTypeName(StringRange typeName, StringRange name,
                                          bool& foundDuplicate)
{
  DataNode* result = nullptr;
  forRange(DataNode& node, mChildren.All())
  {
    if(typeName == node.mTypeName && name == node.mPropertyName)
    {
      // If we already found a node, it cannot be resolved as there are multiple
      if(result != nullptr)
      {
        foundDuplicate = true;
        return nullptr;
      }

      result = &node;
    }
  }

  return result;
}

//******************************************************************************
DataNode* DataNode::FindChildWithUniqueNodeId(Guid childId)
{
  forRange(DataNode& child, mChildren.All())
  {
    if(child.mUniqueNodeId == childId)
      return &child;
  }

  return nullptr;
}

//******************************************************************************
void DataNode::AddAttribute(StringParam name, StringParam value)
{
  // Strip quotes around the value
  StringRange valueRange = value.All();
  if(valueRange.Front() == '\"')
    valueRange.PopFront();
  if(valueRange.Back() == '\"')
    valueRange.PopBack();
  DataAttribute attribute(name, valueRange);
  mAttributes.PushBack(attribute);
}

//******************************************************************************
bool DataNode::IsPatched()
{
  return (mPatchState != PatchState::None);
}

//******************************************************************************
bool DataNode::IsProperty()
{
  return !mPropertyName.Empty();
}

//******************************************************************************
bool DataNode::HasChildProperties()
{
  forRange(DataNode& child, GetChildren())
  {
    if(!child.mPropertyName.Empty())
      return true;
  }

  return false;
}

//******************************************************************************
void DataNode::Patch(DataNode* patchNode, DataTreeContext& c)
{
  // Type checking
  if(this->mNodeType != patchNode->mNodeType)
  {
    c.Error = true;
    String parseError = String::Format("Nodes with name \"%s\" were of different types.", this->mPropertyName.c_str());
    c.Message = String::Format("Error patching file '%s' . Error: %s",
                               c.Filename.c_str(), parseError.c_str());
    Error("%s", c.Message.c_str());
    return;
  }

  if(mNodeType == DataNodeType::Object)
  {
    bool childOrderOverride = patchNode->mFlags.IsSet(DataNodeFlags::ChildOrderOverride);

    // Copy over the flag to the final node as the patch node will be destroyed
    if(childOrderOverride)
    {
      mFlags.SetFlag(DataNodeFlags::ChildOrderOverride);
      mPatchState = PatchState::SelfPatched;
    }

    mUniqueNodeId = patchNode->mUniqueNodeId;

    // Used for re-ordering children when childOrderOverride is set
    DataNode* ourPreviousValidChild = nullptr;

    // Properties should never be re-ordered, only polymorphic children
    forRange(DataNode& child, GetChildren())
    {
      if(!child.IsProperty())
        break;
      ourPreviousValidChild = &child;
    }

    for(uint i = 0; i < patchNode->GetNumberOfChildren();)
    {
      DataNode* patchChild = patchNode->ChildAtIndex(i);
      DataNode* ourChild = FindMatchingChildNode(this, patchChild, i);
      
      // Attempt to remove the child
      if(patchChild->mPatchState == PatchState::ShouldRemove)
      {
        ErrorIf(patchChild->mNodeType != DataNodeType::Object, "Only collections can be locally removed.");

        // Record that it was removed

        RemovedNode removed = { patchChild->mTypeName.All(), patchChild->mUniqueNodeId };
        mRemovedChildren.PushBack(removed);

        // A child was removed
        mPatchState = PatchState::SelfPatched;

        // It could have already been removed on the base, so check
        if(ourChild)
          ourChild->Destroy();
      }
      // If we didn't have that node, it must be being added
      else if(ourChild == nullptr)
      {
        // If it's a property, it was either renamed from the inherited file, or it was newly added.
        // In this case, we just want to attach the node to the parent node
        bool isProperty = patchChild->IsProperty();

        // We have to check for dependencies before adding the node
        // Only need to check dependencies for other data sets
        if(!isProperty && patchChild->mNodeType == DataNodeType::Object)
        {
          Status addStatus;
          DataNode* toReplace = nullptr;

          // I don't think we even have to do this step if childOrderOverride is set because
          // All the dependencies will be 
          DependencyAction::Enum action = c.Loader->ResolveDependencies(this, patchChild, 
                                                                        &toReplace, addStatus);

          if(action == DependencyAction::Add)
          {
            // Add the patch node to the end of our children
            if(toReplace == nullptr)
            {
              patchChild->AttachTo(this);

              // We don't want properties to show up after polymorphic children, so move it to
              // the front (property order doesn't matter)
              if(patchChild->IsProperty())
                patchChild->PlaceAfterSibling(nullptr);
            }
            // Add the patch node in the same place as the one to replace
            else
            {
              ReplaceChild(toReplace, patchChild);

              // We don't need the old node anymore
              toReplace->Destroy();
            }

            // Place the node in the correct place if we're overriding the order
            if(childOrderOverride && !patchChild->IsProperty())
              patchChild->PlaceAfterSibling(ourPreviousValidChild);

            ourPreviousValidChild = patchChild;
          }
          else // action == DependencyAction::Discard
          {
            // Just leave the patch node in place as we're ignoring it
            ++i;
            continue;
          }
        }
        else
        {
          // It's a value node, so the only thing we can do is add it to the end
          patchChild->AttachTo(this);
        }

        // If our tree didn't have this node, there are two cases:
        // 1. It was a locally added Component
        //    - We want to mark the node as 'Added'
        //    - The child properties / objects shouldn't be marked as modified, because they
        //      are a child of a locally added object
        // 2. It's a property but it was renamed and the data tree we're patching has the 
        //    old property name, or the property was added after the base data tree was created
        //    - It should mark child properties as added (or should it be modified?), as well as
        //      itself.
        //    - This doesn't support inherited objects as children of properties. We don't currently
        //      do this anywhere in the engine, but should probably be updated to only mark
        //      children as modified, until we hit another inherited tree
        if (isProperty)
          patchChild->SetPatchStateRecursive(PatchState::Added);
        else
          patchChild->mPatchState = PatchState::Added;

        mPatchState = PatchState::ChildPatched;

        // Custom logic for Cogs because of how Hierarchies work...
        // This helps with the case of when the Archetype doesn't have any children (and therefor
        // doesn't have the Hierarchy Component), and a child was locally added to the
        // other Cog. The Hierarchy Component is the first node we see that's locally added,
        // but we also want to mark the Cog node as locally added, so we're walking the children
        // here to see if there are more locally added 
        forRange(DataNode& addedChild, patchChild->GetChildren())
        {
          if(addedChild.mFlags.IsSet(DataNodeFlags::LocallyAdded))
            addedChild.mPatchState = PatchState::Added;
        }

        // We're removing the node from the set we're walking through, so 
        // we do not want to increment 'i'
        continue;
      }
      else
      {
        // Recursively patch
        ourChild->Patch(patchChild, c);
        if(ourChild->IsPatched())
          mPatchState = PatchState::ChildPatched;

        // If we're overriding the child order, we need move our node to the same
        // location that the patch node is at. We do not want to re-order properties,
        // only polymorphic children
        if(childOrderOverride && !ourChild->IsProperty())
        {
          // We want to move the current child to be after the previous child
          // If there was no previous child, the function will place it at the start
          ourChild->PlaceAfterSibling(ourPreviousValidChild);
        }

        // Store the previous child for use in order override
        if(ourChild)
          ourPreviousValidChild = ourChild;
      }

      ++i;
    }
  }
  else if(mNodeType == DataNodeType::Value)
  {
    if(this->mNodeType != patchNode->mNodeType)
    {
      c.Error = true;
      String parseError = String::Format("Nodes with name \"%s\" were of different types.", 
                                         this->mPropertyName.c_str());
      c.Message = String::Format("Error patching file '%s' . Error: %s",
                                  c.Filename.c_str(), parseError.c_str());
      ErrorIf(true, c.Message.c_str());
      return;
    }

    mTextValue = patchNode->mTextValue;
    mPatchState = PatchState::SelfPatched;
  }
}

//******************************************************************************
void DataNode::SetPatchStateRecursive(PatchState::Enum state)
{
  mPatchState = state;
  forRange(DataNode& child, mChildren.All())
    child.SetPatchStateRecursive(state);
}

//******************************************************************************
void DataNode::PlaceAfterSibling(DataNode* sibling)
{
  ReturnIf(mParent == nullptr, , "Node must have a parent.");

  // Move to front if nothing was given
  if(sibling == nullptr)
  {
    mParent->mChildren.Erase(this);
    mParent->mChildren.PushFront(this);
  }
  else
  {
    ReturnIf(mParent != sibling->mParent, , "Data Nodes are not siblings.");

    mParent->mChildren.Erase(this);
    mParent->mChildren.InsertAfter(sibling, this);
  }
}

//******************************************************************************
bool DataNode::IsLocallyAdded()
{
  return mFlags.IsSet(DataNodeFlags::LocallyAdded);
}

//******************************************************************************
bool DataNode::IsChildOrderOverride()
{
  return mFlags.IsSet(DataNodeFlags::ChildOrderOverride);
}

//******************************************************************************
bool DataNode::IsArray()
{
  return mFlags.IsSet(DataNodeFlags::Array);
}

//******************************************************************************
void DataNode::ClearPatchState()
{
  mPatchState = PatchState::None;
  forRange(DataNode& child, GetChildren())
    child.ClearPatchState();
}

//******************************************************************************
DataNode* DataNode::Clone()
{
  // Clone ourself and copy over all data
  DataNode* clone = new DataNode(mNodeType, nullptr);
  clone->mPropertyName    = mPropertyName;
  clone->mTypeName        = mTypeName;
  clone->mTextValue       = mTextValue;
  clone->mPatchState      = mPatchState;
  clone->mFlags           = mFlags;
  clone->mInheritedFromId = mInheritedFromId;
  clone->mUniqueNodeId    = mUniqueNodeId;

  // Copy attributes
  clone->mAttributes.Assign(mAttributes.All());

  // Copy locally removed nodes
  clone->mRemovedChildren.Assign(mRemovedChildren.All());

  // Clone our children
  forRange(DataNode& child, mChildren.All())
  {
    DataNode* childClone = child.Clone();
    childClone->AttachTo(clone);
  }

  return clone;
}

bool IsVectorType(StringParam typeName)
{
  BoundType* propertyType = MetaDatabase::FindType(typeName);

  return (propertyType == ZilchTypeId(Vec2) ||
    propertyType == ZilchTypeId(Vec3) ||
    propertyType == ZilchTypeId(Vec4) ||
    propertyType == ZilchTypeId(Quat));
}

//******************************************************************************
void DataNode::SaveToStream(Serializer& stream)
{
  ErrorIf(stream.GetMode() != SerializerMode::Saving, "Must be a saving serializer");
  ErrorIf(stream.GetType() != SerializerType::Text, "Must be a text serializer");
  TextSaver& textStream = *(TextSaver*)(&stream);

  if(IsProperty())
  {
    StringRange range = mTextValue.All();
    if(mFlags.IsSet(DataNodeFlags::Enumeration))
    {
      // Enums need to be "`PropertyName`.`Value`" in serialization
      String value = BuildString(mTypeName, ".", mTextValue);
      range = value.All();
      stream.SimpleField(mTypeName.c_str(), mPropertyName.c_str(), range);
    }
    // Property objects such as Vec3, Vec4, CogPath, etc..
    else if(mNodeType == DataNodeType::Object)
    {
      textStream.Tabs();

      StringBuilder& builder = textStream.mStream;
      builder.Append("var ");
      builder.Append(mPropertyName);
      builder.Append(" = ");
      builder.Append(mTypeName);
      builder.Append("{");

      // Temporary solution. We need to refactor the data format again.
      if(IsVectorType(this->mTypeName))
      {
        uint childIndex = 0;
        forRange(DataNode& child, GetChildren())
        {
          textStream.mStream.Append(child.mTextValue);
          if (childIndex < mNumberOfChildren - 1)
            textStream.mStream.Append(", ");
          ++childIndex;
        }
      }
      else
      {
        forRange(DataNode& child, GetChildren())
          child.SaveToStream(stream);
      }
      builder.Append("}\n");
    }
    else if(mTypeName == "String" || mTypeName == "string")
      stream.StringField(mTypeName.c_str(), mPropertyName.c_str(), range);
    else if(IsArray())
      Error("Not implemented");
    else
      stream.SimpleField(mTypeName.c_str(), mPropertyName.c_str(), range);
  }
  else if(mNodeType == DataNodeType::Object)
  {
    stream.StartPolymorphic(mTypeName.c_str());

    forRange(DataNode& child, GetChildren())
      child.SaveToStream(stream);

    stream.EndPolymorphic();
  }
}

//******************************************************************************
DataNode* FindRenamedNode(StringParam className, StringParam propertyTypeName, StringParam propertyName, DataNode* parent)
{
  bool foundDuplicate;

  // Before attaching it, check to see if the property was renamed
  if (BoundType* propertyType = MetaDatabase::FindType(className))
  {
    Property* property = propertyType->GetProperty(propertyName);
    if (property == nullptr)
      return nullptr;

    if (MetaPropertyRename* rename = property->Has<MetaPropertyRename>())
    {
      if(DataNode* node =  parent->FindChildWithTypeName(propertyTypeName, rename->mOldName, foundDuplicate))
      {
        // Rename the old node so that serialization reads the new name
        node->mPropertyName = propertyName;
        return node;
      }
    }
  }

  return nullptr;
}

//******************************************************************************
DataNode* FindMatchingChildNode(DataNode* parent, DataNode* nodeToMatch, uint childIndex)
{
  DataNode* result = nullptr;
  StringRange typeName = nodeToMatch->mTypeName.All();
  StringRange name = nodeToMatch->mPropertyName.All();

  bool validChildIndex = (childIndex != (uint)-1);

  if(nodeToMatch->mNodeType == DataNodeType::Object)
  {
    // Resolve with node id's if available
    Guid nodeId = nodeToMatch->mUniqueNodeId;
    if(nodeId != PolymorphicNode::cInvalidUniqueNodeId)
      return parent->FindChildWithUniqueNodeId(nodeId);

    // First look it up by type name
    bool foundDuplicate = false;
    result = parent->FindChildWithTypeName(typeName, name, foundDuplicate);

    // If we didn't find it, maybe the property was renamed
    if (result == nullptr && nodeToMatch->IsProperty())
      result = FindRenamedNode(parent->mTypeName, typeName, name, parent);

    // If it couldn't be resolved by typename (there were duplicates), attempt
    // to resolve it by index instead
    //if(result == nullptr && validChildIndex)
      //result = parent->ChildAtIndex(childIndex);

    return result;
  }

  // If it's an array, resolve it by index
  ErrorIf(nodeToMatch->mParent->mNodeType != DataNodeType::Object, "Mismatched parents.");
  DataNode* nodeToMatchParent = nodeToMatch->mParent;

  if(nodeToMatchParent->IsArray())
  {
    if(validChildIndex)
      result = parent->ChildAtIndex(childIndex);
  }
  // Otherwise, just resolve it by name
  else
  {
    result = parent->FindChildWithName(name);
    if (result == nullptr && nodeToMatch->IsProperty())
      result = FindRenamedNode(parent->mTypeName, typeName, name, parent);
  }
  return result;
}

//******************************************************************************
// Walks the given data tree and resolves any nodes that inherit from 
// another data tree. Returns whether or not it removed the child node.
//
// There are three cases for when we find an inherited node:
// 1. The node is the root inheritance node
//    - The inherited tree will be created and anything below this node will
//      be applied as a patch
//    - Example:
//
//      Cog("56804bc026360a06:Player") =
//      {
//        // Data here will patch the "Player" Archetype
//      }
//
//
// 2. The node is underneath an inheritance node
//    - The inherited data won't be used as we're just patching the data tree
//      that was inherited from the root inheritance node
//    - Example:
//
//      Cog("56804bc026360a06:Player") =
//      {
//        //...
//      
//        Hierarchy =
//        {
//          // The "Hat" tree will not be resolved here as it should have been
//          // created in the "Player" Archetype
//          Cog:5693db94ab2447d1 ("56804a3c277e2c45:Hat") =
//          {
//            // Data here will patch the data from the "Player" Archetype
//          }
//        }
//      }
//
//
// 3. The node is underneath an inheritance node, but is an additive node
//    - We're underneath an inherited node, but we won't be patching anything
//      because we're an additive node. Because of this, we need to inherit
//      from our 
//    - Example:
//
//      Cog("56804bc026360a06:Player") =
//      {
//        //...
//      
//        Hierarchy =
//        {
//          // This data will result in the entire "Hat" object
//          // With any patches applied
//          + Cog:5693db94ab2447d1 ("56804a3c277e2c45:Hat") =
//          {
//            // Data here will patch the "Hat" Archetype
//          }
//        }
//      }
//
bool PatchDataTree(DataNode*& node, DataTreeLoader* loader, DataTreeContext& c, bool withinPatch)
{
  String inheritId = node->mInheritedFromId;
  bool validInherit = !inheritId.Empty();

  // See function comment
  bool childrenWithinPatch = withinPatch | validInherit;

  // We want to recurse down and patch any data inheritance from bottom up
  for(uint i = 0; i < node->GetNumberOfChildren();)
  {
    DataNode* child = node->ChildAtIndex(i);
    bool removedChild = false;
    if(child->mNodeType == DataNodeType::Object)
      removedChild = PatchDataTree(child, loader, c, childrenWithinPatch);

    if(child->IsPatched())
      node->mPatchState = PatchState::ChildPatched;

    // If our child was removed, the child list will be shifted, so we don't
    // want to increment our iterator
    if(!removedChild)
      ++i;
  }

  // We can only patch if it inherits from other data
  bool shouldPatch = validInherit;

  // If we're a child of a node that's being patched, we only want to resolve
  // our inheritance data if we're additive (see function comment)
  shouldPatch &= (!withinPatch || node->IsLocallyAdded());

  if(shouldPatch)
  {
    // Get the patch tree
    DataNode* newNode = nullptr;
    PatchResolveMethod::Enum resolveMethod = loader->ResolveInheritedData(inheritId, newNode);

    if(newNode)
    {
      // We're going to patch the children of this node, so mark it as patched
      newNode->mPatchState = PatchState::ChildPatched;

      // Copy over locally added flag
      if(node->mFlags.IsSet(DataNodeFlags::LocallyAdded))
        newNode->mFlags.SetFlag(DataNodeFlags::LocallyAdded);

      // Store what the node was inherited from
      newNode->mInheritedFromId = inheritId;
      ErrorIf(resolveMethod == PatchResolveMethod::Error, "Do not return a tree if there was an error.");
      ErrorIf(resolveMethod == PatchResolveMethod::RemoveNode, "Do not return a tree if you want to remove the old one.");
    }

    // Do nothing if we were given an error back
    if(resolveMethod == PatchResolveMethod::Error)
    {
      c.Error = true;
      c.Message = String::Format("Unspecified patch error for %s", inheritId.c_str());
      return false;
    }

    // If they didn't give us a tree, the only valid operation is to remove
    // the node. If something else was returned, it's an error
    if(newNode == nullptr)
    {
      if(resolveMethod == PatchResolveMethod::RemoveNode)
      {
        node->Destroy();
        return true;
      }
      else
      {
        c.Error = true;
        c.Message = String::Format("No patch given for %s", inheritId.c_str());
        return false;
      }
    }

    // Patch the node
    if(resolveMethod == PatchResolveMethod::PatchNode)
      newNode->Patch(node, c);

    // When we either patch or use the new node, we'll want to replace it
    // under the parent node
    if(node->mParent != nullptr)
      node->mParent->ReplaceChild(node, newNode);

    // No longer need the patch node
    node->Destroy();
    if(newNode->mNodeType == DataNodeType::Object)
      node = newNode;
    return false;
  }

  return false;
}

//******************************************************************************
// Version 0 is legacy for before we added file versions. After version 0, the
// file version is the same as the zero engine version
uint GetFileVersion(StringRange fileData)
{
  // If there's no start of an attribute as the first character of the file,
  // it's in the old format
  if(fileData.Front() != '[')
    return DataVersion::Legacy;

  // Find the value of the version attribute
  StringRange attributeValue = fileData.FindRangeExclusive("Version:", "]");
  ErrorIf(attributeValue.Empty(), "The first file attribute was not the version.");

  uint version;
  ToValue(attributeValue, version);
  return version;
}

//******************************************************************************
// Used for debugging when implementing a new data tree loader
void Compare(DataNode* lhs, DataNode* rhs)
{
  ErrorIf(lhs->mNodeType != rhs->mNodeType, "");
  ErrorIf(lhs->mPropertyName != rhs->mPropertyName, "");
  ErrorIf(lhs->mTypeName != rhs->mTypeName, "");
  ErrorIf(lhs->mTextValue != rhs->mTextValue, "");
  ErrorIf(lhs->mNumberOfChildren != rhs->mNumberOfChildren, "");

  // Attributes
  ErrorIf(lhs->mAttributes.Size() != rhs->mAttributes.Size());
  for(uint i = 0; i < lhs->mAttributes.Size(); ++i)
  {
    DataAttribute& lhsAttribute = lhs->mAttributes[i];
    DataAttribute& rhsAttribute = rhs->mAttributes[i];

    ErrorIf(lhsAttribute.mName != rhsAttribute.mName, "");
    ErrorIf(lhsAttribute.mValue != rhsAttribute.mValue, "");
  }

  ErrorIf(lhs->mPatchState != rhs->mPatchState, "");
  ErrorIf(lhs->mFlags.U32Field != rhs->mFlags.U32Field, "");

  // Removed Children don't need to be tested because it's post-patch

  ErrorIf(lhs->mInheritedFromId != rhs->mInheritedFromId, "");
  ErrorIf(lhs->mUniqueNodeId != rhs->mUniqueNodeId, "");

  // Children
  for(uint i = 0; i < lhs->mNumberOfChildren; ++i)
  {
    DataNode* lhsChild = lhs->ChildAtIndex(i);
    DataNode* rhsChild = rhs->ChildAtIndex(i);

    Compare(lhsChild, rhsChild);
  }
}

//******************************************************************************
DataNode* ReadDataSet(Status& status, StringRange data, StringParam source,
                      DataTreeLoader* loader, uint* fileVersion)
{
  DataTreeContext parseContext;
  parseContext.Filename = source;
  parseContext.Loader = loader;

  // Load the data tree with the correct parser
  *fileVersion = GetFileVersion(data);

  // Use the correct loader given the file version
  DataNode* fileTree = nullptr;
  DataNode* newFileTree = nullptr;
  {
    //TimerBlock block("Data Tree Parsing");
    if(*fileVersion == DataVersion::Legacy)
      fileTree = LegacyDataTreeParser::BuildTree(parseContext, data);
    else
      fileTree = DataTreeParser::BuildTree(parseContext, data);
  }

  // Failed to read file
  if(fileTree == NULL)
  {
    status.SetFailed("Failed to parse root element.", ParseErrorCodes::ParsingError);
    return NULL;
  }

  // Check for parse error
  if(parseContext.Error)
  {
    status.SetFailed(parseContext.Message, ParseErrorCodes::ParsingError);
    SafeDelete(fileTree);
    return NULL;
  }

  // Patch the tree if required
  if(parseContext.PatchRequired && !loader->mIgnoreDataInheritance)
    PatchDataTree(fileTree, loader, parseContext, false);

  return fileTree;
}

}//namespace Zero
