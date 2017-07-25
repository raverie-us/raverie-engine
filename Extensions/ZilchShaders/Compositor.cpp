///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ShaderVariableLinkData
String ShaderVariableLinkData::GetAttributeString() const
{
  StringBuilder builder;
  for(HashSet<String>::range attributeRange = mAttributes.All(); !attributeRange.Empty(); attributeRange.PopFront())
    builder.Append(attributeRange.Front());

  String attributes = builder.ToString();
  return attributes;
}

//-------------------------------------------------------------------OrderedVariableMap
OrderedVariableMap::~OrderedVariableMap()
{
  Clear();
}

void OrderedVariableMap::Clear()
{
  for(size_t i = 0; i < mList.Size(); ++i)
    delete mList[i].second;
  mList.Clear();
  mMap.Clear();
}

void OrderedVariableMap::InsertOrOverride(PairType& pair)
{
  InsertOrOverride(pair.first, pair.second);
}

void OrderedVariableMap::InsertOrOverride(const ShaderFieldKey& fieldKey, ShaderVariableLinkData* value)
{
  ShaderVariableLinkData* varData = FindOrCreate(fieldKey);
  *varData = *value;
}

bool OrderedVariableMap::ContainsKey(const ShaderFieldKey& fieldKey)
{
  return mMap.ContainsKey(fieldKey);
}

ShaderVariableLinkData* OrderedVariableMap::FindOrCreate(const ShaderFieldKey& fieldKey)
{
  ShaderVariableLinkData* varData = mMap.FindValue(fieldKey, nullptr);
  if(varData == nullptr)
  {
    varData = new ShaderVariableLinkData();
    mMap.Insert(fieldKey, varData);
    mList.PushBack(PairType(fieldKey, varData));
  }
  return varData;
}

ShaderVariableLinkData* OrderedVariableMap::FindOrCreate(ShaderField* shaderField)
{
  ShaderFieldKey fieldKey(shaderField);
  ShaderVariableLinkData* varData = FindOrCreate(fieldKey);
  varData->mName = shaderField->mZilchName;
  varData->mType = shaderField->mZilchType;
  return varData;
}

ShaderVariableLinkData* OrderedVariableMap::FindValue(const ShaderFieldKey& fieldKey, ShaderVariableLinkData* defaultValue)
{
  return mMap.FindValue(fieldKey, defaultValue);
}

void OrderedVariableMap::Merge(OrderedVariableMap& otherMap)
{
  AutoDeclare(range, otherMap.All());
  for(; !range.Empty(); range.PopFront())
  {
    ShaderVariableLinkData* varData = FindOrCreate(range.Front().first);
    varData->mName = range.Front().second->mName;
    varData->mType = range.Front().second->mType;

    AutoDeclare(attributeRange, range.Front().second->mAttributes.All());
    for(; !attributeRange.Empty(); attributeRange.PopFront())
      varData->mAttributes.Insert(attributeRange.Front());
  }
}

OrderedVariableMap::range OrderedVariableMap::All()
{
  return mList.All();
}

//-------------------------------------------------------------------ShaderLinkDefinition
void ShaderLinkDefinition::AddOutputDef(StringParam name, StringParam type)
{
  ShaderFieldKey varKey(name, type);
  ShaderVariableLinkData* varData = Outputs.FindOrCreate(varKey);
  varData->mName = name;
  varData->mType = type;
}

//-------------------------------------------------------------------ShaderFieldCompositeInfo
ShaderFieldCompositeInfo::ShaderFieldCompositeInfo()
{
  mInputType = FieldInputType::Unknown;
  mInputError = false;
}

//-------------------------------------------------------------------ShaderFragmentCompositeInfo
ShaderFieldCompositeInfo& ShaderFragmentCompositeInfo::GetField(ShaderFieldKey fieldKey)
{
  mFieldMap.InsertOrIgnore(fieldKey, ShaderFieldCompositeInfo());
  return *mFieldMap.FindPointer(fieldKey);
}

ShaderFieldCompositeInfo& ShaderFragmentCompositeInfo::GetField(ShaderField* field)
{
  ShaderFieldKey fieldKey(field);
  return GetField(fieldKey);
}

//-------------------------------------------------------------------ShaderStageInfo
ShaderFragmentsInfoMap::~ShaderFragmentsInfoMap()
{
  Clear();
}

void ShaderFragmentsInfoMap::Generate(Array<ShaderType*>& fragmentsOfType)
{
  for(size_t fragmentIndex = 0; fragmentIndex < fragmentsOfType.Size(); ++fragmentIndex)
  {
    ShaderType* fragment = fragmentsOfType[fragmentIndex];
    ShaderFragmentCompositeInfo* fragInfo = new ShaderFragmentCompositeInfo();
    fragInfo->mFragmentName = fragment->mZilchName;
    mFragments.Insert(fragment->mZilchName, fragInfo);
  }
}

void ShaderFragmentsInfoMap::Clear()
{
  HashMap<String, ShaderFragmentCompositeInfo*>::range fragments = mFragments.All();
  for(; !fragments.Empty(); fragments.PopFront())
  {
    delete fragments.Front().second;
  }
  mFragments.Clear();
}

//-------------------------------------------------------------------ShaderStageInfo
void ShaderStageInfo::Generate(Array<ShaderType*>& fragmentsOfType)
{
  mFragmentMap = new ShaderFragmentsInfoMap();
  mFragmentMap->Generate(fragmentsOfType);
}

ShaderFragmentCompositeInfo* ShaderStageInfo::GetFragment(StringParam zilchName)
{
  return mFragmentMap->mFragments.FindValue(zilchName, nullptr);
}

ShaderStageInfo::FragmentInfoRange ShaderStageInfo::All()
{
  return mFragmentMap->mFragments.All();
}

//-------------------------------------------------------------------ShaderSharedData
CompositeData::CompositeData(StringParam compositeName, StringParam fragmentAttributeName)
{
  mCompositeName = compositeName;
  mFragmentAttributeName = fragmentAttributeName;
  
  mShaderTypeName = BuildString(compositeName, "_", mFragmentAttributeName);
}

//-------------------------------------------------------------------ZilchCompositor
ZilchCompositor::ZilchCompositor()
{
  mEmitVertexCallback = nullptr;
}

void ZilchCompositor::BuildCompositedShader(BaseShaderTranslator* translator, ZilchShaderLibraryRef inputLibraryRef, ZilchShaderDefinition& shaderDef, ZilchShaderSettingsRef& settings)
{
  // Validate that there's no invalid fragment type being composited
  for(size_t i = 0; i < shaderDef.mFragmentTypes.Size(); ++i)
  {
    ShaderType* shaderType = shaderDef.mFragmentTypes[i];
    bool isSupported = translator->SupportsFragmentType(shaderType);
    ErrorIf(!isSupported, "Shader Fragment '%s' of fragment type '%s' is not supported in translator %s.",
      shaderType->mZilchName.c_str(), FragmentType::Names[shaderType->mFragmentType], translator->GetFullLanguageString().c_str());
  }

  mSettings = settings;
  mInputLibraryRef = inputLibraryRef;
  mTranslator = translator;
  mCurrentLanguageName = translator->GetLanguageName();
  mCurrentLanguageVersionNumber = translator->GetLanguageVersionNumber();

  GenerateZilchShader(shaderDef);
}

void ZilchCompositor::CollectFragmentsOfType(ZilchShaderDefinition& shaderDef, uint fragmentType, Array<ShaderType*>& fragmentsOfType)
{
  ZilchShaderLibrary* inputLibrary = mInputLibraryRef;

  for(size_t fragmentIndex = 0; fragmentIndex < shaderDef.mFragmentTypes.Size(); ++fragmentIndex)
  {
    ShaderType* fragmentShaderType = shaderDef.mFragmentTypes[fragmentIndex];
    if(fragmentShaderType == nullptr)
    {
      Error("Invalid fragment. Fragment in shader def cannot be null");
      continue;
    }
    String zilchFragmentName = fragmentShaderType->mZilchName;
    ShaderType* type = inputLibrary->FindType(zilchFragmentName);

    // Add fragments of the correct type
    if(type->mFragmentType == fragmentType)
      fragmentsOfType.PushBack(type);
  }
}

void ZilchCompositor::GenerateZilchShader(ZilchShaderDefinition& shaderDef)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  String real4Type = core.Real4Type->ToString();
  NameSettings& nameSettings = mSettings->mNameSettings;
  ShaderSystemValueSettings& systemValueSettings= mSettings->mSystemValueSettings;

  CompositeData vertexFragmentData(shaderDef.mShaderName, nameSettings.mVertexAttributeName);
  CompositeData geometryFragmentData(shaderDef.mShaderName, nameSettings.mGeometryAttributeName);
  CompositeData pixelFragmentData(shaderDef.mShaderName, nameSettings.mPixelAttributeName);

  // Collect all fragment types into their own arrays (so we don't have to skip fragments during later steps)
  CollectFragmentsOfType(shaderDef, FragmentType::Vertex, vertexFragmentData.mFragments);
  CollectFragmentsOfType(shaderDef, FragmentType::Geometry, geometryFragmentData.mFragments);
  CollectFragmentsOfType(shaderDef, FragmentType::Pixel, pixelFragmentData.mFragments);
  bool generateGeometryShaders = !geometryFragmentData.mFragments.Empty();

  // Create the main stage information helper structs (creates fragment information for each stage)
  ShaderStageInfo vertexStageInfo, geometryPrimitiveStageInfo, geometryStageInfo, pixelStageInfo;
  vertexStageInfo.Generate(vertexFragmentData.mFragments);
  geometryStageInfo.Generate(geometryFragmentData.mFragments);
  geometryPrimitiveStageInfo.Generate(geometryFragmentData.mFragments);
  pixelStageInfo.Generate(pixelFragmentData.mFragments);
  vertexStageInfo.mFragmentStage = ShaderStageType::Vertex;
  geometryStageInfo.mFragmentStage = ShaderStageType::GeometryVertex;
  geometryPrimitiveStageInfo.mFragmentStage = ShaderStageType::GeometryPrimitive;
  pixelStageInfo.mFragmentStage = ShaderStageType::Pixel;

  // Determine what could be output from each stage. An output might not actually be a
  // stage output, it depends on the next stage inputting the same variable.
  CollectExpectedStageOutputs(vertexFragmentData.mFragments, vertexStageInfo);
  CollectExpectedStageOutputs(geometryFragmentData.mFragments, geometryStageInfo);
  CollectExpectedStageOutputs(pixelFragmentData.mFragments, pixelStageInfo);
  // To facilitate pass-through in the geometry shader, all potential vertex shader outputs are also potential geometry shader outputs
  AutoDeclare(vertexExpectedOutputs, vertexStageInfo.mExpectedOutputs.All());
  for(; !vertexExpectedOutputs.Empty(); vertexExpectedOutputs.PopFront())
    geometryStageInfo.mExpectedOutputs.InsertOrOverride(vertexExpectedOutputs.Front());

  // Generate a cpu (vertex definitions) and gpu (render targets) stage.
  // This allows re-using code for linking stages together.
  ShaderStageInfo cpuStage, gpuStage, dummyStage;
  GenerateCpuStage(cpuStage);
  GenerateGpuStage(pixelStageInfo, gpuStage);

  AddForcedSystemValues(vertexStageInfo);
  AddForcedSystemValues(geometryStageInfo);
  AddForcedSystemValues(geometryPrimitiveStageInfo);
  AddForcedSystemValues(pixelStageInfo);

  // Determine what each stage inputs given the previous stage's expected outputs.
  // This evaluates input attributes in order to determine whether or not an input truly is a stage input.
  DetermineFragmentInputTypes(cpuStage, vertexStageInfo, vertexFragmentData.mFragments);
  if(!generateGeometryShaders)
  {
    // Determine pixel stage inputs based upon the vertex stage
    DetermineFragmentInputTypes(vertexStageInfo, pixelStageInfo, pixelFragmentData.mFragments);
  }
  else
  {
    // Determine geometry stage inputs based upon the vertex stage
    DetermineFragmentInputTypes(vertexStageInfo, geometryStageInfo, geometryFragmentData.mFragments);
    // Determine pixel stage inputs based upon the geometry stage
    DetermineFragmentInputTypes(geometryStageInfo, pixelStageInfo, pixelFragmentData.mFragments);
    // Since the geometry shader generates pass-through data to the vertex shader, find out what pixel
    // inputs need to also become geometry inputs because they were output by the vertex shader.
    DetermineGeometryStageInputs(vertexStageInfo, geometryStageInfo, pixelStageInfo);

    // Finally, collect the remaining fragment information about geometry fragments, such as properties,
    // built-ins etc... This is obtained by linking against a dummy shader stage so StageInput's can't happen.
    DetermineFragmentInputTypes(dummyStage, geometryPrimitiveStageInfo, geometryFragmentData.mFragments);
  }

  // Now that all inputs have been determined, tell each stage what it actually outputs.
  // This can only be determined based upon the next stage's inputs.
  LinkStage(cpuStage, vertexStageInfo);
  if(!generateGeometryShaders)
  {
    LinkStage(vertexStageInfo, pixelStageInfo);
  }
  else
  {
    LinkStage(vertexStageInfo, geometryStageInfo, nameSettings.mGeometryShaderOutputTag);
    LinkStage(geometryStageInfo, pixelStageInfo, nameSettings.mGeometryShaderOutputTag);
  }
  LinkStage(pixelStageInfo, gpuStage);


  // Generate the vertex shader's zilch code
  ZilchFragmentInfo& vertexInfo = shaderDef.mShaderData[FragmentType::Vertex];
  vertexInfo.mZilchClassName = vertexFragmentData.mShaderTypeName;
  vertexInfo.mFileName = vertexInfo.mZilchClassName;
  vertexInfo.mZilchCode = GenerateZilchComposite(vertexFragmentData, vertexStageInfo);
  vertexInfo.mFragmentMap = vertexStageInfo.mFragmentMap;

  // Generate the geometry shader's zilch code
  if(generateGeometryShaders)
  {
    ZilchFragmentInfo& geometryInfo = shaderDef.mShaderData[FragmentType::Geometry];
    geometryInfo.mZilchClassName = geometryFragmentData.mShaderTypeName;
    geometryInfo.mFileName = geometryInfo.mZilchClassName;
    geometryInfo.mZilchCode = GenerateZilchComposite(geometryFragmentData, geometryStageInfo, geometryPrimitiveStageInfo);
    geometryInfo.mFragmentMap = geometryPrimitiveStageInfo.mFragmentMap;
  }

  // Generate the pixel shader's zilch code
  ZilchFragmentInfo& pixelInfo = shaderDef.mShaderData[FragmentType::Pixel];
  pixelInfo.mZilchClassName = pixelFragmentData.mShaderTypeName;
  pixelInfo.mFileName = pixelInfo.mZilchClassName;
  pixelInfo.mZilchCode = GenerateZilchComposite(pixelFragmentData, pixelStageInfo);
  pixelInfo.mFragmentMap = pixelStageInfo.mFragmentMap;
}

void ZilchCompositor::AddForcedSystemValues(ShaderStageInfo& stageInfo)
{
  ShaderSystemValueSettings& systemValueSettings = mSettings->mSystemValueSettings;
  NameSettings& nameSettings = mSettings->mNameSettings;

  // Walk over all system values for the current stage
  AutoDeclare(systemValueRange, systemValueSettings.mShaderStages[stageInfo.mFragmentStage].mSystemValues.All());
  for(; !systemValueRange.Empty(); systemValueRange.PopFront())
  {
    ShaderSystemValue& systemValue = systemValueRange.Front().second;
    LanguageSystemValue* languageSystemValue = systemValue.mLanguages.FindPointer(mCurrentLanguageName);
    // If this system value contains a language implementation that is forced, add it to
    // the appropriate map based upon if it is an input, output, or both
    if(languageSystemValue != nullptr && languageSystemValue->GetForced())
    {
      if(languageSystemValue->IsInput())
      {
        ShaderFieldKey fieldKey(systemValue.mName, systemValue.mType);
        ShaderVariableLinkData* varData = stageInfo.mInputs.FindOrCreate(fieldKey);
        varData->mName = systemValue.mName;
        varData->mType = systemValue.mType;
        varData->mAttributes.Insert(nameSettings.mSystemValueInputTag);
      }
      if(languageSystemValue->IsOutput())
      {
        ShaderFieldKey fieldKey(systemValue.mName, systemValue.mType);
        ShaderVariableLinkData* varData = stageInfo.mComputedOutputs.FindOrCreate(fieldKey);
        varData->mName = systemValue.mName;
        varData->mType = systemValue.mType;
        varData->mAttributes.Insert(nameSettings.mSystemValueOutputTag);
      }
    }
  }
}

void ZilchCompositor::GenerateCpuStage(ShaderStageInfo& cpuStage)
{
  ShaderDefinitionSettings& shaderSettings = mSettings->mShaderDefinitionSettings;

  // Add every vertex definition as an expected output
  AutoDeclare(vertexDefinitionRange, shaderSettings.mVertexDefinitions.Values());
  for(; !vertexDefinitionRange.Empty(); vertexDefinitionRange.PopFront())
  {
    ShaderField* field = vertexDefinitionRange.Front();

    ShaderFieldKey varKey(field);
    ShaderVariableLinkData* varData = cpuStage.mExpectedOutputs.FindOrCreate(varKey);
    varData->mType = field->mZilchType;
    varData->mName = field->mZilchName;
  }
}

void ZilchCompositor::GenerateGpuStage(ShaderStageInfo& previousStage, ShaderStageInfo& gpuStage)
{
  ShaderDefinitionSettings& shaderSettings = mSettings->mShaderDefinitionSettings;
  NameSettings& nameSettings = mSettings->mNameSettings;
  ShaderSystemValueSettings& systemValueSettings = mSettings->mSystemValueSettings;

  Zilch::Core& core = Zilch::Core::GetInstance();
  // Hardcode the render target type as a Real4 for now
  String renderTargetType = core.Real4Type->ToString();

  // Add every render target that makes sense (see below) as an input
  for(size_t i = 0; i < shaderSettings.mRenderTargetNames.Size(); ++i)
  {
    String varName = shaderSettings.mRenderTargetNames[i];
    ShaderFieldKey varKey(varName, renderTargetType);

    // Only mark this render target as an input if the previous stage can output it
    if(!previousStage.mExpectedOutputs.ContainsKey(varKey))
      continue;

    ShaderVariableLinkData* varData = gpuStage.mInputs.FindOrCreate(varKey);
    varData->mType = renderTargetType;
    varData->mName = varName;
  }

  // For the pixel shader to properly link to the gpu stage's system values,
  // we need to determine which system values will actually be written out to
  AutoDeclare(systemValues, systemValueSettings.mShaderStages[ShaderStageType::Pixel].mSystemValues.All());
  for(; !systemValues.Empty(); systemValues.PopFront())
  {
    ShaderSystemValue& systemValue = systemValues.Front().second;
    LanguageSystemValue* languageSystemValue = systemValue.mLanguages.FindPointer(mCurrentLanguageName);
    bool isOutputByPixel = previousStage.mExpectedOutputs.ContainsKey(systemValue.mKey);
    if(languageSystemValue != nullptr && languageSystemValue->IsOutput() && isOutputByPixel)
    {
      ShaderVariableLinkData* varData = gpuStage.mInputs.FindOrCreate(systemValue.mKey);
      varData->mType = systemValue.mType;
      varData->mName = systemValue.mName;
      varData->mAttributes.Insert(nameSettings.mSystemValueInputTag);
    }
  }
}

void ZilchCompositor::CollectExpectedStageOutputs(Array<ShaderType*>& fragmentsOfType, ShaderStageInfo& stageInfo)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  ShaderSystemValueSettings& systemValueSettings = mSettings->mSystemValueSettings;

  for(size_t fragmentIndex = 0; fragmentIndex < fragmentsOfType.Size(); ++fragmentIndex)
  {
    ShaderType* fragment = fragmentsOfType[fragmentIndex];
    // If this is a geometry fragment then iterate over the output sub-struct's field instead
    if(stageInfo.mFragmentStage == ShaderStageType::GeometryVertex && fragment->mFragmentType == FragmentType::Geometry)
      fragment = fragment->mOutputType->mOutputType;

    // Check every field to see if it contains the output attribute
    for(size_t fieldIndex = 0; fieldIndex < fragment->mFieldList.Size(); ++fieldIndex)
    {
      ShaderField* field = fragment->mFieldList[fieldIndex];
      if(field->ContainsAttribute(nameSettings.mOutputAttributeName))
      {
        ShaderFieldKey varKey(field);
        stageInfo.mExpectedOutputs.FindOrCreate(field);
        // Mark this fragment as the last one to output this field
        stageInfo.mLastOutputFragments[varKey] = fragment->mZilchName;

        // If this field is also a system value output then add it to the map of system values
        // (so we know what variables to declare) and mark this as a computed output
        // (we will write to this variable no matter what the next stages say).
        LanguageSystemValue* systemValue = systemValueSettings.FindSystemValue(stageInfo.mFragmentStage, varKey, mTranslator);
        if(systemValue != nullptr && systemValue->IsOutput())
        {
          ShaderVariableLinkData* varData = stageInfo.mStageSystemVariables.FindOrCreate(field);
          varData->mAttributes.Insert(nameSettings.mSystemValueOutputTag);

          varData = stageInfo.mComputedOutputs.FindOrCreate(field);
          varData->mAttributes.Insert(nameSettings.mSystemValueOutputTag);
        }
      }
    }
  }
}

void ZilchCompositor::DetermineFragmentInputTypes(ShaderStageInfo& previousStage, ShaderStageInfo& currentStage, Array<ShaderType*>& fragmentsOfType)
{
  NameSettings& nameSettings = mSettings->mNameSettings;
  ShaderDefinitionSettings& shaderSettings = mSettings->mShaderDefinitionSettings;
  ShaderSystemValueSettings& systemValueSettings = mSettings->mSystemValueSettings;

  // FieldKey to ZilchFragmentName
  HashMap<ShaderFieldKey, String> lastFragmentToOutputVariable;

  for(size_t fragmentIndex = 0; fragmentIndex < fragmentsOfType.Size(); ++fragmentIndex)
  {
    ShaderType* fragment = fragmentsOfType[fragmentIndex];
    ShaderFragmentCompositeInfo* fragmentInfo = currentStage.GetFragment(fragment->mZilchName);

    // If this is the geometry vertex stage then iterate over the input struct instead of the fragment
    if(currentStage.mFragmentStage == ShaderStageType::GeometryVertex && fragment->mFragmentType == FragmentType::Geometry)
      fragment = fragment->mInputType->mInputType;

    // Iterate over all fields on this fragment and match the input attributes one by
    // one (in order) to determine what kind of input type it is.
    for(size_t fieldIndex = 0; fieldIndex < fragment->mFieldList.Size(); ++fieldIndex)
    {
      ShaderField* field = fragment->mFieldList[fieldIndex];
      ShaderFieldKey fieldKey(field);
      ShaderFieldCompositeInfo& fieldInfo = fragmentInfo->GetField(fieldKey);
      fieldInfo.mZilchName = field->mZilchName;
      fieldInfo.mZilchType = field->mZilchType;
      fieldInfo.mInputType = FieldInputType::Unknown;
      fieldInfo.mInputError = false;
      
      // Walk over the attributes in order to try to resolve them.
      // Also mark whether or not we ever see an input attribute so we can report errors.
      bool containsInputAttribute = false;
      ShaderAttributeList::range attributeRange = field->mAttributes.All();
      for(; !attributeRange.Empty(); attributeRange.PopFront())
      {
        String attributeName = attributeRange.Front().mAttributeName;

        // This is a fragment input if there is another fragment that has already output this variable.
        // If so, mark this field as a fragment input and store the fragment that fulfills this input.
        if(attributeName == nameSettings.mFragmentInputAttributeName)
        {
          containsInputAttribute = true;
          if(lastFragmentToOutputVariable.ContainsKey(fieldKey))
          {
            fieldInfo.mInputType = FieldInputType::Fragment;
            fieldInfo.mFragmentDependencyName = lastFragmentToOutputVariable[fieldKey];
            break;
          }
        }
        // This is a stage input if the previous stage has an expected output that matches. If so, mark the field 
        // as a stage input and mark the current stage as having this input (give it the StageInput attribute too).
        if(attributeName == nameSettings.mStageInputAttributeName)
        {
          containsInputAttribute = true;
          if(previousStage.mExpectedOutputs.ContainsKey(fieldKey))
          {
            fieldInfo.mInputType = FieldInputType::Stage;

            ShaderVariableLinkData* varData = currentStage.mInputs.FindOrCreate(fieldKey);
            varData->mName = field->mZilchName;
            varData->mType = field->mZilchType;
            // If this is both a system value output of the previous stage and a system value input of this stage
            // then mark this as just a system value input (it's implicitly passed), otherwise this is a stage input (a varying is needed).
            ShaderVariableLinkData* previousVarData = previousStage.mComputedOutputs.FindValue(fieldKey, nullptr);
            LanguageSystemValue* systemValue = systemValueSettings.FindSystemValue(currentStage.mFragmentStage, fieldKey, mTranslator);
            if(previousVarData != nullptr && previousVarData->mAttributes.Contains(nameSettings.mSystemValueOutputTag) && systemValue != nullptr && systemValue->IsInput())
              varData->mAttributes.Insert(nameSettings.mSystemValueInputTag);
            else
              varData->mAttributes.Insert(nameSettings.mStageInputTag);
            break;
          }
        }
        // This is a built-in input if the shader definition settings has a
        // built-in that matches. If so, then mark this as a built-in.
        if(attributeName == nameSettings.mBuiltinInputAttributeName)
        {
          containsInputAttribute = true;
          // Check if this is a built-in
          if(shaderSettings.mBuiltIns.ContainsKey(fieldKey))
          {
            fieldInfo.mInputType = FieldInputType::BuiltInUniform;
            break;
          }
          // If this is a system value then there is an implicit built-in to grab from
          LanguageSystemValue* systemvalue = systemValueSettings.FindSystemValue(currentStage.mFragmentStage, fieldKey, mTranslator);
          if(systemvalue != nullptr && systemvalue->IsInput())
          {
            fieldInfo.mInputType = FieldInputType::BuiltInUniform;
            ShaderVariableLinkData* varData = currentStage.mStageSystemVariables.FindOrCreate(field);
            varData->mAttributes.Insert(nameSettings.mSystemValueInputTag);
            break;
          }
        }
        // Property inputs always hold true so if we see one then this is automatically a property.
        // @JoshD: Maybe check certain types for valid properties down the line?
        if(attributeName == nameSettings.mPropertyInputAttributeName)
        {
          containsInputAttribute = true;
          fieldInfo.mInputType = FieldInputType::Property;
          ShaderVariableLinkData* varData = currentStage.mProperties.FindOrCreate(fieldKey);
          varData->mName = field->mZilchName;
          varData->mType = field->mZilchType;
          break;
        }
      }

      // If we found an input attribute but we never set an input type then
      // this field failed to properly resolve and is an error
      if(containsInputAttribute && fieldInfo.mInputType == FieldInputType::Unknown)
      {
        fieldInfo.mInputError = true;
        fieldInfo.mInputType = FieldInputType::Property;
      }

      // If this field also contains the output attribute then mark the current
      // fragment as the last one to output the variable
      if(field->ContainsAttribute(nameSettings.mOutputAttributeName))
        lastFragmentToOutputVariable[fieldKey] = fragment->mZilchName;
    }
  }
}

void ZilchCompositor::DetermineGeometryStageInputs(ShaderStageInfo& vertexStageInfo, ShaderStageInfo& geometryStageInfo, ShaderStageInfo& pixelStageInfo)
{
  // We need to determine if any geometry shader inputs need to be declared. This can happen because geometry
  // shaders inherit the expected outputs of vertex shaders so that pixel shaders can link to them and pass-through data.
  // However, if there is a variable that is passed through then the geometry shader will not have this as an actual input.
  // To properly declare inputs we have to walk all inputs of the pixel shader and if any of them match an expected
  // output of the vertex shader then it should be added (if it isn't already) as a geometry shader input.
  AutoDeclare(pixelInputs, pixelStageInfo.mInputs.All());
  for(; !pixelInputs.Empty(); pixelInputs.PopFront())
  {
    ShaderFieldKey pixelInputKey = pixelInputs.Front().first;
    if(!vertexStageInfo.mExpectedOutputs.ContainsKey(pixelInputKey))
      continue;

    // If the geometry stage already outputs this then there's no pass-through required
    if(geometryStageInfo.mComputedOutputs.FindValue(pixelInputKey, nullptr) != nullptr)
      continue;

    // Mark all input fields of this variable in the geometry shader stage as StageInputs.
    // This might need to be revisited if we ever allow more than one geometry fragment.
    AutoDeclare(geoFragmentRange, geometryStageInfo.All());
    for(; !geoFragmentRange.Empty(); geoFragmentRange.PopFront())
    {
      ShaderFragmentCompositeInfo* geoInfo = geoFragmentRange.Front().second;
      ShaderFieldCompositeInfo& fieldInfo = geoInfo->GetField(pixelInputKey);
      fieldInfo.mInputType = FieldInputType::Stage;
    }
    // Mark the geometry stage as having this variable as an input (copy so we get the same attributes)
    ShaderVariableLinkData* varData = geometryStageInfo.mInputs.FindOrCreate(pixelInputKey);
    *varData = *pixelInputs.Front().second;
  }
}

void ZilchCompositor::LinkStage(ShaderStageInfo& previousStage, ShaderStageInfo& currentStage, const String& extraAttribute)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  // Iterate over all inputs of the current stage (inputs are guaranteed to actually be linked)
  // and mark them as actual outputs of the previous stage
  ShaderLinkDefinition::LinkDataMap::range inputs = currentStage.mInputs.All();
  for(; !inputs.Empty(); inputs.PopFront())
  {
    AutoDeclare(pair, inputs.Front());
    ShaderVariableLinkData* varData = previousStage.mComputedOutputs.FindOrCreate(pair.first);
    varData->mName = pair.second->mName;
    varData->mType = pair.second->mType;

    // If this field is a stage input then the tag we need to add is stage output.
    /// If this is a system value input then we need system value output.
    String attributeName = nameSettings.mStageOutputTag;
    if(pair.second->mAttributes.Contains(nameSettings.mSystemValueInputTag))
      attributeName = nameSettings.mSystemValueOutputTag;

    // Mark this as a stage output
    varData->mAttributes.Insert(attributeName);
    // If any extra attributes were passed in then set them too (geometry outputs)
    if(!extraAttribute.Empty())
      varData->mAttributes.Insert(extraAttribute);
  }
}

void ZilchCompositor::DeclareBuiltIns(ShaderCodeBuilder& builder, ShaderStageInfo& stageInfo)
{
  ShaderDefinitionSettings& shaderSettings = mSettings->mShaderDefinitionSettings;
  NameSettings& nameSettings = mSettings->mNameSettings;

  // Write out all built-uniforms
  builder << builder.EmitIndent() << "// Built-in Uniforms" << builder.EmitLineReturn();
  AutoDeclare(builtInsRange, shaderSettings.mBuiltIns.Values());
  for(; !builtInsRange.Empty(); builtInsRange.PopFront())
  {
    ShaderField* builtInField = builtInsRange.Front();

    // All built-in variables are automatically uniforms
    builder << builder.EmitIndent() << nameSettings.mUniformTag;
    // Add all attributes to this variable that are on the built-in field
    ShaderAttributeList::range attributes = builtInField->mAttributes.All();
    for(; !attributes.Empty(); attributes.PopFront())
      builder << "[" << attributes.Front().mAttributeName << "]";
    // Write: "var `varName` : `varType`:"
    builder << " var " << builtInField->mZilchName << " : " << builtInField->mZilchType << ";" << builder.EmitLineReturn();
  }
}

void ZilchCompositor::DeclareStageInputsAndOutputs(ShaderCodeBuilder& builder, ShaderStageInfo& stageInfo)
{
  // Merge stage inputs and outputs together since they end up being the same variable with merged attributes
  ShaderLinkDefinition::LinkDataMap stageInputOutputs;
  stageInputOutputs.Merge(stageInfo.mInputs);
  stageInputOutputs.Merge(stageInfo.mComputedOutputs);
  stageInputOutputs.Merge(stageInfo.mStageSystemVariables);

  // Only emit a comment if there are stage inputs/outputs
  if(!stageInputOutputs.mList.Empty())
    builder << builder.EmitIndent() << "// Stage Input/Outputs" << builder.EmitLineReturn();

  // Declare all input/outputs
  AutoDeclare(inputOutputRange, stageInputOutputs.All());
  for(; !inputOutputRange.Empty(); inputOutputRange.PopFront())
  {
    ShaderVariableLinkData* varData = inputOutputRange.Front().second;

    // Get the attributes
    String attributes = varData->GetAttributeString();
    // Write: "`attributes` var `varName` : `varType`;"
    builder << builder.EmitIndent() << attributes << " var " << varData->mName << " : " << varData->mType << ";" << builder.EmitLineReturn();
  }
}

void ZilchCompositor::DeclareFragmentProperties(CompositeData& compositeData, ShaderStageInfo& stageInfo, ShaderCodeBuilder& builder)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  // Emit a comment only if there are properties (not completely accurate
  // since some could be static but ignore this fringe case)
  if(!stageInfo.mProperties.mList.Empty())
    builder << builder.EmitIndent() << "// Fragment Properties" << builder.EmitLineReturn();

  // Emit all properties
  for(size_t i = 0; i < compositeData.mFragments.Size(); ++i)
  {
    ShaderType* shaderType = compositeData.mFragments[i];
    ShaderFragmentCompositeInfo* fragmentInfo = stageInfo.GetFragment(shaderType->mZilchName);

    ShaderFragmentCompositeInfo::FieldMap::Range fieldRange = fragmentInfo->mFieldMap.All();
    for(; !fieldRange.Empty(); fieldRange.PopFront())
    {
      String fieldKey = fieldRange.Front().first;
      ShaderFieldCompositeInfo& fieldInfo = fieldRange.Front().second;

      // If this field isn't a property or it is a forced static type
      // (such as a sampler) then don't add a member to the composite
      if(fieldInfo.mInputType != FieldInputType::Property || IsTypeForceStatic(fieldInfo.mZilchType))
        continue;

      ShaderField* shaderField = shaderType->FindField(fieldInfo.mZilchName);
      String attributes = BuildString(nameSettings.mUniformTag, nameSettings.mInputTag);
      // Mange the variable name so that the composite doesn't have name conflicts
      String mangledVarName = GenerateFieldUniformName(shaderField);
      String varType = shaderField->mZilchType;
      
      // Write: "  `attributes` var `varName` : `varType`;"
      builder << builder.EmitIndent() << attributes << " var " << mangledVarName << " : " << varType << ";" << builder.EmitLineReturn();
    }
  }
}

void ZilchCompositor::DeclareFragmentInMain(ShaderType* shaderType, CompositeData& compositeData, ShaderStageInfo& stageInfo, ShaderCodeBuilder& builder, HashMap<String, String>& fragmentVarNames)
{
  ShaderDefinitionSettings& shaderSettings = mSettings->mShaderDefinitionSettings;
  ShaderSystemValueSettings& systemValueSettings = mSettings->mSystemValueSettings;
  NameSettings& nameSettings = mSettings->mNameSettings;

  ShaderFragmentCompositeInfo* fragmentInfo = stageInfo.GetFragment(shaderType->mZilchName);

  String fragmentName = shaderType->mZilchName;
  String fragmentVarName = fragmentName.ToLower();
  fragmentVarNames[shaderType->mZilchName] = fragmentVarName;

  // Create an instance of this fragment
  builder << builder.EmitIndent() << "var " << fragmentVarName << " = " << fragmentName << "();" << builder.EmitLineReturn();

  // Copy all field inputs from the correct location depending on its input type
  ShaderFragmentCompositeInfo::FieldMap::Range fieldRange = fragmentInfo->mFieldMap.All();
  for(; !fieldRange.Empty(); fieldRange.PopFront())
  {
    ShaderFieldKey fieldKey = fieldRange.Front().first;
    ShaderFieldCompositeInfo& fieldInfo = fieldRange.Front().second;
    ShaderField* shaderField = shaderType->FindField(fieldInfo.mZilchName);
    
    // If there is no field on this class (shouldn't happen except with some fringe geometry shader cases), the field
    // isn't an input type, or it can't be declared because it's a force static type then skip it.
    if(shaderField == nullptr || fieldInfo.mInputType == FieldInputType::Unknown || IsTypeForceStatic(fieldInfo.mZilchType))
      continue;

    // The default expression for a field is to set the field from the compositor's variable: "this.`VarName`"
    String resultExp = BuildString(Zilch::ThisKeyword, ".", shaderField->mZilchName);
    // If this is a fragment input then set the variable from the fragment's result: "`dependency`.`varName`"
    if(fieldInfo.mInputType == FieldInputType::Fragment)
    {
      // @JoshD: Also deal with this coming from a static field
      String dependencyFragmentName = fragmentVarNames[fieldInfo.mFragmentDependencyName];
      resultExp = BuildString(dependencyFragmentName, ".", shaderField->mZilchName);
    }
    // If this is a stage input then don't do anything
    else if(fieldInfo.mInputType == FieldInputType::Stage)
    {
      // resultExp = "this.`varName`"
    }
    // If this is a built-in then grab it from the composite (it might be static though)
    else if(fieldInfo.mInputType == FieldInputType::BuiltInUniform)
    {
      // If this type came from a global static then access the field from the composite, but as a static.
      // Otherwise just use the default expression: "this.`varName`".
      ShaderField* builtInField = shaderSettings.mBuiltIns.FindValue(fieldKey, nullptr);
      bool foundField = builtInField != nullptr;
      // If we have a built-in field and it is marked as static then translate to grab the field via the class name
      if(builtInField != nullptr && builtInField->ContainsAttribute(nameSettings.mStaticAttribute))
        resultExp = BuildString(compositeData.mShaderTypeName, ".", shaderField->mZilchName);
      // If we didn't find a built-in field this could still be a system value
      else if(builtInField == nullptr)
        foundField = systemValueSettings.ContainsSystemValue(stageInfo.mFragmentStage, fieldKey, mCurrentLanguageName, mCurrentLanguageVersionNumber);
      
      // Simple validation
      ErrorIf(!foundField, "Field '%s' on fragment '%s' is of input type built-in but there is no built-in to resolve from",
        fieldInfo.mZilchName.c_str(), shaderType->mZilchName.c_str());
    }
    // If this is a property then grab it from the property name (mangled based upon the fragment's name)
    else if(fieldInfo.mInputType == FieldInputType::Property)
    {
      String mangledVarName = GenerateFieldUniformName(shaderField);
      resultExp = BuildString(Zilch::ThisKeyword, ".", mangledVarName);
    }
    // Write: "`fragment`.`varName` = `resultExp`;"
    builder << builder.EmitIndent() << fragmentVarName << "." << shaderField->mZilchName << " = " << resultExp << ";" << builder.EmitLineReturn();
  }

  // Call the main function of the fragment. Call with the appropriate syntax depending on the fragment type
  if(shaderType->mFragmentType == FragmentType::Geometry)
    builder << builder.EmitIndent() << fragmentVarName << ".Main(fragmentInput, fragmentOutput);" << builder.EmitLineReturn();
  else
    builder << builder.EmitIndent() << fragmentVarName << ".Main();" << builder.EmitLineReturn();
}

void ZilchCompositor::CopyFinalStageOutputs(ShaderStageInfo& stageInfo, ShaderCodeBuilder& builder, HashMap<String, String>& fragmentVarNames)
{
  // For every computed output, copy the value into the composite's variable based upon what fragment last output it
  AutoDeclare(outputRange, stageInfo.mComputedOutputs.All());
  for(; !outputRange.Empty(); outputRange.PopFront())
  {
    ShaderVariableLinkData* varData = outputRange.Front().second;
    ShaderFieldKey fieldKey(varData->mName, varData->mType);
    String fragmentTypeName = stageInfo.mLastOutputFragments.FindValue(fieldKey, String());
    // If no fragment output this variable then the composite should already have the correct value set
    if(fragmentTypeName.Empty())
      continue;
    
    String fragmentVarName = fragmentVarNames[fragmentTypeName];
    // Write: "this.`varName` = `fragment`.`varName`;"
    builder << builder.EmitIndent() << Zilch::ThisKeyword << "." << varData->mName << " = " << fragmentVarName << "." << varData->mName << ";" << builder.EmitLineReturn();
  }
}

void ZilchCompositor::DeclareCompositeMain(CompositeData& compositeData, ShaderStageInfo& stageInfo, ShaderCodeBuilder& builder)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  // ShaderZilchType to fragment name
  HashMap<String, String> fragmentVarNames;
  // Write the declaration for main (use the attribute for main to signify we want
  // the main() function of the shader to call this class)
  builder << builder.EmitIndent() << nameSettings.mMainAttributeTag << builder.EmitLineReturn();
  builder << builder.EmitIndent() << "function Main()" << builder.EmitLineReturn();
  builder.BeginScope();

  // For every fragment, declare the fragment, copy inputs into it, and call its main function
  for(size_t i = 0; i < compositeData.mFragments.Size(); ++i)
  {
    ShaderType* shaderType = compositeData.mFragments[i];
    DeclareFragmentInMain(shaderType, compositeData, stageInfo, builder, fragmentVarNames);
    // Add a newline after every fragment except the last one
    if(i != compositeData.mFragments.Size() - 1)
      builder.WriteLine();
  }
  // Copy the stage outputs into the composite based upon the last fragment to output each variable
  CopyFinalStageOutputs(stageInfo, builder, fragmentVarNames);

  // Close off the scope of main
  builder.EndScope();
}

String ZilchCompositor::GenerateZilchComposite(CompositeData& compositeData, ShaderStageInfo& stageInfo)
{
  ShaderDefinitionSettings& shaderSettings = mSettings->mShaderDefinitionSettings;
  NameSettings& nameSettings = mSettings->mNameSettings;

  // First output the class Declaration
  ShaderCodeBuilder builder;
  String classAttribute = BuildString("[", compositeData.mFragmentAttributeName, "]");
  builder << classAttribute << builder.EmitLineReturn();
  builder << "struct " << compositeData.mShaderTypeName << builder.EmitLineReturn();
  builder.BeginScope();

  // Write out all members on the composite
  DeclareBuiltIns(builder, stageInfo);
  DeclareStageInputsAndOutputs(builder, stageInfo);
  DeclareFragmentProperties(compositeData, stageInfo, builder);
  builder.WriteLine();

  // Declare the main function
  DeclareCompositeMain(compositeData, stageInfo, builder);

  // close the class scope
  builder.EndScope();

  return builder.ToString();
}

void ZilchCompositor::DeclareGeometryDataStruct(ShaderCodeBuilder& builder, StringParam structName, LinkDataMap& linkedDef)
{
  // Write: "struct `StructName`"
  builder << "struct " << structName << builder.EmitLineReturn();
  builder.BeginScope();

  // Declare all linked variables as zilch variables
  ShaderLinkDefinition::LinkDataMap::range varRange = linkedDef.All();
  for(; !varRange.Empty(); varRange.PopFront())
  {
    ShaderVariableLinkData& varData = *varRange.Front().second;
    String attributes = varData.GetAttributeString();
    // Write: "`Attribute` var `varName` : `varType`;"
    builder << builder.EmitIndent() << attributes << " var " << varData.mName << " : " << varData.mType << ";" << builder.EmitLineReturn();
  }
  builder.EndScope();
}

void ZilchCompositor::GenerateEmitVertexHelper(ShaderCodeBuilder& builder, CompositeData& compositeData, ShaderStageInfo& vertexInfo, GeometryCompositeInfo& geometryInfo)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  EmitVertexCallbackData data;
  data.mCompositedShaderTypeName = compositeData.mShaderTypeName;
  data.mFragmentOutputStructShaderType = geometryInfo.mFragmentOutputStructType;
  data.mCompositeOutVarName = "compositeOutput";
  data.mFragmentOutVarName = "fragmentOutput";

  // Write: "[Static][NoMangle]"
  builder << builder.EmitIndent() << "[" << nameSettings.mStaticAttribute << "][" << nameSettings.mNoMangleAttributeName << "]" << builder.EmitLineReturn();
  // Write: "function EmitVertexHelper(`fragmentOutput` : ref `fragmentOutStructType`, `compositeOutput` : ref `compositeOutStructType`)"
  builder << builder.EmitIndent() << "function " << nameSettings.mEmitVertexHelperFunctionName;
  builder << "(" << data.mFragmentOutVarName << " : ref " << geometryInfo.mFragmentOutputStructType->mZilchName;
  builder << ", " << data.mCompositeOutVarName << " : ref " << geometryInfo.mCompositeOutputStructTypeName << ")" << builder.EmitLineReturn();

  builder.BeginScope();
  // Copy all outputs from the fragment to the composite
  for(size_t i = 0; i < geometryInfo.mFragmentOutputStructType->mFieldList.Size(); ++i)
  {
    ShaderField* shaderField = geometryInfo.mFragmentOutputStructType->mFieldList[i];
    // Make sure this field has the output attribute
    if(!shaderField->ContainsAttribute(nameSettings.mOutputAttributeName))
      continue;

    // Make sure this field is on the composited shader
    ShaderFieldKey varKey(shaderField->mZilchName, shaderField->mZilchType);
    if(!vertexInfo.mComputedOutputs.ContainsKey(varKey))
      continue;

    // Write: "`compositeOut`.`varName` = `fragmentOut`.`varName`;"
    builder << builder.EmitIndent() << data.mCompositeOutVarName << "." << shaderField->mZilchName << " = ";
    builder << data.mFragmentOutVarName << "." << shaderField->mZilchName << ";" << builder.EmitLineReturn();
  }
  // Let the user write any extra logic in their emit vertex (such as Zero's api perspective transform)
  if(mEmitVertexCallback != nullptr)
    mEmitVertexCallback(builder, mSettings, data);

  builder.EndScope();
}

void ZilchCompositor::GenerateGeometryFragmentInputsAndStream(ShaderCodeBuilder& builder, GeometryCompositeInfo& geometryInfo, ShaderStageInfo& vertexInfo)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  // Construct the input stream type
  builder << builder.EmitIndent() << "// Copy composite inputs into fragment inputs" << builder.EmitLineReturn();
  builder << builder.EmitIndent() << "var fragmentInput = " << geometryInfo.mFragmentInputStreamType->mZilchName << "();" << builder.EmitLineReturn();
  builder << builder.EmitIndent() << "for(var i = 0; i < input.Count; ++i)" << builder.EmitLineReturn();
  builder.BeginScope();
  // Copy all inputs from the composite's input stream to the fragment's input stream
  for(size_t i = 0; i < geometryInfo.mFragmentInputStructType->mFieldList.Size(); ++i)
  {
    ShaderField* shaderField = geometryInfo.mFragmentInputStructType->mFieldList[i];
    String varName = shaderField->mZilchName;

    // Only copy if the fragment field is an input
    if(!shaderField->ContainsAttribute(nameSettings.mInputAttributeName))
      continue;

    // Only copy this input if it was also on the composite
    ShaderFieldKey varKey(varName, shaderField->mZilchType);
    if(!vertexInfo.mInputs.ContainsKey(varKey))
      continue;

    // Write: "fragmentInput[i].`varName` = input[i].`varName`;"
    builder << builder.EmitIndent() << "fragmentInput[i]." << varName << " = input[i]." << varName << ";" << builder.EmitLineReturn();
  }
  builder.EndScope();

  // Construct the fragment's output stream. This must be constructed from the composite's output stream type so that
  // special translation can happen in some languages. For instance, in hlsl a new stream can't be constructed so this turns into a name alias (#define).
  builder << builder.EmitIndent() << "// Create the fragment output type" << builder.EmitLineReturn();
  builder << builder.EmitIndent() << "var fragmentOutput = GeometryStreamMover[" << geometryInfo.mCompositeOutputStreamTypeName << ", ";
  builder << geometryInfo.mFragmentOutputStreamType->mZilchName << "].Move(output);" << builder.EmitLineReturn();
  builder.WriteLine();
}

void ZilchCompositor::DeclareGeometryMain(GeometryCompositeInfo& geometryInfo, CompositeData& compositeData, ShaderStageInfo& vertexInfo, ShaderStageInfo& primitiveInfo, ShaderCodeBuilder& builder)
{
  NameSettings& nameSettings = mSettings->mNameSettings;

  // Write the declaration for main (use the attribute for main to signify we want
  // the main() function of the shader to call this class)
  builder << builder.EmitIndent() << nameSettings.mMainAttributeTag << builder.EmitLineReturn();
  // Write: "function Main(input : `inputStreamType`, output : `outputStreamType`)
  builder << builder.EmitIndent() << "function Main(";
  builder << "input : " << geometryInfo.mCompositeInputStreamTypeName << ", ";
  builder << "output : " << geometryInfo.mCompositeOutputStreamTypeName;
  builder << ")" << builder.EmitLineReturn();

  // Generate the "main" function's body
  builder.BeginScope();
  // First we have to copy inputs from the composite to the fragment and declare the fragment's output stream type
  GenerateGeometryFragmentInputsAndStream(builder, geometryInfo, vertexInfo);
  // Then declare all of the fragments, properly copying inputs and outputs between them
  HashMap<String, String> fragmentVarNames;
  DeclareFragmentInMain(compositeData.mFragments[0], compositeData, primitiveInfo, builder, fragmentVarNames);
  builder.EndScope();
}

String ZilchCompositor::GenerateZilchComposite(CompositeData& compositeData, ShaderStageInfo& vertexInfo, ShaderStageInfo& primitiveInfo)
{
  ShaderDefinitionSettings& shaderSettings = mSettings->mShaderDefinitionSettings;
  NameSettings& nameSettings = mSettings->mNameSettings;
  ShaderSystemValueSettings& systemValueSettings = mSettings->mSystemValueSettings;

  ShaderType* geoFragment = compositeData.mFragments[0];

  GeometryCompositeInfo geometryInfo;
  // Cache the input/output stream and data type
  geometryInfo.mFragmentInputStreamType = geoFragment->mInputType;
  geometryInfo.mFragmentOutputStreamType = geoFragment->mOutputType;
  geometryInfo.mFragmentInputStructType = geometryInfo.mFragmentInputStreamType->mInputType;
  geometryInfo.mFragmentOutputStructType = geometryInfo.mFragmentOutputStreamType->mOutputType;
  size_t totalMaxVertices = geoFragment->GetTypeData()->mCount;

  // Build a bunch of name strings up
  String classAttribute = BuildString("[", compositeData.mFragmentAttributeName, "]");
  geometryInfo.mCompositeShaderTypeName = compositeData.mShaderTypeName;
  geometryInfo.mCompositeInputStructTypeName = BuildString(compositeData.mShaderTypeName, "_Input");
  geometryInfo.mCompositeOutputStructTypeName = BuildString(compositeData.mShaderTypeName, "_Output");
  // Build up the composite's stream type names (the same as the fragment's names but with the fragment struct type replaced with the composite's)
  geometryInfo.mCompositeInputStreamTypeName = geometryInfo.mFragmentInputStreamType->mZilchName.Replace(geometryInfo.mFragmentInputStructType->mZilchName, geometryInfo.mCompositeInputStructTypeName);
  geometryInfo.mCompositeOutputStreamTypeName = geometryInfo.mFragmentOutputStreamType->mZilchName.Replace(geometryInfo.mFragmentOutputStructType->mZilchName, geometryInfo.mCompositeOutputStructTypeName);

  ShaderCodeBuilder builder;
  // Declare the input/output structs from the linked variables
  DeclareGeometryDataStruct(builder, geometryInfo.mCompositeInputStructTypeName, vertexInfo.mInputs);
  DeclareGeometryDataStruct(builder, geometryInfo.mCompositeOutputStructTypeName, vertexInfo.mComputedOutputs);
  // Declare the actual geometry composite struct
  builder << "[Geometry(maxVertices : " << ToString(totalMaxVertices) << ")]" << builder.EmitLineReturn();
  builder << "struct " << geometryInfo.mCompositeShaderTypeName << builder.EmitLineReturn();
  builder.BeginScope();

  // Declare all built-in variables and mark these as being last output by the global scope
  DeclareBuiltIns(builder, primitiveInfo);
  DeclareStageInputsAndOutputs(builder, primitiveInfo);
  DeclareFragmentProperties(compositeData, primitiveInfo, builder);
  builder.WriteLine();

  // Declare the emit vertex helper function
  GenerateEmitVertexHelper(builder, compositeData, vertexInfo, geometryInfo);
  builder.WriteLine();

  DeclareGeometryMain(geometryInfo, compositeData, vertexInfo, primitiveInfo, builder);

  builder.EndScope();
  return builder.ToString();
}

bool ZilchCompositor::IsTypeForceStatic(StringParam typeName)
{
  // Find the shader type if it exists and check its attributes
  ShaderType* shaderType = mInputLibraryRef->FindType(typeName);
  return shaderType->ContainsAttribute(mSettings->mNameSettings.mForcedStaticAttribute);
}

}//namespace Zero
