// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

/// Backend that converts the input data to spir-v disassembly. The output
/// data's byte stream will be string data. Mostly used for debugging and unit
/// tests.
class ZilchSpirVDisassemblerBackend : public ZilchShaderIRBackend
{
public:
  ZilchSpirVDisassemblerBackend();

  String GetExtension() override;
  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;
  String GetErrorLog() override;

  // The spirv target environment to run.
  int mTargetEnv;
  String mErrorLog;
};

} // namespace Zero
