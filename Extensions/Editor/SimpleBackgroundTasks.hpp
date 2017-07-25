///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Josh Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------DirectorySizeJob
// Computes the size of a directory (which can take a bit of time).
// Set UpdateFrequency to control the frequency of progress reports.
class DirectorySizeJob : public BackgroundTaskJob
{
public:
  typedef DirectorySizeJob ZilchSelf;

  DirectorySizeJob(StringParam directory);

  /// Job Interface.
  int Execute() override;
  void ComputeSizeRecursive(StringParam dir);

  String mDirectory;
  u64 mCurrentSize;
  /// What frequency we should send out progress (every n files send out progress)
  u32 mUpdateFrequency;
  u32 mUpdateCounter;
};

//-------------------------------------------------------------------ExecuteProcess
// Executes any process and writes the output to a specified text saver
class ExecuteProcessTaskJob : public BackgroundTaskJob
{
public:
  typedef DirectorySizeJob ZilchSelf;

  ExecuteProcessTaskJob(StringParam process);

  /// Job Interface.
  int Execute() override;

  String mProcess;
};

}//namespace Zero
