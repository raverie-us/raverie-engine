/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_MEMBERS_HPP
#define ZILCH_MEMBERS_HPP

namespace Zilch
{
  // The types of attribute parameters we support
  namespace ConstantType
  {
    enum Enum
    {
      Null,
      String,
      Integer,
      DoubleInteger,
      Real,
      DoubleReal,
      Boolean,
      Type
    };
    static const cstr Names[] =
    {
      "Null",
      "String",
      "Integer",
      "DoubleInteger",
      "Real",
      "DoubleReal",
      "Boolean",
      "Type",
      nullptr
    };
  }

  // A constant can be any of the following primitives
  class ZeroShared Constant
  {
  public:
    Constant();

    // Helper constructors
    Constant(StringParam value);
    Constant(Integer value);
    Constant(DoubleInteger value);
    Constant(Real value);
    Constant(DoubleReal value);
    Constant(Boolean value);
    Constant(Type* value);
    explicit Constant(NullPointerType value);

    // Clears the constant back to Null and resets all values to default
    void Clear();

    // Convert the value we store into a string
    // The value will be represented as if it were a Zilch token (quotes will be added to strings, etc)
    String ToString() const;

    // The type of the constant (literals only, default Null)
    ConstantType::Enum Type;

    // When Type is String, this will be the fully unescaped version of the string (default empty)
    // When Type is Type, this will be the full name of the type
    String StringValue;

    // Integer and DoubleInteger values are both stored here (default 0)
    DoubleInteger IntegerValue;
    
    // Real and DoubleReal values are both stored here (default 0.0)
    DoubleReal RealValue;
    
    // When Type is Boolean, this will be set to true or false (default false)
    Boolean BooleanValue;

    // When this constant represents a Type (default nullptr)
    Zilch::Type* TypeValue;
  };

  // An attribute parameter can be any of the literal types we support
  class ZeroShared AttributeParameter : public Constant
  {
  public:
    // An optional name given to the parameter (if the user used name parameter calling)
    String Name;
  };

  // An attribute provides extra data about classes, functions, fields,
  // properties, etc that the language normally does not provide
  // An example would be marking a property as have a range of values from 1 to 100 eg [Range(1, 100)]
  class ZeroShared Attribute
  {
  public:
    Attribute();

    // Checks to see if an attribute parameter exists (returns a pointer to the attribute parameter)
    AttributeParameter* HasAttributeParameter(StringParam name);

    // The reflection object that owns this attribute
    ReflectionObject* Owner;

    // The name of the attribute
    String Name;

    // All the parameters we parsed from the attributes in the order they were given
    Array<AttributeParameter> Parameters;

    // Adds an attribute to the owning reflection object (useful for chaining)
    Attribute* AddAttribute(StringParam name);
  };

  // Provides a description
  class ZeroShared ReflectionObject : public Composition
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Constructor
    ReflectionObject();

    // Checks to see if an attribute exists (returns a pointer to the attribute)
    Attribute* HasAttribute(StringParam name);

    // Adds an attribute by name
    Attribute* AddAttribute(StringParam name);

    // Used for chaining within macros
    ReflectionObject* AddAttributeChainable(StringParam name);

    // Gets the library that owns our documented object
    virtual Library* GetOwningLibrary() = 0;

    // If this documented object has a resulting type (or represents a type) this will return it
    virtual Type* GetTypeOrNull();

    // The location of just the name/identifier for this document object
    // This is what gets selected in common IDE commands such as Go-To-Definition
    CodeLocation NameLocation;

    // A basic description for the type (typically parsed from comments)
    String Description;
    
    // Any samples or remarks about the usage of a particular object or member
    StringArray Remarks;

    // All the attributes attached to this type
    Array<Attribute> Attributes;

    // All documented objects can be hidden (for things parsed in language, use the [Hidden] attribute)
    bool IsHidden;

    // The code location at which the object was defined
    CodeLocation Location;

    // A pointer to any data the user wants to attach
    mutable const void* UserData;

    // Any user data that cant simply be represented by a pointer
    // Data can be written to the buffer and will be properly destructed
    // when this object is destroyed (must be read in the order it's written)
    mutable DestructibleBuffer ComplexUserData;
  };

  // Any options we use while building a member
  namespace MemberOptions
  {
    enum Enum
    {
      // No options
      None = 0,

      // A member that is not part of an instance,
      // but rather part of the type itself
      Static = 1,
    };
    typedef unsigned Flags;
  }

  // All primitives that appear on types (properties, fields, functions, etc) are members
  class ZeroShared Member : public ReflectionObject
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Default constructor
    Member();

    // ReflectionObject interface
    Library* GetOwningLibrary() override;

    // A helper function that returns the member options enumeration
    MemberOptions::Enum GetMemberOptions();

    // The owning type that this member belongs to
    BoundType* Owner;

    // Store the name of the function
    String Name;

    // Whether or not the property is considered static
    bool IsStatic;

    // Used for validating a 'this' handle passed to us via reflection
    bool ValidateInstanceHandle(const Any& instance, Handle& thisHandle);
  };

  // A class property basically consists of two functions that let us get and set a variable
  class ZeroShared Property : public Member
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Constructor
    Property();

    // ReflectionObject interface
    Type* GetTypeOrNull() override;

    // Returns whether or not the Set function is null (added for readability).
    bool IsReadOnly();

    // Mark if this property is hidden when null (useful for showing things like optional components)
    // This only applies to nullable types like handles and delegates
    bool IsHiddenWhenNull;

    // The type and offset into the class it belongs to
    Function* Get;
    Function* Set;

    // The type that we represent
    Type* PropertyType;

    // For reflection purposes
    Any GetValue(const Any& instance);
    void SetValue(const Any& instance, const Any& value);
  };

  // A getter setter is a property with an explicitly declared get/set
  // Since Property already has all the members of a getter setter,
  // this class is really just to add type information
  class ZeroShared GetterSetter : public Property
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);
  };

  // A class field basically consists of the type, as well as the offset into the memory block
  class ZeroShared Field : public Property
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Constructor
    Field();

    // The offset into the class it belongs to
    size_t Offset;

    // Used to initialize this field only
    // This functionality is used by on the fly patching of classes, as well as by static fields (which are initialized upon the first use)
    // This function generally Contains the initializer expression and a return opcode (no arguments, no return value)
    // WARNING: Field initializers do not assume a value exists in its previous place and will NOT destruct any previous values
    // If the intializer does not exist then it is generally acceptable to call GenericDefaultConstruct (which will typically just zero out memory)
    // Instance field initializers must be given a valid instance of the object that Contains the field (for the 'this' parameter)
    Function* Initializer;
  };

  // Store information about a variable inside a function
  class ZeroShared Variable : public ReflectionObject
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Constructor
    Variable();

    // ReflectionObject interface
    Library* GetOwningLibrary() override;
    Type* GetTypeOrNull() override;

    // The function that owns this variable
    Function* Owner;

    // The relative position on the stack that this variable exists
    OperandLocal Local;

    // The type of the variable
    Type* ResultType;

    // Store the name of the variable
    String Name;
  };

  // Information about events sent by a class or struct
  class ZeroShared SendsEvent : public ReflectionObject
  {
  public:
    // Constructor
    SendsEvent();
    SendsEvent(BoundType* owner, StringParam name, BoundType* sentType);

    // ReflectionObject interface
    Library* GetOwningLibrary() override;
    Type* GetTypeOrNull() override;

    // The name of the event that is being sent
    String Name;

    // The type of event being sent
    BoundType* SentType;

    // Who owns this 'sends' declaration
    BoundType* Owner;

    // The extension property on the Events/EventsClass associated with this sends
    Property* EventProperty;
  };
}

#endif
