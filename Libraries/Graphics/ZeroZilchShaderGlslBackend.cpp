// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "spirv_glsl.hpp"

namespace Zero
{

ZeroZilchShaderGlslBackend::ZeroZilchShaderGlslBackend()
{
  mTargetVersion = 150;
  mTargetGlslEs = false;
}

String ZeroZilchShaderGlslBackend::GetExtension()
{
  return "glsl";
}

bool ZeroZilchShaderGlslBackend::RunTranslationPass(ShaderTranslationPassResult& inputData,
                                                    ShaderTranslationPassResult& outputData)
{
  ShaderByteStream& inputByteStream = inputData.mByteStream;
  uint32_t* data = (uint32_t*)inputByteStream.Data();
  uint32 wordCount = inputByteStream.WordCount();

  spirv_cross::CompilerGLSL compiler(data, wordCount);
  // Set options
  spirv_cross::CompilerGLSL::Options opts = compiler.get_common_options();
  //opts.force_legacy = true; // welder specific
  opts.emit_uniform_buffer_as_plain_uniforms = true; // replaces welder specific line above after spirv-cross update, welder specific patch not needed anymore
  opts.version = mTargetVersion;
  opts.es = mTargetGlslEs;
  compiler.set_common_options(opts);

  spirv_cross::ShaderResources resources = compiler.get_shader_resources();

  // Forcing buffer typenames to match between different stages so that the
  // uniform instance names can be the same.
  for (auto& ubo : resources.uniform_buffers)
  {
    int id = compiler.get_decoration(ubo.id, spv::DecorationBinding);
    String name = BuildString("Buffer", ToString(id));

    compiler.set_name(ubo.base_type_id, name.c_str());
  }

#ifdef WelderTargetOsEmscripten
  // gles output is going to flatten input/output blocks and prepend the block
  // name to each member. Forcing block typenames to match.
  for (auto stageInput : resources.stage_inputs)
  {
    int isBlock = compiler.get_decoration(stageInput.base_type_id, spv::DecorationBlock);
    if (isBlock == 1)
    {
      std::string name = "block";
      compiler.set_name(stageInput.id, name);
    }
  }
  for (auto stageOutput : resources.stage_outputs)
  {
    int isBlock = compiler.get_decoration(stageOutput.base_type_id, spv::DecorationBlock);
    if (isBlock == 1)
    {
      std::string name = "block";
      compiler.set_name(stageOutput.id, name);
    }
  }

  // When target outputs get flattened the layout decorations do not get copied.
  // gles requires the layout decorations when there are multiple output
  // targets.
  auto entryPoints = compiler.get_entry_points_and_stages();
  auto entryPoint = entryPoints[0];
  if (entryPoint.execution_model == spv::ExecutionModel::ExecutionModelFragment)
  {
    for (auto stageOutput : resources.stage_outputs)
    {
      auto resourceType = compiler.get_type(stageOutput.base_type_id);
      for (size_t i = 0; i < resourceType.member_types.size(); ++i)
        compiler.set_member_decoration(stageOutput.base_type_id, i, spv::DecorationLocation, i);
    }
  }
#endif

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
  FixUniformBufferNameoverlaps(internalData);

  // Fetch the resources again (since we modified them)
  resources = compiler.get_shader_resources();
  internalData.mResources = &resources;
  // Extract reflection data now that we've done all modifications
  ExtractResourceReflectionData(internalData);
  outputData.mReflectionData.mShaderTypeName = inputData.mReflectionData.mShaderTypeName;

  bool success = true;
  try
  {
    std::string source = compiler.compile();
    outputData.mByteStream.Load(source.c_str(), source.size());
  }
  catch (const std::exception& e)
  {
    success = false;
    mErrorLog = e.what();
  }

  return success;
}

String ZeroZilchShaderGlslBackend::GetErrorLog()
{
  return mErrorLog;
}

} // namespace Zero
