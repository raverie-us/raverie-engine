// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class TreeView;
class TreeEvent;
class ResourceLibraryDataSource;
class TileViewWidget;
class SpaceObjectSource;
class KeyboardEvent;
struct TreeFormatting;

void RegisterObjectViewEditors();

/// Object View show the objects in a space in with a Tree.
class ObjectView : public Composite
{
public:
  ZilchDeclareType(ObjectView, TypeCopyMode::ReferenceType);

  ObjectView(Composite* parent);
  ~ObjectView();

  // Widget Interface
  void UpdateTransform() override;
  void SetSpace(Space* space);

  void OnSpaceChange(Event*);
  void OnSpaceDestroyed(Event*);
  void OnTreeRightClick(TreeEvent* event);
  void OnRightMouseUp(MouseEvent* event);
  void OnDataActivated(DataEvent* event);
  void OnKeyDown(KeyboardEvent* event);

  void ShowObject(Cog* cog);
  void SelectAll();

private:
  void BuildFormat(TreeFormatting& formatting);
  void OnRename(ObjectEvent* event);
  void OnDelete(ObjectEvent* event);
  void OnRestore(ObjectEvent* event);
  void OnRestoreChildOrder(ObjectEvent* event);
  void OnSelectionChanged(Event* event);
  void OnMouseEnterRow(TreeEvent* e);
  void OnMouseExitRow(TreeEvent* e);

  TreeViewSearch* mSearch;
  TreeView* mTree;
  HandleOf<Space> mSpace;
  SpaceObjectSource* mSource;
  DataIndex mCommandIndex;
  HandleOf<ToolTip> mToolTip;
  Text* mNoSpaceText;
  Element* mDimSearch;
};

// Removed Entry
class RemovedEntry : public Object
{
public:
  ZilchDeclareType(RemovedEntry, TypeCopyMode::ReferenceType);

  RemovedEntry(Cog* parent, Guid childId);

  u64 ToId();
  static u64 ToId(Cog* parent, Guid childId);
  String GetRemovedChildName();
  DataNode* FindRemovedCogNode(DataNode* dataTree, Cog* currParent, Guid childGuid, Cog* archetypeRoot);
  String GetNameFromCogNode(DataNode* cogNode);

  Cog* mParent;
  Guid mChildId;
};

} // Namespace Zero
