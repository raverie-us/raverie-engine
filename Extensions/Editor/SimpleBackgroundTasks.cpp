///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Josh Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------DirectorySizeJob
DirectorySizeJob::DirectorySizeJob(StringParam directory)
{
  mDirectory = directory;
  mUpdateCounter = 0;
  mUpdateFrequency = 50;
  mCurrentSize = 0;
}

//******************************************************************************
int DirectorySizeJob::Execute()
{
  ComputeSizeRecursive(mDirectory);
  mState = BackgroundTaskState::Completed;
  UpdateProgress(GetName(), 1.0, HumanReadableFileSize(mCurrentSize));
  return 1;
}

//******************************************************************************
void DirectorySizeJob::ComputeSizeRecursive(StringParam dir)
{
  FileRange range(dir);
  for(; !range.Empty(); range.PopFront())
  {
    // If we've been canceled then stop
    if(mState == BackgroundTaskState::Canceled)
      break;

    FileEntry entry = range.frontEntry();
    String filePath = FilePath::Combine(entry.mPath, entry.mFileName);
    // If this is a directory then recurse
    if(IsDirectory(filePath))
      ComputeSizeRecursive(filePath);
    else
      mCurrentSize += entry.mSize;

    // Send progress every so often
    ++mUpdateCounter;
    if(mUpdateCounter >= mUpdateFrequency)
    {
      mUpdateCounter = 0;
      // There's not really a good way to know how many files there is so I can't
      // give an actual progress percent (only the current size)
      UpdateProgress(GetName(), 0, HumanReadableFileSize(mCurrentSize));
    }
  }
}

//******************************************************************************
ExecuteProcessTaskJob::ExecuteProcessTaskJob(StringParam process)
{
  mProcess = process;
}

//******************************************************************************
int ExecuteProcessTaskJob::Execute()
{
  TextStreamDebugPrint debugPrint;
  SimpleProcess process;

  String name = GetName();
  process.ExecProcess(name.c_str(), mProcess.c_str(), &debugPrint, false);
  process.WaitForClose();
  ZPrintFilter(Filter::DefaultFilter, "\n*** Finished %s ***\n", name.c_str());

  mState = BackgroundTaskState::Completed;
  UpdateProgress(name, 1.0f);

  return 0;
}

}//namespace Zero
