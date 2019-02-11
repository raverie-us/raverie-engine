// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zilch
{

class Sampler
{
  ZilchDeclareType(Sampler, TypeCopyMode::ValueType);
};

class Image2d
{
  ZilchDeclareType(Image2d, TypeCopyMode::ValueType);
};

class StorageImage2d
{
  ZilchDeclareType(StorageImage2d, TypeCopyMode::ValueType);
};

class DepthImage2d
{
  ZilchDeclareType(DepthImage2d, TypeCopyMode::ValueType);
};

class ImageCube
{
  ZilchDeclareType(ImageCube, TypeCopyMode::ValueType);
};

/// Represents a sampler combined with an image.
class SampledImage2d
{
  ZilchDeclareType(SampledImage2d, TypeCopyMode::ValueType);
};

class SampledDepthImage2d
{
  ZilchDeclareType(SampledDepthImage2d, TypeCopyMode::ValueType);
};

class SampledImageCube
{
  ZilchDeclareType(SampledImageCube, TypeCopyMode::ValueType);
};

// User data written to ZilchFunctions to make image/sampler functions easier to
// translate
struct ImageUserData
{
  ImageUserData(int optionalOperands,
                int flags,
                Zilch::BoundType* sampledImageType = nullptr)
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
  Zilch::BoundType* mSampledImageType;
};

} // namespace Zilch

namespace Zero
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
  Zilch::BoundType* mSamplerType;
  Zilch::BoundType* mImageType;
  Zilch::BoundType* mSampledImageType;
};

void AddSampleImplicitLod(Zilch::LibraryBuilder& builder,
                          Zilch::BoundType* type,
                          SampledImageSet& set,
                          Zilch::BoundType* coordinateType,
                          Zilch::BoundType* returnType);
void AddSampleExplicitLod(Zilch::LibraryBuilder& builder,
                          Zilch::BoundType* type,
                          SampledImageSet& set,
                          Zilch::BoundType* coordinateType,
                          Zilch::BoundType* lodType,
                          Zilch::BoundType* returnType);
void AddSampleGradExplicitLod(Zilch::LibraryBuilder& builder,
                              Zilch::BoundType* type,
                              SampledImageSet& set,
                              Zilch::BoundType* coordinateType,
                              Zilch::BoundType* derivativeType,
                              Zilch::BoundType* returnType);
void AddSampleDrefImplicitLod(Zilch::LibraryBuilder& builder,
                              Zilch::BoundType* type,
                              SampledImageSet& set,
                              Zilch::BoundType* coordinateType,
                              Zilch::BoundType* depthType,
                              Zilch::BoundType* returnType);
void AddSampleDrefExplicitLod(Zilch::LibraryBuilder& builder,
                              Zilch::BoundType* type,
                              SampledImageSet& set,
                              Zilch::BoundType* coordinateType,
                              Zilch::BoundType* depthType,
                              Zilch::BoundType* lodType,
                              Zilch::BoundType* returnType);
void AddSampleProjImplicitLod(Zilch::LibraryBuilder& builder,
                              Zilch::BoundType* type,
                              SampledImageSet& set,
                              Zilch::BoundType* coordinateType,
                              Zilch::BoundType* returnType);
void AddSampleProjExplicitLod(Zilch::LibraryBuilder& builder,
                              Zilch::BoundType* type,
                              SampledImageSet& set,
                              Zilch::BoundType* coordinateType,
                              Zilch::BoundType* lodType,
                              Zilch::BoundType* returnType);
void AddSampleProjDrefImplicitLod(Zilch::LibraryBuilder& builder,
                                  Zilch::BoundType* type,
                                  SampledImageSet& set,
                                  Zilch::BoundType* coordinateType,
                                  Zilch::BoundType* depthType,
                                  Zilch::BoundType* returnType);
void AddSampleProjDrefExplicitLod(Zilch::LibraryBuilder& builder,
                                  Zilch::BoundType* type,
                                  SampledImageSet& set,
                                  Zilch::BoundType* coordinateType,
                                  Zilch::BoundType* depthType,
                                  Zilch::BoundType* lodType,
                                  Zilch::BoundType* returnType);
void AddImageFetch(Zilch::LibraryBuilder& builder,
                   Zilch::BoundType* type,
                   SampledImageSet& set,
                   Zilch::BoundType* coordianteType,
                   Zilch::BoundType* lodType,
                   Zilch::BoundType* returnType);
void AddImageQuerySizeLod(Zilch::LibraryBuilder& builder,
                          Zilch::BoundType* type,
                          SampledImageSet& set,
                          Zilch::BoundType* lodType,
                          Zilch::BoundType* returnType);
void AddImageQuerySize(Zilch::LibraryBuilder& builder,
                       Zilch::BoundType* type,
                       SampledImageSet& set,
                       Zilch::BoundType* returnType);
void AddImageQueryLod(Zilch::LibraryBuilder& builder,
                      Zilch::BoundType* type,
                      SampledImageSet& set,
                      Zilch::BoundType* coordinateType,
                      Zilch::BoundType* returnType);
void AddImageQueryLevels(Zilch::LibraryBuilder& builder,
                         Zilch::BoundType* type,
                         SampledImageSet& set,
                         Zilch::BoundType* returnType);

// Add all of the relevant image intrinsics to the shader given bound type.
void AddImageFunctions(Zilch::LibraryBuilder& builder,
                       Zilch::BoundType* type,
                       TypeGroups& types);

} // namespace Zero
