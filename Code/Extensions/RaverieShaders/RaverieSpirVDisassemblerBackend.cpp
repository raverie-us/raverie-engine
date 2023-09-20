// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieSpirVDisassemblerBackend::RaverieSpirVDisassemblerBackend()
{
  mTargetEnv = SPV_ENV_UNIVERSAL_1_4;
}

String RaverieSpirVDisassemblerBackend::GetExtension()
{
  return "spvtxt";
}

bool RaverieSpirVDisassemblerBackend::RunTranslationPass(ShaderTranslationPassResult& inputData,
                                                       ShaderTranslationPassResult& outputData)
{
  mErrorLog.Clear();

  ShaderByteStream& inputByteStream = inputData.mByteStream;
  uint32_t* data = (uint32_t*)inputByteStream.Data();
  uint32 wordCount = inputByteStream.WordCount();

  spv_text text;
  spv_diagnostic diagnostic = nullptr;
  uint32_t options =
      SPV_BINARY_TO_TEXT_OPTION_NONE | SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES;

  spv_context context = spvContextCreate((spv_target_env)mTargetEnv);
  spv_result_t spvResult = spvBinaryToText(context, data, wordCount, options, &text, &diagnostic);
  spvContextDestroy(context);

  bool success = (spvResult == SPV_SUCCESS);
  if (!success)
  {
    if (diagnostic != nullptr)
      mErrorLog = diagnostic->error;
    return false;
  }

  outputData.mByteStream.Load(text->str, text->length);
  spvTextDestroy(text);
  outputData.mReflectionData = inputData.mReflectionData;

  return success;
}

String RaverieSpirVDisassemblerBackend::GetErrorLog()
{
  return mErrorLog;
}

} // namespace Raverie
