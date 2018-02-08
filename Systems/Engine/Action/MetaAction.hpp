///////////////////////////////////////////////////////////////////////////////
///
/// \file MetaAction.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once 

namespace Zero
{

// Easing functions
DeclareEnum7(EaseType, Linear, QuadIn, QuadOut, QuadInOut, SinIn, SinOut, SinInOut);
typedef float (*Easer)(float linearT);
Easer GetEaser(uint easeType);

//------------------------------------------------------------ MetaAnimatePropertyAction

// Action to animate a property over time
Action* CreateMetaAnimatePropertyAction(
  HandleParam object, Property* property, float duration, AnyParam ending, EaseType::Enum easeType);

//------------------------------------------------------------ MetaCallAction

/// Action to call method use Meta
class MetaCallAction : public Action
{
public:
  Delegate mDelegate;
  Array<Any> mParameters;
  ActionState::Enum Update(float dt) override;
};

}
