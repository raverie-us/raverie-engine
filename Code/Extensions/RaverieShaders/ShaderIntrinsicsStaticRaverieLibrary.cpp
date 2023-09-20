// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "RaverieShaderIRCore.hpp"
#include "ShaderIRLibraryTranslation.hpp"
#include "RaverieShadersStandard.hpp"

namespace Raverie
{

ShaderIntrinsicsStaticRaverieLibrary* ShaderIntrinsicsStaticRaverieLibrary::mInstance = nullptr;

void ShaderIntrinsicsStaticRaverieLibrary::InitializeInstance()
{
  ReturnIf(mInstance != nullptr, , "Can't initialize a static library more than once");
  mInstance = new ShaderIntrinsicsStaticRaverieLibrary();
}

void ShaderIntrinsicsStaticRaverieLibrary::Destroy()
{
  delete mInstance;
  mInstance = nullptr;
}

ShaderIntrinsicsStaticRaverieLibrary& ShaderIntrinsicsStaticRaverieLibrary::GetInstance()
{
  ErrorIf(mInstance == nullptr, "Attempted to get an uninitialized singleton static library");

  return *mInstance;
}

ShaderIntrinsicsStaticRaverieLibrary::ShaderIntrinsicsStaticRaverieLibrary()
{
}

void ShaderIntrinsicsStaticRaverieLibrary::Parse(RaverieSpirVFrontEnd* translator)
{
  RaverieShaderIRLibrary* shaderLibrary = new RaverieShaderIRLibrary();
  shaderLibrary->mRaverieLibrary = Raverie::ShaderIntrinsicsLibrary::GetInstance().GetLibrary();
  shaderLibrary->mDependencies = new RaverieShaderIRModule();
  shaderLibrary->mDependencies->PushBack(RaverieShaderIRCore::GetInstance().GetLibrary());
  mLibraryRef = shaderLibrary;
  translator->mLibrary = shaderLibrary;

  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::Library* raverieLibrary = shaderLibrary->mRaverieLibrary;

  // Declare the unsigned int type. As this is currently a hack type, only do this for the scalar version. @JoshD:
  // Cleanup
  RaverieShaderIRType* uintType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Uint, 1, nullptr, RaverieTypeId(Raverie::UnsignedInt));

  // Grabbed a bunch of raverie types
  Raverie::BoundType* raverieSamplerType = RaverieTypeId(Raverie::Sampler);
  Raverie::BoundType* raverieImage2d = RaverieTypeId(Raverie::Image2d);
  Raverie::BoundType* raverieDepthImage2d = RaverieTypeId(Raverie::DepthImage2d);
  Raverie::BoundType* raverieImageCube = RaverieTypeId(Raverie::ImageCube);
  Raverie::BoundType* raverieSampledImage2d = RaverieTypeId(Raverie::SampledImage2d);
  Raverie::BoundType* raverieSampledDepthImage2d = RaverieTypeId(Raverie::SampledDepthImage2d);
  Raverie::BoundType* raverieSampledImageCube = RaverieTypeId(Raverie::SampledImageCube);
  Raverie::BoundType* raverieStorageImage2d = RaverieTypeId(Raverie::StorageImage2d);

  // Create the sampler type
  RaverieShaderIRType* samplerType = translator->MakeTypeAndPointer(shaderLibrary, ShaderIRTypeBaseType::Sampler, raverieSamplerType->Name, raverieSamplerType, spv::StorageClassUniformConstant);
  translator->MakeShaderTypeMeta(samplerType, nullptr);

  // Create images + sampledImaged types
  CreateImageAndSampler(translator, shaderLibrary, core.RealType, raverieImage2d, raverieSampledImage2d, spv::Dim2D, 0);
  CreateImageAndSampler(translator, shaderLibrary, core.RealType, raverieDepthImage2d, raverieSampledDepthImage2d, spv::Dim2D, 1);
  CreateImageAndSampler(translator, shaderLibrary, core.RealType, raverieImageCube, raverieSampledImageCube, spv::DimCube, 0);
  CreateStorageImage(translator, shaderLibrary, core.RealType, raverieStorageImage2d, spv::Dim2D, 0, spv::ImageFormatRgba32f);

  RegisterShaderIntrinsics(translator, shaderLibrary);

  // Register the template resolver for fixed array
  shaderLibrary->RegisterTemplateResolver("FixedArray[Type,Integer]", FixedArrayResolver);
  FixedArrayResolver(translator, shaderLibrary->mRaverieLibrary->BoundTypes["FixedArray[Real4x4, 80]"]);
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

RaverieShaderIRLibraryRef ShaderIntrinsicsStaticRaverieLibrary::GetLibrary()
{
  return mLibraryRef;
}

void ShaderIntrinsicsStaticRaverieLibrary::CreateImageAndSampler(RaverieSpirVFrontEnd* translator,
                                                                 RaverieShaderIRLibrary* shaderLibrary,
                                                                 Raverie::BoundType* raverieSampledType,
                                                                 Raverie::BoundType* raverieImageType,
                                                                 Raverie::BoundType* raverieSampledImageType,
                                                                 int dimension,
                                                                 int depthMode)
{
  RaverieShaderIRType* sampledType = shaderLibrary->FindType(raverieSampledType);
  // Create the base image type for a sampled image
  RaverieShaderIRType* imageType = translator->MakeTypeAndPointer(shaderLibrary, ShaderIRTypeBaseType::Image, raverieImageType->Name, raverieImageType, spv::StorageClassUniformConstant);
  imageType->mParameters.PushBack(sampledType);                                              // SampledType
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(dimension)); // Dim
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(depthMode)); // Depth
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // Arrayed
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // MultiSampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(1));         // Sampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(spv::ImageFormatUnknown));
  translator->MakeShaderTypeMeta(imageType, nullptr);

  RaverieShaderIRType* sampledImageType =
      translator->MakeTypeAndPointer(shaderLibrary, ShaderIRTypeBaseType::SampledImage, raverieSampledImageType->Name, raverieSampledImageType, spv::StorageClassUniformConstant);
  sampledImageType->mParameters.PushBack(imageType);
  translator->MakeShaderTypeMeta(sampledImageType, nullptr);
}

void ShaderIntrinsicsStaticRaverieLibrary::CreateStorageImage(RaverieSpirVFrontEnd* translator,
                                                              RaverieShaderIRLibrary* shaderLibrary,
                                                              Raverie::BoundType* raverieSampledType,
                                                              Raverie::BoundType* raverieImageType,
                                                              int dimension,
                                                              int depthMode,
                                                              int imageFormat)
{
  RaverieShaderIRType* sampledType = shaderLibrary->FindType(raverieSampledType);
  // Create the base image type for a sampled image
  RaverieShaderIRType* imageType = translator->MakeTypeAndPointer(shaderLibrary, ShaderIRTypeBaseType::Image, raverieImageType->Name, raverieImageType, spv::StorageClassUniformConstant);
  imageType->mParameters.PushBack(sampledType);                                              // SampledType
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(dimension)); // Dim
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(depthMode)); // Depth
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // Arrayed
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // MultiSampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(2));         // Sampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(imageFormat));
  translator->MakeShaderTypeMeta(imageType, nullptr);
}

void ShaderIntrinsicsStaticRaverieLibrary::PopulateStageRequirementsData(RaverieShaderIRLibrary* shaderLibrary)
{
  // Find all raverie functions that have the [RequiresPixel] attribute.
  // These need to be processed into the stage requirements cached in the
  // library/
  Raverie::Library* raverieLibrary = shaderLibrary->mRaverieLibrary;
  for (size_t i = 0; i < raverieLibrary->OwnedFunctions.Size(); ++i)
  {
    Raverie::Function* raverieFunction = raverieLibrary->OwnedFunctions[i];
    if (raverieFunction->HasAttribute(SpirVNameSettings::mRequiresPixelAttribute))
      shaderLibrary->mStageRequirementsData[raverieFunction].Combine(nullptr, raverieFunction->Location, ShaderStage::Pixel);
  }
}

} // namespace Raverie
