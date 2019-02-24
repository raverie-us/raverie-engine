// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace PreviewWidgetUi
{
const cstr cLocation = "EditorUi/PreviewWidgets";
Tweakable(Vec2, ColorGradientSize, Pixels(170, 30), cLocation);
Tweakable(Vec2, SpacePreviewSize, Pixels(200, 200), cLocation);
} // namespace PreviewWidgetUi

IconPreview::IconPreview(PreviewWidgetInitializer& initializer, StringParam icon) : PreviewWidget(initializer)
{
  static const String className = "TextButton";
  mDefSet = mDefSet->GetDefinitionSet(className);
  mIcon = CreateAttached<Element>(icon);
}

void IconPreview::UpdateTransform()
{
  Vec2 background = mSize;
  Vec2 iconSize = mIcon->GetMinSize();
  iconSize = Math::Min(iconSize, mMinSize);

  Vec3 p = GetCenterPosition(background, iconSize);
  mIcon->SetSize(iconSize);
  mIcon->SetTranslation(p);
  PreviewWidget::UpdateTransform();
}

SoundCuePreview::SoundCuePreview(PreviewWidgetInitializer& initializer) : IconPreview(initializer, "SoundIcon")
{
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
}

void SoundCuePreview::OnLeftClick(MouseEvent* event)
{
  if (SoundCue* cue = mObject.Get<SoundCue*>())
    cue->Preview();
}

CameraViewportDrawer::CameraViewportDrawer(Composite* parent, Cog* cameraObject) :
    Widget(parent),
    mCameraObject(cameraObject)
{
}

void CameraViewportDrawer::SetSize(Vec2 newSize)
{
  Widget::SetSize(newSize);
  if (Cog* cameraObject = mCameraObject)
  {
    if (CameraViewport* cameraViewport = cameraObject->has(CameraViewport))
    {
      cameraViewport->SetResolutionOrAspect(Math::ToIntVec2(mSize));
    }
  }
}

void CameraViewportDrawer::RenderUpdate(
    ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, WidgetRect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  if (Cog* cameraObject = mCameraObject)
  {
    if (CameraViewport* cameraViewport = cameraObject->has(CameraViewport))
    {
      if (Texture* texture = cameraViewport->mFinalTexture)
      {
        Vec2 size = mSize;
        Vec4 color(1, 1, 1, 1);

        ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, texture);
        frameBlock.mRenderQueues->AddStreamedQuad(
            viewNode, Vec3(0, 0, 0), Vec3(size, 0), Vec2(0, 0), Vec2(1, 1), color);
      }
    }
  }
}

SpacePreviewMouseDrag::SpacePreviewMouseDrag(Mouse* mouse, SpacePreview* preview) :
    MouseManipulation(mouse, preview),
    mPreview(preview)
{
}

void SpacePreviewMouseDrag::OnMouseMove(MouseEvent* event)
{
  float sensitivity = Math::DegToRad(1.0f) * 0.5f;
  mPreview->mHorizontalAngle += event->Movement.x * sensitivity;
  mPreview->mVerticalAngle += event->Movement.y * sensitivity;
  mPreview->mVerticalAngle = Math::Clamp(mPreview->mVerticalAngle, -Math::cPi * 0.5f, Math::cPi * 0.5f);
  mPreview->UpdateCameraPosition();
}

void SpacePreviewMouseDrag::OnRightMouseUp(MouseEvent* event)
{
  CloseAndReturnFocus();
}

SpacePreview::SpacePreview(PreviewWidgetInitializer& initializer, StringParam objectArchetype, Cog* objectToView) :
    PreviewWidget(initializer)
{
  mPreviewAnimate = PreviewAnimate::None;

  Space* space = nullptr;
  if (objectToView)
  {
    // Sharing a space
    mOwnsSpace = false;
    space = objectToView->GetSpace();
  }
  else
  {
    // Create a local space
    mOwnsSpace = true;

    // By default, proxy the objects in the preview space
    uint creationFlags = CreationFlags::Editing | CreationFlags::Preview;

    // Unless specified otherwise by the dev config
    if (DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig))
    {
      if (devConfig->mProxyObjectsInPreviews == false)
        creationFlags = 0;
    }

    // Create the space
    GameSession* editorGameSession = Z::gEditor->GetEditGameSession();
    Archetype* spaceArchetype = ArchetypeManager::Find(CoreArchetypes::DefaultSpace);
    space = editorGameSession->CreateSpaceFlags(spaceArchetype, creationFlags);

    String spaceName = BuildString("ResourcePreview ", initializer.Name);
    space->SetName(Cog::SanitizeName(spaceName));

    // Do not update the space
    space->has(TimeSpace)->SetPaused(true);
  }

  uint creationFlags = CreationFlags::Editing | CreationFlags::Preview;
  Cog* camera =
      Z::gFactory->Create(space, CoreArchetypes::PreviewCamera, creationFlags, Z::gEditor->GetEditGameSession());
  mCameraViewportDrawer = new CameraViewportDrawer(this, camera);
  if (GravityEffect* g = space->has(GravityEffect))
    g->SetActive(false);

  Component* renderer = camera->GetComponentByName("ForwardRenderer");
  if (renderer != nullptr)
  {
    Texture* skybox = TextureManager::FindOrNull("WhiteSkybox");
    renderer->SetProperty("Skybox", skybox);
  }

  // Create preview object if necessary
  if (objectToView == nullptr)
  {
    if (!objectArchetype.Empty())
    {
      objectToView = space->CreateAt(objectArchetype, Vec3(0, 0, 0));
      if (objectToView != nullptr)
      {
        // This is being used in the preview space, apply its modifications so
        // that they can be modified in the Archetypes context
        Archetype* archetype = objectToView->GetArchetype();
        archetype->GetLocalCachedModifications().ApplyModificationsToObject(objectToView);
      }
    }
  }

  mHasGraphical = false;
  if (objectToView != nullptr)
    mHasGraphical = objectToView->has(Graphical) != nullptr;

  mObject = objectToView;
  mCamera = camera;
  mSpace = space;

  UpdateViewDistance();
  SetInteractive(mInteractive);
}

SpacePreview::~SpacePreview()
{
  if (mOwnsSpace)
    mSpace.SafeDestroy();

  mCamera.SafeDestroy();
}

void SpacePreview::SetInteractive(bool interactive)
{
  mInteractive = interactive;
  if (mInteractive)
  {
    ConnectThisTo(this, Events::RightMouseDown, OnRightMouseDown);
    ConnectThisTo(this, Events::MouseScroll, OnMouseScroll);
  }
  else
  {
    GetDispatcher()->DisconnectEvent(Events::RightMouseDown, this);
    GetDispatcher()->DisconnectEvent(Events::MouseScroll, this);
  }
}

void SpacePreview::OnRightMouseDown(MouseEvent* event)
{
  // The space preview was right clicked on, create
  event->Handled = true;
  new SpacePreviewMouseDrag(event->EventMouse, this);
}

void SpacePreview::OnMouseScroll(MouseEvent* event)
{
  // How many steps it takes to jump a level
  const float cScrollExponentScalar = 0.1f;

  float expLookDistance = Math::Log(mLookAtDistance);
  expLookDistance += -event->Scroll.y * cScrollExponentScalar;
  mLookAtDistance = Math::Exp(expLookDistance);
  UpdateCameraPosition();
}

void SpacePreview::OnDestroy()
{
  PreviewWidget::OnDestroy();
}

void SpacePreview::OnUpdate(UpdateEvent* updateEvent)
{
  Cog* object = mObject;

  if (!object)
    return;

  bool mouseOverUpdate = (IsMouseOver() && mPreviewAnimate == PreviewAnimate::MouseOver);
  bool alwaysUpdate = (mPreviewAnimate == PreviewAnimate::Always);

  if (mouseOverUpdate || alwaysUpdate)
  {
    Debug::ActiveDrawSpace drawSpace(object->GetSpace()->GetId().Id);

    // We only want to debug draw the components if there's no
    // graphical components
    // if(!mHasGraphical)
    {
      forRange (Component* component, object->GetComponents())
      {
        component->DebugDraw();
      }
    }
  }
}

Vec2 SpacePreview::GetMinSize()
{
  return PreviewWidgetUi::SpacePreviewSize;
}

void SpacePreview::UpdateViewDistance()
{
  Cog* object = mObject;
  if (object)
  {
    Aabb aabb = GetAabb(object);
    mLookAtDistance = GetViewDistance(aabb);

    Cog* camera = mCamera;

    Camera* cameraComponent = camera->has(Camera);
    if (cameraComponent)
    {
      float nearWithBias = cameraComponent->mNearPlane + 0.01f;
      if (mLookAtDistance < nearWithBias)
        mLookAtDistance = nearWithBias;
    }

    Vec3 cameraDirection = cameraComponent->GetWorldDirection().Normalized();
    mVerticalAngle = -Math::ArcSin(cameraDirection.y);
    mHorizontalAngle = Math::ArcTan2(cameraDirection.x, -cameraDirection.z);
    Vec3 cameraPos = -cameraDirection * mLookAtDistance;
    camera->has(Transform)->SetTranslation(cameraPos);
  }
}

void SpacePreview::UpdateViewDistance(Vec3 viewDirection)
{
  Cog* camera = mCamera;
  Transform* transform = camera->has(Transform);
  transform->SetTranslation(-viewDirection.Normalized());
  SetRotationLookAt(transform, Vec3::cZero, Vec3::cYAxis, Facing::NegativeZ);

  Cog* object = mObject;
  if (object != nullptr && object->has(Transform) != nullptr)
  {
    Quat rotation = transform->GetWorldRotation().Inverted();

    Aabb aabb = GetAabb(object);
    aabb.Transform(Math::ToMatrix3(rotation));

    object->has(Transform)->SetWorldTranslation(-aabb.GetCenter());

    float size = Math::Max(aabb.mMax.x - aabb.mMin.x, aabb.mMax.y - aabb.mMin.y);
    mLookAtDistance = size / (2.0f * Math::Tan(Math::DegToRad(22.5f)));
    mLookAtDistance += (aabb.mMax.z - aabb.mMin.z) * 0.5f;

    Camera* cameraComponent = camera->has(Camera);
    Vec3 cameraDirection = viewDirection.Normalized();
    mVerticalAngle = -Math::ArcSin(cameraDirection.y);
    mHorizontalAngle = Math::ArcTan2(cameraDirection.x, -cameraDirection.z);
    mLookAtDistance = Math::Clamp(mLookAtDistance, cameraComponent->mNearPlane, cameraComponent->mFarPlane);
    transform->SetTranslation(-cameraDirection * mLookAtDistance);
  }
}

void SpacePreview::UpdateCameraPosition()
{
  mVerticalAngle = Math::Clamp(mVerticalAngle, -Math::cPi * 0.5f, Math::cPi * 0.5f);

  Vec3 cameraDirection;
  cameraDirection.x = Math::Sin(mHorizontalAngle) * Math::Cos(mVerticalAngle);
  cameraDirection.y = -Math::Sin(mVerticalAngle);
  cameraDirection.z = -Math::Cos(mHorizontalAngle) * Math::Cos(mVerticalAngle);

  Vec3 cameraRight;
  cameraRight.x = Math::Sin(mHorizontalAngle + Math::cPi * 0.5f);
  cameraRight.y = 0.0f;
  cameraRight.z = -Math::Cos(mHorizontalAngle + Math::cPi * 0.5f);

  Vec3 cameraUp = Cross(cameraRight, cameraDirection).Normalized();
  Vec3 cameraPos = -cameraDirection.Normalized() * mLookAtDistance;
  Quat rotation = Math::ToQuaternion(cameraDirection, cameraUp, cameraRight);

  Cog* camera = mCamera;
  Transform* transform = camera->has(Transform);
  transform->SetRotation(rotation);
  transform->SetTranslation(cameraPos);
  transform->UpdateAll();
}

void SpacePreview::AnimatePreview(PreviewAnimate::Enum value)
{
  mPreviewAnimate = value;
  Space* space = mSpace;
  if (space)
  {
    // Attempt to disconnect the event before connecting to it to avoid
    // duplicate event connections
    space->GetDispatcher()->DisconnectEvent(Events::FrameUpdate, this);
    ConnectThisTo(space, Events::FrameUpdate, OnUpdate);
  }
}

void SpacePreview::UpdateTransform()
{
  mCameraViewportDrawer->SetSize(mSize);
  PreviewWidget::UpdateTransform();
}

MaterialPreview::MaterialPreview(PreviewWidgetInitializer& initializer) : SpacePreview(initializer)
{
  Material* material = initializer.Object.Get<Material*>();
  if (Cog* object = mObject)
  {
    object->has(Model)->SetMesh(MeshManager::FindOrNull("Sphere"));
    object->has(Model)->SetMaterial(material);
  }
  UpdateViewDistance(Vec3(-1.0f));
}

MeshPreview::MeshPreview(PreviewWidgetInitializer& initializer) : SpacePreview(initializer)
{
  Mesh* mesh = initializer.Object.Get<Mesh*>();
  if (Cog* object = mObject)
  {
    object->has(Model)->SetMesh(mesh);
    object->has(Model)->SetMaterial(MaterialManager::GetDefault());
  }
  UpdateViewDistance(Vec3(-1.0f));
}

PhysicsMeshPreview::PhysicsMeshPreview(PreviewWidgetInitializer& initializer) :
    SpacePreview(initializer, CoreArchetypes::Transform)
{
  PhysicsMesh* mesh = initializer.Object.Get<PhysicsMesh*>();
  if (Cog* object = mObject)
  {
    object->AddComponentByName("MeshCollider");
    MeshCollider* collider = object->has(MeshCollider);
    collider->SetPhysicsMesh(mesh);
  }

  UpdateViewDistance(Vec3(-1.0f));
  // We want to be called for update every frame so we debug draw
  AnimatePreview(PreviewAnimate::Always);
}

ConvexMeshPreview::ConvexMeshPreview(PreviewWidgetInitializer& initializer) :
    SpacePreview(initializer, CoreArchetypes::Transform)
{
  ConvexMesh* mesh = initializer.Object.Get<ConvexMesh*>();
  if (Cog* object = mObject)
  {
    object->AddComponentByName("ConvexMeshCollider");
    ConvexMeshCollider* collider = object->has(ConvexMeshCollider);
    collider->SetConvexMesh(mesh);
  }

  UpdateViewDistance(Vec3(-1.0f));
  // We want to be called for update every frame so we debug draw
  AnimatePreview(PreviewAnimate::Always);
}

MultiConvexMeshPreview::MultiConvexMeshPreview(PreviewWidgetInitializer& initializer) :
    SpacePreview(initializer, CoreArchetypes::Transform)
{
  MultiConvexMesh* mesh = initializer.Object.Get<MultiConvexMesh*>();
  if (Cog* object = mObject)
  {
    object->AddComponentByName("MultiConvexMeshCollider");
    MultiConvexMeshCollider* collider = object->has(MultiConvexMeshCollider);
    collider->SetMesh(mesh);
  }
  UpdateViewDistance();
  // We want to be called for update every frame so we debug draw
  AnimatePreview(PreviewAnimate::Always);

  // To make it easier to see the debug drawing, change to orthographic and zoom
  // in
  Camera* camera = mCamera.has(Camera);
  if (camera != nullptr)
  {
    camera->mPerspectiveMode = PerspectiveMode::Orthographic;
    camera->mSize = 2.0f;
  }
}

ArchetypePreview::ArchetypePreview(PreviewWidgetInitializer& initializer) :
    SpacePreview(initializer, initializer.Object.Get<Archetype*>()->ResourceIdName)
{
  UpdateViewDistance(Vec3(-1.0f));

  if (Cog* cog = (Cog*)mObject)
  {
    Aabb aabb = GetAabb(cog);
    // Our asset has a thin AABB on the z axis and is most likely a 2D asset
    // Change the camera preview to be from the front of the object
    if (aabb.GetExtents().z < cSpritePreviewThreshold)
      UpdateViewDistance(Vec3(0.0f, 0.0f, -1.0f));
  }
}

Handle ArchetypePreview::GetEditObject()
{
  return (Cog*)mObject;
}

SpriteSourcePreview::SpriteSourcePreview(PreviewWidgetInitializer& initializer) :
    SpacePreview(initializer, CoreArchetypes::Sprite)
{
  Space* space = mSpace;
  if (space)
  {
    SpriteSource* spriteSource = initializer.Object.Get<SpriteSource*>();
    mObject.has(Sprite)->SetSpriteSource(spriteSource);
  }
  UpdateViewDistance(-Vec3::cZAxis);
}

AnimationPreview::AnimationPreview(PreviewWidgetInitializer& initializer) : SpacePreview(initializer, "")
{
  // Destroy the object that was created by the space preview
  mObject.SafeDestroy();

  Animation* animation = initializer.Object.Get<Animation*>();
  mObject = CreateAnimationPreview(mSpace, animation);

  mAnimation = animation;

  UpdateViewDistance(Vec3(-1.0f));

  if (Cog* cog = (Cog*)mObject)
  {
    Aabb aabb = GetAabb(cog);
    // Our asset has a thin AABB on the z axis and is most likely a 2D asset
    // Change the camera preview to be from the front of the object
    if (aabb.GetExtents().z < 0.1f)
      UpdateViewDistance(Vec3(0.0f, 0.0f, -1.0f));
  }
}

Handle AnimationPreview::GetEditObject()
{
  if (Cog* object = mObject)
    return object;

  return Handle();
}

void AnimationPreview::OnReload(ResourceEvent* event)
{
  if (event->EventResource == (Resource*)mAnimation)
  {
    Space* space = mSpace;
    if (space)
    {
      mObject.SafeDestroy();
      mObject = CreateAnimationPreview(mSpace, mAnimation);
      UpdateViewDistance();

      ObjectEvent e(mObject);
      this->DispatchBubble(Events::PreviewObjectChanged, &e);
    }
  }
}

CogPreview::CogPreview(PreviewWidgetInitializer& initializer) :
    SpacePreview(initializer, String(), initializer.Object.Get<Cog*>())
{
  UpdateViewDistance(Vec3(-1.0f));
}

TexturePreview::TexturePreview(PreviewWidgetInitializer& initializer) : PreviewWidget(initializer)
{
  Texture* texture = initializer.Object.Get<Texture*>();
  mImage = new TextureView(this);
  mImage->SetTexture(texture);
}

void TexturePreview::UpdateTransform()
{
  mImage->SetSize(mSize);
  PreviewWidget::UpdateTransform();
}

// preview
FontPreview::FontPreview(PreviewWidgetInitializer& initializer) : SpacePreview(initializer, CoreArchetypes::SpriteText)
{
  Font* font = initializer.Object.Get<Font*>();

  Space* space = mSpace;
  if (space)
  {
    SpriteText* spriteText = mObject.has(SpriteText);
    spriteText->SetFont(font);
    spriteText->SetFontSize(64);
    spriteText->SetText("Aa");
  }

  UpdateViewDistance(-Vec3::cZAxis);
}

TilePaletteSourcePreview::TilePaletteSourcePreview(PreviewWidgetInitializer& initializer) : PreviewWidget(initializer)
{
  mSource = initializer.Object.Get<TilePaletteSource*>();
  mTilePaletteView = new TilePaletteView(this, nullptr);
  mTilePaletteView->SetTilePalette(mSource);
  mTilePaletteView->mSelectionBorder->SetVisible(false);
  // the base tile view has scroll bars which in the preview cover the image
  // so disable them for the preview
  mTilePaletteView->mScrollArea->DisableScrollBar(0);
  mTilePaletteView->mScrollArea->DisableScrollBar(1);
}

void TilePaletteSourcePreview::SizeToContents()
{
  if (mSource.IsNull())
  {
    Destroy();
    return;
  }

  // scale the tile view to fit within the preview widget they are displayed
  // widget size and tile size are in pixels
  IntVec2 paletteTiles = mSource->GetTileDimensions();
  if (paletteTiles.x > paletteTiles.y)
    mTilePaletteView->mTileSize = (int)Math::Floor(mSize.x / float(paletteTiles.x));
  else
    mTilePaletteView->mTileSize = (int)Math::Floor(mSize.y / float(paletteTiles.y));
}

void TilePaletteSourcePreview::UpdateTransform()
{
  if (mSource.IsNull())
  {
    Destroy();
    return;
  }

  SizeToContents();
  mTilePaletteView->SetSize(mSize);
  PreviewWidget::UpdateTransform();
}

ColorGradientPreview::ColorGradientPreview(PreviewWidgetInitializer& initializer) : PreviewWidget(initializer)
{
  mMinSize = PreviewWidgetUi::ColorGradientSize;

  mGradientBlockBuffer = new PixelBuffer(Color::Black, 256, 30);

  // Create the display and set the texture
  mGradientBlockDisplay = new TextureView(this);
  mGradientBlockDisplay->SetTexture(mGradientBlockBuffer->Image);

  ColorGradient* gradient = initializer.Object.Get<ColorGradient*>();
  DrawColorGradient(gradient, mGradientBlockBuffer);
}

ColorGradientPreview::~ColorGradientPreview()
{
  SafeDelete(mGradientBlockBuffer);
}

void ColorGradientPreview::UpdateTransform()
{
  mGradientBlockDisplay->SetTranslation(Vec3::cZero);
  mGradientBlockDisplay->SetSize(mSize);
  PreviewWidget::UpdateTransform();
}

Vec2 ColorGradientPreview::GetHalfSize()
{
  return GetMinSize() * 0.75f;
}

SampleCurveDrawer::SampleCurveDrawer(Composite* parent, HandleParam object) : Widget(parent)
{
  mObject = object;
  parent->SetClipping(true);
}

void SampleCurveDrawer::AddCurve(ViewBlock& viewBlock,
                                 FrameBlock& frameBlock,
                                 WidgetRect clipRect,
                                 SampleCurve* curveObject)
{
  Array<StreamedVertex> lines;
  Array<StreamedVertex> triangles;

  // The color of the curve
  Vec4 color = ToFloatColor(Color::Orange);

  // Create the curve
  Vec3Array curve;
  curveObject->GetCurve(curve);

  // If there are no segments, do nothing
  if (curve.Size() < 2)
    return;

  for (uint i = 0; i < curve.Size() - 1; ++i)
  {
    Vec2 p1(curve[i].x, curve[i].y);
    Vec2 p2(curve[i + 1].x, curve[i + 1].y);

    p1.y = (1.0f - p1.y);
    p2.y = (1.0f - p2.y);
    p1 *= mSize;
    p2 *= mSize;

    if (p1.x == p2.x)
    {
      // Vertical segments
      lines.PushBack(StreamedVertex(SnapToPixels(Vec3(p1)), Vec2::cZero, ToFloatColor(Color::Black)));
      lines.PushBack(StreamedVertex(SnapToPixels(Vec3(p2)), Vec2::cZero, ToFloatColor(Color::Black)));
    }
    else
    {
      // Curve segments
      lines.PushBack(StreamedVertex(SnapToPixels(Vec3(p1)), Vec2::cZero, color));
      lines.PushBack(StreamedVertex(SnapToPixels(Vec3(p2)), Vec2::cZero, color));
    }
  }

  CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);
}

void SampleCurveDrawer::RenderUpdate(
    ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, WidgetRect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  SampleCurve* sampleCurve = mObject.Get<SampleCurve*>();
  if (sampleCurve == nullptr)
    return;

  AddCurve(viewBlock, frameBlock, clipRect, sampleCurve);
}

// SampleCurvePreview
SampleCurvePreview::SampleCurvePreview(PreviewWidgetInitializer& initializer) : PreviewWidget(initializer)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(WindowUi::BackgroundColor);
  mGraph = new GraphWidget(this);
  mGraph->mDrawLabels = false;
  mDrawer = new SampleCurveDrawer(this, initializer.Object);
}

void SampleCurvePreview::UpdateTransform()
{
  SampleCurve* curve = mObject.Get<SampleCurve*>();
  if (curve == nullptr)
  {
    PreviewWidget::UpdateTransform();
    return;
  }
  mBackground->SetSize(mSize);
  mGraph->SetWidthRange(curve->GetWidthMin(), curve->GetWidthMax());
  mGraph->SetHeightRange(curve->GetHeightMin(), curve->GetHeightMax());
  mGraph->SetSize(mSize);
  mDrawer->SetSize(mSize);

  PreviewWidget::UpdateTransform();
}

ResourceTablePreview::ResourceTablePreview(PreviewWidgetInitializer& initializer) : PreviewWidget(initializer)
{
  mGroup = new PreviewWidgetGroup(this);
  ResourceTable* table = initializer.Object.Get<ResourceTable*>();

  // Prevent infinite loops of resource tables of resource tables
  if (table->mResourceType == ZilchTypeId(ResourceTable)->Name)
    return;

  uint size = table->Size();
  size = Math::Min(size, (uint)9);
  for (uint i = 0; i < size; ++i)
  {
    Resource* resource = (*table)[i]->GetResource();
    if (resource)
      mGroup->AddPreviewWidget(resource->Name, resource, PreviewImportance::Simple);
  }
}

void ResourceTablePreview::AnimatePreview(PreviewAnimate::Enum value)
{
  mGroup->AnimatePreview(value);
}

void ResourceTablePreview::UpdateTransform()
{
  mGroup->SetSize(mSize);
  PreviewWidget::UpdateTransform();
}

SpaceArchetypePreview::SpaceArchetypePreview(PreviewWidgetInitializer& initializer, Archetype* archetype) :
    IconPreview(initializer, "Level")
{
  // Space archetype
  Space* space =
      Z::gFactory->CreateSpace(archetype->ResourceIdName, CreationFlags::Editing, Z::gEditor->GetEditGameSession());
  mObject = space;
}

SpaceArchetypePreview::~SpaceArchetypePreview()
{
  mObject.SafeDestroy();
}

Handle SpaceArchetypePreview::GetEditObject()
{
  Space* space = mObject;
  return space;
}

GameArchetypePreview::GameArchetypePreview(PreviewWidgetInitializer& initializer, Archetype* archetype) :
    IconPreview(initializer, "Level")
{
  // If this is the same archetype as the main game session reuse
  // the game session object
  if (Z::gEditor->GetEditGameSession()->GetArchetype() == archetype)
  {
    mObject = Z::gEditor->GetEditGameSession();
    UsingEditorGameSession = true;
  }
  else
  {
    mObject = (GameSession*)Z::gFactory->CreateCheckedType(
        ZilchTypeId(GameSession), nullptr, archetype->Name, CreationFlags::Editing | CreationFlags::Preview, nullptr);
    UsingEditorGameSession = false;
  }
}

GameArchetypePreview::~GameArchetypePreview()
{
  if (!UsingEditorGameSession)
    mObject.SafeDestroy();
}

Handle GameArchetypePreview::GetEditObject()
{
  GameSession* game = mObject;
  return game;
}

PreviewWidget* CreateArchetypePreviewWidget(PreviewWidgetInitializer& initializer)
{
  TemporaryDoNotifyOverride doNotifyOverride(IgnoreDoNotify);

  PreviewWidget* preview = nullptr;
  Archetype* archetype = initializer.Object.Get<Archetype*>();

  if (archetype->mStoredType == ZilchTypeId(Cog))
  {
    // Basic archetype
    preview = new ArchetypePreview(initializer);
  }
  else if (archetype->mStoredType == ZilchTypeId(Space))
  {
    preview = new SpaceArchetypePreview(initializer, archetype);
  }
  else if (archetype->mStoredType == ZilchTypeId(GameSession))
  {
    preview = new GameArchetypePreview(initializer, archetype);
  }
  else
  {
    preview = new IconPreview(initializer, "Level");
  }

  return preview;
}

void RegisterEditorTileViewWidgets()
{
  PreviewWidgetFactory* previewFactory = PreviewWidgetFactory::GetInstance();

  previewFactory->Creators[CoreArchetypes::EmptyTile] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<EmptyPreview>);

  // Core resource types
  previewFactory->Creators["Archetype"] = PreviewWidgetCreator(PreviewImportance::High, CreateArchetypePreviewWidget);
  previewFactory->Creators["Material"] =
      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<MaterialPreview>);
  previewFactory->Creators["Mesh"] = PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<MeshPreview>);
  previewFactory->Creators["PhysicsMesh"] =
      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<PhysicsMeshPreview>);
  previewFactory->Creators["ConvexMesh"] =
      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<ConvexMeshPreview>);
  previewFactory->Creators["MultiConvexMesh"] =
      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<MultiConvexMeshPreview>);
  previewFactory->Creators["Texture"] =
      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<TexturePreview>);
  previewFactory->Creators["Font"] = PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<FontPreview>);
  previewFactory->Creators["SpriteSource"] =
      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<SpriteSourcePreview>);
  previewFactory->Creators["TilePaletteSource"] =
      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<TilePaletteSourcePreview>);
  previewFactory->Creators["ColorGradient"] =
      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<ColorGradientPreview>);
  previewFactory->Creators["SampleCurve"] =
      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<SampleCurvePreview>);
  previewFactory->Creators["ResourceTable"] =
      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<ResourceTablePreview>);
  previewFactory->Creators["Animation"] =
      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<AnimationPreview>);

  previewFactory->Creators["CollisionGroup"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<PhysicsPreview>);
  previewFactory->Creators["CollisionTable"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<PhysicsPreview>);
  previewFactory->Creators["PhysicsMaterial"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<PhysicsPreview>);
  previewFactory->Creators["PhysicsSolverConfig"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<PhysicsPreview>);

  // Simple resource types
  previewFactory->Creators["ZilchScript"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<ScriptPreview>);
  previewFactory->Creators["ZilchFragment"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<ScriptPreview>);
  previewFactory->Creators["TextBlock"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<ScriptPreview>);
  previewFactory->Creators["ZilchPluginSource"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<ScriptPreview>);
  previewFactory->Creators["Level"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<LevelPreview>);
  previewFactory->Creators["Sound"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<SoundPreview>);
  previewFactory->Creators["SoundCue"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<SoundCuePreview>);
  previewFactory->Creators["SoundTag"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<SoundPreview>);
  previewFactory->Creators["SoundAttenuator"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<SoundPreview>);
  previewFactory->Creators["RenderGroup"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<RenderGroupPreview>);

  previewFactory->Creators["NetPropertyConfig"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<NetworkingPreview>);
  previewFactory->Creators["NetChannelConfig"] =
      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<NetworkingPreview>);

  // Cog Object
  previewFactory->Creators["Cog"] = PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<CogPreview>);
}

} // namespace Zero
