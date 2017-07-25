///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorCommands.cpp
/// Various editor commands.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------- Selection
void AddToSelection(Space* space, MetaSelection* selection, BoundType* boundType, 
                    SelectComponentMode::Enum mode)
{
  Space::range r = space->AllObjects();
  while(!r.Empty())
  {
    Cog* cog = &r.Front();

    if(!cog->mFlags.IsSet(CogFlags::Protected))
    {
      //Test to see if the component is present
      Component* component = cog->QueryComponentType(boundType);

      if(mode == SelectComponentMode::WithComponent)
      {
        if(component)
          selection->Add(cog, SendsEvents::False);
      }
      else
      {
        if(component == nullptr)
          selection->Add(cog, SendsEvents::False);
      }
    }

    r.PopFront();
  }

  selection->FinalSelectionChanged();
}

void Deselect(Editor* editor, Space* space)
{
  MetaSelection* selection = editor->GetSelection();
  selection->Clear();
}

//----------------------------------------------------------------- Manipulation
void DuplicateSelection(Editor* editor, Space* space)
{
  SaveSelectionToClipboard(editor, space);
  LoadObjectFromClipboard(editor, space);
}

void DeleteSelectedObjects(Editor* editor, Space* space)
{
  MetaSelection* selection = editor->GetSelection();

  Array<Cog*> cogs;
  FilterChildrenAndProtected(cogs, selection);

  OperationQueue* queue = editor->GetOperationQueue();
  queue->BeginBatch();

  String spaceName = space != nullptr ? space->GetName() : "Null";
  String batchName = BuildString("DeleteSelectedObjects in space \"", spaceName, "\"");
  queue->SetActiveBatchName(batchName);

  Array<Cog*>::range r = cogs.All();
  while(!r.Empty())
  {
    Cog* object = r.Front();
    DestroyObject(queue, object);
    r.PopFront();
  }
  selection->Clear();
  selection->FinalSelectionChanged();

  queue->EndBatch();
}

//--------------------------------------------------------------- QuickSave/Load
void ClearSelection(Editor* editor, Space* space)
{
  MetaSelection* active = editor->GetSelection();
  active->Clear();
  active->FinalSelectionChanged();
}

// Determines whether or not all objects in the Selection have a shared parent
// Returns the Id of the shared parent if it exists
Cog* GetSharedParent(MetaSelection* selection)
{
  Cog* sharedParent = nullptr;

  if(Cog* primary = selection->GetPrimaryAs<Cog>())
  {
    Cog* parent = primary->GetParent();
    if(parent)
    {
      sharedParent = parent;

      MetaSelection::rangeType<Cog> r = selection->AllOfType<Cog>();
      for(;!r.Empty(); r.PopFront())
      {
        Cog* object = r.Front();
        if(Cog* parent = object->GetParent())
        {
          if(parent != sharedParent)
            return nullptr;
        }
        else
        {
          return nullptr;
        }
      }
    }
  }

  return sharedParent;
}

uint GetFirstHierarchyIndex(MetaSelection* selection)
{
  if(selection->Count() > 1)
    return uint(-1);

  Cog* primary = selection->GetPrimaryAs<Cog>();
  if(primary == nullptr)
    return uint(-1);

  // + 1 to paste it after this object
  return primary->GetHierarchyIndex() + 1;
}

void SaveSelectionToClipboard(Editor* editor, Space* space)
{
  MetaSelection* selection = editor->GetSelection();
  editor->mLastCopiedParent = GetSharedParent(selection);
  editor->mPasteHierarchyIndex = GetFirstHierarchyIndex(selection);

  CogSavingContext context;

  ObjectSaver saver;
  saver.OpenBuffer();
  saver.SetSerializationContext(&context);

  CogSerialization::SaveSelection(saver, selection);

  uint size = saver.GetBufferSize();
  byte* data = (byte*)zAllocate(size+1);
  saver.ExtractInto(data, size);
  data[size] = 0;

  OsShell* platform = Z::gEngine->has(OsShell);
  platform->SetClipboardText(StringRange((cstr)data));
  editor->mLastCopy = platform->GetClipboardText();

  zDeallocate(data);
}

void SaveToClipBoardAndDelete(Editor* editor, Space* space)
{
  SaveSelectionToClipboard(editor, space);
  DeleteSelectedObjects(editor, space);

  // The selected object was removed, so just move the object to the end
  editor->mPasteHierarchyIndex = uint(-1);
}

void LoadObjectFromClipboard(Editor* editor, Space* space)
{
  PushErrorContext("Paste Object From Clipboard");

  OsShell* platform = Z::gEngine->has(OsShell);
  String text = platform->GetClipboardText();
  if(!text.Empty())
  {
    Status status;
    ObjectLoader loader;
    bool loaded = loader.OpenBuffer(status, text.All());
    if(loaded)
    {
      // Printing for debugging
      //loader.GetCurrent()->Print(0);
      MetaSelection* selection = editor->GetSelection();

      loader.Start("Selection", "selection", StructureType::Object);
      Space::range objects = space->AddObjectsFromStream("Clipboard", loader);
      loader.End("Selection", StructureType::Object);

      selection->Clear();

      // Get the copied parent
      Cog* sharedParent = editor->mLastCopiedParent;

      // If it's from a different clipboard, we don't want to parent, so
      // set it to nullptr
      if(editor->mLastCopy != text)
        sharedParent = nullptr;

      OperationQueue* queue = editor->GetOperationQueue();
      queue->BeginBatch();
      queue->SetActiveBatchName("LoadObjectFromClipboard");

      bool first = true;
      for(; !objects.Empty(); objects.PopFront())
      {
        Cog* object = &objects.Front();        

        // If the object doesn't have a parent, it was a root object in the
        // object list
        bool pasteRoot = (object->GetParent() == nullptr);

        // When copying a child object, the id of the new object will be
        // conflicted with the one it was copied from. We need to 
        // invalidate the id so that it's assigned a new one when attached
        // to the shared parent
        if(sharedParent && pasteRoot)
        {
          object->mChildId = PolymorphicNode::cInvalidUniqueNodeId;

          // Attach it to the new parent
          object->AttachTo(sharedParent);
        }

        // The child objects that were pasted don't need to be moved in any way
        if(pasteRoot)
        {
          // This object was created
          ObjectCreated(queue, object);

          // For now, we're only 
          if(first && sharedParent && editor->mPasteHierarchyIndex != uint(-1))
          {
            MoveObjectIndex(queue, object, editor->mPasteHierarchyIndex);
            first = false;
          }

          // Add the object to the selection, wait until the end to push changes
          selection->Add(object, SendsEvents::False);
        }
      }

      queue->EndBatch();

      // We don't want to select all child objects of what was copied
      Array<Cog*> newSelection;
      FilterChildrenAndProtected(newSelection, selection);

      selection->Clear();
      forRange(Cog* cog, newSelection.All())
        selection->Add(cog, SendsEvents::False);

      // Signal that the selection has changed
      selection->FinalSelectionChanged();
    }
  }
}

Cog* GetTopParent(Cog* cog)
{
  if(cog->GetParent())
    return GetTopParent(cog->GetParent());
  else
    return cog;
}

void AddChildrenToSelection(MetaSelection* selection, Cog* object)
{
  selection->Add(object);

  if(Hierarchy* h = object->has(Hierarchy))
  {
    HierarchyList::range r = h->Children.All();
    while(!r.Empty())
    {
      AddChildrenToSelection(selection, &r.Front());
      r.PopFront();
    }
  }
}

Cog* GetParentOfSelection(MetaSelection* selection)
{
  MetaSelection::rangeType<Cog> r = selection->AllOfType<Cog>();
  if(!r.Empty())
  {
    Cog* object = r.Front();
    return GetTopParent(object);
  }
  return nullptr;
}

void SelectAllInTree(Editor* editor, Space* space)
{
  MetaSelection* active = editor->GetSelection();
  Cog* cog = GetParentOfSelection(active);
  if(cog)
  {
    AddChildrenToSelection(active, cog);
  }
}

Cog* GetParentOfSelection(Editor* editor, Space* space)
{
  MetaSelection* selection = editor->GetSelection();
  MetaSelection::rangeType<Cog> r = selection->AllOfType<Cog>();
  if(!r.Empty())
  {
    Cog* object = r.Front();
    return GetTopParent(object);
  }
  return nullptr;
}

Cog* SelectTopOfTree(Editor* editor, Space* space)
{
  MetaSelection* active = editor->GetSelection();
  Cog* cog = GetParentOfSelection(active);
  if(cog)
  {
    active->Clear();
    active->Add(cog);
    active->FinalSelectionChanged();
    return cog;
  }
  return nullptr;
}

void SelectTopInTreeNoObj(Editor* editor, Space* space)
{
  SelectTopOfTree(editor, space);
}

void SelectParent(Editor* editor, Space* space)
{
  MetaSelection* active = editor->GetSelection();
  MetaSelection::rangeType<Cog> r = active->AllOfType<Cog>();
  while(!r.Empty())
  {
    Cog* object = r.Front();
    if(object->GetParent())
    {
      active->Clear();
      active->Add(object->GetParent());
      active->FinalSelectionChanged();
      return;
    }
    r.PopFront();
  }
}

void SelectChild(Editor* editor, Space* space)
{
  MetaSelection* selection = editor->GetSelection();
  MetaSelection::rangeType<Cog> r = selection->AllOfType<Cog>();
  while(!r.Empty())
  {
    Cog* object = r.Front();
    Hierarchy* hierarchy = object->has(Hierarchy);
    if(hierarchy && !hierarchy->Children.Empty())
    {
      selection->Clear();
      selection->Add(&hierarchy->Children.Front());
      selection->FinalSelectionChanged();
      return;
    }
    r.PopFront();
  }
}

bool CogHierarchyIndexCompareFn(Cog* lhs, Cog* rhs)
{
  return lhs->GetHierarchyIndex() < rhs->GetHierarchyIndex();
}

// Create a transform at the center of the selected objects
// and parent them to it
void GroupSelected(Editor* editor, Space* space)
{
  MetaSelection* selection = editor->GetSelection();

  // If there's a shared parent, we want to make this new root under
  // the shared parent
  Cog* sharedParent = GetSharedParent(selection);

  //Count Transforms and compute center
  Vec3 center = Vec3::cZero;
  uint transformCount = 0;

  uint lowestHierarchyIndex = uint(-1);
  forRange(Cog* object, selection->AllOfType<Cog>())
  {
    Transform* transform = object->has(Transform);
    if(transform)
    {
      // If there's a shared parent, we want to calculate the center in
      // the parents local space so that if snapped (based on the Translate
      // Gizmo), it will be snapped in the correct space
      if(sharedParent)
        center += transform->GetTranslation();
      else
        center += transform->GetWorldTranslation();
      
      ++transformCount;
    }

    // Only take into account objects under the shared parent. If there was no shared parent,
    // it will only get root objects (which is where the grouped object will go)
    if(object->GetParent() == sharedParent)
    {
      uint hierarchyIndex = object->GetHierarchyIndex();
      if (hierarchyIndex < lowestHierarchyIndex)
        lowestHierarchyIndex = hierarchyIndex;
    }
  }

  //If any of the objects have a transform
  if(transformCount > 0)
  {
    float inverseCount = 1.0f / float(transformCount);
    center *= inverseCount;
  }

  OperationQueue* queue = editor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("ObjectGroupSelection");

  // Snap the center if snapping is enabled in the translation gizmo.
  // This snapping is a convenience for now.  In the future it really
  // needs to be a toggle-property on a command-component for 'GroupSelected'
  Cog* translateToolCog = editor->Tools->GetToolByName("TranslateTool");

  ObjectTranslateTool* tTool = translateToolCog->has(ObjectTranslateTool);
  if(tTool && tTool->GetSnapping( ))
  {
    center = GizmoSnapping::GetSnappedPosition(center, Vec3(0, 0, 0), Quat::cIdentity,
      tTool->GetDragMode( ), GizmoSnapMode::WorldGrid, tTool->GetSnapDistance( ));
  }

  //Create the transform object
  Cog* rootObject = space->CreateAt(CoreArchetypes::Transform, center);

  rootObject->SetName("Root");
  rootObject->ClearArchetype();
  ObjectCreated(queue, rootObject);

  // Attach the root to the shared parent. The translation is already in
  // the shared parents space, so don't parent relatively
  if (sharedParent)
    AttachObject(queue, rootObject, sharedParent, false);

  MoveObjectIndex(queue, rootObject, lowestHierarchyIndex);

  // We want to maintain the order of all objects
  Array<Cog*> orderedSelection;
  forRange(Cog* cog, selection->AllOfType<Cog>())
    orderedSelection.PushBack(cog);

  Zero::Sort(orderedSelection.All(), CogHierarchyIndexCompareFn);

  forRange(Cog* child, orderedSelection)
    AttachObject(queue, child, rootObject);

  queue->EndBatch();

  editor->GetSelection()->SelectOnly(rootObject);
}

void SelectSibling(Editor* editor, Space* space)
{
  MetaSelection* active = editor->GetSelection();
  MetaSelection::rangeType<Cog> r = active->AllOfType<Cog>();
  while(!r.Empty())
  {
    Cog* object = r.Front();
    //Is this a child object at all?
    if(object->GetParent())
    {
      //If this is not the last object in the parent's child list
      //select the next object in the parent's child list
      Hierarchy* hierarchy = object->GetParent()->has(Hierarchy);
      Cog* next = (Cog*)HierarchyList::Next(object);
      if(next!=hierarchy->Children.End())
      {
        active->Clear();
        active->Add(next);
        active->FinalSelectionChanged();
        return;
      }
      else
      {
        //go back to the first
        active->Clear();
        active->Add(&hierarchy->Children.Front());
        active->FinalSelectionChanged();
        return;
      }
    }
    r.PopFront();
  }
}

void SelectAll(Editor* editor, Space* space)
{
  MetaSelection* active = editor->GetSelection();
  AddToSelection(space, active, ZilchTypeId(Transform), SelectComponentMode::WithComponent);
}

/// Parents all selected objects to the primary selected object
/// (highlighted in orange)
void ParentToPrimary(Editor* editor, Space* space)
{
  MetaSelection* selection = editor->GetSelection();
  Cog* parent = selection->GetPrimaryAs<Cog>();
  MetaSelection::rangeType<Cog> range = selection->AllOfType<Cog>();

  OperationQueue* queue = editor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("ParentSelectionToPrimary");

  for(; !range.Empty(); range.PopFront())
  {
    Cog* child = range.Front();
    if(child == parent)
      continue;

    //child->AttachToRelative(parent);
    AttachObject(queue, child, parent);
  }

  queue->EndBatch();
}

/// Detaches all selected objects from their parents
void DetachSelected(Editor* editor, Space* space)
{
  MetaSelection* selection = editor->GetSelection();
  MetaSelection::rangeType<Cog> range = selection->AllOfType<Cog>();

  OperationQueue* queue = editor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("DetachAllSelected");

  for(; !range.Empty(); range.PopFront())
  {
    Cog* cog = range.Front();
    //cog->DetachRelative();
    DetachObject(queue, cog);
  }

  queue->EndBatch();
}

void FlattenTree(Cog* cog, OperationQueue* queue)
{
  // Attempt to grab the hierarchy component
  Hierarchy* hierarchy = cog->has(Hierarchy);

  // Do nothing if there is no hierarchy component
  if (hierarchy != nullptr)
  {
    // We cannot use a range because it will be modified as we iterate over it
    while(!hierarchy->Children.Empty())
    {
      Cog* child = &hierarchy->Children.Front();
      FlattenTree(child, queue);
    }
  }
  
  //cog->DetachRelative();
  DetachObject(queue, cog);
}

/// Detaches all child objects of the selected object
void FlattenTree(Editor* editor, Space* space)
{
  MetaSelection* selection = editor->GetSelection();
  MetaSelection::rangeType<Cog> range = selection->AllOfType<Cog>();

  OperationQueue* queue = editor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("FlattenTree");

  for(; !range.Empty(); range.PopFront())
  {
    Cog* cog = range.Front();

    FlattenTree(cog, queue);
  }

  queue->EndBatch();
}

void ExpandExtents(Cog* object, Vec3 center, float& size)
{
  if(Transform* tx = object->has(Transform))
  {
    Vec3 objectPosition = tx->GetWorldTranslation();
    Vec3 relative = Math::Abs(objectPosition - center);

    //if(Model* model = object->has(Model))
    //{
    //  Mesh* mesh = model->mMesh;
    //  size = Math::Max(relative.x + mesh->Extents.x * tx->Scale.x, size);
    //  size = Math::Max(relative.y + mesh->Extents.y * tx->Scale.y, size);
    //  size = Math::Max(relative.z + mesh->Extents.z * tx->Scale.z, size);
    //}

    if(Collider* collider = object->has(Collider))
    {
      Vec3 halfExtents = collider->mAabb.GetHalfExtents();
      size = Math::Max(relative.x + halfExtents.x, size);
      size = Math::Max(relative.y + halfExtents.y, size);
      size = Math::Max(relative.z + halfExtents.z, size);
    }

    if(Hierarchy* h = object->has(Hierarchy))
    {
      HierarchyList::range r = h->GetChildren();
      forRange(Cog& child, r)
      {
        ExpandExtents(&child, center, size);
      }
      
    }

  }

}


void MoveToLookPoint(Editor* editor, Space* space)
{ 
  Cog* camera = space->FindObjectByName(SpecialCogNames::EditorCamera);
  if(camera == nullptr)
    return;

  if(EditorCameraController* controller = camera->has(EditorCameraController))
  {
    BoundType* transformMeta = ZilchTypeId(Transform);
    OperationQueue* opQueue = editor->GetOperationQueue();
    opQueue->BeginBatch();
    opQueue->SetActiveBatchName("Move objects to LookPoint");

    Any newTranslation = controller->GetLookTarget();

    MetaSelection* activeSelection = Z::gEditor->GetSelection();
    MetaSelection::rangeType<Cog> range = activeSelection->AllOfType<Cog>();
    for(; !range.Empty(); range.PopFront())
    {
      Cog* cog = range.Front();
      if(Transform* transform = cog->has(Transform))
      {
        Property* translationProp = transformMeta->GetProperty("Translation");
        ChangeAndQueueProperty(opQueue, transform, translationProp, newTranslation);
      }
    }
    opQueue->EndBatch();
  }
}

void ResetTransform(Editor* editor, Space* space)
{
  MetaSelection* activeSelection = Z::gEditor->GetSelection();
  MetaSelection::rangeType<Cog> r = activeSelection->AllOfType<Cog>();

  BoundType* transformMeta = ZilchTypeId(Transform);
  OperationQueue* opQueue = editor->GetOperationQueue();
  opQueue->BeginBatch();
  opQueue->SetActiveBatchName("Reset objects' Transforms");
  
  Any newTranslation = Vec3::cZero;
  Any newRotation = Quat::cIdentity;
  Any newScale = Vec3(1, 1, 1);

  for(;!r.Empty();r.PopFront())
  {
    if(Transform* transform = r.Front()->has(Transform))
    {
      // Queue the translation change
      Property* translationProp = transformMeta->GetProperty("Translation");
      ChangeAndQueueProperty(opQueue, transform, translationProp, newTranslation);

      // Queue the rotation change
      Property* rotationProp = transformMeta->GetProperty("Rotation");
      ChangeAndQueueProperty(opQueue, transform, rotationProp, newRotation);

      // Queue the scale change
      Property* scaleProp = transformMeta->GetProperty("Scale");
      ChangeAndQueueProperty(opQueue, transform, scaleProp, newScale);
    }
  }
  opQueue->EndBatch();
}

void CameraFocusSpace(Space* space, EditFocusMode::Enum focusMode)
{
  if(space == nullptr)
    return;

  Cog* editorCamera = space->FindObjectByName(SpecialCogNames::EditorCamera);
  if(editorCamera)
    CameraFocusSpace(space, editorCamera, focusMode);
}

void CameraFocusSpace(Space* space)
{
   CameraFocusSpace(space, EditFocusMode::AutoTime);
}

void FocusOnSelectedObjects()
{
  CameraFocusSpace(Z::gEditor->GetEditSpace());
}

void CameraFocusSpace(Space* space, Cog* cameraObject, EditFocusMode::Enum focusMode)
{
  MetaSelection* activeSelection = Z::gEditor->GetSelection();
  Aabb aabb = GetAabb(activeSelection);
  CameraFocusSpace(space, cameraObject, aabb, focusMode);
}

void CameraFocusSpace(Space* space, Cog* cameraObject, const Aabb& focusAabb, EditFocusMode::Enum focusMode)
{
  if(space == nullptr)
    return;

  if(cameraObject == nullptr)
    return;

  // Get Components
  Camera* camera = cameraObject->has(Camera);
  EditorCameraController* controller = cameraObject->has(EditorCameraController);

  if(camera == nullptr || controller == nullptr)
    return;

  Aabb aabb = focusAabb;
  Vec3 lookCenter = aabb.GetCenter();
  Vec3 extents = aabb.GetExtents();
  float lookDistance = extents.Length()  * 1.5f;
  float viewSize = extents.Length() * 1.5f;

  //const bool previewBox = false;
  //if(previewBox)
  //  gDebugDraw->Add(Debug::Box(aabb).Duration(1.0f).Color(Color::Red));


  Actions* actions = controller->GetOwner()->GetActions();
  actions->SetRealTime(true);
  if( (focusMode == EditFocusMode::AutoTime && actions->IsEmpty()) || focusMode == EditFocusMode::Center)
  {
    Action* action = AnimatePropertyGetSet(EditorCameraController, LookTarget, Ease::Quad::Out, controller, 0.5f, lookCenter);
    actions->Add(action, ActionExecuteMode::FrameUpdate);
  }
  else
  {
    actions->Cancel();
    ActionGroup* group = new ActionGroup();

    if(camera->mPerspectiveMode == PerspectiveMode::Orthographic)
    {
      Action* viewAction = AnimatePropertyGetSet(Camera, Size,  Ease::Quad::Out, camera, 0.5f, viewSize);
      group->Add(viewAction);
    }
    else
    {
      Action* zoom = AnimatePropertyGetSet(EditorCameraController, LookDistance, Ease::Quad::Out, controller, 0.5f, lookDistance);
      group->Add(zoom);
    }

    Action* look = AnimatePropertyGetSet(EditorCameraController, LookTarget,  Ease::Quad::Out, controller, 0.5f, lookCenter);
    group->Add(look);

    actions->Add(group, ActionExecuteMode::FrameUpdate);
  }
}

void HideSelected(Editor* editor, Space* space)
{
  MetaSelection* active = editor->GetSelection();
  MetaSelection::rangeType<Cog> range = active->AllOfType<Cog>();
  for(; !range.Empty(); range.PopFront())
  {
    Cog* cog = range.Front()->GetId();
    cog->mFlags.SetFlag(CogFlags::EditorViewportHidden);
  }

  space->ChangedObjects();
}

void UnHideSelected(Editor* editor, Space* space)
{
  MetaSelection* active = editor->GetSelection();
  MetaSelection::rangeType<Cog> range = active->AllOfType<Cog>();
  for(; !range.Empty(); range.PopFront())
  {
    Cog* cog = range.Front()->GetId();
    cog->mFlags.ClearFlag(CogFlags::EditorViewportHidden);
  }

  space->ChangedObjects();
}

void UnhideAll(Editor* editor, Space* space)
{
  auto range = space->AllObjects();
  for(; !range.Empty(); range.PopFront())
    range.Front().mFlags.ClearFlag(CogFlags::EditorViewportHidden);

  space->ChangedObjects();
}

void ClearObjectStore()
{
  ObjectStore::GetInstance()->ClearStore();
}

void ExportHeightMapToObj(Editor* editor, Space* space)
{
  Cog* primary = editor->GetSelection()->GetPrimaryAs<Cog>();
  if(primary == nullptr)
    return;

  HeightMap* heightMap = primary->has(HeightMap);
  if(heightMap)
  {
    String directory = GetUserDocumentsDirectory();
    String filePath = FilePath::Combine(directory, "HeightMap.obj");
    HeightMap::SaveToObj(filePath, heightMap);
  }
}

void Mode2D(Editor* editor)
{
  editor->SetEditMode(EditorMode::Mode2D);
}

void Mode3D(Editor* editor)
{
  editor->SetEditMode(EditorMode::Mode3D);
}

bool ExecuteShortCuts(Space* space, Viewport* viewport, KeyboardEvent* event)
{
  if(event->Handled)
    return false;

  Editor* editor = Z::gEditor;

  // Delete selected objects
  if(event->Key == Keys::Delete)
  {
    DeleteSelectedObjects(Z::gEditor, space);
    event->Handled = true;
    return true;
  }

  // Focus using editor camera
  if(event->Key == Keys::F && !event->CtrlPressed)
  {
    event->Handled = true;
    
    // If the mouse is over the object view, focus on the objects row there
    ObjectView* objectView = Z::gEditor->mObjectView;
    if(objectView->IsMouseOver())
    {
      Cog* primary = Z::gEditor->GetSelection()->GetPrimaryAs<Cog>();
      if(primary)
        objectView->ShowObject(primary);
    }
    // Otherwise, focus on the object in the viewport
    else if(space != nullptr)
    {
      Cog* editorCamera = nullptr;
      if(viewport)
        editorCamera = viewport->GetCamera()->GetOwner();
      else
        editorCamera = space->FindObjectByName(SpecialCogNames::EditorCamera);

      if(editorCamera)
        CameraFocusSpace(space, editorCamera, EditFocusMode::AutoTime);
    }

    return true;
  }

  return false;

}

void CenterSelected(Space* space)
{
  CameraFocusSpace(space, EditFocusMode::Center);
}

void FrameSelected(Space* space)
{
  CameraFocusSpace(space, EditFocusMode::Frame);
}

// Resource Importing
void ImportSpriteSheet(Editor* editor)
{
  SpriteSheetImport(editor);
}

void ImportGroup(Editor* editor)
{
  GroupImport();
}

void SelectSpace(Editor* editor, Space* space)
{
  editor->ShowWindow("Properties");
  editor->mMainPropertyView->EditObject(space, true);
}

void SelectGame(Editor* editor, Space* space)
{
  editor->ShowWindow("Properties");
  editor->mMainPropertyView->EditObject(space->GetGameSession(), true);
}

void PreviousSelection(Editor* editor)
{
  editor->mMainPropertyView->GetHistory()->Previous();
}

void NextSelection(Editor* editor)
{
  editor->mMainPropertyView->GetHistory()->Next();
}

void EditorUndo(Editor* editor)
{
  editor->Undo();
}

void EditorRedo(Editor* editor)
{
  editor->Redo();
}

void ResetCamera(Editor* editor)
{
  if(EditorViewport* viewport = editor->mEditorViewport)
  {
    if(Cog* editorCamera = viewport->mEditorCamera)
    {
      EditorCameraController* cam = editorCamera->has(EditorCameraController);
      cam->Reset();
    }
  }
}

void AlignCameraToCamera(Editor* editor, Cog* fromCog, Cog* toCog)
{
  if (fromCog->has(Camera) && toCog->has(Camera))
  {
    Transform* fromCamT = fromCog->has(Transform);
    Transform* toCamT = toCog->has(Transform);

    OperationQueue* opQueue = editor->GetOperationQueue();
    opQueue->BeginBatch();

    String batchName = BuildString("Align '", CogDisplayName(fromCog),
      "' to '", CogDisplayName(toCog), "'");

    opQueue->SetActiveBatchName(batchName);

    BoundType* transformMeta = ZilchTypeId(Transform);

    // Queue the translation change
    Property* translationProp = transformMeta->GetProperty("Translation");
    Any newTranslation = toCamT->GetLocalTranslation();
    ChangeAndQueueProperty(opQueue, fromCamT, translationProp, newTranslation);

    // Queue the rotation change
    Property* rotationProp = transformMeta->GetProperty("Rotation");
    Any newRotation = toCamT->GetLocalRotation();
    ChangeAndQueueProperty(opQueue, fromCamT, rotationProp, newRotation);

    // Queue the scale change
    Property* scaleProp = transformMeta->GetProperty("Scale");
    Any newScale = toCamT->GetLocalScale();
    ChangeAndQueueProperty(opQueue, fromCamT, scaleProp, newScale);

    // End the batch
    opQueue->EndBatch();
  }
}

void AlignSelectedCameraToCamera(Editor* editor)
{
  EditorViewport* viewport = editor->mEditorViewport;
  if(viewport == nullptr)
    return;
  
  Cog* editorCamera = viewport->mEditorCamera;
  if(editorCamera == nullptr)
    return;

  if(Cog* selected = editor->mSelection->GetPrimaryAs<Cog>())
    AlignCameraToCamera(editor, selected, editorCamera);
}

void AlignCameraToSelectedCamera(Editor* editor)
{
  EditorViewport* viewport = editor->mEditorViewport;
  if (viewport == nullptr)
    return;

  Cog* editorCamera = viewport->mEditorCamera;
  if (editorCamera == nullptr)
    return;

  if (Cog* selected = editor->mSelection->GetPrimaryAs<Cog>())
    editorCamera->has(EditorCameraController)->AlignToCamera(selected);
}

void AddComponent(Editor* editor)
{
  editor->ShowWindow("Properties");

  // Open the add
  PropertyView* propView = editor->GetPropertyView();
  Event e;
  propView->DispatchBubble(Events::OpenAdd, &e);
}

void GoToDefinition(Editor* editor)
{
  DocumentManager* docManager = DocumentManager::GetInstance();
  DocumentEditor* currentDocument = docManager->CurrentEditor;
  
  if(currentDocument == nullptr)
    return;
  
  DocumentResource* resource = currentDocument->GetResource();
  
  if (resource == nullptr)
    return;
  
  ICodeInspector* codeInspector = resource->GetCodeInspector();
  
  if(codeInspector == nullptr)
    return;

  ICodeEditor* codeEditor = currentDocument->GetCodeEditor();

  if (codeEditor == nullptr)
    return;
  
  forRange(DocumentEditor* docEditor, docManager->Instances.All())
  {
    DocumentResource* docResource = docEditor->GetResource();
    if (docResource != nullptr && ZilchVirtualTypeId(docResource) == ZilchVirtualTypeId(resource))
    {
      // Saves the current text from the document editor to the Document (which may just set a resource or other thing underlying)
      // This should only ever upload the value in memory, but not actually access the disk (we could call docEditor->Save())
      docResource->ReloadData(docEditor->GetAllText());
    }
  }
  
  CodeDefinition definition;
  codeInspector->AttemptGetDefinition(codeEditor, codeEditor->GetCaretPosition(), definition);
  
  DisplayCodeDefinition(definition);
}

void EditCommands(Editor* editor)
{
  if(editor->ShowWindow("Commands"))
    return;
  
  ObjectView* objectView = new ObjectView(editor);
  objectView->SetName("Commands");
  objectView->SetHideOnClose(true);
  objectView->SetSize(Pixels(280, 380));
  objectView->SetSpace(editor->mCogCommands->GetSpace());
  editor->AddManagedWidget(objectView, DockArea::Floating, true);
}

void EnableAutoProjectScreenshot(Editor* editor)
{
  if(Cog* projectCog = editor->mProject)
  {
    if(ProjectSettings* project = projectCog->has(ProjectSettings))
      project->AutoTakeProjectScreenshot = true;
  }
}

void DisableAutoProjectScreenshot(Editor* editor)
{
  if(Cog* projectCog = editor->mProject)
  {
    if(ProjectSettings* project = projectCog->has(ProjectSettings))
      project->AutoTakeProjectScreenshot = false;
  }
}

void TakeProjectScreenshotDelayed(Editor* editor)
{
  bool success = editor->TakeProjectScreenshot();
  if(success)
  {
    DisableAutoProjectScreenshot(editor);
  }
  else
  {
    String message = "Neither an active game nor the editor viewport was visible.";
    DoNotifyWarning("Failed to take screenshot", message);
  }
}

void TakeProjectScreenshot(Editor* editor)
{
  // We have to delay because the screen shot functionality just copies the
  // back buffer, so we have to wait until the command dialog has been
  // destroyed before actually taking the screen shot.
  // There are two delays because sometimes the tooltip is still visible
  ActionSequence* seq = new ActionSequence(editor);
  seq->Add(new ActionDelayOnce());
  seq->Add(new ActionDelayOnce());
  seq->Add(new GlobalCallParamAction<Editor*, TakeProjectScreenshotDelayed>(editor));
}

void DumpMemoryDebuggerStats()
{
  Memory::DumpMemoryDebuggerStats("MyProject");
}

void BindEditorCommands(Cog* configCog, CommandManager* commands)
{
  commands->AddCommand("Undo", BindCommandFunction(EditorUndo));
  commands->AddCommand("Redo", BindCommandFunction(EditorRedo));

  commands->AddCommand("Cut", BindCommandFunction(SaveToClipBoardAndDelete));
  commands->AddCommand("Copy", BindCommandFunction(SaveSelectionToClipboard));
  commands->AddCommand("Paste", BindCommandFunction(LoadObjectFromClipboard));
  commands->AddCommand("Delete", BindCommandFunction(DeleteSelectedObjects));
  commands->AddCommand("Duplicate", BindCommandFunction(DuplicateSelection));

  commands->AddCommand("SelectNone", BindCommandFunction(Deselect));
  commands->AddCommand("SelectAllInTree", BindCommandFunction(SelectAllInTree));
  commands->AddCommand("SelectRoot", BindCommandFunction(SelectTopInTreeNoObj));
  commands->AddCommand("SelectParent", BindCommandFunction(SelectParent));
  commands->AddCommand("SelectChild", BindCommandFunction(SelectChild));
  commands->AddCommand("SelectSibling", BindCommandFunction(SelectSibling));
  commands->AddCommand("SelectAll", BindCommandFunction(SelectAll));

  commands->AddCommand("SelectSpace", BindCommandFunction(SelectSpace));
  commands->AddCommand("SelectGame", BindCommandFunction(SelectGame));

  commands->AddCommand("PreviousSelection", BindCommandFunction(PreviousSelection));
  commands->AddCommand("NextSelection", BindCommandFunction(NextSelection));

  commands->AddCommand("CenterSelected", BindCommandFunction(CenterSelected));
  commands->AddCommand("FrameSelected", BindCommandFunction(FrameSelected));

  commands->AddCommand("UnhideAll", BindCommandFunction(UnhideAll));
  commands->AddCommand("HideSelected", BindCommandFunction(HideSelected));
  commands->AddCommand("UnhideSelected", BindCommandFunction(UnHideSelected));

  commands->AddCommand("ResetTransform", BindCommandFunction(ResetTransform));
  commands->AddCommand("MoveToLookPoint", BindCommandFunction(MoveToLookPoint));

  commands->AddCommand("ClearObjectStore", BindCommandFunction(ClearObjectStore));

  commands->AddCommand("Mode2D", BindCommandFunction(Mode2D));
  commands->AddCommand("Mode3D", BindCommandFunction(Mode3D));

  commands->AddCommand("ParentToPrimary", BindCommandFunction(ParentToPrimary));
  commands->AddCommand("DetachSelected", BindCommandFunction(DetachSelected));
  commands->AddCommand("GroupSelected", BindCommandFunction(GroupSelected));
  commands->AddCommand("ExportHeightMapToObj", BindCommandFunction(ExportHeightMapToObj));

  commands->AddCommand("ImportSpriteSheet", BindCommandFunction(ImportSpriteSheet));
  commands->AddCommand("ImportGroup", BindCommandFunction(ImportGroup));
  commands->AddCommand("ImportHeightMap", BindCommandFunction(ImportHeightMap));


  commands->AddCommand("ResetCamera", BindCommandFunction(ResetCamera));
  commands->AddCommand("AlignSelectedCameraToCamera", BindCommandFunction(AlignSelectedCameraToCamera));
  commands->AddCommand("AlignCameraToSelectedCamera", BindCommandFunction(AlignCameraToSelectedCamera));

  commands->AddCommand("AddComponent", BindCommandFunction(AddComponent));


  commands->AddCommand("TakeProjectScreenshot", BindCommandFunction(TakeProjectScreenshot));
  commands->AddCommand("EnableAutoProjectScreenshot", BindCommandFunction(EnableAutoProjectScreenshot));
  commands->AddCommand("DisableAutoProjectScreenshot", BindCommandFunction(DisableAutoProjectScreenshot));

  commands->AddCommand("GoToDefinition", BindCommandFunction(GoToDefinition));

  commands->AddCommand("EditCommands", BindCommandFunction(EditCommands));

  if(DeveloperConfig* config = configCog->has(DeveloperConfig))
  {
    commands->AddCommand("DumpMemoryDebuggerStats", BindCommandFunction(DumpMemoryDebuggerStats));
  }
}

}//namespace Zero
