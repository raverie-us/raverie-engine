// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

String GetApplicationDirectory()
{
  // Use the parent directory of the executable as the application directory.
  return FilePath::GetDirectoryPath(GetApplication());
}

template <typename Function>
bool TryFileOperation(StringParam dest, StringParam source, Function fileOp)
{
  bool result = false;
  // Keep trying to copy the file until we succeed up to some max number of
  // times. This is attempting to deal with random file locks (from anti-virus
  // or something).
  for (size_t i = 0; i < 10; ++i)
  {
    result = fileOp(dest, source);
    if (result == true)
      break;

    Os::Sleep(1);
  }

  // Maybe log some extra error messages here?
  if (result == false)
  {
  }

  return result;
}

bool CopyFile(StringParam dest, StringParam source)
{
  FileModifiedState::BeginFileModified(dest);
  bool result = TryFileOperation(dest, source, CopyFileInternal);
  FileModifiedState::EndFileModified(dest);
  return result;
}

bool MoveFile(StringParam dest, StringParam source)
{
  FileModifiedState::BeginFileModified(dest);
  bool result = TryFileOperation(dest, source, MoveFileInternal);
  FileModifiedState::EndFileModified(dest);
  return result;
}

bool DeleteFile(StringParam dest)
{
  bool result = false;

  FileModifiedState::BeginFileModified(dest);

  // Keep trying to copy the file until we succeed up to some max number of
  // times. This is attempting to deal with random file locks (from anti-virus
  // or something).
  for (size_t i = 0; i < 10; ++i)
  {
    result = DeleteFileInternal(dest);
    if (result == true)
      break;

    Os::Sleep(1);
  }

  FileModifiedState::EndFileModified(dest);

  // Maybe log some extra error messages here?
  if (result == false)
  {
  }

  return result;
}

bool DeleteDirectoryContents(StringParam directory)
{
  if (!DirectoryExists(directory))
    return false;

  bool success = true;

  // RemoveDirectoryW requires the directory to be empty, so we must delete
  // everything in it
  FileRange range(directory);
  for (; !range.Empty(); range.PopFront())
  {
    String name = range.Front();
    String fullName = BuildString(directory, cDirectorySeparatorCstr, name);
    if (DirectoryExists(fullName))
      success &= DeleteDirectory(fullName);
    else
      success &= DeleteFile(fullName);
  }

  return success;
}

bool EnsureEmptyDirectory(StringParam directory)
{
  bool directoryDeleted = DeleteDirectoryContents(directory);
  CreateDirectoryAndParents(directory);
  if (!directoryDeleted)
    return false;
  return true;
}

int CheckFileTime(StringParam dest, StringParam source)
{
  if (!FileExists(dest))
    return -1;
  if (!FileExists(source))
    return +1;

  TimeType destTime = GetFileModifiedTime(dest);
  TimeType sourceTime = GetFileModifiedTime(source);

  if (destTime < sourceTime)
    return -1;
  if (destTime > sourceTime)
    return +1;

  return 0;
}

bool GetFileDateTime(StringParam filePath, CalendarDateTime& result)
{
  if (!FileExists(filePath))
    return false;

  TimeType time = GetFileModifiedTime(filePath);
  tm localTm = *localtime(&time);

  result.Year = localTm.tm_year;
  result.Month = localTm.tm_mon;
  result.Day = localTm.tm_mday;
  result.Hour = localTm.tm_hour;
  result.Minutes = localTm.tm_min;
  result.Seconds = localTm.tm_sec;
  result.Weekday = localTm.tm_wday;
  result.Yearday = localTm.tm_yday;
  result.IsDaylightSavings = localTm.tm_isdst;

  return true;
}

String FindFirstMissingDirectory(StringParam directory)
{
  // Keep iterating over the parent directories until we find one that does
  // exist. When we do return the previous path as this was the first one to not
  // exist.
  String subPath = directory;
  do
  {
    String sourceDirPath = FilePath::GetDirectoryPath(subPath);
    if (DirectoryExists(sourceDirPath))
      return subPath;
    subPath = sourceDirPath;
  } while (!subPath.Empty());

  // Otherwise all of the parent directories don't exist so return an empty
  // string
  return subPath;
}

} // namespace Zero
