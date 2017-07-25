///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ZilchTypeCollectorContext
ZilchTypeCollectorContext::ZilchTypeCollectorContext()
{
  mCurrentType = nullptr;
  mCurrentLibrary = nullptr;
  mZilchModule = nullptr;
  mErrors = nullptr;
}

//-------------------------------------------------------------------ZilchTypeCollector

void ZilchTypeCollector::CollectTypes(Zilch::SyntaxTree& syntaxTree, ZilchShaderLibraryRef& libraryRef, Zilch::Module* module, ShaderCompilationErrors* errors, ZilchShaderSettingsRef& settings)
{
  mSettings = settings;
  ZilchTypeCollectorContext context;
  context.mCurrentLibrary = libraryRef;
  context.mZilchModule = module;
  context.mErrors = errors;
  context.mErrors->mErrorTriggered = false;

  Zilch::BranchWalker<ZilchTypeCollector, ZilchTypeCollectorContext> walker;
  walker.Register(&ZilchTypeCollector::WalkClassDeclaration);
  walker.Register(&ZilchTypeCollector::WalkClassConstructor);
  walker.Register(&ZilchTypeCollector::WalkClassFunction);
  walker.Register(&ZilchTypeCollector::WalkClassMemberVariables);

  walker.Walk(this, syntaxTree.Root, &context);
}

void ZilchTypeCollector::WalkClassDeclaration(Zilch::ClassNode*& node, ZilchTypeCollectorContext* context)
{
  // Create the type and store the zilch information
  ShaderType* type = context->mCurrentLibrary->CreateType(node->Name.Token);
  type->mFileName = node->Location.Origin;
  type->mOwningLibrary = context->mCurrentLibrary;
  // Copy user data out of the zilch type
  type->mLocation.mUserData = node->Location.CodeUserData;
  type->mLocation.mZilchSourcePath = node->Location.Origin;
  type->mSourceLocation = node->Location;
  type->mComplexUserData = node->Type->ComplexUserData;

  // Add the mapping of this zilch type to our new shader type
  context->mCurrentLibrary->mZilchToShaderTypes[node->Type] = type;

  // Make class type's errors (only allow structs).
  if(node->CopyMode != Zilch::TypeCopyMode::ValueType)
  {
    String msg = "Cannot declare class types in zilch fragments. Use struct instead.";
    context->mErrors->SendTranslationError(node->Location, msg);
    return;
  }

  // Cache this type on the context
  context->mCurrentType = type;

  ParseAttributes(node->Attributes, node->Type->Attributes, type, context);

  if(type->ContainsAttribute(mSettings->mNameSettings.mIntrinsicAttribute))
    type->SetIntrinsic(true);

  // Don't walk constructors if the type is intrinsic
  if(!type->GetIntrinsic())
  {
    // Collect all of the functions and members
    WalkClassPreConstructor(context, node->PreConstructor);
    context->Walker->Walk(this, node->Constructors, context);
    GenerateDefaultConstructor(node, context);
  }

  context->Walker->Walk(this, node->Functions, context);
  context->Walker->Walk(this, node->Variables, context);
}

void ZilchTypeCollector::WalkClassPreConstructor(ZilchTypeCollectorContext* context, Zilch::Function* preConstructorFn)
{
  ShaderType* currentType = context->mCurrentType;
  ShaderFunction* function = currentType->FindOrCreateFunction(mSettings->mNameSettings.mPreConstructorName, preConstructorFn);
  function->mZilchReturnType = Zilch::Core::GetInstance().VoidType->ToString();
}

void ZilchTypeCollector::WalkClassConstructor(Zilch::ConstructorNode*& node, ZilchTypeCollectorContext* context)
{
  AddFunction(node, mSettings->mNameSettings.mConstructorName, context->mCurrentType->mZilchName, node->Attributes, node->DefinedFunction->Attributes, context);
}

void ZilchTypeCollector::WalkClassFunction(Zilch::FunctionNode*& node, ZilchTypeCollectorContext* context)
{
  AddFunction(node, node->Name.Token, node->Type->Return->ToString(), node->Attributes, node->DefinedFunction->Attributes, context);
}

void ZilchTypeCollector::WalkClassMemberVariables(Zilch::MemberVariableNode*& node, ZilchTypeCollectorContext* context)
{
  ShaderType* currentType = context->mCurrentType;

  Zilch::GetterSetter* getterSetter = Zilch::Type::DynamicCast<Zilch::GetterSetter*>(node->CreatedProperty);
  // If this is a getter/setter then add the get function (and report set functions as errors)
  if(getterSetter != nullptr)
  {
    if(node->Get != nullptr)
    {
      String fnName = node->Get->Name.Token.FindRangeExclusive("[", "]");
      AddFunction(node->Get, fnName, node->Get->Type->Return->ToString(), node->Attributes, node->CreatedProperty->Attributes, context);
    }
    if(node->Set != nullptr)
      context->mErrors->SendTranslationError(node->Set->Location, "Field setters are not supported in shaders yet");
    return;
  }

  ShaderField* field = currentType->FindOrCreateField(node->Name.Token);
  field->mZilchType = node->ResultType->ToString();
  field->mSourceLocation = node->Location;

  // Parse all attributes for the field
  ParseAttributes(node->Attributes, node->CreatedField->Attributes, field, context);

  if(field->ContainsAttribute(mSettings->mNameSettings.mStaticAttribute))
    field->mIsStatic = true;

  //@JoshD: Force certain static types here (Samplers) or leave to translator to modify after the fact?
}

void ZilchTypeCollector::AddFunction(Zilch::GenericFunctionNode* node, StringParam functionName, StringParam returnType, AttributeNodeList& attributeNodes, AttributeArray& attributes, ZilchTypeCollectorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  ShaderType* currentType = context->mCurrentType;
  ShaderFunction* function = currentType->FindOrCreateFunction(functionName, node->DefinedFunction);
  function->mZilchReturnType = returnType;
  function->mSourceLocation = node->Location;

  // Parse all attributes for the function
  ParseAttributes(attributeNodes, attributes, function, context);

  // Mark the function as static if it contained the static attribute
  if(function->ContainsAttribute(nameSettings.mStaticAttribute))
    function->mIsStatic = true;

  // Find extension methods (and by proxy implements)
  String extensionAttrStr = nameSettings.mExtensionAttribute;
  ShaderAttribute* extensionAttribute = function->mAttributes.FindFirstAttribute(extensionAttrStr);
  if(extensionAttribute != nullptr && extensionAttribute->mParameters.Size() == 1)
  {
    Zilch::AttributeParameter& param = extensionAttribute->mParameters[0];
    String extensionType = param.TypeValue->ToString();
    AddExtension(node, function, extensionType, context);
  }

  // Notify the user if they add the implements attribute without also having the extension attribute
  String implementsAttrStr = nameSettings.mImplementAttribute;
  if(function->ContainsAttribute(implementsAttrStr) && extensionAttribute == nullptr)
  {
    String errorMsg = String::Format("It's illegal to use the [%s] attribute without also using the [%s] attribute.", implementsAttrStr.c_str(), extensionAttrStr.c_str());
    context->mErrors->SendTranslationError(node->Location, errorMsg);
  }
}

void ZilchTypeCollector::GenerateDefaultConstructor(Zilch::ClassNode* node, ZilchTypeCollectorContext* context)
{
  ShaderType* currentType = context->mCurrentType;

  // If any constructors have been generated then don't generate the default constructor
  if(currentType->mFunctionNameMultiMap.ContainsKey(mSettings->mNameSettings.mConstructorName))
    return;

  // Otherwise insert the constructor using the special key for default constructors (so the translator can map it later)
  ShaderFunction* function = currentType->FindOrCreateFunction(mSettings->mNameSettings.mConstructorName, ZilchShaderSettings::GetDefaultConstructorKey());
  function->mZilchReturnType = currentType->mZilchName;  
}

void ZilchTypeCollector::AddImplements(Zilch::GenericFunctionNode* node, ShaderFunction* shaderFunction, Zilch::BoundType* boundType, ZilchTypeCollectorContext* context)
{
  // Try to find the bound function on this type with the same delegate type (same parameter names, same types, etc...)
  Zilch::DelegateType* implementsDelegateType = node->DefinedFunction->FunctionType;
  Zilch::Function* replacementFn = nullptr;
  // Find the function from zilch's bound type (check static or instance as appropriate)
  if(shaderFunction->IsStatic())
    replacementFn = boundType->FindFunction(node->Name.Token, implementsDelegateType, Zilch::FindMemberOptions::Static);
  else
    replacementFn = boundType->FindFunction(node->Name.Token, implementsDelegateType, Zilch::FindMemberOptions::None);

  // If we failed to find the function then we need to report an error
  if(replacementFn == nullptr)
  {
    // By default, set the error message as we can't find a function to match to
    StringBuilder msgBuilder;
    msgBuilder << "The signature of a function with the [" << mSettings->mNameSettings.mImplementAttribute << "] attribute must match an existing function.\n";

    // For added error reporting, try to find all possible overloads of this function (check static or instance as appropriate)
    const Zilch::FunctionArray* functions = nullptr;
    if(shaderFunction->IsStatic())
      functions = boundType->GetOverloadedStaticFunctions(node->Name.Token);
    else
      functions = boundType->GetOverloadedInstanceFunctions(node->Name.Token);
    // If we find overloads of the same function name then list all possible overloads as candidates
    if(functions != nullptr)
    {
      msgBuilder.Append("Possible overloads are:\n");
      for(size_t i = 0; i < functions->Size(); ++i)
      {
        Zilch::Function* fn = (*functions)[i];
        msgBuilder << "\t" << fn->ToString() << "\n";
      }
    }
    else
    {
      msgBuilder << "Type '" << boundType->ToString() << "' does not contain a function named '" << shaderFunction->mZilchName << "'.";
    }

    context->mErrors->SendTranslationError(node->Location, msgBuilder.ToString());
    return;
  }
  
  // Otherwise, store on this library that we implement the given zilch function with the current shader function
  context->mCurrentLibrary->mImplements[replacementFn] = shaderFunction;
}

void ZilchTypeCollector::AddExtension(Zilch::GenericFunctionNode* node, ShaderFunction* shaderFunction, StringParam extensionType, ZilchTypeCollectorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  // Find the zilch type that we're adding an extension to
  Zilch::BoundType* boundType = context->mZilchModule->FindType(extensionType);
  if(boundType == nullptr)
  {
    String msg = String::Format("Cannot add [%s] attribute on type '%s'", nameSettings.mExtensionAttribute.c_str(), extensionType.c_str());
    context->mErrors->SendTranslationError(node->Location, msg);
    return;
  }

  // If we also have the implements attribute then setup the implements as well
  if(shaderFunction->ContainsAttribute(nameSettings.mImplementAttribute))
  {
    AddImplements(node, shaderFunction, boundType, context);
  }

  context->mCurrentLibrary->mExtensions[node->DefinedFunction] = shaderFunction;
}

void ZilchTypeCollector::ValidateAttribute(ShaderType* shaderType, Zilch::AttributeNode* attributeNode, ShaderAttribute& shaderAttribute, ZilchTypeCollectorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  if(shaderAttribute.mAttributeName == nameSettings.mPixelAttributeName)
    shaderType->mFragmentType = FragmentType::Pixel;
  else if(shaderAttribute.mAttributeName == nameSettings.mVertexAttributeName)
    shaderType->mFragmentType = FragmentType::Vertex;
  else if(shaderAttribute.mAttributeName == nameSettings.mGeometryAttributeName)
  {
    shaderType->mFragmentType = FragmentType::Geometry;
    
    bool maxVerticesFound = false;
    // Validate that the Geometry attribute contains the max vertices parameter and nothing else
    for(size_t paramIndex = 0; paramIndex < shaderAttribute.mParameters.Size(); ++paramIndex)
    {
      Zilch::CodeLocation& paramLocation = attributeNode->AttributeCall->Arguments[paramIndex]->Location;

      Zilch::AttributeParameter& attributeParam = shaderAttribute.mParameters[paramIndex];
      if(attributeParam.Name == nameSettings.mMaxVerticesParamName)
      {
        if(ValidateParamType(attributeParam, Zilch::ConstantType::Integer, paramLocation, context->mErrors))
          shaderType->GetTypeData()->mCount = (int)attributeParam.IntegerValue;
        maxVerticesFound = true;
      }
      else
      {
        String msg = String::Format("Unknown attribute parameter '%s'", attributeParam.Name.c_str());
        context->mErrors->SendTranslationError(paramLocation, msg);
      }
    }

    if(maxVerticesFound == false)
      context->mErrors->SendTranslationError(attributeNode->Location, "Attribute parameter 'maxVertices' is required for geometry shaders");
  }
  else if(shaderAttribute.mAttributeName == nameSettings.mIntrinsicAttribute)
  {
    shaderType->SetIntrinsic(true);

    // Gather all of the parameters of this attribute
    for(size_t paramIndex = 0; paramIndex < shaderAttribute.mParameters.Size(); ++paramIndex)
    {
      Zilch::ExpressionNode* paramExpressionNode = attributeNode->AttributeCall->Arguments[paramIndex];
      Zilch::CodeLocation& paramLocation = paramExpressionNode->Location;

      Zilch::AttributeParameter& attributeParam = shaderAttribute.mParameters[paramIndex];
      if(attributeParam.Name == nameSettings.mLanguageParamName)
        ValidateParamType(attributeParam, Zilch::ConstantType::String, paramLocation, context->mErrors);
      else if(attributeParam.Name == nameSettings.mTypeNameParamName)
        ValidateParamType(attributeParam, Zilch::ConstantType::String, paramLocation, context->mErrors);
      else if(attributeParam.Name == nameSettings.mStorageQualifierParamName)
        ValidateParamType(attributeParam, Zilch::ConstantType::String, paramLocation, context->mErrors);
      else if(attributeParam.Name == nameSettings.mRefTypeParamName)
        ValidateParamType(attributeParam, Zilch::ConstantType::String, paramLocation, context->mErrors);
      else if(attributeParam.Name == nameSettings.mPropertyTypeParamName)
        ValidateParamType(attributeParam, Zilch::ConstantType::String, paramLocation, context->mErrors);
      else if(attributeParam.Name == nameSettings.mNonCopyableParamName)
        ValidateParamType(attributeParam, Zilch::ConstantType::Boolean, paramLocation, context->mErrors);
      else if(attributeParam.Name == nameSettings.mForcedStaticParamName)
        ValidateParamType(attributeParam, Zilch::ConstantType::Boolean, paramLocation, context->mErrors);
      else
      {
        String msg = String::Format("Unknown attribute parameter '%s'", attributeParam.Name.c_str());
        context->mErrors->SendTranslationError(paramLocation, msg);
      }
    }
  }

  // Check if this attribute isn't allowed
  if(!nameSettings.mAllowedClassAttributes.Contains(shaderAttribute.mAttributeName))
  {
    Error("Unknown attribute %s", shaderAttribute.mAttributeName.c_str());
  }
}

void ZilchTypeCollector::ValidateAttribute(ShaderField* shaderField, Zilch::AttributeNode* attributeNode, ShaderAttribute& shaderAttribute, ZilchTypeCollectorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  if(shaderAttribute.mAttributeName == nameSettings.mSharedInputAttributeName)
  {
    for(size_t paramIndex = 0; paramIndex < shaderAttribute.mParameters.Size(); ++paramIndex)
    {
      Zilch::AttributeParameter& attributeParam = shaderAttribute.mParameters[paramIndex];
      Zilch::ExpressionNode* paramExpressionNode = attributeNode->AttributeCall->Arguments[paramIndex];
      Zilch::CodeLocation& paramLocation = paramExpressionNode->Location;

      if(attributeParam.Name == nameSettings.mSharedInputNameParamName)
        ValidateParamType(attributeParam, Zilch::ConstantType::String, paramLocation, context->mErrors);
      else
      {
        String msg = String::Format("Unknown attribute parameter '%s'", attributeParam.Name.c_str());
        context->mErrors->SendTranslationError(paramLocation, msg);
      }
    }
    return;
  }

  // Check if this attribute isn't allowed
  if(!nameSettings.mAllowedFieldAttributes.Contains(shaderAttribute.mAttributeName))
  {
    String msg = String::Format("Unknown attribute %s", shaderAttribute.mAttributeName.c_str());
    context->mErrors->SendTranslationError(attributeNode->Location, msg);
  }
}

void ZilchTypeCollector::ValidateAttribute(ShaderFunction* shaderFunction, Zilch::AttributeNode* attributeNode, ShaderAttribute& shaderAttribute, ZilchTypeCollectorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  if(shaderAttribute.mAttributeName == nameSettings.mStaticAttribute)
    shaderFunction->mIsStatic = true;

  // Check if this attribute isn't allowed
  if(!nameSettings.mAllowedFunctionAttributes.Contains(shaderAttribute.mAttributeName))
  {
    Error("Unknown attribute %s", shaderAttribute.mAttributeName.c_str());
  }
}

void ZilchTypeCollector::ValidateBuiltIns(ShaderField* shaderField, ZilchTypeCollectorContext* context)
{
  // This function checks if a field has property marked as built-in that isn't one. If the property only
  // contains the built-in attribute and the name/type doesn't match a provided built-in then this will throw an error.
  if(shaderField->mAttributes.Size() != 1)
    return;

  NameSettings& nameSettings = mSettings->mNameSettings;
  ShaderDefinitionSettings& defSettings = mSettings->mShaderDefinitionSettings;

  ShaderAttribute* builtinAttribute = shaderField->mAttributes.FindFirstAttribute(nameSettings.mBuiltinInputAttributeName);
  if(builtinAttribute == nullptr)
    return;

  ShaderFieldKey fieldKey(shaderField);
  bool fieldIsBuiltin = defSettings.mBuiltIns.ContainsKey(fieldKey);
  if(!fieldIsBuiltin)
  {
    String msg = String::Format("Property '%s : %s' is not a valid Builtin input.",
      shaderField->mZilchName.c_str(), shaderField->mZilchType.c_str());
    context->mErrors->SendTranslationError(shaderField->mSourceLocation, msg);
  }
}

void ZilchTypeCollector::ValidateAllAttributes(ShaderType* shaderType, ZilchTypeCollectorContext* context)
{
  // Fragment types can't have [PixelIntrinsic] or others, [Intrinsic], [Extension], [Static], etc...
  NameSettings& nameSettings = mSettings->mNameSettings;
  HashMap<String, int> attributeCounts;
  CountAttributes(attributeCounts, shaderType->mAttributes);

  int vertexCount = attributeCounts.FindValue(nameSettings.mVertexAttributeName, 0);
  int geometryCount = attributeCounts.FindValue(nameSettings.mGeometryAttributeName, 0);
  int pixelCount = attributeCounts.FindValue(nameSettings.mPixelAttributeName, 0);
  // Verify that only one fragment type was marked
  if(vertexCount + geometryCount + pixelCount > 1)
  {
    String msg = String::Format("Only one copy of %s or %s or %s is allowed per type",
      nameSettings.mVertexAttributeName.c_str(), nameSettings.mGeometryAttributeName.c_str(), nameSettings.mPixelAttributeName.c_str());
    context->mErrors->SendTranslationError(context->mCurrentType->mSourceLocation, msg);
  }

  // Check for attributes not allowed on fragment types
  if(shaderType->mFragmentType != FragmentType::None)
  {
    String fragmentTypeName = BuildString(FragmentType::Names[shaderType->mFragmentType], " fragments");
    ValidateAttributeCount(attributeCounts, nameSettings.mVertexIntrinsicAttributeName, 0, fragmentTypeName, context);
    ValidateAttributeCount(attributeCounts, nameSettings.mGeometryIntrinsicAttributeName, 0, fragmentTypeName, context);
    ValidateAttributeCount(attributeCounts, nameSettings.mPixelIntrinsicAttributeName, 0, fragmentTypeName, context);

    ValidateAttributeCount(attributeCounts, nameSettings.mExtensionAttribute, 0, fragmentTypeName, context);
    ValidateAttributeCount(attributeCounts, nameSettings.mStaticAttribute, 0, fragmentTypeName, context);
    ValidateAttributeCount(attributeCounts, nameSettings.mIntrinsicAttribute, 0, fragmentTypeName, context);
  }
}

void ZilchTypeCollector::ValidateAllAttributes(ShaderField* shaderField, ZilchTypeCollectorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  HashMap<String, int> attributeCounts;
  CountAttributes(attributeCounts, shaderField->mAttributes);

  // Make sure only 1 [Shared] attribute exists
  ValidateAttributeMaxCount(attributeCounts, nameSettings.mSharedInputAttributeName, 1, "fields", context);

  ValidateBuiltIns(shaderField, context);
} 

void ZilchTypeCollector::ValidateAllAttributes(ShaderFunction* shaderFunction, ZilchTypeCollectorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  HashMap<String, int> attributeCounts;
  CountAttributes(attributeCounts, shaderFunction->mAttributes);

  // Make sure only 1 [Extension] attribute exists
  ValidateAttributeMaxCount(attributeCounts, nameSettings.mExtensionAttribute, 1, "functions", context);  
}

bool ZilchTypeCollector::ValidateParamType(Zilch::AttributeParameter& attributeParam, Zilch::ConstantType::Enum expectedType, Zilch::CodeLocation& location, ShaderCompilationErrors* errors)
{
  // Make sure the parameter type is correct
  if(attributeParam.Type != expectedType)
  {
    String msg = String::Format("Parameter '%s' must be of type %s", attributeParam.Name.c_str(), Zilch::ConstantType::Names[expectedType]);
    errors->SendTranslationError(location, msg);
    return false;
  }
  return true;
}

void ZilchTypeCollector::CountAttributes(HashMap<String, int>& attributeCounts, ShaderAttributeList& attributes)
{
  // Count how many time any attribute name exists
  ShaderAttributeList::range range = attributes.All();
  for(; !range.Empty(); range.PopFront())
  {
    ShaderAttribute& shaderAttribute = range.Front();
    ++attributeCounts[shaderAttribute.mAttributeName];
  }
}

void ZilchTypeCollector::ValidateAttributeCount(HashMap<String, int>& attributeCounts, StringParam attributeName, int expectedCount, StringParam typeName, ZilchTypeCollectorContext* context)
{
  // Verify that an attribute happens a certain number of times
  if(attributeCounts.FindValue(attributeName, 0) != expectedCount)
  {
    String msg = String::Format("Attribute '%s' is not allowed on %s", attributeName.c_str(), typeName.c_str());
    context->mErrors->SendTranslationError(context->mCurrentType->mSourceLocation, msg);
  }
}

void ZilchTypeCollector::ValidateAttributeMaxCount(HashMap<String, int>& attributeCounts, StringParam attributeName, int maxAllowed, StringParam typeName, ZilchTypeCollectorContext* context)
{
  // Verify that an attribute happens at most "maxAllowed" times
  if(attributeCounts.FindValue(attributeName, 0) > maxAllowed)
  {
    String msg;
    if(maxAllowed == 1)
      msg = String::Format("Only 1 attribute of name '%s' is allowed on %s", attributeName.c_str(), typeName.c_str());
    else
      msg = String::Format("Only %d attributes of name '%s' is allowed on %s", maxAllowed, attributeName.c_str(), typeName.c_str());

    context->mErrors->SendTranslationError(context->mCurrentType->mSourceLocation, msg);
  }
}

template <typename Type>
void ZilchTypeCollector::ParseAttributes(AttributeNodeList& attributeNodes, AttributeArray& zilchAttributes, Type* type, ZilchTypeCollectorContext* context)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  
  // Parse the attributes and their parameters
  for(size_t attributeIndex = 0; attributeIndex < zilchAttributes.Size(); ++attributeIndex)
  {
    Zilch::Attribute& attribute = zilchAttributes[attributeIndex];
    ShaderAttribute* shaderAttribute = type->mAttributes.AddAttribute(attribute.Name, attributeNodes[attributeIndex]);

    for(size_t paramIndex = 0; paramIndex < attribute.Parameters.Size(); ++paramIndex)
    {
      Zilch::AttributeParameter& param = attribute.Parameters[paramIndex];
      shaderAttribute->mParameters.PushBack(param);
    }
  }

  // If the input attribute exists then add all of the implied sub-attribute to the field.
  ShaderAttributeList::NamedRange inputRange = type->mAttributes.FindAttributes(nameSettings.mInputAttributeName);
  if(!inputRange.Empty())
  {
    ShaderAttribute* inputAttribute = inputRange.Front();
    for(size_t i = 0; i < nameSettings.mInputSubAttributes.Size(); ++i)
      type->mAttributes.AddAttribute(nameSettings.mInputSubAttributes[i], nullptr);
  }

  // Validate the attributes all of the attributes one at a time (should set flags and only check for that attribute's values/types/etc...)
  for(size_t attributeIndex = 0; attributeIndex < zilchAttributes.Size(); ++attributeIndex)
  {
    Zilch::Attribute& attribute = zilchAttributes[attributeIndex];
    ShaderAttribute* shaderAttribute = type->mAttributes.GetAtIndex(attributeIndex);
    ValidateAttribute(type, attributeNodes[attributeIndex], *shaderAttribute, context);
  }
  // Validate all attributes together (typically checking for attributes that can't exist together, too many occurrences, etc...)
  ValidateAllAttributes(type, context);
}

}//namespace Zero
