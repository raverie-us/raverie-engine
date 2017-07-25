///////////////////////////////////////////////////////////////////////////////
///
/// \file ActionGenerator.hpp
/// Declaration of the Action Builder.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

#define EaseFunc float (*EaseType)(float)

//Property
template<typename objectType, typename elementType, 
  elementType objectType::* PtrToMember >
struct PropertyAccessorMem
{
  static inline elementType Get(objectType* mObject){return mObject->*PtrToMember;}
  static inline void Set(objectType* mObject,const elementType& value){mObject->*PtrToMember = value;}
};

template<typename objectType, typename elementType, typename elementTypeSet,
  elementType (objectType::*MemGetter)() ,
  void (objectType::*MemSetter)(elementTypeSet)>
struct PropertyAccessorMemFunc
{
  static inline elementType Get(objectType* mObject){return (mObject->*MemGetter)();}
  static inline void Set(objectType* mObject,const elementType& newValue){(mObject->*MemSetter)(newValue);}
};


template<typename type, typename objectType, EaseFunc, typename PropertyAccessor>
class ActionGenerator : public Action
{
public:

  ActionGenerator(objectType* instance, float duration, type destination)
  {
    Duration = duration;
    t1 = destination;
    Time = 0.0f;
    Instance = instance;
  }

  type t0;
  type t1;
  float Duration;
  float Time;
  objectType* Instance;

  ActionState::Enum  Update(float dt) override
  {
    if(mFlags.IsSet(ActionFlag::Started))
    {
      Time += dt;
      float t = Time / Duration;
      if(t>1.0f) t = 1.0f;
      float eased = (*EaseType)(t);
      type newValue = Interpolation::Lerp<type>::Interpolate(t0, t1, eased);
      PropertyAccessor::Set(Instance, newValue);

      if(t<1.0f)
        return ActionState::Running;
      else
        return ActionState::Completed;
    }
    else
    {
      t0 = PropertyAccessor::Get(Instance);
      mFlags.SetFlag(ActionFlag::Started);
      return ActionState::Running;
    }
  }
};


//Bind normal getter and setters member functions.
template<typename GetterType, typename SetterType, GetterType MemGetter, SetterType MemSetter, 
         EaseFunc, typename objectType, typename elementType, typename elementSetType>
inline Action* GenerateAction(elementType (objectType::*getDummy)() , void (objectType::*setDummy)(elementSetType),
                                    objectType* instance, float duration, const elementType& destination)
{
  return new ActionGenerator<elementType, objectType, EaseType, 
    PropertyAccessorMemFunc<objectType, elementType, elementSetType, MemGetter, MemSetter> >(instance, duration, destination);
}

template<typename MemberType, MemberType Member, EaseFunc, typename objectType, typename elementType>
inline Action* GenerateAction(objectType* instance, float duration, const elementType& destination)
{
  return new ActionGenerator<elementType, objectType, EaseType, 
    PropertyAccessorMem<objectType, elementType, 
    Member > >(instance, duration, destination);
}

#define AnimateMember(memberPointer, easeType, instance, duration, dest)  \
  GenerateAction< TypeOf(memberPointer), memberPointer, easeType> \
  (instance, duration, dest)

#define AnimatePropertyGetSet(objectType, propName, easeType, instance, duration, dest)  \
  GenerateAction< TypeOf(&objectType::Get##propName), TypeOf(&objectType::Set##propName),  \
     &objectType::Get##propName,  &objectType::Set##propName, easeType>(                \
    &objectType::Get##propName, &objectType::Set##propName, static_cast<objectType*>(instance), duration, dest)

template<typename objectType, void (objectType::*FunctionToCall)()>
class CallAction : public Action
{
public:
  objectType* Instance;
  CallAction(objectType* instance)
  {
    Instance = instance;
  }
  ActionState::Enum Update(float dt) override
  {
    (Instance->*FunctionToCall)();
    return ActionState::Completed;
  }
};

template<typename objectType, typename ParamType, void (objectType::*FunctionToCall)(ParamType)>
class CallParamAction : public Action
{
public:
  objectType* Instance;
  ParamType Value;
  CallParamAction(objectType* instance, ParamType param)
    :Instance(instance), Value(param)
  {
    Instance = instance;
  }

  ActionState::Enum Update(float dt) override
  {
    (Instance->*FunctionToCall)(Value);
    return ActionState::Completed;
  }
};

template<typename ParamType, void (*FunctionToCall)(ParamType)>
class GlobalCallParamAction : public Action
{
public:
  ParamType Value;
  GlobalCallParamAction(ParamType param)
    :Value(param)
  {
    
  }

  ActionState::Enum Update(float dt) override
  {
    FunctionToCall(Value);
    return ActionState::Completed;
  }
};

template<typename objectType, void (objectType::*FunctionToCall)(float)>
class UpdateAction : public Action
{
public:
  objectType* mInstance;
  UpdateAction(objectType* instance)
    :mInstance(instance)
  {
  }
  ActionState::Enum Update(float dt) override
  {
    (mInstance->*FunctionToCall)(dt);
    return ActionState::Running;
  }
};

}
