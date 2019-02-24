// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// Attribute Status
void AttributeStatus::SetFailed(CodeLocation& location, StringParam message)
{
  mLocation = location;
  Status::SetFailed(message);
}

// Attribute Extensions
AttributeExtensions::~AttributeExtensions()
{
  DeleteObjectsIn(mClassExtensions.Values());
  DeleteObjectsIn(mPropertyExtensions.Values());
  DeleteObjectsIn(mFunctionExtensions.Values());
}

void AttributeExtensions::ProcessType(AttributeStatus& status, BoundType* type, bool ignoreUnkownAttributes)
{
  // Class attributes
  ProcessObject(status, type, mClassExtensions, ignoreUnkownAttributes);
  if (status.Failed())
    return;

  // Property attributes
  forRange (Property* property, type->GetProperties(Members::InstanceStatic))
  {
    ProcessObject(status, property, mPropertyExtensions, ignoreUnkownAttributes);
    if (status.Failed())
      return;
  }

  // Function attributes
  forRange (Function* function, type->GetFunctions(Members::InstanceStatic))
  {
    ProcessObject(status, function, mFunctionExtensions, ignoreUnkownAttributes);
    if (status.Failed())
      return;
  }
}

bool AttributeExtensions::IsValidClassAttribute(StringParam name)
{
  return mClassExtensions.ContainsKey(name);
}

bool AttributeExtensions::IsValidPropertyAttribute(StringParam name)
{
  return mPropertyExtensions.ContainsKey(name);
}

bool AttributeExtensions::IsValidFunctionAttribute(StringParam name)
{
  return mFunctionExtensions.ContainsKey(name);
}

String GetDescriptorFromObject(ReflectionObject* object)
{
  if (BoundType* type = Type::DynamicCast<BoundType*>(object))
    return "class";
  else if (Type::DynamicCast<Property*>(object))
    return "property";
  else if (Type::DynamicCast<Function*>(object))
    return "function";

  Error("Invalid type");
  return String();
}

void AttributeExtensions::ProcessObject(AttributeStatus& status,
                                        ReflectionObject* object,
                                        ExtensionMap& extensionMap,
                                        bool ignoreUnkownAttributes)
{
  // Used to keep track of duplicate attributes
  HashSet<String> processedAttributes;

  forRange (Attribute& attribute, object->Attributes.All())
  {
    AttributeExtension* extension = extensionMap.FindValue(attribute.Name, nullptr);

    // The attribute name wasn't registered for properties
    if (extension == nullptr)
    {
      if (ignoreUnkownAttributes)
        continue;

      String message;
      if (mClassExtensions.ContainsKey(attribute.Name) || mPropertyExtensions.ContainsKey(attribute.Name) ||
          mFunctionExtensions.ContainsKey(attribute.Name))
      {
        String descriptor = GetDescriptorFromObject(object);
        message = String::Format("Attribute '%s' is not valid on a %s", attribute.Name.c_str(), descriptor.c_str());
      }
      else
      {
        message = String::Format("Attribute '%s' is not recognized", attribute.Name.c_str());
      }

      status.SetFailed(object->Location, message);
      return;
    }

    // Check if this attribute has already been processed
    if (processedAttributes.Contains(attribute.Name) && extension->mAllowMultiple == false)
    {
      String message = String::Format("You cannot have multiple '%s' attributes", attribute.Name.c_str());
      status.SetFailed(object->Location, message);
      return;
    }

    extension->Validate(status, object, attribute);

    // The attribute name is correct, but something failed when trying to add it
    if (status.Failed())
    {
      status.mLocation = object->Location;
      return;
    }

    processedAttributes.Insert(attribute.Name);
  }
}

AttributeExtension* AttributeExtensions::RegisterClassExtension(AttributeExtension* extension)
{
  return RegisterExtension(extension, mClassExtensions);
}

AttributeExtension* AttributeExtensions::RegisterPropertyExtension(AttributeExtension* extension)
{
  return RegisterExtension(extension, mPropertyExtensions);
}

AttributeExtension* AttributeExtensions::RegisterFunctionExtension(AttributeExtension* extension)
{
  return RegisterExtension(extension, mFunctionExtensions);
}

AttributeExtension* AttributeExtensions::RegisterExtension(AttributeExtension* extension, ExtensionMap& extensionMap)
{
  // Confirm that all optional attribute parameters are at the end
  if (BoundType* componentType = extension->GetMetaComponentType())
  {
    bool foundOptional = false;
    forRange (Property* property, componentType->GetProperties())
    {
      if (property->HasAttribute(PropertyAttributes::cOptional))
      {
        foundOptional = true;
      }
      else
      {
        String message = String::Format("All optional attribute parameters on '%s' must be bound "
                                        "after all required properties",
                                        componentType->Name.c_str());
        ErrorIf(foundOptional, message.c_str());
      }
    }
  }

  extensionMap.Insert(extension->mAttributeName, extension);
  return extension;
}

// Attribute Extension
AttributeExtension::AttributeExtension(StringParam name) :
    mAttributeName(name),
    mMustBeType(nullptr),
    mAllowStatic(false),
    mAllowMultiple(false)
{
}

void AttributeExtension::Validate(Status& status, ReflectionObject* object, Attribute& attribute)
{
  // First validate the type
  ValidateType(status, object);
  if (status.Failed())
    return;

  ValidateStatic(status, object);
  if (status.Failed())
    return;

  HandleOf<MetaAttribute> metaComponent = AllocateMetaComponent(object);
  if (metaComponent.IsNotNull())
  {
    ValidateParameters(status, metaComponent, attribute);

    if (status.Succeeded())
    {
      MetaAttribute* attribute = metaComponent;
      attribute->PostProcess(status, object);
    }
  }
  else
  {
    // If there isn't a meta component associated with this attribute, we cannot
    // accept any parameters
    if (attribute.Parameters.Empty() == false)
    {
      String message = String::Format("Attribute '%s' shouldn't have any parameters", attribute.Name.c_str());
      status.SetFailed(message);
    }
  }
}

void AttributeExtension::ValidateType(Status& status, ReflectionObject* object)
{
  // If the 'must be type' is null, this attribute can go on any class or
  // property type
  if (mMustBeType == nullptr)
    return;

  // This will either be the class type or property type. It should never be
  // null because mMustBeType should never be set for functions
  BoundType* boundType = Type::DynamicCast<BoundType*>(object->GetTypeOrNull());

  ReturnIf(boundType == nullptr, , "Type could not be validated");

  // Make sure they have the appropriate type, checking all inherited types
  bool validType = false;
  Type* baseType = boundType;
  while (!validType && baseType)
  {
    validType |= (baseType == mMustBeType);
    baseType = Type::GetBaseType(baseType);
  }

  if (!validType)
  {
    String message;
    if (Type::DynamicCast<Property*>(object))
    {
      message = String::Format("Attribute '%s' can only exist on a property of type '%s'",
                               mAttributeName.c_str(),
                               mMustBeType->Name.c_str());
    }
    else
    {
      message =
          String::Format("Attribute '%s' can only exist on a '%s'", mAttributeName.c_str(), mMustBeType->Name.c_str());
    }
    status.SetFailed(message);
  }
}

void AttributeExtension::ValidateStatic(Status& status, ReflectionObject* object)
{
  if (Member* member = Type::DynamicCast<Member*>(object))
  {
    if (member->IsStatic && mAllowStatic == false)
    {
      String message = String::Format("Attribute '%s' cannot be on static members", mAttributeName.c_str());
      status.SetFailed(message);
    }
  }
}

Any GetValueFromParameter(AttributeParameter* parameter)
{
  ConstantType::Enum parameterType = parameter->Type;
  switch (parameterType)
  {
  case ConstantType::String:
    return parameter->StringValue;
  case ConstantType::Integer:
    return (int)parameter->IntegerValue;
  case ConstantType::DoubleInteger:
    return parameter->IntegerValue;
  case ConstantType::Real:
    return (float)parameter->RealValue;
  case ConstantType::DoubleReal:
    return parameter->RealValue;
  case ConstantType::Boolean:
    return parameter->BooleanValue;
  case ConstantType::Type:
    return Any(parameter->TypeValue);
  case ConstantType::Null:
    return Any(nullptr);
  }

  Error("Invalid constant");
  return Any();
}

Any Cast(AnyParam value, BoundType* expectedType)
{
  Type* valueType = value.StoredType;
  if (valueType == expectedType)
    return value;

  if (expectedType == ZilchTypeId(float) && valueType == ZilchTypeId(int))
    return (float)value.Get<int>();

  return Any();
}

// The first letter in property names must be capitalized, however attribute
// parameters must be lower case. Convert to lower case before querying the
// attribute for parameters
String PropertyToAttributeName(String name)
{
  return BuildString(String(ToLower(name.Front())), name.SubString(name.Begin() + 1, name.End()));
}

void AppendMissingParametersError(String& currentMessage, BoundType* componentType, StringParam attributeName)
{
  StringBuilder message;
  message.Append(currentMessage);
  message.Append("\n");
  message.Append("Expected parameters: ");

  int propertyCount = 0;
  forRange (Property* expectedProperty, componentType->GetProperties())
    ++propertyCount;

  message.Append("(");
  // Example: (min, max, [increment])
  int index = 0;
  bool hitOptional = false;
  forRange (Property* expectedProperty, componentType->GetProperties())
  {
    // Open optional parameters only once (all optional parameters are right
    // next to each other)
    if (expectedProperty->HasAttribute(PropertyAttributes::cOptional))
    {
      message.Append("[");
      hitOptional = true;
    }

    // Parameter name
    String name = PropertyToAttributeName(expectedProperty->Name);
    message.AppendFormat("%s", name.c_str());

    // Add parameter type name
    if (BoundType* propertyType = Type::GetBoundType(expectedProperty->PropertyType))
      message.AppendFormat(": %s", propertyType->Name.c_str());

    // Add a comma between parameters
    if (index < propertyCount - 1)
      message.Append(", ");

    ++index;
  }

  // Close optional parameters
  if (hitOptional)
    message.Append("]");
  message.Append(")");

  currentMessage = message.ToString();
}

void AttributeExtension::ValidateParameters(Status& status, HandleParam component, Attribute& attribute)
{
  BoundType* componentType = component.StoredType;

  uint parameterCount = attribute.Parameters.Size();

  // Attribute parameters must either all have names, or have no names
  // We need to check how many are named so we know whether or not they're all
  // named
  uint namedParametersCount = 0;
  forRange (AttributeParameter& parameter, attribute.Parameters.All())
  {
    if (!parameter.Name.Empty())
      ++namedParametersCount;
  }

  bool namedParameters = (namedParametersCount != 0);
  if (namedParameters && namedParametersCount != parameterCount)
  {
    status.SetFailed("Attribute parameters must either be all named, or all unnamed");
    return;
  }

  uint currentParameter = 0;
  forRange (Property* property, componentType->GetProperties())
  {
    String name = PropertyToAttributeName(property->Name);

    bool optional = (property->HasAttribute(PropertyAttributes::cOptional));

    BoundType* propertyType = Type::GetBoundType(property->PropertyType);

    AttributeParameter* parameter = nullptr;

    // If we don't have any named parameters, look them up by index
    if (namedParameters == false)
    {
      if (currentParameter >= parameterCount)
      {
        // All non-optional parameters must be specified
        if (!optional)
        {
          String message = String::Format("Attribute '%s' is missing required parameters.", attribute.Name.c_str());
          AppendMissingParametersError(message, componentType, attribute.Name);
          status.SetFailed(message);
        }
        return;
      }

      parameter = &attribute.Parameters[currentParameter];
    }
    // Otherwise, look up the parameter by name
    else
    {
      parameter = attribute.HasAttributeParameter(name);
    }

    if (parameter)
    {
      Any parameterValue = GetValueFromParameter(parameter);
      parameterValue = Cast(parameterValue, propertyType);

      // Set our property if it's the correct type
      if (parameterValue.IsNotNull())
      {
        property->SetValue(component, parameterValue);
        ++currentParameter;
      }
      else
      {
        String message = String::Format("Attribute '%s' expected the parameter '%s' to be a '%s'",
                                        attribute.Name.c_str(),
                                        name.c_str(),
                                        propertyType->Name.c_str());

        // If the parameter was a bound type, append to the message
        if (BoundType* parameterType = Type::GetBoundType(parameterValue.StoredType))
          message = String::Format("%s, but instead got a '%s'.", message.c_str(), parameterType->Name.c_str());

        AppendMissingParametersError(message, componentType, attribute.Name);

        status.SetFailed(message);
        return;
      }
    }
    // If it's a required property, it should be an error
    else if (property->HasAttribute(PropertyAttributes::cOptional) == nullptr)
    {
      String message = String::Format("Attribute '%s' must have a parameter called '%s' of type '%s'.",
                                      attribute.Name.c_str(),
                                      name.c_str(),
                                      propertyType->Name.c_str());
      AppendMissingParametersError(message, componentType, attribute.Name);
      status.SetFailed(message);
      return;
    }
  }

  // If they specified a parameter we didn't process, they probably misspelled
  // an optional parameter
  if (currentParameter != parameterCount)
  {
    String message;
    if (namedParameters)
    {
      AttributeParameter& parameter = attribute.Parameters[currentParameter];
      message = String::Format(
          "Attribute '%s' has an invalid parameter '%s'.", attribute.Name.c_str(), parameter.Name.c_str());
    }
    else
    {
      message = String::Format("Attribute '%s' has too many parameters.", attribute.Name.c_str());
    }

    AppendMissingParametersError(message, componentType, attribute.Name);
    status.SetFailed(message);
  }
}

AttributeExtension* AttributeExtension::MustBeType(BoundType* type)
{
  mMustBeType = type;
  return this;
}

AttributeExtension* AttributeExtension::AllowStatic(bool state)
{
  mAllowStatic = state;
  return this;
}

AttributeExtension* AttributeExtension::AllowMultiple(bool state)
{
  mAllowMultiple = state;
  return this;
}

} // namespace Zero
