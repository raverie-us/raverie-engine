///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// A basic renderer to compile/link/run simple shaders
class GlslRenderer : public BaseRenderer
{
public:
  GlslRenderer(void* context);
  ~GlslRenderer();

  void InitializeSystem(void* context);
  void InitializeBuffers();
  void InitializeTargets();

  bool CompileShader(StringParam filePath, StringParam shaderSource, FragmentType::Enum type, ErrorReporter& reporter) override;
  bool CompileAndLinkShader(Array<FragmentInfo>& fragments, ErrorReporter& reporter) override;
  void RunPostProcess(Array<FragmentInfo>& fragments, RenderResult& result, ErrorReporter& reporter) override;

  // Private interface
  int FragmentTypeToShaderType(FragmentType::Enum fragmentType, ErrorReporter& report);
  bool CompileShaderInternal(StringParam filePath, StringParam shaderSource, int shaderType, int& shaderId, ErrorReporter& report);
  bool LinkInternal(const Array<int>& shaderIds, int& programId, ErrorReporter& reporter);
  bool CompileAndLinkInternal(Array<FragmentInfo>& fragments, int& programId, ErrorReporter& reporter);
  

  unsigned int mTriangleArray;
  unsigned int mTriangleVertex;
  unsigned int mTriangleIndex;
  unsigned int mMultiTargetFbo;
  Array<unsigned int> mTextureIds;
  HashMap<int, int> mZilchFragmentTypeToGlsl;
};