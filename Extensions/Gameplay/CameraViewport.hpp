///////////////////////////////////////////////////////////////////////////////
///
/// \file CameraViewport.hpp
///
/// Authors: Joshua Claeys, Nathan Carlson, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Manages all setup between Camera, Renderer, and viewport UI
/// that is required to define how anything is to be rendered.
class CameraViewport : public ViewportInterface 
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void OnDestroy(uint flags) override;
  void DebugDraw() override;

  // ViewportInterface

  float GetAspectRatio() override;
  Vec2 GetViewportSize() override;
  void SendSortEvent(GraphicalSortEvent* event) override;
  Cog* GetCameraCog() override;

  // Properties

  /// If rendering will be ran in edit mode.
  bool GetRenderInEditor();
  void SetRenderInEditor(bool render);
  bool mRenderInEditor;

  /// If rendering will be ran in play game mode.
  bool GetRenderInGame();
  void SetRenderInGame(bool render);
  bool mRenderInGame;

  /// The order that rendering should be done relative to other CameraViewports, lowest to highest.
  int mRenderOrder;

  /// Object with the Camera component to be used for rendering.
  /// A Camera can only be used by one CameraViewport, if already used by another usage will be stolen when assigned.
  CogPath mCameraPath;

  /// Object with renderer script that connects to RenderTasksUpdate that determines what rendering will be done.
  CogPath mRendererPath;

  /// Target resolution, or the aspect ratio when used with specific viewport scaling modes.
  void SetResolutionOrAspect(Vec2 resolution);
  Vec2 GetResolutionOrAspect();
  Vec2 mResolution;

  /// If the rendering result in FinalTexture should also be placed on the UI viewport.
  bool GetRenderToViewport();
  void SetRenderToViewport(bool render);
  bool mRenderToViewport;

  /// Forwards mouse events to viewports underneath this viewport.
  bool mForwardViewportEvents;

  /// Size of viewport in normalized UI coordinates.
  Vec2 mNormalizedSize;

  /// Offset of the viewport in normalized UI coordinates.
  Vec2 mNormalizedOffset;

  /// Method to use for sizing the viewport.
  ViewportScaling::Enum mViewportScaling;

  /// Color to used for letterbox/scaling margins.
  Vec4 mMarginColor;

  /// Transformation that defines view space as
  /// the Camera at the origin and the view direction as -Z.
  Mat4 GetWorldToView();

  /// Transformation that defines how the view frustum is mapped to normalized coordinates, pre W divide.
  Mat4 GetViewToPerspective();

  /// Concatenation of the WorldToView and ViewToPerspective transformations.
  Mat4 GetWorldToPerspective();

  /// Texture that contains the end result of this CameraViewport's rendering.
  /// Must be explicitly written to in renderer script.
  HandleOf<Texture> GetFinalTexture();

  /// If the viewport created by this CameraViewport, if rendering to one, has focus.
  bool GetViewportHasFocus();

  /// Returns whether or not it succeeded in taking focus. Will always fail if RenderToViewport is false.
  bool ViewportTakeFocus();

  /// Get the world ray starting from the mouse
  Ray GetMouseWorldRay();
  /// Convert a screen point to a ray
  Ray ScreenToWorldRay(Vec2Param screenPoint);
  /// Convert the screen point to a position on the z plane at a given depth.
  Vec3 ScreenToWorldZPlane(Vec2Param screenPoint, float worldDepth);
  /// Convert the screen point to a position on the view plane at a given depth.
  Vec3 ScreenToWorldViewPlane(Vec2Param screenPoint, float viewDepth);
  /// Convert the screen point to a position on a given plane.
  Vec3 ScreenToWorldPlane(Vec2Param screenPoint, Vec3Param worldPlaneNormal, Vec3Param worldPlanePosition);
  /// Convert a world point to a screen point.
  Vec2 WorldToScreen(Vec3Param worldPoint);
  /// Convert a screen point to a point relative to the viewport
  Vec2 ScreenToViewport(Vec2Param screenPoint);
  /// Convert a viewport point to a screen point
  Vec2 ViewportToScreen(Vec2Param viewportPoint);
  /// Size of the screen at a Depth
  Vec2 ViewPlaneSize(float viewDepth);

  /// The current resolution of the viewport
  Vec2 GetViewportResolution();
  /// The current resolution of the viewport including margin
  Vec2 GetViewportResolutionWithMargin();
  /// The current offset of the viewport, in pixels.
  Vec2 GetViewportOffset();
  /// The current offset of the viewport's margin, in pixels.
  Vec2 GetViewportOffsetWithMargin();

  // Internal

  void OnRenderTasksUpdateInternal(RenderTasksEvent* event);
  void OnUpdateActiveCameras(Event* event);
  void OnCameraPathChanged(CogPathEvent* event);
  void OnCameraDestroyed(ObjectEvent* event);

  // Viewport setup
  void CreateViewport(Camera* camera);
  void ConfigureViewport();
  void CheckSetup();
  void ClearSetup();
  void SetActiveCamera(Camera* camera);
  void SetGameWidgetOverride(GameWidget* gameWidget);
  GameWidget* GetGameWidget();

  GraphicsSpace* mGraphicsSpace;

  // By default viewports are attached to the GameWidget from the space's GameSession
  // Assign to this handle to allow attaching viewports to a different widget, i.e. EditInGame tabs
  HandleOf<GameWidget> mGameWidgetOverride;

  Camera* mActiveCamera;
  HandleOf<Viewport> mViewport;
  HandleOf<Texture> mFinalTexture;
  bool mCompleteSetup;
};

} // namespace Zero
