///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

const static size_t mMaxRenderTargets = 4;
const static size_t mScreenWidth = 2;
const static size_t mScreenHeight = 2;
extern String mFragmentExtension;

// The results from one render (all of the render target results)
struct RenderResult
{
  Vec4 mData[mMaxRenderTargets];
};
// A collection of render target results.
struct RenderResults
{
  static String mZilchKey;

  // Which targets were actually used (Cleanup later)
  bool mTargets[mMaxRenderTargets];
  // Map of language to results
  HashMap<String, RenderResult> mLanguageResult;
};

//-------------------------------------------------------------------FragmentInfo
// Basic info for a fragment. Used to pass lots of data around more cleanly
class FragmentInfo
{
public:
  FragmentInfo();
  FragmentInfo(StringParam filePath);
  FragmentInfo(StringParam filePath, StringParam fragmentCode);

  String mFragmentCode;
  String mFilePath;
  FragmentType::Enum mFragmentType;
};

class RenderVertex
{
public:
  Vec3 mPosition;
  Vec2 mUv;
  Vec4 mColor;
};

class UniformByteBuffer
{
public:
  UniformByteBuffer();
  UniformByteBuffer(const UniformByteBuffer& rhs);
  ~UniformByteBuffer();
  UniformByteBuffer& operator=(const UniformByteBuffer& rhs);


  void Set(byte* data, size_t size);
  void CopyFrom(const UniformByteBuffer& rhs);
  void Clear();

  template <typename T>
  void Set(T& data)
  {
    Set((byte*)&data, sizeof(T));
  }

  byte* mData;
  size_t mSize;
};

class UniformBufferData
{
public:
  String mBufferName;
  UniformByteBuffer mBuffer;
};

struct FrameData_Uniforms
{
  float mLogicTime;
  float mFrameTime;
};

struct ObjectTransform_Uniforms
{
  Mat4 mLocalToWorld;
  Mat4 mWorldToView;
};

//-------------------------------------------------------------------BaseRenderer
// Base renderer type to represent compiling/linking/running shaders
class BaseRenderer
{
public:
  BaseRenderer();

  virtual bool CompileShader(StringParam filePath, StringParam shaderSource, FragmentType::Enum type, ErrorReporter& reporter) = 0;
  virtual bool CompileAndLinkShader(Array<FragmentInfo>& fragments, ErrorReporter& reporter) = 0;
  virtual void RunPostProcess(Array<FragmentInfo>& fragments, RenderResult& result, ErrorReporter& reporter) = 0;
  virtual void RunPostProcess(Array<FragmentInfo>& fragments, Array<UniformBufferData>& uniformBuffers, RenderResult& result, ErrorReporter& reporter) = 0;

  RenderVertex mFullScreenTriangleVerts[3];
};

//-------------------------------------------------------------------RendererPackage
// Shared data for unit testing a renderer + backend.
class RendererPackage
{
public:
  RendererPackage();

  typedef Zilch::Ref<ZilchShaderIRBackend> BackendRef;

  BaseRenderer* mRenderer;
  BackendRef mBackend;
  ErrorReporter* mErrorReporter;
};

//-------------------------------------------------------------------UnitTestPackage
// Collection of data used for unit testing.
class UnitTestPackage
{
public:
  typedef Zilch::Ref<ShaderPipelineDescription> PipelineRef;
  typedef Zilch::Ref<ZilchShaderIRTranslationPass> TranslationPassRef;
  typedef Zilch::Ref<ZilchShaderIRBackend> BackendRef;

  // Currently unused
  Array<TranslationPassRef> mToolPasses;
  /// Regular backends to run, mostly during compositing and individual file compilation. 
  Array<BackendRef> mBackends;
  /// Renderer backends to run. These are mainly for testing if the final translation (such as glsl) compiled.
  Array<RendererPackage> mRenderPackages;
};