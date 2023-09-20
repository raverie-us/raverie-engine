// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Computes the size of a directory (which can take a bit of time).
// Set UpdateFrequency to control the frequency of progress reports.
class DirectorySizeJob : public BackgroundTaskJob
{
public:
  typedef DirectorySizeJob RaverieSelf;

  DirectorySizeJob(StringParam directory);

  /// Job Interface.
  void Execute() override;
  void ComputeSizeRecursive(StringParam dir);

  String mDirectory;
  u64 mCurrentSize;
  /// What frequency we should send out progress (every n files send out
  /// progress)
  u32 mUpdateFrequency;
  u32 mUpdateCounter;
};

} // namespace Raverie
