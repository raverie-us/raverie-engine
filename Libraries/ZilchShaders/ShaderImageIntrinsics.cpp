// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zilch
{

using namespace Zero;

ZilchDefineType(Sampler, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

ZilchDefineType(Image2d, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

ZilchDefineType(StorageImage2d, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

ZilchDefineType(DepthImage2d, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

ZilchDefineType(ImageCube, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

ZilchDefineType(SampledImage2d, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

ZilchDefineType(SampledDepthImage2d, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

ZilchDefineType(SampledImageCube, builder, type)
{
  type->AddAttribute(SpirVNameSettings::mNonCopyableAttributeName);
  // Mark the required storage class on this type. Used in the front end
  // translation.
  Attribute* storageAttribute = type->AddAttribute(SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassUniformConstant);
}

} // namespace Zilch

namespace Zero
{

using namespace Zilch;

ParameterArray FourParameters(Type* type1, Type* type2, Type* type3, Type* type4)
{
  ParameterArray parameters;
  parameters.PushBack(type1);
  parameters.PushBack(type2);
  parameters.PushBack(type3);
  parameters.PushBack(type4);
  return parameters;
}

ParameterArray FourParameters(Type* type1,
                              StringParam name1,
                              Type* type2,
                              StringParam name2,
                              Type* type3,
                              StringParam name3,
                              Type* type4,
                              StringParam name4)
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

ParameterArray FiveParameters(Type* type1,
                              StringParam name1,
                              Type* type2,
                              StringParam name2,
                              Type* type3,
                              StringParam name3,
                              Type* type4,
                              StringParam name4,
                              Type* type5,
                              StringParam name5)
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

void WriteImageArguments(ZilchSpirVFrontEnd* translator,
                         Zilch::FunctionCallNode* functionCallNode,
                         ZilchShaderIROp* result,
                         int index,
                         ImageUserData& imageData,
                         ZilchSpirVFrontEndContext* context)
{
  // Find out how many arguments we have to write out before optional image
  // operands
  int nonOptionalOperands = functionCallNode->Arguments.Size() - index - imageData.mOptionalOperands;
  // Write all of the non optional operands from the start index. We might
  // actually skip some initial operands if they were processed on the outside
  // (e.g. combining image + sampler into SampledImage)
  for (int i = 0; i < nonOptionalOperands; ++i)
  {
    ZilchShaderIROp* arg = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[index], context);
    result->mArguments.PushBack(arg);
    ++index;
  }

  // If we have optional operands then write out the flags and then the extra
  // arguments
  if (imageData.mImageOperandFlags != spv::ImageOperandsMaskNone)
  {
    ZilchShaderIRConstantLiteral* literal = translator->GetOrCreateConstantIntegerLiteral(imageData.mImageOperandFlags);
    result->mArguments.PushBack(literal);

    for (int i = 0; i < imageData.mOptionalOperands; ++i)
    {
      ZilchShaderIROp* arg = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[index], context);
      result->mArguments.PushBack(arg);
      ++index;
    }
  }
}

// Resolves an SampledImage function
template <OpType opType>
inline void ResolveCombinedSamplerFunction(ZilchSpirVFrontEnd* translator,
                                           Zilch::FunctionCallNode* functionCallNode,
                                           Zilch::MemberAccessNode* memberAccessNode,
                                           ZilchSpirVFrontEndContext* context)
{
  ImageUserData& imageData = memberAccessNode->AccessedFunction->ComplexUserData.ReadObject<ImageUserData>(0);
  ZilchShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  // Create the op and write out all of the operands
  ZilchShaderIROp* result = translator->BuildIROpNoBlockAdd(opType, resultType, context);
  WriteImageArguments(translator, functionCallNode, result, 0, imageData, context);

  context->GetCurrentBlock()->AddOp(result);
  context->PushIRStack(result);
}

// Resolves a function that operates on a SampledImage but is given a Sampler
// and an Image and must combine them into a temporary.
template <OpType opType>
inline void ResolveSplitImageSamplerFunction(ZilchSpirVFrontEnd* translator,
                                             Zilch::FunctionCallNode* functionCallNode,
                                             Zilch::MemberAccessNode* memberAccessNode,
                                             ZilchSpirVFrontEndContext* context)
{
  ImageUserData& imageData = memberAccessNode->AccessedFunction->ComplexUserData.ReadObject<ImageUserData>(0);
  ZilchShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  ZilchShaderIROp* result = translator->BuildIROpNoBlockAdd(opType, resultType, context);

  // Combine the image and sampler together into a temporary sampled image.
  // To make life easier combined sampled image type is provided via the complex
  // user data.
  ZilchShaderIRType* sampledImageType = translator->FindType(imageData.mSampledImageType, functionCallNode);
  ZilchShaderIROp* imageValue = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  ZilchShaderIROp* samplerValue = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[1], context);
  ZilchShaderIROp* sampledImageValue =
      translator->BuildCurrentBlockIROp(OpType::OpSampledImage, sampledImageType, imageValue, samplerValue, context);
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
inline void ResolveImageFunction(ZilchSpirVFrontEnd* translator,
                                 Zilch::FunctionCallNode* functionCallNode,
                                 Zilch::MemberAccessNode* memberAccessNode,
                                 ZilchSpirVFrontEndContext* context)
{
  ImageUserData& imageData = memberAccessNode->AccessedFunction->ComplexUserData.ReadObject<ImageUserData>(0);
  ZilchShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  ZilchShaderIROp* result = translator->BuildIROpNoBlockAdd(opType, resultType, context);

  // Get the image argument
  ZilchShaderIROp* imageValue = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  // First argument is actually a sampled image then grab the image from the
  // sampled image
  if (imageValue->mResultType->mBaseType == ShaderIRTypeBaseType::SampledImage)
  {
    ZilchShaderIROp* sampledImageValue = imageValue;
    ZilchShaderIRType* imageType = GetImageTypeFromSampledImage(sampledImageValue->mResultType);
    imageValue = translator->BuildCurrentBlockIROp(OpType::OpImage, imageType, sampledImageValue, context);
  }
  result->mArguments.PushBack(imageValue);

  // Write out the remaining image arguments, skipping the image itself (since
  // we already processed it)
  WriteImageArguments(translator, functionCallNode, result, 1, imageData, context);

  context->GetCurrentBlock()->AddOp(result);
  context->PushIRStack(result);
}

void AddSampleImplicitLod(Zilch::LibraryBuilder& builder,
                          Zilch::BoundType* type,
                          SampledImageSet& set,
                          Zilch::BoundType* coordinateType,
                          Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(
      type, "SampleImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSimpleFunction<OpType::OpImageSampleImplicitLod>;
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  parameters = ThreeParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(
      type, "SampleImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
}

void AddSampleExplicitLod(Zilch::LibraryBuilder& builder,
                          Zilch::BoundType* type,
                          SampledImageSet& set,
                          Zilch::BoundType* coordinateType,
                          Zilch::BoundType* lodType,
                          Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(
      type, "SampleExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));

  parameters = FourParameters(
      set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(
      type, "SampleExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask, set.mSampledImageType));
}

void AddSampleGradExplicitLod(Zilch::LibraryBuilder& builder,
                              Zilch::BoundType* type,
                              SampledImageSet& set,
                              Zilch::BoundType* coordinateType,
                              Zilch::BoundType* derivativeType,
                              Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = FourParameters(set.mSampledImageType,
                              "sampledImage",
                              coordinateType,
                              "coordinate",
                              derivativeType,
                              "ddx",
                              derivativeType,
                              "ddy");
  fn = builder.AddBoundFunction(
      type, "SampleGradExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(2, spv::ImageOperandsGradMask));

  parameters = FiveParameters(set.mImageType,
                              "image",
                              set.mSamplerType,
                              "sampler",
                              coordinateType,
                              "coordinate",
                              derivativeType,
                              "ddx",
                              derivativeType,
                              "ddy");
  fn = builder.AddBoundFunction(
      type, "SampleGradExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(2, spv::ImageOperandsGradMask, set.mSampledImageType));
}

void AddSampleDrefImplicitLod(Zilch::LibraryBuilder& builder,
                              Zilch::BoundType* type,
                              SampledImageSet& set,
                              Zilch::BoundType* coordinateType,
                              Zilch::BoundType* depthType,
                              Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate", depthType, "depth");
  fn = builder.AddBoundFunction(
      type, "SampleDRefImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleDrefImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  parameters = FourParameters(
      set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate", depthType, "depth");
  fn = builder.AddBoundFunction(
      type, "SampleDRefImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleDrefImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
}

void AddSampleDrefExplicitLod(Zilch::LibraryBuilder& builder,
                              Zilch::BoundType* type,
                              SampledImageSet& set,
                              Zilch::BoundType* coordinateType,
                              Zilch::BoundType* depthType,
                              Zilch::BoundType* lodType,
                              Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = FourParameters(
      set.mSampledImageType, "sampledImage", coordinateType, "coordinate", depthType, "depth", lodType, "lod");
  fn = builder.AddBoundFunction(
      type, "SampleDRefExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleDrefExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));

  parameters = FiveParameters(set.mImageType,
                              "image",
                              set.mSamplerType,
                              "sampler",
                              coordinateType,
                              "coordinate",
                              depthType,
                              "depth",
                              lodType,
                              "lod");
  fn = builder.AddBoundFunction(
      type, "SampleDRefExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleDrefExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask, set.mSampledImageType));
}

void AddSampleProjImplicitLod(Zilch::LibraryBuilder& builder,
                              Zilch::BoundType* type,
                              SampledImageSet& set,
                              Zilch::BoundType* coordinateType,
                              Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(
      type, "SampleProjImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleProjImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  parameters = ThreeParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(
      type, "SampleProjImplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleProjImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
}

void AddSampleProjExplicitLod(Zilch::LibraryBuilder& builder,
                              Zilch::BoundType* type,
                              SampledImageSet& set,
                              Zilch::BoundType* coordinateType,
                              Zilch::BoundType* lodType,
                              Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(
      type, "SampleProjExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleProjExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));

  parameters = FourParameters(
      set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(
      type, "SampleProjExplicitLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleProjExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask, set.mSampledImageType));
}

void AddSampleProjDrefImplicitLod(Zilch::LibraryBuilder& builder,
                                  Zilch::BoundType* type,
                                  SampledImageSet& set,
                                  Zilch::BoundType* coordinateType,
                                  Zilch::BoundType* depthType,
                                  Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate", depthType, "depth");
  fn = builder.AddBoundFunction(type,
                                "SampleProjDRefImplicitLod",
                                UnTranslatedBoundFunction,
                                parameters,
                                returnType,
                                Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleProjDrefImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  parameters = FourParameters(
      set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate", depthType, "depth");
  fn = builder.AddBoundFunction(type,
                                "SampleProjDRefImplicitLod",
                                UnTranslatedBoundFunction,
                                parameters,
                                returnType,
                                Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleProjDrefImplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
}

void AddSampleProjDrefExplicitLod(Zilch::LibraryBuilder& builder,
                                  Zilch::BoundType* type,
                                  SampledImageSet& set,
                                  Zilch::BoundType* coordinateType,
                                  Zilch::BoundType* depthType,
                                  Zilch::BoundType* lodType,
                                  Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = FourParameters(
      set.mSampledImageType, "sampledImage", coordinateType, "coordinate", depthType, "depth", lodType, "lod");
  fn = builder.AddBoundFunction(type,
                                "SampleProjDRefExplicitLod",
                                UnTranslatedBoundFunction,
                                parameters,
                                returnType,
                                Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageSampleProjDrefExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));

  parameters = FiveParameters(set.mImageType,
                              "image",
                              set.mSamplerType,
                              "sampler",
                              coordinateType,
                              "coordinate",
                              depthType,
                              "depth",
                              lodType,
                              "lod");
  fn = builder.AddBoundFunction(type,
                                "SampleProjDRefExplicitLod",
                                UnTranslatedBoundFunction,
                                parameters,
                                returnType,
                                Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageSampleProjDrefExplicitLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask, set.mSampledImageType));
}

void AddImageFetch(Zilch::LibraryBuilder& builder,
                   Zilch::BoundType* type,
                   SampledImageSet& set,
                   Zilch::BoundType* coordianteType,
                   Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(set.mImageType, "image", coordianteType, "coordinate");
  fn = builder.AddBoundFunction(
      type, "ImageFetch", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageFetch>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));

  parameters = TwoParameters(set.mSampledImageType, "sampledImage", coordianteType, "coordinate");
  fn = builder.AddBoundFunction(
      type, "ImageFetch", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageFetch>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
}

void AddImageFetchLod(Zilch::LibraryBuilder& builder,
                      Zilch::BoundType* type,
                      SampledImageSet& set,
                      Zilch::BoundType* coordianteType,
                      Zilch::BoundType* lodType,
                      Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(set.mImageType, "image", coordianteType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(
      type, "ImageFetch", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageFetch>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));

  parameters = ThreeParameters(set.mSampledImageType, "sampledImage", coordianteType, "coordinate", lodType, "lod");
  fn = builder.AddBoundFunction(
      type, "ImageFetch", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageFetch>;
  fn->ComplexUserData.WriteObject(ImageUserData(1, spv::ImageOperandsLodMask));
}

void AddImageQuerySizeLod(Zilch::LibraryBuilder& builder,
                          Zilch::BoundType* type,
                          SampledImageSet& set,
                          Zilch::BoundType* lodType,
                          Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(set.mImageType, "image", lodType, "lod");
  fn = builder.AddBoundFunction(
      type, "ImageQuerySize", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQuerySizeLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));

  parameters = TwoParameters(set.mSampledImageType, "sampledImage", lodType, "lod");
  fn = builder.AddBoundFunction(
      type, "ImageQuerySize", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQuerySizeLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
}

void AddImageQuerySize(Zilch::LibraryBuilder& builder,
                       Zilch::BoundType* type,
                       SampledImageSet& set,
                       Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  Zilch::BoundType* imageType = set.mImageType;
  if (imageType != nullptr)
  {
    parameters = OneParameter(imageType, "image");
    fn = builder.AddBoundFunction(
        type, "ImageQuerySize", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
    fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQuerySize>;
    fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
  }

  Zilch::BoundType* sampledImageType = set.mSampledImageType;
  if (sampledImageType != nullptr)
  {
    parameters = OneParameter(sampledImageType, "sampledImage");
    fn = builder.AddBoundFunction(
        type, "ImageQuerySize", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
    fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQuerySize>;
    fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
  }
}

void AddImageQueryLod(Zilch::LibraryBuilder& builder,
                      Zilch::BoundType* type,
                      SampledImageSet& set,
                      Zilch::BoundType* coordinateType,
                      Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(set.mSampledImageType, "sampledImage", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(
      type, "ImageQueryLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveCombinedSamplerFunction<OpType::OpImageQueryLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  parameters = ThreeParameters(set.mImageType, "image", set.mSamplerType, "sampler", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(
      type, "ImageQueryLod", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveSplitImageSamplerFunction<OpType::OpImageQueryLod>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, set.mSampledImageType));
  fn->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
}

void AddImageQueryLevels(Zilch::LibraryBuilder& builder,
                         Zilch::BoundType* type,
                         SampledImageSet& set,
                         Zilch::BoundType* returnType)
{
  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = OneParameter(set.mImageType, "image");
  fn = builder.AddBoundFunction(
      type, "ImageQueryLevels", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQueryLevels>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));

  parameters = OneParameter(set.mSampledImageType, "sampledImage");
  fn = builder.AddBoundFunction(
      type, "ImageQueryLevels", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageQueryLevels>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone));
}

void AddImageRead(Zilch::LibraryBuilder& builder,
                  Zilch::BoundType* type,
                  SampledImageSet& set,
                  Zilch::BoundType* coordinateType,
                  Zilch::BoundType* returnType)
{
  Zilch::BoundType* imageType = set.mImageType;
  if (imageType == nullptr)
    return;

  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = TwoParameters(imageType, "image", coordinateType, "coordinate");
  fn = builder.AddBoundFunction(
      type, "ImageRead", UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageRead>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, nullptr));
}

void AddImageWrite(Zilch::LibraryBuilder& builder,
                   Zilch::BoundType* type,
                   SampledImageSet& set,
                   Zilch::BoundType* coordinateType,
                   Zilch::BoundType* texelType)
{
  Zilch::BoundType* imageType = set.mImageType;
  if (imageType == nullptr)
    return;

  Zilch::Function* fn = nullptr;
  ParameterArray parameters;

  parameters = ThreeParameters(imageType, "image", coordinateType, "coordinate", texelType, "texel");
  fn = builder.AddBoundFunction(
      type, "ImageWrite", UnTranslatedBoundFunction, parameters, ZilchTypeId(void), Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveImageFunction<OpType::OpImageWrite>;
  fn->ComplexUserData.WriteObject(ImageUserData(0, spv::ImageOperandsMaskNone, nullptr));
}

void AddImageFunctions(Zilch::LibraryBuilder& builder, Zilch::BoundType* type, TypeGroups& types)
{
  Zilch::BoundType* intType = types.mIntegerVectorTypes[0]->mZilchType;
  Zilch::BoundType* int2Type = types.mIntegerVectorTypes[1]->mZilchType;
  Zilch::BoundType* realType = types.mRealVectorTypes[0]->mZilchType;
  Zilch::BoundType* real2Type = types.mRealVectorTypes[1]->mZilchType;
  Zilch::BoundType* real3Type = types.mRealVectorTypes[2]->mZilchType;
  Zilch::BoundType* real4Type = types.mRealVectorTypes[3]->mZilchType;

  Zilch::BoundType* samplerType = ZilchTypeId(Sampler);

  SampledImageSet sampler2dSet;
  sampler2dSet.mSamplerType = samplerType;
  sampler2dSet.mImageType = ZilchTypeId(Image2d);
  sampler2dSet.mSampledImageType = ZilchTypeId(SampledImage2d);

  SampledImageSet sampler2dDepthSet;
  sampler2dDepthSet.mSamplerType = samplerType;
  sampler2dDepthSet.mImageType = ZilchTypeId(DepthImage2d);
  sampler2dDepthSet.mSampledImageType = ZilchTypeId(SampledDepthImage2d);

  SampledImageSet samplerCubeSet;
  samplerCubeSet.mSamplerType = samplerType;
  samplerCubeSet.mImageType = ZilchTypeId(ImageCube);
  samplerCubeSet.mSampledImageType = ZilchTypeId(SampledImageCube);

  SampledImageSet storageImage2dSet;
  storageImage2dSet.mSamplerType = nullptr;
  storageImage2dSet.mImageType = ZilchTypeId(StorageImage2d);
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

} // namespace Zero
