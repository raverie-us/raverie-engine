// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

ShaderStage::Enum FragmentTypeToShaderStage(FragmentType::Enum fragmentType)
{
  if (fragmentType == FragmentType::Vertex)
    return ShaderStage::Vertex;
  if (fragmentType == FragmentType::Geometry)
    return ShaderStage::Geometry;
  if (fragmentType == FragmentType::Pixel)
    return ShaderStage::Pixel;
  if (fragmentType == FragmentType::Compute)
    return ShaderStage::Compute;
  return (ShaderStage::Enum)0;
}

FragmentType::Enum ShaderStageToFragmentType(ShaderStage::Enum shaderStage)
{
  if (shaderStage & ShaderStage::Vertex)
    return FragmentType::Vertex;
  if (shaderStage & ShaderStage::Geometry)
    return FragmentType::Geometry;
  if (shaderStage & ShaderStage::Pixel)
    return FragmentType::Pixel;
  if (shaderStage & ShaderStage::Compute)
    return FragmentType::Compute;
  return (FragmentType::Enum)0;
}

String SpirVNameSettings::mNonCopyableAttributeName = "NonCopyable";
String SpirVNameSettings::mStorageClassAttribute = "StorageClass";
String SpirVNameSettings::mNameOverrideParam = "name";
String SpirVNameSettings::mRequiresPixelAttribute = "RequiresPixel";
String SpirVNameSettings::mRuntimeArrayTypeName = "RuntimeArray";

SpirVNameSettings::SpirVNameSettings()
{
  mInputAttribute = "Input";
  mFragmentInputAttribute = "FragmentInput";
  mStageInputAttribute = "StageInput";
  mHardwareBuiltInInputAttribute = "HardwareBuiltInInput";
  mAppBuiltInInputAttribute = "AppBuiltInInput";
  mPropertyInputAttribute = "PropertyInput";

  mOutputAttribute = "Output";
  mFragmentOutputAttribute = "FragmentOutput";
  mStageOutputAttribute = "StageOutput";
  mHardwareBuiltInOutputAttribute = "HardwareBuiltInOutput";

  mStaticAttribute = "Static";
  mExtensionAttribute = "Extension";
  mImplementsAttribute = "Implements";
  mVertexAttribute = "Vertex";
  mGeometryAttribute = "Geometry";
  mPixelAttribute = "Pixel";
  mComputeAttribute = "Compute";
  mComputeLocalSizeXParam = "localSizeX";
  mComputeLocalSizeYParam = "localSizeY";
  mComputeLocalSizeZParam = "localSizeZ";
  mMaxVerticesParam = "maxVertices";

  mApiPerspectivePositionName = "ApiPerspectivePosition";
  mPerspectiveToApiPerspectiveName = "PerspectiveToApiPerspective";

  // Make an array for easy indexing of fragment type attribute names
  mFragmentTypeAttributes.Resize(FragmentType::Size);
  mFragmentTypeAttributes[FragmentType::Vertex] = mVertexAttribute;
  mFragmentTypeAttributes[FragmentType::Geometry] = mGeometryAttribute;
  mFragmentTypeAttributes[FragmentType::Pixel] = mPixelAttribute;
  // Do the same for requirement attributes (currently only pixel requirements
  // exist)
  mRequiresAttributes.Resize(FragmentType::Size);
  mRequiresAttributes[FragmentType::Pixel] = mRequiresPixelAttribute;

  mEntryPointAttributeName = "EntryPoint";
  mMainFunctionName = "Main";
  mUnitTestAttribute = "UnitTest";
  mSpecializationConstantAttribute = "SpecConstant";
  mSpecializationConstantInputAttribute = BuildString(mSpecializationConstantAttribute, "Input");
  mFragmentSharedAttribute = "FragmentShared";

  // Create the implied sub-attributes for [Input]
  mInputSubAttributes.PushBack(mFragmentInputAttribute);
  mInputSubAttributes.PushBack(mHardwareBuiltInInputAttribute);
  mInputSubAttributes.PushBack(mStageInputAttribute);
  mInputSubAttributes.PushBack(mAppBuiltInInputAttribute);
  mInputSubAttributes.PushBack(mPropertyInputAttribute);

  // Create the implied sub-attributes for [Output]
  mOutputSubAttributes.PushBack(mFragmentOutputAttribute);
  mOutputSubAttributes.PushBack(mStageOutputAttribute);

  mAllowedFieldAttributes.Insert(mInputAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mFragmentInputAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mStageInputAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mHardwareBuiltInInputAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mAppBuiltInInputAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mPropertyInputAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mOutputAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mFragmentOutputAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mStageOutputAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mHardwareBuiltInOutputAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mStaticAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mSpecializationConstantAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mSpecializationConstantInputAttribute, AttributeInfo());
  mAllowedFieldAttributes.Insert(mFragmentSharedAttribute, AttributeInfo());

  mAllowedFunctionAttributes.Insert(mStaticAttribute, AttributeInfo());
  mAllowedFunctionAttributes.Insert(mExtensionAttribute, AttributeInfo());
  mAllowedFunctionAttributes.Insert(mImplementsAttribute, AttributeInfo());
  mAllowedFunctionAttributes.Insert(mEntryPointAttributeName, AttributeInfo(true));

  mAllowedClassAttributes.Insert(mVertexAttribute, AttributeInfo());
  mAllowedClassAttributes.Insert(mGeometryAttribute, AttributeInfo());
  mAllowedClassAttributes.Insert(mPixelAttribute, AttributeInfo());
  mAllowedClassAttributes.Insert(mComputeAttribute, AttributeInfo());
  mAllowedClassAttributes.Insert(mStorageClassAttribute, AttributeInfo(true));
  mAllowedClassAttributes.Insert(mNonCopyableAttributeName, AttributeInfo(true));
  mAllowedClassAttributes.Insert(mUnitTestAttribute, AttributeInfo(true));
}

ShaderStage::Enum UniformBufferDescription::mAllStagesMask =
    (ShaderStage::Vertex | ShaderStage::PreTesselation | ShaderStage::PostTesselation | ShaderStage::Geometry |
     ShaderStage::Pixel | ShaderStage::Compute);

UniformBufferDescription::UniformBufferDescription()
{
  Set(0, 0, mAllStagesMask, "Uniform");
}

UniformBufferDescription::UniformBufferDescription(u32 bindingId, u32 descriptorSetId)
{
  Set(bindingId, descriptorSetId, mAllStagesMask, "Uniform");
}

UniformBufferDescription::UniformBufferDescription(const UniformBufferDescription& rhs)
{
  CopyFrom(rhs);
}

UniformBufferDescription::~UniformBufferDescription()
{
  for (size_t i = 0; i < mFields.Size(); ++i)
    delete mFields[i];
  mFields.Clear();
}

void UniformBufferDescription::operator=(const UniformBufferDescription& rhs)
{
  CopyFrom(rhs);
}

void UniformBufferDescription::CopyFrom(const UniformBufferDescription& source)
{
  mBindingId = source.mBindingId;
  mDescriptorSetId = source.mDescriptorSetId;
  mDebugName = source.mDebugName;
  mAllowedStages = source.mAllowedStages;

  // Deep copy the fields
  for (size_t i = 0; i < source.mFields.Size(); ++i)
  {
    ShaderIRFieldMeta* fieldMeta = new ShaderIRFieldMeta();
    *fieldMeta = *source.mFields[i];
    mFields.PushBack(fieldMeta);
  }
}

void UniformBufferDescription::Set(u32 bindingId,
                                   u32 descriptorSetId,
                                   ShaderStage::Enum allowedStages,
                                   StringParam debugName)
{
  mBindingId = bindingId;
  mDescriptorSetId = descriptorSetId;
  mAllowedStages = allowedStages;
  mDebugName = debugName;
}

void UniformBufferDescription::AddField(Raverie::BoundType* type, StringParam fieldName)
{
  ShaderIRFieldMeta* fieldMeta = new ShaderIRFieldMeta();
  fieldMeta->mRaverieName = fieldName;
  fieldMeta->mRaverieType = type;
  mFields.PushBack(fieldMeta);
}

BuiltInBlockDescription::BuiltInBlockDescription()
{
  mInterfaceBlock = true;
}

BuiltInBlockDescription::BuiltInFieldMeta::BuiltInFieldMeta()
{
  mMeta = nullptr;
}

BuiltInBlockDescription::BuiltInFieldMeta::BuiltInFieldMeta(const BuiltInFieldMeta& rhs)
{
  CopyFrom(rhs);
}

BuiltInBlockDescription::BuiltInFieldMeta::~BuiltInFieldMeta()
{
  SafeDelete(mMeta);
}

void BuiltInBlockDescription::BuiltInFieldMeta::operator=(const BuiltInFieldMeta& rhs)
{
  CopyFrom(rhs);
}

void BuiltInBlockDescription::BuiltInFieldMeta::CopyFrom(const BuiltInFieldMeta& source)
{
  mMeta = new ShaderIRFieldMeta();
  mMeta->mRaverieName = source.mMeta->mRaverieName;
  mMeta->mRaverieType = source.mMeta->mRaverieType;
  mMeta->mAttributes = source.mMeta->mAttributes;
  mId = source.mId;
}

void BuiltInBlockDescription::AddField(Raverie::BoundType* type,
                                       StringParam fieldName,
                                       spv::BuiltIn builtInId,
                                       StringParam attribute)
{
  BuiltInFieldMeta builtInMeta;
  builtInMeta.mMeta = new ShaderIRFieldMeta();
  builtInMeta.mMeta->mRaverieName = fieldName;
  builtInMeta.mMeta->mRaverieType = type;
  builtInMeta.mMeta->mAttributes.AddAttribute(attribute, nullptr);

  builtInMeta.mId = builtInId;

  mFields.PushBack(builtInMeta);
}

void BuiltInBlockDescription::SetBuiltInName(spv::BuiltIn builtInId, StringParam name)
{
  // Override the raverie name of anything that matches the given id
  for (size_t i = 0; i < mFields.Size(); ++i)
  {
    BuiltInFieldMeta& fieldMeta = mFields[i];
    if (fieldMeta.mId != builtInId)
      continue;

    fieldMeta.mMeta->mRaverieName = name;
  }
}

BuiltInBlockDescription::BuiltInFieldMeta* BuiltInBlockDescription::FindField(ShaderFieldKey fieldKey)
{
  // This could be improved to be O(1) instead of O(n) but this list is
  // typically very small so worry about optimizing this later.
  for (size_t i = 0; i < mFields.Size(); ++i)
  {
    BuiltInFieldMeta* builtInMeta = &mFields[i];
    if (builtInMeta->mMeta->MakeFieldKey() == fieldKey)
      return builtInMeta;
  }
  return nullptr;
}

BuiltInStageDescription::BuiltInStageDescription()
{
  mInputGlobals.mInterfaceBlock = false;
  mOutputGlobals.mInterfaceBlock = false;
}

bool BuiltInStageDescription::IsValidHardwareBuiltIn(ShaderFieldKey& fieldKey, bool isInput)
{
  if (isInput)
    return mInternalInputMappings.FindValue(fieldKey, nullptr) != nullptr;
  else
    return mInternalOutputMappings.FindValue(fieldKey, nullptr) != nullptr;
}

void BuiltInStageDescription::Finalize()
{
  // Shouldn't be necessary, but clear the old mappings
  mInternalInputMappings.Clear();
  mInternalOutputMappings.Clear();

  // Map all of the inputs/outputs fields to their respective descriptions for
  // quick lookup
  Finalize(mInputInterfaceBlock, mInternalInputMappings);
  Finalize(mInputGlobals, mInternalInputMappings);
  Finalize(mOutputInterfaceBlock, mInternalOutputMappings);
  Finalize(mOutputGlobals, mInternalOutputMappings);
}

void BuiltInStageDescription::Finalize(BuiltInBlockDescription& block, FieldKeyToBlockMap& mappings)
{
  // For each filed, add a mapping from its field-key to the block it came from
  for (size_t i = 0; i < block.mFields.Size(); ++i)
  {
    ShaderIRFieldMeta* fieldMeta = block.mFields[i].mMeta;
    ShaderFieldKey key = fieldMeta->MakeFieldKey();
    mappings.Insert(key, &block);
  }
}

bool BuiltInStageDescription::ValidateIfHardwareBuiltIn(ShaderFieldKey& fieldKey)
{
  // Map all of the inputs/outputs fields to their respective descriptions for
  // quick lookup
  bool isValid = ValidateIfHardwareBuiltIn(fieldKey, mInputInterfaceBlock);
  isValid |= ValidateIfHardwareBuiltIn(fieldKey, mInputGlobals);
  isValid |= ValidateIfHardwareBuiltIn(fieldKey, mOutputInterfaceBlock);
  isValid |= ValidateIfHardwareBuiltIn(fieldKey, mOutputGlobals);
  return isValid;
}

bool BuiltInStageDescription::ValidateIfHardwareBuiltIn(ShaderFieldKey& fieldKey, BuiltInBlockDescription& block)
{
  // For each filed, add a mapping from its field-key to the block it came from
  for (size_t i = 0; i < block.mFields.Size(); ++i)
  {
    ShaderIRFieldMeta* fieldMeta = block.mFields[i].mMeta;
    ShaderFieldKey key = fieldMeta->MakeFieldKey();
    if (key == fieldKey)
      return true;
  }
  return false;
}

VertexDefinitionDescription::~VertexDefinitionDescription()
{
  DeleteObjectsIn(mFields);
}

void VertexDefinitionDescription::AddField(Raverie::BoundType* type, StringParam fieldName)
{
  ShaderIRFieldMeta* fieldMeta = new ShaderIRFieldMeta();
  fieldMeta->mRaverieName = fieldName;
  fieldMeta->mRaverieType = type;
  mFields.PushBack(fieldMeta);
}

CallbackSettings::CallbackSettings()
{
  mCompositeCallback = nullptr;
  mCompositeCallbackUserData = nullptr;
  mAppendCallback = nullptr;
  mAppendCallbackUserData = nullptr;
}

void CallbackSettings::SetCompositeCallback(ShaderCompositeCallback callback, void* userData)
{
  mCompositeCallback = callback;
  mCompositeCallbackUserData = userData;
}

void CallbackSettings::SetAppendCallback(AppendVertexCallback callback, void* userData)
{
  mAppendCallback = callback;
  mAppendCallbackUserData = userData;
}

RaverieShaderErrorSettings::RaverieShaderErrorSettings()
{
  mFrontEndErrorOnNoMainFunction = false;
}

String RaverieShaderSpirVSettings::mLanguageSpecConstantName = "LanguageId";
String RaverieShaderSpirVSettings::mLanguageVersionSpecConstantName = "LanguageVersion";

RaverieShaderSpirVSettings::RaverieShaderSpirVSettings()
{
  mFinalized = false;
  mAllowUniformMaterialBufferIndexOverap = false;
  InitializeBuiltIns();
  mRenderTargetType = RaverieTypeId(Raverie::Real4);
}

RaverieShaderSpirVSettings::RaverieShaderSpirVSettings(const SpirVNameSettings& nameSettings) : mNameSettings(nameSettings)
{
  mFinalized = false;
  mAllowUniformMaterialBufferIndexOverap = false;
  InitializeBuiltIns();
  mRenderTargetType = RaverieTypeId(Raverie::Real4);
}

void RaverieShaderSpirVSettings::AddUniformBufferDescription(UniformBufferDescription& description)
{
  ReturnIf(mFinalized, , "Cannot set built-in names once finalized");

  mUniformBufferDescriptions.PushBack(description);
}

void RaverieShaderSpirVSettings::AutoSetDefaultUniformBufferDescription(int descriptorSetId, StringParam debugName)
{
  ReturnIf(mFinalized, , "Cannot set built-in names once finalized");

  SetDefaultUniformBufferDescription(mUniformBufferDescriptions.Size(), descriptorSetId, debugName);
}

void RaverieShaderSpirVSettings::SetDefaultUniformBufferDescription(int bindingId,
                                                                  int descriptorSetId,
                                                                  StringParam debugName)
{
  ReturnIf(mFinalized, , "Cannot set built-in names once finalized");

  mDefaultUniformBufferDescription.mAllowedStages = UniformBufferDescription::mAllStagesMask;
  mDefaultUniformBufferDescription.mBindingId = bindingId;
  mDefaultUniformBufferDescription.mDescriptorSetId = descriptorSetId;
  mDefaultUniformBufferDescription.mDebugName = debugName;
}

bool RaverieShaderSpirVSettings::IsValidUniform(FragmentType::Enum fragmentType,
                                              StringParam fieldType,
                                              StringParam fieldName)
{
  ShaderStage::Enum shaderStage = FragmentTypeToShaderStage(fragmentType);
  ShaderFieldKey fieldKey(fieldName, fieldType);
  for (size_t i = 0; i < mUniformBufferDescriptions.Size(); ++i)
  {
    UniformBufferDescription& bufferDescription = mUniformBufferDescriptions[i];
    // Make sure this buffer is active for the given stage
    if (!bufferDescription.mAllowedStages.IsSet(shaderStage))
      continue;

    for (size_t j = 0; j < bufferDescription.mFields.Size(); ++j)
    {
      ShaderIRFieldMeta* fieldMeta = bufferDescription.mFields[j];
      if (fieldMeta->MakeFieldKey() == fieldKey)
        return true;
    }
  }
  return false;
}

void RaverieShaderSpirVSettings::SetHardwareBuiltInName(spv::BuiltIn builtInId, StringParam name)
{
  ReturnIf(mFinalized, , "Cannot set built-in names once finalized");

  for (size_t i = 0; i < FragmentType::Size; ++i)
  {
    mBuiltIns[i].mInputInterfaceBlock.SetBuiltInName(builtInId, name);
    mBuiltIns[i].mInputGlobals.SetBuiltInName(builtInId, name);
    mBuiltIns[i].mOutputInterfaceBlock.SetBuiltInName(builtInId, name);
    mBuiltIns[i].mOutputGlobals.SetBuiltInName(builtInId, name);
  }
}

bool RaverieShaderSpirVSettings::IsValidHardwareBuiltIn(FragmentType::Enum fragmentType,
                                                      StringParam fieldType,
                                                      StringParam fieldName,
                                                      bool isInput)
{
  ShaderFieldKey fieldKey(fieldName, fieldType);

  // @JoshD: This is technically wrong as Geometry in/out streams do have
  // hardware built-ins but there's no way to know if this type is used in a
  // stream or not at this point. As a fallback, if no fragment type is set then
  // check all available stage descriptions so at least typos / wrong types
  // could be caught.
  if (fragmentType == FragmentType::None)
    return IsValidHardwareBuiltInAnyStage(fieldKey, isInput);

  BuiltInStageDescription& stageBuiltIns = mBuiltIns[fragmentType];
  return stageBuiltIns.IsValidHardwareBuiltIn(fieldKey, isInput);
}

void RaverieShaderSpirVSettings::SetMaxSimultaneousRenderTargets(size_t maxNumber)
{
  ReturnIf(mFinalized, , "Cannot set max render targets once finalized");
  mRenderTargetNames.Resize(maxNumber);
}

void RaverieShaderSpirVSettings::SetRenderTargetName(StringParam varName, size_t targetIndex)
{
  ReturnIf(mFinalized, , "Cannot rename render target once finalized");

  // Make sure the user is not trying to set an invalid render target name
  if (targetIndex >= mRenderTargetNames.Size())
  {
    Error("Render target %d is invalid. There are only %d allowed simultaneous "
          "render targets. "
          "Use SetMaxSimultaneousRenderTargets to increase the limit.",
          targetIndex,
          mRenderTargetNames.Size());
    return;
  }

  // Make sure the user is not setting two render target indices to same name
  for (size_t i = 0; i < mRenderTargetNames.Size(); ++i)
  {
    if (i == targetIndex)
      continue;

    if (mRenderTargetNames[i] == varName)
    {
      Error("Render target name '%s' is already used in target(%d).", varName.c_str(), i);
    }
  }

  mRenderTargetNames[targetIndex] = varName;
}

void RaverieShaderSpirVSettings::SetRenderTargetType(Raverie::BoundType* targetType)
{
  mRenderTargetType = targetType;
}

String RaverieShaderSpirVSettings::GetLanguageSpecializationName()
{
  return mLanguageSpecConstantName;
}

String RaverieShaderSpirVSettings::GetLanguageVersionSpecializationName()
{
  return mLanguageVersionSpecConstantName;
}

void* RaverieShaderSpirVSettings::GetLanguageSpecializationKey()
{
  return &mLanguageSpecConstantName;
}

void* RaverieShaderSpirVSettings::GetLanguageVersionSpecializationKey()
{
  return &mLanguageVersionSpecConstantName;
}

void RaverieShaderSpirVSettings::Finalize()
{
  if (mFinalized)
  {
    Error("Cannot finalize settings twice.");
    return;
  }

  mFinalized = true;

  // Validate everything
  Validate();

  // Finalize all built-ins (process to a more run-time friendly format)
  for (size_t i = 0; i < FragmentType::Size; ++i)
  {
    mBuiltIns[i].Finalize();
  }
}

bool RaverieShaderSpirVSettings::IsFinalized() const
{
  return mFinalized;
}

void RaverieShaderSpirVSettings::Validate()
{
  ValidateUniformsDescriptions();
  ValidateBuiltInNames();
  ValidateAppBuiltInsAgainstHardwareBuiltIns();
}

void RaverieShaderSpirVSettings::ValidateUniformsDescriptions()
{
  ErrorIf(mDefaultUniformBufferDescription.mFields.Size() != 0,
          "No fields can be set in the default uniform buffer description");

  typedef BitField<ShaderStage::Enum> ShaderStageFlags;
  typedef Pair<int, int> DescriptorSetBindingPair;
  HashMap<ShaderFieldKey, ShaderStageFlags> registeredUniforms;
  HashMap<DescriptorSetBindingPair, UniformBufferDescription*> descriptorMap;

  for (size_t i = 0; i < mUniformBufferDescriptions.Size(); ++i)
  {
    UniformBufferDescription& description = mUniformBufferDescriptions[i];

    // Validate that the descriptor set and binding id are only used once
    DescriptorSetBindingPair descriptorPair =
        DescriptorSetBindingPair(description.mDescriptorSetId, description.mBindingId);
    if (descriptorMap.ContainsKey(descriptorPair))
    {
      UniformBufferDescription* oldDescription = descriptorMap[descriptorPair];
      Error("Uniform block '%s' shares the descriptor set '%d' and binding "
            "'%d' with block '%s'.",
            description.mDebugName.c_str(),
            descriptorPair.first,
            descriptorPair.second,
            oldDescription->mDebugName.c_str());
    }
    descriptorMap.Insert(descriptorPair, &description);

    // Validate that this field isn't registered to another block with the same
    // shader stage
    for (size_t fieldId = 0; fieldId < description.mFields.Size(); ++fieldId)
    {
      ShaderIRFieldMeta* fieldMeta = description.mFields[fieldId];
      ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey();

      ShaderStageFlags& usedFlags = registeredUniforms[fieldKey];
      // If any of the same shader stages are set then report an error
      if (usedFlags.IsSet(description.mAllowedStages.U32Field))
      {
        Error("Field '%s' on block '%s' is registered for the same stage "
              "multiple times.",
              fieldKey.mKey.c_str(),
              description.mDebugName.c_str());
      }
      usedFlags.SetFlag(description.mAllowedStages.U32Field);
    }
  }
}

void RaverieShaderSpirVSettings::ValidateBuiltInNames()
{
  HashMap<String, spv::BuiltIn> keyMappings;
  for (size_t i = 0; i < FragmentType::Size; ++i)
  {
    BuiltInStageDescription& stageDescription = mBuiltIns[i];
    ValidateBuiltInNames(stageDescription.mInputInterfaceBlock, keyMappings);
    ValidateBuiltInNames(stageDescription.mInputGlobals, keyMappings);
    ValidateBuiltInNames(stageDescription.mOutputInterfaceBlock, keyMappings);
    ValidateBuiltInNames(stageDescription.mOutputGlobals, keyMappings);
  }
}

void RaverieShaderSpirVSettings::ValidateBuiltInNames(BuiltInBlockDescription& blockDescription,
                                                    HashMap<String, spv::BuiltIn>& keyMappings)
{
  for (size_t i = 0; i < blockDescription.mFields.Size(); ++i)
  {
    BuiltInBlockDescription::BuiltInFieldMeta& fieldMeta = blockDescription.mFields[i];
    spv::BuiltIn builtInId = fieldMeta.mId;
    ShaderFieldKey fieldKey = fieldMeta.mMeta->MakeFieldKey();

    spv::BuiltIn* result = keyMappings.FindPointer(fieldKey);
    if (result != nullptr && *result != builtInId)
    {
      Error("BuiltIn '%s' is declared as multiple spirv builtins", fieldKey.mKey.c_str());
    }
    keyMappings[fieldKey] = builtInId;
  }
}

void RaverieShaderSpirVSettings::ValidateAppBuiltInsAgainstHardwareBuiltIns()
{
  for (size_t i = 0; i < mUniformBufferDescriptions.Size(); ++i)
  {
    UniformBufferDescription& description = mUniformBufferDescriptions[i];

    for (size_t j = 0; j < description.mFields.Size(); ++j)
    {
      ShaderIRFieldMeta* fieldMeta = description.mFields[j];
      ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey();

      bool isHardwardBuiltIn = ValidateAgainstHardwareBuiltIns(fieldKey);
      ErrorIf(isHardwardBuiltIn,
              "AppBuiltIn '%s : %s' matches a HardwareBuiltIn. "
              "This is currently not supported as additional name mangling "
              "would be necessary to prevent name conflicts which would make "
              "reflection significantly more complicated.",
              fieldMeta->mRaverieName.c_str(),
              fieldMeta->mRaverieType->ToString().c_str());
    }
  }
}

void RaverieShaderSpirVSettings::InitializeBuiltIns()
{
  String hardwareBuiltInInput = mNameSettings.mHardwareBuiltInInputAttribute;
  String hardwareBuiltInOutput = mNameSettings.mHardwareBuiltInOutputAttribute;

  Raverie::BoundType* realType = RaverieTypeId(Raverie::Real);
  Raverie::BoundType* real2Type = RaverieTypeId(Raverie::Real2);
  Raverie::BoundType* real4Type = RaverieTypeId(Raverie::Real4);
  Raverie::BoundType* intType = RaverieTypeId(Raverie::Integer);
  Raverie::BoundType* int3Type = RaverieTypeId(Raverie::Integer3);
  Raverie::BoundType* boolType = RaverieTypeId(Raverie::Boolean);

  BuiltInStageDescription& vertexDescriptions = mBuiltIns[FragmentType::Vertex];
  vertexDescriptions.mOutputInterfaceBlock.AddField(real4Type, "Position", spv::BuiltInPosition, hardwareBuiltInOutput);
  vertexDescriptions.mOutputInterfaceBlock.AddField(
      realType, "PointSize", spv::BuiltInPointSize, hardwareBuiltInOutput);
  // Can't add clip distance now because of array types
  // @JoshD: Fix with FixedArray later?
  // vertexDescriptions.mOutputInterfaceBlock.AddField(real4Type,
  // "ClipDistance", spv::BuiltInClipDistance);

  vertexDescriptions.mInputGlobals.AddField(intType, "VertexId", spv::BuiltInVertexId, hardwareBuiltInInput);
  vertexDescriptions.mInputGlobals.AddField(intType, "InstanceId", spv::BuiltInInstanceId, hardwareBuiltInInput);

  BuiltInStageDescription& geometryDescriptions = mBuiltIns[FragmentType::Geometry];
  geometryDescriptions.mOutputInterfaceBlock.AddField(
      real4Type, "Position", spv::BuiltInPosition, hardwareBuiltInOutput);
  geometryDescriptions.mInputInterfaceBlock.AddField(real4Type, "Position", spv::BuiltInPosition, hardwareBuiltInInput);
  geometryDescriptions.mInputGlobals.AddField(intType, "PrimitiveId", spv::BuiltInPrimitiveId, hardwareBuiltInInput);
  geometryDescriptions.mOutputGlobals.AddField(intType, "PrimitiveId", spv::BuiltInPrimitiveId, hardwareBuiltInOutput);
  // @JoshD: Requires glsl 400 to work. Deal with later!
  // geometryDescriptions.mInputGlobals.AddField(intType, "InvocationId",
  // spv::BuiltInInvocationId);

  BuiltInStageDescription& pixelDescriptions = mBuiltIns[FragmentType::Pixel];
  pixelDescriptions.mOutputGlobals.AddField(realType, "FragDepth", spv::BuiltInFragDepth, hardwareBuiltInOutput);
  pixelDescriptions.mInputGlobals.AddField(real4Type, "FragCoord", spv::BuiltInFragCoord, hardwareBuiltInInput);
  pixelDescriptions.mInputGlobals.AddField(real2Type, "PointCoord", spv::BuiltInPointCoord, hardwareBuiltInInput);
  pixelDescriptions.mInputGlobals.AddField(boolType, "FrontFacing", spv::BuiltInFrontFacing, hardwareBuiltInInput);
  // SpirV currently doesn't support this as a pixel input. Seems to be a bug (I
  // filed it and am waiting). Seems to only cause problems currently in the
  // validator.
  pixelDescriptions.mInputGlobals.AddField(intType, "PrimitiveId", spv::BuiltInPrimitiveId, hardwareBuiltInInput);

  BuiltInStageDescription& computeDescriptions = mBuiltIns[FragmentType::Compute];
  computeDescriptions.mInputGlobals.AddField(
      int3Type, "GlobalInvocationId", spv::BuiltInGlobalInvocationId, hardwareBuiltInInput);
  computeDescriptions.mInputGlobals.AddField(
      int3Type, "LocalInvocationId", spv::BuiltInLocalInvocationId, hardwareBuiltInInput);
  computeDescriptions.mInputGlobals.AddField(
      intType, "LocalInvocationIndex", spv::BuiltInLocalInvocationIndex, hardwareBuiltInInput);
  computeDescriptions.mInputGlobals.AddField(
      int3Type, "NumWorkgroups", spv::BuiltInNumWorkgroups, hardwareBuiltInInput);
  computeDescriptions.mInputGlobals.AddField(int3Type, "WorkgroupId", spv::BuiltInWorkgroupId, hardwareBuiltInInput);
  computeDescriptions.mInputGlobals.AddField(
      int3Type, "WorkgroupSize", spv::BuiltInWorkgroupSize, hardwareBuiltInInput);
}

bool RaverieShaderSpirVSettings::IsValidHardwareBuiltInAnyStage(ShaderFieldKey& fieldKey, bool isInput)
{
  for (size_t i = 0; i < FragmentType::Size; ++i)
  {
    BuiltInStageDescription& stageBuiltIns = mBuiltIns[i];
    if (stageBuiltIns.IsValidHardwareBuiltIn(fieldKey, isInput))
      return true;
  }
  return false;
}

bool RaverieShaderSpirVSettings::ValidateAgainstHardwareBuiltIns(ShaderFieldKey& fieldKey)
{
  for (size_t i = 0; i < FragmentType::Size; ++i)
  {
    BuiltInStageDescription& stageBuiltIns = mBuiltIns[i];
    if (stageBuiltIns.ValidateIfHardwareBuiltIn(fieldKey))
      return true;
  }
  return false;
}

} // namespace Raverie
