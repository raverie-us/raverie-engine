// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

/// Zero's version of the glsl backend. Needed to
/// set specific options on the compiler for zero.
class ZeroZilchShaderGlslBackend : public ZilchShaderGlslBackend
{
public:
  ZeroZilchShaderGlslBackend();

  String GetExtension() override;
  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;
  String GetErrorLog() override;

  int mTargetVersion;
  bool mTargetGlslEs;
  String mErrorLog;
};

} // namespace Zero
