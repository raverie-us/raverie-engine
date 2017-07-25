#pragma once

namespace Zero
{

namespace Tags
{
  DeclareTag(Graphical);
}

namespace Events
{
  DeclareEvent(EnterView);
  DeclareEvent(EnterViewAny);
  DeclareEvent(ExitView);
  DeclareEvent(ExitViewAll);
}

void MakeLocalToViewAligned(Mat4& localToView, Mat4Param localToWorld, Mat4Param worldToView, Vec3Param translation);

class PropertyShaderInput
{
public:
  Component* mComponent;
  Property* mMetaProperty;
  ShaderInput mShaderInput;
};

/// Event for changes of visibility state.
class GraphicalEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  /// The viewing CameraViewport for EnterView/ExitView.
  HandleOf<Cog> mViewingObject;
};

/// Base interface for components that require rendering.
class Graphical : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface

  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void OnDestroy(uint flags = 0) override;
  void TransformUpdate(TransformUpdateInfo& info) override;
  void AttachTo(AttachmentInfo& info) override;
  void Detached(AttachmentInfo& info) override;

  // Graphical Interface

  virtual Aabb GetLocalAabb() = 0;
  virtual void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) = 0;
  virtual void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) = 0;
  virtual void MidPhaseQuery(Array<GraphicalEntry>& entries, Camera& camera, Frustum* frustum);
  virtual bool TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo);
  virtual bool TestFrustum(const Frustum& frustum, CastInfo& castInfo);
  virtual void AddToSpace();
  virtual String GetDefaultMaterialName();

  // Properties

  /// If the graphical should be drawn.
  bool GetVisible();
  void SetVisible(bool visible);
  bool mVisible;

  /// If the graphical should not be drawn when its bounding volume is outside of the view frustum.
  bool GetViewCulling();
  void SetViewCulling(bool culling);
  bool mViewCulling;

  /// If object receives events when entering/exiting the view of an active camera.
  bool mVisibilityEvents;

  /// Manually set the bounding box that is used for frustum culling.
  bool GetOverrideBoundingBox();
  void SetOverrideBoundingBox(bool overrideBoundingBox);
  bool mOverrideBoundingBox;

  /// Center of the bounding box defined in local space, world transform will be applied.
  Vec3 GetLocalAabbCenter();
  void SetLocalAabbCenter(Vec3 center);
  Vec3 mLocalAabbCenter;

  /// Half extents of the bounding box defined in local space, world transform will be applied.
  Vec3 GetLocalAabbHalfExtents();
  void SetLocalAabbHalfExtents(Vec3 halfExtents);
  Vec3 mLocalAabbHalfExtents;

  /// Can be used by a RenderGroup to define draw order, from lowest to highest.
  int mGroupSortValue;

  /// The composition of shader fragments that determines how the graphical is rendered.
  Material* GetMaterial();
  void SetMaterial(Material* material);
  HandleOf<Material> mMaterial;

  /// List of shader inputs to be manually overridden only on this object.
  ShaderInputs* GetShaderInputs();
  void SetShaderInputs(ShaderInputs* shaderInputs);
  HandleOf<ShaderInputs> mShaderInputs;

  /// The world space axis aligned bounding volume that is used for frustum culling.
  Aabb GetWorldAabb();

  // Internal

  // Returns local aabb transformed to world space without re-axis aligning.
  Obb GetWorldObb();

  // Returns the local aabb as specified by the graphical type or the override if active.
  Aabb GetLocalAabbInternal();

  void UpdateBroadPhaseAabb();
  void OnShaderInputsModified(ShaderInputsEvent* event);
  void OnMaterialModified(ResourceEvent* event);
  void ComponentAdded(BoundType* typeId, Component* component) override;
  void ComponentRemoved(BoundType* typeId, Component* component) override;
  void RebuildComponentShaderInputs();
  void AddComponentShaderInputs(Component* component);

  Link<Graphical> SpaceLink;
  BroadPhaseProxy mProxy;
  Transform* mTransform;
  GraphicsSpace* mGraphicsSpace;

  Array<PropertyShaderInput> mPropertyShaderInputs;

  // TODO: add this to separate derived class so it is not bloat for HeightMap/MultiSprite
  GraphicalEntryData mGraphicalEntryData;

  VisibilityFlag mVisibleFlags;
  VisibilityFlag mLastVisibleFlags;
};

} // namespace Zero
