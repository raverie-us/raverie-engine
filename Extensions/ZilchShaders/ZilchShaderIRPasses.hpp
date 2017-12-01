///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace spvtools
{
  class Optimizer;
}//namespace spvtools

namespace Zero
{

class ShaderTranslationPassResult;

//-------------------------------------------------------------------BaseSpirVOptimizerPass
/// Base class for all passes that use the optimizer. This handles running the
/// optimizer on the input stream and setting the output stream to the results.
/// Any pass implemention should first set parameters on the optimizer first
/// (such as the performance pass or specialization constant freezing).
class BaseSpirVOptimizerPass : public ZilchShaderIRTranslationPass
{
public:
  BaseSpirVOptimizerPass();
  bool RunOptimizer(spvtools::Optimizer& optimizer, ShaderByteStream& inputByteStream, ShaderByteStream& outputByteStream);
  String GetErrorLog() override;

  // The spirv target environment to run.
  int mTargetEnv;
  String mErrorLog;
};

//-------------------------------------------------------------------SpirVOptimizerPass
/// Runs the spir-v optimizer tool over the given input data. The input data
/// is expected to be spir-v binary and the output will also be spir-v binary.
class SpirVOptimizerPass : public BaseSpirVOptimizerPass
{
public:
  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;
};

//-------------------------------------------------------------------SpirValidatorPass
/// Runs the spir-v validator tool over the given input data. The output data and
/// the error log will be filled out with any errors emitted from the validator.
class SpirVValidatorPass : public ZilchShaderIRTranslationPass
{
public:
  SpirVValidatorPass();

  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;
  String GetErrorLog() override;

  // The spirv target environment to run.
  int mTargetEnv;
  String mErrorLog;
};

//-------------------------------------------------------------------SpirVFileWriterPass
/// Pass that writes out spir-v binary to a file. Mostly used for debugging
/// but can also be used to cache a final result to disk.
struct SpirVFileWriterPass : public ZilchShaderIRTranslationPass
{
  SpirVFileWriterPass(StringParam targetDirectory);

  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;
  String GetErrorLog() override;

  String mTargetDirectory;
  String mExtension;
};

}//namespace Zero
