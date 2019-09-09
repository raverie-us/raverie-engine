// MIT Licensed (see LICENSE.md).

#pragma once

namespace Zero
{

// Interface for tree view to edit RenderGroup hierarchies.
class RenderGroupSource : public DataSource
{
public:
  RenderGroupSource();

  // DataSource interface
  DataEntry* GetRoot() override;
  DataEntry* ToEntry(DataIndex index) override;
  DataIndex ToIndex(DataEntry* dataEntry) override;
  DataEntry* Parent(DataEntry* dataEntry) override;
  uint ChildCount(DataEntry* dataEntry) override;
  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override;
  bool IsExpandable(DataEntry* dataEntry) override;
  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override;
  bool SetData(DataEntry* dataEntry, const Any& variant, StringParam column) override;
  void CanMove(Status& status, DataEntry* source, DataEntry* destination, InsertMode::Type insertMode) override;
  void BeginBatchMove() override;
  bool Move(DataEntry* destinationEntry, DataEntry* movingEntry, InsertMode::Type insertMode) override;
  void EndBatchMove() override;

  // Checks if given RenderGroup has a parent and if that relationship is
  // established by its child list.
  bool InParentsChildList(RenderGroup* renderGroup);
  // Helper to remove entries from a RenderGroup's child list.
  void RemoveFromParent(RenderGroup* renderGroup);

  OperationQueue* mOperationQueue;
  Array<RenderGroup*> mRootGroups;
};

class RenderGroupHierarchies : public Composite
{
public:
  ZilchDeclareType(RenderGroupHierarchies, TypeCopyMode::ReferenceType);

  RenderGroupHierarchies(Composite* parent);
  ~RenderGroupHierarchies();

  void OnResourceModified(ResourceEvent* event);

  TreeView* mTree;
  RenderGroupSource* mSource;
};

} // namespace Zero
