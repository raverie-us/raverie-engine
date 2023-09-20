// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

GlslBackendInternalData::GlslBackendInternalData()
{
  mCompiler = nullptr;
  mResources = nullptr;
  mOutputResult = nullptr;
}

RaverieShaderGlslBackend::RaverieShaderGlslBackend()
{
  mTargetVersion = 150;
  mTargetGlslEs = false;
}

String RaverieShaderGlslBackend::GetExtension()
{
  return "glsl";
}

bool RaverieShaderGlslBackend::RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData)
{
  mErrorLog.Clear();

  ShaderByteStream& inputByteStream = inputData.mByteStream;
  uint32_t* data = (uint32_t*)inputByteStream.Data();
  uint32 wordCount = inputByteStream.WordCount();

  spirv_cross::CompilerGLSL compiler(data, wordCount);
  // Set options
  spirv_cross::CompilerGLSL::Options opts = compiler.get_common_options();
  opts.version = mTargetVersion;
  // mTargetGlslEs = true;
  opts.es = mTargetGlslEs;
  // opts.version = 300;
  compiler.set_common_options(opts);

  spirv_cross::ShaderResources resources = compiler.get_shader_resources();

  // Create a helper struct to pass a lot of data around
  GlslBackendInternalData internalData;
  internalData.mCompiler = &compiler;
  internalData.mResources = &resources;
  internalData.mOutputResult = &outputData;

  // Deal with split image samplers by combining them
  compiler.build_dummy_sampler_for_combined_images();
  compiler.build_combined_image_samplers();

  // Deal with glsl matching interface blocks by name
  FixInterfaceBlockNames(internalData);
  // Find what binding ids we use for sampled images
  FindUsedSampledImageBindings(internalData);
  // Combined sampled images (via the backend) don't get binding ids.
  // Assign ids based upon what is available.
  HandleCompiledSampledImages(internalData);

  // Fetch the resources again (since we modified them)
  resources = compiler.get_shader_resources();
  internalData.mResources = &resources;
  // Extract reflection data now that we've done all modifications
  ExtractResourceReflectionData(internalData);
  outputData.mReflectionData.mShaderTypeName = inputData.mReflectionData.mShaderTypeName;

  bool success = true;
#ifdef RaverieExceptions
  try
  {
#endif
    std::string source = compiler.compile();
    outputData.mByteStream.Load(source.c_str(), source.size());
#ifdef RaverieExceptions
  }
  catch (const std::exception& e)
  {
    success = false;
    mErrorLog = e.what();
  }
#endif

  return success;
}

String RaverieShaderGlslBackend::GetErrorLog()
{
  return mErrorLog;
}

void RaverieShaderGlslBackend::FixInterfaceBlockNames(GlslBackendInternalData& internalData)
{
  spirv_cross::CompilerGLSL* compiler = internalData.mCompiler;
  spirv_cross::ShaderResources* resources = internalData.mResources;

  // Name all input/output interface blocks the same (so they match).
  // Note: Vertex inputs and pixel outputs are not interface blocks.
  // Note: Also assuming there's only one block per stage (per attachment of
  // in/out) Note: There can actually be more than one block per stage if you
  // include built-ins (e.g. gl_Position). The only reasonable way it looks like
  // these can be identified is that they don't contain a decoration for
  // location.
  spv::ExecutionModel executionMode = compiler->get_execution_model();
  if (!resources->stage_inputs.empty() && executionMode != spv::ExecutionModelVertex)
  {
    for (size_t i = 0; i < resources->stage_inputs.size(); ++i)
    {
      spirv_cross::Resource& resource = resources->stage_inputs[i];
      if (compiler->has_decoration(resource.id, spv::DecorationLocation))
      {
        compiler->set_name(resource.base_type_id, "block");
        break;
      }
    }
  }
  if (!resources->stage_outputs.empty() && executionMode != spv::ExecutionModelFragment)
  {
    int id = resources->stage_outputs[0].base_type_id;
    for (size_t i = 0; i < resources->stage_outputs.size(); ++i)
    {
      spirv_cross::Resource& resource = resources->stage_outputs[i];
      if (compiler->has_decoration(resource.id, spv::DecorationLocation))
      {
        compiler->set_name(resource.base_type_id, "block");
        break;
      }
    }
  }
}

void RaverieShaderGlslBackend::FixUniformBufferNameoverlaps(GlslBackendInternalData& internalData)
{
  spirv_cross::CompilerGLSL* compiler = internalData.mCompiler;
  spirv_cross::ShaderResources* resources = internalData.mResources;

  spv::ExecutionModel executionMode = compiler->get_execution_model();
  FragmentType::Enum shaderStage = FragmentType::Vertex;
  if (executionMode == spv::ExecutionModelVertex)
    shaderStage = FragmentType::Vertex;
  else if (executionMode == spv::ExecutionModelGeometry)
    shaderStage = FragmentType::Geometry;
  else if (executionMode == spv::ExecutionModelFragment)
    shaderStage = FragmentType::Pixel;
  else if (executionMode == spv::ExecutionModelGLCompute)
    shaderStage = FragmentType::Compute;

  for (size_t i = 0; i < resources->uniform_buffers.size(); ++i)
  {
    auto&& uniformBuffer = resources->uniform_buffers[i];
    std::string bufferName = compiler->get_name(uniformBuffer.id);
    // It's a linker error in glsl (on some drivers) for multiple stages to have the same buffer
    // names with different definitions. As each stage is currently compiled independently, the best
    // we can do is to force the buffer named "Material" to be named differently. In particular, appending the stage
    // name.
    if (bufferName == "Material")
    {
      String materialBufferName = Raverie::BuildString(bufferName.c_str(), "_", Raverie::FragmentType::Names[shaderStage]);
      compiler->set_name(uniformBuffer.id, materialBufferName.c_str());
    }
  }
}

void RaverieShaderGlslBackend::FindUsedSampledImageBindings(GlslBackendInternalData& internalData)
{
  spirv_cross::CompilerGLSL* compiler = internalData.mCompiler;
  spirv_cross::ShaderResources* resources = internalData.mResources;

  // Get all used sampled image ids (so we can add ids to new sampled images)
  for (auto& resource : resources->sampled_images)
  {
    unsigned binding = compiler->get_decoration(resource.id, spv::DecorationBinding);
    internalData.mImageBindings.Insert(binding);
  }
  for (auto& resource : resources->separate_images)
  {
    unsigned binding = compiler->get_decoration(resource.id, spv::DecorationBinding);
    internalData.mImageBindings.Insert(binding);
  }
  for (auto& resource : resources->separate_samplers)
  {
    unsigned binding = compiler->get_decoration(resource.id, spv::DecorationBinding);
    internalData.mImageBindings.Insert(binding);
  }
}

void RaverieShaderGlslBackend::HandleCompiledSampledImages(GlslBackendInternalData& internalData)
{
  spirv_cross::CompilerGLSL* compiler = internalData.mCompiler;
  spirv_cross::ShaderResources* resources = internalData.mResources;
  ShaderStageInterfaceReflection& stageReflection = internalData.mOutputResult->mReflectionData;

  // Give the remapped combined samplers new names.
  int id = 0;
  for (auto& remap : compiler->get_combined_image_samplers())
  {
    // Make the full name of the sampled image from the image and sampler name
    std::string imageName = compiler->get_name(remap.image_id);
    std::string samplerName = compiler->get_name(remap.sampler_id);
    std::string fullName = imageName;
    fullName = "SPIRV_Cross_Combined" + imageName + samplerName;
    // Update the name in the glsl compiler
    compiler->set_name(remap.combined_id, fullName);

    // Add mappings for the image and sampler saying that they
    // both mapped to this new sampled image
    String fullNameStr = fullName.c_str();
    stageReflection.mImageRemappings[imageName.c_str()].mSampledImageRemappings.PushBack(fullNameStr);
    stageReflection.mSamplerRemappings[samplerName.c_str()].mSampledImageRemappings.PushBack(fullNameStr);

    // Mark this as a generated sampled image so we can make
    // sure to add this to the final reflection data.
    internalData.mGeneratedSampledImage.Insert(fullNameStr);

    // Generated sampled images don't have binding ids. Find the first
    // un-used id and assign that to this new sampled image
    while (internalData.mImageBindings.Contains(id))
      ++id;
    internalData.mImageBindings.Insert(id);
    compiler->set_decoration(remap.combined_id, spv::DecorationBinding, id);
  }
}

// Any type that isn't a struct doesn't have a name returned from spirv_cross.
// To generate this name (mostly for debug) we have to make a large conditional
// table to map this to engine's names.
void RaverieShaderGlslBackend::PopulateTypeName(GlslBackendInternalData& internalData, spirv_cross::SPIRType& spirVType, ShaderResourceReflectionData& reflectionData)
{
  spirv_cross::CompilerGLSL* compiler = internalData.mCompiler;

  // Image types are parameterized types. We have to walk all of
  // the sub-parameters to get the final raverie name.
  if (spirVType.basetype == spirv_cross::SPIRType::Image)
  {
    if (spirVType.image.depth)
    {
      if (spirVType.image.dim == spv::Dim2D)
        reflectionData.mTypeName = RaverieTypeId(Raverie::DepthImage2d)->ToString();
    }
    else
    {
      if (spirVType.image.dim == spv::Dim2D)
        reflectionData.mTypeName = RaverieTypeId(Raverie::Image2d)->ToString();
      else if (spirVType.image.dim == spv::DimCube)
        reflectionData.mTypeName = RaverieTypeId(Raverie::ImageCube)->ToString();
    }
  }
  // Sampled images are also parameterized.
  else if (spirVType.basetype == spirv_cross::SPIRType::SampledImage)
  {
    if (spirVType.image.depth)
    {
      if (spirVType.image.dim == spv::Dim2D)
        reflectionData.mTypeName = RaverieTypeId(Raverie::SampledDepthImage2d)->ToString();
    }
    else
    {
      if (spirVType.image.dim == spv::Dim2D)
        reflectionData.mTypeName = RaverieTypeId(Raverie::SampledImage2d)->ToString();
      else if (spirVType.image.dim == spv::DimCube)
        reflectionData.mTypeName = RaverieTypeId(Raverie::SampledImageCube)->ToString();
    }
  }
  else if (spirVType.basetype == spirv_cross::SPIRType::Sampler)
  {
    reflectionData.mTypeName = RaverieTypeId(Raverie::Sampler)->ToString();
  }
  // Otherwise, assume this is either a primitive type or a struct
  else
  {
    String dimensionStr;
    // Get the base type.
    if (spirVType.basetype == spirv_cross::SPIRType::Float)
      reflectionData.mTypeName = "Real";
    else if (spirVType.basetype == spirv_cross::SPIRType::Int)
      reflectionData.mTypeName = "Integer";
    else if (spirVType.basetype == spirv_cross::SPIRType::Boolean)
      reflectionData.mTypeName = "Boolean";
    else
      reflectionData.mTypeName = compiler->get_name(spirVType.self).c_str();

    // Append dimensionality
    if (spirVType.columns > 1)
      dimensionStr = String::Format("%dx%d", spirVType.columns, spirVType.vecsize);
    else if (spirVType.vecsize > 1)
      dimensionStr = ToString(spirVType.vecsize);

    if (!dimensionStr.Empty())
      reflectionData.mTypeName = BuildString(reflectionData.mTypeName, dimensionStr);
  }
  // @JoshD: Deal with arrays later?
}

void RaverieShaderGlslBackend::PopulateMemberTypeInfo(
    GlslBackendInternalData& internalData, spirv_cross::SPIRType& parentType, int memberIndex, ShaderResourceReflectionData& reflectionData, bool isInterfaceType)
{
  spirv_cross::CompilerGLSL* compiler = internalData.mCompiler;

  spirv_cross::SPIRType memberType = compiler->get_type(parentType.member_types[memberIndex]);
  PopulateTypeName(internalData, memberType, reflectionData);
  reflectionData.mInstanceName = compiler->get_member_name(parentType.self, memberIndex).c_str();
  reflectionData.mSizeInBytes = compiler->get_declared_struct_member_size(parentType, memberIndex);

  reflectionData.mStride = 0;
  // Get array stride
  if (!memberType.array.empty())
    reflectionData.mStride = compiler->type_struct_member_array_stride(parentType, memberIndex);
  // Get matrix stride
  if (memberType.columns > 1)
    reflectionData.mStride = compiler->type_struct_member_matrix_stride(parentType, memberIndex);

  // If this is an interface type then the call to type_struct_member_offset
  // will throw an exception. Also interface types don't actually have valid
  // offsets to be set as they're between stages and can't be interacted with
  // from the gpu (except vertex attributes which can't be structs)
  if (isInterfaceType)
    reflectionData.mOffsetInBytes = 0;
  else
    reflectionData.mOffsetInBytes = compiler->type_struct_member_offset(parentType, memberIndex);
}

void RaverieShaderGlslBackend::PopulateTypeInfo(GlslBackendInternalData& internalData, spirv_cross::SPIRType& spirvType, ShaderResourceReflectionData& reflectionData, bool isInterfaceType)
{
  spirv_cross::CompilerGLSL* compiler = internalData.mCompiler;
  PopulateTypeName(internalData, spirvType, reflectionData);

  // If the type is an interface type then the call to get_declared_struct_size
  // will throw an exception. Instead, approximate the size as knowing the base
  // type's size and whether this is a matrix or vector
  if (isInterfaceType)
  {
    size_t baseTypeByteSize = spirvType.width / 8;
    reflectionData.mSizeInBytes = spirvType.columns * spirvType.vecsize * baseTypeByteSize;
  }
  else
    reflectionData.mSizeInBytes = compiler->get_declared_struct_size(spirvType);
}

void RaverieShaderGlslBackend::ExtractResourceReflection(GlslBackendInternalData& internalData, spirv_cross::Resource& resource, Array<ShaderStageResource>& results, bool isInterfaceType)
{
  spirv_cross::CompilerGLSL* compiler = internalData.mCompiler;
  spirv_cross::ShaderResources* resources = internalData.mResources;
  String instanceName = compiler->get_name(resource.id).c_str();

  // Find the reflection data by name instance name
  int index = results.Size();
  for (size_t i = 0; i < results.Size(); ++i)
  {
    if (results[i].mReflectionData.mInstanceName == instanceName)
    {
      index = i;
      break;
    }
  }

  // If it doesn't exist then create a new reflection object
  if (index == results.Size())
  {
    ShaderStageResource& newResource = results.PushBack();
    spirv_cross::SPIRType bufferType = compiler->get_type(resource.type_id);
    newResource.mMembers.Resize(bufferType.member_types.size());
  }

  spirv_cross::SPIRType spirvType = compiler->get_type(resource.type_id);
  ShaderStageResource& stageResource = results[index];
  ShaderResourceReflectionData& reflectionData = stageResource.mReflectionData;

  // Populate top-level type information (size, name, etc...)
  reflectionData.mInstanceName = compiler->get_name(resource.id).c_str();
  PopulateTypeInfo(internalData, spirvType, reflectionData, isInterfaceType);
  // Grab decorations
  reflectionData.mBinding = compiler->get_decoration(resource.id, spv::DecorationBinding);
  reflectionData.mLocation = compiler->get_decoration(resource.id, spv::DecorationLocation);
  reflectionData.mDescriptorSet = compiler->get_decoration(resource.id, spv::DecorationDescriptorSet);

  // Walk all members
  for (size_t i = 0; i < spirvType.member_types.size(); ++i)
  {
    ShaderResourceReflectionData& memberReflection = stageResource.mMembers[i];
    // Populate the member reflection data (offset, size, etc...)
    PopulateMemberTypeInfo(internalData, spirvType, i, memberReflection, isInterfaceType);
    // Add a string to member index mapping
    stageResource.mLookupMap[memberReflection.mInstanceName] = i;
  }
}

template <typename VectorType>
void RaverieShaderGlslBackend::ExtractResourcesReflection(GlslBackendInternalData& internalData, VectorType& resources, Array<ShaderStageResource>& results, bool isInterfaceType)
{
  // Extract each resource independently
  for (size_t i = 0; i < resources.size(); ++i)
    ExtractResourceReflection(internalData, resources[i], results, isInterfaceType);
}

void RaverieShaderGlslBackend::ExtractResourceReflectionData(GlslBackendInternalData& internalData)
{
  ShaderStageInterfaceReflection& stageReflection = internalData.mOutputResult->mReflectionData;
  spirv_cross::ShaderResources* resources = internalData.mResources;

  // Uniform buffers and sampled images must be extracted since the client must
  // actually hook up bindings
  ExtractResourcesReflection(internalData, resources->uniform_buffers, stageReflection.mUniforms, false);
  ExtractResourcesReflection(internalData, resources->sampled_images, stageReflection.mSampledImages, true);
  // Inputs/outputs aren't really necessary (potentially vertex attributes are)
  // but they're also extracted for completeness. Some data on these can't fully
  // be extracted since these types aren't backed in the same way by physical
  // memory.
  ExtractResourcesReflection(internalData, resources->stage_inputs, stageReflection.mStageInputs, true);
  ExtractResourcesReflection(internalData, resources->stage_outputs, stageReflection.mStageOutputs, true);

  // We already dealt with the Image/Sampler -> SampledImage remappings when
  // extracting resource info. We didn't however deal with non-remapped sampled
  // images. To do this simply walk over all un-visited sampled images and mark
  // that they mapped to themselves.
  for (size_t i = 0; i < stageReflection.mSampledImages.Size(); ++i)
  {
    String name = stageReflection.mSampledImages[i].mReflectionData.mInstanceName;
    if (internalData.mGeneratedSampledImage.Contains(name))
      continue;
    stageReflection.mSampledImageRemappings[name].mSampledImageRemappings.PushBack(name);
    internalData.mGeneratedSampledImage.Insert(name);
  }
  ExtractResourcesReflection(internalData, resources->storage_buffers, stageReflection.mStructedStorageBuffers, false);
  ExtractResourcesReflection(internalData, resources->storage_images, stageReflection.mStorageImages, true);
}

} // namespace Raverie
