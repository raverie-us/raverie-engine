/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchDefineExternalBaseType(Members::Enum, TypeCopyMode::ValueType, builder, type)
  {
    ZilchFullBindEnum(builder, type, SpecialType::Flags);
    ZilchFullBindEnumValue(builder, type, Members::Instance,                "Instance");
    ZilchFullBindEnumValue(builder, type, Members::Static,                  "Static");
    ZilchFullBindEnumValue(builder, type, Members::Inherited,               "Inherited");
    ZilchFullBindEnumValue(builder, type, Members::Extension,               "Extension");
    ZilchFullBindEnumValue(builder, type, Members::InstanceStatic,          "InstanceStatic");
    ZilchFullBindEnumValue(builder, type, Members::InheritedInstanceStatic, "InheritedInstanceStatic");
    ZilchFullBindEnumValue(builder, type, Members::InheritedInstance,       "InheritedInstance");
    ZilchFullBindEnumValue(builder, type, Members::InheritedStatic,         "InheritedStatic");
    ZilchFullBindEnumValue(builder, type, Members::All,                     "All");
  }

  //***************************************************************************
  ZilchDefineType(Type, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);
    ZilchFullBindMethod(builder, type, &Type::IsA, ZilchNoOverload, "IsA", "baseType");
    ZilchFullBindMethod(builder, type, &Type::IsCastableTo, ZilchNoOverload, "IsCastableTo", "toType");
    ZilchFullBindMethod(builder, type, &Type::IsRawCastableTo, ZilchNoOverload, "IsRawCastableTo", "toType");

    ZilchFullBindGetterSetter(builder, type, &Type::ToString, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "Name");

    ZilchFullBindGetterSetter(builder, type, &Type::IsHandle, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsHandle");
    ZilchFullBindGetterSetter(builder, type, &Type::IsValue, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsValue");
    ZilchFullBindGetterSetter(builder, type, &Type::IsDelegate, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsDelegate");
    ZilchFullBindGetterSetter(builder, type, &Type::IsEnum, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsEnum");
    ZilchFullBindGetterSetter(builder, type, &Type::IsFlags, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsFlags");
    ZilchFullBindGetterSetter(builder, type, &Type::IsEnumOrFlags, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsEnumOrFlags");
    ZilchFullBindGetterSetter(builder, type, &Type::IsAny, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsAny");

    ZilchFullBindField(builder, type, &BoundType::SourceLibrary, "Library", PropertyBinding::Get);
  }
  
  //***************************************************************************
  ZilchDefineType(AnyType, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);
  }
  
  //***************************************************************************
  ZilchDefineType(IndirectionType, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);
    ZilchFullBindField(builder, type, &IndirectionType::ReferencedType, "ReferencedType", PropertyBinding::Get);
  }
  
  //***************************************************************************
  ZilchDefineType(DelegateType, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);

    ZilchFullBindGetterSetter(builder, type, &DelegateType::GetSignatureString, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "Signature");
    ZilchFullBindGetterSetter(builder, type, &DelegateType::GetParameters, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "Parameters");
    ZilchFullBindField(builder, type, &DelegateType::Return, "Return", PropertyBinding::Get);
  }
  
  //***************************************************************************
  ZilchDefineType(DelegateParameter, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);
    ZilchFullBindField(builder, type, &DelegateParameter::Name, "Name", PropertyBinding::Get);
    ZilchFullBindField(builder, type, &DelegateParameter::ParameterType, "Type", PropertyBinding::Get);
  }

  //***************************************************************************
  ZilchDefineType(BoundType, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);
    ZilchFullBindField(builder, type, &BoundType::BaseType, "BaseType", PropertyBinding::Get);
    ZilchFullBindField(builder, type, &BoundType::TemplateBaseName, "TemplateBaseName", PropertyBinding::Get);
    ZilchFullBindField(builder, type, &BoundType::Native, "IsNative", PropertyBinding::Get);
    ZilchFullBindGetterSetter(builder, type, &BoundType::IsTypeOrBaseNative, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsTypeOrBaseNative");
    
    ZilchFullBindField(builder, type, &BoundType::Destructor, "Destructor", PropertyBinding::Get);
    ZilchFullBindField(builder, type, &BoundType::PreConstructor, "PreConstructor", PropertyBinding::Get);

    ZilchFullBindMethod(builder, type, &BoundType::InstantiatePreConstructedObject, ZilchNoOverload, "InstantiatePreConstructedObject", ZilchNoNames);

    ZilchFullBindGetterSetter(builder, type, &BoundType::GetGetterSetters,      (MemberRange<GetterSetter>  (BoundType::*)()), ZilchNoSetter, ZilchNoOverload, "GetterSetters");
    ZilchFullBindGetterSetter(builder, type, &BoundType::GetFields,             (MemberRange<Field>         (BoundType::*)()), ZilchNoSetter, ZilchNoOverload, "Fields");
    ZilchFullBindGetterSetter(builder, type, &BoundType::GetProperties,         (MemberRange<Property>      (BoundType::*)()), ZilchNoSetter, ZilchNoOverload, "Properties");
    ZilchFullBindGetterSetter(builder, type, &BoundType::GetFunctions,          (MemberRange<Function>      (BoundType::*)()), ZilchNoSetter, ZilchNoOverload, "Functions");
    ZilchFullBindGetterSetter(builder, type, &BoundType::GetMembers,            (MemberRange<Member>        (BoundType::*)()), ZilchNoSetter, ZilchNoOverload, "Members");
    ZilchFullBindGetterSetter(builder, type, &BoundType::GetDefaultConstructor, (Function*                  (BoundType::*)()), ZilchNoSetter, ZilchNoOverload, "DefaultConstructor");

    ZilchFullBindMethod(builder, type, &BoundType::GetGetterSetters,      (MemberRange<GetterSetter>  (BoundType::*)(Members::Enum)),                             "GetGetterSetters",       "options");
    ZilchFullBindMethod(builder, type, &BoundType::GetProperties,         (MemberRange<Property>      (BoundType::*)(Members::Enum)),                             "GetProperties",          "options");
    ZilchFullBindMethod(builder, type, &BoundType::GetFields,             (MemberRange<Field>         (BoundType::*)(Members::Enum)),                             "GetFields",              "options");
    ZilchFullBindMethod(builder, type, &BoundType::GetFunctions,          (MemberRange<Function>      (BoundType::*)(Members::Enum)),                             "GetFunctions",           "options");
    ZilchFullBindMethod(builder, type, &BoundType::GetFunctions,          (MemberRange<Function>      (BoundType::*)(StringParam, Members::Enum)),                "GetFunctions",           "name, options");
    ZilchFullBindMethod(builder, type, &BoundType::GetMembers,            (MemberRange<Member>        (BoundType::*)(Members::Enum)),                             "GetMembers",             "options");
    ZilchFullBindMethod(builder, type, &BoundType::GetGetterSetter,       (GetterSetter*              (BoundType::*)(StringParam)),                               "GetGetterSetter",        "name");
    ZilchFullBindMethod(builder, type, &BoundType::GetGetterSetter,       (GetterSetter*              (BoundType::*)(StringParam, Members::Enum)),                "GetGetterSetter",        "name, options");
    ZilchFullBindMethod(builder, type, &BoundType::GetGetterSetter,       (GetterSetter*              (BoundType::*)(StringParam, Type*, Members::Enum)),         "GetGetterSetter",        "name, declaredType, options");
    ZilchFullBindMethod(builder, type, &BoundType::GetProperty,           (Property*                  (BoundType::*)(StringParam)),                               "GetProperty",            "name");
    ZilchFullBindMethod(builder, type, &BoundType::GetProperty,           (Property*                  (BoundType::*)(StringParam, Members::Enum)),                "GetProperty",            "name, options");
    ZilchFullBindMethod(builder, type, &BoundType::GetProperty,           (Property*                  (BoundType::*)(StringParam, Type*, Members::Enum)),         "GetProperty",            "name, declaredType, options");
    ZilchFullBindMethod(builder, type, &BoundType::GetField,              (Field*                     (BoundType::*)(StringParam)),                               "GetField",               "name");
    ZilchFullBindMethod(builder, type, &BoundType::GetField,              (Field*                     (BoundType::*)(StringParam, Members::Enum)),                "GetField",               "name, options");
    ZilchFullBindMethod(builder, type, &BoundType::GetField,              (Field*                     (BoundType::*)(StringParam, Type*, Members::Enum)),         "GetField",               "name, declaredType, options");
    ZilchFullBindMethod(builder, type, &BoundType::GetFunction,           (Function*                  (BoundType::*)(StringParam)),                               "GetFunction",            "name");
    ZilchFullBindMethod(builder, type, &BoundType::GetFunction,           (Function*                  (BoundType::*)(StringParam, Members::Enum)),                "GetFunction",            "name, options");
    ZilchFullBindMethod(builder, type, &BoundType::GetFunction,           (Function*                  (BoundType::*)(StringParam, DelegateType*, Members::Enum)), "GetFunction",            "name, signatureType, options");
    ZilchFullBindMethod(builder, type, &BoundType::GetMember,             (Member*                    (BoundType::*)(StringParam)),                               "GetMember",              "name");
    ZilchFullBindMethod(builder, type, &BoundType::GetMember,             (Member*                    (BoundType::*)(StringParam, Members::Enum)),                "GetMember",              "name, options");
    ZilchFullBindMethod(builder, type, &BoundType::GetMember,             (Member*                    (BoundType::*)(StringParam, Type*, Members::Enum)),         "GetMember",              "name, declaredType, options");
    ZilchFullBindMethod(builder, type, &BoundType::GetDefaultConstructor, (Function*                  (BoundType::*)(bool)),                                      "GetDefaultConstructor",  "inherited");
    ZilchFullBindMethod(builder, type, &BoundType::GetConstructor,        (Function*                  (BoundType::*)(DelegateType*, bool)),                       "GetConstructor",         "signatureType, inherited");
  }

  //***************************************************************************
  Type::Type() :
    SourceLibrary(nullptr)
  {
  }
  
  //***************************************************************************
  Library* Type::GetOwningLibrary()
  {
    return this->SourceLibrary;
  }
  
  //***************************************************************************
  Type* Type::GetTypeOrNull()
  {
    return this;
  }

  //***************************************************************************
  Resolver Resolver::Instance(BoundType* type)
  {
    Resolver resolver;
    resolver.IsStatic               = false;
    resolver.TypeInstance           = type;
    resolver.GetOverloadedFunctions = &BoundType::GetOverloadedInstanceFunctions;
    resolver.GetField               = &BoundType::GetInstanceField;
    resolver.GetGetterSetter        = &BoundType::GetInstanceGetterSetter;
    return resolver;
  }

  //***************************************************************************
  Resolver Resolver::Static(BoundType* type)
  {
    Resolver resolver;
    resolver.IsStatic               = true;
    resolver.TypeInstance           = type;
    resolver.GetOverloadedFunctions = &BoundType::GetOverloadedStaticFunctions;
    resolver.GetField               = &BoundType::GetStaticField;
    resolver.GetGetterSetter        = &BoundType::GetStaticGetterSetter;
    return resolver;
  }

  //***************************************************************************
  size_t Type::GetCopyableSize() const
  {
    return this->GetAllocatedSize();
  }

  //***************************************************************************
  byte* Type::GenericGetMemory(const byte* value) const
  {
    return const_cast<byte*>(value);
  }

  //***************************************************************************
  bool Type::IsIndirectionType(Type* type)
  {
    // See if the type is an indirection type...
    IndirectionType* indirectionType = Type::DynamicCast<IndirectionType*>(type);
    return (indirectionType != nullptr);
  }

  //***************************************************************************
  bool Type::IsHandleType(Type* type)
  {
    // See if the type is an indirection type...
    if (IsIndirectionType(type))
      return true;

    // Otherwise, get the type as a named type...
    BoundType* boundType = Type::DynamicCast<BoundType*>(type);

    // If the type is a handle...
    return boundType != nullptr && boundType->CopyMode == TypeCopyMode::ReferenceType;
  }

  //***************************************************************************
  bool Type::IsValueType(Type* type)
  {
    // Otherwise, get the type as a named type...
    BoundType* boundType = Type::DynamicCast<BoundType*>(type);

    // If the type is a value...
    return boundType != nullptr && boundType->CopyMode == TypeCopyMode::ValueType;
  }

  //***************************************************************************
  bool Type::IsDelegateType(Type* type)
  {
    // See if the type is a delegate type...
    DelegateType* delegateType = Type::DynamicCast<DelegateType*>(type);
    return (delegateType != nullptr);
  }

  //***************************************************************************
  bool Type::IsEnumType(Type* type)
  {
    // Otherwise, get the type as a named type...
    BoundType* boundType = Type::DynamicCast<BoundType*>(type);

    // If the type is a bound type set to enumeration...
    return boundType != nullptr && boundType->SpecialType == SpecialType::Enumeration;
  }

  //***************************************************************************
  bool Type::IsFlagsType(Type* type)
  {
    // Otherwise, get the type as a named type...
    BoundType* boundType = Type::DynamicCast<BoundType*>(type);

    // If the type is a bound type set to flags...
    return boundType != nullptr && boundType->SpecialType == SpecialType::Flags;
  }

  //***************************************************************************
  bool Type::IsEnumOrFlagsType(Type* type)
  {
    // Otherwise, get the type as a named type...
    BoundType* boundType = Type::DynamicCast<BoundType*>(type);

    // If the type is a bound type set to flags or enumeration...
    return boundType != nullptr && (boundType->SpecialType == SpecialType::Flags || boundType->SpecialType == SpecialType::Enumeration);
  }

  //***************************************************************************
  bool Type::IsAnyType(Type* type)
  {
    // See if the type is a any type...
    AnyType* anyType = Type::DynamicCast<AnyType*>(type);
    return (anyType != nullptr);
  }

  //***************************************************************************
  Type* Type::GetBaseType(Type* type)
  {
    // Attempt to get the given type as a bound type
    BoundType* boundType = Type::GetBoundType(type);

    // If the type was a bound type then return its base (could be null if it has no base class)
    if (boundType != nullptr)
      return boundType->BaseType;

    // If we got here then either the given type doesn't support base types (delegates, etc)
    return nullptr;
  }
  
  //***************************************************************************
  BoundType* Type::GetBoundType(Type* handleType)
  {
    // First check if the type is already a bound type and directly return it
    BoundType* boundType = Type::DynamicCast<BoundType*>(handleType);
    if (boundType != nullptr)
      return boundType;

    // An indirection type is generally a struct with the 'ref' keyword before it (making it a handle type)
    // We can access members on indirection types by going through the referenced bound type
    IndirectionType* indirectionType = Type::DynamicCast<IndirectionType*>(handleType);
    if (indirectionType != nullptr)
      return indirectionType->ReferencedType;

    // Otherwise the type was not a bound type (and did not contain one)
    return nullptr;
  }

  //***************************************************************************
  BoundType* Type::GetHandleType(Type* handleType)
  {
    // See if the type is an indirection type...
    IndirectionType* indirectionType = Type::DynamicCast<IndirectionType*>(handleType);

    // If so, then it is a handle!
    if (indirectionType != nullptr)
      return indirectionType->ReferencedType;

    // Otherwise, get the type as a named type...
    BoundType* boundType = Type::DynamicCast<BoundType*>(handleType);

    // If the type is a handle...
    if (boundType != nullptr && boundType->CopyMode == TypeCopyMode::ReferenceType)
      return boundType;

    // If it was neither, then it is not a handle type
    return nullptr;
  }

  //***************************************************************************
  bool Type::IsSame(Type* a, Type* b)
  {
    // If the two pointers are exactly the same, then of course its the same!
    if (a == b)
    {
      // Early out and return that they are the same :D
      return true;
    }

    // Compare the hashes (we do this anyways in the linking phase, might as well do it here too)
    GuidType hashA = a->Hash();
    GuidType hashB = b->Hash();
    bool result = (hashA == hashB);

    // We better not get the same hash when hashing two different types...
    ErrorIf(result && ZilchVirtualTypeId(a) != ZilchVirtualTypeId(b),
      "The same hash/guid was computed for two different types!");
    
    // Return the result (true if they are the same, false otherwise)
    return result;
  }


  //***************************************************************************
  bool Type::BoundIsA(BoundType* type, BoundType* base)
  {
    // Loop until the type chain becomes empty
    while (type != nullptr)
    {
      // If the base type is the same as the current type...
      ZilchTodo("Using the names of the BoundType to compare isn't technically correct");
      if (base == type || base->Name == type->Name)
        return true;

      // Iterate to the next parent
      type = type->BaseType;
    }

    // Otherwise, the given type does not inherit from base
    return false;
  }

  //***************************************************************************
  bool Type::IndirectionIsA(IndirectionType* type, IndirectionType* base)
  {
    // The is-a relationship stands for indirect types (the types they point at)
    return BoundIsA
    (
      type->ReferencedType,
      base->ReferencedType
    );
  }

  //***************************************************************************
  bool Type::GenericIsA(Type* type, Type* base)
  {
    // Get the first type as a bound type
    BoundType* boundType = DynamicCast<BoundType*>(type);

    // If the first type is a bound type...
    if (boundType != nullptr)
    {
      // Then hopefully the base is a bound type also
      BoundType* boundBase = DynamicCast<BoundType*>(base);

      // If it's not, then these types do not match
      if (boundBase == nullptr)
        return false;

      // Otherwise, check for the IsA relationship on two bound types
      return BoundIsA(boundType, boundBase);
    }

    // The first type wasn't a bound type, but is it an indirect type?
    IndirectionType* indirectType = DynamicCast<IndirectionType*>(type);

    // If the first type is an indirect type...
    if (indirectType != nullptr)
    {
      // Then hopefully the base is an indirect type also
      IndirectionType* indirectBase = DynamicCast<IndirectionType*>(base);

      // If it's not, then these types do not match
      if (indirectBase == nullptr)
        return false;

      // Otherwise, check for the IsA relationship on two bound types
      return IndirectionIsA(indirectType, indirectBase);
    }

    // If we got here, then we don't know what type these are
    return Type::IsSame(type, base);
  }
  
  //***************************************************************************
  bool Type::IsCastableTo(Type* toType)
  {
    return Shared::GetInstance().GetCastOperator(this, toType).IsValid;
  }

  //***************************************************************************
  bool Type::IsRawCastableTo(Type* toType)
  {
    CastOperator cast = Shared::GetInstance().GetCastOperator(this, toType);
    if (cast.IsValid == false)
      return false;

    return cast.Operation == CastOperation::Raw;
  }
  
  //***************************************************************************
  bool Type::IsA(Type* base)  { return Type::GenericIsA(this, base); }
  bool Type::IsHandle()       { return Type::IsHandleType(this); }
  bool Type::IsValue()        { return Type::IsValueType(this); }
  bool Type::IsDelegate()     { return Type::IsDelegateType(this); }
  bool Type::IsEnum()         { return Type::IsEnumType(this); }
  bool Type::IsFlags()        { return Type::IsFlagsType(this); }
  bool Type::IsEnumOrFlags()  { return Type::IsEnumOrFlagsType(this); }
  bool Type::IsAny()          { return Type::IsAnyType(this); }

  //***************************************************************************
  IndirectionType::IndirectionType() :
    ReferencedType(nullptr)
  {
  }

  //***************************************************************************
  bool IndirectionType::IsCopyComplex() const
  {
    // Handles are always a complex copy
    return true;
  }

  //***************************************************************************
  size_t IndirectionType::GetAllocatedSize() const
  {
    return sizeof(Handle);
  }

  //***************************************************************************
  GuidType IndirectionType::Hash() const
  {
    return 118465894487008423 ^ this->ReferencedType->Hash();
  }
  
  //***************************************************************************
  String IndirectionType::ToString() const
  {
    // Create a string builder since we're doing some concatenation
    StringBuilder output;

    // Add the 'ref' keyword
    output += Grammar::GetKeywordOrSymbol(Grammar::Ref);
    output += " ";
    
    // Add the type we're referencing
    output += this->ReferencedType->ToString();

    // Output the string
    return output.ToString();
  }
  
  //***************************************************************************
  String IndirectionType::GetShortLowerCamelCaseName() const
  {
    return this->ReferencedType->GetShortLowerCamelCaseName();
  }

  //***************************************************************************
  void IndirectionType::GenericDefaultConstruct(byte* toConstruct) const
  {
    // Construct a null / empty handle
    new (toConstruct) Handle();
  }

  //***************************************************************************
  void IndirectionType::GenericCopyConstruct(byte* to, const byte* from) const
  {
    // Indirect types are always represented as handles (just perform a handle copy)
    new (to) Handle(*(Handle*)from);
  }

  //***************************************************************************
  void IndirectionType::GenericDestruct(byte* value) const
  {
    // Destroy the handle object
    ((Handle*)value)->~Handle();
  }
  
  //***************************************************************************
  int IndirectionType::GenericHash(const byte* value) const
  {
    return ((Handle*)value)->Hash();
  }

  //***************************************************************************
  String IndirectionType::GenericToString(const byte* value) const
  {
    // Grab the handle primitive
    Handle* handle = (Handle*)value;

    // By default, we know at least a base class type of what we're referencing
    const BoundType* type = this->ReferencedType;

    // The handle may store a more derived type inside it, if so, we should use that to print instead (virtual)
    if (handle->StoredType != nullptr)
      type = handle->StoredType;
    
    // Get a pointer to the data of the object
    byte* data = handle->Dereference();

    // If converting a null handle to a string... let the user know it's null, and what type it is
    if (data == nullptr)
      return BuildString("(null) ", type->ToString());

    // Run the user provided to-string function
    return type->ToStringFunction(type, data);
  }

  //***************************************************************************
  bool IndirectionType::GenericEquals(const byte* lhs, const byte* rhs) const
  {
    // Compare the two handles
    Handle& lHandle = *((Handle*)lhs);
    Handle& rHandle = *((Handle*)rhs);
    return lHandle == rHandle;
  }
  
  //***************************************************************************
  byte* IndirectionType::GenericGetMemory(const byte* value) const
  {
    // Get a pointer to the data of the object (indirect types are always handles)
    return ((Handle*)value)->Dereference();
  }

  //***************************************************************************
  Type* IndirectionType::GenericGetVirtualType(const byte* value) const
  {
    // Grab the handle
    Handle* handle = ((Handle*)value);
    
    // If it has no type, then return our own type
    if (handle->StoredType == nullptr)
      return (Type*)(this);

    // Otherwise return the type stored on the handle
    return handle->StoredType;
  }

  //***************************************************************************
  Composition* IndirectionType::GetBaseComposition()
  {
    return this->ReferencedType;
  }

  //***************************************************************************
  AnyType::AnyType()
  {
  }

  //***************************************************************************
  bool AnyType::IsCopyComplex() const
  {
    // The 'Any' class has a proper copy constructor
    return true;
  }

  //***************************************************************************
  size_t AnyType::GetAllocatedSize() const
  {
    return sizeof(Any);
  }

  //***************************************************************************
  GuidType AnyType::Hash() const
  {
    return 764373915523575397;
  }

  //***************************************************************************
  String AnyType::ToString() const
  {
    return Grammar::GetKeywordOrSymbol(Grammar::Any);
  }
  
  //***************************************************************************
  String AnyType::GetShortLowerCamelCaseName() const
  {
    return Grammar::GetKeywordOrSymbol(Grammar::Any);
  }

  //***************************************************************************
  void AnyType::GenericDefaultConstruct(byte* toConstruct) const
  {
    // Construct a default instance of 'any', should be null
    new (toConstruct) Any();
  }

  //***************************************************************************
  void AnyType::GenericCopyConstruct(byte* to, const byte* from) const
  {
    // Copy construct the 'any' to the given location
    new (to) Any(*(Any*)from);
  }

  //***************************************************************************
  void AnyType::GenericDestruct(byte* value) const
  {
    // Destroy the any type
    ((Any*)value)->~Any();
  }

  //***************************************************************************
  int AnyType::GenericHash(const byte* value) const
  {
    return ((Any*)value)->Hash();
  }

  //***************************************************************************
  String AnyType::GenericToString(const byte* value) const
  {
    return ((Any*)value)->ToString();
  }

  //***************************************************************************
  bool AnyType::GenericEquals(const byte* lhs, const byte* rhs) const
  {
    // Compare the two any types
    Any& lAny = *((Any*)lhs);
    Any& rAny = *((Any*)rhs);
    return lAny == rAny;
  }

  //***************************************************************************
  byte* AnyType::GenericGetMemory(const byte* value) const
  {
    // Get access to the any primitive
    Any* any = ((Any*)value);

    // Get the data pointed at by the any
    const byte* storedValue = any->GetData();

    // If we have a valid stored type... recursively get its memory
    if (any->StoredType != nullptr)
      return any->StoredType->GenericGetMemory(storedValue);

    // Otherwise we had nothing stored... just return ourselves
    // This MUST match the 'GenericGetVirtualType' behavior!
    return (byte*)value;
  }
  
  //***************************************************************************
  Type* AnyType::GenericGetVirtualType(const byte* value) const
  {
    // Get access to the any primitive
    Any* any = ((Any*)value);

    // If we have a valid stored type... recursively get its memory
    if (any->StoredType != nullptr)
      return any->StoredType->GenericGetVirtualType(value);

    // Otherwise we had nothing stored... just return ourselves
    // This MUST match the 'GenericGetMemory' behavior!
    return (Type*)this;
  }

  //***************************************************************************
  EventHandler* BoundType::GetEventHandler(const byte* data)
  {
    // Walk up the entire type hierarchy starting with this type
    BoundType* type = this;
    while (type != nullptr)
    {
      // If the type has a defined function for getting an event handler then attempt to call it
      if (type->GetEventHandlerFunction != nullptr)
      {
        // This may still result in null if for some reason the user determined there was no event handler
        EventHandler* handler = type->GetEventHandlerFunction(this, data);
        if (handler != nullptr)
          return handler;
      }

      // Iterate to the base type (may be null)
      type = type->BaseType;
    }

    // Otherwise, we know there's no way to get the handler
    return nullptr;
  }

  //***************************************************************************
  bool BoundType::IsTypeOrBaseNative()
  {
    // Start at this class and walk up all base classes and check for the Native flag
    BoundType* baseIterator = this;
    while (baseIterator != nullptr)
    {
      if (baseIterator->Native)
        return true;

      baseIterator = baseIterator->BaseType;
    }
    return false;
  }

  //***************************************************************************
  String BoundType::DefaultTypeToString(const BoundType* type, const byte* data)
  {
    return String::Format("%s (%p)", type->Name.c_str(), data);
  }

  //***************************************************************************
  BoundType::BoundType(BoundTypeAssertFn nativeBindingAssert) :
    BaseType(nullptr),
    BoundNativeVirtualCount(0),
    HandleManager(ZilchManagerId(HeapManager)),
    PostDestructor(nullptr),
    ToStringFunction(DefaultTypeToString),
    GetEventHandlerFunction(nullptr),
    PreConstructor(nullptr),
    Destructor(nullptr),
    SpecialType(SpecialType::Standard),
    CreatableInScript(true),
    Native(true),
    IgnoreNativeDestructor(false),
    Sealed(true),
    IndirectType(nullptr),
    Size(0),
    RawNativeVirtualCount(0),
    CopyMode(TypeCopyMode::ReferenceType),
    AssertOnInvalidBinding(nativeBindingAssert),
    DefaultEnumValue(0),
    DefaultEnumProperty(nullptr)
  {
    // When a BoundType is created via native binding in C++, e.g. ZilchTypeId(X), we want the user
    // to be able to debug whether the type is properly initialized
    if (nativeBindingAssert)
    {
      NativeBindingList& list = NativeBindingList::GetInstance();
      list.Lock.Lock();
      list.AllNativeBoundTypes.Insert(this);
      list.Lock.Unlock();
    }
  }

  //***************************************************************************
  BoundType::~BoundType()
  {
    // This will only be set if this represents a native type bound with ZilchDefineType, etc
    if (this->AssertOnInvalidBinding)
    {
      NativeBindingList& list = NativeBindingList::GetInstance();
      list.Lock.Lock();
      list.AllNativeBoundTypes.Erase(this);
      list.Lock.Unlock();
    }

    ZilchForEach(SendsEvent* sendsEvent, this->SendsEvents)
    {
      delete sendsEvent;
    }
  }

  //***************************************************************************
  GuidType BoundType::Hash() const
  {
    return this->Name.Hash();
  }
  
  //***************************************************************************
  bool BoundType::IsCopyComplex() const
  {
    // We've only got a complex copy if we're a reference type
    return (this->CopyMode == TypeCopyMode::ReferenceType);
  }

  //***************************************************************************
  size_t BoundType::GetCopyableSize() const
  {
    if (this->CopyMode == TypeCopyMode::ReferenceType)
    {
      return sizeof(Handle);
    }
    else
    {
      return this->GetAllocatedSize();
    }
  }

  //***************************************************************************
  size_t BoundType::GetAllocatedSize() const
  {
    // Return the size of the class since it's already been computed
    ErrorIf(this->Size == UndeterminedSize && this->SourceLibrary->TolerantMode == false,
      "Attempting to get the size of a class when it has not been determined!");
    return this->Size;
  }
  
  //***************************************************************************
  String BoundType::ToString() const
  {
    return this->Name;
  }
  
  //***************************************************************************
  String BoundType::GetShortLowerCamelCaseName() const
  {
    return LibraryBuilder::FixIdentifier(this->Name, TokenCheck::IsLower);
  }

  //***************************************************************************
  Composition* BoundType::GetBaseComposition()
  {
    return this->BaseType;
  }

  //***************************************************************************
  void BoundType::GenericDefaultConstruct(byte* toConstruct) const
  {
    if (this->CopyMode == TypeCopyMode::ReferenceType)
    {
      // Create a null handle if it's a reference type...
      new (toConstruct) Handle();
    }
    else
    {
      // Otherwise, clear the memory out to zeros (a valid configuration for all structs)
      memset(toConstruct, 0, this->GetCopyableSize());
    }
  }

  //***************************************************************************
  void BoundType::GenericCopyConstruct(byte* to, const byte* from) const
  {
    if (this->CopyMode == TypeCopyMode::ReferenceType)
    {
      // Copy the handle to it's destination position
      new (to) Handle(*(Handle*)from);
    }
    else
    {
      // Copy the value type directly
      memcpy(to, from, this->GetCopyableSize());
    }
  }

  //***************************************************************************
  void BoundType::GenericDestruct(byte* value) const
  {
    // We only need to do anything if this is a reference type
    // Value types do not need to be released
    if (this->CopyMode == TypeCopyMode::ReferenceType)
    {
      // Copy the handle to it's destination position
      ((Handle*)value)->~Handle();
    }
  }
  
  //***************************************************************************
  int BoundType::GenericHash(const byte* value) const
  {
    // If this is a reference type, it means it's a handle
    if (this->CopyMode == TypeCopyMode::ReferenceType)
    {
      // Hash the handle and return that value
      return ((Handle*)value)->Hash();
    }
    else
    {
      // Otherwise generate a hash from the value-type memory
      return (int)HashString((const char*)value, this->Size);
    }
  }

  //***************************************************************************
  String BoundType::GenericToString(const byte* value) const
  { 
    // By default, we know at least a base class type of what we're referencing
    // For value types, this is always the case
    const BoundType* type = this;

    // For value types, we assume that the type is just the data we get (not for reference types!)
    byte* data = (byte*)value;

    // If this is a reference type (it may be virtual)
    if (this->CopyMode == TypeCopyMode::ReferenceType)
    {
      // Grab the handle primitive
      Handle* handle = (Handle*)value;

      // The handle may store a more derived type inside it, if so, we should use that to print instead (virtual)
      if (handle->StoredType != nullptr)
        type = handle->StoredType;
    
      // Get a pointer to the data of the object
      data = handle->Dereference();

      // If converting a null handle to a string... let the user know it's null, and what type it is
      if (data == nullptr)
        return BuildString("(null) ", type->ToString());
    }

    // Run the user provided to-string function
    return type->ToStringFunction(type, data);
  }

  //***************************************************************************
  bool BoundType::GenericEquals(const byte* lhs, const byte* rhs) const
  {
    // If this is a reference type, it means it's a handle
    if (this->CopyMode == TypeCopyMode::ReferenceType)
    {
      // Compare the two handles
      Handle& lHandle = *((Handle*)lhs);
      Handle& rHandle = *((Handle*)rhs);
      return lHandle == rHandle;
    }
    else
    {
      // Otherwise, do a comparison of the value-type memory
      return (memcmp(lhs, rhs, this->Size) == 0);
    }
  }

  //***************************************************************************
  byte* BoundType::GenericGetMemory(const byte* value) const
  {
    // If this is a reference type, it means it's a handle
    if (this->CopyMode == TypeCopyMode::ReferenceType)
    {
      // Hash the handle and return that value
      return ((Handle*)value)->Dereference();
    }
    else
    {
      // Otherwise this is a value type, so we just directly return the memory
      return const_cast<byte*>(value);
    }
  }
  
  //***************************************************************************
  Type* BoundType::GenericGetVirtualType(const byte* value) const
  {
    // If this is a reference type, it means it's a handle
    if (this->CopyMode == TypeCopyMode::ReferenceType)
    {
      // Grab the handle (this behavior matches indirect type!)
      Handle* handle = ((Handle*)value);
    
      // If it has no type, then return our own type
      if (handle->StoredType == nullptr)
        return (Type*)(this);

      // Otherwise return the type stored on the handle
      return handle->StoredType;
    }
    else
    {
      // Otherwise this is a value type, so we the type is always the same
      return (Type*)this;
    }
  }
  
  //***************************************************************************
  Any BoundType::InstantiatePreConstructedObject()
  {
    ExecutableState* state = ExecutableState::CallingState;
    if (this->CreatableInScript == false)
    {
      String message = String::Format("The type %s cannot be allocated within script", this->Name.c_str());
      state->ThrowException(message);
      return Any();
    }

    ExceptionReport& report = state->GetCallingReport();
    Handle result = state->AllocateHeapObject(this, report, HeapFlags::ReferenceCounted);
    return result;
  }

  //***************************************************************************
  bool BoundType::IsInitialized()
  {
    return !this->Name.Empty();
  }

  //***************************************************************************
  bool BoundType::IsInitializedAssert(const char* prependedMessage)
  {
    if (this->IsInitialized())
      return true;

    if (this->AssertOnInvalidBinding)
      this->AssertOnInvalidBinding(prependedMessage);
    else
      Error("%sA type was not bound or initialized properly", prependedMessage);
    return false;
  }

  //***************************************************************************
  Members::Enum CorrectMemberOptions(Members::Enum options)
  {
    // If neither instance or static is specified, then assume both
    if ((options & (Members::Static | Members::Instance)) == 0)
      options = (Members::Enum)(options | Members::Static | Members::Instance);
    return options;
  }

  //***************************************************************************
  MemberRange<GetterSetter> BoundType::GetGetterSetters()
  {
    return this->GetGetterSetters(Members::All);
  }

  //***************************************************************************
  MemberRange<GetterSetter> BoundType::GetGetterSetters(Members::Enum options)
  {
    return MemberRange<GetterSetter>(this, String(), nullptr, options);
  }


  //***************************************************************************
  MemberRange<Property> BoundType::GetProperties()
  {
    return this->GetProperties(Members::All);
  }
  
  //***************************************************************************
  MemberRange<Property> BoundType::GetProperties(Members::Enum options)
  {
    return MemberRange<Property>(this, String(), nullptr, options);
  }

  //***************************************************************************
  MemberRange<Field> BoundType::GetFields()
  {
    return this->GetFields(Members::All);
  }
  
  //***************************************************************************
  MemberRange<Field> BoundType::GetFields(Members::Enum options)
  {
    return MemberRange<Field>(this, String(), nullptr, options);
  }

  //***************************************************************************
  MemberRange<Function> BoundType::GetFunctions()
  {
    return this->GetFunctions(String(), Members::All);
  }

  //***************************************************************************
  MemberRange<Function> BoundType::GetFunctions(Members::Enum options)
  {
    return this->GetFunctions(String(), options);
  }

  //***************************************************************************
  MemberRange<Function> BoundType::GetFunctions(StringParam name, Members::Enum options)
  {
    return MemberRange<Function>(this, name, nullptr, options);
  }
  
  //***************************************************************************
  MemberRange<Member> BoundType::GetMembers()
  {
    return this->GetMembers(Members::All);
  }

  //***************************************************************************
  MemberRange<Member> BoundType::GetMembers(Members::Enum options)
  {
    return MemberRange<Member>(this, String(), nullptr, options);
  }

  //***************************************************************************
  GetterSetter* BoundType::GetGetterSetter(StringParam name)
  {
    return this->GetGetterSetter(name, Members::All);
  }

  //***************************************************************************
  GetterSetter* BoundType::GetGetterSetter(StringParam name, Members::Enum options)
  {
    return this->GetGetterSetter(name, nullptr, options);
  }

  //***************************************************************************
  GetterSetter* BoundType::GetGetterSetter(StringParam name, Type* declaredType, Members::Enum options)
  {
    return MemberRange<GetterSetter>(this, name, declaredType, options).First();
  }

  //***************************************************************************
  Property* BoundType::GetProperty(StringParam name)
  {
    return this->GetProperty(name, Members::All);
  }

  //***************************************************************************
  Property* BoundType::GetProperty(StringParam name, Members::Enum options)
  {
    return this->GetProperty(name, nullptr, options);
  }

  //***************************************************************************
  Property* BoundType::GetProperty(StringParam name, Type* declaredType, Members::Enum options)
  {
    return MemberRange<Property>(this, name, declaredType, options).First();
  }
  
  //***************************************************************************
  Field* BoundType::GetField(StringParam name)
  {
    return this->GetField(name, nullptr, Members::All);
  }
  
  //***************************************************************************
  Field* BoundType::GetField(StringParam name, Members::Enum options)
  {
    return this->GetField(name, nullptr, options);
  }
  
  //***************************************************************************
  Field* BoundType::GetField(StringParam name, Type* declaredType, Members::Enum options)
  {
    return MemberRange<Field>(this, name, declaredType, options).First();
  }
  
  //***************************************************************************
  Function* BoundType::GetFunction(StringParam name)
  {
    return this->GetFunction(name, Members::All);
  }
  
  //***************************************************************************
  Function* BoundType::GetFunction(StringParam name, Members::Enum options)
  {
    return this->GetFunction(name, nullptr, options);
  }
  
  //***************************************************************************
  Function* BoundType::GetFunction(StringParam name, DelegateType* type, Members::Enum options)
  {
    return MemberRange<Function>(this, name, type, options).First();
  }

  //***************************************************************************
  Member* BoundType::GetMember(StringParam name)
  {
    return this->GetMember(name, nullptr, Members::All);
  }

  //***************************************************************************
  Member* BoundType::GetMember(StringParam name, Members::Enum options)
  {
    return this->GetMember(name, nullptr, options);
  }

  //***************************************************************************
  Member* BoundType::GetMember(StringParam name, Type* declaredType, Members::Enum options)
  {
    return MemberRange<Member>(this, name, declaredType, options).First();
  }
  
  //***************************************************************************
  Function* BoundType::GetDefaultConstructor()
  {
    return this->GetDefaultConstructor(true);
  }
  
  //***************************************************************************
  Function* BoundType::GetDefaultConstructor(bool inherited)
  {
    if (inherited)
      return BoundType::GetDefaultConstructor(this->GetOverloadedInheritedConstructors());
    else
      return BoundType::GetDefaultConstructor(&this->Constructors);
  }

  //***************************************************************************
  Function* BoundType::GetConstructor(DelegateType* type, bool inherited)
  {
    if (type == nullptr)
      return this->GetDefaultConstructor(inherited);

    const FunctionArray* constructors = &this->Constructors;
    if (inherited)
      constructors = this->GetOverloadedInheritedConstructors();

    if (constructors == nullptr)
      return nullptr;

    ZilchForEach(Function* constructor, *constructors)
    {
      if (constructor->FunctionType == type)
        return constructor;
    }

    return nullptr;
  }

  //***************************************************************************
  Attribute* BoundType::HasAttributeInherited(StringParam name)
  {
    BoundType* currentType = this;
    while(currentType)
    {
      if(Attribute* attribute = currentType->HasAttribute(name))
        return attribute;
      currentType = currentType->BaseType;
    }

    return nullptr;
  }
  
  //***************************************************************************
  Function* BoundType::GetDefaultConstructor(const FunctionArray* constructors)
  {
    if (constructors == nullptr)
      return nullptr;

    // Loop through all of our constructors
    for (size_t i = 0; i < constructors->Size(); ++i)
    {
      // Grab the current constructor
      Function* constructor = (*constructors)[i];

      // Make sure none of the constructors have return values
      ErrorIf(constructor->FunctionType->Return != ZilchTypeId(void),
        "A constructor was bound with a non-void return type");

      // As long as we have no parameters... this is the default constructor!
      if (constructor->FunctionType->Parameters.Empty())
        return constructor;
    }

    // If we got here, we failed to find a default constructor
    return nullptr;
  }

  //***************************************************************************
  Function* BoundType::GetCopyConstructor(const FunctionArray* constructors)
  {
    if (constructors == nullptr)
      return nullptr;

    // Loop through all of our constructors
    for (size_t i = 0; i < constructors->Size(); ++i)
    {
      // Grab the current constructor
      Function* constructor = (*constructors)[i];

      // Make sure none of the constructors have return values
      ErrorIf(constructor->FunctionType->Return != ZilchTypeId(void),
        "A constructor was bound with a non-void return type");

      // The copy constructor must have one argument
      ParameterArray& parameters = constructor->FunctionType->Parameters;
      if (parameters.Size() == 1)
      {
        // If the argument is of the same type as our this type, then this is the copy constructor
        // Note that this means all value types are 'ref'
        if (parameters.Front().ParameterType == constructor->This->ResultType)
          return constructor;
      }
    }

    // If we got here, we failed to find a copy constructor
    return nullptr;
  }

  //***************************************************************************
  Function* BoundType::FindFunction(StringParam name, DelegateType* type, FindMemberOptions::Flags options) const
  {
    // The map of functions we'll look in (either static or instance)
    const FunctionMultiMap* functions = nullptr;

    // If we're looking for static functions...
    if (options & FindMemberOptions::Static)
      functions = &this->StaticFunctions;
    else
      functions = &this->InstanceFunctions;

    // Attempt to find the array
    const FunctionArray* foundFunctions = functions->FindPointer(name);

    // Check if we found any functions by that name at this level
    if (foundFunctions != nullptr)
    {
      // Get a range so we can walk through all these functions
      FunctionArray::range foundRange = foundFunctions->All();

      // Loop through all the functions by the same name
      while (foundRange.Empty() == false)
      {
        // Grab the current function and iterate to the next
        Function* function = foundRange.Front();
        foundRange.PopFront();
        
        // If the signature matches...
        if (Type::IsSame(function->FunctionType, type))
        {
          // Return the function we found
          return function;
        }
      }
    }
    // If we found nothing...
    else
    {
      // If we're not searching base classes, just return out
      if (options & FindMemberOptions::DoNotIncludeBaseClasses || this->BaseType == nullptr)
        return nullptr;
      else
        // Walk up the base class looking for that same function
        return this->BaseType->FindFunction(name, type, options);
    }

    return nullptr;
  }
  
  //***************************************************************************
  Property* BoundType::FindProperty(StringParam name, FindMemberOptions::Flags options) const
  {
    // The map of properties and fields we'll look in (either static or instance)
    const GetterSetterMap* properties = nullptr;
    const FieldMap* fields = nullptr;

    // If we're looking for static members...
    if (options & FindMemberOptions::Static)
    {
      properties = &this->StaticGetterSetters;
      fields = &this->StaticFields;
    }
    else
    {
      properties = &this->InstanceGetterSetters;
      fields = &this->InstanceFields;
    }

    // Attempt to find the property or field
    Property* foundProperty = properties->FindValue(name, nullptr);
    if (foundProperty == nullptr)
      foundProperty = fields->FindValue(name, nullptr);

    // Check if we found any properties by that name at this level then immediately return it
    if (foundProperty != nullptr)
    {
      return foundProperty;
    }
    // If we found nothing...
    else
    {
      // If we're not searching base classes, just return out
      if (options & FindMemberOptions::DoNotIncludeBaseClasses || this->BaseType == nullptr)
        return nullptr;
      // Otherwise, walk up the base classes
      else
        return this->BaseType->FindProperty(name, options);
    }
  }

  //***************************************************************************
  Function* BoundType::FindFunction(StringParam name, const Array<Type*>& parameters, Type* returnType, FindMemberOptions::Flags options) const
  {
    // The map of functions we'll look in (either static or instance)
    const FunctionMultiMap* functions = nullptr;

    // If we're looking for static functions...
    if (options & FindMemberOptions::Static)
      functions = &this->StaticFunctions;
    else
      functions = &this->InstanceFunctions;

    // Attempt to find the array
    const FunctionArray* foundFunctions = functions->FindPointer(name);

    // Check if we found any functions by that name at this level
    if (foundFunctions != nullptr)
    {
      // Get a range so we can walk through all these functions
      FunctionArray::range foundRange = foundFunctions->All();

      // Loop through all the functions by the same name
      while (foundRange.Empty() == false)
      {
        // Grab the current function and iterate to the next
        Function* function = foundRange.Front();
        foundRange.PopFront();

        // Skip this function if the return type doesn't match
        if (Type::IsSame(returnType, function->FunctionType->Return) == false)
          continue;

        // Get the type parameters of this function
        ParameterArray& params = function->FunctionType->Parameters;

        // If the function we're looking at has a different number of parameters, skip it
        if (parameters.Size() != params.Size())
          continue;

        // Check to see that all the parameter's types match
        bool sameParameters = true;
        for (size_t i = 0; i < params.Size(); ++i)
        {
          // If any of them don't match, break out early and set that not all were the same
          DelegateParameter& current = params[i];
          if (Type::IsSame(current.ParameterType, parameters[i]) == false)
          {
            sameParameters = false;
            break;
          }
        }

        // If the signature matches...
        if (sameParameters)
        {
          // Return the function we found
          return function;
        }
      }
    }
    // If we found nothing...
    else
    {
      // If we're not searching base classes, just return out
      if (options & FindMemberOptions::DoNotIncludeBaseClasses || this->BaseType == nullptr)
        return nullptr;
      else
        return this->BaseType->FindFunction(name, parameters, returnType, options);
    }

    return nullptr;
  }

  //***************************************************************************
  const FunctionArray* BoundType::GetOverloadedInstanceFunctions(StringParam name) const
  {
    // Attempt to find the array
    const FunctionArray* found = this->InstanceFunctions.FindPointer(name);

    // If we found nothing...
    if (found == nullptr && this->BaseType != nullptr)
    {
      // Search our parent
      return this->BaseType->GetOverloadedInstanceFunctions(name);
    }

    // Return whatever we found
    return found;
  }

  //***************************************************************************
  const FunctionArray* BoundType::GetOverloadedStaticFunctions(StringParam name) const
  {
    // Attempt to find the array
    const FunctionArray* found = this->StaticFunctions.FindPointer(name);

    // If we found nothing...
    if (found == nullptr && this->BaseType != nullptr)
    {
      // Search our parent
      return this->BaseType->GetOverloadedStaticFunctions(name);
    }

    // Return whatever we found
    return found;
  }

  //***************************************************************************
  Field* BoundType::GetInstanceField(StringParam name) const
  {
    // Attempt to find the value
    Field* found = this->InstanceFields.FindValue(name, nullptr);

    // If we found nothing...
    if (found == nullptr && this->BaseType != nullptr)
    {
      // Search our parent
      return this->BaseType->GetInstanceField(name);
    }

    // Return whatever we found
    return found;
  }

  //***************************************************************************
  Field* BoundType::GetStaticField(StringParam name) const
  {
    // Attempt to find the value
    Field* found = this->StaticFields.FindValue(name, nullptr);

    // If we found nothing...
    if (found == nullptr && this->BaseType != nullptr)
    {
      // Search our parent
      return this->BaseType->GetStaticField(name);
    }

    // Return whatever we found
    return found;
  }

  //***************************************************************************
  GetterSetter* BoundType::GetInstanceGetterSetter(StringParam name) const
  {
    // Attempt to find the value
    GetterSetter* found = this->InstanceGetterSetters.FindValue(name, nullptr);

    // If we found nothing...
    if (found == nullptr && this->BaseType != nullptr)
    {
      // Search our parent
      return this->BaseType->GetInstanceGetterSetter(name);
    }

    // Return whatever we found
    return found;
  }

  //***************************************************************************
  GetterSetter* BoundType::GetStaticGetterSetter(StringParam name) const
  {
    // Attempt to find the value
    GetterSetter* found = this->StaticGetterSetters.FindValue(name, nullptr);

    // If we found nothing...
    if (found == nullptr && this->BaseType != nullptr)
    {
      // Search our parent
      return this->BaseType->GetStaticGetterSetter(name);
    }

    // Return whatever we found
    return found;
  }

  //***************************************************************************
  const FunctionArray* BoundType::GetOverloadedInheritedConstructors() const
  {
    // Walk up the base class chain until we find any constructors (we inherit constructors)
    // We start with the current class we're trying to create
    // Note: We can safely look up to our base classes because if we don't have a constructor
    // then we at least have been pre-constructed, and the user opted to not initialize anything with a constructor
    const BoundType* constructedType = this;

    ZilchLoop
    {
      // If we found any constructors, early out
      if (constructedType->Constructors.Empty() == false)
        break;

      // If the type we're constructing is native, then we don't look for inherited constructors
      if (constructedType->Native)
        break;

      // We didn't find anything, so walk up to the base type and see if it has any (constructor inheritance)
      constructedType = constructedType->BaseType;

      // If we had no base, early out
      if (constructedType == nullptr)
        return nullptr;
    }

    // Return the constructors we found, or nothing (or an empty array!)
    return &constructedType->Constructors;
  }

  //***************************************************************************
  // Writes a line out that is tabbed in by two spaces
  // and every newline in the text will also be tabbed in
  void WriteLineTabbed(StringBuilderExtended& builder, StringParam text)
  {
    // Start off with two spaces of tabs
    builder.Write("  ");

    // Loop through every character in the text
    StringIterator end = text.End();
    for (StringIterator it = text.Begin(); it < end; ++it )
    {
      // Get the current character
      Zero::Rune r = text.Front();
      
      // Write the character out
      builder.Write(String(r));

      // If the character was a newline, we need to Insert a tab
      if (r == '\n')
      {
        // Write another two space tab, note that this is AFTER the newline
        builder.Write("  ");
      }
    }

    // Finally, write an ending newline
    builder.WriteLine();
  };

  //***************************************************************************
  AddMemberResult::Enum BoundType::AddRawFunction(Function* function)
  {
    // Assume we properly added the function
    AddMemberResult::Enum result = AddMemberResult::Added;

    // Grab either the static or instance map of functions
    FunctionMultiMap& map = this->GetFunctionMap(function->IsStatic);

    // Grab the array of functions
    FunctionArray& functions = map[function->Name];

    // If we already have an overload of the exact same signature...
    for (size_t i = 0; i < functions.Size(); ++i)
    {
      // If the type of each function exactly matches
      if (Type::IsSame(functions[i]->FunctionType, function->FunctionType))
      {
        result = AddMemberResult::AlreadyExists;
        break;
      }
    }

    // Add the function to the type's list of functions
    functions.PushBack(function);

    // Add the function to the lsit of all functions
    this->AllFunctions.PushBack(function);
    this->AllMembers.PushBack(function);
    return result;
  }

  //***************************************************************************
  AddMemberResult::Enum BoundType::AddRawConstructor(Function* function)
  {
    // Assume we properly added the functino
    AddMemberResult::Enum result = AddMemberResult::Added;

    // Grab the array of functions
    FunctionArray& constructors = this->Constructors;

    // If we already have an overload of the exact same signature...
    for (size_t i = 0; i < constructors.Size(); ++i)
    {
      // If the type of each function exactly matches
      if (Type::IsSame(constructors[i]->FunctionType, function->FunctionType))
      {
        result = AddMemberResult::AlreadyExists;
        break;
      }
    }

    // Add the function to the type's list of functions
    constructors.PushBack(function);
    return result;
  }

  //***************************************************************************
  AddMemberResult::Enum BoundType::AddRawGetterSetter(GetterSetter* property)
  {
    // Grab either the instance or static map of properties
    GetterSetterMap& map = this->GetGetterSetterMap(property->IsStatic);

    // Map the name of the property to the property itself
    bool inserted = map.InsertNoOverwrite(property->Name, property);
    ErrorIf(inserted == false, "Another property with the same name (%s) was added to the BoundType", property->Name.c_str());

    // Add the property to the list of all properties
    this->AllProperties.PushBack(property);
    this->AllMembers.PushBack(property);

    // Return whether we inserted it or not
    if (inserted)
      return AddMemberResult::Added;
    else
      return AddMemberResult::AlreadyExists;
  }

  //***************************************************************************
  AddMemberResult::Enum BoundType::AddRawField(Field* field)
  {
    // Grab either the instance or static map of fields
    FieldMap& map = this->GetFieldMap(field->IsStatic);

    // Map the name of the field to the field itself
    bool inserted = map.InsertNoOverwrite(field->Name, field);
    ErrorIf(inserted == false, "Another member with the same name (%s) was added to the BoundType", field->Name.c_str());

    // Add the member to the list of all properties
    this->AllProperties.PushBack(field);
    this->AllMembers.PushBack(field);

    // Return whether we inserted it or not
    if (inserted)
      return AddMemberResult::Added;
    else
      return AddMemberResult::AlreadyExists;
  }
  
  //***************************************************************************
  FieldMap& BoundType::GetFieldMap(bool isStatic)
  {
    if (isStatic)
      return this->StaticFields;
    else
      return this->InstanceFields;
  }

  //***************************************************************************
  GetterSetterMap& BoundType::GetGetterSetterMap(bool isStatic)
  {
    if (isStatic)
      return this->StaticGetterSetters;
    else
      return this->InstanceGetterSetters;
  }

  //***************************************************************************
  FunctionMultiMap& BoundType::GetFunctionMap(bool isStatic)
  {
    if (isStatic)
      return this->StaticFunctions;
    else
      return this->InstanceFunctions;
  }

  //***************************************************************************
  DelegateParameter::DelegateParameter() :
    ParameterType(nullptr),
    StackOffset(0),
    Index(0)
  {
  }

  //***************************************************************************
  DelegateParameter::DelegateParameter(Type* type) :
    ParameterType(type),
    StackOffset(0),
    Index(0)
  {
  }

  //***************************************************************************
  String DelegateParameter::GetNameOrGenerate()
  {
    if (this->Name.Empty())
      return String::Format("__%d", this->Index);
    return this->Name;
  }

  //***************************************************************************
  DelegateType::DelegateType() :
    Return(nullptr),
    ReturnStackOffset(0),
    ThisHandleStackOffset(0),
    TotalStackSizeExcludingThisHandle(0)
  {
  }

  //***************************************************************************
  GuidType DelegateType::Hash() const
  {
    // Store the hash result
    GuidType result = 0;

    // Loop through all the parameters
    for (size_t i = 0; i < this->Parameters.Size(); ++i)
    {
      // Get a reference to the current parameter
      const DelegateParameter& parameter = this->Parameters[i];
      
      // Add the parameter type to the hash (we can just use the pointer value since it should be unique)
      result ^= parameter.ParameterType->Hash() * 983ULL * (i + 331ULL);

      // We want to use i in some way so that the hash becomes order dependent
      result ^= 12764787846358441471ULL * i + 5463458053ULL + i;
    }

    // Add the return type to the hash (we can just use the pointer value since it should be unique)
    // Note that void is a type, and therefore has its own hash value (no special case)
    result ^= this->Return->Hash();

    // Use a random prime to make returns more unique
    result ^= 2305843009213693951ULL;

    // Return the computed hash
    return result;
  }

  //***************************************************************************
  void DelegateType::BuildParameterString(StringBuilder& builder, bool declaredNames) const
  {
    // Output the opening argument parentheses
    builder.Append("(");

    // Get the last parameter in the list
    size_t lastParameter = this->Parameters.Size() - 1;

    // Loop through all the parameters
    for (size_t i = 0; i < this->Parameters.Size(); ++i)
    {
      // Grab the current parameter
      const DelegateParameter& parameter = this->Parameters[i];

      // If the parameter actually has a name
      if (parameter.Name.Empty() == false && declaredNames)
      {
        // Output the name of the parameter
        builder.Append(parameter.Name.c_str());
        builder.Append(" : ");
      }
      
      // Always output the type of the parameter
      builder.Append(parameter.ParameterType->ToString());

      // If this is not the last parameter...
      if (i < lastParameter)
      {
        // Output an argument separator
        builder.Append(", ");
      }
    }

    // Output the ending argument parentheses
    builder.Append(")");
  }

  //***************************************************************************
  void DelegateType::BuildSignatureString(StringBuilder& builder, bool declaredNames) const
  {
    // Build the parameter portion first
    this->BuildParameterString(builder, declaredNames);

    // Get a reference to the core library
    Core& core = Core::GetInstance();

    // If we have a return type
    if (this->Return != core.VoidType)
    {
      // Output the return type...
      builder.Append(" : ");
      builder.Append(this->Return->ToString());
    }
  }

  //***************************************************************************
  String DelegateType::GetParameterString() const
  {
    StringBuilder builder;
    this->BuildParameterString(builder);
    return builder.ToString();
  }

  //***************************************************************************
  String DelegateType::GetSignatureString() const
  {
    StringBuilder builder;
    this->BuildSignatureString(builder);
    return builder.ToString();
  }

  //***************************************************************************
  String DelegateType::ToString() const
  {
    // Create a string builder for concatenation
    StringBuilder builder;

    // Output the delegate keyword
    builder.Append(Grammar::GetKeywordOrSymbol(Grammar::Delegate));
    builder.Append(" ");
    
    // Build the parameter list string and return
    this->BuildSignatureString(builder);

    // Finally, output the full concatenated type string
    return builder.ToString();
  }
  
  //***************************************************************************
  String DelegateType::GetShortLowerCamelCaseName() const
  {
    return Grammar::GetKeywordOrSymbol(Grammar::Delegate);
  }

  //***************************************************************************
  bool DelegateType::IsCopyComplex() const
  {
    // Delegates always have a complex copy
    return true;
  }

  //***************************************************************************
  size_t DelegateType::GetAllocatedSize() const
  {
    // Functions take up the size of a function index
    return sizeof(Delegate);
  }

  //***************************************************************************
  void DelegateType::GenericDefaultConstruct(byte* toConstruct) const
  {
    // Construct a null / empty delegate
    new (toConstruct) Delegate();
  }

  //***************************************************************************
  void DelegateType::GenericCopyConstruct(byte* to, const byte* from) const
  {
    // Copy construct the delegate to the given location
    new (to) Delegate(*(Delegate*)from);
  }

  //***************************************************************************
  void DelegateType::GenericDestruct(byte* value) const
  {
    // Destroy the delegate type
    ((Delegate*)value)->~Delegate();
  }

  //***************************************************************************
  int DelegateType::GenericHash(const byte* value) const
  {
    return ((Delegate*)value)->Hash();
  }

  //***************************************************************************
  String DelegateType::GenericToString(const byte* value) const
  {
    // Get the delegate value and write it's function out as a string
    Delegate& delegate = *(Delegate*)value;
    if (delegate.BoundFunction == nullptr)
      return BuildString("(null) ", this->ToString());
    else
      return delegate.BoundFunction->ToString();
  }

  //***************************************************************************
  bool DelegateType::GenericEquals(const byte* lhs, const byte* rhs) const
  {
    // Compare the two delegates
    Delegate& lDelegate = *((Delegate*)lhs);
    Delegate& rDelegate = *((Delegate*)rhs);
    return lDelegate == rDelegate;
  }

  //***************************************************************************
  Type* DelegateType::GenericGetVirtualType(const byte* value) const
  {
    Delegate* delegate = (Delegate*)value;
    Function* function = delegate->BoundFunction;
    if (function == nullptr)
      return (Type*)this;

    return function->FunctionType;
  }
  
  //***************************************************************************
  ParameterArray::range DelegateType::GetParameters()
  {
    return this->Parameters.All();
  }
}