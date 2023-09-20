// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieSpirVFrontEndContext::RaverieSpirVFrontEndContext()
{
  mCurrentFunction = nullptr;
  mContinueTarget = nullptr;
  mBreakTarget = nullptr;
}

BasicBlock* RaverieSpirVFrontEndContext::GetCurrentBlock()
{
  return mCurrentBlock;
}

void RaverieSpirVFrontEndContext::PushMergePoints(BasicBlock* continuePoint, BasicBlock* breakPoint)
{
  mMergePointStack.PushBack(Pair<BasicBlock*, BasicBlock*>(continuePoint, breakPoint));
  mContinueTarget = continuePoint;
  mBreakTarget = breakPoint;
}

void RaverieSpirVFrontEndContext::PopMergeTargets()
{
  mMergePointStack.PopBack();
  if (mMergePointStack.Empty())
  {
    mContinueTarget = nullptr;
    mBreakTarget = nullptr;
    return;
  }

  Pair<BasicBlock*, BasicBlock*>& prevMergePoints = mMergePointStack.Back();
  mContinueTarget = prevMergePoints.first;
  mBreakTarget = prevMergePoints.second;
}

void RaverieSpirVFrontEndContext::PushIRStack(IRaverieShaderIR* ir)
{
  mResultStack.PushBack(ir);
}

IRaverieShaderIR* RaverieSpirVFrontEndContext::PopIRStack()
{
  IRaverieShaderIR* ir = mResultStack.Back();
  mResultStack.PopBack();
  return ir;
}

RaverieSpirVFrontEnd::~RaverieSpirVFrontEnd()
{
}

void RaverieSpirVFrontEnd::SetSettings(RaverieShaderSpirVSettingsRef& settings)
{
  mSettings = settings;
  if (!mSettings->IsFinalized())
    mSettings->Finalize();
}

void RaverieSpirVFrontEnd::Setup()
{
  mWalker.RegisterNonLeafBase(&RaverieSpirVFrontEnd::ExtractRaverieAsComments);

  mWalker.Register(&RaverieSpirVFrontEnd::WalkClassNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkClassVariables);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkClassConstructor);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkClassFunction);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkFunctionCallNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkLocalVariable);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkStaticTypeOrCreationCallNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkExpressionInitializerNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkUnaryOperationNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkBinaryOperationNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkCastNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkValueNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkLocalRef);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkMemberAccessNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkIfRootNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkIfNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkContinueNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkBreakNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkReturnNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkWhileNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkDoWhileNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkForNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkForEachNode);
  mWalker.Register(&RaverieSpirVFrontEnd::WalkLoopNode);

  mRaverieCommentParser.Setup();
}

RaverieShaderIRType* RaverieSpirVFrontEnd::MakeTypeInternal(RaverieShaderIRLibrary* shaderLibrary,
                                                        ShaderIRTypeBaseType::Enum baseType,
                                                        StringParam typeName,
                                                        Raverie::BoundType* raverieType,
                                                        spv::StorageClass storageClass)
{
  RaverieShaderIRType* shaderType = shaderLibrary->FindType(typeName);
  ErrorIf(shaderType != nullptr, "Type '%s' already exists.", typeName.c_str());

  shaderType = new RaverieShaderIRType();
  shaderType->mBaseType = baseType;
  shaderType->mRaverieType = raverieType;
  shaderType->mName = typeName;
  shaderType->mShaderLibrary = shaderLibrary;
  shaderType->mStorageClass = storageClass;
  shaderLibrary->AddType(shaderType->mName, shaderType);
  // Don't set the debug name here, that's up to the callee if they want a debug
  // name
  return shaderType;
}

RaverieShaderIRType* RaverieSpirVFrontEnd::MakeTypeAndPointer(RaverieShaderIRLibrary* shaderLibrary,
                                                          ShaderIRTypeBaseType::Enum baseType,
                                                          StringParam typeName,
                                                          Raverie::BoundType* raverieType,
                                                          spv::StorageClass pointerStorageClass)
{
  ErrorIf(baseType == ShaderIRTypeBaseType::Pointer, "BaseType cannot be a pointer type");
  // Make both the regular shader type and the pointer type
  RaverieShaderIRType* shaderType =
      MakeTypeInternal(shaderLibrary, baseType, typeName, raverieType, spv::StorageClassGeneric);
  RaverieShaderIRType* pointerType = MakeTypeInternal(
      shaderLibrary, ShaderIRTypeBaseType::Pointer, BuildString(typeName, "_ptr"), raverieType, pointerStorageClass);
  // Link both types up to each other
  pointerType->mDereferenceType = shaderType;
  shaderType->mPointerType = pointerType;
  return shaderType;
}

RaverieShaderIRType* RaverieSpirVFrontEnd::MakeCoreType(RaverieShaderIRLibrary* shaderLibrary,
                                                    ShaderIRTypeBaseType::Enum baseType,
                                                    size_t components,
                                                    RaverieShaderIRType* componentType,
                                                    Raverie::BoundType* raverieType,
                                                    bool makePointerType)
{
  RaverieShaderIRType* shaderType =
      MakeTypeAndPointer(shaderLibrary, baseType, raverieType->Name, raverieType, spv::StorageClassFunction);
  shaderType->mComponentType = componentType;
  shaderType->mComponents = components;
  return shaderType;
}

RaverieShaderIRType* RaverieSpirVFrontEnd::MakeStructType(RaverieShaderIRLibrary* shaderLibrary,
                                                      StringParam typeName,
                                                      Raverie::BoundType* raverieType,
                                                      spv::StorageClass pointerStorageClass)
{
  return MakeTypeAndPointer(shaderLibrary, ShaderIRTypeBaseType::Struct, typeName, raverieType, pointerStorageClass);
}

RaverieShaderIRType* RaverieSpirVFrontEnd::FindOrCreateInterfaceType(RaverieShaderIRLibrary* shaderLibrary,
                                                                 StringParam baseTypeName,
                                                                 Raverie::BoundType* raverieType,
                                                                 ShaderIRTypeBaseType::Enum baseType,
                                                                 spv::StorageClass storageClass)
{
  // Build the fully qualified type name which is need for interface types
  StringBuilder builder;
  builder.Append(baseTypeName);
  if (baseType == ShaderIRTypeBaseType::Pointer)
    builder.Append("_ptr");
  if (storageClass == spv::StorageClassFunction)
  {
    // Don't add any special name here (otherwise duplicate pointer types will be created).
    // @JoshD: Cleanup later.
  }
  else if (storageClass == spv::StorageClassPrivate)
    builder.Append("_Private");
  else if (storageClass == spv::StorageClassInput)
    builder.Append("_Input");
  else if (storageClass == spv::StorageClassOutput)
    builder.Append("_Output");
  else if (storageClass == spv::StorageClassUniform)
    builder.Append("_Uniform");
  else if (storageClass == spv::StorageClassUniformConstant)
    builder.Append("_UniformConstant");
  else if (storageClass == spv::StorageClassStorageBuffer)
    builder.Append("_StorageBuffer");
  else
  {
    Error("Unknown storage class");
  }

  // Check if this type already exists
  String fullTypeName = builder.ToString();
  RaverieShaderIRType* shaderType = shaderLibrary->FindType(fullTypeName);
  if (shaderType != nullptr)
    return shaderType;

  // Create the type with the fully qualified name
  shaderType = MakeTypeInternal(shaderLibrary, baseType, fullTypeName, raverieType, storageClass);
  return shaderType;
}

RaverieShaderIRType* RaverieSpirVFrontEnd::FindOrCreateInterfaceType(RaverieShaderIRLibrary* shaderLibrary,
                                                                 Raverie::BoundType* raverieType,
                                                                 ShaderIRTypeBaseType::Enum baseType,
                                                                 spv::StorageClass storageClass)
{
  return FindOrCreateInterfaceType(shaderLibrary, raverieType->Name, raverieType, baseType, storageClass);
}

RaverieShaderIRType* RaverieSpirVFrontEnd::FindOrCreatePointerInterfaceType(RaverieShaderIRLibrary* shaderLibrary,
                                                                        RaverieShaderIRType* valueType,
                                                                        spv::StorageClass storageClass)
{
  ErrorIf(valueType->IsPointerType(), "Type must be a value type");
  RaverieShaderIRType* pointerType = FindOrCreateInterfaceType(
      shaderLibrary, valueType->mName, valueType->mRaverieType, ShaderIRTypeBaseType::Pointer, storageClass);

  pointerType->mDereferenceType = valueType;
  return pointerType;
}

ShaderIRTypeMeta* RaverieSpirVFrontEnd::MakeShaderTypeMeta(RaverieShaderIRType* shaderType,
                                                         Raverie::NodeList<Raverie::AttributeNode>* nodeAttributeList)
{
  Raverie::BoundType* raverieType = shaderType->mRaverieType;
  ShaderIRTypeMeta* typeMeta = new ShaderIRTypeMeta();
  typeMeta->mRaverieName = shaderType->mName;
  typeMeta->mRaverieType = raverieType;
  // Parse attributes if possible (interface types don't have backing raverie
  // types)
  if (raverieType != nullptr)
    ParseAttributes(raverieType->Attributes, nodeAttributeList, typeMeta);
  shaderType->mMeta = typeMeta;
  mLibrary->mOwnedTypeMeta.PushBack(typeMeta);
  return typeMeta;
}

bool RaverieSpirVFrontEnd::Translate(Raverie::SyntaxTree& syntaxTree,
                                   RaverieShaderIRProject* project,
                                   RaverieShaderIRLibrary* library)
{
  mErrorTriggered = false;
  RaverieSpirVFrontEndContext context;
  mContext = &context;
  mLibrary = library;
  mProject = project;

  // Collect all types
  TranslatorBranchWalker classTypeCollectorWalker;
  classTypeCollectorWalker.Register(&RaverieSpirVFrontEnd::CollectClassTypes);
  classTypeCollectorWalker.Register(&RaverieSpirVFrontEnd::CollectEnumTypes);
  classTypeCollectorWalker.Walk(this, syntaxTree.Root, &context);
  // If this failed somehow then early return
  if (mErrorTriggered)
  {
    mContext = nullptr;
    return !mErrorTriggered;
  }

  // Now that we know about all types we can generate all template types
  // (templates have dependencies on other types)
  PreWalkTemplateTypes(&context);

  // Now we can walk classes again to get variable and function signatures since
  // we have all types
  TranslatorBranchWalker preWalker;
  preWalker.Register(&RaverieSpirVFrontEnd::PreWalkClassNode);
  preWalker.Register(&RaverieSpirVFrontEnd::PreWalkClassFunction);
  preWalker.Register(&RaverieSpirVFrontEnd::PreWalkClassConstructor);
  preWalker.Register(&RaverieSpirVFrontEnd::PreWalkClassVariables);
  preWalker.Walk(this, syntaxTree.Root, &context);

  // Check for errors again
  if (mErrorTriggered)
  {
    mContext = nullptr;
    return !mErrorTriggered;
  }

  mWalker.Walk(this, syntaxTree.Root, &context);
  library->mTranslated = true;
  mContext = nullptr;

  // Do an additional pass to make sure stage requirements are met
  // (e.g. a vertex doesn't call a pixel only function)
  StageRequirementsGatherer gatherer(mSettings);
  bool stageRequirementsValid = gatherer.Run(syntaxTree, library, mProject);
  mErrorTriggered |= stageRequirementsValid;

  // Do another pass to find cycles (recursion is illegal in shaders)
  CycleDetection cycleDetection(mSettings);
  bool cyclesFound = cycleDetection.Run(syntaxTree, library, mProject);
  mErrorTriggered |= cyclesFound;

  return !mErrorTriggered;
}

void RaverieSpirVFrontEnd::ExtractRaverieAsComments(Raverie::SyntaxNode*& node, RaverieSpirVFrontEndContext* context)
{
  context->Flags = Raverie::WalkerFlags::ChildrenNotHandled;
  if (!Raverie::StatementNode::IsNodeUsedAsStatement(node))
    return;

  // As long as the statement isn't a scoped based node (if, for, while, etc)
  // then we know it requires delimiting
  if (Raverie::Type::DynamicCast<Raverie::ScopeNode*>(node) != nullptr ||
      Raverie::Type::DynamicCast<Raverie::IfRootNode*>(node) != nullptr)
    return;

  ExtractDebugInfo(node, context->mDebugInfo);
}

void RaverieSpirVFrontEnd::ParseAttributes(Raverie::Array<Raverie::Attribute>& raverieAttributes,
                                         Raverie::NodeList<Raverie::AttributeNode>* attributeNodes,
                                         ShaderIRTypeMeta* typeMeta)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  ShaderIRAttributeList& shaderAttributes = typeMeta->mAttributes;

  ParseRaverieAttributes(raverieAttributes, attributeNodes, shaderAttributes);
  ValidateAllowedAttributes(shaderAttributes, nameSettings.mAllowedClassAttributes, "types");

  Array<int> fragmentTypeAttributeIndices;
  for (size_t i = 0; i < shaderAttributes.Size(); ++i)
  {
    ShaderIRAttribute* shaderAttribute = shaderAttributes[i];

    String attributeName = shaderAttribute->mAttributeName;

    if (attributeName == nameSettings.mVertexAttribute)
    {
      fragmentTypeAttributeIndices.PushBack(i);
    }
    else if (attributeName == nameSettings.mGeometryAttribute)
    {
      fragmentTypeAttributeIndices.PushBack(i);
      ValidateSingleParamAttribute(
          shaderAttribute, nameSettings.mMaxVerticesParam, Raverie::ConstantType::Integer, false);
    }
    else if (attributeName == nameSettings.mPixelAttribute)
    {
      fragmentTypeAttributeIndices.PushBack(i);
    }
    else if (attributeName == nameSettings.mComputeAttribute)
    {
      fragmentTypeAttributeIndices.PushBack(i);
      ValidateAndParseComputeAttributeParameters(shaderAttribute, typeMeta);
    }
    else if (attributeName == nameSettings.mStorageClassAttribute)
    {
      ValidateSingleParamAttribute(shaderAttribute, String(), Raverie::ConstantType::Integer, true);
    }
    else
      ValidateAttributeParameters(shaderAttribute, nameSettings.mAllowedClassAttributes, "types");
  }

  // If we found more than one fragment type attribute
  if (fragmentTypeAttributeIndices.Size() > 1)
  {
    ShaderIRAttribute* shaderAttribute0 = shaderAttributes[fragmentTypeAttributeIndices[0]];
    ShaderIRAttribute* shaderAttribute1 = shaderAttributes[fragmentTypeAttributeIndices[1]];
    String msg = String::Format("Attribute '%s' cannot be combined with attribute '%s'",
                                shaderAttribute1->mAttributeName.c_str(),
                                shaderAttribute0->mAttributeName.c_str());
    SendTranslationError(shaderAttribute1->GetLocation(), msg);
  }
}

void RaverieSpirVFrontEnd::ParseAttributes(Raverie::Array<Raverie::Attribute>& raverieAttributes,
                                         Raverie::NodeList<Raverie::AttributeNode>* attributeNodes,
                                         ShaderIRFunctionMeta* functionMeta)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  ShaderIRAttributeList& shaderAttributes = functionMeta->mAttributes;

  ParseRaverieAttributes(raverieAttributes, attributeNodes, shaderAttributes);
  ValidateAllowedAttributes(shaderAttributes, nameSettings.mAllowedFunctionAttributes, "functions");

  for (size_t i = 0; i < shaderAttributes.Size(); ++i)
  {
    ShaderIRAttribute* shaderAttribute = shaderAttributes[i];

    String attributeName = shaderAttribute->mAttributeName;
    // Extension should be validated by raverie?
    if (attributeName == nameSettings.mExtensionAttribute)
      continue;
    else
      ValidateAttributeParameters(shaderAttribute, nameSettings.mAllowedFunctionAttributes, "functions");
  }
}

void RaverieSpirVFrontEnd::ParseAttributes(Raverie::Array<Raverie::Attribute>& raverieAttributes,
                                         Raverie::NodeList<Raverie::AttributeNode>* attributeNodes,
                                         ShaderIRFieldMeta* fieldMeta)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  ShaderIRAttributeList& shaderAttributes = fieldMeta->mAttributes;

  ParseRaverieAttributes(raverieAttributes, attributeNodes, shaderAttributes);
  ValidateAllowedAttributes(shaderAttributes, nameSettings.mAllowedFieldAttributes, "fields");

  Array<String> staticExclusions;
  staticExclusions.PushBack(nameSettings.mStaticAttribute);

  for (size_t i = 0; i < shaderAttributes.Size(); ++i)
  {
    ShaderIRAttribute* shaderAttribute = shaderAttributes[i];

    String attributeName = shaderAttribute->mAttributeName;
    // Inputs
    if (attributeName == nameSettings.mInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mFragmentInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mStageInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mHardwareBuiltInInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateHardwareBuiltIn(fieldMeta, shaderAttribute, true);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mAppBuiltInInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);

      // If this is an explicit attribute then check to see if it matches
      // something in the uniform buffers
      if (!shaderAttribute->mImplicitAttribute)
      {
        FragmentType::Enum currentFragmentType = mContext->mCurrentType->mMeta->mFragmentType;
        String fieldType = fieldMeta->mRaverieType->ToString();
        // Make sure to get the actual field name being searched for (in case of
        // overrides)
        String appBuiltInName = fieldMeta->GetFieldAttributeName(shaderAttribute);
        bool isValid = mSettings->IsValidUniform(currentFragmentType, fieldType, appBuiltInName);
        if (!isValid)
        {
          String msg = String::Format("Field '%s : %s' does not match any "
                                      "provided %s with fragment type '%s'.",
                                      appBuiltInName.c_str(),
                                      fieldType.c_str(),
                                      shaderAttribute->mAttributeName.c_str(),
                                      FragmentType::Names[currentFragmentType]);
          SendTranslationError(shaderAttribute->GetLocation(), msg);
          break;
        }
      }
    }
    else if (attributeName == nameSettings.mPropertyInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mSpecializationConstantInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mFragmentSharedAttribute)
    {
      RaverieShaderIRType* fieldShaderType = FindType(fieldMeta->mRaverieType, nullptr);
      ShaderIRAttribute* storageClassAttribute =
          fieldShaderType->FindFirstAttribute(SpirVNameSettings::mStorageClassAttribute);

      // Currently, fragment shared is only allowed on types that are stored
      // globally (aka, types with non function storage class)
      bool valid = false;
      if (storageClassAttribute != nullptr)
      {
        spv::StorageClass forcedStorageClass = (spv::StorageClass)storageClassAttribute->mParameters[0].GetIntValue();
        if (forcedStorageClass != spv::StorageClassFunction)
          continue;
      }
      // Otherwise this type either doesn't have a forced storage class or is function storage class.
      String msg = String::Format("Attribute [%s] is only valid on opaque data types", attributeName.c_str());
      SendTranslationError(shaderAttribute->GetLocation(), msg);
    }
    // Outputs
    else if (attributeName == nameSettings.mOutputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mFragmentOutputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mStageOutputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mHardwareBuiltInOutputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateHardwareBuiltIn(fieldMeta, shaderAttribute, false);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mSpecializationConstantAttribute)
    {
      ValidateAttributeNoParameters(shaderAttribute);

      Array<String> dependencies;
      dependencies.PushBack(nameSettings.mStaticAttribute);
      ValidateAttributeDependencies(shaderAttribute, shaderAttributes, dependencies);
    }
    else
      ValidateAttributeParameters(shaderAttribute, nameSettings.mAllowedFieldAttributes, "fields");
  }
}

void RaverieSpirVFrontEnd::ParseRaverieAttributes(Raverie::Array<Raverie::Attribute>& raverieAttributes,
                                              Raverie::NodeList<Raverie::AttributeNode>* attributeNodes,
                                              ShaderIRAttributeList& shaderAttributes)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  // Parse the attributes and their parameters
  for (size_t attributeIndex = 0; attributeIndex < raverieAttributes.Size(); ++attributeIndex)
  {
    Raverie::Attribute& attribute = raverieAttributes[attributeIndex];

    Raverie::AttributeNode* node = nullptr;
    if (attributeNodes != nullptr)
      node = (*attributeNodes)[attributeIndex];
    ParseRaverieAttribute(attribute, node, shaderAttributes);
  }
}

void RaverieSpirVFrontEnd::ParseRaverieAttribute(Raverie::Attribute& raverieAttribute,
                                             Raverie::AttributeNode* attributeNode,
                                             ShaderIRAttributeList& shaderAttributes)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  ShaderIRAttribute* shaderAttribute = shaderAttributes.AddAttribute(raverieAttribute.Name, attributeNode);

  for (size_t paramIndex = 0; paramIndex < raverieAttribute.Parameters.Size(); ++paramIndex)
  {
    Raverie::AttributeParameter& param = raverieAttribute.Parameters[paramIndex];
    shaderAttribute->mParameters.PushBack(ShaderIRAttributeParameter(param, nullptr));
  }

  // Set locations if available
  if (attributeNode != nullptr)
  {
    for (size_t paramIndex = 0; paramIndex < raverieAttribute.Parameters.Size(); ++paramIndex)
    {
      Raverie::SyntaxNode* paramNode = attributeNode->AttributeCall->Arguments[paramIndex];
      shaderAttribute->mParameters[paramIndex].SetLocationNode(paramNode);
    }
  }

  // The [Input] attribute implies several other attributes
  if (raverieAttribute.Name == nameSettings.mInputAttribute)
  {
    // Create a copy of the [Input] attribute with all of the sub-attributes
    // (copy the attributes first since they're not safely stored)
    Array<ShaderIRAttributeParameter> params = shaderAttribute->mParameters;
    for (size_t i = 0; i < nameSettings.mInputSubAttributes.Size(); ++i)
    {
      String subAttributeName = nameSettings.mInputSubAttributes[i];
      ShaderIRAttribute* subAttribute = shaderAttributes.AddAttribute(subAttributeName, attributeNode);
      subAttribute->mParameters.Append(params.All());
      subAttribute->mImplicitAttribute = true;
    }
  }
  // [Output] also has sub-attributes
  else if (raverieAttribute.Name == nameSettings.mOutputAttribute)
  {
    Array<ShaderIRAttributeParameter> params = shaderAttribute->mParameters;
    for (size_t i = 0; i < nameSettings.mOutputSubAttributes.Size(); ++i)
    {
      String subAttributeName = nameSettings.mOutputSubAttributes[i];
      ShaderIRAttribute* subAttribute = shaderAttributes.AddAttribute(subAttributeName, attributeNode);
      subAttribute->mParameters.Append(params.All());
      subAttribute->mImplicitAttribute = true;
    }
  }
}

void RaverieSpirVFrontEnd::ValidateAllowedAttributes(ShaderIRAttributeList& shaderAttributes,
                                                   HashMap<String, AttributeInfo>& allowedAttributes,
                                                   StringParam errorTypeName)
{
  for (size_t i = 0; i < shaderAttributes.Size(); ++i)
  {
    ShaderIRAttribute* attribute = shaderAttributes[i];

    if (allowedAttributes.ContainsKey(attribute->mAttributeName))
      continue;

    String msg =
        String::Format("Attribute '%s' is not allowed on %s", attribute->mAttributeName.c_str(), errorTypeName.c_str());
    SendTranslationError(attribute->GetLocation(), msg);
  }
}

void RaverieSpirVFrontEnd::ValidateNameOverrideAttribute(ShaderIRAttribute* shaderAttribute)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  // No parameters is fine
  size_t paramCount = shaderAttribute->mParameters.Size();
  if (paramCount == 0)
    return;

  // More than one parameter is a guaranteed error
  if (paramCount > 1)
  {
    String msg = String::Format("Too many parameters to attribute '%s'. "
                                "Signature must be '%s : String'",
                                shaderAttribute->mAttributeName.c_str(),
                                SpirVNameSettings::mNameOverrideParam.c_str());
    SendTranslationError(shaderAttribute->GetLocation(), msg);
    return;
  }
  // One parameter might be an error
  if (paramCount == 1)
  {
    ShaderIRAttributeParameter& param = shaderAttribute->mParameters[0];

    // The param type must be a string
    if (param.GetType() != Raverie::ConstantType::String)
    {
      String msg = String::Format("Invalid parameter type to attribute '%s'. Signature "
                                  "must be '%s : String'",
                                  shaderAttribute->mAttributeName.c_str(),
                                  SpirVNameSettings::mNameOverrideParam.c_str());
      SendTranslationError(shaderAttribute->GetLocation(), msg);
      return;
    }

    // The parameter name must either be empty or it must be 'name'
    String paramName = param.GetName();
    if (!paramName.Empty() && paramName != SpirVNameSettings::mNameOverrideParam)
    {
      String msg = String::Format("Invalid parameter name to attribute '%s'. Signature "
                                  "must be '%s : String'",
                                  shaderAttribute->mAttributeName.c_str(),
                                  SpirVNameSettings::mNameOverrideParam.c_str());
      SendTranslationError(shaderAttribute->GetLocation(), msg);
      return;
    }

    // Validate that the name parameter is a valid identifier (needed for the
    // compositor and a few other places)
    String paramValue = param.GetStringValue();
    if (Raverie::LibraryBuilder::CheckUpperIdentifier(paramValue) == false)
    {
      String msg = String::Format("Parameter '%s' must be a valid raverie uppercase identifier.",
                                  SpirVNameSettings::mNameOverrideParam.c_str());
      SendTranslationError(shaderAttribute->GetLocation(), msg);
      return;
    }
  }
}

void RaverieSpirVFrontEnd::ValidateSingleParamAttribute(ShaderIRAttribute* shaderAttribute,
                                                      StringParam expectedParamName,
                                                      Raverie::ConstantType::Enum expectedParamType,
                                                      bool allowEmptyName)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  // No parameters is an error
  size_t paramCount = shaderAttribute->mParameters.Size();
  if (paramCount == 0)
  {
    String msg = String::Format("Not enough parameters to attribute '%s'. Signature must be '%s : %s'",
                                shaderAttribute->mAttributeName.c_str(),
                                expectedParamName.c_str(),
                                Raverie::ConstantType::Names[expectedParamType]);
    SendTranslationError(shaderAttribute->GetLocation(), msg);
    return;
  }

  // More than one parameter is an error
  if (paramCount > 1)
  {
    String msg = String::Format("Too many parameters to attribute '%s'. Signature must be '%s : %s'",
                                shaderAttribute->mAttributeName.c_str(),
                                expectedParamName.c_str(),
                                Raverie::ConstantType::Names[expectedParamType]);
    SendTranslationError(shaderAttribute->GetLocation(), msg);
    return;
  }

  // One parameter might be an error
  ShaderIRAttributeParameter& param = shaderAttribute->mParameters[0];

  // The param type must match
  Raverie::ConstantType::Enum actualParamType = param.GetType();
  if (actualParamType != expectedParamType)
  {
    String msg = String::Format("Invalid parameter type '%s' to attribute "
                                "'%s'. Signature must be '%s : %s'",
                                Raverie::ConstantType::Names[actualParamType],
                                shaderAttribute->mAttributeName.c_str(),
                                expectedParamName.c_str(),
                                Raverie::ConstantType::Names[expectedParamType]);
    SendTranslationError(shaderAttribute->GetLocation(), msg);
    return;
  }

  // The parameter name must also match
  String actualParamName = param.GetName();
  if ((actualParamName.Empty() && !allowEmptyName) || (actualParamName != expectedParamName))
  {
    String msg = String::Format("Invalid parameter name '%s' to attribute "
                                "'%s'. Signature must be '%s : %s'",
                                actualParamName.c_str(),
                                shaderAttribute->mAttributeName.c_str(),
                                expectedParamName.c_str(),
                                Raverie::ConstantType::Names[expectedParamType]);
    SendTranslationError(shaderAttribute->GetLocation(), msg);
    return;
  }
}

void RaverieSpirVFrontEnd::ValidateAttributeNoParameters(ShaderIRAttribute* shaderAttribute)
{
  // No parameters is fine
  size_t paramCount = shaderAttribute->mParameters.Size();
  if (paramCount != 0)
  {
    String msg = String::Format("Invalid parameter count. Attribute '%s' doesn't allow any parameters",
                                shaderAttribute->mAttributeName.c_str());
    SendTranslationError(shaderAttribute->GetLocation(), msg);
  }
}

void RaverieSpirVFrontEnd::ValidateAttributeParameters(ShaderIRAttribute* shaderAttribute,
                                                     HashMap<String, AttributeInfo>& allowedAttributes,
                                                     StringParam errorTypeName)
{
  // First find the attribute's info so we know what it allows
  AttributeInfo* attributeInfo = allowedAttributes.FindPointer(shaderAttribute->mAttributeName);
  if (attributeInfo == nullptr)
  {
    // This is often a redundant error check, but whatever
    String msg = String::Format(
        "Attribute '%s' is not allowed on %s", shaderAttribute->mAttributeName.c_str(), errorTypeName.c_str());
    SendTranslationError(shaderAttribute->GetLocation(), msg);
    return;
  }

  // If the attribute doesn't care about signature checking then ignore it. Mostly for legacy
  if (attributeInfo->mCheckSignature == false)
    return;

  // Handle zero params first
  if (attributeInfo->mOverloads.Size() == 0 && shaderAttribute->mParameters.Size() == 0)
    return;

  // Check all of the overloads
  bool isValid = false;
  for (size_t i = 0; i < attributeInfo->mOverloads.Size(); ++i)
  {
    isValid |= ValidateAttributeParameterSignature(shaderAttribute, attributeInfo->mOverloads[i]);
    if (isValid)
      break;
  }

  // If any overload matches then we're cool
  if (isValid)
    return;

  // Otherwise print the error message
  StringBuilder builder;
  builder.AppendFormat("No matching signature for attribute '%s'.\n", shaderAttribute->mAttributeName.c_str());
  builder.AppendFormat("Provided signature is %s(", shaderAttribute->mAttributeName.c_str());
  for (size_t i = 0; i < shaderAttribute->mParameters.Size(); ++i)
  {
    const ShaderIRAttributeParameter& param = shaderAttribute->mParameters[i];
    shaderAttribute->mParameters[i].GetType();
    String typeName = Raverie::ConstantType::Names[param.GetType()];
    builder.Append(typeName);
    if (i != shaderAttribute->mParameters.Size() - 1)
      builder.Append(", ");
  }
  builder.Append(")\n");
  builder.AppendFormat("Possible signatures are:\n");
  for (size_t i = 0; i < attributeInfo->mOverloads.Size(); ++i)
  {
    builder.AppendFormat("%s(", shaderAttribute->mAttributeName.c_str());
    const AttributeInfo::ParameterSignature& signature = attributeInfo->mOverloads[i];
    for (size_t i = 0; i < signature.mTypes.Size(); ++i)
    {
      String typeName = Raverie::ConstantType::Names[signature.mTypes[i]];
      builder.Append(typeName);
      if (i != signature.mTypes.Size() - 1)
        builder.Append(", ");
    }
    builder.Append(")\n");
  }
  SendTranslationError(shaderAttribute->GetLocation(), builder.ToString());
}

bool RaverieSpirVFrontEnd::ValidateAttributeParameterSignature(ShaderIRAttribute* shaderAttribute,
                                                             const AttributeInfo::ParameterSignature& signature) const
{
  if (shaderAttribute->mParameters.Size() != signature.mTypes.Size())
    return false;

  for (size_t i = 0; i < shaderAttribute->mParameters.Size(); ++i)
  {
    const ShaderIRAttributeParameter& attributeParam = shaderAttribute->mParameters[i];
    if (!DoTypesMatch(attributeParam.GetType(), signature.mTypes[i]))
      return false;
  }
  return true;
}

bool RaverieSpirVFrontEnd::DoTypesMatch(const AttributeInfo::ParamType& actualType,
                                      const AttributeInfo::ParamType& expectedType) const
{
  if (actualType == expectedType)
    return true;
  if (expectedType == AttributeInfo::ParamType::Real && actualType == AttributeInfo::ParamType::Integer)
    return true;
  return false;
}

void RaverieSpirVFrontEnd::ValidateAttributeDependencies(ShaderIRAttribute* shaderAttribute,
                                                       ShaderIRAttributeList& shaderAttributeList,
                                                       Array<String>& dependencies)
{
  // Walk all dependencies, keeping track of what we're missing
  Array<String> missingDependencies;
  for (size_t i = 0; i < dependencies.Size(); ++i)
  {
    String dependencyName = dependencies[i];
    if (shaderAttributeList.FindFirstAttribute(dependencyName) == nullptr)
      missingDependencies.PushBack(dependencyName);
  }

  // If there's any missing dependencies then display an error
  if (!missingDependencies.Empty())
  {
    StringBuilder errBuilder;
    errBuilder.AppendFormat("Attribute '%s' requires attribute(s): ", shaderAttribute->mAttributeName.c_str());
    for (size_t i = 0; i < missingDependencies.Size(); ++i)
    {
      errBuilder.Append(missingDependencies[i]);
      if (i != missingDependencies.Size() - 1)
        errBuilder.Append(", ");
    }
    SendTranslationError(shaderAttribute->GetLocation(), errBuilder.ToString());
  }
}

void RaverieSpirVFrontEnd::ValidateAttributeExclusions(ShaderIRAttribute* shaderAttribute,
                                                     ShaderIRAttributeList& shaderAttributeList,
                                                     Array<String>& exclusions)
{
  // Walk all dependencies, keeping track of any that are found
  Array<String> foundExclusions;
  for (size_t i = 0; i < exclusions.Size(); ++i)
  {
    String exclusionName = exclusions[i];
    if (shaderAttributeList.FindFirstAttribute(exclusionName) != nullptr)
      foundExclusions.PushBack(exclusionName);
  }

  // If there's any exclusions found then display an error
  if (!foundExclusions.Empty())
  {
    StringBuilder errBuilder;
    errBuilder.AppendFormat("Attribute '%s' cannot be combined with attribute(s): ",
                            shaderAttribute->mAttributeName.c_str());
    for (size_t i = 0; i < foundExclusions.Size(); ++i)
    {
      errBuilder.Append(foundExclusions[i]);
      if (i != foundExclusions.Size() - 1)
        errBuilder.Append(", ");
    }
    SendTranslationError(shaderAttribute->GetLocation(), errBuilder.ToString());
  }
}

void RaverieSpirVFrontEnd::ValidateHardwareBuiltIn(ShaderIRFieldMeta* fieldMeta,
                                                 ShaderIRAttribute* shaderAttribute,
                                                 bool isInput)
{
  // If this is an explicit attribute then check to see if it matches something
  // in the uniform buffers
  if (!shaderAttribute->mImplicitAttribute)
  {
    FragmentType::Enum currentFragmentType = mContext->mCurrentType->mMeta->mFragmentType;
    String fieldType = fieldMeta->mRaverieType->ToString();
    // Make sure to get the actual field name being searched for (in case of
    // overrides)
    String appBuiltInName = fieldMeta->GetFieldAttributeName(shaderAttribute);
    bool isValid = mSettings->IsValidHardwareBuiltIn(currentFragmentType, fieldType, appBuiltInName, isInput);
    if (!isValid)
    {
      String msg = String::Format("Field '%s : %s' does not match any provided "
                                  "%s with fragment type '%s'.",
                                  appBuiltInName.c_str(),
                                  fieldType.c_str(),
                                  shaderAttribute->mAttributeName.c_str(),
                                  FragmentType::Names[currentFragmentType]);
      SendTranslationError(shaderAttribute->GetLocation(), msg);
    }
  }
}

void RaverieSpirVFrontEnd::ValidateAndParseComputeAttributeParameters(ShaderIRAttribute* shaderAttribute,
                                                                    ShaderIRTypeMeta* typeMeta)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  // Create the user data for the compute fragment to store the parameters we
  // parse from the attribute.
  Raverie::HandleOf<Raverie::ComputeFragmentUserData> handle = RaverieAllocate(Raverie::ComputeFragmentUserData);
  // Default all local sizes to 1 (they're all optional)
  handle->mLocalSizeX = 1;
  handle->mLocalSizeY = 1;
  handle->mLocalSizeZ = 1;
  typeMeta->mRaverieType->Add(*handle);

  for (size_t i = 0; i < shaderAttribute->mParameters.Size(); ++i)
  {
    ShaderIRAttributeParameter& param = shaderAttribute->mParameters[i];
    String paramName = param.GetName();
    if (paramName == nameSettings.mComputeLocalSizeXParam)
      ValidateLocalSize(param, 128, handle->mLocalSizeX);
    else if (paramName == nameSettings.mComputeLocalSizeYParam)
      ValidateLocalSize(param, 128, handle->mLocalSizeY);
    else if (paramName == nameSettings.mComputeLocalSizeZParam)
      ValidateLocalSize(param, 64, handle->mLocalSizeZ);
    else
    {
      String msg = String::Format("Attribute paramater '%s' is invalid.", paramName.c_str());
      SendTranslationError(param.GetLocation(), msg);
    }
  }
}

void RaverieSpirVFrontEnd::ValidateLocalSize(ShaderIRAttributeParameter& param, int max, int& toStore)
{
  int intValue = param.GetIntValue();
  if (intValue <= 0 || intValue > max)
  {
    String msg = String::Format("Parameter '%s' must be in the range of [1, %d].", param.GetName().c_str(), max);
    SendTranslationError(param.GetLocation(), msg);
    return;
  }
  toStore = intValue;
}

String RaverieSpirVFrontEnd::BuildFunctionTypeString(Raverie::Function* raverieFunction, RaverieSpirVFrontEndContext* context)
{
  // Get the return type of the function (use void if there isn't one)
  Raverie::Type* raverieReturnType;
  if (raverieFunction != nullptr && raverieFunction->FunctionType->Return != nullptr)
    raverieReturnType = raverieFunction->FunctionType->Return;
  else
    raverieReturnType = RaverieTypeId(void);

  StringBuilder functionTypeBuilder;
  functionTypeBuilder.Append("(");
  if (raverieFunction != nullptr)
  {
    Raverie::DelegateType* functionType = raverieFunction->FunctionType;
    // Handle adding the self parameter first if this is a member function
    if (!raverieFunction->IsStatic)
    {
      // Get the actual 'this' type from the function (deals with extension
      // methods)
      Raverie::BoundType* raverieSelfType = raverieFunction->Owner;
      functionTypeBuilder.Append(raverieSelfType->IndirectType->ToString());
      functionTypeBuilder.Append(",");
    }
    // Add all parameters
    for (size_t i = 0; i < functionType->Parameters.Size(); ++i)
    {
      Raverie::DelegateParameter& delegateParameter = functionType->Parameters[i];
      functionTypeBuilder.Append(delegateParameter.ParameterType->ToString());
      functionTypeBuilder.Append(",");
    }
  }
  functionTypeBuilder.Append(")");

  // Add the return type
  functionTypeBuilder.Append(" : ");
  String returnType = raverieReturnType->ToString();
  functionTypeBuilder.Append(returnType);

  return functionTypeBuilder.ToString();
}

String RaverieSpirVFrontEnd::BuildFunctionTypeString(Raverie::BoundType* raverieReturnType,
                                                   Array<Raverie::Type*>& signature,
                                                   RaverieSpirVFrontEndContext* context)
{
  ErrorIf(raverieReturnType == nullptr, "Signature must have at least one argument (return type)");

  StringBuilder functionTypeBuilder;
  // Add all parameters
  functionTypeBuilder.Append("(");
  for (size_t i = 0; i < signature.Size(); ++i)
  {
    functionTypeBuilder.Append(signature[i]->ToString());
    functionTypeBuilder.Append(",");
  }
  functionTypeBuilder.Append(")");

  // Add the return type
  functionTypeBuilder.Append(" : ");
  String returnTypeStr = raverieReturnType->ToString();
  functionTypeBuilder.Append(returnTypeStr);

  return functionTypeBuilder.ToString();
}

void RaverieSpirVFrontEnd::GenerateFunctionType(Raverie::SyntaxNode* locationNode,
                                              RaverieShaderIRFunction* function,
                                              Raverie::Function* raverieFunction,
                                              RaverieSpirVFrontEndContext* context)
{
  ErrorIf(raverieFunction == nullptr, "");
  String functionTypeStr = BuildFunctionTypeString(raverieFunction, context);
  function->mFunctionType = mLibrary->FindType(functionTypeStr, true);
  // If the function type already exists then we're done
  if (function->mFunctionType != nullptr)
    return;

  // Otherwise we have to generate the delegate type
  RaverieShaderIRType* functionType = new RaverieShaderIRType();
  function->mFunctionType = functionType;
  function->mFunctionType->mBaseType = ShaderIRTypeBaseType::Function;
  function->mFunctionType->mShaderLibrary = mLibrary;
  mLibrary->AddType(functionTypeStr, function->mFunctionType);

  // Extract the function type for the raverie function if we can
  Raverie::DelegateType* raverieFunctionType = nullptr;
  if (raverieFunction != nullptr)
    raverieFunctionType = raverieFunction->FunctionType;

  // Find the return type. If there isn't one then use void.
  Raverie::Type* raverieReturnType;
  if (raverieFunctionType != nullptr && raverieFunctionType->Return != nullptr)
    raverieReturnType = raverieFunctionType->Return;
  else
    raverieReturnType = RaverieTypeId(void);

  // Add a reference to the return type
  RaverieShaderIRType* returnType = FindType(raverieReturnType, locationNode);
  // If the return is a value type that can't be copied then display an error.
  if (ContainsAttribute(returnType, SpirVNameSettings::mNonCopyableAttributeName))
  {
    String msg =
        String::Format("Type '%s' is an invalid return type as it cannot be copied.", returnType->mName.c_str());
    SendTranslationError(locationNode->Location, msg);
  }
  functionType->mParameters.PushBack(returnType);

  if (raverieFunctionType != nullptr)
  {
    // If this is a member function then we have to add references to the
    // 'this' type and make the first argument the 'this' pointer
    if (!raverieFunction->IsStatic)
    {
      // Get the actual 'this' type from the function (deals with extension
      // methods)
      Raverie::BoundType* raverieThisType = raverieFunction->Owner;
      RaverieShaderIRType* thisType = FindType(raverieThisType, locationNode);
      RaverieShaderIRType* thisPointerType = thisType->mPointerType;
      functionType->mParameters.PushBack(thisPointerType);
    }

    // Add all parameters and add dependencies on all parameter types
    for (size_t i = 0; i < raverieFunctionType->Parameters.Size(); ++i)
    {
      Raverie::DelegateParameter& parameter = raverieFunctionType->Parameters[i];

      RaverieShaderIRType* shaderParameterType = FindType(parameter.ParameterType, locationNode);
      RaverieShaderIRType* shaderParamPointerType = shaderParameterType->mPointerType;

      // Make sure to pass through the correct shader type (pointer/value) based
      // upon the raverie type.
      if (parameter.ParameterType->IsIndirectionType(parameter.ParameterType))
        functionType->mParameters.PushBack(shaderParamPointerType);
      else
      {
        // If this is a value type parameter that can't be copied then display
        // an error.
        if (ContainsAttribute(shaderParameterType, SpirVNameSettings::mNonCopyableAttributeName))
        {
          String msg = String::Format("Type '%s' cannot be copied. This parameter must "
                                      "be passed through by reference (ref keyword).",
                                      shaderParameterType->mName.c_str());
          SendTranslationError(locationNode->Location, msg);
        }
        functionType->mParameters.PushBack(shaderParameterType);
      }
    }
  }
}

void RaverieSpirVFrontEnd::GenerateFunctionType(Raverie::SyntaxNode* locationNode,
                                              RaverieShaderIRFunction* function,
                                              Raverie::BoundType* raverieReturnType,
                                              Array<Raverie::Type*>& signature,
                                              RaverieSpirVFrontEndContext* context)
{
  ErrorIf(raverieReturnType == nullptr, "Signature must have a return type");

  String functionTypeStr = BuildFunctionTypeString(raverieReturnType, signature, context);
  function->mFunctionType = mLibrary->FindType(functionTypeStr);
  // If the function type already exists then we're done
  if (function->mFunctionType != nullptr)
    return;

  // Otherwise we have to generate the delegate type
  RaverieShaderIRType* functionType = new RaverieShaderIRType();
  function->mFunctionType = functionType;
  function->mFunctionType->mBaseType = ShaderIRTypeBaseType::Function;
  function->mFunctionType->mShaderLibrary = mLibrary;
  mLibrary->AddType(functionTypeStr, function->mFunctionType);

  // Set the function return type
  RaverieShaderIRType* returnType = FindType(raverieReturnType, locationNode);
  functionType->mParameters.PushBack(returnType);

  // Add all parameters and add dependencies on all parameter types
  for (size_t i = 0; i < signature.Size(); ++i)
  {
    Raverie::Type* parameter = signature[i];

    RaverieShaderIRType* shaderParameterType = FindType(parameter, locationNode);
    RaverieShaderIRType* shaderParamPointerType = shaderParameterType->mPointerType;

    // Make sure to pass through the correct shader type (pointer/value) based
    // upon the raverie type.
    if (parameter->IsIndirectionType(parameter))
      functionType->mParameters.PushBack(shaderParamPointerType);
    else
      functionType->mParameters.PushBack(shaderParameterType);
  }
}

RaverieShaderIRFunction* RaverieSpirVFrontEnd::GenerateIRFunction(Raverie::SyntaxNode* node,
                                                              Raverie::NodeList<Raverie::AttributeNode>* nodeAttributeList,
                                                              RaverieShaderIRType* owningType,
                                                              Raverie::Function* raverieFunction,
                                                              StringParam functionName,
                                                              RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* currentType = owningType;

  // Generate the function
  RaverieShaderIRFunction* function = currentType->CreateFunction(mLibrary);
  function->mName = GetOverloadedName(functionName, raverieFunction);
  function->mDebugResultName = function->mName;
  context->mCurrentFunction = function;

  // If we don't have a backing function then don't insert this
  // into the function map or extract debug information
  if (raverieFunction != nullptr)
  {
    ExtractDebugInfo(node, function->mDebugInfo);
    mLibrary->mFunctions.InsertOrError(raverieFunction, function);
  }

  // We need to generate the type of a function (delegate type).
  // Just like other types, this needs to be generated only once.
  GenerateFunctionType(node, function, raverieFunction, context);

  ShaderIRFunctionMeta* functionMeta = currentType->mMeta->CreateFunction(mLibrary);
  functionMeta->mRaverieName = function->mName;
  functionMeta->mRaverieFunction = raverieFunction;
  function->mMeta = functionMeta;

  ParseAttributes(raverieFunction->Attributes, nodeAttributeList, functionMeta);
  // If this function is actually a get/set of a property then parse
  // the attributes of property (that has implements and so on)
  if (raverieFunction->OwningProperty != nullptr)
    ParseAttributes(raverieFunction->OwningProperty->Attributes, nodeAttributeList, functionMeta);

  AddImplements(node, raverieFunction, function, functionName, context);

  return function;
}

void RaverieSpirVFrontEnd::AddImplements(Raverie::SyntaxNode* node,
                                       Raverie::Function* raverieFunction,
                                       RaverieShaderIRFunction* shaderFunction,
                                       StringParam functionName,
                                       RaverieSpirVFrontEndContext* context)
{
  if (raverieFunction == nullptr)
    return;

  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  // Check if this has the implements attribute
  ShaderIRFunctionMeta* fnMeta = shaderFunction->mMeta;
  if (!fnMeta->ContainsAttribute(nameSettings.mImplementsAttribute))
    return;

  // Find the extension attribute that corresponds to the implements
  ShaderIRAttribute* extAttribute = fnMeta->mAttributes.FindFirstAttribute(nameSettings.mExtensionAttribute);
  // Get the type we're extending (the first parameter)
  ShaderIRAttributeParameter param = extAttribute->mParameters[0];
  Raverie::BoundType* extType = Raverie::BoundType::GetBoundType(param.GetTypeValue());

  // Find the function from raverie's bound type (check static or instance as
  // appropriate)
  Raverie::FindMemberOptions::Enum findOptions = Raverie::FindMemberOptions::None;
  if (raverieFunction->IsStatic)
    findOptions = Raverie::FindMemberOptions::Static;

  Raverie::Function* replacementFn = nullptr;
  // Check to see if this is actually a getter/setter
  if (raverieFunction->OwningProperty != nullptr)
  {
    // If so find the property
    Raverie::Property* property = extType->FindProperty(functionName, findOptions);
    // Make sure the property has the same type as the one we're extending
    if (property != nullptr && property->PropertyType == raverieFunction->OwningProperty->PropertyType)
    {
      // Find which function to use by name (fix later?)
      if (raverieFunction->Name == property->Get->Name)
        replacementFn = property->Get;
      else if (raverieFunction->Name == property->Set->Name)
        replacementFn = property->Set;
      else
      {
        Error("This should never happen");
      }
    }
  }
  else
    replacementFn = extType->FindFunction(functionName, raverieFunction->FunctionType, findOptions);

  // We found a replacement function. Add a duplicate mapping for the raverie
  // function we're replacing to the target shader function
  if (replacementFn != nullptr)
  {
    mLibrary->mFunctions.InsertOrError(replacementFn, shaderFunction);
    return;
  }

  // Otherwise we failed to find a function to implement (override). Now report
  // an error to the user.

  // By default, set the error message as we can't find a function to match to
  StringBuilder msgBuilder;
  msgBuilder << "The signature of a function with the [" << nameSettings.mImplementsAttribute
             << "] attribute must match an existing function.\n";

  // For added error reporting, try to find all possible overloads of this
  // function (check static or instance as appropriate)
  const Raverie::FunctionArray* functions = nullptr;
  ShaderIRFunctionMeta* shaderFnMeta = shaderFunction->mMeta;
  if (shaderFnMeta->ContainsAttribute("Static"))
    functions = extType->GetOverloadedStaticFunctions(functionName);
  else
    functions = extType->GetOverloadedInstanceFunctions(functionName);
  // If we find overloads of the same function name then list all possible
  // overloads as candidates
  if (functions != nullptr)
  {
    msgBuilder.Append("Possible overloads are:\n");
    for (size_t i = 0; i < functions->Size(); ++i)
    {
      Raverie::Function* fn = (*functions)[i];
      msgBuilder << "\t" << fn->ToString() << "\n";
    }
  }
  else if (raverieFunction->OwningProperty != nullptr)
  {
    msgBuilder << "Type '" << extType->ToString() << "' does not contain the property '";
    msgBuilder << functionName << " : " << raverieFunction->OwningProperty->PropertyType->ToString() << "'.";
  }
  else
  {
    msgBuilder << "Type '" << extType->ToString() << "' does not contain a function named '" << functionName << "'.";
  }

  SendTranslationError(node->Location, msgBuilder.ToString());
}

void RaverieSpirVFrontEnd::CollectClassTypes(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context)
{
  // Make class type's errors (only allow structs).
  if (node->CopyMode != Raverie::TypeCopyMode::ValueType)
  {
    String msg = "Cannot declare class types in raverie fragments. Use struct instead.";
    SendTranslationError(node->Location, msg);
    return;
  }
  if (node->Inheritance.Size() != 0)
  {
    Raverie::SyntaxType* inheritanceNode = node->Inheritance[0];
    String shortMsg = "Inheritance is not supported in raverie fragments.";
    String longMsg = String::Format("Type '%s' inherits from type '%s' which is not supported.",
                                    node->Name.c_str(),
                                    inheritanceNode->ToString().c_str());
    SendTranslationError(inheritanceNode->Location, shortMsg, longMsg);
    return;
  }

  // Make a new ir type for this struct
  String raverieName = node->Name.Token;
  RaverieShaderIRType* type = MakeStructType(mLibrary, raverieName, node->Type, spv::StorageClassFunction);
  type->mDebugResultName = raverieName;

  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  ShaderIRTypeMeta* typeMeta = MakeShaderTypeMeta(type, &node->Attributes);
  if (typeMeta->mAttributes.FindFirstAttribute(nameSettings.mPixelAttribute))
    typeMeta->mFragmentType = FragmentType::Pixel;
  else if (typeMeta->mAttributes.FindFirstAttribute(nameSettings.mVertexAttribute))
    typeMeta->mFragmentType = FragmentType::Vertex;
  else if (typeMeta->mAttributes.FindFirstAttribute(nameSettings.mGeometryAttribute))
    typeMeta->mFragmentType = FragmentType::Geometry;
  else if (typeMeta->mAttributes.FindFirstAttribute(nameSettings.mComputeAttribute))
    typeMeta->mFragmentType = FragmentType::Compute;

  context->mCurrentType = type;

  ExtractDebugInfo(node, type->mDebugInfo);
}

void RaverieSpirVFrontEnd::CollectEnumTypes(Raverie::EnumNode*& node, RaverieSpirVFrontEndContext* context)
{
  // Map the enum type to integer. This is needed to handle
  // any function taking/returning the enum type.
  RaverieShaderIRType* intType = mLibrary->FindType(RaverieTypeId(int));
  mLibrary->mTypes[node->Name.Token] = intType;

  // For each value in the enum, create an integer constant that
  // can be looked up via the enum integer property
  for (size_t i = 0; i < node->Values.Size(); ++i)
  {
    Raverie::EnumValueNode* valueNode = node->Values[i];

    RaverieShaderIROp* constantValue = GetIntegerConstant(valueNode->IntegralValue, context);
    mLibrary->mEnumContants[valueNode->IntegralProperty] = constantValue;
  }
}

void RaverieSpirVFrontEnd::PreWalkClassNode(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context)
{
  context->mCurrentType = FindType(node->Type, node);

  // Destructors cannot be supported as there's no actual
  // call to the destructor in raverie's AST.
  if (node->Destructor != nullptr)
  {
    SendTranslationError(node->Location, "Destructors are not supported in shaders");
    return;
  }

  TranslatorBranchWalker* walker = context->Walker;
  walker->Walk(this, node->Variables, context);
  walker->Walk(this, node->Constructors, context);
  walker->Walk(this, node->Functions, context);
  GeneratePreConstructor(node, context);
  GenerateDefaultConstructor(node, context);
  GenerateDummyMemberVariable(node, context);

  PreWalkErrorCheck(context);

  context->mCurrentType = nullptr;
}

void RaverieSpirVFrontEnd::PreWalkTemplateTypes(RaverieSpirVFrontEndContext* context)
{
  Raverie::Library* raverieLibrary = mLibrary->mRaverieLibrary;
  Raverie::BoundTypeMap::range boundTypes = raverieLibrary->BoundTypes.All();
  for (; !boundTypes.Empty(); boundTypes.PopFront())
  {
    Raverie::BoundType* boundType = boundTypes.Front().second;
    PreWalkTemplateType(boundType, context);
  }
}

void RaverieSpirVFrontEnd::PreWalkTemplateType(Raverie::BoundType* raverieType, RaverieSpirVFrontEndContext* context)
{
  // Make sure this is actually a template
  if (raverieType->TemplateBaseName.Empty())
    return;

  // Deal with already having a translation for the type
  if (mLibrary->FindType(raverieType) != nullptr)
    return;

  // Check if we have a resolver for this template type
  TemplateTypeKey key = GenerateTemplateTypeKey(raverieType);
  TemplateTypeIRResloverFn resolver = mLibrary->FindTemplateResolver(key);
  if (resolver != nullptr)
    resolver(this, raverieType);
}

void RaverieSpirVFrontEnd::PreWalkClassVariables(Raverie::MemberVariableNode*& node, RaverieSpirVFrontEndContext* context)
{
  // This variable is actually a getter setter. Walk its functions instead
  if (node->CreatedGetterSetter != nullptr)
  {
    if (node->Get != nullptr)
      GenerateIRFunction(
          node, &node->Attributes, context->mCurrentType, node->Get->DefinedFunction, node->Name.Token, context);
    if (node->Set != nullptr)
      GenerateIRFunction(
          node, &node->Attributes, context->mCurrentType, node->Set->DefinedFunction, node->Name.Token, context);
    return;
  }

  // For each member type, find out what the member types are
  RaverieShaderIRType* currentType = context->mCurrentType;
  RaverieShaderIRType* memberType = FindType(node->ResultType, node);
  ErrorIf(memberType == nullptr, "Invalid member type");

  ShaderIRFieldMeta* fieldMeta = currentType->mMeta->CreateField(mLibrary);
  fieldMeta->mRaverieName = node->Name.Token;
  fieldMeta->mRaverieType = Raverie::BoundType::GetBoundType(node->ResultType);
  fieldMeta->mRaverieProperty = node->CreatedProperty;
  ParseAttributes(node->CreatedField->Attributes, &node->Attributes, fieldMeta);

  // If this is a runtime array (only detectable by the
  // raverie type's template name) then add a global runtime array.
  if (memberType->mRaverieType->TemplateBaseName == SpirVNameSettings::mRuntimeArrayTypeName)
  {
    AddRuntimeArray(node, memberType, fieldMeta, context);
    return;
  }

  // Check to see if this has a forced storage class
  ShaderIRAttribute* storageClassAttribute = memberType->FindFirstAttribute(SpirVNameSettings::mStorageClassAttribute);
  if (storageClassAttribute != nullptr)
  {
    // @JoshD: Right now I'm assuming this isn't a function storage class.
    // Change later?
    spv::StorageClass forcedStorageClass = (spv::StorageClass)storageClassAttribute->mParameters[0].GetIntValue();
    AddGlobalVariable(node, memberType, fieldMeta, forcedStorageClass, context);
    return;
  }

  // Check to see if this node is static
  if (node->IsStatic)
  {
    SpirVNameSettings& nameSettings = mSettings->mNameSettings;
    // Check for specialization constants
    if (fieldMeta->ContainsAttribute(nameSettings.mSpecializationConstantAttribute))
      AddSpecializationConstant(node, memberType, context);
    // Otherwise, add it as a global variables
    else
      AddGlobalVariable(node, memberType, fieldMeta, spv::StorageClassPrivate, context);
    return;
  }

  // Only actual add the member if this member belongs to the class (not global)
  currentType->AddMember(memberType, node->Name.Token);
}

void RaverieSpirVFrontEnd::AddRuntimeArray(Raverie::MemberVariableNode* node,
                                         RaverieShaderIRType* varType,
                                         ShaderIRFieldMeta* fieldMeta,
                                         RaverieSpirVFrontEndContext* context)
{
  // Make sure no constructor call exists (illegal as this type
  // must be constructed by the client api not by the shader)
  if (node->InitialValue != nullptr)
  {
    String typeName = node->ResultType->ToString();
    String msg = String::Format("Type '%s' does not support an explicit constructor call.", typeName.c_str());
    SendTranslationError(node->InitialValue->Location, msg);
    return;
  }

  RaverieShaderIRType* raverieRuntimeArrayType = varType;
  RaverieShaderIRType* actualRuntimeArrayType = varType->mParameters[0]->As<RaverieShaderIRType>();
  RaverieShaderIRType* containedType = actualRuntimeArrayType->mParameters[0]->As<RaverieShaderIRType>();

  // The glsl backend doesn't seem to properly support this (or it's a glsl
  // error). If the fixed array is put in a struct this all works though. This
  // error has to be reported here instead of during template parsing since this
  // is the only place a location is actually known.
  if (containedType->mBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    String msg = "Runtime array cannot directly contain a FixedArray. Please "
                 "put the FixedArray in a struct.";
    SendTranslationError(node->Location, msg);
    return;
  }
  // Runtime arrays also cannot contain runtime arrays.
  if (containedType->mRaverieType->TemplateBaseName == SpirVNameSettings::mRuntimeArrayTypeName)
  {
    SendTranslationError(node->Location, "Runtime arrays cannot contain runtime arrays");
    return;
  }

  // Otherwise we can create the runtime array (which is the same as creating a global)
  AddGlobalVariable(node, varType, fieldMeta, spv::StorageClassStorageBuffer, context);
}

void RaverieSpirVFrontEnd::AddGlobalVariable(Raverie::MemberVariableNode* node,
                                           RaverieShaderIRType* varType,
                                           ShaderIRFieldMeta* fieldMeta,
                                           spv::StorageClass storageClass,
                                           RaverieSpirVFrontEndContext* context)
{
  String varName = node->Name.Token;
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  // Error check to make sure this global was only created once
  ErrorIf(mLibrary->mRaverieFieldToGlobalVariable.ContainsKey(node->CreatedField), "Global variable already exists");

  GlobalVariableData* globalData = nullptr;
  // Check if this field is marked as being shared across fragments
  if (fieldMeta->ContainsAttribute(nameSettings.mFragmentSharedAttribute))
  {
    // Find if this shared field was already created
    // (use the raw field name instead of a mangled name by fragment type)
    FragmentSharedKey fragmentSharedKey(storageClass, varType, varName);
    globalData = mLibrary->FindFragmentSharedVariable(fragmentSharedKey);
    // If not, create the variable and map it by its key
    if (globalData == nullptr)
    {
      globalData = CreateGlobalVariable(storageClass, varType, varName, context);
      mLibrary->mFragmentSharedGlobalVariables[fragmentSharedKey] = globalData;
    }
  }
  // Otherwise always create the field
  else
  {
    // Update the name to be a proper property name (mangled by fragment name). Purely for debugging.
    varName = GenerateSpirVPropertyName(varName, context->mCurrentType);
    globalData = CreateGlobalVariable(storageClass, varType, varName, context);
  }

  // In either case, record how to look up the global variable from the field.
  // This is necessary when we come across a member access later.
  mLibrary->mRaverieFieldToGlobalVariable[node->CreatedField] = globalData;
}

GlobalVariableData* RaverieSpirVFrontEnd::CreateGlobalVariable(spv::StorageClass storageClass,
                                                             RaverieShaderIRType* varType,
                                                             StringParam varName,
                                                             RaverieSpirVFrontEndContext* context)
{
  // Get the pointer type of this variable. We need to make sure the pointer types is
  // of the correct storage class (e.g. globals have to be Private, uniforms, etc...)
  RaverieShaderIRType* pointerType = varType->mPointerType;
  if (pointerType->mStorageClass != storageClass)
    pointerType = FindOrCreatePointerInterfaceType(mLibrary, varType, storageClass);

  // Make a member variable with the specified storage class and name
  RaverieShaderIROp* globalVar = BuildIROpNoBlockAdd(OpType::OpVariable, pointerType, context);
  globalVar->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(storageClass));
  globalVar->mDebugResultName = varName;

  // Create global variable data to store the instance and
  // initializer function (by default, the function is empty)
  GlobalVariableData* globalData = new GlobalVariableData();
  globalData->mInstance = globalVar;

  // Mark that this library owns the global
  mLibrary->mOwnedGlobals.PushBack(globalData);

  // Always map the variable for the backend so it can know if how to
  // find the initializer function for a random op-code (if it's global)
  mLibrary->mVariableOpLookupMap[globalVar] = globalData;

  return globalData;
}

void RaverieSpirVFrontEnd::PreWalkClassConstructor(Raverie::ConstructorNode*& node, RaverieSpirVFrontEndContext* context)
{
  GenerateIRFunction(node, &node->Attributes, context->mCurrentType, node->DefinedFunction, "Constructor", context);
}

void RaverieSpirVFrontEnd::PreWalkClassFunction(Raverie::FunctionNode*& node, RaverieSpirVFrontEndContext* context)
{
  GenerateIRFunction(node, &node->Attributes, context->mCurrentType, node->DefinedFunction, node->Name.Token, context);

  // Try and parse the correct "Main" function for the current fragment type
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  if (node->Name.Token == nameSettings.mMainFunctionName)
    PreWalkMainFunction(node, context);
}

void RaverieSpirVFrontEnd::PreWalkMainFunction(Raverie::FunctionNode*& node, RaverieSpirVFrontEndContext* context)
{
  // If this is a geometry fragment, try and find the function Main(inputStream,
  // outputStream)
  RaverieShaderIRType* currentType = context->mCurrentType;
  FragmentType::Enum fragmentType = currentType->mMeta->mFragmentType;
  if (fragmentType == FragmentType::Geometry)
  {
    if (node->Parameters.Size() == 2)
    {
      RaverieShaderIRType* inputType = FindType(node->Parameters[0]);
      RaverieShaderIRType* outputType = FindType(node->Parameters[1]);

      Raverie::GeometryStreamUserData* inputUserData = inputType->mRaverieType->Has<Raverie::GeometryStreamUserData>();
      Raverie::GeometryStreamUserData* outputUserData = outputType->mRaverieType->Has<Raverie::GeometryStreamUserData>();

      // Validate that the parameters are the correct input/output types.
      if (inputUserData == nullptr || inputUserData->mInput == false)
        return;
      if (outputUserData == nullptr || outputUserData->mInput == true)
        return;
      // Check for void return type
      if (node->ReturnType != nullptr && node->ReturnType->ResolvedType != RaverieTypeId(void))
        return;

      // Write out user data to the type so the compositor can know what the
      // main function looks like.
      Raverie::HandleOf<Raverie::GeometryFragmentUserData> handle = RaverieAllocate(Raverie::GeometryFragmentUserData);
      handle->mInputStreamType = inputType;
      handle->mOutputStreamType = outputType;
      context->mCurrentType->mRaverieType->Add(*handle);
      currentType->mHasMainFunction = true;
    }
  }
  else if (fragmentType == FragmentType::Vertex || fragmentType == FragmentType::Pixel ||
           fragmentType == FragmentType::Compute)
  {
    if (node->Parameters.Size() != 0)
      return;
    if (node->ReturnType == nullptr || node->ReturnType->ResolvedType == RaverieTypeId(void))
      currentType->mHasMainFunction = true;
  }
}

void RaverieSpirVFrontEnd::PreWalkErrorCheck(RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* currentType = context->mCurrentType;
  ShaderIRTypeMeta* typeMeta = currentType->mMeta;

  if (mSettings->mErrorSettings.mFrontEndErrorOnNoMainFunction)
  {
    if (typeMeta->mFragmentType == FragmentType::Geometry)
    {
      if (!currentType->mHasMainFunction)
      {
        String msg = "Geometry shader must have a 'Main' function of signature "
                     "(InputStream, OutputStream).";
        SendTranslationError(currentType->mRaverieType->Location, msg);
      }
    }
    else if (typeMeta->mFragmentType != FragmentType::None)
    {
      if (!currentType->mHasMainFunction)
      {
        String msg = "Shader must have a function of signature 'Main()'.";
        SendTranslationError(currentType->mRaverieType->Location, msg);
      }
    }
  }
}

void RaverieSpirVFrontEnd::WalkClassNode(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* type = FindType(node->Type, node);
  context->mCurrentType = type;

  mWalker.Walk(this, node->Variables, context);
  mWalker.Walk(this, node->Constructors, context);
  mWalker.Walk(this, node->Functions, context);

  context->mCurrentType = nullptr;
}

void RaverieSpirVFrontEnd::WalkClassVariables(Raverie::MemberVariableNode*& node, RaverieSpirVFrontEndContext* context)
{
  // This variable is actually a getter setter. Walk it's functions instead
  if (node->CreatedGetterSetter != nullptr)
  {
    if (node->Get != nullptr)
      WalkClassFunction(node->Get, context);
    if (node->Set != nullptr)
      WalkClassFunction(node->Set, context);
    return;
  }

  if (node->IsStatic)
    GenerateStaticVariableInitializer(node, context);
}

void RaverieSpirVFrontEnd::GeneratePreConstructor(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* currentType = context->mCurrentType;
  Raverie::Function* raverieFunction = node->PreConstructor;
  RaverieShaderIRFunction* function =
      GenerateIRFunction(node, nullptr, context->mCurrentType, raverieFunction, raverieFunction->Name, context);

  BasicBlock* currentBlock = BuildBlock(String(), context);
  context->mCurrentBlock = currentBlock;

  // Declare the self param
  RaverieShaderIROp* selfOp =
      BuildIROp(&function->mParameterBlock, OpType::OpFunctionParameter, currentType->mPointerType, context);
  selfOp->mDebugResultName = "self";

  // Generate the default initializer values for all member variables
  for (size_t i = 0; i < node->Variables.Size(); ++i)
  {
    Raverie::VariableNode* varNode = node->Variables[i];
    String varName = varNode->Name.Token;

    // If for some reason this variable isn't a member (e.g. samplers) then skip
    // initialization. This variable was probably promoted to a global.
    if (currentType->mMemberNamesToIndex.ContainsKey(varName) == false)
      continue;

    // Generate a pointer to the member variable
    int memberIndex = currentType->mMemberNamesToIndex[varName];
    RaverieShaderIRType* memberType = currentType->GetSubType(memberIndex);
    RaverieShaderIROp* offsetConstant = GetIntegerConstant(memberIndex, context);
    RaverieShaderIROp* memberPtrOp =
        BuildIROp(currentBlock, OpType::OpAccessChain, memberType->mPointerType, selfOp, offsetConstant, context);

    // If the variable has an initializer then walk it and set the variable
    if (varNode->InitialValue != nullptr)
    {
      IRaverieShaderIR* initialValue = WalkAndGetResult(varNode->InitialValue, context);
      RaverieShaderIROp* valueOp = GetOrGenerateValueTypeFromIR(initialValue, context);
      BuildStoreOp(memberPtrOp, initialValue, context);
    }
    // Otherwise call the default constructor for this type
    else
    {
      DefaultConstructType(varNode, memberType, memberPtrOp, context);
    }
  }

  // Generate the required terminator op (return)
  currentBlock->mTerminatorOp = BuildIROp(currentBlock, OpType::OpReturn, nullptr, context);
  context->mCurrentBlock = nullptr;
}

void RaverieSpirVFrontEnd::GenerateDefaultConstructor(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context)
{
  // If the type already has a default-constructor then don't auto-generate one.
  // Otherwise we have to generate one just to call the pre-constructor.
  if (node->Type->GetDefaultConstructor() != nullptr)
    return;

  RaverieShaderIRType* currentType = context->mCurrentType;

  // Generate the function
  RaverieShaderIRFunction* function = currentType->CreateFunction(mLibrary);
  function->mName = "DefaultConstructor";
  function->mDebugResultName = function->mName;
  context->mCurrentFunction = function;

  // We need to generate the type of a function (delegate type).
  // Just like other types, this needs to be generated only once but we
  // unfortunately don't have a backing raverie function so manually generate the
  // signature.
  Array<Raverie::Type*> signature;
  signature.PushBack(node->Type->IndirectType);
  GenerateFunctionType(node, function, RaverieTypeId(void), signature, context);

  ShaderIRFunctionMeta* functionMeta = currentType->mMeta->CreateFunction(mLibrary);
  functionMeta->mRaverieName = function->mName;
  function->mMeta = functionMeta;

  // Make sure to add the self parameter
  RaverieShaderIROp* selfOp =
      BuildIROp(&function->mParameterBlock, OpType::OpFunctionParameter, currentType->mPointerType, context);
  selfOp->mDebugResultName = "self";

  // Begin the block of instructions for the function
  BasicBlock* currentBlock = BuildBlock(String(), context);
  context->mCurrentBlock = currentBlock;

  // Manually invoke the pre-constructor
  RaverieShaderIRFunction* preConstructorFn = mLibrary->mFunctions.FindValue(node->PreConstructor, nullptr);
  Array<IRaverieShaderIR*> arguments;
  arguments.PushBack(selfOp);
  WriteFunctionCall(arguments, preConstructorFn, context);
  currentType->mAutoDefaultConstructor = function;

  // Manually generate the terminator
  currentBlock->mTerminatorOp = BuildIROp(currentBlock, OpType::OpReturn, nullptr, context);
}

void RaverieSpirVFrontEnd::GenerateDummyMemberVariable(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* currentType = context->mCurrentType;

  // If the type has parameters then we don't need to generate a dummy variable
  if (!currentType->mParameters.Empty())
    return;

  // Otherwise, generate a dummy int on this class
  Raverie::Type* dummyType = RaverieTypeId(int);
  String dummyName = "Dummy";
  RaverieShaderIRType* memberType = FindType(dummyType, node);

  ShaderIRFieldMeta* fieldMeta = currentType->mMeta->CreateField(mLibrary);
  fieldMeta->mRaverieName = dummyName;
  fieldMeta->mRaverieType = memberType->mRaverieType;

  // Only actual add the member if this member belongs to the class (not global)
  currentType->AddMember(memberType, dummyName);
}

void RaverieSpirVFrontEnd::GenerateStaticVariableInitializer(Raverie::MemberVariableNode*& node,
                                                           RaverieSpirVFrontEndContext* context)
{
  // Ignore specialization constants. They're global but they can't have an
  // initializer function.
  if (mLibrary->FindSpecializationConstantOp(node->CreatedField) != nullptr)
    return;

  // Find the global variable data
  GlobalVariableData* globalData = mLibrary->FindGlobalVariable(node->CreatedField);
  ReturnIf(globalData == nullptr, , "Global variable data doesn't exist");

  // Generate the function
  RaverieShaderIRFunction* shaderFunction = new RaverieShaderIRFunction();
  // Name the function "`OwningType`_`VarName`_Initializer"
  shaderFunction->mName = BuildString(context->mCurrentType->mName, "_", node->Name.Token, "_Initializer");
  shaderFunction->mDebugResultName = shaderFunction->mName;
  // Generate the meta for the function
  ShaderIRFunctionMeta* functionMeta = new ShaderIRFunctionMeta();
  mLibrary->mOwnedFunctionMeta.PushBack(functionMeta);
  mLibrary->mOwnedFunctions.PushBack(shaderFunction);
  shaderFunction->mMeta = functionMeta;
  // Generate the function type. The signature is () : Void.
  Array<Raverie::Type*> signature;
  GenerateFunctionType(node, shaderFunction, RaverieTypeId(void), signature, context);

  // Mark the initializer function
  globalData->mInitializerFunction = shaderFunction;
  // Create the initial block
  BasicBlock* currentBlock = new BasicBlock();
  shaderFunction->mBlocks.PushBack(currentBlock);

  // Mark the context to operate on the current block and function
  context->mCurrentBlock = currentBlock;
  context->mCurrentFunction = shaderFunction;

  // If the variable has an initializer then walk it and set the variable
  if (node->InitialValue != nullptr)
  {
    IRaverieShaderIR* initialValue = WalkAndGetResult(node->InitialValue, context);
    BuildStoreOp(globalData->mInstance, initialValue, context);
  }
  // Otherwise call the default constructor for this type
  else
  {
    RaverieShaderIROp* globalVarInstance = globalData->mInstance;
    RaverieShaderIRType* globalVarValueType = globalVarInstance->mResultType->mDereferenceType;
    DefaultConstructType(node, globalVarValueType, globalVarInstance, context);
  }
  FixBlockTerminators(currentBlock, context);
}

void RaverieSpirVFrontEnd::WalkClassConstructor(Raverie::ConstructorNode*& node, RaverieSpirVFrontEndContext* context)
{
  GenerateFunctionParameters(node, context);

  RaverieShaderIRType* currentType = context->mCurrentType;
  BasicBlock* currentBlock = context->GetCurrentBlock();
  RaverieShaderIRFunction* preConstructorFn = mLibrary->FindFunction(currentType->mRaverieType->PreConstructor);

  // Manually invoke the pre-constructor
  IRaverieShaderIR* selfType = context->mRaverieVariableToIR[node->DefinedFunction->This];
  Array<IRaverieShaderIR*> arguments;
  arguments.PushBack(selfType);
  WriteFunctionCall(arguments, preConstructorFn, context);

  GenerateFunctionBody(node, context);
}

void RaverieSpirVFrontEnd::WalkClassFunction(Raverie::FunctionNode*& node, RaverieSpirVFrontEndContext* context)
{
  GenerateFunctionParameters(node, context);
  GenerateFunctionBody(node, context);
}

void RaverieSpirVFrontEnd::DefaultConstructType(Raverie::SyntaxNode* locationNode,
                                              RaverieShaderIRType* type,
                                              RaverieShaderIROp* selfVar,
                                              RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  // If this type has an auto-generated default constructor then call it
  RaverieShaderIRFunction* autoDefaultConstructor = type->mAutoDefaultConstructor;
  if (autoDefaultConstructor != nullptr)
  {
    RaverieShaderIRType* returnType = autoDefaultConstructor->GetReturnType();
    Array<IRaverieShaderIR*> arguments;
    arguments.PushBack(selfVar);
    WriteFunctionCall(arguments, autoDefaultConstructor, context);
    return;
  }

  // If we can find a default constructor resolver for this type then call it
  // (e.g. Real)
  TypeResolvers* typeResolver = mLibrary->FindTypeResolver(type->mRaverieType);
  if (typeResolver != nullptr && typeResolver->mDefaultConstructorResolver != nullptr)
  {
    typeResolver->mDefaultConstructorResolver(this, type->mRaverieType, context);
    IRaverieShaderIR* resultIR = context->PopIRStack();
    RaverieShaderIROp* valueOp = GetOrGenerateValueTypeFromIR(resultIR, context);
    BuildStoreOp(currentBlock, selfVar, valueOp, context);
    return;
  }

  // Check if this type is forced to be a global type. If so then ignore the
  // default constructor because this class doesn't actually "own" the variable.
  if (type->IsGlobalType())
    return;

  SendTranslationError(locationNode->Location, "Couldn't default construct type '%s'", type->mName.c_str());
}

void RaverieSpirVFrontEnd::GenerateFunctionParameters(Raverie::GenericFunctionNode* node,
                                                    RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* currentType = context->mCurrentType;
  // Debug sanity
  context->mCurrentBlock = nullptr;

  // Get the shader function defined for this raverie function
  Raverie::Function* raverieFunction = node->DefinedFunction;
  RaverieShaderIRFunction* function = mLibrary->FindFunction(raverieFunction, false);
  ErrorIf(function == nullptr, "Class function wasn't already created");
  context->mCurrentFunction = function;

  // Generate a function's parameter block. This contains the declarations and
  // ids for all input parameters

  // First check if this is a member function. If so then we need to declare
  // that we take in the this pointer.
  if (!raverieFunction->IsStatic)
  {
    // Get the actual 'this' type from the function (deals with extension
    // methods)
    Raverie::BoundType* raverieThisType = raverieFunction->Owner;
    RaverieShaderIRType* thisType = FindType(raverieThisType, node)->mPointerType;
    RaverieShaderIROp* op = BuildIROp(&function->mParameterBlock, OpType::OpFunctionParameter, thisType, context);
    op->mDebugResultName = "self";
    // Map the 'this' variable to the generated this parameter
    context->mRaverieVariableToIR[node->DefinedFunction->This] = op;
  }

  BasicBlock* currentBlock = BuildBlock(String(), context);

  // Add all parameters
  for (size_t i = 0; i < node->Parameters.Size(); ++i)
  {
    Raverie::ParameterNode* parameter = node->Parameters[i];

    RaverieShaderIRType* shaderParameterType = FindType(parameter->ResultType, parameter);
    RaverieShaderIRType* shaderParamPointerType = shaderParameterType->mPointerType;

    if (parameter->ResultType->IsIndirectionType(parameter->ResultType))
      shaderParameterType = shaderParamPointerType;

    // @JoshD: Fix later. We should declare variables for all non-pointer
    // types as the beginning statements so that the signature is actually
    // correct.

    // We take all parameters by pointer type. This makes it significantly
    // easier to generate code in case the user ever assigns the the input
    RaverieShaderIROp* op =
        BuildIROp(&function->mParameterBlock, OpType::OpFunctionParameter, shaderParameterType, context);

    if (!shaderParameterType->IsPointerType())
    {
      RaverieShaderIROp* varOp = BuildOpVariable(shaderParamPointerType, context);
      varOp->mDebugResultName = BuildString(parameter->Name.Token, "_Local");
      BuildStoreOp(currentBlock, varOp, op, context);

      context->mRaverieVariableToIR.Insert(parameter->CreatedVariable, varOp);
    }
    else
    {
      context->mRaverieVariableToIR.Insert(parameter->CreatedVariable, op);
    }

    op->mDebugResultName = parameter->Name.Token;
  }

  // Create the first block in the function

  context->mCurrentBlock = currentBlock;
}

void RaverieSpirVFrontEnd::GenerateFunctionBody(Raverie::GenericFunctionNode* node, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* currentType = context->mCurrentType;
  RaverieShaderIRFunction* function = context->mCurrentFunction;
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Start walking all statements. This may create new blocks
  mWalker.Walk(this, node->Statements, context);

  // Fix all blocks to have exactly one terminator.
  for (size_t i = 0; i < function->mBlocks.Size(); ++i)
    FixBlockTerminators(function->mBlocks[i], context);

  // Cleanup the context
  context->mCurrentFunction = nullptr;
  context->mCurrentBlock = nullptr;
  // Clear mappings of raverie variables to shader variable declarations
  // since the all local variables are now out of scope.
  context->mRaverieVariableToIR.Clear();

  // Generate actual spirv entry point for this function.
  if (function->mMeta->ContainsAttribute("EntryPoint"))
  {
    GenerateEntryPoint(node, function, context);
  }
}

void RaverieSpirVFrontEnd::GenerateEntryPoint(Raverie::GenericFunctionNode* node,
                                            RaverieShaderIRFunction* function,
                                            RaverieSpirVFrontEndContext* context)
{
  // Run some error checking on the entry point function.
  ValidateEntryPoint(this, node, context);

  // If we had an error don't try to generate the actual entry function.
  if (mErrorTriggered)
    return;

  RaverieShaderIRType* currentType = context->mCurrentType;
  FragmentType::Enum fragmentType = currentType->mMeta->mFragmentType;
  EntryPointGeneration entryPointGeneration;

  if (fragmentType == FragmentType::Pixel)
    entryPointGeneration.DeclarePixelInterface(this, node, function, context);
  else if (fragmentType == FragmentType::Vertex)
    entryPointGeneration.DeclareVertexInterface(this, node, function, context);
  else if (fragmentType == FragmentType::Geometry)
    entryPointGeneration.DeclareGeometryInterface(this, node, function, context);
  else if (fragmentType == FragmentType::Compute)
    entryPointGeneration.DeclareComputeInterface(this, node, function, context);
}

void RaverieSpirVFrontEnd::WalkFunctionCallNode(Raverie::FunctionCallNode*& node, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Check if this is a constructor call
  Raverie::StaticTypeNode* constructorNode = Raverie::Type::DynamicCast<Raverie::StaticTypeNode*>(node->LeftOperand);
  if (constructorNode != nullptr)
  {
    WalkConstructorCallNode(node, constructorNode, context);
    return;
  }

  // Check if this is a member access function call (could be a function,
  // member, etc...)
  Raverie::MemberAccessNode* memberAccessNode = Raverie::Type::DynamicCast<Raverie::MemberAccessNode*>(node->LeftOperand);
  if (memberAccessNode)
  {
    WalkMemberAccessCallNode(node, memberAccessNode, context);
    return;
  }
}

void RaverieSpirVFrontEnd::WalkConstructorCallNode(Raverie::FunctionCallNode*& node,
                                                 Raverie::StaticTypeNode* constructorNode,
                                                 RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Check for library constructor translation (e.g. Real3())
  ConstructorCallResolverIRFn resolver =
      mLibrary->FindConstructorResolver(node->LeftOperand->ResultType, constructorNode->ConstructorFunction);
  if (resolver != nullptr)
  {
    resolver(this, node, constructorNode, context);
    return;
  }

  // Otherwise assume we can walk all arguments to translate
  RaverieShaderIRType* resultType = FindType(node->ResultType, node);

  RaverieShaderIRFunction* shaderConstructorFn = nullptr;
  // Deal with auto-generated default constructors (they don't have a generated
  // raverie function)
  if (constructorNode->ConstructorFunction == nullptr && resultType->mAutoDefaultConstructor != nullptr)
    shaderConstructorFn = resultType->mAutoDefaultConstructor;
  // Otherwise, look up the constructor function
  else
    shaderConstructorFn = mLibrary->FindFunction(constructorNode->ConstructorFunction);

  // If we got a shader function then resolve it
  if (shaderConstructorFn != nullptr)
  {
    RaverieShaderIROp* classVarOp = BuildOpVariable(resultType->mPointerType, context);

    // Collect the arguments and generate the function call
    Array<IRaverieShaderIR*> arguments;
    GetFunctionCallArguments(node, classVarOp, arguments, context);
    WriteFunctionCall(arguments, shaderConstructorFn, context);

    context->PushIRStack(classVarOp);
    return;
  }

  SendTranslationError(node->Location, "Failed to translation constructor call");
  context->PushIRStack(GenerateDummyIR(node, context));
}

void RaverieSpirVFrontEnd::WalkMemberAccessCallNode(Raverie::FunctionCallNode*& node,
                                                  Raverie::MemberAccessNode* memberAccessNode,
                                                  RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  Raverie::Function* raverieFunction = memberAccessNode->AccessedFunction;
  // Build the function call op but don't add it as a line in the current block
  // yet. We have to generate a bit more code to reference the parameters
  RaverieShaderIRFunction* shaderFunction = mLibrary->FindFunction(raverieFunction);
  if (shaderFunction != nullptr)
  {
    WalkMemberAccessFunctionCallNode(node, memberAccessNode, shaderFunction, context);
    return;
  }
  // Check for a member function resolver. If we find one on the 'this' type
  // then leave translation up to it.
  Raverie::Type* selfType = memberAccessNode->LeftOperand->ResultType;
  // Make sure to handle indirection types otherwise resolvers won't be found
  // (e.g. turn 'Array ref' into 'Array')
  selfType = Raverie::Type::GetBoundType(selfType);
  MemberFunctionResolverIRFn fnResolver = mLibrary->FindFunctionResolver(selfType, raverieFunction);
  if (fnResolver != nullptr)
  {
    fnResolver(this, node, memberAccessNode, context);
    return;
  }

  SpirVExtensionInstruction* extension = mLibrary->FindExtensionInstruction(raverieFunction);
  if (extension != nullptr)
  {
    WalkMemberAccessExtensionInstructionCallNode(node, memberAccessNode, extension, context);
    return;
  }

  String errorMsg = String::Format("Failed to translation function call: '%s'", memberAccessNode->Name.c_str());
  SendTranslationError(node->Location, errorMsg);
  context->PushIRStack(GenerateDummyIR(node, context));
}

void RaverieSpirVFrontEnd::WalkMemberAccessFunctionCallNode(Raverie::FunctionCallNode*& node,
                                                          Raverie::MemberAccessNode* memberAccessNode,
                                                          RaverieShaderIRFunction* shaderFunction,
                                                          RaverieSpirVFrontEndContext* context)
{
  // Fill out an array with all of the arguments this function takes
  Array<IRaverieShaderIR*> arguments;
  GetFunctionCallArguments(node, memberAccessNode, arguments, context);

  // Now generate a function call from the arguments
  WriteFunctionCall(arguments, shaderFunction, context);
}

void RaverieSpirVFrontEnd::WalkMemberAccessExtensionInstructionCallNode(Raverie::FunctionCallNode*& node,
                                                                      Raverie::MemberAccessNode* memberAccessNode,
                                                                      SpirVExtensionInstruction* extensionInstruction,
                                                                      RaverieSpirVFrontEndContext* context)
{
  RaverieShaderExtensionImport* importLibraryIR = nullptr;
  importLibraryIR = mLibrary->FindExtensionLibraryImport(extensionInstruction->mLibrary);
  if (importLibraryIR == nullptr)
  {
    importLibraryIR = new RaverieShaderExtensionImport(extensionInstruction->mLibrary);
    mLibrary->mExtensionLibraryImports.InsertOrError(extensionInstruction->mLibrary, importLibraryIR);
  }

  if (extensionInstruction->mResolverFn != nullptr)
  {
    extensionInstruction->mResolverFn(this, node, memberAccessNode, importLibraryIR, context);
    return;
  }

  // This should never happen unless we registered a resolver that was null.
  String errorMsg =
      String::Format("Failed to translation extension function call: '%s'", memberAccessNode->Name.c_str());
  SendTranslationError(node->Location, errorMsg);
  context->PushIRStack(GenerateDummyIR(node, context));
}

void RaverieSpirVFrontEnd::WalkLocalVariable(Raverie::LocalVariableNode*& node, RaverieSpirVFrontEndContext* context)
{
  // Should this variable declaration be forwarded to another variable? (Used in
  // expression initializers). If so, we just need to walk the variable this is
  // referencing and return and mark that.
  if (node->ForwardLocalAccessIfPossible)
  {
    mWalker.Walk(this, node->InitialValue, context);

    // Make sure this is a pointer otherwise make a new variable to reference
    // (not a pointer if the initial value is a temp)
    IRaverieShaderIR* intialValueIR = context->PopIRStack();
    intialValueIR = GetOrGeneratePointerTypeFromIR(context->GetCurrentBlock(), intialValueIR, context);
    context->mRaverieVariableToIR[node->CreatedVariable] = intialValueIR;
    context->PushIRStack(intialValueIR);
    return;
  }

  // Since we're declaring a member variable we have to mark a dependency on the
  // variable type
  RaverieShaderIRType* resultShaderType = FindType(node->ResultType, node);
  RaverieShaderIRType* resultShaderPointerType = resultShaderType->mPointerType;

  // Build a variable declaration for the pointer type of this variable
  RaverieShaderIROp* variableIR = BuildOpVariable(resultShaderPointerType, context);
  variableIR->mDebugResultName = node->Name.Token;
  // Map this variable so that anything else that references
  // it later knows how to get the ir for the declaration
  context->mRaverieVariableToIR[node->CreatedVariable] = variableIR;

  // If there's no initial value then we're done (we've already declared the
  // variable)
  if (node->InitialValue == nullptr)
    return;

  // Check if this type is non-copyable. If so we can't assign a default value
  // (generate an error)
  if (CheckForNonCopyableType(resultShaderType, node, context))
    return;

  mWalker.Walk(this, node->InitialValue, context);
  IRaverieShaderIR* intialValueIR = context->PopIRStack();

  // Validate that we can assign to the type (the result could be void or
  // something else)
  RaverieShaderIROp* intialValueOp = intialValueIR->As<RaverieShaderIROp>();
  if (intialValueOp == nullptr || resultShaderType->mBaseType == ShaderIRTypeBaseType::Void)
    return;

  // Assign the initial value (have to get the current block here as
  // walking the initial value can change the current block)
  BasicBlock* currentBlock = context->GetCurrentBlock();
  BuildStoreOp(currentBlock, variableIR, intialValueIR, context);
}

void RaverieSpirVFrontEnd::WalkStaticTypeOrCreationCallNode(Raverie::StaticTypeNode*& node,
                                                          RaverieSpirVFrontEndContext* context)
{
  SendTranslationError(node->Location, "StaticTypeOrCreationCallNode not translatable.");
  context->PushIRStack(GenerateDummyIR(node, context));
}

void RaverieSpirVFrontEnd::WalkExpressionInitializerNode(Raverie::ExpressionInitializerNode*& node,
                                                       RaverieSpirVFrontEndContext* context)
{
  // Check if we have an expression initializer list resolver (e.g. FixedArray)
  TypeResolvers* typeResolver = mLibrary->FindTypeResolver(node->ResultType);
  if (typeResolver != nullptr && typeResolver->mExpressionInitializerListResolver != nullptr)
  {
    typeResolver->mExpressionInitializerListResolver(this, node, context);
    return;
  }

  mWalker.Walk(this, node->LeftOperand, context);
  mWalker.Walk(this, node->InitializerStatements, context);
}

void RaverieSpirVFrontEnd::WalkUnaryOperationNode(Raverie::UnaryOperatorNode*& node, RaverieSpirVFrontEndContext* context)
{
  // If this is a dereference operator then generically generate an OpLoad to
  // turn a pointer type into a value type (should only be used in extension
  // methods that need to pass 'this' into a function)
  if (node->Operator->TokenId == Raverie::Grammar::Dereference)
  {
    IRaverieShaderIR* operandResult = WalkAndGetResult(node->Operand, context);
    RaverieShaderIROp* operand = operandResult->As<RaverieShaderIROp>();
    // Validate this is a pointer type
    if (!operand->mResultType->IsPointerType())
    {
      SendTranslationError(node->Operand->Location, "Operand must be pointer type");
      context->PushIRStack(GenerateDummyIR(node, context));
      return;
    }
    RaverieShaderIROp* dereferenceOp =
        BuildCurrentBlockIROp(OpType::OpLoad, operand->mResultType->mDereferenceType, operand, context);
    context->PushIRStack(dereferenceOp);
    return;
  }

  // If this is an address of operator then this is almost always &variable
  // which just returns the variable pointer itself. The only time this isn't a
  // pointer type is on the result of an expression such as &(a + b) which I
  // will currently always report as an error.
  if (node->Operator->TokenId == Raverie::Grammar::AddressOf)
  {
    IRaverieShaderIR* operandResult = WalkAndGetResult(node->Operand, context);
    RaverieShaderIROp* operand = operandResult->As<RaverieShaderIROp>();
    if (!operand->mResultType->IsPointerType())
    {
      SendTranslationError(node->Operand->Location, "Cannot take the address of a temporary");
      context->PushIRStack(GenerateDummyIR(node, context));
    }
    else
      context->PushIRStack(operand);
    return;
  }

  // If the operand type is an enum then treat it like an integer
  Raverie::Type* operandType = node->Operand->ResultType;
  if (operandType->IsEnumOrFlags())
    operandType = RaverieTypeId(int);

  // Find and use a resolver if we have one
  UnaryOperatorKey opKey = UnaryOperatorKey(operandType, node->OperatorInfo.Operator);
  UnaryOpResolverIRFn unaryOpResolver = mLibrary->FindOperatorResolver(opKey);
  if (unaryOpResolver != nullptr)
  {
    unaryOpResolver(this, node, context);
    return;
  }

  // Report an error if we failed to find a translation for this unary operator
  SendTranslationError(node->Location, "Unary operator not supported");
  context->PushIRStack(GenerateDummyIR(node, context));
}

void RaverieSpirVFrontEnd::WalkBinaryOperationNode(Raverie::BinaryOperatorNode*& node, RaverieSpirVFrontEndContext* context)
{
  // Special case assignment (always just copy the values)
  if (node->OperatorInfo.Operator == Raverie::Grammar::Assignment)
  {
    // Deal with setters. This requires 'promoting' the setter above the
    // assignment op (e.g. A = B -> A.Set(B))
    if (ResolveSetter(node, nullptr, node->RightOperand, context))
      return;

    IRaverieShaderIR* leftIR = WalkAndGetResult(node->LeftOperand, context);
    IRaverieShaderIR* rightIR = WalkAndGetResult(node->RightOperand, context);

    // Check if the left hand side is non-copyable, if so generate an error.
    RaverieShaderIROp* leftOp = leftIR->As<RaverieShaderIROp>();
    if (CheckForNonCopyableType(leftOp->mResultType, node, context))
      return;

    // Validate that we can write to this op
    ValidateLValue(leftOp, node->Location);
    // Validate that the left hand side is a pointer type otherwise we can't
    // store to it
    ValidateResultType(leftIR, ShaderIRTypeBaseType::Pointer, node->Location);

    // Generate the store
    BasicBlock* currentBlock = context->GetCurrentBlock();
    BuildStoreOp(currentBlock, leftIR, rightIR, context);
    return;
  }

  // If any operand type is an enum then treat it like an integer
  Raverie::Type* leftType = node->LeftOperand->ResultType;
  Raverie::Type* rightType = node->RightOperand->ResultType;
  if (leftType->IsEnumOrFlags())
    leftType = RaverieTypeId(int);
  if (rightType->IsEnumOrFlags())
    rightType = RaverieTypeId(int);

  // Find a resolver for the given binary op
  BinaryOperatorKey opKey = BinaryOperatorKey(leftType, rightType, node->OperatorInfo.Operator);
  BinaryOpResolverIRFn binaryOpResolver = mLibrary->FindOperatorResolver(opKey);
  if (binaryOpResolver != nullptr)
  {
    binaryOpResolver(this, node, context);
    return;
  }

  // Report an error if we failed to find a translation for this binary operator
  SendTranslationError(node->Location, "Binary operator not supported");
  context->PushIRStack(GenerateDummyIR(node, context));
}

void RaverieSpirVFrontEnd::WalkCastNode(Raverie::TypeCastNode*& node, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // If the operand or result type is an enum then treat it like an integer
  Raverie::Type* operatorType = node->Operand->ResultType;
  Raverie::Type* resultType = node->ResultType;
  if (operatorType->IsEnumOrFlags())
    operatorType = RaverieTypeId(int);
  if (resultType->IsEnumOrFlags())
    resultType = RaverieTypeId(int);

  // Cast to same type. Do a no-op
  if (operatorType == resultType)
  {
    RaverieShaderIROp* operandValueResult = WalkAndGetValueTypeResult(node->Operand, context);
    context->PushIRStack(operandValueResult);
    return;
  }

  // Find a resolver for the cast operator
  TypeCastKey castOpKey(operatorType, resultType);
  TypeCastResolverIRFn resolverFn = mLibrary->FindOperatorResolver(castOpKey);
  if (resolverFn != nullptr)
  {
    resolverFn(this, node, context);
    return;
  }

  // Report an error if we failed to find a translation for this cast operator
  SendTranslationError(node->Location, "Cast operator not supported");
  context->PushIRStack(GenerateDummyIR(node, context));
}

void RaverieSpirVFrontEnd::WalkValueNode(Raverie::ValueNode*& node, RaverieSpirVFrontEndContext* context)
{
  // @JoshD: This probably has to change later with templates (the type might
  // not exist)

  // Find the result type and mark this function as relying on it
  RaverieShaderIRType* resultType = FindType(node->ResultType, node);
  ErrorIf(resultType == nullptr, "No type");

  // Get the constant that represents this value of the given type.
  // This might create a new constant in the constant pool.
  RaverieShaderIROp* opConstant = GetConstant(resultType, node->Value.Token, context);
  // Mark the constant as the result of this value node expression
  context->PushIRStack(opConstant);
}

void RaverieSpirVFrontEnd::WalkLocalRef(Raverie::LocalVariableReferenceNode*& node, RaverieSpirVFrontEndContext* context)
{
  IRaverieShaderIR* variableOp = context->mRaverieVariableToIR.FindValue(node->AccessedVariable, nullptr);
  ErrorIf(variableOp == nullptr, "Failed to find variable declaration for local reference");

  context->PushIRStack(variableOp);
}

void RaverieSpirVFrontEnd::WalkMemberAccessNode(Raverie::MemberAccessNode*& node, RaverieSpirVFrontEndContext* context)
{
  if (node->AccessedMember)
  {
    RaverieShaderIRType* leftOperandType = FindType(node->LeftOperand->ResultType, node->LeftOperand);

    if (node->AccessedGetterSetter != nullptr)
    {
      // Deal with accessing enums/flags (grab their constant value)
      if (node->AccessedGetterSetter->PropertyType->IsEnumOrFlags())
      {
        RaverieShaderIROp* enumConstant = mLibrary->FindEnumConstantOp(node->AccessedGetterSetter);
        // Sanity check (this should never happen)
        if (enumConstant == nullptr)
        {
          SendTranslationError(node->Location, "Enum unable to be translated");
          context->PushIRStack(GenerateDummyIR(node, context));
          return;
        }

        context->PushIRStack(enumConstant);
        return;
      }

      if (node->IoUsage == Raverie::IoMode::ReadRValue)
      {
        Raverie::Type* ownerType = node->AccessedGetterSetter->Owner;
        Raverie::Function* getFn = node->AccessedGetterSetter->Get;
        MemberFunctionResolverIRFn functionResolver = mLibrary->FindFunctionResolver(ownerType, getFn);
        if (functionResolver != nullptr)
        {
          functionResolver(this, nullptr, node, context);
          return;
        }

        Raverie::Function* raverieGetFunction = node->AccessedGetterSetter->Get;
        RaverieShaderIRFunction* shaderFunction = mLibrary->FindFunction(raverieGetFunction);
        // If this is an existing raverie function then we have to translate this
        // member access into a function call
        if (shaderFunction != nullptr)
        {
          Array<IRaverieShaderIR*> arguments;
          // Pass through 'this' if this is an instance function
          if (!raverieGetFunction->IsStatic)
            arguments.PushBack(WalkAndGetResult(node->LeftOperand, context));

          WriteFunctionCall(arguments, shaderFunction, context);
          return;
        }
      }
      else
      {
        // In the syntax tree setters have to be manually deal with at a higher
        // level (binary ops right now). If we get here then something went
        // wrong.
        Error("Setters should always be hit via a binary op node");
        SendTranslationError(node->Location, "Translation of setters is not supported at this time.");
      }
    }
    else if (node->AccessedField != nullptr)
    {
      // Check if this is a global variable. If so just put the variable on the
      // stack and return (no member access chain). This happens with forced
      // global types like samplers.
      GlobalVariableData* globalVarData = mLibrary->FindGlobalVariable(node->AccessedField);
      if (globalVarData != nullptr)
      {
        context->PushIRStack(globalVarData->mInstance);
        return;
      }

      // Check if this is a specialization constant.
      // This is basically the same as a global except it requires a separate
      // lookup.
      RaverieShaderIROp* specConstant = mLibrary->FindSpecializationConstantOp(node->AccessedField);
      if (specConstant != nullptr)
      {
        context->PushIRStack(specConstant);
        return;
      }
    }
    else
    {
      SendTranslationError(node->Location, "Translation not yet supported");
    }

    // If we failed to translate something more specialized up above then try to
    // find a field resolver. We have to do this last because some types (like
    // vectors) can have backup field resolvers. If we did this first then
    // getters would fail to get called.
    MemberAccessResolverIRFn fieldResolver =
        mLibrary->FindFieldResolver(leftOperandType->mRaverieType, node->AccessedField);
    if (fieldResolver != nullptr)
    {
      fieldResolver(this, node, context);
      return;
    }

    // Find the index and type of the member we're accessing.
    // First map the name to an index.
    String memberName = node->AccessedMember->Name;
    ErrorIf(!leftOperandType->mMemberNamesToIndex.ContainsKey(memberName), "Invalid member name");
    int memberIndex = leftOperandType->mMemberNamesToIndex.FindValue(memberName, 0);
    // Then use that index to get the type of the member
    RaverieShaderIRType* memberType = leftOperandType->GetSubType(memberIndex);

    // Get the member variable pointer. If this is a static variable then we
    // access the globals map, otherwise we walk the left operand and get the
    // variable pointer to this id
    IRaverieShaderIR* operandResult = nullptr;
    if (!node->IsStatic)
      operandResult = WalkAndGetResult(node->LeftOperand, context);
    else
    {
      GlobalVariableData* globalVarData = mLibrary->FindGlobalVariable(node->AccessedField);
      if (globalVarData != nullptr && globalVarData->mInstance != nullptr)
        operandResult = globalVarData->mInstance;
    }

    if (operandResult == nullptr)
    {
      SendTranslationError(node->Location, "Member variable access couldn't be translated");
      context->PushIRStack(GenerateDummyIR(node, context));
      return;
    }

    RaverieShaderIROp* operandResultOp = operandResult->As<RaverieShaderIROp>();

    if (operandResultOp->IsResultPointerType())
    {
      // Make the constant for the sub-index of the member with respect to the
      // base
      RaverieShaderIROp* memberIndexConstant = GetIntegerConstant(memberIndex, context);
      // Generate a member access to reference this member.
      // Note: This must have the same storage class as the left operand.
      RaverieShaderIROp* memberAccessOp =
          BuildCurrentBlockAccessChain(memberType, operandResultOp, memberIndexConstant, context);
      context->PushIRStack(memberAccessOp);
    }
    // @JoshD: Validate (have to find op-code to generate this)
    else
    {
      // Make the constant for the sub-index of the member with respect to the
      // base
      RaverieShaderIRConstantLiteral* memberIndexLiteral = GetOrCreateConstantLiteral(memberIndex);

      // Build the member access operation
      RaverieShaderIROp* memberAccessOp =
          BuildCurrentBlockIROp(OpType::OpCompositeExtract, memberType, operandResultOp, memberIndexLiteral, context);
      context->PushIRStack(memberAccessOp);
    }
  }
  else
  {
    Error("Only member access should reach here right now");
    context->PushIRStack(GenerateDummyIR(node, context));
  }
}

void RaverieSpirVFrontEnd::WalkMultiExpressionNode(Raverie::MultiExpressionNode*& node, RaverieSpirVFrontEndContext* context)
{
}

struct ConditionBlockData
{
  BasicBlock* mIfTrue;
  BasicBlock* mIfFalse;
  BasicBlock* mMergePoint;
};

void RaverieSpirVFrontEnd::WalkIfRootNode(Raverie::IfRootNode*& node, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* prevBlock = context->GetCurrentBlock();

  size_t ifParts = node->IfParts.Size();
  Array<ConditionBlockData> blockPairs;
  blockPairs.Reserve(ifParts);

  // Pre-allocate the blocks for the if-statements (makes it easier to point at
  // later blocks)
  for (size_t i = 0; i < ifParts; ++i)
  {
    // Skip any else with no condition. This is covered by the previous block's
    // ifFalse block
    if (node->IfParts[i]->Condition == nullptr)
      continue;

    String indexStr = ToString(i);

    ConditionBlockData data;

    // Don't add any block to the function so we can properly resolve dominance
    // order. This is particularly important for expressions that change the
    // block inside of the conditional (e.g. Logical Or)

    // Always make the if and merge blocks
    data.mIfTrue = BuildBlockNoStack(BuildString("ifTrue", indexStr), context);
    data.mMergePoint = BuildBlockNoStack(BuildString("ifMerge", indexStr), context);

    // The ifFalse block is not always needed though. If this is the last part
    // in the chain then this must be an if with no else (since we skipped
    // else's with no ifs earlier). In this case do a small optimization of
    // making the ifFalse and mergePoint be the same block. This makes code
    // generation a little easier and makes the resultant code look cleaner.
    int lastIndex = ifParts - 1;
    if (i != lastIndex)
      data.mIfFalse = BuildBlockNoStack(BuildString("ifFalse", indexStr), context);
    else
      data.mIfFalse = data.mMergePoint;

    blockPairs.PushBack(data);
  }

  // Now emit all of the actual if statements and their bodies
  for (size_t i = 0; i < node->IfParts.Size(); ++i)
  {
    Raverie::IfNode* ifNode = node->IfParts[i];
    ExtractDebugInfo(ifNode, context->mDebugInfo);

    // If this part has a condition then we have to emit the appropriate
    // conditional block and branch conditions
    if (ifNode->Condition)
    {
      ConditionBlockData blockPair = blockPairs[i];
      BasicBlock* ifTrueBlock = blockPairs[i].mIfTrue;
      BasicBlock* ifFalseBlock = blockPairs[i].mIfFalse;
      BasicBlock* ifMerge = blockPairs[i].mMergePoint;

      // Walk the conditional and then branch on this value to either the true
      // or false block
      IRaverieShaderIR* conditionalIR = WalkAndGetValueTypeResult(ifNode->Condition, context);

      // Mark the current block we're in (the header block where we write the
      // conditionals) as a selection block and mark it's merge point. Note:
      // This needs to be after we walk the conditional as the block can change
      // (logical and/or expressions)
      BasicBlock* headerBlock = context->GetCurrentBlock();
      headerBlock->mBlockType = BlockType::Selection;
      headerBlock->mMergePoint = ifMerge;

      headerBlock->mTerminatorOp = BuildIROp(
          headerBlock, OpType::OpBranchConditional, nullptr, conditionalIR, ifTrueBlock, ifFalseBlock, context);

      // Start emitting the true block. First mark this as the current active
      // block
      context->mCurrentBlock = ifTrueBlock;
      // Mark the if true block as the next block in dominance order
      context->mCurrentFunction->mBlocks.PushBack(ifTrueBlock);

      // Now walk all of the statements int he block
      mWalker.Walk(this, ifNode->Statements, context);
      // Always emit a branch back to the merge point. If this is dead code
      // because of another termination condition we'll clean this up after
      // generating the entire function.
      ifTrueBlock->mTerminatorOp = BuildIROp(ifTrueBlock, OpType::OpBranch, nullptr, ifMerge, context);

      // Nested if pushed another merge point. Add to a termination condition on
      // the nested if merge point back to our merge point.
      if (context->mCurrentBlock != ifTrueBlock)
      {
        BasicBlock* nestedBlock = context->mCurrentBlock;
        nestedBlock->mTerminatorOp = BuildIROp(nestedBlock, OpType::OpBranch, nullptr, ifMerge, context);
      }

      // Now mark that we're inside the false block
      context->mCurrentBlock = ifFalseBlock;
      // Mark the if false block as the next block in dominance order
      // (if it's not the merge block which is handled at the end)
      if (ifFalseBlock != ifMerge)
        context->mCurrentFunction->mBlocks.PushBack(ifFalseBlock);
      // Keep track of the previous header block so if statements know where to
      // merge back to
      prevBlock = headerBlock;
    }
    // Otherwise this is an else with no if
    else
    {
      BasicBlock* currentBlock = context->GetCurrentBlock();
      // Walk all of the statements in the else
      mWalker.Walk(this, ifNode->Statements, context);

      // Always emit a branch back to the previous block's merge point
      currentBlock->mTerminatorOp = BuildIROp(currentBlock, OpType::OpBranch, nullptr, prevBlock->mMergePoint, context);
      // Nested if pushed another merge point. Add to a termination condition on
      // the nested if merge point back to our merge point.
      if (context->mCurrentBlock != currentBlock)
      {
        BasicBlock* nestedBlock = context->mCurrentBlock;
        currentBlock->mTerminatorOp =
            BuildIROp(nestedBlock, OpType::OpBranch, nullptr, prevBlock->mMergePoint, context);
      }
    }
  }

  // Now write out all merge point blocks in reverse order (requirement of
  // spir-v is that a block must appear before all blocks they dominate).
  // Additionally, add branches for all merge points of all blocks to the
  // previous block's merge point.
  for (size_t i = 0; i < blockPairs.Size(); ++i)
  {
    int blockIndex = blockPairs.Size() - i - 1;
    BasicBlock* block = blockPairs[blockIndex].mMergePoint;
    // If this is not the first block then add a branch on the merge point to
    // the previous block's merge point
    if (blockIndex != 0)
      block->mTerminatorOp =
          BuildIROp(block, OpType::OpBranch, nullptr, blockPairs[blockIndex - 1].mMergePoint, context);
    context->mCurrentFunction->mBlocks.PushBack(block);
  }

  // Mark the first merge point as the new current block (everything after the
  // if goes here).
  context->mCurrentBlock = blockPairs[0].mMergePoint;
}

void RaverieSpirVFrontEnd::WalkIfNode(Raverie::IfNode*& node, RaverieSpirVFrontEndContext* context)
{
  // nothing to do
}

void RaverieSpirVFrontEnd::WalkBreakNode(Raverie::BreakNode*& node, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->mCurrentBlock;
  BasicBlock* breakTarget = context->mBreakTarget;
  ErrorIf(breakTarget == nullptr, "Break statement doesn't have a valid merge point to jump to");

  // Generate a branch to the break target of the current block. Also mark this
  // as a terminator op so we know that no terminator must be generated.
  currentBlock->mTerminatorOp = BuildCurrentBlockIROp(OpType::OpBranch, nullptr, breakTarget, context);
}

void RaverieSpirVFrontEnd::WalkContinueNode(Raverie::ContinueNode*& node, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->mCurrentBlock;
  BasicBlock* continueTarget = context->mContinueTarget;
  ErrorIf(continueTarget == nullptr, "Continue statement doesn't have a valid continue point to jump to");

  // Generate a branch to the continue target of the current block. Also mark
  // this as a terminator op so we know that no terminator must be generated.
  currentBlock->mTerminatorOp = BuildIROp(currentBlock, OpType::OpBranch, nullptr, continueTarget, context);
}

void RaverieSpirVFrontEnd::WalkReturnNode(Raverie::ReturnNode*& node, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // We have to generate different op code depending on if this has a return
  // value or not
  if (node->ReturnValue)
  {
    // @JoshD: Fix
    // For now, assume that all return types must be value types (deal with
    // pointers later)
    IRaverieShaderIR* returnResultOp = WalkAndGetResult(node->ReturnValue, context);
    RaverieShaderIROp* returnResultValueOp = GetOrGenerateValueTypeFromIR(returnResultOp, context);
    currentBlock->mTerminatorOp = BuildCurrentBlockIROp(OpType::OpReturnValue, nullptr, returnResultValueOp, context);
  }
  else
    currentBlock->mTerminatorOp = BuildCurrentBlockIROp(OpType::OpReturn, nullptr, context);
}

void RaverieSpirVFrontEnd::WalkWhileNode(Raverie::WhileNode*& node, RaverieSpirVFrontEndContext* context)
{
  WalkGenericLoop(nullptr, nullptr, node, node, context);
}

void RaverieSpirVFrontEnd::WalkDoWhileNode(Raverie::DoWhileNode*& node, RaverieSpirVFrontEndContext* context)
{
  // A do while looks like a header block that always jumps to a loop block.
  // This loop block will always branch to the continue target (the condition
  // block) unless a break happens which will branch to the merge block (after
  // the loop). The condition block will choose to jump either back to the
  // header block or to the merge point.
  BasicBlock* headerBlock = BuildBlockNoStack("headerBlock", context);
  BasicBlock* loopTrueBlock = BuildBlockNoStack("loop-body", context);
  BasicBlock* conditionBlock = BuildBlockNoStack("conditionBlock", context);
  BasicBlock* mergeBlock = BuildBlockNoStack("after-loop", context);

  // Always jump to the header block
  BuildCurrentBlockIROp(OpType::OpBranch, nullptr, headerBlock, context);

  // Mark the header as the next block and visit it
  context->mCurrentFunction->mBlocks.PushBack(headerBlock);
  GenerateLoopHeaderBlock(headerBlock, loopTrueBlock, mergeBlock, conditionBlock, context);

  // Now create the loop body which has the conditional block as its continue
  // target
  context->mCurrentFunction->mBlocks.PushBack(loopTrueBlock);
  GenerateLoopStatements(node, loopTrueBlock, mergeBlock, conditionBlock, context);

  // Finally create the conditional block (which is the continue target)
  // which will either jump back to the header block or exit the loop
  context->mCurrentFunction->mBlocks.PushBack(conditionBlock);
  GenerateLoopConditionBlock(node, conditionBlock, headerBlock, mergeBlock, context);

  // Afterwards the active block is always the merge point
  context->mCurrentFunction->mBlocks.PushBack(mergeBlock);
  context->mCurrentBlock = mergeBlock;
}

void RaverieSpirVFrontEnd::WalkForNode(Raverie::ForNode*& node, RaverieSpirVFrontEndContext* context)
{
  Raverie::SyntaxNode* initializer = nullptr;
  if (node->ValueVariable != nullptr)
    initializer = node->ValueVariable;
  else if (node->Initialization != nullptr)
    initializer = node->Initialization;

  WalkGenericLoop(initializer, node->Iterator, node, node, context);
}

void RaverieSpirVFrontEnd::WalkForEachNode(Raverie::ForEachNode*& node, RaverieSpirVFrontEndContext* context)
{
  SendTranslationError(node->Location, "foreach is not supported.");
}

void RaverieSpirVFrontEnd::WalkLoopNode(Raverie::LoopNode*& node, RaverieSpirVFrontEndContext* context)
{
  WalkGenericLoop(nullptr, nullptr, nullptr, node, context);
}

void RaverieSpirVFrontEnd::WalkGenericLoop(Raverie::SyntaxNode* initializerNode,
                                         Raverie::SyntaxNode* iterator,
                                         Raverie::ConditionalLoopNode* conditionalNode,
                                         Raverie::LoopScopeNode* loopScopeNode,
                                         RaverieSpirVFrontEndContext* context)
{
  // Always walk the initializer node first if it exists. The contents of this
  // go before any loop block.
  if (initializerNode != nullptr)
    mWalker.Walk(this, initializerNode, context);

  // A basic while looks like a header block that always jumps to a condition
  // block. The condition block will choose to jump either to the loop block or
  // to the merge point. The loop block will always branch to the continue
  // target unless a break happens which will branch to the merge block (after
  // the loop). The continue block will always jump back to the header block.
  BasicBlock* headerBlock = BuildBlockNoStack("headerBlock", context);
  BasicBlock* conditionBlock = BuildBlockNoStack("conditionBlock", context);
  BasicBlock* loopTrueBlock = BuildBlockNoStack("loop-body", context);
  BasicBlock* continueBlock = BuildBlockNoStack("continueBlock", context);
  BasicBlock* mergeBlock = BuildBlockNoStack("after-loop", context);

  // Always jump to the header block
  BuildCurrentBlockIROp(OpType::OpBranch, nullptr, headerBlock, context);

  // The header always jumps to the conditional
  context->mCurrentFunction->mBlocks.PushBack(headerBlock);
  GenerateLoopHeaderBlock(headerBlock, conditionBlock, mergeBlock, continueBlock, context);

  // The conditional will jump to either the loop body or the merge point (after
  // the loop)
  context->mCurrentFunction->mBlocks.PushBack(conditionBlock);
  GenerateLoopConditionBlock(conditionalNode, conditionBlock, loopTrueBlock, mergeBlock, context);

  // Walk all of the statements in the loop body and jump to either the merge or
  // continue block
  context->mCurrentFunction->mBlocks.PushBack(loopTrueBlock);
  GenerateLoopStatements(loopScopeNode, loopTrueBlock, mergeBlock, continueBlock, context);

  // The continue block always just jumps to the header block
  context->mCurrentFunction->mBlocks.PushBack(continueBlock);
  GenerateLoopContinueBlock(iterator, continueBlock, headerBlock, context);

  // Afterwards the active block is always the merge point
  context->mCurrentFunction->mBlocks.PushBack(mergeBlock);
  context->mCurrentBlock = mergeBlock;
}

void RaverieSpirVFrontEnd::GenerateLoopHeaderBlock(BasicBlock* headerBlock,
                                                 BasicBlock* branchTarget,
                                                 BasicBlock* mergeBlock,
                                                 BasicBlock* continueBlock,
                                                 RaverieSpirVFrontEndContext* context)
{
  // Mark the header block as a loop block (so we emit the LoopMerge
  // instruction)
  headerBlock->mBlockType = BlockType::Loop;
  // Being a LoopMerge requires setting the merge and continue points
  headerBlock->mMergePoint = mergeBlock;
  headerBlock->mContinuePoint = continueBlock;

  // The header always jumps to the branch target (typically a continue)
  BuildIROp(headerBlock, OpType::OpBranch, nullptr, branchTarget, context);
}

void RaverieSpirVFrontEnd::GenerateLoopConditionBlock(Raverie::ConditionalLoopNode* conditionalNode,
                                                    BasicBlock* conditionBlock,
                                                    BasicBlock* branchTrueBlock,
                                                    BasicBlock* branchFalseBlock,
                                                    RaverieSpirVFrontEndContext* context)
{
  // The condition builds the conditional and then jumps either to the body of
  // the loop or to the end
  context->mCurrentBlock = conditionBlock;
  // If the conditional node exists
  if (conditionalNode != nullptr)
  {
    ExtractDebugInfo(conditionalNode->Condition, context->mDebugInfo);
    // Get the conditional value (must be a bool via how raverie works)
    IRaverieShaderIR* conditional = WalkAndGetValueTypeResult(conditionalNode->Condition, context);
    // Branch to either the true or false branch
    BuildCurrentBlockIROp(
        OpType::OpBranchConditional, nullptr, conditional, branchTrueBlock, branchFalseBlock, context);
  }
  // Otherwise there is no conditional (e.g. loop) so unconditionally branch to
  // the true block
  else
    BuildCurrentBlockIROp(OpType::OpBranch, nullptr, branchTrueBlock, context);
}

void RaverieSpirVFrontEnd::GenerateLoopStatements(Raverie::LoopScopeNode* loopScopeNode,
                                                BasicBlock* loopBlock,
                                                BasicBlock* mergeBlock,
                                                BasicBlock* continueBlock,
                                                RaverieSpirVFrontEndContext* context)
{
  context->mCurrentBlock = loopBlock;
  // Set the continue and merge points for this block (mainly needed for nested
  // loops)
  loopBlock->mContinuePoint = continueBlock;
  loopBlock->mMergePoint = mergeBlock;
  context->PushMergePoints(continueBlock, mergeBlock);

  // Iterate over all of the statements in the loop body
  mWalker.Walk(this, loopScopeNode->Statements, context);

  // Write out a jump back to the continue block of the loop. Only write this to
  // the active block which will either be the end of the loop block or
  // something like an after if
  if (context->mCurrentBlock->mTerminatorOp == nullptr)
  {
    IRaverieShaderIR* currentBlockContinue = BuildCurrentBlockIROp(OpType::OpBranch, nullptr, continueBlock, context);
    currentBlockContinue->mDebugInfo.mComments.Add("auto continue");
  }
  context->PopMergeTargets();
}

void RaverieSpirVFrontEnd::GenerateLoopContinueBlock(Raverie::SyntaxNode* iterator,
                                                   BasicBlock* continueBlock,
                                                   BasicBlock* headerBlock,
                                                   RaverieSpirVFrontEndContext* context)
{
  // Mark the continue block as the active block
  context->mCurrentBlock = continueBlock;
  // If it exists, walk the iterator statement
  if (iterator != nullptr)
    mWalker.Walk(this, iterator, context);
  // Always jump back to the header block
  BuildIROp(continueBlock, OpType::OpBranch, nullptr, headerBlock, context);
}

void RaverieSpirVFrontEnd::FixBlockTerminators(BasicBlock* block, RaverieSpirVFrontEndContext* context)
{
  size_t opCount = block->mLines.Size();
  size_t firstTerminatorIndex = opCount;
  for (size_t i = 0; i < opCount; ++i)
  {
    IRaverieShaderIR* ir = block->mLines[i];
    RaverieShaderIROp* op = ir->As<RaverieShaderIROp>();
    // If this op is a terminator then mark its id and break
    if (op->mOpType == OpType::OpReturn || op->mOpType == OpType::OpReturnValue || op->mOpType == OpType::OpBranch ||
        op->mOpType == OpType::OpBranchConditional || op->mOpType == OpType::OpSwitch || op->mOpType == OpType::OpKill)
    {
      firstTerminatorIndex = i;
      break;
    }
  }

  // First terminator is the last op in the block. This block is good and
  // there's nothing more to do.
  if (firstTerminatorIndex == opCount - 1)
    return;

  // No terminator in the block
  if (firstTerminatorIndex >= opCount)
  {
    RaverieShaderIRType* returnType = context->mCurrentFunction->GetReturnType();
    // If the return type isn't void then this was likely a block that was
    // generated after a conditional that can't be reached (e.g. if + else and
    // then after the else). These statements should be unreachable otherwise
    // we'd have a raverie error.
    if (returnType->mRaverieType != RaverieTypeId(void))
      block->mTerminatorOp = BuildIROp(block, OpType::OpUnreachable, nullptr, context);
    // Otherwise just emit a return
    else
      block->mTerminatorOp = BuildIROp(block, OpType::OpReturn, nullptr, context);
    return;
  }

  // There's dead instructions after the terminator. Clean up everything
  // afterwards
  size_t newSize = firstTerminatorIndex + 1;
  for (size_t i = newSize; i < block->mLines.Size(); ++i)
    delete block->mLines[i];
  block->mLines.Resize(newSize);
}

Raverie::Function* RaverieSpirVFrontEnd::GetSetter(Raverie::MemberAccessNode* memberAccessNode)
{
  // Check for io-usage of a setter
  int setterFlags = Raverie::IoMode::WriteLValue | Raverie::IoMode::StrictPropertySet;
  bool isSetter = memberAccessNode->IoUsage & setterFlags;
  if (!isSetter)
    return nullptr;

  Raverie::Function* set = nullptr;

  // See if this is a getter/setter 'set'
  if (memberAccessNode->AccessedGetterSetter != nullptr && memberAccessNode->AccessedGetterSetter->Set != nullptr)
    set = memberAccessNode->AccessedGetterSetter->Set;
  // Otherwise check for a property set (happens if this is a field)
  else if (memberAccessNode->AccessedProperty != nullptr && memberAccessNode->AccessedProperty->Set != nullptr)
    set = memberAccessNode->AccessedProperty->Set;
  return set;
}

bool RaverieSpirVFrontEnd::ResolveSetter(Raverie::BinaryOperatorNode* node,
                                       RaverieShaderIROp* resultValue,
                                       Raverie::SyntaxNode* resultNode,
                                       RaverieSpirVFrontEndContext* context)
{
  Raverie::MemberAccessNode* memberAccessNode = Raverie::Type::DynamicCast<Raverie::MemberAccessNode*>(node->LeftOperand);
  if (memberAccessNode == nullptr)
    return false;

  Raverie::Function* set = GetSetter(memberAccessNode);

  // If we didn't find a setter then there's nothing to do
  if (set == nullptr)
    return false;

  RaverieShaderIRFunction* shaderFunction = mLibrary->FindFunction(set);
  // If this is an existing raverie function then we have to translate this member
  // access into a function call
  if (shaderFunction != nullptr)
  {
    Array<IRaverieShaderIR*> arguments;
    // Pass through 'this' if this is an instance function
    if (!set->IsStatic)
      arguments.PushBack(WalkAndGetResult(memberAccessNode->LeftOperand, context));
    // Get the result value if needed and add it as an argument
    if (resultValue == nullptr)
      resultValue = WalkAndGetValueTypeResult(resultNode, context);
    arguments.PushBack(resultValue);

    // Turn this into a function call
    WriteFunctionCall(arguments, shaderFunction, context);
    return true;
  }

  // Try to find a resolver for this setter
  MemberPropertySetterResolverIRFn resolver =
      mLibrary->FindSetterResolver(memberAccessNode->LeftOperand->ResultType, set);
  if (resolver != nullptr)
  {
    // If there was no result value then walk the result node to compute it
    // (prevents a creating redundant instructions in pure setter cases)
    if (resultValue == nullptr)
      resultValue = WalkAndGetValueTypeResult(resultNode, context);
    resolver(this, memberAccessNode, resultValue, context);
    return true;
  }

  return false;
}

IRaverieShaderIR* RaverieSpirVFrontEnd::PerformBinaryOp(Raverie::BinaryOperatorNode*& node,
                                                    OpType opType,
                                                    RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Walk both sides of the op
  IRaverieShaderIR* leftInstruction = WalkAndGetResult(node->LeftOperand, context);
  IRaverieShaderIR* rightInstruction = WalkAndGetResult(node->RightOperand, context);

  return PerformBinaryOp(node, opType, leftInstruction, rightInstruction, context);
}

IRaverieShaderIR* RaverieSpirVFrontEnd::PerformBinaryOp(Raverie::BinaryOperatorNode*& node,
                                                    OpType opType,
                                                    IRaverieShaderIR* lhs,
                                                    IRaverieShaderIR* rhs,
                                                    RaverieSpirVFrontEndContext* context)
{
  // All binary operators require value types
  RaverieShaderIROp* leftValueOp = GetOrGenerateValueTypeFromIR(lhs, context);
  RaverieShaderIROp* rightValueOp = GetOrGenerateValueTypeFromIR(rhs, context);

  // Generate dependencies
  RaverieShaderIRType* resultType = FindType(node->ResultType, node);

  // Generate the binary op
  RaverieShaderIROp* binaryOp = BuildCurrentBlockIROp(opType, resultType, leftValueOp, rightValueOp, context);
  context->PushIRStack(binaryOp);
  return binaryOp;
}

void RaverieSpirVFrontEnd::PerformBinaryAssignmentOp(Raverie::BinaryOperatorNode*& node,
                                                   OpType opType,
                                                   RaverieSpirVFrontEndContext* context)
{
  // Walk both sides of the op
  IRaverieShaderIR* leftInstruction = WalkAndGetResult(node->LeftOperand, context);
  IRaverieShaderIR* rightInstruction = WalkAndGetResult(node->RightOperand, context);
  PerformBinaryAssignmentOp(node, opType, leftInstruction, rightInstruction, context);
}

void RaverieSpirVFrontEnd::PerformBinaryAssignmentOp(Raverie::BinaryOperatorNode*& node,
                                                   OpType opType,
                                                   IRaverieShaderIR* lhs,
                                                   IRaverieShaderIR* rhs,
                                                   RaverieSpirVFrontEndContext* context)
{
  // All binary operators require value types so load from pointers if we had
  // them
  RaverieShaderIROp* leftValueOp = GetOrGenerateValueTypeFromIR(lhs, context);
  RaverieShaderIROp* rightValueOp = GetOrGenerateValueTypeFromIR(rhs, context);

  // Perform the op
  RaverieShaderIRType* resultType = FindType(node->LeftOperand->ResultType, node);
  RaverieShaderIROp* binaryOpInstruction = BuildCurrentBlockIROp(opType, resultType, leftValueOp, rightValueOp, context);

  // Deal with setters. This requires 'promoting' the setter above the
  // assignment op (e.g. A += B -> A.Set(A + B))
  if (ResolveSetter(node, binaryOpInstruction, node->RightOperand, context))
    return;

  // Validate this op can be assigned to (constants)
  ValidateLValue(leftValueOp, node->Location);

  // Since this is a binary assignment op, the left side must be a pointer type
  RaverieShaderIROp* leftOp = lhs->As<RaverieShaderIROp>();
  if (!leftOp->IsResultPointerType())
  {
    SendTranslationError(node->Location, "Left operand in a binary assignment op must be a pointer type");
    // Since this is an assignment op, no dummy variable generation is necessary
    return;
  }

  // Store into the variable
  BasicBlock* currentBlock = context->GetCurrentBlock();
  BuildStoreOp(currentBlock, lhs, binaryOpInstruction, context);
}

IRaverieShaderIR* RaverieSpirVFrontEnd::PerformUnaryOp(Raverie::UnaryOperatorNode*& node,
                                                   OpType opType,
                                                   RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* resultType = FindType(node->ResultType, node);

  // Get the operand for the op (requires a value type)
  RaverieShaderIROp* operandValueOp = WalkAndGetValueTypeResult(node->Operand, context);

  RaverieShaderIROp* unaryOp = BuildCurrentBlockIROp(opType, resultType, operandValueOp, context);
  context->PushIRStack(unaryOp);
  return unaryOp;
}

IRaverieShaderIR* RaverieSpirVFrontEnd::PerformUnaryIncDecOp(Raverie::UnaryOperatorNode*& node,
                                                         IRaverieShaderIR* constantOne,
                                                         OpType opType,
                                                         RaverieSpirVFrontEndContext* context)
{
  // Get the operand to work on (must be a value type)
  IRaverieShaderIR* operand = WalkAndGetResult(node->Operand, context);
  RaverieShaderIROp* operandValueOp = GetOrGenerateValueTypeFromIR(operand, context);

  // Perform the op (add/sub)
  RaverieShaderIRType* resultType = FindType(node->Operand->ResultType, node->Operand);
  IRaverieShaderIR* tempOp = BuildCurrentBlockIROp(opType, resultType, operandValueOp, constantOne, context);

  // If the operand was a pointer type instead of a temporary then we need to
  // store the value back into the pointer
  RaverieShaderIROp* op = operand->As<RaverieShaderIROp>();
  if (op != nullptr && op->IsResultPointerType())
  {
    BasicBlock* currentBlock = context->GetCurrentBlock();
    BuildStoreOp(currentBlock, operand, tempOp, context);
  }
  context->PushIRStack(tempOp);
  return tempOp;
}

IRaverieShaderIR* RaverieSpirVFrontEnd::PerformTypeCast(Raverie::TypeCastNode*& node,
                                                    OpType opType,
                                                    RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* resultType = FindType(node->ResultType, node);

  // Get the thing we're casting. Casts require a value type so generate a value
  // type if necessary.
  RaverieShaderIROp* operandValueResult = WalkAndGetValueTypeResult(node->Operand, context);

  // Generate the cast
  RaverieShaderIROp* castOp = BuildCurrentBlockIROp(opType, resultType, operandValueResult, context);
  context->PushIRStack(castOp);
  return castOp;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GetIntegerConstant(int value, RaverieSpirVFrontEndContext* context)
{
  Raverie::BoundType* raverieIntType = RaverieTypeId(int);
  RaverieShaderIRType* shaderIntType = mLibrary->FindType(raverieIntType);
  RaverieShaderIROp* constantIntOp = GetConstant(shaderIntType, value, context);
  return constantIntOp;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GetConstant(RaverieShaderIRType* type,
                                                 StringParam value,
                                                 RaverieSpirVFrontEndContext* context)
{
  Raverie::Any constantValue;
  ToAny(type, value, constantValue);
  return GetConstant(type, constantValue, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GetConstant(RaverieShaderIRType* type,
                                                 Raverie::Any value,
                                                 RaverieSpirVFrontEndContext* context)
{
  // Each constant should only be declared once. Find if it already exists
  ConstantOpKeyType constantKey = ConstantOpKeyType(type, value);
  RaverieShaderIROp* opConstant = mLibrary->FindConstantOp(constantKey, false);
  if (opConstant == nullptr)
  {
    // It doesn't exist so create the constant. Constants are two parts:
    // The actual constant value and the op to declare the constant.
    RaverieShaderIRConstantLiteral* constant = GetOrCreateConstantLiteral(value);
    opConstant = BuildIROpNoBlockAdd(OpType::OpConstant, type, context);
    opConstant->mArguments.PushBack(constant);
  }

  mLibrary->mConstantOps.InsertNoOverwrite(constantKey, opConstant);

  return opConstant;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::ConstructCompositeFromScalar(BasicBlock* block,
                                                                  RaverieShaderIRType* compositeType,
                                                                  IRaverieShaderIR* scalar,
                                                                  RaverieSpirVFrontEndContext* context)
{
  // If this is already a scalar type then return the value as-is
  if (compositeType->mComponents == 1)
    return scalar->As<RaverieShaderIROp>();

  // Construct the composite from the individual scalar values (splat)
  RaverieShaderIROp* composite = BuildIROp(block, OpType::OpCompositeConstruct, compositeType, context);
  for (size_t i = 0; i < compositeType->mComponents; ++i)
    composite->mArguments.PushBack(scalar);
  return composite;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::AddSpecializationConstant(Raverie::MemberVariableNode* node,
                                                               RaverieShaderIRType* varType,
                                                               RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRConstantLiteral* defaultLiteral = nullptr;
  // If the initial value of the node is a value node then we can initialize
  // this constant to the actual value. Other types are more complicated as the
  // default value requires actually constructing the type to get value
  // (something like var Pi : Real = Math.Pi). @JoshD: Deal with finding these
  // constants later.
  Raverie::ValueNode* valueNode = Raverie::Type::DynamicCast<Raverie::ValueNode*>(node->InitialValue);
  if (valueNode != nullptr)
  {
    Raverie::Any defaultValue;
    ToAny(varType, valueNode->Value.Token, defaultValue);
    defaultLiteral = GetOrCreateConstantLiteral(defaultValue);
  }

  return AddSpecializationConstantRecursively(
      node->CreatedField, varType, node->Name.Token, defaultLiteral, node->Location, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::AddSpecializationConstantRecursively(void* key,
                                                                          RaverieShaderIRType* varType,
                                                                          StringParam varName,
                                                                          RaverieShaderIRConstantLiteral* literalValue,
                                                                          Raverie::CodeLocation& codeLocation,
                                                                          RaverieSpirVFrontEndContext* context)
{
  // The var name is generated the same as any other global
  String propertyName = GenerateSpirVPropertyName(varName, context->mCurrentType);

  // Deal with scalar types.
  if (varType->mBaseType == ShaderIRTypeBaseType::Bool || varType->mBaseType == ShaderIRTypeBaseType::Int ||
      varType->mBaseType == ShaderIRTypeBaseType::Float)
  {
    // Either use the given literal value or construct a default
    RaverieShaderIRConstantLiteral* defaultLiteral = literalValue;
    if (defaultLiteral == nullptr)
    {
      Raverie::Any defaultValue;
      ToAny(varType, String(), defaultValue);
      defaultLiteral = GetOrCreateConstantLiteral(defaultValue);
    }

    RaverieShaderIROp* specConstantOp = CreateSpecializationConstant(key, OpType::OpSpecConstant, varType, context);
    specConstantOp->mArguments.PushBack(defaultLiteral);
    specConstantOp->mDebugResultName = propertyName;
    return specConstantOp;
  }

  // Deal with vectors/matrices
  if (varType->mBaseType == ShaderIRTypeBaseType::Vector || varType->mBaseType == ShaderIRTypeBaseType::Matrix)
  {
    RaverieShaderIROp* specConstantCompositeOp =
        CreateSpecializationConstant(key, OpType::OpSpecConstantComposite, varType, context);
    specConstantCompositeOp->mDebugResultName = propertyName;
    // Create a sub-constant for each constituent. For vectors these are scalar,
    // for matrices these are vectors.
    char subNames[] = {'X', 'Y', 'Z', 'W'};
    RaverieShaderIRType* componentType = GetComponentType(varType);
    for (size_t i = 0; i < varType->mComponents; ++i)
    {
      // Construct the sub-constant's name for debugging purposes.
      String fullSubVarName = BuildString(varName, ".", ToString(subNames[i]));
      // Create the sub-constant. This sub-constant is not given a unique key as
      // there's never a case where we need to lookup vector.x.
      RaverieShaderIROp* constituent =
          AddSpecializationConstantRecursively(nullptr, componentType, fullSubVarName, nullptr, codeLocation, context);
      specConstantCompositeOp->mArguments.PushBack(constituent);
    }
    return specConstantCompositeOp;
  }

  // Structs are handled almost the same as vectors/matrices, the only
  // difference is how members are iterated over and how to create their debug
  // names.
  if (varType->mBaseType == ShaderIRTypeBaseType::Struct)
  {
    RaverieShaderIROp* specConstantCompositeOp =
        CreateSpecializationConstant(key, OpType::OpSpecConstantComposite, varType, context);
    specConstantCompositeOp->mDebugResultName = propertyName;

    for (size_t i = 0; i < varType->mParameters.Size(); ++i)
    {
      String memberName = varType->GetMemberName(i);
      String fullSubVarName = BuildString(varName, ".", memberName);
      RaverieShaderIRType* subType = varType->GetSubType(i);
      RaverieShaderIROp* constituent =
          AddSpecializationConstantRecursively(nullptr, subType, fullSubVarName, nullptr, codeLocation, context);
      specConstantCompositeOp->mArguments.PushBack(constituent);
    }
    return specConstantCompositeOp;
  }

  String errorMsg = String::Format("Type '%s' is not valid as a specialization constant.", varType->mName.c_str());
  SendTranslationError(codeLocation, errorMsg);
  // Return a dummy constant so that we don't crash. This is not valid spir-v
  // though.
  return CreateSpecializationConstant(key, OpType::OpSpecConstantComposite, varType, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::CreateSpecializationConstant(void* key,
                                                                  OpType opType,
                                                                  RaverieShaderIRType* varType,
                                                                  RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* specConstantOp = BuildIROpNoBlockAdd(opType, varType, context);
  // Always add this constant to the library for memory management
  mLibrary->mOwnedSpecializationConstants.PushBack(specConstantOp);
  // Additionally, if there's a valid key then store this in the library's
  // lookup map. This is used to find the constant at a later point in time via
  // the key (such as the raverie field). If the key is null then we don't
  // have/need to lookup this constant later (e.g. a sub-member of a vector).
  if (key != nullptr)
    mLibrary->mSpecializationConstantMap.InsertOrError(key, specConstantOp);

  return specConstantOp;
}

void RaverieSpirVFrontEnd::ToAny(RaverieShaderIRType* type, StringParam value, Raverie::Any& result)
{
  if (type->mBaseType == ShaderIRTypeBaseType::Bool)
  {
    bool temp;
    ToValue(value, temp);
    result = temp;
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::Int)
  {
    int temp;
    ToValue(value, temp);
    result = temp;
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::Float)
  {
    float temp;
    ToValue(value, temp);
    result = temp;
  }
  else
  {
    result = value;
  }
}

void RaverieSpirVFrontEnd::ExtractDebugInfo(Raverie::SyntaxNode* node, RaverieShaderDebugInfo& debugInfo)
{
  if (node == nullptr)
    return;

  debugInfo.mLocations.mCodeLocations.PushBack(node->Location);
  GetLineAsComment(node, debugInfo.mComments);
}

void RaverieSpirVFrontEnd::GetLineAsComment(Raverie::SyntaxNode* node, RaverieShaderIRComments& comments)
{
  String comment = mRaverieCommentParser.Run(node);
  if (!comment.Empty())
  {
    comments.Add(comment);
  }
}

BasicBlock* RaverieSpirVFrontEnd::BuildBlock(StringParam labelDebugName, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* block = BuildBlockNoStack(labelDebugName, context);
  context->mCurrentFunction->mBlocks.PushBack(block);
  return block;
}

BasicBlock* RaverieSpirVFrontEnd::BuildBlockNoStack(StringParam labelDebugName, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* block = new BasicBlock();
  block->mDebugResultName = labelDebugName;
  return block;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildIROpNoBlockAdd(OpType opType,
                                                         RaverieShaderIRType* resultType,
                                                         RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* result = new RaverieShaderIROp(opType);

  if (context != nullptr)
  {
    result->mDebugInfo = context->mDebugInfo;
    result->mResultType = resultType;
    context->mDebugInfo.Clear();
  }
  else
    result->mResultType = resultType;

  return result;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildIROp(BasicBlock* block,
                                               OpType opType,
                                               RaverieShaderIRType* resultType,
                                               RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* result = BuildIROpNoBlockAdd(opType, resultType, context);

  block->mLines.PushBack(result);
  return result;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildIROp(BasicBlock* block,
                                               OpType opType,
                                               RaverieShaderIRType* resultType,
                                               IRaverieShaderIR* arg0,
                                               RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* result = BuildIROp(block, opType, resultType, context);
  result->mArguments.PushBack(arg0);
  return result;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildIROp(BasicBlock* block,
                                               OpType opType,
                                               RaverieShaderIRType* resultType,
                                               IRaverieShaderIR* arg0,
                                               IRaverieShaderIR* arg1,
                                               RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* result = BuildIROp(block, opType, resultType, context);
  result->mArguments.PushBack(arg0);
  result->mArguments.PushBack(arg1);
  return result;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildIROp(BasicBlock* block,
                                               OpType opType,
                                               RaverieShaderIRType* resultType,
                                               IRaverieShaderIR* arg0,
                                               IRaverieShaderIR* arg1,
                                               IRaverieShaderIR* arg2,
                                               RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* result = BuildIROp(block, opType, resultType, context);
  result->mArguments.PushBack(arg0);
  result->mArguments.PushBack(arg1);
  result->mArguments.PushBack(arg2);
  return result;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildCurrentBlockIROp(OpType opType,
                                                           RaverieShaderIRType* resultType,
                                                           RaverieSpirVFrontEndContext* context)
{
  return BuildIROp(context->GetCurrentBlock(), opType, resultType, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildCurrentBlockIROp(OpType opType,
                                                           RaverieShaderIRType* resultType,
                                                           IRaverieShaderIR* arg0,
                                                           RaverieSpirVFrontEndContext* context)
{
  return BuildIROp(context->GetCurrentBlock(), opType, resultType, arg0, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildCurrentBlockIROp(OpType opType,
                                                           RaverieShaderIRType* resultType,
                                                           IRaverieShaderIR* arg0,
                                                           IRaverieShaderIR* arg1,
                                                           RaverieSpirVFrontEndContext* context)
{
  return BuildIROp(context->GetCurrentBlock(), opType, resultType, arg0, arg1, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildCurrentBlockIROp(OpType opType,
                                                           RaverieShaderIRType* resultType,
                                                           IRaverieShaderIR* arg0,
                                                           IRaverieShaderIR* arg1,
                                                           IRaverieShaderIR* arg2,
                                                           RaverieSpirVFrontEndContext* context)
{
  return BuildIROp(context->GetCurrentBlock(), opType, resultType, arg0, arg1, arg2, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildCurrentBlockAccessChain(RaverieShaderIRType* baseResultType,
                                                                  RaverieShaderIROp* selfInstance,
                                                                  IRaverieShaderIR* arg0,
                                                                  RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* resultPointerType = baseResultType->GetPointerType();

  // We have to declare the access chain with the same storage class as
  // the self instance. This shows up primarily with runtime arrays
  // (e.g. accessing the internal data has to be uniform storage class).
  // To avoid creating a duplicate type, only do this if the storage
  // class isn't function, otherwise we already have the correct pointer type.
  spv::StorageClass resultStorageClass = selfInstance->mResultType->mStorageClass;
  if (resultStorageClass != spv::StorageClassFunction)
    resultPointerType = FindOrCreatePointerInterfaceType(mLibrary, baseResultType, resultStorageClass);

  RaverieShaderIROp* accessChainOp =
      BuildCurrentBlockIROp(OpType::OpAccessChain, resultPointerType, selfInstance, context);
  accessChainOp->mArguments.PushBack(arg0);
  return accessChainOp;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildCurrentBlockAccessChain(RaverieShaderIRType* baseResultType,
                                                                  RaverieShaderIROp* selfInstance,
                                                                  IRaverieShaderIR* arg0,
                                                                  IRaverieShaderIR* arg1,
                                                                  RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* accessChainOp = BuildCurrentBlockAccessChain(baseResultType, selfInstance, arg0, context);
  accessChainOp->mArguments.PushBack(arg1);
  return accessChainOp;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildDecorationOp(BasicBlock* block,
                                                       IRaverieShaderIR* decorationTarget,
                                                       spv::Decoration decorationType,
                                                       RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRConstantLiteral* decorationTypeLiteral = GetOrCreateConstantIntegerLiteral(decorationType);
  return BuildIROp(block, OpType::OpDecorate, nullptr, decorationTarget, decorationTypeLiteral, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildDecorationOp(BasicBlock* block,
                                                       IRaverieShaderIR* decorationTarget,
                                                       spv::Decoration decorationType,
                                                       int decorationValue,
                                                       RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRConstantLiteral* decorationTypeLiteral = GetOrCreateConstantIntegerLiteral(decorationType);
  RaverieShaderIRConstantLiteral* decorationValueLiteral = GetOrCreateConstantIntegerLiteral(decorationValue);
  return BuildIROp(
      block, OpType::OpDecorate, nullptr, decorationTarget, decorationTypeLiteral, decorationValueLiteral, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildMemberDecorationOp(BasicBlock* block,
                                                             IRaverieShaderIR* decorationTarget,
                                                             int memberOffset,
                                                             spv::Decoration decorationType,
                                                             RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* resultOp = BuildIROp(block, OpType::OpMemberDecorate, nullptr, decorationTarget, context);
  resultOp->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(memberOffset));
  resultOp->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(decorationType));
  return resultOp;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildMemberDecorationOp(BasicBlock* block,
                                                             IRaverieShaderIR* decorationTarget,
                                                             int memberOffset,
                                                             spv::Decoration decorationType,
                                                             int decorationValue,
                                                             RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* resultOp = BuildIROp(block, OpType::OpMemberDecorate, nullptr, decorationTarget, context);
  resultOp->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(memberOffset));
  resultOp->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(decorationType));
  resultOp->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(decorationValue));
  return resultOp;
}

RaverieShaderIRConstantLiteral* RaverieSpirVFrontEnd::GetOrCreateConstantIntegerLiteral(int value)
{
  return GetOrCreateConstantLiteral(value);
}

RaverieShaderIRConstantLiteral* RaverieSpirVFrontEnd::GetOrCreateConstantLiteral(Raverie::Any value)
{
  RaverieShaderIRConstantLiteral* constantValue = mLibrary->FindConstantLiteral(value);
  if (constantValue != nullptr)
    return constantValue;

  constantValue = new RaverieShaderIRConstantLiteral(value);
  constantValue->mDebugResultName = value.ToString();
  mLibrary->mConstantLiterals.InsertOrError(value, constantValue);
  return constantValue;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildOpVariable(RaverieShaderIRType* resultType, RaverieSpirVFrontEndContext* context)
{
  // All variable declarations must be at the beginning of the first block of a
  // function. To do this we always grab the first block and add to the local
  // variables section of this block (which is emitted first)
  BasicBlock* firstBlock = context->mCurrentFunction->mBlocks[0];
  BasicBlock* oldBlock = context->mCurrentBlock;
  context->mCurrentBlock = firstBlock;

  RaverieShaderIROp* variableOp = BuildOpVariable(firstBlock, resultType, (int)spv::StorageClassFunction, context);

  // Set the context back to the previously active block
  context->mCurrentBlock = oldBlock;

  return variableOp;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildOpVariable(BasicBlock* block,
                                                     RaverieShaderIRType* resultType,
                                                     int storageConstant,
                                                     RaverieSpirVFrontEndContext* context)
{
  // Declare the constant for function storage variables
  RaverieShaderIRConstantLiteral* functionStorageConstant = GetOrCreateConstantLiteral(storageConstant);
  // Make the variable declaration (with function storage) and add it to the
  // local variables section of the block
  RaverieShaderIROp* variableOp = BuildIROpNoBlockAdd(OpType::OpVariable, resultType, context);
  variableOp->mArguments.PushBack(functionStorageConstant);
  block->mLocalVariables.PushBack(variableOp);

  return variableOp;
}

IRaverieShaderIR* RaverieSpirVFrontEnd::WalkAndGetResult(Raverie::SyntaxNode* node, RaverieSpirVFrontEndContext* context)
{
  mWalker.Walk(this, node, context);
  return context->PopIRStack();
}

RaverieShaderIROp* RaverieSpirVFrontEnd::WalkAndGetValueTypeResult(BasicBlock* block,
                                                               Raverie::SyntaxNode* node,
                                                               RaverieSpirVFrontEndContext* context)
{
  IRaverieShaderIR* nodeResult = WalkAndGetResult(node, context);
  RaverieShaderIROp* valueResult = GetOrGenerateValueTypeFromIR(block, nodeResult, context);
  return valueResult;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::WalkAndGetValueTypeResult(Raverie::SyntaxNode* node,
                                                               RaverieSpirVFrontEndContext* context)
{
  IRaverieShaderIR* nodeResult = WalkAndGetResult(node, context);
  // The block must be fetched here as walking the node can change the current
  // block
  RaverieShaderIROp* valueResult = GetOrGenerateValueTypeFromIR(context->GetCurrentBlock(), nodeResult, context);
  return valueResult;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GetOrGenerateValueTypeFromIR(BasicBlock* block,
                                                                  IRaverieShaderIR* instruction,
                                                                  RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* op = instruction->As<RaverieShaderIROp>();
  if (!op->IsResultPointerType())
    return op;

  return BuildIROp(block, OpType::OpLoad, op->mResultType->mDereferenceType, instruction, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GetOrGenerateValueTypeFromIR(IRaverieShaderIR* instruction,
                                                                  RaverieSpirVFrontEndContext* context)
{
  return GetOrGenerateValueTypeFromIR(context->GetCurrentBlock(), instruction, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GetOrGeneratePointerTypeFromIR(BasicBlock* block,
                                                                    IRaverieShaderIR* instruction,
                                                                    RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* op = instruction->As<RaverieShaderIROp>();
  if (op->IsResultPointerType())
    return op;

  RaverieShaderIROp* variableOp = BuildOpVariable(op->mResultType->mPointerType, context);
  BuildStoreOp(variableOp, op, context);
  return variableOp;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GetOrGeneratePointerTypeFromIR(IRaverieShaderIR* instruction,
                                                                    RaverieSpirVFrontEndContext* context)
{
  return GetOrGeneratePointerTypeFromIR(context->GetCurrentBlock(), instruction, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildStoreOp(IRaverieShaderIR* target,
                                                  IRaverieShaderIR* source,
                                                  RaverieSpirVFrontEndContext* context,
                                                  bool forceLoadStore)
{
  return BuildStoreOp(context->GetCurrentBlock(), target, source, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::BuildStoreOp(BasicBlock* block,
                                                  IRaverieShaderIR* target,
                                                  IRaverieShaderIR* source,
                                                  RaverieSpirVFrontEndContext* context,
                                                  bool forceLoadStore)
{
  RaverieShaderIROp* sourceOp = source->As<RaverieShaderIROp>();
  // Change what op we use to store depending on if the source is a value or
  // pointer type. If the source was a pointer type we'd have to load it first
  // before setting, but instead we can just use CopyMemory.
  if (sourceOp->IsResultPointerType())
  {
    // Currently there's various issues with the spirv tool chains that don't
    // handle OpCopyMemory well. For this reason support forcing load/store
    // instead.
    if (forceLoadStore)
    {
      RaverieShaderIROp* loadOp =
          BuildIROp(block, OpType::OpLoad, sourceOp->mResultType->mDereferenceType, sourceOp, context);
      return BuildIROp(block, OpType::OpStore, nullptr, target, loadOp, context);
    }
    else
      return BuildIROp(block, OpType::OpCopyMemory, nullptr, target, sourceOp, context);
  }
  else
    return BuildIROp(block, OpType::OpStore, nullptr, target, sourceOp, context);
}

void RaverieSpirVFrontEnd::GetFunctionCallArguments(Raverie::FunctionCallNode* node,
                                                  Raverie::MemberAccessNode* memberAccessNode,
                                                  Array<IRaverieShaderIR*>& arguments,
                                                  RaverieSpirVFrontEndContext* context)
{
  IRaverieShaderIR* thisOp = nullptr;
  // Get the this operand if the function isn't static
  if (memberAccessNode != nullptr)
  {
    Raverie::Function* raverieFunction = memberAccessNode->AccessedFunction;
    if (!raverieFunction->IsStatic)
      thisOp = WalkAndGetResult(memberAccessNode->LeftOperand, context);
  }

  GetFunctionCallArguments(node, thisOp, arguments, context);
}

void RaverieSpirVFrontEnd::GetFunctionCallArguments(Raverie::FunctionCallNode* node,
                                                  IRaverieShaderIR* thisOp,
                                                  Array<IRaverieShaderIR*>& arguments,
                                                  RaverieSpirVFrontEndContext* context)
{
  // Always add the this operand as the first argument if it exists
  if (thisOp != nullptr)
    arguments.PushBack(thisOp);

  // Add all remaining arguments from the node
  if (node != nullptr)
  {
    for (size_t i = 0; i < node->Arguments.Size(); ++i)
    {
      IRaverieShaderIR* argument = WalkAndGetResult(node->Arguments[i], context);
      arguments.PushBack(argument);
    }
  }
}

void RaverieSpirVFrontEnd::WriteFunctionCallArguments(Array<IRaverieShaderIR*> arguments,
                                                    RaverieShaderIRType* functionType,
                                                    RaverieShaderIROp* functionCallOp,
                                                    RaverieSpirVFrontEndContext* context)
{
  // Add all arguments, making sure to convert types if necessary

  for (size_t i = 0; i < arguments.Size(); ++i)
  {
    IRaverieShaderIR* argument = arguments[i];
    RaverieShaderIRType* paramType = functionType->GetSubType(i + 1);
    WriteFunctionCallArgument(argument, functionCallOp, paramType, context);
  }
}

void RaverieSpirVFrontEnd::WriteFunctionCallArgument(IRaverieShaderIR* argument,
                                                   RaverieShaderIROp* functionCallOp,
                                                   RaverieShaderIRType* paramType,
                                                   RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* paramOp = argument->As<RaverieShaderIROp>();

  // If we know the function parameter's expected type is a value type while
  // the given parameter op is a pointer then convert the pointer to a value
  // type.
  if (paramType == nullptr || !paramType->IsPointerType())
  {
    if (paramOp->IsResultPointerType())
      paramOp = GetOrGenerateValueTypeFromIR(paramOp, context);
  }
  // Otherwise, if the parameter is a pointer type it might require creating a
  // temporary variable to pass through to the function and then copy the result back out.
  else if (paramType->IsPointerType())
  {
    bool requiresCopyArgument = false;

    // The storage class is the wrong type, we have to create a new variable to pass in-out
    if (paramOp->mResultType->mStorageClass != paramType->mStorageClass)
      requiresCopyArgument = true;
    // Pointer arguments must be a memory object declaration or a pointer
    // to an element in an array of type sampler or image (per validation rules)
    else if (paramOp->mOpType != OpType::OpVariable && paramOp->mOpType != OpType::OpFunctionParameter)
    {
      RaverieShaderIRType* dereferencedType = paramOp->mResultType->mDereferenceType;
      if (dereferencedType->mBaseType != ShaderIRTypeBaseType::Image &&
          dereferencedType->mBaseType != ShaderIRTypeBaseType::Sampler)
        requiresCopyArgument = true;
    }

    if (requiresCopyArgument)
    {
      // Create a local variable and copy the parameter into it (this will now have the correct storage class)
      RaverieShaderIRType* localVarPointerType =
          FindOrCreatePointerInterfaceType(mLibrary, paramType->mDereferenceType, paramType->mStorageClass);
      RaverieShaderIROp* localVarOp = BuildOpVariable(localVarPointerType, context);
      BuildStoreOp(localVarOp, paramOp, context);
      // Additionally, we'll need to copy the result back out of the function call to set the original paramter.
      // Unfortunately this has to happen after the function call is made so we just queue up what copies need to
      // happen.
      context->mFunctionPostambleCopies.PushBack(RaverieSpirVFrontEndContext::PostableCopy(paramOp, localVarOp));
      // Replace the paramter with the local temporary
      paramOp = localVarOp;
    }
  }

  functionCallOp->mArguments.PushBack(paramOp);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GenerateFunctionCall(RaverieShaderIRFunction* shaderFunction,
                                                          RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  RaverieShaderIRType* functionType = shaderFunction->mFunctionType;
  RaverieShaderIRType* returnType = shaderFunction->GetReturnType();

  // Generate the function call but don't add it to the block yet (so we can collect all arguments first)
  RaverieShaderIROp* functionCallOp = BuildIROpNoBlockAdd(OpType::OpFunctionCall, returnType, context);

  // The first argument is always the function type
  functionCallOp->mArguments.PushBack(shaderFunction);
  return functionCallOp;
}

void RaverieSpirVFrontEnd::WriteFunctionCallPostamble(RaverieSpirVFrontEndContext* context)
{
  // Write out any postamble copies (needed to deal with varying
  // storage class types of function parameters such as globals)
  for (size_t i = 0; i < context->mFunctionPostambleCopies.Size(); ++i)
  {
    RaverieSpirVFrontEndContext::PostableCopy& copy = context->mFunctionPostambleCopies[i];
    BuildStoreOp(copy.mDestination, copy.mSource, context);
  }
  context->mFunctionPostambleCopies.Clear();
}

void RaverieSpirVFrontEnd::WriteFunctionCall(Array<IRaverieShaderIR*> arguments,
                                           RaverieShaderIRFunction* shaderFunction,
                                           RaverieSpirVFrontEndContext* context)
{
  // Generate the function call but don't add it to the block yet (so we can collect all arguments first)
  RaverieShaderIROp* functionCallOp = GenerateFunctionCall(shaderFunction, context);

  // Write the function call arguments out
  WriteFunctionCallArguments(arguments, shaderFunction->mFunctionType, functionCallOp, context);

  // Now add the function since we have all arguments
  BasicBlock* currentBlock = context->GetCurrentBlock();
  currentBlock->AddOp(functionCallOp);

  // Now we need to invoke any pre-ample copies
  WriteFunctionCallPostamble(context);

  // If there is a return then we have to push the result onto a stack so any assignments can get the value
  RaverieShaderIRType* returnType = shaderFunction->GetReturnType();
  if (returnType != nullptr && returnType->mRaverieType != RaverieTypeId(void))
  {
    context->PushIRStack(functionCallOp);
  }
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GenerateBoolToIntegerCast(BasicBlock* block,
                                                               RaverieShaderIROp* source,
                                                               RaverieShaderIRType* destType,
                                                               RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* zero = GetIntegerConstant(0, context);
  RaverieShaderIROp* one = GetIntegerConstant(1, context);
  return GenerateFromBoolCast(block, source, destType, zero, one, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GenerateFromBoolCast(BasicBlock* block,
                                                          RaverieShaderIROp* source,
                                                          RaverieShaderIRType* destType,
                                                          IRaverieShaderIR* zero,
                                                          IRaverieShaderIR* one,
                                                          RaverieSpirVFrontEndContext* context)
{
  // SpirV doesn't support a bool to type cast. Instead a select op must be
  // generated to choose between two values. This is effectively: bool ?
  // trueResult : falseResult.
  IRaverieShaderIR* condition = source;
  IRaverieShaderIR* trueValue = one;
  IRaverieShaderIR* falseValue = zero;

  // Handle vector types
  if (destType->mComponents != 1)
  {
    // Construct the result vector types from the individual zero/one scalars
    RaverieShaderIROp* zeroVec = BuildIROp(block, OpType::OpCompositeConstruct, destType, context);
    RaverieShaderIROp* oneVec = BuildIROp(block, OpType::OpCompositeConstruct, destType, context);
    zeroVec->mDebugResultName = "zero";
    oneVec->mDebugResultName = "one";
    for (size_t i = 0; i < destType->mComponents; ++i)
    {
      zeroVec->mArguments.PushBack(zero);
      oneVec->mArguments.PushBack(one);
    }
    trueValue = oneVec;
    falseValue = zeroVec;
  }

  // Perform a component-wise selection
  RaverieShaderIROp* operation = BuildIROp(block, OpType::OpSelect, destType, condition, trueValue, falseValue, context);
  return operation;
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GenerateIntegerToBoolCast(BasicBlock* block,
                                                               RaverieShaderIROp* source,
                                                               RaverieShaderIRType* destType,
                                                               RaverieSpirVFrontEndContext* context)
{
  IRaverieShaderIR* zero = GetIntegerConstant(0, context);
  return GenerateToBoolCast(block, OpType::OpINotEqual, source, destType, zero, context);
}

RaverieShaderIROp* RaverieSpirVFrontEnd::GenerateToBoolCast(BasicBlock* block,
                                                        OpType op,
                                                        RaverieShaderIROp* source,
                                                        RaverieShaderIRType* destType,
                                                        IRaverieShaderIR* zero,
                                                        RaverieSpirVFrontEndContext* context)
{
  // SpirV doesn't support a cast to a bool. Instead this must be generated from
  // a comparison operator with the corrsponding zero vector. E.g.
  // (Boolean2)Integer2 => Integer2 != Integer2(0)
  IRaverieShaderIR* rhs = zero;
  RaverieShaderIROp* condition = source;
  RaverieShaderIRType* sourceType = source->mResultType;

  // Handle vector types
  if (sourceType->mComponents != 1)
  {
    // Construct the comparison source vector type from the individual zero
    // scalar
    RaverieShaderIROp* zeroVec = BuildIROp(block, OpType::OpCompositeConstruct, sourceType, context);
    for (size_t i = 0; i < sourceType->mComponents; ++i)
    {
      zeroVec->mArguments.PushBack(zero);
    }
    // Set the comparison rhs op to the vector instead of the scalar
    rhs = zeroVec;
  }

  RaverieShaderIROp* operation = BuildIROp(block, op, destType, condition, rhs, context);
  return operation;
}

RaverieShaderIRType* RaverieSpirVFrontEnd::GetResultValueType(RaverieShaderIROp* op)
{
  RaverieShaderIRType* resultType = op->mResultType;
  if (resultType->IsPointerType())
    return resultType->mDereferenceType;
  return resultType;
}

RaverieShaderIRType* RaverieSpirVFrontEnd::GetPointerValueType(RaverieShaderIROp* op)
{
  RaverieShaderIRType* resultType = op->mResultType;
  if (resultType->IsPointerType())
    return resultType;
  return resultType->mPointerType;
}

bool RaverieSpirVFrontEnd::ContainsAttribute(RaverieShaderIRType* shaderType, StringParam attributeName)
{
  ShaderIRTypeMeta* meta = shaderType->mMeta;
  if (meta == nullptr)
    return false;
  return meta->ContainsAttribute(attributeName);
}

bool RaverieSpirVFrontEnd::CheckForNonCopyableType(RaverieShaderIRType* shaderType,
                                                 Raverie::ExpressionNode* node,
                                                 RaverieSpirVFrontEndContext* context,
                                                 bool generateDummyResult)
{
  RaverieShaderIRType* shaderValueType = shaderType;
  if (shaderValueType->IsPointerType())
    shaderValueType = shaderValueType->mDereferenceType;

  if (ContainsAttribute(shaderValueType, SpirVNameSettings::mNonCopyableAttributeName))
  {
    String msg = String::Format("Type '%s' cannot be copied.", shaderValueType->mName.c_str());
    SendTranslationError(node->Location, msg);
    if (generateDummyResult)
      context->PushIRStack(GenerateDummyIR(node, context));
    return true;
  }
  return false;
}

RaverieShaderIRType* RaverieSpirVFrontEnd::FindType(Raverie::Type* type, Raverie::SyntaxNode* syntaxNode, bool reportErrors)
{
  RaverieShaderIRType* shaderType = mLibrary->FindType(type);
  if (shaderType != nullptr)
    return shaderType;

  if (reportErrors)
  {
    String msg = String::Format("Failed to find type '%s'", type->ToString().c_str());
    Error("%s", msg.c_str());
    SendTranslationError(syntaxNode->Location, msg);
  }

  // Return some default type so that we don't crash on null
  return mLibrary->FindType(RaverieTypeId(void));
}

RaverieShaderIRType* RaverieSpirVFrontEnd::FindType(Raverie::ExpressionNode* syntaxNode, bool reportErrors)
{
  return FindType(syntaxNode->ResultType, syntaxNode, reportErrors);
}

bool RaverieSpirVFrontEnd::ValidateResultType(IRaverieShaderIR* instruction,
                                            ShaderIRTypeBaseType::Enum expectedType,
                                            Raverie::CodeLocation& codeLocation,
                                            bool throwException)
{
  RaverieShaderIROp* op = instruction->As<RaverieShaderIROp>();
  if (op->mResultType != nullptr)
  {
    if (op->mResultType->mBaseType == expectedType)
      return true;
  }
  SendTranslationError(codeLocation, "Invalid result type");
  return false;
}

bool RaverieSpirVFrontEnd::ValidateLValue(RaverieShaderIROp* op, Raverie::CodeLocation& codeLocation, bool throwException)
{
  OpType opType = op->mOpType;
  if (opType == OpType::OpSpecConstant || opType == OpType::OpSpecConstantComposite)
  {
    String errMsg = String::Format("Specialization constants are read-only");
    SendTranslationError(codeLocation, errMsg);
    return false;
  }

  return true;
}

IRaverieShaderIR* RaverieSpirVFrontEnd::GenerateDummyIR(Raverie::ExpressionNode* node, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* shaderResultType = FindType(node->ResultType, node, context);
  return BuildOpVariable(shaderResultType, context);
}

// Send a translation error with a simple message (also marks the translation as
// having failed)
void RaverieSpirVFrontEnd::SendTranslationError(Raverie::CodeLocation& codeLocation, StringParam message)
{
  SendTranslationError(codeLocation, message, message);
}

void RaverieSpirVFrontEnd::SendTranslationError(Raverie::CodeLocation& codeLocation,
                                              StringParam shortMsg,
                                              StringParam fullMsg)
{
  mErrorTriggered = true;
  if (mProject != nullptr)
    mProject->SendTranslationError(codeLocation, shortMsg, fullMsg);
}

void RaverieSpirVFrontEnd::SendTranslationError(Raverie::CodeLocation* codeLocation, StringParam message)
{
  if (codeLocation != nullptr)
  {
    SendTranslationError(*codeLocation, message);
    return;
  }

  Raverie::CodeLocation dummyLocation;
  SendTranslationError(dummyLocation, message);
}

} // namespace Raverie
