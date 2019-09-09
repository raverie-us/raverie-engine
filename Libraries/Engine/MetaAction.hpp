// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Easing functions
DeclareEnum19(EaseType,
              Linear,
              QuadIn,
              QuadOut,
              QuadInOut,
              SinIn,
              SinOut,
              SinInOut,
              ElasticIn,
              ElasticOut,
              ElasticInOut,
              BounceIn,
              BounceOut,
              BounceInOut,
              BackIn,
              BackOut,
              BackInOut,
              WarpIn,
              WarpOut,
              WarpInOut);
typedef float (*Easer)(float linearT);
Easer GetEaser(uint easeType);

// MetaAnimatePropertyAction

// Action to animate a property over time
Action* CreateMetaAnimatePropertyAction(
    HandleParam object, Property* property, float duration, AnyParam ending, EaseType::Enum easeType);

/// Action to call method use Meta
class MetaCallAction : public Action
{
public:
  Delegate mDelegate;
  Array<Any> mParameters;
  ActionState::Enum Update(float dt) override;
};

} // namespace Zero
