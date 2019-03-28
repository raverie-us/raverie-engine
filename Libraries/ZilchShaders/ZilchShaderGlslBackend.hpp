// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

// Forward declarations needed for the backend's internal data
namespace spirv_cross
{
class CompilerGLSL;
struct ShaderResources;
struct SPIRType;
struct Resource;
} // namespace spirv_cross

namespace Zero
{

/// Helper data used by the glsl backend to pass around a bunch of common data.
struct GlslBackendInternalData
{
  GlslBackendInternalData();

  HashSet<int> mImageBindings;
  HashSet<String> mGeneratedSampledImage;
  spirv_cross::CompilerGLSL* mCompiler;
  spirv_cross::ShaderResources* mResources;
  ShaderTranslationPassResult* mOutputResult;
};

/// Backend that converts the input data to glsl shader code. The output data's
/// byte stream will be string data.
class ZilchShaderGlslBackend : public ZilchShaderIRBackend
{
public:
  ZilchShaderGlslBackend();

  String GetExtension() override;
  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;
  String GetErrorLog() override;

protected:
  // Helper functions that allow inherited classes to re-use a ton of code.
  void FixInterfaceBlockNames(GlslBackendInternalData& internalData);
  void FindUsedSampledImageBindings(GlslBackendInternalData& internalData);
  void HandleCompiledSampledImages(GlslBackendInternalData& internalData);
  void PopulateTypeName(GlslBackendInternalData& internalData,
                        spirv_cross::SPIRType& spirVType,
                        ShaderResourceReflectionData& reflectionData);
  void PopulateMemberTypeInfo(GlslBackendInternalData& internalData,
                              spirv_cross::SPIRType& parentType,
                              int memberIndex,
                              ShaderResourceReflectionData& reflectionData,
                              bool isInterfaceType);
  void PopulateTypeInfo(GlslBackendInternalData& internalData,
                        spirv_cross::SPIRType& spirvType,
                        ShaderResourceReflectionData& reflectionData,
                        bool isInterfaceType);
  void ExtractResourceReflection(GlslBackendInternalData& internalData,
                                 spirv_cross::Resource& resource,
                                 Array<ShaderStageResource>& results,
                                 bool isInterfaceType);
  void ExtractResourcesReflection(GlslBackendInternalData& internalData,
                                  std::vector<spirv_cross::Resource>& resources,
                                  Array<ShaderStageResource>& results,
                                  bool isInterfaceType);
  void ExtractResourceReflectionData(GlslBackendInternalData& internalData);

public:
  int mTargetVersion;
  bool mTargetGlslEs;
  String mErrorLog;
};

} // namespace Zero
