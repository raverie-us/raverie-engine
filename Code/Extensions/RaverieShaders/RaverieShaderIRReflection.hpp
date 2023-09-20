// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Reflection data for a shader resource. This could be a top-level
/// struct/field or a nested field within a struct.
struct ShaderResourceReflectionData
{
  ShaderResourceReflectionData();

  /// The name of this object. Typically used for reflection matching or for an
  /// api's reflection.
  String mInstanceName;
  /// The typename of this object. Currently this name depends on the portion of
  /// the pipeline that generated this reflection data (e.g. in Raverie this is
  /// Real3 but in glsl this is vec3).
  String mTypeName;
  /// If this is a member within a struct, this is the offset in bytes from the
  /// start of the parent struct that the memory of this object starts at.
  size_t mOffsetInBytes;
  /// The size of this object in bytes.
  size_t mSizeInBytes;
  /// What descriptor set this object is in. Currently always set to 0.
  int mDescriptorSet;
  /// The binding index of this object (if applicable). This can be used to
  /// directly set a variable without going through an individual api's
  /// reflection.
  int mBinding;
  /// The bound location of this object (if applicable)
  int mLocation;
  /// If this is a matrix, the stride represents the offset of each column of
  /// the matrix. (e.g. Real3x3 has a stride of 16 because each Real3 still has
  /// to be aligned to Real4 boundaries).
  int mStride;
};

/// Represents one top-level resource in a shader stage. This could be an input
/// block (or individual variable), an output block, a uniform block, and so on.
struct ShaderStageResource
{
  /// The reflection data of this entire resource (block or individual variable)
  ShaderResourceReflectionData mReflectionData;
  /// Data about sub-members of this structure (if applicable).
  Array<ShaderResourceReflectionData> mMembers;
  /// A lookup map of original names to member ids. For instance, if the
  /// previous translation stage had a member variable named "MyVariable" then
  /// the location of the member variable can be found with this map which can
  /// be used to find the new name of said variable.
  HashMap<String, int> mLookupMap;
};

/// A collection of all of the reflection data for the interface of one shader
/// stage.
struct ShaderStageInterfaceReflection
{
  /// The name of the type that created this shader. Used to find the actual
  /// raverie type if necessary.
  String mShaderTypeName;

  Array<ShaderStageResource> mUniforms;
  Array<ShaderStageResource> mStageInputs;
  Array<ShaderStageResource> mStageOutputs;
  Array<ShaderStageResource> mSamplers;
  Array<ShaderStageResource> mImages;
  Array<ShaderStageResource> mSampledImages;
  Array<ShaderStageResource> mStorageImages;
  Array<ShaderStageResource> mStructedStorageBuffers;

  /// An individual sampler, image, or sampled image can turn into one or more
  /// of all sampler/image types. This stores what renames happened (a few
  /// technically can't happen such as image to sampler).
  struct SampledImageRemappings
  {
    Array<String> mImageRemappings;
    Array<String> mSamplerRemappings;
    Array<String> mSampledImageRemappings;
  };
  // Remappings for each sampler/image type as to what other sampler/image types
  // it became.
  HashMap<String, SampledImageRemappings> mSampledImageRemappings;
  HashMap<String, SampledImageRemappings> mImageRemappings;
  HashMap<String, SampledImageRemappings> mSamplerRemappings;

  /// Specialization constants by name to the api linkage id
  HashMap<String, int> mSpecializationConstants;
};

} // namespace Raverie
