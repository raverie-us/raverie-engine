// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "spirv_optimizer_options.h"

namespace Raverie
{

BaseSpirVOptimizerPass::BaseSpirVOptimizerPass()
{
  mTargetEnv = SPV_ENV_UNIVERSAL_1_4;
}

bool BaseSpirVOptimizerPass::RunOptimizer(int primaryPass, Array<String>& flags, ShaderByteStream& inputByteStream, ShaderByteStream& outputByteStream)
{
  bool success = true;

  spv_context context = spvContextCreate((spv_target_env)mTargetEnv);
  spv_diagnostic diagnostic = nullptr;

  // Create the options object with the given pass and flags
  spv_optimizer_options options;
  CreateOptimizerOptions(options, primaryPass, flags);

  // Construct spirv binary data from out byte stream
  uint32_t wordCount = inputByteStream.WordCount();
  uint32_t* code = (uint32_t*)inputByteStream.Data();
  spv_const_binary_t binary{code, wordCount};

  // Run the actual optimizer
  spv_binary binaryOut;
  spv_result_t result = spvOptimizeWithOptions(context, options, &binary, &binaryOut, &diagnostic);

  // On success, load the result into the output stream
  if (result == SPV_SUCCESS)
  {
    success = true;
    outputByteStream.LoadWords(binaryOut->code, binaryOut->wordCount);
  }
  else
  {
    // Otherwise, report the error
    success = false;
    mErrorLog = SpirvDiagnosticToString(diagnostic);
  }

  // Destroy the spirv structures in reverse order (just in case)
  spvBinaryDestroy(binaryOut);
  spvDiagnosticDestroy(diagnostic);
  DestroyOptimizerOptions(options);
  spvContextDestroy(context);
  return success;
}

void BaseSpirVOptimizerPass::CreateOptimizerOptions(spv_optimizer_options& options, int primaryPass, Array<String>& flags)
{
  options = spvOptimizerOptionsCreate();
  // We always assume the validator should never be run (that's a separate
  // pass).
  spvOptimizerOptionsSetRunValidator(options, false);
  // Set the primary pass.
  options->passes_ = (spv_optimizer_passes_t)primaryPass;
  // Set any extra flags
  SetOptimizerOptionsFlags(options, flags);
}

void BaseSpirVOptimizerPass::SetOptimizerOptionsFlags(spv_optimizer_options& options, Array<String>& flags)
{
  size_t flagsCount = flags.Size();
  // Allocate an array of c-strings (one extra for null)
  options->flags_ = new char*[flagsCount + 1];
  options->flags_[flagsCount] = nullptr;
  // Copy over each individual flag
  for (size_t i = 0; i < flagsCount; ++i)
  {
    String& srcFlag = flags[i];
    char*& destFlag = options->flags_[i];
    size_t flagSize = srcFlag.SizeInBytes();
    // Allocate the memory for the destination flag (one extra for null)
    destFlag = new char[flagSize + 1];
    destFlag[flagSize] = 0;
    // Copy the actual string data
    memcpy(destFlag, srcFlag.Data(), flagSize);
  }
}

void BaseSpirVOptimizerPass::DestroyOptimizerOptions(spv_optimizer_options& options)
{
  int flagIndex = 0;
  // Deallocate each individual flag followed by the 'array'.
  while (true)
  {
    char* flag = options->flags_[flagIndex];
    if (flag == nullptr)
      break;

    delete[] flag;
    ++flagIndex;
  }
  delete[] options->flags_;
  options->flags_ = nullptr;

  // Destroy the actual options object
  spvOptimizerOptionsDestroy(options);
}

String BaseSpirVOptimizerPass::GetErrorLog()
{
  return mErrorLog;
}

bool SpirVOptimizerPass::RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData)
{
  Array<String> flags;
  bool success = RunOptimizer(SPV_OPTIMIZER_PERFORMANCE_PASS, flags, inputData.mByteStream, outputData.mByteStream);

  // By default, all of the reflection data is the same as the input stage
  if (success)
    outputData.mReflectionData = inputData.mReflectionData;
  return success;
}

SpirVValidatorPass::SpirVValidatorPass()
{
  mTargetEnv = SPV_ENV_UNIVERSAL_1_4;
}

bool SpirVValidatorPass::RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData)
{
  mErrorLog.Clear();

  bool success = true;

  spv_diagnostic diagnostic = nullptr;
  spv_context context = spvContextCreate((spv_target_env)mTargetEnv);
  spv_validator_options options = spvValidatorOptionsCreate();

  // Construct spirv binary data from out byte stream
  uint32_t wordCount = inputData.mByteStream.WordCount();
  uint32_t* code = (uint32_t*)inputData.mByteStream.Data();
  spv_const_binary_t binary{code, wordCount};

  spv_result_t result = spvValidateWithOptions(context, options, &binary, &diagnostic);

  // If there's failure for any reason
  if (result != SPV_SUCCESS)
  {
    success = false;

    // Store the error log.
    mErrorLog = SpirvDiagnosticToString(diagnostic);
    outputData.mByteStream.Load(mErrorLog);
    outputData.mReflectionData = inputData.mReflectionData;
  }

  // Destroy the spirv structures in reverse order (just in case)
  spvDiagnosticDestroy(diagnostic);
  spvValidatorOptionsDestroy(options);
  spvContextDestroy(context);

  return success;
}

String SpirVValidatorPass::GetErrorLog()
{
  return mErrorLog;
}

SpirVFileWriterPass::SpirVFileWriterPass(StringParam targetDirectory)
{
  mTargetDirectory = targetDirectory;
  mExtension = ".spv";
}

bool SpirVFileWriterPass::RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData)
{
  String typeName = inputData.mReflectionData.mShaderTypeName;
  String filePath = FilePath::CombineWithExtension(mTargetDirectory, typeName, mExtension);

  ShaderByteStream& byteStream = inputData.mByteStream;

  File file;
  file.Open(filePath, Raverie::FileMode::Write, FileAccessPattern::Sequential);
  file.Write(byteStream.Data(), byteStream.ByteCount());
  file.Close();

  return true;
}

String SpirVFileWriterPass::GetErrorLog()
{
  return String();
}

} // namespace Raverie
