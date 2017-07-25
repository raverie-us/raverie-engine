///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum6(OrientationBases, ForwardZUpY, ForwardNegativeZUpY, ForwardXUpY, ForwardXUpZ, ForwardYUpZ, Custom);

//-------------------------------------------------------------------Orientation
/// Defines a new basis for a desired right, up, and forward vector. Provides a bunch of helper
/// functions to change between these spaces and to perform simple look-at behavior.
class Orientation : public Component
{
public:
  // Meta Initialization
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void DebugDraw() override;

  /// Debug draws the current orientation bases in world space.
  void DebugDrawBases();

  // Set a default orientation bases.
  OrientationBases::Enum GetDefaultOrientationBases();
  void SetDefaultOrientationBases(OrientationBases::Enum value);

  /// A local-space basis that represents this orientation. If you build a basis from an up 
  /// of (0, 1, 0), and a forward of (0, 0, -1) then this should result in the identity rotation.
  Quat GetLocalOrientationBasis();
  void SetLocalOrientationBasis(QuatParam localOrientationBasis);

  /// The right vector in orientation space. This is always the vector (1, 0, 0)
  /// but is provided for clarity with transformations.
  Vec3 GetOrientationRight();
  /// The up vector in orientation space. This is always the vector (0, 1, 0)
  /// but is provided for clarity with transformations.
  Vec3 GetOrientationUp();
  /// The forward vector in orientation space. This is always the vector (0, 0, -1)
  /// but is provided for clarity with transformations.
  Vec3 GetOrientationForward();

  /// The orientation's right vector after having been transformed into local space.
  Vec3 GetLocalRight();
  /// The orientation's up vector after having been transformed into local space.
  Vec3 GetLocalUp();
  /// The orientation's forward vector after having been transformed into local space.
  Vec3 GetLocalForward();

  /// The orientation's right vector after having been transformed into world space
  Vec3 GetWorldRight();
  /// The orientation's up vector after having been transformed into world space
  Vec3 GetWorldUp();
  /// The orientation's forward vector after having been transformed into world space
  Vec3 GetWorldForward();
  
  /// The rotation that transforms a vector from local space into world space.
  /// For example, this transforms LocalRight into WorldRight.
  Quat GetLocalToWorldRotation();
  /// The rotation that transforms a vector from world space into local space.
  /// For example, this transforms WorldRight into LocalRight.
  Quat GetWorldToLocalRotation();

  /// The rotation that takes an orientation space vector into local space.
  /// For example, this transforms OrientationRight into LocalRight.
  Quat GetOrientationToLocalRotation();
  /// The rotation that takes an local space vector into orientation space.
  /// For example, this transforms LocalRight into OrientationRight.
  Quat GetLocalToOrientationRotation();
  
  /// The rotation that takes an orientation space vector into world space.
  /// For example, this transforms OrientationRight into WorldRight.
  Quat GetOrientationToWorldRotation();
  /// The rotation that takes an world space vector into orientation space.
  /// For example, this transforms WorldRight into OrientationRight.
  Quat GetWorldToOrientationRotation();

  /// The world-space up vector to use for LookAt operations that don't take an up vector.
  Vec3 GetGlobalUp();
  void SetGlobalUp(Vec3Param globalUp);

  /// Set the transform's local rotation such that the orientation's basis vectors will be aligned with
  /// the given rotation (assumed to be a look-at rotation constructed from a right, up, and forward)
  void SetLocalLookAtRotation(QuatParam localLookAtRotation);
  /// Set the transform's world rotation such that the orientation's basis vectors will be aligned with
  /// the given rotation (assumed to be a look-at rotation constructed from a right, up, and forward)
  void SetWorldLookAtRotation(QuatParam worldLookAtRotation);

  /// Sets the forward to look at the given point.
  /// Keeps the current world up.
  void LookAtPoint(Vec3Param lookPoint);
  /// Get the rotation so that the forward will look at the given point.
  /// Keeps the current world up.
  Quat GetLookAtPointRotation(Vec3Param lookPoint);

  /// Same as LookAtPoint but allows the user to specify the up vector.
  void LookAtPointWithUp(Vec3Param lookPoint, Vec3Param up);
  /// Same as GetLookAtPointRotation but allows the user to specify the up vector.
  Quat GetLookAtPointWithUpRotation(Vec3Param lookPoint, Vec3Param up);

  /// Sets the forward to look in the given direction.
  /// Keeps the current world up.
  void LookAtDirection(Vec3Param lookDir);
  /// Gets the forward to look in the given direction.
  /// Keeps the current world up.
  Quat GetLookAtDirectionRotation(Vec3Param lookDir);

  /// Same as LookAtDirection but allows the user to specify the up vector.
  void LookAtDirectionWithUp(Vec3Param lookDir, Vec3Param up);
  /// Same as GetLookAtDirectionRotation but allows the user to specify the up vector.
  Quat GetLookAtDirectionWithUpRotation(Vec3Param lookDir, Vec3Param up);

  /// Get the angle of the object about the up vector
  float GetAbsoluteAngle();
  /// Compute the rotation angle between two vectors (in radians)
  float ComputeSignedAngle(Vec3Param up, Vec3Param forward, Vec3Param newVector);
  
  /// Transform all 3 orientation-space basis vectors into local space.
  void GetLocalBasisVectors(Vec3& localRight, Vec3& localUp, Vec3& localForward);
  /// Transform all 3 orientation-space basis vectors into world space.
  void GetWorldBasisVectors(Vec3& worldRight, Vec3& worldUp, Vec3& worldForward);

  /// Transforms the given orientation-space vector into local space and then normalizes it.
  Vec3 ComputeNormalizedLocalVector(Vec3Param orientationVector);
  /// Transforms the given orientation-space vector into world space and then normalizes it.
  Vec3 ComputeNormalizedWorldVector(Vec3Param orientationVector);

  // Get the rotation angle between two vectors (in radians)
  static float SignedAngle(Vec3Param up, Vec3Param forward, Vec3Param newVector);
  // Get the rotation angle between two vectors (in radians)
  static float AbsoluteAngle(Vec3Param up, Vec3Param forward, Vec3Param newVector);
  // Get the absolute rotation angle of an object (in radians)
  static float AbsoluteAngle(Cog* cog);

  // Get the default axis vectors in local space
  static void LocalVectors(Cog* cog, Vec3* upOut, Vec3* forwardOut);
  // Get the default axis vectors in world space (apply the object's rotation to the forward vector)
  static void WorldVectors(Cog* cog, Vec3* upOut, Vec3* forwardOut);

private:
  // Transform an up and forward vector to world space
  static void ToWorld(Cog* cog, Vec3* upOut, Vec3* forwardOut);

  Transform* mTransform;

  /// Primarily serialized so that we can display the correct enum on
  /// load. This could be detected instead of saved later.
  OrientationBases::Enum mDefaultBases;
  Quat mLocalOrientationBasis;
  /// The world-space up vector to use for LookAt operations that don't take an up vector.
  Vec3 mGlobalUp;
};

} // namespace Zero
