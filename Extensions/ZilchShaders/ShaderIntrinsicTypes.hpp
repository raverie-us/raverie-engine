///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class ZilchShaderIRType;
}

namespace Zilch
{

//------------------------------------------------------------------------ShaderIntrinsics
/// Helper class that contains a bunch of intrinsic functions for spir-v (generated in the cpp).
class ShaderIntrinsics
{
  ZilchDeclareType(ShaderIntrinsics, TypeCopyMode::ReferenceType);
};

//------------------------------------------------------------------------Shader
class Shader
{
  ZilchDeclareType(Shader, TypeCopyMode::ReferenceType);

  static void AddInlineShaderCode(StringParam language, StringParam shaderCode);
  static void AddInlineShaderCode(StringParam language, int minVersion, int maxVersion, StringParam shaderCode);
  static bool IsLanguage(StringParam language);
  static bool IsLanguage(StringParam language, int minVersion, int maxVersion);
};

//------------------------------------------------------------------------GeometryStreamUserData
// Component data for input/output geometry stream types. Used for reflection purposes.
struct GeometryStreamUserData
{
  ZilchDeclareType(GeometryStreamUserData, TypeCopyMode::ReferenceType);
public:
  GeometryStreamUserData() {}

  void Set(spv::ExecutionMode executionMode);

  // Is this stream an input or output?
  bool mInput;
  // What is the size of this stream (how many elements). Deduced from the execution mode.
  int mSize;
  // What execution mode does this stream use? (e.g. triangles, lines)
  spv::ExecutionMode mExecutionMode;
};

//------------------------------------------------------------------------GeometryFragmentUserData
// Component data added to a geometry fragment type. Needed to get the input/output stream
// types which are only detected when walking the main function.
struct GeometryFragmentUserData
{
  ZilchDeclareType(GeometryFragmentUserData, TypeCopyMode::ReferenceType);
  GeometryFragmentUserData();

  Zero::ZilchShaderIRType* GetInputVertexType();
  Zero::ZilchShaderIRType* GetOutputVertexType();

  Zero::ZilchShaderIRType* mInputStreamType;
  Zero::ZilchShaderIRType* mOutputStreamType;
};

}//namespace Zilch
