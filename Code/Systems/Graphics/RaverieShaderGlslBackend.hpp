// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Raverie's version of the glsl backend. Needed to
/// set specific options on the compiler for zero.
class RaverieEngineShaderGlslBackend : public RaverieShaderGlslBackend
{
public:
  RaverieEngineShaderGlslBackend();

  String GetExtension() override;
  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;
  String GetErrorLog() override;

  int mTargetVersion;
  bool mTargetGlslEs;
  String mErrorLog;
};

} // namespace Raverie
