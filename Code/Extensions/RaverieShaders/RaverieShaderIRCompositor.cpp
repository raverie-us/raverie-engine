// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

ShaderCapabilities::ShaderCapabilities()
{
  mSupportedStages.SetFlag(ShaderStage::Vertex | ShaderStage::Geometry | ShaderStage::Pixel);
}

CompositorShaderStage::Enum FragmentTypeToCompostitorShaderStage(FragmentType::Enum fragmentType)
{
  if (fragmentType == FragmentType::Vertex)
    return CompositorShaderStage::Vertex;
  else if (fragmentType == FragmentType::Geometry)
    return CompositorShaderStage::Geometry;
  else if (fragmentType == FragmentType::Pixel)
    return CompositorShaderStage::Pixel;

  Error("Invalid fragment type");
  return (CompositorShaderStage::Enum)0;
}

RaverieShaderIRCompositor::FragmentDescriptionMap::~FragmentDescriptionMap()
{
  Clear();
}

void RaverieShaderIRCompositor::FragmentDescriptionMap::Clear()
{
  AutoDeclare(values, Values());
  for (; !values.Empty(); values.PopFront())
    delete values.Front();
}

RaverieShaderIRCompositor::ShaderStageDescription::~ShaderStageDescription()
{
  Clear();
}

void RaverieShaderIRCompositor::ShaderStageDescription::Clear()
{
}

RaverieShaderIRCompositor::ShaderDefinition::ShaderDefinition()
{
  for (size_t i = 0; i < FragmentType::Size; ++i)
    mResults[i].mFragmentType = (FragmentType::Enum)i;
}

//-------------------------------------------------------------------ComputeShaderProperties
RaverieShaderIRCompositor::ComputeShaderProperties::ComputeShaderProperties()
{
  mLocalSizeX = mLocalSizeY = mLocalSizeZ = 1;
}

RaverieShaderIRCompositor::ComputeShaderProperties::ComputeShaderProperties(int localSizeX,
                                                                          int localSizeY,
                                                                          int localSizeZ)
{
  mLocalSizeX = localSizeX;
  mLocalSizeY = localSizeY;
  mLocalSizeZ = localSizeZ;
}

RaverieShaderIRCompositor::RaverieShaderIRCompositor()
{
  mComputeShaderProperties = nullptr;
}

bool RaverieShaderIRCompositor::Composite(ShaderDefinition& shaderDef,
                                        const ShaderCapabilities& capabilities,
                                        RaverieShaderSpirVSettingsRef& settings)
{
  mSettings = settings;
  mCapabilities = capabilities;
  mShaderCompositeName = shaderDef.mShaderName;

  CompositedShaderInfo compositeInfo;
  // Collect fragments per stage
  CollectFragmentsPerStage(shaderDef.mFragments, compositeInfo);
  // Validate that no unexpected stages exist given the capabilities
  bool isValid = ValidateStages(compositeInfo);
  if (!isValid)
    return false;

  // Figure out what each stage could output. This is necessary to actually link
  // any inputs.
  CollectExpectedOutputs(compositeInfo);

  // Determine what kinds of input each fragment property is. This includes
  // determining if a property is a stage input given the expected outputs.
  for (size_t i = 1; i < compositeInfo.mActiveStages.Size() - 1; ++i)
  {
    StageLinkingInfo* currentStage = compositeInfo.mActiveStages[i];
    StageLinkingInfo* previousStage = compositeInfo.mActiveStages[i - 1];
    ResolveInputs(previousStage, currentStage);
  }
  // The gpu stage is special as it's the only stage where we don't have
  // fragments to walk for inputs. Instead we have to walk the possible render
  // targets and check for stage outputs from previous stages.
  ResolveGpuStage(compositeInfo);

  // Fix the order of stage in/out variables so that the declaration order will
  // match. In a simple scenario the fields will already be in the right order
  // because they're declared when linked. When pass-through happen though the
  // order can get mixed up. For example, a pixel's first input can trigger a
  // pass-through which adds this to the end of the vertex stage, the geometry
  // stage has to make sure it matches these in/outs in the right order.
  ResolveStageLinkOrder(compositeInfo);

  // Now generate the actual raverie composite string
  for (size_t i = 1; i < compositeInfo.mActiveStages.Size() - 1; ++i)
  {
    StageLinkingInfo* currentStage = compositeInfo.mActiveStages[i];
    GenerateRaverieComposite(currentStage, shaderDef.mResults[currentStage->mFragmentType], shaderDef.mExtraAttributes);
  }

  GenerateStageDescriptions(compositeInfo, shaderDef);

  return true;
}

bool RaverieShaderIRCompositor::CompositeCompute(ShaderDefinition& shaderDef,
                                               ComputeShaderProperties* computeProperties,
                                               const ShaderCapabilities& capabilities,
                                               RaverieShaderSpirVSettingsRef& settings)
{
  mSettings = settings;
  mCapabilities = capabilities;
  mShaderCompositeName = shaderDef.mShaderName;
  mComputeShaderProperties = computeProperties;

  Array<RaverieShaderIRType*> fragments;
  // Find all compute fragments
  for (size_t i = 0; i < shaderDef.mFragments.Size(); ++i)
  {
    RaverieShaderIRType* shaderType = shaderDef.mFragments[i];
    ShaderIRTypeMeta* fragmentMeta = shaderType->mMeta;
    // If this is a valid fragment type
    if (fragmentMeta != nullptr && fragmentMeta->mFragmentType == FragmentType::Compute)
      fragments.PushBack(shaderType);
  }

  // Make sure there are fragments to composite together
  if (fragments.Empty())
    return false;

  // Create 3 stages (cpu, compute, gpu). This is required for re-using a lot of
  // functionality.
  StageLinkingInfo stages[3];
  // Do so basic setup on the stage linking info
  for (size_t i = 0; i < 3; ++i)
  {
    StageLinkingInfo& stage = stages[i];
    stage.Clear();
    stage.mVertexLinkingInfo.mOwningStage = &stage;
    stage.mPrimitiveLinkingInfo.mOwningStage = &stage;
  }

  StageLinkingInfo& cpuStageInfo = stages[0];
  StageLinkingInfo& computeStageInfo = stages[1];
  StageLinkingInfo& gpuStageInfo = stages[2];
  computeStageInfo.mShaderStage = ShaderStage::Compute;
  computeStageInfo.mFragmentType = FragmentType::Compute;
  computeStageInfo.mFragmentTypes = fragments;
  computeStageInfo.mInputVertexTypes = fragments;
  computeStageInfo.mOutputVertexTypes = fragments;

  // Create a composite info with the current active stages
  CompositedShaderInfo compositeInfo;
  compositeInfo.mActiveStages.PushBack(&cpuStageInfo);
  compositeInfo.mActiveStages.PushBack(&computeStageInfo);
  compositeInfo.mActiveStages.PushBack(&gpuStageInfo);

  ShaderStageDescription& stageDesc = shaderDef.mResults[computeStageInfo.mFragmentType];

  // Resolve inputs (maps input types and whatnot)
  ResolveInputs(&cpuStageInfo, &computeStageInfo);
  // Generate the compute shader
  GenerateRaverieComposite(&computeStageInfo, stageDesc, shaderDef.mExtraAttributes);
  // Generate reflection info
  GenerateStageDescriptions(compositeInfo, shaderDef);
  return true;
}

void RaverieShaderIRCompositor::CollectFragmentsPerStage(Array<RaverieShaderIRType*>& inputFragments,
                                                       CompositedShaderInfo& compositeInfo)
{
  // Reset all stages
  for (size_t i = 0; i < CompositorShaderStage::Size; ++i)
  {
    StageLinkingInfo& stage = compositeInfo.mStages[i];
    stage.Clear();
    stage.mVertexLinkingInfo.mOwningStage = &stage;
    stage.mPrimitiveLinkingInfo.mOwningStage = &stage;
  }

  // Iterate over all fragments and map them to the correct compositor shader
  // stage
  for (size_t i = 0; i < inputFragments.Size(); ++i)
  {
    RaverieShaderIRType* shaderType = inputFragments[i];
    ShaderIRTypeMeta* fragmentMeta = shaderType->mMeta;
    // If this is a valid fragment type
    if (fragmentMeta != nullptr && fragmentMeta->mFragmentType != FragmentType::None)
    {
      // Add this fragment to the corresponding compositor shader stage (not to
      // be confused with 'ShaderStage')
      CompositorShaderStage::Enum compostiorStage = FragmentTypeToCompostitorShaderStage(fragmentMeta->mFragmentType);
      StageLinkingInfo& stageInfo = compositeInfo.mStages[compostiorStage];
      stageInfo.mFragmentTypes.PushBack(shaderType);
    }
  }

  // Special case the various stages

  // The vertex stage has all fragments map to the in and out vertex types.
  StageLinkingInfo& vertexStageInfo = compositeInfo.mStages[CompositorShaderStage::Vertex];
  vertexStageInfo.mInputVertexTypes = vertexStageInfo.mFragmentTypes;
  vertexStageInfo.mOutputVertexTypes = vertexStageInfo.mInputVertexTypes;
  vertexStageInfo.SetFragmentType(FragmentType::Vertex);

  // The same is true for the pixel stage (they operator on interpolated vertex
  // data)
  StageLinkingInfo& pixelStageInfo = compositeInfo.mStages[CompositorShaderStage::Pixel];
  pixelStageInfo.mInputVertexTypes = pixelStageInfo.mFragmentTypes;
  pixelStageInfo.mOutputVertexTypes = pixelStageInfo.mInputVertexTypes;
  pixelStageInfo.SetFragmentType(FragmentType::Pixel);

  // Geometry shaders are the first more complicated stage
  StageLinkingInfo& geometryStageInfo = compositeInfo.mStages[CompositorShaderStage::Geometry];
  geometryStageInfo.mPrimitiveTypes = geometryStageInfo.mFragmentTypes;
  geometryStageInfo.SetFragmentType(FragmentType::Geometry);
  // The fragments define the primitive in/outs for the geometry shader stage
  geometryStageInfo.mPrimitiveTypes = geometryStageInfo.mFragmentTypes;
  if (!geometryStageInfo.mPrimitiveTypes.Empty())
  {
    RaverieShaderIRType* geometryFragmentType = geometryStageInfo.mPrimitiveTypes[0];
    Raverie::GeometryFragmentUserData* geometryUserData =
        geometryFragmentType->mRaverieType->Has<Raverie::GeometryFragmentUserData>();
    ErrorIf(geometryUserData == nullptr, "Geometry Fragment is missing user data");

    // The vertex in/outs come from the in/out stream types in the main of the
    // geometry fragment.
    RaverieShaderIRType* inputVertexType = geometryUserData->GetInputVertexType();
    RaverieShaderIRType* outputVertexType = geometryUserData->GetOutputVertexType();
    geometryStageInfo.mInputVertexTypes.PushBack(inputVertexType);
    geometryStageInfo.mOutputVertexTypes.PushBack(outputVertexType);
  }

  // Now record what stages are actually active. A stage is active if it has any
  // fragments. The cpu, vertex, pixel, and gpu stage are always active though.
  // The hardware stages are always present and vertex/pixel shaders have to
  // exist. They also must be auto-generated for pass-through to work.
  for (size_t i = 0; i < CompositorShaderStage::Size; ++i)
  {
    StageLinkingInfo* stageInfo = &compositeInfo.mStages[i];
    if (i == CompositorShaderStage::Cpu || i == CompositorShaderStage::Gpu || i == CompositorShaderStage::Vertex ||
        i == CompositorShaderStage::Pixel || !stageInfo->mFragmentTypes.Empty())
      compositeInfo.mActiveStages.PushBack(stageInfo);
  }

  // For each active stage, link the attachment sub-stages to their previous and
  // next respective attachment sub-stages. This is necessary when resolving
  // stage in/outs to facilitate pass-through.
  for (size_t i = 0; i < compositeInfo.mActiveStages.Size() - 1; ++i)
  {
    StageLinkingInfo* currStage = compositeInfo.mActiveStages[i];
    StageLinkingInfo* nextStage = compositeInfo.mActiveStages[i + 1];

    currStage->mVertexLinkingInfo.mNextStage = &nextStage->mVertexLinkingInfo;
    currStage->mPrimitiveLinkingInfo.mNextStage = &nextStage->mPrimitiveLinkingInfo;

    nextStage->mVertexLinkingInfo.mPreviousStage = &currStage->mVertexLinkingInfo;
    nextStage->mPrimitiveLinkingInfo.mPreviousStage = &currStage->mPrimitiveLinkingInfo;
  }
}

bool RaverieShaderIRCompositor::ValidateStages(CompositedShaderInfo& compositeInfo)
{
  // Currently only check for an active stage that our capabilities don't
  // support.
  for (size_t i = 0; i < compositeInfo.mActiveStages.Size(); ++i)
  {
    StageLinkingInfo* stageInfo = compositeInfo.mActiveStages[i];
    // Skip Cpu/Gpu stages
    if (stageInfo->mShaderStage == 0)
      continue;

    bool stageSupported = mCapabilities.mSupportedStages.IsSet(stageInfo->mShaderStage);
    bool stageFragmentsExist = !stageInfo->mFragmentTypes.Empty();
    ReturnIf(!stageSupported && stageFragmentsExist,
             false,
             "ShaderStage isn't supported according to the given capabilities.");

    for (size_t i = 0; i < stageInfo->mFragmentTypes.Size(); ++i)
    {
      RaverieShaderIRType* fragmentType = stageInfo->mFragmentTypes[i];
      if (!fragmentType->mHasMainFunction)
      {
        Raverie::BoundType* raverieType = fragmentType->mMeta->mRaverieType;
        ReturnIf(raverieType == nullptr, false, "Cannot composite a fragment with no backing raverie type");

        String msg;
        if (stageInfo->mFragmentType == FragmentType::Geometry)
          msg = String::Format("Geometry fragment '%s' must have a 'Main' function of "
                               "signature(InputStream, OutputStream).",
                               raverieType->Name.c_str());
        else
          msg = String::Format("Fragment '%s' must have a a function of signature 'Main()'.", raverieType->Name.c_str());
        SendTranslationError(raverieType->Location, msg);
        return false;
      }
    }

    // Check geometry shaders to make sure they only have one fragment
    if (stageInfo->mFragmentType == FragmentType::Geometry)
    {
      RaverieShaderIRType* fragmentType = stageInfo->mFragmentTypes[0];
      if (stageInfo->mFragmentTypes.Size() != 1)
      {
        Raverie::BoundType* raverieType = fragmentType->mMeta->mRaverieType;
        SendTranslationError(raverieType->Location, "Geometry shader stage only supports one fragment at a time.");
        return false;
      }
    }
  }

  return true;
}

void RaverieShaderIRCompositor::CollectExpectedOutputs(CompositedShaderInfo& compositeInfo)
{
  // Create the cpu stage so that we can propagate vertex definition properties
  CreateCpuStage(compositeInfo);

  // Collect each stage's expected outputs
  for (size_t i = 0; i < compositeInfo.mActiveStages.Size(); ++i)
  {
    StageLinkingInfo* stage = compositeInfo.mActiveStages[i];
    CollectExpectedOutputs(stage->mOutputVertexTypes, stage->mVertexLinkingInfo);
    CollectExpectedOutputs(stage->mPrimitiveTypes, stage->mPrimitiveLinkingInfo);
  }

  // Facilitate pass-through by copying all previous stage expected outputs down
  for (size_t i = 1; i < compositeInfo.mActiveStages.Size() - 1; ++i)
  {
    StageLinkingInfo* currentStage = compositeInfo.mActiveStages[i];
    StageLinkingInfo* previousStage = compositeInfo.mActiveStages[i - 1];
    // Any previous stage's expected output could be the current stage's output
    // due to pass-through
    AutoDeclare(previousVertexOuts, previousStage->mVertexLinkingInfo.mExpectedOutputs.All());
    for (; !previousVertexOuts.Empty(); previousVertexOuts.PopFront())
    {
      AutoDeclare(pair, previousVertexOuts.Front());
      ExpectedOutputData& passThroughData = pair.second;
      currentStage->mVertexLinkingInfo.mExpectedOutputs.InsertOrIgnore(pair.first, passThroughData);
    }
  }
}

void RaverieShaderIRCompositor::CollectExpectedOutputs(Array<RaverieShaderIRType*>& fragmentTypes,
                                                     StageAttachmentLinkingInfo& linkingInfo)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  FragmentType::Enum currentFragmentType = linkingInfo.mOwningStage->mFragmentType;

  // For all fragments, find every field that contains the StageOutput
  // attribute and add this to the current stage's potential outputs.
  // Outputs are only actual outputs if a subsequent stage inputs it.
  for (size_t i = 0; i < fragmentTypes.Size(); ++i)
  {
    RaverieShaderIRType* shaderType = fragmentTypes[i];

    ShaderIRTypeMeta* typeMeta = shaderType->mMeta;
    for (size_t fieldIndex = 0; fieldIndex < typeMeta->mFields.Size(); ++fieldIndex)
    {
      ShaderIRFieldMeta* fieldMeta = typeMeta->mFields[fieldIndex];

      for (size_t attributeIndex = 0; attributeIndex < fieldMeta->mAttributes.Size(); ++attributeIndex)
      {
        ShaderIRAttribute* attribute = fieldMeta->mAttributes.GetAtIndex(attributeIndex);
        if (attribute->mAttributeName == nameSettings.mStageOutputAttribute)
        {
          ShaderFieldKey fieldKey = MakeFieldKey(fieldMeta, attribute);
          ExpectedOutputData outData(fieldMeta, currentFragmentType);
          linkingInfo.mExpectedOutputs.InsertOrOverride(fieldKey, outData);
        }
      }
    }
  }
}

void RaverieShaderIRCompositor::CreateCpuStage(CompositedShaderInfo& compositeInfo)
{
  StageLinkingInfo& cpuStageInfo = compositeInfo.mStages[CompositorShaderStage::Cpu];

  // Add all vertex definitions as potential outputs of the cpu stage
  VertexDefinitionDescription& vertexDefinition = mSettings->mVertexDefinitions;
  for (size_t i = 0; i < vertexDefinition.mFields.Size(); ++i)
  {
    ShaderIRFieldMeta* fieldMeta = vertexDefinition.mFields[i];
    ExpectedOutputData outData(fieldMeta, cpuStageInfo.mFragmentType);
    cpuStageInfo.mVertexLinkingInfo.mExpectedOutputs.InsertInternal(fieldMeta->MakeFieldKey(), outData);
  }
}

void RaverieShaderIRCompositor::ResolveInputs(StageLinkingInfo* previousStage, StageLinkingInfo* currentStage)
{
  // Link together the two stages. This requires linking the vertex to vertex
  // and primitive to primitive. Primitive stages are mostly for
  // geometry/tessellation.
  Link(previousStage->mVertexLinkingInfo,
       currentStage,
       currentStage->mInputVertexTypes,
       currentStage->mVertexLinkingInfo);
  Link(previousStage->mPrimitiveLinkingInfo,
       currentStage,
       currentStage->mPrimitiveTypes,
       currentStage->mPrimitiveLinkingInfo);
}

void RaverieShaderIRCompositor::Link(StageAttachmentLinkingInfo& prevStageInfo,
                                   StageLinkingInfo* currentStage,
                                   Array<RaverieShaderIRType*>& fragmentTypes,
                                   StageAttachmentLinkingInfo& currStageInfo)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  FragmentType::Enum stageFragmentType = currStageInfo.mOwningStage->mFragmentType;
  BuiltInStageDescription& builtInStageDescription = mSettings->mBuiltIns[stageFragmentType];

  // Keep track of what fragment has last output a property
  HashMap<ShaderFieldKey, ShaderIRFieldMeta*> fragmentOutputs;

  // Iterate over all of the fragment types (in order) to try and match inputs
  for (size_t fragIndex = 0; fragIndex < fragmentTypes.Size(); ++fragIndex)
  {
    RaverieShaderIRType* fragmentType = fragmentTypes[fragIndex];
    ShaderIRTypeMeta* fragmentTypeMeta = fragmentType->mMeta;

    // Create the information about how we linked the fragment's properties
    FragmentLinkingInfo& fragmentLinkInfo = currentStage->mFragmentLinkInfoMap[fragmentType];
    fragmentLinkInfo.mMeta = fragmentTypeMeta;

    // Walk all fields/properties
    for (size_t fieldIndex = 0; fieldIndex < fragmentTypeMeta->mFields.Size(); ++fieldIndex)
    {
      ShaderIRFieldMeta* fieldMeta = fragmentTypeMeta->mFields[fieldIndex];

      // Non-copyable fields (like samplers) cannot be composited as they can't
      // be copied. These fields are instead output by the owning fragment. We
      // do however need to generate reflection info for these fields (if
      // they're properties)
      if (fieldMeta->mRaverieType->HasAttribute(nameSettings.mNonCopyableAttributeName))
      {
        // Walk all attributes to find the first input/output
        // attribute to use for resolution (PropertyInput or StageOutput)
        for (size_t attributeIndex = 0; attributeIndex < fieldMeta->mAttributes.Size(); ++attributeIndex)
        {
          ShaderIRAttribute* attribute = fieldMeta->mAttributes.GetAtIndex(attributeIndex);
          if (attribute->mAttributeName == nameSettings.mPropertyInputAttribute ||
              attribute->mAttributeName == nameSettings.mStageOutputAttribute)
          {
            String fieldName = GetFieldInOutName(fieldMeta, attribute);
            // @JoshD: Temporarily use the original field name. Currently
            // non-copyable types do not use the property variable's name
            // override. This is reasonably ok for now as this is only a name
            // generated during reflection and doesn't actually deal with
            // input/output property mappings.
            fieldName = fieldMeta->mRaverieName;

            String propertyName = MakePropertyName(fieldName, fieldMeta->mOwner->mRaverieName);
            FieldLinkingInfo& fieldLinkInfo = fragmentLinkInfo.mNonCopyableProperties.PushBack();
            fieldLinkInfo.mFieldPropertyName = propertyName;
            fieldLinkInfo.mLinkedType = LinkedFieldType::Property;
            fieldLinkInfo.mFieldMeta = fieldMeta;
            break;
          }
        }
        continue;
      }

      // Create the information about how this field was linked within this
      // shader stage
      ShaderFieldKey linkFieldKey = fieldMeta->MakeFieldKey();
      FieldLinkingInfo& fieldLinkInfo = fragmentLinkInfo.mFieldMap[linkFieldKey];
      fragmentLinkInfo.mFieldList.PushBack(linkFieldKey);

      fieldLinkInfo.mLinkedType = LinkedFieldType::None;
      fieldLinkInfo.mFieldMeta = fieldMeta;

      // Walk all attributes in order
      for (size_t attributeIndex = 0; attributeIndex < fieldMeta->mAttributes.Size(); ++attributeIndex)
      {
        ShaderIRAttribute* attribute = fieldMeta->mAttributes.GetAtIndex(attributeIndex);

        // Check if this is a fragment input attribute
        if (attribute->mAttributeName == nameSettings.mFragmentInputAttribute)
        {
          // If the fragment input matches a previous fragment's output then
          // this attribute is satisfied
          ShaderFieldKey fieldKey = MakeFieldKey(fieldMeta, attribute);
          ShaderIRFieldMeta* fragmentOutput = fragmentOutputs.FindValue(fieldKey, nullptr);
          if (fragmentOutput != nullptr)
          {
            // Mark which fragment fulfills the input
            fieldLinkInfo.mOutputFieldDependency = fragmentOutput;
            fieldLinkInfo.mLinkedType = LinkedFieldType::Fragment;
            break;
          }
        }
        // Check if this is a stage input (previous shader stage)
        if (attribute->mAttributeName == nameSettings.mStageInputAttribute)
        {
          // If the stage input matches a previous stage's potential output then
          // this attribute is satisfied
          ShaderFieldKey fieldKey = MakeFieldKey(fieldMeta, attribute);
          ExpectedOutputData* previousStageExpectedOutput = prevStageInfo.mExpectedOutputs.FindPointer(fieldKey);
          if (previousStageExpectedOutput != nullptr)
          {
            String fieldVarName, fieldAttributeName;
            GetStageFieldName(fieldMeta, attribute, fieldVarName, fieldAttributeName);
            fieldLinkInfo.mLinkedType = LinkedFieldType::Stage;
            fieldLinkInfo.mOutputFieldDependencyName = fieldVarName;
            // A stage output only becomes an actual output if an input matches
            // it. Mark this property as an input/output between the given
            // stages (including pass-through).
            AddStageInput(previousStageExpectedOutput, &currStageInfo, fieldVarName, fieldAttributeName);
            break;
          }
        }
        // Check for application built-in inputs (pre-defined uniform buffers)
        if (attribute->mAttributeName == nameSettings.mAppBuiltInInputAttribute)
        {
          ShaderFieldKey fieldKey = MakeFieldKey(fieldMeta, attribute);
          ShaderIRFieldMeta* appFieldMeta = FindUniform(fieldKey, fragmentTypeMeta->mFragmentType);
          if (appFieldMeta != nullptr)
          {
            fieldLinkInfo.mLinkedType = LinkedFieldType::AppBuiltIn;
            fieldLinkInfo.mOutputFieldDependency = appFieldMeta;
            currStageInfo.AddResolvedField(appFieldMeta, nameSettings.mAppBuiltInInputAttribute);
            break;
          }
        }
        // Check for hardware built-in inputs (described by the gpu hardware)
        if (attribute->mAttributeName == nameSettings.mHardwareBuiltInInputAttribute)
        {
          ShaderFieldKey fieldKey = MakeFieldKey(fieldMeta, attribute);
          ShaderIRFieldMeta* builtInFieldMeta = FindHardwareBuiltInInput(fieldKey, stageFragmentType);
          if (builtInFieldMeta != nullptr)
          {
            fieldLinkInfo.mLinkedType = LinkedFieldType::HardwareBuiltIn;
            fieldLinkInfo.mOutputFieldDependency = builtInFieldMeta;
            currStageInfo.mHardwareInputs.InsertOrIgnore(fieldKey);
            currStageInfo.AddResolvedField(builtInFieldMeta, nameSettings.mHardwareBuiltInInputAttribute);
            break;
          }
        }
        // Check if this is a property input.
        if (attribute->mAttributeName == nameSettings.mPropertyInputAttribute)
        {
          // Property inputs can always be fulfilled. Make sure to
          // resolve this as a property input (special name mangling)
          currStageInfo.AddResolvedFieldProperty(fieldMeta, attribute);
          fieldLinkInfo.mLinkedType = LinkedFieldType::Property;
          fieldLinkInfo.mOutputFieldDependencyName = GetFieldInOutName(fieldMeta, attribute);
          // Make the property name (mangled to prevent conflicts)
          fieldLinkInfo.mFieldPropertyName =
              MakePropertyName(fieldLinkInfo.mOutputFieldDependencyName, fieldMeta->mOwner->mRaverieName);
          break;
        }
        // Check if this is a specialization constant input.
        if (attribute->mAttributeName == nameSettings.mSpecializationConstantInputAttribute)
        {
          // Specialization constant inputs are treated almost the exact same as
          // property inputs.
          fieldLinkInfo.mLinkedType = LinkedFieldType::SpecConstant;
          fieldLinkInfo.mOutputFieldDependencyName = GetFieldInOutName(fieldMeta, attribute);
          // Make the property name (mangled to prevent conflicts)
          fieldLinkInfo.mFieldPropertyName =
              MakePropertyName(fieldLinkInfo.mOutputFieldDependencyName, fieldMeta->mOwner->mRaverieName);

          String fieldType = fieldMeta->mRaverieType->ToString();
          ResolvedFieldInfo* fieldInfo = currStageInfo.CreateResolvedField(fieldLinkInfo.mFieldPropertyName, fieldType);

          // The main difference is the attributes we add, in particular the
          // actual specialization constant must be declared as a static
          // variable.
          fieldInfo->mAttributes.AddAttribute(nameSettings.mStaticAttribute, nullptr);
          fieldInfo->mAttributes.AddAttribute(nameSettings.mSpecializationConstantAttribute, nullptr);
          break;
        }
      }

      // Check for stage output attributes. We have to loop to check for
      // multiple fulfillment (due to name overrides)
      for (size_t attributeIndex = 0; attributeIndex < fieldMeta->mAttributes.Size(); ++attributeIndex)
      {
        ShaderIRAttribute* attribute = fieldMeta->mAttributes.GetAtIndex(attributeIndex);

        // Check if this is a fragment output, if so then park it as the last
        // fragment to output this field
        if (attribute->mAttributeName == nameSettings.mFragmentOutputAttribute)
        {
          ShaderFieldKey fieldKey = MakeFieldKey(fieldMeta, attribute);
          fragmentOutputs.Insert(fieldKey, fieldMeta);
          continue;
        }
        // Check for hardware built-in outputs (gpu built-ins). This has to
        // match a pre-existing built-in to be valid.
        if (attribute->mAttributeName == nameSettings.mHardwareBuiltInOutputAttribute)
        {
          ShaderFieldKey fieldKey = MakeFieldKey(fieldMeta, attribute);
          ShaderIRFieldMeta* builtInFieldMeta = FindHardwareBuiltInOutput(fieldKey, stageFragmentType);
          if (builtInFieldMeta != nullptr)
          {
            // Mark this as a field with [HardwareBuiltInOutput]
            currStageInfo.AddResolvedField(builtInFieldMeta, nameSettings.mHardwareBuiltInOutputAttribute);
            currStageInfo.mHardwareOutputs.InsertOrIgnore(fieldKey);
            // Also mark this as a resolved output by the built-in name
            ResolvedFieldOutputInfo resolvedFieldOutput(builtInFieldMeta->mRaverieName, fieldMeta);
            currStageInfo.mResolvedOutputs.InsertOrOverride(fieldKey, resolvedFieldOutput);
            continue;
          }
        }
      }
    }
  }
}

void RaverieShaderIRCompositor::AddStageInput(ExpectedOutputData* previousStageOutputData,
                                            StageAttachmentLinkingInfo* currStageAttachment,
                                            StringParam fieldVarName,
                                            StringParam fieldAttributeName)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  ShaderIRFieldMeta* previousStageOutputMeta = previousStageOutputData->mOutputFieldDependency;

  // Record the start/end stage for this input.
  FragmentType::Enum stageOutputFragmentType = previousStageOutputData->mFragmentType;
  FragmentType::Enum stageInputFragmentType = currStageAttachment->mOwningStage->mFragmentType;

  // Iterate from the current stage backwards until we reach the stage that the
  // output was last declared in. For each stage, add in/outs between the
  // current stage and the previous so that pass-through happens.
  StageAttachmentLinkingInfo* prevStageAttachment = currStageAttachment->mPreviousStage;
  while (currStageAttachment->mPreviousStage != nullptr)
  {
    prevStageAttachment->AddResolvedStageField(
        nameSettings, previousStageOutputMeta, fieldVarName, nameSettings.mStageOutputAttribute, fieldAttributeName);
    // Mark the current stage as actually having an input field
    currStageAttachment->AddResolvedStageField(
        nameSettings, previousStageOutputMeta, fieldVarName, nameSettings.mStageInputAttribute, fieldAttributeName);

    // If we've reached the stage that output the property then stop
    if (prevStageAttachment->mOwningStage->mFragmentType == stageOutputFragmentType)
      break;

    currStageAttachment = prevStageAttachment;
    prevStageAttachment = currStageAttachment->mPreviousStage;
  }

  // Mark a resolved output field for the stage that actually emitted the
  // output. This is only needed in this stage as this is the only stage that
  // has to write out the last fragment to the stage variable.
  String fieldType = previousStageOutputMeta->mRaverieType->ToString();
  ShaderFieldKey fieldKey(fieldAttributeName, fieldType);
  ResolvedFieldOutputInfo resolvedFieldOutput(fieldVarName, previousStageOutputMeta);
  resolvedFieldOutput.mOutputFieldDependency = previousStageOutputData->mOutputFieldDependency;
  prevStageAttachment->mResolvedOutputs.InsertOrOverride(fieldKey, resolvedFieldOutput);
}

ShaderIRFieldMeta* RaverieShaderIRCompositor::FindUniform(ShaderFieldKey& fieldKey, FragmentType::Enum fragmentType)
{
  ShaderStage::Enum shaderStage = FragmentTypeToShaderStage(fragmentType);
  // @JoshD: Optimize later!
  RaverieShaderSpirVSettings* settings = mSettings;
  for (size_t bufferIndex = 0; bufferIndex < settings->mUniformBufferDescriptions.Size(); ++bufferIndex)
  {
    UniformBufferDescription& buffer = settings->mUniformBufferDescriptions[bufferIndex];
    // Make sure this buffer allows the current stage
    if (!buffer.mAllowedStages.IsSet(shaderStage))
      continue;

    // Iterate over all uniform fields in this buffer and see if they match
    for (size_t fieldIndex = 0; fieldIndex < buffer.mFields.Size(); ++fieldIndex)
    {
      ShaderIRFieldMeta* fieldMeta = buffer.mFields[fieldIndex];
      if (fieldMeta->MakeFieldKey() == fieldKey)
        return fieldMeta;
    }
  }
  return nullptr;
}

ShaderIRFieldMeta* RaverieShaderIRCompositor::FindHardwareBuiltInInput(ShaderFieldKey& fieldKey,
                                                                     FragmentType::Enum fragmentType)
{
  RaverieShaderSpirVSettings* settings = mSettings;
  BuiltInStageDescription& builtInStageDescription = settings->mBuiltIns[fragmentType];

  BuiltInBlockDescription* blockDescription =
      builtInStageDescription.mInternalInputMappings.FindValue(fieldKey, nullptr);
  if (blockDescription == nullptr)
    return nullptr;

  return blockDescription->FindField(fieldKey)->mMeta;
}

ShaderIRFieldMeta* RaverieShaderIRCompositor::FindHardwareBuiltInOutput(ShaderFieldKey& fieldKey,
                                                                      FragmentType::Enum fragmentType)
{
  RaverieShaderSpirVSettings* settings = mSettings;
  BuiltInStageDescription& builtInStageDescription = settings->mBuiltIns[fragmentType];

  BuiltInBlockDescription* blockDescription =
      builtInStageDescription.mInternalOutputMappings.FindValue(fieldKey, nullptr);
  if (blockDescription == nullptr)
    return nullptr;

  return blockDescription->FindField(fieldKey)->mMeta;
}

ShaderFieldKey RaverieShaderIRCompositor::MakeFieldKey(ShaderIRFieldMeta* fieldMeta, ShaderIRAttribute* attribute)
{
  String fieldName = GetFieldInOutName(fieldMeta, attribute);
  return ShaderFieldKey(fieldName, fieldMeta->mRaverieType->ToString());
}

String RaverieShaderIRCompositor::MakePropertyName(StringParam fieldName, StringParam ownerType)
{
  return GenerateSpirVPropertyName(fieldName, ownerType);
}

String RaverieShaderIRCompositor::GetFieldInOutName(ShaderIRFieldMeta* fieldMeta, ShaderIRAttribute* attribute)
{
  return fieldMeta->GetFieldAttributeName(attribute);
}

void RaverieShaderIRCompositor::GetStageFieldName(ShaderIRFieldMeta* fieldMeta,
                                                ShaderIRAttribute* attribute,
                                                String& fieldVarNameOut,
                                                String& fieldAttributeNameOut)
{
  // Get the target field name given the current attribute. This will be the
  // target name for translation (aka the attribute parameter 'name' value)
  fieldAttributeNameOut = GetFieldInOutName(fieldMeta, attribute);
  // The actual field name needs to be mangled to avoid conflicts. To do this
  // pre-pend with 'stage' and post-pend with the type. Type is needed to
  // prevent conflicts between different stage variables with the same name but
  // different types. 'Stage' is needed to prevent conflicts with BuiltIns.
  fieldVarNameOut = BuildString("Stage_", fieldAttributeNameOut, "_", fieldMeta->mRaverieType->ToString());
}

void RaverieShaderIRCompositor::ResolveGpuStage(CompositedShaderInfo& compositeInfo)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  Raverie::BoundType* renderTargetType = mSettings->mRenderTargetType;

  StageLinkingInfo& gpuStage = compositeInfo.mStages[CompositorShaderStage::Gpu];
  StageAttachmentLinkingInfo* lastStage = gpuStage.mVertexLinkingInfo.mPreviousStage;

  // Create a dummy attribute so as a stage input so we can re-use code
  ShaderIRAttribute stageInAttribute;
  stageInAttribute.mAttributeName = nameSettings.mStageInputAttribute;
  ShaderIRAttributeParameter& stageInAttributeParamName = stageInAttribute.mParameters.PushBack();

  RaverieShaderSpirVSettings* settings = mSettings;
  for (size_t i = 0; i < settings->mRenderTargetNames.Size(); ++i)
  {
    String renderTargetName = settings->mRenderTargetNames[i];
    ShaderFieldKey renderTargetKey(renderTargetName, renderTargetType->ToString());
    // If any previous stage output the render target name then add the render
    // target as a stage output to all required previous stages
    ExpectedOutputData* previousStageExpectedOutput = lastStage->mExpectedOutputs.FindPointer(renderTargetKey);
    if (previousStageExpectedOutput != nullptr)
    {
      // Set the target name of the input to the render target name
      stageInAttributeParamName.SetStringValue(renderTargetName);
      // Get the render target field's name and attribute name (the attribute
      // name is always renderTargetName)
      String fieldVarName, fieldAttributeName;
      GetStageFieldName(
          previousStageExpectedOutput->mOutputFieldDependency, &stageInAttribute, fieldVarName, fieldAttributeName);
      // Add all necessary stage in/out variables up the the previous stage.
      AddStageInput(previousStageExpectedOutput, &gpuStage.mVertexLinkingInfo, fieldVarName, fieldAttributeName);
    }
  }
}

void RaverieShaderIRCompositor::ResolveStageLinkOrder(CompositedShaderInfo& compositeInfo)
{
  // For all programmable shader stage pairings (e.g. no cpu/gpu), make sure the
  // in/out orders match
  for (size_t i = 1; i < compositeInfo.mActiveStages.Size() - 2; ++i)
  {
    RaverieShaderIRCompositor::StageLinkingInfo* currStage = compositeInfo.mActiveStages[i];
    RaverieShaderIRCompositor::StageLinkingInfo* nextStage = compositeInfo.mActiveStages[i + 1];

    OrderedHashSet<ShaderFieldKey>& inputs = nextStage->mVertexLinkingInfo.mInputs;
    OrderedHashSet<ShaderFieldKey>& outputs = currStage->mVertexLinkingInfo.mOutputs;

    // Error checking
    for (AutoDeclare(range, inputs.All()); !range.Empty(); range.PopFront())
    {
      ShaderFieldKey fieldKey = range.Front();
      bool matches = outputs.ContainsKey(fieldKey);
      ErrorIf(!matches, "Input '%s' not found in outputs.", fieldKey.mKey.c_str());
    }

    // The inputs and outputs should match at this point in contents, just not
    // order. Simply override the output order with the inputs
    outputs.Clear();
    AutoDeclare(range, inputs.All());
    for (; !range.Empty(); range.PopFront())
      outputs.InsertOrIgnore(range.Front());
  }
}

void RaverieShaderIRCompositor::GenerateRaverieComposite(StageLinkingInfo* currentStage,
                                                     ShaderStageDescription& stageResults,
                                                     ShaderIRAttributeList& extraAttributes)
{
  // Invoke the custom compositor callback if it exists. This allows forcing
  // certain in/outs.
  CallbackSettings& callbackSettings = mSettings->mCallbackSettings;
  if (callbackSettings.mCompositeCallback != nullptr)
  {
    CompositorCallbackData callbackData;
    callbackData.mCompositor = this;
    callbackData.mSettings = mSettings;
    callbackData.mStageLinkingInfo = currentStage;
    callbackSettings.mCompositeCallback(callbackData, callbackSettings.mCompositeCallbackUserData);
  }

  switch (currentStage->mShaderStage)
  {
  // Treat vertex and pixel stages the same
  case ShaderStage::Vertex:
  case ShaderStage::Pixel:
    GenerateBasicRaverieComposite(currentStage, stageResults, extraAttributes);
    break;
  case ShaderStage::Geometry:
    GenerateGeometryRaverieComposite(currentStage, stageResults, extraAttributes);
    break;
  case ShaderStage::Compute:
    GenerateComputeRaverieComposite(currentStage, stageResults, extraAttributes);
    break;
  default:
  {
    Error("Unexpected shader stage");
    break;
  }
  }
}

void RaverieShaderIRCompositor::GenerateBasicRaverieComposite(StageLinkingInfo* currentStage,
                                                          ShaderStageDescription& stageResults,
                                                          ShaderIRAttributeList& extraAttributes)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  StageAttachmentLinkingInfo& vertexLinkingInfo = currentStage->mVertexLinkingInfo;
  ShaderCodeBuilder builder;

  // Make the name for the fragment. For uniqueness, append the stage's name to
  // the given composite name.
  String stageName = FragmentType::Names[currentStage->mFragmentType];
  stageResults.mClassName = BuildString(mShaderCompositeName, "_", stageName);

  // Emit the struct declaration (attributes followed by name)
  builder << builder.DeclareAttribute(stageName);
  // Add any extra attributes specified by the user
  for (size_t i = 0; i < extraAttributes.Size(); ++i)
    builder.DeclareAttribute(*extraAttributes.GetAtIndex(i));
  builder << builder.EmitLineReturn();
  builder << "struct " << stageResults.mClassName << builder.EmitLineReturn();
  builder.BeginScope();

  // Write out all fields we resolve. Vertex fragments need to preserve their
  // output order so interfaces match with all subsequent stages (vertex
  // definition order doesn't matter). Every other stage can just match inputs
  // first (currently only pixel).
  if (currentStage->mFragmentType == FragmentType::Vertex)
    DeclareFieldsInOrder(builder, vertexLinkingInfo, vertexLinkingInfo.mOutputs);
  else
    DeclareFieldsInOrder(builder, vertexLinkingInfo, vertexLinkingInfo.mInputs);

  // Emit the main function with the entry point attribute
  builder.WriteLine();
  builder << builder.EmitIndent();
  builder << builder.DeclareAttribute(nameSettings.mEntryPointAttributeName);
  builder << builder.EmitLineReturn();

  builder << builder.EmitIndent() << "function Main()" << builder.EmitLineReturn();
  builder.BeginScope();

  // Emit each fragment variable, copy its inputs, then call its main
  for (size_t i = 0; i < currentStage->mFragmentTypes.Size(); ++i)
  {
    RaverieShaderIRType* fragmentType = currentStage->mFragmentTypes[i];
    FragmentLinkingInfo& linkInfo = currentStage->mFragmentLinkInfoMap[fragmentType];

    // Declare the fragment and copy its inputs
    String fragmentVarName = MakeFragmentVarName(fragmentType->mMeta);
    CreateFragmentAndCopyInputs(currentStage, builder, stageResults.mClassName, fragmentType, fragmentVarName);
    // Call the fragment's main function
    builder << builder.EmitIndent() << fragmentVarName << ".Main();" << builder.EmitLineReturn();
    builder << builder.EmitLineReturn();
  }

  // Copy outputs from the last fragment to output a stage output back into the
  // composite
  AutoDeclare(outputRange, vertexLinkingInfo.mResolvedOutputs.Values());
  for (; !outputRange.Empty(); outputRange.PopFront())
  {
    ResolvedFieldOutputInfo& fieldOutput = outputRange.Front();
    ShaderIRFieldMeta* fieldMeta = fieldOutput.mOutputFieldDependency;
    // If a field meta is null then this is a pass-through variable and no copy
    // out is required. If the owner is null then this didn't come from a
    // fragment, it came from the composite (uniform/built-in). This requires no
    // extra copy as the composite's input wasn't changed.
    if (fieldMeta == nullptr || fieldMeta->mOwner == nullptr)
      continue;

    builder << builder.EmitIndent() << "this." << fieldOutput.mFieldName << " = ";
    builder << MakeFragmentVarName(fieldMeta->mOwner) << "." << fieldMeta->mRaverieName << ";"
            << builder.EmitLineReturn();
  }

  builder.EndScope();

  builder.EndScope();

  stageResults.mShaderCode = builder.ToString();
}

void RaverieShaderIRCompositor::GenerateGeometryRaverieComposite(StageLinkingInfo* currentStage,
                                                             ShaderStageDescription& stageResults,
                                                             ShaderIRAttributeList& extraAttributes)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  ShaderCodeBuilder builder;

  // Grab all of the various fragment and stream types
  RaverieShaderIRType* geometryFragmentType = currentStage->mPrimitiveTypes[0];
  Raverie::GeometryFragmentUserData* geometryUserData =
      geometryFragmentType->mRaverieType->Has<Raverie::GeometryFragmentUserData>();
  Raverie::BoundType* raverieInputStreamType = geometryUserData->mInputStreamType->mRaverieType;
  Raverie::BoundType* raverieOutputStreamType = geometryUserData->mOutputStreamType->mRaverieType;
  Raverie::GeometryStreamUserData* inputStreamUserData = raverieInputStreamType->Has<Raverie::GeometryStreamUserData>();
  Raverie::GeometryStreamUserData* outputStreamUserData = raverieOutputStreamType->Has<Raverie::GeometryStreamUserData>();
  RaverieShaderIRType* inputVertexType = geometryUserData->GetInputVertexType();
  StageAttachmentLinkingInfo& vertexLinkingInfo = currentStage->mVertexLinkingInfo;

  // Also get the max vertices count. @JoshD: Cleanup
  ShaderIRAttribute* geometryAttribute = geometryFragmentType->FindFirstAttribute(nameSettings.mGeometryAttribute);
  ShaderIRAttributeParameter* maxVerticesParam = geometryAttribute->FindFirstParameter(nameSettings.mMaxVerticesParam);
  int maxVertices = maxVerticesParam->GetIntValue();

  // Make the name for the fragment. For uniqueness, append the stage's name to
  // the given composite name.
  String stageName = FragmentType::Names[currentStage->mFragmentType];
  stageResults.mClassName = BuildString(mShaderCompositeName, "_", stageName);
  // Also create the input/output vertex type names
  String inputTypeName = BuildString(mShaderCompositeName, "_GeoIn");
  String outputTypeName = BuildString(mShaderCompositeName, "_GeoOut");

  // Declare the input vertex type only from the resolved input fields.
  builder << "struct " << inputTypeName << builder.EmitLineReturn();
  builder.BeginScope();
  DeclareFieldsWithAttribute(builder, vertexLinkingInfo, vertexLinkingInfo.mInputs, nameSettings.mStageInputAttribute);
  DeclareFieldsWithAttribute(
      builder, vertexLinkingInfo, vertexLinkingInfo.mHardwareInputs, nameSettings.mHardwareBuiltInInputAttribute);
  builder.EndScope();

  // Declare the input vertex type only from the resolved output fields.
  builder << "struct " << outputTypeName << builder.EmitLineReturn();
  builder.BeginScope();
  DeclareFieldsWithAttribute(
      builder, vertexLinkingInfo, vertexLinkingInfo.mOutputs, nameSettings.mStageOutputAttribute);
  DeclareFieldsWithAttribute(
      builder, vertexLinkingInfo, vertexLinkingInfo.mHardwareOutputs, nameSettings.mHardwareBuiltInOutputAttribute);
  builder.EndScope();

  // Emit the struct declaration (attributes followed by name)
  builder << "[" << stageName << "(maxVertices : " << ToString(maxVertices) << ")]";
  for (size_t i = 0; i < extraAttributes.Size(); ++i)
    builder.DeclareAttribute(*extraAttributes.GetAtIndex(i));
  builder << builder.EmitLineReturn();
  builder << "struct " << stageResults.mClassName << builder.EmitLineReturn();
  builder.BeginScope();

  // Declare any primitive fields. These don't have a specific order as no stage
  // inputs exist here.
  OrderedHashSet<ShaderFieldKey> primitiveOrderingMap;
  DeclareFieldsInOrder(builder, currentStage->mPrimitiveLinkingInfo, primitiveOrderingMap);

  // Generate the composite's input/output stream names
  String compositeInputStreamType = BuildString(raverieInputStreamType->TemplateBaseName, "[", inputTypeName, "]");
  String compositeOutputStreamType = BuildString(raverieOutputStreamType->TemplateBaseName, "[", outputTypeName, "]");
  // Generate the entry points function signature: [EntryPoint] function
  // Main(input : 'InputType', output : 'OutputType')
  builder << builder.EmitIndent();
  builder.DeclareAttribute(nameSettings.mEntryPointAttributeName);
  builder << builder.EmitLineReturn();
  builder << builder.EmitIndent() << "function Main(";
  builder << "input : " << compositeInputStreamType << ", ";
  builder << "output : " << compositeOutputStreamType << ")" << builder.EmitLineReturn();
  builder.BeginScope();

  // Declare the fragment input data (this is a sub-set of the composite's data)
  builder.WriteLocalVariableDefaultConstruction("fragmentInput", raverieInputStreamType->Name);
  // Copy all inputs from the composite input vertex type to the fragment's
  builder << builder.EmitIndent() << "for(var i = 0; i < " << ToString(inputStreamUserData->mSize) << "; ++i)"
          << builder.EmitLineReturn();
  builder.BeginScope();
  // Walk the linking info of the input vertex type
  FragmentLinkingInfo& inputVertexLinkInfo = currentStage->mFragmentLinkInfoMap[inputVertexType];
  for (size_t i = 0; i < inputVertexLinkInfo.mFieldList.Size(); ++i)
  {
    ShaderFieldKey key = inputVertexLinkInfo.mFieldList[i];
    FieldLinkingInfo& linkInfo = inputVertexLinkInfo.mFieldMap[key];
    // If this was linked as a stage input (should only be stage or none) then
    // copy from the composite input data
    if (linkInfo.mLinkedType == LinkedFieldType::Stage)
    {
      builder << builder.EmitIndent() << "fragmentInput[i]." << linkInfo.mFieldMeta->mRaverieName;
      builder << " = input[i]." << linkInfo.mOutputFieldDependencyName << ";" << builder.EmitLineReturn();
    }
  }
  builder.EndScope();

  // Declare the  output stream (effectively a dummy class)
  builder.WriteLocalVariableDefaultConstruction("fragmentOutput", raverieOutputStreamType->Name);

  // Declare the fragment type and copy its inputs
  String fragmentVarName = MakeFragmentVarName(geometryFragmentType->mMeta);
  CreateFragmentAndCopyInputs(currentStage, builder, stageResults.mClassName, geometryFragmentType, fragmentVarName);
  // Invoke the fragment's main function
  builder << builder.EmitIndent() << fragmentVarName << "." << nameSettings.mMainFunctionName
          << "(fragmentInput, fragmentOutput);" << builder.EmitLineReturn();

  builder.EndScope();

  builder.EndScope();

  stageResults.mShaderCode = builder.ToString();
}

void RaverieShaderIRCompositor::GenerateComputeRaverieComposite(StageLinkingInfo* currentStage,
                                                            ShaderStageDescription& stageResults,
                                                            ShaderIRAttributeList& extraAttributes)
{
  RaverieShaderIRType* computeFragmentType = currentStage->mFragmentTypes[0];
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  ShaderCodeBuilder builder;
  StageAttachmentLinkingInfo& linkingInfo = currentStage->mVertexLinkingInfo;

  Raverie::ComputeFragmentUserData* computeUserData =
      computeFragmentType->mRaverieType->Has<Raverie::ComputeFragmentUserData>();
  int localSizeX = computeUserData->mLocalSizeX;
  int localSizeY = computeUserData->mLocalSizeY;
  int localSizeZ = computeUserData->mLocalSizeZ;
  if (mComputeShaderProperties != nullptr)
  {
    localSizeX = mComputeShaderProperties->mLocalSizeX;
    localSizeY = mComputeShaderProperties->mLocalSizeY;
    localSizeZ = mComputeShaderProperties->mLocalSizeZ;
  }

  // Make the name for the fragment. For uniqueness, append the stage's name to
  // the given composite name.
  String stageName = FragmentType::Names[currentStage->mFragmentType];
  stageResults.mClassName = BuildString(mShaderCompositeName, "_", stageName);
  // Emit the struct declaration (attributes followed by name)
  builder << "[" << stageName;
  // Emit the local size attributes (controls local workgroup size)
  builder << "(" << nameSettings.mComputeLocalSizeXParam << " : " << ToString(localSizeX) << ",";
  builder << nameSettings.mComputeLocalSizeYParam << " : " << ToString(localSizeY) << ",";
  builder << nameSettings.mComputeLocalSizeZParam << " : " << ToString(localSizeZ) << ")";
  builder << "]";
  for (size_t i = 0; i < extraAttributes.Size(); ++i)
    builder.DeclareAttribute(*extraAttributes.GetAtIndex(i));
  builder << builder.EmitLineReturn();
  builder << "struct " << stageResults.mClassName << builder.EmitLineReturn();
  builder.BeginScope();

  // Declare the fields based upon the inputs order (shouldn't matter for
  // compute shaders)
  DeclareFieldsInOrder(builder, linkingInfo, linkingInfo.mInputs);

  // Emit the main function with the entry point attribute
  builder.WriteLine();
  builder << builder.EmitIndent();
  builder << builder.DeclareAttribute(nameSettings.mEntryPointAttributeName);
  builder << builder.EmitLineReturn();

  builder << builder.EmitIndent() << "function Main()" << builder.EmitLineReturn();
  builder.BeginScope();

  // Emit each fragment variable, copy its inputs, then call its main
  for (size_t i = 0; i < currentStage->mFragmentTypes.Size(); ++i)
  {
    RaverieShaderIRType* fragmentType = currentStage->mFragmentTypes[i];
    FragmentLinkingInfo& linkInfo = currentStage->mFragmentLinkInfoMap[fragmentType];

    // Declare the fragment and copy its inputs
    String fragmentVarName = MakeFragmentVarName(fragmentType->mMeta);
    CreateFragmentAndCopyInputs(currentStage, builder, stageResults.mClassName, fragmentType, fragmentVarName);
    // Call the fragment's main function
    builder << builder.EmitIndent() << fragmentVarName << ".Main();" << builder.EmitLineReturn();
    builder << builder.EmitLineReturn();
  }

  // Copy outputs from the last fragment to output a stage output back into the
  // composite
  AutoDeclare(outputRange, linkingInfo.mResolvedOutputs.Values());
  for (; !outputRange.Empty(); outputRange.PopFront())
  {
    ResolvedFieldOutputInfo& fieldOutput = outputRange.Front();
    ShaderIRFieldMeta* fieldMeta = fieldOutput.mOutputFieldDependency;
    // If a field meta is null then this is a pass-through variable and no copy
    // out is required. If the owner is null then this didn't come from a
    // fragment, it came from the composite (uniform/built-in). This requires no
    // extra copy as the composite's input wasn't changed.
    if (fieldMeta == nullptr || fieldMeta->mOwner == nullptr)
      continue;

    builder << builder.EmitIndent() << "this." << fieldOutput.mFieldName << " = ";
    builder << MakeFragmentVarName(fieldMeta->mOwner) << "." << fieldMeta->mRaverieName << ";"
            << builder.EmitLineReturn();
  }

  builder.EndScope();

  builder.EndScope();

  stageResults.mShaderCode = builder.ToString();
}

void RaverieShaderIRCompositor::CreateFragmentAndCopyInputs(StageLinkingInfo* currentStage,
                                                          ShaderCodeBuilder& builder,
                                                          StringParam currentClassName,
                                                          RaverieShaderIRType* fragmentType,
                                                          StringParam fragmentVarName)
{
  FragmentLinkingInfo& linkInfo = currentStage->mFragmentLinkInfoMap[fragmentType];

  // Create the fragment variable
  String fragmentVarType = fragmentType->mRaverieType->ToString();
  builder.WriteLocalVariableDefaultConstruction(fragmentVarName, fragmentVarType);

  // Copy inputs for this fragment from their respective outputs
  for (size_t i = 0; i < linkInfo.mFieldList.Size(); ++i)
  {
    FieldLinkingInfo& fieldLinkInfo = linkInfo.mFieldMap[linkInfo.mFieldList[i]];
    ShaderIRFieldMeta* fieldMeta = fieldLinkInfo.mFieldMeta;
    // If this is a property input, then we have to copy from the mangled
    // property input name
    if (fieldLinkInfo.mLinkedType == LinkedFieldType::Property)
    {
      builder << builder.EmitIndent() << fragmentVarName << "." << fieldMeta->mRaverieName << " = ";
      builder << "this." << fieldLinkInfo.mFieldPropertyName;
      builder << ";" << builder.EmitLineReturn();
    }
    // If this is a fragment input, then copy from the resolved dependency
    // fragment
    else if (fieldLinkInfo.mLinkedType == LinkedFieldType::Fragment)
    {
      ShaderIRFieldMeta* outputFieldDependency = fieldLinkInfo.mOutputFieldDependency;
      String fragmentDependencyVarName = MakeFragmentVarName(outputFieldDependency->mOwner);
      builder << builder.EmitIndent() << fragmentVarName << "." << fieldMeta->mRaverieName << " = ";
      builder << fragmentDependencyVarName << "." << outputFieldDependency->mRaverieName;
      builder << ";" << builder.EmitLineReturn();
    }
    // If this is a stage input, copy from the composite given the dependency
    // variable's name
    else if (fieldLinkInfo.mLinkedType == LinkedFieldType::Stage)
    {
      builder << builder.EmitIndent() << fragmentVarName << "." << fieldMeta->mRaverieName << " = ";
      builder << "this." << fieldLinkInfo.mOutputFieldDependencyName;
      builder << ";" << builder.EmitLineReturn();
    }
    // If this is a application built-in, copy from the composite given the
    // dependency variable's name
    else if (fieldLinkInfo.mLinkedType == LinkedFieldType::AppBuiltIn)
    {
      ShaderIRFieldMeta* outputFieldDependency = fieldLinkInfo.mOutputFieldDependency;
      builder << builder.EmitIndent() << fragmentVarName << "." << fieldMeta->mRaverieName << " = ";
      builder << "this." << outputFieldDependency->mRaverieName;
      builder << ";" << builder.EmitLineReturn();
    }
    // If this is a hardware built-in input (fixed function variables),
    // copy from the composite given the dependency variable's name
    else if (fieldLinkInfo.mLinkedType == LinkedFieldType::HardwareBuiltIn)
    {
      ShaderIRFieldMeta* outputFieldDependency = fieldLinkInfo.mOutputFieldDependency;
      builder << builder.EmitIndent() << fragmentVarName << "." << fieldMeta->mRaverieName << " = ";
      builder << "this." << outputFieldDependency->mRaverieName;
      builder << ";" << builder.EmitLineReturn();
    }
    // If this is a specialization constant, then copy from the static input
    else if (fieldLinkInfo.mLinkedType == LinkedFieldType::SpecConstant)
    {
      builder << builder.EmitIndent() << fragmentVarName << "." << fieldMeta->mRaverieName << " = ";
      builder << currentClassName << "." << fieldLinkInfo.mFieldPropertyName;
      builder << ";" << builder.EmitLineReturn();
    }
  }
}

void RaverieShaderIRCompositor::DeclareFieldsInOrder(ShaderCodeBuilder& builder,
                                                   StageAttachmentLinkingInfo& linkingInfo,
                                                   OrderedHashSet<ShaderFieldKey>& orderMap)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  // Declare everything from the given order map
  AutoDeclare(fieldKeyRange, orderMap.All());
  for (; !fieldKeyRange.Empty(); fieldKeyRange.PopFront())
  {
    ShaderFieldKey& fieldKey = fieldKeyRange.Front();
    ResolvedFieldInfo& fieldInfo = linkingInfo.mResolvedFields[fieldKey];
    builder.WriteVariableDeclaration(fieldInfo.mAttributes, fieldInfo.mFieldName, fieldInfo.mFieldType);
  }

  // Then declare everything else in the resolved fields. If we've already
  // declared a field skip it.
  AutoDeclare(fieldRange, linkingInfo.mResolvedFields.All());
  for (; !fieldRange.Empty(); fieldRange.PopFront())
  {
    AutoDeclare(pair, fieldRange.Front());
    if (orderMap.ContainsKey(pair.first))
      continue;

    ResolvedFieldInfo& fieldInfo = pair.second;
    builder.WriteVariableDeclaration(fieldInfo.mAttributes, fieldInfo.mFieldName, fieldInfo.mFieldType);
  }
}

void RaverieShaderIRCompositor::DeclareFieldsWithAttribute(ShaderCodeBuilder& builder,
                                                         StageAttachmentLinkingInfo& linkingInfo,
                                                         OrderedHashSet<ShaderFieldKey>& fieldSet,
                                                         StringParam attributeName)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  AutoDeclare(fieldKeyRange, fieldSet.All());
  for (; !fieldKeyRange.Empty(); fieldKeyRange.PopFront())
  {
    ShaderFieldKey& fieldKey = fieldKeyRange.Front();
    ResolvedFieldInfo& fieldInfo = linkingInfo.mResolvedFields[fieldKey];
    ShaderIRAttribute* attribute = fieldInfo.mAttributes.FindFirstAttribute(attributeName);
    if (attribute != nullptr)
      builder.WriteVariableDeclaration(*attribute, fieldInfo.mFieldName, fieldInfo.mFieldType);
  }
}

String RaverieShaderIRCompositor::MakeFragmentVarName(ShaderIRTypeMeta* typeMeta)
{
  // Lowercase the first letter of the type name
  String name = typeMeta->mRaverieName;
  StringRange first = name.SubString(name.Begin(), name.Begin() + 1);
  StringRange second = name.SubString(first.End(), name.End());
  return BuildString(first.ToLower(), second);
}

void RaverieShaderIRCompositor::GenerateStageDescriptions(CompositedShaderInfo& compositeInfo,
                                                        ShaderDefinition& shaderDef)
{
  // Walk all active stages (skipping cpu & gpu)
  for (size_t i = 1; i < compositeInfo.mActiveStages.Size() - 1; ++i)
  {
    StageLinkingInfo* currentStage = compositeInfo.mActiveStages[i];

    // Create the fragment's description map
    ShaderStageDescription& stageDescription = shaderDef.mResults[currentStage->mFragmentType];
    stageDescription.mFragmentType = currentStage->mFragmentType;
    FragmentDescriptionMap* fragDescMap = new FragmentDescriptionMap();
    stageDescription.mFragmentDescriptions = fragDescMap;

    for (size_t fragIndex = 0; fragIndex < currentStage->mFragmentTypes.Size(); ++fragIndex)
    {
      // Get the resultant linking info for the fragment
      RaverieShaderIRType* fragType = currentStage->mFragmentTypes[fragIndex];
      FragmentLinkingInfo* fragInfo = currentStage->mFragmentLinkInfoMap.FindPointer(fragType);
      if (fragInfo == nullptr)
        continue;

      // Create the output description for this fragment. Copy this data from
      // the internal linkage info
      ShaderIRTypeMeta* fragMetaType = fragType->mMeta;
      ShaderFragmentDescription* fragDesc = new ShaderFragmentDescription();
      fragDescMap->InsertOrError(fragMetaType, fragDesc);
      fragInfo->CopyTo(*fragDesc);
    }
  }
}

RaverieShaderIRCompositor::FieldLinkingInfo::FieldLinkingInfo()
{
  mLinkedType = LinkedFieldType::None;
  mFieldMeta = nullptr;
  mOutputFieldDependency = nullptr;
}

void RaverieShaderIRCompositor::FragmentLinkingInfo::CopyTo(ShaderFragmentDescription& fragDesc)
{
  fragDesc.mMeta = mMeta;
  AutoDeclare(fieldRange, mFieldMap.Values());
  for (; !fieldRange.Empty(); fieldRange.PopFront())
  {
    FieldLinkingInfo& fieldInfo = fieldRange.Front();
    CopyFieldTo(fieldInfo, fragDesc);
  }

  // Also merge in the non-copyable properties.
  for (size_t i = 0; i < mNonCopyableProperties.Size(); ++i)
  {
    FieldLinkingInfo& fieldInfo = mNonCopyableProperties[i];
    CopyFieldTo(fieldInfo, fragDesc);
  }
}

void RaverieShaderIRCompositor::FragmentLinkingInfo::CopyFieldTo(FieldLinkingInfo& fieldInfo,
                                                               ShaderFragmentDescription& fragDesc)
{
  ShaderFieldDescription& fieldDesc = fragDesc.mFieldDescriptions[fieldInfo.mFieldMeta];

  fieldDesc.mMeta = fieldInfo.mFieldMeta;
  fieldDesc.mLinkedType = fieldInfo.mLinkedType;
  fieldDesc.mOutputFieldDependency = fieldInfo.mOutputFieldDependency;
  fieldDesc.mFieldPropertyName = fieldInfo.mFieldPropertyName;
}

RaverieShaderIRCompositor::ResolvedFieldOutputInfo::ResolvedFieldOutputInfo()
{
  mOutputFieldDependency = nullptr;
}

RaverieShaderIRCompositor::ResolvedFieldOutputInfo::ResolvedFieldOutputInfo(StringParam fieldName,
                                                                          ShaderIRFieldMeta* outputDependency)
{
  mFieldName = fieldName;
  mOutputFieldDependency = outputDependency;
}

RaverieShaderIRCompositor::StageAttachmentLinkingInfo::StageAttachmentLinkingInfo()
{
  mPreviousStage = nullptr;
  mNextStage = nullptr;
  mOwningStage = nullptr;
}

void RaverieShaderIRCompositor::StageAttachmentLinkingInfo::AddResolvedField(ShaderIRFieldMeta* fieldMeta,
                                                                           StringParam attributeName)
{
  AddResolvedField(fieldMeta->mRaverieName, fieldMeta->mRaverieType->ToString(), attributeName);
}

void RaverieShaderIRCompositor::StageAttachmentLinkingInfo::AddResolvedFieldProperty(ShaderIRFieldMeta* fieldMeta,
                                                                                   ShaderIRAttribute* attribute)
{
  String baseFieldName = RaverieShaderIRCompositor::GetFieldInOutName(fieldMeta, attribute);
  String fieldName = RaverieShaderIRCompositor::MakePropertyName(baseFieldName, fieldMeta->mOwner->mRaverieName);
  AddResolvedField(fieldName, fieldMeta->mRaverieType->ToString(), attribute->mAttributeName);
}

void RaverieShaderIRCompositor::StageAttachmentLinkingInfo::AddResolvedStageField(SpirVNameSettings& nameSettings,
                                                                                ShaderIRFieldMeta* fieldMeta,
                                                                                StringParam fieldName,
                                                                                StringParam attributeName,
                                                                                StringParam attributeParameter)
{
  // Add the field with the given attribute
  ShaderIRAttribute* fieldAttribute = AddResolvedField(fieldName, fieldMeta->mRaverieType->ToString(), attributeName);

  // Check to see if we need to write out a parameter
  if (attributeParameter.Empty())
    return;

  String nameOverrideParam = SpirVNameSettings::mNameOverrideParam;

  // Find or create the 'name' parameter
  ShaderIRAttributeParameter* nameParam = fieldAttribute->FindFirstParameter(nameOverrideParam);
  if (nameParam == nullptr)
    nameParam = &fieldAttribute->mParameters.PushBack();
  // Set the name and value of the parameter
  nameParam->SetName(nameOverrideParam);
  nameParam->SetStringValue(attributeParameter);

  // Record this as either an input or output. This records what order
  // inputs/outputs must be declared in.
  ShaderFieldKey fieldKey(fieldName, fieldMeta->mRaverieType->ToString());
  if (attributeName == nameSettings.mStageInputAttribute)
    mInputs.InsertOrIgnore(fieldKey);
  else
    mOutputs.InsertOrIgnore(fieldKey);
}

ShaderIRAttribute* RaverieShaderIRCompositor::StageAttachmentLinkingInfo::AddResolvedField(StringParam fieldName,
                                                                                         StringParam fieldType,
                                                                                         StringParam attributeName)
{
  ResolvedFieldInfo* fieldInfo = CreateResolvedField(fieldName, fieldType);

  // Add the given attribute name if this field doesn't already have it
  ShaderIRAttribute* attribute = fieldInfo->mAttributes.FindFirstAttribute(attributeName);
  if (attribute == nullptr)
    attribute = fieldInfo->mAttributes.AddAttribute(attributeName, nullptr);
  return attribute;
}

RaverieShaderIRCompositor::ResolvedFieldInfo*
RaverieShaderIRCompositor::StageAttachmentLinkingInfo::CreateResolvedField(StringParam fieldName, StringParam fieldType)
{
  ShaderFieldKey fieldKey(fieldName, fieldType);
  ResolvedFieldInfo* fieldInfo = &mResolvedFields[fieldKey];
  fieldInfo->mFieldName = fieldName;
  fieldInfo->mFieldType = fieldType;
  return fieldInfo;
}

RaverieShaderIRCompositor::StageLinkingInfo::StageLinkingInfo()
{
}

void RaverieShaderIRCompositor::StageLinkingInfo::SetFragmentType(FragmentType::Enum fragType)
{
  mFragmentType = fragType;
  mShaderStage = FragmentTypeToShaderStage(mFragmentType);
}

void RaverieShaderIRCompositor::StageLinkingInfo::Clear()
{
  mInputVertexTypes.Clear();
  mOutputVertexTypes.Clear();
  mPrimitiveTypes.Clear();
  mFragmentTypes.Clear();
  mFragmentType = (FragmentType::Enum)FragmentType::None;
  mShaderStage = (ShaderStage::Enum)ShaderStage::None;
}

void RaverieShaderIRCompositor::ApiPerspectivePositionCallback(CompositorCallbackData& callbackData, void* userData)
{
  RaverieShaderSpirVSettings* settings = callbackData.mSettings;
  StageLinkingInfo* stageLinkingInfo = callbackData.mStageLinkingInfo;
  // Only process the geometry stage
  if (stageLinkingInfo->mFragmentType != FragmentType::Geometry)
    return;

  SpirVNameSettings& nameSettings = settings->mNameSettings;
  String real4TypeName = RaverieTypeId(Raverie::Real4)->Name;
  String real4x4TypeName = RaverieTypeId(Raverie::Real4x4)->Name;
  StageAttachmentLinkingInfo& vertexLinkingInfo = stageLinkingInfo->mVertexLinkingInfo;
  // Force ApiPerspectivePosition to be a hardware input and output
  vertexLinkingInfo.AddResolvedField(
      nameSettings.mApiPerspectivePositionName, real4TypeName, nameSettings.mHardwareBuiltInInputAttribute);
  vertexLinkingInfo.mHardwareInputs.InsertOrIgnore(
      ShaderFieldKey(nameSettings.mApiPerspectivePositionName, real4TypeName));
  vertexLinkingInfo.AddResolvedField(
      nameSettings.mApiPerspectivePositionName, real4TypeName, nameSettings.mHardwareBuiltInOutputAttribute);
  vertexLinkingInfo.mHardwareOutputs.InsertOrIgnore(
      ShaderFieldKey(nameSettings.mApiPerspectivePositionName, real4TypeName));

  // Force the api perspective matrix transform to exist
  stageLinkingInfo->mPrimitiveLinkingInfo.AddResolvedField(
      nameSettings.mPerspectiveToApiPerspectiveName, real4x4TypeName, nameSettings.mAppBuiltInInputAttribute);
}

} // namespace Raverie
