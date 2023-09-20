// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
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
class TemplateBinding
{
public:
  // Given a comma delimited string of names (eg, "destination, source, size")
  // this will fill in the parameter array with those names. The number of
  // parameters must match the number of parsed names. All names should be
  // lower-camel case For generic use, if the names list is empty, this will
  // immediately return with no errors
  static void ParseParameterArrays(ParameterArray& parameters, StringRange commaDelimitedNames);

  // Validate that a destructor has been bound (asserts within binding
  // constructors) This just returns the same bound type that is used, which
  // allows us to use this as an expression
  static BoundType* ValidateConstructorBinding(BoundType* type);

// Include all the binding code
#include "MethodBinding.inl"
#include "VirtualMethodBinding.inl"
#include "ConstructorBinding.inl"

  //*** BOUND DESTRUCTOR ***// Wraps a destructor call with the Raverie signature
  template <typename Class>
  static void BoundDestructor(Call& call, ExceptionReport& report)
  {
    //// Get our self object
    // Class* self = (Class*)call.GetHandle(Call::This).Dereference();

    // We need to investigate why the above call doesn't work
    // Unfortunately this comment is written whilst looking back, and I don't
    // understand why I had commented the above out. My guess would be that for
    // struct types (value types) the 'this type' is actually the ref instead of
    // the type itself... something like that Most likely the handle is storing
    // the BoundType*, eg Quaternion, instead of the IndirectionType* Why
    // doesn't this appear elswhere, say the field get functions?
    Handle& selfHandle = call.GetHandle(Call::This);
    Class* self = (Class*)selfHandle.Dereference();

    // Explicitly call the destructor of the class
    // If this is being destructed in an exception scenario the handle could be
    // null
    if (self)
      self->~Class();
  }

  //*** BUILDER DESTRUCTOR ***// Generates a Raverie function to call a class
  // destructor
  template <typename Class>
  static Function* FromDestructor(LibraryBuilder& builder, BoundType* classBoundType)
  {
    return builder.AddBoundDestructor(classBoundType, BoundDestructor<Class>);
  }

  //*** BOUND INSTANCE FIELD GET ***//
  template <typename FieldType, typename Class, FieldType Class::*field>
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
  template <typename FieldType, typename Class, FieldType Class::*field>
  static void BoundInstanceSet(Call& call, ExceptionReport& report)
  {
    // Get our self object
    Class* self = (Class*)call.GetHandle(Call::This).Dereference();

    // Read in the value that we're trying to set
    byte* stackPointer = call.GetArgumentPointer<RaverieBindingType(FieldType)>(0);

    // If read is invalid, throw a more specific exception
    if (report.HasThrownExceptions())
      return ExecutableState::GetCallingState()->ThrowException("Error: Cannot assign null.");

    RaverieBindingType(FieldType) value = call.CastArgumentPointer<RaverieBindingType(FieldType)>(stackPointer);

    // Set the value of the member
    self->*field = value;
  }

  //*** BUILDER INSTANCE CONST FIELD ***//
  template <typename FieldPointer, FieldPointer field, typename Class, typename FieldType>
  static Property* FromField(LibraryBuilder& builder, BoundType* owner, StringParam name, const FieldType Class::*dummy, PropertyBinding::Enum mode)
  {
    ErrorIf(dummy != field, "The dummy should always match our template member");
    BoundFn get = BoundInstanceGet<const FieldType, Class, field>;
    ErrorIf(mode != PropertyBinding::Get,
            "The field is const and therefore a setter cannot be generated "
            "(use PropertyBinding::Get)");
    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(FieldType), nullptr, get, MemberOptions::None);
  }

  //*** BUILDER INSTANCE FIELD ***//
  // Generates a Raverie property by creating get/set functions to wrap the member
  // variable and binding them
  template <typename FieldPointer, FieldPointer field, typename Class, typename FieldType>
  static Property* FromField(LibraryBuilder& builder, BoundType* owner, StringParam name, FieldType Class::*dummy, PropertyBinding::Enum mode)
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

    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(FieldType), set, get, MemberOptions::None);
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
    byte* stackPointer = call.GetArgumentPointer<RaverieBindingType(FieldType)>(0);

    // If read is invalid, throw a more specific exception
    if (report.HasThrownExceptions())
      return ExecutableState::GetCallingState()->ThrowException("Error: Cannot assign null.");

    RaverieBindingType(FieldType) value = call.CastArgumentPointer<RaverieBindingType(FieldType)>(stackPointer);

    // Set the value of the member
    *field = value;
  }

  //*** BUILDER STATIC CONST FIELD ***//
  template <typename FieldPointer, FieldPointer field, typename FieldType>
  static Property* FromField(LibraryBuilder& builder, BoundType* owner, StringParam name, const FieldType* dummy, PropertyBinding::Enum mode)
  {
    ErrorIf(dummy != field, "The dummy should always match our template member");
    BoundFn get = BoundStaticGet<const FieldType, field>;
    ErrorIf(mode != PropertyBinding::Get,
            "The field is const and therefore a setter cannot be generated "
            "(use PropertyBinding::Get)");
    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(FieldType), nullptr, get, MemberOptions::Static);
  }

  //*** BUILDER STATIC FIELD ***//
  // Generates a Raverie property by creating get/set functions to wrap the global
  // variable and binding them
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

    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(FieldType), set, get, MemberOptions::Static);
  }

  //*** BUILDER INSTANCE PROPERTY GET/SET ***//
  template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename Class, typename GetType, typename SetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (Class::*dummyGetter)(), void (Class::*dummySetter)(SetType))
  {
    ReturnIf(RaverieTypeId(GetType) != RaverieTypeId(SetType),
             nullptr,
             "Cannot bind a Get/Set property type that has a different fundamental "
             "type for the getter's return value and setters input value");

    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    ErrorIf(dummySetter != setter, "The dummy getter should always match our template member");

    BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
    BoundFn boundSet = BoundInstance<SetterType, setter, Class, SetType>;

    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(GetType), boundSet, boundGet, MemberOptions::None);
  }

  //*** BUILDER INSTANCE PROPERTY CONST GET/SET ***//
  template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename Class, typename GetType, typename SetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (Class::*dummyGetter)() const, void (Class::*dummySetter)(SetType))
  {
    ReturnIf(RaverieTypeId(GetType) != RaverieTypeId(SetType),
             nullptr,
             "Cannot bind a Get/Set property type that has a different fundamental "
             "type for the getter's return value and setters input value");

    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    ErrorIf(dummySetter != setter, "The dummy getter should always match our template member");

    BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
    BoundFn boundSet = BoundInstance<SetterType, setter, Class, SetType>;

    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(GetType), boundSet, boundGet, MemberOptions::None);
  }

  //*** BUILDER INSTANCE PROPERTY GET ***//
  template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename Class, typename GetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (Class::*dummyGetter)(), NullPointerType)
  {
    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(GetType), nullptr, boundGet, MemberOptions::None);
  }

  //*** BUILDER INSTANCE PROPERTY CONST GET ***//
  template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename Class, typename GetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (Class::*dummyGetter)() const, NullPointerType)
  {
    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(GetType), nullptr, boundGet, MemberOptions::None);
  }

  //*** BUILDER INSTANCE PROPERTY SET ***//
  template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename Class, typename SetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, NullPointerType, void (Class::*dummySetter)(SetType))
  {
    ErrorIf(dummySetter != setter, "The dummy setter should always match our template member");
    BoundFn boundSet = BoundInstance<SetterType, setter, Class, SetType>;
    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(SetType), boundSet, nullptr, MemberOptions::None);
  }

  //*** BUILDER STATIC PROPERTY GET/SET ***//
  template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename GetType, typename SetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (*dummyGetter)(), void (*dummySetter)(SetType))
  {
    ReturnIf(RaverieTypeId(GetType) != RaverieTypeId(SetType),
             nullptr,
             "Cannot bind a Get/Set property type that has a different fundamental "
             "type for the getter's return value and setters input value");

    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    ErrorIf(dummySetter != setter, "The dummy getter should always match our template member");

    BoundFn boundGet = BoundStaticReturn<GetterType, getter, GetType>;
    BoundFn boundSet = BoundStatic<SetterType, setter, SetType>;

    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(GetType), boundSet, boundGet, MemberOptions::Static);
  }

  //*** BUILDER STATIC PROPERTY GET ***//
  template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename GetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (*dummyGetter)(), NullPointerType)
  {
    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    BoundFn boundGet = BoundStaticReturn<GetterType, getter, GetType>;
    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(GetType), nullptr, boundGet, MemberOptions::Static);
  }

  //*** BUILDER STATIC PROPERTY SET ***//
  template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename SetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder, BoundType* owner, StringRange name, NullPointerType, void (*dummySetter)(SetType))
  {
    ErrorIf(dummySetter != setter, "The dummy setter should always match our template member");
    BoundFn boundSet = BoundStatic<SetterType, setter, SetType>;
    return builder.AddBoundGetterSetter(owner, name, RaverieTypeId(SetType), boundSet, nullptr, MemberOptions::Static);
  }
};

// If we want more readable code when not specifying a getter or setter in
// RaverieFullBindGetterSetter
#define RaverieNoSetter nullptr
#define RaverieNoGetter nullptr

// When we want to specify that a method binding has no parameter names or
// documentation, we use this macro (more readable and clear)
#define RaverieNoNames nullptr
#define RaverieNoDocumentation nullptr

// When using the RaverieFullBindMethod macro if we're binding a method that has
// no overloads then we use this constant for the parameter 'OverloadResolution'
#define RaverieNoOverload

// Workhorse macro for binding methods
#define RaverieFullBindMethod(RaverieBuilder, RaverieType, MethodPointer, OverloadResolution, Name, SpaceDelimitedParameterNames)                                                                      \
  ::Raverie::TemplateBinding::FromMethod<decltype(OverloadResolution MethodPointer), MethodPointer>(RaverieBuilder, RaverieType, Name, SpaceDelimitedParameterNames, OverloadResolution(MethodPointer))

// Workhorse macro for binding virtual methods
#define RaverieFullBindVirtualMethod(RaverieBuilder, RaverieType, MethodPointer, NameOrNull)                                                                                                           \
  ::Raverie::TemplateBinding::FromVirtual<decltype(MethodPointer), MethodPointer>(RaverieBuilder, RaverieType, Name, SpaceDelimitedParameterNames, (MethodPointer))

// Bind a constructor that takes any number of arguments
// Due to the inability to get a 'member function pointer' to a constructor, the
// arguments must always be specified
#define RaverieFullBindConstructor(RaverieBuilder, RaverieType, Class, SpaceDelimitedParameterNames, ...)                                                                                              \
  ::Raverie::TemplateBinding::FromConstructor<Class, ##__VA_ARGS__>(RaverieBuilder, RaverieType, SpaceDelimitedParameterNames)

// Bind a constructor that takes any number of arguments (this binds a special
// constructor that lets Raverie know about the type's v-table) Due to the
// inability to get a 'member function pointer' to a constructor, the arguments
// must always be specified
#define RaverieFullBindConstructorVirtual(RaverieBuilder, RaverieType, Class, SpaceDelimitedParameterNames, ...)                                                                                       \
  ::Raverie::TemplateBinding::FromConstructorVirtual<Class, ##__VA_ARGS__>(RaverieBuilder, RaverieType, SpaceDelimitedParameterNames)

// Bind the destructor of a class
// The destructor should ALWAYS be bound if the constructor is bound
#define RaverieFullBindDestructor(RaverieBuilder, RaverieType, Class) ::Raverie::TemplateBinding::FromDestructor<Class>(RaverieBuilder, RaverieType)

// Bind data members as properties
#define RaverieFullBindField(RaverieBuilder, RaverieType, FieldPointer, Name, PropertyBinding)                                                                                                         \
  ::Raverie::TemplateBinding::FromField<decltype(FieldPointer), FieldPointer>(RaverieBuilder, RaverieType, Name, FieldPointer, PropertyBinding)

// Bind data members with an offset
#define RaverieFullBindMember(RaverieBuilder, RaverieType, MemberName, Name, PropertyBinding)                                                                                                          \
  [&]()                                                                                                                                                                                                \
  {                                                                                                                                                                                                    \
    ErrorIf(RaverieTypeId(decltype(RaverieSelf::MemberName))->GetCopyableSize() != sizeof(RaverieSelf::MemberName),                                                                                    \
            "When binding a member the type must match the exact size it is "                                                                                                                          \
            "expected to be in Raverie "                                                                                                                                                               \
            "(e.g. all reference types must be a Handle). Most likely you want "                                                                                                                       \
            "RaverieBindField.");                                                                                                                                                                      \
    return RaverieBuilder.AddBoundField(RaverieType, Name, RaverieTypeId(decltype(RaverieSelf::MemberName)), RaverieOffsetOf(RaverieSelf, MemberName), PropertyBinding);                               \
  }()

// Bind a property (getter and setter in C++) to Raverie
// A property will appear like a member, but it will invoke the getter when
// being read, and the setter when being written to
#define RaverieFullBindGetterSetter(RaverieBuilder, RaverieType, GetterMethodPointer, GetterOverload, SetterMethodPointer, SetterOverload, Name)                                                       \
  ::Raverie::TemplateBinding::FromGetterSetter<decltype(GetterOverload GetterMethodPointer), GetterMethodPointer, decltype(SetterOverload SetterMethodPointer), SetterMethodPointer>(                  \
      RaverieBuilder, RaverieType, Name, GetterMethodPointer, SetterMethodPointer)

// Bind a type as being an enum (verifies that the size matches)
#define RaverieFullBindEnum(RaverieBuilder, RaverieType, SpecialTypeEnum)                                                                                                                              \
  do                                                                                                                                                                                                   \
  {                                                                                                                                                                                                    \
    (RaverieType)->SpecialType = SpecialTypeEnum;                                                                                                                                                      \
    if ((SpecialTypeEnum) == ::Raverie::SpecialType::Enumeration)                                                                                                                                      \
      (RaverieType)->ToStringFunction = ::Raverie::VirtualMachine::EnumerationToString;                                                                                                                \
    else                                                                                                                                                                                               \
      (RaverieType)->ToStringFunction = ::Raverie::VirtualMachine::FlagsToString;                                                                                                                      \
    ErrorIf((RaverieType)->Size != sizeof(::Raverie::Integer), "The sizeof(Enum) bound to Raverie must match the sizeof(Integer)");                                                                    \
    (RaverieType)->BaseType = RaverieTypeId(::Raverie::Enum);                                                                                                                                          \
  } while (false)

// Bind a single value of
#define RaverieFullBindEnumValue(RaverieBuilder, RaverieType, EnumValue, Name) (RaverieBuilder).AddEnumValue((RaverieType), (Name), (EnumValue));

// Extra convenience macros that just wrap the above macros (simpler, intended
// for binding values only to our own type) Note that the Property versions only
// add the attribute "Property" and can be used for displaying within a property
// grid
#define RaverieBindConstructor(...) RaverieFullBindConstructor(builder, type, RaverieSelf, RaverieNoNames, ##__VA_ARGS__)
#define RaverieBindDefaultConstructor() RaverieBindConstructor()
#define RaverieBindCopyConstructor() RaverieBindConstructor(const RaverieSelf&)
#define RaverieBindDestructor() RaverieFullBindDestructor(builder, type, RaverieSelf)
#define RaverieBindDefaultDestructor()                                                                                                                                                                 \
  RaverieBindDefaultConstructor();                                                                                                                                                                     \
  RaverieBindDestructor();
#define RaverieBindDefaultCopyDestructor()                                                                                                                                                             \
  RaverieBindDefaultConstructor();                                                                                                                                                                     \
  RaverieBindCopyConstructor();                                                                                                                                                                        \
  RaverieBindDestructor();

// These versions allow you to rename anything bound
// Note that 'Custom' means we don't apply the Get or Set to the beginning of
// the name
#define RaverieBindOverloadedMethodAs(MethodName, OverloadResolution, Name) RaverieFullBindMethod(builder, type, &RaverieSelf::MethodName, OverloadResolution, Name, RaverieNoNames)
#define RaverieBindMethodAs(MethodName, Name) RaverieBindOverloadedMethodAs(MethodName, RaverieNoOverload, Name)
#define RaverieBindOverloadedMethodPropertyAs(MethodName, OverloadResolution, Name)                                                                                                                    \
  RaverieFullBindMethod(builder, type, &RaverieSelf::MethodName, OverloadResolution, Name, RaverieNoNames)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindMethodPropertyAs(MethodName, Name) RaverieBindOverloadedMethodAs(MethodName, RaverieNoOverload, Name)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindMemberAs(MemberName, Name) RaverieFullBindMember(builder, type, MemberName, Name, Raverie::MemberOptions::None)
#define RaverieBindMemberPropertyAs(MemberName, Name) RaverieBindMemberAs(MemberName, Name)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindFieldAs(FieldName, Name) RaverieFullBindField(builder, type, &RaverieSelf::FieldName, Name, Raverie::PropertyBinding::GetSet)
#define RaverieBindFieldGetterAs(FieldName, Name) RaverieFullBindField(builder, type, &RaverieSelf::FieldName, Name, Raverie::PropertyBinding::Get)
#define RaverieBindFieldSetterAs(FieldName, Name) RaverieFullBindField(builder, type, &RaverieSelf::FieldName, Name, Raverie::PropertyBinding::Set)
#define RaverieBindFieldPropertyAs(FieldName, Name) RaverieBindFieldAs(FieldName, Name)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindFieldGetterPropertyAs(FieldName, Name) RaverieBindFieldGetterAs(FieldName, Name)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindFieldSetterPropertyAs(FieldName, Name) RaverieBindFieldSetterAs(FieldName, Name)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindGetterAs(PropertyName, Name) RaverieFullBindGetterSetter(builder, type, &RaverieSelf::Get##PropertyName, RaverieNoOverload, RaverieNoSetter, RaverieNoOverload, Name)
#define RaverieBindSetterAs(PropertyName, Name) RaverieFullBindGetterSetter(builder, type, RaverieNoGetter, RaverieNoOverload, &RaverieSelf::Set##PropertyName, RaverieNoOverload, Name)
#define RaverieBindGetterSetterAs(PropertyName, Name)                                                                                                                                                  \
  RaverieFullBindGetterSetter(builder, type, &RaverieSelf::Get##PropertyName, RaverieNoOverload, &RaverieSelf::Set##PropertyName, RaverieNoOverload, Name)
#define RaverieBindGetterPropertyAs(PropertyName, Name) RaverieBindGetterAs(PropertyName, Name)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindSetterPropertyAs(PropertyName, Name) RaverieBindSetterAs(PropertyName, Name)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindGetterSetterPropertyAs(PropertyName, Name) RaverieBindGetterSetterAs(PropertyName, Name)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindCustomGetterAs(PropertyName, Name) RaverieFullBindGetterSetter(builder, type, &RaverieSelf::PropertyName, RaverieNoOverload, RaverieNoSetter, RaverieNoOverload, Name)
#define RaverieBindCustomSetterAs(PropertyName, Name) RaverieFullBindGetterSetter(builder, type, RaverieNoGetter, RaverieNoOverload, &RaverieSelf::PropertyName, RaverieNoOverload, Name)
#define RaverieBindCustomGetterSetterAs(PropertyName, Name)                                                                                                                                            \
  RaverieFullBindGetterSetter(builder, type, &RaverieSelf::PropertyName, RaverieNoOverload, &RaverieSelf::PropertyName, RaverieNoOverload, Name)
#define RaverieBindCustomGetterPropertyAs(PropertyName, Name) RaverieBindCustomGetterAs(PropertyName, Name)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindCustomSetterPropertyAs(PropertyName, Name) RaverieBindCustomSetterAs(PropertyName, Name)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindCustomGetterSetterPropertyAs(PropertyName, Name) RaverieBindCustomGetterSetterAs(PropertyName, Name)->AddAttributeChainable(Raverie::PropertyAttribute)
#define RaverieBindEnumValueAs(EnumValue, Name) RaverieFullBindEnumValue(builder, type, EnumValue, Name)

// All these versions assume the name is the same as the property/field/method
// identifier
#define RaverieBindOverloadedMethod(MethodName, OverloadResolution) RaverieBindOverloadedMethodAs(MethodName, OverloadResolution, #MethodName)
#define RaverieBindMethod(MethodName) RaverieBindMethodAs(MethodName, #MethodName)
#define RaverieBindOverloadedMethodProperty(MethodName, OverloadResolution) RaverieBindOverloadedPropertyMethodAs(MethodName, OverloadResolution, #MethodName)
#define RaverieBindMethodProperty(MethodName) RaverieBindMethodPropertyAs(MethodName, #MethodName)
#define RaverieBindMember(MemberName) RaverieBindMemberAs(MemberName, #MemberName)
#define RaverieBindMemberProperty(MemberName) RaverieBindMemberPropertyAs(MemberName, #MemberName)
#define RaverieBindField(FieldName) RaverieBindFieldAs(FieldName, #FieldName)
#define RaverieBindFieldGetter(FieldName) RaverieBindFieldGetterAs(FieldName, #FieldName)
#define RaverieBindFieldSetter(FieldName) RaverieBindFieldSetterAs(FieldName, #FieldName)
#define RaverieBindFieldProperty(FieldName) RaverieBindFieldPropertyAs(FieldName, #FieldName)
#define RaverieBindFieldGetterProperty(FieldName) RaverieBindFieldGetterPropertyAs(FieldName, #FieldName)
#define RaverieBindFieldSetterProperty(FieldName) RaverieBindFieldSetterPropertyAs(FieldName, #FieldName)
#define RaverieBindGetter(PropertyName) RaverieBindGetterAs(PropertyName, #PropertyName)
#define RaverieBindSetter(PropertyName) RaverieBindSetterAs(PropertyName, #PropertyName)
#define RaverieBindGetterSetter(PropertyName) RaverieBindGetterSetterAs(PropertyName, #PropertyName)
#define RaverieBindGetterProperty(PropertyName) RaverieBindGetterPropertyAs(PropertyName, #PropertyName)
#define RaverieBindSetterProperty(PropertyName) RaverieBindSetterPropertyAs(PropertyName, #PropertyName)
#define RaverieBindGetterSetterProperty(PropertyName) RaverieBindGetterSetterPropertyAs(PropertyName, #PropertyName)
#define RaverieBindCustomGetter(PropertyName) RaverieBindCustomGetterAs(PropertyName, #PropertyName)
#define RaverieBindCustomSetter(PropertyName) RaverieBindCustomSetterAs(PropertyName, #PropertyName)
#define RaverieBindCustomGetterSetter(PropertyName) RaverieBindCustomGetterSetterAs(PropertyName, #PropertyName)
#define RaverieBindCustomGetterProperty(PropertyName) RaverieBindCustomGetterPropertyAs(PropertyName, #PropertyName)
#define RaverieBindCustomSetterProperty(PropertyName) RaverieBindCustomSetterPropertyAs(PropertyName, #PropertyName)
#define RaverieBindCustomGetterSetterProperty(PropertyName) RaverieBindCustomGetterSetterPropertyAs(PropertyName, #PropertyName)
#define RaverieBindEnumValue(EnumValue) RaverieBindEnumValueAs(EnumValue, #EnumValue)

// Overload resolution helper macros
#define RaverieStaticOverload(ReturnType, ...) (ReturnType(*)(__VA_ARGS__))
#define RaverieInstanceOverload(ReturnType, ...) (ReturnType(RaverieSelf::*)(__VA_ARGS__))
#define RaverieConstInstanceOverload(ReturnType, ...) (ReturnType(RaverieSelf::*)(__VA_ARGS__) const)

// This macro sets up all the special attributes to make a C++ value into a
// Raverie enumeration
#define RaverieBindEnum(SpecialTypeEnum) RaverieFullBindEnum(builder, type, SpecialTypeEnum)
} // namespace Raverie
