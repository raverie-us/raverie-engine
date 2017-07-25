///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace PreviewWidgetUi
{
const cstr cLocation = "EditorUi/PreviewWidgets";
Tweakable(Vec2, ColorGradientSize, Pixels(170,30), cLocation);
Tweakable(Vec2, SpacePreviewSize, Pixels(200, 200), cLocation);

}

//-------------------------------------------------------------------- Icon Item
class IconPreview : public PreviewWidget
{
public:
  Element* mIcon;

  //****************************************************************************
  IconPreview(PreviewWidgetInitializer& initializer, StringParam icon)
    : PreviewWidget(initializer)
  {
    static const String className = "TextButton";
    mDefSet = mDefSet->GetDefinitionSet(className);
    mIcon = CreateAttached<Element>(icon);
  }

  //****************************************************************************
  void UpdateTransform()
  {
    Vec2 background = mSize;
    Vec2 iconSize = mIcon->GetMinSize();
    iconSize = Math::Min(iconSize, mMinSize);

    Vec3 p = GetCenterPosition(background, iconSize);
    mIcon->SetSize(iconSize);
    mIcon->SetTranslation(p);
    PreviewWidget::UpdateTransform();
  }
};

//------------------------------------------------------------------ Script Item
class ScriptPreview : public IconPreview
{
public:
  //****************************************************************************
  ScriptPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "ScriptIcon")
  {
    //
  }
};

//------------------------------------------------------------------ Script Item
class RenderGroupPreview : public IconPreview
{
public:
  //****************************************************************************
  RenderGroupPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "RenderGroupIcon")
  {
    //
  }
};

//------------------------------------------------------------------ Script Item
class SoundPreview : public IconPreview
{
public:
  //****************************************************************************
  SoundPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "SoundIcon")
  {
    //
  }
};

//------------------------------------------------------------------ Script Item
class NetworkingPreview : public IconPreview
{
public:
  //****************************************************************************
  NetworkingPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "NetworkingIcon")
  {
    //
  }
};

//------------------------------------------------------------------ Script Item
class PhysicsPreview : public IconPreview
{
public:
  //****************************************************************************
  PhysicsPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "PhysicsIcon")
  {
    //
  }
};

//------------------------------------------------------------------- Level Item
class LevelPreview : public IconPreview
{
public:
  //****************************************************************************
  LevelPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "LevelIcon")
  {
    //
  }
};

//--------------------------------------------------------------- Sound Cue Item
class SoundCuePreview : public IconPreview
{
public:
  typedef SoundCuePreview ZilchSelf;

  //****************************************************************************
  SoundCuePreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "SoundIcon")
  {
    ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  }

  //****************************************************************************
  void OnLeftClick(MouseEvent* event)
  {
    if(SoundCue* cue = mObject.Get<SoundCue*>())
      cue->Preview();
  }
};


//-------------------------------------------------------- Physics Material Item
class PhysicsMaterialPreview : public IconPreview
{
public:
  //****************************************************************************
  PhysicsMaterialPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "PhysicsMaterial")
  {
    //
  }
};

//------------------------------------------------------------------- Empty Item
class EmptyPreview : public IconPreview
{
public:
  //****************************************************************************
  EmptyPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "LargeFolder")
  {
    mIcon->SetColor(Vec4(0,0,0,0));
  }
};

class CameraViewportDrawer : public Widget
{
public:
  HandleOf<Cog> mCameraObject;

  CameraViewportDrawer(Composite* parent, Cog* cameraObject)
    : Widget(parent)
    , mCameraObject(cameraObject)
  {
  }

  void SetSize(Vec2 newSize)
  {
    Widget::SetSize(newSize);
    if (Cog* cameraObject = mCameraObject)
    {
      if (CameraViewport* cameraViewport = cameraObject->has(CameraViewport))
      {
        cameraViewport->SetResolutionOrAspect(mSize);
      }
    }
  }

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override
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
          frameBlock.mRenderQueues->AddStreamedQuad(viewNode, Vec3(0, 0, 0), Vec3(size, 0), Vec2(0, 0), Vec2(1, 1), color);
        }
      }
    }
  }
};

//---------------------------------------------------------------- World Preview
class SpacePreview : public PreviewWidget
{
public:
  typedef SpacePreview ZilchSelf;
  PreviewAnimate::Enum mPreviewAnimate;
  CameraViewportDrawer* mCameraViewportDrawer;
  HandleOf<Space> mSpace;
  bool mOwnsSpace;
  bool mHasGraphical;
  CogId mCamera;
  CogId mObject;

  //****************************************************************************
  SpacePreview(PreviewWidgetInitializer& initializer, StringParam objectArchetype = CoreArchetypes::Default, Cog* objectToView = nullptr)
    : PreviewWidget(initializer)
  {
    mPreviewAnimate = PreviewAnimate::None;

    Space* space = nullptr;
    if(objectToView)
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
      if(DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig))
      {
        if(devConfig->mProxyObjectsInPreviews == false)
          creationFlags = 0;
      }

      // Create the space
      GameSession* editorGameSession = Z::gEditor->GetEditGameSession();
      Archetype* spaceArchetype = ArchetypeManager::Find(CoreArchetypes::DefaultSpace);
      space = editorGameSession->CreateSpaceFlags(spaceArchetype, creationFlags);

      String spaceName = BuildString("ResourcePreview ", initializer.Name);
      space->SetName(Cog::SanatizeName(spaceName));
      
      // Do not update the space
      space->has(TimeSpace)->SetPaused(true);
    }
    
    uint creationFlags = CreationFlags::Editing | CreationFlags::Preview;
    Cog* camera = Z::gFactory->Create(space, CoreArchetypes::PreviewCamera, creationFlags, Z::gEditor->GetEditGameSession());
    mCameraViewportDrawer = new CameraViewportDrawer(this, camera);
    if(GravityEffect* g = space->has(GravityEffect))
      g->SetActive(false);

    // Create preview object if necessary
    if(objectToView == nullptr)
    {
      if(!objectArchetype.Empty())
      {
        objectToView = space->CreateAt(objectArchetype, Vec3(0, 0, 0));

        // This is being used in the preview space, apply its modifications so that they can
        // be modified in the Archetypes context
        Archetype* archetype = objectToView->GetArchetype();
        archetype->GetLocalCachedModifications().ApplyModificationsToObject(objectToView);
      }
    }

    mHasGraphical = false;
    if (objectToView != nullptr)
      mHasGraphical = objectToView->has(Graphical) != nullptr;

    mObject = objectToView;
    mCamera = camera;
    mSpace = space;
  }

  //****************************************************************************
  void OnDestroy()
  {
    PreviewWidget::OnDestroy();
  }

  //****************************************************************************
  ~SpacePreview()
  {
    if(mOwnsSpace)
      mSpace.SafeDestroy();

    mCamera.SafeDestroy();
  }

  //****************************************************************************
  Vec2 GetMinSize() override
  {
    return PreviewWidgetUi::SpacePreviewSize;
  }

  //****************************************************************************
  void OnUpdate(UpdateEvent* updateEvent)
  {
    Cog* object = mObject;

    if(!object)
      return;

    bool mouseOverUpdate = (IsMouseOver() && mPreviewAnimate == PreviewAnimate::MouseOver);
    bool alwaysUpdate = (mPreviewAnimate == PreviewAnimate::Always);

    if(mouseOverUpdate ||  alwaysUpdate)
    {
      Debug::ActiveDrawSpace drawSpace(object->GetSpace()->GetId().Id);

      // We only want to debug draw the components if there's no
      // graphical components
      //if(!mHasGraphical)
      {
        forRange(Component* component, object->GetComponents())
        {
          component->DebugDraw();
        }
      }
    }
  }

  //****************************************************************************
  void UpdateViewDistance()
  {
    Cog* object = mObject;
    if(object)
    {
      Aabb aabb = GetAabb(object);
      float lookDistance = GetViewDistance(aabb);

      Cog* camera = mCamera;

      Camera* cameraComponent = camera->has(Camera);
      if(cameraComponent)
      {
        float nearWithBias = cameraComponent->mNearPlane + 0.01f;
        if(lookDistance < nearWithBias)
          lookDistance = nearWithBias;
      }

      Vec3 lookAt = aabb.GetCenter();
      camera->has(Transform)->SetTranslation(aabb.GetCenter() + Vec3(0, 0, lookDistance));
    }
  }

  void UpdateViewDistance(Vec3 viewDirection)
  {
    Cog* cameraCog = mCamera;
    Transform* transform = cameraCog->has(Transform);
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
      float distance = size / (2.0f * Math::Tan(Math::DegToRad(22.5f)));
      distance += (aabb.mMax.z - aabb.mMin.z) * 0.5f;

      Camera* camera = cameraCog->has(Camera);
      distance = Math::Clamp(distance, camera->mNearPlane, camera->mFarPlane);
      transform->SetTranslation(-viewDirection.Normalized() * distance);
    }
  }

  //****************************************************************************
  void AnimatePreview(PreviewAnimate::Enum value) override
  {
    mPreviewAnimate = value;
    Space* space = mSpace;
    if(value != PreviewAnimate::None && mOwnsSpace)
      space->has(TimeSpace)->SetPaused(false);
    if(space)
      ConnectThisTo(space, Events::FrameUpdate, OnUpdate);
  }

  //****************************************************************************
  void UpdateTransform()
  {
    mCameraViewportDrawer->SetSize(mSize);
    PreviewWidget::UpdateTransform();
  }
};

//----------------------------------------------------------- Material Grid Tile
class MaterialPreview : public SpacePreview
{
public:
  //****************************************************************************
  MaterialPreview(PreviewWidgetInitializer& initializer)
    : SpacePreview(initializer)
  {
    Material* material = initializer.Object.Get<Material*>();
    if(Cog* object = mObject)
    {
      object->has(Model)->SetMesh(MeshManager::FindOrNull("Sphere"));
      object->has(Model)->SetMaterial(material);
    }
    UpdateViewDistance(Vec3(-1.0f));
  }
};

//--------------------------------------------------------------- Mesh Grid Item
class MeshPreview : public SpacePreview
{
public:
  //****************************************************************************
  MeshPreview(PreviewWidgetInitializer& initializer)
    : SpacePreview(initializer)
  {
    Mesh* mesh = initializer.Object.Get<Mesh*>();
    if(Cog* object = mObject)
    {
      object->has(Model)->SetMesh(mesh);
      object->has(Model)->SetMaterial(MaterialManager::GetDefault());
    }
    UpdateViewDistance(Vec3(-1.0f));
  }
};

//------------------------------------------------------- Physics Mesh Grid Item
class PhysicsMeshPreview : public SpacePreview
{
public:
  //****************************************************************************
  PhysicsMeshPreview(PreviewWidgetInitializer& initializer)
    : SpacePreview(initializer, CoreArchetypes::Transform)
  {
    PhysicsMesh* mesh = initializer.Object.Get<PhysicsMesh*>();
    if(Cog* object = mObject)
    {
      object->AddComponentByName("MeshCollider");
      MeshCollider* collider = object->has(MeshCollider);
      collider->SetPhysicsMesh(mesh);
    }
    UpdateViewDistance();
    //We want to be called for update every frame so we debug draw
    AnimatePreview(PreviewAnimate::Always);

    //To make it easier to see the debug drawing, change to orthographic and zoom in
    Camera* camera = mCamera.has(Camera);
    if(camera != nullptr)
    {
      camera->mPerspectiveMode = PerspectiveMode::Orthographic;
      camera->mSize = 2.0f;
    }
    
  }
};

//------------------------------------------------------- Convex Mesh Grid Item
class ConvexMeshPreview : public SpacePreview
{
public:
  //****************************************************************************
  ConvexMeshPreview(PreviewWidgetInitializer& initializer)
    : SpacePreview(initializer, CoreArchetypes::Transform)
  {
    ConvexMesh* mesh = initializer.Object.Get<ConvexMesh*>();
    if(Cog* object = mObject)
    {
      object->AddComponentByName("ConvexMeshCollider");
      ConvexMeshCollider* collider = object->has(ConvexMeshCollider);
      collider->SetConvexMesh(mesh);
    }
    UpdateViewDistance();
    //We want to be called for update every frame so we debug draw
    AnimatePreview(PreviewAnimate::Always);

    //To make it easier to see the debug drawing, change to orthographic and zoom in
    Camera* camera = mCamera.has(Camera);
    if(camera != nullptr)
    {
      camera->mPerspectiveMode = PerspectiveMode::Orthographic;
      camera->mSize = 2.0f;
    }

  }
};

//------------------------------------------------------- Physics Mesh Grid Item
class MultiConvexMeshPreview : public SpacePreview
{
public:
  //****************************************************************************
  MultiConvexMeshPreview(PreviewWidgetInitializer& initializer)
    : SpacePreview(initializer, CoreArchetypes::Transform)
  {
    MultiConvexMesh* mesh = initializer.Object.Get<MultiConvexMesh*>();
    if(Cog* object = mObject)
    {
      object->AddComponentByName("MultiConvexMeshCollider");
      MultiConvexMeshCollider* collider = object->has(MultiConvexMeshCollider);
      collider->SetMesh(mesh);
    }
    UpdateViewDistance();
    //We want to be called for update every frame so we debug draw
    AnimatePreview(PreviewAnimate::Always);

    //To make it easier to see the debug drawing, change to orthographic and zoom in
    Camera* camera = mCamera.has(Camera);
    if(camera != nullptr)
    {
      camera->mPerspectiveMode = PerspectiveMode::Orthographic;
      camera->mSize = 2.0f;
    }
    
  }
};

//---------------------------------------------------------- Archetype Grid Item
class ArchetypePreview : public SpacePreview
{
public:
  typedef ArchetypePreview ZilchSelf;

  //****************************************************************************
  ArchetypePreview(PreviewWidgetInitializer& initializer)
    : SpacePreview(initializer, initializer.Object.Get<Archetype*>()->ResourceIdName)
  {
    UpdateViewDistance(Vec3(-1.0f));
  }
};

//------------------------------------------------------------- SpriteSourceTile
class SpriteSourcePreview : public SpacePreview
{
public:
  //****************************************************************************
  SpriteSourcePreview(PreviewWidgetInitializer& initializer)
    : SpacePreview(initializer, CoreArchetypes::Sprite)
  {
    Space* space = mSpace;
    if(space)
    {
      SpriteSource* spriteSource = initializer.Object.Get<SpriteSource*>();
      mObject.has(Sprite)->SetSpriteSource(spriteSource);
    }
    UpdateViewDistance(-Vec3::cZAxis);
  }
};

//---------------------------------------------------------- Animation Grid Item
class AnimationPreview : public SpacePreview
{
public:
  HandleOf<Animation> mAnimation;
  typedef AnimationPreview ZilchSelf;

  //****************************************************************************
  AnimationPreview(PreviewWidgetInitializer& initializer)
    : SpacePreview(initializer, "")
  {
    // Destroy the object that was created by the space preview
    mObject.SafeDestroy();

    Animation* animation = initializer.Object.Get<Animation*>();
    mObject = CreateAnimationPreview(mSpace, animation);

    mAnimation = animation;

    UpdateViewDistance();
  }

  //****************************************************************************
  Handle GetEditObject() override
  {
    if(Cog* object = mObject)
      return object;
    
    return Handle();
  }

  //****************************************************************************
  void OnReload(ResourceEvent* event)
  {
    if(event->EventResource == (Resource*)mAnimation)
    {
      Space* space = mSpace;
      if(space)
      {
        mObject.SafeDestroy();
        mObject = CreateAnimationPreview(mSpace, mAnimation);
        UpdateViewDistance();

        ObjectEvent e(mObject);
        this->DispatchBubble(Events::PreviewObjectChanged, &e);
      }
    }
  }
};

//------------------------------------------------------------------ CogGridItem
class CogPreview : public SpacePreview
{
public:
  //****************************************************************************
  CogPreview(PreviewWidgetInitializer& initializer)
    : SpacePreview(initializer, String(), initializer.Object.Get<Cog*>())
  {
    UpdateViewDistance(Vec3(-1.0f));
  }
};

//------------------------------------------------------------------ TextureTile
class TexturePreview : public PreviewWidget
{
public:
  TextureView* mImage;

  //****************************************************************************
  TexturePreview(PreviewWidgetInitializer& initializer)
    : PreviewWidget(initializer)
  {
    Texture* texture = initializer.Object.Get<Texture*>();
    mImage = new TextureView(this);
    mImage->SetTexture(texture);
  }

  //****************************************************************************
  void UpdateTransform()
  {
    mImage->SetSize(mSize);
    PreviewWidget::UpdateTransform();
  }
};

//------------------------------------------------------------------ Font preview
class FontPreview : public SpacePreview
{
public:
  //****************************************************************************
  FontPreview(PreviewWidgetInitializer& initializer)
    : SpacePreview(initializer, CoreArchetypes::SpriteText)
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
};

//------------------------------------------------------------------ TextureTile
class TilePaletteSourcePreview : public PreviewWidget
{
  TilePaletteView* mTilePaletteView;
  TilePaletteSource* mSource;
public:
  //****************************************************************************
  TilePaletteSourcePreview(PreviewWidgetInitializer& initializer)
    : PreviewWidget(initializer)
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

  //****************************************************************************
  void SizeToContents()
  {
    // scale the tile view to fit within the preview widget they are displayed
    // widget size and tile size are in pixels
    IntVec2 paletteTiles = mSource->GetTileDimensions();
    if (paletteTiles.x > paletteTiles.y)
      mTilePaletteView->mTileSize = (int)Math::Floor(mSize.x / float(paletteTiles.x));
    else
      mTilePaletteView->mTileSize = (int)Math::Floor(mSize.y / float(paletteTiles.y));
  }

  //****************************************************************************
  void UpdateTransform()
  {
    SizeToContents();
    mTilePaletteView->SetSize(mSize);
    PreviewWidget::UpdateTransform();
  }
};

//---------------------------------------------------------- Color Gradient Tile
class ColorGradientPreview : public PreviewWidget
{
public:
  //****************************************************************************
  ColorGradientPreview(PreviewWidgetInitializer& initializer)
    : PreviewWidget(initializer)
  {
    mMinSize = PreviewWidgetUi::ColorGradientSize;

    mGradientBlockBuffer = new PixelBuffer(Color::Black, 256, 30);

    // Create the display and set the texture
    mGradientBlockDisplay = new TextureView(this);
    mGradientBlockDisplay->SetTexture(mGradientBlockBuffer->Image);

    ColorGradient* gradient = initializer.Object.Get<ColorGradient*>();
    DrawColorGradient(gradient, mGradientBlockBuffer);
  }

  ~ColorGradientPreview()
  {
    SafeDelete(mGradientBlockBuffer);
  }

  //****************************************************************************
  void UpdateTransform() override
  {
    mGradientBlockDisplay->SetTranslation(Vec3::cZero);
    mGradientBlockDisplay->SetSize(mSize);
    PreviewWidget::UpdateTransform();
  }

  //****************************************************************************
  Vec2 GetHalfSize() override
  {
    return GetMinSize() * 0.75f;
  }

  PixelBuffer* mGradientBlockBuffer;
  TextureView* mGradientBlockDisplay;
};

//---------------------------------------------------------- Sample Curve Drawer
class SampleCurveDrawer : public Widget
{
public:
  Handle mObject;

  //****************************************************************************
  SampleCurveDrawer(Composite* parent, HandleParam object)
    : Widget(parent)
  {
    mObject = object;
    parent->SetClipping(true);
  }

  void AddCurve(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, SampleCurve* curveObject)
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

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
  {
    Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

    SampleCurve* sampleCurve = mObject.Get<SampleCurve*>();
    if (sampleCurve == nullptr)
      return;

    AddCurve(viewBlock, frameBlock, clipRect, sampleCurve);
  }
};

//------------------------------------------------------------ SampleCurvePreview
class SampleCurvePreview : public PreviewWidget
{
public:
  Element* mBackground;
  GraphWidget* mGraph;
  SampleCurveDrawer* mDrawer;

  //****************************************************************************
  SampleCurvePreview(PreviewWidgetInitializer& initializer)
    : PreviewWidget(initializer)
  {
    mBackground = CreateAttached<Element>(cWhiteSquare);
    mBackground->SetColor(WindowUi::BackgroundColor);
    mGraph = new GraphWidget(this);
    mGraph->mDrawLabels = false;
    mDrawer = new SampleCurveDrawer(this, initializer.Object);
  }

  //****************************************************************************
  void UpdateTransform() override
  {
    SampleCurve* curve = mObject.Get<SampleCurve*>();
    if(curve == nullptr)
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
};

//------------------------------------------------------- Resource Table Preview
class ResourceTablePreview : public PreviewWidget
{
public:
  PreviewWidgetGroup* mGroup;

  //****************************************************************************
  ResourceTablePreview(PreviewWidgetInitializer& initializer)
    : PreviewWidget(initializer)
  {
    mGroup = new PreviewWidgetGroup(this);
    ResourceTable* table = initializer.Object.Get<ResourceTable*>();

    uint size = table->Size();
    size = Math::Min(size, (uint)9);
    for (uint i = 0; i < size; ++i)
    {
      Resource* resource = (*table)[i]->GetResource();
      if (resource)
        mGroup->AddPreviewWidget(resource->Name, resource, PreviewImportance::Simple);
    }
  }

  //****************************************************************************
  void AnimatePreview(PreviewAnimate::Enum value) override
  {
    mGroup->AnimatePreview(value);
  }

  //****************************************************************************
  void UpdateTransform() override
  {
    mGroup->SetSize(mSize);
    PreviewWidget::UpdateTransform();
  }
};

//--------------------------------------------------------- Space Archetype Tile
class SpaceArchetypePreview : public IconPreview
{
public:
  HandleOf<Space> mObject;

  //****************************************************************************
  SpaceArchetypePreview(PreviewWidgetInitializer& initializer, Archetype* archetype)
    : IconPreview (initializer, "Level")
  {
    //Space archetype
    Space* space =  Z::gFactory->CreateSpace(archetype->ResourceIdName, CreationFlags::Editing, Z::gEditor->GetEditGameSession());
    mObject = space;
  }

  //****************************************************************************
  ~SpaceArchetypePreview()
  {
    mObject.SafeDestroy();
  }

  //****************************************************************************
  Handle GetEditObject() override
  {
    Space* space = mObject;
    return space;
  }
  
};


class GameArchetypePreview : public IconPreview
{
public:
  HandleOf<GameSession> mObject;
  bool UsingEditorGameSession;

  //****************************************************************************
  GameArchetypePreview(PreviewWidgetInitializer& initializer, Archetype* archetype)
    : IconPreview (initializer, "Level")
  {
    // If this is the same archetype as the main game session reuse
    // the game session object
    if(Z::gEditor->GetEditGameSession()->GetArchetype() == archetype)
    {
      mObject = Z::gEditor->GetEditGameSession();
      UsingEditorGameSession = true;
    }
    else
    {
      mObject = (GameSession*)Z::gFactory->CreateCheckedType(ZilchTypeId(GameSession), nullptr, 
        archetype->Name, CreationFlags::Editing | CreationFlags::Preview, nullptr);
      UsingEditorGameSession = false;
    }
  }

  //****************************************************************************
  ~GameArchetypePreview()
  {
    if(!UsingEditorGameSession)
      mObject.SafeDestroy();
  }

  //****************************************************************************
  Handle GetEditObject() override
  {
    GameSession* game = mObject;
    return game;
  }
};

//****************************************************************************
PreviewWidget* CreateArchetypePreviewWidget(PreviewWidgetInitializer& initializer)
{
  TemporaryDoNotifyOverride doNotifyOverride(IgnoreDoNotify);

  PreviewWidget* preview = nullptr;
  Archetype* archetype = initializer.Object.Get<Archetype*>();

  if(archetype->mStoredType == ZilchTypeId(Cog))
  {
    //Basic archetype
    preview = new ArchetypePreview(initializer);
  }
  else if(archetype->mStoredType == ZilchTypeId(Space))
  {
    preview = new SpaceArchetypePreview(initializer, archetype);
  }
  else if(archetype->mStoredType == ZilchTypeId(GameSession))
  {
    preview = new GameArchetypePreview(initializer, archetype);
  }
  else
  {
    preview = new IconPreview(initializer, "Level");
  }

  return preview;
}

//****************************************************************************
void RegisterEditorTileViewWidgets()
{
  PreviewWidgetFactory* previewFactory = PreviewWidgetFactory::GetInstance();

  previewFactory->Creators[CoreArchetypes::EmptyTile] =     PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<EmptyPreview>);

  //Core resource types
  previewFactory->Creators["Archetype"] =         PreviewWidgetCreator(PreviewImportance::High, CreateArchetypePreviewWidget);
  previewFactory->Creators["Material"] =          PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<MaterialPreview>);
  previewFactory->Creators["Mesh"] =              PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<MeshPreview>);
  previewFactory->Creators["PhysicsMesh"] =       PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<PhysicsMeshPreview>);
  previewFactory->Creators["ConvexMesh"] =        PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<ConvexMeshPreview>);
  previewFactory->Creators["MultiConvexMesh"] =   PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<MultiConvexMeshPreview>);
  previewFactory->Creators["Texture"] =           PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<TexturePreview>);
  previewFactory->Creators["Font"] =              PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<FontPreview>);
  previewFactory->Creators["SpriteSource"] =      PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<SpriteSourcePreview>);
  previewFactory->Creators["TilePaletteSource"] = PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<TilePaletteSourcePreview>);
  previewFactory->Creators["ColorGradient"] =     PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<ColorGradientPreview>);
  previewFactory->Creators["SampleCurve"] =       PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<SampleCurvePreview>);
  previewFactory->Creators["ResourceTable"] =     PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<ResourceTablePreview>);
  previewFactory->Creators["Animation"] =         PreviewWidgetCreator(PreviewImportance::High, &CreatePreviewWidgetT<AnimationPreview>);

  previewFactory->Creators["CollisionGroup"] =      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<PhysicsPreview>);
  previewFactory->Creators["CollisionTable"] =      PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<PhysicsPreview>);
  previewFactory->Creators["PhysicsMaterial"] =     PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<PhysicsPreview>);
  previewFactory->Creators["PhysicsSolverConfig"] = PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<PhysicsPreview>);

  //Simple resource types
  previewFactory->Creators["ZilchScript"] =     PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<ScriptPreview>);
  previewFactory->Creators["ZilchFragment"] =   PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<ScriptPreview>);
  previewFactory->Creators["TextBlock"] =       PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<ScriptPreview>);
  previewFactory->Creators["Level"] =           PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<LevelPreview>);
  previewFactory->Creators["Sound"] =           PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<SoundPreview>);
  previewFactory->Creators["SoundCue"] =        PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<SoundCuePreview>);
  previewFactory->Creators["SoundTag"] =        PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<SoundPreview>);
  previewFactory->Creators["SoundAttenuator"] = PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<SoundPreview>);
  previewFactory->Creators["RenderGroup"] =     PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<RenderGroupPreview>);

  previewFactory->Creators["NetPropertyConfig"] = PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<NetworkingPreview>);
  previewFactory->Creators["NetChannelConfig"] =  PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<NetworkingPreview>);

  //Cog Object
  previewFactory->Creators["Cog"] =             PreviewWidgetCreator(PreviewImportance::Simple, &CreatePreviewWidgetT<CogPreview>);
}

}//namespace Zero
