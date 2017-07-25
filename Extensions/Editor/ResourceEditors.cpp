///////////////////////////////////////////////////////////////////////////////
///
/// \file ResourceEditors.cpp
/// 
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const bool allowArchetypeEditing = true;
const bool allowMultiLevelEditing = false;

ZilchDefineType(ResourceEditors, builder, type)
{
}

void EditLevel(Editor* editor, Resource* resource)
{
  // Editing a level
  Level* level = Type::DynamicCast<Level*>(resource);

  editor->SaveAll(false);

  // Update the last edited level in the user configuration
  Cog* configCog = Z::gEngine->GetConfigCog();
  HasOrAdd<EditorConfig>(configCog)->EditingLevel = level->ResourceIdName;

  Space* space = editor->CreateNewSpace(CreationFlags::Editing);
  space->has(TimeSpace)->SetPaused(true);

  // EditorViewport must be hooked up to the space before loading the level
  editor->SetEditorViewportSpace(space);

  level->LoadSpace(space);

  // Clear the undo queue
  editor->mQueue->ClearAll();

  //Force tab area to update so name is changed
  editor->GetCenterWindow()->mTabArea->MarkAsNeedsUpdate();
}

void EditArchetype(Editor* editor, Resource* resource)
{
  Archetype* archetype = Type::DynamicCast<Archetype*>(resource);

  if(!allowArchetypeEditing)
  {
    // Just use property grid
    editor->mMainPropertyView->EditObject(resource, true);
    return;
  }

  // Can only edit cog archetypes
  if(archetype->mStoredType != ZilchTypeId(Cog))
    return;

  Space* space = editor->CreateNewSpace(CreationFlags::Editing);

  space->has(TimeSpace)->SetPaused(true);

  EditorViewport* editorViewport = new EditorViewport(editor, OwnerShip::Owner);
  editorViewport->SetTargetSpace(space);
  editorViewport->mEditArchetype = archetype;

  Level* level = LevelManager::FindOrNull("EmptyLevel");
  level->LoadSpace(space);

  Cog* objectToView = space->CreateAt(archetype->ResourceIdName, Vec3(0, 0, 0));
  editorViewport->mArchetypedObject = objectToView;

  // We want the modifications local to the Archetype to be on the object as we're editing it
  // in that Archetype's context
  archetype->GetLocalCachedModifications().ApplyModificationsToObject(objectToView);


  editorViewport->SetName(BuildString("Archetype: ", archetype->Name));

  // Set the archetype we opened as the current selection
  editor->GetSelection()->SelectOnly(objectToView);

  // Clear modified since we added objects
  space->MarkNotModified();

  editor->GetCenterWindow()->AttachAsTab(editorViewport);
}

void EditDocumentResource(Editor* editor, Resource* resource)
{
  editor->OpenDocumentResource((DocumentResource*)resource);
}

void EditSampleCurve(Editor* editor, Resource* resource)
{
  SampleCurve* curve = (SampleCurve*)resource;
  MultiSampleCurveEditor* curveEditor = new MultiSampleCurveEditor(editor);
  curveEditor->LoadCurve(curve);
  curveEditor->SetName(BuildString("Sample Curve: ", curve->Name));
  curveEditor->SetSize(Pixels(280, 400));
  editor->AddManagedWidget(curveEditor, DockArea::Floating, true);
}

void EditColorGradient(Editor* editor, Resource* resource)
{
  ColorGradient* gradient = (ColorGradient*)resource;
  ColorGradientEditor* gradientEditor = new ColorGradientEditor(editor, gradient);
  gradientEditor->SetName(gradient->Name);
  gradientEditor->SetSize(Pixels(375, 80));
  editor->AddManagedWidget(gradientEditor, DockArea::Floating, true);
}

void EditResourceTable(Editor* editor, Resource* resource)
{
  ResourceTable* table = (ResourceTable*)resource;
  ResourceTableEditor* tableEditor = new ResourceTableEditor(editor, table);
  tableEditor->SetSize(Pixels(600, 400));
  tableEditor->SetName(BuildString("ResourceTable: ", table->Name));
  editor->AddManagedWidget(tableEditor, DockArea::Floating, true);
}

void EditCollisionTable(Editor* editor, Resource* resource)
{
  CollisionTable* table = (CollisionTable*)resource;
  CollisionTableEditor* tableEditor = new CollisionTableEditor(editor, table);
  tableEditor->SetSize(Pixels(600, 400));
  tableEditor->SetName("CollisionTableEditor");
  editor->AddManagedWidget(tableEditor, DockArea::Floating, true);
}

void EditMultiConvexMesh(Editor* editor, Resource* resource)
{
  MultiConvexMesh* mesh = (MultiConvexMesh*)resource;
  //prevent the user from editing the default resource
  if(mesh == MultiConvexMeshManager::GetInstance()->GetDefaultResource())
  {
    DoNotifyWarning("Cannot edit default resource.", "Please add a new MultiConvexMesh resource to start editing");
    return;
  }

  MultiConvexMeshEditor* meshEditor = new MultiConvexMeshEditor(editor, mesh);

  //clamp the mesh editor's size to the main editor's size (so the window isn't off screen)
  Vec2 size = Pixels(1000, 800);
  size = Math::Min(editor->GetSize(), size);
  meshEditor->SetSize(size);
  meshEditor->SetName(BuildString("MultiConvexMesh: ", mesh->Name));

  //try to set the current sprite on the object as the preview sprite
  Cog* primarySelection = editor->GetSelection()->GetPrimaryAs<Cog>();
  if(primarySelection != nullptr)
  {
    Sprite* sprite = primarySelection->has(Sprite);
    if(sprite != nullptr)
    {
      meshEditor->UpdatePreview(sprite->mSpriteSource);
      meshEditor->SetGridSizeToPixels();
      if(mesh->mFlags.IsSet(MultiConvexMeshFlags::NewlyCreatedInEditor))
      {
        meshEditor->AutoCompute();
        mesh->mFlags.ClearFlag(MultiConvexMeshFlags::NewlyCreatedInEditor);
      }
    }
  }
  //zoom in to the appropriate level for the current mesh and sprite
  meshEditor->FocusOnPreviewCog();
  editor->AddManagedWidget(meshEditor, DockArea::Floating, true);
}

void EditAnimation(Editor* editor, Resource* resource)
{
  Animation* animation = (Animation*)resource;

  Archetype* previewArchetype = GetAnimationPreviewArchetype(animation);
  if(previewArchetype == nullptr)
  {
    DoNotifyWarning("Cannot Edit Resource", "No Preview Archetype is set on the Animation.");
    return;
  }

  // Create and pause the space
  Space* space = editor->CreateNewSpace(CreationFlags::Editing);
  space->has(TimeSpace)->SetPaused(true);

  // Create the editor viewport
  EditorViewport* editorViewport = new EditorViewport(editor, OwnerShip::Owner);
  editorViewport->SetTargetSpace(space);

  // Load the animation preview level
  Level* level = LevelManager::FindOrNull("EmptyLevel");
  level->LoadSpace(space);

  // Create the animation preview object and select it
  Cog* cog = CreateAnimationPreview(space, animation, previewArchetype);
  editor->GetSelection()->SelectOnly(cog);

  // Clear the undo queue
  editor->mQueue->ClearAll();

  editorViewport->SetName(BuildString("Animation: ", animation->Name));

  editor->GetCenterWindow()->AttachAsTab(editorViewport);

  editor->mManager->ShowWidget("Animator");
}

void EditSpriteSource(Editor* editor, Resource* resource)
{
  SpriteSource* spriteSource = (SpriteSource*)resource;
  EditSprite(spriteSource);
}

void EditHotKeys(Editor* editor, Resource* resource)
{
  HotKeyDataSet* hotKeys = (HotKeyDataSet*)resource;
  
  //HotKeyEditor* hotKeyEditor = new HotKeyEditor(editor);

  Widget* widget = editor->mManager->ShowWidget("CommandListViewer");
  //widget->SetSize(Pixels(850, 500));
  //widget->TakeFocus();

  ((HotKeyEditor*)widget)->EditResource(hotKeys, (HotKeyManager*)Z::gResources->Managers["HotKeyDataSet"]);
  //hotKeyEditor->SetName("HotKey Editor");
  //hotKeyEditor->SetSize(Pixels(850, 500));

  //editor->AddManagedWidget(hotKeyEditor, DockArea::Floating, true);
}

ResourceEditors::ResourceEditors()
{
  //Editors[ZilchTypeId(Archetype)] = EditArchetype;
  Editors[ZilchTypeId(Level)] = EditLevel;
  Editors[ZilchTypeId(DocumentResource)] = EditDocumentResource;
  Editors[ZilchTypeId(SampleCurve)] = EditSampleCurve;
  Editors[ZilchTypeId(ColorGradient)] = EditColorGradient;
  Editors[ZilchTypeId(CollisionTable)] = EditCollisionTable;
  Editors[ZilchTypeId(ResourceTable)] = EditResourceTable;
  Editors[ZilchTypeId(MultiConvexMesh)] = EditMultiConvexMesh;
  Editors[ZilchTypeId(Animation)] = EditAnimation;
  Editors[ZilchTypeId(SpriteSource)] = EditSpriteSource;
  Editors[ZilchTypeId(HotKeyDataSet)] = EditHotKeys;
}

void ResourceEditors::FindResourceEditor(Resource* resource)
{
  // Get the type of the resource
  BoundType* resourceType = ZilchVirtualTypeId(resource);
  Editor* editor = Z::gEditor;

  // Try to find a resource editor for this resource type
  // Also check base types of the resource for resources
  // that share editors like document resources
  BoundType* currentType =  resourceType;
  while(currentType)
  {
    // Get resource editor
    EditResourceFunction editResource = Editors.FindValue(currentType, nullptr);
    if(editResource)
    {
      editResource(editor, resource);
      return;
    }

    // Move to base type
    currentType = currentType->BaseType;
  }

  // Just use property grid
  editor->mMainPropertyView->EditObject(resource, true);
}

}
