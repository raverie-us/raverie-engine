/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_TEMPLATE_BINDING_HPP
#define ZILCH_TEMPALTE_BINDING_HPP

namespace Zilch
{
  namespace PropertyBinding
  {
    enum Enum
    {
      Get,
      Set,
      GetSet
    };
  }

  // All things relevant to binding methods
  class ZeroShared TemplateBinding
  {
  public:

    // Given a comma delimited string of names (eg, "destination, source, size") this will fill in the parameter array with
    // those names. The number of parameters must match the number of parsed names. All names should be lower-camel case
    // For generic use, if the names list is empty, this will immediately return with no errors
    static void ParseParameterArrays(ParameterArray& parameters, StringRange commaDelimitedNames);

    // Validate that a destructor has been bound (asserts within binding constructors)
    // This just returns the same bound type that is used, which allows us to use this as an expression
    static BoundType* ValidateConstructorBinding(BoundType* type);

    // Include all the binding code
    #include "MethodBinding.inl"
    #include "VirtualMethodBinding.inl"
    #include "ConstructorBinding.inl"

    //*** BOUND DESTRUCTOR ***// Wraps a destructor call with the Zilch signature
    template <typename Class>
    static void BoundDestructor(Call& call, ExceptionReport& report)
    {
      //// Get our self object
      //Class* self = (Class*)call.GetHandle(Call::This).Dereference();

      // We need to investigate why the above call doesn't work
      // Unfortunately this comment is written whilst looking back, and I don't understand
      // why I had commented the above out. My guess would be that for struct types (value types)
      // the 'this type' is actually the ref instead of the type itself... something like that
      // Most likely the handle is storing the BoundType*, eg Quaternion, instead of the IndirectionType*
      // Why doesn't this appear elswhere, say the field get functions?
      Handle& selfHandle = call.GetHandle(Call::This);
      Class* self = (Class*)selfHandle.Dereference();

      // Explicitly call the destructor of the class
      self->~Class();
    }
    
    //*** BUILDER DESTRUCTOR ***// Generates a Zilch function to call a class destructor
    template <typename Class>
    static Function* FromDestructor(LibraryBuilder& builder, BoundType* classBoundType)
    {
      return builder.AddBoundDestructor(classBoundType, BoundDestructor<Class>);
    }
    
    //*** BOUND INSTANCE FIELD GET ***//
    template <typename FieldType, typename Class, FieldType Class::* field>
    static void BoundInstanceGet(Call& call, ExceptionReport& report)
    {
      // Get our self object
      Class* self = (Class*)call.GetHandle(Call::This).Dereference();

      // Get the value of the member
      FieldType& value = self->*field;

      // Get the member's value by returning it
      call.Set(Call::Return, value);
    }
    
    //*** BOUND INSTANCE FIELD SET ***//
    template <typename FieldType, typename Class, FieldType Class::* field>
    static void BoundInstanceSet(Call& call, ExceptionReport& report)
    {
      // Get our self object
      Class* self = (Class*)call.GetHandle(Call::This).Dereference();

      // Read in the value that we're trying to set
      byte* stackPointer = call.GetArgumentPointer<ZilchBindingType(FieldType)>(0);

      // If read is invalid, throw a more specific exception
      if (report.HasThrownExceptions())
        return ExecutableState::GetCallingState()->ThrowException("Error: Cannot assign null.");

      ZilchBindingType(FieldType) value = call.CastArgumentPointer<ZilchBindingType(FieldType)>(stackPointer);

      // Set the value of the member
      self->*field = value;
    }

    //*** BUILDER INSTANCE CONST FIELD ***//
    template <typename FieldPointer, FieldPointer field, typename Class, typename FieldType>
    static Property* FromField(LibraryBuilder& builder, BoundType* owner, StringParam name, const FieldType Class::* dummy, PropertyBinding::Enum mode)
    {
      ErrorIf(dummy != field, "The dummy should always match our template member");
      BoundFn get = BoundInstanceGet<const FieldType, Class, field>;
      ErrorIf(mode != PropertyBinding::Get, "The field is const and therefore a setter cannot be generated (use PropertyBinding::Get)");
      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(FieldType), nullptr, get, MemberOptions::None);
    }

    //*** BUILDER INSTANCE FIELD ***//
    // Generates a Zilch property by creating get/set functions to wrap the member variable and binding them
    template <typename FieldPointer, FieldPointer field, typename Class, typename FieldType>
    static Property* FromField(LibraryBuilder& builder, BoundType* owner, StringParam name, FieldType Class::* dummy, PropertyBinding::Enum mode)
    {
      ErrorIf(dummy != field, "The dummy should always match our template member");
      BoundFn set = BoundInstanceSet<FieldType, Class, field>;
      BoundFn get = BoundInstanceGet<FieldType, Class, field>;
      
      if (mode == PropertyBinding::Get)
      {
        set = nullptr;
      }
      if (mode == PropertyBinding::Set)
      {
        get = nullptr;
      }

      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(FieldType), set, get, MemberOptions::None);
    }

    //*** BOUND STATIC FIELD GET ***//
    template <typename FieldType, FieldType* field>
    static void BoundStaticGet(Call& call, ExceptionReport& report)
    {
      // Get the value of the member
      FieldType& value = *field;

      // Get the member's value by returning it
      call.Set(Call::Return, value);
    }
    
    //*** BOUND STATIC FIELD SET ***//
    template <typename FieldType, FieldType* field>
    static void BoundStaticSet(Call& call, ExceptionReport& report)
    {
      // Read in the value that we're trying to set
      byte* stackPointer = call.GetArgumentPointer<ZilchBindingType(FieldType)>(0);

      // If read is invalid, throw a more specific exception
      if (report.HasThrownExceptions())
        return ExecutableState::GetCallingState()->ThrowException("Error: Cannot assign null.");

      ZilchBindingType(FieldType) value = call.CastArgumentPointer<ZilchBindingType(FieldType)>(stackPointer);
      
      // Set the value of the member
      *field = value;
    }

    //*** BUILDER STATIC CONST FIELD ***//
    template <typename FieldPointer, FieldPointer field, typename FieldType>
    static Property* FromField(LibraryBuilder& builder, BoundType* owner, StringParam name, const FieldType* dummy, PropertyBinding::Enum mode)
    {
      ErrorIf(dummy != field, "The dummy should always match our template member");
      BoundFn get = BoundStaticGet<const FieldType, field>;
      ErrorIf(mode != PropertyBinding::Get, "The field is const and therefore a setter cannot be generated (use PropertyBinding::Get)");
      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(FieldType), nullptr, get, MemberOptions::Static);
    }

    //*** BUILDER STATIC FIELD ***//
    // Generates a Zilch property by creating get/set functions to wrap the global variable and binding them
    template <typename FieldPointer, FieldPointer field, typename FieldType>
    static Property* FromField(LibraryBuilder& builder, BoundType* owner, StringParam name, FieldType* dummy, PropertyBinding::Enum mode)
    {
      ErrorIf(dummy != field, "The dummy should always match our template member");
      BoundFn set = BoundStaticSet<FieldType, field>;
      BoundFn get = BoundStaticGet<FieldType, field>;
      
      if (mode == PropertyBinding::Get)
        set = nullptr;
      if (mode == PropertyBinding::Set)
        get = nullptr;

      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(FieldType), set, get, MemberOptions::Static);
    }

    //*** BUILDER INSTANCE PROPERTY GET/SET ***//
    template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename Class, typename GetType, typename SetType>
    static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (Class::*dummyGetter)(), void (Class::*dummySetter)(SetType))
    {
      ReturnIf
      (
        ZilchTypeId(GetType) != ZilchTypeId(SetType),
        nullptr,
        "Cannot bind a Get/Set property type that has a different fundamental type for the getter's return value and setters input value"
      );
  
      ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
      ErrorIf(dummySetter != setter, "The dummy getter should always match our template member");
  
      BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
      BoundFn boundSet = BoundInstance<SetterType, setter, Class, SetType>;
  
      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(GetType), boundSet, boundGet, MemberOptions::None);
    }

    //*** BUILDER INSTANCE PROPERTY CONST GET/SET ***//
    template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename Class, typename GetType, typename SetType>
    static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (Class::*dummyGetter)() const, void (Class::*dummySetter)(SetType))
    {
      ReturnIf
      (
        ZilchTypeId(GetType) != ZilchTypeId(SetType),
        nullptr,
        "Cannot bind a Get/Set property type that has a different fundamental type for the getter's return value and setters input value"
      );
  
      ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
      ErrorIf(dummySetter != setter, "The dummy getter should always match our template member");
  
      BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
      BoundFn boundSet = BoundInstance<SetterType, setter, Class, SetType>;
  
      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(GetType), boundSet, boundGet, MemberOptions::None);
    }

    //*** BUILDER INSTANCE PROPERTY GET ***//
    template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename Class, typename GetType>
    static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (Class::*dummyGetter)(), NullPointerType)
    {
      ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
      BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(GetType), nullptr, boundGet, MemberOptions::None);
    }

    //*** BUILDER INSTANCE PROPERTY CONST GET ***//
    template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename Class, typename GetType>
    static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (Class::*dummyGetter)() const, NullPointerType)
    {
      ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
      BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(GetType), nullptr, boundGet, MemberOptions::None);
    }

    //*** BUILDER INSTANCE PROPERTY SET ***//
    template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename Class, typename SetType>
    static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, NullPointerType, void (Class::*dummySetter)(SetType))
    {
      ErrorIf(dummySetter != setter, "The dummy setter should always match our template member");
      BoundFn boundSet = BoundInstance<SetterType, setter, Class, SetType>;
      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(SetType), boundSet, nullptr, MemberOptions::None);
    }

    //*** BUILDER STATIC PROPERTY GET/SET ***//
    template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename GetType, typename SetType>
    static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (*dummyGetter)(), void (*dummySetter)(SetType))
    {
      ReturnIf
      (
        ZilchTypeId(GetType) != ZilchTypeId(SetType),
        nullptr,
        "Cannot bind a Get/Set property type that has a different fundamental type for the getter's return value and setters input value"
      );
  
      ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
      ErrorIf(dummySetter != setter, "The dummy getter should always match our template member");
  
      BoundFn boundGet = BoundStaticReturn<GetterType, getter, GetType>;
      BoundFn boundSet = BoundStatic<SetterType, setter, SetType>;
  
      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(GetType), boundSet, boundGet, MemberOptions::Static);
    }

    //*** BUILDER STATIC PROPERTY GET ***//
    template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename GetType>
    static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (*dummyGetter)(), NullPointerType)
    {
      ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
      BoundFn boundGet = BoundStaticReturn<GetterType, getter, GetType>;
      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(GetType), nullptr, boundGet, MemberOptions::Static);
    }

    //*** BUILDER STATIC PROPERTY SET ***//
    template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename SetType>
    static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, NullPointerType, void (*dummySetter)(SetType))
    {
      ErrorIf(dummySetter != setter, "The dummy setter should always match our template member");
      BoundFn boundSet = BoundStatic<SetterType, setter, SetType>;
      return builder.AddBoundGetterSetter(owner, name, ZilchTypeId(SetType), boundSet, nullptr, MemberOptions::Static);
    }
  };

  // If we want more readable code when not specifying a getter or setter in ZilchFullBindGetterSetter
  #define ZilchNoSetter nullptr
  #define ZilchNoGetter nullptr
  
  // When we want to specify that a method binding has no parameter names or documentation, we use this macro (more readable and clear)
  #define ZilchNoNames nullptr
  #define ZilchNoDocumentation nullptr

  // When using the ZilchFullBindMethod macro if we're binding a method that has no overloads then we use this constant for the parameter 'OverloadResolution'
  #define ZilchNoOverload

  // Workhorse macro for binding methods
  #define ZilchFullBindMethod(ZilchBuilder, ZilchType, MethodPointer, OverloadResolution, Name, SpaceDelimitedParameterNames) \
    ZZ::TemplateBinding::FromMethod<ZilchTypeOf(OverloadResolution MethodPointer), MethodPointer>(ZilchBuilder, ZilchType, Name, SpaceDelimitedParameterNames, OverloadResolution(MethodPointer))

  // Workhorse macro for binding virtual methods
  #define ZilchFullBindVirtualMethod(ZilchBuilder, ZilchType, MethodPointer, NameOrNull) \
    ZZ::TemplateBinding::FromVirtual<ZilchTypeOf(MethodPointer), MethodPointer>(ZilchBuilder, ZilchType, Name, SpaceDelimitedParameterNames, (MethodPointer))

  // Bind a constructor that takes any number of arguments
  // Due to the inability to get a 'member function pointer' to a constructor, the arguments must always be specified
  #define ZilchFullBindConstructor(ZilchBuilder, ZilchType, Class, SpaceDelimitedParameterNames, ...) \
    ZZ::TemplateBinding::FromConstructor<Class, ##__VA_ARGS__>(ZilchBuilder, ZilchType, SpaceDelimitedParameterNames)
  
  // Bind a constructor that takes any number of arguments (this binds a special constructor that lets Zilch know about the type's v-table)
  // Due to the inability to get a 'member function pointer' to a constructor, the arguments must always be specified
  #define ZilchFullBindConstructorVirtual(ZilchBuilder, ZilchType, Class, SpaceDelimitedParameterNames, ...) \
    ZZ::TemplateBinding::FromConstructorVirtual<Class, ##__VA_ARGS__>(ZilchBuilder, ZilchType, SpaceDelimitedParameterNames)

  // Bind the destructor of a class
  // The destructor should ALWAYS be bound if the constructor is bound
  #define ZilchFullBindDestructor(ZilchBuilder, ZilchType, Class) \
    ZZ::TemplateBinding::FromDestructor<Class >(ZilchBuilder, ZilchType)

  // Bind data members as properties
  #define ZilchFullBindField(ZilchBuilder, ZilchType, FieldPointer, Name, PropertyBinding) \
    ZZ::TemplateBinding::FromField<ZilchTypeOf(FieldPointer), FieldPointer>(ZilchBuilder, ZilchType, Name, FieldPointer, PropertyBinding)

  // Bind data members with an offset
  #define ZilchFullBindMember(ZilchBuilder, ZilchType, MemberName, Name, PropertyBinding) \
    ZilchBuilder.AddBoundField(ZilchType, Name, ZilchTypeId(ZilchTypeOf(ZilchSelf::MemberName)), offsetof(ZilchSelf, MemberName), PropertyBinding)
  
  // Bind a property (getter and setter in C++) to Zilch
  // A property will appear like a member, but it will invoke the getter when being read, and the setter when being written to
  #define ZilchFullBindGetterSetter(ZilchBuilder, ZilchType, GetterMethodPointer, GetterOverload, SetterMethodPointer, SetterOverload, Name) \
    ZZ::TemplateBinding::FromGetterSetter<ZilchTypeOf(GetterOverload GetterMethodPointer), GetterMethodPointer, ZilchTypeOf(SetterOverload SetterMethodPointer), SetterMethodPointer>(ZilchBuilder, ZilchType, Name, GetterMethodPointer, SetterMethodPointer)
  
  // Bind a type as being an enum (verifies that the size matches)
  #define ZilchFullBindEnum(ZilchBuilder, ZilchType, SpecialTypeEnum)                                                         \
    do                                                                                                                        \
    {                                                                                                                         \
      (ZilchType)->SpecialType = SpecialTypeEnum;                                                                             \
      if ((SpecialTypeEnum) == ZZ::SpecialType::Enumeration)                                                                  \
        (ZilchType)->ToStringFunction = ZZ::VirtualMachine::EnumerationToString;                                              \
      else                                                                                                                    \
        (ZilchType)->ToStringFunction = ZZ::VirtualMachine::FlagsToString;                                                    \
      ErrorIf((ZilchType)->Size != sizeof(ZZ::Integer), "The sizeof(Enum) bound to Zilch must match the sizeof(Integer)");    \
      (ZilchType)->BaseType = ZilchTypeId(ZZ::Enum);                                                                          \
    }                                                                                                                         \
    while(false)

  // Bind a single value of 
  #define ZilchFullBindEnumValue(ZilchBuilder, ZilchType, EnumValue, Name) \
    (ZilchBuilder).AddEnumValue((ZilchType), (Name), (EnumValue));

  // Extra convenience macros that just wrap the above macros (simpler, intended for binding values only to our own type)
  // Note that the Property versions only add the attribute "Property" and can be used for displaying within a property grid
  #define ZilchBindConstructor(...)                         ZilchFullBindConstructor(builder, type, ZilchSelf, ZilchNoNames, ##__VA_ARGS__)
  #define ZilchBindDefaultConstructor()                     ZilchBindConstructor()
  #define ZilchBindCopyConstructor()                        ZilchBindConstructor(const ZilchSelf&)
  #define ZilchBindDestructor()                             ZilchFullBindDestructor(builder, type, ZilchSelf)
  #define ZilchBindDefaultDestructor()                      ZilchBindDefaultConstructor(); ZilchBindDestructor();
  #define ZilchBindDefaultCopyDestructor()                  ZilchBindDefaultConstructor(); ZilchBindCopyConstructor(); ZilchBindDestructor();

  // These versions allow you to rename anything bound
  // Note that 'Custom' means we don't apply the Get or Set to the beginning of the name
  #define ZilchBindOverloadedMethodAs(MethodName, OverloadResolution, Name)         ZilchFullBindMethod(builder, type, &ZilchSelf::MethodName, OverloadResolution, Name, ZilchNoNames)
  #define ZilchBindMethodAs(MethodName, Name)                                       ZilchBindOverloadedMethodAs(MethodName, ZilchNoOverload, Name)
  #define ZilchBindOverloadedMethodPropertyAs(MethodName, OverloadResolution, Name) ZilchFullBindMethod(builder, type, &ZilchSelf::MethodName, OverloadResolution, Name, ZilchNoNames)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindMethodPropertyAs(MethodName, Name)                               ZilchBindOverloadedMethodAs(MethodName, ZilchNoOverload, Name)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindMemberAs(MemberName, Name)                                       ZilchFullBindMember(builder, type, MemberName, Name, Zilch::MemberOptions::None)
  #define ZilchBindMemberPropertyAs(MemberName, Name)                               ZilchBindMemberAs(MemberName, Name)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindFieldAs(FieldName, Name)                                         ZilchFullBindField(builder, type, &ZilchSelf::FieldName, Name, Zilch::PropertyBinding::GetSet)
  #define ZilchBindFieldGetterAs(FieldName, Name)                                   ZilchFullBindField(builder, type, &ZilchSelf::FieldName, Name, Zilch::PropertyBinding::Get)
  #define ZilchBindFieldSetterAs(FieldName, Name)                                   ZilchFullBindField(builder, type, &ZilchSelf::FieldName, Name, Zilch::PropertyBinding::Set)
  #define ZilchBindFieldPropertyAs(FieldName, Name)                                 ZilchBindFieldAs(FieldName, Name)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindFieldGetterPropertyAs(FieldName, Name)                           ZilchBindFieldGetterAs(FieldName, Name)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindFieldSetterPropertyAs(FieldName, Name)                           ZilchBindFieldSetterAs(FieldName, Name)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindGetterAs(PropertyName, Name)                                     ZilchFullBindGetterSetter(builder, type, &ZilchSelf::Get##PropertyName, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, Name)
  #define ZilchBindSetterAs(PropertyName, Name)                                     ZilchFullBindGetterSetter(builder, type, ZilchNoGetter, ZilchNoOverload, &ZilchSelf::Set##PropertyName, ZilchNoOverload, Name)
  #define ZilchBindGetterSetterAs(PropertyName, Name)                               ZilchFullBindGetterSetter(builder, type, &ZilchSelf::Get##PropertyName, ZilchNoOverload, &ZilchSelf::Set##PropertyName, ZilchNoOverload, Name)
  #define ZilchBindGetterPropertyAs(PropertyName, Name)                             ZilchBindGetterAs(PropertyName, Name)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindSetterPropertyAs(PropertyName, Name)                             ZilchBindSetterAs(PropertyName, Name)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindGetterSetterPropertyAs(PropertyName, Name)                       ZilchBindGetterSetterAs(PropertyName, Name)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindCustomGetterAs(PropertyName, Name)                               ZilchFullBindGetterSetter(builder, type, &ZilchSelf::PropertyName, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, Name)
  #define ZilchBindCustomSetterAs(PropertyName, Name)                               ZilchFullBindGetterSetter(builder, type, ZilchNoGetter, ZilchNoOverload, &ZilchSelf::PropertyName, ZilchNoOverload, Name)
  #define ZilchBindCustomGetterSetterAs(PropertyName, Name)                         ZilchFullBindGetterSetter(builder, type, &ZilchSelf::PropertyName, ZilchNoOverload, &ZilchSelf::PropertyName, ZilchNoOverload, Name)
  #define ZilchBindCustomGetterPropertyAs(PropertyName, Name)                       ZilchBindCustomGetterAs(PropertyName, Name)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindCustomSetterPropertyAs(PropertyName, Name)                       ZilchBindCustomSetterAs(PropertyName, Name)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindCustomGetterSetterPropertyAs(PropertyName, Name)                 ZilchBindCustomGetterSetterAs(PropertyName, Name)->AddAttributeChainable(Zilch::PropertyAttribute)
  #define ZilchBindEnumValueAs(EnumValue, Name)                                     ZilchFullBindEnumValue(builder, type, EnumValue, Name)

  // All these versions assume the name is the same as the property/field/method identifier
  #define ZilchBindOverloadedMethod(MethodName, OverloadResolution)                 ZilchBindOverloadedMethodAs(MethodName, OverloadResolution, #MethodName)
  #define ZilchBindMethod(MethodName)                                               ZilchBindMethodAs(MethodName, #MethodName)
  #define ZilchBindOverloadedMethodProperty(MethodName, OverloadResolution)         ZilchBindOverloadedPropertyMethodAs(MethodName, OverloadResolution, #MethodName)
  #define ZilchBindMethodProperty(MethodName)                                       ZilchBindMethodPropertyAs(MethodName, #MethodName)
  #define ZilchBindMember(MemberName)                                               ZilchBindMemberAs(MemberName, #MemberName)
  #define ZilchBindMemberProperty(MemberName)                                       ZilchBindMemberPropertyAs(MemberName, #MemberName)
  #define ZilchBindField(FieldName)                                                 ZilchBindFieldAs(FieldName, #FieldName)
  #define ZilchBindFieldGetter(FieldName)                                           ZilchBindFieldGetterAs(FieldName, #FieldName)
  #define ZilchBindFieldSetter(FieldName)                                           ZilchBindFieldSetterAs(FieldName, #FieldName)
  #define ZilchBindFieldProperty(FieldName)                                         ZilchBindFieldPropertyAs(FieldName, #FieldName)
  #define ZilchBindFieldGetterProperty(FieldName)                                   ZilchBindFieldGetterPropertyAs(FieldName, #FieldName)
  #define ZilchBindFieldSetterProperty(FieldName)                                   ZilchBindFieldSetterPropertyAs(FieldName, #FieldName)
  #define ZilchBindGetter(PropertyName)                                             ZilchBindGetterAs(PropertyName, #PropertyName)
  #define ZilchBindSetter(PropertyName)                                             ZilchBindSetterAs(PropertyName, #PropertyName)
  #define ZilchBindGetterSetter(PropertyName)                                       ZilchBindGetterSetterAs(PropertyName, #PropertyName)
  #define ZilchBindGetterProperty(PropertyName)                                     ZilchBindGetterPropertyAs(PropertyName, #PropertyName)
  #define ZilchBindSetterProperty(PropertyName)                                     ZilchBindSetterPropertyAs(PropertyName, #PropertyName)
  #define ZilchBindGetterSetterProperty(PropertyName)                               ZilchBindGetterSetterPropertyAs(PropertyName, #PropertyName)
  #define ZilchBindCustomGetter(PropertyName)                                       ZilchBindCustomGetterAs(PropertyName, #PropertyName)
  #define ZilchBindCustomSetter(PropertyName)                                       ZilchBindCustomSetterAs(PropertyName, #PropertyName)
  #define ZilchBindCustomGetterSetter(PropertyName)                                 ZilchBindCustomGetterSetterAs(PropertyName, #PropertyName)
  #define ZilchBindCustomGetterProperty(PropertyName)                               ZilchBindCustomGetterPropertyAs(PropertyName, #PropertyName)
  #define ZilchBindCustomSetterProperty(PropertyName)                               ZilchBindCustomSetterPropertyAs(PropertyName, #PropertyName)
  #define ZilchBindCustomGetterSetterProperty(PropertyName)                         ZilchBindCustomGetterSetterPropertyAs(PropertyName, #PropertyName)
  #define ZilchBindEnumValue(EnumValue)                                             ZilchBindEnumValueAs(EnumValue, #EnumValue)

  // Overload resolution helper macros
  #define ZilchStaticOverload(ReturnType, ...)                                      (ReturnType (*)(__VA_ARGS__))
  #define ZilchInstanceOverload(ReturnType, ...)                                    (ReturnType (ZilchSelf::*)(__VA_ARGS__))
  #define ZilchConstInstanceOverload(ReturnType, ...)                               (ReturnType (ZilchSelf::*)(__VA_ARGS__) const)

  // This macro sets up all the special attributes to make a C++ value into a Zilch enumeration
  #define ZilchBindEnum(SpecialTypeEnum)                                            ZilchFullBindEnum(builder, type, SpecialTypeEnum)
}
#endif
