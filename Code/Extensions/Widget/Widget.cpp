// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{
//#define WIDGET_DEBUGGING

RaverieDefineType(SizePolicies, builder, type)
{
}

RaverieDefineType(Widget, builder, type)
{
  type->HandleManager = RaverieManagerId(WidgetHandleManager);
}

void WidgetHandleManager::ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize)
{
  if (object == nullptr)
    return;

  Widget* instance = (Widget*)object;
  instance->DebugValidate();
  handleToInitialize.HandleU64 = instance->mId;
}

byte* WidgetHandleManager::HandleToObject(const Handle& handle)
{
  return (byte*)Z::gWidgetManager->Widgets.FindValue(handle.HandleU64, nullptr);
}

void WidgetHandleManager::Delete(const Handle& handle)
{
  Widget* widget = handle.Get<Widget*>();
  if (widget)
  {
    widget->DebugValidate();
    widget->Destroy();
  }
}

bool WidgetHandleManager::CanDelete(const Handle& handle)
{
  return true;
}

u64 WidgetHandleManager::HandleToId(const Handle& handle)
{
  if (handle.StoredType == nullptr)
    return 0;

  ReturnIf(!Type::BoundIsA(handle.StoredType, RaverieTypeId(Widget)), 0, "A handle to a non widget was passed in.");

  return handle.HandleU64;
}

HashSet<const Widget*> gValidWidgets;
HashSet<const Widget*> gValidWidgetsAllTime;

static Array<String> gWidgetStack;
void WidgetBreak(const char* message, const Widget* widget)
{
  forRange (String& str, gWidgetStack)
  {
    printf("STACK: %s\n", str.c_str());
  }
  Error("%s %p\n", message, widget);
  printf("%s %p\n", message, widget);
  fflush(stdout);
  RaverieDebugBreak();
}
bool Widget::sDisableDeletes = false;

Widget::Widget(Composite* parent, AttachType::Enum attachType)
{
#ifdef WIDGET_DEBUGGING
  gValidWidgets.InsertOrError(this);
  gValidWidgetsAllTime.Insert(this);
#endif
  mId = ++Z::gWidgetManager->IdCounter;
  Z::gWidgetManager->Widgets.Insert(mId, this);

  ClearValues();

  if (parent)
  {
    mRootWidget = parent->mRootWidget;
    mDefSet = parent->GetDefinitionSet();
    parent->AttachChildWidget(this, attachType);
  }
  else
  {
    mDefSet = DefinitionSetManager::GetInstance()->Main;
    mRootWidget = (RootWidget*)this;
  }
  DebugValidate();
}

void Widget::ClearValues()
{
  mWorldTx = Mat4::cIdentity;
  mCurDockMode = DockMode::DockNone;
  mTransformUpdateState = TransformUpdateState::LocalUpdate;
  mDocker = nullptr;
  mParent = nullptr;
  mDefSet = nullptr;
  mSize = Vec2(10, 10);
  mTranslation = Vec3::cZero;
  mColor = Vec4(1, 1, 1, 1);
  mNotInLayout = false;
  mHideOnClose = false;
  mDestroyed = false;
  mActions = nullptr;
  mAngle = 0.0f;
  mActive = true;
  mVisible = true;
  mClipping = false;
  mInteractive = true;
  mOrigin = DisplayOrigin::TopLeft;
  mTakeFocusMode = FocusMode::Soft;
  mNeedsRedraw = true;
  mSizePolicy = SizePolicies(SizePolicy::Flex, SizePolicy::Flex);
  mDragDistance = 6.0f;
  mHorizontalAlignment = HorizontalAlignment::Left;
  mVerticalAlignment = VerticalAlignment::Top;
  mManager = nullptr;
}

Widget::~Widget()
{
  DebugValidate();
  SafeDelete(mDocker);
  SafeRelease(mActions);

  if (mManager)
    mManager->Destroyed(this);

  if (mParent)
  {
    mParent->MarkAsNeedsUpdate(true);
    mParent->mChildren.Erase(this);
    mParent = nullptr;
  }

#ifdef WIDGET_DEBUGGING
  if (!gValidWidgets.Erase(this))
  {
    WidgetBreak("Did not find widget, double delete? %p\n", this);
  }
#endif

  if (sDisableDeletes)
  {
    WidgetBreak("Widget deletes disabled %p\n", this);
  }
}

String Widget::GetDebugName() const
{
  if (!mName.Empty())
    return mName;

  return RaverieVirtualTypeId(this)->Name;
}

void Widget::InternalDestroy()
{
  DebugValidate();
  if (!mDestroyed)
  {
    mDestroyed = true;
    mNotInLayout = true;

    Z::gWidgetManager->Widgets.Erase(mId);
    Z::gWidgetManager->DestroyList.PushBack(this);

    if (HasFocus())
      this->LoseFocus();
  }
}

void Widget::OnDestroy()
{
  DebugValidate();
}

void Widget::Destroy()
{
  if (!mDestroyed)
  {
    // Call on Destroy for this chain
    this->OnDestroy();
    InternalDestroy();
  }
}

void Widget::SetName(StringParam name)
{
  DebugValidate();
  mName = name;
}

String Widget::GetName()
{
  DebugValidate();
  return mName;
}

bool Widget::InputBlocked()
{
  DebugValidate();
  return !mInteractive || !mActive || !mVisible || mDestroyed;
}

bool Widget::CheckClipping(Vec2Param screenPoint)
{
  DebugValidate();
  if (mClipping)
  {
    WidgetRect rect = GetLocalRect();
    Vec2 localMousePos = this->ToLocal(screenPoint);
    bool withIn = rect.Contains(localMousePos);
    if (!withIn)
      return false;
  }
  return true;
}

Widget* Widget::HitTest(Vec2 screenPoint, Widget* ignore)
{
  DebugValidate();
  // Skip inactive object
  if (InputBlocked())
    return nullptr;

  if (ignore == this)
    return nullptr;

  // Check for containment
  if (!CheckClipping(screenPoint))
    return nullptr;

  if (!Contains(screenPoint))
    return nullptr;

  return this;
}

RootWidget* Widget::GetRootWidget()
{
  DebugValidate();
  return mRootWidget;
}

void Widget::SetDocker(Docker* docker)
{
  DebugValidate();
  SafeDelete(mDocker);
  mDocker = docker;
}

void Widget::DispatchBubble(StringParam eventId, Event* event)
{
  DebugValidate();
  GetDispatcher()->Dispatch(eventId, event);
  if (mParent)
    mParent->DispatchBubble(eventId, event);
}

Actions* Widget::GetActions()
{
  DebugValidate();
  if (mActions == nullptr)
  {
    mActions = new Actions(Z::gWidgetManager->mWidgetActionSpace);
    mActions->AddReference();
  }
  return mActions;
}

bool Widget::HasFocus()
{
  DebugValidate();
  return mFlags.IsSet(DisplayFlags::FocusHierarchy);
}

bool Widget::IsMouseOver()
{
  DebugValidate();
  return mFlags.IsSet(DisplayFlags::MouseOverHierarchy);
}

void Widget::SetColor(Vec4Param color)
{
  DebugValidate();
  if (color == mColor)
    return;
  mColor = color;
  MarkAsNeedsUpdate();
}

void Widget::SetSize(Vec2 newSize)
{
  DebugValidate();
  mSize = SnapToPixels(newSize);
  MarkAsNeedsUpdate();
}

void Widget::SetSizing(SizeAxis::Enum axis, SizePolicy::Enum policy, float size)
{
  DebugValidate();
  mSizePolicy.Policy[axis] = policy;
  mSizePolicy.Size[axis] = size;
  if (policy == SizePolicy::Fixed)
    mSize[axis] = size;
}

void Widget::SetSizing(SizePolicy::Enum policy, Vec2Param size)
{
  DebugValidate();
  SetSizing(SizeAxis::X, policy, size.x);
  SetSizing(SizeAxis::Y, policy, size.y);
}

void Widget::SetSizing(SizePolicy::Enum policy, float size)
{
  DebugValidate();
  SetSizing(policy, Vec2(size));
}

void Widget::SetTranslation(Vec3 newTranslation)
{
  DebugValidate();
  mTranslation = SnapToPixels(newTranslation);
  MarkAsNeedsUpdate();
}

size_t Widget::GetDepth()
{
  DebugValidate();
  size_t depth = 0;
  Widget* it = this;
  while ((it = it->GetParent()))
    ++depth;
  return depth;
}

void Widget::SetTranslationAndSize(Vec3 newTranslation, Vec2 newSize)
{
  DebugValidate();
  mSize = SnapToPixels(newSize);
  mTranslation = SnapToPixels(newTranslation);
  MarkAsNeedsUpdate();
}

void Widget::NeedsRedraw()
{
  DebugValidate();
  mNeedsRedraw = true;
  if (mParent)
    mParent->NeedsRedraw();
}

void Widget::MarkAsNeedsUpdate(bool local)
{
  DebugValidate();
  mNeedsRedraw = true;

  if (mTransformUpdateState == TransformUpdateState::Updated)
  {
    if (local)
      mTransformUpdateState = TransformUpdateState::LocalUpdate;
    else
      mTransformUpdateState = TransformUpdateState::ChildUpdate;

    if (mParent)
      mParent->MarkAsNeedsUpdate(false);
  }
  else if (local && mTransformUpdateState == TransformUpdateState::ChildUpdate)
  {
    mTransformUpdateState = TransformUpdateState::LocalUpdate;
  }
}

bool Widget::IsAncestorOf(Widget* child)
{
  DebugValidate();
  Widget* current = child;
  while (current)
  {
    current = current->GetParent();
    if (current == this)
      return true;
  }
  return false;
}

void Widget::MoveToFront()
{
  DebugValidate();
  Composite* parent = mParent;
  parent->mChildren.Erase(this);
  parent->mChildren.PushBack(this);
}

void Widget::MoveToBack()
{
  DebugValidate();
  Composite* parent = mParent;
  parent->mChildren.Erase(this);
  parent->mChildren.PushFront(this);
}

Vec2 Widget::ToLocal(Vec2Param screenPoint)
{
  DebugValidate();
  Mat4 toLocal = Invert2D(mWorldTx);
  Vec3 localPoint = TransformPointCol(toLocal, Vec3(screenPoint.x, screenPoint.y, 0.0f));
  return Vec2(localPoint.x, localPoint.y);
}

Vec3 Widget::ToLocal(Vec3Param screenPoint)
{
  DebugValidate();
  Mat4 toLocal = Invert2D(mWorldTx);
  return TransformPointCol(toLocal, screenPoint);
}

Vec2 Widget::ToScreen(Vec2Param localPoint)
{
  DebugValidate();
  Vec3 screenPoint = Math::ToVector3(localPoint, 0.0f);
  screenPoint = TransformPointCol(mWorldTx, screenPoint);
  return Vec2(screenPoint.x, screenPoint.y);
}

Vec3 Widget::GetScreenPosition() const
{
  DebugValidate();
  if (mParent)
    return mTranslation + mParent->GetScreenPosition();
  return mTranslation;
}

WidgetRect Widget::GetRectInParent()
{
  DebugValidate();
  WidgetRect local = GetLocalRect();
  local.X = mTranslation.x;
  local.Y = mTranslation.y;
  return local;
}

WidgetRect Widget::GetLocalRect() const
{
  DebugValidate();
  if (mOrigin == DisplayOrigin::Center)
    return WidgetRect::PointAndSize(mSize * 0.5f, mSize);
  else
    return WidgetRect::PointAndSize(Vec2::cZero, mSize);
}

WidgetRect Widget::GetScreenRect() const
{
  DebugValidate();
  Vec3 screenPos = GetScreenPosition();
  WidgetRect rect = GetLocalRect();
  rect.X += screenPos.x;
  rect.Y += screenPos.y;
  return rect;
}

Vec2 Widget::GetClientCenterPosition() const
{
  DebugValidate();
  WidgetRect clientRect = GetScreenRect();
  return Vec2(clientRect.X + clientRect.SizeX / 2.0f, clientRect.Y + clientRect.SizeY / 2.0f);
}

bool Widget::Contains(Vec2 screenPoint)
{
  DebugValidate();
  WidgetRect localRect = GetLocalRect();
  Vec2 localMousePos = this->ToLocal(screenPoint);
  return localRect.Contains(localMousePos);
}

bool Widget::TryTakeFocus()
{
  DebugValidate();
  return this->TakeFocusOverride();
}

void Widget::DebugValidate() const
{
#ifdef WIDGET_DEBUGGING
  if ((void*)this == nullptr || this->mId > INT_MAX)
  {
    WidgetBreak("Widget was null or had an invalid id", this);
  }

  if (!gValidWidgets.Contains(this))
  {
    if (gValidWidgetsAllTime.Contains(this))
    {
      WidgetBreak("Invalid widget (but it was valid at one point in time)", this);
    }
    else
    {
      WidgetBreak("Invalid widget", this);
    }
  }

  static HashSet<void*> vtables;

  void* vtable = *(void**)this;
  if (!vtables.Contains(vtable))
  {
    vtables.Insert(vtable);
    printf("New vtable: %p\n", vtable);
    fflush(stdout);
  }

  gWidgetStack.PushBack(GetDebugName());
  Composite* composite = const_cast<Widget*>(this)->GetSelfAsComposite();
  if (composite)
  {
    WidgetListRange children = composite->GetChildren();
    while (!children.Empty())
    {
      Widget& child = children.Front();
      child.DebugValidate();
      children.PopFront();
    }
  }
  gWidgetStack.PopBack();

  // Access all the widget memory
  static byte sMemory[sizeof(Widget)];
  memcpy(sMemory, (void*)this, sizeof(Widget));

  if (!mDestroyed)
  {
    Widget* widget = Z::gWidgetManager->Widgets.FindValue(this->mId, nullptr);
    if (widget == nullptr || widget != this || widget->mId != this->mId)
    {
      WidgetBreak("Widget was not found in the widget manager", this);
    }
  }
#endif
}

void Widget::TakeFocus()
{
  DebugValidate();
  bool focusTaken = this->TakeFocusOverride();
  ErrorIf(!focusTaken, "The widget can not take focus.");
}

void Widget::SoftTakeFocus()
{
  DebugValidate();
  GetRootWidget()->RootSoftTakeFocus(this);
}

void Widget::HardTakeFocus()
{
  DebugValidate();
  GetRootWidget()->RootChangeFocus(this, FocusMode::Hard);
}

bool Widget::TakeFocusOverride()
{
  DebugValidate();
  // By default do not take focus
  return false;
}

void Widget::LoseFocus()
{
  DebugValidate();
  GetRootWidget()->RootRemoveFocus(this);
  MarkAsNeedsUpdate(true);
}

void Widget::ChangeDefinition(BaseDefinition* def)
{
  DebugValidate();
}

void Widget::CaptureMouse()
{
  DebugValidate();
  GetRootWidget()->RootCaptureMouse(this);
}

void Widget::ReleaseMouseCapture()
{
  GetRootWidget()->RootReleaseMouseCapture(this);
}

void Widget::SetRotation(float angle)
{
  DebugValidate();
  mAngle = angle;
}

float Widget::GetRotation()
{
  DebugValidate();
  return mAngle;
}

void Widget::ScreenCaptureBackBuffer(Image& image)
{
  DebugValidate();
  WidgetRect rect = GetLocalRect();
  ScreenCaptureBackBuffer(image, rect);
}

void Widget::ScreenCaptureBackBuffer(Image& image, WidgetRect& subRect)
{
  DebugValidate();
}

void Widget::SetTakeFocusMode(FocusMode::Type focusMode)
{
  DebugValidate();
  mTakeFocusMode = focusMode;
}

void Widget::SetClipping(bool clipping)
{
  DebugValidate();
  mClipping = clipping;
}

void Widget::DispatchAt(DispatchAtParams& params)
{
  DebugValidate();
  Widget* hit = this->HitTest(params.Position, params.Ignore);
  if (!hit)
    return;

  if (params.BubbleEvent)
    hit->DispatchBubble(params.EventId, params.EventObject);
  else
    hit->DispatchEvent(params.EventId, params.EventObject);
  params.ObjectHit = true;
}

bool Widget::GetClipping()
{
  DebugValidate();
  return mClipping;
}

bool GetZIndexDepthFirst(Widget* widget, Widget* target, int* zindex)
{
  if (!widget)
    return false;

  Composite* composite = widget->GetSelfAsComposite();
  if (composite)
  {
    forRange (Widget& child, composite->GetChildren())
    {
      if (&child == target)
        return true;
      else
        ++*zindex;

      // If we found the child, stop execution and return all the way up.
      if (GetZIndexDepthFirst(&child, target, zindex))
        return true;
    }
  }
  return false;
}

int Widget::GetZIndex()
{
  DebugValidate();
  // This is a potentially expensive function to call because it walks
  // all widgets until we find our own widget (from the root).
  int zindex = 0;
  GetZIndexDepthFirst(GetRootWidget(), this, &zindex);
  return zindex;
}

void Widget::BuildLocalMatrix(Mat4& output)
{
  DebugValidate();
  Build2dTransform(output, this->mTranslation, this->mAngle);
}

void Widget::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, WidgetRect clipRect)
{
  DebugValidate();
  Mat4 localTx;
  BuildLocalMatrix(localTx);
  mWorldTx = localTx * parentTx;
}

ViewNode& Widget::AddRenderNodes(ViewBlock& viewBlock, FrameBlock& frameBlock, WidgetRect clipRect, Texture* texture)
{
  DebugValidate();
  FrameNode& frameNode = frameBlock.mFrameNodes.PushBack();
  ViewNode& viewNode = viewBlock.mViewNodes.PushBack();

  frameNode.mGraphicalEntry = nullptr;
  viewNode.mGraphicalEntry = nullptr;

  frameNode.mBorderThickness = 1.0f;
  frameNode.mBlendSettingsOverride = false;
  frameNode.mRenderingType = RenderingType::Streamed;
  frameNode.mCoreVertexType = CoreVertexType::Streamed;
  frameNode.mLocalToWorld = mWorldTx.Transposed();
  viewNode.mFrameNodeIndex = frameBlock.mFrameNodes.Size() - 1;
  viewNode.mLocalToView = viewBlock.mWorldToView * frameNode.mLocalToWorld;

  frameNode.mClip = Vec4(clipRect.X, clipRect.Y, clipRect.SizeX, clipRect.SizeY);

  // maybe cache this lookup on root
  Material* spriteMaterial = nullptr;

  if (texture->mType == TextureType::TextureCube)
    spriteMaterial = MaterialManager::FindOrNull("TextureCubePreview");
  else
    spriteMaterial = MaterialManager::FindOrNull("AlphaSprite");

  frameNode.mMeshRenderData = nullptr;
  frameNode.mMaterialRenderData = spriteMaterial->mRenderData;
  frameNode.mTextureRenderData = texture->mRenderData;

  // default setup for adding streamed data
  viewNode.mStreamedVertexType = PrimitiveType::Triangles;
  viewNode.mStreamedVertexStart = frameBlock.mRenderQueues->mStreamedVertices.Size();
  viewNode.mStreamedVertexCount = 0;

  return viewNode;
}

void Widget::CreateRenderData(ViewBlock& viewBlock, FrameBlock& frameBlock, WidgetRect clipRect, Array<StreamedVertex>& vertices, PrimitiveType::Enum primitiveType)
{
  DebugValidate();
  if (vertices.Empty())
    return;

  StreamedVertexArray& streamedVertices = frameBlock.mRenderQueues->mStreamedVertices;

  static Texture* white = TextureManager::Find("White");
  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, white);
  viewNode.mStreamedVertexType = primitiveType;

  for (uint i = 0; i < vertices.Size(); ++i)
  {
    StreamedVertex vertex = vertices[i];
    vertex.mPosition = Math::TransformPoint(viewNode.mLocalToView, vertex.mPosition);
    streamedVertices.PushBack(vertex);
  }

  viewNode.mStreamedVertexCount = streamedVertices.Size() - viewNode.mStreamedVertexStart;
}

void Widget::SizeToContents()
{
  DebugValidate();
  SetSize(GetMinSize());
}

void Widget::UpdateTransformExternal()
{
  UpdateTransform();

  static String message = "Improper Widget Update. Did you forget to call your "
                          "base UpdateTransform at the end of "
                          "your UpdateTransform?";
  ErrorIf(!mDestroyed && mTransformUpdateState != TransformUpdateState::Updated, message.c_str());
}

void Widget::UpdateTransform()
{
  DebugValidate();
  // Clear the update flag
  mTransformUpdateState = TransformUpdateState::Updated;
}

void Widget::SetDockMode(DockMode::Enum dockMode)
{
  DebugValidate();
  mCurDockMode = dockMode;
  mParent->MarkAsNeedsUpdate();
}

void Widget::SetDockArea(DockArea::Enum dockArea)
{
  DebugValidate();
  if (mDocker)
    mDocker->Dock(this, dockArea);
  if (dockArea == DockArea::Floating)
    SetDockMode(DockMode::DockNone);
  else
    SetDockMode(DockMode::DockFill);
}

bool Widget::GetActive()
{
  DebugValidate();
  return mActive;
}

bool Widget::GetGlobalActive()
{
  DebugValidate();
  Widget* current = this;
  while (current)
  {
    if (current->mActive == false)
      return false;
    current = current->mParent;
  }

  return true;
}

void Widget::SetActive(bool active)
{
  DebugValidate();
  if (mActive != active)
  {
    mActive = active;
    MarkAsNeedsUpdate();
    mParent->MarkAsNeedsUpdate();

    // Activated does not bubble.
    ObjectEvent event(this);
    if (active)
      DispatchEvent(Events::Activated, &event);
    else
    {
      DispatchEvent(Events::Deactivated, &event);
      LoseFocus();
    }
  }
}

void Widget::SetInteractive(bool interactive)
{
  DebugValidate();
  mInteractive = interactive;
}

Vec2 Widget::GetMinSize()
{
  DebugValidate();
  return Pixels(100, 100);
}

Vec2 Widget::Measure(LayoutArea& data)
{
  DebugValidate();
  Vec2 measureSize = GetMinSize();
  if (mSizePolicy.XPolicy == SizePolicy::Fixed)
    measureSize.x = mSize.x;
  if (mSizePolicy.YPolicy == SizePolicy::Fixed)
    measureSize.y = mSize.y;
  return measureSize;
}

Element* Widget::CreateAttachedGeneric(StringParam name)
{
  DebugValidate();
  Composite* thisComposite = this->GetSelfAsComposite();
  ImageWidget* w = new ImageWidget(thisComposite, name, AttachType::Direct);
  w->SetNotInLayout(true);
  return w;
}

Thickness Widget::GetBorderThickness()
{
  DebugValidate();
  return Thickness::cZero;
}

} // namespace Raverie
