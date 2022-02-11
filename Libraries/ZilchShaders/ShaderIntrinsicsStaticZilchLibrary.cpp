// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "ZilchShaderIRCore.hpp"
#include "ShaderIRLibraryTranslation.hpp"
#include "ZilchShadersStandard.hpp"

namespace Zero
{

ShaderIntrinsicsStaticZilchLibrary* ShaderIntrinsicsStaticZilchLibrary::mInstance = nullptr;

void ShaderIntrinsicsStaticZilchLibrary::InitializeInstance()
{
  ReturnIf(mInstance != nullptr, , "Can't initialize a static library more than once");
  mInstance = new ShaderIntrinsicsStaticZilchLibrary();
}

void ShaderIntrinsicsStaticZilchLibrary::Destroy()
{
  delete mInstance;
  mInstance = nullptr;
}

ShaderIntrinsicsStaticZilchLibrary& ShaderIntrinsicsStaticZilchLibrary::GetInstance()
{
  ErrorIf(mInstance == nullptr, "Attempted to get an uninitialized singleton static library");

  return *mInstance;
}

ShaderIntrinsicsStaticZilchLibrary::ShaderIntrinsicsStaticZilchLibrary()
{
}

void ShaderIntrinsicsStaticZilchLibrary::Parse(ZilchSpirVFrontEnd* translator)
{
  ZilchShaderIRLibrary* shaderLibrary = new ZilchShaderIRLibrary();
  shaderLibrary->mZilchLibrary = Zilch::ShaderIntrinsicsLibrary::GetInstance().GetLibrary();
  shaderLibrary->mDependencies = new ZilchShaderIRModule();
  shaderLibrary->mDependencies->PushBack(ZilchShaderIRCore::GetInstance().GetLibrary());
  mLibraryRef = shaderLibrary;
  translator->mLibrary = shaderLibrary;

  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::Library* zilchLibrary = shaderLibrary->mZilchLibrary;

  // Declare the unsigned int type. As this is currently a hack type, only do this for the scalar version. @JoshD:
  // Cleanup
  ZilchShaderIRType* uintType =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Uint, 1, nullptr, ZilchTypeId(Zilch::UnsignedInt));

  // Grabbed a bunch of zilch types
  Zilch::BoundType* zilchSamplerType = ZilchTypeId(Zilch::Sampler);
  Zilch::BoundType* zilchImage2d = ZilchTypeId(Zilch::Image2d);
  Zilch::BoundType* zilchDepthImage2d = ZilchTypeId(Zilch::DepthImage2d);
  Zilch::BoundType* zilchImageCube = ZilchTypeId(Zilch::ImageCube);
  Zilch::BoundType* zilchSampledImage2d = ZilchTypeId(Zilch::SampledImage2d);
  Zilch::BoundType* zilchSampledDepthImage2d = ZilchTypeId(Zilch::SampledDepthImage2d);
  Zilch::BoundType* zilchSampledImageCube = ZilchTypeId(Zilch::SampledImageCube);
  Zilch::BoundType* zilchStorageImage2d = ZilchTypeId(Zilch::StorageImage2d);

  // Create the sampler type
  ZilchShaderIRType* samplerType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                  ShaderIRTypeBaseType::Sampler,
                                                                  zilchSamplerType->Name,
                                                                  zilchSamplerType,
                                                                  spv::StorageClassUniformConstant);
  translator->MakeShaderTypeMeta(samplerType, nullptr);

  // Create images + sampledImaged types
  CreateImageAndSampler(translator, shaderLibrary, core.RealType, zilchImage2d, zilchSampledImage2d, spv::Dim2D, 0);
  CreateImageAndSampler(
      translator, shaderLibrary, core.RealType, zilchDepthImage2d, zilchSampledDepthImage2d, spv::Dim2D, 1);
  CreateImageAndSampler(
      translator, shaderLibrary, core.RealType, zilchImageCube, zilchSampledImageCube, spv::DimCube, 0);
  CreateStorageImage(
      translator, shaderLibrary, core.RealType, zilchStorageImage2d, spv::Dim2D, 0, spv::ImageFormatRgba32f);

  RegisterShaderIntrinsics(translator, shaderLibrary);

  // Register the template resolver for fixed array
  shaderLibrary->RegisterTemplateResolver("FixedArray[Type,Integer]", FixedArrayResolver);
  String runtimeArayResolverName = BuildString(SpirVNameSettings::mRuntimeArrayTypeName, "[Type]");
  shaderLibrary->RegisterTemplateResolver(runtimeArayResolverName, RuntimeArrayResolver);

  shaderLibrary->RegisterTemplateResolver("PointInput[Type]", GeometryStreamInputResolver);
  shaderLibrary->RegisterTemplateResolver("LineInput[Type]", GeometryStreamInputResolver);
  shaderLibrary->RegisterTemplateResolver("TriangleInput[Type]", GeometryStreamInputResolver);
  shaderLibrary->RegisterTemplateResolver("PointOutput[Type]", GeometryStreamOutputResolver);
  shaderLibrary->RegisterTemplateResolver("LineOutput[Type]", GeometryStreamOutputResolver);
  shaderLibrary->RegisterTemplateResolver("TriangleOutput[Type]", GeometryStreamOutputResolver);
  shaderLibrary->mTranslated = true;

  PopulateStageRequirementsData(shaderLibrary);
}

ZilchShaderIRLibraryRef ShaderIntrinsicsStaticZilchLibrary::GetLibrary()
{
  return mLibraryRef;
}

void ShaderIntrinsicsStaticZilchLibrary::CreateImageAndSampler(ZilchSpirVFrontEnd* translator,
                                                               ZilchShaderIRLibrary* shaderLibrary,
                                                               Zilch::BoundType* zilchSampledType,
                                                               Zilch::BoundType* zilchImageType,
                                                               Zilch::BoundType* zilchSampledImageType,
                                                               int dimension,
                                                               int depthMode)
{
  ZilchShaderIRType* sampledType = shaderLibrary->FindType(zilchSampledType);
  // Create the base image type for a sampled image
  ZilchShaderIRType* imageType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                ShaderIRTypeBaseType::Image,
                                                                zilchImageType->Name,
                                                                zilchImageType,
                                                                spv::StorageClassUniformConstant);
  imageType->mParameters.PushBack(sampledType);                                              // SampledType
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(dimension)); // Dim
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(depthMode)); // Depth
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // Arrayed
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // MultiSampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(1));         // Sampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(spv::ImageFormatUnknown));
  translator->MakeShaderTypeMeta(imageType, nullptr);

  ZilchShaderIRType* sampledImageType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                       ShaderIRTypeBaseType::SampledImage,
                                                                       zilchSampledImageType->Name,
                                                                       zilchSampledImageType,
                                                                       spv::StorageClassUniformConstant);
  sampledImageType->mParameters.PushBack(imageType);
  translator->MakeShaderTypeMeta(sampledImageType, nullptr);
}

void ShaderIntrinsicsStaticZilchLibrary::CreateStorageImage(ZilchSpirVFrontEnd* translator,
                                                            ZilchShaderIRLibrary* shaderLibrary,
                                                            Zilch::BoundType* zilchSampledType,
                                                            Zilch::BoundType* zilchImageType,
                                                            int dimension,
                                                            int depthMode,
                                                            int imageFormat)
{
  ZilchShaderIRType* sampledType = shaderLibrary->FindType(zilchSampledType);
  // Create the base image type for a sampled image
  ZilchShaderIRType* imageType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                ShaderIRTypeBaseType::Image,
                                                                zilchImageType->Name,
                                                                zilchImageType,
                                                                spv::StorageClassUniformConstant);
  imageType->mParameters.PushBack(sampledType);                                              // SampledType
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(dimension)); // Dim
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(depthMode)); // Depth
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // Arrayed
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // MultiSampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(2));         // Sampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(imageFormat));
  translator->MakeShaderTypeMeta(imageType, nullptr);
}

void ShaderIntrinsicsStaticZilchLibrary::PopulateStageRequirementsData(ZilchShaderIRLibrary* shaderLibrary)
{
  // Find all zilch functions that have the [RequiresPixel] attribute.
  // These need to be processed into the stage requirements cached in the
  // library/
  Zilch::Library* zilchLibrary = shaderLibrary->mZilchLibrary;
  for (size_t i = 0; i < zilchLibrary->OwnedFunctions.Size(); ++i)
  {
    Zilch::Function* zilchFunction = zilchLibrary->OwnedFunctions[i];
    if (zilchFunction->HasAttribute(SpirVNameSettings::mRequiresPixelAttribute))
      shaderLibrary->mStageRequirementsData[zilchFunction].Combine(
          nullptr, zilchFunction->Location, ShaderStage::Pixel);
  }
}

} // namespace Zero
