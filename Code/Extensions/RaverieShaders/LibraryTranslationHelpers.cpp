// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

Raverie::Function* GetFunction(Array<String>& params, const Raverie::FunctionArray* functions)
{
  if (functions == nullptr)
    return nullptr;

  for (uint i = 0; i < functions->Size(); ++i)
  {
    // we need to evaluate the delegate to see if the types are the same
    Raverie::Function* fn = (*functions)[i];
    Raverie::DelegateType* d = fn->FunctionType;

    // if the parameter sizes are different then this obviously can't be the
    // same function
    if (d->Parameters.Size() != params.Size())
      continue;

    bool isEqual = true;
    // check all of the parameters on this delegate
    for (uint j = 0; j < d->Parameters.Size(); ++j)
    {
      const Raverie::DelegateParameter& param = d->Parameters[j];
      // if the parameter is not the same type then this is not the same
      // function
      if (param.ParameterType->ToString() != params[j])
      {
        isEqual = false;
        break;
      }
    }

    // if all parameters match and there are the same number
    // of params then these functions are equal
    if (isEqual)
      return fn;
  }
  return nullptr;
}

Raverie::Function*
GetFunction(Raverie::Type* type, StringParam fnName, Array<String>& params, const Raverie::FunctionArray* functions)
{
  return GetFunction(params, functions);
}

// simple (not pretty or efficient) function to get a raverie
// function by name and parameter types, only in the Math module for static
// functions
Raverie::Function* GetMemberOverloadedFunction(Raverie::Type* type, StringParam fnName, Array<String>& params)
{
  // get all of the overloaded static functions by this name
  Raverie::BoundType* boundType = Raverie::Type::GetBoundType(type);
  if (boundType == nullptr)
    return nullptr;

  const Raverie::FunctionArray* functions = boundType->GetOverloadedInstanceFunctions(fnName);
  return GetFunction(type, fnName, params, functions);
}

Raverie::Function* GetStaticFunction(Raverie::Type* type, StringParam fnName, Array<String>& params)
{
  // get all of the overloaded static functions by this name
  Raverie::BoundType* boundType = Raverie::Type::GetBoundType(type);
  if (boundType == nullptr)
    return nullptr;

  const Raverie::FunctionArray* functions = boundType->GetOverloadedStaticFunctions(fnName);
  return GetFunction(type, fnName, params, functions);
}

Raverie::Function* GetConstructor(Raverie::Type* type, StringParam p0)
{
  Array<String> params;
  params.PushBack(p0);
  return GetConstructor(type, params);
}

Raverie::Function* GetConstructor(Raverie::Type* type, Array<String>& params)
{
  // get all of the overloaded static functions by this name
  Raverie::BoundType* boundType = Raverie::Type::GetBoundType(type);
  if (boundType == nullptr)
    return nullptr;

  const Raverie::FunctionArray* constructors = boundType->GetOverloadedInheritedConstructors();
  return GetFunction(params, constructors);
}

Raverie::Function* GetMemberOverloadedFunction(Raverie::Type* type, StringParam fnName)
{
  Array<String> params;
  return GetMemberOverloadedFunction(type, fnName, params);
}

Raverie::Function* GetMemberOverloadedFunction(Raverie::Type* type, StringParam fnName, StringParam p0)
{
  Array<String> params;
  params.PushBack(p0);
  return GetMemberOverloadedFunction(type, fnName, params);
}

Raverie::Function* GetMemberOverloadedFunction(Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  return GetMemberOverloadedFunction(type, fnName, params);
}

Raverie::Function*
GetMemberOverloadedFunction(Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  return GetMemberOverloadedFunction(type, fnName, params);
}

Raverie::Function* GetMemberOverloadedFunction(
    Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  params.PushBack(p3);
  return GetMemberOverloadedFunction(type, fnName, params);
}

// These are just some simple overloads, could maybe be fixed with a template
// later or something

Raverie::Function* GetStaticFunction(Raverie::Type* type, StringParam fnName)
{
  Array<String> params;
  return GetStaticFunction(type, fnName, params);
}

Raverie::Function* GetStaticFunction(Raverie::Type* type, StringParam fnName, StringParam p0)
{
  Array<String> params;
  params.PushBack(p0);
  return GetStaticFunction(type, fnName, params);
}

Raverie::Function* GetStaticFunction(Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  return GetStaticFunction(type, fnName, params);
}

Raverie::Function*
GetStaticFunction(Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  return GetStaticFunction(type, fnName, params);
}

Raverie::Function*
GetStaticFunction(Raverie::Type* type, StringParam fnName, StringParam p0, StringParam p1, StringParam p2, StringParam p3)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  params.PushBack(p3);
  return GetStaticFunction(type, fnName, params);
}

Raverie::Function* GetStaticFunction(Raverie::Type* type,
                                   StringParam fnName,
                                   StringParam p0,
                                   StringParam p1,
                                   StringParam p2,
                                   StringParam p3,
                                   StringParam p4)
{
  Array<String> params;
  params.PushBack(p0);
  params.PushBack(p1);
  params.PushBack(p2);
  params.PushBack(p3);
  params.PushBack(p4);
  return GetStaticFunction(type, fnName, params);
}

Raverie::Field* GetStaticMember(Raverie::Type* type, StringParam memberName)
{
  Raverie::BoundType* boundType = Raverie::Type::GetBoundType(type);
  if (boundType == nullptr)
    return nullptr;
  return boundType->GetStaticField(memberName);
}

Raverie::Property* GetInstanceProperty(Raverie::Type* type, StringParam propName)
{
  Raverie::BoundType* boundType = Raverie::Type::GetBoundType(type);
  if (boundType == nullptr)
    return nullptr;
  return boundType->GetInstanceGetterSetter(propName);
}

Raverie::Property* GetStaticProperty(Raverie::Type* type, StringParam propName)
{
  Raverie::BoundType* boundType = Raverie::Type::GetBoundType(type);
  if (boundType == nullptr)
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
  for (size_t i = 0; i < count; ++i)
  {
    builder.Append(str);
    if (i != count - 1)
      builder.Append(separator);
  }
  return builder.ToString();
}

} // namespace Raverie
