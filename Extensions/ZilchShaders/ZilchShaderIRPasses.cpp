///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------BaseSpirVOptimizerPass
BaseSpirVOptimizerPass::BaseSpirVOptimizerPass()
{
  mTargetEnv = SPV_ENV_UNIVERSAL_1_3;
}

bool BaseSpirVOptimizerPass::RunOptimizer(spvtools::Optimizer& optimizer, ShaderByteStream& inputByteStream, ShaderByteStream& outputByteStream)
{
  mErrorLog.Clear();

  StringBuilder builder;
  // Set the message consumer to just append the message into a string builder
  optimizer.SetMessageConsumer([&builder](spv_message_level_t level, const char* source,
    const spv_position_t& position,
    const char* message) {
    builder.Append(message);
    builder.Append("\n");
  });


  uint32_t* data = (uint32_t*)inputByteStream.Data();
  uint32 wordCount = inputByteStream.WordCount();

  std::vector<uint32_t> optimizerBinaryOutput;
  bool success = optimizer.Run(data, wordCount, &optimizerBinaryOutput);
  if(!success)
  {
    mErrorLog = builder.ToString();
    return false;
  }

  byte* outByteData = (byte*)optimizerBinaryOutput.data();
  size_t outByteSize = optimizerBinaryOutput.size() * 4;
  outputByteStream.Load(outByteData, outByteSize);
  return true;
}

String BaseSpirVOptimizerPass::GetErrorLog()
{
  return mErrorLog;
}

//-------------------------------------------------------------------SpirVOptimizerPass
bool SpirVOptimizerPass::RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData)
{
  spvtools::Optimizer optimizer((spv_target_env)mTargetEnv);

  // Currently the performance pass crashes on several shaders but the size pass doesn't. Temporarily use this?
  optimizer.RegisterSizePasses();

  bool success = RunOptimizer(optimizer, inputData.mByteStream, outputData.mByteStream);
  
  if(success)
    outputData.mReflectionData = inputData.mReflectionData;

  return success;
}

//-------------------------------------------------------------------SpirVValidatorPass
SpirVValidatorPass::SpirVValidatorPass()
{
  mTargetEnv = SPV_ENV_UNIVERSAL_1_3;
}

bool SpirVValidatorPass::RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData)
{
  mErrorLog.Clear();

  spvtools::ValidatorOptions options;
  spvtools::SpirvTools tools((spv_target_env)mTargetEnv);

  StringBuilder builder;
  tools.SetMessageConsumer([&builder](spv_message_level_t level, const char*,
    const spv_position_t& position,
    const char* message) {
    switch(level) {
      case SPV_MSG_FATAL:
      case SPV_MSG_INTERNAL_ERROR:
      case SPV_MSG_ERROR:
        builder.Append("error: ");
        builder.Append(ToString(position.index));
        builder.Append(": ");
        builder.Append(message);
        builder.Append("\n");
        break;
      case SPV_MSG_WARNING:
        builder.Append("warning: ");
        builder.Append(ToString(position.index));
        builder.Append(": ");
        builder.Append(message);
        builder.Append("\n");
        break;
      case SPV_MSG_INFO:
        builder.Append("info: ");
        builder.Append(ToString(position.index));
        builder.Append(": ");
        builder.Append(message);
        builder.Append("\n");
        break;
      default:
        break;
    }
  });

  ShaderByteStream& inputByteStream = inputData.mByteStream;
  uint32_t* data = (uint32_t*)inputByteStream.Data();
  uint32 wordCount = inputByteStream.WordCount();

  bool success = tools.Validate(data, wordCount, options);
  if(!success)
  {
    mErrorLog = builder.ToString();
    outputData.mByteStream.Load(mErrorLog);
    outputData.mReflectionData = inputData.mReflectionData;
  }

  return success;
}

String SpirVValidatorPass::GetErrorLog()
{
  return mErrorLog;
}

//-------------------------------------------------------------------SpirVFileWriterPass
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
  file.Open(filePath, Zero::FileMode::Write, FileAccessPattern::Sequential);
  file.Write(byteStream.Data(), byteStream.ByteCount());
  file.Close();

  return true;
}

String SpirVFileWriterPass::GetErrorLog()
{
  return String();
}

}//namespace Zero
