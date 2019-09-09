// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class AttributeExtension;

// Attribute Status
class AttributeStatus : public Status
{
public:
  void SetFailed(CodeLocation& location, StringParam message);

  CodeLocation mLocation;
};

// Attribute Extensions
/// Processes all supported attributes and attribute parameters.
class AttributeExtensions : public ExplicitSingleton<AttributeExtensions>
{
public:
  ~AttributeExtensions();

  /// Validates all attributes on the given type. If any are invalid, it will
  /// raise an error on the given project. 'ignoreUnkownAttributes' was added
  /// for processing attributes in fragment libraries. Fragments have a lot of
  /// extra parameters that aren't registered here that we would otherwise throw
  /// an unknown attribute error. If the systems were combined, we could remove
  /// this extra flag.
  void ProcessType(AttributeStatus& status, BoundType* type, bool ignoreUnkownAttributes = false);

  bool IsValidClassAttribute(StringParam name);
  bool IsValidPropertyAttribute(StringParam name);
  bool IsValidFunctionAttribute(StringParam name);

  //----- Internals
  typedef HashMap<String, AttributeExtension*> ExtensionMap;

  /// Validates all attributes on the given object.
  void ProcessObject(AttributeStatus& status,
                     ReflectionObject* object,
                     ExtensionMap& extensionMap,
                     bool ignoreUnkownAttributes);

  AttributeExtension* RegisterClassExtension(AttributeExtension* extension);
  AttributeExtension* RegisterPropertyExtension(AttributeExtension* extension);
  AttributeExtension* RegisterFunctionExtension(AttributeExtension* extension);
  AttributeExtension* RegisterExtension(AttributeExtension* extension, ExtensionMap& extensionMap);

  /// All registered attributes.
  ExtensionMap mClassExtensions;
  ExtensionMap mPropertyExtensions;
  ExtensionMap mFunctionExtensions;
};

// Attribute Extension
class AttributeExtension
{
public:
  AttributeExtension(StringParam name);

  /// Used to specify restricted class or property types that this attribute can
  /// be on.
  AttributeExtension* MustBeType(BoundType* type);
  /// Disallows attributes to be on static members. Defaults to false.
  AttributeExtension* AllowStatic(bool state);
  /// Allows multiple of the same attribute on an object. Defaults to false.
  AttributeExtension* AllowMultiple(bool state);

  /// Confirms that the attribute is complete, and it is on a valid object.
  void Validate(Status& status, ReflectionObject* object, Attribute& attribute);

  //----- Internals
  void ValidateType(Status& status, ReflectionObject* object);
  void ValidateStatic(Status& status, ReflectionObject* object);
  void ValidateParameters(Status& status, HandleParam component, Attribute& attribute);

  virtual BoundType* GetMetaComponentType()
  {
    return nullptr;
  }
  virtual HandleOf<MetaAttribute> AllocateMetaComponent(ReflectionObject* object)
  {
    return nullptr;
  }

  String mAttributeName;
  BoundType* mMustBeType;
  bool mAllowStatic;
  bool mAllowMultiple;
};

// Attribute Extension Type
template <typename T>
class AttributeExtensionType : public AttributeExtension
{
public:
  using AttributeExtension::AttributeExtension;
  BoundType* GetMetaComponentType() override
  {
    return ZilchTypeId(T);
  }
  HandleOf<MetaAttribute> AllocateMetaComponent(ReflectionObject* object) override
  {
    T* component = new T();
    object->Add(component);
    return component;
  }
};

#define RegisterClassAttribute(name)                                                                                   \
  AttributeExtensions::GetInstance()->RegisterClassExtension(new AttributeExtension(name))
#define RegisterClassAttributeType(name, type)                                                                         \
  AttributeExtensions::GetInstance()->RegisterClassExtension(new AttributeExtensionType<type>(name))
#define RegisterPropertyAttribute(name)                                                                                \
  AttributeExtensions::GetInstance()->RegisterPropertyExtension(new AttributeExtension(name))
#define RegisterPropertyAttributeType(name, type)                                                                      \
  AttributeExtensions::GetInstance()->RegisterPropertyExtension(new AttributeExtensionType<type>(name))
#define RegisterFunctionAttribute(name)                                                                                \
  AttributeExtensions::GetInstance()->RegisterFunctionExtension(new AttributeExtension(name))
#define RegisterFunctionAttributeType(name, type)                                                                      \
  AttributeExtensions::GetInstance()->RegisterFunctionExtension(new AttributeExtensionType<type>(name))
#define TypeMustBe(type) MustBeType(ZilchTypeId(type))

} // namespace Zero
