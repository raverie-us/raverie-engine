// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

class TrackedFile
{
public:
  TrackedFile() : mVisited(false)
  {
  }

  TrackedFile(FileEntry& entry) :
      mFilename(entry.mFileName),
      mFileSize(GetFileSize(entry.GetFullPath())),
      mLastWrite(GetFileModifiedTime(entry.GetFullPath())),
      mVisited(false)
  {
  }

  size_t Hash() const
  {
    return HashString(mFilename.c_str(), mFilename.SizeInBytes());
  }

  bool operator!=(const TrackedFile& rhs)
  {
    if (mFilename != rhs.mFilename)
      return false;

    if (mFileSize != rhs.mFileSize)
      return false;

    if (mLastWrite != rhs.mLastWrite)
      return false;

    return true;
  }

  String mFilename;
  u64 mFileSize;
  TimeType mLastWrite;
  bool mVisited;
};

OsInt DirectoryWatcher::RunThreadEntryPoint()
{
  // Temporarily disabled because this fails on shutdown on SDL build.
  if (true)
    return 0;

  HashSet<TrackedFile> fileEntries;

  // Loop until cancel
  for (;;)
  {
    // Mark any existing file entries as not visited,
    forRange (TrackedFile& fileEntry, fileEntries)
      fileEntry.mVisited = false;

    // Iterate over the watched directory and get all the file entries
    FileRange dir(mDirectoryToWatch);

    while (!dir.Empty())
    {
      FileEntry currentFile = dir.FrontEntry();
      dir.PopFront();

      TrackedFile fileEntry(currentFile);
      FileOperationInfo info;
      info.FileName = fileEntry.mFilename;

      // See if the file entry is already present
      // Existing entries should be checked for it they were updated
      if (fileEntries.Contains(fileEntry))
      {
        TrackedFile& existingEntry = *fileEntries.FindPointer(fileEntry);
        // If the file already exists mark it as visited to identify removed
        // files
        existingEntry.mVisited = true;
        if (existingEntry != fileEntry)
        {
          // Last write times for file does not match, notify file as modified
          info.Operation = Modified;
          (*mCallback)(mCallbackInstance, info);
          continue;
        }
      }
      // Add the new entry for tracking, notify file was added
      else
      {
        fileEntry.mVisited = true;
        fileEntries.Insert(fileEntry);
        info.Operation = Added;
        (*mCallback)(mCallbackInstance, info);
        continue;
      }
    }

    // Check for entries that are no longer present, notify files as removed
    Array<TrackedFile> toRemove;
    forRange (TrackedFile& fileEntry, fileEntries)
    {
      if (fileEntry.mVisited == false)
      {
        FileOperationInfo info;
        info.FileName = fileEntry.mFilename;
        toRemove.PushBack(fileEntry);
        info.Operation = Removed;
        (*mCallback)(mCallbackInstance, info);
      }
    }

    // Erase the removed files from being tracked
    forRange (TrackedFile& fileEntry, toRemove)
      fileEntries.Erase(fileEntry);
  }
}

} // namespace Zero
