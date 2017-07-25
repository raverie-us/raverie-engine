/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_TYPE_HPP
#define ZILCH_TYPE_HPP

namespace Zilch
{
  // A structure that is responsible for resolving members and functions of a class
  class ZeroShared Resolver
  {
  public:
    // Get the instance resolver
    static Resolver Instance(BoundType* type);

    // Get the static resolver
    static Resolver Static(BoundType* type);

    // The virtual member functions that we'll call on the compiled type
    GetOverloadedFunctionsFn  GetOverloadedFunctions;
    GetFieldFn                GetField;
    GetGetterSetterFn         GetGetterSetter;
    BoundType*                TypeInstance;
    bool                      IsStatic;
  };

  // In the case where we're attempting to find members
  namespace FindMemberOptions
  {
    enum Enum
    {
      // No options
      None = 0,

      // A member that is not part of an instance,
      // but rather part of the type itself
      Static = 1,

      // Specifies that we will search only this type,
      // and not the hierarchy (base types) for the function
      DoNotIncludeBaseClasses = 2,
    };
    typedef unsigned Flags;
  }

  namespace CastOperation
  {
    enum Enum
    {
      // The casting operation was not valid
      Invalid,

      // A cast that has nothing to do (the underlying values are equivalent in raw form)
      // Includes casting from a derived class to a base class (ex: Cat to Animal)
      // Also include casting from NullType to any reference/handle type (null to Animal)
      Raw,
      
      // Casting from a built in primitive (ex: Integer to Real)
      // This really implies that there is an entire instruction made to deal with this cast
      Primitive,
      
      // Casting a base class to a derived class, which must be checked (ex: Animal to Cat)
      // This cast is also used for 'AnyHandle' to anything
      DynamicDown,

      // Casting to the 'Any' type, a special type that can contain or be anything (ex: Integer to Any)
      ToAny,

      // Casting from the 'Any' type which must be checked (ex: Any to Integer)
      FromAny,

      // A special cast from null to a delegate type (null is a handle type)
      NullToDelegate,
    };
  }

  // A type is the base representation of all types
  class ZeroShared Type : public ReflectionObject
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Constructor
    Type();

    // ReflectionObject interface
    Library* GetOwningLibrary() override;
    Type* GetTypeOrNull() override;

    // Hash the type
    virtual GuidType Hash() const = 0;

    // Gets a human readable name for the type
    // This name is only used for debugging and printing
    virtual String ToString() const = 0;

    // Gets a short non-unique name in lower camel case form (can be a keyword)
    virtual String GetShortLowerCamelCaseName() const = 0;

    // Checks whether a type has a complicated copy operator (handles, delegates, etc)
    // The opposite of this being that the data is POD (plain old data / value types)
    virtual bool IsCopyComplex() const = 0;

    // Get the size of the type (or if it's a reference type, the size of a handle)
    virtual size_t GetCopyableSize() const;

    // Get the size of the type if we were to allocate it in a memory block
    virtual size_t GetAllocatedSize() const = 0;

    // Constructs a default instance of this type
    // For as many types as possible, this will construct something akin to null
    // If the type does not support a null (such as value types) it will clear the memory to zeros
    // This is mainly used for templating, and the concept of 'default(T)'
    virtual void GenericDefaultConstruct(byte* toConstruct) const = 0;

    // Copies a value at one place to another
    // If the given type is a reference / delegate type, a handle copy will be performed
    // All value types will be directly memory copied
    virtual void GenericCopyConstruct(byte* to, const byte* from) const = 0;

    // Performs a handle release in the case that the type is a reference / delegate
    // Otherwise in the case of value types, this does nothing
    virtual void GenericDestruct(byte* value) const = 0;

    // Get the memory of the underlying object (only used for debugging)
    // For a reference/handle type, we will Dereference the handle (so this could be null)
    // Otherwise for value types, it will just directly return the passed in memory
    // For delegate types, this will also directly return the memory
    // For the AnyType, this will recursively call GenericGetMemory on the value stored inside the Any (if none, it will return itself)
    virtual byte* GenericGetMemory(const byte* value) const;

    // Get the most derived type of whatever we're looking at
    // In all case that the values that the value is null of (whatever that means) it will return this types itself
    // For bound value types this will just return the type directly (value types are always the correct type)
    // For all handle types (bound reference, indirect, AnyHandleType) this will return the type stored in the handle
    // For all delegate types (delegate type, AnyDelegateType) this will return the type of the function stored in the delegate
    // For the AnyType, this will recursively call GenericGetVirtualType on the value stored within the Any (if none, it will return itself)
    virtual Type* GenericGetVirtualType(const byte* value) const = 0;

    // Hashes an object of this type
    virtual Integer GenericHash(const byte* value) const = 0;

    // Converts the object or value into a human readable string
    virtual String GenericToString(const byte* value) const = 0;

    // Tests equality of two objects of the exact same type
    virtual Boolean GenericEquals(const byte* lhs, const byte* rhs) const = 0;

    // Check if a given type is ref struct (indirection)
    static bool IsIndirectionType(Type* type);

    // Check if a given type is a handle type
    static bool IsHandleType(Type* type);

    // Check if a given type is a value type
    static bool IsValueType(Type* type);

    // Check if a given type is a delegate type
    static bool IsDelegateType(Type* type);

    // Check if a given type is an enum type
    static bool IsEnumType(Type* type);

    // Check if a given type is an flags type
    static bool IsFlagsType(Type* type);

    // Check if a given type is an enum or flags type
    static bool IsEnumOrFlagsType(Type* type);

    // Check if a given type is a any type
    static bool IsAnyType(Type* type);

    // Generically get the base class type (if we have one) or return null
    static Type* GetBaseType(Type* type);

    // If the given type is a BoundType, it will cast it and return it directly
    // If the given type is an IndirectionType, then it will return the 'ReferencedType' on that
    static BoundType* GetBoundType(Type* handleType);

    // Returns the type referenced by a handle type, or nullptr if the type was not a handle
    // Note that this function works for both indirect types as well as class bound types
    static BoundType* GetHandleType(Type* handleType);

    // Check if two types are of the same type
    static bool IsSame(Type* a, Type* b);

    // Check if a type is derived from a given base
    static bool BoundIsA(BoundType* type, BoundType* base);

    // Check if a type is derived from a given base
    static bool IndirectionIsA(IndirectionType* type, IndirectionType* base);

    // Check if a type is derived from a given base
    static bool GenericIsA(Type* type, Type* base);

    // Dynamically cast a base type into a derived type
    template <typename Derived, typename Base>
    static Derived DynamicCast(Base baseClassPointer)
    {
      // If the passed in pointer was null, then return null
      if (baseClassPointer == nullptr)
      {
        // We can't possibly cast it or know what it even is...
        return nullptr;
      }

      // If the cast is safe...
      if (BoundIsA(baseClassPointer->ZilchGetDerivedType(), ZilchTypeId(Derived)))
      {
        // Get the base class pointer
        return static_cast<Derived>(baseClassPointer);
      }

      // Otherwise the cast failed, return a null pointer
      return nullptr;
    }

    // Dynamically cast a base type into a derived type
    template <typename Derived, typename Base>
    static Derived DebugOnlyDynamicCast(Base baseClassPointer)
    {
#if ZeroDebug
      if (baseClassPointer == nullptr)
        return nullptr;

      Derived derived = DynamicCast<Derived, Base>(baseClassPointer);
      ErrorIf(derived == nullptr, "The debug only dynamic cast, which means the type was not what we expected");
      return derived;
#else
      return static_cast<Derived>(baseClassPointer);
#endif
    }

    // Whether we can cast our type to another type
    bool IsCastableTo(Type* toType);

    // Whether we can cast our type to another type without any work (binary compatible)
    bool IsRawCastableTo(Type* toType);

    // Binding for reflection
    bool IsA(Type* base);
    bool IsHandle();
    bool IsValue();
    bool IsDelegate();
    bool IsEnum();
    bool IsFlags();
    bool IsEnumOrFlags();
    bool IsAny();

    // Store the parent library from which the type originated
    // This information is generally used for linking purposes
    Library* SourceLibrary;

    // Not copyable
    ZilchNoCopy(Type);
  };

  // A qualified type represents a true-type with qualifications
  class ZeroShared IndirectionType : public Type
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Constructor
    IndirectionType();

    // Implement the type interface
    bool IsCopyComplex() const override;
    size_t GetAllocatedSize() const override;
    GuidType Hash() const override;
    String ToString() const override;
    String GetShortLowerCamelCaseName() const override;
    void GenericDefaultConstruct(byte* toConstruct) const override;
    void GenericCopyConstruct(byte* to, const byte* from) const override;
    void GenericDestruct(byte* value) const override;
    int GenericHash(const byte* value) const override;
    String GenericToString(const byte* value) const override;
    bool GenericEquals(const byte* lhs, const byte* rhs) const override;
    byte* GenericGetMemory(const byte* value) const override;
    Type* GenericGetVirtualType(const byte* value) const override;

    // Composition interface
    Composition* GetBaseComposition() override;

    // Store the referenced type (the parent type that this qualified type represents)
    BoundType* ReferencedType;
  };

  // The 'any type' is a special type that can contain any primitive (delegates, handles, values, etc)
  // Note: There should only ever be one instantiation of this type (core.AnyType)
  class ZeroShared AnyType : public Type
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Constructor
    AnyType();

    // Implement the type interface
    bool IsCopyComplex() const override;
    size_t GetAllocatedSize() const override;
    GuidType Hash() const override;
    String ToString() const override;
    String GetShortLowerCamelCaseName() const override;
    void GenericDefaultConstruct(byte* toConstruct) const override;
    void GenericCopyConstruct(byte* to, const byte* from) const override;
    void GenericDestruct(byte* value) const override;
    int GenericHash(const byte* value) const override;
    String GenericToString(const byte* value) const override;
    bool GenericEquals(const byte* lhs, const byte* rhs) const override;
    byte* GenericGetMemory(const byte* value) const override;
    Type* GenericGetVirtualType(const byte* value) const override;
  };

  // A special constant that means we haven't yet figured out the size of an object
  static const size_t UndeterminedSize = (size_t)-1;

  // A function used for converting a value or object to a string
  typedef String (*ToStringFn)(const BoundType* type, const byte* data);

  // Retrieves the event handler for an object (or null if this object cannot send/receive events)
  typedef EventHandler* (*GetEventHandlerFn)(const BoundType* type, const byte* data);

  namespace SpecialType
  {
    enum Enum
    {
      Standard,
      Enumeration,
      Flags,

      // All enums bound to Zilch must be of Integer size, so force it on all platforms
      ForceIntegerSize = 0x7FFFFFFF
    };
  }

  namespace AddMemberResult
  {
    enum Enum
    {
      AlreadyExists,
      Added
    };
  }

  namespace Members
  {
    enum Enum
    {
      Instance  = 1,
      Static    = 2,
      Inherited = 4,
      Extension = 8,

      InstanceStatic             = Instance  | Static,
      InheritedInstanceStatic    = Inherited | InstanceStatic,
      InheritedInstance          = Inherited | Instance,
      InheritedInstanceExtension = Extension | InheritedInstance,
      InheritedStatic            = Inherited | Static,
      All                        = Extension | InheritedInstanceStatic,

      // All enums bound to Zilch must be of Integer size, so force it on all platforms
      ForceIntegerSize = 0x7FFFFFFF
    };
  }

  // This is only used for native type binding
  // (an assert we can give to the user to give them feedback if they forgot to bind a type)
  typedef void (*BoundTypeAssertFn)(const char* prependedMessage);

  // Allows us to walk through filtered members of a BoundType (including up the inheritance chain if the option is specified)
  template <typename MemberType = Member>
  class ZeroSharedTemplate MemberRange
  {
  public:
    typedef MemberType* FrontResult;

    MemberRange() :
      IteratingType(nullptr),
      DeclaredType(nullptr),
      Options(Members::InheritedInstanceStatic),
      MemberKind(nullptr),
      CurrentMember(nullptr)
    {
    }

    MemberRange(BoundType* type, StringParam name, Type* declaredType, Members::Enum options, BoundType* memberKind = nullptr) :
      IteratingType(type),
      DerivedType(type),
      Name(name),
      DeclaredType(declaredType),
      CurrentMember(nullptr)
    {
      // If neither instance or static is specified, then assume both
      if ((options & (Members::Static | Members::Instance)) == 0)
        options = (Members::Enum)(options | Members::Static | Members::Instance);
      this->Options = options;

      // If the user did not specify a type of member, then look to see if the template argument provides more information
      // In the case where the template argument is also just 'Member', then for efficiency we just leave it at null
      if (memberKind == nullptr && ZilchTypeId(MemberType) != ZilchTypeId(Member))
        memberKind = ZilchTypeId(MemberType);
      this->MemberKind = memberKind;

      if (type == nullptr)
        return;

      // If we're including all inherited members, we want them to be in base to derived order,
      // so walk down and start with the base
      if ((options & Members::Inherited) != 0)
      {
        while(this->IteratingType->BaseType)
          this->IteratingType = this->IteratingType->BaseType;
      }

      this->MembersRange = this->IteratingType->AllMembers.All();
      this->PopFront();
    }

    MemberRange All()
    {
      return *this;
    }

    MemberType* Front()
    {
      return static_cast<MemberType*>(this->CurrentMember);
    }

    // This helper either returns the front, or nullptr if the range is empty
    MemberType* First()
    {
      if (this->Empty())
        return nullptr;
      return this->Front();
    }

    void PopFront()
    {
      // Assume that we won't find the next matching member
      this->CurrentMember = nullptr;

      ZilchLoop
      {
        // If we exhausted the range last time, then we need a new range
        if (this->MembersRange.Empty())
        {
          // If we were already on the most derived type, we're done
          if (this->IteratingType == this->DerivedType)
            return;

          // Find the next type below the iterating type (more derived)
          BoundType* currentType = this->DerivedType;
          while (currentType->BaseType != this->IteratingType)
            currentType = currentType->BaseType;
          this->IteratingType = currentType;

          this->MembersRange = this->IteratingType->AllMembers.All();
        }

        while (this->MembersRange.Empty() == false)
        {
          Member* member = this->MembersRange.Front();
          this->MembersRange.PopFront();

          // We only check if the name matches if one was provided
          if (this->Name.Empty() == false && this->Name != member->Name)
            continue;

          // Check if the user is attempting to filter by members of a specific type
          if (this->DeclaredType != nullptr && member->GetTypeOrNull() != this->DeclaredType)
            continue;

          // The user might specifically want a Field, GetterSetter, Function, or even a base type such as Property or Member (which is everything)
          BoundType* derivedType = ZilchVirtualTypeId(member);
          if (this->MemberKind != nullptr && derivedType->IsA(this->MemberKind) == false)
            continue;

          bool optionsStatic    = (this->Options & Members::Static)   != 0;
          bool optionsInstance  = (this->Options & Members::Instance) != 0;
          bool memberStatic     = member->IsStatic;
          bool memberInstance   = !memberStatic;

          if (memberStatic && !optionsStatic)
            continue;

          if (memberInstance && !optionsInstance)
            continue;

          this->CurrentMember = member;
          return;
        }
      }
    }

    bool Empty()
    {
      return this->CurrentMember == nullptr;
    }

  public:
    // The current type we're iterating over (from base to derived if Inherited)
    BoundType* IteratingType;

    // The exact type the user passed in (does not change)
    BoundType* DerivedType;

    // If the name is empty, it means we aren't filtering members by name
    String Name;

    // If the declared type is null, it means we aren't filtering members by type
    Type* DeclaredType;

    // Any options that help filter out members (such as if we walk up the inheritance chain)
    Members::Enum Options;

    // Specific types of members that we look for, or null if we want to allow all member types (Field, GetterSetter, Function, etc)
    BoundType* MemberKind;

    // The current member sub-range that we are iterating through
    MemberArrayRange MembersRange;

    // The current library sub-range that we are iterating through
    LibraryRange Libraries;

    // The current member that is our front
    Member* CurrentMember;
  };

  // A type that provides us with a simple way of grabbing members and functions from maps
  class ZeroShared BoundType : public Type
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Constructor
    BoundType(BoundTypeAssertFn nativeBindingAssert);

    // Destructor
    ~BoundType();

    // Implement the type interface
    bool IsCopyComplex() const override;
    size_t GetCopyableSize() const override;
    size_t GetAllocatedSize() const override;
    void GenericDefaultConstruct(byte* toConstruct) const override;
    void GenericCopyConstruct(byte* to, const byte* from) const override;
    void GenericDestruct(byte* value) const override;
    int GenericHash(const byte* value) const override;
    String GenericToString(const byte* value) const override;
    bool GenericEquals(const byte* lhs, const byte* rhs) const override;
    byte* GenericGetMemory(const byte* value) const override;
    Type* GenericGetVirtualType(const byte* value) const override;
    GuidType Hash() const override;
    String ToString() const override;
    String GetShortLowerCamelCaseName() const override;

    // Composition interface
    Composition* GetBaseComposition() override;

    // Finds a function by name given the type signature
    Function* FindFunction(StringParam name, const Array<Type*>& parameters, Type* returnType, FindMemberOptions::Flags options) const;

    // Finds a function given a delegate type
    Function* FindFunction(StringParam name, DelegateType* type, FindMemberOptions::Flags options) const;

    // Finds a property or field by a name (can be static or instance)
    Property* FindProperty(StringParam name, FindMemberOptions::Flags options) const;

    // Get an array of overloaded instance functions under the same name. Returning null
    // or an empty array will signal that the function does not exist. Moreover, returning
    // a single function in the array will also work as if it were not overloaded
    const FunctionArray* GetOverloadedInstanceFunctions(StringParam name) const;

    // Get an array of overloaded static functions under the same name. Returning null
    // or an empty array will signal that the function does not exist. Moreover, returning
    // a single function in the array will also work as if it were not overloaded
    const FunctionArray* GetOverloadedStaticFunctions(StringParam name) const;

    // Get an array of overloaded constructors (either on your type, or inherited). Returning
    // null or an empty array will signal that no constructors exist. Moreover, returning
    // a single constructor in the array will also work as if it were not overloaded
    const FunctionArray* GetOverloadedInheritedConstructors() const;

    // Get an instance member by name
    Field* GetInstanceField(StringParam name) const;

    // Get a static member by name
    Field* GetStaticField(StringParam name) const;

    // Get an instance property by name
    GetterSetter* GetInstanceGetterSetter(StringParam name) const;

    // Get a static property by name
    GetterSetter* GetStaticGetterSetter(StringParam name) const;

    // Add a function (created through the library) to the bound type
    AddMemberResult::Enum AddRawFunction(Function* function);

    // Add a constructor (created through the library) to the bound type
    AddMemberResult::Enum AddRawConstructor(Function* function);

    // Add a property to the bound type
    AddMemberResult::Enum AddRawGetterSetter(GetterSetter* property);

    // Add a field to the bound type
    AddMemberResult::Enum AddRawField(Field* field);

    // Get either the static or instance field map (maps strings to the fields)
    FieldMap& GetFieldMap(bool isStatic);

    // Get either the static or instance property map (maps strings to the fields)
    GetterSetterMap& GetGetterSetterMap(bool isStatic);

    // Get either the static or instance function map (maps strings to the fields)
    FunctionMultiMap& GetFunctionMap(bool isStatic);

    // Get an event handler from an instance of the object
    EventHandler* GetEventHandler(const byte* data);

    // Check if this type or any base type is a native type
    bool IsTypeOrBaseNative();
    
    // Attempt to get a default constructor (or return null if one cannot be found)
    static Function* GetDefaultConstructor(const FunctionArray* constructors);

    // Attempt to get a copy constructor (or return null if one cannot be found)
    static Function* GetCopyConstructor(const FunctionArray* constructors);

    // By default, when our object is converted into
    // a string it simply prints out the type name
    static String DefaultTypeToString(const BoundType* type, const byte* data);

    // Creates an allocated instance of the current type and returns
    // Note that for value types, this will return a ref (indirection type)
    // The value will be allocated within the current running state
    // The object will NOT have a constructor called on it
    Any InstantiatePreConstructedObject();

    // Lets us know whether the type has been initialized
    // When finalizing a library we check that all types referenced are initialized
    bool IsInitialized();

    // Throws an assert if this type is not initialized
    // Returns true if it is initialized, false if it is not
    bool IsInitializedAssert(const char* prependedMessage = "");

    // Binding for reflection (internally bound in Zilch)
    MemberRange<GetterSetter> GetGetterSetters();
    MemberRange<GetterSetter> GetGetterSetters(Members::Enum options);
    MemberRange<Property> GetProperties();
    MemberRange<Property> GetProperties(Members::Enum options);
    MemberRange<Field> GetFields();
    MemberRange<Field> GetFields(Members::Enum options);
    MemberRange<Function> GetFunctions();
    MemberRange<Function> GetFunctions(Members::Enum options);
    MemberRange<Function> GetFunctions(StringParam name, Members::Enum options);
    MemberRange<Member> GetMembers();
    MemberRange<Member> GetMembers(Members::Enum options);
    
    // If you look for instance AND static, you will get whichever is declared first!
    GetterSetter* GetGetterSetter(StringParam name);
    GetterSetter* GetGetterSetter(StringParam name, Members::Enum options);
    GetterSetter* GetGetterSetter(StringParam name, Type* declaredType, Members::Enum options);
    Property* GetProperty(StringParam name);
    Property* GetProperty(StringParam name, Members::Enum options);
    Property* GetProperty(StringParam name, Type* declaredType, Members::Enum options);
    Field* GetField(StringParam name);
    Field* GetField(StringParam name, Members::Enum options);
    Field* GetField(StringParam name, Type* declaredType, Members::Enum options);
    Function* GetFunction(StringParam name);
    Function* GetFunction(StringParam name, Members::Enum options);
    Function* GetFunction(StringParam name, DelegateType* type, Members::Enum options);
    Member* GetMember(StringParam name);
    Member* GetMember(StringParam name, Members::Enum options);
    Member* GetMember(StringParam name, Type* declaredType, Members::Enum options);
    
    Function* GetDefaultConstructor();
    Function* GetDefaultConstructor(bool inherited);
    Function* GetConstructor(DelegateType* type, bool inherited);

    // Checks to see if an attribute exists on this, or any base type
    Attribute* HasAttributeInherited(StringParam name);

  public:

    // Store the name of the type
    // If this type represents an instantiated template, then this will be the full name, eg "Array[Integer]"
    String Name;

    // Tells us whether this is considered to be a value type or not
    // Value types have the property that they are memory copyable
    // and can therefore not store any references of any kind (only POD data)
    TypeCopyMode::Enum CopyMode;

    // Whether or not we can use new/local to create this type
    bool CreatableInScript;

    // Whether this type was bound natively
    bool Native;

    // A flag that lets us know we can ignore the destructor for a native type (default false)
    bool IgnoreNativeDestructor;

    // Whether or not we allow types to inherit from this type
    bool Sealed;

    // If this type is a value type, then this will point at the indirect type (handle)
    // Otherwise this is null
    IndirectionType* IndirectType;

    // If this type is a template, then these are the arguments it was instantiated with
    // Any empty array means this type is not a template (and visa versa)
    Array<Constant> TemplateArguments;

    // Store the pre-constructor
    Function* PreConstructor;

    // Store the constructors
    FunctionArray Constructors;

    // The singular destructor (can be null if no destructor exists)
    Function* Destructor;

    // Responsible for the destruction of members
    PostDestructorFn PostDestructor;

    // Store all the instance functions defined within this class
    FunctionMultiMap InstanceFunctions;

    // Store all the static functions defined within this class
    FunctionMultiMap StaticFunctions;

    // Store all the instance class fields
    // Even though fields are properties, they do not exist in the property map
    FieldMap InstanceFields;

    // Store all the static class fields
    // Even though fields are properties, they do not exist in the property map
    FieldMap StaticFields;

    // Store all the instance class properties
    GetterSetterMap InstanceGetterSetters;

    // Store all the static class properties
    GetterSetterMap StaticGetterSetters;

    // Which events get sent by this type
    SendsEventArray SendsEvents;

    // All the properties in the order they are declared (includes both static and instance!)
    // This also includes all fields and properties (unlike the PropertyMap)
    // Includes Fields and GetterSetters
    PropertyArray AllProperties;

    // All the functions in the order they are declared (includes both static and instance!)
    // This does not include constructors, however it does include generated Get/Set functions (check for OwningProperty)
    FunctionArray AllFunctions;

    // All the members in the order they are declared (includes both static and instance!)
    // Includes Functions, Fields, and GetterSetters
    MemberArray AllMembers;
    
    // Store the size of the object
    size_t Size;

    // For template instantiations, this is just the base without the template brackets
    // E.g. for "Array[Integer]" this will be just "Array"
    // For all other non-template types this will just be the same as Name
    String TemplateBaseName;

    // In the case that this type is being generically allocated or a handle
    // is being pushed via binding/pointer, we need to know which handle manager
    // we are associated with
    HandleManagerId HandleManager;

    // The number of native virtual functions that the type has (0 if the type is non-native)
    // This is basically the exact number of entries in the C++ virtual table
    size_t RawNativeVirtualCount;

    // How many virtual functions we've actually bound
    // This is used to determine whether or not we need to build a v-table for this type upon creation
    size_t BoundNativeVirtualCount;

    // A user provided function that converts an instance of this type into a string
    ToStringFn ToStringFunction;

    // A user provided function that retrieves an event handler from an instance of this type
    GetEventHandlerFn GetEventHandlerFunction;

    // A bound type could be an enum, flags, or just a regular old type
    // This is important because it affects what operators can be applied to the object
    SpecialType::Enum SpecialType;

    // Any interface types that we implement
    Array<BoundType*> InterfaceTypes;

    // The base type (or null if there is no base)
    BoundType* BaseType;

    // The handles that need to be cleaned up
    Array<size_t> Handles;

    // The delegates that need to be cleaned up
    Array<size_t> Delegates;

    // This is a function we call when a type was not properly initialized via native binding
    BoundTypeAssertFn AssertOnInvalidBinding;

    // If this is an enum, this is the default value
    Integer DefaultEnumValue;
    Property* DefaultEnumProperty;

    // Maps the name of an enum value to its value
    HashMap<Property*, Integer> PropertyToEnumValue;
    HashMap<Integer, Array<Property*>> EnumValueToProperties;
    HashMap<String, Integer> StringToEnumValue;
    HashMap<Integer, Array<String>> EnumValueToStrings;
  };

  // Structure for delegate parameters
  class ZeroShared DelegateParameter
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Constructor
    DelegateParameter();

    // Implicit conversion from a type
    DelegateParameter(Type* type);

    // If the name is not empty, it returns it, otherwise it generates a name
    String GetNameOrGenerate();

    // The name of the parameter
    String Name;

    // The type of the parameter
    Type* ParameterType;

    // The offset on the stack where this parameter exists
    // Note that the first parameter always comes after the
    // return value, and every parameter comes after that
    OperandIndex StackOffset;

    // Which parameter this is (set afterwards, when we initialize StackOffset)
    size_t Index;
  };

  // Type-defines
  typedef Array<DelegateParameter> ParameterArray;

  // The delegate type essentially stores the signature of a function (regardless of the class it belongs to)
  class ZeroShared DelegateType : public Type
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Constructor
    DelegateType();
    
    // Implement the type interface
    bool IsCopyComplex() const override;
    size_t GetAllocatedSize() const override;
    GuidType Hash() const override;
    String ToString() const override;
    String GetShortLowerCamelCaseName() const override;
    void GenericDefaultConstruct(byte* toConstruct) const override;
    void GenericCopyConstruct(byte* to, const byte* from) const override;
    void GenericDestruct(byte* value) const override;
    int GenericHash(const byte* value) const override;
    String GenericToString(const byte* value) const override;
    bool GenericEquals(const byte* lhs, const byte* rhs) const override;
    Type* GenericGetVirtualType(const byte* value) const override;

    // Builds the just the parameter part of the string (used in overload resolving)
    void BuildParameterString(StringBuilder& builder, bool declaredNames = true) const;

    // Builds the entire signature string, without the name
    void BuildSignatureString(StringBuilder& builder, bool declaredNames = true) const;

    // Gets the entire parameter string, without the function name (includes parameter names)
    String GetParameterString() const;

    // Gets the entire signature string, without the function name (includes parameter names)
    String GetSignatureString() const;

    // For reflection
    ParameterArray::range GetParameters();

  public:

    // The list of parameter types this function will take
    ParameterArray Parameters;

    // The single return type of the delegate (should never be null, but rather core.VoidType)
    Type* Return;

    // The return value's stack offset
    // This should always be zero (just added for clarity)
    OperandIndex ReturnStackOffset;

    // If we're calling a delegate that uses a 'this' handle, then this
    // parameter represents the stack offset to where the handle exists
    // In our calling convention, the 'this' handle always resides after
    // the return value and all parameters
    OperandIndex ThisHandleStackOffset;

    // The total stack size, excluding the optional this handle size at the end
    // Note that this should always be the same value as 'ThisHandleStackOffset'
    // (it is provided for clarification)
    size_t TotalStackSizeExcludingThisHandle;
  };

  // Helper template to set the ToStringFunction on BoundType.
  // Assumes a global ToString function exists for the given template type.
  template <typename T>
  String BoundTypeToGlobalToString(const BoundType* type, const byte* data)
  {
    T& value = *(T*)data;
    return ToString(value);
  }
}

#endif
