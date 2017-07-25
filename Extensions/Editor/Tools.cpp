///////////////////////////////////////////////////////////////////////////////
///
/// \file Tools.cpp
/// Implementation of the Tools classes.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------------------------- Events
namespace Events
{
DefineEvent(SelectToolPreSelect);
DefineEvent(SelectToolFrustumCast);
DefineEvent(SelectToolPreDraw);
}

/******************************************************************************/
ZilchDefineType(SelectToolFrustumEvent, builder, type)
{
  ZilchBindFieldProperty(Handled);
  ZilchBindFieldProperty(HandledEventScript);

  ZilchBindGetterProperty(Space);
  ZilchBindGetterProperty(Frustum);
}

/******************************************************************************/
Space* SelectToolFrustumEvent::GetSpace( ) { return mSpace; }
Frustum SelectToolFrustumEvent::GetFrustum( ) { return mFrustum; }

//------------------------------------------------------------ Group Select Drag
class GroupSelectDrag : public MouseManipulation
{
public:
  HandleOf<EditorViewport> mViewport;
  HandleOf<Element> mDragElement;
  SelectTool* mTool;
  MetaSelection mCurrentSelection;

  //****************************************************************************
  GroupSelectDrag(Composite* owner, Mouse* mouse, EditorViewport* editorViewport, 
                  SelectTool* tool)
    : MouseManipulation(mouse, owner)
  {
    mViewport = editorViewport;
    mTool = tool;

    if(Viewport* viewport = editorViewport->GetReactiveViewport())
    {
      Element* dragElement = viewport->CreateAttached<Element>("DragBox");
      dragElement->SetVisible(false);
      mDragElement = dragElement;
    }

    // copy the current selection
    forRange(Cog* cog, Z::gEditor->GetSelection()->AllOfType<Cog>())
      mCurrentSelection.Add(cog, SendsEvents::False);
  }

  //****************************************************************************
  ~GroupSelectDrag()
  {
    mDragElement.SafeDestroy();

    // If the user was dragging while the engine closed then the
    // viewport might already be dead and therefore we can't take focus
    EditorViewport* viewport = mViewport;
    if(viewport != NULL)
      viewport->TakeFocus();
  }

  //****************************************************************************
  void OnKeyDown(KeyboardEvent* event) override
  {
    EditorViewport* viewport = mViewport;
    viewport->DispatchEvent(Events::KeyDown, event);
  }

  //****************************************************************************
  void OnKeyUp(KeyboardEvent* event) override
  {
    EditorViewport* viewport = mViewport;
    viewport->DispatchEvent(Events::KeyUp, event);
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* event) override
  {
    // Check objects
    EditorViewport* editorViewport = mViewport;
    if(editorViewport == NULL)
      return;

    Viewport* viewport = editorViewport->GetReactiveViewport();
    if(viewport == NULL)
      return;

    Space* space = editorViewport->GetTargetSpace();
    if(space == NULL)
      return;

    Element* dragElement = mDragElement;
    if(dragElement == NULL)
      return;

    // Get the selection to modify
    MetaSelection* selection = Z::gEditor->GetSelection();

    Vec2 mouseStart = mMouseStartPosition;
    Vec2 mouseCurrent = event->Position;

    // Can be dragged left to right or right to left
    Vec2 upperLeftScreen = Vec2(Math::Min(mouseStart.x, mouseCurrent.x),
                                Math::Min(mouseStart.y, mouseCurrent.y));
    Vec2 lowerRightScreen = Vec2(Math::Max(mouseStart.x, mouseCurrent.x), 
                                 Math::Max(mouseStart.y, mouseCurrent.y));

    // Clamp to the view port size
    Vec2 minSelectSize = viewport->ViewportToScreen(Vec2(0, 0));
    Vec2 maxSelectSize = viewport->ViewportToScreen(viewport->GetSize());
    upperLeftScreen = Vec2(Math::Max(upperLeftScreen.x, minSelectSize.x), 
                           Math::Max(upperLeftScreen.y, minSelectSize.y));
    lowerRightScreen = Vec2(Math::Min(lowerRightScreen.x, maxSelectSize.x), 
                            Math::Min(lowerRightScreen.y, maxSelectSize.y));

    // If the drag area is too small do nothing
    const float minDragSize = 5.0f;
    if((upperLeftScreen - lowerRightScreen).Length() < minDragSize || 
       upperLeftScreen.x == lowerRightScreen.x || 
       upperLeftScreen.y == lowerRightScreen.y)
      return;

    // Update the drag area
    dragElement->SetVisible(true);
    Vec3 size = Vec3(lowerRightScreen - upperLeftScreen);
    dragElement->SetSize(Vec2(size.x, size.y));
    dragElement->SetTranslation(Vec3(viewport->ScreenToViewport(upperLeftScreen)));

    SelectToolFrustumEvent frustumEvent;

    // Frustum cast to find selected objects
    frustumEvent.mFrustum = viewport->SubFrustum(upperLeftScreen, lowerRightScreen);
    space->DispatchEvent(Events::SelectToolFrustumCast, &frustumEvent);

    if(frustumEvent.Handled || frustumEvent.HandledEventScript)
      return;

    // allow up to 1000 results from raycasting
    RaycastResultList results(1000);
    // build up the info we pass around
    CastInfo castInfo(space, viewport->mViewportInterface->GetCameraCog(), mouseStart, mouseCurrent);

    // do the frustum cast here if no one else handled it
    mTool->mRaycaster.FrustumCast(frustumEvent.mFrustum, castInfo, results);

    selection->Clear(SendsEvents::False);

    // while shift is held copy in the current selection
    if (event->ShiftPressed || event->CtrlPressed || mTool->mSmartGroupSelect)
    {
      forRange(Cog* cog, mCurrentSelection.AllOfType<Cog>())
        selection->Add(cog, SendsEvents::False);
    }

    for(uint i = 0; i < results.mSize; ++i)
    {
      Cog* hitCog = results.mEntries[i].HitCog;

      // if we have something selected and mSmartGroupSelect is enabled or ctrl pressed
      // only selects direct children of the currently selected archetype
      if(!mCurrentSelection.Empty() && (mTool->mSmartGroupSelect || event->CtrlPressed))
      {
        Cog* currentHitParent = hitCog->FindNearestParentArchetype();
        if(mCurrentSelection.Contains(currentHitParent))
          selection->Add(hitCog, SendsEvents::False);
      }
      // only add parent cogs to the selection if select root is set
      else if (mTool->mSmartSelect)
      {
        Cog* rootArchetype = hitCog ? hitCog->FindRootArchetype() : nullptr;
        // if we have a root archetype just add it
        if (rootArchetype)
        {
          // we don't want to add the parent of multiple hit children more than once
          if (!selection->Contains(rootArchetype))
            selection->Add(rootArchetype, SendsEvents::False);
        }
        else
        {
          // if there is no archetype add the cog as is
          selection->Add(hitCog, SendsEvents::False);
        }
      }
      // add everything otherwise
      else
      {
        selection->Add(hitCog, SendsEvents::False);
      }
    }
    selection->SelectionChanged( );
  }

  //****************************************************************************
  void OnMouseUp(MouseEvent* event) override
  {
    MetaSelection* selection = Z::gEditor->GetSelection();
    selection->FinalSelectionChanged();
    this->Destroy();
  }
};

//******************************************************************************
SelectionResult EditorRayCast(Viewport* viewport, Vec2 mousePosition)
{
  return Z::gEditor->Tools->mSelectTool->RayCastSelect(viewport, mousePosition);
}

//------------------------------------------------------------------ Select Tool
ZilchDefineType(SelectTool, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  
  ZilchBindFieldProperty(mSmartSelect);
  ZilchBindFieldProperty(mSmartGroupSelect);
  ZeroBindTag(Tags::Tool);
  ZilchBindMethod(RayCast);

  ZeroBindEvent(Events::SelectToolPreSelect, ViewportMouseEvent);

  type->Add(new RaycasterMetaComposition(offsetof(SelectTool, mRaycaster)));
}

//******************************************************************************
SelectTool::SelectTool()
{
  mRaycaster.AddProvider(new PhysicsRaycastProvider());
  mRaycaster.AddProvider(new GraphicsRaycastProvider());
}

//******************************************************************************
void SelectTool::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSmartSelect, true);
  SerializeNameDefault(mSmartGroupSelect, false);
}

//******************************************************************************
void SelectTool::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::ToolDraw, OnToolDraw);
  ConnectThisTo(GetOwner(), Events::MouseUpdate, OnMouseUpdate);
  ConnectThisTo(GetOwner(), Events::LeftMouseUp, OnLeftMouseUp);
}

//******************************************************************************
void SelectTool::OnToolDraw(Event* e)
{
  this->DispatchEvent(Events::SelectToolPreDraw, e);
  if(e->mTerminated == true)
    return;

  MetaSelection* selection = Z::gEditor->mSelection;

  Cog* primary = selection->GetPrimaryAs<Cog>();
  if(!primary)
    return;

  // Draw the selection
  forRange(Cog* cog, selection->AllOfType<Cog>())
  {
    // The object could have been destroyed, but not yet removed
    if(cog->GetMarkedForDestruction())
      continue;

    if(cog->GetSpace() != Z::gEditor->GetEditSpace())
      continue;

    EventReceiver* receiver = cog->GetReceiver();
    ReceiverList::range connections = receiver->GetConnections();
    for(;!connections.Empty();connections.PopFront())
    {
      EventConnection* connect = &connections.Front();
      connect->DebugDraw();
    }

    cog->DebugDraw();

    if(Transform* transform = cog->has(Transform))
    {
      Mat4 worldMatrix = transform->GetWorldMatrix();
      Vec3 rootTranslation;
      Vec3 rootScale;
      Mat3 rootRotation;

      worldMatrix.Decompose(&rootScale, &rootRotation, &rootTranslation);

      Vec3 worldSize = Vec3(0.1f);

      const float visualExpansion = 0.01f;
        
      Vec3 expansion = Vec3(visualExpansion);
      Quat rotation = Quat::cIdentity;
      Vec3 translation = rootTranslation;

      if (Graphical* graphical = cog->has(Graphical))
      {
        Obb obb = graphical->GetWorldObb();
        worldSize = obb.HalfExtents;
        translation = obb.Center;
        rotation = ToQuaternion(obb.Basis);
      }
      else
      {
        Aabb aabb = GetAabb(cog);
        worldSize = aabb.GetHalfExtents();
        translation = aabb.GetCenter();
      }

      worldSize += expansion;

      if(primary == cog)
        gDebugDraw->Add(Debug::Obb(translation, worldSize, rotation).Color(Color::Orange).Corners(true));
      else
        gDebugDraw->Add(Debug::Obb(translation, worldSize, rotation).Color(Color::White).Corners(true));
    }
  }

}

//******************************************************************************
void SelectTool::OnMouseUpdate(ViewportMouseEvent* e)
{
  RaycastResultList result = RayCastSelectInternal(e->mViewport, e->Position);
  Cog* selectedCog = result.mEntries[0].HitCog;

  if(selectedCog)// && !Z::gEditor->mActiveSelection->IsPresent(selectedCog))
    selectedCog->DispatchEvent(Events::MouseHover, e);
}

//******************************************************************************
void SelectTool::OnLeftMouseUp(ViewportMouseEvent* e)
{
  if(e->Handled || e->HandledEventScript)
    return;
  
  if(Viewport* viewport = e->mViewport)
  {
    Space* targetSpace = viewport->GetTargetSpace();
    targetSpace->DispatchEvent(Events::SelectToolPreSelect, e);

    if(e->Handled || e->HandledEventScript)
      return;
  }

  this->Select(e);
}

//******************************************************************************
bool SelectTool::IsLastHitArchetype(Cog* cog)
{
  return cog == mLastHitArchetype.Get<Cog*>();
}

//******************************************************************************
bool SelectTool::IsChildOfLastHitArchetype(Cog* cog)
{
  if (mLastHitArchetype.IsNull())
    return false;

  Cog* parent = cog;
  Cog* lastArchetype = mLastHitArchetype.Get<Cog*>();

  while (parent != nullptr)
  {
    if (parent == lastArchetype)
      return true;

    // If we hit another Archetype, we're not part of the last hit Archetype context
    if (parent->GetArchetype() != nullptr)
      return false;

    parent = parent->GetParent();
  }
  return false;
}

//******************************************************************************
void SelectTool::Select(ViewportMouseEvent* e)
{
  RaycastResultList result = RayCastSelectInternal(e->mViewport, e->Position);
  Cog* toSelect = result.mEntries[0].HitCog;

  Keyboard* keyboard = Keyboard::GetInstance();
  bool multiSelect = keyboard->KeyIsDown(Keys::Shift);

  MetaSelection* selection = Z::gEditor->mSelection;

  if (toSelect != nullptr)
  {
    // if we are multi selecting objects just add the object to the selection
    // and clear our tracking of archetype based selection
    if (multiSelect)
    {
      selection->Add(toSelect, SendsEvents::False);
      mLastHitArchetype.Clear();
    }
    // smart select is not enabled so just select whatever the user clicked on regardless of hierarchy
    else if (!mSmartSelect)
    {
      selection->SelectOnly(toSelect);
    }
    else
    {
      selection->Clear(SendsEvents::False);
      // for smart select if we have an archetype selected take the next object hit
      // to account for sub objects contained within the parent
      if (IsLastHitArchetype(toSelect))
      {
        RayCastEntries::range entries = result.mEntries.All();
        // we want to remove the first entry as we already checked it
        entries.PopFront();
        while(!entries.Empty())
        {
          RayCastEntry entry = entries.Front();
          if(IsChildOfLastHitArchetype(entry.HitCog))
          {
            toSelect = entry.HitCog;
            break;
          }
          entries.PopFront();
        }
      }
      // if the object is a child of our archetype select it
      if (IsChildOfLastHitArchetype(toSelect))
      {
        selection->SelectOnly(toSelect);
      }
      // otherwise select the new objects nearest archetype parent
      // and if it is not part of an archetype just select the object
      else
      {
        mLastHitArchetype = toSelect->FindNearestArchetype();
        if (mLastHitArchetype.IsNotNull())
          selection->SelectOnly(mLastHitArchetype);
        else
          selection->SelectOnly(toSelect);
      }
    }
    selection->FinalSelectionChanged();
  }
  // we clicked on nothing so clear out current selection
  else
  {
    mLastHitArchetype.Clear();
    selection->Clear(SendsEvents::False);
    selection->FinalSelectionChanged();
  }
}

//******************************************************************************
SelectionResult SelectTool::RayCastSelect(Viewport* viewport, Vec2 mousePosition)
{
  // Extract the 1st result
  Cog* hitCog = NULL;
  Vec3 position = Vec3::cZero;
  Vec3 normal = Vec3::cZAxis;

  // Get the target space (and make sure it exists)
  Space* space = viewport->GetTargetSpace();
  if(space == NULL)
    return SelectionResult(hitCog, position, normal);

  // For the editor raycast, we only want 1 result
  RaycastResultList results(1);
  // Create the structure of the info to pass around
  CastInfo castInfo(space, viewport->mViewportInterface->GetCameraCog(), mousePosition);
  Ray ray = viewport->ScreenToWorldRay(mousePosition);
  // Get all of the results
  mRaycaster.RayCast(ray,castInfo,results);

  if(results.mSize != 0)
  {
    hitCog = results.mEntries[0].HitCog;
    normal = results.mEntries[0].HitWorldNormal;
    position = ray.GetPoint(results.mEntries[0].T);
  }

  return SelectionResult(hitCog, position, normal);
}

//******************************************************************************
RaycastResultList SelectTool::RayCastSelectInternal(Viewport* viewport, Vec2 mousePosition)
{
  // For the editor raycast, we only want the first few results
  RaycastResultList results(5);

  // Get the target space (and make sure it exists)
  Space* space = viewport->GetTargetSpace();
  if(space == NULL)
    return results;

  // Create the structure of the info to pass around
  CastInfo castInfo(space, viewport->mViewportInterface->GetCameraCog(), mousePosition);
  Ray ray = viewport->ScreenToWorldRay(mousePosition);
  // Get all of the results
  mRaycaster.RayCast(ray, castInfo, results);

  return results;
}

//******************************************************************************
Cog* SelectTool::RayCast(Viewport* viewport, Vec2 mousePosition)
{
  SelectionResult result = RayCastSelect(viewport, mousePosition);
  return result.Object;
}

//******************************************************************************
void SelectTool::AddCastProvider(RaycastProvider* provider)
{
  mRaycaster.AddProvider(provider);
}

//******************************************************************************
void ColliderRayCast(Viewport* viewport, Vec2Param mousePosition, 
                     ColliderRayCastResult& result, BaseCastFilter* filter)
{
  CastFilter defaultFilter;
  if(filter == NULL)
    filter = &defaultFilter;

  uint objectCount = 1;
  CastResults results(objectCount, *filter);

  PhysicsSpace* physicsSpace = viewport->mTargetSpace->has(PhysicsSpace);
  if(physicsSpace)
  {
    Ray worldRay(result.PositionInWorld, result.DirectionInWorld);
    physicsSpace->CastRay(worldRay, results);
    CastResults::range r = results.All();
    if(r.Empty())
    {
      result.HitCollider = NULL;
    }
    else
    {
      result.PositionOnObject = r.Front().mPoints[0];
      result.HitCollider = r.Front().GetCollider();
    }
  }
  else
  {
    result.HitCollider = NULL;
  }
}

//******************************************************************************
void BeginSelectDrag(EditorViewport* viewport, MouseEvent* event, SelectTool* tool)
{
  new GroupSelectDrag(viewport->GetParent(), event->GetMouse(), viewport, tool);
}

//---------------------------------------------------------------- Creation Tool
ZilchDefineType(CreationTool, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindFieldProperty(mPlacementMode);
  ZilchBindGetterSetterProperty(ObjectToSpawn);
  ZilchBindFieldProperty(mOffset);
  ZilchBindFieldProperty(mSnapping);
  ZilchBindFieldProperty(mSnapDistance);
  ZilchBindFieldProperty(mDepth);
  ZilchBindFieldProperty(mDepthPlane);

  ZeroBindTag(Tags::Tool);

  type->Add(new RaycasterMetaComposition(offsetof(CreationTool, mRaycaster)));
}

//******************************************************************************
CreationTool::CreationTool()
{
  PhysicsRaycastProvider* physicsProvider = new PhysicsRaycastProvider();
  physicsProvider->mActive = true;
  GraphicsRaycastProvider* graphicsProvider = new GraphicsRaycastProvider();
  graphicsProvider->mVisibleOnly = true;
  mRaycaster.AddProvider(physicsProvider);
  mRaycaster.AddProvider(graphicsProvider);
}

//******************************************************************************
void CreationTool::Serialize(Serializer& stream)
{
  SerializeResourceName(mObjectToSpawn, ArchetypeManager);
  SerializeNameDefault(mOffset, Vec3::cZero);
  SerializeNameDefault(mDepth, 20.0f);
  SerializeNameDefault(mDepthPlane, 0.0f);
  SerializeNameDefault(mSnapDistance, 0.5f);
  SerializeNameDefault(mSnapping, false);
  SerializeEnumNameDefault(Placement, mPlacementMode, Placement::OnTop);
  SerializeNameDefault(mTargetPoint, Vec3::cZero);
}

//******************************************************************************
void CreationTool::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(GetOwner(), Events::MouseMove, OnMouseMove);
  ConnectThisTo(GetOwner(), Events::ToolDraw, OnToolDraw);
}

//******************************************************************************
void CreationTool::OnLeftMouseDown(ViewportMouseEvent* e)
{
  CreateWithViewport(e->mViewport, e->Position, mObjectToSpawn);
  e->Handled = true;
}

//******************************************************************************
void CreationTool::OnMouseMove(ViewportMouseEvent* e)
{
  mMousePos = e->Position;
  UpdateMouse(e->mViewport, e->Position);
}

//******************************************************************************
void CreationTool::OnToolDraw(Event* e)
{
  gDebugDraw->Add(Debug::LineCross(mTargetPoint, 1));
}

//******************************************************************************
void CreationTool::UpdateMouse(Viewport* viewport, Vec2 screenPosition)
{
  mTargetPoint = GetPlacementLocation(viewport, screenPosition);
  mValidPoint = true;
}

//******************************************************************************
Cog* CreationTool::CreateAt(Viewport* viewport, Archetype* archetype, 
                            Vec3Param position)
{
  Space* space = viewport->GetTargetSpace();
  Cog* object = CreateFromArchetype(Z::gEditor->mQueue, space, archetype,  position);
  Z::gEditor->SelectOnly(object);

  // Mark the transform as modified 
  object->MarkTransformModified();

  return object;
}

//******************************************************************************
Vec3 CreationTool::PointOnViewPlane(EditorCameraController* controller, Ray& worldRay)
{
  // Get the point in the direction the camera is facing
  // on the same plane as the look at point
  Vec3 cameraDirection = controller->GetCameraDirection( );
  Vec3 lookAtPoint = controller->GetLookTarget( );

  if(controller->GetControlMode( ) == ControlMode::FirstPerson)
    lookAtPoint += mDepth * cameraDirection;

  // Compute the plane from the eye direction and the near plane.
  Plane plane(cameraDirection, lookAtPoint);
  // Intersect the ray with the plane.
  Intersection::IntersectionPoint point;
  RayPlane(worldRay.Start, mMouseDir, plane.GetNormal( ), plane.GetDistance( ), &point);
  // Take the start point from the intersection
  return point.Points[0];
}

//******************************************************************************
Vec3 CreationTool::PointOnTopOrViewPlane(Viewport* viewport, EditorCameraController* controller, Ray& worldRay)
{
  Space* objectSpace = viewport->GetTargetSpace( );
  Camera* camera = viewport->GetCamera( );

  Vec3 position;

  // Attempt a raycast against other objects.
  CastInfo info(objectSpace, camera->mOwner, mMousePos);
  RaycastResultList results;
  mRaycaster.RayCast(worldRay, info, results);

  // If anything's hit - take the position hit as the creation point.
  if(results.mSize)
  {
    position = results.mEntries.Front( ).HitWorldPosition;
  }
  else // Otherwise use the view plane method of creation.
  {
    position = PointOnViewPlane(controller, worldRay);
  }

  return position;
}

//******************************************************************************
Vec3 CreationTool::GetPlacementLocation(Viewport* viewport, Vec2 screenPosition)
{
  Space* objectSpace = viewport->GetTargetSpace();
  Camera* camera = viewport->GetCamera();

  if(objectSpace && camera)
  {
    Vec3 cameraPosition = camera->mTransform->GetWorldTranslation();

    Ray worldRay = viewport->ScreenToWorldRay(screenPosition);
    mMouseDir = worldRay.Direction;

    Vec3 position(0,0,0);

    switch(mPlacementMode)
    {
      //------------------------------------------------------------------------
      case Placement::OnTop:
        if(EditorCameraController* controller = camera->GetOwner( )->has(EditorCameraController))
          position = PointOnTopOrViewPlane(viewport, controller, worldRay);

        break;

      //------------------------------------------------------------------------
      case Placement::LookAtPlane:
        if(EditorCameraController* controller = camera->GetOwner()->has(EditorCameraController))
          position = PointOnViewPlane(controller, worldRay);
 
        break;

      //------------------------------------------------------------------------
      case Placement::LookAtPoint:
        if(EditorCameraController* controller = camera->GetOwner( )->has(EditorCameraController))
          position = controller->GetLookTarget( );

        break;

      //------------------------------------------------------------------------
      case Placement::ViewAtDepth:
        position = cameraPosition + mMouseDir * mDepth;
        break;

      //------------------------------------------------------------------------
      case Placement::CameraLocation:
        position = cameraPosition;
        break;

      //------------------------------------------------------------------------
      case Placement::PlaneXY:
        Intersection::IntersectionPoint point;
        RayPlane(worldRay.Start, mMouseDir, Vec3(0,0,1), mDepthPlane, &point);
        position = point.Points[0];

        break;
    }

    position += mOffset;

    if(mSnapping)
      position = Snap(position, mSnapDistance);

    return position;
  }

  return Vec3(0,0,0);
}

//******************************************************************************
Cog* CreationTool::CreateWithViewport(Viewport* viewport, Vec2 screenPosition, 
                                      StringParam archetypeName)
{
  Archetype* archetype = ArchetypeManager::Find(archetypeName);
  return CreateWithViewport(viewport, screenPosition, archetype);
}

//******************************************************************************
Cog* CreationTool::CreateWithViewport(Viewport* viewport, Vec2 screenPosition, 
                                      Archetype* archetype)
{
  UpdateMouse(viewport, screenPosition);
  Vec3 placement = GetPlacementLocation(viewport, screenPosition);
  return CreateAt(viewport, archetype, placement);
}

//******************************************************************************
Archetype* CreationTool::GetObjectToSpawn()
{
  return mObjectToSpawn;
}

//******************************************************************************
void CreationTool::SetObjectToSpawn(Archetype* archetype)
{
  mObjectToSpawn = archetype;
}

//------------------------------------------------------- Object Connecting Tool
ZilchDefineType(ObjectConnectingTool, builder, type)
{
  ZeroBindDependency(MouseCapture);
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZeroBindTag(Tags::Tool);
}

//******************************************************************************
ObjectConnectingTool::ObjectConnectingTool()
{
  ObjectA = cInvalidCogId;
  ObjectB = cInvalidCogId;
}

//******************************************************************************
void ObjectConnectingTool::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(GetOwner(), Events::KeyDown, OnKeyDown);
  ConnectThisTo(GetOwner(), Events::MouseDragMove, OnMouseDragMove);
  ConnectThisTo(GetOwner(), Events::MouseDragEnd, OnMouseEndDrag);
  ConnectThisTo(GetOwner(), Events::ToolDraw, OnToolDraw);
  ConnectThisTo(GetOwner(), Events::ToolDeactivate, OnToolDeactivate);
}

//******************************************************************************
void ObjectConnectingTool::OnLeftMouseDown(ViewportMouseEvent* e)
{
  // Cast for the first object to connect to
  SelectionResult results = EditorRayCast(e->mViewport, e->Position);
  if(results.Object)
  {
    PointOnObjectA = results.Position;
    ObjectA = results.Object;
    
    if(MouseCapture* mouseCapture = GetOwner()->has(MouseCapture))
    {
      mouseCapture->Capture(e);
      e->Handled = true;
    }
  }
}

//******************************************************************************
void ObjectConnectingTool::OnToolDraw(Event* e)
{
  if(ObjectA)
  {
    gDebugDraw->Add(Debug::Sphere(PointOnObjectA, 0.25f).Color(Color::Red).ViewScaled(true));
    if(ObjectB)
    {
      gDebugDraw->Add(Debug::Line(PointOnObjectA, PointOnObjectB).HeadSize(0.3f).Color(Color::Green));
      gDebugDraw->Add(Debug::Sphere(PointOnObjectB, 0.25f).Color(Color::Red).ViewScaled(true));
    }
  }
}

//******************************************************************************
void ObjectConnectingTool::OnKeyDown(KeyboardEvent* e)
{
  if(e->Key == Keys::Escape)
    OnToolDeactivate(nullptr);
}

//******************************************************************************
void ObjectConnectingTool::OnMouseDragMove(ViewportMouseEvent* e)
{
  SelectionResult rcr = EditorRayCast(e->GetViewport(), e->Position);
  if(rcr.Object && !(rcr.Object == ObjectA))
  {
    PointOnObjectB = rcr.Position;
    ObjectB = rcr.Object;
  }

  e->Handled = true;
};

//******************************************************************************
void ObjectConnectingTool::OnMouseEndDrag(Event*)
{
  if(ObjectA.IsValid() && ObjectB.IsValid())
    DoConnection();
  
  ObjectA = cInvalidCogId;
  ObjectB = cInvalidCogId;
};

//******************************************************************************
void ObjectConnectingTool::OnToolDeactivate(Event*)
{
  ObjectA = cInvalidCogId;
  ObjectB = cInvalidCogId;
}

//--------------------------------------------------------------- Parenting Tool
ZilchDefineType(ParentingTool, builder, type)
{
  ZeroBindComponent();
  ZeroBindTag(Tags::Tool);
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindFieldProperty(mMaintainPosition);
}

//******************************************************************************
ParentingTool::ParentingTool()
{
  
}

//******************************************************************************
void ParentingTool::Serialize(Serializer& stream)
{
  SerializeNameDefault(mMaintainPosition, true);
}

//******************************************************************************
void ParentingTool::DoConnection()
{
  if(Cog* a = ObjectA)
  {
    if(Cog* b = ObjectB)
    {
      OperationQueue* queue = Z::gEditor->GetOperationQueue();
      AttachObject(queue, a, b, mMaintainPosition);
    }
  }
}

//----------------------------------------------------------- Hull 3D Debug Tool
uint GenerateDirections(uint granularity, Vec3Ptr& directions)
{
  uint vertexCount;
  uint* indexBuffer = NULL;
  uint indexCount;
  Vec3* normalBuffer = NULL;
  Vec2* textureCoords = NULL;
  Geometry::BuildIcoSphere(granularity, directions, vertexCount, indexBuffer, 
                           indexCount);
  return vertexCount;
}

}//namespace Zero
