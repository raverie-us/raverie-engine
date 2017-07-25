#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(UpdateActiveCameras);
  DeclareEvent(UpdateSkeletons);
}

class VisibilityEvent
{
public:
  Cog* mVisibleObject;
  Cog* mViewingObject;
  String mName;
};

typedef HashMap<uint, Camera*> VisibilityMap;
typedef Array<VisibilityEvent> VisibilityEventList;

class UpdateEvent;
class RaycastResultList;

typedef AvlDynamicAabbTree<Graphical*> GraphicsBroadPhase;

/// Core space component that manages all interactions between graphics related objects.
class GraphicsSpace : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  void OnSpaceDestroyed(ObjectEvent* event);

  void AddGraphical(Graphical* graphical);
  void AddDebugGraphical(Graphical* graphical);
  void RemoveGraphical(Graphical* graphical);

  void AddCamera(Camera* camera);
  void RemoveCamera(Camera* camera);

  void OnLogicUpdate(UpdateEvent* event);
  //void OnFrameUpdate(UpdateEvent* updateEvent);
  void OnFrameUpdate(float frameDt);

  void RenderTasksUpdate(RenderTasks& renderTasks);
  void RenderQueuesUpdate(RenderTasks& renderTasks, RenderQueues& renderQueues);

  void AddToVisibleGraphicals(Graphical& graphical, Camera& camera, Vec3 cameraPos, Vec3 cameraDir, Frustum* frustum = nullptr);
  void CreateDebugGraphicals();

  Link<GraphicsSpace> EngineLink;
  GraphicsEngine* mGraphicsEngine;

  typedef InList<Graphical, &Graphical::SpaceLink> GraphicalList;
  GraphicalList mGraphicalsToAdd;
  GraphicalList mGraphicals;
  GraphicalList mGraphicalsNeverCulled;
  GraphicalList mGraphicalsAlwaysCulled;
  GraphicalList mDebugGraphicals;

  typedef InList<Camera, &Camera::SpaceLink> CameraList;
  CameraList mCameras;
  CameraList mRemovedCameras;

  /// If graphics for this Space should be running.
  bool mActive;

  // Using only 8 graphicals currently to handle wireframe/fill/thick-line/text and on-top flag
  // Will need to be more generic when custom materials can be added to debug objects
  DebugGraphical* mDebugDrawGraphicals[8];

  // Fills out the passed in vector with all objects inside the given Aabb.
  // void CastAabb(Aabb& aabb, Array<Cog*>& cogs);

  void RegisterVisibility(Camera* camera);
  void UnregisterVisibility(Camera* camera);

  void QueueVisibilityEvents(Graphical& graphical);
  void QueueVisibilityEvents(GraphicalList& graphicals);
  void QueueVisibilityEvents(GraphicalList& graphicals, Camera* camera);
  void SendVisibilityEvents();

  GraphicsBroadPhase mBroadPhase;

  Array<GraphicalEntry> mVisibleGraphicals;

  Array<uint> mRenderTaskRangeIndices;

  float mFrameTime;
  float mLogicTime;

  VisibilityFlag mRegisteredVisibility;

  VisibilityMap mRegisteredVisibilityMap;
  VisibilityEventList mVisibilityEvents;

  /// If the random number generator used by graphics objects should be seeded randomly.
  bool mRandomSeed;
  /// Value to seed the random number generator with.
  uint mSeed;
  Math::Random mRandom;
};

} // namespace Zero
