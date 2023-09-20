// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class Sampler
{
  RaverieDeclareType(Sampler, TypeCopyMode::ValueType);
};

class Image2d
{
  RaverieDeclareType(Image2d, TypeCopyMode::ValueType);
};

class StorageImage2d
{
  RaverieDeclareType(StorageImage2d, TypeCopyMode::ValueType);
};

class DepthImage2d
{
  RaverieDeclareType(DepthImage2d, TypeCopyMode::ValueType);
};

class ImageCube
{
  RaverieDeclareType(ImageCube, TypeCopyMode::ValueType);
};

/// Represents a sampler combined with an image.
class SampledImage2d
{
  RaverieDeclareType(SampledImage2d, TypeCopyMode::ValueType);
};

class SampledDepthImage2d
{
  RaverieDeclareType(SampledDepthImage2d, TypeCopyMode::ValueType);
};

class SampledImageCube
{
  RaverieDeclareType(SampledImageCube, TypeCopyMode::ValueType);
};

// User data written to RaverieFunctions to make image/sampler functions easier to
// translate
struct ImageUserData
{
  ImageUserData(int optionalOperands, int flags, Raverie::BoundType* sampledImageType = nullptr)
  {
    mOptionalOperands = optionalOperands;
    mImageOperandFlags = flags;
    mSampledImageType = sampledImageType;
  }
  // How many optional operands exist
  int mOptionalOperands;
  // Flags that have to come before the before the optional
  // operands that specify what the extra operands are.
  int mImageOperandFlags;
  // The combined SampledImage type that some functions use. Added here to make
  // it easier to not have to look up the type of the SampledImage when needed.
  Raverie::BoundType* mSampledImageType;
};

} // namespace Raverie

namespace Raverie
{

// Represents a sampled image and what sampler + image were combined to make it
// (to make it easier to bind a bunch of functions on this "grouping")
struct SampledImageSet
{
  SampledImageSet()
  {
    mSamplerType = nullptr;
    mImageType = nullptr;
    mSampledImageType = nullptr;
  }
  Raverie::BoundType* mSamplerType;
  Raverie::BoundType* mImageType;
  Raverie::BoundType* mSampledImageType;
};

void AddSampleImplicitLod(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* returnType);
void AddSampleExplicitLod(
    Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* lodType, Raverie::BoundType* returnType);
void AddSampleGradExplicitLod(
    Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* derivativeType, Raverie::BoundType* returnType);
void AddSampleDrefImplicitLod(
    Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* depthType, Raverie::BoundType* returnType);
void AddSampleDrefExplicitLod(Raverie::LibraryBuilder& builder,
                              Raverie::BoundType* type,
                              SampledImageSet& set,
                              Raverie::BoundType* coordinateType,
                              Raverie::BoundType* depthType,
                              Raverie::BoundType* lodType,
                              Raverie::BoundType* returnType);
void AddSampleProjImplicitLod(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* returnType);
void AddSampleProjExplicitLod(
    Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* lodType, Raverie::BoundType* returnType);
void AddSampleProjDrefImplicitLod(
    Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* depthType, Raverie::BoundType* returnType);
void AddSampleProjDrefExplicitLod(Raverie::LibraryBuilder& builder,
                                  Raverie::BoundType* type,
                                  SampledImageSet& set,
                                  Raverie::BoundType* coordinateType,
                                  Raverie::BoundType* depthType,
                                  Raverie::BoundType* lodType,
                                  Raverie::BoundType* returnType);
void AddImageFetch(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordianteType, Raverie::BoundType* lodType, Raverie::BoundType* returnType);
void AddImageQuerySizeLod(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* lodType, Raverie::BoundType* returnType);
void AddImageQuerySize(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* returnType);
void AddImageQueryLod(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* coordinateType, Raverie::BoundType* returnType);
void AddImageQueryLevels(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, SampledImageSet& set, Raverie::BoundType* returnType);

// Add all of the relevant image intrinsics to the shader given bound type.
void AddImageFunctions(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, RaverieTypeGroups& types);

} // namespace Raverie
