// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// MetaAnimatePropertyAction

typedef Any (*Interpolator)(AnyParam starting, AnyParam ending, float t);
typedef float (*Easer)(float inT);

template <typename type>
Any InterpolateType(AnyParam starting, AnyParam ending, float t)
{
  type* a = starting.Get<type*>();
  type* b = ending.Get<type*>();
  return Interpolation::Lerp<type>::Interpolate(*a, *b, t);
}

Any NoInterpolation(AnyParam starting, AnyParam ending, float t)
{
  return ending;
}

#define InterpolatorFor(type)                                                                                          \
  if (typeId == ZilchTypeId(type))                                                                                     \
    return InterpolateType<type>;

Interpolator GetInterpolator(Type* typeId)
{
  InterpolatorFor(int);
  InterpolatorFor(float);
  InterpolatorFor(double);
  InterpolatorFor(Vec2);
  InterpolatorFor(Vec3);
  InterpolatorFor(Vec4);
  InterpolatorFor(Quat);
  InterpolatorFor(bool);
  InterpolatorFor(String);
  return NoInterpolation;
}

Easer GetEaser(uint easeType)
{
  switch (easeType)
  {
  case EaseType::QuadIn:
    return Ease::Quad::In;
  case EaseType::QuadOut:
    return Ease::Quad::Out;
  case EaseType::QuadInOut:
    return Ease::Quad::InOut;
  case EaseType::SinIn:
    return Ease::Sin::In;
  case EaseType::SinOut:
    return Ease::Sin::Out;
  case EaseType::SinInOut:
    return Ease::Sin::InOut;
  case EaseType::ElasticIn:
    return Ease::Elastic::In;
  case EaseType::ElasticOut:
    return Ease::Elastic::Out;
  case EaseType::ElasticInOut:
    return Ease::Elastic::InOut;
  case EaseType::BounceIn:
    return Ease::Bounce::In;
  case EaseType::BounceOut:
    return Ease::Bounce::Out;
  case EaseType::BounceInOut:
    return Ease::Bounce::InOut;
  case EaseType::BackIn:
    return Ease::Back::In;
  case EaseType::BackOut:
    return Ease::Back::Out;
  case EaseType::BackInOut:
    return Ease::Back::InOut;
  case EaseType::WarpIn:
    return Ease::Warp::In;
  case EaseType::WarpOut:
    return Ease::Warp::Out;
  case EaseType::WarpInOut:
    return Ease::Warp::InOut;
  }
  return Ease::Linear::InOut;
}

class MetaAnimateProperty : public Action
{
public:
  Handle mObject;
  // This is safe, but slow. We should maybe make this faster
  PropertyHandle mProperty;
  Interpolator mInterpolator;
  Easer mEaser;
  float mDuration;
  Any mStarting;
  Any mEnding;
  float mTime;

  MetaAnimateProperty()
  {
  }

  ActionState::Enum Update(float dt) override
  {
    // Check for valid object
    if (mObject.IsNull())
      return ActionState::Completed;

    if (mFlags.IsSet(ActionFlag::Started))
    {
      // Update time
      mTime += dt;
      float t = mTime / mDuration;

      // Cap the at 1.0 to prevent overshooting
      if (t > 1.0f)
        t = 1.0f;

      // Compute eased value
      float easedT = (*mEaser)(t);
      Any newValue = (*mInterpolator)(mStarting, mEnding, easedT);

      // Set the value
      mProperty->SetValue(mObject, newValue);

      // Check for completion
      if (t < 1.0f)
        return ActionState::Running;
      else
        return ActionState::Completed;
    }
    else
    {
      // Get the current value at the start of this action
      mStarting = mProperty->GetValue(mObject);
      mFlags.SetFlag(ActionFlag::Started);

      return ActionState::Running;
    }
  }
};

Action* CreateMetaAnimatePropertyAction(
    HandleParam handle, Property* property, float duration, AnyParam ending, EaseType::Enum ease)
{
  MetaAnimateProperty* action = new MetaAnimateProperty();
  action->mDuration = duration;
  action->mEaser = GetEaser(ease);
  action->mInterpolator = GetInterpolator(property->PropertyType);
  action->mEnding = ending;
  action->mProperty = property;
  action->mObject = handle;
  action->mTime = 0.0f;
  return action;
}

ActionState::Enum MetaCallAction::Update(float dt)
{
  if (!mDelegate.IsNull())
    mDelegate.Invoke(mParameters);
  return ActionState::Completed;
}

} // namespace Zero
