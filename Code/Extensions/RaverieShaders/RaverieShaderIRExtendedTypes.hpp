// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Simple wrapper around an image type. Contains helpers to look up parameters
/// about an image type so the index doesn't have to be remembered.
class RaverieShaderIRImageType
{
public:
  RaverieShaderIRImageType();
  RaverieShaderIRImageType(RaverieShaderIRType* type);

  /// Load the given ir type. Returns false if the type isn't an image.
  bool Load(RaverieShaderIRType* type);

  /// Returns the component type of this image (e.g. Real).
  /// Note: this will not be Real4 as that's controlled by the format.
  RaverieShaderIRType* GetSampledType();
  /// The dimensionality of the image. Return type is of spv::Dim.
  int GetDim();
  /// Is this a depth image? 0 indicates no depth, 1 indicates depth, 2 is
  /// unknown.
  int GetDepth();
  /// Is the image arrayed? 0 indicates non-arrayed, 1 indicates arrayed.
  int GetArrayed();
  /// Is the image multi-sampled? 0 indicates single-sampled, 1 indicates
  /// multi-sampled.
  int GetMultiSampled();
  /// Is this image used with a sampler? 0 is unknown (only known at run time),
  /// 1 indicates this will be used with a sampler, 2 indicates this will not
  /// be used with a sampler (a storage image).
  int GetSampled();
  /// The image format. Returns type of spv::ImageFormat.
  int GetFormat();

  /// Helper to determine if this is a storage image (cannot be used with a
  /// sampler).
  bool IsStorageImage();

  int GetIntegerConstantParameterValue(int parameterIndex);

  RaverieShaderIRType* mIRType;
};

/// Simple wrapper around a runtime array type. Contains helpers to look up
/// various underlying type data.
class RaverieShaderIRRuntimeArrayType
{
public:
  RaverieShaderIRRuntimeArrayType();

  /// Load the given ir type. Returns false if the type isn't a runtime array.
  bool Load(RaverieShaderIRType* type);

  /// Returns the actual underlying spirv runtime array type.
  /// The type generated for raverie's RuntimeArray is actually the
  /// struct wrapper around the spirv runtime array.
  RaverieShaderIRType* GetSpirVRuntimeArrayType();
  /// Returns the contained data type of the runtime array.
  RaverieShaderIRType* GetContainedType();

  RaverieShaderIRType* mIRType;
};

} // namespace Raverie
