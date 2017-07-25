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

// Generation functions
template<typename objectType>
MetaCallAction* CreateMetaCallAction(objectType* object, StringParam name)
{
  BoundType* metaType = object->GetMeta();
  MetaCallAction* callAction = new MetaCallAction();
  callAction->mObject = object;
  callAction->mMethod = metaType->GetFunction(name);
  ErrorIf(callAction->mMethod == nullptr, "No method named %s", name.c_str());
  return callAction;
}

template<typename objectType, typename type0>
Action* CreateMetaCallAction(objectType* object, StringParam name, type0& param0)
{
  MetaCallAction* callAction = CreateMetaCallAction(object, name);
  const uint numberOfParams = 1;
  callAction->mParameters.Resize(numberOfParams);
  callAction->mParameters[0] = param0;
  ErrorIf(callAction->mMethod->FunctionType->Parameters.Size() != numberOfParams, "Invalid parameter count");
  return callAction;
}

}
