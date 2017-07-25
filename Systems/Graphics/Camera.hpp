///////////////////////////////////////////////////////////////////////////////
///
/// \file Camera.hpp
/// Declaration of the Camera component class.
///
/// Authors: Chris Peters, Nathan Carlson
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(CameraUpdate);
  DeclareEvent(CameraDestroyed);
}

/// Represents a viewpoint for rendering.
class Camera : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnDestroy(uint flags = 0) override;
  void TransformUpdate(TransformUpdateInfo& info) override;

  // Properties

  /// The near clipping plane, always positive and in the view direction.
  float GetNearPlane();
  void SetNearPlane(float nearPlane);
  float mNearPlane;

  /// The far clipping plane, always positive and in the view direction.
  float GetFarPlane();
  void SetFarPlane(float farPlane);
  float mFarPlane;

  /// How the scene is projected on to the view plane.
  PerspectiveMode::Enum GetPerspectiveMode();
  void SetPerspectiveMode(PerspectiveMode::Enum perspectiveMode);
  PerspectiveMode::Enum mPerspectiveMode;

  /// The vertical field of view of the Camera, in degrees. Horizontal fov derived from aspect ratio (Hor+).
  float GetFieldOfView();
  void SetFieldOfView(float fieldOfView);
  float mFieldOfView;

  /// Size (width and height) of the orthographic projection, in world units.
  float GetSize();
  void SetSize(float size);
  float mSize;

  /// The object that has a CameraViewport component using this Camera, if any.
  HandleOf<Cog> GetCameraViewportCog();

  /// Translation of the Camera, in world space.
  Vec3 GetWorldTranslation();

  /// Direction the Camera is facing, in world space.
  Vec3 GetWorldDirection();

  /// The upright direction of the Camera (perpendicular to facing direction), in world space.
  Vec3 GetWorldUp();

  // Internal

  // Set by CameraViewport, not an exposed property.
  float GetAspectRatio();
  void SetAspectRatio(float aspectRatio);
  float mAspectRatio;

  // Transform getters to recompute when needed.
  Mat4 GetViewTransform();
  Mat4 GetPerspectiveTransform();
  Mat4 GetApiPerspectiveTransform();
  // Fills ViewBlock with Camera specific data for rendering.
  void GetViewData(ViewBlock& block);
  // Creates a frustum using the Camera's settings along with the given aspect ratio.
  Frustum GetFrustum(float aspect) const;

  Link<Camera> SpaceLink;
  Transform* mTransform;

  // View transforms.
  Mat4 mWorldToView;
  Mat4 mViewToPerspective;
  Mat4 mViewToApiPerspective;
  // Flags for caching view transforms.
  bool mDirtyView;
  bool mDirtyPerspective;

  // CameraViewport that is using this Camera.
  ViewportInterface* mViewportInterface;
  // Used to identify which cameras have visibility of Graphicals.
  uint mVisibilityId;

  // Needed by GraphicsSpace to access graphical entries.
  Array<uint> mRenderGroupCounts;
  Array<IndexRange> mGraphicalIndexRanges;

  Array<uint> mRenderTaskRangeIndices;
  bool mRenderQueuesDataNeeded;
};

} // namespace Zero
