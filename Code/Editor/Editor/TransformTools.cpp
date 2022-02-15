// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// ManipulatorTool Events
namespace Events
{
DefineEvent(ManipulatorToolStart);
DefineEvent(ManipulatorToolModified);
DefineEvent(ManipulatorToolEnd);
} // namespace Events

ZilchDefineType(ManipulatorToolEvent, builder, type)
{
  ZilchBindGetter(OperationQueue);
  ZilchBindGetter(Finished);

  ZilchBindFieldGetter(mGrabLocation);
  ZilchBindFieldGetter(mStartWorldRectangle);
  ZilchBindField(mEndWorldRectangle);
}

ManipulatorToolEvent::ManipulatorToolEvent(ViewportMouseEvent* event) :
    ViewportMouseEvent(*event),
    mOperationQueue(nullptr),
    mStartWorldRectangle(Rectangle::cZero),
    mEndWorldRectangle(Rectangle::cZero)
{
}

OperationQueue* ManipulatorToolEvent::GetOperationQueue()
{
  return mOperationQueue;
}

bool ManipulatorToolEvent::GetFinished()
{
  return mOperationQueue.IsNotNull();
}

DeclareBitField4(DirectionFlags, Left, Right, Top, Bottom);

static const uint cMiddlePoint = 8;
static const uint cPointCount = 9;

static const uint cGrabPoints[] = {
    DirectionFlags::Left,
    DirectionFlags::Left | DirectionFlags::Top,
    DirectionFlags::Top,
    DirectionFlags::Top | DirectionFlags::Right,
    DirectionFlags::Right,
    DirectionFlags::Right | DirectionFlags::Bottom,
    DirectionFlags::Bottom,
    DirectionFlags::Bottom | DirectionFlags::Left,
    0,
};

static const Location::Enum cLocations[] = {
    Location::CenterLeft,
    Location::TopLeft,
    Location::TopCenter,
    Location::TopRight,
    Location::CenterRight,
    Location::BottomRight,
    Location::BottomCenter,
    Location::BottomLeft,
    Location::Center,
};

static const float cGripSize = 0.07f;
static const float cCornerGripScalar = 1.3f;
static const float cToolEpsilon = 0.001f;

// Due to 'sMinimumBoundingSize' in Graphical.cpp, the size epsilon here must
// be the same magnitude. This is the size considered to be equal to zero.
// Addtionally: add in the standard tool epsilon to capture float-rounding
// errors.
//
// This size epsilon will be used for detecting a zero size object. If an object
// is zero size then it's scale change needs to be special-cased. [ie, cannot
// scale 0].
static const float cSizeEpsilon = cMinimumBoundingSize + cToolEpsilon;

ZilchDefineType(ManipulatorTool, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  type->AddAttribute(ObjectAttributes::cTool);

  ZeroBindEvent(Events::ManipulatorToolStart, ManipulatorToolEvent);
  ZeroBindEvent(Events::ManipulatorToolModified, ManipulatorToolEvent);
  ZeroBindEvent(Events::ManipulatorToolEnd, ManipulatorToolEvent);

  ZeroBindDependency(MouseCapture);

  ZilchBindFieldProperty(mSizeBoxCollider);
  ZilchBindFieldProperty(mDuplicateOnCtrlDrag);
  ZilchBindGetterSetterProperty(IncludeMode);
  ZilchBindFieldProperty(mGrabMode);
  ZilchBindFieldProperty(mSnapping);
  ZilchBindGetterSetterProperty(SnapDistance);

  ZilchBindFieldProperty(mToolColor)->ZeroSetPropertyGroup("Colors");
  ZilchBindFieldProperty(mHoverColor)->ZeroSetPropertyGroup("Colors");
}

ManipulatorTool::ManipulatorTool()
{
  mSnapping = false;
  mSnapDistance = 0.5f;

  mSizeBoxCollider = true;
  mDuplicateOnCtrlDrag = true;

  mDuplicateCorrectIndex = unsigned(-1);

  mIncludeMode = IncludeMode::Children;

  mGizmoMode = GizmoMode::Inactive;
  mGrabMode = GizmoGrab::Hold;

  mSelectedPoint = -1;

  mValidLocation = false;
  mLocation = Location::Center;

  mToolColor = FloatColorRGBA(255, 122, 0, 255);
  mHoverColor = FloatColorRGBA(255, 199, 0, 255);
}

void ManipulatorTool::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSnapping, false);
  SerializeNameDefault(mSnapDistance, 0.5f);

  SerializeNameDefault(mSizeBoxCollider, true);
}

void ManipulatorTool::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::ToolActivate, OnToolActivate);
  ConnectThisTo(GetOwner(), Events::ToolDeactivate, OnToolDeactivate);

  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);

  ConnectThisTo(GetOwner(), Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(GetOwner(), Events::LeftMouseUp, OnLeftMouseUp);
  ConnectThisTo(GetOwner(), Events::MouseMove, OnMouseMove);

  /// Drag Events
  ConnectThisTo(GetOwner(), Events::MouseDragStart, OnMouseDragStart);
  ConnectThisTo(GetOwner(), Events::MouseDragEnd, OnMouseDragEnd);
  ConnectThisTo(GetOwner(), Events::MouseDragMove, OnMouseDragMove);

  ConnectThisTo(GetOwner(), Events::ToolDraw, OnToolDraw);
}

bool ManipulatorTool::Active()
{
  return mGizmoMode != GizmoMode::Inactive;
}

IncludeMode::Enum ManipulatorTool::GetIncludeMode()
{
  return mIncludeMode;
}

void ManipulatorTool::SetIncludeMode(IncludeMode::Enum mode)
{
  mIncludeMode = mode;

  UpdateRectAndBasis();
}

float ManipulatorTool::GetSnapDistance()
{
  return mSnapDistance;
}

void ManipulatorTool::SetSnapDistance(float distance)
{
  if (distance < 0.001f)
    distance = 0.001f;

  mSnapDistance = distance;
}

bool ManipulatorTool::CanManipulate()
{
  return mValidLocation && ActiveWithSelection();
}

bool ManipulatorTool::ActiveWithSelection()
{
  return mObjects.Size() > 0 && Active();
}

bool ManipulatorTool::CheckMouseManipulation(ViewportMouseEvent* event)
{
  if (CanManipulate())
  {
    MouseCapture* capture = GetOwner()->has(MouseCapture);
    capture->Capture(event);
    return true;
  }

  return false;
}

void ManipulatorTool::ZeroOutManipulator(Vec3Param center)
{
  mGizmoMode = GizmoMode::Inactive;

  mStartCenter3D = center;
  mActiveRect.SetCenter(ToVector2(center));
  mActiveRect.SetSize(Location::Center, Vec2::cZero);
}

bool ManipulatorTool::PrioritizeUiWidget(Array<Handle>::range& range)
{
  Aabb aabb;
  forRange (Handle& object, range.All())
  {
    Cog* cog = object.Get<Cog*>();
    if (cog == nullptr)
      continue;

    UiWidget* ui = cog->has(UiWidget);
    if (ui == nullptr)
      continue;

    Aabb subAabb;
    ExpandAabb(cog, subAabb, IncludeMode::OnlyRoot);

    aabb.Combine(subAabb);
  }

  if (!aabb.Valid())
  {
    ZeroOutManipulator(Vec3::cZero);
    return false;
  }

  SetToolRect(aabb);
  return true;
}

bool ManipulatorTool::CheckAlignment(Array<Handle>::range& range, Vec3Param toolNormal)
{
  // If the tool normal is close enough to z-axis alignment, then
  // return false so that a WorldSpace rect is computed [ie, no tool
  // orientation].
  if (Math::Abs(1 - toolNormal.Dot(Vec3::cZAxis)) < cToolEpsilon)
    return false;

  // Check if all objects have the same XY-plane alignment.
  forRange (Handle handle, range)
  {
    MetaTransform* mt = handle.StoredType->HasInherited<MetaTransform>();
    if (mt == nullptr)
      continue;

    MetaTransformInstance t = mt->GetInstance(handle);
    if (t.IsNull())
      continue;

    Vec3 objectNormal = Math::Multiply(t.GetWorldRotation(), Vec3::cZAxis).Normalized();
    float dot = toolNormal.Dot(objectNormal);

    // Normals aren't close enough to being aligned.
    if (dot * dot < 0.99f)
      return false;
  }

  return true;
}

void ManipulatorTool::ComputeWorldToolRect(Array<Handle>::range& range,
                                           Vec3Param toolPosition,
                                           bool includeChildren,
                                           bool childPass)
{
  Aabb aabb;

  Array<Handle>::range objects = range;

  // World aabb encapsulating all other objects in the selection.
  forRange (Handle handle, objects)
  {
    Aabb subAabb;
    // MetaTransform validity is handled internally in 'ExpandAabb'.
    ExpandAabb(handle, subAabb, IncludeMode::OnlyRoot, true, false, childPass);

    aabb.Combine(subAabb);
  }

  if (includeChildren)
  {
    forRange (Handle handle, range)
    {
      Aabb subAabb;
      // MetaTransform validity is handled internally in 'ExpandAabb'.
      ExpandAabbChildrenOnly(handle, subAabb, true, true);

      aabb.Combine(subAabb);
    }
  }

  // Couldn't calculate an aabb for the ManipulatorTool to work with.
  if (!aabb.Valid())
  {
    ZeroOutManipulator(toolPosition);
    return;
  }

  mObbBasis = Quat::cIdentity;
  SetToolRect(aabb);
}

void ManipulatorTool::ComputeLocalToolRect(Array<Handle>::range& range,
                                           Vec3Param toolNormal,
                                           Vec3Param pointOnToolPlane,
                                           Aabb* aabbOut,
                                           bool includeChildren,
                                           bool childPass)
{
  bool valid = false;
  Array<Aabb> obbs;
  obbs.Reserve(range.Size());

  Vec2 toolMin(-Math::PositiveMax());
  Vec2 toolMax(-Math::PositiveMax());

  Vec3 toolX = Math::Multiply(mObbBasis, Vec3::cXAxis);
  Vec3 toolY = Math::Multiply(mObbBasis, Vec3::cYAxis);

  Vec3 v[4];

  float* corners[4] = {&toolMin.x, &toolMin.y, &toolMax.x, &toolMax.y};
  Vec3 axes[4] = {-toolX, -toolY, toolX, toolY};

  forRange (Handle handle, range)
  {
    MetaTransform* mt = handle.StoredType->HasInherited<MetaTransform>();
    if (mt == nullptr)
      continue;

    MetaTransformInstance t = mt->GetInstance(handle);
    if (t.IsNull())
      continue;

    Aabb& subObb = obbs.PushBack();
    ExpandAabb(handle, subObb, IncludeMode::OnlyRoot, false, false, childPass);

    // If the parent's aabb isn't valid, then no need to expand children aabbs.
    if (!(valid |= subObb.Valid()))
      continue;

    if (includeChildren)
      ExpandAabbChildrenOnly(handle, subObb, false, true);

    Vec3 center = t.GetWorldTranslation();
    Vec3 extents = subObb.GetExtents();

    // Proper 'subObb' with volume [ie, not just a point].
    if (extents.Dot(extents) > cToolEpsilon)
    {
      Mat4 m = BuildTransform(t.GetLocalTranslation(), t.GetLocalRotation(), t.GetLocalScale());
      Mat4 pm = t.GetParentWorldMatrix();

      center = Math::TransformPoint(Multiply(pm, m), subObb.GetCenter());

      // Extents in parent space.
      extents = subObb.GetExtents() * t.GetLocalScale();
      extents = Math::Multiply(t.GetLocalRotation(), extents);
      subObb.SetExtents(extents);

      // Half-extents in world space.
      extents = Math::MultiplyNormal(pm, 0.5f * extents);
    }

    // Apply translation, note: 'subObb' now defines a world space obb.
    subObb.mMin = center - extents;
    subObb.mMax = center + extents;

    Vec3 minMax[2] = {subObb.mMin, subObb.mMax};

    // Save the points farthest in distance along the +/- tool X,Y axes.
    // Then project the associated points onto the tool plane.
    for (int i = 0; i < 4; ++i)
    {
      Vec3 p = minMax[i / 2];
      float d = p.Dot(axes[i]);
      if (d > *(corners[i]))
      {
        *(corners[i]) = d;
        v[i] = p - toolNormal * toolNormal.Dot(p - pointOnToolPlane);
      }
    }
  }

  aabbOut->SetInvalid();

  // No need to finish aabb computation if it isn't valid.
  if (valid)
  {
    Vec3 minSpan = v[1] - v[0];
    Vec3 maxSpan = v[3] - v[2];

    // Align the point, containing the minX obb data, along the tool-yAxis to
    // the point containing the minY obb data. [ie, v[0] will be the obb min].
    Vec3 delta = toolY * toolY.Dot(minSpan);
    v[0] += delta;

    // Align the point, containing the maxX obb data, along the tool-yAxis to
    // the point containing the maxY obb data. [ie, v[2] will be the obb max].
    delta = toolY * toolY.Dot(maxSpan);
    v[2] += delta;

    aabbOut->mMin = v[0];
    aabbOut->mMax = v[2];

    Vec3 origin = aabbOut->GetCenter();
    aabbOut->SetCenter(Vec3::cZero);

    // Undo rotation [ie, make an aabb from the obb].
    Quat toLocal = mObbBasis.Inverted();
    aabbOut->mMin = Math::Multiply(toLocal, aabbOut->mMin);
    aabbOut->mMax = Math::Multiply(toLocal, aabbOut->mMax);

    // Shouldn't have any thickness along the z-axis.
    aabbOut->mMin.z = 0.0f;
    aabbOut->mMax.z = 0.0f;

    // Return the tool aabb to its proper location.
    aabbOut->SetCenter(origin);
  }
}

void ManipulatorTool::ExtractPrimary(Array<Handle>::range& range, Handle& primaryOut)
{
  // Find the first object in the selection with a Transform.
  forRange (Handle handle, range)
  {
    MetaTransform* metaTransform = handle.StoredType->HasInherited<MetaTransform>();
    if (metaTransform == nullptr || metaTransform->GetInstance(handle).IsNull())
      continue;

    primaryOut = handle;
    break;
  }
}

void ManipulatorTool::CompileChildObjects(Cog* parent, Array<Handle>& children)
{
  if (parent == nullptr)
    return;

  forRange (Cog& child, parent->GetChildren())
  {
    CompileChildObjects(&child, children);
    children.PushBack(child);
  }
}

void ManipulatorTool::UpdateSelectedObjects()
{
  mObjects.Clear();

  MetaSelection* selection = Z::gEditor->GetSelection();

  bool transformFound = false;
  forRange (Handle handle, selection->All())
  {
    MetaTransform* transform = handle.StoredType->HasInherited<MetaTransform>();
    if (transform == nullptr)
      continue;

    MetaTransformInstance transformInstance = transform->GetInstance(handle);
    if (transformInstance.IsNotNull())
    {
      mObjects.PushBack(handle);

      if (Object* object = handle.Get<Object*>())
      {
        if (EventDispatcher* dispatcher = object->GetDispatcher())
        {
          // Don't duplicate connections.
          if (!dispatcher->IsConnected(Events::ComponentsModified, this))
            ConnectThisTo(object, Events::ComponentsModified, OnComponentsChanged);
        }
      }
    }
  }

  UpdateRectAndBasis();
}

void ManipulatorTool::UpdateRectAndBasis()
{
  if (mObjects.Empty())
    return;

  Array<Handle> metaObjects;
  FilterChildrenAndProtected(mObjects, metaObjects);

  if (metaObjects.Empty())
    return;

  Handle primary;
  Array<Handle>::range range = metaObjects.All();
  ExtractPrimary(range, primary);

  ErrorIf(primary.IsNull(),
          "ManipulationTool requires at least one object in the selection to "
          "have a Trasform");

  MetaTransform* metaTransform = primary.StoredType->HasInherited<MetaTransform>();
  MetaTransformInstance transform = metaTransform->GetInstance(primary);

  Quat primaryBasis = transform.GetWorldRotation();
  Vec3 toolNormal = Math::Multiply(primaryBasis, Vec3::cZAxis);
  Vec3 pp = transform.GetWorldTranslation();

  mObbBasis = Quat::cIdentity;

  // UiWidget has priority over the selection's aabb computation.
  Array<Handle>::range objects = metaObjects.All();
  if (PrioritizeUiWidget(objects) == true)
    return;

  objects = range;
  if (CheckAlignment(objects, toolNormal) == false)
  {
    // Object rotations aren't aligned, so compute a world axes aligned
    // tool-aabb.
    objects = metaObjects.All();
    ComputeWorldToolRect(objects, pp, mIncludeMode == IncludeMode::Children);
    return;
  }

  Aabb aabb;

  // Computing a local tool rect requires the basis to be set.
  mObbBasis = primaryBasis;

  objects = metaObjects.All();
  ComputeLocalToolRect(objects, toolNormal, pp, &aabb, mIncludeMode == IncludeMode::Children);

  // Couldn't calculate an aabb, try to compute one from the children of
  // the objects in the selection.
  if (!aabb.Valid())
  {
    // Gather up all the children.
    Array<Handle> children;
    forRange (Handle handle, metaObjects.All())
    {
      CompileChildObjects(handle.Get<Cog*>(), children);
    }

    objects = children.All();
    ExtractPrimary(objects, primary);

    ErrorIf(primary.IsNull(),
            "ManipulationTool requires at least one object in the selection to "
            "have a Trasform");

    metaTransform = primary.StoredType->HasInherited<MetaTransform>();
    transform = metaTransform->GetInstance(primary);

    // Computing a local tool rect requires the basis to be set, properly.
    mObbBasis = primaryBasis = transform.GetWorldRotation();
    toolNormal = Math::Multiply(primaryBasis, Vec3::cZAxis);
    pp = transform.GetWorldTranslation();

    if (CheckAlignment(objects, toolNormal) == false)
    {
      // Children rotations aren't aligned, so compute a world axes aligned
      // tool-aabb.
      objects = children.All();
      ComputeWorldToolRect(objects, pp, false, true);
      return;
    }

    // Children are all aligned so try to compute an oriented rect for the tool.
    objects = children.All();
    ComputeLocalToolRect(objects, toolNormal, pp, &aabb, false, true);
  }

  // Couldn't calculate an aabb for the ManipulatorTool to work with.
  if (!aabb.Valid())
  {
    ZeroOutManipulator(pp);
    return;
  }

  SetToolRect(aabb);
}

void ManipulatorTool::SetToolRect(const Aabb& aabb)
{
  mStartCenter3D = aabb.GetCenter();

  mActiveRect.SetCenter(ToVector2(mStartCenter3D));
  mActiveRect.SetSize(Location::Center, ToVector2(aabb.GetExtents()));

  mGizmoMode = GizmoMode::Active;
}

void ManipulatorTool::OnToolActivate(Event*)
{
  mGizmoMode = GizmoMode::Active;

  MetaSelection* selection = Z::gEditor->mSelection;
  ConnectThisTo(selection, Events::SelectionChanged, OnSelectionChanged);
  ConnectThisTo(selection, Events::SelectionFinal, OnFinalSelectionChanged);

  UpdateSelectedObjects();
}

void ManipulatorTool::OnToolDeactivate(Event*)
{
  mGizmoMode = GizmoMode::Inactive;

  MetaSelection* selection = Z::gEditor->GetSelection();

  selection->GetDispatcher()->DisconnectEvent(Events::SelectionChanged, this);
  selection->GetDispatcher()->DisconnectEvent(Events::SelectionFinal, this);

  // Remove all objects connections to this event.  This disconnect should
  // hit all objects whether they are part of the current selection or not.
  GetReceiver()->Disconnect(Events::ComponentsModified);

  mObjects.Clear();
  mTransformingObjects.Clear();
}

void ManipulatorTool::OnFrameUpdate(UpdateEvent*)
{
  if (mGizmoMode == GizmoMode::Transforming)
    return;

  // Covers the case when ManipulatorTool changes are undone/redone.
  UpdateRectAndBasis();
}

void ManipulatorTool::OnComponentsChanged(Event*)
{
  UpdateSelectedObjects();
}

void ManipulatorTool::OnSelectionChanged(Event*)
{
  UpdateSelectedObjects();
}

void ManipulatorTool::OnFinalSelectionChanged(Event*)
{
  UpdateSelectedObjects();
}

void ManipulatorTool::OnLeftMouseDown(ViewportMouseEvent* event)
{
  if (mGrabMode == GizmoGrab::Hold)
    event->Handled = CheckMouseManipulation(event);
  else
    event->Handled = CanManipulate();
}

void ManipulatorTool::OnLeftMouseUp(ViewportMouseEvent* event)
{
  if (mGrabMode == GizmoGrab::Toggle)
    event->Handled = CheckMouseManipulation(event);
}

void ManipulatorTool::OnMouseMove(ViewportMouseEvent* event)
{
  if (ActiveWithSelection())
    TestMouseMove(event);
}

void ManipulatorTool::TestMouseMove(ViewportMouseEvent* event)
{
  mValidLocation = false;
  mSelectedPoint = -1;

  Camera* camera = event->GetViewport()->GetCamera();
  Ray mouseRay = event->mWorldRay;

  Vec2 halfSize = mActiveRect.GetSize() * 0.5f;

  Vec3 moveX = Vec3(halfSize.x, 0, 0);
  Vec3 moveY = Vec3(0, halfSize.y, 0);

  moveX = Math::Multiply(mObbBasis, moveX);
  moveY = Math::Multiply(mObbBasis, moveY);

  Vec3 center = mStartCenter3D;
  Matrix3 obbBasis = ToMatrix3(mObbBasis);

  Vec3 middles[] = {center - moveX, // Center Left
                    center + moveY, // Center Top
                    center + moveX, // Center Right
                    center - moveY, // Center Bottom
                    center};

  for (uint i = 0; i <= 4; ++i)
  {
    float scaledGripSize = GizmoHelpers::GetViewScale(camera, middles[i]) * cGripSize;
    Vec3 boxSize(scaledGripSize, scaledGripSize, scaledGripSize);

    Intersection::Type result = Intersection::RayObb(mouseRay.Start, mouseRay.Direction, middles[i], boxSize, obbBasis);

    if (result != Intersection::None)
    {
      mSelectedPoint = 2 * i;
      mWorldGrabPoint = middles[i];
      mLocation = cLocations[2 * i];
      mValidLocation = true;
    }
  }

  Vec3 corner[] = {
      center - moveX + moveY, // Top Left
      center + moveX + moveY, // Top Right
      center + moveX - moveY, // Bottom Right
      center - moveX - moveY  // Bottom Left
  };

  for (uint i = 0; i < 4; ++i)
  {
    // Corner grips are double size.
    float scaledGripSize = cCornerGripScalar * GizmoHelpers::GetViewScale(camera, corner[i]) * cGripSize;
    Vec3 boxSize(scaledGripSize, scaledGripSize, scaledGripSize);

    Intersection::Type result = Intersection::RayObb(mouseRay.Start, mouseRay.Direction, corner[i], boxSize, obbBasis);

    if (result != Intersection::None)
    {
      mSelectedPoint = 2 * i + 1;
      mWorldGrabPoint = corner[i];
      mLocation = cLocations[2 * i + 1];
      mValidLocation = true;
    }
  }
}

void ManipulatorTool::DuplicationObjects(Array<Handle>& metaObjects)
{
  mNewObjects.Clear();
  mObjects.Clear();

  MetaSelection* selection = Z::gEditor->GetSelection();
  selection->Clear(SendsEvents::False);

  forRange (Handle object, metaObjects.All())
  {
    Cog* cog = object.Get<Cog*>();
    if (cog && CogSerialization::ShouldSave(*cog))
    {
      Cog* copy = cog->Clone();
      mNewObjects.Insert(copy);

      // new object will go directly after the object it was copied from
      mDuplicateCorrectIndex = cog->GetHierarchyIndex() + 1;
      copy->PlaceInHierarchy(mDuplicateCorrectIndex);
    }
    else // currently unnecessary to check for protection on non-cogs
    {
      mNewObjects.Insert(object);
    }
  }

  metaObjects.Clear();

  forRange (Handle handle, mNewObjects.All())
  {
    if (handle.IsNull())
      continue;

    // Valid manipulation objects must have a Transform.
    MetaTransform* metaTransform = handle.StoredType->HasInherited<MetaTransform>();
    if (!metaTransform)
      continue;

    if (Object* object = handle.Get<Object*>())
    {
      if (EventDispatcher* dispatcher = object->GetDispatcher())
      {
        // Don't duplicate connections.
        if (!dispatcher->IsConnected(Events::ComponentsModified, this))
          ConnectThisTo(object, Events::ComponentsModified, OnComponentsChanged);
      }
    }

    mObjects.PushBack(handle);
    metaObjects.PushBack(handle);

    selection->Add(handle);
  }

  UpdateRectAndBasis();

  // Signal that the objects were duplicated
  Event eventToSend;
  GetOwner()->DispatchEvent(Events::GizmoObjectsDuplicated, &eventToSend);
  GetOwner()->DispatchUp(Events::GizmoObjectsDuplicated, &eventToSend);
}

void ManipulatorTool::AddTransformingObject(Handle target, ManipulatorToolEvent& eventToSend)
{
  if (Object* object = target.Get<Object*>())
  {
    if (EventDispatcher* dispatcher = object->GetDispatcher())
      dispatcher->Dispatch(Events::ManipulatorToolStart, &eventToSend);
  }

  TransformingObject data;
  data.MetaObject = target;

  MetaTransform* metaTransform = target.StoredType->HasInherited<MetaTransform>();
  ErrorIf(metaTransform == nullptr, "No MetaTransform on object being transformed.");

  MetaTransformInstance transform = metaTransform->GetInstance(target);

  if (transform.mLocalTranslation != nullptr)
  {
    // local
    data.StartTranslation = transform.GetLocalTranslation();
    data.EndTranslation = data.StartTranslation;

    // world
    data.StartWorldTranslation = transform.GetWorldTranslation();
  }
  if (transform.mLocalRotation != nullptr)
  {
    data.StartRotation = transform.GetLocalRotation();
    data.EndRotation = data.StartRotation;
  }
  if (transform.mLocalScale != nullptr)
  {
    data.StartScale = transform.GetLocalScale();
    data.EndScale = data.StartScale;
  }

  if (Cog* cog = target.Get<Cog*>())
  {
    if (Area* area = cog->has(Area))
    {
      data.StartSize = area->mSize;
      data.EndSize = area->mSize;

      if (BoxCollider* collider = cog->has(BoxCollider))
      {
        data.StartColliderSize = collider->GetSize();
        data.StartColliderOffset = collider->GetOffset();
      }
    }
  }
  else
  {
    data.StartSize = Vec2::cZero;
    data.EndSize = Vec2::cZero;
  }

  mTransformingObjects.PushBack(data);
}

void ManipulatorTool::OnMouseDragStart(ViewportMouseEvent* event)
{
  mStartRect = mActiveRect;
  mMouseDragStart = event->Position;

  if (!mValidLocation)
    return;

  Array<Handle> metaObjects;
  FilterChildrenAndProtected(mObjects, metaObjects);

  if (metaObjects.Empty())
    return;

  if (mLocation == Location::Center && event->CtrlPressed && mDuplicateOnCtrlDrag)
    DuplicationObjects(metaObjects);

  mTransformingObjects.Clear();

  ManipulatorToolEvent eventToSend(event);
  eventToSend.mGrabLocation = mLocation;
  eventToSend.mStartWorldRectangle = mStartRect;
  eventToSend.mEndWorldRectangle = eventToSend.mStartWorldRectangle;

  forRange (Handle target, metaObjects.All())
    AddTransformingObject(target, eventToSend);
}

void ManipulatorTool::OnMouseDragMove(ViewportMouseEvent* event)
{
  // Gizmo is now moving.
  mGizmoMode = GizmoMode::Transforming;
  mActiveRect = mStartRect;

  Vec3 toolWorldNormal = Math::Multiply(mObbBasis, Vec3::cZAxis);
  toolWorldNormal.Normalize();

  // Compute tool plane movement/drag.
  Viewport* viewport = event->GetViewport();
  Vec3 grabPosition = viewport->ScreenToWorldPlane(mMouseDragStart, toolWorldNormal, mWorldGrabPoint);
  Vec3 dragPosition = viewport->ScreenToWorldPlane(event->Position, toolWorldNormal, mWorldGrabPoint);
  Vec3 movement = dragPosition - grabPosition;

  // Movement in world to local tool 2D space.
  Quat inverseObbBasis = mObbBasis.Inverted();
  movement = Math::Multiply(inverseObbBasis, movement);
  Vec2 move2D(movement.x, movement.y);

  // Shift key modifies the snapping flag temporarily
  bool nonShiftModified = (mSnapping && Keyboard::Instance->KeyIsUp(Keys::Shift));
  bool tempSnappingOn = (!mSnapping && Keyboard::Instance->KeyIsDown(Keys::Shift));

  if ((nonShiftModified || tempSnappingOn))
    move2D = Snap(move2D, mSnapDistance);

  Vec2 startSize = mStartRect.GetSize();

  // Translation only.
  if (mSelectedPoint == cMiddlePoint)
  {
    mActiveRect.SetCenter(mActiveRect.GetCenter() + move2D);
  }
  else // Sizing and Translation.
  {
    uint grabFlags = cGrabPoints[mSelectedPoint];

    // If true:  1 * value = value [ie, use movement on axis].
    // If false: 0 * value =     0 [ie, no movement on axis].
    Vec2 moveOnAxis;
    moveOnAxis.x = (bool)(grabFlags & (DirectionFlags::Left | DirectionFlags::Right));
    moveOnAxis.y = (bool)(grabFlags & (DirectionFlags::Top | DirectionFlags::Bottom));

    move2D.x = moveOnAxis.x * move2D.x;
    move2D.y = moveOnAxis.y * move2D.y;

    // Maintain aspect ratio.
    if (event->CtrlPressed && Keyboard::Instance->KeyIsDown(Keys::A) && moveOnAxis.x && moveOnAxis.y)
    {
      Vec2 grab2D = mStartRect.GetLocation(mLocation);
      Vec2 axis = grab2D - mStartRect.GetCenter();
      float length = axis.AttemptNormalize();

      // Prevent a "zero-size" state.
      if (length >= Math::Epsilon() * Math::Epsilon())
        move2D = axis * axis.Dot(move2D);
    }

    // Moving left is negative on the x-axis, but the size needs to increase
    // [ie, move the min farther from the max]. So, flip the sign of the move.
    if (grabFlags & DirectionFlags::Left)
    {
      // Prevent negative size.
      move2D.x = Math::Min(move2D.x, startSize.x);
      mActiveRect.SetSize(Location::CenterRight, startSize - move2D);
    }
    else
    {
      // Prevent negative size.
      move2D.x = Math::Max(move2D.x, -startSize.x);
      mActiveRect.SetSize(Location::CenterLeft, startSize + move2D);
    }

    // Moving down is negative on the y-axis, but the size needs to increase
    // [ie, move the max farther from the min]. So, flip the sign of the move.
    if (grabFlags & DirectionFlags::Bottom)
    {
      // Prevent negative size.
      move2D.y = Math::Min(move2D.y, startSize.y);
      mActiveRect.SetSize(Location::TopCenter, startSize - move2D);
    }
    else
    {
      // Prevent negative size.
      move2D.y = Math::Max(move2D.y, -startSize.y);
      mActiveRect.SetSize(Location::BottomCenter, startSize + move2D);
    }
  }

  Vec2 startCenter = mStartRect.GetCenter();

  // Old Center to new BottomLeft in local tool 2D-space [ie, no Z-component].
  mActiveOrigin3D = Vec3(mActiveRect.GetBottomLeft(), 0) - Vec3(startCenter, 0);
  // Current BottomLeft of tool rect in world space.
  mActiveOrigin3D = mStartCenter3D + Math::Multiply(mObbBasis, mActiveOrigin3D);

  Vec3 oppositeGrab = Vec3(mStartRect.GetLocation(Location::GetOpposite(mLocation)), 0);
  oppositeGrab -= Vec3(startCenter, 0);
  oppositeGrab = mStartCenter3D + Math::Multiply(mObbBasis, oppositeGrab);

  Property* areaSizeProperty = ZilchTypeId(Area)->GetProperty("Size");
  Property* colliderOffsetProperty = ZilchTypeId(BoxCollider)->GetProperty("Offset");
  Property* colliderSizeProperty = ZilchTypeId(BoxCollider)->GetProperty("Size");

  ManipulatorToolEvent eventToSend(event);
  eventToSend.mGrabLocation = mLocation;
  eventToSend.mStartWorldRectangle = mStartRect;
  eventToSend.mEndWorldRectangle = mActiveRect;

  // Compute updated transforms for the objects the tool is affecting.
  forRange (TransformingObject& target, mTransformingObjects.All())
  {
    Handle handle = target.MetaObject;
    if (handle.IsNull())
      continue;

    MetaTransform* metaTransform = handle.StoredType->HasInherited<MetaTransform>();
    if (metaTransform == nullptr)
      continue;

    MetaTransformInstance transform = metaTransform->GetInstance(handle);
    if (transform.IsNull())
      continue;

    // Reset for each object incase 'EndWorldRectangle' was modified, or
    // 'HandledEventScript' was set.
    eventToSend.mEndWorldRectangle = mActiveRect;
    eventToSend.HandledEventScript = false;

    EventDispatcher* dispatcher = nullptr;
    Object* object = handle.Get<Object*>();
    if (object != nullptr)
      dispatcher = object->GetDispatcher();

    if (dispatcher != nullptr)
      dispatcher->Dispatch(Events::ManipulatorToolModified, &eventToSend);

    if (eventToSend.Handled || eventToSend.HandledEventScript)
      continue;

    Vec2 returnedSize = eventToSend.mEndWorldRectangle.GetSize();
    Vec2 returnedCenter = eventToSend.mEndWorldRectangle.GetCenter();

    // Negative size not allowed
    returnedSize = Math::Max(returnedSize, Vec2::cZero);

    Vec3 scaleChange(1);
    scaleChange.x = (startSize.x <= cSizeEpsilon) ? 1 : returnedSize.x / startSize.x;
    scaleChange.y = (startSize.y <= cSizeEpsilon) ? 1 : returnedSize.y / startSize.y;

    Vec3 newTranslation;
    if (mSelectedPoint == cMiddlePoint)
    {
      // Translation change in world.
      newTranslation = Math::Multiply(mObbBasis, Vec3(returnedCenter - startCenter, 0));
      newTranslation = target.StartWorldTranslation + newTranslation;
    }
    else
    {
      // World space.
      Vec3 offset = target.StartWorldTranslation - oppositeGrab;
      // MUST apply scaling information in tool local 2D space.
      offset = Math::Multiply(inverseObbBasis, offset);
      offset *= scaleChange;
      // Back to world.
      offset = Math::Multiply(mObbBasis, offset);

      newTranslation = oppositeGrab + offset;
    }

    Mat4 inverseMatrix = transform.GetParentWorldMatrix();
    if (transform.mParentWorldMatrix != nullptr)
    {
      inverseMatrix.Invert();

      // World to object's local space [ie, parent-space].
      newTranslation = Math::TransformPoint(inverseMatrix, newTranslation);
    }

    // Apply local translation.
    target.EndTranslation = newTranslation;
    transform.SetLocalTranslation(target.EndTranslation);

    if (dispatcher != nullptr)
    {
      PropertyEvent propertyEvent(
          transform.mInstance, transform.mLocalTranslation, target.StartTranslation, target.EndTranslation);
      dispatcher->Dispatch(Events::PropertyModifiedIntermediate, &propertyEvent);
    }

    if (Cog* cog = handle.Get<Cog*>())
    {
      // Only resize the area if children are not affected by manipulation
      bool affectRootOnly = (mIncludeMode == IncludeMode::OnlyRoot || cog->GetChildren().Empty());

      Area* area = cog->has(Area);
      if (area != nullptr && affectRootOnly)
      {
        target.EndSize = target.StartSize * ToVector2(scaleChange);

        // Recover from zero size if the start size was zero.
        //
        // NOTE:
        //   (startSize.x < cSizeEpsilon) == 0 --> 0 * returnedSize + EndSize =
        //   EndSize (startSize.x < cSizeEpsilon) == 1 --> 1 * returnedSize + ~0
        //   = returnedSize
        // target.EndSize.x = (startSize.x <= cSizeEpsilon) * returnedSize.x +
        // target.EndSize.x; target.EndSize.y = (startSize.y <= cSizeEpsilon) *
        // returnedSize.y + target.EndSize.y;

        area->SetSize(target.EndSize);

        if (dispatcher != nullptr)
        {
          PropertyEvent propertyEvent(area, areaSizeProperty, target.StartSize, target.EndSize);
          dispatcher->Dispatch(Events::PropertyModifiedIntermediate, &propertyEvent);
        }

        // No need to update an object's BoxCollider if there isn't one, or if
        // that option isn't enabled.
        BoxCollider* collider = cog->has(BoxCollider);
        if (collider != nullptr && mSizeBoxCollider)
        {
          Vec3 endOffset(area->OffsetOfOffset(Location::Center) * area->GetSize());
          collider->SetOffset(endOffset);

          Vec3 endSize(target.EndSize.x, target.EndSize.y, target.StartColliderSize.z);
          collider->SetSize(endSize);

          if (dispatcher != nullptr)
          {
            PropertyEvent propertyEvent1(collider, colliderOffsetProperty, target.StartColliderOffset, endOffset);
            dispatcher->Dispatch(Events::PropertyModifiedIntermediate, &propertyEvent1);

            PropertyEvent propertyEvent2(collider, colliderSizeProperty, target.StartColliderSize, endSize);
            dispatcher->Dispatch(Events::PropertyModifiedIntermediate, &propertyEvent2);
          }
        }

        continue;
      }
    }

    // Scale is always applied in world space, and only on the X & Y axes.
    target.EndScale = target.StartScale * scaleChange;
    transform.SetLocalScale(target.EndScale);

    if (dispatcher != nullptr)
    {
      PropertyEvent propertyEvent(transform.mInstance, transform.mLocalScale, target.StartScale, target.EndScale);
      dispatcher->Dispatch(Events::PropertyModifiedIntermediate, &propertyEvent);
    }
  }

  event->Handled = true;
}

void ManipulatorTool::OnMouseDragEnd(ViewportMouseEvent* event)
{
  Property* areaSizeProperty = ZilchTypeId(Area)->GetProperty("Size");

  Property* colliderOffsetProperty = ZilchTypeId(BoxCollider)->GetProperty("Offset");
  Property* colliderSizeProperty = ZilchTypeId(BoxCollider)->GetProperty("Size");

  OperationQueue* queue = Z::gEditor->GetOperationQueue();

  ManipulatorToolEvent eventToSend(event);
  eventToSend.mGrabLocation = mLocation;
  eventToSend.mOperationQueue = queue;
  eventToSend.mStartWorldRectangle = mStartRect;
  eventToSend.mEndWorldRectangle = mActiveRect;

  queue->BeginBatch();
  queue->SetActiveBatchName("MultiObject Manipulation");

  forRange (TransformingObject& target, mTransformingObjects.All())
  {
    Handle handle = target.MetaObject;
    if (handle.IsNull())
      continue;

    if (Object* object = handle.Get<Object*>())
    {
      if (EventDispatcher* dispatcher = object->GetDispatcher())
        dispatcher->Dispatch(Events::ManipulatorToolEnd, &eventToSend);
    }

    if (eventToSend.Handled || eventToSend.HandledEventScript)
      continue;

    MetaTransform* metaTransform = handle.StoredType->HasInherited<MetaTransform>();
    if (metaTransform == nullptr)
      continue;

    MetaTransformInstance transform = metaTransform->GetInstance(handle);
    if (transform.IsNull())
      continue;

    if (Space* space = transform.mSpace.Get<Space*>())
      space->MarkModified();

    String name;
    if (MetaDisplay* display = handle.StoredType->HasInherited<MetaDisplay>())
      name = display->GetName(handle);

    queue->BeginBatch();
    queue->SetActiveBatchName(BuildString(name, " Manipulation"));

    // Send the final GizmoFinish transform update
    uint flag = 0;

    Cog* cog = handle.Get<Cog*>();
    if (cog && mNewObjects.Contains(handle))
    {
      String createName;
      if (MetaDisplay* display = handle.StoredType->HasInherited<MetaDisplay>())
        createName = display->GetName(handle);
      else
        createName = handle.StoredType->Name;

      queue->SetActiveBatchName(BuildString("Create ", createName));

      ObjectCreated(queue, cog);

      // Single duplicated object proper hierarchy, emulates ctrl+c, ctrl+v
      // behavior.
      if (mTransformingObjects.Size() == 1 && mDuplicateCorrectIndex != unsigned(-1))
      {
        MoveObjectIndex(queue, cog, mDuplicateCorrectIndex);
        mDuplicateCorrectIndex = unsigned(-1);
      }
    }

    // When manipulating multiple objects, translation might have changed.
    if ((target.StartTranslation - target.EndTranslation).LengthSq() != 0.0f)
    {
      flag |= TransformUpdateFlags::Translation;
      transform.SetLocalTranslation(target.StartTranslation);

      PropertyPath propertyPath(transform.mLocalTranslation);
      ChangeAndQueueProperty(queue, transform.mInstance, propertyPath, target.EndTranslation);
    }

    // Scale might have changed.
    if ((target.StartScale - target.EndScale).LengthSq() != 0.0f)
    {
      flag |= TransformUpdateFlags::Scale;
      transform.SetLocalScale(target.StartScale);

      PropertyPath propertyPath(transform.mLocalScale);
      ChangeAndQueueProperty(queue, transform.mInstance, propertyPath, target.EndScale);
    }

    if (Cog* areaCog = handle.Get<Cog*>())
    {
      // Area might have changed.
      Area* area = areaCog->has(Area);
      if (area && (target.StartSize - target.EndSize).LengthSq() != 0.0f)
      {
        area->SetSize(target.StartSize);

        PropertyPath areaPath(area, areaSizeProperty);
        ChangeAndQueueProperty(queue, areaCog, areaPath, target.EndSize);

        // No need to update an object's BoxCollider if there isn't one, or if
        // that option isn't enabled.
        BoxCollider* collider = areaCog->has(BoxCollider);
        if (collider != nullptr && mSizeBoxCollider)
        {
          collider->SetOffset(target.StartColliderOffset);

          Vec3 endOffset(area->OffsetOfOffset(Location::Center) * area->GetSize());
          PropertyPath offsetPath(collider, colliderOffsetProperty);
          ChangeAndQueueProperty(queue, areaCog, offsetPath, endOffset);

          collider->SetSize(target.StartColliderSize);

          Vec3 endSize(target.EndSize.x, target.EndSize.y, target.StartColliderSize.z);
          PropertyPath sizePath(collider, colliderSizeProperty);
          ChangeAndQueueProperty(queue, areaCog, sizePath, endSize);
        }
      }
    }

    queue->EndBatch(); // end specific object Manipulation

    // transform->Update(flag | TransformUpdateFlags::GizmoFinish);
  }

  queue->EndBatch(); // end MultiObjectManipulation

  mGizmoMode = GizmoMode::Active;
  UpdateRectAndBasis();
}

void ManipulatorTool::OnToolDraw(Event*)
{
  // Nothing to draw.
  if (mObjects.Empty() || mGizmoMode == GizmoMode::Inactive)
    return;

  ByteColor toolColor = ToByteColor(mToolColor);
  ByteColor hoverColor = ToByteColor(mHoverColor);

  Vec2 halfSize = mActiveRect.GetSize() * 0.5f;

  Vec3 moveX = Vec3(halfSize.x, 0, 0);
  Vec3 moveY = Vec3(0, halfSize.y, 0);

  moveX = Math::Multiply(mObbBasis, moveX);
  moveY = Math::Multiply(mObbBasis, moveY);

  Vec3 center;
  if (mGizmoMode == GizmoMode::Transforming)
    center = mActiveOrigin3D + moveX + moveY;
  else
    center = mStartCenter3D;

  Vec3 placementX(0.0f * cGripSize, 0, 0);
  Vec3 placementY(0, 0.0f * cGripSize, 0);
  placementX = Math::Multiply(mObbBasis, placementX);
  placementY = Math::Multiply(mObbBasis, placementY);

  Vec3 middles[] = {center - moveX, // Center Left
                    center + moveY, // Center Top
                    center + moveX, // Center Right
                    center - moveY, // Center Bottom
                    center};

  Vec3 gripSize(cGripSize, cGripSize, cGripSize * 0.1f);

  for (uint i = 0; i <= 4; ++i)
  {
    ByteColor c = toolColor;
    if (2 * i == (uint)mSelectedPoint)
      c = hoverColor;

    gDebugDraw->Add(Debug::Box(middles[i], gripSize, mObbBasis).Filled(true).ViewScaled(true).Color(c).OnTop(true));
  }

  Vec3 corner[] = {
      center - moveX + moveY + placementY, // Top Left
      center + moveX + moveY + placementY, // Top Right
      center + moveX - moveY + placementY, // Bottom Right
      center - moveX - moveY + placementY  // Bottom Left
  };

  // Double grip size for corners.
  gripSize *= cCornerGripScalar;

  Vec3 axis = Math::Multiply(mObbBasis, Vec3::cZAxis);

  // Draw the corners on top of middles.
  for (uint i = 0; i < 4; ++i)
  {
    ByteColor c = toolColor;
    if (2 * i + 1 == (uint)mSelectedPoint)
      c = hoverColor;

    // Corner grab spots.
    gDebugDraw->Add(Debug::Circle(corner[i], axis, gripSize.x).Filled(true).ViewScaled(true).Color(c).OnTop(true));
  }

  // Entire tool fill.
  ByteColor triColor = ColorWithAlpha(toolColor, 0.05f);
  gDebugDraw->Add(Debug::Triangle(corner[3], corner[2], corner[1]).Color(triColor).Filled(true));
  gDebugDraw->Add(Debug::Triangle(corner[3], corner[1], corner[0]).Color(triColor).Filled(true));

  // Tool wireframe
  gDebugDraw->Add(Debug::Box(center, halfSize, mObbBasis).Color(toolColor).OnTop(true));
}

} // namespace Zero
