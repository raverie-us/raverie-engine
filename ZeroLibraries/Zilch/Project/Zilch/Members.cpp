/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchDefineType(ReflectionObject, builder, type)
  {
    type->CopyMode = TypeCopyMode::ReferenceType;
    type->HandleManager = ZilchManagerId(PointerManager);
  }

  //***************************************************************************
  ZilchDefineType(Member, builder, type)
  {
    type->CopyMode = TypeCopyMode::ReferenceType;
    type->HandleManager = ZilchManagerId(PointerManager);
    ZilchFullBindField(builder, type, &Member::Name, "Name", PropertyBinding::Get);
    ZilchFullBindField(builder, type, &Member::Owner, "Owner", PropertyBinding::Get);
    ZilchFullBindField(builder, type, &Member::IsStatic, "IsStatic", PropertyBinding::Get);
    ZilchFullBindGetterSetter(builder, type, &Member::GetOwningLibrary, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "Library");
    ZilchFullBindGetterSetter(builder, type, &Member::GetTypeOrNull, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "Type");
  }

  //***************************************************************************
  ZilchDefineType(Property, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);
    ZilchFullBindField(builder, type, &Property::Get, "Getter", PropertyBinding::Get);
    ZilchFullBindField(builder, type, &Property::Set, "Setter", PropertyBinding::Get);
    ZilchFullBindMethod(builder, type, &Property::GetValue, ZilchNoOverload, "GetValue", "instance");
    ZilchFullBindMethod(builder, type, &Property::SetValue, ZilchNoOverload, "SetValue", "instance, value");
  }

  //***************************************************************************
  ZilchDefineType(GetterSetter, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);
  }

  //***************************************************************************
  ZilchDefineType(Field, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);
    //ZilchFullBindField(builder, type, &Field::Offset, "Offset", PropertyBinding::Get);
  }

  //***************************************************************************
  ZilchDefineType(Variable, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);
  }

  //***************************************************************************
  Constant::Constant()
  {
    this->Clear();
  }
  
  //***************************************************************************
  Constant::Constant(StringParam value)
  {
    this->Clear();
    this->Type = ConstantType::String;
    this->StringValue = value;
  }
  
  //***************************************************************************
  Constant::Constant(Integer value)
  {
    this->Clear();
    this->Type = ConstantType::Integer;
    this->IntegerValue = value;
  }
  
  //***************************************************************************
  Constant::Constant(DoubleInteger value)
  {
    this->Clear();
    this->Type = ConstantType::DoubleInteger;
    this->IntegerValue = value;
  }
  
  //***************************************************************************
  Constant::Constant(Real value)
  {
    this->Clear();
    this->Type = ConstantType::Real;
    this->RealValue  = value;
  }
  
  //***************************************************************************
  Constant::Constant(DoubleReal value)
  {
    this->Clear();
    this->Type = ConstantType::DoubleReal;
    this->RealValue  = value;
  }
  
  //***************************************************************************
  Constant::Constant(Boolean value)
  {
    this->Clear();
    this->Type = ConstantType::Boolean;
    this->BooleanValue  = value;
  }
  
  //***************************************************************************
  Constant::Constant(NullPointerType value)
  {
    this->Clear();
  }
  
  //***************************************************************************
  Constant::Constant(Zilch::Type* value)
  {
    this->Clear();
    this->Type = ConstantType::Type;
    this->TypeValue  = value;
  }

  //***************************************************************************
  void Constant::Clear()
  {
    this->Type = ConstantType::Null;
    this->StringValue.Clear();
    this->IntegerValue = 0;
    this->RealValue = 0.0;
    this->BooleanValue = false;
  }
  
  //***************************************************************************
  String Constant::ToString() const
  {
    switch (this->Type)
    {
      case ConstantType::Null:
        return Grammar::GetKeywordOrSymbol(Grammar::Null);
      case ConstantType::String:
        return EscapeStringAndAddQuotes(this->StringValue);
      case ConstantType::Integer:
        return IntegerToString((Integer)this->IntegerValue);
      case ConstantType::DoubleInteger:
        return DoubleIntegerToString(this->IntegerValue);
      case ConstantType::Real:
        return RealToString((Real)this->RealValue);
      case ConstantType::DoubleReal:
        return DoubleRealToString(this->RealValue);
      case ConstantType::Boolean:
        return BooleanToString(this->BooleanValue);
      case ConstantType::Type:
        return this->TypeValue->ToString();
    }

    Error("Invalid case detected in Zilch constant");
    return String();
  }

  //***************************************************************************
  Attribute::Attribute() :
    Owner(nullptr)
  {
  }

  //***************************************************************************
  AttributeParameter* Attribute::HasAttributeParameter(StringParam name)
  {
    // Walk through the array of parameters (usually small)
    for (size_t i = 0; i < this->Parameters.Size(); ++i)
    {
      // If we found a matching name, then we do have that parameter
      AttributeParameter* parameter = &this->Parameters[i];
      if (parameter->Name == name)
        return parameter;
    }

    // If we got here, we didn't find the parameter
    return nullptr;
  }

  //***************************************************************************
  Attribute* Attribute::AddAttribute(StringParam name)
  {
    return this->Owner->AddAttribute(name);
  }

  //***************************************************************************
  ReflectionObject::ReflectionObject() :
    IsHidden(false),
    UserData(nullptr)
  {
  }

  //***************************************************************************
  Attribute* ReflectionObject::HasAttribute(StringParam name)
  {
    // Walk through the array of attributes (usually small)
    for (size_t i = 0; i < this->Attributes.Size(); ++i)
    {
      // If we found a matching name, then we do have that attribute
      Attribute* attribute = &this->Attributes[i];
      if (attribute->Name == name)
        return attribute;
    }

    // If we got here, we didn't find the attribute
    return nullptr;
  }
  
  //***************************************************************************
  Attribute* ReflectionObject::AddAttribute(StringParam name)
  {
    Attribute& attribute = this->Attributes.PushBack();
    attribute.Name = name;
    return &attribute;
  }

  //***************************************************************************
  ReflectionObject* ReflectionObject::AddAttributeChainable(StringParam name)
  {
    Attribute& attribute = this->Attributes.PushBack();
    attribute.Name = name;
    return this;
  }
  
  //***************************************************************************
  Type* ReflectionObject::GetTypeOrNull()
  {
    return nullptr;
  }

  //***************************************************************************
  Member::Member() :
    IsStatic(false),
    Owner(nullptr)
  {
  }

  //***************************************************************************
  bool Member::ValidateInstanceHandle(const Any& instance, Handle& thisHandle)
  {
    if (this->IsStatic)
      return true;

    Type* type = instance.StoredType;
    if (type != nullptr)
    {
      if (type->IsA(this->Owner))
      {
        BoundType* boundType = Type::DynamicCast<BoundType*>(type);
        if (boundType != nullptr && boundType->CopyMode == TypeCopyMode::ValueType)
        {
          ZilchTodo("This is unsafe, should this be allocated and boxed?");
          HandleManager* manager = HandleManagers::GetInstance().GetManager(ZilchManagerId(PointerManager));
          thisHandle.StoredType = boundType;
          thisHandle.Manager = manager;
          manager->ObjectToHandle(instance.GetData(), boundType, thisHandle);
        }
        else
        {
          thisHandle = *(Handle*)instance.GetData();
        }
        return true;
      }
    }

    ExecutableState::CallingState->ThrowException("The 'this' instance handle was either null or not of the correct type");
    return false;
  }
  
  //***************************************************************************
  Library* Member::GetOwningLibrary()
  {
    return this->Owner->SourceLibrary;
  }

  //***************************************************************************
  MemberOptions::Enum Member::GetMemberOptions()
  {
    if (this->IsStatic)
      return MemberOptions::Static;
    return MemberOptions::None;
  }

  //***************************************************************************
  Property::Property() :
    PropertyType(nullptr),
    Get(nullptr),
    Set(nullptr),
    IsHiddenWhenNull(false)
  {
  }
  
  //***************************************************************************
  Type* Property::GetTypeOrNull()
  {
    return this->PropertyType;
  }
  
  //***************************************************************************
  bool Property::IsReadOnly()
  {
    return (this->Set == nullptr);
  }
  
  //***************************************************************************
  Any Property::GetValue(const Any& instance)
  {
    ExecutableState* state = ExecutableState::CallingState;

    if (this->Get == nullptr)
    {
      state->ThrowException("The property does not have a getter");
      return Any();
    }

    Handle thisHandle;
    if (this->ValidateInstanceHandle(instance, thisHandle) == false)
      return Any();

    // There are no arguments to copy to the stack (other than the this)
    ExceptionReport& report = state->GetCallingReport();
    Call call(this->Get, state);
    if (this->IsStatic == false)
      call.SetHandle(Call::This, thisHandle);

    // Finally invoke the function and get our result back
    call.Invoke(report);

    if (report.HasThrownExceptions())
      return Any();
    
    // Fetch the return value and construct and any from it
    byte* returnValue = call.GetReturnUnchecked();
    return Any(returnValue, this->PropertyType);
  }

  //***************************************************************************
  void Property::SetValue(const Any& instance, const Any& value)
  {
    ExecutableState* state = ExecutableState::CallingState;

    if (this->Set == nullptr)
      return state->ThrowException("The property does not have a setter");
    
    Handle thisHandle;
    if (this->ValidateInstanceHandle(instance, thisHandle) == false)
      return;

    // Validate that the argument is of the same type (or raw convertable)
    Type* expectedType = this->PropertyType;
    Type* argumentType = value.StoredType;

    // Look up a cast operator between the two types
    // Note that if the types are the same, a cast always technically exists of 'Raw' type
    // This ALSO gives us an invalid cast and handles if the user gave us a empty any (with a null StoredType)
    CastOperator cast = Shared::GetInstance().GetCastOperator(argumentType, expectedType);
    if (cast.IsValid == false || cast.Operation != CastOperation::Raw)
    {
      String message = String::Format
      (
        "The setter expected the type '%s' but was given '%s' (which could not be raw-converted)",
        expectedType->ToString().c_str(),
        argumentType->ToString().c_str()
      );
      return state->ThrowException(message);
    }

    // Now call the function by copying all the arguments to the Zilch stack
    ExceptionReport& report = state->GetCallingReport();
    Call call(this->Set, state);
    if (this->IsStatic == false)
      call.SetHandle(Call::This, thisHandle);

    // The call will assert because we don't bother to mark all parameters as set (we validated already)
    call.DisableParameterChecks();

    // Copy the value from the any to the Zilch stack (as its actual value)
    byte* stackParameter = call.GetParameterUnchecked(0);
    const byte* argumentData = value.GetData();
    value.StoredType->GenericCopyConstruct(stackParameter, argumentData);

    // Finally invoke the function and get our result back
    call.Invoke(report);
    return;
  }

  //***************************************************************************
  Field::Field() :
    Offset(0),
    Initializer(nullptr)
  {
  }

  //***************************************************************************
  Variable::Variable() :
    Local(0),
    ResultType(Core::GetInstance().ErrorType),
    Owner(nullptr)
  {
  }

  //***************************************************************************
  Library* Variable::GetOwningLibrary()
  {
    return this->Owner->Owner->SourceLibrary;
  }

  //***************************************************************************
  Type* Variable::GetTypeOrNull()
  {
    return this->ResultType;
  }

  //***************************************************************************
  SendsEvent::SendsEvent() :
    Owner(Core::GetInstance().ErrorType),
    SentType(Core::GetInstance().ErrorType),
    EventProperty(nullptr)
  {
  }

  //***************************************************************************
  SendsEvent::SendsEvent(BoundType* owner, StringParam name, BoundType* sentType) :
    Owner(owner),
    Name(name),
    SentType(sentType),
    EventProperty(nullptr)
  {
  }

  //***************************************************************************
  Library* SendsEvent::GetOwningLibrary()
  {
    return this->Owner->SourceLibrary;
  }

  //***************************************************************************
  Type* SendsEvent::GetTypeOrNull()
  {
    return this->SentType;
  }
}