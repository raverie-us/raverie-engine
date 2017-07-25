///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// How long we consider a file as modified (in seconds)
const float cSecondsModified = 0.5f;
const float cCleanTime = 20.0f;
const TimeType cFileIsOpen = 1;

//-------------------------------------------------------------- File Save State


//******************************************************************************
FileModifiedState::FileModifiedState()
{
  mLastCleanup = Time::Clock();

}
//******************************************************************************
bool FileModifiedState::HasModifiedRecently(StringParam filePath)
{
  return HasModifiedSinceTime(filePath, Time::Clock());
}

//******************************************************************************
bool FileModifiedState::HasModifiedSinceTime(StringParam filePath, TimeType time)
{
  FileModifiedState* instance = GetInstance();
  instance->mThreadLock.Lock();

  String normalizedPath = FilePath::Normalize(filePath);
  String canonicalPath = CanonicalizePath(normalizedPath);
  String fileId = UniqueFileId(filePath);

  // Prefer the fileId over the file path
  TimeType modifiedTime = instance->mFileLastModified.FindValue(fileId, instance->mFileLastModified.FindValue(canonicalPath, 0));

  instance->mThreadLock.Unlock();

  float seconds = GetSecondsBetween(modifiedTime, time);
  return (modifiedTime == cFileIsOpen) || (seconds < cSecondsModified);
}

//******************************************************************************
void FileModifiedState::BeginFileModified(StringParam filePath)
{
  FileModifiedState* instance = GetInstance();
  instance->mThreadLock.Lock();

  String normalizedPath = FilePath::Normalize(filePath);
  String canonicalPath = CanonicalizePath(normalizedPath);
  String fileId = UniqueFileId(filePath);

  instance->mFileLastModified.Insert(fileId, cFileIsOpen);
  instance->mFileLastModified.Insert(canonicalPath, cFileIsOpen);

  instance->mThreadLock.Unlock();
}

//******************************************************************************
void FileModifiedState::EndFileModified(StringParam filePath)
{
  FileModifiedState* instance = GetInstance();
  instance->mThreadLock.Lock();

  TimeType currentTime = Time::Clock();
  
  String normalizedPath = FilePath::Normalize(filePath);
  String canonicalPath = CanonicalizePath(normalizedPath);
  String fileId = UniqueFileId(filePath);

  instance->mFileLastModified.Insert(fileId, currentTime);
  instance->mFileLastModified.Insert(canonicalPath, currentTime);

  instance->Cleanup(currentTime);

  instance->mThreadLock.Unlock();
}

//******************************************************************************
FileModifiedState* FileModifiedState::GetInstance()
{
  static FileModifiedState mInstance;
  return &mInstance;
}

//******************************************************************************
float FileModifiedState::GetSecondsBetween(TimeType begin, TimeType end)
{
  return (end - begin) / float(Time::ClocksPerSecond());
}

//******************************************************************************
void FileModifiedState::Cleanup(TimeType currentTime)
{
  // Just an optimization so we aren't checking every file every time we save
  float secondsSinceLastCleanup = GetSecondsBetween(mLastCleanup, currentTime);
  if(secondsSinceLastCleanup < cCleanTime)
    return;

  mLastCleanup = currentTime;

  // Attempt to clean up every file
  ModifiedMap::range r = mFileLastModified.All();
  while(!r.Empty())
  {
    ModifiedMap::value_type current = r.Front();
    String path = current.first;
    TimeType modifiedTime = current.second;
    r.PopFront();

    // We still need to check the time of each file because the file could
    // have been saved right before the cleanup time was up
    float seconds = GetSecondsBetween(modifiedTime, currentTime);
    if(seconds > cCleanTime)
      mFileLastModified.Erase(path);
  }
}

}//namespace Zero
