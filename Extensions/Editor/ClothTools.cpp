///////////////////////////////////////////////////////////////////////////////
///
/// \file ClothTools.cpp
/// Implementation of the Cloth Tool classes.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

uint GetClosestTrianglePoint(const Triangle& tri, Vec3Param pointOnTriangle)
{
  //get the distance from each corner of the triangle
  real distance0 = Math::LengthSq(pointOnTriangle - tri.p0);
  real distance1 = Math::LengthSq(pointOnTriangle - tri.p1);
  real distance2 = Math::LengthSq(pointOnTriangle - tri.p2);

  //select the index of the closest point
  if(distance0 < distance1)
  {
    if(distance0 < distance2)
      return 0;
    else
      return 2;
  }
  else if(distance1 < distance2)
    return 1;
  else
    return 2;
}

//-------------------------------------------------------------------SpringSubTool
ZilchDefineType(SpringSubTool, builder, type)
{
  // These options are referred to directly by pointer on the import options (unsafe for script)
  type->HandleManager = ZilchManagerId(PointerManager);
  type->Add(new TypeNameDisplay());
  ZeroBindExpanded();
}

SpringSystem* SpringSubTool::GetSystem()
{
  Cog* cog = mSelectedSystem;
  if(cog != nullptr)
    return cog->has(SpringSystem);
  return nullptr;
}

bool SpringSubTool::RayCastSpringSystem(const Ray& ray, SpringSystem* system, Vec3& closestPoint, uint& index)
{
  if(system == nullptr)
    return false;

  Vec3 intersectionPoint;
  SpringSystem::Face hitFace;
  //see if a raycast hits any triangles on the spring system
  if(system->Cast(ray, hitFace, intersectionPoint) == false)
    return false;
  
  //find out what triangle point the ray intersection was closest to
  Triangle tri;
  tri.p0 = system->mPointMasses[hitFace.mIndex0].mPosition;
  tri.p1 = system->mPointMasses[hitFace.mIndex1].mPosition;
  tri.p2 = system->mPointMasses[hitFace.mIndex2].mPosition;

  uint selectedCoord = GetClosestTrianglePoint(tri, intersectionPoint);
  
  //set what point was the closest and what index it was
  closestPoint = tri[selectedCoord];
  index = ((uint*)(&hitFace))[selectedCoord];

  return true;
}

bool SpringSubTool::RayCastCog(ViewportMouseEvent* e, CogId& hitCog,
                               Vec3& hitPoint, uint& hitIndex)
{
  Viewport* viewport = e->mViewport;

  SelectionResult result = EditorRayCast(viewport, e->Position);
  //if we didn't get an object then make sure to clear out the hit cog
  if(result.Object == nullptr)
  {
    hitCog = CogId();
    return false;
  }

  //by default store the position that we hit the object
  hitPoint = result.Position;

  //if we had a spring system then check it instead
  SpringSystem* system = result.Object->has(SpringSystem);
  if(system != nullptr)
  {
    //check where the ray hits this spring system
    Ray ray = viewport->ScreenToWorldRay(e->Position);
    bool hitSystem = RayCastSpringSystem(ray, system, hitPoint, hitIndex);  
    //if we hit the object but not a triangle on the spring system then don't count it as hit
    if(hitSystem == false)
      return false;
  }

  //store the object we hit
  hitCog = result.Object;

  return true;
}

//-------------------------------------------------------------------DragSelectSubTool
ZilchDefineType(DragSelectSubTool, builder, type)
{
}

DragSelectSubTool::DragSelectSubTool()
{
  
}

void DragSelectSubTool::OnMouseDragStart(ViewportMouseEvent* e)
{
  Viewport* viewport = e->mViewport;
  //if we haven't already created the drag element then do so
  Element* dragElement = mDragElement;
  if(dragElement == nullptr)
  {
    dragElement = viewport->CreateAttached<Element>("DragBox");
    mDragElement = dragElement;
  }

  //make it invisible till we actually start dragging
  dragElement->SetVisible(false);
  mMouseStartPosition = e->Position;
}

void DragSelectSubTool::OnMouseDragMove(ViewportMouseEvent* e)
{
  Viewport* viewport = e->mViewport;
  Vec2 mouseStart = mMouseStartPosition;
  Vec2 newPosition = e->Position;

  // Get the corners (Can be dragged left to right or right to left)
  Vec2 upperLeftScreen = Vec2(Math::Min(mouseStart.x, mouseStart.x),
    Math::Min(mouseStart.y, newPosition.y));
  Vec2 lowerRightScreen = Vec2(Math::Max(mouseStart.x, newPosition.x), 
    Math::Max(mouseStart.y, newPosition.y));
  
  // Clamp to the viewport size
  Vec2 minSelectSize = viewport->ViewportToScreen(Vec2(0, 0));
  Vec2 maxSelectSize = viewport->ViewportToScreen(viewport->GetSize());
  upperLeftScreen = Vec2(Math::Max(upperLeftScreen.x, minSelectSize.x), 
    Math::Max(upperLeftScreen.y, minSelectSize.y));
  lowerRightScreen = Vec2(Math::Min(lowerRightScreen.x, maxSelectSize.x), 
    Math::Min(lowerRightScreen.y, maxSelectSize.y));
  
  //can't cast a frustum of zero size
  if(upperLeftScreen.x != lowerRightScreen.x && upperLeftScreen.y != lowerRightScreen.y)
  {
    //call the virtual function of frustum casting to let an individual tool do something
    MouseDragFrustum(viewport, upperLeftScreen, lowerRightScreen);
  }
  
  Element* dragElement = mDragElement;
  if(dragElement == nullptr)
    return;
  
  //set the position and size of the drag element
  dragElement->SetVisible(true);
  Vec3 size = Vec3(lowerRightScreen - upperLeftScreen);
  dragElement->SetSize(Vec2(size.x, size.y));
  dragElement->SetTranslation(Vec3(viewport->ScreenToViewport(upperLeftScreen)));
  
  return;
}

void DragSelectSubTool::OnMouseEndDrag(Event* e)
{
  //make the drag element invisible if we can
  Element* dragElement = mDragElement;
  if(dragElement == nullptr)
    return;

  dragElement->SetVisible(false);
}
//-------------------------------------------------------------------SelectorSpringSubTool
ZilchDefineType(SelectorSpringSubTool, builder, type)
{
}

SelectorSpringSubTool::SelectorSpringSubTool()
{
}

void SelectorSpringSubTool::OnMouseDragStart(ViewportMouseEvent* e)
{
  //do the default drag selection logic (to start drawing the drag select element)
  DragSelectSubTool::OnMouseDragStart(e);

  //if shift is down then take all of current selection and add them to the previous selection
  Keyboard* keyboard = Keyboard::GetInstance();
  if(keyboard->KeyIsDown(Keys::Shift))
  {
    PointMap::range range = mCurrentSelection.All();
    for(; !range.Empty(); range.PopFront())
    {
      uint index = range.Front();
      mPreviousSelection.Insert(index);
    }
  }
  //otherwise a new drag is starting so clear out our previous selection
  else
    mPreviousSelection.Clear();
}

void SelectorSpringSubTool::OnMouseDragMove(ViewportMouseEvent* e)
{
  //make sure we have a valid spring system
  SpringSystem* system = GetSystem();
  if(system == nullptr)
    return;

  Viewport* viewport = e->mViewport;
  Vec2 mouseStart = mMouseStartPosition;
  Vec2 newPosition = e->Position;

  // Get the corners (Can be dragged left to right or right to left)
  Vec2 upperLeftScreen = Vec2(Math::Min(mouseStart.x, newPosition.x),
                              Math::Min(mouseStart.y, newPosition.y));
  Vec2 lowerRightScreen = Vec2(Math::Max(mouseStart.x, newPosition.x),
                               Math::Max(mouseStart.y, newPosition.y));

  // Clamp to the viewport size
  Vec2 minSelectSize = viewport->ViewportToScreen(Vec2(0, 0));
  Vec2 maxSelectSize = viewport->ViewportToScreen(viewport->GetSize());
  upperLeftScreen = Vec2(Math::Max(upperLeftScreen.x, minSelectSize.x),
                         Math::Max(upperLeftScreen.y, minSelectSize.y));
  lowerRightScreen = Vec2(Math::Min(lowerRightScreen.x, maxSelectSize.x),
                          Math::Min(lowerRightScreen.y, maxSelectSize.y));

  //can't cast a frustum of zero size
  if(upperLeftScreen.x != lowerRightScreen.x && upperLeftScreen.y != lowerRightScreen.y)
  {
    //we constantly overwrite our current selection based upon
    //the current frustum (so clear the current selection)
    mCurrentSelection.Clear();
    //build a frustum from the viewport and the screen coordinates
    Frustum frustum = viewport->SubFrustum(upperLeftScreen, lowerRightScreen);

    //cast the frustum to see what we hit (virtual)
    CastFrustum(frustum);
  }

  Element* dragElement = mDragElement;
  if(dragElement == nullptr)
    return;

  //set the position and size of the drag element
  dragElement->SetVisible(true);
  Vec3 size = Vec3(lowerRightScreen - upperLeftScreen);
  dragElement->SetSize(Vec2(size.x, size.y));
  dragElement->SetTranslation(Vec3(viewport->ScreenToViewport(upperLeftScreen)));
}

void SelectorSpringSubTool::OnLeftMouseUp(ViewportMouseEvent* e)
{
  //if the mouse went up and shift wasn't being held then the user
  //clicked somewhere random so clear all of our selections
  Keyboard* keyboard = Keyboard::GetInstance();
  if(keyboard->KeyIsDown(Keys::Shift) == false)
  {
    mPreviousSelection.Clear();
    mCurrentSelection.Clear();
  }
}

void SelectorSpringSubTool::GetSelection(PointMap& indices)
{
  //combine the current and previous selection into one map
  PointMap::range range = mCurrentSelection.All();
  for(; !range.Empty(); range.PopFront())
  {
    uint index = range.Front();
    indices.Insert(index);
  }

  range = mPreviousSelection.All();
  for(; !range.Empty(); range.PopFront())
  {
    uint index = range.Front();
    indices.Insert(index);
  }
}

//-------------------------------------------------------------------PointMassSelectorSubTool
ZilchDefineType(PointMassSelectorSubTool, builder, type)
{
}

PointMassSelectorSubTool::PointMassSelectorSubTool()
{
}

void PointMassSelectorSubTool::Draw()
{
  SpringSystem* system = GetSystem();
  if(system == nullptr)
    return;

  //get a unique mapping of all points (this will avoid double drawing
  //points we previously selected and points we're currently selecting).
  PointMap indices;
  GetSelection(indices);

  PointMap::range range = indices.All();
  for(; !range.Empty(); range.PopFront())
  {
    uint index = range.Front();
    //make sure we don't have an invalid index for some reason
    if(index >= system->mPointMasses.Size())
      continue;

    Vec3 position = system->mPointMasses[index].mPosition;
    gDebugDraw->Add(Debug::Sphere(position, real(0.1)));
  }
}

void PointMassSelectorSubTool::CastFrustum(const Frustum& frustum)
{
  SpringSystem* system = GetSystem();

  //check each point mass to see if it is in the frustum
  //and if so add its index to our current selection
  for(uint i = 0; i < system->mPointMasses.Size(); ++i)
  {
    Vec3 position = system->mPointMasses[i].mPosition;
    if(Overlap(frustum, position))
      mCurrentSelection.Insert(i);
  }
}

//-------------------------------------------------------------------AnchoringSubTool
ZilchDefineType(AnchoringSubTool, builder, type)
{
  ZilchBindFieldProperty(mDrawAnchoredPoints);
  ZilchBindFieldProperty(mAnchoredPointMassColor);
}

AnchoringSubTool::AnchoringSubTool()
{
  mAnchoredPointMassColor = Vec4(1, 0, 0, 1);
}

void AnchoringSubTool::Draw()
{
  SpringSystem* system = GetSystem();
  if(system == nullptr)
    return;

  //draw all the point masses that are anchors a special color
  if(mDrawAnchoredPoints)
  {
    for(uint i = 0; i < system->mPointMasses.Size(); ++i)
    {
      SpringSystem::PointMass& pointMass = system->mPointMasses[i];
      if(pointMass.mAnchor != nullptr)
        gDebugDraw->Add(Debug::Sphere(pointMass.mPosition, real(0.1)).Color(mAnchoredPointMassColor));
    }
  }

  //draw all of the selected points
  PointMassSelectorSubTool::Draw();
}

//-------------------------------------------------------------------PointSelectorSubTool
ZilchDefineType(PointSelectorSubTool, builder, type)
{
}

PointSelectorSubTool::PointSelectorSubTool()
{
}

//-------------------------------------------------------------------SpringSelectorSubTool
ZilchDefineType(SpringSelectorSubTool, builder, type)
{
}

SpringSelectorSubTool::SpringSelectorSubTool()
{
}

void SpringSelectorSubTool::Draw()
{
  SpringSystem* system = GetSystem();
  if(system == nullptr)
    return;

  //get a unique mapping of all points (this will avoid double drawing
  //points we previously selected and points we're currently selecting).
  PointMap indices;
  GetSelection(indices);

  PointMap::range range = indices.All();
  for(; !range.Empty(); range.PopFront())
  {
    uint index = range.Front();
    //make sure the edge we stored was valid
    //(should always be valid unless the mesh was changed underneath us)
    if(index >= system->mEdges.Size())
      continue;

    SpringSystem::Edge& edge = system->mEdges[index];
    Vec3 p0 = system->mPointMasses[edge.mIndex0].mPosition;
    Vec3 p1 = system->mPointMasses[edge.mIndex1].mPosition;

    gDebugDraw->Add(Debug::Cylinder(p0, p1, real(0.05)).Color(Color::Red));
  }
}

void SpringSelectorSubTool::CastFrustum(const Frustum& frustum)
{
  SpringSystem* system = GetSystem();
  if(system == nullptr)
    return;

  //check each edge
  for(uint i = 0; i < system->mEdges.Size(); ++i)
  {
    SpringSystem::Edge& edge = system->mEdges[i];
    Vec3 p0 = system->mPointMasses[edge.mIndex0].mPosition;
    Vec3 p1 = system->mPointMasses[edge.mIndex1].mPosition;

    //currently don't have a segment vs. frustum check, so just check each vertex for now
    if(Overlap(frustum, p0) && Overlap(frustum, p1))
      mCurrentSelection.Insert(i);
  }
}

//-------------------------------------------------------------------SpringCreatorSubTool
ZilchDefineType(SpringCreatorSubTool, builder, type)
{
}

SpringCreatorSubTool::SpringCreatorSubTool()
{
}

void SpringCreatorSubTool::Draw()
{
  if(mIsDragging)
    gDebugDraw->Add(Debug::Line(mStartPos, mEndPos).HeadSize(0.1f).OnTop(true));
}

void SpringCreatorSubTool::OnMouseDragStart(ViewportMouseEvent* e)
{
  SelectionResult result = EditorRayCast(e->mViewport, e->Position);
  if(result.Object == nullptr)
    return;

  SpringSystem* system = result.Object->has(SpringSystem);
  if(system == nullptr)
    return;

  mStartCog = result.Object;
  Ray ray = e->mWorldRay;
  RayCastSpringSystem(ray, system, mStartPos, mStartIndex);
  mIsDragging = true;
}

void SpringCreatorSubTool::OnMouseDragMove(ViewportMouseEvent* e)
{
  Viewport* viewport = e->mViewport;
  SelectionResult result = EditorRayCast(viewport, e->Position);
  if(result.Object == nullptr)
    return;
  
  SpringSystem* system = result.Object->has(SpringSystem);
  if(system == nullptr)
    return;
  
  mEndCog = result.Object;
  Ray ray = viewport->ScreenToWorldRay(e->Position);
  RayCastSpringSystem(ray, system, mEndPos, mEndIndex);
  //mEndPos = result.PositionOnObject;
}

void SpringCreatorSubTool::OnMouseEndDrag(Event* e)
{
  if(mStartCog == mEndCog && mStartCog.IsValid())
  {
    SpringSystem* system = mStartCog.has(SpringSystem);

    system->AddEdge(mStartIndex, mEndIndex);
  }
  else
  {
    ConnectDifferentSystems();
  }

  mIsDragging = false;
  mStartCog = mEndCog = CogId();
  mStartIndex = mEndIndex = (uint)-1;
}

void SpringCreatorSubTool::ConnectDifferentSystems()
{
  SpringSystem* system0 = mStartCog.has(SpringSystem);
  SpringSystem* system1 = mEndCog.has(SpringSystem);

  if(system0 == nullptr || system1 == nullptr)
    return;

  system0->AddConnection(system1, mStartIndex, mEndIndex);
  system0->GetSpace()->MarkModified();
}

//-------------------------------------------------------------------RopeCreatorSubTool
ZilchDefineType(RopeCreatorSubTool, builder, type)
{
  ZilchBindFieldProperty(mNumberOfLinks);
}

RopeCreatorSubTool::RopeCreatorSubTool()
{
  mNumberOfLinks = 10;
}

void RopeCreatorSubTool::Draw()
{
  if(mStartCog.IsValid() && mEndCog.IsValid())
    gDebugDraw->Add(Debug::Line(mStartPos, mEndPos).HeadSize(0.1f).OnTop(true));
}

void RopeCreatorSubTool::OnLeftMouseDown(ViewportMouseEvent* e)
{
  //figure out what cog we hit
  mIsDragging = RayCastCog(e, mStartCog, mStartPos, mStartIndex);
}

void RopeCreatorSubTool::OnMouseDragMove(ViewportMouseEvent* e)
{
  //if we never hit anyone on the first click then we don't want to do
  //any dragging logic (since we're connecting two points together)
  if(mIsDragging == false)
    return;
  
  RayCastCog(e, mEndCog, mEndPos, mEndIndex);
}

void RopeCreatorSubTool::OnMouseEndDrag(Event* e)
{
  Cog* startCog = mStartCog;
  Cog* endCog = mEndCog;

  //only create the rope if we were connecting to valid objects
  if(startCog != nullptr && endCog != nullptr && startCog != endCog)
  {
    //create a cog with the rope (and do simple error checking on everything being valid)
    Space* space = startCog->GetSpace();
    if(space == nullptr)
      return;

    Cog* cog = space->CreateNamed(CoreArchetypes::Transform);
    if(cog == nullptr)
      return;

    //create the spring rope component and fill out the
    //values needed to create the rope at runtime
    DecorativeRope* rope = new DecorativeRope();
    //default serialize it first so we don't wipe out the values we
    //need to set but we make sure all defaults get correct values
    DefaultSerializer defaultSerializer;
    rope->Serialize(defaultSerializer);

    rope->mCogA = startCog;
    rope->mCogB = endCog;
    rope->mPointMassIndexA = mStartIndex;
    rope->mPointMassIndexB = mEndIndex;
    rope->mNumberOfLinks = mNumberOfLinks;

    //compute the local points on the object if possible
    //(doesn't do anything on point masses, but whatever)
    Transform* tA = startCog->has(Transform);
    if(tA != nullptr)
      rope->mLocalPointA = tA->TransformPointInverse(mStartPos);
    Transform* tB = endCog->has(Transform);
    if(tB != nullptr)
      rope->mLocalPointB = tB->TransformPointInverse(mEndPos);

    //now that we can add the component since we've given it the correct values to initialize
    cog->AddComponent(rope);
    
    //make sure to mark the space modified otherwise the level won't think it needs to save
    space->MarkModified();
  }

  //make sure to clear out the what cogs we were connecting to
  Clear();
}


void RopeCreatorSubTool::OnKeyDown(KeyboardEvent* e)
{
  //if the user hits escape then cancel the operation
  if(e->Key == Keys::Escape)
    Clear();
}

void RopeCreatorSubTool::Clear()
{
  mStartCog = mEndCog = CogId();
}

uint RopeCreatorSubTool::GetNumberOfLinks()
{
  return mNumberOfLinks;
}

void RopeCreatorSubTool::SetNumberOfLinks(uint numberOfLinks)
{
  mNumberOfLinks = Math::Clamp(numberOfLinks, 2u, 100u);
}

//-------------------------------------------------------------------SpringPointProxy

// Simple helper to set the group value of a property
template <typename ValueType, typename CallbackType>
void SetGroupProperty(AnchoringSubTool* anchorTool, ValueType& value, CallbackType setPropertyFn)
{
  SpringSystem* system = anchorTool->GetSystem();
  if(system == nullptr)
    return;

  //get all of the indices to walk through
  HashSet<uint> indices;
  anchorTool->GetSelection(indices);

  //set each property
  HashSet<uint>::range range = indices.All();
  for(; !range.Empty(); range.PopFront())
  {
    uint index = range.Front();
    setPropertyFn(system, index, value);
  }

  //make sure to mark the space as modified (since we change objects in the level,
  //without this the level won't know it needs to save)
  system->GetSpace()->MarkModified();
}

template <typename CallbackType>
PropertyState GetGroupState(AnchoringSubTool* anchorTool, CallbackType getPropertyFn)
{
  SpringSystem* system = anchorTool->GetSystem();
  if(system == nullptr)
    return PropertyState();

  //get all of the indices to check
  HashSet<uint> indices;
  anchorTool->GetSelection(indices);

  //if we have no indices then return the empty property set (which means conflicted)
  if(indices.Empty())
    return PropertyState();

  HashSet<uint>::range range = indices.All();
  //grab the value and move to the next item in the range
  uint index = range.Front();
  range.PopFront();
  AutoDeclare(firstValue, getPropertyFn(system, index));

  //check to see if any value is different than the first,
  //if so return that we have a conflicted property state
  for(; !range.Empty(); range.PopFront())
  {
    index = range.Front();
    AutoDeclare(currentValue, getPropertyFn(system, index));

    if(firstValue != currentValue)
      return PropertyState();
  }

  //otherwise they're all the same so just return the value of the first item
  return PropertyState(firstValue);
}

bool GetFixedStateHelper(SpringSystem* system, uint index)
{
  return system->mPointMasses[index].mInvMass == real(0.0);
}

void SetFixedState(SpringSystem* system, uint index, bool& fixed)
{
  if(fixed)
    system->mPointMasses[index].mInvMass = real(0.0);
  else
    system->mPointMasses[index].mInvMass = real(1.0);
}

Cog* GetAnchorStateHelper(SpringSystem* system, uint index)
{
  SpringSystem::PointMass& pointMass = system->mPointMasses[index];
  if(pointMass.mAnchor != nullptr)
    return pointMass.mAnchor->mAnchorObject;
  return nullptr;
}

void SetAnchorState(SpringSystem* system, uint index, Cog* object)
{
  system->SetPointMassAnchor(index, object);
}

real GetMassStateHelper(SpringSystem* system, uint index)
{
  //return the inverse mass (if we can't invert it then just return 0)
  SpringSystem::PointMass& point = system->mPointMasses[index];
  real invMass = point.mInvMass;
  if(invMass != real(0.0))
    invMass = real(1.0) / invMass;
  return invMass;
}

void SetInvMassHelper(SpringSystem* system, uint index, real invMass)
{
  SpringSystem::PointMass& point = system->mPointMasses[index];
  point.mInvMass = invMass;
}
ZilchDefineType(SpringPointProxy, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindGetterSetterProperty(Fixed);
  ZilchBindGetterSetterProperty(Anchor);
  ZilchBindGetterSetterProperty(Mass);
}

SpringPointProxy::SpringPointProxy()
{
}

bool SpringPointProxy::GetFixed()
{
  return false;
}

void SpringPointProxy::SetFixed(bool fixed)
{
  SetGroupProperty(mAnchorTool, fixed, SetFixedState);
}

PropertyState SpringPointProxy::GetFixedState()
{
  return GetGroupState(mAnchorTool, GetFixedStateHelper);
}

Cog* SpringPointProxy::GetAnchor()
{
  return nullptr;
}

void SpringPointProxy::SetAnchor(Cog* object)
{
  SetGroupProperty(mAnchorTool, object, SetAnchorState);
}

PropertyState SpringPointProxy::GetAnchorState()
{
  return GetGroupState(mAnchorTool, GetAnchorStateHelper);
}

real SpringPointProxy::GetMass()
{
  return 0;
}

void SpringPointProxy::SetMass(real mass)
{
  real invMass = real(0.0);
  if(invMass != real(0.0))
    invMass = real(1.0) / mass;

  SetGroupProperty(mAnchorTool, invMass, SetInvMassHelper);
}

PropertyState SpringPointProxy::GetMassState()
{
  return GetGroupState(mAnchorTool, GetMassStateHelper);
}

//-------------------------------------------------------------------SpringPointProxyProperty
ZilchDefineType(SpringPointProxyProperty, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
}

PropertyState SpringPointProxyProperty::GetValue(HandleParam object, PropertyPathParam property)
{
  SpringPointProxy* proxy = object.Get<SpringPointProxy*>();

  Property* metaProp = property.GetPropertyFromRoot(proxy);

  if (metaProp->Name == "Fixed")
    return proxy->GetFixedState();
  if (metaProp->Name == "Anchor")
    return proxy->GetAnchorState();
  if (metaProp->Name == "Mass")
    return proxy->GetMassState();
  return PropertyState();
}

//-------------------------------------------------------------------SpringTools
ZilchDefineType(SpringTools, builder, type)
{
  ZeroBindComponent();
  ZeroBindDependency(MouseCapture);
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZeroBindTag(Tags::Tool);

  //changing this property will invalidate us (so we refresh the property view)
  ZilchBindGetterSetterProperty(CurrentSubToolType)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mCurrentSubTool);
}

SpringTools::SpringTools()
{
  
}

SpringTools::~SpringTools()
{
  for(size_t i = 0; i < mSubTools.Size(); ++i)
    delete mSubTools[i];
  mSubTools.Clear();
}

void SpringTools::Initialize(CogInitializer& initializer)
{
  mCurrentSubTool = nullptr;
  mCurrentSubToolType = SpringSubTools::Anchoring;

  mSubTools.PushBack(new AnchoringSubTool());
  mSubTools.PushBack(new PointSelectorSubTool());
  mSubTools.PushBack(new SpringSelectorSubTool());
  mSubTools.PushBack(new SpringCreatorSubTool());
  mSubTools.PushBack(new RopeCreatorSubTool());

  mCurrentSubTool = mSubTools[0];
  mProxy.mAnchorTool = (AnchoringSubTool*)mSubTools[0];

  mMouseIsDown = false;


  ConnectThisTo(GetOwner(), Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(GetOwner(), Events::LeftMouseUp, OnLeftMouseUp);
  ConnectThisTo(GetOwner(), Events::MouseMove, OnMouseMove);
  ConnectThisTo(GetOwner(), Events::KeyDown, OnKeyDown);
  ConnectThisTo(GetOwner(), Events::MouseDragStart, OnMouseDragStart);
  ConnectThisTo(GetOwner(), Events::MouseDragMove, OnMouseDragMove);
  ConnectThisTo(GetOwner(), Events::MouseDragEnd, OnMouseEndDrag);
  ConnectThisTo(GetOwner(), Events::ToolDraw, OnToolDraw);

  ConnectThisTo(GetOwner(), Events::GetToolInfo, OnGetToolInfo);
}

void SpringTools::OnToolDraw(Event* e)
{
  mCurrentSubTool->Draw();
}

void SpringTools::OnLeftMouseDown(ViewportMouseEvent* e)
{
  //if we don't already have a spring system selected then fallback to the selection tool (return false)
  SpringSystem* system = GetSpringSystem();
  if(system == nullptr)
    return;

  //we have to keep track of the mouse being down so that
  //we can try to detect drag logic only while the mouse is down
  mMouseIsDown = true;
  //also keep track of where the mouse started down since we determine
  //when to start a drag based upon how far we've moved from the initial down click
  mMouseDownScreenPosition = e->Position;
  
  //let the sub-tool do whatever it wants for mouse down
  mCurrentSubTool->OnLeftMouseDown(e);
  //we handle this (don't do the selection tool logic)
  e->Handled = true;
}

void SpringTools::OnLeftMouseUp(ViewportMouseEvent* e)
{
  mMouseIsDown = false;

  //if we don't have a system selected then we don't want to run a sub-tool
  SpringSystem* system = GetSpringSystem();
  if(system == NULL)
    return;

  //let the sub-tool handle mouse up
  mCurrentSubTool->OnLeftMouseUp(e);
  e->Handled = true;
}

void SpringTools::OnMouseMove(ViewportMouseEvent* e)
{
  //the mouse isn't down so don't do anything
  if(mMouseIsDown == false)
    return;

  //determine if the mouse has moved far enough to start a drag
  Vec2 newMousePosition = e->Position;
  Vec2 delta = newMousePosition - mMouseDownScreenPosition;
  if(delta.Length() > 5)
  {
    if(MouseCapture* mouseCapture = GetOwner()->has(MouseCapture))
    {
      mouseCapture->Capture(e);
      e->Handled = true;
    }
    //start the manipulation (now StartDrag, MouseMovementDrag, and EndDrag will be called and MouseUp won't)
    //BeginDrag(e);
  }
}

void SpringTools::OnMouseDragStart(ViewportMouseEvent* e)
{
  mCurrentSubTool->OnMouseDragStart(e);
}

void SpringTools::OnMouseDragMove(ViewportMouseEvent* e)
{
  //if we don't have a system selected then we don't want to run a sub-tool
  SpringSystem* system = GetSpringSystem();
  if(system == nullptr)
    return;

  //mark the selected system on the sub-tool (so it knows who to operate on)
  mCurrentSubTool->mSelectedSystem = system->GetOwner();
  //have the sub-tool do drag logic
  mCurrentSubTool->OnMouseDragMove(e);

  e->Handled = true;
}

void SpringTools::OnMouseEndDrag(Event* e)
{
  //since mouse up won't get called we have to make sure to mark the mouse as not being down
  mMouseIsDown = false;
  mCurrentSubTool->OnMouseEndDrag(e);
}

void SpringTools::OnKeyDown(KeyboardEvent* e)
{
  mCurrentSubTool->OnKeyDown(e);
}

SpringSystem* SpringTools::GetSpringSystem()
{
  // Get the current selection
  MetaSelection* selection = Z::gEditor->GetSelection();

  // Try to get the map from the primary selection first...
  Cog* cog = selection->GetPrimaryAs<Cog>();
  if(cog == nullptr)
    return nullptr;

  SpringSystem* springs = cog->has(SpringSystem);

  if(springs != nullptr)
  {
    SetEditingSystem(springs);
    return springs;
  }

  // Check if our last stored selection had the height map...
  springs = mSelectedSystem.has(SpringSystem);
  if (springs != nullptr)
    return springs;

  // Otherwise, we need to try and see if any selected object has it
  springs = selection->FindInSelection<Cog, SpringSystem>();
  if(springs != nullptr)
  {
    SetEditingSystem(springs);
    return springs;
  }
  return nullptr;
}

void SpringTools::SetEditingSystem(SpringSystem* system)
{
  mSelectedSystem = system->GetOwner();
}

SpringSubTools::Enum SpringTools::GetCurrentSubToolType()
{
  return mCurrentSubToolType;
}

void SpringTools::SetCurrentSubToolType(SpringSubTools::Enum toolType)
{
  //update what current type of sub tool we have selected
  mCurrentSubToolType = toolType;

  //update the current selected sub tool pointer
  mCurrentSubTool = mSubTools[(uint)mCurrentSubToolType];
  //also tell it what system to operate on
  mCurrentSubTool->mSelectedSystem = mSelectedSystem;

  //test currently to only show a property view if we are using the anchoring tool
  if(mCurrentSubToolType == SpringSubTools::Anchoring)
  {
    mPropertyView->SetObject(&mProxy, &mPointProxyPropertyInterface);
    mPropertyView->Rebuild();
  }
  else
  {
    mPropertyView->SetObject(Handle());
    mPropertyView->Rebuild();
  }
}

void SpringTools::OnGetToolInfo(ToolUiEvent* e)
{
  Composite* ui = new Composite(e->mParent);
  ui->SetLayout(CreateStackLayout());
  ui->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);

  mPropertyView = new PropertyView(ui);
  mPropertyView->mFixedHeight = true;
  mPropertyView->ActivateAutoUpdate();
  
  mPropertyView->SetObject(&mProxy, &mPointProxyPropertyInterface);
  mPropertyView->Rebuild();

  e->mCustomUi = ui;
  e->mNeedsPropertyGrid = true;
}

}//namespace Zero
