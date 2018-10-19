#pragma once

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderIRImageType
/// Simple wrapper around an image type. Contains helpers to look up parameters
/// about an image type so the index doesn't have to be remembered.
class ZilchShaderIRImageType
{
public:
  ZilchShaderIRImageType();
  ZilchShaderIRImageType(ZilchShaderIRType* type);

  /// Load the given ir type. Returns false if the type isn't an image.
  bool Load(ZilchShaderIRType* type);

  /// Returns the component type of this image (e.g. Real).
  /// Note: this will not be Real4 as that's controlled by the format.
  ZilchShaderIRType* GetSampledType();
  /// The dimensionality of the image. Return type is of spv::Dim.
  int GetDim();
  /// Is this a depth image? 0 indicates no depth, 1 indicates depth, 2 is unknown.
  int GetDepth();
  /// Is the image arrayed? 0 indicates non-arrayed, 1 indicates arrayed.
  int GetArrayed();
  /// Is the image multi-sampled? 0 indicates single-sampled, 1 indicates multi-sampled.
  int GetMultiSampled();
  /// Is this image used with a sampler? 0 is unknown (only known at run time),
  /// 1 indicates this will be used with a sampler, 2 indicates this will not
  /// be used with a sampler (a storage image).
  int GetSampled();
  /// The image format. Returns type of spv::ImageFormat.
  int GetFormat();

  //-----------------------------------------------------------------Internal
  int GetIntegerConstantParameterValue(int parameterIndex);

  ZilchShaderIRType* mIRType;
};

}//namespace Zero