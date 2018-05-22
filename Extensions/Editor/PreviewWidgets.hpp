///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------- Icon Item
class IconPreview : public PreviewWidget
{
public:
  IconPreview(PreviewWidgetInitializer& initializer, StringParam icon);
  void UpdateTransform();

  Element* mIcon;
};

//------------------------------------------------------------------ Script Item
class ScriptPreview : public IconPreview
{
public:
  ScriptPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "ScriptIcon")
  {};
};

class RenderGroupPreview : public IconPreview
{
public:
  RenderGroupPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "RenderGroupIcon")
  {};
};

class SoundPreview : public IconPreview
{
public:
  SoundPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "SoundIcon")
  {};
};

class NetworkingPreview : public IconPreview
{
public:
  NetworkingPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "NetworkingIcon")
  {};
};

class PhysicsPreview : public IconPreview
{
public:
  PhysicsPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "PhysicsIcon")
  {};
};

//------------------------------------------------------------------- Level Item
class LevelPreview : public IconPreview
{
public:
  LevelPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "LevelIcon")
  {};
};

//--------------------------------------------------------------- Sound Cue Item
class SoundCuePreview : public IconPreview
{
public:
  typedef SoundCuePreview ZilchSelf;

  SoundCuePreview(PreviewWidgetInitializer& initializer);
  void OnLeftClick(MouseEvent* event);
};

//-------------------------------------------------------- Physics Material Item
class PhysicsMaterialPreview : public IconPreview
{
public:
  PhysicsMaterialPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "PhysicsMaterial")
  {};
};

//------------------------------------------------------------------- Empty Item
class EmptyPreview : public IconPreview
{
public:
  EmptyPreview(PreviewWidgetInitializer& initializer)
    : IconPreview(initializer, "LargeFolder")
  {
    mIcon->SetColor(Vec4(0, 0, 0, 0));
  }
};

//--------------------------------------------------------- CameraViewportDrawer
class CameraViewportDrawer : public Widget
{
public:
  CameraViewportDrawer(Composite* parent, Cog* cameraObject);

  void SetSize(Vec2 newSize);
  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, WidgetRect clipRect) override;
  
  HandleOf<Cog> mCameraObject;
};
//--------------------------------------------- Space Preview Mouse Manipulation
class SpacePreview;
class SpacePreviewMouseDrag : public MouseManipulation
{
public:
  SpacePreviewMouseDrag(Mouse* mouse, SpacePreview* preview);

  void OnMouseMove(MouseEvent* event) override;
  void OnRightMouseUp(MouseEvent* event) override;

  SpacePreview* mPreview;
};

//---------------------------------------------------------------- Space Preview
class SpacePreview : public PreviewWidget
{
public:
  typedef SpacePreview ZilchSelf;

  SpacePreview(PreviewWidgetInitializer& initializer, StringParam objectArchetype = CoreArchetypes::Default, Cog* objectToView = nullptr);
  ~SpacePreview();

  void SetInteractive(bool interactive) override;

  void OnRightMouseDown(MouseEvent* event);
  void OnMouseScroll(MouseEvent* event);
  void OnDestroy();
  void OnUpdate(UpdateEvent* updateEvent);
  
  Vec2 GetMinSize() override;
  void UpdateViewDistance();
  void UpdateViewDistance(Vec3 viewDirection);
  void UpdateCameraPosition();
  void AnimatePreview(PreviewAnimate::Enum value) override;
  void UpdateTransform();

  PreviewAnimate::Enum mPreviewAnimate;
  CameraViewportDrawer* mCameraViewportDrawer;
  HandleOf<Space> mSpace;
  bool mOwnsSpace;
  bool mHasGraphical;
  CogId mCamera;
  CogId mObject;
  // Distance the camera is from the previewed object
  float mLookAtDistance;
  // Angle the look at vector is from the y-axis from [Pi/2, -Pi/2]
  float mVerticalAngle;
  // Angle between the right vector and the positive x-axis from [0, Pi]
  float mHorizontalAngle;
};

//----------------------------------------------------------- Material Grid Tile
class MaterialPreview : public SpacePreview
{
public:
  MaterialPreview(PreviewWidgetInitializer& initializer);
};

//--------------------------------------------------------------- Mesh Grid Item
class MeshPreview : public SpacePreview
{
public:
  MeshPreview(PreviewWidgetInitializer& initializer);
};

//------------------------------------------------------- Physics Mesh Grid Item
class PhysicsMeshPreview : public SpacePreview
{
public:
  PhysicsMeshPreview(PreviewWidgetInitializer& initializer);
};

//------------------------------------------------------- Convex Mesh Grid Item
class ConvexMeshPreview : public SpacePreview
{
public:
  ConvexMeshPreview(PreviewWidgetInitializer& initializer);
};

//------------------------------------------------------- Physics Mesh Grid Item
class MultiConvexMeshPreview : public SpacePreview
{
public:
  MultiConvexMeshPreview(PreviewWidgetInitializer& initializer);
};

//---------------------------------------------------------- Archetype Grid Item
class ArchetypePreview : public SpacePreview
{
public:
  typedef ArchetypePreview ZilchSelf;

  ArchetypePreview(PreviewWidgetInitializer& initializer);
  Handle GetEditObject() override;

  const float cSpritePreviewThreshold = 0.1f;
};

//------------------------------------------------------------- SpriteSourceTile
class SpriteSourcePreview : public SpacePreview
{
public:
  SpriteSourcePreview(PreviewWidgetInitializer& initializer);
};

//---------------------------------------------------------- Animation Grid Item
class AnimationPreview : public SpacePreview
{
public:
  typedef AnimationPreview ZilchSelf;

  AnimationPreview(PreviewWidgetInitializer& initializer);

  Handle GetEditObject() override;
  void OnReload(ResourceEvent* event);
  
  HandleOf<Animation> mAnimation;
};

//------------------------------------------------------------------ CogGridItem
class CogPreview : public SpacePreview
{
public:
  CogPreview(PreviewWidgetInitializer& initializer);
};

//------------------------------------------------------------------ TextureTile
class TexturePreview : public PreviewWidget
{
public:
  TexturePreview(PreviewWidgetInitializer& initializer);
  void UpdateTransform();
  
  TextureView* mImage;
};

//------------------------------------------------------------------ Font preview
class FontPreview : public SpacePreview
{
public:
  FontPreview(PreviewWidgetInitializer& initializer);
};

//------------------------------------------------------------------ TextureTile
class TilePaletteSourcePreview : public PreviewWidget
{
public:
  TilePaletteSourcePreview(PreviewWidgetInitializer& initializer);
  void SizeToContents();
  void UpdateTransform();

private:
  TilePaletteView* mTilePaletteView;
  HandleOf<TilePaletteSource> mSource;
};

//---------------------------------------------------------- Color Gradient Tile
class ColorGradientPreview : public PreviewWidget
{
public:
  ColorGradientPreview(PreviewWidgetInitializer& initializer);
  ~ColorGradientPreview();

  void UpdateTransform() override;
  Vec2 GetHalfSize() override;
  
  PixelBuffer* mGradientBlockBuffer;
  TextureView* mGradientBlockDisplay;
};

//---------------------------------------------------------- Sample Curve Drawer
class SampleCurveDrawer : public Widget
{
public:
  SampleCurveDrawer(Composite* parent, HandleParam object);

  void AddCurve(ViewBlock& viewBlock, FrameBlock& frameBlock, WidgetRect clipRect, SampleCurve* curveObject);
  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, WidgetRect clipRect);
  
  Handle mObject;
};

//------------------------------------------------------------ SampleCurvePreview
class SampleCurvePreview : public PreviewWidget
{
public:
  SampleCurvePreview(PreviewWidgetInitializer& initializer);
  void UpdateTransform() override;

  Element* mBackground;
  GraphWidget* mGraph;
  SampleCurveDrawer* mDrawer;
};

//------------------------------------------------------- Resource Table Preview
class ResourceTablePreview : public PreviewWidget
{
public:
  ResourceTablePreview(PreviewWidgetInitializer& initializer);

  void AnimatePreview(PreviewAnimate::Enum value) override;
  void UpdateTransform() override;

  PreviewWidgetGroup* mGroup;
};

//--------------------------------------------------------- Space Archetype Tile
class SpaceArchetypePreview : public IconPreview
{
public:
  SpaceArchetypePreview(PreviewWidgetInitializer& initializer, Archetype* archetype);
  ~SpaceArchetypePreview();

  Handle GetEditObject() override;

  HandleOf<Space> mObject;
};

//--------------------------------------------------------- GameArchetypePreview
class GameArchetypePreview : public IconPreview
{
public:
  GameArchetypePreview(PreviewWidgetInitializer& initializer, Archetype* archetype);
  ~GameArchetypePreview();

  Handle GetEditObject() override;
  
  HandleOf<GameSession> mObject;
  bool UsingEditorGameSession;
};

}// namespace Zero