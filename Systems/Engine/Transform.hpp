///////////////////////////////////////////////////////////////////////////////
///
/// \file Transform.hpp
/// Declaration of the Transform component class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Tags
{
DeclareTag(Core);
}

DeclareBitField7(TransformUpdateFlags, 
  Translation,      // Translation was modified
  Rotation,         // Rotation was modified
  Scale,            // Scale was modified
  GizmoIncremental, // The gizmo is currently active and moving
  GizmoFinish,      // The gizmo has finished its operation
  Physics,          // Transform was changed by the physics update
  Animation);       // Transform was changed by an animation playing

/// Transform component class. The transform component provides the 
/// position, rotation and scale of an object.
class Transform : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Concatenating matrices to generate a world matrix can be very expensive when dealing with
  /// deep hierarchies that are constantly changing. Caching these world matrices can substantially
  /// increase the performance, but at the cost of memory. Instead of adding a Matrix 4 to each
  /// Transform, we can enable and disable the caching by using a memory pool and a single pointer
  /// on the Transform. This way we can enable / disable the optimization depending on what
  /// the user needs for their simulation. This is especially important for memory
  /// restrictive platforms.
  static bool sCacheWorldMatrices;
  static Memory::Pool* sCachedWorldMatrixPool;

  /// Constructor / Destructor.
  Transform();
  ~Transform();

  // Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void AttachTo(AttachmentInfo& info) override;
  void Detached(AttachmentInfo& info) override;
  void SetDefaults() override;
  void TransformUpdate(TransformUpdateInfo& info) override;
  
  /// Sets up the TransformUpdateInfo with the delta
  /// transformation that goes from oldWorldMat to newWorldMat.
  void ComputeDeltaTransform(TransformUpdateInfo& info, Mat4Param oldWorldMat, Mat4Param newWorldMat);
  void Update(uint flags);
  /// Send the TransformUpdateInfo and compute the delta transform from
  /// the old transform to the current transform (mostly for the gizmo).
  void Update(uint flags, Mat4Param oldMat);
  /// Same as the above update function but it also auto sets the scale, rotation, and
  /// translation flags. Currently used for attach/detach.
  void UpdateAll(Mat4Param oldMat, uint flags = 0);
  void UpdateAll(uint flags = 0);
  void Reset();

  Mat4 GetLocalMatrix();
  Mat4 GetWorldMatrix();
  Mat4 GetParentWorldMatrix( );
  Transform* GetParent() { return TransformParent; } 
  
  /// Local Scale relative to parent.
  Vec3 GetScale();
  void SetScale(Vec3Param scale);
  /// Local rotation relative to parent.
  Quat GetRotation();
  void SetRotation(QuatParam rotation);
  /// Local Translation relative to parent.
  Vec3 GetTranslation();
  void SetTranslation(Vec3Param translation);
  
  /// Local Scale relative to parent.
  Vec3 GetLocalScale();
  void SetLocalScale(Vec3Param localScale);
  void SetLocalScaleInternal(Vec3Param localScale);
  /// Local rotation relative to parent.
  Quat GetLocalRotation();
  void SetLocalRotation(QuatParam localRotation);
  void SetLocalRotationInternal(QuatParam localRotation);
  /// Local Translation relative to parent.
  Vec3 GetLocalTranslation();
  void SetLocalTranslation(Vec3Param localTranslation);
  void SetLocalTranslationInternal(Vec3Param localTranslation);

  /// Scale in World Space.
  Vec3 GetWorldScale();
  void SetWorldScale(Vec3Param worldScale);
  void SetWorldScaleInternal(Vec3Param worldScale);
  /// Rotation in World Space.
  Quat GetWorldRotation();
  void SetWorldRotation(QuatParam worldRotation);
  void SetWorldRotationInternal(QuatParam worldRotation);
  /// Translation in World Space.
  Vec3 GetWorldTranslation();
  void SetWorldTranslation(Vec3Param worldTranslation);
  void SetWorldTranslationInternal(Vec3Param worldTranslation);

  /// Generates a rotation matrix from the given bases.
  void SetRotationBases(Vec3Param facing, Vec3Param up, Vec3Param right);

  /// Sets the rotation of the transform by the given Euler angles in radians.
  void SetEulerAnglesXYZ(float xRadians, float yRadians, float zRadians);

  /// The rotation of the transform as an Euler angle vector in radians.
  Vec3 GetEulerAngles();
  void SetEulerAngles(Vec3Param angles);

  /// Rotate object in world space by the given Euler angle vector (in radians).
  void RotateAnglesWorld(Vec3Param angles);

  /// Rotate object in local space by the given Euler angle vector (in radians).
  void RotateAnglesLocal(Vec3Param angles);

  /// Rotate object in local space
  void RotateLocal(Quat rotation);

  /// Rotate object in world space
  void RotateWorld(Quat rotation);

  /// Rotate around a given point with the given rotation
  void RotateAround(Vec3 point, Quat rotation);

  void NormalizeRotation();

  /// Transforms a local normal (direction) into world space.
  Vec3 TransformNormal(Vec3Param normal);

  /// Transforms a local point into world space.
  Vec3 TransformPoint(Vec3Param point);

  /// Transforms a world normal (direction) into local space.
  Vec3 TransformNormalInverse(Vec3Param normal);

  /// Transforms a world point into local space.
  Vec3 TransformPointInverse(Vec3Param point); 

  /// Transforms a normal by the local matrix (ignores parent's transform)
  /// Needed now because there is no quaternion times vector in script
  Vec3 TransformNormalLocal(Vec3Param normal);

  /// Transforms a point by the local matrix (ignores parent's transform)
  Vec3 TransformPointLocal(Vec3Param point);

  void SetInWorld(bool state);
  bool GetInWorld();

  /// Free's the cached world matrix for this and all child objects.
  void SetDirty();

  /// Clamps a translation value between the max values on the space.
  /// This will display a notification if any value was clamped.
  static Vec3 ClampTranslation(Space* space, Cog* owner, Vec3 translation);

  Transform* TransformParent;

private:
  void OnDestroy(uint flags = 0) override;

  /// If null, the matrix is dirty.
  Mat4* mCachedWorldMatrix;
  Vec3 Translation;
  Vec3 Scale;
  Quat Rotation;
  bool InWorld;
};

/// Transform Utility
Vec3 GetTranslationFrom(Mat4Param mat);
void SetTranslationOn(Mat4* mat, Vec3Param newTraslation);
Aabb FromTransformAndExtents(Transform* transform, Vec3Param extents, Vec3Param translation = Vec3::cZero);
Aabb FromMatrix(Mat4Param worldMatrix, Vec3Param extents, Vec3Param translation);

DeclareEnum2(Facing, NegativeZ, PositiveZ);
void SetRotationLookAt(Transform* transform, Vec3 point, Vec3 up, Facing::Enum facing);
Quat LookAt(Vec3 eyePoint, Vec3 lookAtPoint, Vec3 up, Facing::Enum facing);
Quat LookTowards(Vec3 direction, Vec3 up, Facing::Enum facing);

}//namespace Zero
