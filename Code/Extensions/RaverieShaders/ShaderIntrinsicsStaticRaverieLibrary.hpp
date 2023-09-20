// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// The raverie shader library wrapper around Raverie's ShaderIntrinsics library.
/// This needs to be built and have the Parse function called once before all
/// shader translation. Contains the image/sampler types.
class ShaderIntrinsicsStaticRaverieLibrary
{
public:
  static void InitializeInstance();
  static void Destroy();
  static ShaderIntrinsicsStaticRaverieLibrary& GetInstance();

  ShaderIntrinsicsStaticRaverieLibrary();
  /// Parse the ShaderIntrinsics library and make all backing shader types.
  void Parse(RaverieSpirVFrontEnd* translator);
  RaverieShaderIRLibraryRef GetLibrary();

private:
  void CreateImageAndSampler(RaverieSpirVFrontEnd* translator,
                             RaverieShaderIRLibrary* shaderLibrary,
                             Raverie::BoundType* raverieSampledType,
                             Raverie::BoundType* raverieImageType,
                             Raverie::BoundType* raverieSampledImageType,
                             int dimension,
                             int depthMode);
  void CreateStorageImage(RaverieSpirVFrontEnd* translator,
                          RaverieShaderIRLibrary* shaderLibrary,
                          Raverie::BoundType* raverieSampledType,
                          Raverie::BoundType* raverieImageType,
                          int dimension,
                          int depthMode,
                          int imageFormat);

  void PopulateStageRequirementsData(RaverieShaderIRLibrary* shaderLibrary);

  RaverieShaderIRLibraryRef mLibraryRef;
  static ShaderIntrinsicsStaticRaverieLibrary* mInstance;
};

} // namespace Raverie
