///////////////////////////////////////////////////////////////////////////////
///
/// \file Transform.cpp
/// Implementation of the Transform class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Tags
{
DefineTag(Core);
}

Vec3 GetTranslationFrom(Mat4Param mat)
{
  return Vec3(mat.m03, mat.m13, mat.m23);
}

void SetTranslationOn(Mat4* mat, Vec3Param newTraslation)
{
  mat->m03 = newTraslation.x;
  mat->m13 = newTraslation.y;
  mat->m23 = newTraslation.z;
}

Quat LookTowards(Vec3 direction, Vec3 up, Facing::Enum facing)
{
  Vec3 zaxis = direction;

  if(facing == Facing::NegativeZ)
    zaxis = -zaxis;

  zaxis.AttemptNormalize( );
  up.AttemptNormalize( );

  Vec3 xaxis = Cross(up, zaxis);
  xaxis.AttemptNormalize( );

  Vec3 yaxis = Cross(zaxis, xaxis);
  yaxis.AttemptNormalize( );

  Mat3 matrix = Mat3(xaxis.x, yaxis.x, zaxis.x,
    xaxis.y, yaxis.y, zaxis.y,
    xaxis.z, yaxis.z, zaxis.z);
  Quat rotation;
  Math::ToQuaternion(matrix, &rotation);
  return rotation;
}

Quat LookAt(Vec3 eyePoint, Vec3 lookAtPoint, Vec3 up, Facing::Enum facing)
{
  return LookTowards(lookAtPoint - eyePoint, up, facing);
}

void SetRotationLookAt(Transform* transform, Vec3 lookAtPoint, Vec3 up, Facing::Enum facing)
{
  transform->SetWorldRotation(LookAt(transform->GetWorldTranslation( ), lookAtPoint, up, facing));
}

Memory::Pool* Transform::sCachedWorldMatrixPool = new Memory::Pool("TransformWorldMatrixCache",
  Memory::GetRoot( ), sizeof(Mat4), 100);

bool Transform::sCacheWorldMatrices = true;

ZilchDefineType(Transform, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::CallSetDefaults);
  ZilchBindGetterSetterProperty(Translation)->AddAttribute(PropertyAttributes::cLocalModificationOverride);
  ZilchBindGetterSetterProperty(Rotation)->AddAttribute(PropertyAttributes::cLocalModificationOverride);
  ZilchBindGetterSetterProperty(Scale)->AddAttribute(PropertyAttributes::cLocalModificationOverride);
  ZilchBindMethod(SetRotationBases);

  ZilchBindMethod(TransformNormal);
  ZilchBindMethod(TransformPoint);
  ZilchBindMethod(TransformNormalLocal);
  ZilchBindMethod(TransformPointLocal);
  ZilchBindMethod(TransformNormalInverse);
  ZilchBindMethod(TransformPointInverse);

  ZilchBindMethod(RotateLocal);
  ZilchBindMethod(RotateWorld);
  ZilchBindMethod(RotateAround);

  ZilchBindMethod(RotateAnglesLocal);
  ZilchBindMethod(RotateAnglesWorld);

  ZilchBindMethod(SetEulerAnglesXYZ);

  ZilchBindGetterSetter(EulerAngles);

  ZilchBindGetterSetter(LocalScale);
  ZilchBindGetterSetter(LocalRotation);
  ZilchBindGetterSetter(LocalTranslation);

  ZilchBindGetterSetter(WorldScale);
  ZilchBindGetterSetter(WorldRotation);
  ZilchBindGetterSetter(WorldTranslation);

  ZilchBindGetter(WorldMatrix);

  ZeroBindTag(Tags::Core);
}

Transform::Transform( )
{
  TransformParent = NULL;
  InWorld = false;
  mCachedWorldMatrix = nullptr;
}

Transform::~Transform( )
{

}

void Transform::Serialize(Serializer& stream)
{
  if(InWorld && stream.GetMode( ) == SerializerMode::Saving)
  {
    Vec3 translation = GetLocalTranslation( );
    Vec3 scale = GetLocalScale( );
    Quat rotation = GetLocalRotation( );

    stream.SerializeFieldDefault("Translation", translation, Vec3::cZero);
    stream.SerializeFieldDefault("Scale", scale, Vec3(1, 1, 1));
    stream.SerializeFieldDefault("Rotation", rotation, Quat::cIdentity);
  }
  else
  {
    SerializeNameDefault(Translation, Vec3::cZero);
    SerializeNameDefault(Scale, Vec3(1, 1, 1));
    SerializeNameDefault(Rotation, Quat::cIdentity);
  }
}

void Transform::Initialize(CogInitializer& initializer)
{
  if(initializer.mParent)
    TransformParent = initializer.mParent->has(Transform);
}

void Transform::AttachTo(AttachmentInfo& info)
{
  if(info.Child != GetOwner( ))
    return;

  Cog* parent = info.Parent;

  if(TransformParent == NULL)
  {
    ErrorIf(parent->has(Transform) == NULL, "Parent does not have a Transform.");
    TransformParent = parent->has(Transform);
  }

  SetDirty( );
}

void Transform::Detached(AttachmentInfo& info)
{
  if(info.Child != GetOwner( ))
    return;

  if(TransformParent!=NULL)
    TransformParent = NULL;
  SetDirty( );
}

void Transform::SetDefaults( )
{
  Reset( );
};

void Transform::TransformUpdate(TransformUpdateInfo& info)
{
  //If we were in-world and not the object that started transform update
  //then we have to apply the delta to ourself. (Don't apply this if it
  //was done by physics as it'll be identity anyways)
  if(info.mTransform != this && InWorld &&
    !(info.TransformFlags & TransformUpdateFlags::Physics))
  {
    Mat4 newTransform = Math::Multiply(info.mDelta, GetWorldMatrix( ));

    Mat3 rotation;
    newTransform.Decompose(&Scale, &rotation, &Translation);
    Rotation = Math::ToQuaternion(rotation).Normalized( );
  }
}

void Transform::Update(uint flags)
{
  SetDirty( );
  TransformUpdateInfo info;
  info.TransformFlags = flags;
  info.mTransform = this;
  GetOwner( )->TransformUpdate(info);
}

void Transform::ComputeDeltaTransform(TransformUpdateInfo& info, Mat4Param oldWorldMat, Mat4Param newWorldMat)
{
  //compute the delta transform
  Mat4 origMatInv = oldWorldMat.Inverted( );
  Mat4 delta = newWorldMat * origMatInv;
  info.mDelta = delta;
  info.mTransform = this;
}

void Transform::Update(uint flags, Mat4Param oldMat)
{
  TransformUpdateInfo info;
  info.TransformFlags = flags;
  ComputeDeltaTransform(info, oldMat, GetWorldMatrix( ));
  GetOwner( )->TransformUpdate(info);
}

void Transform::UpdateAll(Mat4Param oldMat, uint flags)
{
  flags = TransformUpdateFlags::Translation | TransformUpdateFlags::Scale | TransformUpdateFlags::Rotation | flags;
  Update(flags, oldMat);
}

void Transform::UpdateAll(uint flags)
{
  SetDirty( );
  TransformUpdateInfo info;
  info.mTransform = this;
  info.TransformFlags = TransformUpdateFlags::Translation |
    TransformUpdateFlags::Scale |
    TransformUpdateFlags::Rotation | flags;
  GetOwner( )->TransformUpdate(info);
}

void Transform::Reset( )
{
  Translation = Vec3::cZero;
  Scale = Vec3(1, 1, 1);
  Rotation = Quat::cIdentity;
}

Mat4 Transform::GetLocalMatrix( )
{
  Mat4 roatationTranslate = Math::ToMatrix4(Rotation);
  roatationTranslate.m03 = Translation.x;
  roatationTranslate.m13 = Translation.y;
  roatationTranslate.m23 = Translation.z;

  Mat4 scale;
  scale.SetIdentity( );
  scale.m00 = Scale.x;
  scale.m11 = Scale.y;
  scale.m22 = Scale.z;

  return roatationTranslate* scale;
}

Mat4 Transform::GetWorldMatrix( )
{
  // Return it if it's already cached
  if(mCachedWorldMatrix != nullptr)
    return *mCachedWorldMatrix;

  // Calculate the world matrix
  Mat4 worldMatrix;
  if(mCachedWorldMatrix == nullptr)
  {
    Mat4 local = GetLocalMatrix( );

    if(!InWorld && TransformParent)
      worldMatrix = TransformParent->GetWorldMatrix( ) * local;
    else
      worldMatrix = local;
  }

  // Cache it if we should
  if(sCacheWorldMatrices)
  {
    mCachedWorldMatrix = (Mat4*)sCachedWorldMatrixPool->Allocate(sizeof(Mat4));
    *mCachedWorldMatrix = worldMatrix;
  }

  return worldMatrix;
}

Mat4 Transform::GetParentWorldMatrix( )
{
  if(TransformParent)
    return TransformParent->GetWorldMatrix();

  return Mat4::cIdentity;
}

Vec3 Transform::GetScale( )
{
  return GetLocalScale( );
}

void Transform::SetScale(Vec3Param scale)
{
  SetLocalScale(scale);
}

Quat Transform::GetRotation( )
{
  return GetLocalRotation( );
}

void Transform::SetRotation(QuatParam rotation)
{
  SetLocalRotation(rotation);
}

Vec3 Transform::GetTranslation( )
{
  return GetLocalTranslation( );
}

void Transform::SetTranslation(Vec3Param translation)
{
  SetLocalTranslation(translation);
}

Vec3 Transform::GetLocalScale( )
{
  if(InWorld && TransformParent)
  {
    Mat4 world = TransformParent->GetWorldMatrix( );
    Mat4 localScale;
    localScale.Scale(Scale);
    world.Invert( );
    Mat4 local = world * localScale;
    Vec3 scale, translation;
    Mat3 rotation;
    local.Decompose(&scale, &rotation, &translation);
    return scale;
  }
  else
    return Scale;
}

void Transform::SetLocalScale(Vec3Param localScale)
{
  Mat4 oldMat;
  if(IsInitialized( ))
    oldMat = GetWorldMatrix( );

  SetLocalScaleInternal(localScale);

  if(IsInitialized( ))
  {
    TransformUpdateInfo info;
    //compute the delta of this transform so that child in-world objects can be updated
    info.TransformFlags = TransformUpdateFlags::Scale;
    ComputeDeltaTransform(info, oldMat, GetWorldMatrix( ));
    GetOwner( )->TransformUpdate(info);
  }
}

void Transform::SetLocalScaleInternal(Vec3Param localScale)
{
  //clamp max scale, I don't care that this isn't necessarily world
  //scale right now or about telling the user...
  const float minScale = 0.0001f;
  const float maxScale = 1000000.0f;
  Vec3 newScale = Math::Clamp(localScale, Vec3(minScale), Vec3(maxScale));

  if(InWorld && TransformParent)
  {
    //bring the scale into world space to get our new world scale
    Mat4 localScaleMat;
    localScaleMat.Scale(newScale);
    Mat4 world = TransformParent->GetWorldMatrix( );
    Mat4 local = world * localScaleMat;

    //it's easiest to get the world scale by decomposing the world matrix
    Vec3 translation;
    Mat3 rotation;
    local.Decompose(&Scale, &rotation, &translation);
  }
  else
    Scale = newScale;

  SetDirty( );
}

Quat Transform::GetLocalRotation( )
{
  if(InWorld && TransformParent)
  {
    Quat parentRotation = TransformParent->GetWorldRotation( );
    Quat localRotation = parentRotation.Inverted( ) * Rotation;
    return localRotation;
  }
  else
    return Rotation;
}

void Transform::SetLocalRotation(QuatParam localRotation)
{
  Mat4 oldMat;
  if(IsInitialized( ))
    oldMat = GetWorldMatrix( );

  SetLocalRotationInternal(localRotation.Normalized( ));

  if(IsInitialized( ))
  {
    TransformUpdateInfo info;
    //compute the delta of this transform so that child in-world objects can be updated
    ComputeDeltaTransform(info, oldMat, GetWorldMatrix( ));
    info.TransformFlags = TransformUpdateFlags::Rotation;
    GetOwner( )->TransformUpdate(info);
  }
}

void Transform::SetLocalRotationInternal(QuatParam localRotation)
{
  //we only need to do special logic if we are marked as in world and have a parent
  if(InWorld && TransformParent)
  {
    Quat parentRot = TransformParent->GetWorldRotation( );

    Rotation = parentRot * localRotation;
  }
  else
    Rotation = localRotation;

  Rotation.Normalize( );

  SetDirty( );
}

Vec3 Transform::GetLocalTranslation( )
{
  if(InWorld && TransformParent)
  {
    Mat4 parentTransform = TransformParent->GetWorldMatrix( );
    parentTransform.Invert( );
    return Math::TransformPoint(parentTransform, Translation);
  }
  else
    return Translation;
}

void Transform::SetLocalTranslation(Vec3Param localTranslation)
{
  Mat4 oldMat;
  if(IsInitialized( ))
    oldMat = GetWorldMatrix( );

  SetLocalTranslationInternal(localTranslation);

  if(IsInitialized( ))
  {
    TransformUpdateInfo info;
    //compute the delta of this transform so that child in-world objects can be updated
    ComputeDeltaTransform(info, oldMat, GetWorldMatrix( ));
    info.TransformFlags = TransformUpdateFlags::Translation;
    GetOwner( )->TransformUpdate(info);
  }
}

void Transform::SetLocalTranslationInternal(Vec3Param localTranslation)
{
  //clamp to max transform values (should maybe clamp the world values, don't care right now)
  Vec3 newLocalTranslation = ClampTranslation(GetSpace( ), GetOwner( ), localTranslation);

  //we only need to do special logic if we are marked as in world and have a parent
  if(InWorld && TransformParent)
  {
    Mat4 parentMat = TransformParent->GetWorldMatrix( );

    Translation = Math::TransformPoint(parentMat, newLocalTranslation);
  }
  else
    Translation = newLocalTranslation;

  SetDirty( );
}

Vec3 Transform::GetWorldScale( )
{
  if(!InWorld && TransformParent)
  {
    Mat4 parentMat = TransformParent->GetWorldMatrix( );

    //bring the scale matrix to world space
    Mat4 localScaleMat;
    localScaleMat.Scale(Scale);
    Mat4 worldMat = parentMat * localScaleMat;
    //extract out the scale
    Vec3 scale, translation;
    Mat3 rotation;
    worldMat.Decompose(&scale, &rotation, &translation);

    return scale;
  }
  else
    return Scale;
}

void Transform::SetWorldScale(Vec3Param worldScale)
{
  Mat4 oldMat;
  if(IsInitialized( ))
    oldMat = GetWorldMatrix( );

  SetWorldScaleInternal(worldScale);

  if(IsInitialized( ))
  {
    TransformUpdateInfo info;
    //compute the delta of this transform so that child in-world objects can be updated
    ComputeDeltaTransform(info, oldMat, GetWorldMatrix( ));
    info.TransformFlags = TransformUpdateFlags::Scale;
    GetOwner( )->TransformUpdate(info);
  }
}

void Transform::SetWorldScaleInternal(Vec3Param worldScale)
{
  //clamp max scale, I don't care that this isn't necessarily world
  //scale right now or about telling the user...
  const float minScale = 0.0001f;
  const float maxScale = 1000000.0f;
  Vec3 newScale = Math::Clamp(worldScale, Vec3(minScale), Vec3(maxScale));

  if(!InWorld && TransformParent)
  {
    Mat4 parentWorld = TransformParent->GetWorldMatrix( );
    Mat4 toParentSpace = parentWorld.Inverted( );
    Mat4 worldScaleMat;
    worldScaleMat.Scale(newScale);
    Mat4 localTransform = toParentSpace * worldScaleMat;

    Vec3 translation;
    Mat3 rotation;
    localTransform.Decompose(&Scale, &rotation, &translation);
  }
  else
    Scale = newScale;

  SetDirty( );
}

Quat Transform::GetWorldRotation()
{
  if(!InWorld && TransformParent)
  {
    Mat4 world = GetWorldMatrix();
    Vec3 scale, translation;
    Mat3 rotation;
    world.Decompose(&scale, &rotation, &translation);
    return Math::ToQuaternion(rotation);
  }
  else
  {
    return Rotation;
  }
}

void Transform::SetWorldRotation(QuatParam worldRotation)
{
  Mat4 oldMat;
  if(IsInitialized())
    oldMat = GetWorldMatrix();

  SetWorldRotationInternal(worldRotation);

  if(IsInitialized())
  {
    TransformUpdateInfo info;
    //compute the delta of this transform so that child in-world objects can be updated
    ComputeDeltaTransform(info, oldMat, GetWorldMatrix());
    info.TransformFlags = TransformUpdateFlags::Rotation;
    GetOwner()->TransformUpdate(info);
  }
}

void Transform::SetWorldRotationInternal(QuatParam worldRotation)
{
  if(!InWorld && TransformParent)
  {
    Quat parentRotation = TransformParent->GetWorldRotation();
    Quat parentInv = parentRotation.Inverted();

    Rotation = parentInv * worldRotation;
  }
  else
    Rotation = worldRotation;

  Rotation.Normalize();

  SetDirty();
}

Vec3 Transform::GetWorldTranslation()
{
  if(!InWorld && TransformParent)
  {
    Mat4 world = GetWorldMatrix();
    return GetTranslationFrom(world);
  }
  else
  {
    return Translation;
  }
}

void Transform::SetWorldTranslation(Vec3Param worldTranslation)
{
  Mat4 oldMat;
  if(IsInitialized())
    oldMat = GetWorldMatrix();

  SetWorldTranslationInternal(worldTranslation);

  if(IsInitialized())
  {
    TransformUpdateInfo info;
    //compute the delta of this transform so that child in-world objects can be updated
    ComputeDeltaTransform(info, oldMat, GetWorldMatrix());
    info.TransformFlags = TransformUpdateFlags::Translation;
    GetOwner()->TransformUpdate(info);
  }
}

void Transform::SetWorldTranslationInternal(Vec3Param worldTranslation)
{
  //clamp to max transform values (should maybe clamp the world values, don't care right now)
  Vec3 newWorldTranslation = ClampTranslation(GetSpace(),GetOwner(),worldTranslation);

  if(!InWorld && TransformParent)
  {
    Mat4 parentMat = TransformParent->GetWorldMatrix();
    Mat4 parentInv = parentMat.Inverted();

    Translation = Math::TransformPoint(parentInv,newWorldTranslation);
  }
  else
    Translation = newWorldTranslation;

  SetDirty();
}

void Transform::SetInWorld(bool state)
{
  //don't do anything
  if(state == InWorld)
    return;

  Mat4 worldTransform = GetWorldMatrix();
  InWorld = state;

  if(state)
  {
    Vec3 translation,scale;
    Mat3 rotation;
    worldTransform.Decompose(&scale,&rotation,&translation);

    Scale = scale;
    Rotation = Math::ToQuaternion(rotation);
    Translation = translation;
  }
  else
  {
    //no parent means we can't go to local
    if(TransformParent == NULL)
      return;

    Mat4 parentTransform = TransformParent->GetWorldMatrix();
    Mat4 localTransform = parentTransform.Inverted() * worldTransform;

    Vec3 translation,scale;
    Mat3 rotation;
    localTransform.Decompose(&scale,&rotation,&translation);

    Scale = scale;
    Rotation = Math::ToQuaternion(rotation);
    Translation = translation;
  }

  //signal the transform update (maybe add some flags?)
  UpdateAll();
}

bool Transform::GetInWorld()
{
  return InWorld;
}

void Transform::SetDirty()
{
  // Don't need to do anything if we're already dirty
  if(mCachedWorldMatrix == nullptr)
    return;

  // Free the memory
  sCachedWorldMatrixPool->Deallocate(mCachedWorldMatrix, sizeof(Mat4));
  mCachedWorldMatrix = nullptr;

  forRange(Cog& child, GetOwner()->GetChildren())
  {
    if(Transform* t = child.has(Transform))
      t->SetDirty();
  }
}

Vec3 Transform::ClampTranslation(Space* space, Cog* owner, Vec3 translation)
{
  // The Space can be null if the translation was set before being initialized
  if(space == nullptr)
    return translation;

  //clamp to the space's max translation
  real maxTranslation = space->mMaxObjectPosition;
  bool wasClamped = false;
  Vec3 t = Math::DebugClamp(translation, Vec3(-maxTranslation), Vec3(maxTranslation), wasClamped);

  //if anything was clamped and we haven't already
  //had a bad object then we'll print a message
  if(wasClamped && space->mInvalidObjectPositionOccurred == false)
  {
    //we only want to display an error message once, however if
    //we're in editor we want to display the error message every time.
    if(!space->IsEditorMode())
      space->mInvalidObjectPositionOccurred = true;
    
    String objName = owner->GetDescription();
    String errStr = String::Format("Translation was set beyond the range of [%g, %g] on object %s. "
                                   "The translation will be clamped to this range.",
                                   -maxTranslation, maxTranslation, objName.c_str());
    DoNotifyWarning("Setting Invalid Translation",errStr);
  }
  return t;
}

void Transform::OnDestroy(uint flags /*= 0*/)
{
  Cog* owner = GetOwner();

  forRange(Cog& cog, owner->GetChildren().All())
  {
    Transform* transform = cog.has(Transform);
    if (transform)
      transform->TransformParent = nullptr;
  }

  if (mCachedWorldMatrix != nullptr)
    sCachedWorldMatrixPool->Deallocate(mCachedWorldMatrix, sizeof(Mat4));
}

void Transform::SetRotationBases(Vec3Param facing, Vec3Param up, Vec3Param right)
{
  Mat3 rotation;
  GenerateRotationMatrix(facing, up, right, &rotation);
  SetRotation(Math::ToQuaternion(rotation));
}

void Transform::SetEulerAnglesXYZ(float xRadians, float yRadians, float zRadians)
{
  Mat3 rotation;
  GenerateRotationMatrix(xRadians, yRadians, zRadians, &rotation);
  SetRotation(Math::ToQuaternion(rotation));
}

Vec3 Transform::GetEulerAngles()
{
  Math::EulerAngles angles = Math::ToEulerAngles(Rotation);
  return angles.Angles;
}

void Transform::SetEulerAngles(Vec3Param v)
{
  SetEulerAnglesXYZ(v.x, v.y, v.z);
}

void Transform::RotateAnglesLocal(Vec3Param eulerVector)
{
  Math::EulerAngles eulerAngles(eulerVector, Math::EulerOrders::XYZs);
  Quat rotation = Math::ToQuaternion(eulerAngles);
  RotateLocal(rotation);
}

void Transform::RotateAnglesWorld(Vec3Param eulerVector)
{
  Math::EulerAngles eulerAngles(eulerVector, Math::EulerOrders::XYZs);
  Quat rotation = Math::ToQuaternion(eulerAngles);
  RotateWorld(rotation);
}

void Transform::RotateLocal(Quat rotation)
{
  Quat newRotation = GetRotation() * rotation;
  SetRotation(newRotation);
}

void Transform::RotateWorld(Quat rotation)
{
  Quat newRotation = rotation * GetRotation();
  SetRotation(newRotation);
}

void Transform::RotateAround(Vec3 point, Quat rotation)
{
  Vec3 direction = GetWorldTranslation() - point;
  Vec3 rotatedDirection = Math::Multiply(rotation, direction);
  Vec3 newPoint = point + rotatedDirection;
  SetWorldTranslation(newPoint);
  RotateWorld(rotation);
}

void Transform::NormalizeRotation()
{

}

Vec3 Transform::TransformNormal(Vec3Param normal)
{
  Mat4 m = GetWorldMatrix();
  return Math::TransformNormal(m,normal);
}

Vec3 Transform::TransformPoint(Vec3Param point)
{
  Mat4 m = GetWorldMatrix();
  return Math::TransformPoint(m,point);
}

Vec3 Transform::TransformNormalInverse(Vec3Param normal)
{
  Mat4 m = GetWorldMatrix();
  m.Invert();
  return Math::TransformNormal(m,normal);
}

Vec3 Transform::TransformPointInverse(Vec3Param point)
{
  Mat4 m = GetWorldMatrix();
  m.Invert();
  return Math::TransformPoint(m,point);
}

Vec3 Transform::TransformNormalLocal(Vec3Param normal)
{
  Mat4 m = GetLocalMatrix();
  return Math::TransformNormal(m,normal);
}

Vec3 Transform::TransformPointLocal(Vec3Param point)
{
  Mat4 m = GetLocalMatrix();
  return Math::TransformPoint(m,point);
}

Aabb FromTransformAndExtents(Transform* transform, Vec3Param extents, Vec3Param translation)
{
  Mat4 worldMatrix = transform->GetWorldMatrix();
  return FromMatrix(worldMatrix, extents, translation);
}

Aabb FromMatrix(Mat4Param worldMatrix, Vec3Param extents, Vec3Param translation)
{
  Vec3 t = GetTranslationFrom(worldMatrix);
  Mat3 rotation = Math::ToMatrix3(worldMatrix);
  Aabb aabb(t, extents);
  aabb.Transform(rotation);
  Vec3 modelTrans = Math::TransformNormal(worldMatrix, translation);
  aabb.Translate(modelTrans);
  return aabb;
}

}//namespace Zero
