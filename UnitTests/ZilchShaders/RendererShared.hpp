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

//-------------------------------------------------------------------ErrorReporter
// A cached message from a compilation or linking log
class LogMessage
{
public:
  LogMessage();
  LogMessage(StringParam filePath, StringParam lineNumber, StringParam message);

  String mFilePath;
  String mLineNumber;
  String mMessage;
};

//-------------------------------------------------------------------ErrorReporter
// Basic class to wrap reporting errors
class ErrorReporter
{
public:
  ErrorReporter();

  void Report(StringParam message);
  void Report(StringParam filePath, StringParam lineNumber, StringParam message);
  void Report(StringParam filePath, StringParam lineNumber, StringParam header, StringParam message);
  void ReportCompilationWarning(StringParam filePath, StringParam lineNumber, StringParam message);
  void ReportCompilationWarning(const Array<LogMessage>& messages);
  void ReportCompilationError(StringParam filePath, StringParam lineNumber, StringParam message);
  void ReportCompilationError(const Array<LogMessage>& messages);
  void ReportLinkerError(StringParam filePath, StringParam lineNumber, StringParam message);
  void ReportPostProcessError(StringParam testName, Vec4Param expected, Vec4Param result, int renderTargetIndex);

  void DisplayDiffs(StringParam expectedFile, StringParam resultFile);

  bool mAssert;
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
//-------------------------------------------------------------------BaseRenderer
// Base renderer type to represent compiling/linking/running shaders
class BaseRenderer
{
public:
  BaseRenderer();

  virtual bool CompileShader(StringParam filePath, StringParam shaderSource, FragmentType::Enum type, ErrorReporter& report) = 0;
  virtual bool CompileAndLinkShader(Array<FragmentInfo>& fragments, ErrorReporter& report) = 0;
  virtual void RunPostProcess(Array<FragmentInfo>& fragments, RenderResult& result, ErrorReporter& report) = 0;

  RenderVertex mFullScreenTriangleVerts[3];
};
