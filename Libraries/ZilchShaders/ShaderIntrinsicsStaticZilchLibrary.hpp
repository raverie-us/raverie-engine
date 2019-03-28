// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

/// The zilch shader library wrapper around Zilch's ShaderIntrinsics library.
/// This needs to be built and have the Parse function called once before all
/// shader translation. Contains the image/sampler types.
class ShaderIntrinsicsStaticZilchLibrary
{
public:
  static void InitializeInstance();
  static void Destroy();
  static ShaderIntrinsicsStaticZilchLibrary& GetInstance();

  ShaderIntrinsicsStaticZilchLibrary();
  /// Parse the ShaderIntrinsics library and make all backing shader types.
  void Parse(ZilchSpirVFrontEnd* translator);
  ZilchShaderIRLibraryRef GetLibrary();

private:
  void CreateImageAndSampler(ZilchSpirVFrontEnd* translator,
                             ZilchShaderIRLibrary* shaderLibrary,
                             Zilch::BoundType* zilchSampledType,
                             Zilch::BoundType* zilchImageType,
                             Zilch::BoundType* zilchSampledImageType,
                             int dimension,
                             int depthMode);
  void CreateStorageImage(ZilchSpirVFrontEnd* translator,
                          ZilchShaderIRLibrary* shaderLibrary,
                          Zilch::BoundType* zilchSampledType,
                          Zilch::BoundType* zilchImageType,
                          int dimension,
                          int depthMode,
                          int imageFormat);

  void PopulateStageRequirementsData(ZilchShaderIRLibrary* shaderLibrary);

  ZilchShaderIRLibraryRef mLibraryRef;
  static ShaderIntrinsicsStaticZilchLibrary* mInstance;
};

} // namespace Zero
