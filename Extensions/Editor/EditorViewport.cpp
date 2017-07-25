///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorViewport.cpp
/// Implementation of the EditorViewport class.
///
/// Authors: Chris Peters, Joshua Claeys, Nathan Carlson
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace ViewportMenuUi
{
  const cstr cLocation = "EditorUi/Viewport/ViewportMenu";
  Tweakable(float, Height, Pixels(30), cLocation);
}

namespace EditorUiSettings
{
  const cstr cLocation = "EditorUi/Viewport";
  Tweakable(bool, EditorViewportAutoTakeFocus, true, cLocation);
  Tweakable(float, DragSelectDistance, 5, cLocation);
}

//-------------------------------------------------------------- Editor Viewport
ZilchDefineType(EditorViewport, builder, type)
{
  
}

EditorViewport::EditorViewport(Composite* parent, OwnerShip::Enum ownership)
  : Composite(parent)
{
  mTools = Z::gEditor->Tools;
  mOwnerShip = ownership;

  ConnectThisTo(this, Events::ObjectPoll, OnObjectPoll);

  ConnectThisTo(Z::gResources, Events::ResourceRemoved, OnResourcesRemoved);
  ConnectThisTo(this, Events::CommandCaptureContext, OnCaptureContext);
  ConnectThisTo(this, Events::TabFind, OnTabFind);
  ConnectThisTo(this, Events::FocusGainedHierarchy, OnFocusGained);
  ConnectThisTo(this, Events::FocusLostHierarchy, OnFocusLost);

  ConnectThisTo(Z::gEditor, Events::Save, OnSave);
  ConnectThisTo(Z::gEditor, Events::SaveCheck, OnSaveCheck);

  ConnectThisTo(this, Events::LeftMouseDrag, OnLeftMouseDrag);
  ConnectThisTo(this, Events::MouseDown, OnMouseDown);
  ConnectThisTo(this, Events::MouseMove, OnMouseMove);
  ConnectThisTo(this, Events::MouseUpdate, OnMouseUpdate);
  ConnectThisTo(this, Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(this, Events::DoubleClick, OnMouseDoubleClick);
  ConnectThisTo(this, Events::LeftMouseUp, OnMouseUp);
  ConnectThisTo(this, Events::RightMouseDown, OnRightMouseDown);
  ConnectThisTo(this, Events::RightMouseUp, OnRightMouseUp);
  ConnectThisTo(this, Events::MiddleMouseDown, OnMiddleMouseDown);
  ConnectThisTo(this, Events::MouseScroll, OnMouseScroll);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(this, Events::KeyUp, OnKeyUp);

  ConnectThisTo(this, Events::MouseDrop, OnMouseDrop);
  ConnectThisTo(this, Events::MetaDrop, OnMetaDrop);
  ConnectThisTo(this, Events::MetaDropTest, OnMetaDrop);
  ConnectThisTo(this, Events::MetaDropUpdate, OnMetaDrop);

  mMouseOperation = MouseMode::Default;
  mOrientationGizmo = nullptr;

  mGameWidget = new GameWidget(this);

  // Create the menu and hide button
  mMenu = new EditorViewportMenu(this);

  mMenuToggleButton = CreateAttached<Element>("ViewportMenuRetract");
  mMenuToggleButton->SetColor(Vec4(0.8f, 0.8f, 0.8f, 1));
  ConnectThisTo(mMenuToggleButton, Events::MouseEnter, OnMouseEnterMenuToggle);
  ConnectThisTo(mMenuToggleButton, Events::MouseExit, OnMouseExitMenuToggle);
  ConnectThisTo(mMenuToggleButton, Events::LeftClick, OnMenuToggleClicked);

  mMenuHidden = false;

  EditorSettings* settings = Z::gEngine->GetConfigCog()->has(EditorSettings);
  ConnectThisTo(settings, Events::PropertyModified, OnSettingsChanged);
}

void EditorViewport::OnDestroy()
{
  uint index = Z::gEditor->mEditInGameViewports.FindIndex(this);
  if (index != Z::gEditor->mEditInGameViewports.InvalidIndex)
    Z::gEditor->mEditInGameViewports.EraseAt(index);
  CleanUp();
  Composite::OnDestroy();
}

void EditorViewport::CleanUp()
{
  Space* editSpace = mEditSpace;
  if (editSpace == nullptr)
    return;

  // No more events
  editSpace->GetDispatcher()->Disconnect(this);

  EditorCameraController* cameraController = mEditorCamera.has(EditorCameraController);
  if(cameraController)
  {
    cameraController->Deactivate();
    CameraViewport* cameraViewport = mEditorCamera.has(CameraViewport);
    cameraViewport->SetRenderInGame(false);
  }

  if (mOwnerShip == OwnerShip::Sharing)
    return;

  editSpace->Destroy();
}

void EditorViewport::SetAsActive()
{
  /// When this editor view port is focus or activated
  /// switch the object view to the space edited by this viewport
  if (Space* space = mEditSpace)
  {
    // If the space being viewed is editing an archetype do not set it as our edit level
    // but we still need to set the space or we will not properly display the object view
    Level* editLevel = space->mLevelLoaded;
    if (editLevel && !mEditArchetype)
      Z::gEditor->mEditLevel = editLevel;

    Z::gEditor->SetEditSpace(space);
  }
}

void EditorViewport::SetActiveSelection()
{
  MetaSelection* selection = Z::gEditor->GetSelection();
  // If we are editing an archetype set it as our selection
  if(mArchetypedObject.IsNotNull())
    selection->SelectOnly(mArchetypedObject);
}

void EditorViewport::SetUpEditorCamera()
{
  Space* space = mEditSpace;

  Cog* editorCamera = space->FindObjectByName(SpecialCogNames::EditorCamera);
  if (editorCamera == NULL)
  {
    // No editor camera create one
    editorCamera = Z::gFactory->Create(space, CoreArchetypes::EditorCamera, 0, nullptr);
    // Clear the archetype so the full object is always saved
    editorCamera->ClearArchetype();
    // Name it so we can find again
    editorCamera->SetName(SpecialCogNames::EditorCamera);
  }

  //Protect the camera
  editorCamera->mFlags.SetFlag(CogFlags::Protected);

  // Enabled the camera controller
  EditorCameraController* controller = HasOrAdd<EditorCameraController>(editorCamera);
  controller->SetEnabled(true);

  // temporary until archetypes are updated
  CameraViewport* cameraViewport = HasOrAdd<CameraViewport>(editorCamera);
  cameraViewport->mCameraPath.SetCog(editorCamera);
  cameraViewport->SetRenderInGame(false);
  cameraViewport->SetRenderInEditor(true);

  ConnectThisTo(editorCamera, Events::CameraUpdate, OnCameraUpdate);

  mEditorCamera = editorCamera;
}

ReactiveViewport* EditorViewport::GetReactiveViewport()
{
  CameraViewport* cameraViewport = mEditorCamera.has(CameraViewport);
  return Type::DynamicCast<ReactiveViewport*>((Viewport*)cameraViewport->mViewport);
}

void EditorViewport::SetTargetSpace(Space* space)
{
  CleanUp();

  mEditSpace = space;
  space->mGameWidgetOverride = mGameWidget;

  //When a level is loaded check for editor objects
  ConnectThisTo(space, Events::SpaceLevelLoaded, OnSpaceLevelLoaded);
  ConnectThisTo(space, Events::SpaceModified, OnSpaceModified);
  ConnectThisTo(space, Events::SpaceDestroyed, OnSpaceDestroyed);
}

Space* EditorViewport::GetTargetSpace()
{
  return mEditSpace;
}

bool EditorViewport::TakeFocusOverride()
{
  // Set this as the active viewport
  SetAsActive();
  // Take soft focus so hard focus items can block it
  SoftTakeFocus();
  // Change the object focus based on the scene we are entering
  SetActiveSelection();

  return true;
}

void EditorViewport::UpdateTransform()
{
  mGameWidget->SetSize(mSize);

  // Update the menu transform
  mMenu->SetSize(Vec2(mSize.x, ViewportMenuUi::Height));

  // Update the hide/show button for the menu
  Vec3 pos(mSize.x - Pixels(6) - mMenuToggleButton->mSize.x, Pixels(8), 0);
  mMenuToggleButton->SetTranslation(pos);

  Composite::UpdateTransform();
}

Vec2 EditorViewport::GetMinSize()
{
  return Pixels(200, 200);
}

void EditorViewport::ConfigureViewCube(bool active, real viewportSize)
{
  EditorMode::Enum mode = Z::gEditor->GetEditMode();
  active = active && (mode != EditorMode::Mode2D);

  if (active && !mOrientationGizmo)
  {
    mOrientationGizmo = new OrientationGizmoViewport(this);
  }
  else if (!active && mOrientationGizmo)
  {
    mOrientationGizmo->Destroy();
    mOrientationGizmo = nullptr;
  }

  if (mOrientationGizmo)
  {
    float height = ViewportMenuUi::Height.mValue;

    Vec2 parentSize = GetSize();
    parentSize.y -= height;
    Vec2 size = parentSize * viewportSize;
    // Want the max dimension if it still fits in the parent size
    float dim = Math::Max(size.x, size.y);
    dim = Math::Min(dim, Math::Min(parentSize.x, parentSize.y));
    size = Vec2(dim);
    Vec3 pos = Vec3(mSize.x - size.x, height, 0);
    if (pos != mOrientationGizmo->GetTranslation())
      mOrientationGizmo->SetTranslation(pos);
    if (size != mOrientationGizmo->GetSize())
      mOrientationGizmo->SetSize(size);
    mOrientationGizmo->UpdateTransformExternal();
  }
}

void EditorViewport::OnMouseEnterMenuToggle(Event* event)
{
  mMenuToggleButton->SetColor(Vec4(1,1,1,1));
}

void EditorViewport::OnMouseExitMenuToggle(Event* event)
{
  mMenuToggleButton->SetColor(Vec4(0.8f, 0.8f, 0.8f, 1));
}

void EditorViewport::OnMenuToggleClicked(Event* event)
{
  const Vec3 cOnScreenPos = Vec3::cZero;
  const Vec3 cOffScreenPos = Vec3(0, -mMenu->mSize.y, 0);
  const float cAnimTime = 0.2f;
  
  if(mMenuHidden)
  {
    ActionSequence* seq = new ActionSequence(this, ActionExecuteMode::FrameUpdate);
    seq->Add(MoveWidgetAction(mMenu, cOnScreenPos, cAnimTime));
    mMenuToggleButton->ChangeDefinition( mDefSet->GetDefinition("ViewportMenuRetract") );
  }
  else
  {
    ActionSequence* seq = new ActionSequence(this, ActionExecuteMode::FrameUpdate);
    seq->Add(MoveWidgetAction(mMenu, cOffScreenPos, cAnimTime));
    mMenuToggleButton->ChangeDefinition( mDefSet->GetDefinition("ViewportMenuExpand") );
  }

  mMenuHidden = !mMenuHidden;
}

void EditorViewport::OnSpaceDestroyed(Event* event)
{
  // Null out space or clean up will try to destroy it again
  mEditSpace = nullptr;

  // Auto close tab when space destroyed
  if(!this->mDestroyed)
    CloseTabContaining(this);
}

void EditorViewport::OnSpaceModified(Event* event)
{
  // Set the modified icon
  if(Space* editSpace = mEditSpace)
  {
    TabModifiedEvent toSend(editSpace->GetModified());
    GetDispatcher()->Dispatch(Events::TabModified, &toSend);
  }
}

void EditorViewport::OnSpaceLevelLoaded(Event* event)
{
  Space* editSpace = mEditSpace;

  if (Level* level = editSpace->mLevelLoaded)
    SetName(BuildString("Level: ", level->Name));

  // A level has been loaded need to find editor camera
  SetUpEditorCamera();

  Cog* levelSettings = editSpace->FindObjectByName(SpecialCogNames::LevelSettings);

  if (levelSettings == NULL)
  {
    levelSettings = Z::gFactory->CreateCheckedType(ZilchTypeId(Cog), editSpace, CoreArchetypes::LevelSettings, 0, editSpace->GetGameSession());
    levelSettings->ClearArchetype();
  }

  // Move to front of space root objects, WorldAnchor is at index 0
  if (levelSettings->GetHierarchyIndex() != 1)
    levelSettings->PlaceInHierarchy(1);
  levelSettings->mFlags.SetFlag(CogFlags::Protected);

  // Now that there is a new level set it as active
  SetAsActive();
  mMenu->InitializeFromSpace(mEditSpace);
  
  EditorSettings* editorSettings = Z::gEngine->GetConfigCog()->has(EditorSettings);
  ConfigureViewCube(editorSettings->mViewCube, editorSettings->mViewCubeSize);
}

void EditorViewport::OnTabFind(WindowTabEvent* event)
{
  Space* space = mEditSpace;
  if(space == NULL)
    return;

  // Is this the viewport that Contains the object?
  if(event->SearchObject.IsNotNull())
  {
    if(event->SearchObject == space)
      event->TabWidgetFound = this;
    if(event->SearchObject == (Level*)space->mLevelLoaded)
      event->TabWidgetFound = this;
    if(event->SearchObject == (Archetype*)mEditArchetype)
      event->TabWidgetFound = this;
    if(Cog* cog = event->SearchObject.Get<Cog*>())
    {
      if(cog->GetSpace() == space)
        event->TabWidgetFound = this;
    }
  }
}

void EditorViewport::OnSave(SavingEvent* event)
{
  // If this is shared space let the owner save it
  if(mOwnerShip == OwnerShip::Sharing)
    return;

  // Do we have a space?
  Space* editSpace = mEditSpace;
  if(editSpace == NULL)
    return;

  // Auto upload the Space Archetype
  if(editSpace->IsModifiedFromArchetype())
    editSpace->UploadToArchetype();

  // Has anything in the level changed?
  if(!editSpace->GetModified())
    return;

  // If editing an archetype upload it
  if(mEditArchetype && mArchetypedObject)
  {
    Cog* object = mArchetypedObject;

    // Make sure it is still the same archetype
    // and upload if it has changed
    if(object->GetArchetype() == mEditArchetype && object->IsModifiedFromArchetype())
    {
      object->UploadToArchetype();
      Z::gEngine->RebuildArchetypes(mEditArchetype);
    }
    return;
  }

  // Editing a level
  Level* editLevel = editSpace->mLevelLoaded;

  // Is there a  level loaded?
  if(editLevel == NULL)
    return;

  // Can this level be saved?
  if(editLevel->IsWritable())
    editLevel->SaveSpace(editSpace);

}

void EditorViewport::OnSaveCheck(SavingEvent* event)
{
  if(mOwnerShip == OwnerShip::Sharing)
    return;

  if(Space* space = mEditSpace)
  {
    if(space->GetModified())
      event->NeedSaving = true;
  }
}

void EditorViewport::OnResourcesRemoved(ResourceEvent* event)
{
  // Handle when a resource is removed or unloaded
  Space* editSpace = mEditSpace;
  if(editSpace == NULL)
    return;

  Level* editLevel = editSpace->mLevelLoaded;
  bool resourceBeingEdited = false;
  // Is the level this viewport is editing been removed?
  if (editLevel == (Level*)event->EventResource)
    resourceBeingEdited = true;
  // Is the archetype this viewport is editing been removed?
  if ((Archetype*)mEditArchetype == (Archetype*)event->EventResource)
    resourceBeingEdited = true;

  if (editLevel && resourceBeingEdited)
    CloseTabContaining(this);
}

void EditorViewport::OnCaptureContext(CommandCaptureContextEvent* event)
{
  event->ActiveSet->SetContext(this);
  event->ActiveSet->SetContext((Space*)mEditSpace);
}

void EditorViewport::OnObjectPoll(ObjectPollEvent* event)
{
  ReactiveViewport* viewport = GetReactiveViewport();
  if (viewport == nullptr)
    return;

  SelectionResult result = EditorRayCast(viewport, event->Position);

  // Return the found object
  event->FoundObject = result.Object;
}

void EditorViewport::OnMouseEnter(MouseEvent* event)
{
  if (EditorUiSettings::EditorViewportAutoTakeFocus)
    SoftTakeFocus();
}

void EditorViewport::OnMiddleMouseDown(MouseEvent* e)
{
  if(e->Handled)
    return;

  // Allow active tool to have the first chance response.
  if(ForwardMouseEventToTool(e))
    return;

  EditorCameraController* controller = mEditorCamera.has(EditorCameraController);
  if(controller)
  {
    controller->ProcessMiddleMouseDown( );
    new EditorCameraMouseDrag(e->GetMouse( ), this, controller);
    return;
  }

  if(ForwardEventToGizmos(e))
    return;
}

void EditorViewport::OnRightMouseDown(MouseEvent* e)
{
  if(e->Handled)
    return;

  // Allow active tool to first chance response.
  if(ForwardMouseEventToTool(e))
    return;

  EditorCameraController* controller = mEditorCamera.has(EditorCameraController);
  if(controller)
  {
    if(controller->mControlMode == ControlMode::ZPlane)
      controller->MouseDrag(CameraDragMode::Pan);
    else
      controller->MouseDrag(CameraDragMode::Rotation);

    new EditorCameraMouseDrag(e->GetMouse( ), this, controller);
    return;
  }

  if(ForwardEventToGizmos(e))
    return;
}

void EditorViewport::OnLeftMouseDown(MouseEvent* e)
{
  if(e->Handled)
    return;

  mMouseDownScreenPosition = e->Position;

  // Unlike the other mouse events left mouse specifically has an override to
  // allow the camera a first-chance response before the active tool has a chance.
  EditorCameraController* controller = mEditorCamera.has(EditorCameraController);
  if(controller)
  {
    // Camera control button (ctrl) is down
    if(controller->IsActive( ))
    {
      new EditorCameraMouseDrag(e->GetMouse( ), this, controller);
      return;
    }

  }

  // Allow active tool to respond before gizmos.
  if(ForwardMouseEventToTool(e))
    return;

  if(ForwardEventToGizmos(e))
    return;

  // If no one handled the event, we can assume they're trying to
  // select (or de-select)
  mMouseOperation = MouseMode::Select;
}

void EditorViewport::OnMouseDoubleClick(MouseEvent* e)
{
  if(e->Handled)
    return;

  mMouseDownScreenPosition = e->Position;

  // Allow active tool to first chance response.
  if(ForwardMouseEventToTool(e))
  {
    mMouseOperation = MouseMode::Default;
    return;
  }

  if(ForwardEventToGizmos(e))
  {
    mMouseOperation = MouseMode::Default;
    return;
  } 

}

void EditorViewport::OnLeftMouseDrag(MouseEvent* e)
{
  mMouseOperation = MouseMode::Default;

  // Unlike the other mouse events left mouse specifically has an override to
  // allow the camera a first-chance response before the active tool has a chance.
  EditorCameraController* controller = mEditorCamera.has(EditorCameraController);
  if(controller)
  {
    // Early out if the camera is performing a left-click enabled operation
    //  - [ie, do not use any tool]
    if(controller->IsActive( ))
      return;
  }

  // Allow active tool to respond before gizmos.
  if(ForwardMouseEventToTool(e))
    return;

  if(ForwardEventToGizmos(e))
    return;

  BeginSelectDrag(this, e, mTools->mSelectTool);
}

void EditorViewport::OnMouseUpdate(MouseEvent* e)
{
  if(e->Handled)
    return;

  // Allow active tool to have the first chance response.
  if(ForwardMouseEventToTool(e))
    return;

  if(ForwardEventToGizmos(e))
    return;

  // If the tool didn't handle the event, forward it so that reactive objects get updated
  if(ReactiveViewport* viewport = GetReactiveViewport())
    viewport->OnMouseUpdate(e);
}

void EditorViewport::OnMouseMove(MouseEvent* e)
{
  if(e->Handled)
    return;

  if(mMouseOperation != MouseMode::Select)
  {
    e->GetMouse()->SetCursor(Cursor::Arrow);

    // Allow active tool to have the first chance response.
    if(ForwardMouseEventToTool(e))
      return;

    if(ForwardEventToGizmos(e))
      return;
  }
}

void EditorViewport::OnMouseUp(MouseEvent* e)
{
  if(e->Handled)
    return;

  // Allow active tool to have the first chance response.
  if(ForwardMouseEventToTool(e))
    return;

  if(ForwardEventToGizmos(e))
    return;

  // Other tools fall through to select tool

  // Only forward to select tool if the mouse was pressed down in the viewport
  if(!mTools->IsSelectToolActive( ) && mMouseOperation == MouseMode::Select)
    ForwardMouseEventToTool(e, mTools->mSelectTool->GetOwner( ));

  mMouseOperation = MouseMode::Default;
}

void EditorViewport::OnMouseDown(MouseEvent* event)
{
  SetAsActive();
}

void EditorViewport::OnRightMouseUp(MouseEvent* e)
{
  if(e->Handled)
    return;

  // Allow active tool to have the first chance response.
  if(ForwardMouseEventToTool(e))
    return;

  if(ForwardEventToGizmos(e))
    return;  

  // Other tools fall through to select tool
  if(!mTools->IsSelectToolActive() && mMouseOperation == MouseMode::Select)
    ForwardMouseEventToTool(e, mTools->mSelectTool->GetOwner());

  mMouseOperation = MouseMode::Default;
}

void EditorViewport::OnMouseScroll(MouseEvent* e)
{
  if(e->Handled)
    return;

  // Allow active tool to have the first chance response.
  if(ForwardMouseEventToTool(e))
    return;

  if(ForwardEventToGizmos(e))
    return;

  if(EditorCameraController* controller = mEditorCamera.has(EditorCameraController))
    controller->MouseScroll(e->Scroll);
}

void EditorViewport::OnMouseDrop(MouseEvent* event)
{
  ReactiveViewport* viewport = GetReactiveViewport();
  if (viewport == nullptr)
    return;

  if (Space* space = mEditSpace)
  {
    if (event->OsEvent)
    {
      Vec2 mousePos = ToLocal(event->Position);
      SelectionResult result = EditorRayCast(viewport, mousePos);

      OsMouseDropEvent* mouseDrop = (OsMouseDropEvent*)event->OsEvent;
      LoadFilesDroppedOnViewport(event, viewport, space, result.Object, mouseDrop->Files);
    }
  }
}

void EditorViewport::OnMetaDrop(MetaDropEvent* e)
{
  ReactiveViewport* viewport = GetReactiveViewport();
  if (viewport == nullptr)
    return;

  if(Space* space = mEditSpace)
  {
    // Se the draw space for any debug draw done in the meta drop event
    Debug::ActiveDrawSpace drawSpace(space->GetId().Id);

    Vec2 mousePos = e->Position;
    SelectionResult result = EditorRayCast(viewport, mousePos);
    e->MouseOverObject = result.Object;

    // Create the viewport mouse event
    ViewportMouseEvent viewportMouseEvent(e->mMouseEvent);
    viewport->InitViewportEvent(viewportMouseEvent);
    e->mViewportMouseEvent = &viewportMouseEvent;

    // Dispatch to the space to allow the user to do something with it first
    space->DispatchEvent(e->EventId, e);

    // The ViewportMouseEvent will be destructed once we leave this scope.
    // This is temporary until we combine MouseEvent and ViewportMouseEvent.
    e->mViewportMouseEvent = nullptr;

    if(e->Handled)
      return;

    EditorDrop(e, viewport, space, result.Object);
  }
}

void EditorViewport::OnKeyDown(KeyboardEvent* e)
{
  if(e->Handled)
    return;

  ReactiveViewport* viewport = GetReactiveViewport();
  if (viewport == nullptr)
    return;

  Space* targetSpace = mEditSpace;
  if(targetSpace == NULL)
    return;

  // Allow active tool to have the first chance response.
  //   - Note: Tools can block commands
  if(ForwardEventToTool(e))
    return;

  if(ForwardEventToGizmos(e))
    return;

  //Core short cuts
  if(ExecuteShortCuts(mEditSpace, viewport, e))
    return;

  CommandManager* commands = CommandManager::GetInstance();
  if(commands->TestCommandKeyboardShortcuts(e))
    return;

  //Ctrl for standard commands
  if(e->CtrlPressed)
  {
    //if(event->Key == Keys::L)
    //{
    //  ToggleLighting();
    //}
  }
  else
  {
    //if(e->Key == Keys::F12)
    //{
    //  String fileName = BuildString("ZeroScreenShot-", GetTimeAndDateStamp(), ".png");
    //  String filePath = FilePath::Combine(GetUserDocumentsDirectory(), fileName);
    //  this->ScreenCapture(filePath);

    //  //Print to console only
    //  ZPrint("Screen Shot Taken %s\n", filePath.c_str());
    //}

    if(e->Key == Keys::Tab)
    {
      if(Space* space = mEditSpace)
      {
        // Only unpause / pause the game if it is
        // not the space that is being edited
        // used for edit in game
        if(!space->IsEditorMode())
        {
          if(TimeSpace* ts = mEditSpace.has(TimeSpace))
            ts->TogglePause();
        }
      }
    }

    if(e->Key == Keys::Space)
    {
      Z::gEditor->OpenSearchWindow(this);
    }
  }

  if(EditorCameraController* controller = mEditorCamera.has(EditorCameraController))
  {
    controller->ProcessKeyboardEvent(e);
  }
}

void EditorViewport::OnKeyUp(KeyboardEvent* e)
{
  if(e->Handled)
    return;

  // Allow active tool to have the first chance response.
  if(ForwardEventToTool(e))

    return;
  if(ForwardEventToGizmos(e))
    return;

  // If shift pressed disable camera
  if(e->Key == Keys::Shift)
  {
    if(EditorCameraController* controller = mEditorCamera.has(EditorCameraController))
    {
      if(controller)
        controller->ProcessKeyboardEvent(e);
    }
  }
}

void EditorViewport::OnFocusLost(FocusEvent* focusEvent)
{
  if(EditorCameraController* controller = mEditorCamera.has(EditorCameraController))
    controller->Deactivate();

  if(Cog* activeTool = mTools->GetActiveCog())
    activeTool->DispatchEvent(focusEvent->EventId, focusEvent);
}

void EditorViewport::OnFocusGained(FocusEvent* focusEvent)
{
  SetAsActive();
}

bool EditorViewport::ForwardMouseEventToTool(MouseEvent* e, Cog* tool)
{
  ReactiveViewport* viewport = GetReactiveViewport();
  if (viewport == nullptr)
    return false;

  // Create the viewport mouse event
  ViewportMouseEvent eventToSend(e);
  viewport->InitViewportEvent(eventToSend);

  return ForwardEventToTool(&eventToSend, tool);
}

bool EditorViewport::ForwardEventToTool(MouseEvent* e, Cog* tool)
{
  if(tool == nullptr)
    tool = mTools->GetActiveCog();

  if(tool)
  {
    tool->DispatchEvent(e->EventId, e);

    return (e->Handled || e->HandledEventScript);
  }

  return false;
}

bool EditorViewport::ForwardEventToTool(KeyboardEvent* e, Cog* tool)
{
  if(tool == nullptr)
    tool = mTools->GetActiveCog();

  if(tool)
  {
    tool->DispatchEvent(e->EventId, e);

    return (e->Handled || e->HandledEventScript);
  }

  return false;
}

bool EditorViewport::ForwardEventToGizmos(MouseEvent* e)
{
  ReactiveViewport* viewport = GetReactiveViewport();
  if (viewport == nullptr)
    return false;

  // Create the viewport mouse event
  ViewportMouseEvent eventToSend(e);
  viewport->InitViewportEvent(eventToSend);

  GizmoSpace* gizmoSpace = HasOrAdd<GizmoSpace>(mEditSpace);

  if(e->EventId == Events::MouseUpdate)
    gizmoSpace->OnMouseUpdate(&eventToSend);

  gizmoSpace->ForwardEvent(&eventToSend);
  
  e->Handled = eventToSend.Handled;
  e->HandledEventScript = eventToSend.HandledEventScript;

  return (eventToSend.Handled || eventToSend.HandledEventScript);
}

bool EditorViewport::ForwardEventToGizmos(KeyboardEvent* e)
{
  GizmoSpace* gizmoSpace = HasOrAdd<GizmoSpace>(mEditSpace);

  gizmoSpace->ForwardEvent(e);

  return (e->Handled || e->HandledEventScript);
}

void EditorViewport::OnCameraUpdate(ObjectEvent* event)
{
  EditorSettings* settings = Z::gEngine->GetConfigCog()->has(EditorSettings);
  ConfigureViewCube(settings->mViewCube, settings->mViewCubeSize);

  if (mOrientationGizmo && mEditorCamera)
    mOrientationGizmo->SetGizmoRotation(mEditorCamera.has(Transform)->GetRotation().Inverted());
}

void EditorViewport::OnSettingsChanged(Event* event)
{
  EditorSettings* settings = Z::gEngine->GetConfigCog()->has(EditorSettings);
  ConfigureViewCube(settings->mViewCube, settings->mViewCubeSize);
}


} // namespace Zero
