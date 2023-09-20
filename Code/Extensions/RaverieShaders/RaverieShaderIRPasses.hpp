// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class ShaderTranslationPassResult;

/// Base class for all passes that use the optimizer. This handles running the
/// optimizer on the input stream and setting the output stream to the results.
/// Any pass implemention should first set parameters on the optimizer first
/// (such as the performance pass or specialization constant freezing).
class BaseSpirVOptimizerPass : public RaverieShaderIRTranslationPass
{
public:
  BaseSpirVOptimizerPass();
  String GetErrorLog() override;

protected:
  /// Run the optimzer with the given primary pass and flags.
  /// Operates on the given input stream and writes to the given output stream.
  /// Primarily a helper for the several passes that use the optimizer.
  bool RunOptimizer(int primaryPass, Array<String>& flags, ShaderByteStream& inputByteStream, ShaderByteStream& outputByteStream);

  // Helpers

  /// Create the optimizer options given a primary pass and extra flags to
  /// apply.
  void CreateOptimizerOptions(spv_optimizer_options& options, int primaryPass, Array<String>& flags);
  /// Helper to set extra flags on the options (requires low-level c-style
  /// work).
  void SetOptimizerOptionsFlags(spv_optimizer_options& options, Array<String>& flags);
  /// Destroys the optizer options (including the extra flags).
  /// This could change if the optimizer ever gets proper c-api support (this is
  /// a custom implementation).
  void DestroyOptimizerOptions(spv_optimizer_options& options);

public:
  /// The spirv target environment to run.
  int mTargetEnv;
  String mErrorLog;
};

/// Runs the spir-v optimizer tool over the given input data. The input data
/// is expected to be spir-v binary and the output will also be spir-v binary.
class SpirVOptimizerPass : public BaseSpirVOptimizerPass
{
public:
  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;
};

/// Runs the spir-v validator tool over the given input data. The output data
/// and the error log will be filled out with any errors emitted from the
/// validator.
class SpirVValidatorPass : public RaverieShaderIRTranslationPass
{
public:
  SpirVValidatorPass();

  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;
  String GetErrorLog() override;

  // The spirv target environment to run.
  int mTargetEnv;
  String mErrorLog;
};

/// Pass that writes out spir-v binary to a file. Mostly used for debugging
/// but can also be used to cache a final result to disk.
struct SpirVFileWriterPass : public RaverieShaderIRTranslationPass
{
  SpirVFileWriterPass(StringParam targetDirectory);

  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;
  String GetErrorLog() override;

  String mTargetDirectory;
  String mExtension;
};

} // namespace Raverie
