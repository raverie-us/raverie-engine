///////////////////////////////////////////////////////////////////////////////
///
/// \file Widget.cpp
/// Implementation of the base widget class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------- Size Policies
//******************************************************************************
ZilchDefineType(SizePolicies, builder, type)
{
}

//----------------------------------------------------------------------- Widget
//******************************************************************************
ZilchDefineType(Widget, builder, type)
{
  type->HandleManager = ZilchManagerId(WidgetHandleManager);
}

void WidgetHandleManager::ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize)
{
  if (object == nullptr)
    return;

  Widget* instance = (Widget*)object;
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
    widget->Destroy();
}

bool WidgetHandleManager::CanDelete(const Handle& handle)
{
  return true;
}


Widget::Widget(Composite* parent, AttachType::Enum attachType)
{
  mId = ++Z::gWidgetManager->IdCounter;
  Z::gWidgetManager->Widgets.Insert(mId, this);

  ClearValues();

  if(parent)
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
  SafeDelete(mDocker);
  SafeRelease(mActions);

  if(mManager)
    mManager->Destroyed(this);

  if(mParent)
  {
    mParent->MarkAsNeedsUpdate(true);
    mParent->mChildren.Erase(this);
    mParent = nullptr;
  }
}

String Widget::GetDebugName()
{
  if(!mName.Empty())
    return mName;

  return ZilchVirtualTypeId(this)->Name;
}

void Widget::InternalDestroy()
{
  if(!mDestroyed)
  {
    mDestroyed = true;
    mNotInLayout = true;

    Z::gWidgetManager->Widgets.Erase(mId);
    Z::gWidgetManager->DestroyList.PushBack(this);

    if(HasFocus())
      this->LoseFocus();
  }
}

void Widget::OnDestroy()
{

}

void Widget::Destroy()
{
  if(!mDestroyed)
  {
    // Call on Destroy for this chain
    this->OnDestroy();
    InternalDestroy();
  }
}

void Widget::SetName(StringParam name)
{
  mName = name;
}

String Widget::GetName()
{
  return mName;
}

bool Widget::InputBlocked()
{
  return !mInteractive || !mActive || !mVisible || mDestroyed;
}

bool Widget::CheckClipping(Vec2Param screenPoint)
{
  if(mClipping)
  {
    Rect rect = GetLocalRect();
    Vec2 localMousePos = this->ToLocal(screenPoint);
    bool withIn = rect.Contains(localMousePos);
    if(!withIn)
      return false;
  }
  return true;
}

Widget* Widget::HitTest(Vec2 screenPoint, Widget* ignore)
{
  // Skip inactive object
  if(InputBlocked())
    return nullptr;

  if(ignore == this)
    return nullptr;

  // Check for containment
  if(!CheckClipping(screenPoint))
    return nullptr;

  if(!Contains(screenPoint))
    return nullptr;

  return this;
}

RootWidget* Widget::GetRootWidget()
{
  return mRootWidget;
}

void Widget::SetDocker(Docker* docker)
{
  SafeDelete(mDocker);
  mDocker = docker;
}

void Widget::DispatchBubble(StringParam eventId, Event* event)
{
  GetDispatcher()->Dispatch(eventId, event);
  if(mParent) mParent->DispatchBubble(eventId, event);
}

Actions* Widget::GetActions()
{
  if(mActions == nullptr)
  {
    mActions = new Actions(Z::gWidgetManager->mWidgetActionSpace);
    mActions->AddReference();
  }
  return mActions;
}

bool Widget::HasFocus()
{
  return mFlags.IsSet(DisplayFlags::FocusHierarchy);
}

bool Widget::IsMouseOver()
{
  return mFlags.IsSet(DisplayFlags::MouseOverHierarchy);
}

void Widget::SetColor(Vec4Param color)
{
  mColor = color;
  MarkAsNeedsUpdate();
}

void Widget::SetSize(Vec2 newSize)
{
  mSize = SnapToPixels(newSize);
  MarkAsNeedsUpdate();
}

void Widget::SetSizing(SizeAxis::Enum axis, SizePolicy::Enum policy, float size)
{
  mSizePolicy.Policy[axis] = policy;
  mSizePolicy.Size[axis] = size;
  if (policy == SizePolicy::Fixed)
    mSize[axis] = size;
}

void Widget::SetSizing(SizePolicy::Enum policy, Vec2Param size)
{
  SetSizing(SizeAxis::X, policy, size.x);
  SetSizing(SizeAxis::Y, policy, size.y);
}

void Widget::SetSizing(SizePolicy::Enum policy, float size)
{
  SetSizing(policy, Vec2(size));
}

void Widget::SetTranslation(Vec3 newTranslation)
{
  mTranslation = SnapToPixels(newTranslation);
  MarkAsNeedsUpdate();
}

void Widget::SetTranslationAndSize(Vec3 newTranslation, Vec2 newSize)
{
  mSize = SnapToPixels(newSize);
  mTranslation = SnapToPixels(newTranslation);
  MarkAsNeedsUpdate();
}

void Widget::NeedsRedraw()
{
  mNeedsRedraw = true;
  if(mParent)
    mParent->NeedsRedraw();
}

void Widget::MarkAsNeedsUpdate(bool local)
{
  mNeedsRedraw = true;

  if(mTransformUpdateState == TransformUpdateState::Updated)
  {
    if(local)
      mTransformUpdateState = TransformUpdateState::LocalUpdate;
    else
      mTransformUpdateState = TransformUpdateState::ChildUpdate;

    if(mParent)
      mParent->MarkAsNeedsUpdate(false);
  }
  else if(local && mTransformUpdateState == TransformUpdateState::ChildUpdate)
  {
    mTransformUpdateState = TransformUpdateState::LocalUpdate;
  }
}

bool Widget::IsAncestorOf(Widget* child)
{
  Widget* current = child;
  while(current)
  {
    current = current->GetParent();
    if(current == this)
      return true;
  }
  return false;
}

void Widget::MoveToFront()
{
  Composite* parent = mParent;
  parent->mChildren.Erase(this);
  parent->mChildren.PushBack(this);
}

void Widget::MoveToBack()
{
  Composite* parent = mParent;
  parent->mChildren.Erase(this);
  parent->mChildren.PushFront(this);
}

Vec2 Widget::ToLocal(Vec2Param screenPoint)
{
  Mat4 toLocal = Invert2D(mWorldTx);
  Vec3 localPoint = TransformPointCol(toLocal, Vec3(screenPoint.x, screenPoint.y, 0.0f));
  return Vec2(localPoint.x, localPoint.y);
}

Vec3 Widget::ToLocal(Vec3Param screenPoint)
{
  Mat4 toLocal = Invert2D(mWorldTx);
  return TransformPointCol(toLocal, screenPoint);
}

Vec2 Widget::ToScreen(Vec2Param localPoint)
{
  Vec3 screenPoint = Math::ToVector3(localPoint, 0.0f);
  screenPoint = TransformPointCol(mWorldTx, screenPoint);
  return Vec2(screenPoint.x, screenPoint.y);
}

Vec3 Widget::GetScreenPosition()
{
  if(mParent)
    return mTranslation + mParent->GetScreenPosition();
  return mTranslation;
}

Rect Widget::GetRectInParent()
{
  Rect local = GetLocalRect();
  local.X = mTranslation.x;
  local.Y = mTranslation.y;
  return local;
}

Rect Widget::GetLocalRect()
{
  if(mOrigin == DisplayOrigin::Center)
    return Rect::PointAndSize(mSize * 0.5f, mSize);
  else
    return Rect::PointAndSize(Vec2::cZero, mSize);
}

Rect Widget::GetScreenRect()
{
  Vec3 screenPos = GetScreenPosition();
  Rect rect = GetLocalRect();
  rect.X += screenPos.x;
  rect.Y += screenPos.y;
  return rect;
}

bool Widget::Contains(Vec2 screenPoint)
{
  Rect localRect = GetLocalRect();
  Vec2 localMousePos = this->ToLocal(screenPoint);
  return localRect.Contains(localMousePos);
}

bool Widget::TryTakeFocus()
{
  return this->TakeFocusOverride();
}

void Widget::TakeFocus()
{
  bool focusTaken = this->TakeFocusOverride();
  ErrorIf(!focusTaken, "The widget can not take focus.");
}

void Widget::SoftTakeFocus()
{
  GetRootWidget()->RootSoftTakeFocus(this);
}

void Widget::HardTakeFocus()
{
  GetRootWidget()->RootChangeFocus(this, FocusMode::Hard);
}

bool Widget::TakeFocusOverride()
{
  // By default do not take focus
  return false;
}

void Widget::LoseFocus()
{
  GetRootWidget()->RootRemoveFocus(this);
  MarkAsNeedsUpdate(true);
}

void Widget::ChangeDefinition(BaseDefinition* def)
{

}

void Widget::CaptureMouse()
{
  GetRootWidget()->RootCaptureMouse(this);
}

void Widget::ReleaseMouseCapture()
{
  GetRootWidget()->RootReleaseMouseCapture(this);
}

void Widget::SetRotation(float angle)
{
  mAngle = angle;
}

float Widget::GetRotation()
{
  return mAngle;
}

void Widget::ScreenCaptureBackBuffer(Image& image)
{
  Rect rect = GetLocalRect();
  ScreenCaptureBackBuffer(image, rect);
}

void Widget::ScreenCaptureBackBuffer(Image& image, Rect& subRect)
{
  //GraphicsViewport viewport = GenerateSubViewport(mWorldTx, subRect.TopLeft(), subRect.Size());

  //RootWidget* root = GetRootWidget();
  //CaptureViewport(root->GetOsWindow()->GetGraphicsContext(), &image, viewport);
}

void Widget::SetTakeFocusMode(FocusMode::Type focusMode)
{
  mTakeFocusMode = focusMode;
}

void Widget::SetClipping(bool clipping)
{
  mClipping = clipping;
}

void Widget::DispatchAt(DispatchAtParams& params)
{
  Widget* hit = this->HitTest(params.Position, params.Ignore);
  if(!hit) return;

  if(params.BubbleEvent)
    hit->DispatchBubble(params.EventId, params.EventObject);
  else
    hit->DispatchEvent(params.EventId, params.EventObject);
  params.ObjectHit = true;
}

bool Widget::GetClipping()
{
  return mClipping;
}

void Widget::BuildLocalMatrix(Mat4& output)
{
  Build2dTransform(output, this->mTranslation, this->mAngle);
}

void Widget::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Mat4 localTx;
  BuildLocalMatrix(localTx);
  mWorldTx = localTx * parentTx;
}

ViewNode& Widget::AddRenderNodes(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Texture* texture)
{
  FrameNode& frameNode = frameBlock.mFrameNodes.PushBack();
  ViewNode& viewNode = viewBlock.mViewNodes.PushBack();

  frameNode.mGraphicalEntry = nullptr;
  viewNode.mGraphicalEntry = nullptr;

  frameNode.mBorderThickness = 1.0f;
  frameNode.mRenderingType = RenderingType::Streamed;
  frameNode.mCoreVertexType = CoreVertexType::Streamed;
  frameNode.mLocalToWorld = mWorldTx.Transposed();
  viewNode.mFrameNodeIndex = frameBlock.mFrameNodes.Size() - 1;
  viewNode.mLocalToView = viewBlock.mWorldToView * frameNode.mLocalToWorld;

  frameNode.mClip = Vec4(clipRect.X, clipRect.Y, clipRect.SizeX, clipRect.SizeY);

  // maybe cache this lookup on root
  Material* spriteMaterial = nullptr;
  
  if(texture->mType == TextureType::TextureCube)
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

void Widget::CreateRenderData(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& vertices, PrimitiveType::Enum primitiveType)
{
  if (vertices.Empty())
    return;

  Array<StreamedVertex>& streamedVertices = frameBlock.mRenderQueues->mStreamedVertices;

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
  SetSize(GetMinSize());
}

void Widget::UpdateTransformExternal()
{
  UpdateTransform();

  static String message = "Improper Widget Update. Did you forget to call your base UpdateTransform at the end of "
                         "your UpdateTransform?";
  ErrorIf(!mDestroyed && mTransformUpdateState != TransformUpdateState::Updated, message.c_str());
}

void Widget::UpdateTransform()
{
  //Clear the update flag
  mTransformUpdateState = TransformUpdateState::Updated;
}

void Widget::SetDockMode(DockMode::Enum dockMode)
{
  mCurDockMode = dockMode;
  mParent->MarkAsNeedsUpdate();
}

void Widget::SetDockArea(DockArea::Enum dockArea)
{
  if(mDocker)
    mDocker->Dock(this, dockArea);
  if(dockArea == DockArea::Floating)
    SetDockMode(DockMode::DockNone);
  else
    SetDockMode(DockMode::DockFill);
}

bool Widget::GetActive()
{
  return mActive;
}

bool Widget::GetGlobalActive()
{
  Widget* current = this;
  while(current)
  {
    if (current->mActive == false)
      return false;
    current = current->mParent;
  }

  return true;
}

void Widget::SetActive(bool active)
{
  if(mActive != active)
  {
    mActive = active;
    MarkAsNeedsUpdate();
    mParent->MarkAsNeedsUpdate();

    // Activated does not bubble.
    ObjectEvent event(this);
    if(active)
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
  mInteractive = interactive;
}

Vec2 Widget::GetMinSize()
{
  return Pixels(100, 100);
}

Vec2 Widget::Measure(LayoutArea& data)
{
  Vec2 measureSize = GetMinSize();
  if(mSizePolicy.XPolicy == SizePolicy::Fixed)
    measureSize.x = mSize.x;
  if(mSizePolicy.YPolicy == SizePolicy::Fixed)
    measureSize.y = mSize.y;
  return measureSize;
}

Element* Widget::CreateAttachedGeneric(StringParam name)
{
  Composite* thisComposite = this->GetSelfAsComposite();
  ImageWidget* w = new ImageWidget(thisComposite, name, AttachType::Direct);
  w->SetNotInLayout(true);
  return w;
}

Thickness Widget::GetBorderThickness()
{
  return Thickness::cZero;
}

}//namespace Zero
