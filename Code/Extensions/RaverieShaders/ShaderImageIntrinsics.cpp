// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

using namespace Raverie;

RaverieDefineType(Sampler, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

RaverieDefineType(Image2d, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

RaverieDefineType(StorageImage2d, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

RaverieDefineType(DepthImage2d, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

RaverieDefineType(ImageCube, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

RaverieDefineType(SampledImage2d, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

RaverieDefineType(SampledDepthImage2d, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

RaverieDefineType(SampledImageCube, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

} // namespace Raverie

namespace Raverie
{

using namespace Raverie;

ParameterArray FourParameters(Type* type1, Type* type2, Type* type3, Type* type4)
{
  ParameterArray parameters;
  parameters.PushBack(type1);
  parameters.PushBack(type2);
  parameters.PushBack(type3);
  parameters.PushBack(type4);
  return parameters;
}

ParameterArray FourParameters(Type* type1, StringParam name1, Type* type2, StringParam name2, Type* type3, StringParam name3, Type* type4, StringParam name4)
{
  ParameterArray parameters;
  DelegateParameter& a = parameters.PushBack();
  a.Name = name1;
  a.ParameterType = type1;

  DelegateParameter& b = parameters.PushBack();
  b.Name = name2;
  b.ParameterType = type2;

  DelegateParameter& c = parameters.PushBack();
  c.Name = name3;
  c.ParameterType = type3;

  DelegateParameter& d = parameters.PushBack();
  d.Name = name4;
  d.ParameterType = type4;
  return parameters;
}

ParameterArray FiveParameters(Type* type1, StringParam name1, Type* type2, StringParam name2, Type* type3, StringParam name3, Type* type4, StringParam name4, Type* type5, StringParam name5)
{
  ParameterArray parameters;
  DelegateParameter& a = parameters.PushBack();
  a.Name = name1;
  a.ParameterType = type1;

  DelegateParameter& b = parameters.PushBack();
  b.Name = name2;
  b.ParameterType = type2;

  DelegateParameter& c = parameters.PushBack();
  c.Name = name3;
  c.ParameterType = type3;

  DelegateParameter& d = parameters.PushBack();
  d.Name = name4;
  d.ParameterType = type4;

  DelegateParameter& e = parameters.PushBack();
  e.Name = name5;
  e.ParameterType = type5;
  return parameters;
}

void WriteImageArguments(
    RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, RaverieShaderIROp* result, int index, ImageUserData& imageData, RaverieSpirVFrontEndContext* context)
{
  // Find out how many arguments we have to write out before optional image
  // operands
  int nonOptionalOperands = functionCallNode->Arguments.Size() - index - imageData.mOptionalOperands;
  // Write all of the non optional operands from the start index. We might
  // actually skip some initial operands if they were processed on the outside
  // (e.g. combining image + sampler into SampledImage)
  for (int i = 0; i < nonOptionalOperands; ++i)
  {
    RaverieShaderIROp* arg = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[index], context);
    result->mArguments.PushBack(arg);
    ++index;
  }

  // If we have optional operands then write out the flags and then the extra
  // arguments
  if (imageData.mImageOperandFlags != spv::ImageOperandsMaskNone)
  {
    RaverieShaderIRConstantLiteral* literal = translator->GetOrCreateConstantIntegerLiteral(imageData.mImageOperandFlags);
    result->mArguments.PushBack(literal);

    for (int i = 0; i < imageData.mOptionalOperands; ++i)
    {
      RaverieShaderIROp* arg = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[index], context);
      result->mArguments.PushBack(arg);
      ++index;
    }
  }
}

// Resolves an SampledImage function
template <OpType opType>
inline void
ResolveCombinedSamplerFunction(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  ImageUserData& imageData = memberAccessNode->AccessedFunction->ComplexUserData.ReadObject<ImageUserData>(0);
  RaverieShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  // Create the op and write out all of the operands
  RaverieShaderIROp* result = translator->BuildIROpNoBlockAdd(opType, resultType, context);
  WriteImageArguments(translator, functionCallNode, result, 0, imageData, context);

  context->GetCurrentBlock()->AddOp(result);
  context->PushIRStack(result);
}

// Resolves a function that operates on a SampledImage but is given a Sampler
// and an Image and must combine them into a temporary.
template <OpType opType>
inline void
ResolveSplitImageSamplerFunction(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  ImageUserData& imageData = memberAccessNode->AccessedFunction->ComplexUserData.ReadObject<ImageUserData>(0);
  RaverieShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  RaverieShaderIROp* result = translator->BuildIROpNoBlockAdd(opType, resultType, context);

  // Combine the image and sampler together into a temporary sampled image.
  // To make life easier combined sampled image type is provided via the complex
  // user data.
  RaverieShaderIRType* sampledImageType = translator->FindType(imageData.mSampledImageType, functionCallNode);
  RaverieShaderIROp* imageValue = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  RaverieShaderIROp* samplerValue = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[1], context);
  RaverieShaderIROp* sampledImageValue = translator->BuildCurrentBlockIROp(OpType::OpSampledImage, sampledImageType, imageValue, samplerValue, context);
  result->mArguments.PushBack(sampledImageValue);

  // Now write out any remaining image operands while skipping the first two
  // arguments in the function call (since we already processed them)
  WriteImageArguments(translator, functionCallNode, result, 2, imageData, context);

  context->GetCurrentBlock()->AddOp(result);
  context->PushIRStack(result);
}

// Resolves a function that operates on an Image. If the input is a SampledImage
// this will first grab the image from the sampler.
template <OpType opType>
inline void ResolveImageFunction(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  ImageUserData& imageData = memberAccessNode->AccessedFunction->ComplexUserData.ReadObject<ImageUserData>(0);
  RaverieShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  RaverieShaderIROp* result = translator->BuildIROpNoBlockAdd(opType, resultType, context);

  // Get the image argument
  RaverieShaderIROp* imageValue = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  // First argument is actually a sampled image then grab the image from the
  // sampled image
  if (imageValue->mResultType->mBaseType == ShaderIRTypeBaseType::SampledImage)
  {
    RaverieShaderIROp* sampledImageValue = imageValue;
    RaverieShaderIRType* imageType = GetImageTypeFromSampledImage(sampledImageValue->mResultType);
    imageValue = translator->BuildCurrentBlockIROp(OpType::OpImage, imageType, sampledImageValue, context);
  }
  result->mArguments.PushBack(imageValue);

  // Write out the remaining image arguments, skipping the image itself (since
  // we already processed it)
  WriteImageArguments(translator, functionCallNode, result, 1, imageData, context);

  context->GetCurrentBlock()->AddOp(result);
  context->PushIRStack(result);
}

void AddSampleImplicitLod(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(type, "SampleImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSimpleFunction<OpType::OpImageSampleImplicitLod>;
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  parameters = ThreeParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(type, "SampleImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
}

void AddSampleExplicitLod(
    Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* lodType, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(type, "SampleExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));

  parameters = FourParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(type, "SampleExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask, set.mSampledImageType));
}

void AddSampleGradExplicitLod(
    Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* derivativeType, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = FourParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate", derivativeType, "ddx", derivativeType, "ddy");
  fn = builder.AddBoundFunction(type, "SampleGradExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(2, spv::ImageOperandsGradMask));

  parameters = FiveParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate", derivativeType, "ddx", derivativeType, "ddy");
  fn = builder.AddBoundFunction(type, "SampleGradExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(2, spv::ImageOperandsGradMask, set.mSampledImageType));
}

void AddSampleDrefImplicitLod(
    Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* depthType, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate", depthType, "depth");
  fn = builder.AddBoundFunction(type, "SampleDRefImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleDrefImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  parameters = FourParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate", depthType, "depth");
  fn = builder.AddBoundFunction(type, "SampleDRefImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleDrefImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
}

void AddSampleDrefExplicitLod(Raverie::LibraryBuilder& builder,
                              Raverie::BoundType* type,
                              SampledImageSet& set,
                              Raverie::BoundType* coordinateType,
                              Raverie::BoundType* depthType,
                              Raverie::BoundType* lodType,
                              Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = FourParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate", depthType, "depth", lodType, "lod");
  fn = builder.AddBoundFunction(type, "SampleDRefExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleDrefExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));

  parameters = FiveParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate", depthType, "depth", lodType, "lod");
  fn = builder.AddBoundFunction(type, "SampleDRefExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleDrefExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask, set.mSampledImageType));
}

void AddSampleProjImplicitLod(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(type, "SampleProjImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleProjImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  parameters = ThreeParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(type, "SampleProjImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleProjImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
}

void AddSampleProjExplicitLod(
    Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* lodType, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(type, "SampleProjExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleProjExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));

  parameters = FourParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(type, "SampleProjExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleProjExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask, set.mSampledImageType));
}

void AddSampleProjDrefImplicitLod(
    Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* depthType, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate", depthType, "depth");
  fn = builder.AddBoundFunction(type, "SampleProjDRefImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleProjDrefImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  parameters = FourParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate", depthType, "depth");
  fn = builder.AddBoundFunction(type, "SampleProjDRefImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleProjDrefImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
}

void AddSampleProjDrefExplicitLod(Raverie::LibraryBuilder& builder,
                                  Raverie::BoundType* type,
                                  SampledImageSet& set,
                                  Raverie::BoundType* coordinateType,
                                  Raverie::BoundType* depthType,
                                  Raverie::BoundType* lodType,
                                  Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = FourParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate", depthType, "depth", lodType, "lod");
  fn = builder.AddBoundFunction(type, "SampleProjDRefExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleProjDrefExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));

  parameters = FiveParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate", depthType, "depth", lodType, "lod");
  fn = builder.AddBoundFunction(type, "SampleProjDRefExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleProjDrefExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask, set.mSampledImageType));
}

void AddImageFetch(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordianteType, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(set.mImageType, "image", coordianteType, "coordinate");
  fn = builder.AddBoundFunction(type, "ImageFetch", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageFetch>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));

  parameters = TwoParameters(set.mSampledImageType, "sampledImage", coordianteType, "coordinate");
  fn = builder.AddBoundFunction(type, "ImageFetch", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageFetch>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
}

void AddImageFetchLod(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordianteType, Raverie::BoundType* lodType, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(set.mImageType, "image", coordianteType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(type, "ImageFetch", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageFetch>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));

  parameters = ThreeParameters(set.mSampledImageType, "sampledImage", coordianteType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(type, "ImageFetch", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageFetch>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));
}

void AddImageQuerySizeLod(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* lodType, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(set.mImageType, "image", lodType, "lod");
  fn = builder.AddBoundFunction(type, "ImageQuerySize", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQuerySizeLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));

  parameters = TwoParameters(set.mSampledImageType, "sampledImage", lodType, "lod");
  fn = builder.AddBoundFunction(type, "ImageQuerySize", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQuerySizeLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
}

void AddImageQuerySize(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  Raverie::BoundType* imageType = set.mImageType;
  if (imageType != nullptr)
  {
    parameters = OneParameter(imageType, "image");
    fn = builder.AddBoundFunction(type, "ImageQuerySize", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
    fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQuerySize>;
    fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
  }

  Raverie::BoundType* sampledImageType = set.mSampledImageType;
  if (sampledImageType != nullptr)
  {
    parameters = OneParameter(sampledImageType, "sampledImage");
    fn = builder.AddBoundFunction(type, "ImageQuerySize", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
    fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQuerySize>;
    fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
  }
}

void AddImageQueryLod(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(type, "ImageQueryLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageQueryLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  parameters = ThreeParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(type, "ImageQueryLod", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageQueryLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
}

void AddImageQueryLevels(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* returnType)
{
  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = OneParameter(set.mImageType, "image");
  fn = builder.AddBoundFunction(type, "ImageQueryLevels", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQueryLevels>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));

  parameters = OneParameter(set.mSampledImageType, "sampledImage");
  fn = builder.AddBoundFunction(type, "ImageQueryLevels", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQueryLevels>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
}

void AddImageRead(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* returnType)
{
  Raverie::BoundType* imageType = set.mImageType;
  if (imageType == nullptr)
    return;

  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(imageType, "image", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(type, "ImageRead", UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageRead>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, nullptr));
}

void AddImageWrite(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* texelType)
{
  Raverie::BoundType* imageType = set.mImageType;
  if (imageType == nullptr)
    return;

  Raverie::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(imageType, "image", coordinateType, "coordinate", texelType, "texel");
  fn = builder.AddBoundFunction(type, "ImageWrite", UnTranslatedBoundFunction, parameters, RaverieTypeId(void), Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageWrite>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, nullptr));
}

void AddImageFunctions(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, RaverieTypeGroups& types)
{
  Raverie::BoundType* intType = types.mIntegerVectorTypes[0];
  Raverie::BoundType* int2Type = types.mIntegerVectorTypes[1];
  Raverie::BoundType* realType = types.mRealVectorTypes[0];
  Raverie::BoundType* real2Type = types.mRealVectorTypes[1];
  Raverie::BoundType* real3Type = types.mRealVectorTypes[2];
  Raverie::BoundType* real4Type = types.mRealVectorTypes[3];

  Raverie::BoundType* samplerType = RaverieTypeId(Sampler);

  SampledImageSet sampler2dSet;
  sampler2dSet.mSamplerType = samplerType;
  sampler2dSet.mImageType = RaverieTypeId(Image2d);
  sampler2dSet.mSampledImageType = RaverieTypeId(SampledImage2d);

  SampledImageSet sampler2dDepthSet;
  sampler2dDepthSet.mSamplerType = samplerType;
  sampler2dDepthSet.mImageType = RaverieTypeId(DepthImage2d);
  sampler2dDepthSet.mSampledImageType = RaverieTypeId(SampledDepthImage2d);

  SampledImageSet samplerCubeSet;
  samplerCubeSet.mSamplerType = samplerType;
  samplerCubeSet.mImageType = RaverieTypeId(ImageCube);
  samplerCubeSet.mSampledImageType = RaverieTypeId(SampledImageCube);

  SampledImageSet storageImage2dSet;
  storageImage2dSet.mSamplerType = nullptr;
  storageImage2dSet.mImageType = RaverieTypeId(StorageImage2d);
  storageImage2dSet.mSampledImageType = nullptr;

  // Sample Implicit Lod
  AddSampleImplicitLod(builder, type, sampler2dSet, real2Type, real4Type);
  AddSampleImplicitLod(builder, type, samplerCubeSet, real3Type, real4Type);
  // Sample Explicit Lod
  AddSampleExplicitLod(builder, type, sampler2dSet, real2Type, realType, real4Type);
  AddSampleExplicitLod(builder, type, samplerCubeSet, real3Type, realType, real4Type);
  // Sample Grad Explicit Lod
  // Note: Grad functions are explicit Lod even though they cannot be mixed with
  // an lod param because the lod is computed from the gradient values.
  AddSampleGradExplicitLod(builder, type, sampler2dSet, real2Type, real2Type, real4Type);
  AddSampleGradExplicitLod(builder, type, samplerCubeSet, real3Type, real3Type, real4Type);
  // Sample Dref Implicit Lod
  AddSampleDrefImplicitLod(builder, type, sampler2dDepthSet, real2Type, realType, realType);
  // Sample Dref Explicit Lod
  AddSampleDrefExplicitLod(builder, type, sampler2dDepthSet, real2Type, realType, realType, realType);
  // Sample Proj Implicit Lod
  AddSampleProjImplicitLod(builder, type, sampler2dSet, real3Type, real4Type);
  // Sample Proj Explicit Lod
  AddSampleProjExplicitLod(builder, type, sampler2dSet, real3Type, realType, real4Type);
  // Sample Proj Dref Implicit Lod
  AddSampleProjDrefImplicitLod(builder, type, sampler2dDepthSet, real3Type, realType, realType);
  // Sample Proj Dref Explicit Lod
  AddSampleProjDrefExplicitLod(builder, type, sampler2dDepthSet, real3Type, realType, realType, realType);

  // Image Fetch
  AddImageFetch(builder, type, sampler2dSet, int2Type, real4Type);
  AddImageFetchLod(builder, type, sampler2dSet, int2Type, intType, real4Type);
  // Image Query Size Lod
  AddImageQuerySizeLod(builder, type, sampler2dSet, intType, int2Type);
  AddImageQuerySizeLod(builder, type, sampler2dDepthSet, intType, int2Type);
  AddImageQuerySizeLod(builder, type, samplerCubeSet, intType, int2Type);
  // Image Query Size
  // @JoshD: Backend fix required for these functions
  // AddImageQuerySize(builder, type, sampler2dSet, int2Type);
  // AddImageQuerySize(builder, type, sampler2dDepthSet, int2Type);
  // Image Query Lod
  AddImageQueryLod(builder, type, sampler2dSet, real2Type, real2Type);
  AddImageQueryLod(builder, type, sampler2dDepthSet, real2Type, real2Type);
  AddImageQueryLod(builder, type, samplerCubeSet, real3Type, real2Type);
  // Image Query Levels
  AddImageQueryLevels(builder, type, sampler2dSet, intType);
  AddImageQueryLevels(builder, type, sampler2dDepthSet, intType);
  AddImageQueryLevels(builder, type, samplerCubeSet, intType);

  // Read/Write images
  AddImageRead(builder, type, storageImage2dSet, int2Type, real4Type);
  AddImageWrite(builder, type, storageImage2dSet, int2Type, real4Type);
  AddImageQuerySize(builder, type, storageImage2dSet, int2Type);
}

} // namespace Raverie
