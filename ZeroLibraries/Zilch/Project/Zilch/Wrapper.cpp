/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchDefineType(Wrapper, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);

  }

  //***************************************************************************
  BoundType* Wrapper::Generate(LibraryBuilder& builder, Wrapper* wrapper, BoundType* innerType, BoundType* outerBaseType)
  {
    size_t outerBaseSize = 0;
    if (outerBaseType != nullptr)
    {
      outerBaseSize = outerBaseType->Size;
      ErrorIf(outerBaseType->GetDefaultConstructor() == nullptr,
        "The base type MUST have a default constructor");
    }

    BoundType* outerType = builder.AddBoundType(innerType->Name, TypeCopyMode::ReferenceType, outerBaseSize + sizeof(Handle));
    outerType->Native = false;
    outerType->Add(wrapper);
    outerType->BaseType = outerBaseType;

    wrapper->InnerType = innerType;
    wrapper->OuterType = outerType;
    
    ZilchForEach (Property* innerProperty, innerType->GetProperties())
    {
      BoundFn getter = nullptr;
      BoundFn setter = nullptr;
      if (innerProperty->Get != nullptr)
        getter = &FunctionCall;
      if (innerProperty->Set != nullptr)
        setter = &FunctionCall;

      GetterSetter* outerProperty = builder.AddBoundGetterSetter
      (
        outerType,
        innerProperty->Name,
        innerProperty->PropertyType,
        setter,
        getter,
        innerProperty->GetMemberOptions()
      );

      if (outerProperty->Get != nullptr)
        outerProperty->Get->UserData = innerProperty->Get;
      if (outerProperty->Set != nullptr)
        outerProperty->Set->UserData = innerProperty->Set;
    }

    ZilchForEach (Function* innerFunction, innerType->GetFunctions())
    {
      DelegateType* delegateType = innerFunction->FunctionType;
      Function* outerFunction = builder.AddBoundFunction
      (
        outerType,
        innerFunction->Name,
        &FunctionCall,
        delegateType->Parameters,
        delegateType->Return,
        innerFunction->GetFunctionOptions()
      );
      outerFunction->UserData = innerFunction;
    }

    // There will always be one handle pointing at the inner type, and it will start AFTER our base class
    outerType->Handles.PushBack(outerBaseSize);

    Function* outerDefaultConstructor = builder.AddBoundDefaultConstructor(outerType, &ConstructorCall);
    outerDefaultConstructor->UserData = innerType;

    return outerType;
  }

  //***************************************************************************
  void Wrapper::ConstructorCall(Call& call, ExceptionReport& report)
  {
    Function* outer = call.GetFunction();
    BoundType* outerType = outer->Owner;
    BoundType* innerType = (BoundType*)outer->UserData;

    Handle& outerThisHandle = call.GetHandle(Call::This);

    BoundType* outerBaseType = outerType->BaseType;
    size_t outerBaseSize = 0;
    if (outerBaseType != nullptr)
    {
      outerBaseSize = outerBaseType->Size;
      Call outerBaseConstructorCall(outerBaseType->GetDefaultConstructor(), call.GetState());
      outerBaseConstructorCall.SetHandle(Call::This, outerThisHandle);
      outerBaseConstructorCall.Invoke();
    }

    Handle innerHandle = call.GetState()->AllocateDefaultConstructedHeapObject(innerType, report, HeapFlags::ReferenceCounted);

    // Copy the handle after our base class
    byte* outerBytes = outerThisHandle.Dereference() + outerBaseSize;
    new (outerBytes) Handle(innerHandle);
  }

  //***************************************************************************
  void Wrapper::FunctionCall(Call& outerCall, ExceptionReport& report)
  {
    Function* outer = outerCall.GetFunction();
    Function* inner = (Function*)outer->UserData;

    BoundType* outerType = outer->Owner;
    
    Wrapper* wrapper = outer->Owner->Has<Wrapper>();
    if (wrapper != nullptr)
      wrapper->PreFunction(outerCall);

    Call innerCall(inner, outerCall.GetState());
    if (!inner->IsStatic)
    {
      byte* ourMemory = outerCall.GetHandle(Call::This).Dereference();
      byte* handleMemory = ourMemory + outerType->Size - sizeof(Handle);
      Handle& innerHandle = *(Handle*)handleMemory;
      innerCall.SetHandle(Call::This, innerHandle);
    }
    
    DelegateType* delegateType = inner->FunctionType;
    ParameterArray& parameters = delegateType->Parameters;
    for (size_t i = 0; i < parameters.Size(); ++i)
    {
      DelegateParameter& parameter = parameters[i];
      byte* outerParameterBytes = outerCall.GetParameterUnchecked(i);
      byte* innerParameterBytes = innerCall.GetParameterUnchecked(i);
      parameter.ParameterType->GenericCopyConstruct(innerParameterBytes, outerParameterBytes);
    }
    innerCall.Invoke(report);

    byte* outerReturnBytes = outerCall.GetReturnUnchecked();
    byte* innerReturnBytes = innerCall.GetReturnUnchecked();
    delegateType->Return->GenericCopyConstruct(outerReturnBytes, innerReturnBytes);

    outerCall.DisableReturnChecks();

    if (wrapper != nullptr)
      wrapper->PostFunction(outerCall);
  }

  //***************************************************************************
  void Wrapper::PreFunction(Call& call)
  {
  }

  //***************************************************************************
  void Wrapper::PostFunction(Call& call)
  {
  }
}
