///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

struct spv_diagnostic_t;
typedef spv_diagnostic_t* spv_diagnostic;
struct spv_optimizer_options_t;
typedef spv_optimizer_options_t* spv_optimizer_options;

namespace Zero
{

//-------------------------------------------------------------------ShaderTranslationPassResult
class ShaderTranslationPassResult
{
public:
  String ToString()
  {
    return mByteStream.ToString();
  }
  ShaderByteStream mByteStream;
  ShaderStageInterfaceReflection mReflectionData;

  ZilchRefLink(ShaderTranslationPassResult);
};

//-------------------------------------------------------------------ZilchShaderIRTranslationPass
class ZilchShaderIRTranslationPass : public Zilch::EventHandler
{
public:
  virtual ~ZilchShaderIRTranslationPass() {};

  /// Runs a translation pass that transforms the input data into the output data.
  /// This pass could be something like a tool (e.g. the optimizer) or a backend.
  /// Reflection data will be filled out that describes what transformations took place on the
  /// input data to produce the output data. Most tools will not change the reflection mapping 
  /// (other than removing bindings) but backends may have to do significant transformations.
  virtual bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) = 0;
  
  virtual String GetErrorLog() { return String(); }

  ZilchRefLink(ZilchShaderIRTranslationPass);

protected:
  /// Internal helper that converts a spirv diagnostic object into a string.
  String SpirvDiagnosticToString(spv_diagnostic& diagnostic);
};

//-------------------------------------------------------------------ZilchShaderIRBackend
class ZilchShaderIRBackend : public ZilchShaderIRTranslationPass
{
public:

  /// Return an extension for the given backend. Mostly used for unit
  /// testing so that a backend can be written to a file.
  virtual String GetExtension() = 0;
};

}//namespace Zero
