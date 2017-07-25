///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

Zilch::Function* GetFunction(Zilch::Type* type, StringParam fnName, Array<String>& params, const Zilch::FunctionArray* functions)
{
  if(functions == nullptr)
    return nullptr;

  for(uint i = 0; i < functions->Size(); ++i)
  {
    //we need to evaluate the delegate to see if the types are the same
    Zilch::Function* fn = (*functions)[i];
    Zilch::DelegateType* d = fn->FunctionType;

    //if the parameter sizes are different then this obviously can't be the same function
    if(d->Parameters.Size() != params.Size())
      continue;

    bool isEqual = true;
    //check all of the parameters on this delegate
    for(uint j = 0; j < d->Parameters.Size(); ++j)
    {
      const Zilch::DelegateParameter& param = d->Parameters[j];
      //if the parameter is not the same type then this is not the same function
      if(param.ParameterType->ToString() != params[j])
      {
        isEqual = false;
        break;
      }
    }

    //if all parameters match and there are the same number
    //of params then these functions are equal
    if(isEqual)
      return fn;
  }
  return nullptr;
}

//simple (not pretty or efficient) function to get a zilch
//function by name and parameter types, only in the Math module for static functions
Zilch::Function* GetMemberOverloadedFunction(Zilch::Type* type, StringParam fnName, Array<String>& params)
{
  //get all of the overloaded static functions by this name
  Zilch::BoundType* boundType = Zilch::Type::GetBoundType(type);
  if(boundType == nullptr)
    return nullptr;

  const Zilch::FunctionArray* functions = boundType->GetOverloadedInstanceFunctions(fnName);
  return GetFunction(type, fnName, params, functions);
}

Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName, Array<String>& params)
{
  //get all of the overloaded static functions by this name
  Zilch::BoundType* boundType = Zilch::Type::GetBoundType(type);
  if(boundType == nullptr)
    return nullptr;

  const Zilch::FunctionArray* functions = boundType->GetOverloadedStaticFunctions(fnName);
  return GetFunction(type, fnName, params, functions);
}

Zilch::Function* GetMemberOverloadedFunction(Zilch::Type* type, StringParam fnName)
{
  Array<String> params;
  return GetMemberOverloadedFunction(type, fnName, params);
}

Zilch::Function* GetMemberOverloadedFunction(Zilch::Type* type, StringParam fnName, StringParam p0)
{
  Array<String> params;
  params.PushBack(p0);
  return GetMemberOverloadedFunction(type, fnName, params);
}

Zilch::Function* GetMemberOverloadedFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  return GetMemberOverloadedFunction(type, fnName, params);
}

Zilch::Function* GetMemberOverloadedFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  return GetMemberOverloadedFunction(type, fnName, params);
}

Zilch::Function* GetMemberOverloadedFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  params.PushBack(p3);
  return GetMemberOverloadedFunction(type, fnName, params);
}

// These are just some simple overloads, could maybe be fixed with a template later or something

Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName)
{
  Array<String> params;
  return GetStaticFunction(type, fnName, params);
}

Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName, StringParam p0)
{
  Array<String> params;
  params.PushBack(p0);
  return GetStaticFunction(type, fnName, params);
}

Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  return GetStaticFunction(type, fnName, params);
}

Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  return GetStaticFunction(type, fnName, params);
}

Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  params.PushBack(p3);
  return GetStaticFunction(type, fnName, params);
}

Zilch::Function* GetStaticFunction(Zilch::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3, StringParam p4)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  params.PushBack(p3);
  params.PushBack(p4);
  return GetStaticFunction(type, fnName, params);
}

Zilch::Field* GetStaticMember(Zilch::Type* type, StringParam memberName)
{
  Zilch::BoundType* boundType = Zilch::Type::GetBoundType(type);
  if(boundType == nullptr)
    return nullptr;
  return boundType->GetStaticField(memberName);
}

Zilch::Property* GetInstanceProperty(Zilch::Type* type, StringParam propName)
{
  Zilch::BoundType* boundType = Zilch::Type::GetBoundType(type);
  if(boundType == nullptr)
    return nullptr;
  return boundType->GetInstanceGetterSetter(propName);
}

Zilch::Property* GetStaticProperty(Zilch::Type* type, StringParam propName)
{
  Zilch::BoundType* boundType = Zilch::Type::GetBoundType(type);
  if(boundType == nullptr)
    return nullptr;
  return boundType->GetStaticGetterSetter(propName);
}

Array<String> BuildParams(StringParam p0)
{
  Array<String> params;
  params.PushBack(p0);
  return params;
}

Array<String> BuildParams(StringParam p0, StringParam p1)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  return params;
}

Array<String> BuildParams(StringParam p0, StringParam p1, StringParam p2)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  return params;
}

Array<String> BuildParams(StringParam p0, StringParam p1, StringParam p2, StringParam p3)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  params.PushBack(p3);
  return params;
}

String JoinRepeatedString(StringParam str, StringParam separator, size_t count)
{
  StringBuilder builder;
  for(size_t i = 0; i < count; ++i)
  {
    builder.Append(str);
    if(i != count - 1)
      builder.Append(separator);
  }
  return builder.ToString();
}

}//namespace Zero
