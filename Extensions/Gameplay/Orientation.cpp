///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------Orientation
ZilchDefineType(Orientation, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  ZeroBindDependency(Transform);

  ZilchBindMethod(DebugDrawBases);

  ZilchBindGetterSetterProperty(DefaultOrientationBases);
  ZilchBindGetterSetterProperty(LocalOrientationBasis)->Add(new EditorRotationBasis("OrientationBasisGizmo"));

  ZilchBindGetter(OrientationRight);
  ZilchBindGetter(OrientationUp);
  ZilchBindGetter(OrientationForward);

  ZilchBindGetter(LocalRight);
  // Show the local up and forward as read-only properties
  ZilchBindGetterProperty(LocalUp);
  ZilchBindGetterProperty(LocalForward);

  ZilchBindGetter(WorldRight);
  ZilchBindGetter(WorldUp);
  ZilchBindGetter(WorldForward);

  ZilchBindGetter(LocalToWorldRotation);
  ZilchBindGetter(LocalToOrientationRotation);
  ZilchBindGetter(OrientationToLocalRotation);
  ZilchBindGetter(OrientationToWorldRotation);
  ZilchBindGetter(WorldToLocalRotation);
  ZilchBindGetter(WorldToOrientationRotation);

  ZilchBindMethod(SetLocalLookAtRotation);
  ZilchBindMethod(SetWorldLookAtRotation);

  ZilchBindGetterSetterProperty(GlobalUp);
  ZilchBindMethod(LookAtPoint);
  ZilchBindMethod(LookAtDirection);

  ZilchBindMethod(GetLookAtPointRotation);
  ZilchBindMethod(GetLookAtDirectionRotation);

  ZilchBindMethod(LookAtPointWithUp);
  ZilchBindMethod(LookAtDirectionWithUp);

  ZilchBindMethod(GetLookAtPointWithUpRotation);
  ZilchBindMethod(GetLookAtDirectionWithUpRotation);

  ZilchBindGetter(AbsoluteAngle);
  ZilchBindMethod(ComputeSignedAngle);
}

void Orientation::Serialize(Serializer& stream)
{
  SerializeNameDefault(mGlobalUp, Vec3::cYAxis);
  SerializeNameDefault(mLocalOrientationBasis, Quat::cIdentity);
  SerializeEnumNameDefault(OrientationBases, mDefaultBases, OrientationBases::ForwardNegativeZUpY);
}

void Orientation::Initialize(CogInitializer& initializer)
{
  mTransform =  GetOwner()->has(Transform);
}

void Orientation::DebugDraw()
{
  // This is commented out for now so that this doesn't conflict with gizmos.
  // Maybe re-enable when we have per-object or per-property selection for debug drawing.
  //DebugDrawBases();
}

void Orientation::DebugDrawBases()
{
  Vec3 pos = mTransform->GetWorldTranslation();
  Vec3 worldRight = GetWorldRight();
  Vec3 worldUp = GetWorldUp();
  Vec3 worldForward = GetWorldForward();

  gDebugDraw->Add(Debug::Line(pos, pos + worldRight).HeadSize(0.1f).Color(Color::Red));
  gDebugDraw->Add(Debug::Line(pos, pos + worldUp).HeadSize(0.1f).Color(Color::Green));
  gDebugDraw->Add(Debug::Line(pos, pos + worldForward).HeadSize(0.1f).Color(Color::Blue));
}

OrientationBases::Enum Orientation::GetDefaultOrientationBases()
{
  return mDefaultBases;
}

void Orientation::SetDefaultOrientationBases(OrientationBases::Enum value)
{
  if(OperationQueue::IsListeningForSideEffects())
  {
    // @JoshD: Can't do this now due to cyclic side-effects
    //OperationQueue::RegisterSideEffect(this, "LocalBasis", GetLocalBasis());
  }

  mDefaultBases = value;
  // Nothing to do special when going to custom (the basis doesn't change)
  if(value == OrientationBases::Custom)
    return;
  
  // Find what our desired forward and up are from the enum
  Vec3 forward = Vec3::cZAxis;
  Vec3 up = Vec3::cYAxis;
  if(value == OrientationBases::ForwardZUpY)
  {
    Vec3 forward = Vec3::cZAxis;
    Vec3 up = Vec3::cYAxis;
  }
  else if(value == OrientationBases::ForwardNegativeZUpY)
  {
    forward = -Vec3::cZAxis;
    up = Vec3::cYAxis;
  }
  else if(value == OrientationBases::ForwardXUpY)
  {
    forward = Vec3::cXAxis;
    up = Vec3::cYAxis;
  }
  else if(value == OrientationBases::ForwardXUpZ)
  {
    forward = Vec3::cXAxis;
    up = Vec3::cZAxis;
  }
  else if(value == OrientationBases::ForwardYUpZ)
  {
    forward = Vec3::cYAxis;
    up = Vec3::cZAxis;
  }

  Quat newBasis = Math::ToQuaternion(forward, up);
  mLocalOrientationBasis = newBasis;
}

Quat Orientation::GetLocalOrientationBasis()
{
  return mLocalOrientationBasis;
}

void Orientation::SetLocalOrientationBasis(QuatParam localOrientationBasis)
{
  // Register side-effect properties
  if(OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(this, "DefaultOrientationBases", GetDefaultOrientationBases());

  mLocalOrientationBasis = localOrientationBasis;
  mDefaultBases = OrientationBases::Custom;
}

Vec3 Orientation::GetOrientationRight()
{
  return Vec3::cXAxis;
}

Vec3 Orientation::GetOrientationUp()
{
  return Vec3::cYAxis;
}

Vec3 Orientation::GetOrientationForward()
{
  return -Vec3::cZAxis;
}

Vec3 Orientation::GetLocalRight()
{
  return ComputeNormalizedLocalVector(GetOrientationRight());
}

Vec3 Orientation::GetLocalUp()
{
  return ComputeNormalizedLocalVector(GetOrientationUp());
}

Vec3 Orientation::GetLocalForward()
{
  return ComputeNormalizedLocalVector(GetOrientationForward());
}

Vec3 Orientation::GetWorldRight()
{
  return ComputeNormalizedWorldVector(GetOrientationRight());
}

Vec3 Orientation::GetWorldUp()
{
  return ComputeNormalizedWorldVector(GetOrientationUp());
}

Vec3 Orientation::GetWorldForward()
{
  return ComputeNormalizedWorldVector(GetOrientationForward());
}

Quat Orientation::GetLocalToWorldRotation()
{
  // To go from local space into world space is simply the transform's world rotation
  return mTransform->GetWorldRotation();
}

Quat Orientation::GetWorldToLocalRotation()
{
  return GetLocalToWorldRotation().Inverted();
}

Quat Orientation::GetOrientationToLocalRotation()
{
  // To go from orientation to local is the basis defined by this orientation. This might seem backwards,
  // but the change of basis from orientation space to local space is one that takes the orientation space
  // vector (such as the right vector in orientation space which is (1, 0, 0))  and transforms it into local space (LocalRight).
  return mLocalOrientationBasis;
}

Quat Orientation::GetLocalToOrientationRotation()
{
  return GetOrientationToLocalRotation().Inverted();
}

Quat Orientation::GetOrientationToWorldRotation()
{
  // Simply chain orientation -> Local -> World
  return GetLocalToWorldRotation() * GetOrientationToLocalRotation();
}

Quat Orientation::GetWorldToOrientationRotation()
{
  return GetOrientationToWorldRotation().Inverted();
}

Vec3 Orientation::GetGlobalUp()
{
  return mGlobalUp;
}

void Orientation::SetGlobalUp(Vec3Param globalUp)
{
  if(globalUp == Vec3::cZero)
  {
    DoNotifyWarning("Invalid Global Up", "The global up cannot be the zero vector");
    return;
  }
  mGlobalUp = globalUp;
}

void Orientation::SetLocalLookAtRotation(QuatParam localLookAtRotation)
{
  Quat localRotation = localLookAtRotation * GetLocalToOrientationRotation();
  mTransform->SetWorldRotation(localRotation);
}

void Orientation::SetWorldLookAtRotation(QuatParam worldLookAtRotation)
{
  Quat worldRotation = worldLookAtRotation * GetLocalToOrientationRotation();
  mTransform->SetWorldRotation(worldRotation);
}

void Orientation::LookAtPoint(Vec3Param lookPoint)
{
  Vec3 worldPos = mTransform->GetWorldTranslation();
  LookAtDirection(lookPoint - worldPos);
}

Quat Orientation::GetLookAtPointRotation(Vec3Param lookPoint)
{
  Vec3 worldPos = mTransform->GetWorldTranslation();
  return GetLookAtDirectionRotation(lookPoint - worldPos);
}

void Orientation::LookAtPointWithUp(Vec3Param lookPoint, Vec3Param up)
{
  Vec3 worldPos = mTransform->GetWorldTranslation();
  LookAtDirectionWithUp(lookPoint - worldPos, up);
}

Quat Orientation::GetLookAtPointWithUpRotation(Vec3Param lookPoint, Vec3Param up)
{
  Vec3 worldPos = mTransform->GetWorldTranslation();
  return GetLookAtDirectionWithUpRotation(lookPoint - worldPos, up);
}

void Orientation::LookAtDirection(Vec3Param lookDir)
{
  Vec3 worldUp = GetGlobalUp();
  LookAtDirectionWithUp(lookDir, worldUp);
}

Quat Orientation::GetLookAtDirectionRotation(Vec3Param direction)
{
  Vec3 worldUp = GetGlobalUp();
  return GetLookAtDirectionWithUpRotation(direction, worldUp);
}

void Orientation::LookAtDirectionWithUp(Vec3Param lookDir, Vec3Param up)
{
  // Compute the rotation of our desired end orientation
  Quat destRot = GetLookAtDirectionWithUpRotation(lookDir, up);

  // Now just set the transform's rotation
  mTransform->SetWorldRotation(destRot);
}

Quat Orientation::GetLookAtDirectionWithUpRotation(Vec3Param lookDir, Vec3Param up)
{
  // Deal with bad values (lookDir can easily be 0 if the user is specifying a movement
  // direction and chooses not to move that frame, hence no rotation should happen)
  if(lookDir.LengthSq() == real(0.0) || up.LengthSq() == real(0.0))
    return GetOrientationToLocalRotation();

  // The forward vector needs to be normalized
  Vec3 worldForward = Math::Normalized(lookDir);
  // Make sure the up vector is orthonormal to the forward vector
  Vec3 worldUp = Math::ProjectOnPlane(up, worldForward);
  worldUp = worldUp.AttemptNormalized();
  // Now we can compute right (should be unit length since forward and up are perpendicular)
  Vec3 worldRight = Math::Cross(worldForward, worldUp);

  // Compute the rotation of our desired end orientation
  Quat destRot = Math::ToQuaternion(worldForward, worldUp, worldRight);

  // To end up in our destination orientation, we have to undo our local orientation so
  // that we're aligned with the world bases, then we can just apply the desired
  // rotation to get the correct end result
  Quat worldRot = destRot * GetLocalToOrientationRotation();
  worldRot.Normalize();

  return worldRot;
}

float Orientation::GetAbsoluteAngle()
{
  return AbsoluteAngle(this->GetOwner());
}

float Orientation::ComputeSignedAngle(Vec3Param up, Vec3Param forward, Vec3Param newVector)
{
  return SignedAngle(up, forward, newVector);
}

void Orientation::GetLocalBasisVectors(Vec3& localRight, Vec3& localUp, Vec3& localForward)
{
  Quat orientationToLocal = GetOrientationToLocalRotation();
  // Transform all vectors at the same time, this avoids re-calculating the rotation several times
  localRight = Math::Multiply(orientationToLocal, GetOrientationRight());
  localUp = Math::Multiply(orientationToLocal, GetOrientationUp());
  localForward = Math::Multiply(orientationToLocal, GetOrientationForward());
}

void Orientation::GetWorldBasisVectors(Vec3& worldRight, Vec3& worldUp, Vec3& worldForward)
{
  Quat orientationToWorld = GetOrientationToWorldRotation();
  // Transform all vectors at the same time, this avoids re-calculating the rotation several times
  worldRight = Math::Multiply(orientationToWorld, GetOrientationRight());
  worldUp = Math::Multiply(orientationToWorld, GetOrientationUp());
  worldForward = Math::Multiply(orientationToWorld, GetOrientationForward());
}

Vec3 Orientation::ComputeNormalizedLocalVector(Vec3Param orientationVector)
{
  Quat orientationToLocal = GetOrientationToLocalRotation();
  Vec3 localVector = Math::Multiply(orientationToLocal, orientationVector);
  localVector.AttemptNormalize();
  return localVector;
}

Vec3 Orientation::ComputeNormalizedWorldVector(Vec3Param orientationVector)
{
  Quat orientationToWorld = GetOrientationToWorldRotation();
  Vec3 worldVector = Math::Multiply(orientationToWorld, orientationVector);
  worldVector.AttemptNormalize();
  return worldVector;
}

float Orientation::SignedAngle(Vec3Param up, Vec3Param forward, Vec3Param newVector)
{
  // Get the right vector
  Vec3 right = Math::Cross(forward, up);
  right.AttemptNormalize();

  // Get the forward and right dot products
  float forwardDot = Math::Clamp(Math::Dot(forward, newVector), -1.0f, 1.0f);
  float rightDot = Math::Clamp(Math::Dot(right, newVector), -1.0f, 1.0f);

  // Get the actual angle from the forward dot product
  float finalAngle = Math::ArcCos(forwardDot);

  // If we're actually on the left side...
  if (rightDot > 0.0f)
  {
    // Compute the real final angle given the quadrant it's in (kinda like atan2)
    finalAngle = -finalAngle;
  }

  // Return the finally computed angle
  return finalAngle;
}

float Orientation::AbsoluteAngle(Vec3Param up, Vec3Param forward, Vec3Param newVector)
{
  // Get the right vector
  Vec3 right = Math::Cross(forward, up);
  right.AttemptNormalize();

  // Get the forward and right dot products
  float forwardDot = Math::Clamp(Math::Dot(forward, newVector), -1.0f, 1.0f);
  float rightDot = Math::Clamp(Math::Dot(right, newVector), -1.0f, 1.0f);

  // Get the actual angle from the forward dot product
  float finalAngle = Math::ArcCos(forwardDot);

  // If we're actually on the left side...
  if (rightDot > 0.0f)
  {
    // Compute the real final angle given the quadrant it's in (kinda like atan2)
    finalAngle = Math::cPi * 2.0f - finalAngle;
  }

  // Return the finally computed angle
  return finalAngle;
}

float Orientation::AbsoluteAngle(Cog* cog)
{
  // Get the local vectors
  Vec3 up, forward;
  LocalVectors(cog, &up, &forward);

  // Transform the vectors to world
  Vec3 forwardNew = forward;
  Vec3 upNew = up;
  ToWorld(cog, &upNew, &forwardNew);

  // Get the angle between the forward vectors
  // We assume that the up and upNew are actually the same vector (no rotation that would cause up to be different)
  // Otherwise, this function would not work very well...
  return AbsoluteAngle(up, forward, forwardNew);
}

void Orientation::LocalVectors(Cog* cog, Vec3* upOut, Vec3* forwardOut)
{
  Orientation* orientation = nullptr;
  // Try and get an orientation from this cog
  if(cog != nullptr)
    orientation = cog->has(Orientation);

  // If we didn't have a valid cog or the cog didn't have an orientation
  // component, try and get one from the space
  if(orientation == nullptr)
  {
    Space* space = cog->GetSpace();
    orientation = space->has(Orientation);
  }

  // If we managed to get a valid orientation component then get it's vectors
  if(orientation != nullptr)
  {
    Vec3 right;
    orientation->GetLocalBasisVectors(right, *upOut, *forwardOut);
    return;
  }

  // Otherwise, use the default axes
  *upOut = Vec3::cYAxis;
  *forwardOut = Vec3::cZAxis;
}

void Orientation::WorldVectors(Cog* cog, Vec3* upOut, Vec3* forwardOut)
{
  // Get the local vectors
  LocalVectors(cog, upOut, forwardOut);

  // Transform the vectors into world space
  ToWorld(cog, upOut, forwardOut);
}

void Orientation::ToWorld(Cog* cog, Vec3* upOut, Vec3* forwardOut)
{
  // If this cog has a transform the take the vectors from local space to world space
  if(cog != nullptr)
  {
    Transform* transform = cog->has(Transform);
    if(transform != nullptr)
    {
      // Transform the forward and up vectors and then normalize them
      (*forwardOut) = transform->TransformNormal(*forwardOut);
      (*forwardOut).AttemptNormalize();
      (*upOut) = transform->TransformNormal(*upOut);
      (*upOut).AttemptNormalize();
    }
  }
}

} // namespace Zero
