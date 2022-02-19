// MIT Licensed (see LICENSE.md).

#pragma once

namespace Zero
{

template <typename ResourceList>
class ResourceListOperation : public Operation
{
public:
  UndoHandle mObjectHandle;
  // Storing a pointer to this is safe because it only ever points to native
  // types, which never get destructed
  BoundTypeHandle mMeta;
  String mResourceIdName;
  uint mIndex;
  bool mAddOp;

  ResourceListOperation(HandleParam object, StringParam resourceIdName, uint index = -1, bool addOp = true) :
      mResourceIdName(resourceIdName),
      mIndex(index),
      mAddOp(addOp),
      mObjectHandle(object)
  {
    mName = "Removed resource";
    if (mAddOp)
      mName = "Added resource";

    if (Resource* resource = GetResourceList()->mOwner)
      BuildString(mName, ": ", resource->Name);

    mMeta = object.StoredType;
  }

  void Undo() override
  {
    if (mAddOp)
      RemoveResource();
    else
      AddResource();
  }

  void Redo() override
  {
    if (mAddOp)
      AddResource();
    else
      RemoveResource();
  }

  void AddResource()
  {
    GetResourceList()->AddResource(mResourceIdName, mIndex);
    SendPropertyEvent();
  }

  void RemoveResource()
  {
    GetResourceList()->RemoveResource(mResourceIdName);
    SendPropertyEvent();
  }

  ResourceList* GetResourceList()
  {
    return mObjectHandle.Get<ResourceList*>();
  }

  void SendPropertyEvent()
  {
    Resource* resource = GetResourceList()->mOwner;

    MetaOperations::NotifyComponentsModified(resource);
  }
};

} // namespace Zero
