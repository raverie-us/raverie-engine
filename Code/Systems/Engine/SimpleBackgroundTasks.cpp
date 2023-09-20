// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

DirectorySizeJob::DirectorySizeJob(StringParam directory)
{
  mDirectory = directory;
  mUpdateCounter = 0;
  mUpdateFrequency = 50;
  mCurrentSize = 0;
}

void DirectorySizeJob::Execute()
{
  ComputeSizeRecursive(mDirectory);
  mState = BackgroundTaskState::Completed;
  UpdateProgress(GetName(), 1.0, HumanReadableFileSize(mCurrentSize));
}

void DirectorySizeJob::ComputeSizeRecursive(StringParam dir)
{
  FileRange range(dir);
  for (; !range.Empty(); range.PopFront())
  {
    // If we've been canceled then stop
    if (mState == BackgroundTaskState::Canceled)
      break;

    FileEntry entry = range.FrontEntry();
    String filePath = FilePath::Combine(entry.mPath, entry.mFileName);
    // If this is a directory then recurse
    if (DirectoryExists(filePath))
      ComputeSizeRecursive(filePath);
    else
      mCurrentSize += entry.mSize;

    // Send progress every so often
    ++mUpdateCounter;
    if (mUpdateCounter >= mUpdateFrequency)
    {
      mUpdateCounter = 0;
      // There's not really a good way to know how many files there is so I
      // can't give an actual progress percent (only the current size)
      UpdateProgress(GetName(), 0, HumanReadableFileSize(mCurrentSize));
    }
  }
}

} // namespace Raverie
