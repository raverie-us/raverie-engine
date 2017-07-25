/// Authors: Nathan Carlson, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
#pragma once

namespace Zero
{

class CameraViewport;
class ReactiveViewport;

//--------------------------------------------------------- Viewport Mouse Event
/// All mouse events that are forwarded to reactive
/// components or the space use this event to add extra data
class ViewportMouseEvent : public MouseEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ViewportMouseEvent();
  ViewportMouseEvent(MouseEvent* event);

  /// Object hit in ray cast
  Cog* GetHitObject();
  /// Camera viewport that generated this event
  CameraViewport* GetCameraViewport();
  ReactiveViewport* GetViewport() { return mViewport; }

  /// The world mouse position on the z plane at depth.
  Vec3 ToWorldZPlane(float worldDepth);
  /// The world mouse position on the view plane at view depth.
  Vec3 ToWorldViewPlane(float viewDepth);
  /// The world mouse position on any arbitrary plane.
  Vec3 ToWorldPlane(Vec3Param worldPlaneNormal, Vec3Param worldPlanePosition);
  /// The world mouse ray.
  Ray mWorldRay;

  /// Mouse Ray start
  Vec3 mRayStart;
  /// Mouse Ray Direction
  Vec3 mRayDirection;

  /// The intersection point with an object. Used with Reactive components.
  Vec3 mHitPosition;
  /// The normal at the intersection point with an object. Used with Reactive components.
  Vec3 mHitNormal;
  /// The distance away the hit point is. Used with Reactive components.
  float mHitDistance;
  /// The Reactive object hit. In normal mouse events this will be null.
  CogId mHitObject;
  /// The camera viewport that the mouse is moving on.
  /// Will be invalid when in an editor viewport.
  CogId mCameraViewportCog;
  HandleOf<ReactiveViewport> mViewport;
};

//------------------------------------------------------------ Reactive Viewport
class ReactiveViewport : public Viewport
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  ReactiveViewport(Composite* parent, Space* space, Camera* camera, CameraViewport* cameraViewport);

  /// Get the reactive space component from our target space
  ReactiveSpace* GetReactiveSpace();

  void OnMouseEnter(MouseEvent* e);

  /// When the mouse leaves the viewport, we want to send all mouse exit and
  /// mouse up events to any reactive object.
  void OnMouseExit(MouseEvent* e);

  ///
  virtual void OnMouseUpdate(MouseEvent* e);

  /// Used to forward all mouse events to one function.
  void OnMouseGeneric(MouseEvent* e);

  /// Updates which object the mouse is currently over.
  void UpdateOverObject(MouseEvent* e);

  /// Builds a ViewportMouseEvent from the given event and forwards it to
  /// the reactive object the mouse is over, and the active camera.
  virtual void ForwardReactiveEvent(StringParam eventName, MouseEvent* e);

  /// Adds extra data to the MouseEvent.
  virtual void InitViewportEvent(ViewportMouseEvent& viewportEvent);

  Widget* HitTest(Vec2 screenPoint, Widget* skip) override;

  Ray mReactiveRay;
  Vec3 mReactiveHitPosition;
  Vec3 mReactiveHitNormal;
  float mReactiveHitDistance;

  ComponentHandle<CameraViewport> mCameraViewport;
  GameWidget* mGameWidget;
};

//------------------------------------------------------------------ Game Widget
// Game Widget contains viewports and the
// GameSession object
class GameWidget : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GameWidget(Composite* composite);
  ~GameWidget();

  // Widget Interface
  void OnDestroy() override;
  bool TakeFocusOverride() override;

  void SetGameSession(GameSession* gameSession);

  void OnKeyDown(KeyboardEvent* event);
  void OnGameQuit(GameEvent* gameEvent);
  void OnUpdate(UpdateEvent* event);
  ReactiveViewport* GetViewportUnder(ReactiveViewport* current);

  void SaveScreenshot(StringParam filename);
  void OnUiRenderUpdate(Event* event);

  HandleOf<GameSession> mGame;
  String mScreenshotFilename;
};

} // namespace Zero
