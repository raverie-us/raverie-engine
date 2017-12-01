///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderIntrinsics
// The zilch shader wrapper around Zilch's ShaderIntrinsics library. Contains the image/sampler types.
class ZilchShaderIntrinsics
{
public:
  static void InitializeInstance();
  static void Destroy();
  static ZilchShaderIntrinsics& GetInstance();

  ZilchShaderIntrinsics();
  // Parse the ShaderIntrinsics library and make all backing shader types.
  void Parse(ZilchSpirVFrontEnd* translator);
  ZilchShaderIRLibraryRef GetLibrary();

private:
  void CreateImageAndSampler(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary,
    Zilch::BoundType* zilchSampledType, Zilch::BoundType* zilchImageType, Zilch::BoundType* zilchSampledImageType,
    int dimension, int depthMode);

  void PopulateStageRequirementsData(ZilchShaderIRLibrary* shaderLibrary);

  ZilchShaderIRLibraryRef mLibraryRef;
  static ZilchShaderIntrinsics* mInstance;
};

}//namespace Zero
