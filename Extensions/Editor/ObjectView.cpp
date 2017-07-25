///////////////////////////////////////////////////////////////////////////////
///
/// \file ObjectView.cpp
/// 
/// 
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace ObjectViewUi
{
  const cstr cLocation = "EditorUi/ObjectView";
  Tweakable(Vec4, SearchIconColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, LocallyModifiedColor, Vec4(1, 1, 1, 1), cLocation);
  Tweakable(Vec4, LocallyRemovedColor, Vec4(1, 1, 1, 1), cLocation);
  Tweakable(Vec4, LocallyAddedColor, Vec4(1, 1, 1, 1), cLocation);
  Tweakable(Vec4, ChildOrderOverrideColor, Vec4(1, 1, 1, 1), cLocation);
}

const String cHiddenColumn = "Hidden";
const String cLockedColumn = "Locked";

//---------------------------------------------------------------------- Editors
DeclareEnum3(FlagState, Disabled, Enabled, Hidden);

class FlagEditor : public ValueEditor
{
public:
  typedef FlagEditor ZilchSelf;
  Element* mIcon;
  bool mEnabled;

  FlagEditor(Composite* parent, StringParam icon) 
    : ValueEditor(parent)
  {
    mIcon = CreateAttached<Element>(icon);
    mIcon->SetTranslation(Pixels(2,1,0));
    ConnectThisTo(mIcon, Events::LeftClick, OnLeftClick);
    ConnectThisTo(mIcon, Events::DoubleClick, OnDoubleClick);
  }

  void SetVariant(AnyParam variant) override
  {
    FlagState::Enum state = (FlagState::Enum)variant.Get<uint>();
    mEnabled = (state == FlagState::Enabled);
    mIcon->SetActive(state != FlagState::Hidden);
    Update();
  }

  void GetVariant(Any& variant) override
  {
    variant = mEnabled;
  }

  void OnLeftClick(MouseEvent* event)
  {
    mEnabled = !mEnabled;
    Update();
    ObjectEvent objectEvent(this);
    this->DispatchEvent(Events::ValueChanged, &objectEvent);
    event->Handled = true;
  }

  void OnDoubleClick(MouseEvent* event)
  {
    event->Handled = true;
  }

  void Update()
  {
    if(mEnabled)
      mIcon->SetColor(Vec4(1,1,1,1));
    else
      mIcon->SetColor(Vec4(0.1f, 0.1f, 0.1f, 0.2f));
  }
};

ValueEditor* CreateEyeEditor(Composite* parent, AnyParam data, u32 flags)
{
  return new FlagEditor(parent, "Dot");
}

ValueEditor* CreateLockEditor(Composite* parent, AnyParam data, u32 flags)
{
  return new FlagEditor(parent, "Dot");
}


DeclareEnum3(FilterSearchMode,
  ObjectName,
  ComponentName,
  ResourceName);

class ObjectViewFilter : public DataSourceFilter
{
public:
  StringRange SubFilterString;
  FilterSearchMode::Enum mSearchMode;

  ObjectViewFilter()
  {
    mSearchMode = FilterSearchMode::ObjectName;
  }

  void Filter(StringParam filterString) override
  {
    mFilteredList.Clear();

    if(filterString.Empty())
      return;

    DataEntry* root = mSource->GetRoot();

    SubFilterString = filterString.All();

    FilterSearchMode::Enum searchMode = mSearchMode;

    Rune rune = SubFilterString.Front();
    if(rune == '.')
    {
      SubFilterString.PopFront();
      searchMode = FilterSearchMode::ComponentName;
    }

    if(rune == '$')
    {
      SubFilterString.PopFront();
      searchMode = FilterSearchMode::ResourceName;
    }

    if(SubFilterString.Empty())
      return;

    // Perform the search
    switch(searchMode)
    {
    case FilterSearchMode::ObjectName:
      {
        BindMethodPtr<ObjectViewFilter, &ObjectViewFilter::FilterNames> filter(this);
        FilterNodes(filter, root);
        return;
      }
    case FilterSearchMode::ComponentName:
      {
        BindMethodPtr<ObjectViewFilter, &ObjectViewFilter::FilterComponentTypes> filter(this);
        FilterNodes(filter, root);
        return;
      }
    case FilterSearchMode::ResourceName:
      {
        BindMethodPtr<ObjectViewFilter, &ObjectViewFilter::FilterResourceUsage> filter(this);
        FilterNodes(filter, root);
        return;
      }
    }
  }

  bool FilterComponentTypes(DataEntry* entry)
  {
    Cog* cog = (Cog*)entry;

    forRange(Component* component, cog->GetComponents())
    {
      int priority = PartialMatch(SubFilterString, ZilchVirtualTypeId(component)->Name.All(), CaseInsensitiveCompare);

      if(priority != cNoMatch)
        return true;
    }

    return false;
  }

  bool FilterResourceUsage(DataEntry* entry)
  {
    Cog* cog = (Cog*)entry;

    forRange(Component* component, cog->GetComponents())
    {
      BoundType* metaType = ZilchVirtualTypeId(component);

      forRange(Property* property, metaType->GetProperties())
      {
        if(property->PropertyType->IsA(ZilchTypeId(Resource)))
        {
          Any var = property->GetValue(component);

          Resource* resource = var.Get<Resource*>();

          //safeguard (seems to happen during project changes if the editor doesn't restart)
          if(resource == nullptr)
            continue;

          int priority = PartialMatch(SubFilterString, resource->Name.All(), CaseInsensitiveCompare);

          if(priority != cNoMatch)
            return true;
        }
      }
    }

    return false;
  }

  bool FilterNames(DataEntry* entry)
  {
    Cog* cog = (Cog*)entry;
    int priority = PartialMatch(SubFilterString, cog->GetName().All(), CaseInsensitiveCompare);
    return priority != cNoMatch;
  }

};

class TreeViewSearchObjectTree : public TreeViewSearch
{
public:
  typedef TreeViewSearchObjectTree ZilchSelf;
  ObjectViewFilter* mObjectFilter;

  TreeViewSearchObjectTree(Composite* parent)
    :TreeViewSearch(parent, nullptr, nullptr)
  {
    mObjectFilter = new ObjectViewFilter();
    mFiltered = mObjectFilter;
    mSearchField->SetHintText("Search by object name... ");
    ConnectThisTo(mIcon, Events::LeftClick, OnMouseClick);
    ConnectThisTo(mIcon, Events::RightClick, OnMouseClick);
    mIcon->SetColor(ObjectViewUi::SearchIconColor);
  }

  void UpdateTransform() override
  {
    mIcon->SetColor(ObjectViewUi::SearchIconColor);
    TreeViewSearch::UpdateTransform();
  }

  void OnMouseClick(Event* event)
  {
    ContextMenu* menu = new ContextMenu(this);
    Mouse* mouse = Z::gMouse;
    menu->SetBelowMouse(mouse, Pixels(0,0) );

    ConnectMenu(menu, "By Name", OnName);
    ConnectMenu(menu, "By Component (.)", OnComponent);
    ConnectMenu(menu, "By Resource ($)", OnResource);
  }

  void OnName(Event* event)
  {
    mSearchField->SetText(String());
    mSearchField->SetHintText("Search by object name..");
    mObjectFilter->mSearchMode = FilterSearchMode::ObjectName;
  }

  void OnComponent(Event* event)
  {
    mSearchField->SetText(String());
    mSearchField->SetHintText("Search by component name..");
    mObjectFilter->mSearchMode = FilterSearchMode::ComponentName;
  }

  void OnResource(Event* event)
  {
    mSearchField->SetText(String());
    mSearchField->SetHintText("Search by resource name..");
    mObjectFilter->mSearchMode = FilterSearchMode::ResourceName;
  }
};

//------------------------------------------------------------------------------------ Removed Entry
ZilchDefineType(RemovedEntry, builder, type)
{
}

RemovedEntry::RemovedEntry(Cog* parent, Guid childId) :
  mParent(parent),
  mChildId(childId)
{

}

u64 RemovedEntry::ToId()
{
  return ToId(mParent, mChildId);
}

u64 RemovedEntry::ToId(Cog* parent, Guid childId)
{
  return (parent->GetId().ToUint64() ^ childId.mValue);
}

String RemovedEntry::GetRemovedChildName()
{
  // Walk up and find the nearest Archetype
  Cog* root = mParent->FindNearestArchetypeContext();
  Archetype* archetype = root->mArchetype;
  DataNode* dataTree = archetype->GetDataTree();

  DataNode* removedNode = FindRemovedCogNode(dataTree, mParent, mChildId, root);
  if(removedNode)
    return GetNameFromCogNode(removedNode);
  return "[Invalid]";
}

DataNode* RemovedEntry::FindRemovedCogNode(DataNode* dataTree, Cog* currParent, Guid childGuid,
                              Cog* archetypeRoot)
{
  if(currParent != archetypeRoot)
  {
    dataTree = FindRemovedCogNode(dataTree, currParent->GetParent(),
                                  currParent->mChildId, archetypeRoot);
  }

  DataNode* hierarchyNode = dataTree->FindChildWithTypeName("Hierarchy");
  DataNode* cogNode = hierarchyNode->FindChildWithUniqueNodeId(childGuid);
  return cogNode;
}

String RemovedEntry::GetNameFromCogNode(DataNode* cogNode)
{
  forRange(DataNode& childNode, cogNode->GetChildren())
  {
    if(childNode.mNodeType == DataNodeType::Value && childNode.mPropertyName == "Name")
    {
      // Show the Archetype name if it exists
      String archetypeName = cogNode->mInheritedFromId;
      if(!archetypeName.Empty())
      {
        StringRange name = archetypeName.FindLastOf(':');
        if(!name.Empty())
          archetypeName = archetypeName.SubString(name.End(), archetypeName.End());

        return String::Format("%s [%s]", childNode.mTextValue.c_str(), archetypeName.c_str());
      }
      return childNode.mTextValue;
    }
  }

  return "[Invalid]";
}

//-------------------------------------------------------------------------------------- Data Source
//Adapter class to bind a space and objects to a DataBaseSource.
class SpaceObjectSource : public DataSource
{
public:
  typedef u64 RemovedEntryId;
  typedef SpaceObjectSource ZilchSelf;

  Space* mSpace;

  // Entries are added as rows become visible
  HashMap<RemovedEntryId, RemovedEntry*> mVisibleRemovedEntries;

  SpaceObjectSource(Space* space)
  {
    mSpace = space;
    
    ConnectThisTo(space, Events::CogReplaced, OnCogReplaced);
  }

  ~SpaceObjectSource()
  {
    forRange(RemovedEntry* entry, mVisibleRemovedEntries.Values())
      delete entry;
    mVisibleRemovedEntries.Clear();
  }

  bool ShowObject(Cog* cog)
  {
    return !cog->mFlags.IsSet(CogFlags::ObjectViewHidden) && !cog->GetMarkedForDestruction();
  }

  void ClearRemovedObjects()
  {
    forRange(RemovedEntry* entry, mVisibleRemovedEntries.Values())
      delete entry;
    mVisibleRemovedEntries.Clear();
  }

  //DataSource Interface
  
  //Data base Indexing
  DataEntry* GetRoot() override
  {
    return mSpace;
  }

  //Safe Indexing
  DataEntry* ToEntry(DataIndex index) override
  {
    if(RemovedEntry* removed = mVisibleRemovedEntries.FindValue(index.Id, nullptr))
      return removed;

    Cog* object = Z::gTracker->GetObjectWithId(index.Id);
    // Even if the object id valid check to see
    // to see if the object is in the same space
    if(object && object->GetSpace() == mSpace)
      return object;
    else
      return nullptr;
  }

  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    Object* object = (Object*)dataEntry;
    if(Cog* cog = Type::DynamicCast<Cog*>(object))
    {
      CogId id = (Cog*)dataEntry;
      return id.ToUint64();
    }
    else
    {
      RemovedEntry* removed = (RemovedEntry*)object;
      if(removed)
        return removed->ToId();
      return DataIndex(-1);
    }
  }

  Handle ToHandle(DataEntry* dataEntry) override
  {
    Object* object = (Object*)dataEntry;
    return object;
  }

  DataEntry* Parent(DataEntry* dataEntry) override
  {
    Object* object = (Object*)dataEntry;
    if(Cog* cog = Type::DynamicCast<Cog*>(object))
    {
      // Space is the root object
      if(cog == mSpace)
        return nullptr;

      // Return parent of object
      Cog* parent = cog->GetParent();
      if(parent == nullptr)
        return cog->GetSpace();
      else
        return parent;
    }
    else
    {
      RemovedEntry* removed = (RemovedEntry*)object;
      return removed->mParent;
    }
  }

  uint ChildCount(DataEntry* dataEntry) override
  {
    if(dataEntry == mSpace)
    {
      // Count all non-transient roots
      uint count = 0;
      forRange(Cog& cog, mSpace->mRoots.All())
      {
        if(ShowObject(&cog))
          ++count;
      }
      return count;
    }
    else
    {
      // Count all non-transient objects
      Object* object = (Object*)dataEntry;
      if(RemovedEntry* removed = Type::DynamicCast<RemovedEntry*>(object))
        return 0;

      Cog* cog = (Cog*)object;

      if(Hierarchy* hierarchy = cog->has(Hierarchy))
      {
        uint i = 0;
        forRange(Cog& child, hierarchy->GetChildren())
        {
          if(ShowObject(&child))
            ++i;
        }

        // Count locally removed children
        LocalModifications* modifications = LocalModifications::GetInstance();
        if(ObjectState* state = modifications->GetObjectState(hierarchy))
          i += state->mRemovedChildren.Size();

        return i;
      }
      return 0;
    }
  }

  Cog* GetNextValidCog(Cog* cog)
  {
    // Skip over transient objects
    while(cog && !ShowObject(cog))
      cog = cog->FindNextSibling();
    
    return cog;
  }

  RemovedEntry* GetRemovedEntry(Cog* parent, ObjectState::ChildId& childId)
  {
    u64 hashId = RemovedEntry::ToId(parent, childId.mId);

    RemovedEntry* entry = mVisibleRemovedEntries.FindValue(hashId, nullptr);
    if(entry == nullptr)
    {
      entry = new RemovedEntry(parent, childId.mId);
      mVisibleRemovedEntries[hashId] = entry;
    }

    return entry;
  }

  RemovedEntry* GetFirstRemovedEntry(Cog* parentObject)
  {
    LocalModifications* modifications = LocalModifications::GetInstance();
    if(ObjectState* state = modifications->GetObjectState(parentObject->has(Hierarchy)))
    {
      ObjectState::ChildId childId = state->mRemovedChildren.All().Front();
      return GetRemovedEntry(parentObject, childId);
    }

    return nullptr;
  }

  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    if(dataEntry == mSpace)
    {
      Cog* prevObject = (Cog*)prev;
      Cog* child = prev ? (Cog*)prevObject->HierarchyLink.Next : &mSpace->mRoots.Begin().Front();

      // Skip over hidden objects
      while(child && !ShowObject(child))
        child = (Cog*)child->HierarchyLink.Next;

      return child;
    }
    else
    {
      Object* parentObject = (Object*)dataEntry;
      Cog* parentCog = (Cog*)parentObject;

      if(prev == nullptr)
      {
        // Find the first valid Cog
        if(!parentCog->GetChildren().Empty())
        {
          Cog* firstChild = &parentCog->GetChildren().Front();
          if(Cog* firstValidChild = GetNextValidCog(firstChild))
            return firstValidChild;
        }

        // If there were no valid Cogs, get the first valid removed object
        if(RemovedEntry* removedEntry = GetFirstRemovedEntry(parentCog))
          return removedEntry;

        Error("Should not get here - There should be a valid child if this function is called.");
      }
      else
      {
        // If we had a previous object, we can start the search from there
        Object* previousObject = (Object*)prev;

        // The previous could either be a Cog or a RemovedEntry
        if(Cog* previousCog = Type::DynamicCast<Cog*>(previousObject))
        {
          if(Cog* nextValidSibling = GetNextValidCog(previousCog->FindNextSibling()))
            return nextValidSibling;

          // If it was the last Cog in the list, move on to locally removed Cogs
          if(RemovedEntry* removedEntry = GetFirstRemovedEntry(parentCog))
            return removedEntry;

          Error("Should not get here - There should be a valid child if this function is called.");
        }

        LocalModifications* modifications = LocalModifications::GetInstance();

        // If the previous object wasn't a Cog, it's a removed object
        RemovedEntry* previousRemovedEntry = (RemovedEntry*)prev;
        ObjectState* state = modifications->GetObjectState(parentCog->has(Hierarchy));
        ReturnIf(state == nullptr, nullptr, "We've already returned a removed child, the object "
                                            "state should still be alive.");

        ObjectState::ChildrenMap::range removedChildren = state->GetRemovedChildren();
        while(!removedChildren.Empty())
        {
          ObjectState::ChildId currRemovedChild = removedChildren.Front();
          removedChildren.PopFront();

          if(currRemovedChild.mId == previousRemovedEntry->mChildId)
            return GetRemovedEntry(parentCog, removedChildren.Front());
        }
      }
    }
    return nullptr;
  }

  //Tree expanding
  bool IsExpandable(DataEntry* dataEntry) override
  {
    if(dataEntry==mSpace)
    {
      return true;
    }
    else
    {
      Object* object = (Object*)dataEntry;
      if(Cog* cog = Type::DynamicCast<Cog*>(object))
      {
        if(!cog->GetChildren().Empty())
          return true;
        LocalModifications* modifications = LocalModifications::GetInstance();
        if(Hierarchy* hierarchy = cog->has(Hierarchy))
        {
          if (ObjectState* state = modifications->GetObjectState(hierarchy))
            return !state->GetRemovedChildren().Empty();
        }
      }
      return false;
    }
  }

  //Data Base Cell Modification and Inspection
  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    Object* object = (Object*)dataEntry;
    if(Cog* cog = Type::DynamicCast<Cog*>(object))
    {
      LocalModifications* modifications = LocalModifications::GetInstance();
      if(column == CommonColumns::Name)
      {
        FormattedInPlaceText formatting;

        const String cNoNameDisplay = "(Unnamed)";

        formatting.mTextColor = Vec4(1);

        // Set the name of the cog
        formatting.mText = cog->GetName();
        if(cog->IsModifiedFromArchetype())
          formatting.mTextColor = ObjectViewUi::LocallyModifiedColor;
        if(formatting.mText.Empty())
          formatting.mText = cNoNameDisplay;

        // Add an icon for locally added
        if(modifications->IsObjectLocallyAdded(cog, false))
        {
          formatting.mCustomIcons.PushBack("ObjectLocallyAdded");
          //formatting.mTextColor = ObjectViewUi::LocallyAddedColor;// Vec4(1, 1, 1, 1);
          //formatting.mCustomIcon = "ObjectLocallyAdded";
        }

        // Add icon for child order modified
        if(modifications->IsChildOrderModified(cog->has(Hierarchy)))
        {
          formatting.mCustomIcons.PushBack("ChildOrderOverride");
          //formatting.mTextColor = ObjectViewUi::LocallyAddedColor;// Vec4(1, 1, 1, 1);
          //formatting.mCustomIcon = "ObjectLocallyAdded";
        }

        // If it has an Archetype, add the Archetype name at the end
        if(Archetype* archetype = cog->GetArchetype())
        {
          formatting.mAdditionalText = String::Format("[%s]", archetype->Name.c_str());
          formatting.mAdditionalTextColor = Vec4(1, 1, 1, 0.35f);
        }

        variant = formatting;
      }
      if(column == CommonColumns::Type)
      {
        variant = "Object";
      }
      if(column == cHiddenColumn)
      {
        if(cog->mFlags.IsSet(CogFlags::EditorViewportHidden))
          variant = (uint)FlagState::Disabled;
        else
          variant = (uint)FlagState::Enabled;
      }
      if(column == cLockedColumn)
      {
        if(cog->mFlags.IsSet(CogFlags::Locked))
          variant = (uint)FlagState::Enabled;
        else
          variant = (uint)FlagState::Disabled;
      }
    }
    else
    {
      RemovedEntry* removedEntry = (RemovedEntry*)object;
      if(column == CommonColumns::Name)
      {
        String removedName = removedEntry->GetRemovedChildName();
        if(removedName.Empty())
          removedName = "(Unnamed)";
        FormattedInPlaceText formatting;
        formatting.mText = removedName;
        //formatting.mTextColor = ObjectViewUi::LocallyRemovedColor; //Vec4(1, 1, 1, 0.4f);
        formatting.mTextColor = Vec4(1, 1, 1, 0.4f);
        formatting.mCustomIcons.PushBack("ObjectLocallyRemoved");
        //formatting.mCustomIcon = "ObjectLocallyRemoved";
        variant = formatting;
      }
      if(column == cHiddenColumn || column == cLockedColumn)
      {
        variant = (uint)FlagState::Hidden;
      }
    }
  }

  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    Object* object = (Object*)dataEntry;
    Cog* cog = Type::DynamicCast<Cog*>(object);
    if(cog == nullptr)
      return false;
    if(column == CommonColumns::Name)
    {
      Property* property = ZilchTypeId(Cog)->GetProperty("Name");
      ChangeAndQueueProperty(Z::gEditor->GetOperationQueue(), cog, property, variant);
    }
    else if(column == cHiddenColumn)
    {
      if(variant.Get<bool>())
        cog->SetEditorViewportHidden(false);
      else
        cog->SetEditorViewportHidden(true);
    }
    else if(column == cLockedColumn)
    {
      if(variant.Get<bool>())
        cog->SetLocked(true);
      else
        cog->SetLocked(false);
    }

    cog->GetSpace()->MarkModified();
    return true;
  }

  void CanMove(Status& status, DataEntry* source, DataEntry* destination,
               InsertMode::Type insertMode)
  {
    if(destination == mSpace)
    {
      status.SetSucceeded("Unparent");
    }
    else
    {
      Object* sourceObject = (Object*)source;
      Cog* sourceCog = Type::DynamicCast<Cog*>(sourceObject);
      Cog* destinationObj = Type::DynamicCast<Cog*>((Object*)destination);

      if(sourceCog == nullptr ||  destinationObj == nullptr)
      {
        status.SetFailed("Cannot move removed children", InsertError::NotSupported);
        return;
      }

      // Cannot move to a child object
      if(sourceCog->IsDescendant(destinationObj))
      {
        String message = String::Format("Cannot move to a child object");
        status.SetFailed(message, InsertError::Invalid);
        return;
      }

      // Cannot move objects above the level settings object
      if(destinationObj->GetLevelSettings() == destinationObj &&
         insertMode == InsertMode::Before)
      {
        status.SetFailed("", InsertError::NotSupported);
        return;
      }

      // Cannot move an object with a transform to an object without a Transform
      bool sourceHasTransform = (sourceCog->has(Transform) != nullptr);
      bool destinationHasTransform = (destinationObj->has(Transform) != nullptr);

      if(sourceHasTransform && !destinationHasTransform)
      {
        String message = String::Format("Cannot parent an object with a Transform to an object without a Transform");
        status.SetFailed(message, InsertError::Invalid);
        return;
      }

      String displayName = destinationObj->GetDescription();

      if(insertMode == InsertMode::On)
      {
        String message = String::Format("Parent to %s", displayName.c_str());
        status.SetSucceeded(message);
      }
      else
      {
        String message;
        if(insertMode == InsertMode::Before)
          message = String::Format("Move before %s", displayName.c_str());
        else
          message = String::Format("Move after %s", displayName.c_str());

        status.SetSucceeded(message);
      }
    }
  }

  void BeginBatchMove() override
  {
    OperationQueue* queue = Z::gEditor->GetOperationQueue();
    queue->BeginBatch();
    queue->SetActiveBatchName("ObjectView_BeginBatchMove");
  }

  bool Move(DataEntry* destinationEntry, DataEntry* movingEntry, InsertMode::Type insertMode)
  {
    OperationQueue* queue = Z::gEditor->GetOperationQueue();
    
    Cog* dest = (Cog*)destinationEntry;
    Cog* moving = (Cog*)movingEntry;

    if(moving->GetLevelSettings() == moving)
    {
      DoNotifyWarning("Cannot move LevelSettings", "Cannot move LevelSettings");
      return false;
    }

    // No need to do anything if they're the same object
    if(dest == moving)
      return false;

    if(destinationEntry == mSpace)
      DetachObject(queue, moving);
    else if(insertMode == InsertMode::On)
      AttachObject(queue, moving, dest);
    else
    {
      uint destIndex = dest->GetHierarchyIndex();
      if(insertMode == InsertMode::Before)
        MoveObject(queue, moving, dest->GetParent(), destIndex);
      else
        MoveObject(queue, moving, dest->GetParent(), destIndex + 1);
    }

    return true;
  }

  bool Move(DataEntry* destinationEntry, Array<DataIndex>& indicesToMove,
            InsertMode::Type insertMode) override
  {
    OperationQueue* queue = Z::gEditor->GetOperationQueue();
    Cog* dest = (Cog*)destinationEntry;

    // We want all objects to be sorted by their hierarchy index so they stay in the current order
    Array<Cog*> sortedCogs;
    forRange(DataIndex& index, indicesToMove.All())
      sortedCogs.PushBack((Cog*)ToEntry(index));

    Zero::Sort(sortedCogs.All(), CogHierarchyIndexCompareFn);

    if (insertMode == InsertMode::After)
      Zero::Reverse(sortedCogs.Begin(), sortedCogs.End());

    forRange(Cog* cog, sortedCogs)
      Move(destinationEntry, cog, insertMode);

    return true;
  }

  void EndBatchMove() override
  {
    OperationQueue* queue = Z::gEditor->GetOperationQueue();
    queue->EndBatch();
  }

  bool Remove(DataEntry* dataEntry) override
  {
    return false;
  }

  void OnMetaDrop(MetaDropEvent* e, DataEntry* mouseOver, InsertMode::Enum mode) override
  {
    // If an archetype was dropped on the row, attach 
    if(Archetype* archetype = e->Instance.Get<Archetype*>())
    {
      e->Handled = true;
      
      Object* mouseOverObject = (Object*)mouseOver;
      Cog* mouseOverCog = Type::DynamicCast<Cog*>(mouseOverObject);
      if(mouseOverCog == nullptr)
        return;

      // Set the tooltip and return if we're only testing
      if(e->Testing)
      {
        e->Result = String::Format("Create '%s'", archetype->Name.c_str());
        return;
      }

      // We want this operation to be able to be un-done
      OperationQueue* queue = Z::gEditor->GetOperationQueue();

      // The object will be created, parented, and potentially moved. All those operations should
      // be batched as the user made one operation
      queue->BeginBatch();
      queue->SetActiveBatchName("ObjectView_OnMetaDrop");

      // Create the object
      Cog* newChild = CreateFromArchetype(queue, mSpace, archetype, Vec3::cZero);

      if(mouseOver == mSpace)
      {
        // We don't need to attach to anything if it's on the Space
      }
      else if(mode == InsertMode::On)
      {
        Cog* parent = (Cog*)mouseOver;
        AttachObject(queue, newChild, parent, false);
      }
      else
      {
        Cog* destination = (Cog*)mouseOver;
        Cog* parent = destination->GetParent();

        uint destIndex = destination->GetHierarchyIndex();
        if(mode == InsertMode::Before)
          MoveObject(queue, newChild, parent, destIndex, false);
        else
          MoveObject(queue, newChild, parent, destIndex + 1, false);
      }

      // We've finished making modifications
      queue->EndBatch();

      // Select the new object
      MetaSelection* selection = Z::gEditor->GetSelection();
      selection->SelectOnly(newChild);
      selection->FinalSelectionChanged();
    }
  }

  void OnCogReplaced(CogReplaceEvent* e)
  {
    // We need to send replace events on the entire hierarchy
    SendReplaceEvents(e->mOldCog, e->mNewCog);
  }

  void SendReplaceEvents(Cog* oldCog, Cog* newCog)
  {
    CogReplaceEvent eventToSend(oldCog, newCog);

    // Forward the event to ourself as a data event
    DispatchEvent(Events::DataReplaced, &eventToSend);

    // Walk all children and dispatch replace events for each one
    forRange(Cog& oldChild, oldCog->GetChildren())
    {
      if(Cog* newChild = newCog->FindChildByChildId(oldChild.mChildId))
        SendReplaceEvents(&oldChild, newChild);
    }
  }
};

Cog* GetCogFromIndex(DataIndex index)
{
  return Z::gTracker->GetObjectWithId(index.Id);
}

//-------------------------------------------------------------- Space Selection
//Simple adapter class to bind a engine selection to a DataSelection.
class SpaceSelection : public DataSelection
{
public:
  MetaSelection* mSelection;

  SpaceSelection(MetaSelection* selection)
    :mSelection(selection)
  { 

  }

  void GetSelected(Array<DataIndex>& selected) override
  {
    forRange(Cog* object, mSelection->AllOfType<Cog>())
      selected.PushBack(object->GetId().ToUint64());
  }

  uint Size() override
  {
    return mSelection->Count();
  }

  bool IsSelected(DataIndex index) override
  {
    Cog* object = GetCogFromIndex(index);
    if(object)
      return mSelection->Contains( object );
    else
      return false;
  }

  void Select(DataIndex index, bool sendsEvents) override
  {
    Cog* object = GetCogFromIndex(index);
    if(object)
      mSelection->Add(object, (SendsEvents::Enum)sendsEvents);
  }

  void Deselect(DataIndex index) override
  {
    Cog* object = GetCogFromIndex(index);
    if(object)
      mSelection->Remove(object);
  }

  void SelectNone(bool sendsEvents) override
  {
    mSelection->Clear((SendsEvents::Enum)sendsEvents);
  }

  /// Selection modified
  void SelectionModified() override
  {
    mSelection->SelectionChanged();
    DataSelection::SelectionModified();
  }

  void SelectFinal() override
  {
    mSelection->FinalSelectionChanged();
    DataSelection::SelectFinal();
  }
};

void RegisterObjectViewEditors()
{
  ValueEditorFactory* factory = ValueEditorFactory::GetInstance();
  factory->RegisterEditor("EyeEditor", CreateEyeEditor);
  factory->RegisterEditor("LockEditor", CreateLockEditor);
}

//------------------------------------------------------------------ Object View
ZilchDefineType(ObjectView, builder, type)
{
  
}

ObjectView::ObjectView(Composite* parent)
  : Composite(parent)
{
  this->SetLayout(CreateStackLayout());

  mSearch = new TreeViewSearchObjectTree(this);
  mSearch->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(18));
  mDimSearch = CreateAttached<Element>(cWhiteSquare);
  mDimSearch->SetNotInLayout(true);
  mDimSearch->SetColor(Vec4::cZero);
  mDimSearch->SetActive(false);

  mTree = new TreeView(this);

  mSearch->mTreeView = mTree;

  mTree->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);

  mSource = nullptr;

  TreeFormatting formatting;
  BuildFormat(formatting);
  mTree->SetFormat(formatting);

  mNoSpaceText = new Text(this, "NotoSans-Regular", 18);
  mNoSpaceText->SetText("no Level loaded...");
  mNoSpaceText->SizeToContents();
  mNoSpaceText->SetVisible(false);
  mNoSpaceText->SetNotInLayout(true);

  ConnectThisTo(mTree, Events::TreeRightClick, OnTreeRightClick);
  ConnectThisTo(mTree, Events::KeyDown, OnKeyDown);
  ConnectThisTo(mTree, Events::MouseEnterRow, OnMouseEnterRow);
  ConnectThisTo(mTree, Events::MouseExitRow, OnMouseExitRow);
}

ObjectView::~ObjectView()
{
  SafeDelete(mSource);
}

void ObjectView::UpdateTransform()
{
  Rect rect = mSearch->GetLocalRect();
  PlaceWithRect(rect, mDimSearch);

  mNoSpaceText->SetColor(Vec4(1, 1, 1, 0.4f));
  mNoSpaceText->SetTranslation(ToVector3(mSize * 0.5f - mNoSpaceText->GetMinSize() * 0.5f));
  Composite::UpdateTransform();
}

void ObjectView::SetSpace(Space* space)
{
  // Space already set
  Space* oldSpace = mSpace;
  if(oldSpace == space)
    return;

  if (oldSpace)
    DisconnectAll(oldSpace, this);

  mSpace = space;

  mDimSearch->SetActive(false);
  mSearch->SetColor(Vec4(1, 1, 1, 1));
  mNoSpaceText->SetVisible(false);

  // Create a new data source and selection
  SpaceObjectSource* oldSource = mSource;
  mSource = new SpaceObjectSource(mSpace);

  mTree->SetDataSource(mSource);
  mTree->SetSelection(new SpaceSelection(Z::gEditor->GetSelection()));

  // Delete the old source after we set the new data source since setting 
  // the data source can reference the old data source.
  SafeDelete(oldSource);

  ConnectThisTo(mSource, Events::DataActivated, OnDataActivated);
  ConnectThisTo(space, Events::SpaceObjectsChanged, OnSpaceChange);
  ConnectThisTo(space, Events::SpaceDestroyed, OnSpaceDestroyed);

  MetaSelection* selection = Z::gEditor->GetSelection();
  ConnectThisTo(selection, Events::SelectionChanged, OnSelectionChanged);
  ConnectThisTo(selection, Events::SelectionFinal, OnSelectionChanged);
}

void ObjectView::OnKeyDown(KeyboardEvent* event)
{
  if(event->Handled)
    return;

  if(event->Key == Keys::F)
  {
    FocusOnSelectedObjects();
  }

  if(event->Key == Keys::F2)
  {
    Cog* object = Z::gEditor->GetSelection()->GetPrimaryAs<Cog>();

    if(object)
    {
      DataIndex index(object->GetId().ToUint64());
      TreeRow* row = mTree->FindRowByIndex(index);
      if(row)
        row->Edit(CommonColumns::Name);
    }
  }

  if(event->CtrlPressed && event->Key == Keys::A)
  {
    SelectAll();
  }

  ExecuteShortCuts(mSpace, nullptr, event);
}


void ObjectView::ShowObject(Cog* cog)
{
  DataIndex index = mSource->ToIndex(cog);
  mTree->ShowRow(index);
}

void ObjectView::SelectAll()
{
  mTree->SelectAll();
}

void ObjectView::BuildFormat(TreeFormatting& formatting)
{
  formatting.Flags.SetFlag(FormatFlags::ShowHeaders);

  // The name column
  ColumnFormat* format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = CommonColumns::Name;
  format->HeaderName = "Name";
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 2;
  format->MinWidth = Pixels(60.0f);
  format->Editable = true;
  format->CustomEditor = cDefaultValueEditor;

  // Hidden button
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->FixedSize = Pixels(20, 20);
  format->MinWidth = Pixels(20);
  format->Name = cHiddenColumn;
  format->HeaderIcon = "Eye";
  format->Editable = true;
  format->ColumnType = ColumnType::Fixed;
  format->CustomEditor = "EyeEditor";

  // Locked button
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->FixedSize = Pixels(20, 20);
  format->MinWidth = Pixels(20);
  format->Name = cLockedColumn;
  format->HeaderIcon = "Lock";
  format->Editable = true;
  format->ColumnType = ColumnType::Fixed;
  format->CustomEditor = "LockEditor";
}

void ObjectView::OnRename(ObjectEvent* event)
{
  TreeRow* row = mTree->FindRowByIndex(mCommandIndex);
  row->Edit(CommonColumns::Name);
}

void ObjectView::OnDelete(ObjectEvent* event)
{
  MetaSelection* selection = Z::gEditor->GetSelection();
  if(selection->Empty())
    return;

  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("ObjectView_OnDelete");
  forRange(Cog* object, selection->AllOfType<Cog>())
    DestroyObject(queue, object);

  selection->Clear();
  selection->FinalSelectionChanged();

  queue->EndBatch();
}

void ObjectView::OnSpaceChange(Event*)
{
  mSource->ClearRemovedObjects();

  // Rebuild the tree now that there is new objects
  DataEvent dataEvent;
  mSource->DispatchEvent(Events::DataModified, &dataEvent);
}

void ObjectView::OnSpaceDestroyed(Event*)
{
  mDimSearch->SetActive(true);
  mSearch->SetColor(Vec4(1, 1, 1, 0.5f));
  mNoSpaceText->SetVisible(true);
  mTree->SetDataSource(nullptr);
  SafeDelete(mSource);
}

void ObjectView::OnSelectionChanged(Event* event)
{
  // Need to update selected objects
  mTree->MarkAsNeedsUpdate();

  // Do not show row when focus change is 
  // caused by the object view itself
  if(this->HasFocus())
    return;

  // Show the object row that was selected
  
  MetaSelection* selection = Z::gEditor->GetSelection();
  Cog* primary = selection->GetPrimaryAs<Cog>();
  if(primary && selection->Count() == 1)
    ShowObject(primary);
}

void ObjectView::OnMouseEnterRow(TreeEvent* e)
{
  mToolTip.SafeDestroy();
  Object* object = (Object*)mSource->ToEntry(e->Row->mIndex);
  if(object == nullptr)
    return;
  
  String toolTipMessage;
  ToolTipColor::Enum toolTipColor;

  if(RemovedEntry* removed = Type::DynamicCast<RemovedEntry*>(object))
  {
    Cog* cog = removed->mParent->FindNearestArchetypeContext();
    toolTipMessage = "Object has been locally removed from the Archetype.\n\n"
                     "Right click to restore it.";
    toolTipColor = ToolTipColor::Red;
  }
  else
  {
    Cog* cog = (Cog*)object;
    if(LocalModifications::GetInstance()->IsObjectLocallyAdded(cog, false))
    {
      Cog* archetypeParent = cog->GetParent()->FindNearestArchetypeContext();
      String archetypeName = archetypeParent->GetArchetype()->Name;
      toolTipMessage = String::Format("Object is locally added to the '%s' Archetype.", archetypeName.c_str());
      toolTipColor = ToolTipColor::Green;
    }
    else if(LocalModifications::GetInstance()->IsChildOrderModified(cog->has(Hierarchy)))
    {
      toolTipMessage = "Child order is locally modified.\n\n"
                       "This objects children will ignore the order specified in the Archetype.\n\n"
                       "Right click to revert order.";

      toolTipColor = ToolTipColor::Yellow;
    }
  }

  if(!toolTipMessage.Empty())
  {
    ToolTip* toolTip = new ToolTip(this);
    toolTip->SetText(toolTipMessage);

    Rect rect = e->Row->GetScreenRect();

    ToolTipPlacement placement;
    placement.mScreenRect = rect;
    placement.mHotSpot = rect.Center();
    placement.SetPriority(IndicatorSide::Left, IndicatorSide::Right,
                          IndicatorSide::Bottom, IndicatorSide::Top);

    toolTip->SetArrowTipTranslation(placement);
    toolTip->SetColor(toolTipColor);
    mToolTip = toolTip;
  }
}

void ObjectView::OnMouseExitRow(TreeEvent* e)
{
  mToolTip.SafeDestroy();
}

void ObjectView::OnTreeRightClick(TreeEvent* event)
{
  ContextMenu* menu = new ContextMenu(event->Row);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0));
  mCommandIndex = event->Row->mIndex;

  Object* object = (Object*)mSource->ToEntry(mCommandIndex);
  if(Cog* cog = Type::DynamicCast<Cog*>(object))
  {
    ConnectMenu(menu, "Rename", OnRename);
    ConnectMenu(menu, "Delete", OnDelete);
    if(LocalModifications::GetInstance()->IsChildOrderModified(cog->has(Hierarchy)))
      ConnectMenu(menu, "Restore Child Order", OnRestoreChildOrder);
  }
  else
  {
    ConnectMenu(menu, "Restore", OnRestore);
  }
}

void ObjectView::OnRestore(ObjectEvent* event)
{
  RemovedEntry* removedEntry = (RemovedEntry*)mSource->ToEntry(mCommandIndex);

  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  Cog* parent = removedEntry->mParent;
  ObjectState::ChildId childId("Cog", removedEntry->mChildId);
  RestoreLocallyRemovedChild(queue, parent->has(Hierarchy), childId);
}

void ObjectView::OnRestoreChildOrder(ObjectEvent* event)
{
  MetaSelection* selection = Z::gEditor->GetSelection();
  if(selection->Empty())
    return;

  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("RestoreChildOrder");
  forRange(Cog* object, selection->AllOfType<Cog>())
    RestoreChildOrder(queue, object->has(Hierarchy));

  queue->EndBatch();
}

void ObjectView::OnDataActivated(DataEvent* event)
{
  Cog* cog = GetCogFromIndex(event->Index);
  if(cog)
  {
    CameraFocusSpace(cog->GetSpace());
  }
}

}//namespace Zero
