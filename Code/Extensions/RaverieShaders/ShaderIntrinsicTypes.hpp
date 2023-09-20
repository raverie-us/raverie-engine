// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
class RaverieShaderIRType;
}

namespace Raverie
{

/// Helper class that contains a bunch of intrinsic functions for spir-v
/// (generated in the cpp).
class ShaderIntrinsics
{
  RaverieDeclareType(ShaderIntrinsics, TypeCopyMode::ReferenceType);
};

// Component data for input/output geometry stream types. Used for reflection
// purposes.
struct GeometryStreamUserData
{
  RaverieDeclareType(GeometryStreamUserData, TypeCopyMode::ReferenceType);

public:
  GeometryStreamUserData()
  {
  }

  void Set(spv::ExecutionMode executionMode);

  // Is this stream an input or output?
  bool mInput;
  // What is the size of this stream (how many elements). Deduced from the
  // execution mode.
  int mSize;
  // What execution mode does this stream use? (e.g. triangles, lines)
  spv::ExecutionMode mExecutionMode;
};

// Component data added to a geometry fragment type. Needed to get the
// input/output stream types which are only detected when walking the main
// function.
struct GeometryFragmentUserData
{
  RaverieDeclareType(GeometryFragmentUserData, TypeCopyMode::ReferenceType);
  GeometryFragmentUserData();

  Raverie::RaverieShaderIRType* GetInputVertexType();
  Raverie::RaverieShaderIRType* GetOutputVertexType();

  Raverie::RaverieShaderIRType* mInputStreamType;
  Raverie::RaverieShaderIRType* mOutputStreamType;
};

/// User data for a compute shader to know what data was parsed from the
/// [Compute] attribute.
struct ComputeFragmentUserData
{
  RaverieDeclareType(ComputeFragmentUserData, TypeCopyMode::ReferenceType);
  ComputeFragmentUserData();

  int mLocalSizeX;
  int mLocalSizeY;
  int mLocalSizeZ;
};

/// This is a hack type used for binding instructions that require a unsigned int
/// while dealing with raverie not actually having any unsigned types.
class UnsignedInt
{
  RaverieDeclareType(UnsignedInt, TypeCopyMode::ValueType);
  unsigned int mValue;
};

} // namespace Raverie
