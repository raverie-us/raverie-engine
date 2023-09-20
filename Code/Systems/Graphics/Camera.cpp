// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

// For all properties that affect the perspective transforms
#define SetPerspectiveProperty(member, value)                                                                          \
  if (value == member)                                                                                                 \
    return;                                                                                                            \
  member = value;                                                                                                      \
  mDirtyPerspective = true;

namespace Raverie
{

namespace Events
{
DefineEvent(CameraUpdate);
DefineEvent(CameraDestroyed);
} // namespace Events

RaverieDefineType(Camera, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDependency(Transform);
  RaverieBindDocumented();

  RaverieBindGetterSetterProperty(NearPlane);
  RaverieBindGetterSetterProperty(FarPlane);
  RaverieBindGetterSetterProperty(PerspectiveMode)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  RaverieBindGetterSetterProperty(FieldOfView)
      ->Add(new EditorSlider(45, 135, 1))
      ->RaverieFilterEquality(mPerspectiveMode, PerspectiveMode::Enum, PerspectiveMode::Perspective);
  RaverieBindGetterSetterProperty(Size)->RaverieFilterEquality(
      mPerspectiveMode, PerspectiveMode::Enum, PerspectiveMode::Orthographic);

  RaverieBindGetter(CameraViewportCog);
  RaverieBindGetter(WorldTranslation);
  RaverieBindGetter(WorldDirection);
  RaverieBindGetter(WorldUp);

  RaverieBindMethod(GetFrustum);
}

void Camera::Serialize(Serializer& stream)
{
  SerializeNameDefault(mNearPlane, 0.5f);
  SerializeNameDefault(mFarPlane, 100.0f);
  SerializeEnumNameDefault(PerspectiveMode, mPerspectiveMode, PerspectiveMode::Perspective);
  SerializeNameDefault(mFieldOfView, 45.0f);
  SerializeNameDefault(mSize, 20.0f);
}

void Camera::Initialize(CogInitializer& initializer)
{
  mTransform = GetOwner()->has(Transform);

  mDirtyView = true;
  mDirtyPerspective = true;
  mViewportInterface = nullptr;
  mVisibilityId = (uint)-1;
  mRenderQueuesDataNeeded = false;
}

void Camera::OnDestroy(uint flags)
{
  // Tell CameraViewport, if there is one, to remove this camera
  ObjectEvent event(this);
  DispatchEvent(Events::CameraDestroyed, &event);
}

void Camera::TransformUpdate(TransformUpdateInfo& info)
{
  mDirtyView = true;
  ObjectEvent event(this);
  DispatchEvent(Events::CameraUpdate, &event);
}

float Camera::GetNearPlane()
{
  return mNearPlane;
}

void Camera::SetNearPlane(float nearPlane)
{
  nearPlane = Math::Max(nearPlane, 0.01f);
  float farPlane = Math::Max(mFarPlane, nearPlane + 0.1f);
  SetPerspectiveProperty(mNearPlane, nearPlane);
  SetPerspectiveProperty(mFarPlane, farPlane);
}

float Camera::GetFarPlane()
{
  return mFarPlane;
}

void Camera::SetFarPlane(float farPlane)
{
  farPlane = Math::Max(farPlane, mNearPlane + 0.1f);
  SetPerspectiveProperty(mFarPlane, farPlane);
}

PerspectiveMode::Enum Camera::GetPerspectiveMode()
{
  return mPerspectiveMode;
}

void Camera::SetPerspectiveMode(PerspectiveMode::Enum perspectiveMode)
{
  SetPerspectiveProperty(mPerspectiveMode, perspectiveMode);
}

float Camera::GetFieldOfView()
{
  return mFieldOfView;
}

void Camera::SetFieldOfView(float fieldOfView)
{
  fieldOfView = Math::Clamp(fieldOfView, 10.0f, 170.0f);
  SetPerspectiveProperty(mFieldOfView, fieldOfView);
}

float Camera::GetSize()
{
  return mSize;
}

void Camera::SetSize(float size)
{
  size = Math::Max(size, 0.01f);
  SetPerspectiveProperty(mSize, size);
}

HandleOf<Cog> Camera::GetCameraViewportCog()
{
  if (mViewportInterface != nullptr)
    return mViewportInterface->GetOwner();
  return nullptr;
}

Vec3 Camera::GetWorldTranslation()
{
  return mTransform->GetWorldTranslation();
}

Vec3 Camera::GetWorldDirection()
{
  return Math::Multiply(mTransform->GetWorldRotation(), -Vec3::cZAxis);
}

Vec3 Camera::GetWorldUp()
{
  return Math::Multiply(mTransform->GetWorldRotation(), Vec3::cYAxis);
}

float Camera::GetAspectRatio()
{
  return mAspectRatio;
}

void Camera::SetAspectRatio(float aspectRatio)
{
  SetPerspectiveProperty(mAspectRatio, aspectRatio);
}

Mat4 Camera::GetViewTransform()
{
  if (mDirtyView == false)
    return mWorldToView;

  Mat4 rotation = ToMatrix4(mTransform->GetWorldRotation());

  Mat4 translation;
  translation.Translate(-mTransform->GetWorldTranslation());

  mWorldToView = rotation.Transposed() * translation;
  mDirtyView = false;
  return mWorldToView;
}

Mat4 Camera::GetPerspectiveTransform()
{
  if (mDirtyPerspective == false)
    return mViewToPerspective;

  if (mPerspectiveMode == PerspectiveMode::Perspective)
  {
    BuildPerspectiveTransformEngine(
        mViewToPerspective, Math::DegToRad(mFieldOfView), mAspectRatio, mNearPlane, mFarPlane);
    Z::gRenderer->BuildPerspectiveTransform(
        mViewToApiPerspective, Math::DegToRad(mFieldOfView), mAspectRatio, mNearPlane, mFarPlane);
  }
  else
  {
    BuildOrthographicTransformEngine(mViewToPerspective, mSize, mAspectRatio, mNearPlane, mFarPlane);
    Z::gRenderer->BuildOrthographicTransform(mViewToApiPerspective, mSize, mAspectRatio, mNearPlane, mFarPlane);
  }

  mDirtyPerspective = false;
  return mViewToPerspective;
}

Mat4 Camera::GetApiPerspectiveTransform()
{
  // Will update both perspective transforms if dirty
  GetPerspectiveTransform();
  return mViewToApiPerspective;
}

void Camera::GetViewData(ViewBlock& block)
{
  ErrorIf(mViewportInterface == nullptr, "Invalid Camera to get view data from.");

  SetAspectRatio(mViewportInterface->GetAspectRatio());

  block.mWorldToView = GetViewTransform();
  block.mViewToPerspective = GetPerspectiveTransform();

  Mat4 apiPerspective = GetApiPerspectiveTransform();
  block.mEnginePerspectiveToApiPerspective = apiPerspective * block.mViewToPerspective.SafeInverted();

  block.mNearPlane = mNearPlane;
  block.mFarPlane = mFarPlane;

  Vec2 viewportSize = mViewportInterface->GetViewportSize();
  block.mViewportSize = viewportSize;
  block.mInverseViewportSize = Vec2(1.0f / viewportSize.x, 1.0f / viewportSize.y);

  block.mEyePosition = GetWorldTranslation();
  block.mEyeDirection = GetWorldDirection();
  block.mEyeUp = GetWorldUp();
  block.mFieldOfView = mFieldOfView;
  block.mOrthographicSize = mSize;
  block.mOrthographic = mPerspectiveMode == PerspectiveMode::Orthographic;

  block.mCameraId = GetOwner()->GetId().ToUint64();
}

Frustum Camera::GetFrustum(float aspect) const
{
  Vec3 position = mTransform->GetWorldTranslation();
  Mat3 rotation = Math::ToMatrix3(mTransform->GetWorldRotation());

  Frustum f;

  if (mPerspectiveMode == PerspectiveMode::Perspective)
    f.Generate(position, rotation, mNearPlane, mFarPlane, aspect, Math::DegToRad(mFieldOfView));
  else
    f.Generate(position - rotation.BasisZ() * mNearPlane,
               -rotation.BasisZ(),
               rotation.BasisY(),
               Vec3(mSize * 0.5f * aspect, mSize * 0.5f, mFarPlane));

  return f;
}

} // namespace Raverie
